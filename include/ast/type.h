#ifndef INCLUDE_AST_TYPE_H_
#define INCLUDE_AST_TYPE_H_

#include <ast/visitor.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <memory>

class Type {
public:
  virtual ~Type() {}
  virtual void Accept(ASTVisitor &v) = 0;
};

class TypeInt : public Type {
public:
  void Accept(ASTVisitor &v) override { v.Visit(this); };
};

class TypeFloat : public Type {
public:
  void Accept(ASTVisitor &v) override { v.Visit(this); };
};

class TypeBool : public Type {
public:
  void Accept(ASTVisitor &v) override { v.Visit(this); };
};

class TypeByte : public Type {
public:
  void Accept(ASTVisitor &v) override { v.Visit(this); };
};

class TypeString : public Type {
public:
  void Accept(ASTVisitor &v) override { v.Visit(this); };
};

class TypeVoid : public Type {
public:
  void Accept(ASTVisitor &v) override { v.Visit(this); };
};

class TypeStruct : public Type {
public:
  void Accept(ASTVisitor &v) override { v.Visit(this); };
};

class TypeArray : public Type {
public:
  std::unique_ptr<Type> T;
  explicit TypeArray(std::unique_ptr<Type> t) : T(std::move(t)) {}
  void Accept(ASTVisitor &v) override { v.Visit(this); };
};

class TypeSlice : public Type {
public:
  std::unique_ptr<Type> T;
  explicit TypeSlice(std::unique_ptr<Type> t) : T(std::move(t)) {}
  void Accept(ASTVisitor &v) override { v.Visit(this); };
};

#endif // INCLUDE_AST_TYPE_H_
