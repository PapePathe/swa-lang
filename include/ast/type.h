#ifndef INCLUDE_AST_TYPE_H_
#define INCLUDE_AST_TYPE_H_

#include <ast/visitor.h>
#include <lexer/lexer.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <memory>

class Type {
public:
  Span span;
  explicit Type(Span s) : span(s) {}
  virtual ~Type() {}
  virtual void Accept(ASTVisitor &v) = 0;
};

class TypeInt : public Type {
public:
  explicit TypeInt(Span s) : Type(s) {}
  void Accept(ASTVisitor &v) override { v.Visit(this); };
};

class TypeFloat : public Type {
public:
  explicit TypeFloat(Span s) : Type(s) {}
  void Accept(ASTVisitor &v) override { v.Visit(this); };
};

class TypeBool : public Type {
public:
  explicit TypeBool(Span s) : Type(s) {}
  void Accept(ASTVisitor &v) override { v.Visit(this); };
};

class TypeByte : public Type {

public:
  explicit TypeByte(Span s) : Type(s) {}
  void Accept(ASTVisitor &v) override { v.Visit(this); };
};

class TypeString : public Type {
public:
  explicit TypeString(Span s) : Type(s) {}
  void Accept(ASTVisitor &v) override { v.Visit(this); };
};

class TypeVoid : public Type {
public:
  explicit TypeVoid(Span s) : Type(s) {}
  void Accept(ASTVisitor &v) override { v.Visit(this); };
};

class TypeStruct : public Type {
public:
  std::string Name;
  TypeStruct(std::string name, Span s) : Type(s), Name(name) {}
  void Accept(ASTVisitor &v) override { v.Visit(this); };
};

class TypePointer : public Type {
public:
  std::unique_ptr<Type> T;
  TypePointer(std::unique_ptr<Type> t, Span s) : Type(s), T(std::move(t)) {}
  void Accept(ASTVisitor &v) override { v.Visit(this); };
};

class TypeArray : public Type {
public:
  std::unique_ptr<Type> T;
  TypeArray(std::unique_ptr<Type> t, Span s) : Type(s), T(std::move(t)) {}
  void Accept(ASTVisitor &v) override { v.Visit(this); };
};

class TypeSlice : public Type {
public:
  std::unique_ptr<Type> T;
  TypeSlice(std::unique_ptr<Type> t, Span s) : Type(s), T(std::move(t)) {}
  void Accept(ASTVisitor &v) override { v.Visit(this); };
};

#endif // INCLUDE_AST_TYPE_H_
