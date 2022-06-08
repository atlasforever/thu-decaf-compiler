#ifndef _AST_PRINTER_H_
#define _AST_PRINTER_H_

#include "DecafParserBaseListener.h"
#include "DecafParserBaseVisitor.h"
#include "antlr4-runtime.h"
#include <vector>

class ASTPrinter : public DecafParserBaseVisitor {
private:
    antlr4::Parser *parser = nullptr;
    int exDepth = 0;
    int depth = -1;

    void printIndent();
    void printIndent(int depth);
    void printRuleName(antlr4::ParserRuleContext *ctx, bool withPos);
    void printString(const std::string &text, antlr4::Token *posTok);
    void printString(const std::string &text, antlr4::Token *posTok, int depth);
    void printString(antlr4::Token *tok);
    void printList(const std::vector<antlr4::ParserRuleContext *> &v);

public:
    ASTPrinter(antlr4::Parser *p);

    virtual antlrcpp::Any
    visitTopLevel(DecafParserParser::TopLevelContext *ctx) override;

    virtual antlrcpp::Any
    visitExtendClause(DecafParserParser::ExtendClauseContext *ctx) override;

    virtual antlrcpp::Any
    visitClassDef(DecafParserParser::ClassDefContext *ctx) override;

    virtual antlrcpp::Any
    visitField(DecafParserParser::FieldContext *ctx) override;

    virtual antlrcpp::Any
    visitVarDef(DecafParserParser::VarDefContext *ctx) override;

    virtual antlrcpp::Any
    visitMethodDef(DecafParserParser::MethodDefContext *ctx) override;

    virtual antlrcpp::Any visitVar(DecafParserParser::VarContext *ctx) override;

    virtual antlrcpp::Any
    visitParaVarDef(DecafParserParser::ParaVarDefContext *ctx) override;

    virtual antlrcpp::Any
    visitVarList(DecafParserParser::VarListContext *ctx) override;

    virtual antlrcpp::Any
    visitType(DecafParserParser::TypeContext *ctx) override;

    virtual antlrcpp::Any
    visitBlock(DecafParserParser::BlockContext *ctx) override;

    virtual antlrcpp::Any
    visitLocalVarDef(DecafParserParser::LocalVarDefContext *ctx) override;

    virtual antlrcpp::Any
    visitAssign(DecafParserParser::AssignContext *ctx) override;

    virtual antlrcpp::Any
    visitExprStmt(DecafParserParser::ExprStmtContext *ctx) override;

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
    visitCastExpr(DecafParserParser::CastExprContext *ctx) override;

    virtual antlrcpp::Any
    visitClassNewExpr(DecafParserParser::ClassNewExprContext *ctx) override;

    virtual antlrcpp::Any
    visitUnarySubExpr(DecafParserParser::UnarySubExprContext *ctx) override;

    virtual antlrcpp::Any
    visitUnaryNotExpr(DecafParserParser::UnaryNotExprContext *ctx) override;

    virtual antlrcpp::Any visitMultiplicativeExpr(
        DecafParserParser::MultiplicativeExprContext *ctx) override;

    virtual antlrcpp::Any
    visitAddictiveExpr(DecafParserParser::AddictiveExprContext *ctx) override;

    virtual antlrcpp::Any
    visitRelationExpr(DecafParserParser::RelationExprContext *ctx) override;

    virtual antlrcpp::Any
    visitEqualityExpr(DecafParserParser::EqualityExprContext *ctx) override;

    virtual antlrcpp::Any
    visitLogicalAndExpr(DecafParserParser::LogicalAndExprContext *ctx) override;

    virtual antlrcpp::Any
    visitLogicalOrExpr(DecafParserParser::LogicalOrExprContext *ctx) override;

    virtual antlrcpp::Any
    visitReadLineExpr(DecafParserParser::ReadLineExprContext *ctx) override;

    virtual antlrcpp::Any
    visitIndexSelExpr(DecafParserParser::IndexSelExprContext *ctx) override;

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
    visitIdExpr(DecafParserParser::IdExprContext *ctx) override;

    virtual antlrcpp::Any
    visitIntLit(DecafParserParser::IntLitContext *ctx) override;

    virtual antlrcpp::Any
    visitBoolLit(DecafParserParser::BoolLitContext *ctx) override;

    virtual antlrcpp::Any
    visitNullLit(DecafParserParser::NullLitContext *ctx) override;

    virtual antlrcpp::Any
    visitStringLit(DecafParserParser::StringLitContext *ctx) override;

    virtual antlrcpp::Any
    visitExprList(DecafParserParser::ExprListContext *ctx) override;

    virtual antlrcpp::Any visitId(DecafParserParser::IdContext *ctx) override;

    virtual antlrcpp::Any
    visitIfStmt(DecafParserParser::IfStmtContext *ctx) override;

    virtual antlrcpp::Any
    visitWhileStmt(DecafParserParser::WhileStmtContext *ctx) override;

    virtual antlrcpp::Any
    visitReturnStmt(DecafParserParser::ReturnStmtContext *ctx) override;

    virtual antlrcpp::Any
    visitPrintStmt(DecafParserParser::PrintStmtContext *ctx) override;
};
#endif