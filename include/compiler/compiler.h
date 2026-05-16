#include <lexer/keywords.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/IR/DIBuilder.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <memory>
#include <parser/parser.h>
#include <string>

class SwaCompiler {
public:
  SwaCompiler() {}
  void Build(const std::string &_source);
  void Run(const std::string &_source);
  void Test(const std::string &_source);
};
