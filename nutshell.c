#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include "ns_parser.tab.h"
extern int yyparse();
extern char** environ;

struct Node {
	const char* data;
	struct Node* next;
};

struct Node* makeNode(struct Node* node, const char* chars) {
	node = (struct Node*)malloc(sizeof(struct Node));
	node->data = chars;
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

void setDirectoryToHome()
{
	char* home = getenv("HOME");
	if(home)
		changeDirectory(home);
	else 
		printf("There is no home!");
}

int main(int argc, char* argv[])
{
	while(1) 
	{
		char *userName=getenv("USER");
		printf("%s>> ",userName);

		yyparse();
	}

	return 0;
}
