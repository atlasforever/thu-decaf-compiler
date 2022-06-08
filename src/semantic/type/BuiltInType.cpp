#include <stdexcept>

#include "Type.h"

BuiltInType::BuiltInType(TypeKind kind) : Type(kind) {
    if (kind != INTEGER_TYPE && kind != BOOL_TYPE && kind != STRING_TYPE &&
        kind != NULL_TYPE && kind != VOID_TYPE) {
        throw std::invalid_argument("Wrong built-in-type.");
    }
}

Type::Relation BuiltInType::compare(const std::shared_ptr<Type> other) const {
    if (other->getKind() == ERROR_TYPE) {
        return SUBTYPE;
    }

    if (getKind() == NULL_TYPE && other->getKind() == CLASS_TYPE) {
        return SUBTYPE;
    } else {
        return Type::compare(other);
    }
}

std::string BuiltInType::toString() const {
    std::string res;

    switch (getKind()) {
    case INTEGER_TYPE:
        res = "int";
        break;
    case BOOL_TYPE:
        res = "bool";
        break;
    case STRING_TYPE:
        res = "string";
        break;
    case NULL_TYPE:
        res = "null";
        break;
    case VOID_TYPE:
        res = "void";
        break;
    default:
        res = "";
        break;
    }
    return res;
}