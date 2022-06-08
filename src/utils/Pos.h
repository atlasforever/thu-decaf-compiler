#ifndef _POS_H_
#define _POS_H_

#include <string>
#include "DecafParserParser.h"

class Pos
{
public:
    int linePos = 0;
    int charPos = 0;

    Pos();
    Pos(int l, int c);
    bool operator<(const Pos &r) const;
    bool operator==(const Pos &r) const;
    std::string toString() const;
};

class PosHash
{
public:
    std::size_t operator()(Pos const &p) const noexcept
    {
        std::size_t h1 = std::hash<int>{}(p.linePos);
        std::size_t h2 = std::hash<int>{}(p.charPos);
        return h1 ^ (h2 << 1);
    }
};

Pos getTokenPos(const antlr4::Token *tok);
Pos getClassPos(DecafParserParser::ClassDefContext *ctx);
Pos getMethodPos(DecafParserParser::MethodDefContext *ctx);
Pos getVarPos(DecafParserParser::VarContext *ctx);
Pos getBlockPos(DecafParserParser::BlockContext *ctx);
Pos getForPos(DecafParserParser::ForStmtContext *ctx);
Pos getIdPos(DecafParserParser::IdContext *ctx);
Pos getBreakPos(DecafParserParser::BreakStmtContext *ctx);
Pos getThisPos(DecafParserParser::ThisExprContext *ctx);
Pos getInstanceofPos(DecafParserParser::InstanceofExprContext *ctx);
#endif