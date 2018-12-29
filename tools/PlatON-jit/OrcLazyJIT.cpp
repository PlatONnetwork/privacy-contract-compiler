//===- OrcLazyJIT.cpp - Basic Orc-based JIT for lazy execution ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "../lli/OrcLazyJIT.h"
#include "llvm/ADT/Triple.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/Support/CodeGen.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <system_error>

using namespace llvm;

namespace {


// ------ Begin ------ Added by cyf 2018-10-15 ------
// Reason: Lazy jit cache
//===----------------------------------------------------------------------===//
// Object cache for lazy jits
//
// This object cache implementation writes cached objects to disk to the
// directory specified by CacheDir, using a filename provided in the module
// descriptor. The cache tries to load a saved object using that path if the
// file exists. CacheDir defaults to "", in which case objects are cached
// alongside their originating bitcodes.
//===----------------------------------------------------------------------===//
class LazyJitObjectCache : public ObjectCache {
public:
  LazyJitObjectCache(const std::string &CacheDir) : CacheDir(CacheDir) {
    // Add trailing '/' to cache dir if necessary.
    if (!this->CacheDir.empty() &&
        this->CacheDir[this->CacheDir.size() - 1] != '/')
      this->CacheDir += '/';
  }
  ~LazyJitObjectCache() override {}

  void notifyObjectCompiled(const Module *M, MemoryBufferRef Obj) override {
    const std::string &ModuleID = M->getModuleIdentifier();
    std::string CacheName;
    if (!getCacheFilename(ModuleID, CacheName))
      return;
    if (!CacheDir.empty()) { // Create user-defined cache dir.
      SmallString<128> dir(sys::path::parent_path(CacheName));
      sys::fs::create_directories(Twine(dir));
    }
    std::error_code EC;
    raw_fd_ostream outfile(CacheName, EC, sys::fs::F_None);
	if (EC) return;
    outfile.write(Obj.getBufferStart(), Obj.getBufferSize());
    outfile.close();
  }

  std::unique_ptr<MemoryBuffer> getObject(const Module *M) override {
    const std::string &ModuleID = M->getModuleIdentifier();
    std::string CacheName;
    if (!getCacheFilename(ModuleID, CacheName))
      return nullptr;
    // Load the object from the cache filename
    ErrorOr<std::unique_ptr<MemoryBuffer>> IRObjectBuffer =
        MemoryBuffer::getFile(CacheName, -1, false);
    // If the file isn't there, that's OK.
    if (!IRObjectBuffer)
      return nullptr;
    // MCJIT will want to write into this buffer, and we don't want that
    // because the file has probably just been mmapped.  Instead we make
    // a copy.  The filed-based buffer will be released when it goes
    // out of scope.
    return MemoryBuffer::getMemBufferCopy(IRObjectBuffer.get()->getBuffer());
  }

private:
  std::string CacheDir;

  bool getCacheFilename(const std::string &ModID, std::string &CacheName) {
    CacheName = ModID;
    replacePathSpecialCharWith('\\', "/", CacheName);
    std::string::size_type pos = CacheName.rfind("/");
    if (pos != std::string::npos) {
      std::string strPath = CacheName.substr(0, pos + 1);
      std::string strName = CacheName.substr(pos + 1);
      strPath += ORC_LAZY_CACHE_FOLDER_NAME;
      if (std::error_code ec = sys::fs::create_directories(Twine(strPath)))
        errs() << "warning: could not create directory '"
              << strPath << "': " << ec.message() << '\n';
      CacheName = strPath + strName;
	}
    replacePathSpecialCharWith('?', "-", CacheName);
    replacePathSpecialCharWith('\x1', "-", CacheName);
    CacheName += ".o";
    return true;
  }

  void replacePathSpecialCharWith(const char oldCh, const std::string &newStr, std::string &CacheName) {
    std::string::size_type pos = 0;
    pos = CacheName.find(oldCh, pos);
    while (pos != std::string::npos) {
      CacheName.replace(pos, 1, newStr);
      pos = CacheName.find(oldCh, (pos + 1));
    }
  }
};
// ------  End  ------ Added by cyf 2018-10-15 ------

enum class DumpKind {
  NoDump,
  DumpFuncsToStdOut,
  DumpModsToStdOut,
  DumpModsToDisk
};

} // end anonymous namespace

//static cl::opt<DumpKind> OrcDumpKind(
//    "orc-lazy-debug", cl::desc("Debug dumping for the orc-lazy JIT."),
//    cl::init(DumpKind::DumpFuncsToStdOut), // Modified by cyf 2018-09-27
//    cl::values(clEnumValN(DumpKind::NoDump, "no-dump", "Don't dump anything."),
//               clEnumValN(DumpKind::DumpFuncsToStdOut, "funcs-to-stdout",
//                          "Dump function names to stdout."),
//               clEnumValN(DumpKind::DumpModsToStdOut, "mods-to-stdout",
//                          "Dump modules to stdout."),
//               clEnumValN(DumpKind::DumpModsToDisk, "mods-to-disk",
//                          "Dump modules to the current "
//                          "working directory. (WARNING: "
//                          "will overwrite existing files).")),
//    cl::Hidden);
//
//static cl::opt<bool> OrcInlineStubs("orc-lazy-inline-stubs",
//                                    cl::desc("Try to inline stubs"),
//                                    cl::init(true), cl::Hidden);

OrcLazyJIT::TransformFtor OrcLazyJIT::createDebugDumper() {
 // switch (OrcDumpKind) {
 // case DumpKind::NoDump:
 //   return [](std::shared_ptr<Module> M) { return M; };

 // case DumpKind::DumpFuncsToStdOut:
    return [](std::shared_ptr<Module> M) {
      //printf("[ ");

      //for (const auto &F : *M) {
      //  if (F.isDeclaration())
      //    continue;

      //  if (F.hasName()) {
      //    std::string Name(F.getName());
      //    printf("%s ", Name.c_str());
      //  } else
      //    printf("<anon> ");
      //}

      //printf("]\n");
      return M;
    };

  //case DumpKind::DumpModsToStdOut:
  //  return [](std::shared_ptr<Module> M) {
  //           outs() << "----- Module Start -----\n" << *M
  //                  << "----- Module End -----\n";

  //           return M;
  //         };

  //case DumpKind::DumpModsToDisk:
  //  return [](std::shared_ptr<Module> M) {
  //           std::error_code EC;
  //           raw_fd_ostream Out(M->getModuleIdentifier() + ".ll", EC,
  //                              sys::fs::F_Text);
  //           if (EC) {
  //             errs() << "Couldn't open " << M->getModuleIdentifier()
  //                    << " for dumping.\nError:" << EC.message() << "\n";
  //             exit(1);
  //           }
  //           Out << *M;
  //           return M;
  //         };
  //}
  //llvm_unreachable("Unknown DumpKind");
}

// Defined in jit.cpp.
//CodeGenOpt::Level getOptLevel();

template <typename PtrTy>
static PtrTy fromTargetAddress(JITTargetAddress Addr) {
  return reinterpret_cast<PtrTy>(static_cast<uintptr_t>(Addr));
}

int llvm::runOrcLazyJIT(std::vector<std::unique_ptr<Module>> Ms,
                        const std::vector<std::string> &Args,
                        const char *pMainFuncName /*= nullptr*/,
                        char **pRetBuf /*= nullptr*/, 
                        char **pArgV   /*= nullptr*/,
                        unsigned *pUnit_arg_len /*= nullptr*/ ) {  //Modified by cyf 2018-10-19(Added param)
  // Add the program's symbols into the JIT's search space.
  if (sys::DynamicLibrary::LoadLibraryPermanently(nullptr)) {
    errs() << "Error loading program symbols.\n";
    return -1;
  }

  // Grab a target machine and try to build a factory function for the
  // target-specific Orc callback manager.
  EngineBuilder EB;
  EB.setOptLevel(CodeGenOpt::Level::Default);
  auto TM = std::unique_ptr<TargetMachine>(EB.selectTarget());
  Triple T(TM->getTargetTriple());
  auto CompileCallbackMgr = orc::createLocalCompileCallbackManager(T, 0);

  // If we couldn't build the factory function then there must not be a callback
  // manager for this target. Bail out.
  if (!CompileCallbackMgr) {
    errs() << "No callback manager available for target '"
           << TM->getTargetTriple().str() << "'.\n";
    return -1;
  }

  auto IndirectStubsMgrBuilder = orc::createLocalIndirectStubsManagerBuilder(T);

  // If we couldn't build a stubs-manager-builder for this target then bail out.
  if (!IndirectStubsMgrBuilder) {
    errs() << "No indirect stubs manager available for target '"
           << TM->getTargetTriple().str() << "'.\n";
    return -1;
  }

  // ------ Begin ------ Added by cyf 2018-10-15 ------
  // Reason:add cache for lazy jit
  std::unique_ptr<LazyJitObjectCache> LazyJitCacheMgr;
  LazyJitCacheMgr.reset(new LazyJitObjectCache(""));
  // ------  End  ------ Added by cyf 2018-10-15 ------

  // Everything looks good. Build the JIT.
  OrcLazyJIT J(std::move(TM), std::move(CompileCallbackMgr),
               std::move(IndirectStubsMgrBuilder),
               true,                   // Marked by cyf 2018-09-27(orc-lazy-inline-stubs)
               LazyJitCacheMgr.get()); // Added by cyf 2018-10-15(for cache)

  // Add the module to jit
  for (auto &M : Ms)
    cantFail(J.addModule(std::shared_ptr<Module>(std::move(M))));

  // look up main and run it.
  // ---------- Begin ------ Modified by cyf 2018-10-25---------
  // Reason:Modified main func prototype, add output paramete. 
  if (auto MainSym = J.findSymbol(pMainFuncName?pMainFuncName:"main")) {
    typedef int (*MainFnPtr)(char**, char**, unsigned*);
    //std::vector<const char *> ArgV;
    //for (auto &Arg : Args)
    //  ArgV.push_back(Arg.c_str());
    auto Main = fromTargetAddress<MainFnPtr>(cantFail(MainSym.getAddress()));
    return Main(pRetBuf, pArgV, pUnit_arg_len);
  } else if (auto Err = MainSym.takeError())
    logAllUnhandledErrors(std::move(Err), llvm::errs(), "");
  else
    errs() << "Could not find " << pMainFuncName << " function.\n";

  return -1;
  // ----------  End  ------ Modified by cyf 2018-10-25---------
}
