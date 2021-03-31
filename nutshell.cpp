#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include "ns_parser.tab.h"
#include "command.h"

extern int yyparse();

char* PATH;
char* HOME;

std::vector<Command> command_table;
std::vector<std::string> built_in_cmds;

void shell_init();

int main()
{
	shell_init();

	while(1) 
	{
		std::cout << std::endl << ">> ";
		yyparse();

		// Print and clear contents of the command table
		for(int i = 0; i < command_table.size(); ++i)
		{
			//print_command(command_table[i]);
			execute_command(command_table[i]);
		}
		command_table.clear();
	}

	return 0;
}

void shell_init() 
{
	HOME = getenv("HOME");
	PATH = getenv("PATH");

	built_in_cmds.push_back("setenv");
	built_in_cmds.push_back("printenv");
	built_in_cmds.push_back("unsetenv");
	built_in_cmds.push_back("cd");
	built_in_cmds.push_back("alias");
	built_in_cmds.push_back("unalias");
}