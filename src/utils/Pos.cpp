#include "Pos.h"

Pos::Pos(){};

Pos::Pos(int l, int c) : linePos(l), charPos(c){};

bool Pos::operator<(const Pos &r) const {
    return linePos < r.linePos || (linePos == r.linePos && charPos < r.charPos);
}

bool Pos::operator==(const Pos &r) const {
    return linePos == r.linePos && charPos == r.charPos;
}

std::string Pos::toString() const {
    std::string s;
    s.append("(")
        .append(std::to_string(linePos))
        .append(",")
        .append(std::to_string(charPos + 1))
        .append(")");
    return s;
}

Pos getTokenPos(const antlr4::Token *tok) {
    return Pos(tok->getLine(), tok->getCharPositionInLine());
}

Pos getClassPos(DecafParserParser::ClassDefContext *ctx) {
    return getTokenPos(ctx->CLASS()->getSymbol());
}

Pos getMethodPos(DecafParserParser::MethodDefContext *ctx) {
    return getTokenPos(ctx->id()->IDENTIFIER()->getSymbol());
}

Pos getVarPos(DecafParserParser::VarContext *ctx) {
    return getTokenPos(ctx->id()->IDENTIFIER()->getSymbol());
}

Pos getBlockPos(DecafParserParser::BlockContext *ctx) {
    return getTokenPos(ctx->LBRACE()->getSymbol());
}

Pos getForPos(DecafParserParser::ForStmtContext *ctx) {
    return getTokenPos(ctx->FOR()->getSymbol());
}

Pos getIdPos(DecafParserParser::IdContext *ctx) {
    return getTokenPos(ctx->IDENTIFIER()->getSymbol());
}

Pos getBreakPos(DecafParserParser::BreakStmtContext *ctx) {
    return getTokenPos(ctx->BREAK()->getSymbol());
}

Pos getThisPos(DecafParserParser::ThisExprContext *ctx) {
    return getTokenPos(ctx->THIS()->getSymbol());
}

Pos getInstanceofPos(DecafParserParser::InstanceofExprContext *ctx) {
    return getTokenPos(ctx->INSTANCEOF()->getSymbol());
}