set(LLVM_LINK_COMPONENTS
  CodeGen
  Core
  ExecutionEngine
  IRReader
  Interpreter
  MC
  MCJIT
  Object
  OrcJIT
  RuntimeDyld
  SelectionDAG
  Support
  Target
  TransformUtils
  native
  )

if( LLVM_USE_OPROFILE )
  set(LLVM_LINK_COMPONENTS
    ${LLVM_LINK_COMPONENTS}
    OProfileJIT
    )
endif( LLVM_USE_OPROFILE )

if( LLVM_USE_INTEL_JITEVENTS )
  set(LLVM_LINK_COMPONENTS
    ${LLVM_LINK_COMPONENTS}
    DebugInfoDWARF
    IntelJITEvents
    Object
    )
endif( LLVM_USE_INTEL_JITEVENTS )

add_llvm_library(mpc-jit SHARED
  OrcLazyJIT.cpp
  jit.cpp
  jit.def

  DEPENDS
  intrinsics_gen
  )

link_libraries(PlatON-jit ${LLVM_LINK_COMPONENTS})
