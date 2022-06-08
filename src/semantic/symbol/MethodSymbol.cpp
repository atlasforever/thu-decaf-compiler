#include "MethodSymbol.h"

MethodSymbol::MethodSymbol() { kind = Kind::METHOD; }

MethodSymbol::~MethodSymbol() {}

std::string MethodSymbol::toString() {
    std::string s;
    
    s.append(pos.toString()).append(" -> ");
    if (isStatic()) {
        s.append("STATIC ");
    }
    s.append("function ").append(name).append(" : ").append(type->toString());

    return s;
}