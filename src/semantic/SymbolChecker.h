#ifndef _SYMBOL_CHECKER_H_
#define _SYMBOL_CHECKER_H_

#include "Scope.h"
#include "parser/antlr/DecafParserBaseVisitor.h"

class SymbolChecker : public DecafParserBaseVisitor {
public:
    enum class Phase { CHECK_CLASS, CHECK_BASE, CHECK_MEMBER };

    Phase phase = Phase::CHECK_CLASS;

    SymbolChecker(antlr4::tree::ParseTree *ast);

    std::shared_ptr<Scope> buildTable();

    virtual antlrcpp::Any
    visitTopLevel(DecafParserParser::TopLevelContext *ctx) override;

    virtual antlrcpp::Any
    visitClassDef(DecafParserParser::ClassDefContext *ctx) override;

    virtual antlrcpp::Any
    visitVarDef(DecafParserParser::VarDefContext *ctx) override;

    virtual antlrcpp::Any
    visitMethodDef(DecafParserParser::MethodDefContext *ctx) override;

    virtual antlrcpp::Any
    visitVarList(DecafParserParser::VarListContext *ctx) override;

    virtual antlrcpp::Any
    visitForStmt(DecafParserParser::ForStmtContext *ctx) override;

    virtual antlrcpp::Any
    visitBlock(DecafParserParser::BlockContext *ctx) override;

    virtual antlrcpp::Any visitLocalVarDef(
        DecafParserParser::LocalVarDefContext *ctx) override;

private:
    std::shared_ptr<Scope> cur;
    std::shared_ptr<Scope> globalScope;
    antlr4::tree::ParseTree *ast;
    bool symbolFailed = false;

    antlrcpp::Any addClasses(DecafParserParser::ClassDefContext *ctx);
    antlrcpp::Any checkBase(DecafParserParser::ClassDefContext *ctx);
    std::shared_ptr<Type> getType(DecafParserParser::TypeContext *ctx);

    std::shared_ptr<MethodType>
    getMethodType(DecafParserParser::MethodDefContext *ctx);

    bool checkMain();
};
#endif