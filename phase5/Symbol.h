/*
 * File:	Symbol.h
 *
 * Description:	This file contains the class definition for symbols in
 *		Simple C.  At this point, a symbol merely consists of a
 *		name and a type, neither of which you can change.
 */

# ifndef SYMBOL_H
# define SYMBOL_H
# include <string>
# include "Type.h"

class Symbol {
    typedef std::string string;
    string _name;
    Type _type;

public:
    int _offset;
    Symbol(const string &name, const Type &type, int offset = 0);
    const string &name() const;
    const Type &type() const;
};

# endif /* SYMBOL_H */
