#include <map>

#include "llvm/ADT/Triple.h"
#include "llvm/IR/Module.h"

using namespace llvm;
using namespace std;

enum { BOOL, INT32, INT64, FLOAT, DOUBLE, STRING, MESSAGE };

Type *i1, *i8star, *stringTypeStar;

Constant *MallocFunction;

Type *i8, *i32, *i64;
Type *Types[7];
Constant *ParseFunctions[7];
Constant *SerialFunctions[7];

map<Type *, Constant *> CtorMap, DtorMap, SizeMap;

StringRef microsoftParseFunctionsName[6] = {
    "\01?ReadVarBool@mpc@platon@@YAHPBXHAA_N@Z",
    "\01?ReadVarUint32@mpc@platon@@YAHPBXHAAI@Z",
    "\01?ReadVarUint64@mpc@platon@@YAHPBXHAA_K@Z",
    "\01?ReadVarFloat@mpc@platon@@YAHPBXHAAM@Z",
    "\01?ReadVarDouble@mpc@platon@@YAHPBXHAAN@Z",
    "\01?ReadVarString@mpc@platon@@YAHPBXHAAV?$basic_string@DU?$char_traits@D@"
    "std@@V?$allocator@D@2@@std@@@Z"};

StringRef microsoftSerialFunctionsName[6] = {
    "\01?WriteVarBool@mpc@platon@@YAHPAXH_N@Z",
    "\01?WriteVarUint32@mpc@platon@@YAHPAXHI@Z",
    "\01?WriteVarUint64@mpc@platon@@YAHPAXH_K@Z",
    "\01?WriteVarFloat@mpc@platon@@YAHPAXHM@Z",
    "\01?WriteVarDouble@mpc@platon@@YAHPAXHN@Z",
    "\01?WriteVarString@mpc@platon@@YAHPAXHABV?$basic_string@DU?$char_traits@D@"
    "std@@V?$allocator@D@2@@std@@@Z"};

StringRef itaniumParseFunctionsName[6] = {
    "_ZN6platon3mpc11ReadVarBoolEPKviRb",
    "_ZN6platon3mpc13ReadVarUint32EPKviRj",
    "_ZN6platon3mpc13ReadVarUint64EPKviRm",
    "_ZN6platon3mpc12ReadVarFloatEPKviRf",
    "_ZN6platon3mpc13ReadVarDoubleEPKviRd",
    "_ZN6platon3mpc13ReadVarStringEPKviRNSt7__cxx1112basic_stringIcSt11char_"
    "traitsIcESaIcEEE"};

StringRef itaniumSerialFunctionsName[6] = {
    "_ZN6platon3mpc12WriteVarBoolEPvib",
    "_ZN6platon3mpc14WriteVarUint32EPvij",
    "_ZN6platon3mpc14WriteVarUint64EPvim",
    "_ZN6platon3mpc13WriteVarFloatEPvif",
    "_ZN6platon3mpc14WriteVarDoubleEPvid",
    "_ZN6platon3mpc14WriteVarStringEPviRKNSt7__cxx1112basic_stringIcSt11char_"
    "traitsIcESaIcEEE"};

bool isProtobufStruct(Type *);

Function *getMessageLiteFunction(Module &M, StringRef Name) {
  vector<Type *> ParamsType;
  ParamsType.push_back(
      PointerType::getUnqual(Types[MESSAGE]->getStructElementType(0)));
  ParamsType.push_back(i8star);
  ParamsType.push_back(i32);

  FunctionType *FT = FunctionType::get(i1, ParamsType, false);

  Constant *F = M.getOrInsertFunction(Name, FT);

  return cast<Function>(F);
}

Constant *getReadWriteTypesFunction(Module &M, StringRef Name, Type *T) {
  vector<Type *> ParamsType;
  ParamsType.push_back(i8star);
  ParamsType.push_back(i32);
  ParamsType.push_back(T);

  FunctionType *FT = FunctionType::get(i32, ParamsType, false);

  return M.getOrInsertFunction(Name, FT);
}

std::string getMangleName(StringRef className) {
  assert(className.startswith("class.") &&
         "class name is not startswith 'class.'");

  StringRef name = className.drop_front(6);

  SmallVector<StringRef, 5> parts;
  name.split(parts, "::");

  std::string result;

  for (StringRef part : parts)
    result = result + to_string(part.size()) + part.str();

  return result;
}

StructType *findMessageType(Module &M) {
  FunctionType *FT = FunctionType::get(i32, ArrayRef<Type *>(), true);
  PointerType *PPFT = PointerType::getUnqual(PointerType::getUnqual(FT));

  for (StructType *ST : M.getIdentifiedStructTypes()) {
    if (ST->getNumElements() == 1)
      if (StructType *Lite = dyn_cast<StructType>(ST->getStructElementType(0)))
        if (Lite->getNumElements() == 1 &&
            Lite->getStructElementType(0) == PPFT)
          return ST;
  }
  return nullptr;
}

StructType *findStringType(Module &M) {
  ArrayType *i8x8 = ArrayType::get(i8, 8);

  for (StructType *ST : M.getIdentifiedStructTypes()) {
    if (ST->getNumElements() == 3)
      if (ST->getElementType(1) == i64)
        if (StructType *Base =
                dyn_cast<StructType>(ST->getStructElementType(0)))
          if (Base->getNumElements() == 1 && Base->getElementType(0) == i8star)
            if (StructType *anon =
                    dyn_cast<StructType>(ST->getStructElementType(2)))
              if (anon->getNumElements() == 2 &&
                  anon->getElementType(0) == i64 &&
                  anon->getElementType(1) == i8x8)
                return ST;
  }
  return nullptr;
}

void InitCtorDtorSize(Module &M, bool isWindows) {
  for (StructType *ST : M.getIdentifiedStructTypes()) {

    if (isProtobufStruct(ST)) {
      PointerType *PT = PointerType::getUnqual(ST);
      FunctionType *FT = FunctionType::get(Type::getVoidTy(M.getContext()),
                                           ArrayRef<Type *>(PT), false);
      FunctionType *SizeFT =
          FunctionType::get(i64, ArrayRef<Type *>(PT), false);

      std::string mangle = getMangleName(ST->getName());

      if (isWindows) {
        CtorMap[PT] = M.getOrInsertFunction("\01??0" + mangle + "@@QAE@XZ", FT);
        DtorMap[PT] = M.getOrInsertFunction("\01??1" + mangle + "@@UAE@XZ", FT);
        SizeMap[PT] = M.getOrInsertFunction(
            "\01?ByteSizeLong@" + mangle + "@@UBEIXZ", SizeFT);
      } else {
        CtorMap[PT] = M.getOrInsertFunction("_ZN" + mangle + "C1Ev", FT);
        DtorMap[PT] = M.getOrInsertFunction("_ZN" + mangle + "D1Ev", FT);
        SizeMap[PT] =
            M.getOrInsertFunction("_ZNK" + mangle + "12ByteSizeLongEv", SizeFT);
      }
    }
  }
}

void InitMessageType(Module &M, bool isWindows) {

  Types[MESSAGE] = findMessageType(M);

  if (!Types[MESSAGE])
    return;
  if (isWindows) {
    ParseFunctions[MESSAGE] = getMessageLiteFunction(
        M, "\01?ParseFromArray@MessageLite@protobuf@google@@QAE_NPBXH@Z");
    SerialFunctions[MESSAGE] = getMessageLiteFunction(
        M, "\01?SerializeToArray@MessageLite@protobuf@google@@QBE_NPAXH@Z");
  } else {
    ParseFunctions[MESSAGE] = getMessageLiteFunction(
        M, "_ZN6google8protobuf11MessageLite14ParseFromArrayEPKvi");
    SerialFunctions[MESSAGE] = getMessageLiteFunction(
        M, "_ZNK6google8protobuf11MessageLite16SerializeToArrayEPvi");
  }

  InitCtorDtorSize(M, isWindows);
}

void InitStringType(Module &M, bool isWindows) {
  Types[STRING] = findStringType(M);

  if (!Types[STRING])
    return;

  stringTypeStar = PointerType::getUnqual(Types[STRING]);

  FunctionType *FT = FunctionType::get(Type::getVoidTy(M.getContext()),
                                       ArrayRef<Type *>(stringTypeStar), false);

  if (isWindows) {

    ParseFunctions[STRING] = getReadWriteTypesFunction(
        M, microsoftParseFunctionsName[STRING], stringTypeStar);

    SerialFunctions[STRING] = getReadWriteTypesFunction(
        M, microsoftSerialFunctionsName[STRING], stringTypeStar);

    CtorMap[stringTypeStar] =
        M.getOrInsertFunction("\01??0?$basic_string@DU?$char_traits@D@std@@V?$"
                              "allocator@D@2@@std@@QAE@XZ",
                              FT);
    DtorMap[stringTypeStar] =
        M.getOrInsertFunction("\01??1?$basic_string@DU?$char_traits@D@std@@V?$"
                              "allocator@D@2@@std@@QAE@XZ",
                              FT);
    SizeMap[stringTypeStar] = M.getOrInsertFunction(
        "\01?length@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@"
        "std@@QBEIXZ",
        FunctionType::get(Types[INT64], ArrayRef<Type *>(stringTypeStar),
                          false));
  } else {

    ParseFunctions[STRING] = getReadWriteTypesFunction(
        M, itaniumParseFunctionsName[STRING], stringTypeStar);

    SerialFunctions[STRING] = getReadWriteTypesFunction(
        M, itaniumSerialFunctionsName[STRING], stringTypeStar);

    CtorMap[stringTypeStar] = M.getOrInsertFunction(
        "_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEC1Ev", FT);
    DtorMap[stringTypeStar] = M.getOrInsertFunction(
        "_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEED1Ev", FT);
    SizeMap[stringTypeStar] = M.getOrInsertFunction(
        "_ZNKSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE6lengthEv",
        FunctionType::get(Types[INT64], ArrayRef<Type *>(stringTypeStar),
                          false));
  }
}

void InitTypes(llvm::Module &M, bool isWindows) {
  LLVMContext &Context = M.getContext();
  i1 = Type::getInt1Ty(Context);
  Types[BOOL] = i8 = Type::getInt8Ty(Context);
  Types[INT32] = i32 = Type::getInt32Ty(Context);
  Types[INT64] = i64 = Type::getInt64Ty(Context);
  Types[FLOAT] = Type::getFloatTy(Context);
  Types[DOUBLE] = Type::getDoubleTy(Context);
  i8star = Type::getInt8PtrTy(Context);

  StringRef *ParseFunctionsName =
      isWindows ? microsoftParseFunctionsName : itaniumParseFunctionsName;

  StringRef *SerialFunctionsName =
      isWindows ? microsoftSerialFunctionsName : itaniumSerialFunctionsName;

  ParseFunctions[BOOL] =
      getReadWriteTypesFunction(M, ParseFunctionsName[BOOL], i8star);

  SerialFunctions[BOOL] =
      getReadWriteTypesFunction(M, SerialFunctionsName[BOOL], i1);

  for (int i = INT32; i <= DOUBLE; i++) {
    ParseFunctions[i] = getReadWriteTypesFunction(
        M, ParseFunctionsName[i], PointerType::getUnqual(Types[i]));

    SerialFunctions[i] =
        getReadWriteTypesFunction(M, SerialFunctionsName[i], Types[i]);
  }
}

bool Init(Module &M) {

  Triple Trip(M.getTargetTriple());
  bool isWindows = Trip.isOSWindows();

  InitTypes(M, isWindows);

  InitStringType(M, isWindows);

  InitMessageType(M, isWindows);

  MallocFunction = M.getOrInsertFunction(
      "malloc", FunctionType::get(i8star, ArrayRef<Type *>(i64), false));

  return true;
}
