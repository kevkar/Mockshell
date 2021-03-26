#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include "ns_parser.tab.h"
extern int yyparse();
extern char** environ;
char* homeDirectory;
char* path;

struct Node {
	const char* data;
	struct Node* next;
};

struct Node* makeNode(const char* chars, struct Node* next) {
	struct Node* node = (struct Node*)malloc(sizeof(struct Node));
	node->data = chars;
	node->next = next;
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

void changeDirectory(const char* directory)
{
	chdir(directory);
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
