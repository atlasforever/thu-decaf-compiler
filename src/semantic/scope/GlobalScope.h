#ifndef _GLOBAL_SCOPE_H_
#define _GLOBAL_SCOPE_H_

#include "Scope.h"

class GlobalScope : public Scope {
public:
    GlobalScope();
    virtual bool declare(const std::string &name, std::shared_ptr<Symbol> symbol) override;
    virtual void print(int level) override;
};

#endif