#include <ast/symboltable.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <string>

void SymbolTable::define(const std::string &name, llvm::Value *val) {
  Symbols[name] = val;
}

llvm::Value *SymbolTable::lookup(const std::string &name) {
  if (Symbols.count(name)) {
    return Symbols[name];
  }
  if (Parent) {
    return Parent->lookup(name);
  }
  return nullptr;
}

SymbolTable *SymbolTable::getParent() const { return Parent; }
