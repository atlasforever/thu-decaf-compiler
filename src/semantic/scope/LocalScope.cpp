#include "LocalScope.h"
#include "utils/printer.h"

LocalScope::LocalScope() { kind = Kind::LOCAL; }

std::shared_ptr<Symbol> LocalScope::lookupBefore(const Pos &pos,
                                                 const std::string &name) {
    auto res = lookupThis(name);
    auto p = getParent();

    if (res && res->pos < pos) {
        return res;
    } else {
        return p ? p->lookupBefore(pos, name) : nullptr;
    }
}

bool LocalScope::declare(const std::string &name,
                         std::shared_ptr<Symbol> symbol) {
    std::shared_ptr<Symbol> preSym = lookup(name);

    // local-variable and class-field may have same name
    if (preSym && preSym->getParent()->kind != Scope::CLASS) {
        reportErrorText(symbol->pos, CompileErrors::CONFLICT_DECLAR,
                        {name, preSym->pos.toString()});
        return false;
    }

    // no void-type variable
    if (symbol->type->getKind() == Type::VOID_TYPE) {
        reportErrorText(symbol->pos, CompileErrors::VOID_IDENTIFIER, {name});
        return false;
    }

    return Scope::declare(name, symbol);
}

void LocalScope::print(int level) {
    std::string indent = printIdent(level);
    std::string innerIndent = printIdent(level + 1);
    std::vector<std::shared_ptr<Symbol>> orderedSymbols = getOrderedSymbols();
    std::vector<std::shared_ptr<Scope>> orderedScopes = getOrderedScopes();

    std::cout << indent << "LOCAL SCOPE:" << std::endl;
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