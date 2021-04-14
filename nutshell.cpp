#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "global.h"
#include "ns_parser.tab.h"
#include "command.h"
#include "builtin.h"

extern int yyparse();

char* PATH;
char* HOME;

std::vector<std::string> built_in_cmds;
std::map<std::string,std::string> variableMap;
std::map<std::string,std::string> aliasTable;

Command_Table* cmd_tbl;

void shell_init();

int main()
{
	shell_init();

	while(1)
	{
		std::cout << ">> ";
		yyparse();

		// Temp fix for ENDLINE parsing error (Jon will fix)
		std::cout << std::endl;

		process_command_table(cmd_tbl);

		// After command is finished executing, clear command table and free memory
		cmd_tbl->reset();
	}

	return 0;
}

void shell_init() 
{
	cmd_tbl = new Command_Table();

	HOME = getenv("HOME");
	PATH = getenv("PATH");

	variableMap["HOME"] = HOME;
	variableMap["PATH"] = PATH;

	built_in_cmds.push_back("setenv");
	built_in_cmds.push_back("printenv");
	built_in_cmds.push_back("unsetenv");
	built_in_cmds.push_back("cd");
	built_in_cmds.push_back("alias");
	built_in_cmds.push_back("unalias");
}
