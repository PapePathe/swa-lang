#include "compiler/codegen.h"
#include <ast/symboltable.h>
#include <ast/visitor.h>
#include <compiler/compiler.h>
#include <compiler/typechecker.h>

#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/Casting.h>
#include <stdexcept>

void TypeCheckVisitor::Visit(AddExpr *expr) {}

void TypeCheckVisitor::Visit(BoolExpr *expr) {}

void TypeCheckVisitor::Visit(BlockExpr *expr) {
  auto oldTable = driver->Symbols;
  auto localTable = SymbolTable(oldTable);
  driver->Symbols = localTable;

  for (auto &e : expr->Exprs) {
    if (!e) {
      continue;
    }

    e->Accept(*this);
  }

  driver->Symbols = oldTable;
}

void TypeCheckVisitor::Visit(DeclarationExpr *expr) {}
void TypeCheckVisitor::Visit(DivExpr *expr) {}
void TypeCheckVisitor::Visit(EqExpr *expr) {}
void TypeCheckVisitor::Visit(IdExpr *expr) {}
void TypeCheckVisitor::Visit(IfExpr *expr) {}
void TypeCheckVisitor::Visit(GTExpr *expr) {}
void TypeCheckVisitor::Visit(GTEExpr *expr) {}
void TypeCheckVisitor::Visit(LTExpr *expr) {}
void TypeCheckVisitor::Visit(LTEExpr *expr) {}
void TypeCheckVisitor::Visit(FuncExpr *expr) {
  if (expr->Proto->Name == "main") {
    auto retval = dynamic_cast<TypeInt *>(expr->Proto->Ret.get());
    if (retval == nullptr) {
      throw CodeGenException("return value of main should be TypeInt",
                             expr->Proto->Ret->span);
    }

    if (expr->Proto->ArgsTypes.size() > 3) {
      throw CodeGenException("main should have at most 3 arguments",
                             expr->Proto->span);
    }

    if (expr->Proto->ArgsTypes.size() >= 1) {
      auto arg0 = dynamic_cast<TypeInt *>(expr->Proto->ArgsTypes[0].get());

      if (arg0 == nullptr) {
        throw CodeGenException("first argument of main should be of TypeInt",
                               expr->Proto->ArgsTypes[0]->span);
      }
    }

    if (expr->Proto->ArgsTypes.size() >= 2) {
      auto arg1 = dynamic_cast<TypeSlice *>(expr->Proto->ArgsTypes[1].get());

      if (arg1 == nullptr) {
        throw CodeGenException(
            "second argument of main should be a slice of strings",
            expr->Proto->ArgsTypes[1]->span);
      }
    }

    if (expr->Proto->ArgsTypes.size() >= 3) {
      auto arg1 = dynamic_cast<TypeSlice *>(expr->Proto->ArgsTypes[2].get());

      if (arg1 == nullptr) {
        throw CodeGenException(
            "third argument of main should be a slice of strings",
            expr->Proto->ArgsTypes[2]->span);
      }
    }
  }
}
void TypeCheckVisitor::Visit(MainExpr *expr) {}
void TypeCheckVisitor::Visit(MulExpr *expr) {}
void TypeCheckVisitor::Visit(PrintExpr *expr) {}
void TypeCheckVisitor::Visit(Formatted_Print_Expr *expr) {}
void TypeCheckVisitor::Visit(ProtoExpr *expr) {}
void TypeCheckVisitor::Visit(ReturnExpr *expr) {}
void TypeCheckVisitor::Visit(StrExpr *expr) {}
void TypeCheckVisitor::Visit(NumberExpr *expr) {}
void TypeCheckVisitor::Visit(StructDefExpr *expr) {}
void TypeCheckVisitor::Visit(SubExpr *expr) {}
void TypeCheckVisitor::Visit(UnaryMinusExpr *expr) {}
void TypeCheckVisitor::Visit(UnaryNotExpr *expr) {}
void TypeCheckVisitor::Visit(CallExpr *expr) {}
void TypeCheckVisitor::Visit(Logical_Or_Expr *expr) {}
void TypeCheckVisitor::Visit(Logical_And_Expr *expr) {}
void TypeCheckVisitor::Visit(Test_Expr *expr) {}
void TypeCheckVisitor::Visit(Assert_Equal_Expr *expr) {}
void TypeCheckVisitor::Visit(Assert_Not_Equal_Expr *expr) {
  throw std::runtime_error("Not implemented");
}
void TypeCheckVisitor::Visit(Assert_True_Expr *expr) {
  throw std::runtime_error("Not implemented");
}
void TypeCheckVisitor::Visit(Assert_False_Expr *expr) {
  throw std::runtime_error("Not implemented");
}
void TypeCheckVisitor::Visit(Assert_Less_Than_Expr *expr) {
  throw std::runtime_error("Not implemented");
}
void TypeCheckVisitor::Visit(Assert_Less_Than_Equal_Expr *expr) {
  throw std::runtime_error("Not implemented");
}
void TypeCheckVisitor::Visit(Assert_Greater_Than_Expr *expr) {
  throw std::runtime_error("Not implemented");
}
void TypeCheckVisitor::Visit(Assert_Greater_Than_Equals_Expr *expr) {
  throw std::runtime_error("Not implemented");
}

// Types

void TypeCheckVisitor::Visit(TypeSlice *expr) {}
void TypeCheckVisitor::Visit(TypeArray *expr) {}
void TypeCheckVisitor::Visit(TypeInt *expr) {}
void TypeCheckVisitor::Visit(TypeFloat *expr) {}
void TypeCheckVisitor::Visit(TypeString *expr) {}
void TypeCheckVisitor::Visit(TypeVoid *expr) {}
void TypeCheckVisitor::Visit(TypeBool *expr) {}
void TypeCheckVisitor::Visit(TypeByte *expr) {}
void TypeCheckVisitor::Visit(TypeStruct *expr) {}
void TypeCheckVisitor::Visit(TypePointer *expr) {}
