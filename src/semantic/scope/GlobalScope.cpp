#include "GlobalScope.h"
#include "utils/printer.h"

GlobalScope::GlobalScope() {
    kind = Kind::GLOBAL;
    parent = std::weak_ptr<Scope>();
}

bool GlobalScope::declare(const std::string &name,
                          std::shared_ptr<Symbol> symbol) {
    std::shared_ptr<Symbol> preSym = lookupThis(name);
    if (preSym) {
        reportErrorText(symbol->pos, CompileErrors::CONFLICT_DECLAR,
                        {preSym->name, preSym->pos.toString()});
        return false;
    }

    return Scope::declare(name, symbol);
}

void GlobalScope::print(int level) {
    std::string indent = printIdent(level);
    std::string innerIndent = printIdent(level + 1);
    std::vector<std::shared_ptr<Symbol>> orderedSymbols = getOrderedSymbols();

    std::cout << indent << "GLOBAL SCOPE:" << std::endl;
    for (auto &x : orderedSymbols) {
        std::cout << innerIndent << x->toString() << std::endl;
    }
    for (auto &x : orderedSymbols) {
        scopes.find(x->pos)->second->print(level + 1);
    }
}