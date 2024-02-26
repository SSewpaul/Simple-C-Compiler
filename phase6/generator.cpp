/*
 * File:	generator.cpp
 *
 * Description:	This file contains the public and member function
 *		definitions for the code generator for Simple C.
 *
 *		Extra functionality:
 *		- putting all the global declarations at the end
 */

# include <vector>
# include <cassert>
# include <iostream>
# include <map>
# include "generator.h"
# include "machine.h"
# include "Tree.h"
# include "Label.h"
# include "string.h"

using namespace std;

static int offset;
static string funcname;
static string suffix(Expression *expr);
static ostream &operator <<(ostream &ostr, Expression *expr);

static Register *rax = new Register("%rax", "%eax", "%al");
static Register *rbx = new Register("%rbx", "%ebx", "%bl");
static Register *rcx = new Register("%rcx", "%ecx", "%cl");
static Register *rdx = new Register("%rdx", "%edx", "%dl");
static Register *rsi = new Register("%rsi", "%esi", "%sil");
static Register *rdi = new Register("%rdi", "%edi", "%dil");
static Register *r8 = new Register("%r8", "%r8d", "%r8b");
static Register *r9 = new Register("%r9", "%r9d", "%r9b");
static Register *r10 = new Register("%r10", "%r10d", "%r10b");
static Register *r11 = new Register("%r11", "%r11d", "%r11b");
static Register *r12 = new Register("%r12", "%r12d", "%r12b");
static Register *r13 = new Register("%r13", "%r13d", "%r13b");
static Register *r14 = new Register("%r14", "%r14d", "%r14b");
static Register *r15 = new Register("%r15", "%r15d", "%r15b");

static vector<Register *> parameters = {rdi, rsi, rdx, rcx, r8, r9};
static vector<Register *> registers = {rax, rdi, rsi, rdx, rcx, r8, r9, r10, r11};
static map<string, Label> strings;

/* These will be replaced with functions in the next phase.  They are here
   as placeholders so that Call::generate() is finished. */

static void assign(Expression *expr, Register *reg)
{
    if (expr != nullptr)
    {
        if(expr->_register != nullptr)
            expr->_register->_node = nullptr;
        
        expr->_register = reg;
    }

    if(reg != nullptr)
    {
        if(reg->_node != nullptr)
            reg->_node->_register = nullptr;
        
        reg->_node = expr;
    }
}

static void load(Expression *expr, Register *reg)
{
    if (reg->_node != expr)
    {
        if (reg->_node != nullptr){
            unsigned size = reg -> _node ->type().size();
            offset -= size;
            reg->_node->_offset = offset;
            cout << "\tmov" << suffix(reg->_node);
            cout << reg->name(size) << ", ";
            cout << offset << "(%rbp)" << endl;
        }

        if(expr != nullptr)
        {
            unsigned size = expr->type().size();
            cout << "\tmov" << suffix(expr) << expr;
            cout << ", " << reg->name(size) << endl;
        }

        assign(expr, reg);
    }
}

static Register* getreg()
{
    for(auto reg : registers)
    {
        if(reg->_node == nullptr)
            return reg;
    }

    abort();
}

/*
 * Function:	suffix (private)
 *
 * Description:	Return the suffix for an opcode based on the given size.
 */

static string suffix(unsigned long size)
{
    return size == 1 ? "b\t" : (size == 4 ? "l\t" : "q\t");
}


/*
 * Function:	suffix (private)
 *
 * Description:	Return the suffix for an opcode based on the size of the
 *		given expression.
 */

static string suffix(Expression *expr)
{
    return suffix(expr->type().size());
}


/*
 * Function:	align (private)
 *
 * Description:	Return the number of bytes necessary to align the given
 *		offset on the stack.
 */

static int align(int offset)
{
    if (offset % STACK_ALIGNMENT == 0)
	return 0;

    return STACK_ALIGNMENT - (abs(offset) % STACK_ALIGNMENT);
}


/*
 * Function:	operator << (private)
 *
 * Description:	Convenience function for writing the operand of an
 *		expression using the output stream operator.
 */

static ostream &operator <<(ostream &ostr, Expression *expr)
{
    if (expr->_register != nullptr)
	return ostr << expr->_register;

    expr->operand(ostr);
    return ostr;
}


/*
 * Function:	Expression::operand
 *
 * Description:	Write an expression as an operand to the specified stream.
 */

void Expression::operand(ostream &ostr) const
{
    assert(_offset != 0);
    ostr << _offset << "(%rbp)";
}


/*
 * Function:	Identifier::operand
 *
 * Description:	Write an identifier as an operand to the specified stream.
 */

void Identifier::operand(ostream &ostr) const
{
    if (_symbol->_offset == 0)
	ostr << global_prefix << _symbol->name() << global_suffix;
    else
	ostr << _symbol->_offset << "(%rbp)";
}


/*
 * Function:	Number::operand
 *
 * Description:	Write a number as an operand to the specified stream.
 */

void Number::operand(ostream &ostr) const
{
    ostr << "$" << _value;
}


/*
 * Function:	Call::generate
 *
 * Description:	Generate code for a function call expression.
 *
 *		On a 64-bit platform, the stack needs to be aligned on a
 *		16-byte boundary.  So, if the stack will not be aligned
 *		after pushing any arguments, we first adjust the stack
 *		pointer.
 *
 *		Since all arguments are 8-bytes wide, we could simply do:
 *
 *		    if (args.size() > 6 && args.size() % 2 != 0)
 *			subq $8, %rsp
 */

void Call::generate()
{
    unsigned numBytes;


    /* Generate code for the arguments first. */ 

    numBytes = 0;

    for (int i = _args.size() - 1; i >= 0; i --)
	_args[i]->generate();


    /* Adjust the stack if necessary */

    if (_args.size() > NUM_PARAM_REGS) {
	numBytes = align((_args.size() - NUM_PARAM_REGS) * SIZEOF_PARAM);

	if (numBytes > 0)
	    cout << "\tsubq\t$" << numBytes << ", %rsp" << endl;
    }


    /* Move the arguments into the correct registers or memory locations. */

    for (int i = _args.size() - 1; i >= 0; i --) {
	if (i >= NUM_PARAM_REGS) {
	    numBytes += SIZEOF_PARAM;
	    load(_args[i], rax);
	    cout << "\tpushq\t%rax" << endl;

	} else
	    load(_args[i], parameters[i]);

	assign(_args[i], nullptr);
    }


    /* Call the function and then reclaim the stack space.  Technically, we
       only need to assign the number of floating point arguments passed in
       vector registers to %eax if the function being called takes a
       variable number of arguments.  But, it never hurts. */

    for (auto reg : registers)
	load(nullptr, reg);

    if (_id->type().parameters() == nullptr)
	cout << "\tmovl\t$0, %eax" << endl;

    cout << "\tcall\t" << global_prefix << _id->name() << endl;

    if (numBytes > 0)
	cout << "\taddq\t$" << numBytes << ", %rsp" << endl;

    assign(this, rax);
}


/*
 * Function:	Block::generate
 *
 * Description:	Generate code for this block, which simply means we
 *		generate code for each statement within the block.
 */

void Block::generate()
{
    for (auto stmt : _stmts) {
	stmt->generate();

	for (auto reg : registers)
	    assert(reg->_node == nullptr);
    }
}


/*
 * Function:	Simple::generate
 *
 * Description:	Generate code for a simple (expression) statement, which
 *		means simply generating code for the expression.
 */

void Simple::generate()
{
    _expr->generate();
    assign(_expr, nullptr);
}


/*
 * Function:	Function::generate
 *
 * Description:	Generate code for this function, which entails allocating
 *		space for local variables, then emitting our prologue, the
 *		body of the function, and the epilogue.
 */

void Function::generate()
{
    int param_offset;
    unsigned size;
    Parameters *params;
    Symbols symbols;


    /* Assign offsets to the parameters and local variables. */

    param_offset = 2 * SIZEOF_REG;
    offset = param_offset;
    allocate(offset);


    /* Generate our prologue. */

    funcname = _id->name();
    cout << global_prefix << funcname << ":" << endl;
    cout << "\tpushq\t%rbp" << endl;
    cout << "\tmovq\t%rsp, %rbp" << endl;
    cout << "\tmovl\t$" << funcname << ".size, %eax" << endl;
    cout << "\tsubq\t%rax, %rsp" << endl;


    /* Spill any parameters. */

    params = _id->type().parameters();
    symbols = _body->declarations()->symbols();

    for (unsigned i = 0; i < NUM_PARAM_REGS; i ++)
	if (i < params->size()) {
	    size = symbols[i]->type().size();
	    cout << "\tmov" << suffix(size) << parameters[i]->name(size);
	    cout << ", " << symbols[i]->_offset << "(%rbp)" << endl;
	} else
	    break;


    /* Generate the body of this function. */

    _body->generate();


    /* Generate our epilogue. */

    cout << endl << global_prefix << funcname << ".exit:" << endl;
    cout << "\tmovq\t%rbp, %rsp" << endl;
    cout << "\tpopq\t%rbp" << endl;
    cout << "\tret" << endl << endl;

    offset -= align(offset - param_offset);
    cout << "\t.set\t" << funcname << ".size, " << -offset << endl;
    cout << "\t.globl\t" << global_prefix << funcname << endl << endl;
}


/*
 * Function:	generateGlobals
 *
 * Description:	Generate code for any global variable declarations.
 */

void generateGlobals(Scope *scope)
{
    const Symbols &symbols = scope->symbols();

    for (auto symbol : symbols)
	if (!symbol->type().isFunction()) {
	    cout << "\t.comm\t" << global_prefix << symbol->name() << ", ";
	    cout << symbol->type().size() << endl;
	}

    if (strings.size() > 0){
        cout << "\t.data\n";
        for (auto str : strings)
            cout << str.second << ":\t.asciz\t\"" << escapeString(str.first) << "\"\n";
    }
}


/*
 * Function:	Assignment::generate
 *
 * Description:	Generate code for an assignment statement.
 *
 *		NOT FINISHED: Only works if the right-hand side is an
 *		integer literal and the left-hand side is an integer
 *		scalar.
 */

void Assignment::generate()
{
    Expression * pointer;

    _right->generate();
    // assert(dynamic_cast<Number *>(_right));
    // assert(dynamic_cast<Identifier *>(_left));

    if(_left->isDereference(pointer))
    {
        pointer->generate();

        if(pointer->_register == nullptr)
            load(pointer, getreg());

        if  (_right->_register == nullptr)
            load(_right, getreg());

        cout << "\tmov" << suffix(_left) << _right << ", (" << pointer << ")\n";

        assign(pointer, nullptr);
    }
    else{
        unsigned long val;
        if(!_right->isNumber(val))
        {
                load(_right, getreg());
        }
        
        cout << "\tmov" << suffix(_right) << _right;
        cout << ", " << _left << endl;
    }
    assign(_right, nullptr);
}

/* Add, mul, sub  computation */
static void compute(Expression* result, Expression* left, Expression* right, const string& opcode)
{
    left->generate();
    right->generate();

    if(left->_register == nullptr){
        load(left, getreg());
    }

    cout << "\t" << opcode << suffix(result) << right;
    cout << ", " << left << endl;

    assign(right, nullptr);
    assign(result, left->_register);
}

void Add::generate()
{
    compute(this, _left, _right, "add");
}

void Subtract::generate()
{
    compute(this, _left, _right, "sub");
}

void Multiply::generate()
{
    compute(this, _left, _right, "imul");
}

/* Div and Rem compuation */
static void computeDiv(Expression* result, Expression* left, Expression* right, const string& op)
{
    left->generate();
    right->generate();

    load(left, rax);
    load(nullptr, rdx);
    load(right, rcx);

    if(result->type().size() == 4)
        cout << "\tcltd\n";
    else
        cout << "\tcqto\n";

    cout << "\tidiv" << suffix(result) << right << endl;

    assign(left, nullptr);
    assign(right, nullptr);

    if(op == "div")
        assign(result, rax);
    else
        assign(result, rdx);
}


void Divide::generate()
{
    computeDiv(this, _left, _right, "div");
}

void Remainder::generate()
{
    computeDiv(this, _left, _right, "rem");
}

/* Relational and Equality Operators */
static void computeComp(Expression* result, Expression* left, Expression* right, const string& op)
{
    left->generate();
    right->generate();

    if(left->_register == nullptr)
        load(left, getreg());
    cout << "\tcmp" << suffix(left) << right << ", " << left->_register << endl;

    assign(left, nullptr);
    assign(right, nullptr);

    assign(result, getreg());

    cout << "\tset" << op << "\t" << result->_register->byte() << endl;
    cout << "\tmovzbl\t" << result->_register->byte() << ", " << result->_register->name(4) << endl;
}

void LessThan::generate()
{
    computeComp(this, _left, _right, "l");
}

void GreaterThan::generate()
{
    computeComp(this, _left, _right, "g");
}

void LessOrEqual::generate()
{
    computeComp(this, _left, _right, "le");
}

void GreaterOrEqual::generate()
{
    computeComp(this, _left, _right, "ge");
}

void Equal::generate()
{
    computeComp(this, _left, _right, "e");
}

void NotEqual::generate()
{
    computeComp(this, _left, _right, "ne");
}

/* unary not and neg */
void Not::generate()
{
    _expr->generate();

    if(_expr->_register == nullptr)
        load(_expr, getreg());

    cout << "\tcmp" << suffix(_expr) << "$0, " << _expr << endl;
    cout << "\tsete\t" << _expr->_register->byte() << endl;
    cout << "\tmovzbl\t" << _expr->_register->byte() << ", " << _expr->_register << endl;

    assign(this, _expr->_register);
}

void Negate::generate()
{
    _expr->generate();
    if(_expr->_register == nullptr)
        load(_expr, getreg());

    cout << "\tneg" <<  suffix(_expr) << _expr->_register << endl;

    assign(this, _expr->_register);
}


void Expression::test(const Label& label, bool ifTrue){
    generate();

    if(_register == nullptr)
        load(this, getreg());

    cout << "\tcmp" << suffix(this) << "$0, " << this << endl;
    cout << (ifTrue ? "\tjne\t" : "\tje\t") << label << endl;

    assign(this, nullptr);
}

void While::generate()
{
    cout << "# while" <<endl;
    Label loop, exit;
    cout << loop << ":" << endl;

    _expr->test(exit, false);
    _stmt->generate();

    cout << "\tjmp\t" << loop << endl;
    cout << exit << ":" << endl;
}

void Address::generate(){

    Expression *pointer;

    if(_expr->isDereference(pointer)) {
        pointer->generate();

        if(pointer->_register == nullptr)
            load(pointer, getreg());
        
        assign(this, pointer->_register);
    }
    else {
        assign(this, getreg());
        cout << "\tleaq\t" << _expr << ", " << this <<endl;
    }
}

void Dereference::generate(){
    _expr -> generate();

    if(_expr->_register == nullptr)
        load(_expr, getreg());
    
    cout << "\tmov" << suffix(this->type().size()) << "(" << _expr << "), " << _expr->_register->name(this->type().size()) <<endl;

    assign(this, _expr->_register);
}

void String::operand(ostream &ostr) const{

    auto val = strings.find(value());

    if (val == strings.end()){
        auto label = strings.insert({value(), Label()});
        ostr << label.first->second;
    }
    else{
        ostr << val->second;
    }
}

void Return::generate(){
    _expr->generate();

    load(_expr, rax);

    cout << "\tjmp\t" << funcname << ".exit\n";

    assign(_expr, nullptr);
}

void Cast::generate(){
    Register* reg;
    unsigned source, target;

    source = _expr->type().size();
    target = _type.size();

    _expr->generate();

    if(source >= target)
    {
        load(_expr, getreg());
        assign(this, _expr->_register);
    }
    else {

        if(_expr->_register == nullptr)
            load(_expr, getreg());

        if(source == 1 &&  target == 4){
            cout << "\tmovsbl\t" << _expr->_register << ", " << _expr->_register->name(target) << endl;
        }
        else if(source == 1 && target == 8){
            cout << "\tmovsbq\t" << _expr->_register << ", " << _expr->_register->name(target) << endl;
        }
        else{
            cout << "\tmovslq\t" << _expr->_register << ", " << _expr->_register->name(target) << endl;
        }

        assign(this, _expr->_register);
    }
}

void LogicalOr::generate(){
    cout << "# or" <<endl;
    auto success = Label();
    auto exit = Label();

    assign(this, getreg());

    _left->test(success, true);
    _right->test(success, true);

    cout << "\tmov" << suffix(this) << "$0, " << this << endl;
    cout << "\tjmp\t" << exit << endl;
    cout << success << ":\n" << "\tmov" << suffix(this) << "$1, " << this << endl;
    cout << exit << ":\n";
}

void LogicalAnd::generate(){
    auto success = Label();
    auto exit = Label();

    assign(this, getreg());

    _left->test(exit, false);
    _right->test(exit, false);
    cout << "\tmov" << suffix(this) << "$1, " << this << endl;
    cout << "\tjmp\t" << success << endl;

    cout << exit << ":\n" << "\tmov" << suffix(this) << "$0, " << this << endl;
    cout << success << ":\n";
}

void For::generate(){
    cout << "# for" <<endl;
    Label loop, exit;
    _init->generate();
    cout << loop << ":" << endl;
    _expr->test(exit, false);
    _stmt->generate();
    _incr->generate();

    cout << "\tjmp\t" << loop << endl;
    cout << exit << ":" << endl;
}

void If::generate(){
    cout << "# if" <<endl;
    Label then, elseblk, exit;

    _expr->test(then, true);
    cout << "\tjmp\t" << elseblk << endl;

    cout << then << ":\n";
    cout << "# then" <<endl;
    _thenStmt->generate();
    cout << "\tjmp\t" << exit << endl;

    cout << elseblk << ":\n";
    cout << "# else" <<endl;
    if(_elseStmt != nullptr)
        _elseStmt->generate();
    
    cout << "\tjmp\t" << exit << endl;

    cout << exit << ":\n";

}