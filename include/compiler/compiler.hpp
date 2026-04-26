#include "../lexer/keywords.hpp"
#include "../parser/parser.h"
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <string>
#include <vector>

class SwaCompiler {
public:
  SwaCompiler() { moduleInit(); }

  void Run(const std::string &_source) {
    Lexer l(_source, KEYWORDS_ENGLISH);
    Parser parser(l.tokenize());
    auto program = parser.parseProgram();
    auto st = SymbolTable();
    program->Codegen(context, m, builder, st);

    llvm::ExecutionEngine *engine = llvm::EngineBuilder(std::move(m)).create();
    auto Fn = engine->FindFunctionNamed("main");
    engine->runFunctionAsMain(Fn, std::vector<std::string>(), nullptr);
  }

  void Build(std::string program) {}

private:
  std::unique_ptr<llvm::LLVMContext> context;
  std::unique_ptr<llvm::Module> m;
  std::unique_ptr<llvm::IRBuilder<>> builder;
  llvm::Function *fn;

  void moduleInit() {
    context = std::make_unique<llvm::LLVMContext>();
    m = std::make_unique<llvm::Module>("swac", *context);
    builder = std::make_unique<llvm::IRBuilder<>>(*context);
  }
};
