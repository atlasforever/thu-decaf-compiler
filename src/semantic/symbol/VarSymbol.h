#ifndef _VAR_SYMBOL_H_
#define _VAR_SYMBOL_H_

#include "Symbol.h"

class VarSymbol : public Symbol
{
public:
    VarSymbol();
    virtual ~VarSymbol();
    virtual std::string toString() override;

private:
    bool isParam();
};

#endif