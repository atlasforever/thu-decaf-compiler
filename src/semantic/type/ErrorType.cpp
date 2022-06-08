#include "Type.h"

ErrorType::ErrorType() : Type(TypeKind::ERROR_TYPE) {}

Type::Relation ErrorType::compare(const std::shared_ptr<Type> other) const{
    (void)other;
    return SUBTYPE;
}

std::string ErrorType::toString() const {
    return "error";
}