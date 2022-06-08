#include "SymbolChecker.h"
#include "ArrayType.h"
#include "BaseChecker.h"
#include "ClassScope.h"
#include "ClassSymbol.h"
#include "GlobalScope.h"
#include "MethodSymbol.h"
#include "Pos.h"
#include "VarSymbol.h"
#include "parser/antlr/DecafParserParser.h"
#include "utils/printer.h"
#include <cassert>

using namespace std;

SymbolChecker::SymbolChecker(antlr4::tree::ParseTree *ast) {
    this->ast = ast;
    globalScope = std::make_shared<GlobalScope>();
    cur = globalScope;
}

std::shared_ptr<Scope> SymbolChecker::buildTable() {
    visit(ast);
    if (symbolFailed) {
        return nullptr;
    } else {
        return globalScope;
    }
}

antlrcpp::Any
SymbolChecker::visitTopLevel(DecafParserParser::TopLevelContext *ctx) {
    // Check Definitions for Class
    phase = Phase::CHECK_CLASS;
    visitChildren(ctx); // add classes, no duplication

    // check base-class, remove cyclic inheritance
    phase = Phase::CHECK_BASE;
    visitChildren(ctx);
    if (symbolFailed) {
        return nullptr;
    }

    // Check Definitions for Methods
    phase = Phase::CHECK_MEMBER;
    visitChildren(ctx);

    // should be a 'Main' class contains main method 'static void main()'
    if (!checkMain()) {
        symbolFailed = true;
        reportErrorText(CompileErrors::NO_LEGAL_MAIN, {});
    }

    return nullptr;
}

antlrcpp::Any
SymbolChecker::visitClassDef(DecafParserParser::ClassDefContext *ctx) {
    if (phase == Phase::CHECK_CLASS) {
        return addClasses(ctx);
    } else if (phase == Phase::CHECK_BASE) {
        return checkBase(ctx);
    } else if (phase == Phase::CHECK_MEMBER) {
        Pos pos = getClassPos(ctx);
        auto p = cur->enterScope(pos);
        if (p) {
            cur = p;
            visitChildren(ctx);
            cur = cur->exitScope();
        }
    }
    return nullptr;
}

antlrcpp::Any
SymbolChecker::visitVarDef(DecafParserParser::VarDefContext *ctx) {
    if (phase == Phase::CHECK_MEMBER) {
        antlr4::Token *idTok = ctx->var()->id()->IDENTIFIER()->getSymbol();
        std::string id = idTok->getText();
        Pos pos = getVarPos(ctx->var());

        std::shared_ptr<Symbol> symbol = std::make_shared<VarSymbol>();
        symbol->pos = pos;
        symbol->name = id;
        symbol->type = getType(ctx->var()->type());
        if (!cur->declare(id, symbol)) {
            symbolFailed = true;
            return nullptr;
        }
    }
    return nullptr;
}

antlrcpp::Any
SymbolChecker::visitMethodDef(DecafParserParser::MethodDefContext *ctx) {
    if (phase == Phase::CHECK_MEMBER) {
        antlr4::Token *idTok = ctx->id()->IDENTIFIER()->getSymbol();
        std::string id = idTok->getText();
        bool isStatic = ctx->STATIC() != nullptr;
        Pos pos = getMethodPos(ctx);

        std::shared_ptr<Symbol> symbol = std::make_shared<MethodSymbol>();
        symbol->pos = pos;
        symbol->name = id;
        symbol->setStatic(isStatic);
        symbol->type = getMethodType(ctx);

        if (!cur->declare(id, symbol)) {
            symbolFailed = true;
        }

        cur->createScope(pos, id);
        cur = cur->enterScope(pos);

        cur->setSymbol(symbol);
        symbol->setScope(cur);

        visitVarList(ctx->varList()); // parameters
        visitBlock(ctx->block());     // method block
        cur = cur->exitScope();
    }
    return nullptr;
}

antlrcpp::Any
SymbolChecker::visitVarList(DecafParserParser::VarListContext *ctx) {
    if (phase == Phase::CHECK_MEMBER) {
        auto paras = ctx->paraVarDef();
        std::vector<std::shared_ptr<Type>> parasType;

        if (!cur->getSymbol()->isStatic()) {
            std::shared_ptr<Symbol> classSym = cur->getParent()->getSymbol();
            std::shared_ptr<Symbol> varSym = std::make_shared<VarSymbol>();

            varSym->pos = cur->pos;
            varSym->name = "this";
            varSym->type = classSym->type;
            if (!cur->declare(varSym->name, varSym)) {
                symbolFailed = true;
            }
        }

        for (std::size_t i = 0; i < paras.size(); i++) {
            DecafParserParser::VarContext *varCtx = paras[i]->var();
            std::shared_ptr<Type> ptype = getType(varCtx->type());
            antlr4::Token *idTok = varCtx->id()->IDENTIFIER()->getSymbol();
            Pos pos = getVarPos(varCtx);

            std::shared_ptr<Symbol> varSym = std::make_shared<VarSymbol>();
            varSym->pos = pos;
            varSym->name = idTok->getText();
            varSym->type = ptype;

            if (!cur->declare(varSym->name, varSym)) {
                symbolFailed = true;
            }
        }
    }
    return nullptr;
}

antlrcpp::Any
SymbolChecker::visitForStmt(DecafParserParser::ForStmtContext *ctx) {
    Pos pos = getForPos(ctx);

    cur = cur->createScope(pos, "");
    visitChildren(ctx);
    cur = cur->exitScope();
    return nullptr;
}

antlrcpp::Any SymbolChecker::visitBlock(DecafParserParser::BlockContext *ctx) {
    Pos pos = getBlockPos(ctx);

    cur = cur->createScope(pos, "");
    visitChildren(ctx);
    cur = cur->exitScope();
    return nullptr;
}

antlrcpp::Any
SymbolChecker::visitLocalVarDef(DecafParserParser::LocalVarDefContext *ctx) {
    if (phase == Phase::CHECK_MEMBER) {
        antlr4::Token *idTok = ctx->var()->id()->IDENTIFIER()->getSymbol();
        std::string id = idTok->getText();
        Pos pos = getVarPos(ctx->var());

        std::shared_ptr<Symbol> varSym = std::make_shared<VarSymbol>();
        varSym->pos = pos;
        varSym->name = id;
        varSym->type = getType(ctx->var()->type());
        if (!cur->declare(varSym->name, varSym)) {
            symbolFailed = true;
            return nullptr;
        }
    }
    return visitChildren(ctx);
}

antlrcpp::Any
SymbolChecker::addClasses(DecafParserParser::ClassDefContext *ctx) {
    Pos pos = getClassPos(ctx);

    std::shared_ptr<Symbol> symbol = std::make_shared<ClassSymbol>();
    symbol->pos = pos;
    symbol->name = ctx->id()->getText();
    symbol->type = std::make_shared<ClassType>(symbol->name);

    bool succ = cur->declare(symbol->name, symbol);
    if (!succ) {
        symbolFailed = true;
        return nullptr;
    }

    std::shared_ptr<Scope> newclass = cur->createScope(pos, symbol->name);

    newclass->setSymbol(symbol);
    symbol->setScope(newclass);

    // set baseclass
    if (ctx->extendClause()) {
        std::string baseName = ctx->extendClause()->id()->getText();
        BaseChecker::setBase(symbol->name, baseName);
    }
    return nullptr;
}

antlrcpp::Any
SymbolChecker::checkBase(DecafParserParser::ClassDefContext *ctx) {
    if (!ctx->extendClause()) {
        return nullptr;
    }
    Pos pos = getClassPos(ctx);
    std::string curName = ctx->id()->getText();
    std::shared_ptr<Symbol> curSymbol = cur->lookup(curName);

    // baseclass not defined
    std::string baseName = ctx->extendClause()->id()->getText();
    std::shared_ptr<Symbol> baseSymbol = cur->lookup(baseName);
    if (baseSymbol == nullptr) {
        vector<string> texts = {baseName};
        reportErrorText(pos, CompileErrors::CLASS_NOT_FOUND, texts);
        symbolFailed = true;
        return nullptr;
    }

    // detect cyclic inheritance
    std::shared_ptr<Symbol> ptr = baseSymbol;
    while (!ptr->name.empty()) {
        if (ptr->name == curName) {
            reportErrorText(pos, CompileErrors::CYCLIC_INHERITANCE, {});

            // cut off the relation for no duplicated errors
            BaseChecker::setBase(ptr->name, "");
            symbolFailed = true;
            return nullptr;
        }

        ptr = cur->lookup(BaseChecker::getBase(ptr->name));
        if (!ptr) {
            // base not defined. But leave it
            break;
        }
    }

    // let the base-class be parent scope
    // std::shared_ptr<Scope> baseScope = cur->enterScope(baseSymbol->pos);
    // std::shared_ptr<Scope> curScope = cur->enterScope(curSymbol->pos);
    // curScope->setParent(baseScope);

    return nullptr;
}

// get Built-in or Array type
std::shared_ptr<Type>
SymbolChecker::getType(DecafParserParser::TypeContext *ctx) {
    if (ctx->INT()) {
        return std::make_shared<BuiltInType>(Type::INTEGER_TYPE);
    } else if (ctx->BOOL()) {
        return std::make_shared<BuiltInType>(Type::BOOL_TYPE);
    } else if (ctx->STRING()) {
        return std::make_shared<BuiltInType>(Type::STRING_TYPE);
    } else if (ctx->VOID()) {
        return std::make_shared<BuiltInType>(Type::VOID_TYPE);
    } else if (ctx->classType()) {
        return std::make_shared<ClassType>(ctx->classType()->id()->getText());
    } else if (ctx->LBRACKET()) {
        if (ctx->type()->VOID()) {
            Pos pos = getTokenPos(ctx->type()->VOID()->getSymbol());
            reportErrorText(pos, CompileErrors::VOID_ARRAY, {});
            symbolFailed = true;
        }
        return std::make_shared<ArrayType>(getType(ctx->type()));
    }
    return nullptr;
}

std::shared_ptr<MethodType>
SymbolChecker::getMethodType(DecafParserParser::MethodDefContext *ctx) {
    std::shared_ptr<Type> retType = getType(ctx->type());
    auto paras = ctx->varList()->paraVarDef();
    std::vector<std::shared_ptr<Type>> parasType;

    // add each parameter to scope
    for (std::size_t i = 0; i < paras.size(); i++) {
        DecafParserParser::VarContext *varCtx = paras[i]->var();
        std::shared_ptr<Type> ptype = getType(varCtx->type());
        antlr4::Token *idTok = varCtx->id()->IDENTIFIER()->getSymbol();
        Pos pos = getMethodPos(ctx);

        // no VOID parameter
        if (ptype->getKind() == Type::VOID_TYPE) {
            reportErrorText(pos, CompileErrors::VOID_IDENTIFIER,
                            {idTok->getText()});
            symbolFailed = true;
            continue;
        }
        parasType.push_back(ptype);
    }

    return std::make_shared<MethodType>(retType, parasType);
}

bool SymbolChecker::checkMain() {
    std::shared_ptr<Symbol> classSym = globalScope->lookup("Main");
    if (!classSym) {
        return false;
    }

    std::shared_ptr<Symbol> methodSym = classSym->getScope()->lookup("main");
    if (!methodSym || methodSym->getKind() != Symbol::METHOD ||
        !methodSym->isStatic()) {
        return false;
    }

    std::shared_ptr<MethodType> mType =
        std::dynamic_pointer_cast<MethodType>(methodSym->type);

    if (mType->getRetType()->getKind() != Type::VOID_TYPE ||
        mType->getArgsType().size() != 0) {
        return false;
    }

    return true;
}