#ifndef _CLASS_SCOPE_H_
#define _CLASS_SCOPE_H_

#include "Scope.h"

class ClassScope : public Scope {
public:
    ClassScope();
    virtual bool declare(const std::string &name, std::shared_ptr<Symbol> symbol) override;
    virtual std::shared_ptr<Symbol> lookup(const std::string &name) override;
    virtual void print(int level) override;
    
private:
    bool declareVar(const std::string &name, std::shared_ptr<Symbol> symbol);
    bool declareMethod(const std::string &name, std::shared_ptr<Symbol> symbol);
    std::shared_ptr<Symbol> lookupInBase(const std::string &name);
    bool isMethodConflicted(std::shared_ptr<Symbol> preSym);
    bool isMethodOverrided(std::shared_ptr<Symbol> preSym, std::shared_ptr<Symbol> newSym);
};

#endif
