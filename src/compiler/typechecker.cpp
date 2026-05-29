#include <ast/symboltable.h>
#include <ast/type.h>
#include <ast/visitor.h>
#include <cmath>
#include <compiler/codegen.h>
#include <compiler/compiler.h>
#include <compiler/typechecker.h>
#include <exception>
#include <parser/exception.h>

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
  log("Visit AddExpr");
  ValidateArithmetic(expr->Left, expr->Right, expr->span, "+");

  if (expr->Left->datatype->GetKind() == TypeKind::DirtyType ||
      expr->Right->datatype->GetKind() == TypeKind::DirtyType) {
    log("Visit AddExpr left or right is dirty");
    expr->datatype = std::make_unique<TypeDirty>(expr->span);
    return;
  }

  expr->datatype = expr->Left->datatype->Clone();
}

void TypeCheckVisitor::Visit(BoolExpr *expr) {
  expr->datatype = std::make_unique<TypeBool>(expr->span);
}

void TypeCheckVisitor::Visit(BlockExpr *expr) {
  auto oldTable = driver->currentSymbols;
  auto localTable = oldTable->CreateChild();
  driver->currentSymbols = localTable;

  for (auto &e : expr->Exprs) {
    if (!e) {
      continue;
    }

    e->Accept(*this);
  }

  driver->currentSymbols = oldTable;
}

void TypeCheckVisitor::Visit(DeclarationExpr *expr) {
  log("Visit DeclarationExpr");
  auto sym = std::make_unique<SwaSymbol>(std::move(expr->T.get()->Clone()),
                                         SwaSymbolKind::Variable);
  log("Visit DeclarationExpr lookupSwaSymbol " + expr->Name);
  driver->currentSymbols->defineSwaSymbol(expr->Name, std::move(sym));

  if (expr->Value.get() == nullptr) {
    log("Visit DeclarationExpr value is nil");
    return;
  }

  expr->Value->Accept(*this);

  if (expr->Value->datatype->GetKind() == TypeKind::DirtyType) {
    log("Visit DeclarationExpr: Value is DirtyType. Aborting further checks.");
    return;
  }

  if (!expr->T->IsEqual(expr->Value->datatype.get())) {
    log("Visit DeclarationExpr types are not equal");
    log("Visit DeclarationExpr Value type = " +
        expr->Value->datatype->GetName());
    log("Visit DeclarationExpr Var   Type = " + expr->T->GetName());
    auto err = CodeGenException("Incompatible type", expr->Value->span,
                                "Expected " + expr->T->GetName() + " but got " +
                                    expr->Value->datatype->GetName(),
                                "");
    errors.push_back(err);
  }
}

void TypeCheckVisitor::Visit(DivExpr *expr) {
  ValidateArithmetic(expr->Left, expr->Right, expr->span, "/");

  if (expr->Left->datatype->GetKind() == TypeKind::DirtyType ||
      expr->Right->datatype->GetKind() == TypeKind::DirtyType) {
    log("Visit DivExpr left or right is dirty");
    expr->datatype = std::make_unique<TypeDirty>(expr->span);
    return;
  }

  expr->datatype = expr->Left->datatype->Clone();
}
void TypeCheckVisitor::Visit(EqExpr *expr) {}
void TypeCheckVisitor::Visit(IdExpr *expr) {
  try {
    auto sym = driver->currentSymbols->lookupSwaSymbol(expr->Name);
    setLastSymbol(sym);
    expr->datatype = sym->type.get()->Clone();
  } catch (std::runtime_error e) {
    auto err = ParserException(e.what(), expr->span, "", "");
    errors.push_back(err);
    expr->datatype = std::make_unique<TypeDirty>(expr->span);
  }
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

  driver->currentSymbols->defineSwaSymbol(
      expr->Proto->Name,
      std::make_unique<SwaSymbol>(expr->Proto->Ret.get()->Clone(),
                                  SwaSymbolKind::Function, expr->Proto.get()));

  expr->Body->Accept(*this);
}
void TypeCheckVisitor::Visit(MainExpr *expr) {}
void TypeCheckVisitor::Visit(MulExpr *expr) {
  log("Visit MulExpr");
  ValidateArithmetic(expr->Left, expr->Right, expr->span, "*");

  if (expr->Left->datatype->GetKind() == TypeKind::DirtyType ||
      expr->Right->datatype->GetKind() == TypeKind::DirtyType) {
    log("Visit MulExpr left or right is dirty");

    expr->datatype = std::make_unique<TypeDirty>(expr->span);
    return;
  }

  expr->datatype = expr->Left->datatype->Clone();
}
void TypeCheckVisitor::Visit(PrintExpr *expr) {
  for (auto &e : expr->Values) {
    e->Accept(*this);
  }
}
void TypeCheckVisitor::Visit(Formatted_Print_Expr *expr) {}
void TypeCheckVisitor::Visit(ProtoExpr *expr) {}
void TypeCheckVisitor::Visit(ReturnExpr *expr) {}
void TypeCheckVisitor::Visit(StrExpr *expr) {
  expr->datatype = std::make_unique<TypeString>(expr->span);
}
void TypeCheckVisitor::Visit(NumberExpr *expr) {
  log("Visit NumberExpr");
  expr->datatype = std::make_unique<TypeInt>(expr->span);
}
void TypeCheckVisitor::Visit(StructDefExpr *expr) {}
void TypeCheckVisitor::Visit(SubExpr *expr) {
  ValidateArithmetic(expr->Left, expr->Right, expr->span, "-");

  expr->datatype = expr->Left->datatype->Clone();
}
void TypeCheckVisitor::Visit(UnaryMinusExpr *expr) {
  log("Visit UnaryMinusExpr");
  expr->Right->Accept(*this);

  expr->datatype = expr->Right->datatype->Clone();
}
void TypeCheckVisitor::Visit(UnaryNotExpr *expr) {
  expr->Right->Accept(*this);

  auto btyp = std::make_unique<TypeBool>(expr->span);

  if (!expr->datatype->IsEqual(btyp.get())) {
    CodeGenException("Type Error", expr->span,
                     "The '!' operator cannot be applied to type 'Bool'.",
                     ""
                     "");
  }

  expr->datatype = std::move(btyp);
}

void TypeCheckVisitor::Visit(CallExpr *expr) {
  log("Visit CallExpr");
  expr->Callee->Accept(*this);
  if (auto id = dynamic_cast<IdExpr *>(expr->Callee.get())) {
    try {
      log("Visit CallExpr lookupSwaFunc with " + id->Name);
      auto sym = driver->currentSymbols->lookupSwaFunc(id->Name);
      expr->datatype = sym->type->Clone();
      return;
    } catch (...) {
      expr->datatype = std::make_unique<TypeDirty>(expr->span);
      return;
    }
  }

  expr->datatype = expr->Callee->datatype->Clone();
}

void TypeCheckVisitor::Visit(Logical_Or_Expr *expr) {
  expr->Left->Accept(*this);
  expr->Right->Accept(*this);

  expr->datatype = std::make_unique<TypeBool>(expr->span);
}
void TypeCheckVisitor::Visit(Logical_And_Expr *expr) {
  expr->Left->Accept(*this);
  expr->Right->Accept(*this);

  expr->datatype = std::make_unique<TypeBool>(expr->span);
}
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

void TypeCheckVisitor::ValidateArithmetic(std::unique_ptr<Expr> &left,
                                          std::unique_ptr<Expr> &right,
                                          const Span &span,
                                          const std::string &opName) {
  left->Accept(*this);
  right->Accept(*this);

  std::string opTitle;

  if (opName == "+") {
    opTitle = "addition";
  } else if (opName == "-") {
    opTitle = "subtraction";
  } else if (opName == "*") {
    opTitle = "division";
  } else if (opName == "-") {
    opTitle = "multiplication";
  }

  if (!left->datatype->IsEqual(right->datatype.get())) {
    auto err = CodeGenException("Type mismatch in " + opTitle + " ", span,
                                "Cannot '" + opName + "' variable of type " +
                                    left->datatype->GetName() +
                                    " with a value of type " +
                                    right->datatype->GetName(),
                                "");
    errors.push_back(err);
    left->datatype = std::make_unique<TypeDirty>(left->span);
    right->datatype = std::make_unique<TypeDirty>(right->span);
  }

  if (left->datatype->GetKind() == TypeKind::String &&
      right->datatype->GetKind() == TypeKind::String) {
    std::string msg = "Operator '" + opName + "' is not supported for types '" +
                      left->datatype->GetName() + "' and '" +
                      right->datatype->GetName() + "'.";

    auto err = CodeGenException("Invalid Operation", span, msg,
                                "Check your types or use the appropriate "
                                "library function for this operation.");
    errors.push_back(err);
    left->datatype = std::make_unique<TypeDirty>(left->span);
    right->datatype = std::make_unique<TypeDirty>(right->span);
  }

  if (left->datatype->GetKind() == TypeKind::Bool &&
      right->datatype->GetKind() == TypeKind::Bool) {
    auto err = CodeGenException(
        "Type Error", span,
        "The '" + opName + "'  operator cannot be applied to type 'Bool'.",
        "Consider using logical operators like '&&' or '||' "
        "if you intended to perform a logical operation.");
    errors.push_back(err);
    left->datatype = std::make_unique<TypeDirty>(left->span);
    right->datatype = std::make_unique<TypeDirty>(right->span);
  }
}

void TypeCheckVisitor::ValidateComparison(std::unique_ptr<Expr> &left,
                                          std::unique_ptr<Expr> &right,
                                          const Span &span,
                                          const std::string &opName) {
  left->Accept(*this);
  right->Accept(*this);

  if (!left->datatype->IsEqual(right->datatype.get())) {
    errors.push_back(CodeGenException("Type mismatch in comparison", span,
                                      "Cannot compare " +
                                          left->datatype->GetName() + " with " +
                                          right->datatype->GetName()));
    return;
  }

  auto kind = left->datatype->GetKind();
  if (kind == TypeKind::Bool || kind == TypeKind::String) {
    errors.push_back(CodeGenException(
        "Invalid Comparison", span,
        "The '" + opName + "' operator is not supported for type '" +
            left->datatype->GetName() + "'"));
  }
}

void TypeCheckVisitor::Visit(Array_Access_Expr *expr) {
  log("Array_Access_Expr visiting Index");
  expr->Index->Accept(*this);

  log("Array_Access_Expr visiting Array");
  expr->Array->Accept(*this);

  if (expr->Index->datatype->GetKind() == TypeKind::DirtyType ||
      expr->Array->datatype->GetKind() == TypeKind::DirtyType) {
    log("Array_Access_Expr Array or Index is dirty");
    expr->datatype = std::make_unique<TypeDirty>(expr->span);
    return;
  }

  if (expr->Index->datatype->GetKind() != TypeKind::Int) {
    log("Array_Access_Expr Index is not a number");
    errors.push_back(
        CodeGenException("Array index must be of type int", expr->Index->span));
    expr->datatype = std::make_unique<TypeDirty>(expr->span);
    return;
  }

  if (auto v = dynamic_cast<UnaryMinusExpr *>(expr->Index.get())) {
    log("Array_Access_Expr Index is a uminus expr");
    errors.push_back(
        CodeGenException("Array index must be positive", expr->Index->span));
    expr->datatype = std::make_unique<TypeDirty>(expr->span);

    return;
  }

  if (auto num = dynamic_cast<NumberExpr *>(expr->Index.get())) {
    if (num->Value < 0) {
      log("Array_Access_Expr Index is a negative number");
      errors.push_back(
          CodeGenException("Array index must be positive", expr->Index->span));
      expr->datatype = std::make_unique<TypeDirty>(expr->span);
      return;
    }

    if (expr->Array->datatype->GetKind() != TypeKind::Array) {
      return;
    }

    auto arr = dynamic_cast<TypeArray *>(expr->Array->datatype.get());

    // FIXME, we should check bounds
    // when index is an identifier
    if (num->Value >= arr->Size) {
      errors.push_back(CodeGenException(
          "Out of bounds array access", expr->Index->span,
          "Index " + std::to_string(num->Value) + " is out of bounds",
          "Array contains " + std::to_string(arr->Size) +
              " values and valid indexes go from 0 to " +
              std::to_string(arr->Size - 1)));
      expr->datatype = std::make_unique<TypeDirty>(expr->span);
      return;
    }
  }

  auto t = dynamic_cast<TypeArray *>(expr->Array->datatype.get());
  expr->datatype = t->T->Clone();
}

void TypeCheckVisitor::Visit(Array_Init_Expr *expr) {
  log("Visit Array_Init_Expr ");

  if (expr->Elements.size() == 0) {
    log("Visit Array_Init_Expr array is empty");
    auto err = CodeGenException("Array initialization must have values",
                                expr->span, "", "Add values to the array");
    errors.push_back(err);

    expr->datatype = std::make_unique<TypeDirty>(expr->span);

    return;
  }

  expr->Elements[0]->Accept(*this);

  std::unique_ptr<Type> datatype = expr->Elements[0]->datatype->Clone();

  for (auto &e : expr->Elements) {
    e->Accept(*this);

    if (!datatype->IsEqual(e->datatype.get())) {
      auto err = CodeGenException("Mixed datatatypes in array initialization",
                                  e->span, "", "Use only one datatype");
      errors.push_back(err);

      expr->datatype = std::make_unique<TypeDirty>(expr->span);
      return;
    }
  }

  expr->datatype = std::make_unique<TypeArray>(expr->Elements.size(),
                                               datatype->Clone(), expr->span);
}
void TypeCheckVisitor::Visit(FloatExpr *expr) {
  expr->datatype = std::make_unique<TypeFloat>(expr->span);
}
