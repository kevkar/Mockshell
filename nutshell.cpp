#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include "global.h"
#include "ns_parser.tab.h"
#include "command.h"
#include <map>

extern int yyparse();

char* PATH;
char* HOME;

bool DEBUG;

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

		// DEBUG: Print Command Table
		if (DEBUG) { print_command_table(cmd_tbl); }
		if (DEBUG) { std::cout << std::endl << std::endl; }

		if(DEBUG) { std::cout << "----- Starting Command -----" << std::endl << std::endl; }

		process_command_table(cmd_tbl);

		if(DEBUG) { std::cout << "----- Command Finished -----" << std::endl << std::endl; }

		cmd_tbl->reset();
	}

	return 0;
}

void shell_init() 
{
	DEBUG = true;

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
