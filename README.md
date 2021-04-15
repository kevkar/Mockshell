# COP4600 Term Project: The Nutshell
Team 67: Kevin Karch and Jonathan Sessa

Unix command interpreter written in C/C++ using Flex and Bison.

## Features Not Implemented/Known Bugs
- Wildcard matching does not work correctly when searching a different directory than current one
- Tilde Expansion Bug: Using tidle expasion with an non-existant user (e.g. \~none/) will result in a shell crash
- File Name Completion was not implemented (extra credit)

## Features Implemented:

### Built-In Commands

Environment Variables:
- setenv: Adds or updates environment variable
- printenv: Prints all environment variables (supports I/O redirection)
- unsetenv: Remove environmental variables (with PATH and HOME protection)
- Variable Expansion: Variables can be inluded as parts of a WORD in a command line (e.g., echo ${var})

Aliases:
- alias: Prints all assigned aliases (supports I/O redirection)
- alias NAME WORD: Adds an alias to the shell (with alias expansion and protection for infinite loops)
- unalias NAME: Removes alias from shell

Change Directory:
- cd WORD: Change directory to the provided WORD. If no WORD is provided, directory is changed to user's home directory. Support is also available for using '\~', '.', and '..''

### Other Commands

Other Command Execution:
- Commands: Basic UNIX commands (e.g., time, ls, etc.) are supported
- Commands with Arguments: UNIX commands that accept arguments (e.g., ls -l, sort file.txt)
- Piped Commands: Commands can be piped together (e.g., ls | wc)
- I/O Redirection: Input and output to commands can be sent to files (e.g., sort < list.txt > sorted.txt)
- Standard Error Redirection: Standard error can be redirected to stdout (2>&1) or to a specified file (2>errors.txt)
- Backgrounding: Commands can be executed in the backround with out waiting by adding & at the end of a command line

### Other Features

Wildcard Matching:
- Single Character: WORDs that include a ? are compared to files in the current directory to find a match (e.g., file.t?t)
- Strings: WORDs that include a * are compared to files in the current directory to find a matching string (e.g., file.\*)

Tilde Expansion:
- \~ is replaced by the home directory
- \~NAME is replaced with the home directory of the user (if user exists)
- \~\directory is replaced with the provided directory appended to the home directory

File Name Completion:
- The parsing detection for the ESC character was added to our Flex/Bison system, but we did not have time to work on it further 

Exiting:
- bye: Typing 'bye' will exit the shell (also supports EOF exiting)


## Work Responsibilities

Kevin Karsh
- Environmental Variables
- Variable expansion
- Aliases (including expansion and infinite loop detection)
- Change Directory (including '.' and '..' parsing)
- Wildcard matching (? and \*)
- Parsing rules for built-in commands, aliases, and environmental variables

Jonathan Sessa
- Other command execution (including the command table and command data structures)
- Command piping
- I/O redirection (including redirection for built-in commands)
- Standard error redirection
- Backgrounding
- Tilde Expansion
- Parsing rules for commands, arguments, and I/O redirection

Both
- Bug fixing
- Testing
