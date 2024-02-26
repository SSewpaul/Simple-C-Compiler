#include "Checker.h"

using namespace std;

void Checker::openScope()
{
    Symbols v;
    Scope* curr = new Scope(v, head);
    head = curr;
}

void Checker::closeScope()
{
    Scope * temp = head->scope();
    delete head;
    head = temp;
}

void Checker::defineFunction(const std::string &name, const Type &type)
{
    // cout << name << ": " << type << endl;
    Scope* curr = head->scope();
    const Symbol* sym = curr->find(name);

    if(sym == nullptr)
        curr->insert(new Symbol(name, type));

    else if(sym->type().parameters() != nullptr)
    {
        curr->remove(name);
        curr->insert(new Symbol(name, type));
        report(redefinition, name);
    }
    else if(sym->type().specifier() != type.specifier() || sym->type().indirection() != type.indirection())
    {
        curr->remove(name);
        curr->insert(new Symbol(name, type));
        report(redeclaration, name);
    }

}

void Checker::declareFunction(const std::string &name, const Type &type)
{
    const Symbol* sym = head->find(name);
    // cout << name << ": " << type << endl;

    if(sym == nullptr)
        head->insert(new Symbol(name, type));

    else if(sym->type() != type)
    {    report(conflicting, name);
        head->remove(name);
        head->insert(new Symbol(name, type));
    }
 
}

void Checker::declareVariable(const std::string &name, const Type &type)
{
    // cout << name << ": " << type << endl;

    const Symbol * sym = head->find(name);

    if(sym == nullptr)
    {
        if(type.specifier() == VOID && type.indirection() == 0)
            report(void_type, name);

        else
            head->insert(new Symbol(name, type));
    }
    else if(head->scope() != nullptr)
        report(redeclaration, name);

    else if(sym->type() != type)
        report(conflicting, name);
    
}

void Checker::checkIdentifier(const std::string &name)
{
    const Symbol* sym = head->lookup(name);

    if (sym == nullptr)
        report(undeclared, name);
}