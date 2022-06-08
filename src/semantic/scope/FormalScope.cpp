#include "FormalScope.h"
#include "printer.h"

FormalScope::FormalScope() { kind = Scope::FORMAL; }

bool FormalScope::declare(const std::string &name,
                          std::shared_ptr<Symbol> symbol) {
    std::shared_ptr<Symbol> preSym = lookupThis(name);

    // Check Conflicts
    if (preSym) {
        reportErrorText(symbol->pos, CompileErrors::CONFLICT_DECLAR,
                        {preSym->name, preSym->pos.toString()});
        return false;
    } else {
        Scope::declare(symbol->name, symbol);

        if (symbol->getKind() == Symbol::VAR) {
            params.push_back(symbol);
        }
        return true;
    }
}

void FormalScope::print(int level) {
    std::string indent = printIdent(level);
    std::string innerIndent = printIdent(level + 1);
    std::vector<std::shared_ptr<Symbol>> orderedSymbols = getOrderedSymbols();
    std::vector<std::shared_ptr<Scope>> orderedScopes = getOrderedScopes();

    std::cout << indent << "FORMAL SCOPE OF '" << name << "':" << std::endl;
    if (orderedSymbols.empty()) {
        std::cout << innerIndent << "<empty>" << std::endl;
    } else {
        for (auto &x : orderedSymbols) {
            std::cout << innerIndent << x->toString() << std::endl;
        }
    }
    for (auto &x : orderedScopes) {
        x->print(level + 1);
    }
}
std::vector<std::shared_ptr<Symbol>> FormalScope::getParams() {
    return params;    
}
