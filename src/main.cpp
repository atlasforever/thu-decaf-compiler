#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <unistd.h>

#include "DecafParserBaseListener.h"
#include "DecafParserParser.h"
#include "antlr4-runtime.h"
#include "codegen/CodeGen.h"
#include "parser/ASTPrinter.h"
#include "parser/CommonLexer.h"
#include "semantic/SymbolChecker.h"
#include "semantic/TypeChecker.h"
#include "utils/ASTAttrManager.h"
#include "utils/printer.h"

using namespace std;
using namespace antlr4;

typedef enum { NO_TASK, PA1_TASK, PA2_TASK, PA3_TASK } PATASK;

static int optHandle(int argc, char *const *argv, PATASK &task, string &file) {
    int opt;
    task = NO_TASK;
    map<string, PATASK> paMap = {
        {"PA1", PA1_TASK},
        {"PA2", PA2_TASK},
        {"PA3", PA3_TASK},
    };
    map<string, PATASK>::iterator iter;

    while ((opt = getopt(argc, argv, "d:t:")) != -1) {
        switch (opt) {
        case 'd':
            break;
        case 't':
            iter = paMap.find(optarg);
            task = (iter != paMap.end()) ? iter->second : task;
            break;
        default:
            break;
        }
    }
    if (optind == argc - 1) {
        file = argv[optind];
    }
    return 0;
}

int main(int argc, char *argv[]) {
    PATASK task;
    string file;

    optHandle(argc, argv, task, file);

    if (file.empty()) {
        cerr << "[error] source file requested!" << endl;
        return EXIT_FAILURE;
    }

    ifstream ifs(file);

    if (!ifs.is_open()) {
        cerr << "[error] fail to open " << file << " " << strerror(errno) << endl;
        return EXIT_FAILURE;
    }

    ANTLRInputStream input(ifs);
    CommonLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    tokens.fill();
    ifs.close();

    if (lexer.lexerFailed) {
        return EXIT_FAILURE;
    }

    DecafParserParser parser(&tokens);
    tree::ParseTree *tree = parser.topLevel();

    if (task == PA1_TASK) {
        ASTPrinter astPrinter(&parser);
        astPrinter.visit(tree);
        return EXIT_SUCCESS;
    }

    // Build Symbol Table
    SymbolChecker symChker(tree);
    std::shared_ptr<Scope> globalScope = symChker.buildTable();
    if (globalScope == nullptr) {
        return EXIT_FAILURE;
    }

    // Type-checking
    std::shared_ptr<ASTAttrManager> attrManager =
        std::make_shared<ASTAttrManager>();
    TypeChecker typeChker(tree, globalScope, attrManager);
    if (!typeChker.check()) {
        return EXIT_FAILURE;
    }
    if (task == PA2_TASK) {
        globalScope->print(0);
        return EXIT_SUCCESS;
    }

    // Code Generation
    CodeGenVisitor cgen(tree, globalScope, attrManager);
    cgen.codegen();

    return EXIT_SUCCESS;
}
