#include "ast/type.h"
#include <ast/symboltable.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <stdexcept>
#include <string>

void SymbolTable::define(const std::string &name, llvm::Value *val) {
  Symbols[name] = val;
}

void SymbolTable::define(const std::string &name, Type *val) {
  SwaSymbols[name] = val;
}

Type *SymbolTable::lookupSwaSymbol(const std::string &name) {
  if (SwaSymbols.count(name)) {
    return SwaSymbols[name];
  }
  if (Parent) {
    return Parent->lookupSwaSymbol(name);
  }

  throw std::runtime_error("variable (" + name + ") does not exist");
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
