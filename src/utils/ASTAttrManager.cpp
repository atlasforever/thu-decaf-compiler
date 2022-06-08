#include "ASTAttrManager.h"

void ASTAttrManager::setHasRet(antlr4::tree::ParseTree *tree, bool b) {
    hasRet[tree] = b;
}

bool ASTAttrManager::getHasRet(antlr4::tree::ParseTree *tree) {
    return hasRet[tree];
}

void ASTAttrManager::setIsClassName(antlr4::tree::ParseTree *tree, bool b) {
    isClassName[tree] = b;
}

bool ASTAttrManager::getIsClassName(antlr4::tree::ParseTree *tree) {
    return isClassName[tree];
}

void ASTAttrManager::setExprType(DecafParserParser::ExprContext *ctx,
                                 const std::shared_ptr<Type> &t) {
    exprTypes[ctx] = t;
}

std::shared_ptr<Type>
ASTAttrManager::getExprType(DecafParserParser::ExprContext *ctx) {
    return exprTypes[ctx];
}

void ASTAttrManager::setSymbolLLVMValue(const std::shared_ptr<Symbol> &sym,
                                        llvm::Value *v) {
    symLLVMVals[sym] = v;
}

llvm::Value*
ASTAttrManager::getSymbolLLVMVal(const std::shared_ptr<Symbol> &sym) {
    return symLLVMVals[sym];
}
