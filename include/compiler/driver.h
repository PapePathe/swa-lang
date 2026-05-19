#pragma once

#include <ast/expr.h>
#include <ast/symboltable.h>
#include <ast/visitor.h>
#include <llvm/IR/DIBuilder.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <string>

struct SourceLocation {
  size_t line;
  size_t column;
};

struct DebugInfoContext {
  std::unique_ptr<llvm::DIBuilder> builder;
  llvm::DICompileUnit *compileUnit = nullptr;
  llvm::DIScope *currentScope = nullptr;

  void emitLocation(SourceLocation loc, llvm::IRBuilder<> &b) {
    if (!currentScope) {
      b.SetCurrentDebugLocation(llvm::DebugLoc());
      return;
    }
    llvm::DILocation *DILoc = llvm::DILocation::get(b.getContext(), loc.line,
                                                    loc.column, currentScope);
    b.SetCurrentDebugLocation(DILoc);
  }
};
class SwaCompilerDriver {
public:
  llvm::LLVMContext Context;
  llvm::IRBuilder<> Builder;
  std::unique_ptr<llvm::Module> Module;
  SymbolTable Symbols;
  DebugInfoContext DebugInfo;
  bool TestMode;

  explicit SwaCompilerDriver(std::string moduleName)
      : Builder(Context),
        Module(std::make_unique<llvm::Module>(moduleName, Context)),
        TestMode(false) {
    Symbols = SymbolTable();
  }
  SwaCompilerDriver(std::string moduleName, bool testMode)
      : Builder(Context),
        Module(std::make_unique<llvm::Module>(moduleName, Context)),
        TestMode(testMode) {
    Symbols = SymbolTable();
  }
};
