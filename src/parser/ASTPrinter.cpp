#include "ASTPrinter.h"
#include <algorithm>
#include <iostream>
#include <string>

using namespace std;
using namespace antlr4;

static std::string getPosString(const Token *tok) {
    std::string s;
    return s.append(" @ (")
        .append(std::to_string(tok->getLine()))
        .append(",")
        .append(std::to_string(tok->getCharPositionInLine() + 1))
        .append(")");
}

ASTPrinter::ASTPrinter(Parser *p) { this->parser = p; }

void ASTPrinter::printIndent() {
    for (int i = 0; i < depth; i++) {
        std::cout << "    ";
    }
}

void ASTPrinter::printIndent(int depth) {
    for (int i = 0; i < depth; i++) {
        std::cout << "    ";
    }
}

void ASTPrinter::printRuleName(antlr4::ParserRuleContext *ctx, bool withPos) {
    std::string name = parser->getRuleNames().at(ctx->getRuleIndex());
    std::transform(name.begin(), name.begin() + 1, name.begin(), ::toupper);

    printIndent();
    std::cout << name;
    if (withPos) {
        Token *t = ctx->getStart();
        std::cout << getPosString(t);
    }
    std::cout << std::endl;
}

void ASTPrinter::printString(const std::string &text, Token *posTok) {
    std::string pos;
    const string &content = text.empty() ? "<none>" : text;

    if (text.empty()) {
    }
    if (posTok) {
        pos.append(" @ (")
            .append(std::to_string(posTok->getLine()))
            .append(",")
            .append(std::to_string(posTok->getCharPositionInLine() + 1))
            .append(")");
    }

    printIndent();
    std::cout << content << pos << std::endl;
}

void ASTPrinter::printString(const std::string &text, Token *posTok,
                             int depth) {
    int bak = this->depth;

    this->depth = depth;
    printString(text, posTok);
    this->depth = bak;
}

void ASTPrinter::printString(Token *tok) {
    printIndent();
    if (!tok) {
        std::cout << "<none>" << std::endl;
    } else {
        std::string pos;
        pos.append(" @ (")
            .append(std::to_string(tok->getLine()))
            .append(",")
            .append(std::to_string(tok->getCharPositionInLine() + 1))
            .append(")");
        std::cout << tok->getText() << pos << std::endl;
    }
}

void ASTPrinter::printList(const std::vector<antlr4::ParserRuleContext *> &v) {
    depth++;

    printIndent();
    std::cout << "List" << std::endl;

    if (v.size() == 0) {
        printIndent(depth + 1);
        std::cout << "<empty>" << std::endl;
    } else {
        for (ParserRuleContext *x : v) {
            visit(x);
        }
    }

    depth--;
}

antlrcpp::Any
ASTPrinter::visitTopLevel(DecafParserParser::TopLevelContext *ctx) {
    depth++;

    auto classDefs = ctx->classDef();

    printString("TopLevel", ctx->getStart());
    printList(vector<ParserRuleContext *>(classDefs.begin(), classDefs.end()));

    depth--;

    return nullptr;
}

antlrcpp::Any
ASTPrinter::visitExtendClause(DecafParserParser::ExtendClauseContext *ctx) {
    visit(ctx->id());
    return nullptr;
}

antlrcpp::Any
ASTPrinter::visitClassDef(DecafParserParser::ClassDefContext *ctx) {
    depth++;
    auto fields = ctx->field();

    printString("ClassDef", ctx->getStart());
    visit(ctx->id());
    if (ctx->extendClause()) {
        visit(ctx->extendClause());
    } else {
        printString("", nullptr, depth + 1);
    }
    printList(vector<ParserRuleContext *>(fields.begin(), fields.end()));

    depth--;
    return nullptr;
}

antlrcpp::Any ASTPrinter::visitField(DecafParserParser::FieldContext *ctx) {
    if (ctx->varDef()) {
        visit(ctx->varDef());
    } else if (ctx->methodDef()) {
        visit(ctx->methodDef());
    }

    return nullptr;
}

antlrcpp::Any ASTPrinter::visitVarDef(DecafParserParser::VarDefContext *ctx) {
    depth++;

    printString("VarDef", ctx->var()->id()->getStart());
    visit(ctx->var());
    printString("", nullptr, depth + 1);

    depth--;
    return nullptr;
}

antlrcpp::Any
ASTPrinter::visitMethodDef(DecafParserParser::MethodDefContext *ctx) {
    depth++;

    printString("MethodDef", ctx->id()->getStart());
    if (ctx->STATIC()) {
        printString("STATIC", nullptr, depth + 1);
    }
    visit(ctx->id());
    visit(ctx->type());
    visit(ctx->varList());
    visit(ctx->block());

    depth--;
    return nullptr;
}

antlrcpp::Any ASTPrinter::visitVar(DecafParserParser::VarContext *ctx) {
    visit(ctx->type());
    visit(ctx->id());

    return nullptr;
}

antlrcpp::Any ASTPrinter::visitVarList(DecafParserParser::VarListContext *ctx) {
    auto paraVarDefs = ctx->paraVarDef();
    printList(
        vector<ParserRuleContext *>(paraVarDefs.begin(), paraVarDefs.end()));
    return nullptr;
}

antlrcpp::Any
ASTPrinter::visitParaVarDef(DecafParserParser::ParaVarDefContext *ctx) {
    depth++;
    printString("LocalVarDef", ctx->var()->id()->getStart());
    visit(ctx->var());
    printString("", nullptr, depth + 1);
    depth--;

    return nullptr;
}

antlrcpp::Any ASTPrinter::visitType(DecafParserParser::TypeContext *ctx) {
    depth++;

    if (ctx->INT()) {
        printString("TInt", ctx->INT()->getSymbol());
    } else if (ctx->BOOL()) {
        printString("TBool", ctx->BOOL()->getSymbol());
    } else if (ctx->STRING()) {
        printString("TString", ctx->STRING()->getSymbol());
    } else if (ctx->VOID()) {
        printString("TVoid", ctx->VOID()->getSymbol());
    } else if (ctx->classType()) {
        printString("TClass", ctx->classType()->getStart());
        visit(ctx->classType()->id());
    } else if (ctx->type()) {
        printString("TArray", ctx->type()->getStart());
        visit(ctx->type());
    }

    depth--;
    return nullptr;
}

antlrcpp::Any
ASTPrinter::visitBlock(DecafParserParser::BlockContext *ctx) {
    depth++;
    auto stmts = ctx->blockStmt();

    printString("Block", ctx->LBRACE()->getSymbol());
    printList(vector<ParserRuleContext *>(stmts.begin(), stmts.end()));

    depth--;
    return nullptr;
}

antlrcpp::Any ASTPrinter::visitLocalVarDef(
    DecafParserParser::LocalVarDefContext *ctx) {
    depth++;

    printString("LocalVarDef", ctx->var()->id()->getStart());
    visit(ctx->var());
    if (ctx->ASSIGN()) {
        visit(ctx->expr());
    } else {
        printString("", nullptr, depth + 1);
    }

    depth--;
    return nullptr;
}

antlrcpp::Any
ASTPrinter::visitAssign(DecafParserParser::AssignContext *ctx) {
    depth++;

    printString("Assign", ctx->ASSIGN()->getSymbol());
    visit(ctx->lValue());
    visit(ctx->expr());

    depth--;
    return nullptr;
}

antlrcpp::Any
ASTPrinter::visitExprStmt(DecafParserParser::ExprStmtContext *ctx) {
    depth++;

    printString("ExprEval", nullptr);
    visit(ctx->expr());

    depth--;
    return nullptr;
}

antlrcpp::Any ASTPrinter::visitIfStmt(DecafParserParser::IfStmtContext *ctx) {
    depth++;

    printString("If", ctx->IF()->getSymbol());
    visit(ctx->expr());
    visit(ctx->stmt(0));
    if (ctx->ELSE()) {
        visit(ctx->stmt(1));
    } else {
        printString("", nullptr, depth + 1);
    }

    depth--;
    return nullptr;
}

antlrcpp::Any
ASTPrinter::visitWhileStmt(DecafParserParser::WhileStmtContext *ctx) {
    depth++;

    printString("While", ctx->WHILE()->getSymbol());
    visit(ctx->expr());
    visit(ctx->stmt());

    depth--;
    return nullptr;
}

antlrcpp::Any
ASTPrinter::visitReturnStmt(DecafParserParser::ReturnStmtContext *ctx) {
    depth++;

    printString("Return", ctx->RETURN()->getSymbol());
    visit(ctx->expr());

    depth--;
    return nullptr;
}

antlrcpp::Any
ASTPrinter::visitPrintStmt(DecafParserParser::PrintStmtContext *ctx) {
    depth++;

    printString("Print", ctx->PRINT()->getSymbol());
    visit(ctx->exprList());

    depth--;
    return nullptr;
}

antlrcpp::Any
ASTPrinter::visitVarSelLValue(DecafParserParser::VarSelLValueContext *ctx) {
    depth++;

    printString("VarSel", ctx->id()->getStart());
    if (ctx->expr()) {
        visit(ctx->expr());
    } else {
        printString("", nullptr, depth + 1);
    }
    visit(ctx->id());

    depth--;
    return nullptr;
}

antlrcpp::Any
ASTPrinter::visitIndexSelLValue(DecafParserParser::IndexSelLValueContext *ctx) {
    depth++;

    printString("IndexSel", ctx->LBRACKET()->getSymbol());
    visit(ctx->expr(0));
    visit(ctx->expr(1));

    depth--;
    return nullptr;
}

antlrcpp::Any
ASTPrinter::visitThisExpr(DecafParserParser::ThisExprContext *ctx) {
    depth++;

    printString("This", ctx->THIS()->getSymbol());

    depth--;
    return nullptr;
}

antlrcpp::Any
ASTPrinter::visitInstanceofExpr(DecafParserParser::InstanceofExprContext *ctx) {
    depth++;

    printString("ClassTest", ctx->INSTANCEOF()->getSymbol());
    visit(ctx->expr());
    visit(ctx->id());

    depth--;
    return nullptr;
}

antlrcpp::Any
ASTPrinter::visitLocalCallExpr(DecafParserParser::LocalCallExprContext *ctx) {
    depth++;

    printString("Call", ctx->LPAREN()->getSymbol());
    printString("", nullptr, depth + 1);
    visit(ctx->id());
    visit(ctx->exprList());

    depth--;
    return nullptr;
}

antlrcpp::Any
ASTPrinter::visitCastExpr(DecafParserParser::CastExprContext *ctx) {
    depth++;

    printString("ClassCast", nullptr);
    visit(ctx->expr());
    visit(ctx->id());

    depth--;
    return nullptr;
}

antlrcpp::Any
ASTPrinter::visitClassNewExpr(DecafParserParser::ClassNewExprContext *ctx) {
    depth++;

    printString("NewClass", ctx->NEW()->getSymbol());
    visit(ctx->id());

    depth--;
    return nullptr;
}

antlrcpp::Any ASTPrinter::visitMultiplicativeExpr(
    DecafParserParser::MultiplicativeExprContext *ctx) {
    depth++;

    printString("Binary", ctx->bop);
    if (ctx->MUL()) {
        printString("MUL", nullptr, depth + 1);
    } else if (ctx->DIV()) {
        printString("DIV", nullptr, depth + 1);
    } else if (ctx->MOD()) {
        printString("MOD", nullptr, depth + 1);
    }
    visit(ctx->expr(0));
    visit(ctx->expr(1));

    depth--;
    return nullptr;
}

antlrcpp::Any
ASTPrinter::visitAddictiveExpr(DecafParserParser::AddictiveExprContext *ctx) {
    depth++;

    printString("Binary", ctx->bop);
    if (ctx->ADD()) {
        printString("ADD", nullptr, depth + 1);
    } else if (ctx->SUB()) {
        printString("SUB", nullptr, depth + 1);
    }
    visit(ctx->expr(0));
    visit(ctx->expr(1));

    depth--;
    return nullptr;
}

antlrcpp::Any
ASTPrinter::visitRelationExpr(DecafParserParser::RelationExprContext *ctx) {
    depth++;

    printString("Binary", ctx->bop);
    if (ctx->LE()) {
        printString("LE", nullptr, depth + 1);
    } else if (ctx->LT()) {
        printString("LT", nullptr, depth + 1);
    } else if (ctx->GE()) {
        printString("GE", nullptr, depth + 1);
    } else if (ctx->GT()) {
        printString("GT", nullptr, depth + 1);
    }
    visit(ctx->expr(0));
    visit(ctx->expr(1));

    depth--;
    return nullptr;
}

antlrcpp::Any
ASTPrinter::visitEqualityExpr(DecafParserParser::EqualityExprContext *ctx) {
    depth++;

    printString("Binary", ctx->bop);
    if (ctx->EQ()) {
        printString("EQ", nullptr, depth + 1);
    } else if (ctx->NE()) {
        printString("NE", nullptr, depth + 1);
    }
    visit(ctx->expr(0));
    visit(ctx->expr(1));

    depth--;
    return nullptr;
}

antlrcpp::Any
ASTPrinter::visitLogicalAndExpr(DecafParserParser::LogicalAndExprContext *ctx) {
    depth++;

    printString("Binary", ctx->AND()->getSymbol());
    if (ctx->AND()) {
        printString("AND", nullptr, depth + 1);
    }
    visit(ctx->expr(0));
    visit(ctx->expr(1));

    depth--;
    return nullptr;
}

antlrcpp::Any
ASTPrinter::visitLogicalOrExpr(DecafParserParser::LogicalOrExprContext *ctx) {
    depth++;

    printString("Binary", ctx->OR()->getSymbol());
    if (ctx->OR()) {
        printString("OR", nullptr, depth + 1);
    }
    visit(ctx->expr(0));
    visit(ctx->expr(1));

    depth--;
    return nullptr;
}

antlrcpp::Any
ASTPrinter::visitReadLineExpr(DecafParserParser::ReadLineExprContext *ctx) {
    depth++;

    printString("ReadLine", ctx->READLINE()->getSymbol());

    depth--;
    return nullptr;
}

antlrcpp::Any
ASTPrinter::visitUnarySubExpr(DecafParserParser::UnarySubExprContext *ctx) {
    depth++;

    printString("Unary", ctx->SUB()->getSymbol());
    printString("NEG", nullptr, depth + 1);
    visit(ctx->expr());

    depth--;
    return nullptr;
}

antlrcpp::Any
ASTPrinter::visitUnaryNotExpr(DecafParserParser::UnaryNotExprContext *ctx) {
    depth++;

    printString("Unary", ctx->NOT()->getSymbol());
    printString("NOT", nullptr, depth + 1);
    visit(ctx->expr());

    depth--;
    return nullptr;
}

antlrcpp::Any
ASTPrinter::visitArrayNewExpr(DecafParserParser::ArrayNewExprContext *ctx) {
    depth++;

    printString("NewArray", ctx->NEW()->getSymbol());
    visit(ctx->type());
    visit(ctx->expr());

    depth--;
    return nullptr;
}

antlrcpp::Any
ASTPrinter::visitReadIntExpr(DecafParserParser::ReadIntExprContext *ctx) {
    depth++;

    printString("ReadInt", ctx->READINTEGER()->getSymbol());

    depth--;
    return nullptr;
}

antlrcpp::Any
ASTPrinter::visitVarCallExpr(DecafParserParser::VarCallExprContext *ctx) {
    depth++;

    printString("Call", ctx->LPAREN()->getSymbol());
    visit(ctx->expr());
    visit(ctx->id());
    visit(ctx->exprList());

    depth--;
    return nullptr;
}

antlrcpp::Any ASTPrinter::visitLitExpr(DecafParserParser::LitExprContext *ctx) {
    return visitChildren(ctx);
}

antlrcpp::Any ASTPrinter::visitIdExpr(DecafParserParser::IdExprContext *ctx) {
    depth++;

    printString("VarSel", ctx->id()->getStart());
    printString("", nullptr, depth + 1);
    visit(ctx->id());

    depth--;
    return nullptr;
}

antlrcpp::Any ASTPrinter::visitIntLit(DecafParserParser::IntLitContext *ctx) {
    depth++;

    printString("IntLit", ctx->INTLIT()->getSymbol());
    printString(ctx->INTLIT()->getText(), nullptr, depth + 1);

    depth--;
    return nullptr;
}

antlrcpp::Any ASTPrinter::visitBoolLit(DecafParserParser::BoolLitContext *ctx) {
    depth++;

    if (ctx->TRUE()) {
        printString("BoolLit", ctx->TRUE()->getSymbol());
        printString("true", nullptr, depth + 1);
    } else if (ctx->FALSE()) {
        printString("BoolLit", ctx->FALSE()->getSymbol());
        printString("false", nullptr, depth + 1);
    }

    depth--;
    return nullptr;
}

antlrcpp::Any ASTPrinter::visitNullLit(DecafParserParser::NullLitContext *ctx) {
    depth++;

    printString("NullLit", ctx->NULLLIT()->getSymbol());

    depth--;
    return nullptr;
}

antlrcpp::Any
ASTPrinter::visitStringLit(DecafParserParser::StringLitContext *ctx) {
    depth++;

    printString("StringLit", ctx->STRING_LIT()->getSymbol());
    printString("\"" + ctx->STRING_LIT()->getText() + "\"", nullptr, depth + 1);

    depth--;
    return nullptr;
}

antlrcpp::Any
ASTPrinter::visitVarSelExpr(DecafParserParser::VarSelExprContext *ctx) {
    depth++;

    printString("VarSel", ctx->id()->getStart());
    visit(ctx->expr());
    visit(ctx->id());

    depth--;
    return nullptr;
}

antlrcpp::Any
ASTPrinter::visitIndexSelExpr(DecafParserParser::IndexSelExprContext *ctx) {
    depth++;

    printString("IndexSel", ctx->LBRACKET()->getSymbol());
    visit(ctx->expr(0));
    visit(ctx->expr(1));

    depth--;
    return nullptr;
}

antlrcpp::Any
ASTPrinter::visitExprList(DecafParserParser::ExprListContext *ctx) {
    auto exprs = ctx->expr();

    printList(vector<ParserRuleContext *>(exprs.begin(), exprs.end()));
    return nullptr;
}

antlrcpp::Any ASTPrinter::visitId(DecafParserParser::IdContext *ctx) {
    depth++;

    printString(ctx->IDENTIFIER()->getText(), nullptr);

    depth--;
    return nullptr;
}