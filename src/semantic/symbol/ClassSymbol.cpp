#include "ClassSymbol.h"
#include "BaseChecker.h"

ClassSymbol::ClassSymbol() { kind = Kind::CLASS; }

ClassSymbol::~ClassSymbol() {}

std::string ClassSymbol::toString() {
    std::string s;
    std::string base;

    s.append(pos.toString()).append(" -> class ").append(name);
    
    base = BaseChecker::getBase(name);
    if (!base.empty()) {
        s.append(" : ").append(base);
    }
    return s;
}