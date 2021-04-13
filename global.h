#ifndef GLOBAL_H
#define GLOBAL_H

#include "command.h"
#include <vector>
#include <string>
#include <map>

extern Command_Table* cmd_tbl;

extern std::vector<std::string> built_in_cmds;
extern std::map<std::string,std::string> variableMap;
extern std::map<std::string,std::string> aliasTable;

extern bool DEBUG;

#endif
