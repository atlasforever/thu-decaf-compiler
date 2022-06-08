#ifndef _AST_ATTR_MANAGER_H_
#define _AST_ATTR_MANAGER_H_
#include "DecafParserParser.h"
#include "antlr4-runtime.h"
#include "semantic/Symbol.h"
#include "semantic/Type.h"
#include "llvm/IR/Value.h"
#include <memory>
#include <unordered_map>

// This acts like a collection of antlr4::ParseTreeProperty. It stores
// various attributes of AST nodes
class ASTAttrManager {
public:
    void setHasRet(antlr4::tree::ParseTree *tree, bool b);
    bool getHasRet(antlr4::tree::ParseTree *tree);

    void setIsClassName(antlr4::tree::ParseTree *tree, bool b);
    bool getIsClassName(antlr4::tree::ParseTree *tree);

    void setExprType(DecafParserParser::ExprContext *ctx,
                     const std::shared_ptr<Type> &t);
    std::shared_ptr<Type> getExprType(DecafParserParser::ExprContext *ctx);

    void setSymbolLLVMValue(const std::shared_ptr<Symbol> &sym, llvm::Value *v);
    llvm::Value* getSymbolLLVMVal(const std::shared_ptr<Symbol> &sym);

private:
    std::unordered_map<antlr4::tree::ParseTree *, bool> hasRet;
    std::unordered_map<antlr4::tree::ParseTree *, bool> isClassName;
    std::unordered_map<DecafParserParser::ExprContext *, std::shared_ptr<Type>>
        exprTypes;
    std::unordered_map<std::shared_ptr<Symbol>, llvm::Value *> symLLVMVals;
};

#endif