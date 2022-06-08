#include "TypeChecker.h"
#include "ArrayType.h"
#include "BaseChecker.h"
#include "Pos.h"

TypeChecker::TypeChecker(antlr4::tree::ParseTree *ast,
                         std::shared_ptr<Scope> global,
                         std::shared_ptr<ASTAttrManager> am) {
    this->ast = ast;
    this->global = global;
    cur = global;
    attrManager = am;
}

bool TypeChecker::check() {
    visit(ast);
    return !typeFailed;
}

antlrcpp::Any
TypeChecker::visitClassDef(DecafParserParser::ClassDefContext *ctx) {
    Pos pos = getClassPos(ctx);
    std::shared_ptr<Scope> classScope = cur->enterScope(pos);

    if (classScope) {
        cur = classScope;
        curClass = classScope->getSymbol();
        for (auto &x : ctx->field()) {
            visit(x);
        }
        cur = cur->exitScope();
        curClass = nullptr;
    }
    return nullptr;
}

antlrcpp::Any
TypeChecker::visitMethodDef(DecafParserParser::MethodDefContext *ctx) {
    Pos pos = getMethodPos(ctx);

    cur = cur->enterScope(pos);
    curMethod = cur->getSymbol();
    visit(ctx->block());
    cur = cur->exitScope();

    // detect missing return
    std::shared_ptr<Type> retType =
        std::dynamic_pointer_cast<MethodType>(curMethod->type)->getRetType();
    if (retType->getKind() != Type::VOID_TYPE && !attrManager->getHasRet(ctx->block())) {
        Pos pos = getBlockPos(ctx->block());
        fail(pos, CompileErrors::MISSING_RETURN, {});
    }

    curMethod = nullptr;
    return nullptr;
}

antlrcpp::Any TypeChecker::visitType(DecafParserParser::TypeContext *ctx) {
    std::shared_ptr<Type> ret;

    if (ctx->INT()) {
        ret = std::make_shared<BuiltInType>(Type::INTEGER_TYPE);
    } else if (ctx->BOOL()) {
        ret = std::make_shared<BuiltInType>(Type::BOOL_TYPE);
    } else if (ctx->STRING()) {
        ret = std::make_shared<BuiltInType>(Type::STRING_TYPE);
    } else if (ctx->VOID()) {
        ret = std::make_shared<BuiltInType>(Type::VOID_TYPE);
    } else if (ctx->classType()) {
        ret = std::make_shared<ClassType>(ctx->classType()->id()->getText());
    } else if (ctx->LBRACKET()) {
        if (ctx->type()->VOID()) {
            Pos pos = getTokenPos(ctx->type()->VOID()->getSymbol());
            fail(pos, CompileErrors::VOID_ARRAY, {});
            ret = std::make_shared<ErrorType>();
        } else {
            std::shared_ptr<Type> bt =
                visit(ctx->type()).as<std::shared_ptr<Type>>();
            ret = std::make_shared<ArrayType>(bt);
        }
    }

    return ret;
}

antlrcpp::Any
TypeChecker::visitForControl(DecafParserParser::ForControlContext *ctx) {
    if (ctx->forInit()) {
        visit(ctx->forInit());
    }

    std::shared_ptr<Type> testT = visit(ctx->expr());
    if (testT->getKind() != Type::BOOL_TYPE) {
        Pos pos = getTokenPos(ctx->expr()->getStart());
        fail(pos, CompileErrors::TEST_NOT_BOOL, {});
    }

    if (ctx->forUpdate()) {
        visit(ctx->forUpdate());
    }
    return nullptr;
}

antlrcpp::Any TypeChecker::visitStmt(DecafParserParser::StmtContext *ctx) {
    size_t n = ctx->children.size();
    for (size_t i = 0; i < n; i++) {
        visit(ctx->children[i]);
        if (attrManager->getHasRet(ctx->children[i])) {
            attrManager->setHasRet(ctx, true);
            break;
        }
    }
    return nullptr;
}

antlrcpp::Any TypeChecker::visitBlock(DecafParserParser::BlockContext *ctx) {
    Pos pos = getBlockPos(ctx);

    cur = cur->enterScope(pos);
    visitChildren(ctx);
    cur = cur->exitScope();

    size_t n = ctx->blockStmt().size();
    if (n > 0 && attrManager->getHasRet(ctx->blockStmt(n - 1))) {
        attrManager->setHasRet(ctx, true);
    }
    return nullptr;
}

antlrcpp::Any
TypeChecker::visitBlockStmt(DecafParserParser::BlockStmtContext *ctx) {
    size_t n = ctx->children.size();
    for (size_t i = 0; i < n; i++) {
        visit(ctx->children[i]);
        if (attrManager->getHasRet(ctx->children[i])) {
            attrManager->setHasRet(ctx, true);
        }
    }
    return nullptr;
}

antlrcpp::Any
TypeChecker::visitLocalVarDef(DecafParserParser::LocalVarDefContext *ctx) {
    if (!ctx->bop) {
        return nullptr;
    }

    std::string id = ctx->var()->id()->IDENTIFIER()->getSymbol()->getText();
    std::shared_ptr<Type> lt = cur->lookup(id)->type;
    std::shared_ptr<Type> rt = visit(ctx->expr());

    if (!isCompat(rt, lt)) {
        Pos pos = getTokenPos(ctx->bop);
        fail(pos, CompileErrors::INCOMPAT_BIN_OP,
             {lt->toString(), ctx->bop->getText(), rt->toString()});
    }
    return nullptr;
}

antlrcpp::Any TypeChecker::visitAssign(DecafParserParser::AssignContext *ctx) {
    std::shared_ptr<Type> lt = visit(ctx->lValue());
    std::shared_ptr<Type> rt = visit(ctx->expr());

    if (!isCompat(rt, lt)) {
        Pos pos = getTokenPos(ctx->bop);
        fail(pos, CompileErrors::INCOMPAT_BIN_OP,
             {lt->toString(), ctx->bop->getText(), rt->toString()});
    }

    return nullptr;
}

antlrcpp::Any TypeChecker::visitIfStmt(DecafParserParser::IfStmtContext *ctx) {
    std::shared_ptr<Type> testT = visit(ctx->expr());
    if (testT->getKind() != Type::BOOL_TYPE) {
        Pos pos = getTokenPos(ctx->expr()->getStart());
        fail(pos, CompileErrors::TEST_NOT_BOOL, {});
    }

    visit(ctx->stmt(0));
    if (ctx->ELSE()) {
        visit(ctx->stmt(1));
    }

    if (ctx->ELSE() && attrManager->getHasRet(ctx->stmt(0)) &&
        attrManager->getHasRet(ctx->stmt(1))) {
        attrManager->setHasRet(ctx, true);
    }

    return nullptr;
}

antlrcpp::Any
TypeChecker::visitWhileStmt(DecafParserParser::WhileStmtContext *ctx) {
    loopLevel++;

    std::shared_ptr<Type> testT = visit(ctx->expr());
    if (testT->getKind() != Type::BOOL_TYPE) {
        Pos pos = getTokenPos(ctx->expr()->getStart());
        fail(pos, CompileErrors::TEST_NOT_BOOL, {});
    }

    visit(ctx->stmt());
    loopLevel--;

    return nullptr;
}

antlrcpp::Any
TypeChecker::visitForStmt(DecafParserParser::ForStmtContext *ctx) {
    Pos pos = getForPos(ctx);

    loopLevel++;
    cur = cur->enterScope(pos);
    visit(ctx->forControl());
    visit(ctx->stmt());
    cur = cur->exitScope();
    loopLevel--;

    return nullptr;
}

antlrcpp::Any
TypeChecker::visitBreakStmt(DecafParserParser::BreakStmtContext *ctx) {
    Pos pos = getBreakPos(ctx);
    if (loopLevel == 0) {
        fail(pos, CompileErrors::BREAK_OUTSIDE_LOOP, {});
    }
    return nullptr;
}

antlrcpp::Any
TypeChecker::visitReturnStmt(DecafParserParser::ReturnStmtContext *ctx) {

    attrManager->setHasRet(ctx, true);

    if (!ctx->expr()) {
        std::static_pointer_cast<Type>(std::make_shared<BuiltInType>(Type::VOID_TYPE));
    }

    std::shared_ptr<Type> rt = visit(ctx->expr());

    if (rt->getKind() == Type::ERROR_TYPE) {
        return rt;
    }

    Pos pos = getTokenPos(ctx->RETURN()->getSymbol());
    std::shared_ptr<Type> mrt =
        std::dynamic_pointer_cast<MethodType>(curMethod->type)->getRetType();
    if (!isCompat(rt, mrt)) {
        fail(pos, CompileErrors::INCOMPAT_RETURN,
             {rt->toString(), mrt->toString()});
        return mrt;
    }
    return mrt;
}

antlrcpp::Any
TypeChecker::visitPrintStmt(DecafParserParser::PrintStmtContext *ctx) {

    size_t n = ctx->exprList()->expr().size();
    for (size_t i = 0; i < n; i++) {
        DecafParserParser::ExprContext *expr = ctx->exprList()->expr(i);
        std::shared_ptr<Type> argT = visit(expr);

        Type::TypeKind argTK = argT->getKind();
        if (argTK != Type::ERROR_TYPE && argTK != Type::INTEGER_TYPE &&
            argTK != Type::BOOL_TYPE && argTK != Type::STRING_TYPE) {
            Pos pos = getTokenPos(expr->getStart());
            fail(pos, CompileErrors::INCOMPAT_ARG,
                 {std::to_string(i + 1), argT->toString(), "int/bool/string"});
        }
    }

    return nullptr;
}

// antlrcpp::Any
// visitLocalVarDefStmt(DecafParserParser::LocalVarDefStmtContext *ctx){}

// antlrcpp::Any visitAssignStmt(DecafParserParser::AssignStmtContext
// *ctx){}

antlrcpp::Any
TypeChecker::visitVarSelLValue(DecafParserParser::VarSelLValueContext *ctx) {
    if (ctx->DOT()) {
        return checkVarSel(ctx->expr(), ctx->id());
    } else {
        return checkVar(ctx->id());
    }
}

antlrcpp::Any TypeChecker::visitIndexSelLValue(
    DecafParserParser::IndexSelLValueContext *ctx) {
    return checkIndexSel(ctx->expr(0), ctx->expr(1),
                         getTokenPos(ctx->LBRACKET()->getSymbol()));
}

antlrcpp::Any
TypeChecker::visitThisExpr(DecafParserParser::ThisExprContext *ctx) {
    Pos pos = getThisPos(ctx);

    if (curMethod->isStatic()) {
        fail(pos, CompileErrors::THIS_IN_STATIC, {});
        return returnExprType(ctx, std::make_shared<ErrorType>());
    }
    return returnExprType(ctx, std::make_shared<ClassType>(curClass->name));
}

antlrcpp::Any TypeChecker::visitInstanceofExpr(
    DecafParserParser::InstanceofExprContext *ctx) {
    std::shared_ptr<Type> expr = visit(ctx->expr());
    Type::TypeKind exprType = expr->getKind();
    if (exprType != Type::ERROR_TYPE && exprType != Type::CLASS_TYPE) {
        fail(getInstanceofPos(ctx), CompileErrors::NOT_CLASS,
             {expr->toString()});
    }

    antlr4::Token *tok = ctx->id()->IDENTIFIER()->getSymbol();
    std::string id = tok->getText();
    std::shared_ptr<Symbol> classSym = cur->lookup(id);
    // id must be a class's name
    if (!classSym || classSym->getKind() != Symbol::CLASS) {
        fail(getTokenPos(tok), CompileErrors::CLASS_NOT_FOUND, {id});
    }
    return returnExprType(ctx, std::make_shared<BuiltInType>(Type::BOOL_TYPE));
}

antlrcpp::Any
TypeChecker::visitLocalCallExpr(DecafParserParser::LocalCallExprContext *ctx) {
    std::string methodName = ctx->id()->IDENTIFIER()->getText();
    Pos pos = getTokenPos(ctx->LPAREN()->getSymbol());
    std::shared_ptr<Type> t = checkCall(curClass, methodName, ctx->exprList(), pos, true, false);
    return returnExprType(ctx, t);
}

antlrcpp::Any TypeChecker::visitLogicalAndExpr(
    DecafParserParser::LogicalAndExprContext *ctx) {
    Pos pos = getTokenPos(ctx->bop);

    std::shared_ptr<Type> lh = visit(ctx->expr(0));
    std::shared_ptr<Type> rh = visit(ctx->expr(1));
    Type::TypeKind lt = lh->getKind();
    Type::TypeKind rt = rh->getKind();

    if (lt != Type::ERROR_TYPE && rt != Type::ERROR_TYPE &&
        (lt != Type::BOOL_TYPE || rt != Type::BOOL_TYPE)) {
        fail(pos, CompileErrors::INCOMPAT_BIN_OP,
             {lh->toString(), ctx->bop->getText(), rh->toString()});
    }
        return returnExprType(ctx, std::make_shared<BuiltInType>(Type::BOOL_TYPE));
}

antlrcpp::Any
TypeChecker::visitLogicalOrExpr(DecafParserParser::LogicalOrExprContext *ctx) {
    Pos pos = getTokenPos(ctx->bop);

    std::shared_ptr<Type> lh = visit(ctx->expr(0));
    std::shared_ptr<Type> rh = visit(ctx->expr(1));
    Type::TypeKind lt = lh->getKind();
    Type::TypeKind rt = rh->getKind();

    if (lt != Type::ERROR_TYPE && rt != Type::ERROR_TYPE &&
        (lt != Type::BOOL_TYPE || rt != Type::BOOL_TYPE)) {
        fail(pos, CompileErrors::INCOMPAT_BIN_OP,
             {lh->toString(), ctx->bop->getText(), rh->toString()});
    }
    return returnExprType(ctx, std::make_shared<BuiltInType>(Type::BOOL_TYPE));
}

antlrcpp::Any
TypeChecker::visitUnarySubExpr(DecafParserParser::UnarySubExprContext *ctx) {
    Pos pos = getTokenPos(ctx->uop);
    std::shared_ptr<Type> t = visit(ctx->expr());
    Type::TypeKind tk = t->getKind();

    if (tk != Type::ERROR_TYPE && tk != Type::INTEGER_TYPE) {
        fail(pos, CompileErrors::INCOMPAT_UN_OP,
             {ctx->uop->getText(), t->toString()});
    }
    return returnExprType(ctx, std::make_shared<BuiltInType>(Type::INTEGER_TYPE));
}

antlrcpp::Any
TypeChecker::visitCastExpr(DecafParserParser::CastExprContext *ctx) {
    std::shared_ptr<Type> expr = visit(ctx->expr());
    Type::TypeKind exprType = expr->getKind();
    if (exprType != Type::ERROR_TYPE && exprType != Type::CLASS_TYPE) {
        fail(getTokenPos(ctx->expr()->getStart()), CompileErrors::NOT_CLASS,
             {expr->toString()});
    }

    antlr4::Token *tok = ctx->id()->IDENTIFIER()->getSymbol();
    std::string id = tok->getText();
    std::shared_ptr<Symbol> classSym = cur->lookup(id);
    if (!classSym || classSym->getKind() != Symbol::CLASS) {
        fail(getTokenPos(tok), CompileErrors::CLASS_NOT_FOUND, {id});
    }

    return returnExprType(ctx, classSym->type);
}

antlrcpp::Any
TypeChecker::visitClassNewExpr(DecafParserParser::ClassNewExprContext *ctx) {
    std::string id = ctx->id()->IDENTIFIER()->getText();
    Pos pos = getTokenPos(ctx->NEW()->getSymbol());

    std::shared_ptr<Symbol> sym = cur->lookup(id);
    if (sym && sym->getKind() == Symbol::CLASS) {
        return returnExprType(ctx, std::make_shared<ClassType>(id));
    } else {
        fail(pos, CompileErrors::CLASS_NOT_FOUND, {id});
        return returnExprType(ctx, std::make_shared<ErrorType>());
    }
}

antlrcpp::Any
TypeChecker::visitReadLineExpr(DecafParserParser::ReadLineExprContext *ctx) {
    return returnExprType(ctx, std::make_shared<BuiltInType>(Type::STRING_TYPE));
}

antlrcpp::Any
TypeChecker::visitAddictiveExpr(DecafParserParser::AddictiveExprContext *ctx) {
    Pos pos = getTokenPos(ctx->bop);
    std::shared_ptr<Type> lh = visit(ctx->expr(0));
    std::shared_ptr<Type> rh = visit(ctx->expr(1));
    Type::TypeKind lt = lh->getKind();
    Type::TypeKind rt = rh->getKind();
    if (lt != Type::ERROR_TYPE && rt != Type::ERROR_TYPE &&
        (lt != Type::INTEGER_TYPE || rt != Type::INTEGER_TYPE)) {
        fail(pos, CompileErrors::INCOMPAT_BIN_OP,
             {lh->toString(), ctx->bop->getText(), rh->toString()});
    }
    return returnExprType(ctx, std::make_shared<BuiltInType>(Type::INTEGER_TYPE));
}

antlrcpp::Any
TypeChecker::visitIndexSelExpr(DecafParserParser::IndexSelExprContext *ctx) {
    return returnExprType(ctx, checkIndexSel(ctx->expr(0), ctx->expr(1),
                         getTokenPos(ctx->LBRACKET()->getSymbol())));
}

antlrcpp::Any
TypeChecker::visitParenExpr(DecafParserParser::ParenExprContext *ctx) {
    return returnExprType(ctx, visit(ctx->expr()));
}

antlrcpp::Any
TypeChecker::visitVarSelExpr(DecafParserParser::VarSelExprContext *ctx) {
    return returnExprType(ctx, checkVarSel(ctx->expr(), ctx->id()));
}

antlrcpp::Any
TypeChecker::visitArrayNewExpr(DecafParserParser::ArrayNewExprContext *ctx) {
    std::shared_ptr<Type> base = visit(ctx->type());
    std::shared_ptr<Type> idx = visit(ctx->expr());

    Pos idxPos = getTokenPos(ctx->expr()->start);
    Type::TypeKind idxType = idx->getKind();

    if (idxType != Type::ERROR_TYPE && idxType != Type::INTEGER_TYPE) {
        fail(idxPos, CompileErrors::NEW_ARRY_LEN_NOT_INT, {});
    }

    if (base->getKind() == Type::ERROR_TYPE) {
        return returnExprType(ctx, base);
    } else {
        return returnExprType(ctx, std::make_shared<ArrayType>(base));
    }
}

antlrcpp::Any
TypeChecker::visitReadIntExpr(DecafParserParser::ReadIntExprContext *ctx) {
    return returnExprType(ctx, std::make_shared<BuiltInType>(Type::INTEGER_TYPE));
}

antlrcpp::Any
TypeChecker::visitVarCallExpr(DecafParserParser::VarCallExprContext *ctx) {

    allowClassName = true;
    std::shared_ptr<Type> exprT = visit(ctx->expr());
    allowClassName = false;

    std::string methodName = ctx->id()->IDENTIFIER()->getSymbol()->getText();
    Pos pos = getTokenPos(ctx->LPAREN()->getSymbol());

    if (exprT->getKind() == Type::ERROR_TYPE) {
        return returnExprType(ctx, exprT);
    }

    // pre-defined "length()" for array
    // NO arguments to array.length()
    if (exprT->getKind() == Type::ARRAY_TYPE && methodName == "length") {
        size_t argCount = ctx->exprList()->expr().size();
        if (argCount > 0) {
            fail(pos, CompileErrors::BAD_ARG_COUNT,
                 {"length", "0", std::to_string(argCount)});
        }
        return returnExprType(ctx, std::make_shared<BuiltInType>(Type::INTEGER_TYPE));
    }

    // cannot access field of non-CLASS type
    if (exprT->getKind() != Type::CLASS_TYPE) {
        fail(pos, CompileErrors::CANNOT_ACCESS_FIELD,
             {methodName, exprT->toString()});
        return returnExprType(ctx, std::make_shared<ErrorType>());
    }

    std::string className =
        std::dynamic_pointer_cast<ClassType>(exprT)->getName();
    std::shared_ptr<Symbol> classSym = global->lookup(className);
    std::shared_ptr<Type> t = checkCall(classSym, methodName, ctx->exprList(), pos, false,
                     attrManager->getIsClassName(ctx->expr()));
    return returnExprType(ctx, t);
}

antlrcpp::Any
TypeChecker::visitLitExpr(DecafParserParser::LitExprContext *ctx) {
    return returnExprType(ctx, visitChildren(ctx));
}

antlrcpp::Any
TypeChecker::visitUnaryNotExpr(DecafParserParser::UnaryNotExprContext *ctx) {
    Pos pos = getTokenPos(ctx->uop);
    std::shared_ptr<Type> t = visit(ctx->expr());
    Type::TypeKind tk = t->getKind();

    if (tk != Type::ERROR_TYPE && tk != Type::BOOL_TYPE) {
        fail(pos, CompileErrors::INCOMPAT_UN_OP,
             {ctx->uop->getText(), t->toString()});
    }
    return returnExprType(ctx, std::make_shared<BuiltInType>(Type::BOOL_TYPE));
}

antlrcpp::Any TypeChecker::visitMultiplicativeExpr(
    DecafParserParser::MultiplicativeExprContext *ctx) {
    Pos pos = getTokenPos(ctx->bop);

    std::shared_ptr<Type> lh = visit(ctx->expr(0));
    std::shared_ptr<Type> rh = visit(ctx->expr(1));
    Type::TypeKind lt = lh->getKind();
    Type::TypeKind rt = rh->getKind();

    if (lt != Type::ERROR_TYPE && rt != Type::ERROR_TYPE &&
        (lt != Type::INTEGER_TYPE || rt != Type::INTEGER_TYPE)) {
        fail(pos, CompileErrors::INCOMPAT_BIN_OP,
             {lh->toString(), ctx->bop->getText(), rh->toString()});
    }
    return returnExprType(ctx, std::make_shared<BuiltInType>(Type::INTEGER_TYPE));
}

antlrcpp::Any
TypeChecker::visitRelationExpr(DecafParserParser::RelationExprContext *ctx) {
    Pos pos = getTokenPos(ctx->bop);

    std::shared_ptr<Type> lh = visit(ctx->expr(0));
    std::shared_ptr<Type> rh = visit(ctx->expr(1));
    Type::TypeKind lt = lh->getKind();
    Type::TypeKind rt = rh->getKind();

    if (lt != Type::ERROR_TYPE && rt != Type::ERROR_TYPE &&
        (lt != Type::INTEGER_TYPE || rt != Type::INTEGER_TYPE)) {
        fail(pos, CompileErrors::INCOMPAT_BIN_OP,
             {lh->toString(), ctx->bop->getText(), rh->toString()});
    }
    return returnExprType(ctx, std::make_shared<BuiltInType>(Type::BOOL_TYPE));
}

antlrcpp::Any
TypeChecker::visitEqualityExpr(DecafParserParser::EqualityExprContext *ctx) {
    Pos pos = getTokenPos(ctx->bop);
    std::shared_ptr<Type> lh = visit(ctx->expr(0));
    std::shared_ptr<Type> rh = visit(ctx->expr(1));

    if (!isCompat(lh, rh) && !isCompat(rh, lh)) {
        fail(pos, CompileErrors::INCOMPAT_BIN_OP,
             {lh->toString(), ctx->bop->getText(), rh->toString()});
    }
    return returnExprType(ctx, std::make_shared<BuiltInType>(Type::BOOL_TYPE));
}

antlrcpp::Any TypeChecker::visitIdExpr(DecafParserParser::IdExprContext *ctx) {
    std::shared_ptr<Type> vt = checkVar(ctx->id());
    if (attrManager->getIsClassName(ctx->id())) {
        attrManager->setIsClassName(ctx, true);
    }
    return returnExprType(ctx, vt);
}

// antlrcpp::Any
// TypeChecker::visitExprList(DecafParserParser::ExprListContext *ctx) {
//     std::vector<std::shared_ptr<Type>> typelist;

//     for (auto &x : ctx->expr()) {
//         std::shared_ptr<Type> t = visit(x);
//         typelist.push_back(t);
//     }
//     return typelist;
// }

antlrcpp::Any TypeChecker::visitIntLit(DecafParserParser::IntLitContext *ctx) {
    return std::static_pointer_cast<Type>(
        std::make_shared<BuiltInType>(Type::INTEGER_TYPE));
}

antlrcpp::Any
TypeChecker::visitBoolLit(DecafParserParser::BoolLitContext *ctx) {
    return std::static_pointer_cast<Type>(
        std::make_shared<BuiltInType>(Type::BOOL_TYPE));
}

antlrcpp::Any
TypeChecker::visitNullLit(DecafParserParser::NullLitContext *ctx) {
    return std::static_pointer_cast<Type>(
        std::make_shared<BuiltInType>(Type::NULL_TYPE));
}

antlrcpp::Any
TypeChecker::visitStringLit(DecafParserParser::StringLitContext *ctx) {
    return std::static_pointer_cast<Type>(
        std::make_shared<BuiltInType>(Type::STRING_TYPE));
}

bool TypeChecker::checkArgs(std::shared_ptr<Symbol> methodSym, Pos callPos,
                            DecafParserParser::ExprListContext *argsCtx) {
    std::vector<std::shared_ptr<Type>> parasType =
        (std::dynamic_pointer_cast<MethodType>(methodSym->type))->getArgsType();

    if (argsCtx->expr().size() != parasType.size()) {
        fail(callPos, CompileErrors::BAD_ARG_COUNT,
             {methodSym->name, std::to_string(parasType.size()),
              std::to_string(argsCtx->expr().size())});
        return false;
    }

    for (size_t i = 0; i < parasType.size(); i++) {
        std::shared_ptr<Type> argT = visit(argsCtx->expr(i));
        if (!isCompat(argT, parasType[i])) {
            fail(getTokenPos(argsCtx->expr(i)->getStart()),
                 CompileErrors::INCOMPAT_ARG,
                 {std::to_string(i + 1), argT->toString(),
                  parasType[i]->toString()});
            return false;
        }
    }

    return true;
}

bool TypeChecker::isCompat(std::shared_ptr<Type> a, std::shared_ptr<Type> b) {
    Type::Relation rel = a->compare(b);
    return (rel == Type::SAMETPYE || rel == Type::SUBTYPE);
}

std::shared_ptr<Type>
TypeChecker::checkCall(std::shared_ptr<Symbol> classSym,
                       const std::string &methodName,
                       DecafParserParser::ExprListContext *exprlist,
                       const Pos &pos, bool thisClass, bool isClassName) {
    std::shared_ptr<Symbol> methodSym =
        classSym->getScope()->lookup(methodName);
    if (!methodSym) {
        fail(pos, CompileErrors::FIELD_NOT_FOUND,
             {methodName, classSym->type->toString()});
        return std::static_pointer_cast<Type>(std::make_shared<ErrorType>());
    }

    if (methodSym->getKind() != Symbol::METHOD) {
        fail(pos, CompileErrors::NOT_A_METHOD,
             {methodName, curClass->type->toString()});
        return std::static_pointer_cast<Type>(std::make_shared<ErrorType>());
    }

    // require called method to be static
    if (thisClass) {
        if (curMethod->isStatic() && !methodSym->isStatic()) {
            if (isClassName) {
                fail(pos, CompileErrors::CANNOT_ACCESS_FIELD,
                     {methodName, classSym->type->toString()});
            } else {
                fail(pos, CompileErrors::REF_NON_STATIC,
                     {methodName, curMethod->name});
            }
            return std::static_pointer_cast<Type>(
                std::make_shared<ErrorType>());
        }
    } else {
        if (isClassName && !methodSym->isStatic()) {
            fail(pos, CompileErrors::CANNOT_ACCESS_FIELD,
                 {methodName, classSym->type->toString()});
            return std::static_pointer_cast<Type>(
                std::make_shared<ErrorType>());
        }
    }

    if (!checkArgs(methodSym, pos, exprlist)) {
        return std::static_pointer_cast<Type>(std::make_shared<ErrorType>());
    }

    return std::dynamic_pointer_cast<MethodType>(methodSym->type)->getRetType();
}

antlrcpp::Any TypeChecker::checkVarSel(DecafParserParser::ExprContext *exprCtx,
                                       DecafParserParser::IdContext *idCtx) {
    // prefix expr cannot be class name (e.g., MyClass.foo)
    // But for a better error hint, just allowed here
    allowClassName = true;
    std::shared_ptr<Type> exprT = visit(exprCtx);
    allowClassName = false;

    if (exprT->getKind() == Type::ERROR_TYPE) {
        return exprT;
    }

    std::string varName = idCtx->IDENTIFIER()->getSymbol()->getText();
    Pos pos = getIdPos(idCtx);

    // prefix expr cannot be class name (e.g., MyClass.foo)
    if (attrManager->getIsClassName(exprCtx)) {
        fail(pos, CompileErrors::CANNOT_ACCESS_FIELD,
             {varName, curClass->type->toString()});
        return std::static_pointer_cast<Type>(std::make_shared<ErrorType>());
    }

    // only class-variable has field
    if (exprT->getKind() != Type::CLASS_TYPE) {
        fail(pos, CompileErrors::CANNOT_ACCESS_FIELD,
             {varName, exprT->toString()});
        return std::static_pointer_cast<Type>(std::make_shared<ErrorType>());
    }

    // cannot access protected fields of other unrelated classes
    if (!isCompat(curClass->type, exprT)) {
        fail(pos, CompileErrors::FIELD_NOT_ACCESS,
             {varName, exprT->toString()});
        return std::static_pointer_cast<Type>(std::make_shared<ErrorType>());
    }

    // expr's class has no such member
    std::shared_ptr<ClassType> ct = std::static_pointer_cast<ClassType>(exprT);
    std::shared_ptr<Scope> scope = global->lookup(ct->getName())->getScope();
    std::shared_ptr<Symbol> varSym = scope->lookup(varName);
    if (!varSym) {
        fail(pos, CompileErrors::FIELD_NOT_FOUND,
             {varName, exprT->toString()});
        return std::static_pointer_cast<Type>(std::make_shared<ErrorType>());
    }

    // that member is not a field
    if (varSym->getKind() != Symbol::VAR) {
        fail(pos, CompileErrors::CANNOT_ACCESS_FIELD,
             {varName, exprT->toString()});
        return std::static_pointer_cast<Type>(std::make_shared<ErrorType>());
    }

    if (curMethod->isStatic()) {
        fail(pos, CompileErrors::REF_NON_STATIC, {varName, curMethod->name});
        return std::static_pointer_cast<Type>(std::make_shared<ErrorType>());
    }

    return varSym->type;
}

antlrcpp::Any TypeChecker::checkVar(DecafParserParser::IdContext *idCtx) {
    std::string id = idCtx->IDENTIFIER()->getText();
    Pos pos = getIdPos(idCtx);
    // variables can not be used before defination
    std::shared_ptr<Symbol> preSym = cur->lookupBefore(pos, id);

    if (preSym) {
        // variable can be a class name
        if (allowClassName && preSym->getKind() == Symbol::CLASS) {
            attrManager->setIsClassName(idCtx, true);
            return preSym->type;
        }

        if (preSym->getKind() == Symbol::VAR) {
            // cannot access non-static field in static field
            bool isMember = preSym->getParent()->kind == Scope::Kind::CLASS;
            if (isMember && curMethod->isStatic()) {
                fail(pos, CompileErrors::REF_NON_STATIC, {id, curMethod->name});
                return std::static_pointer_cast<Type>(
                    std::make_shared<ErrorType>());
            }
            return preSym->type;
        }
    }

    fail(pos, CompileErrors::UNDECLARE_VAR, {id});
    return std::static_pointer_cast<Type>(std::make_shared<ErrorType>());
}

antlrcpp::Any
TypeChecker::checkIndexSel(DecafParserParser::ExprContext *varExpr,
                           DecafParserParser::ExprContext *idxExpr,
                           const Pos &pos) {
    std::shared_ptr<Type> ret =
        std::static_pointer_cast<Type>(std::make_shared<ErrorType>());

    std::shared_ptr<Type> varT = visit(varExpr);
    Type::TypeKind varTK = varT->getKind();
    if (varTK == Type::ARRAY_TYPE) {
        ret = std::dynamic_pointer_cast<ArrayType>(varT)->getBase();
    } else if (varTK != Type::ERROR_TYPE) {
        Pos p = getTokenPos(varExpr->getStart());
        fail(p, CompileErrors::INDEX_SEL_NONARRAY, {});
    }

    std::shared_ptr<Type> indexT = visit(idxExpr);
    if (indexT->getKind() != Type::INTEGER_TYPE &&
        indexT->getKind() != Type::ERROR_TYPE) {
        fail(pos, CompileErrors::BAD_ARRAY_INDEX, {});
    }

    return ret;
}

void TypeChecker::fail(const Pos &pos, CompileErrors err,
                       const std::vector<std::string> &texts) {
    typeFailed = true;
    reportErrorText(pos, err, texts);
}

std::shared_ptr<Type> TypeChecker::returnExprType(DecafParserParser::ExprContext *exprCtx, const std::shared_ptr<Type> &type) {
    attrManager->setExprType(exprCtx, type);
    return type;
}
