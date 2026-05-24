#include "ast/node.h"
#include "compiler/codegen.h"
#include "compiler/declaration.h"
#include "parser/exception.h"
#include <ast/visitor.h>
#include <compiler/compiler.h>
#include <compiler/declaration.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/Casting.h>

void DeclarationVisitor::Visit(ProtoExpr *expr) {
  std::vector<llvm::Type *> Ints;

  for (auto &typ : expr->ArgsTypes) {
    auto lltyp = evaluate(typ.get());
    Ints.push_back(lltyp);
  }

  auto ret = evaluate(expr->Ret.get());
  llvm::FunctionType *functionType = llvm::FunctionType::get(ret, Ints, false);

  llvm::Function *F =
      llvm::Function::Create(functionType, llvm::Function::ExternalLinkage,
                             expr->Name, driver->Module.get());

  unsigned Idx = 0;
  for (auto &Arg : F->args()) {
    Arg.setName(expr->Args[Idx++]);
  }

  setLastFunc(F);
}
void DeclarationVisitor::Visit(MainExpr *expr) {
  //  std::string funcName = "main";
  //  if (driver->TestMode) {
  //    funcName = "user_main";
  //  }
  //  auto proto = std::make_unique<ProtoExpr>(funcName,
  //  std::vector<std::string>(),
  //                                           std::vector<std::unique_ptr<Type>>(),
  //                                           std::unique_ptr<Type>());
  //  proto->Accept(*this);
}
void DeclarationVisitor::Visit(FuncExpr *expr) { expr->Proto->Accept(*this); }
void DeclarationVisitor::Visit(StructDefExpr *expr) {
  llvm::StructType *structDef =
      llvm::StructType::getTypeByName(driver->Context, expr->Name);
  if (!structDef) {
    // Only create a fresh entry if the context doesn't track this name yet
    // FIXME struct should be defined by DeclarationVisitor
    structDef = llvm::StructType::create(driver->Context, expr->Name);
  }
}
void DeclarationVisitor::Visit(BlockExpr *expr) {
  for (auto &e : expr->Exprs) {
    if (!e) {
      continue;
    }

    e->Accept(*this);
  }
}

void DeclarationVisitor::setLastType(llvm::Type *t) { lastType = t; }
llvm::Type *DeclarationVisitor::getLastType() {
  auto old = lastType;
  lastType = nullptr;
  return old;
}

void DeclarationVisitor::setLastFunc(llvm::Function *v) { lastFunc = v; }

llvm::Type *DeclarationVisitor::evaluate(Type *expr) {
  if (!expr)
    return nullptr;
  expr->Accept(*this);
  return getLastType();
}
//
// Types

void DeclarationVisitor::Visit(TypeSlice *expr) {
  auto inner = evaluate(expr->T.get());

  auto t = llvm::PointerType::get(inner, 0);
  setLastType(t);
}

void DeclarationVisitor::Visit(TypeArray *expr) {
  auto inner = evaluate(expr->T.get());

  auto t = llvm::ArrayType::get(inner, 10);
  setLastType(t);
}

void DeclarationVisitor::Visit(TypeInt *expr) {
  auto t = llvm::Type::getInt32Ty(driver->Context);
  setLastType(t);
}

void DeclarationVisitor::Visit(TypeFloat *expr) {
  auto t = llvm::Type::getFloatTy(driver->Context);
  setLastType(t);
}
void DeclarationVisitor::Visit(TypeString *expr) {
  auto t = llvm::PointerType::get(llvm::Type::getInt8Ty(driver->Context), 0);
  setLastType(t);
}
void DeclarationVisitor::Visit(TypeVoid *expr) {
  auto t = llvm::Type::getVoidTy(driver->Context);
  setLastType(t);
}
void DeclarationVisitor::Visit(TypeBool *expr) {
  auto t = llvm::Type::getInt1Ty(driver->Context);
  setLastType(t);
}
void DeclarationVisitor::Visit(TypeByte *expr) {
  auto t = llvm::Type::getInt8Ty(driver->Context);
  setLastType(t);
}
void DeclarationVisitor::Visit(TypeStruct *expr) {
  llvm::StructType *structDef =
      llvm::StructType::getTypeByName(driver->Context, expr->Name);
  if (!structDef) {
    throw CodeGenException("Undefined struct " + expr->Name, expr->span);
  }

  setLastType(structDef);
}

void DeclarationVisitor::Visit(TypePointer *expr) {
  setLastType(llvm::PointerType::getUnqual(driver->Context));
}
