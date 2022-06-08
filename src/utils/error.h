#ifndef _DECAF_ERROR_H_
#define _DECAF_ERROR_H_

#include <string>
#include <exception>

enum class CompileErrors {
    UNRECOGNIZED_CHAR,
    INT_TOO_LARGE,
    ILLEGAL_NEWLINE_IN_STR,
    UNTERMINATED_STR,
    SYNTAX_ERROR,
    ILLEGAL_ESC_IN_STR,

    // Semantic errors
    CONFLICT_DECLAR,
    CLASS_NOT_FOUND,
    CYCLIC_INHERITANCE,
    VOID_IDENTIFIER,
    VOID_ARRAY,
    OVERRIDE_VAR,
    OVERRIDE_METHOD,

    // Type errors
    UNDECLARE_VAR,
    BREAK_OUTSIDE_LOOP,
    THIS_IN_STATIC,
    INCOMPAT_BIN_OP,
    INCOMPAT_UN_OP,
    INCOMPAT_RETURN,
    NEW_ARRY_LEN_NOT_INT,
    NOT_CLASS,
    FIELD_NOT_FOUND,
    BAD_ARG_COUNT,
    INCOMPAT_ARG,
    NOT_A_METHOD,
    REF_NON_STATIC,
    CANNOT_ACCESS_FIELD,
    FIELD_NOT_ACCESS,
    TEST_NOT_BOOL,
    MISSING_RETURN,
    NO_LEGAL_MAIN,
    INDEX_SEL_NONARRAY,
    BAD_ARRAY_INDEX,
};

class BaseDecafParseException : public std::exception
{
public:
    int line;
    int linePos;
    std::string text;
};

class UnrecognizedChar : public BaseDecafParseException {};

class IntTooLarge : public BaseDecafParseException {};

bool isIntegerTooLarge(const std::string &input);

#endif