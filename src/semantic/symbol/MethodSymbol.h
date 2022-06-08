#ifndef _METHOD_SYMBOL_H_
#define _METHOD_SYMBOL_H_
#include "Symbol.h"

class MethodSymbol : public Symbol {
public:
    MethodSymbol();
    virtual ~MethodSymbol();
    virtual std::string toString() override;
};

#endif