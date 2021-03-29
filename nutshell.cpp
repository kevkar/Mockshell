#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <iostream>
#include <string.h>
#include "ns_parser.tab.h"

using namespace std;

extern int yyparse();
extern char** environ;
char* homeDirectory;
char* path;

struct Node {
	const char* data;
	Node* next;
};

struct Node* makeNode(const char* chars, Node* next) {
	struct Node* node = new Node();
	node->data = chars;
	node->next = next;
    return node;
}

void setEnvironmentVariable(const char* variable, const char* value)
{
	setenv(variable,value,0);
}

void unsetEnvironmentVariable(const char* variable)
{
	unsetenv(variable);
}

void printEnvVariables()
{
for (char** envVar = environ; *envVar != 0; envVar++)
    printf("%s\n", *envVar);    
}

// pwd for testing purposes
void printWorkingDirectory()
{
	char cwd[PATH_MAX];
	printf("PWD: %s\n",getcwd(cwd,sizeof(cwd)));
}

// for cd word
void changeDirectory(const char* directory)
{
	char cwd[PATH_MAX];
	getcwd(cwd,sizeof(cwd));
	strcat(cwd,"/");
	const char* newDir = strcat(cwd,directory);
	chdir(newDir);
}

// for cd ..
void goToPreviousDirectory(const char* path)
{
	const char* last = strrchr(path,'/');
	int index = (int)(last - path);
	char dest[index];
	strncpy(dest,path,index);
	dest[index] = '\0';
	chdir(dest);
	printWorkingDirectory();
}

void changeDirectoryToHome()
{
	char* home = getenv("HOME");
	if(home)
		changeDirectory(home);
	else 
		printf("There is no HOME!");
}

void executeOtherCommand(const char* path, char *const argv[], char *const envp[])
{
	execve(path, argv, envp);
}

void init()
{
	homeDirectory = getenv("HOME");
	path = getenv("PATH");
}

bool isEnvVariable(const char* word)
{
	int n = strlen(word) - 1;
	return word[0] == '$' && word[1] == '{' && word[n] == '}';
}

const char* transformEnvVariable(string word)
{
	int n = word.length() - 3;
	word = word.substr(2, n);
	return getenv(word.c_str());
}

int main(int argc, char* argv[])
{
	init();
	while(1) 
	{

		char *userName=getenv("USER");
		printf("%s>> ",userName);

		yyparse();
	}

	return 0;
}
