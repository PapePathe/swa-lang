#include "expr.h"
#include "type.h"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/Casting.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

class NumberExpr : public Expr {
public:
  int Value;
  NumberExpr(int value) : Value(value) {}

  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class StructDefExpr : public Expr {
public:
  std::string Name;
  std::vector<std::string> FieldNames;
  std::vector<std::unique_ptr<Type>> FieldTypes;
  StructDefExpr(std::string n, std::vector<std::string> fnames,
                std::vector<std::unique_ptr<Type>> ftypes)
      : Name(std::move(n)), FieldTypes(std::move(ftypes)), FieldNames(fnames) {}

  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class IdExpr : public Expr {
public:
  std::string Name;
  IdExpr(std::string n) : Name(std::move(n)) {}
  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class PrintExpr : public Expr {
public:
  std::vector<std::unique_ptr<Expr>> Values;
  PrintExpr(std::vector<std::unique_ptr<Expr>> values)
      : Values(std::move(values)) {}
  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class Formatted_Print_Expr : public Expr {
public:
  std::vector<std::unique_ptr<Expr>> Values;
  Formatted_Print_Expr(std::vector<std::unique_ptr<Expr>> values)
      : Values(std::move(values)) {}
  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class BlockExpr : public Expr { // Added 'public'
public:
  std::vector<std::unique_ptr<Expr>> Exprs;
  BlockExpr(std::vector<std::unique_ptr<Expr>> exprs)
      : Exprs(std::move(exprs)) {}
  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class DivExpr : public Expr {
public:
  std::unique_ptr<Expr> Left;
  std::unique_ptr<Expr> Right;
  DivExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
      : Left(std::move(left)), Right(std::move(right)) {}
  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class EqExpr : public Expr {
public:
  std::unique_ptr<Expr> Left;
  std::unique_ptr<Expr> Right;
  EqExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
      : Left(std::move(left)), Right(std::move(right)) {}
  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class MulExpr : public Expr {
public:
  std::unique_ptr<Expr> Left;
  std::unique_ptr<Expr> Right;
  MulExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
      : Left(std::move(left)), Right(std::move(right)) {}
  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class SubExpr : public Expr {
public:
  std::unique_ptr<Expr> Left;
  std::unique_ptr<Expr> Right;
  SubExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
      : Left(std::move(left)), Right(std::move(right)) {}
  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class AddExpr : public Expr {
public:
  std::unique_ptr<Expr> Left;
  std::unique_ptr<Expr> Right;
  AddExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
      : Left(std::move(left)), Right(std::move(right)) {}
  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class GTExpr : public Expr {
public:
  std::unique_ptr<Expr> Left;
  std::unique_ptr<Expr> Right;
  GTExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
      : Left(std::move(left)), Right(std::move(right)) {}
  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class GTEExpr : public Expr {
public:
  std::unique_ptr<Expr> Left;
  std::unique_ptr<Expr> Right;
  GTEExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
      : Left(std::move(left)), Right(std::move(right)) {}
  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class LTExpr : public Expr {
public:
  std::unique_ptr<Expr> Left;
  std::unique_ptr<Expr> Right;
  LTExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
      : Left(std::move(left)), Right(std::move(right)) {}
  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class LTEExpr : public Expr {
public:
  std::unique_ptr<Expr> Left;
  std::unique_ptr<Expr> Right;
  LTEExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
      : Left(std::move(left)), Right(std::move(right)) {}
  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class IfExpr : public Expr {
public:
  std::unique_ptr<Expr> Cond;
  std::unique_ptr<BlockExpr> Success;
  std::unique_ptr<BlockExpr> Failure;
  IfExpr(

      std::unique_ptr<Expr> cond, std::unique_ptr<BlockExpr> success,
      std::unique_ptr<BlockExpr> failure)
      : Cond(std::move(cond)), Success(std::move(success)),
        Failure(std::move(failure)) {}
};

class ProtoExpr : public Expr {
public:
  std::string Name;
  std::vector<std::string> Args;
  std::vector<Type> ArgsTypes;
  Type Ret;

  ProtoExpr(const std::string &name, std::vector<std::string> args,
            std::vector<Type> argsTypes)
      : Name(name), Args(args), ArgsTypes(argsTypes) {}
  ProtoExpr(const std::string &name, std::vector<std::string> args,
            std::vector<Type> argsTypes, Type ret)
      : Name(name), Args(args), ArgsTypes(argsTypes), Ret(ret) {}

  const std::string &getName() const { return Name; }

  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class FuncExpr : public Expr {
public:
  std::unique_ptr<ProtoExpr> Proto;
  std::unique_ptr<BlockExpr> Body;
  FuncExpr(std::unique_ptr<ProtoExpr> p, std::unique_ptr<BlockExpr> b)
      : Proto(std::move(p)), Body(std::move(b)) {}

  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class MainExpr : public Expr {

public:
  std::unique_ptr<BlockExpr> Body;
  MainExpr(std::unique_ptr<BlockExpr> b) : Body(std::move(b)) {}

  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class DeclarationExpr : public Expr {
public:
  std::string Name;
  Type T;
  std::unique_ptr<Expr> Value;
  DeclarationExpr(std::string name, std::unique_ptr<Expr> value, Type typ)
      : Name(name), Value(std::move(value)), T(typ) {}

  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class StrExpr : public Expr {
public:
  std::string Name;
  StrExpr(std::string n) : Name(n) {}

  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class UnaryMinusExpr : public Expr {
public:
  std::unique_ptr<Expr> Right;
  UnaryMinusExpr(std::unique_ptr<Expr> right) : Right(std::move(right)) {}

  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class UnaryNotExpr : public Expr {
public:
  std::unique_ptr<Expr> Right;
  UnaryNotExpr(std::unique_ptr<Expr> right) : Right(std::move(right)) {}

  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class ReturnExpr : public Expr {
public:
  std::unique_ptr<Expr> Value;
  ReturnExpr(std::unique_ptr<Expr> value) : Value(std::move(value)) {}

  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class BoolExpr : public Expr {
public:
  bool Value;
  BoolExpr(bool value) : Value(value) {}

  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};
