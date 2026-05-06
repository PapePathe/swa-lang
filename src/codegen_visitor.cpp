#include <ast/visitor.h>
#include <compiler/codegen.h>
#include <compiler/compiler.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>
#include <stdexcept>

// Uncomment to enable tracing
// #define SWA_CODEGEN_TRACE ;

void codegenvisittrace(std::string s) {
#ifdef SWA_CODEGEN_TRACE
  std::cout << "[TRACE] " << s << "\n";
#endif // SWA_CODEGEN_TRACE
}

void CodeGenVisitor::setLastFunc(llvm::Function *v) { lastFunc = v; }
void CodeGenVisitor::setLastValue(llvm::Value *v) { lastValue = v; }
llvm::Value *CodeGenVisitor::getLastValue() {
  auto old = lastValue;
  lastValue = nullptr;
  return old;
}
llvm::Function *CodeGenVisitor::getLastFunc() {
  auto old = lastFunc;
  lastFunc = nullptr;
  return old;
}
llvm::Value *CodeGenVisitor::evaluate(Expr *expr) {
  if (!expr)
    return nullptr;
  expr->Accept(*this);
  return getLastValue();
}

void CodeGenVisitor::Visit(AddExpr *expr) {
  auto l = evaluate(expr->Left.get());
  auto r = evaluate(expr->Right.get());
  auto res = driver->Builder.CreateAdd(l, r);

  setLastValue(res);
}

void CodeGenVisitor::Visit(BoolExpr *expr) {}
void CodeGenVisitor::Visit(BlockExpr *expr) {
  auto oldTable = driver->Symbols;
  auto localTable = SymbolTable(driver->Symbols);
  driver->Symbols = localTable;

  codegenvisittrace("start block expr");
  for (auto &e : expr->Exprs) {
    e->Accept(*this);
  }
  codegenvisittrace("finish block expr");

  driver->Symbols = oldTable;
}

void CodeGenVisitor::Visit(DeclarationExpr *expr) {
  auto val = evaluate(expr->Value.get());

  if (!val) {
    throw std::runtime_error("Value is nil");
  }

  if (driver->Symbols.getParent() != nullptr) {
    auto alloc = driver->Builder.CreateAlloca(val->getType(), nullptr,
                                              "alloc-" + expr->Name);
    driver->Symbols.define(expr->Name, alloc);
    driver->Builder.CreateStore(val, alloc);

    setLastValue(alloc);

    return;
  }

  driver->Module->getOrInsertGlobal(expr->Name, val->getType());
  llvm::GlobalVariable *glob = driver->Module->getNamedGlobal(expr->Name);

  glob->setAlignment(llvm::MaybeAlign(4));
  glob->setConstant(false); // It's a variable, not a constant

  if (auto *constVal = llvm::dyn_cast<llvm::Constant>(val)) {
    glob->setInitializer(constVal);
  } else {
    glob->setInitializer(llvm::Constant::getNullValue(val->getType()));
    driver->Builder.CreateStore(val, glob);
  }

  driver->Symbols.define(expr->Name, val);

  setLastValue(glob);
}

void CodeGenVisitor::Visit(DivExpr *expr) {
  auto l = evaluate(expr->Left.get());
  auto r = evaluate(expr->Right.get());

  auto res = driver->Builder.CreateSDiv(l, r);
  setLastValue(res);
}

void CodeGenVisitor::Visit(EqExpr *expr) {
  auto l = evaluate(expr->Left.get());
  auto r = evaluate(expr->Right.get());

  auto res = driver->Builder.CreateICmpEQ(l, r);
  setLastValue(res);
}
void CodeGenVisitor::Visit(IdExpr *expr) {
  auto v = driver->Symbols.lookup(expr->Name);
  if (!v) {
    throw std::runtime_error("Undefined variable: " + expr->Name + "\n");
  }

  lastValue = v;
}
void CodeGenVisitor::Visit(IfExpr *expr) {}
void CodeGenVisitor::Visit(GTExpr *expr) {}

void CodeGenVisitor::Visit(GTEExpr *expr) {}
void CodeGenVisitor::Visit(LTExpr *expr) {}
void CodeGenVisitor::Visit(LTEExpr *expr) {}
void CodeGenVisitor::Visit(FuncExpr *expr) {
  llvm::Function *TheFunction =
      driver->Module->getFunction(expr->Proto->getName());

  if (!TheFunction) {
    expr->Proto->Accept(*this);
    TheFunction = getLastFunc();
  }

  if (!TheFunction) {
    throw std::runtime_error("Func not defined");
  }

  llvm::BasicBlock *BB =
      llvm::BasicBlock::Create(driver->Context, "entry", TheFunction);
  driver->Builder.SetInsertPoint(BB);

  if (expr->Body) {
    expr->Body->Accept(*this);

    driver->Builder.CreateRet(driver->Builder.getInt32(0));
  }
}

void CodeGenVisitor::Visit(MainExpr *expr) {
  codegenvisittrace("start main");

  auto proto = std::make_unique<ProtoExpr>("main", std::vector<std::string>(),
                                           std::vector<Type>());
  auto fn = std::make_unique<FuncExpr>(std::move(proto), std::move(expr->Body));

  fn->Accept(*this);

  codegenvisittrace("finish main");
}

void CodeGenVisitor::Visit(MulExpr *expr) {
  auto l = evaluate(expr->Left.get());
  auto r = evaluate(expr->Right.get());

  auto res = driver->Builder.CreateMul(l, r);
  setLastValue(res);
}

void CodeGenVisitor::Visit(NumberExpr *expr) {
  lastValue = driver->Builder.getInt32(expr->Value);
}

void CodeGenVisitor::Visit(PrintExpr *expr) {
  auto printfTy = llvm::FunctionType::get(driver->Builder.getInt32Ty(),
                                          driver->Builder.getPtrTy(), true);
  auto printfn = driver->Module->getOrInsertFunction("printf", printfTy);

  std::vector<llvm::Value *> vals;

  for (auto &v : expr->Values) {
    auto r = evaluate(v.get());

    if (!r) {
      throw std::runtime_error("CodeGen failed\n");
    }

    if (auto alloca = llvm::dyn_cast<llvm::AllocaInst>(r)) {
      r = driver->Builder.CreateLoad(alloca->getAllocatedType(), alloca,
                                     "load_val");
    }

    vals.push_back(std::move(r));
  }

  auto val = driver->Builder.CreateCall(printfn, vals);
  setLastValue(val);
}

void CodeGenVisitor::Visit(ProtoExpr *expr) {
  codegenvisittrace("start proto expr " + expr->Name);

  std::vector<llvm::Type *> Ints(expr->Args.size(),
                                 driver->Builder.getInt32Ty());

  llvm::FunctionType *FT =
      llvm::FunctionType::get(driver->Builder.getInt32Ty(), Ints, false);

  llvm::Function *F = llvm::Function::Create(
      FT, llvm::Function::ExternalLinkage, expr->Name, driver->Module.get());

  unsigned Idx = 0;
  for (auto &Arg : F->args()) {
    Arg.setName(expr->Args[Idx++]);
  }

  setLastFunc(F);

  codegenvisittrace("finish proto expr");
}
void CodeGenVisitor::Visit(ReturnExpr *expr) { expr->Value->Accept(*this); }
void CodeGenVisitor::Visit(StrExpr *expr) {
  setLastValue(driver->Builder.CreateGlobalString(expr->Name));
}
void CodeGenVisitor::Visit(StructDefExpr *expr) {}
void CodeGenVisitor::Visit(SubExpr *expr) {
  auto l = evaluate(expr->Left.get());
  auto r = evaluate(expr->Right.get());

  auto res = driver->Builder.CreateSub(l, r);
  setLastValue(res);
}

void CodeGenVisitor::Visit(UnaryMinusExpr *expr) {
  auto val = evaluate(expr->Right.get());
  auto res = driver->Builder.CreateNeg(val);

  setLastValue(res);
}

void CodeGenVisitor::Visit(UnaryNotExpr *expr) {
  auto val = evaluate(expr->Right.get());
  auto res = driver->Builder.CreateNot(val);

  setLastValue(res);
}
