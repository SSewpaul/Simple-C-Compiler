#include "Type.h"

Type::Type(int specifier, unsigned indirection):_specifier(specifier), _indirection(indirection), _declarator(SCALAR)
{

}

Type::Type(int specifier, unsigned indirection, unsigned long length): _specifier(specifier), _indirection(indirection), _declarator(ARRAY), _length(length)
{

}

Type::Type(int specifier, unsigned indirection, Parameters* parameters):_specifier(specifier), _indirection(indirection), _declarator(FUNCTION), _parameters(parameters)
{

}

Type::Type():_declarator(ERROR)
{

}

//Type::~Type()
//{
//    if (_parameters != nullptr)
//        delete _parameters;
//}

bool Type::operator== (const Type &rhs) const{
    if(this->_specifier != rhs.specifier() || this->_indirection != rhs.indirection() || this->_declarator != rhs.declarator())
    {
        return false;
    }
    else
    {
        if(this->_declarator == ARRAY)
        {
            if(this->_length == rhs.length())
                return true;

            return false;
        }
        else if(this->_declarator == FUNCTION)
        {
            if(this->_parameters == rhs.parameters())
                return true;

            return false;
        }
        else    //scalar
        {
            return true;
        }
    }
}

bool Type::operator!= (const Type &rhs) const{
    return !operator==(rhs);
}

std::ostream& operator<< (std::ostream &ostr, const Type &t)
{
    if (t.specifier() == INT)
        ostr<<"int";
    else if (t.specifier() == VOID)
        ostr<<"void";
    else if (t.specifier() == CHAR)
        ostr<<"char";
    else if (t.specifier() == LONG)
        ostr<<"long";

    if(t.indirection() > 0)
    {
        ostr<<" ";
        for(unsigned i  = 0; i < t.indirection(); i++)
            ostr<<"*";
    }

    if(t.isFunction())
    {
        ostr<<"()";
    }
    else if(t.isArray())
    {
        ostr<<"["<<t.length()<<"]";
    }

    return ostr;
}