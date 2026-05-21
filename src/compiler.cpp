#include <compiler/codegen.h>
#include <compiler/declaration.h>
#include <compiler/typechecker.h>
#include <lexer/lexer.h>

#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Verifier.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Passes/StandardInstrumentations.h>
#include <llvm/Support/CodeGen.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/TargetParser/Triple.h>

#include <compiler/codegen.h>
#include <compiler/compiler.h>
#include <cstdlib>
#include <iostream>

#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

void SwaCompiler::Run(const std::string &_source) {
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  SourceManager sm(_source);

  Lexer l(_source, KEYWORDS_ENGLISH);
  Parser parser(l.tokenize());

  try {
    auto program = parser.parseProgram();
    auto driver = std::make_unique<SwaCompilerDriver>("swa_module");

    DeclarationVisitor decl = DeclarationVisitor(std::move(driver));
    program->Accept(decl);
    driver = decl.finalize();
    dumpModuleToFile(driver->Module.get(), "decl_module.ll");

    TypeCheckVisitor check = TypeCheckVisitor(std::move(driver));
    program->Accept(check);
    driver = check.finalize();

    CodeGenVisitor gen = CodeGenVisitor(std::move(driver));

    program->Accept(gen);
    driver = gen.finalize();
    dumpModuleToFile(driver->Module.get(), "run_module.ll");
    verifyModule(driver->Module.get());

    llvm::ExecutionEngine *engine =
        llvm::EngineBuilder(std::move(driver->Module)).create();
    auto Fn = engine->FindFunctionNamed("main");
    if (!Fn) {
      throw std::runtime_error("Function main not defined");
    }
    engine->runFunctionAsMain(Fn, std::vector<std::string>(), nullptr);
  } catch (const CodeGenException &err) {
    err.emitDiagnostic(sm, "code generation error");
  } catch (const ParserException &err) {
    err.emitDiagnostic(sm, "error");
  }
}

void SwaCompiler::Build(const std::string &_source) {
  llvm::InitializeAllTargetInfos();
  llvm::InitializeNativeTarget();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();
  auto TargetTriple = llvm::sys::getDefaultTargetTriple();

  std::string Error;
  auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);

  auto CPU = "generic";
  auto Features = "";
  llvm::TargetOptions opt;
  auto RM = llvm::Reloc::Model::PIC_;
  auto TargetMachine =
      Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);

  Lexer l(_source, KEYWORDS_ENGLISH);
  Parser parser(l.tokenize());
  auto program = parser.parseProgram();

  auto driver = std::make_unique<SwaCompilerDriver>("swa_module");
  CodeGenVisitor gen = CodeGenVisitor(std::move(driver));
  program->Accept(gen);
  driver = gen.finalize();
  verifyModule(driver->Module.get());
  emitObjectFile(driver->Module.get(), TargetMachine);
  link("output.o", "output");
}

void SwaCompiler::Test(const std::string &_source) {
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  Lexer l(_source, KEYWORDS_ENGLISH);
  Parser parser(l.tokenize());
  auto program = parser.parseProgram();

  auto driver = std::make_unique<SwaCompilerDriver>("swa_test_module", true);
  CodeGenVisitor gen = CodeGenVisitor(std::move(driver));

  program->Accept(gen);
  auto testNames = gen.visitTestExpressions(std::move(parser.Tests()));
  gen.generateTestEntrypoint(testNames);

  driver = gen.finalize();
  dumpModuleToFile(driver->Module.get(), "test_module.ll");
  verifyModule(driver->Module.get());

  llvm::ExecutionEngine *engine =
      llvm::EngineBuilder(std::move(driver->Module)).create();

  auto Fn = engine->FindFunctionNamed("main");
  engine->runFunctionAsMain(Fn, std::vector<std::string>(), nullptr);
}

void dumpModuleToFile(llvm::Module *module, const std::string &filename) {
  std::error_code errorCode;

  llvm::raw_fd_ostream fileStream(filename, errorCode, llvm::sys::fs::OF_None);

  if (errorCode) {
    llvm::errs() << "Error opening file for dumping IR: " << errorCode.message()
                 << "\n";
    return;
  }

  module->print(fileStream, nullptr);

  fileStream.flush();
}

void emitObjectFile(llvm::Module *TheModule,
                    llvm::TargetMachine *TargetMachine) {
  auto TargetTriple = llvm::sys::getDefaultTargetTriple();
  TheModule->setTargetTriple(llvm::Triple(TargetTriple));
  TheModule->setDataLayout(TargetMachine->createDataLayout());

  std::error_code EC;
  llvm::raw_fd_ostream dest("output.o", EC, llvm::sys::fs::OF_None);

  if (EC) {
    llvm::errs() << "Could not open file: " << EC.message();
    return;
  }

  llvm::LoopAnalysisManager LAM;
  llvm::FunctionAnalysisManager FAM;
  llvm::CGSCCAnalysisManager CGAM;
  llvm::ModuleAnalysisManager MAM;

  llvm::PassBuilder PB(TargetMachine);

  PB.registerModuleAnalyses(MAM);
  PB.registerCGSCCAnalyses(CGAM);
  PB.registerFunctionAnalyses(FAM);
  PB.registerLoopAnalyses(LAM);
  PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

  llvm::ModulePassManager MPM =
      PB.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O0);

  MPM.run(*TheModule, MAM);

  llvm::legacy::PassManager CodeGenPass;
  if (TargetMachine->addPassesToEmitFile(CodeGenPass, dest, nullptr,
                                         llvm::CodeGenFileType::ObjectFile)) {
    llvm::errs() << "TargetMachine can't emit a file of this type";
    return;
  }

  CodeGenPass.run(*TheModule);
  dest.flush();
}

void link(const std::string &objectFile, const std::string &executableName) {
  std::string command = "clang " + objectFile + " -o " + executableName;

  std::cout << "Linking executable: " << executableName << "..." << std::endl;
  int result = std::system(command.c_str());

  if (result != 0) {
    throw std::runtime_error("Linking failed with exit code " +
                             std::to_string(result));
  }

  std::cout << "Successfully generated " << executableName << std::endl;
}

void verifyModule(llvm::Module *module) {
  if (llvm::verifyModule(*module, &llvm::errs())) {
    llvm::errs()
        << "> Fatal Error: Generated LLVM IR is structurally malformed!\n";
    std::exit(1);
  }
}
