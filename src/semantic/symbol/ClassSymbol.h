#ifndef _CLASS_SYMBOL_H_
#define _CLASS_SYMBOL_H_
#include "Symbol.h"

class ClassSymbol : public Symbol
{
public:
    ClassSymbol();
    virtual ~ClassSymbol();
    virtual std::string toString() override;
};

#endif