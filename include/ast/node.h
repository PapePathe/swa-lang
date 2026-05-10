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
  explicit NumberExpr(int value) : Value(value) {}

  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class StructDefExpr : public Expr {
public:
  std::string Name = "";
  std::vector<std::string> FieldNames = {};
  std::vector<std::unique_ptr<Type>> FieldTypes = {};
  StructDefExpr(std::string n, std::vector<std::string> fnames,
                std::vector<std::unique_ptr<Type>> ftypes)
      : Name(std::move(n)), FieldTypes(std::move(ftypes)),
        FieldNames(std::move(fnames)) {}

  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class IdExpr : public Expr {
public:
  std::string Name = "";
  explicit IdExpr(std::string n) : Name(std::move(n)) {}
  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class PrintExpr : public Expr {
public:
  std::vector<std::unique_ptr<Expr>> Values = {};
  explicit PrintExpr(std::vector<std::unique_ptr<Expr>> values)
      : Values(std::move(values)) {}
  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class Formatted_Print_Expr : public Expr {
public:
  std::vector<std::unique_ptr<Expr>> Values;
  explicit Formatted_Print_Expr(std::vector<std::unique_ptr<Expr>> values)
      : Values(std::move(values)) {}
  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class BlockExpr : public Expr {
public:
  std::vector<std::unique_ptr<Expr>> Exprs;
  explicit BlockExpr(std::vector<std::unique_ptr<Expr>> exprs)
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
  std::vector<std::unique_ptr<Type>> ArgsTypes;
  std::unique_ptr<Type> Ret;

  ProtoExpr(std::string name, std::vector<std::string> args,
            std::vector<std::unique_ptr<Type>> argsTypes,
            std::unique_ptr<Type> ret)
      : Name(std::move(name)), Args(std::move(args)),
        ArgsTypes(std::move(argsTypes)), Ret(std::move(ret)) {}

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
  std::unique_ptr<BlockExpr> Body = {};
  explicit MainExpr(std::unique_ptr<BlockExpr> b) : Body(std::move(b)) {}

  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class DeclarationExpr : public Expr {
public:
  std::string Name;
  std::unique_ptr<Type> T;
  std::unique_ptr<Expr> Value;
  DeclarationExpr(std::string name, std::unique_ptr<Expr> value,
                  std::unique_ptr<Type> typ)
      : Name(std::move(name)), Value(std::move(value)), T(std::move(typ)) {}

  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class StrExpr : public Expr {
public:
  std::string Name;
  explicit StrExpr(std::string n) : Name(std::move(n)) {}

  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class UnaryMinusExpr : public Expr {
public:
  std::unique_ptr<Expr> Right = {};
  explicit UnaryMinusExpr(std::unique_ptr<Expr> right)
      : Right(std::move(right)) {}

  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class UnaryNotExpr : public Expr {
public:
  std::unique_ptr<Expr> Right;
  explicit UnaryNotExpr(std::unique_ptr<Expr> right)
      : Right(std::move(right)) {}

  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class ReturnExpr : public Expr {
public:
  std::unique_ptr<Expr> Value;
  explicit ReturnExpr(std::unique_ptr<Expr> value) : Value(std::move(value)) {}

  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class BoolExpr : public Expr {
public:
  bool Value;
  explicit BoolExpr(bool value) : Value(value) {}

  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};
