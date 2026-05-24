#include <ast/symboltable.h>
#include <ast/visitor.h>
#include <compiler/codegen.h>
#include <compiler/compiler.h>
#include <cstddef>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/Casting.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

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
void CodeGenVisitor::setLastType(llvm::Type *t) { lastType = t; }
llvm::Value *CodeGenVisitor::getLastValue() {
  auto old = lastValue;
  lastValue = nullptr;
  return old;
}
// FIXME this should be named getlastType
llvm::Type *CodeGenVisitor::setLastType() {
  auto old = lastType;
  lastType = nullptr;
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

llvm::Type *CodeGenVisitor::evaluate(Type *expr) {
  if (!expr)
    return nullptr;
  expr->Accept(*this);
  return setLastType();
}

std::pair<llvm::Value *, llvm::Value *>
CodeGenVisitor::evaluate_bin_expr(Expr *left, Expr *right) {
  auto l = evaluate(left);
  auto r = evaluate(right);

  if (auto a = llvm::dyn_cast<llvm::AllocaInst>(l)) {
    l = driver->Builder.CreateLoad(a->getAllocatedType(), a);
  }

  if (auto a = llvm::dyn_cast<llvm::AllocaInst>(r)) {
    r = driver->Builder.CreateLoad(a->getAllocatedType(), a);
  }

  return {l, r};
}

void CodeGenVisitor::Visit(AddExpr *expr) {
  auto [l, r] = evaluate_bin_expr(expr->Left.get(), expr->Right.get());
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
  auto localTable = SymbolTable(oldTable);
  driver->Symbols = localTable;

  codegenvisittrace("start block expr");
  for (auto &e : expr->Exprs) {
    if (!e) {
      continue;
    }

    e->Accept(*this);
  }
  codegenvisittrace("finish block expr");

  driver->Symbols = oldTable;
}

void CodeGenVisitor::Visit(DeclarationExpr *expr) {
  auto val = evaluate(expr->Value.get());
  auto typ = evaluate(expr->T.get());

  if (!val) {
    if (typ && typ->isStructTy()) {
      val = llvm::ConstantAggregateZero::get(typ);
    } else if (typ->isPointerTy()) {
      val = llvm::Constant::getNullValue(typ);
    } else {
      val = llvm::Constant::getNullValue(typ);
    }
  }

  if (driver->Symbols.getParent() != nullptr) {
    auto alloctype = val->getType();
    if (typ && typ->isStructTy()) {
      alloctype = typ;
    }

    auto alloc =
        driver->Builder.CreateAlloca(alloctype, nullptr, "alloc-" + expr->Name);
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
  auto [l, r] = evaluate_bin_expr(expr->Left.get(), expr->Right.get());

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

  auto old = driver->Symbols;
  auto localTable = SymbolTable{old};
  driver->Symbols = localTable;

  unsigned Idx = 0;
  for (auto &Arg : TheFunction->args()) {
    std::string ArgName = expr->Proto->Args[Idx++];
    llvm::AllocaInst *Alloca =
        driver->Builder.CreateAlloca(Arg.getType(), nullptr, ArgName);

    driver->Builder.CreateStore(&Arg, Alloca);
    driver->Symbols.define(ArgName, Alloca);
  }

  if (expr->Body) {
    expr->Body->Accept(*this);
  }

  driver->Symbols = old;
}

std::vector<std::string> CodeGenVisitor::visitTestExpressions(
    std::vector<std::unique_ptr<Test_Expr>> tests) {
  std::vector<std::string> testNames;
  for (const auto &testNode : tests) {
    std::string mangled = "swa_test_" + testNode->Name;

    testNode->Accept(*this);

    std::replace(mangled.begin(), mangled.end(), ' ', '_');
    testNames.push_back(mangled);
  }

  return testNames;
}

// FIXME we should remove MainExpr
void CodeGenVisitor::Visit(MainExpr *expr) {
  //  codegenvisittrace("start main");
  //
  //  std::string funcName = "main";
  //  if (driver->TestMode) {
  //    funcName = "user_main";
  //  }
  //
  //  auto proto = std::make_unique<ProtoExpr>(funcName,
  //  std::vector<std::string>(),
  //                                           std::vector<std::unique_ptr<Type>>(),
  //                                           std::unique_ptr<Type>());
  //  auto fn = std::make_unique<FuncExpr>(std::move(proto),
  //  std::move(expr->Body));
  //
  //  fn->Accept(*this);
  //
  //  codegenvisittrace("finish main");
}

void CodeGenVisitor::Visit(MulExpr *expr) {
  auto [l, r] = evaluate_bin_expr(expr->Left.get(), expr->Right.get());

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

  std::vector<llvm::Type *> Ints;

  for (auto &typ : expr->ArgsTypes) {
    auto lltyp = evaluate(typ.get());
    Ints.push_back(lltyp);
  }

  auto ret = evaluate(expr->Ret.get());
  llvm::FunctionType *FT = llvm::FunctionType::get(ret, Ints, false);

  llvm::Function *F = llvm::Function::Create(
      FT, llvm::Function::ExternalLinkage, expr->Name, driver->Module.get());

  unsigned Idx = 0;
  for (auto &Arg : F->args()) {
    Arg.setName(expr->Args[Idx++]);
  }

  setLastFunc(F);

  codegenvisittrace("finish proto expr");
}
void CodeGenVisitor::Visit(ReturnExpr *expr) {
  auto res = evaluate(expr->Value.get());

  driver->Builder.CreateRet(res);
}
void CodeGenVisitor::Visit(StrExpr *expr) {
  setLastValue(driver->Builder.CreateGlobalString(expr->Name));
}
void CodeGenVisitor::Visit(StructDefExpr *expr) {
  llvm::StructType *structDef =
      llvm::StructType::getTypeByName(driver->Context, expr->Name);
  if (!structDef) {
    // Only create a fresh entry if the context doesn't track this name yet
    // FIXME struct should be defined by DeclarationVisitor
    structDef = llvm::StructType::create(driver->Context, expr->Name);
  }

  // Prevent redefining the layout if it has already been processed
  if (!structDef->isOpaque()) {
    return;
  }

  std::vector<llvm::Type *> types;

  for (size_t i = 0; i < expr->FieldNames.size(); i++) {

    auto type = evaluate(expr->FieldTypes.at(i).get());
    if (type->isStructTy()) {
      auto t = llvm::dyn_cast<llvm::StructType>(type);
      if (t->getStructName() == expr->Name) {
        throw CodeGenException(
            "Compile Error: Invalid recursive type. Struct '" + expr->Name +
                "' cannot directly contain itself without a pointer.",
            expr->span);
      }
    }
    types.push_back(type);
  }

  structDef->setBody(types);
}
void CodeGenVisitor::Visit(SubExpr *expr) {
  auto [l, r] = evaluate_bin_expr(expr->Left.get(), expr->Right.get());

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

void CodeGenVisitor::Visit(CallExpr *expr) {
  if (auto s = dynamic_cast<IdExpr *>(expr->Callee.get()); s != nullptr) {
    auto ft = driver->Module->getFunction(s->Name);

    if (ft == nullptr) {
      throw CodeGenException("Function named " + s->Name + " does not exist",
                             expr->span);
    }

    std::vector<llvm::Value *> args;

    for (auto &arg : expr->Args) {
      auto val = evaluate(arg.get());

      if (auto alloca = llvm::dyn_cast<llvm::AllocaInst>(val)) {
        val = driver->Builder.CreateLoad(alloca->getAllocatedType(), alloca,
                                         "load_val");
      }
      args.push_back(val);
    }

    auto c = driver->Builder.CreateCall(ft, args);
    setLastValue(c);

    return;
  }

  throw std::runtime_error("Callee not supported for CallExpr");
}

void CodeGenVisitor::Visit(Logical_Or_Expr *expr) {
  throw std::runtime_error("Not implemented Logical_Or_Expr");
}
void CodeGenVisitor::Visit(Logical_And_Expr *expr) {
  throw std::runtime_error("Not implemented Logical_And_Expr");
}

void CodeGenVisitor::Visit(Test_Expr *expr) {
  std::string mangledName = "swa_test_" + expr->Name;
  std::replace(mangledName.begin(), mangledName.end(), ' ', '_');

  codegenvisittrace("Visit Test_Expr " + mangledName);
  llvm::FunctionType *testType =
      llvm::FunctionType::get(driver->Builder.getVoidTy(), false);
  llvm::Function *testFunc =
      llvm::Function::Create(testType, llvm::Function::ExternalLinkage,
                             mangledName, driver->Module.get());

  auto *oldInsertPoint = driver->Builder.GetInsertBlock();
  llvm::BasicBlock *entry =
      llvm::BasicBlock::Create(driver->Context, "entry", testFunc);
  driver->Builder.SetInsertPoint(entry);

  expr->Body->Accept(*this);
  driver->Builder.CreateRetVoid();
  if (oldInsertPoint) {
    driver->Builder.SetInsertPoint(oldInsertPoint);
  }

  codegenvisittrace("Visit Test_Expr");
}

void CodeGenVisitor::Visit(Assert_Equal_Expr *expr) {
  auto l = evaluate(expr->Left.get());
  auto r = evaluate(expr->Right.get());

  auto is_eq = driver->Builder.CreateICmpEQ(l, r);
  llvm::Function *parent = driver->Builder.GetInsertBlock()->getParent();
  llvm::BasicBlock *failBB =
      llvm::BasicBlock::Create(driver->Context, "assert_fail", parent);
  llvm::BasicBlock *continueBB =
      llvm::BasicBlock::Create(driver->Context, "assert_cont", parent);

  driver->Builder.CreateCondBr(is_eq, continueBB, failBB);
  driver->Builder.SetInsertPoint(failBB);
  auto printfTy = llvm::FunctionType::get(driver->Builder.getInt32Ty(),
                                          driver->Builder.getPtrTy(), true);
  auto printfunc = driver->Module->getOrInsertFunction("printf", printfTy);

  llvm::Value *formatStr = driver->Builder.CreateGlobalString(
      "  -> \033[31m[FAIL]\033[0m Expected %d, but got %d\n");

  driver->Builder.CreateCall(printfunc, {formatStr, r, l});
  driver->Builder.CreateBr(continueBB);

  driver->Builder.SetInsertPoint(continueBB);
}

void CodeGenVisitor::Visit(Assert_Not_Equal_Expr *expr) {
  throw std::runtime_error("Not implemented");
}
void CodeGenVisitor::Visit(Assert_True_Expr *expr) {
  throw std::runtime_error("Not implemented");
}
void CodeGenVisitor::Visit(Assert_False_Expr *expr) {
  throw std::runtime_error("Not implemented");
}
void CodeGenVisitor::Visit(Assert_Less_Than_Expr *expr) {
  throw std::runtime_error("Not implemented");
}
void CodeGenVisitor::Visit(Assert_Less_Than_Equal_Expr *expr) {
  throw std::runtime_error("Not implemented");
}
void CodeGenVisitor::Visit(Assert_Greater_Than_Expr *expr) {
  throw std::runtime_error("Not implemented");
}
void CodeGenVisitor::Visit(Assert_Greater_Than_Equals_Expr *expr) {
  throw std::runtime_error("Not implemented");
}
void CodeGenVisitor::generateTestEntrypoint(
    const std::vector<std::string> &testNames) {
  llvm::FunctionType *mainType =
      llvm::FunctionType::get(driver->Builder.getInt32Ty(), false);
  llvm::Function *mainFunc = llvm::Function::Create(
      mainType, llvm::Function::ExternalLinkage, "main", driver->Module.get());

  llvm::BasicBlock *entry =
      llvm::BasicBlock::Create(driver->Context, "test-entry", mainFunc);
  driver->Builder.SetInsertPoint(entry);

  // Direct string logging for diagnostics
  auto printfTy = llvm::FunctionType::get(driver->Builder.getInt32Ty(),
                                          driver->Builder.getPtrTy(), true);
  auto printfunc = driver->Module->getOrInsertFunction("printf", printfTy);

  for (const auto &testMangledName : testNames) {
    llvm::Function *targetTest = driver->Module->getFunction(testMangledName);
    if (targetTest) {
      // UI Update: Informs terminal which unit block is active
      std::string runMsg = "[RUN] " + testMangledName + "\n";
      driver->Builder.CreateCall(printfunc,
                                 {driver->Builder.CreateGlobalString(runMsg)});

      driver->Builder.CreateCall(targetTest);

      std::string passMsg = "  -> \033[32m[PASS]\033[0m\n";
      driver->Builder.CreateCall(printfunc,
                                 {driver->Builder.CreateGlobalString(passMsg)});
    } else {
      driver->Builder.CreateCall(printfunc, {driver->Builder.CreateGlobalString(
                                                "Function " + testMangledName +
                                                " does not exist \n")});
    }
  }

  driver->Builder.CreateRet(driver->Builder.getInt32(0));
}

// Types

void CodeGenVisitor::Visit(TypeSlice *expr) {
  auto inner = evaluate(expr->T.get());

  auto t = llvm::PointerType::get(inner, 0);
  setLastType(t);
}
void CodeGenVisitor::Visit(TypeArray *expr) {
  auto inner = evaluate(expr->T.get());

  auto t = llvm::ArrayType::get(inner, 10);
  setLastType(t);
}
void CodeGenVisitor::Visit(TypeInt *expr) {
  auto t = llvm::Type::getInt32Ty(driver->Context);
  setLastType(t);
}
void CodeGenVisitor::Visit(TypeFloat *expr) {
  auto t = llvm::Type::getFloatTy(driver->Context);
  setLastType(t);
}
void CodeGenVisitor::Visit(TypeString *expr) {
  auto t = llvm::PointerType::get(llvm::Type::getInt8Ty(driver->Context), 0);
  setLastType(t);
}
void CodeGenVisitor::Visit(TypeVoid *expr) {
  auto t = llvm::Type::getVoidTy(driver->Context);
  setLastType(t);
}
void CodeGenVisitor::Visit(TypeBool *expr) {
  auto t = llvm::Type::getInt1Ty(driver->Context);
  setLastType(t);
}
void CodeGenVisitor::Visit(TypeByte *expr) {
  auto t = llvm::Type::getInt8Ty(driver->Context);
  setLastType(t);
}
void CodeGenVisitor::Visit(TypeStruct *expr) {
  llvm::StructType *structDef =
      llvm::StructType::getTypeByName(driver->Context, expr->Name);
  if (!structDef) {
    throw CodeGenException("Undefined struct " + expr->Name, expr->span, "",
                           "");
  }
  setLastType(structDef);
}

void CodeGenVisitor::Visit(TypePointer *expr) {
  setLastType(llvm::PointerType::getUnqual(driver->Context));
}
