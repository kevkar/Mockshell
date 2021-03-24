#include <stdio.h>
#include "ns_parser.tab.h"
extern int yyparse();

int main()
{
	while(1) 
	{
		printf(">> ");
		yyparse();
	}

	return 0;
}
