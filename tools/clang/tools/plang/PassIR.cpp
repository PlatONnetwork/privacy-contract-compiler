#include <map>

#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalObject.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Scalar.h"

#include "clang/AST/Decl.h"

#include "Option.h"

using namespace llvm;
using namespace std;

ModulePass *createAddProtobufEntryPass(vector<Function *> &);

void GenJavaCode(std::string &, vector<Function *> &, vector<string>, string,
                 string);

void GenMPCC(std::string &, vector<Function *> &, string, string);

void GenCode(llvm::Module &M, vector<Function *> &Interfaces,
             PlangOption &PlangOpts) {

  string bc;
  raw_string_ostream OS(bc);
  WriteBitcodeToFile(&M, OS, false);
  OS.flush();

  GenMPCC(bc, Interfaces, PlangOpts.MPCC, PlangOpts.Config);

  GenJavaCode(bc, Interfaces, PlangOpts.ProtobufJava, PlangOpts.JavaCode,
              PlangOpts.InputFilenames[0]);

}

void MPCPass(llvm::Module &M, vector<Function *> &Interfaces) {

  legacy::PassManager Passes;

  Passes.add(createAddProtobufEntryPass(Interfaces));

  Passes.run(M);
}
