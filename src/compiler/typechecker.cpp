#include "ast/type.h"
#include "compiler/codegen.h"
#include "parser/exception.h"
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

#include <memory>
#include <stdexcept>
#include <utility>

void TypeCheckVisitorException::emitDiagnostic(
    const SourceManager &sm, std::string error_category) const {
  for (auto &err : errors) {
    err.emitDiagnostic(sm, error_category);
  }
}

void TypeCheckVisitor::checkErrors() {
  if (errors.size() == 0) {
    return;
  }

  throw TypeCheckVisitorException(std::move(errors));
}

void TypeCheckVisitor::Visit(AddExpr *expr) {
  expr->Left->Accept(*this);
  expr->Right->Accept(*this);

  if (!expr->Left->datatype->IsEqual(expr->Right->datatype.get())) {
    auto err = CodeGenException("Type mismatch in declaration", expr->span,
                                "Cannot initialize variable of type " +
                                    expr->Left->datatype->GetName() +
                                    " with a value of type " +
                                    expr->Right->datatype->GetName() +
                                    "Try changing the type to 'string' or "
                                    "parse the string as an integer.");
    errors.push_back(err);
  }

  if (expr->Left->datatype->GetKind() == TypeKind::String &&
      expr->Right->datatype->GetKind() == TypeKind::String) {
    std::string msg = "Operator '+' is not supported for types '" +
                      expr->Left->datatype->GetName() + "' and '" +
                      expr->Right->datatype->GetName() + "'.";

    auto err = CodeGenException("Invalid Operation", expr->span, msg,
                                "Check your types or use the appropriate "
                                "library function for this operation.");
    errors.push_back(err);
  }

  if (expr->Left->datatype->GetKind() == TypeKind::Bool &&
      expr->Right->datatype->GetKind() == TypeKind::Bool) {
    auto err =
        CodeGenException("Type Error", expr->span,
                         "The '+' operator cannot be applied to type 'Bool'.",
                         "Consider using logical operators like '&&' or '||' "
                         "if you intended to perform a logical operation.");
    errors.push_back(err);
  }

  expr->datatype = std::make_unique<TypeInt>(expr->span);
}

void TypeCheckVisitor::Visit(BoolExpr *expr) {
  expr->datatype = std::make_unique<TypeBool>(expr->span);
}

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

void TypeCheckVisitor::Visit(DeclarationExpr *expr) {
  if (expr->Value.get() == nullptr) {
    return;
  }

  expr->Value->Accept(*this);

  if (expr->Value->datatype == nullptr) {
    auto err = CodeGenException("Datatype failed to infer", expr->Value->span);
    errors.push_back(err);
  }
  if (!expr->T) {
    auto err = CodeGenException("Declaration has no type", expr->span);
    errors.push_back(err);
  }
  if (expr->T == nullptr) {
    auto err =
        CodeGenException("Variable declared without a type", expr->Value->span);
    errors.push_back(err);
  }

  if (!expr->T->IsEqual(expr->Value->datatype.get())) {
    auto err = CodeGenException("Incompatible type", expr->Value->span,
                                "Expected " + expr->T->GetName() + " but got " +
                                    expr->Value->datatype->GetName(),
                                "");
    errors.push_back(err);
  }

  driver->Symbols.define(expr->Name, expr->T.get());
}

void TypeCheckVisitor::Visit(DivExpr *expr) {
  expr->Left->Accept(*this);
  expr->Right->Accept(*this);

  //  if (!expr->Left->datatype->IsEqual(expr->Right->datatype.get())) {
  //    throw CodeGenException("Substraction error", expr->span,
  //                           "Datatype " + expr->Right->datatype->GetName() +
  //                               " and " + expr->Right->datatype->GetName(),
  //                           "cannot be used together");
  //  }

  // FIXME add a symbols table and check type there
  expr->datatype = std::make_unique<TypeInt>(expr->span);
}
void TypeCheckVisitor::Visit(EqExpr *expr) {}
void TypeCheckVisitor::Visit(IdExpr *expr) {
  auto typ = driver->Symbols.lookupSwaSymbol(expr->Name);

  if (typ == nullptr) {
    auto err = ParserException("variable " + expr->Name + " does not exist",
                               expr->span, "", "");
    errors.push_back(err);

    return;
  }

  expr->datatype = typ->Clone();
}
void TypeCheckVisitor::Visit(IfExpr *expr) {}
void TypeCheckVisitor::Visit(GTExpr *expr) {}
void TypeCheckVisitor::Visit(GTEExpr *expr) {}
void TypeCheckVisitor::Visit(LTExpr *expr) {}
void TypeCheckVisitor::Visit(LTEExpr *expr) {}
void TypeCheckVisitor::Visit(FuncExpr *expr) {
  if (expr->Proto->Name == "main") {
    auto retval = dynamic_cast<TypeInt *>(expr->Proto->Ret.get());
    if (retval == nullptr) {
      auto err = CodeGenException("return value of main should be TypeInt",
                                  expr->Proto->Ret->span);
      errors.push_back(err);
    }

    if (expr->Proto->ArgsTypes.size() > 3) {
      auto err = CodeGenException("main should have at most 3 arguments",
                                  expr->Proto->span);
      errors.push_back(err);
    }

    if (expr->Proto->ArgsTypes.size() >= 1) {
      auto arg0 = dynamic_cast<TypeInt *>(expr->Proto->ArgsTypes[0].get());

      if (arg0 == nullptr) {
        auto err =
            CodeGenException("first argument of main should be of TypeInt",
                             expr->Proto->ArgsTypes[0]->span);
        errors.push_back(err);
      }
    }

    if (expr->Proto->ArgsTypes.size() >= 2) {
      auto arg1 = dynamic_cast<TypeSlice *>(expr->Proto->ArgsTypes[1].get());

      if (arg1 == nullptr) {
        auto err = CodeGenException(
            "second argument of main should be a slice of strings",
            expr->Proto->ArgsTypes[1]->span);
        errors.push_back(err);
      }
    }

    if (expr->Proto->ArgsTypes.size() >= 3) {
      auto arg1 = dynamic_cast<TypeSlice *>(expr->Proto->ArgsTypes[2].get());

      if (arg1 == nullptr) {
        auto err = CodeGenException(
            "third argument of main should be a slice of strings",
            expr->Proto->ArgsTypes[2]->span);
        errors.push_back(err);
      }
    }
  }

  expr->Body->Accept(*this);
}
void TypeCheckVisitor::Visit(MainExpr *expr) {}
void TypeCheckVisitor::Visit(MulExpr *expr) {
  expr->Left->Accept(*this);
  expr->Right->Accept(*this);

  //  if (!expr->Left->datatype->IsEqual(expr->Right->datatype.get())) {
  //    throw CodeGenException("Multiplication error", expr->span,
  //                           "Datatype " + expr->Right->datatype->GetName() +
  //                               " and " + expr->Right->datatype->GetName(),
  //                           "cannot be multipled together");
  //  }

  // FIXME add a symbols table and check type there
  expr->datatype = std::make_unique<TypeInt>(expr->span);
}
void TypeCheckVisitor::Visit(PrintExpr *expr) {}
void TypeCheckVisitor::Visit(Formatted_Print_Expr *expr) {}
void TypeCheckVisitor::Visit(ProtoExpr *expr) {}
void TypeCheckVisitor::Visit(ReturnExpr *expr) {}
void TypeCheckVisitor::Visit(StrExpr *expr) {
  expr->datatype = std::make_unique<TypeString>(expr->span);
}
void TypeCheckVisitor::Visit(NumberExpr *expr) {
  expr->datatype = std::make_unique<TypeInt>(expr->span);
}
void TypeCheckVisitor::Visit(StructDefExpr *expr) {}
void TypeCheckVisitor::Visit(SubExpr *expr) {
  // if (!expr->Left->datatype->IsEqual(expr->Right->datatype.get())) {
  //   throw CodeGenException("Substraction error", expr->span,
  //                          "Datatype " + expr->Right->datatype->GetName() +
  //                              " and " + expr->Right->datatype->GetName(),
  //                          "cannot be used together");
  // }

  // FIXME add a symbols table and check type there
  expr->datatype = std::make_unique<TypeInt>(expr->span);
}
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
