#include "ArrayType.h"
#include <memory>

ArrayType::ArrayType(const std::shared_ptr<Type> base)
    : Type(TypeKind::ARRAY_TYPE), base(base) {}

Type::Relation ArrayType::compare(const std::shared_ptr<Type> other) const {
    if (other->getKind() == ERROR_TYPE) {
        return SUBTYPE;
    }
    
    if (other->getKind() != ARRAY_TYPE) {
        return DIFFTPYE;
    }

    const std::shared_ptr<ArrayType> o = std::dynamic_pointer_cast<ArrayType>(other);
    if (base->compare(o->getBase()) == SAMETPYE) {
        return SAMETPYE;
    } else {
        return DIFFTPYE;
    }
}

std::string ArrayType::toString() const {
    return base->toString().append("[]");
}

const std::shared_ptr<Type> ArrayType::getBase() const {
    return base;
}