/*
 * File:	checker.h
 *
 * Description:	This file contains the public function declarations for the
 *		semantic checker for Simple C.
 */

# ifndef CHECKER_H
# define CHECKER_H
# include "Scope.h"

Scope *openScope();
Scope *closeScope();

Symbol *defineFunction(const std::string &name, const Type &type);
Symbol *declareFunction(const std::string &name, const Type &type);
Symbol *declareVariable(const std::string &name, const Type &type);
Symbol *checkIdentifier(const std::string &name);

Type checkLogicalOr(const Type& left, const Type& right);
Type checkLogicalAnd(const Type& left, const Type& right);
Type checkIfWhileFor(const Type& expr);
Type checkNot(const Type& expr);
Type checkMulDivRem(const Type& left, const Type& right, const std::string& s);
Type checkRelationalExpr(const Type& left, const Type& right, const std::string& s);
Type checkEquality(const Type& left, const Type& right, const std::string& s);
Type checkPlus(const Type& left, const Type& right);
Type checkSub(const Type& left, const Type& right);
Type checkDeref(const Type& expr);
Type checkAddr(const Type& expr, const bool& lvalue);
Type checkIndex(const Type& left, const Type& expr);
Type checkSizeof(const Type& expr);
Type checkNeg(const Type& left);
Type checkReturn(const Type& left, const Type& expr);
void checkAssignment(const Type& left, const Type& right, const bool& lvalue);
Type checkFunction(const Type& ftype, const Parameters* params);
# endif /* CHECKER_H */ 
