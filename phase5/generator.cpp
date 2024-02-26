#include "Tree.h"
#include "generator.h"
#include <iostream>

static std::ostream& operator<<(std::ostream& ostr, Expression* expr);

void Block::generate(){
    for(auto stmt: _stmts)
        stmt->generate();
}

void Simple::generate(){
    _expr->generate();
}

static std::ostream& operator<<(std::ostream& ostr, Expression* expr){
    expr->operand(ostr);
    return ostr;
}

void Expression::operand(std::ostream&  ostr) const{

}

void Number::operand(std::ostream&  ostr) const{
    ostr<<"$"<<_value; 
}

void Identifier::operand(std::ostream&  ostr) const{
    if(_symbol->_offset == 0)
        ostr<<_symbol->name();
    else
        ostr<<_symbol->_offset<<"(%rbp)";
}

void generateGlobals(Scope *scope){
    for(unsigned i = 0; i < scope->symbols().size(); i++){
        if(!scope->symbols().at(i)->type().isFunction())
            std::cout <<".comm\t" << scope->symbols().at(i)->name() << ", " << scope->symbols().at(i)->type().size()<<std::endl;
    }
}

void Function::generate(){
    long offsetCounter = 0;
    Symbols symbols = _body->declarations()->symbols();
    for(auto& symbol: symbols)
    {
        offsetCounter -= symbol->type().size();
        symbol->_offset = offsetCounter;
    }

    if(offsetCounter %16 != 0)
        offsetCounter -= 16 + offsetCounter %16;

    std::cout<<_id->name()<<": \n";
    std::cout<<"pushq\t%rbp\n";
    std::cout<<"movq\t%rsp, %rbp\n";

    std::cout<<"# determining stack size\n";
    std::cout<<"subq\t$"<<-offsetCounter<<", %rsp\n";

    std::cout<<"# storing params on stack\n";
    Parameters* params = _id->type().parameters();
    std::string registers[6] = {"\%edi", "\%esi", "\%edx", "\%ecx", "\%r8d", "\%r9d"};
    for(int i = 0; i < params->size(); i++)
    {
        std::cout<<"movl\t"<<registers[i]<<", "<<symbols[i]->_offset<<"(%rbp)"<<std::endl;
    }

    std::cout<<"# generate body\n";
    _body->generate();

    std::cout<<"movq\t%rbp, %rsp\n";
    std::cout<<"popq\t%rbp\n";
    std::cout<<"ret\n";
    std::cout<<".globl\t"<<_id->name()<<std::endl;

}

void Assignment::generate(){
    std::cout<<"movl\t"<<_right<<", "<<_left<<std::endl;
}

void Call::generate(){
    std::cout<<"# generate call\n";
    std::string registers[6] = {"\%edi", "\%esi", "\%edx", "\%ecx", "\%r8d", "\%r9d"};
    int i;

    for(i = _args.size() - 1; i >= 0; i--)
    {
        std::cout<<"movl\t"<<_args[i]<<", "<<registers[i]<<std::endl;
    }
    std::cout<<"call\t"<<_id->name()<<std::endl;
}