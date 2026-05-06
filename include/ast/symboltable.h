#ifndef SYMBOL_TABLE
#define SYMBOL_TABLE

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <map>
#include <string>

class SymbolTable {
  SymbolTable *Parent;
  std::map<std::string, llvm::Value *> Symbols;

public:
  SymbolTable() : Parent(nullptr) {}
  SymbolTable(SymbolTable *parent) : Parent(parent) {}

  void define(const std::string &name, llvm::Value *val);

  llvm::Value *lookup(const std::string &name);
  SymbolTable *getParent() const;
};
#endif // !SYMBOL_TABLE
