%{
#include <stdio.h>
#include <vector>
#include "command.h"
#include "global.h"

int yylex();
int yyerror(const char* s);


%}

%union
{
	char* str;
};

%token <str> WORD
%token GT GTGT LT PIPE AMP ERR_FILE ERR_STDOUT

%%
goal:
	command_line;

command:
	WORD					{ 	Command* new_cmd = new Command();
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
