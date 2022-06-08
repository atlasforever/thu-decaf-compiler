#include "Type.h"
#include "BaseChecker.h"

ClassType::ClassType(const std::string &name) : Type(CLASS_TYPE), name(name) {}

Type::Relation ClassType::compare(const std::shared_ptr<Type> other) const {
    if (other->getKind() == ERROR_TYPE) {
        return SUBTYPE;
    }
    
    if (other->getKind() == CLASS_TYPE) {
        std::string otherName = std::dynamic_pointer_cast<ClassType>(other)->getName();

        if (otherName == name) {
            return SAMETPYE;
        } else if (BaseChecker::isBase(name, otherName)) {
            return SUBTYPE;
        }
    }
    return DIFFTPYE;
}

std::string ClassType::toString() const {
    return std::string("class ").append(name);
}

std::string ClassType::getName() const { return name; }