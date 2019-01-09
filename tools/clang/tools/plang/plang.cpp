#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

#include "llvm/ADT/StringRef.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Linker/Linker.h"
#include "llvm/Option/ArgList.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/SystemUtils.h"
#include "llvm/Support/Timer.h"
#include "llvm/Support/ToolOutputFile.h"

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Expr.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/Stmt.h"
#include "clang/Sema/Sema.h"
#include "clang/Basic/IdentifierTable.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/CodeGen/BackendUtil.h"
#include "clang/CodeGen/CodeGenAction.h"
#include "clang/CodeGen/ModuleBuilder.h"
#include "clang/Driver/Options.h"
#include "clang/Frontend/ASTUnit.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Tooling/Tooling.h"

#include "Option.h"

using namespace std;
using namespace llvm;
using namespace clang;
using namespace tooling;
using namespace opt;

bool ParseArgs(int, char *[], PlangOption &);

void modifyAST(TranslationUnitDecl *) {}

void MPCPass(llvm::Module &, vector<Function *> &);

void GenCode(llvm::Module &, vector<Function *> &, PlangOption &);

class FunctionTemplateCGVisitor : public RecursiveASTVisitor<FunctionTemplateCGVisitor> {
  CodeGenerator *CG;
public:
  FunctionTemplateCGVisitor(CodeGenerator *CG):CG(CG){}
  bool VisitFunctionTemplateDecl(FunctionTemplateDecl* FTD){
    for(FunctionDecl* FD : FTD->specializations())
      CG->HandleTopLevelDecl(DeclGroupRef(FD));
    return true;
  }
};

class BuilderAction : public ToolAction {
public:
  std::unique_ptr<llvm::Module> Module;
  LLVMContext &llvmContext;

  BuilderAction(LLVMContext &llvmContext) : llvmContext(llvmContext) {}

  bool runInvocation(std::shared_ptr<CompilerInvocation> Invocation,
                     FileManager *Files,
                     std::shared_ptr<PCHContainerOperations> PCHContainerOps,
                     DiagnosticConsumer *DiagConsumer) override {

    FrontendInputFile &Input = Invocation->getFrontendOpts().Inputs[0];

    IntrusiveRefCntPtr<clang::DiagnosticsEngine> Diags =
        CompilerInstance::createDiagnostics(&Invocation->getDiagnosticOpts(),
                                            DiagConsumer, false);

    if (Input.getKind().getLanguage() == InputKind::LLVM_IR) {

      SMDiagnostic Err;

      Module = parseIRFile(Input.getFile(), Err, llvmContext);

    } else {
      std::unique_ptr<ASTUnit> AST = ASTUnit::LoadFromCompilerInvocation(
          Invocation, std::move(PCHContainerOps), Diags, Files);

      if (!AST)
        return false;

      ASTContext &astContext = AST->getASTContext();
      TranslationUnitDecl *UnitDecl = astContext.getTranslationUnitDecl();

      modifyAST(UnitDecl);

      CodeGenerator *CG = CreateLLVMCodeGen(
          *Diags, "module0", Invocation->getHeaderSearchOpts(),
          Invocation->getPreprocessorOpts(), Invocation->getCodeGenOpts(),
          llvmContext);

      CG->Initialize(astContext);

      for (Decl *D : UnitDecl->decls())
         CG->HandleTopLevelDecl(DeclGroupRef(D));

      FunctionTemplateCGVisitor Visitor(CG);
      Visitor.TraverseDecl(UnitDecl);

      for (Decl *D : AST->getSema().WeakTopLevelDecls())
        CG->HandleTopLevelDecl(DeclGroupRef(D));

      CG->HandleTranslationUnit(AST->getASTContext());

      Module = std::unique_ptr<llvm::Module>(CG->GetModule());
    }

    std::unique_ptr<raw_null_ostream> nullstream;

    clang::EmitBackendOutput(
        *Diags, *Invocation->HeaderSearchOpts, Invocation->getCodeGenOpts(),
        Invocation->getTargetOpts(), *Invocation->getLangOpts(),
        Module->getDataLayout(), Module.get(), Backend_EmitNothing,
        std::move(nullstream));


    return true;
  }
};

void Output(llvm::Module &M, string OutputFilename) {

  std::error_code EC;
  ToolOutputFile Out(OutputFilename, EC, sys::fs::F_None);
  if (EC) {
    errs() << EC.message() << '\n';
    return;
  }

  if (verifyModule(M, &errs())) {
    errs() << ": error: linked module is broken!\n";
    return;
  }

  M.print(Out.os(), nullptr, false);

  // Declare success.
  Out.keep();

  return;
}

int main(int argc, char **argv) {
  sys::PrintStackTraceOnErrorSignal(argv[0]);
  llvm::PrettyStackTraceProgram X(argc, argv);

  PlangOption Option;

  if (!ParseArgs(argc, argv, Option))
    return 0;

  LLVMContext Context;
  llvm::Module MainModule("module", Context);
  Linker Link(MainModule);

  BuilderAction Builder(Context);
  FixedCompilationDatabase Compilations(".", Option.clangArgs);

  for (string InputFile : Option.InputFilenames) {
    ClangTool Tool(Compilations, ArrayRef<string>(InputFile));
    Tool.run(&Builder);
    Link.linkInModule(std::move(Builder.Module));
  }

  vector<Function *> Interfaces;
  MPCPass(MainModule, Interfaces);

  for (string ProtobufCC : Option.ProtobufCC) {
    ClangTool Tool(Compilations, ArrayRef<string>(ProtobufCC));
    Tool.run(&Builder);
    Link.linkInModule(std::move(Builder.Module));
  }

  if(!Option.OutputFilename.empty())
    Output(MainModule, Option.OutputFilename);

  GenCode(MainModule, Interfaces, Option);

  return 0;
}
