#include "llvm/IR/Function.h"
#include "llvm/Support/MD5.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Path.h"

#include "demangle/Demangle.h"
#include "gencode.h"

using namespace std;
using namespace llvm;

string getMD5(StringRef input) {

  MD5::MD5Result Result;
  MD5 H;
  H.update(input);
  H.final(Result);

  SmallString<32> output;

  MD5::stringifyResult(Result, output);

  string ret = output.str();

  return ret;
}

StringRef getSimpleName(StringRef SymbolName) {
  int Status;
  StringRef DemangledSymbol;

  if (SymbolName.startswith("_Z"))
    DemangledSymbol =
        itaniumDemangle(SymbolName.data(), nullptr, nullptr, &Status);
  else if (SymbolName.startswith("\01?"))
    DemangledSymbol =
        microsoftDemangle(SymbolName.data() + 1, nullptr, nullptr, &Status);
  else
    Status = -1;

  if (Status != 0 || DemangledSymbol.empty())
    return SymbolName;
  else {
    StringRef Name = DemangledSymbol.split('(').first;

    size_t len0 = Name.rfind(' ') + 1;

    Name = Name.drop_front(len0);

    size_t len1 = Name.rfind(':') + 1;

    Name = Name.drop_front(len1);

    return Name;

  }
}

string TypePrint(Type *T) {
  string str;
  raw_string_ostream SOS(str);
  T->print(SOS);
  SOS.flush();
  return str;
}

void GenJavaCode(std::string &bc, vector<Function *> &Interfaces,
                 vector<string> protobufJava, string JavaCodeDir,
                 string InputFilename) {

  IR ir;
  ir.name = llvm::sys::path::filename(InputFilename);
  ir.hash = getMD5(bc);
  //ir.pb_path = protobufJava[0];
  ir.java_files = protobufJava;
  for (Function *F : Interfaces) {
    IR::func func;

    func.name = getSimpleName(F->getName());
    func.hash_name = getMD5(func.name);

    func.rett = TypePrint(F->getReturnType());

    for (Argument &A : F->args()) {
      if (A.hasStructRetAttr())
        func.rett = TypePrint(A.getType());
      else
        func.argtypes.push_back(TypePrint(A.getType()));
    }

    ir.funcs.push_back(func);
  }

  gen_all(ir, JavaCodeDir);
}
