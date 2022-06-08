#ifndef _METHOD_SCOPE_H_
#define _METHOD_SCOPE_H_

#include "Scope.h"

class FormalScope : public Scope {
public:
    FormalScope();
    virtual bool declare(const std::string &name,
                         std::shared_ptr<Symbol> symbol) override;
    virtual void print(int level) override;

    // Use this function to get list of params in right order. If we use
    // 'getOrderedSymbols()' from 'Scope', the order will be messed up because
    // of unordered_map
    std::vector<std::shared_ptr<Symbol>> getParams();

private:
    std::vector<std::shared_ptr<Symbol>> params;
};

#endif