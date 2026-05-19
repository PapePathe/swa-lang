#include "ast/node.h"
#include "compiler/declaration.h"
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

void DeclarationVisitor::Visit(ProtoExpr *expr) {}
void DeclarationVisitor::Visit(MainExpr *expr) {}
void DeclarationVisitor::Visit(FuncExpr *expr) {}
void DeclarationVisitor::Visit(StructDefExpr *expr) {}
