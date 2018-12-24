
#include <vector>

#include "clang/CodeGen/ModuleBuilder.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Demangle/Demangle.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Scalar.h"

using namespace std;
using namespace llvm;

// ccache g++ -g -shared -fPIC addMain.cpp -o addMain.so `llvm-config
// --cxxflags` -lLLVMDemangle

#define DEBUG_TYPE "maininterfact"
STATISTIC(InterfaceCounter, "Counts number of interface");

enum { BOOL, INT32, INT64, FLOAT, DOUBLE, STRING, MESSAGE };

extern Type *i1, *i8star, *stringTypeStar;

extern Constant *MallocFunction;

extern Type *i8, *i32, *i64;
extern Type *Types[7];
extern Constant *ParseFunctions[7];
extern Constant *SerialFunctions[7];

extern map<Type *, Constant *> CtorMap, DtorMap, SizeMap;

bool Init(Module &M);

StringRef getSimpleName(StringRef);


bool isProtobufStruct(Type *T) {

  if (!T->isStructTy())
    return false;

  if (T->getStructNumElements() == 0)
    return false;

  bool eq = T->getStructElementType(0) == Types[MESSAGE];

  return eq;
}

bool isBaseType(Type *T) { return T->isFloatingPointTy() || T->isIntegerTy(); }

bool isBaseTypeOrProtobufPointer(Type *T) {
  return isBaseType(T) || (T == stringTypeStar) ||
         (T->isPointerTy() && isProtobufStruct(T->getPointerElementType()));
}

bool isIdentName(StringRef Name){
  for(char c : Name){
    if(!isalnum(c) && c!='_')
      return false;
  }
  return isalpha(Name[0]) || Name[0]=='_';
}

bool isInterfact(Function &F) {

  bool Res = true;

  Res &= !F.isDeclaration();

  FunctionType *FT = F.getFunctionType();

  Res &= FT->getNumParams() > (F.hasStructRetAttr() ? 3 : 2);

  Type *RetType =
      F.hasStructRetAttr() ? FT->getParamType(0) : FT->getReturnType();

  Res &= isBaseTypeOrProtobufPointer(RetType);

  for (Type *T : FT->params()) {
    Res &= isBaseTypeOrProtobufPointer(T);
  }

  Res &= isIdentName(getSimpleName(F.getName()));
  return Res;
}

Value *makeParseProtobuf(IRBuilder<> &Builder, Type *PT, Value *Argv,
                         Value *Lengths, unsigned i) {

  AllocaInst *msg =
      Builder.CreateAlloca(PT->getPointerElementType(), 0, nullptr, "msg");

  Builder.CreateCall(CtorMap[PT], ArrayRef<Value *>(msg), "");

  Value *messageLite = Builder.CreateGEP(
      msg, vector<Value *>(3, Builder.getInt32(0)), "messageLite");

  Value *indexGEP =
      Builder.CreateGEP(Argv, ArrayRef<Value *>(Builder.getInt32(i)));
  LoadInst *index = Builder.CreateLoad(indexGEP, "index");

  Value *lenGEP =
      Builder.CreateGEP(Lengths, ArrayRef<Value *>(Builder.getInt32(i)));
  LoadInst *len = Builder.CreateLoad(lenGEP);

  vector<Value *> ParseArgs;
  ParseArgs.push_back(messageLite);
  ParseArgs.push_back(index);
  ParseArgs.push_back(len);

  Builder.CreateCall(ParseFunctions[MESSAGE], ParseArgs);

  return msg;
}

Constant *getParseFunction(Type *T) {
  if (T == i1)
    return ParseFunctions[BOOL];
  else if (T == i32)
    return ParseFunctions[INT32];
  else if (T == i64)
    return ParseFunctions[INT64];
  else if (T == Types[FLOAT])
    return ParseFunctions[FLOAT];
  else if (T == Types[DOUBLE])
    return ParseFunctions[DOUBLE];
  else
    llvm_unreachable("can not handle base type");
}

Value *makeParseBaseValue(IRBuilder<> &Builder, Type *T, Value *Argv,
                          Value *Lengths, unsigned i) {

  Type *TT = (T == i1 ? i8 : T);

  AllocaInst *msg = Builder.CreateAlloca(TT, 0, nullptr, "msg");

  Value *arg = Builder.CreateGEP(Argv, ArrayRef<Value *>(Builder.getInt32(i)));
  Value *load = Builder.CreateLoad(arg);

  Value *lenGEP =
      Builder.CreateGEP(Lengths, ArrayRef<Value *>(Builder.getInt32(i)));
  LoadInst *len = Builder.CreateLoad(lenGEP);

  vector<Value *> ParseArgv;
  ParseArgv.push_back(load);
  ParseArgv.push_back(len);
  ParseArgv.push_back(msg);

  Builder.CreateCall(getParseFunction(T), ParseArgv);

  Value *ret = Builder.CreateLoad(msg);

  if (T != i1)
    return ret;
  else
    return Builder.CreateTrunc(ret, i1);
}

Value *makeParseStringValue(IRBuilder<> &Builder, Type *T, Value *Argv,
                            Value *Lengths, unsigned i) {

  AllocaInst *msg = Builder.CreateAlloca(Types[STRING], 0, nullptr, "msg");

  Builder.CreateCall(CtorMap[stringTypeStar], ArrayRef<Value *>(msg));

  Value *arg = Builder.CreateGEP(Argv, ArrayRef<Value *>(Builder.getInt32(i)));
  Value *load = Builder.CreateLoad(arg);

  Value *lenGEP =
      Builder.CreateGEP(Lengths, ArrayRef<Value *>(Builder.getInt32(i)));
  LoadInst *len = Builder.CreateLoad(lenGEP);

  vector<Value *> ParseArgv;
  ParseArgv.push_back(load);
  ParseArgv.push_back(len);
  ParseArgv.push_back(msg);

  Builder.CreateCall(ParseFunctions[STRING], ParseArgv);

  return msg;
}

Value *makeParseValue(IRBuilder<> &Builder, Type *T, Value *Argv,
                      Value *Lengths, unsigned i) {
  if (isBaseType(T))
    return makeParseBaseValue(Builder, T, Argv, Lengths, i);
  else if (T == stringTypeStar)
    return makeParseStringValue(Builder, T, Argv, Lengths, i);
  else
    return makeParseProtobuf(Builder, T, Argv, Lengths, i);
}

Value *makeSerialProtobuf(IRBuilder<> &Builder, Value *ret, Value *bufstar) {

  Value *gep =
      Builder.CreateGEP(ret, vector<Value *>(3, Builder.getInt32(0)), "");

  CallInst *size = Builder.CreateCall(SizeMap[ret->getType()],
                                      ArrayRef<Value *>(ret), "size");
  Value *trunc = Builder.CreateTrunc(size, Builder.getInt32Ty(), "trunc");

  CallInst *mem = Builder.CreateCall(MallocFunction, size, "mem");

  Builder.CreateStore(mem, bufstar);

  vector<Value *> SerialArgs;
  SerialArgs.push_back(gep);
  SerialArgs.push_back(mem);
  SerialArgs.push_back(trunc);

  Builder.CreateCall(SerialFunctions[MESSAGE], SerialArgs);

  return trunc;
}

Constant *getSerialFunction(Type *T) {
  if (T == i1)
    return SerialFunctions[BOOL];
  else if (T == i32)
    return SerialFunctions[INT32];
  else if (T == i64)
    return SerialFunctions[INT64];
  else if (T == Types[FLOAT])
    return SerialFunctions[FLOAT];
  else if (T == Types[DOUBLE])
    return SerialFunctions[DOUBLE];
  else
    llvm_unreachable("can not handle base type");
}

Value *makeSerialBaseValue(IRBuilder<> &Builder, Value *ret, Value *bufstar) {

  CallInst *mem = Builder.CreateCall(
      MallocFunction, ArrayRef<Value *>(Builder.getInt64(32)), "mem");

  Builder.CreateStore(mem, bufstar);

  vector<Value *> Args;
  Args.push_back(mem);
  Args.push_back(Builder.getInt32(32));
  Args.push_back(ret);

  Builder.CreateCall(getSerialFunction(ret->getType()), Args);

  return Builder.getInt32(32);
}

Value *makeSerialStringValue(IRBuilder<> &Builder, Value *ret, Value *bufstar) {

  Value *Len =
      Builder.CreateCall(SizeMap[stringTypeStar], ArrayRef<Value *>(ret));
  Value *Len1 = Builder.CreateAdd(Len, Builder.getInt64(1));

  CallInst *mem =
      Builder.CreateCall(MallocFunction, ArrayRef<Value *>(Len1), "mem");

  Builder.CreateStore(mem, bufstar);

  vector<Value *> Args;
  Args.push_back(mem);
  Args.push_back(Builder.getInt32(32));
  Args.push_back(ret);

  Builder.CreateCall(SerialFunctions[STRING], Args);

  return Builder.CreateTrunc(Len1, i32);
}

Value *makeSerialValue(IRBuilder<> &Builder, Value *ret, Value *bufstar) {
  Type *T = ret->getType();
  if (isBaseType(T))
    return makeSerialBaseValue(Builder, ret, bufstar);
  else if (T == stringTypeStar)
    return makeSerialStringValue(Builder, ret, bufstar);
  else
    return makeSerialProtobuf(Builder, ret, bufstar);
}


Function *makeEntryFunctionProto(Function *F) {
  vector<Type *> Params;

  PointerType *i8starstar = PointerType::getUnqual(i8star);

  Params.push_back(i8starstar);
  Params.push_back(i8starstar);
  Params.push_back(PointerType::getUnqual(i32));

  FunctionType *MainFT = FunctionType::get(i32, Params, false);

  string MainName("PlatON_entry_" + getSimpleName(F->getName()).str());

  Function *Main =
      cast<Function>(F->getParent()->getOrInsertFunction(MainName, MainFT));

  return Main;
}

Function *makeProtobufEntry(Function *F) {

  //  FunctionType* FT = F->getFunctionType();

  Function *Entry = makeEntryFunctionProto(F);

  Value *buf = Entry->arg_begin();
  Value *argv = Entry->arg_begin() + 1;
  Value *lengths = Entry->arg_begin() + 2;

  BasicBlock *BB = BasicBlock::Create(F->getContext(), "", Entry);

  IRBuilder<> Builder(BB);

  vector<Value *> messages;

  unsigned i = 0;
  for (Argument &Arg : F->args()) {
    Type *PT = Arg.getType();
    Value *msg;

    if (Arg.hasStructRetAttr())
      msg =
          Builder.CreateAlloca(PT->getPointerElementType(), 0, nullptr, "ret");
    else {
      msg = makeParseValue(Builder, PT, argv, lengths, i);
      i++;
    }

    messages.push_back(msg);
  }

  CallInst *Freturn = Builder.CreateCall(F, messages);

  Value *ret = F->hasStructRetAttr() ? messages[0] : Freturn;

  Value *size = makeSerialValue(Builder, ret, buf);

  for (Value *msg : messages) {
    if (msg->getType()->isPointerTy()) {
      if (msg->getType() == stringTypeStar)
        Builder.CreateCall(DtorMap[stringTypeStar], ArrayRef<Value *>(msg));
      else if (isProtobufStruct(msg->getType()->getPointerElementType()))
        Builder.CreateCall(DtorMap[msg->getType()], ArrayRef<Value *>(msg), "");
    }
  }

  Builder.CreateRet(size);

  return Entry;
}

Function* makeFree(llvm::Module &M){
  LLVMContext &Context = M.getContext();
  FunctionType* FT = FunctionType::get(Type::getVoidTy(Context), ArrayRef<Type*>(Type::getInt8PtrTy(Context)), false);

  Function* PlatON_Free = cast<Function>(M.getOrInsertFunction("PlatON_entry_free", FT));

  BasicBlock* BB = BasicBlock::Create(Context, "", PlatON_Free);

  Instruction* call = CallInst::CreateFree(PlatON_Free->arg_begin(), BB);

  IRBuilder<> Builder(BB);

  Builder.Insert(call);
  
  Builder.CreateRetVoid();

  return PlatON_Free;
}

// void GenMPCC(vector<Function *> &Interfaces, clang::CodeGenerator* CG);

namespace {
struct AddProtobufEntryPass : public ModulePass {
  static char ID; // Pass identification, replacement for typeid
  vector<Function *> *InterfacesRet;

  AddProtobufEntryPass() : ModulePass(ID), InterfacesRet(nullptr) {}
  AddProtobufEntryPass(vector<Function *> *InterfacesRet)
      : ModulePass(ID), InterfacesRet(InterfacesRet) {}

  bool runOnModule(Module &M) override {

    Init(M);

    vector<Function *> Interfaces;

    for (Function &F : M.functions()) {
      if (isInterfact(F))
        Interfaces.push_back(&F);
    }
    for (Function *F : Interfaces) {
      makeProtobufEntry(F);
    }

    makeFree(M);

    for (Function *F : Interfaces) {
      InterfacesRet->push_back(F);
    }

    InterfaceCounter += Interfaces.size();
    return Interfaces.size() > 0;
  }
};
} // namespace

char AddProtobufEntryPass::ID = 0;
static RegisterPass<AddProtobufEntryPass> X("add-protobuf-entry",
                                            "Add Main Pass");

// ModulePass *createAddProtobufEntryPass(clang::CodeGenerator* CG) { return new
// AddProtobufEntryPass(CG); }

ModulePass *createAddProtobufEntryPass() { return new AddProtobufEntryPass(); }
ModulePass *createAddProtobufEntryPass(vector<Function *> &Interfaces) {
  return new AddProtobufEntryPass(&Interfaces);
}
