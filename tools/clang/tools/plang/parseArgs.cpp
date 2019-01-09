#include <algorithm>
#include <set>
#include <string>
#include <vector>

#include "llvm/Option/Arg.h"
#include "llvm/Option/ArgList.h"
#include "llvm/Option/OptTable.h"
#include "llvm/Option/Option.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/raw_ostream.h"

#include "clang/Driver/Options.h"

#include "Option.h"

using namespace std;
using namespace llvm;
using namespace opt;

static const char *const prefix_0[] = {nullptr};
static const char *const prefix_1[] = {"-", nullptr};
static const char *const prefix_3[] = {"--", nullptr};

enum ID {
  OPT_INVALID,
  OPT_INPUT,
  OPT_UNKNOWN,

  OPT_config,

  OPT__help,

  OPT_java_code,
  OPT_mpcc,

  OPT__output_EQ,
  OPT__output,
  OPT_o,

  OPT_protobuf_CC,
  OPT_protobuf_Java,

};

static const OptTable::Info InfoTable[] = {
    //    struct Info { Prefixes, Name, HelpText, MetaVar, ID, Kind, Param,
    //    Flags, GroupID, AliasID, AliasArgs, Values};
    //
    {prefix_0, "<input>", nullptr, nullptr, OPT_INPUT, Option::InputClass, 0, 0,
     OPT_INVALID, OPT_INVALID, nullptr, nullptr},

    {prefix_0, "<unknown>", nullptr, nullptr, OPT_UNKNOWN, Option::UnknownClass,
     0, 0, OPT_INVALID, OPT_INVALID, nullptr, nullptr},

    {prefix_1, "config", "Set config file path", "<file>", OPT_config,
     Option::JoinedOrSeparateClass, 0, 0, OPT_INVALID, OPT_INVALID, nullptr,
     nullptr},

    {prefix_3, "help", "Display available options", nullptr, OPT__help,
     Option::FlagClass, 0, 0, OPT_INVALID, OPT_INVALID, nullptr, nullptr},

    {prefix_1, "java-code", "Write java code to directoy", "<directoy>",
     OPT_java_code, Option::JoinedOrSeparateClass, 0, 0, OPT_INVALID,
     OPT_INVALID, nullptr, nullptr},

    {prefix_1, "mpcc", "Write mpcc to <file>", "<file>", OPT_mpcc,
     Option::JoinedOrSeparateClass, 0, 0, OPT_INVALID, OPT_INVALID, nullptr,
     nullptr},

    {prefix_3, "output=", nullptr, nullptr, OPT__output_EQ, Option::JoinedClass,
     0, 0, OPT_INVALID, OPT_o, nullptr, nullptr},
    {prefix_3, "output", nullptr, nullptr, OPT__output, Option::SeparateClass,
     0, 0, OPT_INVALID, OPT_o, nullptr, nullptr},
    {prefix_1, "o", "Write output to <file>", "<file>", OPT_o,
     Option::JoinedOrSeparateClass, 0, 0, OPT_INVALID, OPT_INVALID, nullptr,
     nullptr},

    {prefix_1, "protobuf-cc", "Set protobuf *.cc file path", "<file>",
     OPT_protobuf_CC, Option::JoinedOrSeparateClass, 0, 0, OPT_INVALID,
     OPT_INVALID, nullptr, nullptr},

    {prefix_1, "protobuf-java", "Set protobuf *.java file path", "<file>",
     OPT_protobuf_Java, Option::JoinedOrSeparateClass, 0, 0, OPT_INVALID,
     OPT_INVALID, nullptr, nullptr},

};

class PlangOptTable : public OptTable {
public:
  PlangOptTable() : OptTable(InfoTable) {}
};

std::unique_ptr<OptTable> createPlangOptTable() {
  return llvm::make_unique<PlangOptTable>();
}


bool ParseArgs(int argc, char *argv[], PlangOption &PlangOpts) {

  unique_ptr<OptTable> Opts = createPlangOptTable();
  unique_ptr<OptTable> clangOpts = clang::driver::createDriverOptTable();

  unsigned MissingArgIndex, MissingArgCount;

  InputArgList PlangArgs = Opts->ParseArgs(makeArrayRef(argv + 1, argc - 1),
                                           MissingArgIndex, MissingArgCount);

  PlangOpts.Help = false;

  vector<const char *> clangArgv;

  for (const Arg *A : PlangArgs) {
    const Option &Option = A->getOption();

    if (Option.matches(OPT__help))
      PlangOpts.Help = true;

    else if (Option.matches(OPT_config))
      PlangOpts.Config = A->getValue();

    else if (Option.matches(OPT_java_code))
      PlangOpts.JavaCode = A->getValue();
    else if (Option.matches(OPT_mpcc))
      PlangOpts.MPCC = A->getValue();

    else if (Option.matches(OPT_o))
      PlangOpts.OutputFilename = A->getValue();

    else if (Option.matches(OPT_protobuf_CC))
      PlangOpts.ProtobufCC.push_back(A->getValue());

    else if (Option.matches(OPT_protobuf_Java))
      PlangOpts.ProtobufJava.push_back(A->getValue());

    else
      clangArgv.push_back(A->getSpelling().data());
  }

  InputArgList clangArgs =
      clangOpts->ParseArgs(clangArgv, MissingArgIndex, MissingArgCount);

  for (const Arg *A : clangArgs) {
    const Option &Option = A->getOption();
    if (Option.matches(clang::driver::options::OPT_INPUT))
      PlangOpts.InputFilenames.push_back(A->getValue());

    else if (Option.matches(clang::driver::options::OPT_help) ||
             Option.matches(clang::driver::options::OPT_config) ||
             Option.matches(clang::driver::options::OPT_o) ||
             Option.matches(clang::driver::options::OPT_g_Flag) ||
             Option.matches(clang::driver::options::OPT_UNKNOWN))
      continue;
    else {
      // PlangOpts.clangArgs.push_back(A->getSpelling().data());
      ArgStringList ASL;
      A->render(clangArgs, ASL);
      for (auto it : ASL)
        PlangOpts.clangArgs.push_back(it);
    }
  }

  if (PlangOpts.Help) {
    Opts->PrintHelp(llvm::outs(), "plang [clang args]", "PlatON C++ compiler",
                    0, 0, false);
    return false;
  }

  if (PlangOpts.MPCC.length() == 0)
    PlangOpts.MPCC = "mpcc.cpp";

  if (PlangOpts.JavaCode.length() == 0)
    PlangOpts.JavaCode = ".";

  if (PlangOpts.Config.length() == 0) {
    llvm::outs() << "no config file\n";
    return false;
  }

  return true;
}
