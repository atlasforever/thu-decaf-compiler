#include "Type.h"

Type::Type(Type::TypeKind kind) { this->kind = kind; }

Type::TypeKind Type::getKind() const { return kind; }

Type::Relation Type::compare(const std::shared_ptr<Type> other) const {
    if (getKind() == other->getKind()) {
        return SAMETPYE;
    } else {
        return DIFFTPYE;
    }
}

Type::~Type() {}
