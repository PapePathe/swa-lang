#include <compiler/compiler.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <string>
#include <vector>

void SwaCompiler::Run(const std::string &_source) {
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  Lexer l(_source, KEYWORDS_ENGLISH);
  Parser parser(l.tokenize());
  auto program = parser.parseProgram();
  auto st = SymbolTable();
  program->Codegen(context, m, builder, st);

  llvm::ExecutionEngine *engine = llvm::EngineBuilder(std::move(m)).create();
  auto Fn = engine->FindFunctionNamed("main");
  engine->runFunctionAsMain(Fn, std::vector<std::string>(), nullptr);
}

void SwaCompiler::Build(const std::string &_source) {
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  Lexer l(_source, KEYWORDS_ENGLISH);
  Parser parser(l.tokenize());
  auto program = parser.parseProgram();
  auto st = SymbolTable();
  program->Codegen(context, m, builder, st);

  llvm::WriteBitcodeToFile(*m, llvm::outs());
}

// void SwaCompiler::Test(const std::string &_source) {}

void SwaCompiler::moduleInit() {
  context = std::make_unique<llvm::LLVMContext>();
  m = std::make_unique<llvm::Module>("swac", *context);
  builder = std::make_unique<llvm::IRBuilder<>>(*context);
}
