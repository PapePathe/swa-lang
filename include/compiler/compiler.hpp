#include "../lexer/keywords.hpp"
#include "../parser/parser.h"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <string>

class SwaCompiler {
public:
  SwaCompiler() { moduleInit(); }

  void Run(const std::string &_source) {
    Lexer l(_source, KEYWORDS_ENGLISH);
    Parser parser(l.tokenize());
    auto program = parser.parseProgram();
    auto st = SymbolTable();
    program->Codegen(context, m, builder, st);

    saveModule("./out.ll");
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

  void saveModule(const std::string &name) {
    std::error_code code;
    llvm::raw_fd_ostream outF(name, code);
    m->print(outF, nullptr);
  }
};
