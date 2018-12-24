set(LLVM_LINK_COMPONENTS
 )


#ADD_LIBRARY(demangle
add_library(demangle
  ItaniumDemangle.cpp  
  MicrosoftDemangle.cpp  
  MicrosoftDemangleNodes.cpp
  )
