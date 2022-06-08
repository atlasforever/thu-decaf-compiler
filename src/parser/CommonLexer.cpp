#include "CommonLexer.h"
#include "printer.h"

using namespace antlr4;

CommonLexer::CommonLexer(antlr4::CharStream *input) : DecafLexer(input) {}

std::unique_ptr<Token> CommonLexer::nextToken() {
    std::unique_ptr<Token> tok = DecafLexer::nextToken();
    ssize_t tokType = tok->getType();

    if (tokType == STRING_LIT) {
        return checkStringLit(std::move(tok));
    } else if (tokType == INTLIT) {
        tok = checkIntLit(std::move(tok));
    } else if (tokType == UNKNWON_TOKEN) {
        reportErrorText(getTokenPos(tok.get()),
                        CompileErrors::UNRECOGNIZED_CHAR, {tok->getText()});
        lexerFailed = true;
    }

    return tok;
}

std::unique_ptr<Token>
CommonLexer::checkStringLit(std::unique_ptr<Token> startTok) {
    // begin with STRING_LIT
    CommonToken *strTok = new CommonToken(startTok->getType());
    strTok->setLine(startTok->getLine());
    strTok->setCharPositionInLine(startTok->getCharPositionInLine());
    strTok->setChannel(startTok->getChannel());
    strTok->setStartIndex(startTok->getStartIndex());
    strTok->setStopIndex(startTok->getStopIndex());
    strTok->setTokenIndex(startTok->getTokenIndex());

    std::unique_ptr<Token> tok = DecafLexer::nextToken();
    std::string text = strTok->getText();
    while (true) {
        if (tok->getType() == ILL_NEWLINE) {
            text.append(tok->getText());
            reportErrorText(getTokenPos(tok.get()),
                            CompileErrors::ILLEGAL_NEWLINE_IN_STR,
                            {text});
            lexerFailed = true;

        } else if (tok->getType() == ILL_ESCAPE) {
            reportErrorText(getTokenPos(tok.get()),
                            CompileErrors::ILLEGAL_ESC_IN_STR,
                            {text});
            text.append(tok->getText());
            lexerFailed = true;

        } else if (tok->getType() == EOF) {
            reportErrorText(getTokenPos(startTok.get()),
                            CompileErrors::UNTERMINATED_STR,
                            {text});
            lexerFailed = true;
            break;

        } else if (tok->getType() == STRING_TERM) {
            break;

        } else if (tok->getType() == ALLOWED_CHARS) {
            text.append(tok->getText());
        }

        tok = DecafLexer::nextToken();
    }
    strTok->setText(text);
    std::unique_ptr<Token> res = std::unique_ptr<Token>(strTok);
    return res;
}

std::unique_ptr<Token> CommonLexer::checkIntLit(std::unique_ptr<Token> tok) {
    std::string text = tok->getText();

    if (isIntegerTooLarge(text)) {
        reportErrorText(getTokenPos(tok.get()), CompileErrors::INT_TOO_LARGE,
                        {tok->getText()});
        lexerFailed = true;
    }
    return tok;
}