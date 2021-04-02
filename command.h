#ifndef COMMAND_H
#define COMMAND_H

#include <iostream>
#include <vector>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <unistd.h>
#include <algorithm>
#include <sys/types.h>
#include <sys/wait.h>
#include <map>
#include "Alias.h"

using namespace std;

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
void execute_built_in(std::string name,std::vector<char*> args);

//parsing
const char* do_parse(const char* word);
char* tilde_parse(char* word);
char* dot_dot_parse(char* word);
char* one_dot_parse(char* word);
bool has_valid_env_variable(char* word);
char* transform_env_variable(char* word);


//built in commands
void setEnvironmentVariable(const char* variable, const char* value);
void printEnvVariables();
void unsetEnvironmentVariable(const char* variable);
void changeDirectory(const char* directory);
void changeDirectoryToHome();
void printAliases();
void doAlias(std::string name, std::vector<char*> args);
void setAlias(std::vector<char*> args);
char* transform_alias(char* name);
void unAlias(std::vector<char*> args);
bool wouldMakeInfiniteLoop(char* name, int depth);

bool is_available(std::string file);
bool is_readable(std::string file);
bool is_writable(std::string file);
bool is_executable(std::string file);

#endif