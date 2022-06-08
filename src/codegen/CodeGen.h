#ifndef _CODE_GEN_H_
#define _CODE_GEN_H_

#include "VTable.h"
#include "parser/antlr/DecafParserBaseVisitor.h"
#include "semantic/Scope.h"
#include "utils/ASTAttrManager.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include <memory>

class CodeGenVisitor : public DecafParserBaseVisitor {
public:
    CodeGenVisitor(antlr4::tree::ParseTree *ast, std::shared_ptr<Scope> &scope,
                   std::shared_ptr<ASTAttrManager> &am);
    ~CodeGenVisitor();

    void codegen();

    virtual antlrcpp::Any
    visitTopLevel(DecafParserParser::TopLevelContext *ctx) override;
    virtual antlrcpp::Any
    visitClassDef(DecafParserParser::ClassDefContext *ctx) override;
    // virtual antlrcpp::Any visitField(DecafParserParser::FieldContext *ctx)
    // override; virtual antlrcpp::Any
    // visitVarDef(DecafParserParser::VarDefContext *ctx) override;
    virtual antlrcpp::Any
    visitMethodDef(DecafParserParser::MethodDefContext *ctx) override;
    // virtual antlrcpp::Any visitVar(DecafParserParser::VarContext *ctx)
    // override; virtual antlrcpp::Any
    // visitParaVarDef(DecafParserParser::ParaVarDefContext *ctx) override;
    // virtual antlrcpp::Any visitVarList(DecafParserParser::VarListContext
    // *ctx) override; virtual antlrcpp::Any
    // visitClassType(DecafParserParser::ClassTypeContext *ctx) override;
    // virtual antlrcpp::Any visitType(DecafParserParser::TypeContext *ctx)
    // override; virtual antlrcpp::Any
    // visitForInit(DecafParserParser::ForInitContext *ctx) override; virtual
    // antlrcpp::Any visitForUpdate(DecafParserParser::ForUpdateContext *ctx)
    // override; virtual antlrcpp::Any
    // visitForControl(DecafParserParser::ForControlContext *ctx) override;
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
    // virtual antlrcpp::Any visitEmptyStmt(DecafParserParser::EmptyStmtContext
    // *ctx) override; virtual antlrcpp::Any
    // visitLocalVarDefStmt(DecafParserParser::LocalVarDefStmtContext *ctx)
    // override; virtual antlrcpp::Any
    // visitAssignStmt(DecafParserParser::AssignStmtContext *ctx) override;
    // virtual antlrcpp::Any visitExprStmt(DecafParserParser::ExprStmtContext
    // *ctx) override; virtual antlrcpp::Any
    // visitStmt(DecafParserParser::StmtContext *ctx) override;
    virtual antlrcpp::Any
    visitBlock(DecafParserParser::BlockContext *ctx) override;
    // virtual antlrcpp::Any visitBlockStmt(DecafParserParser::BlockStmtContext
    // *ctx) override;
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
    // virtual antlrcpp::Any
    // visitLogicalAndExpr(DecafParserParser::LogicalAndExprContext *ctx)
    // override;
    virtual antlrcpp::Any
    visitUnarySubExpr(DecafParserParser::UnarySubExprContext *ctx) override;
    virtual antlrcpp::Any visitCastExpr(DecafParserParser::CastExprContext
    *ctx) override;
    virtual antlrcpp::Any
    visitClassNewExpr(DecafParserParser::ClassNewExprContext *ctx) override;
    // virtual antlrcpp::Any
    // visitReadLineExpr(DecafParserParser::ReadLineExprContext *ctx) override;
    virtual antlrcpp::Any
    visitAddictiveExpr(DecafParserParser::AddictiveExprContext *ctx) override;
    virtual antlrcpp::Any
    visitIndexSelExpr(DecafParserParser::IndexSelExprContext *ctx) override;
    // virtual antlrcpp::Any visitParenExpr(DecafParserParser::ParenExprContext
    // *ctx) override; virtual antlrcpp::Any
    // visitLogicalOrExpr(DecafParserParser::LogicalOrExprContext *ctx)
    // override;
    virtual antlrcpp::Any
    visitVarSelExpr(DecafParserParser::VarSelExprContext *ctx) override;
    virtual antlrcpp::Any
    visitArrayNewExpr(DecafParserParser::ArrayNewExprContext *ctx) override;
    // virtual antlrcpp::Any
    // visitReadIntExpr(DecafParserParser::ReadIntExprContext *ctx) override;
    virtual antlrcpp::Any
    visitVarCallExpr(DecafParserParser::VarCallExprContext *ctx) override;
    // virtual antlrcpp::Any visitLitExpr(DecafParserParser::LitExprContext
    // *ctx) override;
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
    virtual antlrcpp::Any
    visitIntLit(DecafParserParser::IntLitContext *ctx) override;
    virtual antlrcpp::Any
    visitBoolLit(DecafParserParser::BoolLitContext *ctx) override;
    virtual antlrcpp::Any
    visitNullLit(DecafParserParser::NullLitContext *ctx) override;
    virtual antlrcpp::Any
    visitStringLit(DecafParserParser::StringLitContext *ctx) override;
    // virtual antlrcpp::Any visitUnaryOp(DecafParserParser::UnaryOpContext
    // *ctx) override; virtual antlrcpp::Any
    // visitExprList(DecafParserParser::ExprListContext *ctx) override; virtual
    // antlrcpp::Any visitId(DecafParserParser::IdContext *ctx) override;

private:
    // AST and symbol-table stuff
    antlr4::tree::ParseTree *ast;
    std::shared_ptr<Scope> scope;
    std::shared_ptr<Scope> cur;
    std::shared_ptr<Scope> global;
    std::shared_ptr<ASTAttrManager> attrManager;
    std::shared_ptr<Symbol> curClass;
    std::shared_ptr<Symbol> curMethod;

    // LLVM
    llvm::Module *module;
    llvm::IRBuilder<> *builder;
    llvm::LLVMContext context;
    int targetSize;
    // for break-statment
    std::list<llvm::BasicBlock *> loopExits;

    VTable *vtable;

    void genLLVMStruct(const std::shared_ptr<Symbol> &classSym);
    void genClasses(const std::vector<std::shared_ptr<Symbol>> &classes);
    void genMethodProto(const std::shared_ptr<Symbol> &classSym,
                        const std::shared_ptr<Symbol> &methodSym);
    void genBuiltInProtos();
    llvm::Type *getLLVMType(const std::shared_ptr<Type> &t);
    llvm::Value *getLLVMDefaultValue(const std::shared_ptr<Type> &t);
    llvm::Value *getIndexSelValue(DecafParserParser::ExprContext *varExpr,
                                  DecafParserParser::ExprContext *idExpr,
                                  bool lValue);
    llvm::Value *getVarValue(const std::string &varId, const Pos &pos,
                             bool lValue);
    llvm::Value *getVarSelValue(DecafParserParser::ExprContext *expr,
                                DecafParserParser::IdContext *id, bool lValue);
    llvm::Value *getVarCallValue(DecafParserParser::ExprContext *expr,
                                 DecafParserParser::IdContext *id,
                                 DecafParserParser::ExprListContext *exprList);
    llvm::Value *getFieldPtr(const std::shared_ptr<Symbol> &fieldSym,
                             llvm::Value *objPtr);
    llvm::Value *getFunctionPtr(const std::string &cname,
                                const std::string &fname, llvm::Value *objPtr);
    std::pair<llvm::Value *, llvm::Value *>
    getArrayLength(llvm::Value *arrayPtr);
    llvm::Value *compString(llvm::Value *s1, llvm::Value *s2);
    void checkArrayIdx(llvm::Value *len, llvm::Value *idx);
    void checkArrayLen(llvm::Value *len);
};

#endif