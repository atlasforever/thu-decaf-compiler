#ifndef _RUNTIME_CHECKER_H_
#define _RUNTIME_CHECKER_H_
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

class RuntimeChecker {
public:
    RuntimeChecker(llvm::Module *m, llvm::IRBuilder<> *b);
    void generate();
private:
    llvm::Module *module;
    llvm::IRBuilder<> *builder;
};

#endif