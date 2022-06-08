#include "Symbol.h"

Symbol::~Symbol() {}

Symbol::Kind Symbol::getKind() const { return kind; }

// std::string Symbol::toString() {
    
// }

void Symbol::setParent(const std::shared_ptr<Scope> &p) { parent = p; }

std::shared_ptr<Scope> Symbol::getParent() { return parent.lock(); }

void Symbol::setScope(const std::shared_ptr<Scope> &s) { relatedScope = s; }

std::shared_ptr<Scope> Symbol::getScope() { return relatedScope.lock(); }

void Symbol::setStatic(bool isStatic) { _isStatic = isStatic; }

bool Symbol::isStatic() const { return _isStatic; }