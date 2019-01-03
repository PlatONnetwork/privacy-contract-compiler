//===- jit.cpp - LLVM Interpreter / Dynamic compiler ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//


#include "../lli/OrcLazyJIT.h"
#include "../lli/RemoteJITUtils.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Bitcode/BitcodeReader.h"
//#include "llvm/CodeGen/CommandFlags.def"  //Marked by cyf 2018-10-11
#include "llvm/CodeGen/LinkAllCodegenComponents.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/Interpreter.h"
#include "llvm/ExecutionEngine/JITEventListener.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/ExecutionEngine/ObjectCache.h"
#include "llvm/ExecutionEngine/Orc/OrcRemoteTargetClient.h"
#include "llvm/ExecutionEngine/OrcMCJITReplacement.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/TypeBuilder.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Object/Archive.h"
#include "llvm/Object/ObjectFile.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/Memory.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
//#include "llvm/Support/PluginLoader.h"  //Marked by cyf 2018-10-11
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Instrumentation.h"
#include <cerrno>
#include <stdio.h>
#include "jit.h"

#ifdef __CYGWIN__
#include <cygwin/version.h>
#if defined(CYGWIN_VERSION_DLL_MAJOR) && CYGWIN_VERSION_DLL_MAJOR < 1007
#define DO_NOTHING_ATEXIT 1
#endif
#endif

using namespace llvm;


extern "C" 
{
  //init jit module
  bool PlatON_InitJIT(const std::vector<std::string> &vectPluginName) {
    std::string Error;
    for (auto Plugin : vectPluginName) {
      if (sys::DynamicLibrary::LoadLibraryPermanently(Plugin.c_str(), &Error)) {
        errs() << "Error opening '" << Plugin.c_str() << "' => " << Error
              << "\n  load plugin request ignored.\n";
        return false;
      }
    }
    return true;
  }

  //Run user func by name, default cache is enable.
  int PlatON_RunFuncByLazyJIT(const char *pModuleName, const char *pFuncName,
                              char **pRetBuf, char **argv, unsigned *unit_arg_len) {
  assert(pModuleName && pFuncName);
  printf("Begin to run %s.\n", pFuncName);

  atexit(llvm_shutdown); // Call llvm_shutdown() on exit.

  // If we have a native target, initialize it to ensure it is linked in and
  // usable by the JIT.
  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter();
  InitializeNativeTargetAsmParser();

  LLVMContext Context;

  //Load the bitcode...
  SMDiagnostic Err;
  std::vector<std::unique_ptr<Module>> Ms;
  std::unique_ptr<Module> Owner = parseIRFile(pModuleName, Err, Context);
  Module *Mod = Owner.get();
  if (!Mod) {
    errs() << "parser ir module file: " << pModuleName << " fail.\n";
    return -1;
  }

  //Make parameters...
  Ms.push_back(std::move(Owner));
  std::vector<std::string> Args;
  Args.push_back(pModuleName);

  //Get the mangle function name
  //eg:Test(int, int) => PlatON.entry.Test(protobuf message type);
  std::string strMangleFuncName;
  strMangleFuncName = "PlatON_entry_";
  strMangleFuncName += pFuncName;

  //Run orc-lazy JIT...
  int RetVal = runOrcLazyJIT(std::move(Ms), Args, strMangleFuncName.c_str(), 
                             pRetBuf, argv, unit_arg_len);
  printf("End to run %s.\n", strMangleFuncName.c_str());
  return RetVal;
}

  //Clear ir module cache
  bool PlatON_ClearJITCache(const char *pModuleName) {
    assert(pModuleName);
    std::string strPath = pModuleName;
    std::string::size_type pos = strPath.rfind('/');
    if (pos != std::string::npos)
      strPath = strPath.substr(0, pos + 1);
    strPath += ORC_LAZY_CACHE_FOLDER_NAME;
    if (std::error_code ec = sys::fs::remove_directories(strPath)) {
      errs() << "warning: could not remove directory '"
            << strPath << "': " << ec.message() << '\n';
      return false;
    }

    return true;
  }
}


