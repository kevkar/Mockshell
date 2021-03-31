#include "command.h"
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

extern std::vector<std::string> built_in_cmds;

// Debugging fuction to print a command (remove from final)
void print_command(Command c)
{
	std::cout << "Command name: " << c.command_name << std::endl;

	if(c.args.size() > 0) {
		std::cout << "Arguments:" << std::endl;
		for(int i = 0; i < c.args.size(); i++)
		{
			std::cout << " -> " << c.args[i];
		}
		std::cout << std::endl;
	}

	if(c.input != nullptr) {
		std::cout << "Input File: " << c.input << std::endl;
	}

	if(c.output != nullptr) {
		std::cout << "Output File: " << c.output << std::endl;
	}

	return;
}


void execute_command(Command cmd)
{
	std::string name(cmd.command_name);

	if (std::find(built_in_cmds.begin(), built_in_cmds.end(), name) != built_in_cmds.end()) {
		std::cout << "Built-in Command Found!" << std::endl;
		return;
	}

	std::string p(getenv("PATH"));
	std::stringstream paths(p);
	std::string test_path;

	while (std::getline(paths, test_path, ':'))
	{
		test_path = test_path + "/" + cmd.command_name;

		// Returns 0 if file is accessible, -1 if not
		if (is_available(test_path))
		{
			cmd.command_path = (char*)malloc(test_path.length() + 1);
			strcpy(cmd.command_path, test_path.c_str());

			break;
		}
	}
	
	if (cmd.command_path == nullptr)
	{
		std::cout << "Error: Command not found in PATH" << std::endl;
		return;
	}

	// Fork child process
	pid_t pid;
	pid = fork();

	if (pid < 0)
	{
		std::cout << "Unable to fork new process" << std::endl;
		return;
	}
	// If child process
	else if (pid == 0)
	{
		// Create argument array for execution
		int n = cmd.args.size() + 2;
		char* args[n];

		args[0] = cmd.command_path;

		for(int i = 1; i <= cmd.args.size(); i++) {
			args[i] = cmd.args[i-1];
		}

		args[n-1] = NULL;


		// If there is an output file for the command
		if (cmd.output != nullptr) {
			// If file doesn't exists create it, if it does exist check for write permission
			if (access(cmd.output, F_OK) != 0 || access(cmd.output, W_OK) == 0) {
				// Changes file descriptor 
				int fd = open(cmd.output, O_WRONLY | O_CREAT | O_TRUNC);
				dup2(fd, 1);
				close(fd);
			}
			else
			{
				std::cout << "ERROR: User does not have write permission to file \'" << cmd.output << "\'" << std::endl;
			}
		}


		// If there is an input file for the command
		if (cmd.input != nullptr) {
			// Check if file is accessible, then if it is readable
			if (access(cmd.input, F_OK) != 0) {
				std::cout << "ERROR: Input file \'" << cmd.input << "\' does not exist" << std::endl;
			}
			else if (access(cmd.input, R_OK) != 0){
				std::cout << "ERROR: User does not have read access to Input file \'" << cmd.input << "\'" << std::endl;
			}
			else {
				// Changes file descriptor 
				int fd = open(cmd.input, O_RDONLY);
				dup2(fd, 0);
				close(fd);
			}
		}


		// Execute command
		execv(args[0], args);

		// Exit child process
		exit(0);
	}
	// If parent Process
	else {
		// Wait for child process to finish
		wait(0);
	}
	return;


	// TO DO:
	// Input Files
	// Pipes to other commands
	// Processing in background (if & present)

}



bool is_available(std::string file)
{
	return access(file.c_str(), F_OK) == 0;
}

bool is_readable(std::string file)
{
	return access(file.c_str(), R_OK) == 0;
}

bool is_writeable(std::string file)
{
	return access(file.c_str(), W_OK) == 0;
}

bool is_executable(std::string file)
{
	return access(file.c_str(), X_OK) == 0;
}


/* Refereneces Used
https://www.geeksforgeeks.org/tokenizing-a-string-cpp/
https://www.geeksforgeeks.org/access-command-in-linux-with-examples/

I/O Redirects
https://homepages.uc.edu/~thomam/Intro_Unix_Text/IO_Redir_Pipes.html
http://www.rozmichelle.com/pipes-forks-dups/
*/