#pragma once

#include <ast/type.h>
#include <ast/visitor.h>
#include <lexer/lexer.h>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <memory>

class Expr {
public:
  Span span;
  std::unique_ptr<Type> datatype;
  virtual ~Expr() = default;
  explicit Expr(Span s) : span(s) {}
  virtual void Accept(ASTVisitor &visitor) {}
};
