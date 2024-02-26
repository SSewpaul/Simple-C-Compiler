#include "Scope.h"

void Scope::insert(Symbol* sym){
    _symbols.push_back(sym);
}

void Scope::remove(const string &name){
    for(auto sym = _symbols.begin(); sym < _symbols.end(); sym++)
    {
        if ((*sym)->name() == name)
        {
            _symbols.erase(sym);
            break;
        }
    }
}

const Symbol* Scope::find(const string &name) const{
    for(auto sym = _symbols.begin(); sym < _symbols.end(); sym++)
    {
        if ((*sym)->name() == name)
        {
            return *sym;
        }
    }

    return nullptr;
}

const Symbol* Scope::lookup(const string &name) const{
    const Symbol* sym = find(name);

    if(sym)
        return sym;
    else if(_scope == nullptr)
        return nullptr;
    else
        return _scope->lookup(name);
}