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

void link(const std::string &objectFile, const std::string &executableName);
void verifyModule(llvm::Module *module);
void dumpModuleToFile(llvm::Module *module, const std::string &filename);
void emitObjectFile(llvm::Module *TheModule,
                    llvm::TargetMachine *TargetMachine);
