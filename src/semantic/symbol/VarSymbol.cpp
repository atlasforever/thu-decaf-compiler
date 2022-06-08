#include "VarSymbol.h"

VarSymbol::VarSymbol() { kind = Kind::VAR; }

VarSymbol::~VarSymbol() {}

std::string VarSymbol::toString() {
    std::string s;

    s.append(pos.toString())
        .append(" -> ")
        .append("variable ")
        .append(isParam() ? "@" : "")
        .append(name)
        .append(" : ")
        .append(type->toString());
    return s;
}

bool VarSymbol::isParam() { return getParent()->kind == Scope::Kind::FORMAL; }