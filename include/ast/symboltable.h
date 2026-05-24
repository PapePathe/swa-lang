#pragma once

#include <ast/type.h>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include <map>
#include <string>

class SymbolTable {
  SymbolTable *Parent;
  std::map<std::string, llvm::Value *> Symbols;
  std::map<std::string, Type *> SwaSymbols;

public:
  SymbolTable() : Parent(nullptr) {}
  explicit SymbolTable(SymbolTable *parent) : Parent(parent) {}
  void define(const std::string &name, llvm::Value *val);
  void define(const std::string &name, Type *val);
  llvm::Value *lookup(const std::string &name);
  Type *lookupSwaSymbol(const std::string &name);
  SymbolTable *getParent() const;
};
