#ifndef SWA_CODEGEN
#define SWA_CODEGEN

#include <ast/expr.h>
#include <ast/symboltable.h>
#include <ast/visitor.h>
#include <llvm/IR/DIBuilder.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>

struct SourceLocation {
  size_t line;
  size_t column;
};

struct DebugInfoContext {
  std::unique_ptr<llvm::DIBuilder> builder;
  llvm::DICompileUnit *compileUnit = nullptr;
  llvm::DIScope *currentScope = nullptr;

  void emitLocation(SourceLocation loc, llvm::IRBuilder<> &b) {
    if (!currentScope) {
      b.SetCurrentDebugLocation(llvm::DebugLoc());
      return;
    }
    llvm::DILocation *DILoc = llvm::DILocation::get(b.getContext(), loc.line,
                                                    loc.column, currentScope);
    b.SetCurrentDebugLocation(DILoc);
  }
};

class SwaCompilerDriver {
public:
  llvm::LLVMContext Context;
  llvm::IRBuilder<> Builder;
  std::unique_ptr<llvm::Module> Module;
  SymbolTable Symbols;
  DebugInfoContext DebugInfo;

  SwaCompilerDriver(std::string moduleName)
      : Builder(Context),
        Module(std::make_unique<llvm::Module>(moduleName, Context)) {}
};

class CodeGenVisitor : public ASTVisitor {
private:
  std::unique_ptr<SwaCompilerDriver> driver;

  llvm::Value *lastValue = nullptr;
  llvm::Function *lastFunc = nullptr;

  void setLastFunc(llvm::Function *v);
  void setLastValue(llvm::Value *v);
  llvm::Value *getLastValue();
  llvm::Function *getLastFunc();

public:
  CodeGenVisitor(std::unique_ptr<SwaCompilerDriver> d) : driver(std::move(d)) {}

  std::unique_ptr<SwaCompilerDriver> finalize() { return std::move(driver); }

  llvm::Value *evaluate(Expr *expr);

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
  void Visit(PrintExpr *expr);
  void Visit(ProtoExpr *expr);
  void Visit(ReturnExpr *expr);
  void Visit(StrExpr *expr);
  void Visit(StructDefExpr *expr);
  void Visit(SubExpr *expr);
  void Visit(UnaryMinusExpr *expr);
  void Visit(UnaryNotExpr *expr);
};
#endif // !SWA_CODEGEN
