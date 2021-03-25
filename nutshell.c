#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ns_parser.tab.h"
extern int yyparse();

struct Node {
	char* data;
	struct Node* next;
};

struct Node* makeNode(struct Node* node, char* chars) {
	node = (struct Node*)malloc(sizeof(struct Node));
	node->data = chars;
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
