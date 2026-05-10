#include <ast/visitor.h>
#include <compiler/codegen.h>
#include <compiler/compiler.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <memory>
#include <stdexcept>

// Uncomment to enable tracing
// #define SWA_CODEGEN_TRACE ;

void codegenvisittrace(std::string s) {
#ifdef SWA_CODEGEN_TRACE
  std::cout << "[TRACE] " << s << "\n";
#endif // SWA_CODEGEN_TRACE
}

struct PrintTypeConfig {
  std::string specifier;
  bool isBoolean = false;
};

PrintTypeConfig getTypeConfig(llvm::Value *val) {
  llvm::Type *type = val->getType();

  if (type->isIntegerTy(1))
    return {"%s", true};
  if (type->isIntegerTy(32))
    return {"%d", false};
  if (type->isIntegerTy(64))
    return {"%ld", false};
  if (type->isFloatTy() || type->isDoubleTy())
    return {"%f", false};

  if (type->isPointerTy()) {
    // Safe string extraction via GlobalVariable inspection
    if (auto *globalVar = llvm::dyn_cast<llvm::GlobalVariable>(val)) {
      if (globalVar->hasInitializer() &&
          llvm::isa<llvm::ConstantDataArray>(globalVar->getInitializer())) {
        return {"%s", false};
      }
    }
    // Safe string extraction via GEP element arrays
    if (auto *gep = llvm::dyn_cast<llvm::GEPOperator>(val)) {
      if (gep->getResultElementType()->isIntegerTy(8)) {
        return {"%s", false};
      }
    }
    // Fallback safety fallback: Treat raw pointers as addresses to prevent
    // runtime segfaults
    return {"%p", false};
  }

  throw std::runtime_error("Unsupported type passed to print engine.");
}

std::pair<std::string, std::vector<llvm::Value *>>
CodeGenVisitor::buildFormatStringAndArgs(
    const std::vector<std::unique_ptr<Expr>> &expressions) {
  std::string formatStr = "";
  std::vector<llvm::Value *> printfArgs;
  auto size = expressions.size();
  printfArgs.reserve(size);

  for (size_t i = 0; i < size; ++i) {
    auto *val = evaluate(expressions[i].get());
    if (!val)
      throw std::runtime_error(
          "CodeGen failed inside print string evaluation pass.");

    if (auto *alloca = llvm::dyn_cast<llvm::AllocaInst>(val)) {
      val = driver->Builder.CreateLoad(alloca->getAllocatedType(), alloca,
                                       "load_val");
    }

    auto [specifier, isBoolean] = getTypeConfig(val);
    formatStr += specifier;

    if (isBoolean) {
      auto *trueStr = driver->Builder.CreateGlobalString("true", "str_true");
      auto *falseStr = driver->Builder.CreateGlobalString("false", "str_false");
      val = driver->Builder.CreateSelect(val, trueStr, falseStr, "bool_to_str");
    }

    if (i < size - 1) {
      formatStr += " ";
    }
    printfArgs.push_back(val);
  }

  return {formatStr, printfArgs};
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

void CodeGenVisitor::Visit(BoolExpr *expr) {
  auto btyp = llvm::Type::getInt1Ty(driver->Context);
  if (expr->Value) {
    setLastValue(llvm::ConstantInt::get(btyp, 1));

    return;
  }

  setLastValue(llvm::ConstantInt::get(btyp, 0));
}
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
                                           std::vector<std::unique_ptr<Type>>(),
                                           std::unique_ptr<Type>());
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

  auto [formatStr, printfArgs] = buildFormatStringAndArgs(expr->Values);

  llvm::Value *formatStrValue =
      driver->Builder.CreateGlobalString(formatStr, "print_fmt");
  printfArgs.insert(printfArgs.begin(), formatStrValue);

  auto val = driver->Builder.CreateCall(printfn, printfArgs);
  setLastValue(val);
}

void CodeGenVisitor::Visit(Formatted_Print_Expr *expr) {
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
