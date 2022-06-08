#include "Constructor.h"
#include "semantic/type/BaseChecker.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"

Constructor::Constructor(llvm::Module *m, llvm::IRBuilder<> *b, VTable *v) {
    module = m;
    builder = b;
    vtable = v;
}

std::string Constructor::getName(const std::string &cname) {
    return std::string("_").append(cname).append(cname);
}

void Constructor::generate(const std::string &cname) {
    std::string constrName = getName(cname);

    // Don't create it repeatedly
    if (module->getFunction(constrName)) {
        return;
    }

    llvm::StructType *st =
        llvm::StructType::getTypeByName(module->getContext(), cname);

    std::vector<llvm::Type *> argTypes = {st->getPointerTo()};
    llvm::FunctionType *ft =
        llvm::FunctionType::get(builder->getVoidTy(), argTypes, false);
    llvm::Function *f = llvm::Function::Create(
        ft, llvm::Function::ExternalLinkage, constrName, module);
    llvm::BasicBlock *bb =
        llvm::BasicBlock::Create(module->getContext(), "", f);

    // Start emiting constructor
    builder->SetInsertPoint(bb);

    // Call base's constructor first
    std::string baseName = BaseChecker::getBase(cname);
    bool hasBase = !baseName.empty();
    if (hasBase) {
        generate(baseName);

        llvm::Function *baseConstr = module->getFunction(getName(baseName));
        llvm::Type *baseTy =
            llvm::StructType::getTypeByName(module->getContext(), baseName);

        
        llvm::Value *basePtr =
            builder->CreatePointerCast(f->getArg(0), baseTy->getPointerTo());
        builder->CreateCall(baseConstr, {basePtr});
    }

    // let vptr point to its own vtable
    // cast the object to an array of i8*, vptr is the first element
    llvm::Type *interTy = builder->getInt8PtrTy();
    llvm::Value *casted = builder->CreatePointerCast(f->getArg(0), interTy->getPointerTo());
    std::vector<llvm::Value*> idxList = {builder->getInt32(0)};
    llvm::Value *vpptr = builder->CreateGEP(interTy, casted, idxList);
    llvm::Value *vtbl = module->getNamedGlobal(VTable::getVtableName(cname));
    llvm::Value *castedVtbl = builder->CreatePointerCast(vtbl, interTy);
    builder->CreateStore(castedVtbl, vpptr);

    builder->CreateRetVoid();
}
