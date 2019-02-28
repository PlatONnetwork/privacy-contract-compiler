#include <map>
#include <set>
#include<algorithm>

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

set<StructType*> ProtobufStructs;
map<Type *, Constant *> CtorMap, DtorMap, SizeMap;

StringRef microsoftParseFunctionsName[6] = {
    "ReadVarBool",
    "ReadVarUint32",
    "ReadVarUint64",
    "ReadVarFloat",
    "ReadVarDouble",
    "ReadVarString"};

StringRef microsoftSerialFunctionsName[6] = {
    "WriteVarBool",
    "WriteVarUint32",
    "WriteVarUint64",
    "WriteVarFloat",
    "WriteVarDouble",
    "WriteVarString"};

StringRef itaniumParseFunctionsName[6] = {
    "ReadVarBool",
    "ReadVarUint32",
    "ReadVarUint64",
    "ReadVarFloat",
    "ReadVarDouble",
    "ReadVarString"};

StringRef itaniumSerialFunctionsName[6] = {
    "WriteVarBool",
    "WriteVarUint32",
    "WriteVarUint64",
    "WriteVarFloat",
    "WriteVarDouble",
    "WriteVarString"};

Function *getMessageLiteFunction(Module &M, StringRef Name) {
  vector<Type *> ParamsType;
  ParamsType.push_back(
      PointerType::getUnqual(Types[MESSAGE]));
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

std::string getWindowsMangleName(StringRef className) {
  assert(className.startswith("class.") &&
         "class name is not startswith 'class.'");

  StringRef name = className.drop_front(6);

  SmallVector<StringRef, 5> parts;
  name.split(parts, "::");

  std::reverse(parts.begin(), parts.end());

  std::string result;

  for (StringRef part : parts)
    result = result + part.str() + '@';

  return result;
}

StructType *findMessageLiteType(Module &M) {
  FunctionType *FT = FunctionType::get(i32, ArrayRef<Type *>(), true);
  PointerType *PPFT = PointerType::getUnqual(PointerType::getUnqual(FT));

  for (StructType *ST : M.getIdentifiedStructTypes()) {
    if (ST->getNumElements() == 1 &&
        ST->getStructElementType(0) == PPFT)
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

bool isProtobufStruct(Type *T) {
  bool succ = true;

  succ = succ && T->isStructTy();
  succ = succ && T->getStructNumElements() > 0;
  if(!succ)return false;
  Type* ST0 = T->getStructElementType(0);
  if(ST0 == Types[MESSAGE])return true;

  succ = succ && ST0->isStructTy();
  succ = succ && ST0->getStructNumElements() > 0;
  if(!succ)return false;
  Type* ST00 = ST0->getStructElementType(0);
  if(ST00 == Types[MESSAGE])return true;

  return false;
}

void InitCtorDtorSize(Module &M, bool isWindows, Triple::ArchType Arch){
  for (StructType *ST : ProtobufStructs){

      PointerType *PT = PointerType::getUnqual(ST);
      FunctionType *FT = FunctionType::get(Type::getVoidTy(M.getContext()),
                                           ArrayRef<Type *>(PT), false);
      FunctionType *SizeFT =
          FunctionType::get(i64, ArrayRef<Type *>(PT), false);


      if(!isWindows) {
        std::string mangle = getMangleName(ST->getName());

        CtorMap[PT] = M.getOrInsertFunction("_ZN" + mangle + "C1Ev", FT);
        DtorMap[PT] = M.getOrInsertFunction("_ZN" + mangle + "D1Ev", FT);
        SizeMap[PT] =
            M.getOrInsertFunction("_ZNK" + mangle + "12ByteSizeLongEv", SizeFT);
      } else if(Arch == Triple::x86_64){
        std::string mangle = getWindowsMangleName(ST->getName());

        
        CtorMap[PT] = M.getOrInsertFunction("\01??0" + mangle + "@QEAA@XZ", FT);
        DtorMap[PT] = M.getOrInsertFunction("\01??1" + mangle + "@UEAA@XZ", FT);  ////////
        SizeMap[PT] = M.getOrInsertFunction(
            "\01?ByteSizeLong@" + mangle + "@UEBA_KXZ", SizeFT);
      } else {
        std::string mangle = getWindowsMangleName(ST->getName());

        CtorMap[PT] = M.getOrInsertFunction("\01??0" + mangle + "@QAE@XZ", FT);
        DtorMap[PT] = M.getOrInsertFunction("\01??1" + mangle + "@UAE@XZ", FT);
        SizeMap[PT] = M.getOrInsertFunction(
            "\01?ByteSizeLong@" + mangle + "@UBEIXZ", SizeFT);
      }


  }
}

void InitMessageType(Module &M, bool isWindows, Triple::ArchType Arch){

  Types[MESSAGE] = findMessageLiteType(M);

  if (!Types[MESSAGE])
    return;

  if(!isWindows){
    ParseFunctions[MESSAGE] = getMessageLiteFunction(
        M, "_ZN6google8protobuf11MessageLite14ParseFromArrayEPKvi");
    SerialFunctions[MESSAGE] = getMessageLiteFunction(
        M, "_ZNK6google8protobuf11MessageLite16SerializeToArrayEPvi");
  } else if(Arch == Triple::x86_64){

    ParseFunctions[MESSAGE] = getMessageLiteFunction(
        M, "\01?ParseFromArray@MessageLite@protobuf@google@@QEAA_NPEBXH@Z");
    SerialFunctions[MESSAGE] = getMessageLiteFunction(
        M, "\01?SerializeToArray@MessageLite@protobuf@google@@QEBA_NPEAXH@Z");
  } else {

    ParseFunctions[MESSAGE] = getMessageLiteFunction(
        M, "\01?ParseFromArray@MessageLite@protobuf@google@@QAE_NPBXH@Z");
    SerialFunctions[MESSAGE] = getMessageLiteFunction(
        M, "\01?SerializeToArray@MessageLite@protobuf@google@@QBE_NPAXH@Z");
  }

  for (StructType *ST : M.getIdentifiedStructTypes())
    if (isProtobufStruct(ST)) 
      ProtobufStructs.insert(ST);

  InitCtorDtorSize(M, isWindows, Arch);
}

void InitStringType(Module &M, bool isWindows, Triple::ArchType Arch){
  Types[STRING] = findStringType(M);

  if (!Types[STRING])
    return;

  stringTypeStar = PointerType::getUnqual(Types[STRING]);

  FunctionType *FT = FunctionType::get(Type::getVoidTy(M.getContext()),
                                       ArrayRef<Type *>(stringTypeStar), false);


  if(!isWindows){
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
  } else if(Arch == Triple::x86_64){

    ParseFunctions[STRING] = getReadWriteTypesFunction(
        M, microsoftParseFunctionsName[STRING], stringTypeStar);

    SerialFunctions[STRING] = getReadWriteTypesFunction(
        M, microsoftSerialFunctionsName[STRING], stringTypeStar);

    CtorMap[stringTypeStar] =
        M.getOrInsertFunction("\01??0?$basic_string@DU?$char_traits@D@std@@V?$"
                              "allocator@D@2@@std@@QEAA@XZ",
                              FT);
    DtorMap[stringTypeStar] =
        M.getOrInsertFunction("\01??1?$basic_string@DU?$char_traits@D@std@@V?$"
                              "allocator@D@2@@std@@QEAA@XZ",
                              FT);
    SizeMap[stringTypeStar] = M.getOrInsertFunction(
        "\01?length@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@"
        "std@@QEBA_KXZ",
        FunctionType::get(Types[INT64], ArrayRef<Type *>(stringTypeStar),
                          false));
  
  } else {

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

  InitStringType(M, isWindows, Trip.getArch());

  InitMessageType(M, isWindows, Trip.getArch());

  MallocFunction = M.getOrInsertFunction(
      "malloc", FunctionType::get(i8star, ArrayRef<Type *>(i64), false));

  return true;
}
