#include <stdio.h>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <map>
#include <algorithm>
#include <iostream>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <sstream>
#include "global.h"
#include "builtin.h"
#include "command.h"
#include "limits.h"
#include "ns_parser.tab.h"

extern std::vector<std::string> built_in_cmds;
extern std::map<std::string,std::string> variableMap;
extern std::map<std::string,std::string> aliasTable;

// COMMAND TO RETURN TO PARSER -> PRINTENV
// printEnvVariables();

void built_in_command_dispatcher(Command_Table *tbl)
{
	Command* c = tbl->command[0];
	std::string name(c->command_name);

	if (name == "alias")
	{
		if(c->num_args == 0)
		{
			print_aliases(tbl);
		}
		else
		{
			set_alias(c->args[0], c->args[1]);
		}
	}
	else if (name == "unalias")
	{
		remove_alias(c->args[0]);
	}
	else if (name.compare("cd") == 0)
	{
		if(c->num_args == 0)
		{
			change_directory_to_home();
		}
		else
		{
			change_directory(c->args[0]);
		}
	}
	else if (name == "printenv")
	{
		print_env_variables(tbl);
	}
	else if (name == "setenv")
	{
		set_env_variable(c->args[0], c->args[1]);
	}
	else if (name == "unsetenv")
	{
		unset_env_variable(c->args[0]);
	}
	else
	{
		std::cout << "ERROR: Unable to identify built-in command" << std::endl;
	}
	return;
}


// Prints all current aliases to stdout or file (if specified)
void print_aliases(Command_Table *tbl)
{
	int stdout = dup(1);
	int out_file;

	// If an output file has been provided, set stdout to it 
	if (tbl->output != nullptr)
	{
		// Check if files exists or is writable
		if (access(tbl->output, F_OK) != 0 || access(tbl->output, W_OK) == 0) {
			// If appending to end of existing file
			if (tbl->append_output)
			{
				out_file = open(tbl->output, O_WRONLY | O_APPEND);
			}
			// Else open existing file to write over, or create if it doesn't exist
			else
			{
				out_file = open(tbl->output, O_WRONLY | O_CREAT | O_TRUNC);
			}
			// Redirect output from stdout to opened file
			dup2(out_file, 1);
			close(out_file);
		}
		else
		{
			std::cout << "ERROR: User does not have write permission to file \'" << tbl->output << "\'" << std::endl;
			return;
		}
	}

	if (aliasTable.size() == 0)
	{
		std::cout << "No aliases to show!" << std::endl;
		return;
	}
		
	for (auto iter = aliasTable.begin(); iter != aliasTable.end(); iter++)
		std::cout << iter->first << "=" << iter->second << std::endl;

	dup2(stdout, 1);
	close(out_file);

	return;
}

// Create alias from provided values, checking for loops
void set_alias(std::string key, std::string value)
{
	// Check if alias currently exists
	std::string existingValue = "0";
	std::map<std::string, std::string>::iterator itr = aliasTable.find(key);

	if(itr != aliasTable.end())
	{
		existingValue = itr->second;
	}

	aliasTable[key] = value;

	if (would_make_infinite_loop(strdup(value.c_str()), 10))
	{
		std::cout << "ERROR: Adding alias would create an infinite loop!" << std::endl;
		aliasTable.erase(key);

		// If key had existing value, restore it
		if (existingValue != "0")
		{
			aliasTable[key] = existingValue;
		}
		
		return;
	}

	// TODO: Why is this called here? What does it do?
	std::string nested_command(transform_alias(strdup(value.c_str())));
	return;
}

// TODO: What does this function do? Was this for checking infinite loops?
char* transform_alias(char* name)
{
	std::string word(name);

	for (auto iter = aliasTable.begin(); iter != aliasTable.end(); iter++)
		if (iter->first == word)
			return transform_alias(strdup(iter->second.c_str()));
	
	return name;
}

// Repeatedly evaluates an alias to determine if it is part of a looping definition
bool would_make_infinite_loop(char* name, int depth)
{
	if (depth == 0)
		return true;

	std::string word(name);

	for (auto iter = aliasTable.begin(); iter != aliasTable.end(); iter++)
		if (iter->first == word)
			return would_make_infinite_loop(strdup(iter->second.c_str()), depth - 1);

	return false;
}

// Remove alias from dictionary if it exists
void remove_alias(std::string key)
{	
	bool found = false;

	for (auto iter = aliasTable.begin(); iter != aliasTable.end(); iter++)
		if (iter->first == key)
			found = true;
	
	if (found == false)
	{
		std::cout << "No alias with name \'" << key << "\' found!" << std::endl;
		return;
	}
		
	aliasTable.erase(key);
	return;
}


void change_directory(char* directory)
{
	const char* dir = directory;

	if (directory[0] == '/')
	{
		int error = chdir(dir);
		if (error != 0)
			std::cout << "ERROR: Directory \'" << directory << "\'' not found." << std::endl;
		return;
	}

	char cwd[PATH_MAX];
	getcwd(cwd, sizeof(cwd));
	strcat(cwd, "/");
	const char* newDir = strcat(cwd, dir);
	
	int error = chdir(newDir);
	if (error != 0)
		std::cout << "ERROR: directory \'" << newDir << "\'' not found." << std::endl;

	return;
}


void change_directory_to_home()
{
	char* home = strdup(variableMap["HOME"].c_str());
	chdir(home);
	return;
}


char* parse_dir(char* input)
{
	std::string word(input);

	if (word[0] == '~')
	{
		char* output = parse_tilde(input);
		return output;
	}
	else if (word.length() > 1 && word[0] == '.' && word[1] == '.' )
	{
		char* output = parse_dot_dot(input);
		return output;
	}
	else if (word[0] == '.')
	{
		char* output = parse_dot(input);
		return output;
	}

	char* output = (char*)malloc(word.length() + 1);
	strcpy(output, word.c_str());

	return output;
}

char* parse_dot(char* input)
{
	char currentDirectory[PATH_MAX];
	getcwd(currentDirectory, sizeof(currentDirectory));

	std::string cwd(currentDirectory);
	std::string input_str(input);

	if (input_str.length() > 1)
	{
		input_str = input_str.substr(1);
		cwd += input_str;
	}
	
	char* output = (char*)malloc(cwd.length() + 1);
	strcpy(output, cwd.c_str());

	free (input);

	return output;
}

char* parse_dot_dot(char* input)
{
	char currentDirectory[PATH_MAX];
	getcwd(currentDirectory, sizeof(currentDirectory));

	// cwd = /user/jonsessa/nutshell/test/test2
	std::string cwd(currentDirectory);
	std::string input_str(input);
	std::string slash = "/";

	int index = cwd.find_last_of(slash);
	
	// CWD is root
	if (index == 0)
		return strdup(slash.c_str());

	// Remove last directory from cwd
	cwd = cwd.substr(0, index);

	// Append input string to cwd if more than just ".."
	if (input_str.length() > 2)
		cwd += input_str.substr(2);
	
	char* output = (char*)malloc(cwd.length() + 1);
	strcpy(output, cwd.c_str());

	free (input);

	return output;
}


// KNOWN ERROR: Providing an invalid username crashes programa
char* parse_tilde(char* input)
{
	std::string input_str(input);
	std::string username;
	std::string dir;

	// Extract substring between tilde and slash (if present)
	int i = input_str.find("/");

	// If a slash was found, get substring after ~ and before /
	if (i != std::string::npos)
	{
		username = input_str.substr(1, i - 1);
	}
	// If no slash was found, get substring after ~ to end
	else
	{
		username = input_str.substr(1);
	}

	// Input is ~ or ~/, return user's HOME directory with appended content
	if (username.length() == 0)
	{
		dir = variableMap["HOME"];
	}
	// Look up user directory and return
	else
	{
		// Access /etc/passwd file to get home directory for given username
		struct passwd* pw = getpwnam(username.c_str());

		// Check for errors
		if(pw == 0)
		{
			std::cout << "ERROR: User \'" << username << "/' not found" << std::endl;
			return 0;
		}

		// Get home directory from returned struct
		std::string userDir(pw->pw_dir);

		dir = userDir;
	}

	// Append directories from original input after /
	if (i > 0)
	{
		dir += input_str.substr(i);
	}

	char* output = (char*)malloc(dir.length() + 1);
	strcpy(output, dir.c_str());

	free(input);
	
	return output;
}


// TODO: Add Logic Here to parse values for ~, expand if present

// Adds given variable and associated value to the dictionary
void set_env_variable(std::string variable, std::string value)
{
	if(variable == "PATH") {
		std::string p = value;
		std::stringstream paths(p);
		std::string test_path;
		std::string newValue = "";

		// Check each path between : to see if tilde expansion is necessary
		while (std::getline(paths, test_path, ':'))
		{
			if(test_path.at(0) == '~')
			{
				test_path = parse_tilde(strdup(test_path.c_str()));
			}

			newValue += test_path + ":";
		}
		// Remove last :
		newValue.pop_back();

		variableMap[variable] = newValue;
	}
	else
	{
		variableMap[variable] = value;
	}
	
	return;
}

// Removes given variable from dictionary unless it doesn't exists or is PATH/HOME
void unset_env_variable(std::string variable)
{
	if (variable == "PATH" || variable == "HOME")
	{
		std::cout << "ERROR: Cannot unset " << variable << " variable!" << std::endl;
		return;
	}

	bool found = false;
	for (auto iter = variableMap.begin(); iter != variableMap.end(); iter++)
		if (iter->first == variable)
			found = true;

	if (found == false)
	{
		std::cout << "ERROR: No variable with name \'" << variable << "\' exists." << std::endl;
		return;
	}

	variableMap.erase(variable);
	return;
}

// Prints all variables in dictionary to stdout or file (if specified)
void print_env_variables(Command_Table* tbl)
{
	int stdout = dup(1);
	int out_file;

	// If an output file has been provided, set stdout to it 
	if (tbl->output != nullptr)
	{
		// Check if files exists or is writable
		if (access(tbl->output, F_OK) != 0 || access(tbl->output, W_OK) == 0) {
			// If appending to end of existing file
			if (tbl->append_output)
			{
				out_file = open(tbl->output, O_WRONLY | O_APPEND);
			}
			// Else open existing file to write over, or create if it doesn't exist
			else
			{
				out_file = open(tbl->output, O_WRONLY | O_CREAT | O_TRUNC);
			}
			// Redirect output from stdout to opened file
			dup2(out_file, 1);
			close(out_file);
		}
		else
		{
			std::cout << "ERROR: User does not have write permission to file \'" << tbl->output << "\'" << std::endl;
			return;
		}
	}

	// Print variables
	for (auto iter = variableMap.begin(); iter != variableMap.end(); iter++ )
	{
		std::cout << iter->first << "=" << iter->second << std::endl;
	}

	// Restore stdout and close output file
	dup2(stdout, 1);
	close(out_file);

	return;
}





