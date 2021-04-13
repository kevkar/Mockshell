#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include "global.h"
#include "ns_parser.tab.h"
#include "command.h"

extern int yyparse();

char* PATH;
char* HOME;

Command_Table* cmd_tbl;

std::vector<std::string> built_in_cmds;


void shell_init();

int main()
{
	shell_init();

	while(1)
	{
		std::cout << ">> ";
		yyparse();

		process_command_table(cmd_tbl);
		cmd_tbl->reset();
	}

	return 0;
}

void shell_init() 
{

	cmd_tbl = new Command_Table();

	HOME = getenv("HOME");
	PATH = getenv("PATH");

	built_in_cmds.push_back("setenv");
	built_in_cmds.push_back("printenv");
	built_in_cmds.push_back("unsetenv");
	built_in_cmds.push_back("cd");
	built_in_cmds.push_back("alias");
	built_in_cmds.push_back("unalias");
}