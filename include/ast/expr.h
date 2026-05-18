#include <ast/type.h>
#include <ast/visitor.h>
#include <lexer/lexer.h>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>

#ifndef AST_EXPR
#define AST_EXPR

class Expr {
public:
  Span span;
  virtual ~Expr() {}
  virtual void Accept(ASTVisitor &visitor) {}
};

#endif // !AST_EXPR
