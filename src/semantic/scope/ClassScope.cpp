#include "ClassScope.h"
#include "BaseChecker.h"
#include "utils/printer.h"

ClassScope::ClassScope() { kind = Kind::CLASS; }

bool ClassScope::declare(const std::string &name,
                         std::shared_ptr<Symbol> symbol) {
    if (symbol->getKind() == Symbol::VAR) {
        return declareVar(name, symbol);
    } else if (symbol->getKind() == Symbol::METHOD) {
        return declareMethod(name, symbol);
    }
    return false;
}

std::shared_ptr<Symbol> ClassScope::lookup(const std::string &name) {
    std::shared_ptr<Symbol> sym;

    sym = lookupThis(name);
    if (sym) {
        return sym;
    }

    sym = lookupInBase(name);
    if (sym) {
        return sym;
    }

    sym = lookupParent(name);
    if (sym) {
        return sym;
    }

    return sym;
}

void ClassScope::print(int level) {
    std::string indent = printIdent(level);
    std::string innerIndent = printIdent(level + 1);
    std::vector<std::shared_ptr<Symbol>> orderedSymbols = getOrderedSymbols();

    std::cout << indent << "CLASS SCOPE OF '" << name << "':" << std::endl;
    if (orderedSymbols.empty()) {
        std::cout << innerIndent << "<empty>" << std::endl;
    } else {
        for (auto &x : orderedSymbols) {
            std::cout << innerIndent << x->toString() << std::endl;
        }
        for (auto &x : orderedSymbols) {
            if (x->getKind() == Symbol::METHOD) {
                scopes.find(x->pos)->second->print(level + 1);
            }
        }
    }
}

bool ClassScope::declareVar(const std::string &name,
                            std::shared_ptr<Symbol> symbol) {
    std::shared_ptr<Symbol> preSym = lookup(name);

    if (preSym) {
        if (preSym->getParent() != shared_from_this() &&
            preSym->getKind() == Symbol::VAR) {
            reportErrorText(symbol->pos, CompileErrors::OVERRIDE_VAR,
                            {preSym->name});
            return false;
        } else {
            reportErrorText(symbol->pos, CompileErrors::CONFLICT_DECLAR,
                            {preSym->name, preSym->pos.toString()});
            return false;
        }
    }

    if (symbol->type->getKind() == Type::VOID_TYPE) {
        reportErrorText(symbol->pos, CompileErrors::VOID_IDENTIFIER,
                        {symbol->name});
        return false;
    }
    return Scope::declare(name, symbol);
}

bool ClassScope::declareMethod(const std::string &name,
                               std::shared_ptr<Symbol> symbol) {
    std::shared_ptr<Symbol> preSym = lookup(name);

    if (preSym) {
        if (isMethodConflicted(preSym)) {
            reportErrorText(symbol->pos, CompileErrors::CONFLICT_DECLAR,
                            {preSym->name, preSym->pos.toString()});
            return false;
        } else if (!isMethodOverrided(preSym, symbol)) {
            reportErrorText(symbol->pos, CompileErrors::OVERRIDE_METHOD,
                            {preSym->name, preSym->getParent()->name});
            return false;
        }
    }
    return Scope::declare(name, symbol);
}

std::shared_ptr<Symbol> ClassScope::lookupInBase(const std::string &name) {
    std::string basename = BaseChecker::getBase(this->name);
    std::shared_ptr<Symbol> sym = nullptr;

    if (!basename.empty()) {
        std::shared_ptr<Symbol> baseSym = Scope::lookup(basename);
        if (baseSym && baseSym->getScope()) {
            sym = baseSym->getScope()->lookup(name);
        }
    }
    return sym;
}

bool ClassScope::isMethodConflicted(std::shared_ptr<Symbol> preSym) {
    if (preSym->getKind() != Symbol::METHOD) {
        return true;
    } else {
        if (preSym->getParent() == shared_from_this() || preSym->isStatic()) {
            return true;
        }
    }
    return false;
}

bool ClassScope::isMethodOverrided(std::shared_ptr<Symbol> preSym,
                                   std::shared_ptr<Symbol> newSym) {
    if (preSym->getKind() == Symbol::METHOD &&
        preSym->getParent() != shared_from_this()) {

        if (!preSym->isStatic() &&
            newSym->type->compare(preSym->type) != Type::DIFFTPYE) {
            return true;
        }
    }
    return false;
}