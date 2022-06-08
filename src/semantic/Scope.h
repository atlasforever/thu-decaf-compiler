#ifndef _SCOPE_H_
#define _SCOPE_H_

#include "Pos.h"
#include "Symbol.h"
#include "antlr4-runtime.h"
#include <memory>

class Symbol;
class SymbolHash;

class Scope : public std::enable_shared_from_this<Scope> {
public:
    enum Kind { GLOBAL, CLASS, FORMAL, LOCAL };

    Pos pos;
    std::string name;
    Kind kind;

    virtual ~Scope();
    std::shared_ptr<Scope> getParent();
    std::shared_ptr<Symbol> getSymbol();
    void setSymbol(std::shared_ptr<Symbol> s);

    void setParent(const std::shared_ptr<Scope> &p);
    virtual std::shared_ptr<Symbol> lookup(const std::string &name);
    virtual std::shared_ptr<Symbol> lookupBefore(const Pos &pos,
                                                 const std::string &name);
    virtual bool declare(const std::string &name,
                         std::shared_ptr<Symbol> symbol);

    virtual std::shared_ptr<Scope> createScope(const Pos &p,
                                               const std::string &name);

    virtual void print(int level) = 0;

    std::vector<std::shared_ptr<Symbol>> getOrderedSymbols();

    std::shared_ptr<Scope> enterScope(const Pos &p);
    std::shared_ptr<Scope> exitScope();

protected:
    // outer scope
    std::weak_ptr<Scope> parent;
    // symbol related to this scope
    std::weak_ptr<Symbol> selfSymbol;

    // symbols inside this scope
    std::unordered_map<std::string, std::shared_ptr<Symbol>> symbols;
    // sub-scopes inside this scope
    std::unordered_map<Pos, std::shared_ptr<Scope>, PosHash> scopes;

    std::shared_ptr<Symbol> lookupThis(const std::string &name);
    std::shared_ptr<Symbol> lookupParent(const std::string &name);
    std::string printIdent(int level);

    std::vector<std::shared_ptr<Scope>> getOrderedScopes();
};
#endif