#include "Symbol.h"
#include <vector>

typedef std::vector<Symbol*> Symbols;
typedef std::string string;

class Scope{
    Symbols _symbols;
    Scope* _scope;
    
    public:

        Scope(Symbols symbols, Scope* scope): _symbols(symbols), _scope(scope){}
        Scope* scope() const { return _scope; }
        Symbols symbols() const { return _symbols; }

        void insert(Symbol* sym);
        void remove(const string &name);
        const Symbol* find(const string &name) const;
        const Symbol* lookup(const string &name) const;
};
