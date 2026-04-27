#include <lexer/keywords.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <parser/parser.h>
#include <string>

class SwaCompiler {
public:
  SwaCompiler() { moduleInit(); }

  void Run(const std::string &_source);

  void Build(std::string program) {}

private:
  std::unique_ptr<llvm::LLVMContext> context;
  std::unique_ptr<llvm::Module> m;
  std::unique_ptr<llvm::IRBuilder<>> builder;
  llvm::Function *fn;

  void moduleInit();
};
