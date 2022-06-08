#ifndef _DECAF_ARRAY_TYPE_H_
#define _DECAF_ARRAY_TYPE_H_

#include "Type.h"

class ArrayType : public Type {
public:
    ArrayType(const std::shared_ptr<Type> base);
    virtual Type::Relation compare(const std::shared_ptr<Type> other) const override;
    virtual std::string toString() const override;
    const std::shared_ptr<Type> getBase() const;
private:
    const std::shared_ptr<Type> base;

};
#endif