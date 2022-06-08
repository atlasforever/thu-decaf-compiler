#include "Scope.h"
#include "ClassScope.h"
#include "FormalScope.h"
#include "LocalScope.h"

Scope::~Scope() {}

void Scope::setParent(const std::shared_ptr<Scope> &p) { parent = p; }

std::shared_ptr<Scope> Scope::getParent() { return parent.lock(); }

std::shared_ptr<Symbol> Scope::getSymbol() { return selfSymbol.lock(); }

void Scope::setSymbol(std::shared_ptr<Symbol> s) { selfSymbol = s; }

std::shared_ptr<Symbol> Scope::lookup(const std::string &name) {
    if (name.empty()) {
        return nullptr;
    }

    auto res = lookupThis(name);

    if (!res) {
        res = lookupParent(name);
    }
    return res;
}

std::shared_ptr<Symbol> Scope::lookupBefore(const Pos &pos,
                                            const std::string &name) {
    (void)pos;
    return lookup(name);
}

bool Scope::declare(const std::string &name, std::shared_ptr<Symbol> symbol) {
    symbol->setParent(shared_from_this());

    std::pair<std::string, std::shared_ptr<Symbol>> p = {name, symbol};

    auto res = symbols.insert(p);
    return res.second;
}

std::shared_ptr<Scope> Scope::createScope(const Pos &p,
                                          const std::string &name) {
    std::shared_ptr<Scope> scope;
    if (kind == Kind::GLOBAL) {
        scope = std::make_shared<ClassScope>();
    } else if (kind == Kind::CLASS) {
        scope = std::make_shared<FormalScope>();
    } else {
        scope = std::make_shared<LocalScope>();
    }

    scope->pos = p;
    scope->name = name;
    scope->setParent(shared_from_this());
    std::pair<Pos, std::shared_ptr<Scope>> pair = {p, scope};
    scopes.insert(pair);
    return scope;
}

std::shared_ptr<Scope> Scope::enterScope(const Pos &p) {
    auto iter = scopes.find(p);
    if (iter != scopes.end()) {
        return iter->second;
    } else {
        return nullptr;
    }
}

std::shared_ptr<Scope> Scope::exitScope() { return parent.lock(); }

std::shared_ptr<Symbol> Scope::lookupThis(const std::string &name) {
    auto iter = symbols.find(name);
    if (iter != symbols.end()) {
        return iter->second;
    } else {
        return nullptr;
    }
}

std::shared_ptr<Symbol> Scope::lookupParent(const std::string &name) {
    auto p = parent.lock();
    if (!p) {
        return nullptr;
    } else {
        return p->lookup(name);
    }
}

std::string Scope::printIdent(int level) { return std::string(level * 4, ' '); }

std::vector<std::shared_ptr<Symbol>> Scope::getOrderedSymbols() {
    std::map<Pos, std::shared_ptr<Symbol>> map;

    for (auto &x : symbols) {
        map.insert({x.second->pos, x.second});
    }

    std::vector<std::shared_ptr<Symbol>> res;
    for (auto &x : map) {
        res.push_back(x.second);
    }
    return res;
}

std::vector<std::shared_ptr<Scope>> Scope::getOrderedScopes() {
    std::vector<Pos> keys;

    keys.reserve(scopes.size());
    for (auto &it : scopes) {
        keys.push_back(it.first);
    }
    std::sort(keys.begin(), keys.end());

    std::vector<std::shared_ptr<Scope>> res;
    for (Pos &key : keys) {
        res.push_back(scopes.find(key)->second);
    }
    return res;
}