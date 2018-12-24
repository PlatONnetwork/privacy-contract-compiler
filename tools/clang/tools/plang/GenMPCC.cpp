#include <iostream>
#include <vector>

#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/ToolOutputFile.h"

#include "Option.h"
#include "rapidjson/document.h"

using namespace llvm;
using namespace std;
using namespace rapidjson;

string MPCCTemp =
    "#ifndef PLATON_MPCC_CPP\n"
    "#define PLATON_MPCC_CPP\n"
    "\n"
    "#include <platon/platon.hpp>\n"
    "#include <stdio.h>\n"
    "#include <stdlib.h>\n"
    "#include <string>\n"
    "\n"
    "namespace mpc {\n"
    "\n"
    "#define COMMON_SPLIT_CHAR \"&\"\n"
    "#define OWNER \"__OWNER__\"\n"
    "\n"
    "#define PREFIX \"__MPC__\"\n"
    "#define PREFIX_BONUS_RULE \"__BONUS_RULE__\"\n"
    "\n"
    "#define KEY_IR \"__IR__\"\n"
    "#define KEY_PARTIES PREFIX \"__PARTIES__\"\n"
    "#define KEY_URLS PREFIX \"__URLS__\"\n"
    "#define KEY_INVITORS PREFIX \"__INVITOR__\"\n"
    "#define KEY_TASK_INFO PREFIX \"__TASK__\"\n"
    "#define KEY_METHOD_PRICE PREFIX \"__METHOD_PRICE__\"\n"
    "\n"
    "/// IR data -> IR_VALUE_TEMPLATE\n"
    "#define IR_VALUE \"$IR_VALUE_TEMPLATE$\"\n"
    "\n"
    "/// Starter -> INVITOR_VALUE_TEMPLATE\n"
    "#define INVITOR_VALUE \"$INVITOR_VALUE_TEMPLATE$\"\n"
    "\n"
    "/// Participator, use symbol '&' to split, eg:a&b&c -> "
    "PARTIES_VALUE_TEMPLATE\n"
    "#define PARTIES_VALUE \"$PARTIES_VALUE_TEMPLATE$\"\n"
    "\n"
    "/// Urls for participators, use symbol '&' to split.\n"
    "/// URLS -> URLS_VALUE_TEMPLATE\n"
    "#define URLS_VALUE \"$URLS_VALUE_TEMPLATE$\"\n"
    "\n"
    "/// Rule of allocation.\n"
    "/// format:k1:v1,k2:v2,k3:v3 -> PROFIT_RULES_VALUE\n"
    "#define PROFIT_RULES_VALUE \"$PROFIT_RULES_VALUE$\"\n"
    "\n"
    "/// Calu method's price -> METHOD_PRICE_VALUE\n"
    "/// rule:${METHOD}&${VALUE},${METHOD}&${VALUE}\n"
    "/// eg:\n"
    "/// "
    "func01$10000000000000000,func02$200000000000000,func03$"
    "4000000000000000000000000000\n"
    "#define METHOD_PRICE_VALUE \"$METHOD_PRICE_VALUE$\"\n"
    "\n"
    "/// the prefix key of map struct\n"
    "#define PREFIX_RULES_MAP PREFIX \"_MAP_RULE_\"\n"
    "#define PREFIX_RESULT_MAP PREFIX \"_MAP_RESULT_\"\n"
    "#define PREFIX_ALLOT_MAP PREFIX \"_MAP_ALLOT_\"\n"
    "#define PREFIX_METHOD_MAP PREFIX \"_MAP_PRICE_\"\n"
    "\n"
    "class MPC : public platon::Contract {\n"
    "public:\n"
    "  // define event.\n"
    "  PLATON_EVENT(start_calc_event, uint64_t, const char *)\n"
    "  PLATON_EVENT(set_result_event, uint64_t, const char *)\n"
    "  PLATON_EVENT(set_fees_event, uint64_t, const char *)\n"
    "\n"
    "  void init() {\n"
    "    platon::setState(OWNER, platon::caller().toString()); // save "
    "msg.sender\n"
    "    platon::setState(KEY_INVITORS, INVITOR_VALUE);        // save "
    "invitor\n"
    "    platon::setState(KEY_PARTIES, PARTIES_VALUE);         // save "
    "partner\n"
    "    platon::setState(KEY_URLS, URLS_VALUE);               // save all "
    "urls\n"
    "    platon::setState(KEY_IR, IR_VALUE);                   // save ir "
    "data\n"
    "    init_set_urls();                                      // save each "
    "one by id\n"
    "    init_set_price(); // save price of method\n"
    "  }\n"
    "\n"
    "  /// get the owner address of contract.\n"
    "  const char *get_owner() const {\n"
    "    std::string _owner_;\n"
    "    platon::getState(OWNER, _owner_);\n"
    "    return _owner_.c_str();\n"
    "  }\n"
    "\n"
    "$Functions$"
    "\n"
    "  /// trigger mpc compute.\n"
    "  void start_calc(const char *method, const char *extra) {\n"
    "\n"
    "    // check msg.send is valid\n"
    "    bool _isPartner = is_partner();\n"
    "    if (!_isPartner) {\n"
    "      platon::println(\"Invalid msg.sender, not belong to partners.\");\n"
    "      PLATON_EMIT_EVENT(start_calc_event, 0,\n"
    "                        \"Invalid msg.sender, not belong to "
    "partners.\");\n"
    "      return;\n"
    "    }\n"
    "\n"
    "    // check value is greater than preprice.\n"
    "    bool is_enough_res = is_enough_price(method);\n"
    "    if (!is_enough_res) {\n"
    "      platon::println(\"insufficient value to the method of \", method);\n"
    "      PLATON_EMIT_EVENT(start_calc_event, 0,\n"
    "                        \"Insufficient value for the method.\");\n"
    "      return;\n"
    "    }\n"
    "\n"
    "    // gen task id and save value\n"
    "    const char *task_id = gen_task_id().c_str();\n"
    "    platon::println(\"New task id: \", task_id);\n"
    "    bool save_res = save_allot_amount(task_id);\n"
    "    if (!save_res) {\n"
    "      platon::println(\"save allot amount fail\");\n"
    "      PLATON_EMIT_EVENT(start_calc_event, 0, \"save allot amount "
    "fail\");\n"
    "      return;\n"
    "    }\n"
    "\n"
    "    // save task information. INFO_PREFIX + TASK_ID\n"
    "    std::string task_key_str = KEY_TASK_INFO + std::string(task_id);\n"
    "    std::string info_str =\n"
    "        std::string(method) + COMMON_SPLIT_CHAR + std::string(extra);\n"
    "    platon::setState(task_key_str, info_str);\n"
    "    platon::println(\"start success..\");\n"
    "    PLATON_EMIT_EVENT(start_calc_event, 1, task_id);\n"
    "  }\n"
    "\n"
    "  /// The result recored, and allot profit to participators.\n"
    "  void set_result(const char *taskId, uint64_t status, const char *data) "
    "{\n"
    "    // check msg.send is valid\n"
    "    bool _isPartner = is_partner();\n"
    "    if (!_isPartner) {\n"
    "      platon::println(\n"
    "          \"set_result->Invalid msg.sender, not belong to partners.\");\n"
    "      PLATON_EMIT_EVENT(start_calc_event, 0,\n"
    "                        \"Invalid msg.sender, not belong to "
    "partners.\");\n"
    "      return;\n"
    "    }\n"
    "\n"
    "    std::string key_str = PREFIX_RESULT_MAP + std::string(taskId);\n"
    "    // check result set\n"
    "    std::string res_value_str;\n"
    "    platon::getState(key_str, res_value_str);\n"
    "    if (res_value_str.length() > 1) {\n"
    "      platon::println(\"set_result-> set already. Can not reset "
    "again.\");\n"
    "      PLATON_EMIT_EVENT(start_calc_event, 0,\n"
    "                        \"set already. Can not reset again.\");\n"
    "      return;\n"
    "    }\n"
    "\n"
    "    // save result\n"
    "    std::string status_str = toString(status);\n"
    "    std::string data_str = std::string(data);\n"
    "    std::string value_str = status_str + COMMON_SPLIT_CHAR + data_str;\n"
    "    platon::println(\"save result, key: \", key_str, \" value:\", "
    "value_str);\n"
    "    platon::setState(key_str, value_str);\n"
    "\n"
    "    // divide the money. equal division.\n"
    "    if (status != 1) { // 1 success 0 fail.\n"
    "      platon::println(\"set_result-> status must equal 1.\");\n"
    "      PLATON_EMIT_EVENT(start_calc_event, 0, \"status must equal 1.\");\n"
    "      return;\n"
    "    }\n"
    "    platon::u256 amount_i = get_saved_amount(taskId);\n"
    "    const char *partners_c = get_participants();\n"
    "    std::string partner_str = std::string(partners_c);\n"
    "    std::vector<std::string> partner_vec =\n"
    "        split(partner_str, COMMON_SPLIT_CHAR);\n"
    "\n"
    "    platon::u256 count_i(partner_vec.size());\n"
    "\n"
    "    platon::println(\"part per:\", count_i.convert_to<std::string>());\n"
    "    platon::u256 each_one_i = amount_i / count_i;\n"
    "    platon::println(\"avg :\", each_one_i.convert_to<std::string>());\n"
    "\n"
    "    // transfer to partner\n"
    "    for (unsigned int i = 0; i < partner_vec.size(); ++i) {\n"
    "      std::string p_str = partner_vec[i];\n"
    "      platon::println(\"transfer to:\", p_str,\n"
    "                      \" value:\", "
    "each_one_i.convert_to<std::string>());\n"
    "      platon::Address _addr(p_str, true);\n"
    "      platon::callTransfer(_addr, each_one_i);\n"
    "    }\n"
    "    platon::println(\"Set result success.\");\n"
    "    PLATON_EMIT_EVENT(set_result_event, 1, \"Setting result success.\");\n"
    "  }\n"
    "\n"
    "  /// k1:v1,k2:v2,k3:v3\n"
    "  void set_fees(const char *fees) {\n"
    "    platon::println(\"into set_fees...\");\n"
    "    PLATON_EMIT_EVENT(set_fees_event, 1, \"set fees done.\");\n"
    "  }\n"
    "\n"
    "  const char *get_ir_data() const {\n"
    "    std::string irdata;\n"
    "    platon::getState(KEY_IR, irdata);\n"
    "    return irdata.c_str();\n"
    "  }\n"
    "\n"
    "  const char *get_participants() const {\n"
    "    std::string parties;\n"
    "    platon::getState(KEY_PARTIES, parties);\n"
    "    platon::println(\"get_participants:\", parties);\n"
    "    return parties.c_str();\n"
    "  }\n"
    "\n"
    "  const char *get_urls() const {\n"
    "    std::string urls;\n"
    "    platon::getState(KEY_URLS, urls);\n"
    "    return urls.c_str();\n"
    "  }\n"
    "\n"
    "  const char *get_url_by_id(const char *id) const {\n"
    "    std::string url;\n"
    "    std::string key_str = KEY_URLS + std::string(id);\n"
    "    platon::getState(key_str, url);\n"
    "    return url.c_str();\n"
    "  }\n"
    "\n"
    "  const char *get_result(const char *task_id) const {\n"
    "    std::string id_str = std::string(task_id);\n"
    "    std::string key_str = PREFIX_RESULT_MAP + std::string(id_str);\n"
    "    std::string result;\n"
    "    platon::getState(key_str, result);\n"
    "    // std::string value_str = status_str + COMMON_SPLIT_CHAR + "
    "data_str;\n"
    "    std::vector<std::string> partner_vec = split(result, "
    "COMMON_SPLIT_CHAR);\n"
    "    if (partner_vec.size() != 2) {\n"
    "      return \"\";\n"
    "    }\n"
    "    return partner_vec[1].c_str();\n"
    "  }\n"
    "\n"
    "  uint64_t get_status(const char *task_id) const {\n"
    "    std::string id_str = std::string(task_id);\n"
    "    std::string key_str = PREFIX_RESULT_MAP + std::string(id_str);\n"
    "    std::string result;\n"
    "    platon::getState(key_str, result);\n"
    "    std::vector<std::string> partner_vec = split(result, "
    "COMMON_SPLIT_CHAR);\n"
    "    if (partner_vec.size() != 2) {\n"
    "      return 0;\n"
    "    }\n"
    "    uint64_t st = stouint64(partner_vec[0]);\n"
    "    return st;\n"
    "  }\n"
    "\n"
    "  const char *get_fee(const char *method) const {\n"
    "    platon::u256 fee;\n"
    "    std::string key_str = KEY_METHOD_PRICE + std::string(method);\n"
    "    platon::getState(key_str, fee);\n"
    "    platon::println(\"call get_fee. - \", "
    "fee.convert_to<std::string>());\n"
    "    return fee.convert_to<std::string>().c_str();\n"
    "  }\n"
    "\n"
    "  const char *get_invitor() const {\n"
    "    std::string invitor;\n"
    "    platon::getState(KEY_INVITORS, invitor);\n"
    "    return invitor.c_str();\n"
    "  }\n"
    "\n"
    "private:\n"
    "  std::string gen_task_id() const {\n"
    "    uint64_t nonce_i = getCallerNonce();\n"
    "    std::string nonce_str = toString(nonce_i);\n"
    "    platon::h160 caller_h = platon::caller();\n"
    "    std::string caller_h_str = caller_h.toString();\n"
    "    platon::println(\"get nonce: \", nonce_str);\n"
    "    platon::println(\"get caller_h_str: \", caller_h_str);\n"
    "    std::string src_str = nonce_str + caller_h_str;\n"
    "    char *t_src;\n"
    "    strcpy(t_src, src_str.c_str());\n"
    "\n"
    "    platon::h256 sha3_h = platon::sha3((platon::byte *)t_src, "
    "strlen(t_src));\n"
    "    std::string sha3_h_str = sha3_h.toString();\n"
    "    platon::println(\"task id : \", sha3_h_str);\n"
    "    return sha3_h_str;\n"
    "  }\n"
    "\n"
    "  bool is_partner() {\n"
    "    platon::h160 caller_h = platon::caller();\n"
    "    std::string caller_h_str = caller_h.toString();\n"
    "    std::string partner_str;\n"
    "    platon::getState(KEY_PARTIES, partner_str);\n"
    "    platon::println(\"Participator :\", partner_str);\n"
    "    platon::println(\"Caller :\", caller_h_str);\n"
    "    std::string::size_type idx = caller_h_str.find(\"0x\");\n"
    "    if (idx == std::string::npos) {\n"
    "      caller_h_str = \"0x\" + caller_h_str;\n"
    "    }\n"
    "    platon::println(\"Caller End:\", caller_h_str);\n"
    "    std::vector<std::string> partner_vec =\n"
    "        split(partner_str, COMMON_SPLIT_CHAR);\n"
    "    for (unsigned int i = 0; i < partner_vec.size(); ++i) {\n"
    "      platon::println(\"v_caller:\", partner_vec[i]);\n"
    "      if (strcmp(caller_h_str.c_str(), partner_vec[i].c_str()) == 0) {\n"
    "        return true;\n"
    "      }\n"
    "    }\n"
    "    return false;\n"
    "  }\n"
    "\n"
    "  /// blance is engouth for caller to pay the method's price.\n"
    "  bool is_enough_price(const char *method) {\n"
    "    // the price of method.\n"
    "    platon::u256 fee;\n"
    "    std::string key_str = KEY_METHOD_PRICE + std::string(method);\n"
    "    platon::println(\"is_enough_price : key_str\", key_str);\n"
    "    platon::getState(key_str, fee);\n"
    "    platon::println(\"is_enough_price:call get_fee. - \",\n"
    "                    fee.convert_to<std::string>());\n"
    "    // the pricee of value to caller.\n"
    "    platon::u256 _value = platon::callValue();\n"
    "    platon::println(\"is_enough_price:call get_value. - \",\n"
    "                    _value.convert_to<std::string>());\n"
    "    if (_value >= fee) {\n"
    "      platon::println(\"is_enough_price: return true.\");\n"
    "      return true;\n"
    "    }\n"
    "    platon::println(\"is_enough_price: return false.\");\n"
    "    return false;\n"
    "  }\n"
    "\n"
    "  /// transfer of value, from user's account to contract's account.\n"
    "  bool save_allot_amount(const char *task_id) {\n"
    "    // platon::balance().convert_to<std::string>().c_str();\n"
    "    platon::u256 value_u = platon::callValue();\n"
    "    std::string key_str = PREFIX_ALLOT_MAP + std::string(task_id);\n"
    "\n"
    "    std::string value_str = value_u.convert_to<std::string>();\n"
    "    platon::println(\"save_allot_amount: \", key_str, \" - \", "
    "value_str);\n"
    "    platon::setState(key_str, value_u);\n"
    "    return true;\n"
    "  }\n"
    "\n"
    "  platon::u256 get_saved_amount(const char *task_id) {\n"
    "    std::string key_str = PREFIX_ALLOT_MAP + std::string(task_id);\n"
    "    platon::u256 value_u;\n"
    "    platon::getState(key_str, value_u);\n"
    "    platon::println(\"get_saved_amout: \", "
    "value_u.convert_to<std::string>());\n"
    "    return value_u;\n"
    "  }\n"
    "\n"
    "  void init_set_urls() {\n"
    "    // URLS_VALUE -> \"id01$tcpSession:tcp -p 9527,id02$tcpSession:tcp "
    "-p\n"
    "    // 9527,id03$tcpSession:tcp -p 9527\"\n"
    "    char *values = URLS_VALUE;\n"
    "    char *pattern = \",\";\n"
    "    // [\"id01$tcpSession:tcp -p 9527\", \"id02$tcpSession:tcp -p "
    "9527\",\n"
    "    // \"id03$tcpSession:tcp -p 9527\"] KEY_URLS + \"id01\" -> "
    "tcpSession:tcp -p\n"
    "    // 9527\n"
    "    platon::println(\"---- begin split ------\");\n"
    "    std::vector<std::string> arr = split(values, pattern);\n"
    "    for (unsigned int i = 0; i < arr.size(); ++i) {\n"
    "      char *trim_res = new char[100];\n"
    "      char *str_in = new char[100];\n"
    "      strcpy(str_in, arr[i].c_str());\n"
    "\n"
    "      str_trim(str_in, trim_res);\n"
    "\n"
    "      std::vector<std::string> id_valule_arr = split(str_in, \"$\");\n"
    "      if (id_valule_arr.size() != 2) {\n"
    "        platon::println(\"Invalid value str of urls.\");\n"
    "        platon::platonThrow(\"Invalid value str of urls.\");\n"
    "      }\n"
    "      // do set state\n"
    "      std::string _key = KEY_URLS + id_valule_arr[0];\n"
    "      std::string _value = id_valule_arr[1];\n"
    "      platon::println(\"init urls, key : \", _key, \" , values: \", "
    "_value);\n"
    "      platon::setState(_key, _value);\n"
    "      delete[] str_in;\n"
    "      delete[] trim_res;\n"
    "    }\n"
    "  }\n"
    "\n"
    "  void init_set_price() {\n"
    "    // METHOD_PRICE_VALUE ->\n"
    "    // "
    "\"func01$10000000000000000,func02$200000000000000,func03$"
    "4000000000000000000000000000\"\n"
    "    char *values = METHOD_PRICE_VALUE;\n"
    "    char *pattern = \",\";\n"
    "    // [\"func01$10000000000000000\", \"func01$10000000000000000\",\n"
    "    // \"func01$10000000000000000\"] PREFIX_METHOD_MAP + \"func01\" ->\n"
    "    // 10000000000000000\n"
    "    platon::println(\"---- Begin split price ------\");\n"
    "\n"
    "    std::vector<std::string> arr = split(values, pattern);\n"
    "    platon::println(\"---- End split price ------\", arr.size());\n"
    "    for (unsigned int i = 0; i < arr.size(); ++i) {\n"
    "      char *trim_res = new char[100];\n"
    "      char *str_in = new char[100];\n"
    "      strcpy(str_in, arr[i].c_str());\n"
    "      str_trim(str_in, trim_res);\n"
    "\n"
    "      std::vector<std::string> id_valule_arr = split(str_in, \"$\");\n"
    "      if (id_valule_arr.size() != 2) {\n"
    "        platon::println(\"Invalid value str of price.\");\n"
    "        platon::platonThrow(\"Invalid value str of price.\");\n"
    "      }\n"
    "      // do set state\n"
    "      std::string key_str = KEY_METHOD_PRICE + id_valule_arr[0];\n"
    "      std::string value_str = id_valule_arr[1];\n"
    "      platon::println(\"init price, key : \", key_str, \" , values: \", "
    "value_str);\n"
    "      platon::u256 value_u(value_str);\n"
    "      platon::setState(key_str, value_u);\n"
    "      delete[] str_in;\n"
    "      delete[] trim_res;\n"
    "    }\n"
    "  }\n"
    "\n"
    "private:\n"
    "  uint64_t stouint64(const std::string &num) const {\n"
    "    uint64_t res = 0;\n"
    "    for (size_t i = 0; i < num.length(); i++) {\n"
    "      res = res * 10 + (num[i] - '0');\n"
    "    }\n"
    "    platon::println(\"stouint64 \", num, \"->\", res);\n"
    "    return res;\n"
    "  }\n"
    "\n"
    "  std::string toString(uint64_t num) const {\n"
    "    std::string res;\n"
    "    while (num != 0) {\n"
    "      char c = num % 10 + '0';\n"
    "      num /= 10;\n"
    "      res.insert(0, 1, c);\n"
    "    }\n"
    "    platon::println(\"toString \", num, \"->\", res);\n"
    "    return res;\n"
    "  }\n"
    "\n"
    "  std::vector<std::string> split(const std::string &str,\n"
    "                                 const std::string &pattern) const {\n"
    "    // const char* convert to char*\n"
    "    char *strc = new char[std::strlen(str.c_str()) + 1];\n"
    "    strcpy(strc, str.c_str());\n"
    "    std::vector<std::string> result_vec;\n"
    "    char *tmp_str = strtok(strc, pattern.c_str());\n"
    "    while (tmp_str != NULL) {\n"
    "      result_vec.push_back(std::string(tmp_str));\n"
    "      tmp_str = strtok(NULL, pattern.c_str());\n"
    "    }\n"
    "    delete[] strc;\n"
    "    return result_vec;\n"
    "  }\n"
    "\n"
    "  void str_trim(char *str_in, char *str_out) {\n"
    "    int i, j;\n"
    "    i = 0;\n"
    "    j = strlen(str_in) - 1;\n"
    "    while (str_in[i] == ' ') {\n"
    "      i++;\n"
    "    }\n"
    "    while (str_in[j] == ' ') {\n"
    "      --j;\n"
    "    }\n"
    "    strncpy(str_out, str_in + i, j - i + 1);\n"
    "    str_out[j - i + 1] = '\\0'; // end flag\n"
    "  }\n"
    "};\n"
    "} // namespace mpc\n"
    "\n"
    "$ABIs$"
    "PLATON_ABI(mpc::MPC, start_calc);\n"
    "PLATON_ABI(mpc::MPC, get_owner);\n"
    "PLATON_ABI(mpc::MPC, set_result);\n"
    "PLATON_ABI(mpc::MPC, get_ir_data);\n"
    "PLATON_ABI(mpc::MPC, get_participants);\n"
    "PLATON_ABI(mpc::MPC, get_urls);\n"
    "PLATON_ABI(mpc::MPC, get_result);\n"
    "PLATON_ABI(mpc::MPC, get_status);\n"
    "PLATON_ABI(mpc::MPC, get_fee);\n"
    "PLATON_ABI(mpc::MPC, get_invitor);\n"
    "PLATON_ABI(mpc::MPC, get_url_by_id);\n"
    "\n"
    "#endif";

llvm::StringRef getSimpleName(llvm::StringRef);

string getInvitor(Document &document) {
  auto iter = document.FindMember("invitor");
  if (iter == document.MemberEnd())
    return string();
  if (iter->value.GetStringLength() != 42)
    return string();

  return iter->value.GetString();
}

string getParties(Document &document) {

  string result;
  const rapidjson::Value &parties = document["parties"];

  for (SizeType i = 0; i < parties.Size(); i++) {
    if (parties[i].GetStringLength() != 42)
      return string();

    result += parties[i].GetString();
    result += "&";
  }

  result.pop_back();
  return result;
}

string getProfitRules(Document &document) {
  string result;
  const rapidjson::Value &rules = document["profit-rules"];

  for (auto iter = rules.MemberBegin(); iter != rules.MemberEnd(); ++iter) {
    if (iter->name.GetStringLength() != 42)
      return string();

    result += iter->name.GetString();
    result += ":";
    result += to_string(iter->value.GetInt());
    result += ",";
  }

  result.pop_back();
  return result;
}

string getMethodPrice(Document &document) {
  string result;
  const rapidjson::Value &ms = document["method-price"];

  for (auto iter = ms.MemberBegin(); iter != ms.MemberEnd(); ++iter) {

    result += iter->name.GetString();
    result += "$";
    result += to_string(iter->value.GetInt());
    result += ",";
  }

  result.pop_back();
  return result;
}

string getUrls(Document &document) {
  string result;
  const rapidjson::Value &urls = document["urls"];

  for (auto iter = urls.MemberBegin(); iter != urls.MemberEnd(); ++iter) {
    if (iter->name.GetStringLength() != 42)
      return string();

    result += iter->name.GetString();
    result += "$";
    result += iter->value.GetString();
    result += ",";
  }

  result.pop_back();
  return result;
}

void GenMPCC(std::string &bc, vector<llvm::Function *> &Interfaces,
             string mpccPath, string configPath) {

  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> configBuffer =
      llvm::MemoryBuffer::getFile(configPath);

  if (std::error_code Result = configBuffer.getError()) {
    errs() << Result.message() << '\n';
    return;
  }

  Document document;
  document.Parse(configBuffer->get()->getBuffer().data());

  string Invitor = getInvitor(document);
  string Parties = getParties(document);
  string Urls = getUrls(document);
  string ProfitRules = getProfitRules(document);
  string MethodPrice = getMethodPrice(document);

  if (Invitor.empty() || Parties.empty() || Urls.empty() ||
      ProfitRules.empty() || MethodPrice.empty()) {
    llvm::errs() << "Config file error\n";
    return;
  }

  std::error_code EC;
  ToolOutputFile Out(mpccPath, EC, sys::fs::F_None);
  if (EC) {
    errs() << EC.message() << '\n';
    return;
  }

  string IRStr;
  raw_string_ostream IR(IRStr);

  for (unsigned char c : bc)
    IR << format("%.2x", c);

  IR.flush();

  MPCCTemp.replace(MPCCTemp.find("$IR_VALUE_TEMPLATE$"),
                   strlen("$IR_VALUE_TEMPLATE$"), IRStr);

  MPCCTemp.replace(MPCCTemp.find("$INVITOR_VALUE_TEMPLATE$"),
                   strlen("$INVITOR_VALUE_TEMPLATE$"), Invitor);

  MPCCTemp.replace(MPCCTemp.find("$PARTIES_VALUE_TEMPLATE$"),
                   strlen("$PARTIES_VALUE_TEMPLATE$"), Parties);

  MPCCTemp.replace(MPCCTemp.find("$URLS_VALUE_TEMPLATE$"),
                   strlen("$URLS_VALUE_TEMPLATE$"), Urls);

  MPCCTemp.replace(MPCCTemp.find("$PROFIT_RULES_VALUE$"),
                   strlen("$PROFIT_RULES_VALUE$"), ProfitRules);

  MPCCTemp.replace(MPCCTemp.find("$METHOD_PRICE_VALUE$"),
                   strlen("$METHOD_PRICE_VALUE$"), MethodPrice);

  string FStr;
  raw_string_ostream Fstream(FStr);

  for (llvm::Function *F : Interfaces) {
    llvm::StringRef Name = getSimpleName(F->getName());
    Fstream << "  void ";
    Fstream << Name;
    Fstream << "(const char *extra) {start_calc(\"";
    Fstream << Name;
    Fstream << "\", extra);}\n";
  }

  Fstream.flush();

  MPCCTemp.replace(MPCCTemp.find("$Functions$"), strlen("$Functions$"), FStr);

  string ABIStr;
  raw_string_ostream ABIstream(ABIStr);

  for (llvm::Function *F : Interfaces) {
    llvm::StringRef Name = getSimpleName(F->getName());
    ABIstream << "PLATON_ABI(mpc::MPC, ";
    ABIstream << Name;
    ABIstream << ");\n";
  }

  ABIstream.flush();

  MPCCTemp.replace(MPCCTemp.find("$ABIs$"), strlen("$ABIs$"), ABIStr);

  Out.os() << MPCCTemp;

  Out.keep();
}
