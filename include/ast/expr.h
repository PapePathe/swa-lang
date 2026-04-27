#include "symboltable.h"
#include "type.h"
#include <iostream>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#ifndef AST_EXPR
#define AST_EXPR

using SwaContext = std::unique_ptr<llvm::LLVMContext>;
using SwaModule = std::unique_ptr<llvm::Module>;
using SwaBuilder = std::unique_ptr<llvm::IRBuilder<>>;

class Expr {
public:
  virtual ~Expr() {}
  virtual llvm::Value *Codegen(SwaContext &c, SwaModule &m, SwaBuilder &b,
                               SymbolTable &s) {
    return nullptr;
  }
};

#endif // !AST_EXPR
