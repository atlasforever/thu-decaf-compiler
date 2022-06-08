#ifndef _SYMBOL_H_
#define _SYMBOL_H_

#include <string>
#include <memory>
#include <unordered_map>
#include "Pos.h"
#include "Scope.h"
#include "Type.h"

class Scope;

class Symbol
{
public:
    enum Attr {
        BASECLASS,
        TYPE
    };

    enum Kind {
        CLASS,
        METHOD,
        VAR
    };

    Pos pos;
    std::string name;
    std::shared_ptr<Type> type;
    
    ~Symbol();
    Symbol::Kind getKind() const;
    void setParent(const std::shared_ptr<Scope> &p);
    std::shared_ptr<Scope> getParent();

    void setScope(const std::shared_ptr<Scope> &s);
    std::shared_ptr<Scope> getScope();

    virtual std::string toString() = 0;

    void setStatic(bool isStatic);
    bool isStatic() const;

protected:
    Kind kind;
private:
    bool _isStatic = false;
    std::weak_ptr<Scope> parent;
    std::weak_ptr<Scope> relatedScope;
};

class SymbolHash
{
    std::size_t operator()(Symbol const &s) const noexcept
    {
        std::size_t h1 = std::hash<int>{}(s.pos.linePos);
        std::size_t h2 = std::hash<int>{}(s.pos.charPos);
        return h1 ^ (h2 << 1);
    }
};

#endif