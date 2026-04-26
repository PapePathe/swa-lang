#include "../ast/node.hpp"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <string>
#include <vector>

class SwaCompiler {
public:
  SwaCompiler() { moduleInit(); }

  void Run(const std::string &_source) {
    std::vector<std::unique_ptr<Expr>> programStmts;

    auto proto1 = std::make_unique<ProtoExpr>(
        "add_one", std::vector<std::string>{"val"}, std::vector<Type>());
    std::vector<std::unique_ptr<Expr>> body1;
    body1.push_back(std::make_unique<PrintExpr>("Inc...\n"));
    programStmts.push_back(std::make_unique<FuncExpr>(
        std::move(proto1), std::make_unique<BlockExpr>(std::move(body1))));

    auto mainProto = std::make_unique<ProtoExpr>(
        "main", std::vector<std::string>{}, std::vector<Type>());
    std::vector<std::unique_ptr<Expr>> mainBody;
    mainBody.push_back(std::make_unique<DeclarationExpr>(
        "x", std::make_unique<NumberExpr>(21), TypeInt()));
    mainBody.push_back(std::make_unique<PrintExpr>("Inside Main\n"));

    programStmts.push_back(std::make_unique<FuncExpr>(
        std::move(mainProto),
        std::make_unique<BlockExpr>(std::move(mainBody))));

    // 2. The entire program is a block of these functions
    auto program = std::make_unique<BlockExpr>(std::move(programStmts));

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
