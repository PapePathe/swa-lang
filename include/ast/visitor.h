#ifndef SWA_VISITOR
#define SWA_VISITOR

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
};

#endif // !SWA_VISITOR
