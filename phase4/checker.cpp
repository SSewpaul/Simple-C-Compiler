/*
 * File:	checker.cpp
 *
 * Description:	This file contains the public and private function and
 *		variable definitions for the semantic checker for Simple C.
 *
 *		If a symbol is redeclared, the redeclaration is discarded
 *		and the original declaration is retained.
 *
 *		Extra functionality:
 *		- inserting an undeclared symbol with the error type
 */

# include <iostream>
# include "lexer.h"
# include "checker.h"
# include "tokens.h"
# include "Symbol.h"
# include "Scope.h"
# include "Type.h"


using namespace std;

static Scope *outermost, *toplevel;
static const Type error;

static string redefined = "redefinition of '%s'";
static string redeclared = "redeclaration of '%s'";
static string conflicting = "conflicting types for '%s'";
static string undeclared = "'%s' undeclared";
static string void_object = "'%s' has type void";
static string invalid_return = "invalid return type";
static string invalid_type = "invalid type for test expression";
static string require_lval = "lvalue required in expression";
static string binary_op = "invalid operands to binary %s";
static string unary_op = "invalid operand to unary %s";
static string not_func = "called object is not a function";
static string invalid_args = "invalid arguments to called function";


/*
 * Function:	openScope
 *
 * Description:	Create a scope and make it the new top-level scope.
 */

Scope *openScope()
{
    toplevel = new Scope(toplevel);

    if (outermost == nullptr)
	outermost = toplevel;

    return toplevel;
}


/*
 * Function:	closeScope
 *
 * Description:	Remove the top-level scope, and make its enclosing scope
 *		the new top-level scope.
 */

Scope *closeScope()
{
    Scope *old = toplevel;
    toplevel = toplevel->enclosing();
    return old;
}


/*
 * Function:	defineFunction
 *
 * Description:	Define a function with the specified NAME and TYPE.  A
 *		function is always defined in the outermost scope.  This
 *		definition always replaces any previous definition or
 *		declaration.
 */

Symbol *defineFunction(const string &name, const Type &type)
{
    //cout << name << ": " << type << endl;
    Symbol *symbol = outermost->find(name);

    if (symbol != nullptr) {
	if (symbol->type().isFunction() && symbol->type().parameters()) {
	    report(redefined, name);
	    delete symbol->type().parameters();

	} else if (type != symbol->type())
	    report(conflicting, name);

	outermost->remove(name);
	delete symbol;
    }

    symbol = new Symbol(name, type);
    outermost->insert(symbol);
    return symbol;
}


/*
 * Function:	declareFunction
 *
 * Description:	Declare a function with the specified NAME and TYPE.  A
 *		function is always declared in the outermost scope.  Any
 *		redeclaration is discarded.
 */

Symbol *declareFunction(const string &name, const Type &type)
{
    //cout << name << ": " << type << endl;
    Symbol *symbol = outermost->find(name);

    if (symbol == nullptr) {
	symbol = new Symbol(name, type);
	outermost->insert(symbol);

    } else if (type != symbol->type()) {
	report(conflicting, name);
	delete type.parameters();

    } else
	delete type.parameters();

    return symbol;
}


/*
 * Function:	declareVariable
 *
 * Description:	Declare a variable with the specified NAME and TYPE.  Any
 *		redeclaration is discarded.
 */

Symbol *declareVariable(const string &name, const Type &type)
{
    //cout << name << ": " << type << endl;
    Symbol *symbol = toplevel->find(name);

    if (symbol == nullptr) {
	if (type.specifier() == VOID && type.indirection() == 0)
	    report(void_object, name);

	symbol = new Symbol(name, type);
	toplevel->insert(symbol);

    } else if (outermost != toplevel)
	report(redeclared, name);

    else if (type != symbol->type())
	report(conflicting, name);

    return symbol;
}


/*
 * Function:	checkIdentifier
 *
 * Description:	Check if NAME is declared.  If it is undeclared, then
 *		declare it as having the error type in order to eliminate
 *		future error messages.
 */

Symbol *checkIdentifier(const string &name)
{
    Symbol *symbol = toplevel->lookup(name);

    if (symbol == nullptr) {
	report(undeclared, name);
	symbol = new Symbol(name, error);
	toplevel->insert(symbol);
    }

    return symbol;
}

Type checkLogicalOr(const Type& left, const Type& right)
{
    if (left == error || right == error)
        return error;

    Type lhs = left.promote();
    Type rhs = right.promote();
    
    if(lhs.isPredicate() && rhs.isPredicate())
    {
        return Type(INT);
    }
    else{
        report(binary_op, "||");
        return Type();
    }
}

Type checkLogicalAnd(const Type& left, const Type& right)
{
    if (left == error || right == error)
        return error;
    
    Type lhs = left.promote();
    Type rhs = right.promote();

    if(lhs.isPredicate() && rhs.isPredicate())
    {
        return Type(INT);
    }
    else{
        report(binary_op,"&&");
        return Type();
    }
}



Type checkIfWhileFor(const Type& expr)
{
    if (expr == error)
        return error;

    else if(expr.isPredicate())
        return expr;
    else
    {
        report(invalid_type);
        return Type();
    }
}

Type checkNot(const Type& expr)
{
    if (expr == error)
        return error;
    else if(expr.isPredicate())
        return Type(INT);
    else
    {
        report(unary_op, "!");
        return Type();
    }
}

Type checkMulDivRem(const Type& left, const Type& right, const string& s)
{
    if (left == error || right == error)
        return error;

    if(!left.isNumeric() && !right.isNumeric())
    {
        report(binary_op, s);
        return Type();
    }
    else if(left.specifier() == LONG || right.specifier() == LONG)
        return Type(LONG);
    else
        return Type(INT);
}


Type checkNeg(const Type& left)
{
    if (left == error)
        return error;

    if(!left.isNumeric())
    {
        report(unary_op, "-");
        return Type();
    }
    else if(left.specifier() == LONG)
        return Type(LONG);
    else
        return Type(INT);
}

Type checkRelationalExpr(const Type& left, const Type& right, const string& s)
{
    if (left == error || right == error)
        return error;

    Type lhs = left.promote();
    Type rhs = right.promote();

    if(lhs == rhs || (lhs.isNumeric() && rhs.isNumeric()))
    {
        if(lhs.isPredicate() && rhs.isPredicate())
        {
            return Type(INT);
        }
    }
    
    report(binary_op, s);
    return Type();
}

Type checkEquality(const Type& left, const Type& right, const string& s)
{
    if (left == error || right == error)
        return error;

    if(left.isCompatibleWith(right))
        return Type(INT);
    else
    {
        report(binary_op, s);
        return Type();
    }
}

Type checkPlus(const Type& left, const Type& right)
{
    if (left == error || right == error)
        return error;

    Type lhs = left.promote();
    Type rhs = right.promote();

    if(lhs.isNumeric() && right.isNumeric()){
        if(lhs.specifier() == LONG)
            return Type(LONG);
        else if(lhs.specifier() == INT)
            return Type(INT);
    }
    else if((lhs.isPointer() && !(lhs.specifier() == VOID && lhs.indirection() == 1)) && rhs.isNumeric())
            return Type(lhs.specifier(), 1);
    else if((rhs.isPointer() && !(rhs.specifier() == VOID && rhs.indirection() == 1)) && lhs.isNumeric())
            return Type(rhs.specifier(), 1);
    report(binary_op, "+");
    return Type();
}

Type checkSub(const Type& left, const Type& right)
{
    if (left == error || right == error)
        return error;

    Type lhs = left.promote();
    Type rhs = right.promote();

    if(lhs.isNumeric() && right.isNumeric()){
        if(lhs.specifier() == LONG)
            return Type(LONG);
        else if(lhs.specifier() == INT)
            return Type(INT);
    }
    else if((lhs.isPointer() && !(lhs.specifier() == VOID && lhs.indirection() == 1)) && rhs.isNumeric())
            return Type(lhs.specifier(), 1);
    else if((rhs.isPointer() && !(rhs.specifier() == VOID && rhs.indirection() == 1)) && (lhs.isPointer() && !(lhs.specifier() == VOID && lhs.indirection() == 1)) && (lhs.specifier() == rhs.specifier()))
            return Type(LONG);
    report(binary_op, "-");
    return Type();
}

Type checkDeref(const Type& expr)
{
    if (expr == error)
        return error;

    Type val = expr.promote();

    if (val.isPointer() && !(val.specifier() == VOID && val.indirection() == 1))
    {
        return Type(val.specifier(), val.indirection() - 1);
    }
    else{
        report(unary_op, "*");
        return Type();
    }
}

Type checkAddr(const Type& expr, const bool& lvalue)
{
    if (expr == error)
        return error;

    if(!lvalue)
    {
        //cout<<"addr:"<<expr<<endl;
        report(require_lval);
        return Type();
    }

    //Type val = expr.promote();

    if (expr != Type())
    {
        return Type(expr.specifier(), 1);
    }
    else{
        report(unary_op,"&");
        return Type();
    }
}

Type checkIndex(const Type& left, const Type& expr)
{
    if (left == error || expr == error)
        return error;

    Type lhs= left.promote();
    Type rhs = expr.promote();

    if((lhs.isPointer() && !(lhs.specifier() == VOID && lhs.indirection() == 1)) && rhs.isNumeric())
        return Type(lhs.specifier(), lhs.indirection() - 1);
    report(binary_op,"[]");
    return Type();
}

Type checkSizeof(const Type& expr)
{
    if (expr == error)
        return error;

    Type lhs = expr.promote();

    if (lhs.isPredicate())
        return Type(LONG);
    report(unary_op,"sizeof");
    return Type();
}

Type checkFunction(const Type& ftype, const Parameters* params)
{
    if(ftype == Type() || !ftype.isFunction())
    {
        report(not_func);
        return Type();
    }
    else if(ftype.parameters() == nullptr)
    {
        for(auto it = params->begin(); it < params->end(); it++)
        {
            Type tmp = it->promote();

            if(tmp == Type() || !tmp.isPredicate())
            {
                report(invalid_args);
                return Type();
            }
        }
    }
    else{

        if (ftype.parameters()->size() != params->size())
        {
            report(invalid_args);
            return Type();
        }
        else
        {
            for(unsigned i = 0; i < params->size(); i++)
            {
                Type tmp = params->at(i).promote();
                Type refparam = ftype.parameters()->at(i).promote();

                if(tmp == Type() || !tmp.isCompatibleWith(refparam))
                {
                    report(invalid_args);
                    return Type();
                }
            }
        }
    }

    return Type(ftype.specifier(), ftype.indirection());
}

Type checkReturn(const Type& left, const Type& expr){
    if(left == Type() || expr == Type())
        return error;

    if(!left.isCompatibleWith(expr))
    {
        report(invalid_return);
        return Type();
    }
    return left;
}

void checkAssignment(const Type& left, const Type& right, const bool& lvalue)
{
    if(left == Type() || right == Type())
        return;

    if(!lvalue)
    {
        //cout<<"assignment: left: "<<left<<"right: "<<right<<endl;
        report(require_lval);
        return;
    }
    
    Type lhs = left.promote();
    Type rhs = right.promote();

    if(!lhs.isCompatibleWith(rhs))
    {
        report(binary_op, "=");
    }

}