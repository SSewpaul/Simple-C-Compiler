/*
 * File:        parser.cpp
 *
 * Description: This file contains the public and private function and
 *              variable definitions for the recursive-descent parser for
 *              Simple C.
 */

# include <cstdlib>
# include <iostream>
# include "tokens.h"
# include "lexer.h"
# include "Checker.h"
# include "Type.h"

using namespace std;

static int lookahead;
static string lexbuf;

static void expression();
static void statement();

Checker* checker = new Checker();

/*
 * Function:    error
 *
 * Description: Report a syntax error to standard error.
 */

static void error()
{
	if (lookahead == DONE)
		report("syntax error at end of file");
	else
		report("syntax error at '%s'", lexbuf);

	exit(EXIT_FAILURE);
}


/*
 * Function:    match
 *
 * Description: Match the next token against the specified token.  A
 *              failure indicates a syntax error and will terminate the
 *              program since our parser does not do error recovery.
 */

static void match(int t)
{
	if (lookahead != t)
		error();

	lookahead = yylex();
	lexbuf = yytext;
}


/*
 * Function:    isSpecifier
 *
 * Description: Return whether the given token is a type specifier.
 */

static bool isSpecifier(int token)
{
	return token == INT || token == CHAR || token == LONG || token == VOID;
}


/*
 * Function:    specifier
 *
 * Description: Parse a type specifier.  Simple C has only ints, chars,
 *              longs,and void types.
 *
 *              specifier:
 *                int
 *                char
 *                long
 *                void
 */

int specifier()
{
		int typespec = lookahead;

	if (isSpecifier(lookahead))
		match(lookahead);
	else
		error();

		return typespec;
}


/*
 * Function:    pointers
 *
 * Description: Parse pointer declarators (i.e., zero or more asterisks).
 *
 *              pointers:
 *                empty
 *                * pointers
 */

unsigned pointers()
{
		unsigned count = 0;
	while (lookahead == '*')
		{
				match('*');
				count ++;
		}

		return count;
}


/*
 * Function:    declarator
 *
 * Description: Parse a declarator, which in Simple C is either a scalar
 *              variable or an array, with optional pointer declarators.
 *
 *              declarator:
 *                pointers identifier
 *                pointers identifier [ num ]
 */

static void declarator(int typespec)
{
	unsigned indirection = pointers();
	string id = lexbuf;
	match(ID);

	if (lookahead == '[') {
		match('[');
		char * p;
		unsigned long len = strtoul(lexbuf.c_str(), &p , 10);
		match(NUM);
		match(']');
		Type var(typespec, indirection, len);
		checker->declareVariable(id, var);
	}
	else{
		Type var(typespec, indirection);
		checker->declareVariable(id, var);
	}
}


/*
 * Function:    declaration
 *
 * Description: Parse a local variable declaration.  Global declarations
 *              are handled separately since we need to detect a function
 *              as a special case.
 *
 *              declaration:
 *                specifier declarator-list ';'
 *
 *              declarator-list:
 *                declarator
 *                declarator , declarator-list
 */

static void declaration()
{
	int typespec = specifier();
	declarator(typespec);

	while (lookahead == ',') {
		match(',');
		declarator(typespec);
	}

	match(';');
}


/*
 * Function:    declarations
 *
 * Description: Parse a possibly empty sequence of declarations.
 *
 *              declarations:
 *                empty
 *                declaration declarations
 */

static void declarations()
{
	while (isSpecifier(lookahead))
		declaration();
}


/*
 * Function:    primaryExpression
 *
 * Description: Parse a primary expression.
 *
 *              primary-expression:
 *                ( expression )
 *                identifier ( expression-list )
 *                identifier ( )
 *                identifier
 *                character
 *                string
 *                num
 *
 *              expression-list:
 *                expression
 *                expression , expression-list
 */

static void primaryExpression()
{
	if (lookahead == '(') {
		match('(');
		expression();
		match(')');

	} else if (lookahead == CHARACTER) {
		match(CHARACTER);

	} else if (lookahead == STRING) {
		match(STRING);

	} else if (lookahead == NUM) {
		match(NUM);

	} else if (lookahead == ID) {
		string id = lexbuf;       
		match(ID);

		if (lookahead == '(') {
			match('(');

			if (lookahead != ')') {
				expression();

				while (lookahead == ',') {
					match(',');
					expression();
				}
			}

			match(')');
		}
		checker->checkIdentifier(id);

	} else
		error();
}


/*
 * Function:    postfixExpression
 *
 * Description: Parse a postfix expression.
 *
 *              postfix-expression:
 *                primary-expression
 *                postfix-expression [ expression ]
 */

static void postfixExpression()
{
	primaryExpression();

	while (lookahead == '[') {
		match('[');
		expression();
		match(']');
		cout << "index" << endl;
	}
}


/*
 * Function:    prefixExpression
 *
 * Description: Parse a prefix expression.
 *
 *              prefix-expression:
 *                postfix-expression
 *                ! prefix-expression
 *                - prefix-expression
 *                * prefix-expression
 *                & prefix-expression
 *                sizeof prefix-expression
 */

static void prefixExpression()
{
	if (lookahead == '!') {
		match('!');
		prefixExpression();
		cout << "not" << endl;

	} else if (lookahead == '-') {
		match('-');
		prefixExpression();
		cout << "neg" << endl;

	} else if (lookahead == '*') {
		match('*');
		prefixExpression();
		cout << "deref" << endl;

	} else if (lookahead == '&') {
		match('&');
		prefixExpression();
		cout << "addr" << endl;

	} else if (lookahead == SIZEOF) {
		match(SIZEOF);
		prefixExpression();
		cout << "sizeof" << endl;

	} else
		postfixExpression();
}


/*
 * Function:    multiplicativeExpression
 *
 * Description: Parse a multiplicative expression.  Simple C does not have
 *              cast expressions, so we go immediately to prefix
 *              expressions.
 *
 *              multiplicative-expression:
 *                prefix-expression
 *                multiplicative-expression * prefix-expression
 *                multiplicative-expression / prefix-expression
 *                multiplicative-expression % prefix-expression
 */

static void multiplicativeExpression()
{
	prefixExpression();

	while (1) {
		if (lookahead == '*') {
			match('*');
			prefixExpression();
			cout << "mul" << endl;

		} else if (lookahead == '/') {
			match('/');
			prefixExpression();
			cout << "div" << endl;

		} else if (lookahead == '%') {
			match('%');
			prefixExpression();
			cout << "rem" << endl;

		} else
			break;
	}
}


/*
 * Function:    additiveExpression
 *
 * Description: Parse an additive expression.
 *
 *              additive-expression:
 *                multiplicative-expression
 *                additive-expression + multiplicative-expression
 *                additive-expression - multiplicative-expression
 */

static void additiveExpression()
{
	multiplicativeExpression();

	while (1) {
		if (lookahead == '+') {
			match('+');
			multiplicativeExpression();
			cout << "add" << endl;

		} else if (lookahead == '-') {
			match('-');
			multiplicativeExpression();
			cout << "sub" << endl;

		} else
			break;
	}
}


/*
 * Function:    relationalExpression
 *
 * Description: Parse a relational expression.  Note that Simple C does not
 *              have shift operators, so we go immediately to additive
 *              expressions.
 *
 *              relational-expression:
 *                additive-expression
 *                relational-expression < additive-expression
 *                relational-expression > additive-expression
 *                relational-expression <= additive-expression
 *                relational-expression >= additive-expression
 */

static void relationalExpression()
{
	additiveExpression();

	while (1) {
		if (lookahead == '<') {
			match('<');
			additiveExpression();
			cout << "ltn" << endl;

		} else if (lookahead == '>') {
			match('>');
			additiveExpression();
			cout << "gtn" << endl;

		} else if (lookahead == LEQ) {
			match(LEQ);
			additiveExpression();
			cout << "leq" << endl;

		} else if (lookahead == GEQ) {
			match(GEQ);
			additiveExpression();
			cout << "geq" << endl;

		} else
			break;
	}
}


/*
 * Function:    equalityExpression
 *
 * Description: Parse an equality expression.
 *
 *              equality-expression:
 *                relational-expression
 *                equality-expression == relational-expression
 *                equality-expression != relational-expression
 */

static void equalityExpression()
{
	relationalExpression();

	while (1) {
		if (lookahead == EQL) {
			match(EQL);
			relationalExpression();
			cout << "eql" << endl;

		} else if (lookahead == NEQ) {
			match(NEQ);
			relationalExpression();
			cout << "neq" << endl;

		} else
			break;
	}
}


/*
 * Function:    logicalAndExpression
 *
 * Description: Parse a logical-and expression.  Note that Simple C does
 *              not have bitwise-and expressions.
 *
 *              logical-and-expression:
 *                equality-expression
 *                logical-and-expression && equality-expression
 */

static void logicalAndExpression()
{
	equalityExpression();

	while (lookahead == AND) {
		match(AND);
		equalityExpression();
		cout << "and" << endl;
	}
}


/*
 * Function:    expression
 *
 * Description: Parse an expression, or more specifically, a logical-or
 *              expression, since Simple C does not allow comma or
 *              assignment as an expression operator.
 *
 *              expression:
 *                logical-and-expression
 *                expression || logical-and-expression
 */

static void expression()
{
	logicalAndExpression();

	while (lookahead == OR) {
		match(OR);
		logicalAndExpression();
		cout << "or" << endl;
	}
}


/*
 * Function:    statements
 *
 * Description: Parse a possibly empty sequence of statements.  Rather than
 *              checking if the next token starts a statement, we check if
 *              the next token ends the sequence, since a sequence of
 *              statements is always terminated by a closing brace.
 *
 *              statements:
 *                empty
 *                statement statements
 */

static void statements()
{
	while (lookahead != '}')
		statement();
}


/*
 * Function:    Assignment
 *
 * Description: Parse an assignment statement.
 *
 *              assignment:
 *                expression = expression
 *                expression
 */

static void assignment()
{
	expression();

	if (lookahead == '=') {
		match('=');
		expression();
	}
}


/*
 * Function:    statement
 *
 * Description: Parse a statement.  Note that Simple C has so few
 *              statements that we handle them all in this one function.
 *
 *              statement:
 *                { declarations statements }
 *                return expression ;
 *                while ( expression ) statement
 *                for ( assignment ; expression ; assignment ) statement
 *                if ( expression ) statement
 *                if ( expression ) statement else statement
 *                assignment ;
 */

static void statement()
{
	if (lookahead == '{') {
		checker->openScope();
		match('{');
		declarations();
		statements();
		match('}');
		checker->closeScope();

	} else if (lookahead == RETURN) {
		match(RETURN);
		expression();
		match(';');

	} else if (lookahead == WHILE) {
		match(WHILE);
		match('(');
		expression();
		match(')');
		statement();

	} else if (lookahead == FOR) {
		match(FOR);
		match('(');
		assignment();
		match(';');
		expression();
		match(';');
		assignment();
		match(')');
		statement();

	} else if (lookahead == IF) {
		match(IF);
		match('(');
		expression();
		match(')');
		statement();

		if (lookahead == ELSE) {
			match(ELSE);
			statement();
		}

	} else {
		assignment();
		match(';');
	}
}


/*
 * Function:    parameter
 *
 * Description: Parse a parameter, which in Simple C is always a scalar
 *              variable with optional pointer declarators.
 *
 *              parameter:
 *                specifier pointers identifier
 */

Type parameter()
{
	int typespec = specifier();
	unsigned indirection = pointers();
	string id = lexbuf;
	match(ID);
	Type param(typespec, indirection);
	checker->declareVariable(id, param);
	return param;
}


/*
 * Function:    parameters
 *
 * Description: Parse the parameters of a function, but not the opening or
 *              closing parentheses.
 *
 *              parameters:
 *                void
 *                void pointers identifier remaining-parameters
 *                char pointers identifier remaining-parameters
 *                int pointers identifier remaining-parameters
 *
 *              remaining-parameters:
 *                empty
 *                , parameter remaining-parameters
 */

Parameters* parameters()
{
	Parameters* params = new vector<class Type>;
	int typespec;
	if (lookahead == VOID) {
		typespec = VOID;
		match(VOID);

		if (lookahead == ')')
			return nullptr;

	} else
		typespec = specifier();

	unsigned indirection = pointers();
	string id = lexbuf;
	match(ID);
	Type var(typespec, indirection);
	checker->declareVariable(id, var);

	while (lookahead == ',') {
		match(',');
		Type param = parameter();
		params->push_back(param);
	}

	return params;
}


/*
 * Function:    globalDeclarator
 *
 * Description: Parse a declarator, which in Simple C is either a scalar
 *              variable, an array, or a function, with optional pointer
 *              declarators.
 *
 *              global-declarator:
 *                pointers identifier
 *                pointers identifier ( )
 *                pointers identifier [ num ]
 */

static void globalDeclarator(int typespec)
{
	unsigned indirection = pointers();
	string id = lexbuf;
	match(ID);

	if (lookahead == '(') {
		match('(');
		match(')');

		Type func(typespec, indirection, nullptr);
		checker->declareFunction(id, func);

	} else if (lookahead == '[') {
		match('[');
		char *ptr;
		unsigned long length = strtoul(lexbuf.c_str(), &ptr, 10);
		match(NUM);
		match(']');

		Type var(typespec, indirection, length);
		checker->declareVariable(id, var);
	}
	else{

	Type var(typespec, indirection);
	checker->declareVariable(id, var);
	}
}


/*
 * Function:    remainingDeclarators
 *
 * Description: Parse any remaining global declarators after the first.
 *
 *              remaining-declarators:
 *                ;
 *                , global-declarator remaining-declarators
 */

static void remainingDeclarators(int typespec)
{
	while (lookahead == ',') {
		match(',');
		globalDeclarator(typespec);
	}

	match(';');
}


/*
 * Function:    globalOrFunction
 *
 * Description: Parse a global declaration or function definition.
 *
 *              global-or-function:
 *                specifier pointers identifier remaining-decls
 *                specifier pointers identifier ( ) remaining-decls 
 *                specifier pointers identifier [ num ] remaining-decls
 *                specifier pointers identifier ( parameters ) { ... }
 */

static void globalOrFunction()
{
	int typespec = specifier();
	unsigned indirection = pointers();
	string id = lexbuf;
	match(ID);

	if (lookahead == '[') {
		match('[');
		char * p;
		unsigned long len = strtoul(lexbuf.c_str(), &p, 10);
		match(NUM);
		match(']');
		Type var(typespec, indirection, len);
		checker->declareVariable(id, var);
		remainingDeclarators(typespec);

	} else if (lookahead == '(') {
				match('(');

		if (lookahead == ')') {
			match(')');
			Type func(typespec, indirection, nullptr);
			checker->declareFunction(id,func);
			remainingDeclarators(typespec);

		} else {
			checker->openScope();
			Parameters* params = parameters();
			Type func(typespec, indirection, params);
			checker->defineFunction(id, func);
			match(')');
			match('{');
			declarations();
			statements();
			match('}');
			
			checker->closeScope();
		}

	} else {
		Type var(typespec, indirection);
		checker->declareVariable(id, var);
		remainingDeclarators(typespec);
	}

}


/*
 * Function:    main
 *
 * Description: Analyze the standard input stream.
 */

int main()
{
	lookahead = yylex();
	lexbuf = yytext;

		checker->openScope();
	while (lookahead != DONE)
		globalOrFunction();
		checker->closeScope();

	exit(EXIT_SUCCESS);
}
