#ifndef _COMMON_LEXER_H_
#define _COMMON_LEXER_H_

#include <memory>
#include <string>

#include "antlr4-runtime.h"
#include "DecafLexer.h"

class CommonLexer: public DecafLexer
{
public:
    bool lexerFailed = false;

    CommonLexer(antlr4::CharStream *input);
    virtual std::unique_ptr<antlr4::Token> nextToken() override;

private:
    std::unique_ptr<antlr4::Token> checkStringLit(std::unique_ptr<antlr4::Token> startTok);
    std::unique_ptr<antlr4::Token> checkIntLit(std::unique_ptr<antlr4::Token> tok);
};
#endif