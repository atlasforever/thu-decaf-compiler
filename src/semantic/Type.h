#ifndef _DECAF_TYPE_H_
#define _DECAF_TYPE_H_

#include <memory>
#include <string>
#include <vector>

class Type {
public:
    enum TypeKind {
        INTEGER_TYPE,
        BOOL_TYPE,
        STRING_TYPE,
        CLASS_TYPE,
        ARRAY_TYPE,
        METHOD_TYPE,
        NULL_TYPE,
        VOID_TYPE,
        ERROR_TYPE
    };

    enum Relation { SAMETPYE, DIFFTPYE, SUBTYPE };

    Type(TypeKind kind);
    virtual ~Type();
    TypeKind getKind() const;
    virtual Type::Relation compare(const std::shared_ptr<Type> other) const = 0;
    virtual std::string toString() const = 0;

private:
    TypeKind kind;
};

class BuiltInType : public Type {
public:
    BuiltInType(TypeKind kind);
    virtual Type::Relation compare(const std::shared_ptr<Type> other) const override;
    virtual std::string toString() const override;
};

class ClassType : public Type {
public:
    ClassType(const std::string &name);
    virtual Type::Relation compare(const std::shared_ptr<Type> other) const override;
    virtual std::string toString() const override;
    std::string getName() const;

private:
    std::string name;
};

class MethodType : public Type {
public:
    MethodType(std::shared_ptr<Type> retType,
               std::vector<std::shared_ptr<Type>> argsType);
    virtual Type::Relation compare(const std::shared_ptr<Type> other) const override;
    virtual std::string toString() const override;

    Type::Relation
    compareArgs(const std::vector<std::shared_ptr<Type>> &args) const;
    
    Type::Relation compareRet(const std::shared_ptr<Type> &ret) const;
    std::shared_ptr<Type> getRetType();
    std::vector<std::shared_ptr<Type>> getArgsType();

private:
    std::shared_ptr<Type> retType;
    std::vector<std::shared_ptr<Type>> argsType;
};

class ErrorType : public Type {
public:
    ErrorType();
    virtual Type::Relation compare(const std::shared_ptr<Type> other) const override;
    virtual std::string toString() const override;
};

#endif