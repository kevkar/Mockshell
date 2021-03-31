#ifndef COMMAND_H
#define COMMAND_H

#include <iostream>
#include <vector>

struct Command {
	char* command_name;
	char* command_path;

	std::vector<char*> args;
	
	char* input;
	char* output;

	Command(char* name)
	{
		command_name = name;
		command_path = nullptr;
		input = nullptr;
		output = nullptr;
	}
};

void print_command(Command);
void execute_command(Command);

bool is_available(std::string file);
bool is_readable(std::string file);
bool is_writable(std::string file);
bool is_executable(std::string file);

#endif