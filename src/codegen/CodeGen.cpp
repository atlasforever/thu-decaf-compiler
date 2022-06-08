#include "CodeGen.h"
#include "Constructor.h"
#include "Pos.h"
#include "Type.h"
#include "semantic/scope/FormalScope.h"
#include "semantic/type/ArrayType.h"
#include "semantic/type/BaseChecker.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"

CodeGenVisitor::CodeGenVisitor(antlr4::tree::ParseTree *ast,
                               std::shared_ptr<Scope> &scope,
                               std::shared_ptr<ASTAttrManager> &am) {
    this->ast = ast;
    this->scope = scope;
    this->global = scope;
    this->cur = scope;
    attrManager = am;

    module = new llvm::Module("my module", context);
    builder = new llvm::IRBuilder<>(context);

    auto targetTriple = llvm::sys::getDefaultTargetTriple();
    module->setTargetTriple(targetTriple);
    targetSize = 64;

    vtable = new VTable(module, builder);
}

CodeGenVisitor::~CodeGenVisitor() {
    delete vtable;
    delete builder;
    delete module;
}

void CodeGenVisitor::codegen() {
    visit(ast);
    module->print(llvm::outs(), nullptr);

    if (llvm::verifyModule(*module, &llvm::outs())) {
        // return;
    }
}

antlrcpp::Any
CodeGenVisitor::visitTopLevel(DecafParserParser::TopLevelContext *ctx) {
    std::vector<std::shared_ptr<Symbol>> classes = cur->getOrderedSymbols();
    genClasses(classes);
    genBuiltInProtos();
    // generate the rests
    visitChildren(ctx);
    return nullptr;
}

antlrcpp::Any
CodeGenVisitor::visitClassDef(DecafParserParser::ClassDefContext *ctx) {
    Pos pos = getClassPos(ctx);
    cur = cur->enterScope(pos);
    curClass = cur->getSymbol();
    visitChildren(ctx);
    cur = cur->exitScope();

    return nullptr;
}

antlrcpp::Any
CodeGenVisitor::visitMethodDef(DecafParserParser::MethodDefContext *ctx) {
    std::string cname = cur->name;
    std::shared_ptr<Type> classType = cur->getSymbol()->type;
    Pos pos = getMethodPos(ctx);

    cur = cur->enterScope(pos);
    curMethod = cur->getSymbol();

    std::shared_ptr<FormalScope> fScope =
        std::static_pointer_cast<FormalScope>(cur);
    std::string fname = fScope->name;

    // Start emiting
    llvm::Function *f = module->getFunction(VTable::getFuncName(cname, fname));
    llvm::BasicBlock *bb = llvm::BasicBlock::Create(context, "", f);
    builder->SetInsertPoint(bb);

    // alloca a location on stack for every paramter. So we can treat them and
    // local variables in the same way
    auto params = fScope->getParams();
    for (size_t i = 0; i < params.size(); i++) {
        llvm::AllocaInst *alloca =
            builder->CreateAlloca(getLLVMType(params[i]->type));
        builder->CreateStore(f->getArg(i), alloca);
        attrManager->setSymbolLLVMValue(params[i], alloca);
    }

    // Emit the body
    visitChildren(ctx);
    // BasicBlock must have a terminator
    if (f->getReturnType()->isVoidTy()) {
        builder->CreateRetVoid();
    }

    cur = cur->exitScope();
    return nullptr;
}

//     antlrcpp::Any CodeGenVisitor::visitField(DecafParserParser::FieldContext
//     *ctx) {} antlrcpp::Any
//     CodeGenVisitor::visitVarDef(DecafParserParser::VarDefContext *ctx) {}
//     antlrcpp::Any
//     CodeGenVisitor::visitMethodDef(DecafParserParser::MethodDefContext *ctx)
//     {} antlrcpp::Any CodeGenVisitor::visitVar(DecafParserParser::VarContext
//     *ctx) {} antlrcpp::Any
//     CodeGenVisitor::visitParaVarDef(DecafParserParser::ParaVarDefContext
//     *ctx) {} antlrcpp::Any
//     CodeGenVisitor::visitVarList(DecafParserParser::VarListContext *ctx) {}
//     antlrcpp::Any
//     CodeGenVisitor::visitClassType(DecafParserParser::ClassTypeContext *ctx)
//     {} antlrcpp::Any CodeGenVisitor::visitType(DecafParserParser::TypeContext
//     *ctx) {}antlrcpp::Any
//     CodeGenVisitor::visitForInit(DecafParserParser::ForInitContext *ctx) {}
//     antlrcpp::Any
//     CodeGenVisitor::visitForUpdate(DecafParserParser::ForUpdateContext *ctx)
//     {}antlrcpp::Any
//     CodeGenVisitor::visitForControl(DecafParserParser::ForControlContext
//     *ctx) {}
antlrcpp::Any
CodeGenVisitor::visitLocalVarDef(DecafParserParser::LocalVarDefContext *ctx) {
    std::string id = ctx->var()->id()->IDENTIFIER()->getSymbol()->getText();
    std::shared_ptr<Symbol> varSym = cur->lookup(id);
    std::shared_ptr<Type> lt = varSym->type;

    llvm::Type *llvmlt = getLLVMType(lt);
    llvm::AllocaInst *alloca = builder->CreateAlloca(llvmlt);
    llvm::Value *initVal;

    if (ctx->bop) {
        initVal = visit(ctx->expr()).as<llvm::Value *>();
    } else {
        initVal = getLLVMDefaultValue(lt);
    }

    // LLVM-IR is typed, so we need to manually cast for subtyping
    if (lt->getKind() == Type::CLASS_TYPE) {
        initVal = builder->CreatePointerCast(initVal, llvmlt);
    }

    llvm::Value *v = builder->CreateStore(initVal, alloca);
    attrManager->setSymbolLLVMValue(varSym, alloca);

    return nullptr;
}

antlrcpp::Any
CodeGenVisitor::visitAssign(DecafParserParser::AssignContext *ctx) {
    llvm::Value *r = visit(ctx->expr()).as<llvm::Value *>();
    llvm::Value *l = visit(ctx->lValue()).as<llvm::Value *>();

    // LLVM-IR is typed, so we need to manually cast for subtyping
    Type::TypeKind tk = attrManager->getExprType(ctx->expr())->getKind();
    if (tk == Type::CLASS_TYPE || tk == Type::NULL_TYPE) {
        // LValue is always a LLVM pointer
        llvm::PointerType *pt = llvm::dyn_cast<llvm::PointerType>(l->getType());
        r = builder->CreatePointerCast(r, pt->getElementType());
    }

    builder->CreateStore(r, l);
    return nullptr;
}

antlrcpp::Any
CodeGenVisitor::visitIfStmt(DecafParserParser::IfStmtContext *ctx) {
    // Convert condition to a bool by comparing non-equal to 0
    llvm::Value *condV = visit(ctx->expr()).as<llvm::Value *>();
    condV = builder->CreateICmpNE(condV, builder->getInt32(0), "ifcond");

    // Create blocks for the then and else cases.  Insert the 'then' block at
    // the end of the function.
    llvm::Function *f = builder->GetInsertBlock()->getParent();
    llvm::BasicBlock *thenBB = llvm::BasicBlock::Create(context, "then", f);
    llvm::BasicBlock *elseBB = llvm::BasicBlock::Create(context, "else");
    llvm::BasicBlock *mergeBB = llvm::BasicBlock::Create(context, "ifcont");

    builder->CreateCondBr(condV, thenBB, elseBB);

    // Emit then block
    builder->SetInsertPoint(thenBB);
    visit(ctx->stmt(0));
    // No br-instruction if 'then-statment' returns, as LLVM requires one
    // terminator in a basic block
    if (!attrManager->getHasRet(ctx->stmt(0))) {
        builder->CreateBr(mergeBB);
    }
    // Codegen of 'then' can change the current block, update thenBB for the PHI
    thenBB = builder->GetInsertBlock();

    // Emit else block
    f->getBasicBlockList().push_back(elseBB);
    builder->SetInsertPoint(elseBB);
    // Every basic block must have only one terminator
    if (ctx->ELSE()) {
        visit(ctx->stmt(1));
        if (!attrManager->getHasRet(ctx->stmt(1))) {
            builder->CreateBr(mergeBB);
        }
    } else {
        builder->CreateBr(mergeBB);
    }
    // Codegen of 'else' can change the current block, update elseBB for the PHI
    elseBB = builder->GetInsertBlock();

    // Emit merge block
    if (!attrManager->getHasRet(ctx)) {
        f->getBasicBlockList().push_back(mergeBB);
        builder->SetInsertPoint(mergeBB);
    }

    return nullptr;
}

antlrcpp::Any
CodeGenVisitor::visitWhileStmt(DecafParserParser::WhileStmtContext *ctx) {
    llvm::Function *f = builder->GetInsertBlock()->getParent();

    llvm::BasicBlock *inBB = llvm::BasicBlock::Create(context, "loopin", f);
    llvm::BasicBlock *bodyBB = llvm::BasicBlock::Create(context, "loopbody");
    llvm::BasicBlock *outBB = llvm::BasicBlock::Create(context, "loopout");

    // go into loop
    builder->CreateBr(inBB);
    builder->SetInsertPoint(inBB);

    loopExits.push_back(outBB);

    // Convert condition to a bool by comparing non-equal to 0
    llvm::Value *condV = visit(ctx->expr()).as<llvm::Value *>();
    condV = builder->CreateICmpNE(condV, builder->getInt32(0), "loopcond");
    builder->CreateCondBr(condV, bodyBB, outBB);

    // while-statment's body
    f->getBasicBlockList().push_back(bodyBB);
    builder->SetInsertPoint(bodyBB);
    visit(ctx->stmt());
    // Codegen of 'body' may change the current block
    bodyBB = builder->GetInsertBlock();
    builder->CreateBr(inBB);

    // out of loop
    f->getBasicBlockList().push_back(outBB);
    builder->SetInsertPoint(outBB);

    loopExits.pop_back();

    return nullptr;
}

antlrcpp::Any
CodeGenVisitor::visitForStmt(DecafParserParser::ForStmtContext *ctx) {
    cur = cur->enterScope(getForPos(ctx));

    llvm::Function *f = builder->GetInsertBlock()->getParent();

    llvm::BasicBlock *inBB = llvm::BasicBlock::Create(context, "loopin", f);
    llvm::BasicBlock *bodyBB = llvm::BasicBlock::Create(context, "loopbody");
    llvm::BasicBlock *outBB = llvm::BasicBlock::Create(context, "loopout");

    DecafParserParser::ForControlContext *forControl = ctx->forControl();

    visit(forControl->forInit());

    // go into loop
    builder->CreateBr(inBB);
    builder->SetInsertPoint(inBB);

    loopExits.push_back(outBB);

    // Convert condition to a bool by comparing non-equal to 0
    llvm::Value *condV = visit(forControl->expr()).as<llvm::Value *>();
    condV = builder->CreateICmpNE(condV, builder->getInt32(0), "loopcond");
    builder->CreateCondBr(condV, bodyBB, outBB);

    // for-statment's body
    f->getBasicBlockList().push_back(bodyBB);
    builder->SetInsertPoint(bodyBB);
    visit(ctx->stmt());
    visit(forControl->forUpdate());
    // Codegen of 'body' may change the current block
    bodyBB = builder->GetInsertBlock();
    builder->CreateBr(inBB);

    // out of loop
    f->getBasicBlockList().push_back(outBB);
    builder->SetInsertPoint(outBB);

    loopExits.pop_back();

    cur = cur->exitScope();
    return nullptr;
}

antlrcpp::Any
CodeGenVisitor::visitBreakStmt(DecafParserParser::BreakStmtContext *ctx) {
    llvm::BasicBlock *outBB = loopExits.back();

    llvm::Function *f = builder->GetInsertBlock()->getParent();
    builder->CreateBr(outBB);

    // BasicBlock for codes after 'break'. There can not be two 'br' instruction
    // in one basic block
    llvm::BasicBlock *bb = llvm::BasicBlock::Create(context, "", f);
    builder->SetInsertPoint(bb);

    return nullptr;
}

antlrcpp::Any
CodeGenVisitor::visitReturnStmt(DecafParserParser::ReturnStmtContext *ctx) {
    llvm::Value *v = visit(ctx->expr()).as<llvm::Value *>();

    if (llvm::PointerType::classof(v->getType())) {
        llvm::Function *f = builder->GetInsertBlock()->getParent();
        v = builder->CreatePointerCast(v, f->getReturnType());
    }
    builder->CreateRet(v);
    return nullptr;
}

antlrcpp::Any
CodeGenVisitor::visitPrintStmt(DecafParserParser::PrintStmtContext *ctx) {
    for (auto &expr : ctx->exprList()->expr()) {
        Type::TypeKind tk = attrManager->getExprType(expr)->getKind();
        if (tk == Type::INTEGER_TYPE) {
            std::vector<llvm::Value *> argsV = {
                visit(expr).as<llvm::Value *>()};
            llvm::Function *f = module->getFunction("_dcf_PRINT_INT");
            builder->CreateCall(f, argsV);
        } else if (tk == Type::BOOL_TYPE) {
            std::vector<llvm::Value *> argsV = {
                visit(expr).as<llvm::Value *>()};
            llvm::Function *f = module->getFunction("_dcf_PRINT_BOOL");
            builder->CreateCall(f, argsV);
        } else if (tk == Type::STRING_TYPE) {
            std::vector<llvm::Value *> argsV = {
                visit(expr).as<llvm::Value *>()};
            llvm::Function *f = module->getFunction("_dcf_PRINT_STRING");
            builder->CreateCall(f, argsV);
        }
    }
    return nullptr;
}

//     antlrcpp::Any
//     CodeGenVisitor::visitEmptyStmt(DecafParserParser::EmptyStmtContext *ctx)
//     {}

// antlrcpp::Any CodeGenVisitor::visitLocalVarDefStmt(
//     DecafParserParser::LocalVarDefStmtContext *ctx) {
//     int a;
//     return nullptr;
// }

// antlrcpp::Any
//     CodeGenVisitor::visitAssignStmt(DecafParserParser::AssignStmtContext
//     *ctx) {} antlrcpp::Any
//     CodeGenVisitor::visitExprStmt(DecafParserParser::ExprStmtContext *ctx) {}
//     antlrcpp::Any CodeGenVisitor::visitStmt(DecafParserParser::StmtContext
//     *ctx) {}
antlrcpp::Any CodeGenVisitor::visitBlock(DecafParserParser::BlockContext *ctx) {
    Pos pos = getBlockPos(ctx);

    cur = cur->enterScope(pos);
    visitChildren(ctx);
    cur = cur->exitScope();
    return nullptr;
}
//     antlrcpp::Any
//     CodeGenVisitor::visitBlockStmt(DecafParserParser::BlockStmtContext *ctx)
//     {}

antlrcpp::Any
CodeGenVisitor::visitVarSelLValue(DecafParserParser::VarSelLValueContext *ctx) {
    if (ctx->DOT()) {
        return getVarSelValue(ctx->expr(), ctx->id(), true);
    } else {
        std::string varId = ctx->id()->IDENTIFIER()->getText();
        Pos pos = getIdPos(ctx->id());
        return getVarValue(varId, pos, true);
    }
}

antlrcpp::Any CodeGenVisitor::visitIndexSelLValue(
    DecafParserParser::IndexSelLValueContext *ctx) {
    return getIndexSelValue(ctx->expr(0), ctx->expr(1), true);
}

antlrcpp::Any
CodeGenVisitor::visitThisExpr(DecafParserParser::ThisExprContext *ctx) {
    Pos pos = getThisPos(ctx);
    return getVarValue("this", pos, false);
}

antlrcpp::Any CodeGenVisitor::visitInstanceofExpr(
    DecafParserParser::InstanceofExprContext *ctx) {
    llvm::Value *objPtr = visit(ctx->expr()).as<llvm::Value *>();
    std::string cname = ctx->id()->IDENTIFIER()->getText();

    // cast the object to an array of i8*, vptr is the first element
    llvm::Type *interTy = builder->getInt8PtrTy();
    llvm::Value *i8pArr =
        builder->CreatePointerCast(objPtr, interTy->getPointerTo());
    // it points to the vptr
    llvm::Value *vpptr = builder->CreateGEP(i8pArr, builder->getInt32(0));
    llvm::Value *vptr = builder->CreateLoad(interTy, vpptr);

    return vtable->instanceOf(vptr, cname);
}

antlrcpp::Any CodeGenVisitor::visitLocalCallExpr(
    DecafParserParser::LocalCallExprContext *ctx) {
    return getVarCallValue(nullptr, ctx->id(), ctx->exprList());
}

// antlrcpp::Any
//     CodeGenVisitor::visitLogicalAndExpr(DecafParserParser::LogicalAndExprContext
//     *ctx) {}
antlrcpp::Any
CodeGenVisitor::visitUnarySubExpr(DecafParserParser::UnarySubExprContext *ctx) {
    llvm::Value *v = visit(ctx->expr()).as<llvm::Value *>();
    return builder->CreateNeg(v);
}

antlrcpp::Any
CodeGenVisitor::visitCastExpr(DecafParserParser::CastExprContext *ctx) {
    llvm::Value *objPtr = visit(ctx->expr()).as<llvm::Value *>();
    std::shared_ptr<ClassType> exprT = std::static_pointer_cast<ClassType>(
        attrManager->getExprType(ctx->expr()));
    std::string cname = ctx->id()->IDENTIFIER()->getText();

    // cast the object to an array of i8*, vptr is the first element
    llvm::Type *interTy = builder->getInt8PtrTy();
    llvm::Value *i8pArr =
        builder->CreatePointerCast(objPtr, interTy->getPointerTo());
    // it points to the vptr
    llvm::Value *vpptr = builder->CreateGEP(i8pArr, builder->getInt32(0));
    llvm::Value *vptr = builder->CreateLoad(interTy, vpptr);

    llvm::Function *f = builder->GetInsertBlock()->getParent();
    llvm::BasicBlock *chkBB = llvm::BasicBlock::Create(context, "", f);
    llvm::BasicBlock *outBB = llvm::BasicBlock::Create(context, "");

    llvm::Value *condV = builder->CreateICmpEQ(vtable->instanceOf(vptr, cname),
                                               builder->getInt32(0));

    builder->CreateCondBr(condV, chkBB, outBB);
    // halt with error string
    builder->SetInsertPoint(chkBB);
    std::string err = exprT->getName() + " cannot be cast to " + cname;
    llvm::Value *errStr = builder->CreateGlobalStringPtr(err, "", 0, module);
    llvm::Function *halt = module->getFunction("_dcf_HALT");
    builder->CreateCall(halt, {errStr});
    builder->CreateBr(outBB);

    f->getBasicBlockList().push_back(outBB);
    builder->SetInsertPoint(outBB);

    llvm::Type *dstT =
        llvm::StructType::getTypeByName(module->getContext(), cname)
            ->getPointerTo();
    return builder->CreatePointerCast(objPtr, dstT);
}

antlrcpp::Any
CodeGenVisitor::visitClassNewExpr(DecafParserParser::ClassNewExprContext *ctx) {
    std::string cname = ctx->id()->IDENTIFIER()->getText();
    llvm::Type *ct =
        llvm::StructType::getTypeByName(module->getContext(), cname);
    llvm::DataLayout dl(module);
    uint64_t size = dl.getTypeAllocSize(ct);

    std::vector<llvm::Value *> allocArgs = {builder->getIntN(targetSize, size)};
    llvm::Function *f = module->getFunction("_dcf_ALLOCATE");
    llvm::Value *allocMem = builder->CreateCall(f, allocArgs);
    llvm::Value *castedMem =
        builder->CreatePointerCast(allocMem, ct->getPointerTo());

    // Call constructor on the allocated memory
    std::vector<llvm::Value *> constrArgs = {castedMem};
    llvm::Function *constr = module->getFunction(Constructor::getName(cname));
    builder->CreateCall(constr, constrArgs);

    return castedMem;
}

// antlrcpp::Any
//     CodeGenVisitor::visitReadLineExpr(DecafParserParser::ReadLineExprContext
//     *ctx) {}

antlrcpp::Any CodeGenVisitor::visitAddictiveExpr(
    DecafParserParser::AddictiveExprContext *ctx) {
    llvm::Value *l = visit(ctx->expr(0));
    llvm::Value *r = visit(ctx->expr(1));
    llvm::Value *result = nullptr;

    if (ctx->ADD()) {
        result = builder->CreateAdd(l, r);
    } else if (ctx->SUB()) {
        result = builder->CreateSub(l, r);
    }
    return result;
}

antlrcpp::Any
CodeGenVisitor::visitIndexSelExpr(DecafParserParser::IndexSelExprContext *ctx) {
    return getIndexSelValue(ctx->expr(0), ctx->expr(1), false);
}

// antlrcpp::Any
//     CodeGenVisitor::visitParenExpr(DecafParserParser::ParenExprContext *ctx)
//     {} antlrcpp::Any
//     CodeGenVisitor::visitLogicalOrExpr(DecafParserParser::LogicalOrExprContext
//     *ctx) {}

antlrcpp::Any
CodeGenVisitor::visitVarSelExpr(DecafParserParser::VarSelExprContext *ctx) {
    return getVarSelValue(ctx->expr(), ctx->id(), false);
}

antlrcpp::Any
CodeGenVisitor::visitArrayNewExpr(DecafParserParser::ArrayNewExprContext *ctx) {
    std::shared_ptr<ArrayType> arrTy =
        std::static_pointer_cast<ArrayType>(attrManager->getExprType(ctx));
    llvm::Type *baseTy = getLLVMType(arrTy->getBase());
    llvm::DataLayout dl(module);
    llvm::Value *baseSize =
        builder->getIntN(targetSize, dl.getTypeAllocSize(baseTy));

    llvm::Value *len = visit(ctx->expr()).as<llvm::Value *>();

    // Runtime Checking: Array's bounds checking
    checkArrayLen(len);

    llvm::Value *castedLen =
        builder->CreateIntCast(len, builder->getIntNTy(targetSize), false);
    llvm::Value *arrSize = builder->CreateMul(baseSize, castedLen);

    // leave 4 bytes for storing length
    llvm::Value *total =
        builder->CreateAdd(builder->getIntN(targetSize, 4), arrSize);

    // call the allocator
    std::vector<llvm::Value *> argsV = {total};
    llvm::Function *f = module->getFunction("_dcf_ALLOCATE");
    llvm::Value *ptr = builder->CreateCall(f, argsV);

    // store the length
    ptr = builder->CreatePointerCast(ptr, llvm::Type::getInt32PtrTy(context));
    llvm::Value *lenPtr = builder->CreateGEP(ptr, builder->getInt32(0));
    builder->CreateStore(len, lenPtr);

    ptr = builder->CreatePointerCast(ptr, baseTy->getPointerTo());
    return ptr;
}

// antlrcpp::Any
//     CodeGenVisitor::visitReadIntExpr(DecafParserParser::ReadIntExprContext
//     *ctx) {}

antlrcpp::Any
CodeGenVisitor::visitVarCallExpr(DecafParserParser::VarCallExprContext *ctx) {
    return getVarCallValue(ctx->expr(), ctx->id(), ctx->exprList());
}

antlrcpp::Any
CodeGenVisitor::visitUnaryNotExpr(DecafParserParser::UnaryNotExprContext *ctx) {
    llvm::Value *v = visit(ctx->expr()).as<llvm::Value *>();
    llvm::Value *condV = builder->CreateICmpEQ(v, builder->getInt32(1));
    return builder->CreateSelect(condV, builder->getInt32(0),
                                 builder->getInt32(1));
}

antlrcpp::Any CodeGenVisitor::visitMultiplicativeExpr(
    DecafParserParser::MultiplicativeExprContext *ctx) {
    llvm::Value *l = visit(ctx->expr(0));
    llvm::Value *r = visit(ctx->expr(1));
    llvm::Value *result = nullptr;

    if (ctx->MUL()) {
        result = builder->CreateMul(l, r);
    } else if (ctx->DIV()) {
        result = builder->CreateSDiv(l, r);
    } else if (ctx->MOD()) {
        result = builder->CreateSRem(l, r);
    }
    return result;
}

antlrcpp::Any
CodeGenVisitor::visitRelationExpr(DecafParserParser::RelationExprContext *ctx) {
    llvm::Value *l = visit(ctx->expr(0));
    llvm::Value *r = visit(ctx->expr(1));
    llvm::Value *condV = nullptr;

    if (ctx->LE()) {
        condV = builder->CreateICmpSLE(l, r);
    } else if (ctx->LT()) {
        condV = builder->CreateICmpSLT(l, r);
    } else if (ctx->GE()) {
        condV = builder->CreateICmpSGE(l, r);
    } else if (ctx->GT()) {
        condV = builder->CreateICmpSGT(l, r);
    }
    return builder->CreateSelect(condV, builder->getInt32(1),
                                 builder->getInt32(0));
}

antlrcpp::Any
CodeGenVisitor::visitEqualityExpr(DecafParserParser::EqualityExprContext *ctx) {
    Type::TypeKind type = attrManager->getExprType(ctx->expr(0))->getKind();
    llvm::Value *l = visit(ctx->expr(0));
    llvm::Value *r = visit(ctx->expr(1));
    llvm::Value *condV = nullptr;

    if (type == Type::CLASS_TYPE || type == Type::ARRAY_TYPE) {
        llvm::Value *ptrDiff = builder->CreatePtrDiff(l, r);
        if (ctx->EQ()) {
            condV = builder->CreateICmpEQ(ptrDiff, builder->getInt64(0));
        } else if (ctx->NE()) {
            condV = builder->CreateICmpNE(ptrDiff, builder->getInt64(0));
        }
    } else if (type == Type::STRING_TYPE) {
        llvm::Value *cmpRes = compString(l, r);
        if (ctx->EQ()) {
            condV = builder->CreateICmpEQ(cmpRes, builder->getInt32(1));
        } else if (ctx->NE()) {
            condV = builder->CreateICmpEQ(cmpRes, builder->getInt32(0));
        }
    } else {
        if (ctx->EQ()) {
            condV = builder->CreateICmpEQ(l, r);
        } else if (ctx->NE()) {
            condV = builder->CreateICmpNE(l, r);
        }
    }

    return builder->CreateSelect(condV, builder->getInt32(1),
                                 builder->getInt32(0));
}

antlrcpp::Any
CodeGenVisitor::visitIdExpr(DecafParserParser::IdExprContext *ctx) {
    std::string name = ctx->id()->IDENTIFIER()->getText();
    Pos pos = getIdPos(ctx->id());
    return getVarValue(name, pos, false);
}

antlrcpp::Any
CodeGenVisitor::visitIntLit(DecafParserParser::IntLitContext *ctx) {
    std::string intText = ctx->INTLIT()->getText();
    int intVal = std::stoi(intText);
    return static_cast<llvm::Value *>(builder->getInt32(intVal));
}

antlrcpp::Any
CodeGenVisitor::visitBoolLit(DecafParserParser::BoolLitContext *ctx) {
    int boolVal = ctx->TRUE() ? 1 : 0;
    return static_cast<llvm::Value *>(builder->getInt32(boolVal));
}

antlrcpp::Any
CodeGenVisitor::visitNullLit(DecafParserParser::NullLitContext *ctx) {
    return static_cast<llvm::Value *>(
        llvm::ConstantPointerNull::get(llvm::Type::getInt8PtrTy(context)));
}

antlrcpp::Any
CodeGenVisitor::visitStringLit(DecafParserParser::StringLitContext *ctx) {
    std::string text = ctx->STRING_LIT()->getText();

    // Replace escapes in string
    size_t idx = 0;
    while (true) {
        idx = text.find("\\", idx);
        if (idx == std::string::npos) {
            break;
        }

        // Don't worry about bound-checking, as it is checked in lexer
        char next = text[idx + 1];
        if (next == '\\') {
            text.replace(idx, 2, "\\");
            idx += 2;
        } else if (next == '\"') {
            text.replace(idx, 2, "\"");
            idx += 2;
        } else if (next == 'n') {
            text.replace(idx, 2, "\n");
            idx += 2;
        } else if (next == 't') {
            text.replace(idx, 2, "\t");
            idx += 2;
        } else if (next == 'r') {
            text.replace(idx, 2, "\r");
            idx += 2;
        } else {
            // Wouldn't go here, already checked in lexer
            idx++;
        }
    }
    return static_cast<llvm::Value *>(
        builder->CreateGlobalStringPtr(text, "", 0, module));
}

//     antlrcpp::Any
//     CodeGenVisitor::visitUnaryOp(DecafParserParser::UnaryOpContext *ctx) {}
//     antlrcpp::Any
//     CodeGenVisitor::visitExprList(DecafParserParser::ExprListContext *ctx) {}
//     antlrcpp::Any CodeGenVisitor::visitId(DecafParserParser::IdContext *ctx)
//     {}

void CodeGenVisitor::genLLVMStruct(const std::shared_ptr<Symbol> &classSym) {
    llvm::StructType *ct =
        llvm::StructType::getTypeByName(module->getContext(), classSym->name);
    llvm::StructType *vtable = llvm::StructType::getTypeByName(
        module->getContext(), VTable::getVtableName(classSym->name));

    std::vector<llvm::Type *> contents;
    auto fields = classSym->getScope()->getOrderedSymbols();

    // class's memory construction:
    // 1. a pointer(i8*) to vtable
    // 2. all fields from all base classes
    // 3. fields in this class

    // add base-class fields
    std::string base = BaseChecker::getBase(classSym->name);
    bool hasBase = !base.empty();
    if (hasBase) {
        contents.push_back(
            llvm::StructType::getTypeByName(module->getContext(), base));
    } else {
        // ptr to vtable
        contents.push_back(builder->getInt8PtrTy());
    }

    // add fields
    std::vector<std::string> methods;
    for (auto &field : fields) {
        if (field->getKind() == Symbol::VAR) {
            contents.push_back(getLLVMType(field->type));
        } else if (field->getKind() == Symbol::METHOD) {
            methods.push_back(field->name);
            genMethodProto(classSym, field);
        }
    }

    ct->setBody(llvm::ArrayRef<llvm::Type *>(contents));
}

llvm::Type *CodeGenVisitor::getLLVMType(const std::shared_ptr<Type> &t) {
    Type::TypeKind tk = t->getKind();

    if (tk == Type::VOID_TYPE) {
        return builder->getVoidTy();
    } else if (tk == Type::INTEGER_TYPE) {
        return builder->getInt32Ty();
    } else if (tk == Type::BOOL_TYPE) {
        return builder->getInt32Ty();
    } else if (tk == Type::STRING_TYPE) {
        return builder->getInt8PtrTy();
    } else if (tk == Type::CLASS_TYPE) {
        std::string name =
            std::dynamic_pointer_cast<const ClassType>(t)->getName();
        return llvm::StructType::getTypeByName(module->getContext(), name)
            ->getPointerTo();
    } else if (tk == Type::ARRAY_TYPE) {
        std::shared_ptr<Type> baseTy =
            std::dynamic_pointer_cast<ArrayType>(t)->getBase();
        return getLLVMType(baseTy)->getPointerTo();
    }

    return nullptr;
}

llvm::Value *
CodeGenVisitor::getLLVMDefaultValue(const std::shared_ptr<Type> &t) {
    Type::TypeKind tk = t->getKind();

    if (tk == Type::INTEGER_TYPE) {
        return builder->getInt32(0);
    } else if (tk == Type::BOOL_TYPE) {
        return builder->getInt32(0);
    } else if (tk == Type::STRING_TYPE) {
        return builder->CreateGlobalStringPtr("", "", 0, module);
    } else if (tk == Type::CLASS_TYPE) {
        auto *casted = llvm::dyn_cast<llvm::PointerType>(getLLVMType(t));
        return llvm::ConstantPointerNull::get(casted);
    } else if (tk == Type::ARRAY_TYPE) {
        std::shared_ptr<ArrayType> arrTy =
            std::static_pointer_cast<ArrayType>(t);
        llvm::Type *baseTy = getLLVMType(arrTy->getBase());
        return llvm::ConstantPointerNull::get(baseTy->getPointerTo());
    }
    return nullptr;
}

void CodeGenVisitor::genClasses(
    const std::vector<std::shared_ptr<Symbol>> &classes) {
    for (auto &c : classes) {
        llvm::StructType::create(context, llvm::StringRef(c->name));
        llvm::StructType::create(context, VTable::getVtableName(c->name));
    }

    // Generate llvm-StructType for all classes
    for (auto &c : classes) {
        genLLVMStruct(c);
    }

    // Generate vtables of all classes
    vtable->generate(classes);

    Constructor constr(module, builder, vtable);
    for (auto &c : classes) {
        constr.generate(c->name);
    }
}

void CodeGenVisitor::genBuiltInProtos() {
    std::vector<
        std::tuple<std::string, llvm::Type *, std::vector<llvm::Type *>>>
        methods = {
            // Protos for built-in methods, see runtime.c
            // {name, retTy, {paraTy1, paraTy2, ... }}
            {"_dcf_ALLOCATE",
             builder->getInt8PtrTy(),
             {builder->getIntNTy(targetSize)}},
            {"_dcf_READ_LINE", builder->getInt8PtrTy(), {}},
            {"_dcf_READ_INT", builder->getInt32Ty(), {}},
            {"_dcf_STRING_EQUAL",
             builder->getInt32Ty(),
             {builder->getInt8PtrTy(), builder->getInt8PtrTy()}},
            {"_dcf_PRINT_INT", builder->getVoidTy(), {builder->getInt32Ty()}},
            {"_dcf_PRINT_STRING",
             builder->getVoidTy(),
             {builder->getInt8PtrTy()}},
            {"_dcf_PRINT_BOOL", builder->getVoidTy(), {builder->getInt32Ty()}},
            {"_dcf_HALT", builder->getVoidTy(), {builder->getInt8PtrTy()}},
            {"_dcf_rt_INSTANCE_OF",
             builder->getInt32Ty(),
             {builder->getInt8PtrTy(), builder->getInt8PtrTy()}}};

    for (auto &method : methods) {
        const std::string &name = std::get<0>(method);
        const auto &retTy = std::get<1>(method);
        const auto &paraTys = std::get<2>(method);

        llvm::FunctionType *ft = llvm::FunctionType::get(retTy, paraTys, false);
        llvm::Function::Create(ft, llvm::Function::ExternalLinkage, name,
                               module);
    }
}

void CodeGenVisitor::genMethodProto(const std::shared_ptr<Symbol> &classSym,
                                    const std::shared_ptr<Symbol> &methodSym) {
    std::shared_ptr<MethodType> type =
        std::dynamic_pointer_cast<MethodType>(methodSym->type);
    std::vector<llvm::Type *> argTypes;

    if (!methodSym->isStatic()) {
        argTypes.push_back(getLLVMType(classSym->type));
    }

    std::vector<std::shared_ptr<Type>> args = type->getArgsType();
    for (const auto &a : args) {
        argTypes.push_back(getLLVMType(a));
    }

    const std::string methodName = methodSym->name;
    const std::string className = classSym->name;

    llvm::FunctionType *ft = llvm::FunctionType::get(
        getLLVMType(type->getRetType()), argTypes, false);

    // Create the function prototype
    llvm::Function *f = llvm::Function::Create(
        ft, llvm::Function::ExternalLinkage,
        VTable::getFuncName(className, methodName), module);
}

llvm::Value *CodeGenVisitor::getVarValue(const std::string &varId,
                                         const Pos &pos, bool lValue) {
    std::shared_ptr<Symbol> sym = cur->lookupBefore(pos, varId);
    bool isField = sym->getParent()->kind == Scope::CLASS;
    llvm::Value *v = nullptr;

    if (isField) {
        llvm::Value *thisPtr = getVarValue("this", pos, false);
        v = getFieldPtr(sym, thisPtr);
    } else {
        // local variable or parameter
        v = attrManager->getSymbolLLVMVal(sym);
    }

    if (lValue) {
        return v;
    } else {
        return static_cast<llvm::Value *>(
            builder->CreateLoad(getLLVMType(sym->type), v));
    }
}

llvm::Value *
CodeGenVisitor::getIndexSelValue(DecafParserParser::ExprContext *varExpr,
                                 DecafParserParser::ExprContext *idExpr,
                                 bool lValue) {
    llvm::Value *arrV = visit(varExpr).as<llvm::Value *>();
    llvm::Value *idxV = visit(idExpr).as<llvm::Value *>();

    std::shared_ptr<ArrayType> arrTy =
        std::static_pointer_cast<ArrayType>(attrManager->getExprType(varExpr));
    std::shared_ptr<Type> baseTy = arrTy->getBase();

    auto p = getArrayLength(arrV);
    llvm::Value *len = p.first;
    llvm::Value *firstPtr = p.second;

    // Runtime Checking: Array's bounds checking
    checkArrayIdx(len, idxV);

    llvm::Value *elmPtr = builder->CreateGEP(firstPtr, idxV);

    if (lValue) {
        return elmPtr;
    } else {
        return static_cast<llvm::Value *>(
            builder->CreateLoad(getLLVMType(baseTy), elmPtr));
    }
}

// For 'expr.var', expr must be a object with class type T.
// And you can only access it within the scope of T or its descendants
llvm::Value *
CodeGenVisitor::getVarSelValue(DecafParserParser::ExprContext *expr,
                               DecafParserParser::IdContext *id, bool lValue) {
    llvm::Value *v = nullptr;
    std::string varId = id->IDENTIFIER()->getText();

    std::shared_ptr<ClassType> exprTy =
        std::static_pointer_cast<ClassType>(attrManager->getExprType(expr));
    std::shared_ptr<Scope> scope =
        global->lookup(exprTy->getName())->getScope();
    std::shared_ptr<Symbol> varSym = scope->lookup(varId);

    llvm::Value *objPtr = visit(expr).as<llvm::Value *>();

    v = getFieldPtr(varSym, objPtr);

    if (lValue) {
        return v;
    } else {
        return static_cast<llvm::Value *>(
            builder->CreateLoad(getLLVMType(varSym->type), v));
    }
}

llvm::Value *
CodeGenVisitor::getVarCallValue(DecafParserParser::ExprContext *expr,
                                DecafParserParser::IdContext *id,
                                DecafParserParser::ExprListContext *exprList) {
    std::string fname = id->IDENTIFIER()->getText();
    llvm::Value *objPtr;
    std::shared_ptr<Type> exprTy;

    if (expr) {
        // call method from a object
        exprTy = attrManager->getExprType(expr);

        if (attrManager->getIsClassName(expr)) {
            // Call static method from other class
            objPtr = nullptr;
        } else {
            objPtr = visit(expr).as<llvm::Value *>();
        }
    } else {
        // local call
        exprTy = curClass->type;
        if (curMethod->isStatic()) {
            // Call static method from self
            objPtr = nullptr;
        } else {
            objPtr = getVarValue("this", getIdPos(id), false);
        }
    }

    // special case: array.length()
    if (exprTy->getKind() == Type::ARRAY_TYPE && fname == "length") {
        return getArrayLength(objPtr).first;
    }

    std::string cname = std::static_pointer_cast<ClassType>(exprTy)->getName();
    llvm::Value *fptr = getFunctionPtr(cname, fname, objPtr);
    llvm::FunctionType *ft =
        vtable->getFunction(cname, fname)->getFunctionType();

    std::vector<llvm::Value *> argsV;

    // pass the object as 'this' for non-static method
    if (objPtr) {
        llvm::Value *castedObj =
            builder->CreatePointerCast(objPtr, ft->getParamType(0));
        argsV.push_back(castedObj);
    }

    for (size_t i = 0; i < exprList->expr().size(); i++) {
        // LLVM-IR is typed, so we need to manually cast for subtyping
        llvm::Value *v = visit(exprList->expr(i)).as<llvm::Value *>();
        size_t idx = (objPtr ? 1 : 0) + i;
        if (llvm::PointerType::classof(ft->getParamType(idx))) {
            // Make argument's type the same with parameter's
            v = builder->CreatePointerCast(v, ft->getParamType(idx));
        }
        argsV.push_back(v);
    }

    return static_cast<llvm::Value *>(builder->CreateCall(ft, fptr, argsV));
}

llvm::Value *
CodeGenVisitor::getFieldPtr(const std::shared_ptr<Symbol> &fieldSym,
                            llvm::Value *objPtr) {

    std::shared_ptr<Scope> scope = fieldSym->getParent();
    auto fields = scope->getOrderedSymbols();
    std::string name = fieldSym->name;
    int idx = 0;

    for (size_t i = 0; i < fields.size(); i++) {
        if (fields[i]->name == fieldSym->name) {
            idx = i;
            break;
        }
    }
    // the first thing in a object is vptr or baseclass-part. Skip it
    idx += 1;

    std::string cname = scope->name;
    llvm::Type *ct =
        llvm::StructType::getTypeByName(module->getContext(), cname);
    llvm::Value *casted =
        builder->CreatePointerCast(objPtr, ct->getPointerTo());
    llvm::Value *fieldPtr = builder->CreateStructGEP(ct, casted, idx);

    return fieldPtr;
}

// Get the function pointer from a object's vtable.
// When visit static function by class name(like MyClass.bar() ), objPtr is
// nullptr
llvm::Value *CodeGenVisitor::getFunctionPtr(const std::string &cname,
                                            const std::string &fname,
                                            llvm::Value *objPtr) {
    if (!objPtr) {
        return module->getFunction(VTable::getFuncName(cname, fname));
    }

    // cast the object to an array of i8*, vptr is the first element
    llvm::Type *interTy = builder->getInt8PtrTy();
    llvm::Value *i8pArr =
        builder->CreatePointerCast(objPtr, interTy->getPointerTo());
    // it points to the vptr
    llvm::Value *vpptr = builder->CreateGEP(i8pArr, builder->getInt32(0));
    llvm::Value *vptr = builder->CreateLoad(interTy, vpptr);

    return vtable->getFunctionPtr(cname, fname, vptr);
}

// return a pair of (array's length, pointer to first element)
std::pair<llvm::Value *, llvm::Value *>
CodeGenVisitor::getArrayLength(llvm::Value *arrayPtr) {
    // Array's length is stored in its first 4 bytes
    llvm::Type *interTy = builder->getInt32Ty();
    llvm::Value *casted =
        builder->CreatePointerCast(arrayPtr, interTy->getPointerTo());
    llvm::Value *lengthPtr = builder->CreateGEP(casted, builder->getInt32(0));
    llvm::Value *length = builder->CreateLoad(interTy, lengthPtr);

    llvm::Value *firstElmPtr = builder->CreateGEP(casted, builder->getInt32(1));
    firstElmPtr = builder->CreatePointerCast(firstElmPtr, arrayPtr->getType());

    return std::pair<llvm::Value *, llvm::Value *>(length, firstElmPtr);
}

llvm::Value *CodeGenVisitor::compString(llvm::Value *s1, llvm::Value *s2) {
    std::vector<llvm::Value *> argsV = {s1, s2};
    llvm::Function *comp = module->getFunction("_dcf_STRING_EQUAL");
    return builder->CreateCall(comp, argsV);
}

void CodeGenVisitor::checkArrayIdx(llvm::Value *len, llvm::Value *idx) {
    llvm::Function *f = builder->GetInsertBlock()->getParent();
    llvm::BasicBlock *chkBB = llvm::BasicBlock::Create(context, "", f);
    llvm::BasicBlock *outBB = llvm::BasicBlock::Create(context, "");

    // bad: idx >= length or idx < 0
    llvm::Value *condV = builder->CreateICmpSLT(idx, builder->getInt32(0));
    if (len) {
        condV = builder->CreateOr(condV, builder->CreateICmpSGE(idx, len));
    }

    builder->CreateCondBr(condV, chkBB, outBB);
    // halt with error string
    builder->SetInsertPoint(chkBB);
    std::string err = "Array subscript out of bounds";
    llvm::Value *errStr = builder->CreateGlobalStringPtr(err, "", 0, module);
    llvm::Function *halt = module->getFunction("_dcf_HALT");
    builder->CreateCall(halt, {errStr});
    builder->CreateBr(outBB);

    f->getBasicBlockList().push_back(outBB);
    builder->SetInsertPoint(outBB);
}

void CodeGenVisitor::checkArrayLen(llvm::Value *len) {
    //llvm::BasicBlock::Adj
    llvm::Function *f = builder->GetInsertBlock()->getParent();
    llvm::BasicBlock *chkBB = llvm::BasicBlock::Create(context, "", f);
    llvm::BasicBlock *outBB = llvm::BasicBlock::Create(context, "");

    // bad: idx >= length or idx < 0
    llvm::Value *condV = builder->CreateICmpSLT(len, builder->getInt32(0));

    builder->CreateCondBr(condV, chkBB, outBB);
    // halt with error string
    builder->SetInsertPoint(chkBB);
    std::string err = "Cannot create negative-sized array";
    llvm::Value *errStr = builder->CreateGlobalStringPtr(err, "", 0, module);
    llvm::Function *halt = module->getFunction("_dcf_HALT");
    builder->CreateCall(halt, {errStr});
    builder->CreateBr(outBB);

    f->getBasicBlockList().push_back(outBB);
    builder->SetInsertPoint(outBB);
}
