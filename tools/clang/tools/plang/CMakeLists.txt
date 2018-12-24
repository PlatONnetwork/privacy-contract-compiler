set(LLVM_LINK_COMPONENTS

  BitReader
  BitWriter
  Core
  IRReader
  Linker
  Instrumentation
  Option
  Support
  )

add_subdirectory(demangle)

add_clang_executable(plang
  plang.cpp
  parseArgs.cpp
  AddProtobufEntry.cpp
  InitType.cpp
  PassIR.cpp
  gencode.cpp
  GenJavaCode.cpp
  GenMPCC.cpp
  )


target_link_libraries(plang
  PRIVATE
  clangCodeGen
  clangTooling
  clangFrontend
  clangParse
  clangSema
  clangDriver
  clangFormat
  clangToolingCore
  clangAST
  clangBasic

  demangle
  )
 
