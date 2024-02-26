#ifndef CHECKER_H
#define CHECKER_H

#include <iostream>
#include "Type.h"
#include "Symbol.h"
#include "Scope.h"
#include "tokens.h"
#include "lexer.h"

class Checker{
    Scope * head = nullptr;
    const string redeclaration = "redeclaration of \'\%s\'";
    const string redefinition = "redefinition of \'\%s\'";
    const string conflicting = "conflicting types for \'\%s\'";
    const string undeclared = "\'\%s\' undeclared";
    const string void_type = "\'\%s\' has type void";

    public:
        void openScope();
        void closeScope();
        void defineFunction(const std::string &name, const Type &type);
        void declareFunction(const std::string &name, const Type &type);
        void declareVariable(const std::string &name, const Type &type);
        void checkIdentifier(const std::string &name);
};

#endif