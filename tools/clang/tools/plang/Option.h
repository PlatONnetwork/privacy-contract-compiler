#include <string>
#include <vector>

struct PlangOption {

  std::vector<std::string> InputFilenames;

  std::string Config;

  bool Help;

  std::string JavaCode;
  std::string MPCC;

  std::string OutputFilename;

  std::vector<std::string> ProtobufCC;

  std::vector<std::string> ProtobufJava;

  std::vector<std::string> clangArgs;
};
