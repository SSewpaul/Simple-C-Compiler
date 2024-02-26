#include <iostream>
#include "tokens.h"
#include "lexer.h"

using namespace std;

/* function  definitions */
void match(int token);
void error(int token);
void or_expr();
void and_expr();
void eq_expr();
void comp_expr();
void add_expr();
void mul_expr();
void prefix_expr();
void index_expr();
void term_expr();
void expr_list();
void statements();
void statement();
void declarations();
void assignment();
void specifier();
void pointers();
void declaration();
void declarator_list();
void declarator();
void pointers();
void function_or_global();
void global_declarator();
void global_declarator_list();
void remaining_decls();
void parameters();
void parameter();
void parameter_list();

int lookahead;

void match(int token)
{
	if (token == lookahead)
	{
		//cout<<lookahead<<endl;
		lookahead = yylex();
	}
	else
	{
		report("syntax error at ", yytext);
		exit(1);
	}
}

void error(int token)
{
	cout << "Expected " << token << ". Recieved " << lookahead;
	exit(-1);
}

/*
 * A -> B A’
 * A’ -> || B A’
 * 		 |  e
 */
void or_expr()
{
	and_expr();
	while (1)
	{
		if (lookahead == OR)
		{
			match(OR);
			and_expr();
			cout << "or" << endl;
		}
		else
		{
			break;
		}
	}
}

/*
 * B -> C B’
 * B’ -> && C B’
 *       | e
 */
void and_expr()
{
	eq_expr();
	while (1)
	{
		if (lookahead == AND)
		{
			match(AND);
			eq_expr();
			cout << "and" << endl;
		}
		else
		{
			break;
		}
	}
}

/*
 * C -> D C’
 * C’ -> == D C’
 *      | != D C’
 *      | e
 */
void eq_expr()
{
	comp_expr();
	while (1)
	{
		if (lookahead == EQL)
		{
			match(EQL);
			comp_expr();
			cout << "eql" << endl;
		}
		else if (lookahead == NEQ)
		{
			match(NEQ);
			comp_expr();
			cout << "neq" << endl;
		}
		else
		{
			break;
		}
	}
}

/*
 * D -> E D’
 * D’ -> < E D’
 *      | > E D’
 *      | <= E D’
 *      | >= E D’
 *      | e
 */
void comp_expr()
{
	add_expr();
	while (1)
	{
		if (lookahead == LTN)
		{
			match(LTN);
			add_expr();
			cout << "ltn" << endl;
		}
		else if (lookahead == GTN)
		{
			match(GTN);
			add_expr();
			cout << "gtn" << endl;
		}
		else if (lookahead == LTE)
		{
			match(LTE);
			add_expr();
			cout << "leq" << endl;
		}
		else if (lookahead == GTE)
		{
			match(GTE);
			add_expr();
			cout << "geq" << endl;
		}
		else
		{
			break;
		}
	}
}

/*
 * E -> F E’
 * E’ -> + F E’
 *      | - F E’
 *      | e
 */
void add_expr()
{
	mul_expr();
	while (1)
	{
		if (lookahead == PLUS)
		{
			match(PLUS);
			mul_expr();
			cout << "add" << endl;
		}
		else if (lookahead == MINUS)
		{
			match(MINUS);
			mul_expr();
			cout << "sub" << endl;
		}
		else
		{
			break;
		}
	}
}

/*
 * F -> G F’
 * F’ -> * G F’
 *     | / G F’
 *     | % G F’
 *     | e
 */
void mul_expr()
{
	prefix_expr();
	while (1)
	{
		if (lookahead == STAR)
		{
			match(STAR);
			prefix_expr();
			cout << "mul" << endl;
		}
		else if (lookahead == DIV)
		{
			match(DIV);
			prefix_expr();
			cout << "div" << endl;
		}
		else if (lookahead == REM)
		{
			match(REM);
			prefix_expr();
			cout << "rem" << endl;
		}
		else
		{
			break;
		}
	}
}

/*
 * G -> &G
 *      | * G
 *      | ! G
 *      | - G
 *      | sizeof G
 *       | H
 */
void prefix_expr()
{

	if (lookahead == ADDR)
	{
		match(ADDR);
		prefix_expr();
		cout << "addr" << endl;
	}
	else if (lookahead == STAR)
	{
		match(STAR);
		prefix_expr();
		cout << "deref" << endl;
	}
	else if (lookahead == NOT)
	{
		match(NOT);
		prefix_expr();
		cout << "not" << endl;
	}
	else if (lookahead == MINUS)
	{
		match(MINUS);
		prefix_expr();
		cout << "neg" << endl;
	}
	else if (lookahead == SIZEOF)
	{
		match(SIZEOF);
		prefix_expr();
		cout << "sizeof" << endl;
	}
	else
	{
		index_expr();
	}
}

/*
 * H -> I H’
 * H’ -> [A] H’
 *      | e
 */
void index_expr()
{
	term_expr();
	while (1)
	{
		if (lookahead == LEFTBRACKET)
		{
			match(LEFTBRACKET);
			// cout<<"left bracket"<<endl;
			or_expr();
			match(RIGHTBRACKET);
			// cout<<"right bracket"<<endl;
			term_expr();
			cout << "index" << endl;
		}
		else
		{
			break;
		}
	}
}

/*
 * I -> id I’
 *   | num
 *   | string
 *   | character
 *   | ( ASTAR )
 * I’ -> ( I’’
 *    | e
 * I’’-> K)
 *     | )
 */

void term_expr()
{
	if (lookahead == ID)
	{
		match(ID);
		// cout<<"id"<<endl;
		if (lookahead == LEFTPAREN)
		{
			match(LEFTPAREN);
			// cout<<"left paren"<<endl;
			if (lookahead == RIGHTPAREN)
			{
				match(RIGHTPAREN);
				// cout<<"right paren"<<endl;
			}
			else
			{
				expr_list();
				match(RIGHTPAREN);
				// cout<<"right paren"<<endl;
			}
		}
		else
		{
			/* do nothing */
		}
	}
	else if (lookahead == NUM)
	{
		match(NUM);
		// cout<<"num"<<endl;
	}
	else if (lookahead == STRING)
	{
		match(STRING);
		// cout<<"string"<<endl;
	}
	else if (lookahead == CHARACTER)
	{
		match(CHARACTER);
		// cout<<"char"<<endl;
	}
	else if (lookahead == LEFTPAREN)
	{
		match(LEFTPAREN);
		// cout<<"left paren"<<endl;
		or_expr();
		match(RIGHTPAREN);
		// cout<<"right paren"<<endl;
	}
}

/*
 * K -> A, K
 *     | A
 */
void expr_list()
{
	or_expr();
	while (1)
	{
		if (lookahead == COMMA)
		{
			match(COMMA);
			// cout<<"comma"<<endl;
			or_expr();
		}
		else
		{
			break;
		}
	}
}

void statement()
{
	if (lookahead == LEFTBRACE)
	{
		match(LEFTBRACE);
		declarations();
		statements();
		match(RIGHTBRACE);
	}
	else if(lookahead == RETURN)
	{
		match(RETURN);
		or_expr();
		match(SEMICOLON);
	}
	else if(lookahead == WHILE)
	{
		match(WHILE);
		match(LEFTPAREN);
		or_expr();
		match(RIGHTPAREN);
		statement();
	}
	else if(lookahead == FOR)
	{
		match(FOR);
		match(LEFTPAREN);
		assignment();
		match(SEMICOLON);
		or_expr();
		match(SEMICOLON);
		assignment();
		match(RIGHTPAREN);
		statement();
	}
	else if(lookahead == IF)
	{
		match(IF);
		match(LEFTPAREN);
		or_expr();
		match(RIGHTPAREN);
		statement();

		if(lookahead == ELSE)
		{
			match(ELSE);
			statement();
		}
	}
	else{
		assignment();
		match(SEMICOLON);
	}
}

void assignment()
{
	or_expr();

	if (lookahead == ASSIGN)
	{
		match(ASSIGN);
		or_expr();
	}
	else{
		/* do nothing */
	}
}

void declarations()
{
	
		if(lookahead == INT || lookahead == CHAR || lookahead == LONG || lookahead == VOID)
		{
			declaration();
			declarations();
		}
		else
		{
			//break;
		}
	
}

void statements()
{
	if(lookahead == RIGHTBRACE)
	{
		return;
		//match(RIGHTBRACE);
		//break;
	}
	else
	{
		statement();
		statements();
	}
}

void declaration()
{
	specifier();
	declarator_list();
	match(SEMICOLON);
}

void declarator_list()
{
	declarator();

	while(1)
	{
		if(lookahead == COMMA)
		{
			match(COMMA);
			declarator();
		}
		else
		{
			break;
		}
	}
}

void declarator()
{
	pointers();
	match(ID);

	if(lookahead == LEFTBRACKET)
	{
		match(LEFTBRACKET);
		match(NUM);
		match(RIGHTBRACKET);
	}
}

void pointers()
{
	while(1)
	{
		if(lookahead == STAR)
		{
			match(STAR);
			pointers();
		}	
		else
		{
			break;
		}
	}
}

void specifier()
{
	if(lookahead == INT)
	{
		match(INT);
	}
	else if(lookahead == CHAR)
	{
		match(CHAR);
	}
	else if(lookahead == LONG)
	{
		match(LONG);
	}
	else if(lookahead == VOID)
	{
		match(VOID);
	}
	else
	{
		/* do nothing */
	}
}

void function_or_global()
{
	
	specifier();
	pointers();
	match(ID);

	if(lookahead == LEFTPAREN)
	{
		match(LEFTPAREN);

		if(lookahead == RIGHTPAREN)
		{
			match(RIGHTPAREN);
			remaining_decls();
		}
		else
		{
			parameters();
			match(RIGHTPAREN);
			match(LEFTBRACE);
			declarations();
			statements();
			match(RIGHTBRACE);
		}
		
	}
	else if(lookahead == LEFTBRACKET)
	{
		match(LEFTBRACKET);
		match(NUM);
		match(RIGHTBRACKET);
		remaining_decls();
	}
	else
	{
		remaining_decls();
	}
	
}

void remaining_decls()
{
	if(lookahead == SEMICOLON)
	{
		match(SEMICOLON);
	}
	else
	{
		match(COMMA);
		global_declarator();
		remaining_decls();
	}
}

void parameters()
{
	if(lookahead == VOID)
	{
		match(VOID);

		if(lookahead == STAR)
		{
			pointers();
			match(ID);
			
			if(lookahead == COMMA)
			{
				match(COMMA);
				parameter_list();
			}
		}
	}
	else
	{
		parameter_list();
	}
}

void parameter_list()
{
	parameter();

	while(1)
	{
		if(lookahead == COMMA)
		{
			match(COMMA);
			parameter_list();
		}
		else{
			break;
		}
	}
}

void parameter()
{
	specifier();
	pointers();
	match(ID);
}

void global_declarator_list()
{
	global_declarator();

	while(1)
	{
		if(lookahead == COMMA)
		{
			match(COMMA);
			global_declarator_list();
		}
		else
		{
			break;
		}
	}
}

void global_declarator(){
	pointers();
	match(ID);
	//cout<<yytext<<endl;
	if(lookahead == LEFTPAREN)
	{
		match(LEFTPAREN);
		match(RIGHTPAREN);
	}
	else if(lookahead == LEFTBRACKET)
	{
		match(LEFTBRACKET);
		match(NUM);
		match(RIGHTBRACKET);
	}
	else
	{
		/* do nothing */
	}
}

int main()
{
	lookahead = yylex();

	while(lookahead != Done)
		function_or_global();
}
