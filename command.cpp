#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <algorithm>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fstream>
#include <fcntl.h>
#include "global.h"
#include "limits.h"
#include "command.h"
#include "builtin.h"

extern std::vector<std::string> built_in_cmds;

void process_command_table(Command_Table* tbl)
{
	std::string name(tbl->command[0]->command_name);

	// If command is a built-in command, send to built-in command processor
 	if (std::find(built_in_cmds.begin(), built_in_cmds.end(), name) != built_in_cmds.end()) {
 		built_in_command_dispatcher(tbl);
 		return;
 	}

	// Validate and setup arguments for each command
	for(int i = 0; i < tbl->num_cmds; ++i)
	{
		if(!verify_command_and_args(tbl->command[i]))
		{
			return;
		}
	}

	// Stored copies of stdin/stdout file descriptors to restore after finished
	int stdin = dup(0);
	int stdout = dup(1);
	int stderr = dup(2);
	
	pid_t pid;
	int in_file;
	int out_file;
	int e_file;

	// Redirect stderr if file is set
	if (tbl->err_file != nullptr)
	{
		// Check if files exists or is writable
		if (access(tbl->err_file, F_OK) != 0 || access(tbl->err_file, W_OK) == 0) {
			e_file = open(tbl->err_file, O_WRONLY | O_CREAT | O_TRUNC);
			
			// Redirect stderr to file
			dup2(e_file, 2);
			close(e_file);
		}
		else
		{
			std::cout << "ERROR: User does not have write permission to file \'" << tbl->output << "\'" << std::endl;
			return;
		}
	}


	// Setup input if there in a file, else set as stdin
	if (tbl->input != nullptr)
	{
		// Check if file exists
		if (access(tbl->input, F_OK) != 0) {
			std::cout << "ERROR: Input file \'" << tbl->input << "\' does not exist" << std::endl;
			return;
		}
		// Check if file is readable
		else if (access(tbl->input, R_OK) != 0){
			std::cout << "ERROR: User does not have read access to input file \'" << tbl->input << "\'" << std::endl;
			return;
		}
		// Set file descriptor to input file
		else {
			in_file = open(tbl->input, O_RDONLY);
		}
	}
	else
	{
		in_file = dup(stdin);
	}



	// For each command in table, redirect input/output then execute command
	for(int i = 0; i < tbl->num_cmds; ++i)
	{	
		// Redirect Command Input (from file or previous command)
		dup2(in_file, 0);
		close(in_file);

		// If last command, redirect output
		if(i == tbl->num_cmds - 1) {
			// Check if an output file was set
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

					// Redirect stderr to same output if 2>&1 parameter present
					if(tbl->err_stdout)
					{	
						dup2(out_file, 2);
					}
					
					close(out_file);
				}
				else
				{
					std::cout << "ERROR: User does not have write permission to file \'" << tbl->output << "\'" << std::endl;
					return;
				}
			}
			// If no input file was specified, set output to stdout
			else
			{
				out_file = dup(stdout);
			}
		}
		// If not last command
		else
		{
			// Create pipe and redirect input/output
			int cmd_pipe[2];
			if(pipe(cmd_pipe) < 0)
			{
				std::cout << "ERROR: Failed to create pipe" << std::endl;
				return;
			}

			in_file = cmd_pipe[0];
			out_file = cmd_pipe[1];
		}

		// Redirect comand output (to file or pipe to other command)
		dup2(out_file, 1);
		close(out_file);

		// Create a child process
		pid = fork();

		// If child process, run command
		if (pid == 0)
		{
			if (execv(tbl->command[i]->args[0], tbl->command[i]->args) < 0)
				{
					// Exit if error occurs executing command
					exit(0);
				}
		}
	}

	// Parent Process
	// Restore stdin and stdout, close input/output to end command	
	dup2(stdin, 0);
	dup2(stdout, 1);
	dup2(stderr, 2);
	close(in_file);
	close(out_file);
	close(e_file);

	// If backgrounding not set, wait for children to finish
	if(tbl->wait_for_exec)
	{
		int status;
		waitpid(pid, &status, 0);
	}

	return;
}


// Function verifies comand exists and user has permission to execute,
// and adds valid path to command struct and args[0]
bool verify_command_and_args(Command* c)
{
	std::string name(c->command_name);

	// If command starts with . replace it with current current directory
	if (name.at(0) == '.')
	{
		c->command_name = parse_dot(c->command_name);
		name = c->command_name;
	}

	// If command starts with /, check and set as command path
	if (name.at(0) == '/')
	{
		// If command exists at location, add to arguments array
		if (is_available(name))
		{
			char* path = (char*)malloc(name.length() + 1);
			strcpy(path, name.c_str());
			c->args[0] = path;
		}
		else
		{
			std::cout << "Error: Command not found at: " << name << std::endl;
			return false;
		}
	}

	// Search for command using PATH directories
	else
	{
		std::string p = variableMap["PATH"];
		std::stringstream paths(p);
		std::string test_path;

		while (std::getline(paths, test_path, ':'))
		{
			if(test_path.at(0) == '.')
			{
				test_path = parse_dot(strdup(test_path.c_str()));
			}

			test_path = test_path + "/" + name;

			// Returns 0 if file is accessible, -1 if not
			if (is_available(test_path))
			{
				// If command exists, put command path in argument array
				char* path = (char*)malloc(test_path.length() + 1);
				strcpy(path, test_path.c_str());
				c->args[0] = path;

				break;
			}
		}
		
		// If not valid path for command was found, print error
		if (c->args[0] == nullptr)
		{
			std::cout << "Error: Command not found in PATH directories" << std::endl;
			return false;
		}
	}

	// Check if user has permission to execute command
	if (access(c->args[0], X_OK) != 0)
	{
		std::cout << "Error: User does not have permission to execute command" << std::endl;
		return false;
	}

	return true;
}


// Returns true if the provide string points to an existing file
bool is_available(std::string file)
{
	return access(file.c_str(), F_OK) == 0;
}


// Debugging fuction to print a command (remove from final)
void print_command_table(Command_Table* tbl)
{
	std::cout << std::endl;
	std::cout << "Command Table with " << tbl->num_cmds << " Command(s):" << std::endl;
	
	std::cout << " -> Input file: ";
	if(tbl->input != nullptr) { std::cout << tbl->input << std::endl; }
	else { std::cout << "None" << std::endl; }

	std::cout << " -> Output file: ";
	if(tbl->output != nullptr) { std::cout << tbl->output << std::endl; }
	else { std::cout << "None" << std::endl; }

	std::cout << "  -> Append Output? ";
	if(tbl->append_output) { std::cout << "Yes" << std::endl; }
	else { std::cout << "No" << std::endl; }

	std::cout << " -> Error file: ";
	if(tbl->err_file != nullptr) { std::cout << tbl->err_file << std::endl; }
	else { std::cout << "None" << std::endl; }

	std::cout << "  -> Errors to stdout? ";
	if(tbl->err_stdout) { std::cout << "Yes" << std::endl; }
	else { std::cout << "No" << std::endl; }

	std::cout << " -> Background execution? ";
	if(tbl->wait_for_exec) { std::cout << "No" << std::endl; }
	else { std::cout << "Yes" << std::endl; }

	std::cout << std::endl;

	for(int i = 0; i < tbl->num_cmds; ++i)
	{
		Command* c = tbl->command[i];
		std::cout << "Command " << i << ": " << c->command_name << std::endl;

		for(int j = 0; j < 10; ++j)
		{
			if (c->args[j] == nullptr)
			{
				std::cout << " -> Argument " << j << ": (Null)" << std::endl;
			}
			else {
				std::cout << " -> Argument " << j << ": " << c->args[j] << std::endl;
			}
		}
	}

	std::cout << std::endl;

	return;
}

void print_commands(Command_Table* tbl)
{
	for(int i = 0; i < tbl->num_cmds; ++i)
	{
		Command* c = tbl->command[i];
		std::cout << c->command_name;

		for(int j = 0; j < 10; ++j)
		{
			if (c->args[j] != nullptr)
			{
				std::cout << " " << c->args[j];
			}
		}

		if(i+1 < tbl->num_cmds)
		{
			std::cout << " | ";
		}

	}
	
	if(tbl->input != nullptr)
	{
		std::cout << " < " << tbl->input;
	}

	if(tbl->output != nullptr)
	{ 
		if(tbl->append_output)
		{
			std::cout << " >> " << tbl->output;
		}
		else
		{
			std::cout << " > " << tbl->output;
		}
	}

	if(tbl->err_file != nullptr)
	{
		std::cout << " 2>" << tbl->err_file;
	}

	if(tbl->err_stdout)
	{ 
		std::cout << " 2>&1";
	}

	if(!tbl->wait_for_exec)
	{ 
		std::cout << " &";
	}

	return;
}
