#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#define access _access
#define mkdir _mkdir
#else
#include <dirent.h>
#include <fcntl.h>
#include <sys/io.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include "gencode.h"

/// public function
int gen_cs(IR &ir);
int gen_go(IR &ir);
int gen_cpp(IR &ir);
int gen_lua(IR &ir);
int gen_php(IR &ir);
int gen_java(IR &ir);
int gen_java_proxy(IR &ir);
int gen_python(IR &ir);
int gen_nodejs(IR &ir);
///

#define USE_FUNC_PROT_HASH 0
#define USE_FUNC_NAME_HASH 1

///

static string tpl_content =
    "# this, only for java\n# default directory/package is "
    "mpc\n\nPACKAGE_BEG\n/*\n * may be some declaration here.\n */\npackage "
    "net.platon.vm.mpc;\n\nimport net.platon.vm.sdk.client.Data;\nimport "
    "net.platon.vm.sdk.client.ErrorCode;\nimport "
    "net.platon.vm.sdk.client.IIInterface;\nimport "
    "net.platon.vm.sdk.client.InputRequestPara;\nimport "
    "net.platon.vm.sdk.client.MpcCallbackInterface;\n\nimport "
    "java.util.HashMap;\n\nPACKAGE_END\n\nCOMMENT_BEG\n/**\n * Attention! This "
    "file was auto-generated, you just need to implement the \"TODO "
    "SECTIONS\".\n * The class name \"IRNAME\" is just to named this file, you "
    "can rename \"IRNAME\" what you like.\n * More details ref "
    "\"IRNAME-README.TXT\".\n * <p>\n * DIGEST:\n * <p>\nIRDIGEST */\n "
    "COMMENT_END\n\nIR_FUNC_INTERFACE_BEG\n        "
    "put(\"mpc_f_IRFUNCHASH_IRFUNCPOSX\", new "
    "IRNAME_IRFUNCPROTA_IRFUNCPOSX());\nIR_FUNC_INTERFACE_END\n\nABSTRACT_BASE_"
    "BEG\ninterface mpc_ii_IRHASH extends IIInterface {\n}\n\npublic class "
    "IRNAME implements mpc_ii_IRHASH {\n    private HashMap<String, "
    "MpcCallbackInterface> funcInterfaces = new HashMap<String, "
    "MpcCallbackInterface>() {{IR_FUNC_INTERFACES    }};\n\n    public "
    "MpcCallbackInterface getInstance(String instance_hash) {\n        if "
    "(funcInterfaces.containsKey(instance_hash)) {\n            return "
    "funcInterfaces.get(instance_hash);\n        }\n        return null;\n    "
    "}\n\n    public HashMap<String, MpcCallbackInterface> getInstances() {\n  "
    "      return funcInterfaces;\n    }\n\n    abstract class "
    "MpcCallbackBase_IRHASH implements MpcCallbackInterface {\n        public "
    "abstract byte[] inputImpl(final InputRequestPara para);\n        \n       "
    " public byte[] input(final InputRequestPara para) {\n            // TODO: "
    "do what you want to do, before call inputImpl\n            return "
    "inputImpl(para);\n        }\n\n        public void error(final "
    "InputRequestPara para, ErrorCode error) {\n            // TODO: do what "
    "you want to do\n        }\n\n        public void result(final "
    "InputRequestPara para, final byte[] data) {\n            // TODO: do what "
    "you want to do\n        }\n    }\n\n    abstract class mpc_i_IRHASH "
    "extends MpcCallbackBase_IRHASH {\n    }\n    CONTENT_FUNC_ABSTRACT\n    "
    "CONTENT_FUNC_EXTENDS\n}\nABSTRACT_BASE_END\n\nFUNC_ABSTRACT_BEG\n    "
    "abstract class mpc_f_IRFUNCHASH_IRFUNCPOSX extends mpc_i_IRHASH {\n       "
    " public byte[] input_x_default(){\n            return X_DEFAULT_INPUT;\n  "
    "      }\n    }\nFUNC_ABSTRACT_END\n\nFUNC_EXTENDS_BEG\n    /**\n     * "
    "IRFUNCPROT\n     */\n    final class IRNAME_IRFUNCPROTA_IRFUNCPOSX "
    "extends mpc_f_IRFUNCHASH_IRFUNCPOSX {\n        public byte[] "
    "inputImpl(final InputRequestPara para) {\n            "
    "RETTYPE_DATADEFAULTVALUE;\n            // TODO: assemble data\n           "
    " \n            return RETVALUEFUNC;\n        }\n    "
    "}\nFUNC_EXTENDS_END\n\n\nREADME_BEG\n/*\n * may be some declaration "
    "here.\n */\nTHIS FILE IS AUTO-GENERATED!\n\nDIGEST:\n/**\nIRDIGEST "
    "*/\n\nIN GENERAL, YOU JUST NEED TO IMPLEMENT THE \"TODO SECTION\" IN "
    "IRNAME.java.\n\nYou can rename \"IRNAME.java\" to any you like, but do "
    "not change the md5-string in that .java file.\n\nWhen you implement "
    "\"inputImpl\", you can do some work before, log or store the request "
    "info,\nin the MpcCallbackBasexx.\n\ntodo......\n\nREADME_END\n";
static string tpl_proxy_content =
    "# this, only for java\n# default directory/package is "
    "mpc\n\nPACKAGE_BEG\n/*\n * may be some declaration here.\n */\npackage "
    "platon.mpc.proxy;\n\nimport "
    "com.google.protobuf.InvalidProtocolBufferException;\nimport "
    "net.platon.mpc.proxy.sdk.*;\nimport org.web3j.crypto.Credentials;\nimport "
    "org.web3j.protocol.Web3j;\nimport org.web3j.utils.Numeric;\n\nimport "
    "java.util.HashMap;\n\nPACKAGE_END\n\nCOMMENT_BEG\n/**\n * Attention! This "
    "file was auto-generated, DO NOT EDIT!\n * More details ref "
    "\"IRNAME-README.TXT\".\n * <p>\n * DIGEST:\n * <p>\nIRDIGEST "
    "*/\nCOMMENT_END\n\nPROXY_CLASS_BEG\npublic class IRNAME extends "
    "ProxyClient {\n    private String irHash = \"IRHASH\";\n    private "
    "static String sIrHash;\n\n    /**\n     * Enum Method\n     */\n    "
    "public enum Method {\nIR_FUNC_PROT\n    }\n\n    private static "
    "HashMap<Method, MethodInfo> mapMethodInfo = new HashMap<Method, "
    "MethodInfo>() {{METHOD_MAP\n    }};\n\n    public static void "
    "showMethodMap() {\n        System.out.println(\"==== Method Map Beg "
    "====\");\n        for (MethodInfo info : mapMethodInfo.values()) {\n      "
    "      System.out.println(info.name + \"    \" + info.prot + \"    \" + "
    "info.enum_name);\n        }\n        System.out.println(\"==== Method Map "
    "End ====\");\n    }\n    \n    /**\n     * Constructor\n     */\n    "
    "public IRNAME(Web3j web3j, Credentials credentials) {\n        "
    "super(web3j, credentials);\n    }\n\n    public IRNAME(String url, "
    "Credentials credentials) {\n        super(url, credentials);\n    }\n\n   "
    " public IRNAME(String url, String walletPath, String walletPass) {\n      "
    "  super(url, walletPath, walletPass);\n    }\n\n    public IRNAME(Web3j "
    "web3j, String walletPath, String walletPass) {\n        super(web3j, "
    "walletPath, walletPass);\n    }\n\n    /**\n     * Public method\n     "
    "*/\n    public String startCalc(Method method) {\n        return "
    "startCalc(method, 0);\n    }\n\n    public String startCalc(Method "
    "method, int retry) {\n        if (!mapMethodInfo.containsKey(method)) {\n "
    "           // no this method\n            return null;\n        }\n       "
    " return startCalc(mapMethodInfo.get(method).name, retry);\n    }\n\n    "
    "/*\n     * Below getXXX, if have, only for this mpc contract\n     */\n   "
    "  "
    "PROXY_PROTO_RETURN_FUNC_S\n}\nPROXY_CLASS_END\n\nPROXY_PROTO_RETURN_FUNC_"
    "BEG\n    /**\n     * Get RETPKG.RETTYPE\n     */\n    public "
    "RETPKG.RETTYPE getRETTYPE(byte[] val) {\n        RETPKG.RETTYPE __RETTYPE "
    "= null;\n        try {\n            __RETTYPE = "
    "RETPKG.RETTYPE.parseFrom(ethECIES.Decrypt(priHexString, val));\n        } "
    "catch (InvalidProtocolBufferException e) {\n            "
    "e.printStackTrace();\n        }\n        return __RETTYPE;\n    }\n\n    "
    "public RETPKG.RETTYPE getRETTYPE(String cipher) {\n        return "
    "getRETTYPE(Numeric.hexStringToByteArray(cipher));\n    "
    "}\nPROXY_PROTO_RETURN_FUNC_END\n\nMETHOD_MAP_BEG\n        "
    "put(Method.IR_FUNC_PROT, new MethodInfo(\"IRFUNCNAME\", \"IRFUNCPROTO\", "
    "\"IRFUNCHASH\", \"\", "
    "\"IR_FUNC_PROT\"));\nMETHOD_MAP_END\n\nREADME_BEG\nTHIS FILE IS "
    "AUTO-GENERATED! DO NOT EDIT!\n\nDIGEST:\n/**\nIRDIGEST */\n\nREADME_END\n";

static string outputdir;
static string digest;

static map<string, string> mapJavaTypes = {
    {"%\"class.std::__cxx11::basic_string\"*", "String"},
    {"i1", "boolean"},
    {"i8", "int"},
    {"i16", "int"},
    {"i32", "int"},
    {"i64", "long"},
    {"float", "float"},
    {"double", "double"}};

static set<string> java_basic_types = {"int",   "long",   "boolean",
                                       "float", "double", "String"};

bool makedirs(const std::string &dirpath) {
  string path(dirpath);
  path = regex_replace(path, regex("\\\\"), string("/"));

  std::string folder_builder;
  std::string sub;
  sub.reserve(path.size());
  for (auto it = path.begin(); it != path.end(); ++it) {
    const char c = *it;
    sub.push_back(c);
    if (c == '/' || it == path.end() - 1) {
      folder_builder.append(sub);
      if (0 != access(folder_builder.c_str(), 0)) {
#ifdef _WIN32
        if (0 != mkdir(folder_builder.c_str())) {
#else
        if (0 != mkdir(folder_builder.c_str(), 0755)) {
#endif
          return false;
        }
      }
      sub.clear();
    }
  }
  return true;
}

struct pkginfo_st {
  string filename = "";
  string pkgname = "";
  string outclass = "";
  string pkgfullname = ""; // pkgname.outclass
  set<string> classes;     // [Foo, Bar, ...]
};

static string print_digest(const IR &ir) {
  /*
  **	IR NAME: IR1
  **	IR HASH: 1291be25871314f67f4e779f1946ea3b
  **
  **	IR FUNC HASH(MD5)                 IR FUNC NAME  IR FUNC PROT
  **	c34bb37739a6a758afb1c726ee9719c2  add           MPCInteger32
  *add(MPCInteger32,MPCInteger32) *	3e7eacd3b10bfb32bd431f3647f0147f  sub
  *MPCInteger32 sub(MPCInteger32,MPCInteger32)
  **
  */
  int width = strlen("IR FUNC NAME");
  for (auto ir_func : ir.funcs) {
    int len = ir_func.name.length();
    if (len > width)
      width = len;
  }
  width += 2;

  stringstream sss;
  ostream::fmtflags oldflags = sss.flags();
  sss.setf(ios::left);

  sss << " * IR NAME: " << ir.name << endl
      << " * IR HASH: " << ir.hash << endl
      << " * <p>" << endl;
  sss << " * ";
  sss.width(34);
  sss << "IR FUNC HASH(MD5)";
  sss.width(width);
  sss << "IR FUNC NAME"
      << "IR FUNC PROT" << endl;
  for (auto ir_func : ir.funcs) {
    sss << " * ";
    sss.width(34);
#if USE_FUNC_PROT_HASH
    sss << ir_func.hash;
#else
    sss << ir_func.hash_name;
#endif
    sss.width(width);
    sss << ir_func.name << ir_func.prot << endl;
  }
  sss.flags(oldflags);

  return sss.str();
}

static bool have_protobuf = false;
static vector<pkginfo_st> pkgs;

int gen_all(IR &ir, string outdir /* = "./"*/) {
  have_protobuf = false;
  pkgs.clear();

  if (ir.funcs.size() <= 0) {
    cerr << "no function" << endl;
    return -1;
  }
  for (auto jf : ir.java_files) {
    cout << jf << endl;
  }

  outputdir = outdir + string("/code/");
  if (!makedirs(outdir)) {
    std::cerr << "can not make output directory [" << outputdir << "]!" << endl;
    return -1;
  }

  string ir_name = ir.name;
  ir_name = regex_replace(ir_name, regex(".ll$"), string(""));
  ir_name = regex_replace(ir_name, regex(".bc$"), string(""));
  ir_name = regex_replace(ir_name, regex(".cpp$"), string(""));
  ir_name = regex_replace(ir_name, regex("[^a-zA-Z0-9_]"), string("_"));

  ir.name = "MPC" + ir_name;
  digest = print_digest(ir);
#ifdef _DEBUG
  cout << "debug digest:\n" << digest << endl;
#endif
  if (gen_java(ir) != 0) {
    cerr << "gen java failed" << endl;
    return -1;
  }
  cout << "digest:\n" << digest << endl;

  ir.name = "Proxy" + ir_name;
  // digest = print_digest(ir);
  if (gen_java_proxy(ir) != 0) {
    cerr << "gen java proxy failed" << endl;
    return -1;
  }

  return 0;
}

static string gp_package;
static string gp_comment;
static string gp_ir_func_interface;
static string gp_abstract_base;
static string gp_func_abstract;
static string gp_func_extends;
static string gp_readme;

static int getTplFile() {
  match_results<std::string::iterator> results;
#define gen_pattern(X) string(#X "_BEG([\\s\\S]*?)" #X "_END")
#define get_content(v, X)                                                      \
  do {                                                                         \
    if (regex_search(tpl_content.begin(), tpl_content.end(), results,          \
                     regex(gen_pattern(X)))) {                                 \
      v = results[1].str();                                                    \
    }                                                                          \
  } while (0)

  get_content(gp_package, PACKAGE);
  get_content(gp_comment, COMMENT);
  get_content(gp_ir_func_interface, IR_FUNC_INTERFACE);
  get_content(gp_abstract_base, ABSTRACT_BASE);
  get_content(gp_func_abstract, FUNC_ABSTRACT);
  get_content(gp_func_extends, FUNC_EXTENDS);
  get_content(gp_readme, README);
#undef gen_pattern
#undef get_content

  return 0;
}

static string gp_proxy_class;
static string gp_proxy_proto_return_func;
static string gp_method_map;

static int getTplProxyFile() {
  match_results<std::string::iterator> results;
#define gen_pattern(X) string(#X "_BEG([\\s\\S]*?)" #X "_END")
#define get_content(v, X)                                                      \
  do {                                                                         \
    if (regex_search(tpl_proxy_content.begin(), tpl_proxy_content.end(),       \
                     results, regex(gen_pattern(X)))) {                        \
      v = results[1].str();                                                    \
    }                                                                          \
  } while (0)

  get_content(gp_package, PACKAGE);
  get_content(gp_comment, COMMENT);
  get_content(gp_proxy_class, PROXY_CLASS);
  get_content(gp_proxy_proto_return_func, PROXY_PROTO_RETURN_FUNC);
  get_content(gp_method_map, METHOD_MAP);
  get_content(gp_readme, README);
#undef gen_pattern
#undef get_content

  return 0;
}

static int get_java_pkgname(const string &javafile, string &pkg,
                            pkginfo_st &pkginfo) {

  ifstream ifile(javafile);
  if (!ifile.good()) {
    cerr << "open javafile[" << javafile << "] failed!" << endl;
    exit(1);
  }

  string s("");
  while (!ifile.eof()) {
    char buf[4097] = {0};
    memset(buf, 0, 4096);
    ifile.read(buf, 4096);
    s += buf;
  }
  ifile.close();

  match_results<std::string::iterator> results;
  if (regex_search(s.begin(), s.end(), results, regex("package (.*);"))) {
    if (results.size() != 2) {
      std::cerr << "can not get java pkgname 1" << std::endl;
      return -1;
    }
    pkg = results[1].str();
    pkginfo.pkgname = results[1].str();
  }
  // else {
  //    std::cerr << "can not get java pkgname 2" << std::endl;
  //    return -1;
  //}

  if (regex_search(s.begin(), s.end(), results,
                   regex("public final class (.*) \\{"))) {
    if (results.size() != 2) {
      std::cerr << "can not get java pkg outclass 1" << std::endl;
      return -1;
    }
    if (!pkg.empty())
      pkg = pkg + ".";
    pkg = pkg + results[1].str();
    pkginfo.outclass = results[1].str();
    if (!pkginfo.pkgname.empty())
      pkginfo.pkgname = pkginfo.pkgname + ".";
    pkginfo.pkgfullname = pkginfo.pkgname + pkginfo.outclass;
  } else {
    std::cerr << "can not get java pkg outclass 2" << std::endl;
    return -1;
  }

  auto start = s.begin();
  auto end = s.end();
  while (regex_search(start, end, results,
                      regex("static final class (.*) extends"))) {
    if (results.size() != 2) {
      std::cerr << "can not get java classes 1" << std::endl;
      return -1;
    }
    if (results[1].str() != "Builder") {
      pkginfo.classes.insert(results[1].str());
    }
    start = results[0].second;
  }
  if (pkginfo.classes.size() < 1) {
    std::cerr << "can not get java classes 2" << std::endl;
    return -1;
  }

  return 0;
}

static int get_java_pkgname(const IR &ir, vector<pkginfo_st> &pkgs) {
  int ret = -1;

  if (pkgs.size() > 0) {
    return 0;
  }

  for (auto &javafile : ir.java_files) {
    string pkg("");
    pkginfo_st pkginfo;
    ret = get_java_pkgname(javafile, pkg, pkginfo);
    if (ret != 0) {
      cerr << "get java package info failed" << endl;
      exit(1);
    }
    pkgs.push_back(pkginfo);
  }

  return 0;
}

static void putResult(const string &filename, const string &content) {
  ofstream ofile(filename);
  if (ofile.good()) {
    ofile.write(content.c_str(), content.length());
    ofile.close();
  }
}

static string getRetType(const string &type) {
  if (type == "bool")
    return "boolean";
  if (type == "string")
    return "String";
  else
    return type;
}

static string getDataType(const string &type) {
  if (type == "boolean")
    return "Bool";
  if (type == "int")
    return "Int32";
  if (type == "long")
    return "Int64";
  if (type == "float")
    return "Float";
  if (type == "double")
    return "Double";
  if (type == "String")
    return "Strings";
  else
    return type;
}

static string getDefaultValue(const string &type) {
  if (type == "boolean")
    return "false";
  if (type == "int")
    return "0";
  if (type == "long")
    return "0";
  if (type == "float")
    return "0.0f";
  if (type == "double")
    return "0.0";
  if (type == "String")
    return "\"\"";
  else
    return "";
}

int get_types_pkgs(string arg_types[3], string s_java_pkg[3],
                   string s_java_pkg_[3], const IR &ir, const IR::func &func) {

  if (func.argtypes.size() < 2) {
    cerr << "error func.argtypes.size(): " << func.argtypes.size() << endl;
    return -1;
  }

  // 0,rett; 1,arg2; 2,arg2
  vector<string> types = {func.rett, func.argtypes[0], func.argtypes[1]};
  // vector<string> types = { "%\"class.Foo\"*", "%class.XX::Foo*" ,
  // "%\"class.XX::Foo\"*" };

  for (int i = 0; i < 3; i++) {
    string argtype = types[i];
    // std::cout << "argt type:" << argtype << std::endl;
    bool proto_type = false;
    if (mapJavaTypes.find(argtype) != mapJavaTypes.end()) { // basic type
      argtype = mapJavaTypes[argtype];
    } else {
      if ((argtype.find("%\"class.") == 0) ||
          (argtype.find("%class.") == 0)) { // protobuffer message type
        int dif = 3;                        // for substr
        if (argtype.find("%class.") == 0)
          dif = 2;

        if (!have_protobuf) {
          if (0 != get_java_pkgname(ir, pkgs)) {
            return -1;
          }
          /// for (auto _pkg : pkgs) {
          ///    for (auto _cls : _pkg.classes) {
          ///        cout << "cls:" << _cls << endl;
          ///    }
          ///}
          have_protobuf = true;
        }
        size_t pos = argtype.find_last_of("::");
        if (pos == string::npos) {
          pos = argtype.find_last_of(".");
        }
        if (pos != string::npos) {
          // string s_proto_full_type = argtype.substr(7, argtype.length() - 8);
          // // "class.XXXXX" string s_proto_prfx_type = argtype.substr(7, pos -
          // 8);

          argtype = argtype.substr(pos + 1, argtype.length() - pos - dif);
          proto_type = true;
          {
            bool got = false;
            for (auto pkginfo : pkgs) {
              for (auto cls : pkginfo.classes) {
                if (cls == argtype) {
                  got = true;
                  s_java_pkg[i] = pkginfo.pkgfullname;
                  s_java_pkg_[i] =
                      regex_replace(s_java_pkg[i], regex("\\."), string("_"));
                  break;
                }
              }
              if (got)
                break;
            }
            if (!got) {
              cerr << "can not got arg: " << argtype << endl;
              return -1;
            }
          }
        } else {
          std::cerr << "cannot get type1: " << argtype << std::endl;
          return -1;
        }
      } else {
        std::cerr << "cannot get type2: " << argtype << std::endl;
        return -1;
      }
    }
    (void)proto_type;
    // std::cout << "java type:" << argtype << std::endl;

    arg_types[i] = argtype;
  }

  return 0;
}

int gen_java(IR &ir) {
  string java_outputdir = outputdir + "java/";
  makedirs(java_outputdir);
  if (0 != getTplFile()) {
    return -1;
  }

  string content_res;
  content_res += gp_package;
  content_res += gp_comment;
  content_res += gp_abstract_base;

  regex rfunchash("IRFUNCHASH");
  regex rfuncposx("IRFUNCPOSX");
  regex rfuncintf("IR_FUNC_INTERFACES");

  string s_instances("");
  string s_abstractf("");
  string s_extendsfs("");
  string temp, tmp1, tmp2;

  for (auto &func : ir.funcs) {
#if USE_FUNC_PROT_HASH
    string func_hash = func.hash_prot;
#else // USE_FUNC_NAME_HASH
    string func_hash = func.hash_name;
#endif

    temp = gp_ir_func_interface;
    temp = regex_replace(temp, rfunchash, func_hash);
    temp = regex_replace(temp, rfuncposx, string("01"));
    s_instances += temp;
    temp = gp_ir_func_interface;
    temp = regex_replace(temp, rfunchash, func_hash);
    temp = regex_replace(temp, rfuncposx, string("02"));
    s_instances += temp;

    tmp1 = gp_func_abstract;
    tmp1 = regex_replace(tmp1, rfunchash, func_hash);
    tmp1 = regex_replace(tmp1, rfuncposx, string("01"));
    tmp2 = gp_func_abstract;
    tmp2 = regex_replace(tmp2, rfunchash, func_hash);
    tmp2 = regex_replace(tmp2, rfuncposx, string("02"));

    string s_java_pkg[3];  // 0,rett; 1,arg2; 2,arg2
    string s_java_pkg_[3]; // 0,rett; 1,arg2; 2,arg2
    string arg_types[3];   // 0,rett; 1,arg2; 2,arg2
    if (0 != get_types_pkgs(arg_types, s_java_pkg, s_java_pkg_, ir, func)) {
      return -1;
    }

    ///
    bool proto_type = false;
    string ret_type1, ret_type2;
    string def_value1, def_value2;
    string data_type1, data_type2;

    temp = gp_func_extends;
    ret_type1 = getRetType(arg_types[1]);
    def_value1 = getDefaultValue(ret_type1);
    data_type1 = getDataType(ret_type1);
    if (java_basic_types.find(ret_type1) == java_basic_types.end()) { // pb type
      proto_type = true;
      data_type1 = s_java_pkg[1] + "." + ret_type1 +
                   ".newBuilder().build().toByteArray()";
      def_value1 = s_java_pkg[1] + "." + ret_type1 + ".newBuilder()";
      ret_type1 = s_java_pkg[1] + "." + ret_type1 + ".Builder";

      tmp2 =
          regex_replace(tmp2, regex("X_DEFAULT_INPUT"), string("RETVALUEFUNC"));
      temp = regex_replace(temp, regex("RETTYPE_DATADEFAULTVALUE"),
                           string("RETTYPE builder = DATADEFAULTVALUE"));
      temp = regex_replace(temp, regex("RETVALUEFUNC"),
                           string("builder.build().toByteArray()"));
    } else {
      tmp2 = regex_replace(tmp2, regex("X_DEFAULT_INPUT"),
                           string("Data.RETVALUEFUNC(ALICE_INPUT)"));
      temp = regex_replace(temp, regex("RETTYPE_DATADEFAULTVALUE"),
                           string("RETTYPE ret_value = DATADEFAULTVALUE"));
      temp = regex_replace(temp, regex("RETVALUEFUNC"),
                           string("Data.RETVALUEFUNC(ret_value)"));
    }

    temp = regex_replace(temp, rfunchash, func_hash);
    temp = regex_replace(temp, rfuncposx, string("01"));
    temp = regex_replace(temp, regex("RETVALUEFUNC"), data_type1);
    temp = regex_replace(temp, regex("DATADEFAULTVALUE"), def_value1);
    temp = regex_replace(temp, regex("RETTYPE"), ret_type1);

    s_extendsfs += temp;

    temp = gp_func_extends;
    proto_type = false;
    ret_type2 = getRetType(arg_types[2]);
    def_value2 = getDefaultValue(ret_type2);
    data_type2 = getDataType(ret_type2);
    if (java_basic_types.find(ret_type2) == java_basic_types.end()) { // pb type
      proto_type = true;
      data_type2 = s_java_pkg[2] + "." + ret_type2 +
                   ".newBuilder().build().toByteArray()";
      def_value2 = s_java_pkg[2] + "." + ret_type2 + ".newBuilder()";
      ret_type2 = s_java_pkg[2] + "." + ret_type2 + ".Builder";

      tmp1 =
          regex_replace(tmp1, regex("X_DEFAULT_INPUT"), string("RETVALUEFUNC"));
      temp = regex_replace(temp, regex("RETTYPE_DATADEFAULTVALUE"),
                           string("RETTYPE builder = DATADEFAULTVALUE"));
      temp = regex_replace(temp, regex("RETVALUEFUNC"),
                           string("builder.build().toByteArray()"));
    } else {
      tmp1 = regex_replace(tmp1, regex("X_DEFAULT_INPUT"),
                           string("Data.RETVALUEFUNC(BOB_INPUT)"));
      temp = regex_replace(temp, regex("RETTYPE_DATADEFAULTVALUE"),
                           string("RETTYPE ret_value = DATADEFAULTVALUE"));
      temp = regex_replace(temp, regex("RETVALUEFUNC"),
                           string("Data.RETVALUEFUNC(ret_value)"));
    }

    temp = regex_replace(temp, rfunchash, func_hash);
    temp = regex_replace(temp, rfuncposx, string("02"));
    temp = regex_replace(temp, regex("RETVALUEFUNC"), data_type2);
    temp = regex_replace(temp, regex("DATADEFAULTVALUE"), def_value2);
    temp = regex_replace(temp, regex("RETTYPE"), ret_type2);
    s_extendsfs += temp;
    ///

    (void)proto_type;

    ///
    tmp1 = regex_replace(tmp1, regex("RETVALUEFUNC"), data_type2);
    tmp1 = regex_replace(tmp1, regex("BOB_INPUT"), def_value2);
    s_abstractf += tmp1;
    tmp2 = regex_replace(tmp2, regex("RETVALUEFUNC"), data_type1);
    tmp2 = regex_replace(tmp2, regex("ALICE_INPUT"), def_value1);
    s_abstractf += tmp2;
    ///

    temp = func.name + "_" + arg_types[1] + "_" + arg_types[2];
    s_extendsfs = regex_replace(s_extendsfs, regex("IRFUNCPROTA"), temp);
    s_abstractf = regex_replace(s_abstractf, regex("IRFUNCPROTA"), temp);
    s_instances = regex_replace(s_instances, regex("IRFUNCPROTA"), temp);

    temp = func.name + "(" + arg_types[1] + "," + arg_types[2] + ")";
    s_extendsfs = regex_replace(s_extendsfs, regex("IRFUNCPROT"), temp);

    func.prot = temp;
  }
  digest = print_digest(ir);

  s_instances = regex_replace(s_instances, regex("\n\n"), string("\n"));

  content_res =
      regex_replace(content_res, regex("CONTENT_FUNC_ABSTRACT"), s_abstractf);
  content_res =
      regex_replace(content_res, regex("CONTENT_FUNC_EXTENDS"), s_extendsfs);

  content_res = regex_replace(content_res, rfuncintf, s_instances);
  content_res = regex_replace(content_res, regex("IRHASH"), ir.hash);
  content_res = regex_replace(content_res, regex("IRNAME"), ir.name);
  content_res =
      regex_replace(content_res, regex("IRDIGEST"), digest); // digest);

  putResult(java_outputdir + ir.name + ".java", content_res);

  gp_readme = regex_replace(gp_readme, regex("IRDIGEST"), digest); // digest);
  gp_readme = regex_replace(gp_readme, regex("IRNAME"), ir.name);
  putResult(java_outputdir + ir.name + "-README.TXT", gp_readme);

  return 0;
}

int gen_java_proxy(IR &ir) {
  string java_outputdir = outputdir + "java/";
  makedirs(java_outputdir);
  if (0 != getTplProxyFile()) {
    return -1;
  }

  string content_res;
  content_res += gp_package;
  content_res += gp_comment;
  content_res += gp_proxy_class;

  regex rfunchash("IRFUNCHASH");
  regex rfuncposx("IRFUNCPOSX");
  regex rfuncintf("IR_FUNC_INTERFACES");

  string s_instances("");
  string s_abstractf("");
  string s_extendsfs("");
  string s_methods("");
  string s_methodmaps("");
  string s_proxy_proto_returns("");
  string temp, tmp1, tmp2;

  for (auto func : ir.funcs) {
#if USE_FUNC_PROT_HASH
    string func_hash = func.hash_prot;
#else // USE_FUNC_NAME_HASH
    string func_hash = func.hash_name;
#endif

    string s_java_pkg[3];  // 0,rett; 1,arg2; 2,arg2
    string s_java_pkg_[3]; // 0,rett; 1,arg2; 2,arg2
    string arg_types[3];   // 0,rett; 1,arg2; 2,arg2
    if (0 != get_types_pkgs(arg_types, s_java_pkg, s_java_pkg_, ir, func)) {
      return -1;
    }

    string rett = arg_types[0];
    if (java_basic_types.find(rett) == java_basic_types.end()) {
      static set<string> pprs;
      string stmp = s_java_pkg[0] + rett;
      if (pprs.find(stmp) == pprs.end()) {
        pprs.insert(stmp);
        string tmp_returns = gp_proxy_proto_return_func;
        tmp_returns = regex_replace(tmp_returns, regex("RETTYPE"), rett);
        tmp_returns =
            regex_replace(tmp_returns, regex("RETPKG"), s_java_pkg[0]);
        s_proxy_proto_returns += tmp_returns;
      }
    }

    temp = func.name + "_" + arg_types[1] + "_" + arg_types[2];

    string func_rett = rett;
    string tmpmethod = rett + "_" + temp;
    string tmpmethodmap = gp_method_map;
    tmpmethodmap =
        regex_replace(tmpmethodmap, regex("IR_FUNC_PROT"), tmpmethod);
    tmpmethodmap = regex_replace(tmpmethodmap, regex("IRFUNCNAME"), func.name);

    temp =
        func.name + "(" + arg_types[1] + "," + arg_types[2] + ")"; // func prot
    tmpmethodmap = regex_replace(tmpmethodmap, regex("IRFUNCPROTO"), temp);
    tmpmethodmap = regex_replace(tmpmethodmap, regex("IRFUNCHASH"), func_hash);
    s_methodmaps += tmpmethodmap;
    s_methods += "        " + tmpmethod + ",\n";
  }

  content_res = regex_replace(content_res, regex("IRHASH"), ir.hash);
  content_res = regex_replace(content_res, regex("IRDIGEST"), digest);

  string proxyname = ir.name;
  content_res = regex_replace(content_res, regex("IRNAME"), ir.name);

  s_methods.pop_back();
  content_res = regex_replace(content_res, regex("IR_FUNC_PROT"), s_methods);
  s_methodmaps.pop_back();
  s_methodmaps = regex_replace(s_methodmaps, regex("\n\n"), string("\n"));
  content_res = regex_replace(content_res, regex("METHOD_MAP"), s_methodmaps);
  content_res = regex_replace(content_res, regex("PROXY_PROTO_RETURN_FUNC_S"),
                              s_proxy_proto_returns);

  putResult(java_outputdir + ir.name + ".java", content_res);

  gp_readme = regex_replace(gp_readme, regex("IRDIGEST"), digest);
  gp_readme = regex_replace(gp_readme, regex("IRNAME"), ir.name);
  putResult(java_outputdir + ir.name + "-README.TXT", gp_readme);

  return 0;
}

int gen_cs(IR &ir) { return 0; }
int gen_go(IR &ir) { return 0; }
int gen_cpp(IR &ir) { return 0; }
int gen_lua(IR &ir) { return 0; }
int gen_php(IR &ir) { return 0; }
int gen_python(IR &ir) { return 0; }
int gen_nodejs(IR &ir) { return 0; }
