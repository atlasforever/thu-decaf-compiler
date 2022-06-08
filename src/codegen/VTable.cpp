#include "VTable.h"
#include "semantic/type/BaseChecker.h"
#include "llvm/IR/Constants.h"

VTable::VTable(llvm::Module *m, llvm::IRBuilder<> *b) : module(m), builder(b) {}

void VTable::generate(const std::vector<std::shared_ptr<Symbol>> &classes) {
    methodMap methods = getMethodMap(classes);

    for (const auto &cur : classes) {
        generate(methods, cur->name);
    }
}

llvm::Function *VTable::getFunction(const std::string &cname,
                                    const std::string &fname) {
    // should use map for better performance...
    llvmFunVec &methods = funsVec.find(cname)->second;

    for (auto &v : methods) {
        if (v.first == fname) {
            return v.second;
        }
    }
    return nullptr;
}

llvm::Value *VTable::getFunctionPtr(const std::string &cname,
                                    const std::string &fname,
                                    llvm::Value *vtablePtr) {
    int idx = 0;
    llvm::Function *f = nullptr;

    llvmFunVec &methods = funsVec.find(cname)->second;
    for (auto &v : methods) {
        if (v.first == fname) {
            f = v.second;
            break;
        }
        idx++;
    }

    // Skip the pointer to base class and the pointer to class name
    idx += 2;

    llvm::Type *vtblTy = llvm::StructType::getTypeByName(
        module->getContext(), VTable::getVtableName(cname));
    llvm::Value *castedVtbl =
        builder->CreatePointerCast(vtablePtr, vtblTy->getPointerTo());

    // Pointer to vtable's item, the item itself is a pointer to the function
    llvm::Value *fpp = builder->CreateStructGEP(vtblTy, castedVtbl, idx);

    llvm::Value *fp = builder->CreateLoad(f->getType(), fpp);

    return fp;
}

llvm::Value *VTable::instanceOf(llvm::Value *vptr, const std::string &cname) {
    llvm::Value *dstVtbl = module->getNamedGlobal(getVtableName(cname));

    llvm::Type *compTy = builder->getInt8PtrTy();
    dstVtbl = builder->CreatePointerCast(dstVtbl, compTy);
    vptr = builder->CreatePointerCast(vptr, compTy);

    llvm::Function *f = module->getFunction("_dcf_rt_INSTANCE_OF");
    return builder->CreateCall(f, {vptr, dstVtbl});
}

std::string VTable::getVtableName(const std::string &cname) {
    return std::string(cname).append("Vtable");
}

std::string VTable::getFuncName(const std::string &cname,
                                const std::string &fname) {
    if (cname == "Main" && fname == "main") {
        return fname;
    } else {
        return std::string(cname).append("_").append(fname);
    }
}

methodMap
VTable::getMethodMap(const std::vector<std::shared_ptr<Symbol>> &classes) {
    methodMap mmap;

    for (auto &c : classes) {
        std::vector<std::string> ms;
        const std::string cname = c->name;

        for (auto &field : c->getScope()->getOrderedSymbols()) {
            if (field->getKind() == Symbol::METHOD) {
                ms.push_back(field->name);
            }
        }
        std::pair<std::string, std::vector<std::string>> pair = {cname, ms};
        mmap.insert(pair);
    }
    return mmap;
}

llvmFunVec VTable::generate(const methodMap &mmap,
                            const std::string className) {
    // avoid creating same vtable again
    auto it = funsVec.find(className);
    if (it != funsVec.end()) {
        return it->second;
    }

    // get its base-class's virtual method table
    std::string basename = BaseChecker::getBase(className);
    llvmFunVec parentMethodTable;
    if (!basename.empty()) {
        parentMethodTable = generate(mmap, basename);
    }

    // fill the virtual method table with its parent's one
    llvmFunVec curFunVec = parentMethodTable;
    const std::vector<std::string> methods = mmap.find(className)->second;

    // replace overrided methods. add additional method
    for (const std::string &mName : methods) {
        llvm::Function *f = module->getFunction(getFuncName(className, mName));
        bool replaced = false;

        for (auto &item : curFunVec) {
            if (mName == item.first) {
                item.second = f;
                replaced = true;
                break;
            }
        }
        if (!replaced) {
            std::pair<std::string, llvm::Function *> p = {mName, f};
            curFunVec.push_back(p);
        }
    }

    // set llvm StructType for that vtable
    std::string vtblName = getVtableName(className);
    llvm::StructType *vtbl =
        llvm::StructType::getTypeByName(module->getContext(), vtblName);
    std::vector<llvm::Type *> contents;
    std::vector<llvm::Constant *> initVals;

    // vtable's content:
    // 1. a pointer(i8*) to its base-class's vtable
    // 2. a pointer(i8*) to a string of this class's name
    // 3. pointers to non-static methods

    // 1. pointer to its base-class's vtable
    if (!basename.empty()) {
        std::string baseVtblName = getVtableName(basename);
        contents.push_back(builder->getInt8PtrTy());

        llvm::GlobalVariable *vtblVar = module->getNamedGlobal(baseVtblName);
        llvm::Constant *castedVtbl = llvm::dyn_cast<llvm::Constant>(
            builder->CreatePointerCast(vtblVar, builder->getInt8PtrTy()));
        initVals.push_back(castedVtbl);
    } else {
        // no base-class, point to NULL
        llvm::PointerType *pt =
            llvm::Type::getInt8PtrTy(module->getContext(), 0);
        contents.push_back(pt);
        initVals.push_back(llvm::ConstantPointerNull::get(pt));
    }

    // 2. pointer to a string of this class's name
    llvm::Constant *cnameVar = builder->CreateGlobalStringPtr(
        className, std::string(className).append("Name"), 0, module);
    contents.push_back(cnameVar->getType());
    initVals.push_back(cnameVar);

    // 3. pointers to non-static methods
    for (std::pair<std::string, llvm::Function *> &item : curFunVec) {
        contents.push_back(item.second->getType());
        initVals.push_back(item.second);
    }

    // set the llvm StructType
    vtbl->setBody(contents);

    // create the vtable as a global variable
    module->getOrInsertGlobal(vtblName, vtbl);
    llvm::GlobalVariable *vtblVar = module->getNamedGlobal(vtblName);
    vtblVar->setInitializer(llvm::ConstantStruct::get(vtbl, initVals));

    std::pair<std::string, llvmFunVec> p = {className, curFunVec};
    funsVec.insert(p);
    return curFunVec;
}
