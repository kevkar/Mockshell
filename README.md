# cop4600_nutshell

This is a command interpreter written in C++ using Flex and Bison.

#Features Implemented:

Environment Variables:

setenv variable word sets the environment variable.

printenv prints all environment variables.

unsetenv variable removes the variable.

HOME and PATH are initialized in the shell and cannot be removed but can be changed.


Aliases:

alias name word adds an alias. For example alias b "ls -al" will execute the command in quotes whenever b is entered.

alias prints all aliases.

unalias name removes an alias from the list.

There is infiniite loop detection for aliases. If you try the commands: alias a b, alias b c, alias c a, an error will print.

Change Directory:

cd word changes the directory to word. The '.' and '..' characters will substitue for the current directory and the parent directory, respectively.

Wildcard Matching:

The character ? matches any single character and * matches all characters. For example, file.? might match file.o and file.c while file.* could match file.c, file.o and file.cpp.

Exiting the shell:

Just type in 'bye' to exit the shell.
