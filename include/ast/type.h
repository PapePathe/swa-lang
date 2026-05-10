#include "symboltable.h"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <memory>

#ifndef INCLUDE_AST_TYPE_H
#define INCLUDE_AST_TYPE_H

using SwaContext = std::unique_ptr<llvm::LLVMContext>;
using SwaModule = std::unique_ptr<llvm::Module>;
using SwaBuilder = std::unique_ptr<llvm::IRBuilder<>>;

class Type {
public:
  virtual ~Type() {}
  virtual llvm::Type *Codegen(SwaContext &c, SymbolTable &s) { return nullptr; }
};

class TypeInt : public Type {};
class TypeFloat : public Type {};
class TypeBool : public Type {};
class TypeByte : public Type {};
class TypeString : public Type {};
class TypeVoid : public Type {};
class TypeStruct : public Type {};
class TypeArray : public Type {
public:
  std::unique_ptr<Type> T;
  explicit TypeArray(std::unique_ptr<Type> t) : T(std::move(t)) {}
};

class TypeSlice : public Type {
public:
  std::unique_ptr<Type> T;
  explicit TypeSlice(std::unique_ptr<Type> t) : T(std::move(t)) {}
};

#endif // INCLUDE_AST_TYPE_H_
