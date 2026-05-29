#pragma once

#include <ast/expr.h>
#include <ast/symboltable.h>
#include <ast/visitor.h>
#include <compiler/driver.h>
#include <parser/exception.h>

#include <llvm/IR/DIBuilder.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include <fstream>
#include <stdexcept>
#include <utility>

class TypeCheckVisitorException : public std::exception {
  std::vector<ParserException> errors;

public:
  explicit TypeCheckVisitorException(std::vector<ParserException> errors)
      : errors(std::move(errors)) {}
  void emitDiagnostic(const SourceManager &sm,
                      std::string error_category) const;
};

class TypeCheckVisitor : public ASTVisitor {
private:
  std::unique_ptr<SwaCompilerDriver> driver;
  std::vector<ParserException> errors;
  SwaSymbol *lastSymbol;
  std::ofstream debugLog;

  void ValidateArithmetic(std::unique_ptr<Expr> &left,
                          std::unique_ptr<Expr> &right, const Span &span,
                          const std::string &opName);
  void ValidateComparison(std::unique_ptr<Expr> &left,
                          std::unique_ptr<Expr> &right, const Span &span,
                          const std::string &opName);

public:
  TypeCheckVisitor(std::unique_ptr<SwaCompilerDriver> d)
      : driver(std::move(d)) {
    debugLog.open("typecheck_debug.log", std::ios::out | std::ios::trunc);
  }

  void checkErrors();
  void setLastSymbol(SwaSymbol *s) { lastSymbol = s; }
  SwaSymbol *getLastSymbol() {
    auto old = lastSymbol;
    lastSymbol = nullptr;
    return old;
  }

  void log(const std::string &msg) {
    if (debugLog.is_open()) {
      debugLog << msg << std::endl;
      debugLog.flush();
    }
  }

  std::unique_ptr<SwaCompilerDriver> finalize() { return std::move(driver); }
  void Visit(AddExpr *expr);
  void Visit(BoolExpr *expr);
  void Visit(BlockExpr *expr);
  void Visit(DeclarationExpr *expr);
  void Visit(DivExpr *expr);
  void Visit(EqExpr *expr);
  void Visit(IdExpr *expr);
  void Visit(IfExpr *expr);
  void Visit(GTExpr *expr);
  void Visit(GTEExpr *expr);
  void Visit(LTExpr *expr);
  void Visit(LTEExpr *expr);
  void Visit(FuncExpr *expr);
  void Visit(MainExpr *expr);
  void Visit(NumberExpr *expr);
  void Visit(MulExpr *expr);
  void Visit(Formatted_Print_Expr *expr);
  void Visit(PrintExpr *expr);
  void Visit(ProtoExpr *expr);
  void Visit(ReturnExpr *expr);
  void Visit(StrExpr *expr);
  void Visit(StructDefExpr *expr);
  void Visit(SubExpr *expr);
  void Visit(UnaryMinusExpr *expr);
  void Visit(UnaryNotExpr *expr);
  void Visit(CallExpr *expr);
  void Visit(Logical_Or_Expr *expr);
  void Visit(Logical_And_Expr *expr);
  void Visit(Test_Expr *expr);
  void Visit(Assert_Equal_Expr *expr);
  void Visit(Assert_Not_Equal_Expr *expr);
  void Visit(Assert_True_Expr *expr);
  void Visit(Assert_False_Expr *expr);
  void Visit(Assert_Less_Than_Expr *expr);
  void Visit(Assert_Less_Than_Equal_Expr *expr);
  void Visit(Assert_Greater_Than_Expr *expr);
  void Visit(Assert_Greater_Than_Equals_Expr *expr);
  void Visit(Array_Access_Expr *expr);
  void Visit(Array_Init_Expr *expr);
  void Visit(FloatExpr *expr);

  virtual void Visit(TypeSlice *expr);
  virtual void Visit(TypeArray *expr);
  virtual void Visit(TypeInt *expr);
  virtual void Visit(TypeFloat *expr);
  virtual void Visit(TypeString *expr);
  virtual void Visit(TypeVoid *expr);
  virtual void Visit(TypeBool *expr);
  virtual void Visit(TypeByte *expr);
  virtual void Visit(TypeStruct *expr);
  virtual void Visit(TypePointer *expr);
};
