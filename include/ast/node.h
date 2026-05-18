#ifndef SWA_AST_NODE_H
#define SWA_AST_NODE_H

#include "expr.h"
#include "lexer/lexer.h"
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
  NumberExpr(int value, Span s) : Value(value) { this->span = span; }

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
  StructDefExpr(std::string n, std::vector<std::string> fnames,
                std::vector<std::unique_ptr<Type>> ftypes, Span s)
      : Name(std::move(n)), FieldTypes(std::move(ftypes)),
        FieldNames(std::move(fnames)) {
    this->span = s;
  }

  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class IdExpr : public Expr {
public:
  std::string Name = "";
  explicit IdExpr(std::string n) : Name(std::move(n)) {}
  IdExpr(std::string n, Span s) : Name(std::move(n)) { this->span = s; }
  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class PrintExpr : public Expr {
public:
  std::vector<std::unique_ptr<Expr>> Values = {};
  explicit PrintExpr(std::vector<std::unique_ptr<Expr>> values)
      : Values(std::move(values)) {}
  PrintExpr(std::vector<std::unique_ptr<Expr>> values, Span s)
      : Values(std::move(values)) {
    this->span = s;
  }
  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class Formatted_Print_Expr : public Expr {
public:
  std::vector<std::unique_ptr<Expr>> Values;
  explicit Formatted_Print_Expr(std::vector<std::unique_ptr<Expr>> values)
      : Values(std::move(values)) {}
  Formatted_Print_Expr(std::vector<std::unique_ptr<Expr>> values, Span s)
      : Values(std::move(values)) {
    this->span = s;
  }

  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class BlockExpr : public Expr {
public:
  std::vector<std::unique_ptr<Expr>> Exprs;
  explicit BlockExpr(std::vector<std::unique_ptr<Expr>> exprs)
      : Exprs(std::move(exprs)) {}
  BlockExpr(std::vector<std::unique_ptr<Expr>> exprs, Span s)
      : Exprs(std::move(exprs)) {
    this->span = s;
  }

  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class DivExpr : public Expr {
public:
  std::unique_ptr<Expr> Left;
  std::unique_ptr<Expr> Right;
  DivExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
      : Left(std::move(left)), Right(std::move(right)) {}
  DivExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right, Span s)
      : Left(std::move(left)), Right(std::move(right)) {
    this->span = s;
  }
  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class EqExpr : public Expr {
public:
  std::unique_ptr<Expr> Left;
  std::unique_ptr<Expr> Right;
  EqExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
      : Left(std::move(left)), Right(std::move(right)) {}
  EqExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right, Span s)
      : Left(std::move(left)), Right(std::move(right)) {
    this->span = s;
  }

  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class MulExpr : public Expr {
public:
  std::unique_ptr<Expr> Left;
  std::unique_ptr<Expr> Right;
  MulExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
      : Left(std::move(left)), Right(std::move(right)) {}
  MulExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right, Span s)
      : Left(std::move(left)), Right(std::move(right)) {
    this->span = s;
  }

  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class SubExpr : public Expr {
public:
  std::unique_ptr<Expr> Left;
  std::unique_ptr<Expr> Right;
  SubExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
      : Left(std::move(left)), Right(std::move(right)) {}
  SubExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right, Span s)
      : Left(std::move(left)), Right(std::move(right)) {
    this->span = s;
  }

  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class AddExpr : public Expr {
public:
  std::unique_ptr<Expr> Left;
  std::unique_ptr<Expr> Right;
  AddExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
      : Left(std::move(left)), Right(std::move(right)) {}
  AddExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right, Span s)
      : Left(std::move(left)), Right(std::move(right)) {
    this->span = s;
  }

  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class GTExpr : public Expr {
public:
  std::unique_ptr<Expr> Left;
  std::unique_ptr<Expr> Right;
  GTExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
      : Left(std::move(left)), Right(std::move(right)) {}
  GTExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right, Span s)
      : Left(std::move(left)), Right(std::move(right)) {
    this->span = s;
  }

  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class GTEExpr : public Expr {
public:
  std::unique_ptr<Expr> Left;
  std::unique_ptr<Expr> Right;
  GTEExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
      : Left(std::move(left)), Right(std::move(right)) {}
  GTEExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right, Span s)
      : Left(std::move(left)), Right(std::move(right)) {
    this->span = s;
  }

  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class LTExpr : public Expr {
public:
  std::unique_ptr<Expr> Left;
  std::unique_ptr<Expr> Right;
  LTExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
      : Left(std::move(left)), Right(std::move(right)) {}
  LTExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right, Span s)
      : Left(std::move(left)), Right(std::move(right)) {
    this->span = s;
  }
  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class LTEExpr : public Expr {
public:
  std::unique_ptr<Expr> Left;
  std::unique_ptr<Expr> Right;
  LTEExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
      : Left(std::move(left)), Right(std::move(right)) {}
  LTEExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right, Span s)
      : Left(std::move(left)), Right(std::move(right)) {
    this->span = s;
  }
  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class IfExpr : public Expr {
public:
  std::unique_ptr<Expr> Cond;
  std::unique_ptr<BlockExpr> Success;
  std::unique_ptr<BlockExpr> Failure;
  IfExpr(std::unique_ptr<Expr> cond, std::unique_ptr<BlockExpr> success,
         std::unique_ptr<BlockExpr> failure)
      : Cond(std::move(cond)), Success(std::move(success)),
        Failure(std::move(failure)) {}
  IfExpr(std::unique_ptr<Expr> cond, std::unique_ptr<BlockExpr> success,
         std::unique_ptr<BlockExpr> failure, Span s)
      : Cond(std::move(cond)), Success(std::move(success)),
        Failure(std::move(failure)) {
    this->span = s;
  }
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
  ProtoExpr(std::string name, std::vector<std::string> args,
            std::vector<std::unique_ptr<Type>> argsTypes,
            std::unique_ptr<Type> ret, Span s)
      : Name(std::move(name)), Args(std::move(args)),
        ArgsTypes(std::move(argsTypes)), Ret(std::move(ret)) {
    this->span = s;
  }

  const std::string &getName() const { return Name; }

  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class FuncExpr : public Expr {
public:
  std::unique_ptr<ProtoExpr> Proto;
  std::unique_ptr<BlockExpr> Body;
  FuncExpr(std::unique_ptr<ProtoExpr> p, std::unique_ptr<BlockExpr> b)
      : Proto(std::move(p)), Body(std::move(b)) {}
  FuncExpr(std::unique_ptr<ProtoExpr> p, std::unique_ptr<BlockExpr> b, Span s)
      : Proto(std::move(p)), Body(std::move(b)) {
    this->span = s;
  }

  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class MainExpr : public Expr {
public:
  std::unique_ptr<BlockExpr> Body = {};
  explicit MainExpr(std::unique_ptr<BlockExpr> b) : Body(std::move(b)) {}
  MainExpr(std::unique_ptr<BlockExpr> b, Span s) : Body(std::move(b)) {
    this->span = s;
  }

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
  DeclarationExpr(std::string name, std::unique_ptr<Expr> value,
                  std::unique_ptr<Type> typ, Span s)
      : Name(std::move(name)), Value(std::move(value)), T(std::move(typ)) {
    this->span = s;
  }

  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class StrExpr : public Expr {
public:
  std::string Name;
  explicit StrExpr(std::string n) : Name(std::move(n)) {}
  StrExpr(std::string n, Span s) : Name(std::move(n)) { this->span = s; }

  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class UnaryMinusExpr : public Expr {
public:
  std::unique_ptr<Expr> Right = {};
  explicit UnaryMinusExpr(std::unique_ptr<Expr> right)
      : Right(std::move(right)) {}
  UnaryMinusExpr(std::unique_ptr<Expr> right, Span s)
      : Right(std::move(right)) {
    this->span = s;
  }

  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class UnaryNotExpr : public Expr {
public:
  std::unique_ptr<Expr> Right;
  explicit UnaryNotExpr(std::unique_ptr<Expr> right)
      : Right(std::move(right)) {}
  UnaryNotExpr(std::unique_ptr<Expr> right, Span s) : Right(std::move(right)) {
    this->span = s;
  }

  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class ReturnExpr : public Expr {
public:
  std::unique_ptr<Expr> Value;
  explicit ReturnExpr(std::unique_ptr<Expr> value) : Value(std::move(value)) {}
  ReturnExpr(std::unique_ptr<Expr> value, Span s) : Value(std::move(value)) {
    this->span = s;
  }

  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class BoolExpr : public Expr {
public:
  bool Value;
  explicit BoolExpr(bool value) : Value(value) {}
  BoolExpr(bool value, Span s) : Value(value) { this->span = s; }

  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class CallExpr : public Expr {
public:
  std::unique_ptr<Expr> Callee;
  std::vector<std::unique_ptr<Expr>> Args;

  CallExpr(std::unique_ptr<Expr> callee,
           std::vector<std::unique_ptr<Expr>> args)
      : Callee(std::move(callee)), Args(std::move(args)) {}
  CallExpr(std::unique_ptr<Expr> callee,
           std::vector<std::unique_ptr<Expr>> args, Span s)
      : Callee(std::move(callee)), Args(std::move(args)) {
    this->span = s;
  }

  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class Logical_And_Expr : public Expr {
public:
  std::unique_ptr<Expr> Left;
  std::unique_ptr<Expr> Right;
  Logical_And_Expr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
      : Left(std::move(left)), Right(std::move(right)) {}
  Logical_And_Expr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right,
                   Span s)
      : Left(std::move(left)), Right(std::move(right)) {
    this->span = s;
  }

  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class Logical_Or_Expr : public Expr {
public:
  std::unique_ptr<Expr> Left;
  std::unique_ptr<Expr> Right;
  Logical_Or_Expr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
      : Left(std::move(left)), Right(std::move(right)) {}
  Logical_Or_Expr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right,
                  Span s)
      : Left(std::move(left)), Right(std::move(right)) {
    this->span = s;
  }
  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class Test_Expr : public Expr {
public:
  std::string Name;
  std::unique_ptr<BlockExpr> Body;
  Test_Expr(std::string name, std::unique_ptr<BlockExpr> body)
      : Name(name), Body(std::move(body)) {}
  Test_Expr(std::string name, std::unique_ptr<BlockExpr> body, Span s)
      : Name(name), Body(std::move(body)) {
    this->span = s;
  }

  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class Assert_True_Expr : public Expr {
public:
  std::unique_ptr<Expr> Assertion;
  Assert_True_Expr(std::unique_ptr<Expr> assertion)
      : Assertion(std::move(assertion)) {}
  Assert_True_Expr(std::unique_ptr<Expr> assertion, Span s)
      : Assertion(std::move(assertion)) {
    this->span = s;
  }
  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class Assert_False_Expr : public Expr {
public:
  std::unique_ptr<Expr> Assertion;
  Assert_False_Expr(std::unique_ptr<Expr> assertion, Span s)
      : Assertion(std::move(assertion)) {
    this->span = s;
  }
  Assert_False_Expr(std::unique_ptr<Expr> assertion)
      : Assertion(std::move(assertion)) {}
  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class Assert_Equal_Expr : public Expr {
public:
  std::unique_ptr<Expr> Left;
  std::unique_ptr<Expr> Right;
  Assert_Equal_Expr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right,
                    Span s)
      : Left(std::move(left)), Right(std::move(right)) {
    this->span = s;
  }
  Assert_Equal_Expr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
      : Left(std::move(left)), Right(std::move(right)) {}
  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class Assert_Not_Equal_Expr : public Expr {
public:
  std::unique_ptr<Expr> Left;
  std::unique_ptr<Expr> Right;
  Assert_Not_Equal_Expr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right,
                        Span s)
      : Left(std::move(left)), Right(std::move(right)) {
    this->span = s;
  }
  Assert_Not_Equal_Expr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
      : Left(std::move(left)), Right(std::move(right)) {}
  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class Assert_Less_Than_Expr : public Expr {
public:
  std::unique_ptr<Expr> Left;
  std::unique_ptr<Expr> Right;
  Assert_Less_Than_Expr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
      : Left(std::move(left)), Right(std::move(right)) {}
  Assert_Less_Than_Expr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right,
                        Span s)
      : Left(std::move(left)), Right(std::move(right)) {
    this->span = s;
  }
  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class Assert_Less_Than_Equal_Expr : public Expr {
public:
  std::unique_ptr<Expr> Left;
  std::unique_ptr<Expr> Right;
  Assert_Less_Than_Equal_Expr(std::unique_ptr<Expr> left,
                              std::unique_ptr<Expr> right)
      : Left(std::move(left)), Right(std::move(right)) {}
  Assert_Less_Than_Equal_Expr(std::unique_ptr<Expr> left,
                              std::unique_ptr<Expr> right, Span s)
      : Left(std::move(left)), Right(std::move(right)) {
    this->span = s;
  }
  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class Assert_Greater_Than_Expr : public Expr {
public:
  std::unique_ptr<Expr> Left;
  std::unique_ptr<Expr> Right;
  Assert_Greater_Than_Expr(std::unique_ptr<Expr> left,
                           std::unique_ptr<Expr> right)
      : Left(std::move(left)), Right(std::move(right)) {}
  Assert_Greater_Than_Expr(std::unique_ptr<Expr> left,
                           std::unique_ptr<Expr> right, Span s)
      : Left(std::move(left)), Right(std::move(right)) {
    this->span = s;
  }
  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};

class Assert_Greater_Than_Equals_Expr : public Expr {
public:
  std::unique_ptr<Expr> Left;
  std::unique_ptr<Expr> Right;
  Assert_Greater_Than_Equals_Expr(std::unique_ptr<Expr> left,
                                  std::unique_ptr<Expr> right)
      : Left(std::move(left)), Right(std::move(right)) {}
  Assert_Greater_Than_Equals_Expr(std::unique_ptr<Expr> left,
                                  std::unique_ptr<Expr> right, Span s)
      : Left(std::move(left)), Right(std::move(right)) {
    this->span = s;
  }

  void Accept(ASTVisitor &visitor) override { visitor.Visit(this); }
};
#endif // !SWA_AST_NODE_H
