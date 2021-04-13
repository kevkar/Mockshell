#include "global.h"
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


int process_command_table()
{
	// Only one command to execute
	if (num_cmds == 1)
	{
		Command* c = command_table[0];

		if(!verify_command(command_table[0]))		
		{ 
			std::cout << "ERROR: Failure to verify command" << std::endl;
			return -1;
		}

		pid_t pid = fork();

		if (pid == 0)
		{
			if (c->input != nullptr)
			{
				// Check if file is accessible and readable
				if (access(c->input, F_OK) != 0) {
					std::cout << "ERROR: Input file \'" << c->input << "\' does not exist" << std::endl;
					return false;
				}
				else if (access(c->input, R_OK) != 0){
					std::cout << "ERROR: User does not have read access to input file \'" << c->input << "\'" << std::endl;
					return false;
				}
				else {
					// Redirect input from stdin to file
					int fd = open(c->input, O_RDONLY);
					dup2(fd, 0);
					close(fd);
				}
			}

			if (c->output != nullptr)
			{
				// If file doesn't exists create it, if it does exist check for write permission
				if (access(c->output, F_OK) != 0 || access(c->output, W_OK) == 0)
				{
					// Redirect output from stdout to file 
					int fd = open(c->output, O_WRONLY | O_CREAT | O_TRUNC);
					dup2(fd, 1);
					close(fd);
				}
				else
				{
					std::cout << "ERROR: User does not have write permission to file \'" << c->output << "\'" << std::endl;
					return false;
				}
			}
		
			return execv(command_table[0]->args[0], command_table[0]->args);
		}

		else 
		{
			wait(NULL);
			return pid;
		}
	}

	/* Two or more commands to execute
	for(int i = 0; i < num_cmds; ++i)
	{
		// First Command
		if(i == 0)
		{

		}
		// Last Command
		else if (i == num_cmds - 1)
		{

		}
		// Middle Command
		else
		{

		}

	}
	*/

	return 0;

}

// Redirect the input of a command if a file was indicated
bool redirect_input(Command* c)
{
	if (c->input != nullptr) {
		
		// Check if file is accessible and readable
		if (access(c->input, F_OK) != 0) {
			std::cout << "ERROR: Input file \'" << c->input << "\' does not exist" << std::endl;
			return false;
		}
		else if (access(c->input, R_OK) != 0){
			std::cout << "ERROR: User does not have read access to input file \'" << c->input << "\'" << std::endl;
			return false;
		}
		else {
			// Redirect input from stdin to file
			int fd = open(c->input, O_RDONLY);
			dup2(fd, 0);
			close(fd);
		}
	}

	return true;
}

// Redirect the output of a command if a file was indicated
bool redirect_output(Command* c)
{
	if (c->output != nullptr) {

		// If file doesn't exists create it, if it does exist check for write permission
		if (access(c->output, F_OK) != 0 || access(c->output, W_OK) == 0) {
			// Redirect output from stdout to file 
			int fd = open(c->output, O_WRONLY | O_CREAT | O_TRUNC);
			dup2(fd, 1);
			close(fd);
		}
		else
		{
			std::cout << "ERROR: User does not have write permission to file \'" << c->output << "\'" << std::endl;
			return false;
		}
	}	

	return true;
}

/*
int execute_command(Command* c, int pipe_in, int pipe_out)
{
	// Fork child process
	pid_t pid = fork();

	// Check for fork() error
	if (pid < 0)
	{
		std::cout << "Unable to fork new process" << std::endl;
		return 0;
	}

	// If child process
	if (pid == 0)
	{
		if (pipe_in != 0)
		{
			dup2(pipe_in, 0);
			close(pipe_in);
		}

		if (pipe_out != 1)
		{
			dup2(pipe_out, 1);
			close(pipe_out);
		}
		
		// Execute command
		return execv(c->args[0], c->args);
	}
	
	// Parent process
	wait(NULL);
	return pid;
}
*/

// Debugging fuction to print a command (remove from final)
void print_command(Command* c)
{
	std::cout << "Command name: " << c->command_name << std::endl;

	if(c->num_args > 0) {
		std::cout << "Arguments: ";
		for(int i = 0; i <= c->num_args; i++)
		{
			printf("%s ", c->args[i]);
		}
		std::cout << std::endl;
	}

	if(c->input != nullptr) {
		std::cout << "Input File: " << c->input << std::endl;
	}

	if(c->output != nullptr) {
		std::cout << "Output File: " << c->output << std::endl;
	}

	return;
}

void execute_two_commands(Command* c1, Command* c2) {

	if(!verify_command(c1))
	{
		std::cout << "First command fail" << std::endl;
		return;
	}
	if(!verify_command(c2))
	{
		std::cout << "Second command fail" << std::endl;
		return;
	}

	// Create pipe input/output between c1 and c2
	int pipe1[2];

	if(pipe(pipe1) < 0)
	{
		std::cout << "Failed to create pipe" << std::endl;
		return;
	};

	pid_t pid;
	pid = fork();

	// First Child Process (first command)
	if (pid == 0)
	{		
		
		// Add Argument array creator
		char* c1_args[2];
		c1_args[0] = c1->command_path;
		c1_args[1] = NULL;

		// Add Input File

		// Change write of process from stdout to pipe
		dup2(pipe1[1], 1);
		close(pipe1[0]);

		// Execute command and exits fork
		execv(c1_args[0], c1_args);
	}

	pid = fork();

	if(pid == 0) {

		// Create argument array for execution
		char* c2_args[2];
		c2_args[0] = c2->command_path;
		c2_args[1] = NULL;

		// Create output files

		// Change input to pipe read segment (fd[0]) from stdin
		dup2(pipe1[0], 0);
		close(pipe1[1]);

		// Execute command and exits fork
		execv(c2_args[0], c2_args);

	}

	int status;
	close(pipe1[0]);
	close(pipe1[1]);
	
	// Wait for children to finish
	waitpid(pid, &status, 0);
	
	return;
}

// Function verifies comand exists and user has permission to execute,
// and adds valid path to command struct and args[0]
bool verify_command(Command* cmd)
{
	std::string name(cmd->command_name);

	// Check if provided command is a built-in command
	if (std::find(built_in_cmds.begin(), built_in_cmds.end(), name) != built_in_cmds.end())
	{
		std::cout << "Built-in Command Found! Now what?" << std::endl;
		return true;
	}

	// If command starts with . or /, set as command path
	if (name.at(0) == '.' || name.at(0) == '/')
	{
		// If command exists at location, add to arguments array
		if (is_available(name))
		{
			cmd->command_path = (char*)malloc(name.length() + 1);
			strcpy(cmd->command_path, name.c_str());
			cmd->args[0] = cmd->command_path;
		}
		else
		{
			std::cout << "Error: Command not found at: " << cmd->command_name << std::endl;
			return false;
		}
	}

	// Search for command using PATH directories
	else
	{
		std::string p(getenv("PATH"));
		std::stringstream paths(p);
		std::string test_path;

		while (std::getline(paths, test_path, ':'))
		{
			test_path = test_path + "/" + name;

			// Returns 0 if file is accessible, -1 if not
			if (is_available(test_path))
			{
				// If command exists, put command path in argument array
				cmd->command_path = (char*)malloc(test_path.length() + 1);
				strcpy(cmd->command_path, test_path.c_str());
				cmd->args[0] = cmd->command_path;

				break;
			}
		}
		
		// If not valid path for command was found, print error
		if (cmd->command_path == nullptr)
		{
			std::cout << "Error: Command not found in PATH directories" << std::endl;
			return false;
		}
	}

	// Check if user has permission to execute command
	if (access(cmd->command_path, X_OK) != 0)
	{
		std::cout << "Error: User does not have permission to execute command" << std::endl;
		return false;
	}

	return true;
}


void execute_command(Command* cmd)
{
	if(!verify_command(cmd))
	{
		return;
	}

	// Fork child process
	pid_t pid = fork();

	// Check for fork() error
	if (pid < 0)
	{
		std::cout << "Unable to fork new process" << std::endl;
		return;
	}
	// If child process
	else if (pid == 0)
	{
		// Create argument array for execution
		int n = cmd->num_args + 2;
		char* c_args[n];

		c_args[0] = cmd->command_path;

		for(int i = 1; i <= cmd->num_args; i++) {
			c_args[i] = cmd->args[i-1];
		}

		c_args[n-1] = NULL;


		// If there is an output file for the command
		if (cmd->output != nullptr) {
			// If file doesn't exists create it, if it does exist check for write permission
			if (access(cmd->output, F_OK) != 0 || access(cmd->output, W_OK) == 0) {
				// Redirect output from stdout to file 
				int fd = open(cmd->output, O_WRONLY | O_CREAT | O_TRUNC);
				dup2(fd, 1);
				close(fd);
			}
			else
			{
				std::cout << "ERROR: User does not have write permission to file \'" << cmd->output << "\'" << std::endl;
			}
		}


		// If there is an input file for the command
		if (cmd->input != nullptr) {
			// Check if file is accessible, then if it is readable
			if (access(cmd->input, F_OK) != 0) {
				std::cout << "ERROR: Input file \'" << cmd->input << "\' does not exist" << std::endl;
			}
			else if (access(cmd->input, R_OK) != 0){
				std::cout << "ERROR: User does not have read access to input file \'" << cmd->input << "\'" << std::endl;
			}
			else {
				// Redirect input from stdin to file
				int fd = open(cmd->input, O_RDONLY);
				dup2(fd, 0);
				close(fd);
			}
		}


		// Execute command
		execv(c_args[0], c_args);

		// Exit child process
		exit(0);
	}
	// If parent Process
	else {
		// Wait for child process to finish
		wait(0);
	}
	return;
}



bool is_available(std::string file)
{
	return access(file.c_str(), F_OK) == 0;
}


/*  FILE DESCRIPTORS
    0 = stdin
    1 = stdout
    2 = stderr
*/

/* PIPES
	int pipe(int fds[2])
	fd[0] is the file descriptor for the read end of the pipe
	fd[1] is the file descriptor for the write end of the pipe
*/


/* Refereneces Used

https://stackoverflow.com/questions/8082932/connecting-n-commands-with-pipes-in-a-shell
https://www.cs.purdue.edu/homes/grr/SystemsProgrammingBook/Book/Chapter5-WritingYourOwnShell.pdf

https://www.geeksforgeeks.org/tokenizing-a-string-cpp/
https://www.geeksforgeeks.org/access-command-in-linux-with-examples/

I/O Redirects
https://homepages.uc.edu/~thomam/Intro_Unix_Text/IO_Redir_Pipes.html
http://www.rozmichelle.com/pipes-forks-dups/
*/