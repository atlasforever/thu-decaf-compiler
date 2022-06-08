#ifndef _TYPE_CHECKER_H_
#define _TYPE_CHECKER_H_

#include "DecafParserBaseVisitor.h"
#include "DecafParserParser.h"
#include "Scope.h"
#include "Type.h"
#include "antlr4-runtime.h"
#include "utils/ASTAttrManager.h"
#include "utils/printer.h"

class TypeChecker : public DecafParserBaseVisitor {
public:
    TypeChecker(antlr4::tree::ParseTree *ast, std::shared_ptr<Scope> global,
                std::shared_ptr<ASTAttrManager> am);
    bool check();

    virtual antlrcpp::Any
    visitClassDef(DecafParserParser::ClassDefContext *ctx) override;

    virtual antlrcpp::Any
    visitMethodDef(DecafParserParser::MethodDefContext *ctx) override;

    virtual antlrcpp::Any
    visitStmt(DecafParserParser::StmtContext *ctx) override;

    virtual antlrcpp::Any
    visitBlock(DecafParserParser::BlockContext *ctx) override;

    virtual antlrcpp::Any
    visitBlockStmt(DecafParserParser::BlockStmtContext *ctx) override;

    virtual antlrcpp::Any
    visitType(DecafParserParser::TypeContext *ctx) override;

    virtual antlrcpp::Any
    visitForControl(DecafParserParser::ForControlContext *ctx) override;

    virtual antlrcpp::Any
    visitLocalVarDef(DecafParserParser::LocalVarDefContext *ctx) override;

    virtual antlrcpp::Any
    visitAssign(DecafParserParser::AssignContext *ctx) override;

    virtual antlrcpp::Any
    visitIfStmt(DecafParserParser::IfStmtContext *ctx) override;

    virtual antlrcpp::Any
    visitWhileStmt(DecafParserParser::WhileStmtContext *ctx) override;

    virtual antlrcpp::Any
    visitForStmt(DecafParserParser::ForStmtContext *ctx) override;

    virtual antlrcpp::Any
    visitBreakStmt(DecafParserParser::BreakStmtContext *ctx) override;

    virtual antlrcpp::Any
    visitReturnStmt(DecafParserParser::ReturnStmtContext *ctx) override;

    virtual antlrcpp::Any
    visitPrintStmt(DecafParserParser::PrintStmtContext *ctx) override;

    // virtual antlrcpp::Any
    // visitLocalVarDefStmt(DecafParserParser::LocalVarDefStmtContext *ctx)
    // override;

    // virtual antlrcpp::Any
    // visitAssignStmt(DecafParserParser::AssignStmtContext *ctx) override;

    virtual antlrcpp::Any
    visitVarSelLValue(DecafParserParser::VarSelLValueContext *ctx) override;

    virtual antlrcpp::Any
    visitIndexSelLValue(DecafParserParser::IndexSelLValueContext *ctx) override;

    virtual antlrcpp::Any
    visitThisExpr(DecafParserParser::ThisExprContext *ctx) override;

    virtual antlrcpp::Any
    visitInstanceofExpr(DecafParserParser::InstanceofExprContext *ctx) override;

    virtual antlrcpp::Any
    visitLocalCallExpr(DecafParserParser::LocalCallExprContext *ctx) override;

    virtual antlrcpp::Any
    visitLogicalAndExpr(DecafParserParser::LogicalAndExprContext *ctx) override;

    virtual antlrcpp::Any
    visitLogicalOrExpr(DecafParserParser::LogicalOrExprContext *ctx) override;

    virtual antlrcpp::Any
    visitUnarySubExpr(DecafParserParser::UnarySubExprContext *ctx) override;

    virtual antlrcpp::Any
    visitCastExpr(DecafParserParser::CastExprContext *ctx) override;

    virtual antlrcpp::Any
    visitClassNewExpr(DecafParserParser::ClassNewExprContext *ctx) override;

    virtual antlrcpp::Any
    visitReadLineExpr(DecafParserParser::ReadLineExprContext *ctx) override;

    virtual antlrcpp::Any
    visitAddictiveExpr(DecafParserParser::AddictiveExprContext *ctx) override;

    virtual antlrcpp::Any
    visitIndexSelExpr(DecafParserParser::IndexSelExprContext *ctx) override;

    virtual antlrcpp::Any
    visitParenExpr(DecafParserParser::ParenExprContext *ctx) override;

    virtual antlrcpp::Any
    visitVarSelExpr(DecafParserParser::VarSelExprContext *ctx) override;

    virtual antlrcpp::Any
    visitArrayNewExpr(DecafParserParser::ArrayNewExprContext *ctx) override;

    virtual antlrcpp::Any
    visitReadIntExpr(DecafParserParser::ReadIntExprContext *ctx) override;

    virtual antlrcpp::Any
    visitVarCallExpr(DecafParserParser::VarCallExprContext *ctx) override;

    virtual antlrcpp::Any
    visitLitExpr(DecafParserParser::LitExprContext *ctx) override;

    virtual antlrcpp::Any
    visitUnaryNotExpr(DecafParserParser::UnaryNotExprContext *ctx) override;

    virtual antlrcpp::Any visitMultiplicativeExpr(
        DecafParserParser::MultiplicativeExprContext *ctx) override;

    virtual antlrcpp::Any
    visitRelationExpr(DecafParserParser::RelationExprContext *ctx) override;

    virtual antlrcpp::Any
    visitEqualityExpr(DecafParserParser::EqualityExprContext *ctx) override;

    virtual antlrcpp::Any
    visitIdExpr(DecafParserParser::IdExprContext *ctx) override;

    // virtual antlrcpp::Any visitId(DecafParserParser::IdContext *ctx)
    // override;

    // virtual antlrcpp::Any
    // visitExprList(DecafParserParser::ExprListContext *ctx) override;

    virtual antlrcpp::Any
    visitIntLit(DecafParserParser::IntLitContext *ctx) override;

    virtual antlrcpp::Any
    visitBoolLit(DecafParserParser::BoolLitContext *ctx) override;

    virtual antlrcpp::Any
    visitNullLit(DecafParserParser::NullLitContext *ctx) override;

    virtual antlrcpp::Any
    visitStringLit(DecafParserParser::StringLitContext *ctx) override;

private:
    antlr4::tree::ParseTree *ast;
    std::shared_ptr<Scope> global;
    std::shared_ptr<Scope> cur;
    std::shared_ptr<ASTAttrManager> attrManager;

    bool typeFailed = false;

    std::shared_ptr<Symbol> curClass;
    std::shared_ptr<Symbol> curMethod;

    int loopLevel = 0;
    bool allowClassName = false;

    bool checkArgs(std::shared_ptr<Symbol> methodSym, Pos callPos,
                   DecafParserParser::ExprListContext *argsCtx);
    bool isCompat(std::shared_ptr<Type> a, std::shared_ptr<Type> b);

    std::shared_ptr<Type>
    checkCall(std::shared_ptr<Symbol> classSym, const std::string &methodName,
              DecafParserParser::ExprListContext *exprlist, const Pos &pos,
              bool thisClass, bool isClassName);

    antlrcpp::Any checkVarSel(DecafParserParser::ExprContext *exprCtx,
                              DecafParserParser::IdContext *idCtx);

    antlrcpp::Any checkVar(DecafParserParser::IdContext *idCtx);

    antlrcpp::Any checkIndexSel(DecafParserParser::ExprContext *varExpr,
                                DecafParserParser::ExprContext *idxExpr,
                                const Pos &pos);

    void fail(const Pos &pos, CompileErrors err,
              const std::vector<std::string> &texts);
    std::shared_ptr<Type>
    returnExprType(DecafParserParser::ExprContext *exprCtx,
                   const std::shared_ptr<Type> &type);
};

#endif