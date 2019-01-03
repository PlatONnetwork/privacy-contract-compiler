//===--------------------------------------------------------===//
//
//                     The PlatON Lazy JIT Compiler
//
// This file is jit compiler head file, the following function is exported
// form the PlatON-jit.so library.
// Copyright: PlatON.inc 2018
// Version:V1.0.1
//===---------------------------------------------------------===//
#include <string>
#include <vector>


extern "C"
{
    /*--------------------------------
    Function:init PlatON jit, load the jit plugin to jit space address;
    Param:@vectPluginName: plugin full file name(.so);
    Return:true=ok, flase=fail;
    ----------------------------------*/
	bool PlatON_InitJIT(const std::vector<std::string> &vectPluginName);
    
    /*--------------------------------
    Function:run the specified function by jit
    Param:@pModuleName:IR Module full file name
    Param:@pFuncName:function name
    Param:@pRetBuf:return value when the function ended
    Param:@argc:the pFuncName function argument count
    Param:@argv:the pFuncName function argument context
    Return:pRetBuf data lenght.
    Attention:you will delete the pRetBuf memory when don't need it
    ----------------------------------*/
	int PlatON_RunFuncByLazyJIT(const char *pModuleName, const char *pFuncName,
											char **pRetBuf, char **argv, unsigned *unit_arg_len);

    /*--------------------------------
    Function:clear the module all object cache
    Param:@pModuleName:IR Module full file name
    Return:true=ok, flase=fail;
    Attention:you mush call the PlatON_ClearJITCache to clear 
    object cache when the ir module file has be changed
    ----------------------------------*/
	bool PlatON_ClearJITCache(const char *pModuleName);
}

