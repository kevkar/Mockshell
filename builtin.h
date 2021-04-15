#ifndef BUILTIN_H
#define BUILTIN_H

#include <iostream>
#include "command.h"

void built_in_command_dispatcher(Command_Table*);

void print_aliases(Command_Table*);
void set_alias(char*, char*);
void remove_alias(char*);

bool would_make_infinite_loop(char*, int);

void change_directory(char*);
void change_directory_to_home();

char* parse_dir(char*);
char* parse_dot(char*);
char* parse_dot_dot(char*);
char* parse_tilde(char*);

void set_env_variable(char*, char*);
void unset_env_variable(char*);
void print_env_variables(Command_Table*);

#endif
