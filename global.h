#ifndef GLOBAL_H
#define GLOBAL_H

#include "command.h"
#include <vector>
#include <string>

extern Command_Table* cmd_tbl;

extern std::vector<std::string> built_in_cmds;

extern bool DEBUG;

#endif