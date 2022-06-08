#ifndef _CODEGEN_CONSTRUCTOR_H_
#define _CODEGEN_CONSTRUCTOR_H_
#include <string>
#include <memory>
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "semantic/Symbol.h"
#include "VTable.h"

class Constructor {
public:
    Constructor(llvm::Module *m, llvm::IRBuilder<> *b, VTable *v);
    static std::string getName(const std::string &cname);
    void generate(const std::string &cname);
private:
    llvm::Module *module;
    llvm::IRBuilder<> *builder;
    VTable *vtable;
};

#endif