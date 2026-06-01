#pragma once

#include <ast/node.h>
#include <ast/type.h>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

enum SwaSymbolKind { Variable, Function, Enum };

struct SwaSymbol {
  std::unique_ptr<Type> type;
  SwaSymbolKind kind;
  ProtoExpr *proto = nullptr;
  Expr *initExpr = nullptr;
  SwaSymbol(std::unique_ptr<Type> type, SwaSymbolKind kind)
      : type(std::move(type)), kind(kind) {}
  SwaSymbol(std::unique_ptr<Type> type, SwaSymbolKind kind, Expr *initExpr)
      : type(std::move(type)), kind(kind), initExpr(initExpr) {}
  SwaSymbol(std::unique_ptr<Type> type, SwaSymbolKind kind, ProtoExpr *proto)
      : type(std::move(type)), kind(kind), proto(proto) {}
};

class SymbolTable {
  SymbolTable *Parent;
  std::map<std::string, llvm::Value *> Symbols;
  std::map<std::string, llvm::Type *> SymbolsTypes;
  std::map<std::string, std::unique_ptr<SwaSymbol>> SwaSymbols;
  std::vector<std::unique_ptr<SymbolTable>> Children;

public:
  explicit SymbolTable(SymbolTable *parent = nullptr) : Parent(parent) {}

  void define(const std::string &name, llvm::Value *val);
  void define(const std::string &name, llvm::Value *val, llvm::Type *typ);
  llvm::Value *lookup(const std::string &name);
  llvm::Type *lookupType(const std::string &name);

  void defineSwaSymbol(const std::string &name, std::unique_ptr<SwaSymbol> val);
  SwaSymbol *lookupSwaSymbol(const std::string &name);
  SwaSymbol *lookupSwaFunc(const std::string &name);
  SymbolTable *getParent() const;

  SymbolTable *CreateChild() {
    auto child = std::make_unique<SymbolTable>(this);
    SymbolTable *ptr = child.get();
    Children.push_back(std::move(child));

    return ptr;
  }
};
