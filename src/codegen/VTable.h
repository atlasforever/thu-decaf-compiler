#ifndef _V_TABLE_H_
#define _V_TABLE_H_
#include "semantic/Symbol.h"
#include "unordered_map"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"

using methodMap = std::unordered_map<std::string, std::vector<std::string>>;
using llvmFunVec = std::vector<std::pair<std::string, llvm::Function*>>;

class VTable {
public:
    VTable(llvm::Module *m, llvm::IRBuilder<> *b);
    void generate(const std::vector<std::shared_ptr<Symbol>> &classes);
    llvm::Function *getFunction(const std::string &cname,
                                const std::string &fname);
    llvm::Value *getFunctionPtr(const std::string &cname,
                                const std::string &fname,
                                llvm::Value *vtablePtr);
    llvm::Value *instanceOf(llvm::Value *vptr, const std::string &cname);
    static std::string getVtableName(const std::string &cname);
    static std::string getFuncName(const std::string &cname,
                                   const std::string &fname);

private:
    
    // LLVM
    llvm::Module *module;
    llvm::IRBuilder<> *builder;

    std::unordered_map<std::string, llvmFunVec> funsVec;

    methodMap getMethodMap(const std::vector<std::shared_ptr<Symbol>> &classes);

    llvmFunVec generate(const methodMap &mmap, const std::string className);
};

#endif