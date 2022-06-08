#include "printer.h"
#include "antlr4-runtime.h"
#include "error.h"
#include <string>

static std::string getErrorText(CompileErrors err,
                                const std::vector<std::string> &texts) {
    std::string errText;

    if (err == CompileErrors::UNRECOGNIZED_CHAR) {
        errText.append("unrecognized character '").append(texts[0]).append("'");
    } else if (err == CompileErrors::INT_TOO_LARGE) {
        errText.append("integer literal ")
            .append(texts[0])
            .append(" is too large");
    } else if (err == CompileErrors::ILLEGAL_NEWLINE_IN_STR) {
        errText.append("illegal newline in string constant \"")
            .append(texts[0]);
    } else if (err == CompileErrors::ILLEGAL_ESC_IN_STR) {
        errText.append("illegal escape character");
    } else if (err == CompileErrors::UNTERMINATED_STR) {
        errText.append("unterminated string constant \"").append(texts[0]);
    }

    if (err == CompileErrors::CONFLICT_DECLAR) {
        errText.append("declaration of '")
            .append(texts[0])
            .append("' here conflicts with earlier declaration at ")
            .append(texts[1]);
    } else if (err == CompileErrors::CLASS_NOT_FOUND) {
        errText.append("class '").append(texts[0]).append("' not found");
    } else if (err == CompileErrors::CYCLIC_INHERITANCE) {
        errText.append("illegal class inheritance (should be acyclic)");
    } else if (err == CompileErrors::VOID_IDENTIFIER) {
        errText.append("cannot declare identifier '")
            .append(texts[0])
            .append("' as void type");
    } else if (err == CompileErrors::OVERRIDE_VAR) {
        errText.append("overriding variable is not allowed for var '")
            .append(texts[0])
            .append("'");
    } else if (err == CompileErrors::OVERRIDE_METHOD) {
        errText.append("overriding method '")
            .append(texts[0])
            .append("' doesn't match the type signature in class '")
            .append(texts[1])
            .append("'");
    } else if (err == CompileErrors::VOID_ARRAY) {
        errText.append("array element type must be non-void known type");
    }

    if (err == CompileErrors::UNDECLARE_VAR) {
        errText.append("undeclared variable '").append(texts[0]).append("'");
    } else if (err == CompileErrors::BREAK_OUTSIDE_LOOP) {
        errText.append("'break' is only allowed inside a loop");
    } else if (err == CompileErrors::THIS_IN_STATIC) {
        errText.append("can not use this in static function");
    } else if (err == CompileErrors::INCOMPAT_BIN_OP) {
        errText.append("incompatible operands: ")
            .append(texts[0])
            .append(" ")
            .append(texts[1])
            .append(" ")
            .append(texts[2]);
    } else if (err == CompileErrors::INCOMPAT_UN_OP) {
        errText.append("incompatible operand: ")
            .append(texts[0])
            .append(" ")
            .append(texts[1]);
    } else if (err == CompileErrors::INCOMPAT_RETURN) {
        errText.append("incompatible return: ")
            .append(texts[0])
            .append(" given, ")
            .append(texts[1])
            .append(" expected");
    } else if (err == CompileErrors::NEW_ARRY_LEN_NOT_INT) {
        errText.append("new array length must be an integer");
    } else if (err == CompileErrors::NOT_CLASS) {
        errText.append(texts[0]).append(" is not a class type");
    } else if (err == CompileErrors::FIELD_NOT_FOUND) {
        errText.append("field '")
            .append(texts[0])
            .append("' not found in '")
            .append(texts[1])
            .append("'");
    } else if (err == CompileErrors::BAD_ARG_COUNT) {
        errText.append("function '")
            .append(texts[0])
            .append("' expects ")
            .append(texts[1])
            .append(" argument(s) but ")
            .append(texts[2])
            .append(" given");
    } else if (err == CompileErrors::INCOMPAT_ARG) {
        errText.append("incompatible argument ")
            .append(texts[0])
            .append(": ")
            .append(texts[1])
            .append(" given, ")
            .append(texts[2])
            .append(" expected");
    } else if (err == CompileErrors::NOT_A_METHOD) {
        errText.append("'")
            .append(texts[0])
            .append("' is not a method in class '")
            .append(texts[1])
            .append("'");
    } else if (err == CompileErrors::REF_NON_STATIC) {
        errText.append("can not reference a non-static field '")
            .append(texts[0])
            .append("' from static method '")
            .append(texts[1])
            .append("'");
    } else if (err == CompileErrors::CANNOT_ACCESS_FIELD) {
        errText.append("cannot access field '")
            .append(texts[0])
            .append("' from '")
            .append(texts[1])
            .append("'");
    } else if (err == CompileErrors::FIELD_NOT_ACCESS) {
        errText.append("field '")
            .append(texts[0])
            .append("' of '")
            .append(texts[1])
            .append("' not accessible here");
    } else if (err == CompileErrors::TEST_NOT_BOOL) {
        errText.append("test expression must have bool type");
    } else if (err == CompileErrors::MISSING_RETURN) {
        errText.append(
            "missing return statement: control reaches end of non-void block");
    } else if (err == CompileErrors::NO_LEGAL_MAIN) {
        errText.append("no legal Main class named 'Main' was found");
    } else if (err == CompileErrors::INDEX_SEL_NONARRAY) {
        errText.append("[] can only be applied to arrays");
    } else if (err == CompileErrors::BAD_ARRAY_INDEX) {
        errText.append("array subscript must be an integer");
    }
    return errText;
}

std::string reportErrorText(CompileErrors err,
                            const std::vector<std::string> &texts) {
    std::string errText;

    errText.append("*** Error: ");
    errText.append(getErrorText(err, texts));
    errText += "\n";
    
    std::cerr << errText;
    return errText;
}

std::string reportErrorText(const Pos &pos, CompileErrors err,
                            const std::vector<std::string> &texts) {
    std::string errText;

    errText.append("*** Error at (")
        .append(std::to_string(pos.linePos))
        .append(",")
        .append(std::to_string(pos.charPos + 1))
        .append("): ");

    errText.append(getErrorText(err, texts));

    errText += "\n";
    std::cerr << errText;
    return errText;
}