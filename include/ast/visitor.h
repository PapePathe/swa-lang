#ifndef INCLUDE_AST_VISITOR_H_
#define INCLUDE_AST_VISITOR_H_

#include <llvm/IR/Value.h>

class NumberExpr;
class AddExpr;
class DeclarationExpr;
class PrintExpr;
class Formatted_Print_Expr;
class StructDefExpr;
class IfExpr;
class IdExpr;
class BlockExpr;
class DivExpr;
class EqExpr;
class MulExpr;
class SubExpr;
class AddExpr;
class GTExpr;
class LTExpr;
class GTEExpr;
class LTEExpr;
class ProtoExpr;
class FuncExpr;
class MainExpr;
class StrExpr;
class UnaryMinusExpr;
class UnaryNotExpr;
class ReturnExpr;
class BoolExpr;
class CallExpr;
class Logical_And_Expr;
class Logical_Or_Expr;

class Test_Expr;
class Assert_True_Expr;
class Assert_False_Expr;
class Assert_Equal_Expr;
class Assert_Not_Equal_Expr;
class Assert_Less_Than_Expr;
class Assert_Less_Than_Equal_Expr;
class Assert_Greater_Than_Expr;
class Assert_Greater_Than_Equals_Expr;

class TypeInt;
class TypeFloat;
class TypeBool;
class TypeByte;
class TypeString;
class TypeVoid;
class TypeStruct;
class TypeArray;
class TypeSlice;
class TypePointer;

class ASTVisitor {
public:
  virtual ~ASTVisitor() = default;
  virtual void Visit(AddExpr *expr) = 0;
  virtual void Visit(BoolExpr *expr) = 0;
  virtual void Visit(BlockExpr *expr) = 0;
  virtual void Visit(DeclarationExpr *expr) = 0;
  virtual void Visit(DivExpr *expr) = 0;
  virtual void Visit(EqExpr *expr) = 0;
  virtual void Visit(IdExpr *expr) = 0;
  virtual void Visit(IfExpr *expr) = 0;
  virtual void Visit(GTExpr *expr) = 0;
  virtual void Visit(GTEExpr *expr) = 0;
  virtual void Visit(LTExpr *expr) = 0;
  virtual void Visit(LTEExpr *expr) = 0;
  virtual void Visit(FuncExpr *expr) = 0;
  virtual void Visit(MainExpr *expr) = 0;
  virtual void Visit(NumberExpr *expr) = 0;
  virtual void Visit(MulExpr *expr) = 0;
  virtual void Visit(Formatted_Print_Expr *expr) = 0;
  virtual void Visit(PrintExpr *expr) = 0;
  virtual void Visit(ProtoExpr *expr) = 0;
  virtual void Visit(ReturnExpr *expr) = 0;
  virtual void Visit(StrExpr *expr) = 0;
  virtual void Visit(StructDefExpr *expr) = 0;
  virtual void Visit(SubExpr *expr) = 0;
  virtual void Visit(UnaryMinusExpr *expr) = 0;
  virtual void Visit(UnaryNotExpr *expr) = 0;
  virtual void Visit(CallExpr *expr) = 0;
  virtual void Visit(Logical_And_Expr *expr) = 0;
  virtual void Visit(Logical_Or_Expr *expr) = 0;
  virtual void Visit(Test_Expr *expr) = 0;
  virtual void Visit(Assert_Equal_Expr *expr) = 0;
  virtual void Visit(Assert_Not_Equal_Expr *expr) = 0;
  virtual void Visit(Assert_True_Expr *expr) = 0;
  virtual void Visit(Assert_False_Expr *expr) = 0;
  virtual void Visit(Assert_Less_Than_Expr *expr) = 0;
  virtual void Visit(Assert_Less_Than_Equal_Expr *expr) = 0;
  virtual void Visit(Assert_Greater_Than_Expr *expr) = 0;
  virtual void Visit(Assert_Greater_Than_Equals_Expr *expr) = 0;

  virtual void Visit(TypeSlice *expr) = 0;
  virtual void Visit(TypeArray *expr) = 0;
  virtual void Visit(TypeInt *expr) = 0;
  virtual void Visit(TypeFloat *expr) = 0;
  virtual void Visit(TypeString *expr) = 0;
  virtual void Visit(TypeVoid *expr) = 0;
  virtual void Visit(TypeBool *expr) = 0;
  virtual void Visit(TypeByte *expr) = 0;
  virtual void Visit(TypeStruct *expr) = 0;
  virtual void Visit(TypePointer *expr) = 0;
};

#endif // INCLUDE_SWA_VISITOR_H_
