#ifndef INCLUDE_COMPILER_CODEGEN_H_
#define INCLUDE_COMPILER_CODEGEN_H_

#include <ast/expr.h>
#include <ast/symboltable.h>
#include <ast/visitor.h>
#include <compiler/driver.h>
#include <llvm/IR/DIBuilder.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <string>
#include <vector>

class CodeGenVisitor : public ASTVisitor {
private:
  std::unique_ptr<SwaCompilerDriver> driver;

  llvm::Value *lastValue = nullptr;
  llvm::Function *lastFunc = nullptr;
  llvm::Type *lastType = nullptr;

  void setLastFunc(llvm::Function *v);
  void setLastValue(llvm::Value *v);
  void setLastType(llvm::Type *t);
  llvm::Type *setLastType();
  llvm::Value *getLastValue();
  llvm::Function *getLastFunc();
  llvm::Value *evaluate(Expr *expr);
  llvm::Type *evaluate(Type *expr);
  std::pair<llvm::Value *, llvm::Value *> evaluate_bin_expr(Expr *left,
                                                            Expr *right);
  std::pair<std::string, std::vector<llvm::Value *>> buildFormatStringAndArgs(
      const std::vector<std::unique_ptr<Expr>> &expressions);

public:
  CodeGenVisitor(std::unique_ptr<SwaCompilerDriver> d) : driver(std::move(d)) {}

  std::vector<std::string>
  visitTestExpressions(std::vector<std::unique_ptr<Test_Expr>> tests);

  std::unique_ptr<SwaCompilerDriver> finalize() { return std::move(driver); }
  void generateTestEntrypoint(const std::vector<std::string> &testNames);
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

  virtual void Visit(TypeSlice *expr);
  virtual void Visit(TypeArray *expr);
  virtual void Visit(TypeInt *expr);
  virtual void Visit(TypeFloat *expr);
  virtual void Visit(TypeString *expr);
  virtual void Visit(TypeVoid *expr);
  virtual void Visit(TypeBool *expr);
  virtual void Visit(TypeByte *expr);
  virtual void Visit(TypeStruct *expr);
};
#endif // INCLUDE_COMPILER_CODEGEN_H_
