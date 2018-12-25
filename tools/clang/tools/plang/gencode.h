#ifndef _GENCODE_H
#define _GENCODE_H

#ifdef _WIN32
#  if defined(GENCODE_STATIC)
#    define GENCODE_DLL_API 
#  else
#    if defined(GENCODE_EXPORTS)
#      define GENCODE_DLL_API __declspec(dllexport)
#    else
#      define GENCODE_DLL_API __declspec(dllimport)
#    endif
#  endif
#else
#  define GENCODE_DLL_API 
#endif

#include <string>
#include <vector>

struct IR {
	std::string name;
	std::string hash;
    std::string pb_path; // protobuffer file path
    std::vector<std::string> java_files; // java files
	struct func {
		std::string rett; // return type
		std::string name; // add
		std::string prot; // add(int,int)
		std::vector<std::string> argtypes;
		std::string hash_prot; // md5("add(int,int)") == e523a297ab4dc7fb9d30b72b34b49a47
		std::string hash_name; // md5("add") == 34ec78fcc91ffb1e54cd85e4a0924332
	};
	std::vector<func> funcs;

};
/*
return 0 is success, other is failed
*/
GENCODE_DLL_API
int gen_all(IR& ir, std::string outdir = ".");

#endif //!_GENCODE_H



