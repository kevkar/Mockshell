%{

#include <stdio.h>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <map>
#include <algorithm>
#include "global.h"
#include "builtin.h"
#include "command.h"
#include "limits.h"

int yylex();
int yyerror(const char* s);

extern std::map<std::string,std::string> variableMap;
extern std::map<std::string,std::string> aliasTable;

%}

%union
{
	char* str;
};

%token <str> WORD ALIAS UNALIAS SETENV UNSETENV PRINTENV CD
%token GT GTGT LT PIPE AMP ERR_FILE ERR_STDOUT ESCAPE

%%
goal:
	command_line;

command:
	ALIAS					{	Command* new_cmd = new Command();
								new_cmd->command_name = $1;
								new_cmd->num_args = 0;
								cmd_tbl->command[cmd_tbl->num_cmds] = new_cmd;
								cmd_tbl->num_cmds += 1;
							}
	
	| ALIAS WORD WORD		{	Command* new_cmd = new Command();
								new_cmd->command_name = $1;
								new_cmd->args[0] = $2;
								new_cmd->args[1] = $3;
								new_cmd->num_args = 2;
								
								cmd_tbl->command[cmd_tbl->num_cmds] = new_cmd;
								cmd_tbl->num_cmds += 1;
							}

	| UNALIAS WORD			{	Command* new_cmd = new Command();
								new_cmd->command_name = $1;
								new_cmd->args[0] = $2;
								new_cmd->num_args = 1;
								
								cmd_tbl->command[cmd_tbl->num_cmds] = new_cmd;
								cmd_tbl->num_cmds += 1;
							}
	
	| SETENV WORD WORD		{	Command* new_cmd = new Command();
								new_cmd->command_name = $1;
								new_cmd->args[0] = $2;
								new_cmd->args[1] = $3;
								new_cmd->num_args = 2;
								
								cmd_tbl->command[cmd_tbl->num_cmds] = new_cmd;
								cmd_tbl->num_cmds += 1;
							}

	| UNSETENV WORD			{	Command* new_cmd = new Command();
								new_cmd->command_name = $1;
								new_cmd->args[0] = $2;
								
								cmd_tbl->command[cmd_tbl->num_cmds] = new_cmd;
								cmd_tbl->num_cmds += 1;
							}

	| PRINTENV				{	Command* new_cmd = new Command();
								new_cmd->command_name = $1;
								cmd_tbl->command[cmd_tbl->num_cmds] = new_cmd;
								cmd_tbl->num_cmds += 1;
							}

	| CD 					{	Command* new_cmd = new Command();
								new_cmd->command_name = $1;
								new_cmd->num_args = 0;
								cmd_tbl->command[cmd_tbl->num_cmds] = new_cmd;
								cmd_tbl->num_cmds += 1;
							}

	| CD WORD 				{	Command* new_cmd = new Command();
								new_cmd->command_name = $1;
								new_cmd->args[0] = parse_dir($2);
								
								cmd_tbl->command[cmd_tbl->num_cmds] = new_cmd;
								cmd_tbl->num_cmds += 1;
							}
	| CD WORD ESCAPE		{	Command* new_cmd = new Command();
								new_cmd->command_name = $1;
								new_cmd->num_args = 0;
								cmd_tbl->command[cmd_tbl->num_cmds] = new_cmd;
								cmd_tbl->num_cmds += 1;

								printf("TODO: Auto Complete for Filename/Username")

							}
	| WORD					{ 	Command* new_cmd = new Command();
								new_cmd->command_name = $1;
								cmd_tbl->command[cmd_tbl->num_cmds] = new_cmd;
								cmd_tbl->num_cmds += 1;
							}
	| command WORD			{	Command* cmd = cmd_tbl->command[cmd_tbl->num_cmds - 1];
								cmd->args[cmd->num_args] = $2;
								cmd->num_args += 1;
							}
	;


command_list:
	command_list PIPE command
	| command
	;

io_redirect:
	GT WORD					{	cmd_tbl->output = $2;
							}	
	| GTGT WORD				{	cmd_tbl->output = $2;
								cmd_tbl->append_output = true;
							}
	| LT WORD				{	cmd_tbl->input = $2;
							}
	| ERR_FILE WORD			{	cmd_tbl->err_file = $2;
							}
	| ERR_STDOUT			{	cmd_tbl->err_stdout = true;
							}
	| AMP					{	cmd_tbl->wait_for_exec = false;
							}
	;

io_redirect_list:
	io_redirect_list io_redirect
	| /* empty */
	;

command_line:
	command_list io_redirect_list
	;


%%

int yyerror(const char *s) {
	fprintf(stderr, "Error: %s\n", s);
	return 0;
}
