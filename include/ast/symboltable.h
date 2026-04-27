#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <map>
#include <string>

#ifndef SYMBOL_TABLE
#define SYMBOL_TABLE
class SymbolTable {
  SymbolTable *Parent; // Pointer to the enclosing scope
  std::map<std::string, llvm::Value *> Symbols;

public:
  SymbolTable(SymbolTable *parent = nullptr) : Parent(parent) {}

  void define(const std::string &name, llvm::Value *val);

  llvm::Value *lookup(const std::string &name);
  SymbolTable *getParent() const;
};
#endif // !SYMBOL_TABLE
