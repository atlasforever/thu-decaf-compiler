#ifndef _LOCAL_SCOPE_H_
#define _LOCAL_SCOPE_H_

#include "Scope.h"

class LocalScope : public Scope {
public:
    LocalScope();
    virtual std::shared_ptr<Symbol>
    lookupBefore(const Pos &pos, const std::string &name) override;
    virtual bool declare(const std::string &name,
                         std::shared_ptr<Symbol> symbol) override;
    virtual void print(int level) override;
};
#endif