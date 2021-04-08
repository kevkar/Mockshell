#ifndef COMMAND_H
#define COMMAND_H

#include <iostream>

struct Command
{
	char* 	command_name;
	char* 	args[10];
	int 	num_args;

	Command()
	{
		command_name = nullptr;
		num_args = 1;

		for(int i = 0; i < 10; ++i) {
			args[i] = NULL;
		}

	}

	~Command()
	{
		free(command_name);

		for(int i = 0; i < num_args; ++i) {
			free(args[i]);
		}
	}
};

struct Command_Table
{
	Command* command[10];

	int 	num_cmds;

	char*	input;
	char*	output;
	char*	err_file;

	bool	append_output;
	bool	wait_for_exec;
	bool	err_stdout;

	Command_Table()
	{
		num_cmds = 0;
		input = nullptr;
		output = nullptr;
		err_file = nullptr;
		append_output = false;
		wait_for_exec = true;
		err_stdout = false;
	}

	void reset()
	{
		if (input != nullptr) 
		{ 
			free(input);
			input = nullptr;
		}
		if (output != nullptr)
		{ 
			free(output);
			output = nullptr;
		}
		if (err_file != nullptr)
		{
			free(err_file);
			err_file = nullptr;
		}

		for(int i = 0; i < num_cmds; ++i) {
			delete command[i];
		}

		num_cmds = 0;
		append_output = false;
		wait_for_exec = true;
		err_stdout = false;
	}
};

void print_command_table(Command_Table*);

void process_command_table(Command_Table*);

bool verify_command_and_args(Command*);
bool is_available(std::string);

#endif