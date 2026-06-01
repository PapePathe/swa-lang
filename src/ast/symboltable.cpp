#include <ast/symboltable.h>
#include <ast/type.h>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

void SymbolTable::define(const std::string &name, llvm::Value *val) {
  Symbols[name] = val;
}

void SymbolTable::define(const std::string &name, llvm::Value *val,
                         llvm::Type *typ) {
  Symbols[name] = val;
  SymbolsTypes[name] = typ;
}

void SymbolTable::defineSwaSymbol(const std::string &name,
                                  std::unique_ptr<SwaSymbol> val) {
  // Function should be added to the top level scope
  if (val.get()->kind == SwaSymbolKind::Function) {
    if (Parent) {
      Parent->defineSwaSymbol(name, std::move(val));
      return;
    }

    SwaSymbols[name] = std::move(val);
    return;
  }

  SwaSymbols[name] = std::move(val);
}

SwaSymbol *SymbolTable::lookupSwaSymbol(const std::string &name) {
  if (SwaSymbols.count(name)) {
    return SwaSymbols[name].get();
  }
  if (Parent) {
    return Parent->lookupSwaSymbol(name);
  }

  throw std::runtime_error(name + " does not exist");
}

SwaSymbol *SymbolTable::lookupSwaFunc(const std::string &name) {
  if (SwaSymbols.count(name)) {
    auto s = SwaSymbols[name].get();

    if (s->kind != SwaSymbolKind::Function) {
      throw std::runtime_error(name + " exist but it's not a function");
    }

    return s;
  }
  if (Parent) {
    return Parent->lookupSwaFunc(name);
  }

  throw std::runtime_error("function (" + name + ") does not exist");
}
llvm::Value *SymbolTable::lookup(const std::string &name) {
  if (Symbols.count(name)) {
    return Symbols[name];
  }
  if (Parent) {
    return Parent->lookup(name);
  }

  // FIXME better raise an error than return nil
  return nullptr;
}

llvm::Type *SymbolTable::lookupType(const std::string &name) {
  if (SymbolsTypes.count(name)) {
    return SymbolsTypes[name];
  }
  if (Parent) {
    return Parent->lookupType(name);
  }

  throw std::runtime_error("type (" + name +
                           ") does not exist in symbol table");
}

SymbolTable *SymbolTable::getParent() const { return Parent; }

void SymbolTable::registerForCleanup(std::string &name, llvm::Value *ptr) {
  // trach which variable holds this resource
  // we need this in order to not free values
  // that are used in return statements
  resourcesToFree.push_back(ptr);
}

const std::vector<llvm::Value *> &SymbolTable::getResourcesToFree() const {
  return resourcesToFree;
}
