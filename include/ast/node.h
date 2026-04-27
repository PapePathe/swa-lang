#include "expr.h"
#include "symboltable.h"
#include "type.h"
#include <iostream>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using SwaContext = std::unique_ptr<llvm::LLVMContext>;
using SwaModule = std::unique_ptr<llvm::Module>;
using SwaBuilder = std::unique_ptr<llvm::IRBuilder<>>;

class NumberExpr : public Expr {
public:
  int Value;
  NumberExpr(int value) : Value(value) {}

  virtual llvm::Value *Codegen(SwaContext &c, SwaModule &m, SwaBuilder &b,
                               SymbolTable &s) override {
    return b->getInt32(Value);
  }
};

class StructDefExpr : public Expr {
public:
  std::string Name;
  std::vector<std::string> FieldNames;
  std::vector<std::unique_ptr<Type>> FieldTypes;
  StructDefExpr(std::string n, std::vector<std::string> fnames,
                std::vector<std::unique_ptr<Type>> ftypes)
      : Name(n), FieldTypes(std::move(ftypes)), FieldNames(fnames) {}

  virtual llvm::Value *Codegen(SwaContext &c, SwaModule &m, SwaBuilder &b,
                               SymbolTable &s) override {
    return 0;
  }
};

class IdExpr : public Expr {
public:
  std::string Name;
  IdExpr(std::string n) : Name(n) {}
};

class PrintExpr : public Expr {
public:
  std::string Name;
  PrintExpr(std::string n) : Name(n) {}
  virtual llvm::Value *Codegen(SwaContext &c, SwaModule &m, SwaBuilder &b,
                               SymbolTable &s) override {
    auto printfTy =
        llvm::FunctionType::get(b->getInt32Ty(), b->getPtrTy(), true);
    auto printfn = m->getOrInsertFunction("printf", printfTy);

    auto str = b->CreateGlobalString(Name);
    return b->CreateCall(printfn, {str});
  }
};

class BlockExpr : public Expr { // Added 'public'
public:
  std::vector<std::unique_ptr<Expr>> Exprs;
  BlockExpr(std::vector<std::unique_ptr<Expr>> exprs)
      : Exprs(std::move(exprs)) {}
  virtual llvm::Value *Codegen(SwaContext &c, SwaModule &m, SwaBuilder &b,
                               SymbolTable &s) override {
    for (const auto &expr : Exprs) {
      expr->Codegen(c, m, b, s);
    }
    return nullptr;
  }
};

class DivExpr : public Expr {
public:
  std::unique_ptr<Expr> Left;
  std::unique_ptr<Expr> Right;
  DivExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
      : Left(std::move(left)), Right(std::move(right)) {}
};

class EqExpr : public Expr {
public:
  std::unique_ptr<Expr> Left;
  std::unique_ptr<Expr> Right;
  EqExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
      : Left(std::move(left)), Right(std::move(right)) {}
};

class MulExpr : public Expr {
public:
  std::unique_ptr<Expr> Left;
  std::unique_ptr<Expr> Right;
  MulExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
      : Left(std::move(left)), Right(std::move(right)) {}
};

class SubExpr : public Expr {
public:
  std::unique_ptr<Expr> Left;
  std::unique_ptr<Expr> Right;
  SubExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
      : Left(std::move(left)), Right(std::move(right)) {}
};

class AddExpr : public Expr {
public:
  std::unique_ptr<Expr> Left;
  std::unique_ptr<Expr> Right;
  AddExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
      : Left(std::move(left)), Right(std::move(right)) {}
};

class GTExpr : public Expr {
public:
  std::unique_ptr<Expr> Left;
  std::unique_ptr<Expr> Right;
  GTExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
      : Left(std::move(left)), Right(std::move(right)) {}
};

class GTEExpr : public Expr {
public:
  std::unique_ptr<Expr> Left;
  std::unique_ptr<Expr> Right;
  GTEExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
      : Left(std::move(left)), Right(std::move(right)) {}
};

class LTExpr : public Expr {
public:
  std::unique_ptr<Expr> Left;
  std::unique_ptr<Expr> Right;
  LTExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
      : Left(std::move(left)), Right(std::move(right)) {}
};

class LTEExpr : public Expr {
public:
  std::unique_ptr<Expr> Left;
  std::unique_ptr<Expr> Right;
  LTEExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
      : Left(std::move(left)), Right(std::move(right)) {}
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

  virtual llvm::Function *Codegen(std::unique_ptr<llvm::LLVMContext> &c,
                                  std::unique_ptr<llvm::Module> &m,
                                  std::unique_ptr<llvm::IRBuilder<>> &b,
                                  SymbolTable &s) override {
    std::vector<llvm::Type *> Ints(Args.size(), b->getInt32Ty());

    llvm::FunctionType *FT =
        llvm::FunctionType::get(b->getInt32Ty(), Ints, false);

    llvm::Function *F = llvm::Function::Create(
        FT, llvm::Function::ExternalLinkage, Name, m.get());

    unsigned Idx = 0;
    for (auto &Arg : F->args()) {
      Arg.setName(Args[Idx++]);
    }

    return F;
  }
};

class FuncExpr : public Expr {
public:
  std::unique_ptr<ProtoExpr> Proto;
  std::unique_ptr<BlockExpr> Body;
  FuncExpr(std::unique_ptr<ProtoExpr> p, std::unique_ptr<BlockExpr> b)
      : Proto(std::move(p)), Body(std::move(b)) {}

  virtual llvm::Value *Codegen(SwaContext &c, SwaModule &m, SwaBuilder &b,
                               SymbolTable &s) override {
    llvm::Function *TheFunction = m->getFunction(Proto->getName());

    if (!TheFunction) {
      TheFunction = Proto->Codegen(c, m, b, s);
    }

    if (!TheFunction)
      return nullptr;

    // 2. Create a new basic block to start insertion into
    llvm::BasicBlock *BB = llvm::BasicBlock::Create(*c, "entry", TheFunction);
    b->SetInsertPoint(BB);

    // 3. Generate the body code
    if (Body) {
      Body->Codegen(c, m, b, s);

      // 4. Ensure every function has a return (simple default for now)
      // In a real compiler, you'd check if the body already returned.
      b->CreateRet(b->getInt32(0));
    }

    return 0;
  }
};

class MainExpr : public Expr { // Added 'public'
  std::unique_ptr<BlockExpr> Body;

public:
  MainExpr(std::unique_ptr<BlockExpr> b) : Body(std::move(b)) {}
  virtual llvm::Value *Codegen(SwaContext &c, SwaModule &m, SwaBuilder &b,
                               SymbolTable &s) override {
    auto proto = std::make_unique<ProtoExpr>("main", std::vector<std::string>(),
                                             std::vector<Type>());
    auto fn = std::make_unique<FuncExpr>(std::move(proto), std::move(Body));
    fn->Codegen(c, m, b, s);

    return 0;
  }
};

class DeclarationExpr : public Expr {
public:
  std::string Name;
  Type T;
  std::unique_ptr<Expr> Value;
  DeclarationExpr(std::string name, std::unique_ptr<Expr> value, Type typ)
      : Name(name), Value(std::move(value)), T(typ) {}

  virtual llvm::Value *Codegen(SwaContext &c, SwaModule &m, SwaBuilder &b,
                               SymbolTable &s) override {
    std::cerr << "processing decl stmt: " << Name << "\n";

    llvm::Value *val = Value->Codegen(c, m, b, s);
    if (!val)
      return nullptr;

    m->getOrInsertGlobal(Name, val->getType());
    llvm::GlobalVariable *glob = m->getNamedGlobal(Name);

    glob->setAlignment(llvm::MaybeAlign(4));
    glob->setConstant(false); // It's a variable, not a constant

    if (auto *constVal = llvm::dyn_cast<llvm::Constant>(val)) {
      glob->setInitializer(constVal);
    } else {
      // If the value isn't a constant (like a function call result),
      // you'd typically initialize to 0 and 'store' the value later.
      glob->setInitializer(llvm::Constant::getNullValue(val->getType()));
      b->CreateStore(val, glob);
    }

    return glob;
  }
};
