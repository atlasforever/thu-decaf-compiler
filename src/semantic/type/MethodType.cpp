#include "Type.h"

MethodType::MethodType(std::shared_ptr<Type> retType,
                       std::vector<std::shared_ptr<Type>> argsType)
    : Type(TypeKind::METHOD_TYPE), retType(retType), argsType(argsType) {}

Type::Relation MethodType::compare(const std::shared_ptr<Type> other) const {
    if (other->getKind() == ERROR_TYPE) {
        return SUBTYPE;
    }

    if (other->getKind() != METHOD_TYPE) {
        return DIFFTPYE;
    }

    const std::shared_ptr<MethodType> o =
        std::dynamic_pointer_cast<MethodType>(other);

    Relation argRel = o->compareArgs(argsType);
    Relation retRel = compareRet(o->retType);
    if (argRel == DIFFTPYE || retRel == DIFFTPYE) {
        return DIFFTPYE;
    } else if (argRel == SUBTYPE && retRel == SUBTYPE) {
        return SUBTYPE;
    } else {
        return SAMETPYE;
    }
}

std::string MethodType::toString() const {
    std::string ret;
    int size = argsType.size();

    if (size == 0) {
        ret.append("()");
    } else if (size == 1) {
        ret.append(argsType[0]->toString());
    } else {
        ret.append("(");
        for (int i = 0; i < size; i++) {
            ret.append(argsType[i]->toString());
            if (i < size - 1) {
                ret.append(", ");
            }
        }
        ret.append(")");
    }

    ret.append(" => ");
    ret.append(retType->toString());
    return ret;
}

Type::Relation
MethodType::compareArgs(const std::vector<std::shared_ptr<Type>> &args) const {
    if (args.size() != argsType.size()) {
        return DIFFTPYE;
    }

    bool hasSub = false;
    for (std::size_t i = 0; i < args.size(); i++) {
        Relation r = argsType[i]->compare(args[i]);
        if (r == DIFFTPYE) {
            return DIFFTPYE;
        } else if (r == SUBTYPE) {
            hasSub = true;
        }
    }
    return hasSub ? SUBTYPE : SAMETPYE;
}

Type::Relation MethodType::compareRet(const std::shared_ptr<Type> &ret) const {
    return retType->compare(ret);
}

std::shared_ptr<Type> MethodType::getRetType() { return retType; }

std::vector<std::shared_ptr<Type>> MethodType::getArgsType() {
    return argsType;
}