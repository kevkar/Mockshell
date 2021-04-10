#include "command.h"
#include "tables.h"

extern std::vector<std::string> built_in_cmds;
extern map<string,string> variableMap;
extern map<string,string> aliasTable;
extern vector<Alias> aliases;


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
		std::cout << "Output File: " << c.input << std::endl;
	}

	return;
}

void setEnvironmentVariable(std::vector<char*> args)
{
	if (args.size() > 2)
	{
		cout << "Too many arguments! No variable set." << endl;
		return;
	}
	variableMap[args[0]] = args[1];
	//setenv(variable,value,0);
}

void printEnvVariables()
{
	for (auto iter = variableMap.begin(); iter != variableMap.end(); iter++ )
		cout << iter->first << " : " << iter->second << endl;

	//for (char** envVar = environ; *envVar != 0; envVar++)
    //	printf("%s\n", *envVar);    
}

void unsetEnvironmentVariable(const char* variable)
{
	bool found = false;
	for (auto iter = variableMap.begin(); iter != variableMap.end(); iter++ )
		if (iter->first == variable) found = true;

	if (!found)
	{
		cout << "No variable with name " << variable << " exists.";
		return;
	}
		
	variableMap.erase(variable);
	//unsetenv(variable);
}

void changeDirectory(const char* directory)
{
	if (directory[0] == '/')
	{
		int error = chdir(directory);
		if (error != 0)
			std::cout << "ERROR: directory " << directory << " not found." << std::endl;
		return;
	}
	char cwd[PATH_MAX];
	getcwd(cwd,sizeof(cwd));
	strcat(cwd,"/");
	const char* newDir = strcat(cwd,directory);
	
	int error = chdir(newDir);
	if (error != 0)
		std::cout << "ERROR: directory " << newDir << " not found." << std::endl;
}

void changeDirectoryToHome()
{
	char* home = getenv("HOME");
	if(home)
		chdir(home);
	else 
		printf("There is no HOME!");
}

void cd(std::string name, std::vector<char*> args)
{
	if (args.size() == 0)
		changeDirectoryToHome();
	 else
	 	changeDirectory(args[0]);
}

// cant do switch statements with strings
void execute_built_in(std::string name,std::vector<char*> args) {
	if (name == "printenv")
		printEnvVariables();
//	else if (name == "setenv")
//		setEnvironmentVariable(args[0],args[1]);
	else if (name == "unsetenv")
		unsetEnvironmentVariable(args[0]);
	else if (name == "cd")
		cd(name,args);
}

char* tilde_parse(char* word)
{
	string home = getenv("HOME");
	string s(word);
	s = s.substr(1);
	s = s[0] == '/' ? home + s : home + '/' + s;
	
	return strdup(s.c_str());
}

char* dot_dot_parse(char* word)
{
	char currentDirectory[PATH_MAX];
	getcwd(currentDirectory,sizeof(currentDirectory));

	string cwd(currentDirectory);
	string s(word);

	string slash = "/";

	int index = cwd.find_last_of(slash);
	if (index == 0)
		return strdup(slash.c_str());

	cwd = cwd.substr(0, index);

	if (s.length() > 2)
		cwd += s.substr(2);
	
	return strdup(cwd.c_str());
}

char* one_dot_parse(char* word)
{
	char currentDirectory[PATH_MAX];
	getcwd(currentDirectory,sizeof(currentDirectory));

	string cwd(currentDirectory);
	string s(word);
	
	if (s.length() == 0)
		s = cwd;
	else
		{
			s = s.substr(1);
			s = cwd + s;
		}
	return strdup(s.c_str());
}

char* do_parse(char* word)
{
	if (word[0] == '~')
		word = tilde_parse(word);
	else if (strlen(word) > 1 && word[0] == '.' && word[1] == '.' )
		word = dot_dot_parse(word);
	else if (word[0] == '.')
		word = one_dot_parse(word);

	return word;
}

//test for invalid env variable???
bool has_valid_env_variable(char* word)
{
	string s(word);
	if (s.find("$") != string::npos &&
		s.find("{") != string::npos &&
		s.find("}") != string::npos) 
		return true;

	return false;
}

char* transform_env_variable(char* word)
{
	if (!has_valid_env_variable(word)) return word;

	cout << "env variable found!" << endl;

	string s(word);
	int n = s.length();
	string res = "";
	string dollarBracket("${");
	string closeBracket("}");

	int startIndex = s.find(dollarBracket);
	int endIndex = s.find(closeBracket);

	string begin = s.substr(0,startIndex);
	string middle = s.substr(startIndex+2,endIndex - startIndex - 2);
	const char* var = strdup(middle.c_str());

	string value = variableMap[var];

	string end = s.substr(endIndex + 1,n - endIndex);
	
	res += begin + value + end;

	return strdup(res.c_str());
}


void printAliases()
{
	if (aliases.size() == 0)
	{
		cout << "No aliases to show!" << endl;
		return;
	}
		
	cout << "List of aliases" << endl;
	for (int i = 0; i < aliases.size(); i++)
		cout  << aliases[i].key << " : " << aliases[i].value << endl;
}

void doAlias(std::string name, std::vector<char*> args)
{
	if (args.size() == 0)
		printAliases();
	else
		setAlias(args);
}

void setAlias(std::vector<char*> args)
{
	if (args.size() > 2 || args[0] == args[1])
	{
		cout << "Invalid setting of alias!" << endl;
		return;
	}

	string key = args[0];
	string value = args[1];
	aliasTable[key] = value;
	if (wouldMakeInfiniteLoop(strdup(value.c_str()),10))
	{
		cout << "Adding alias would make infinite loop!!!" << endl;
		aliasTable.erase(key);
		return;
	}

	string nested_command(transform_alias(strdup(value.c_str())));
		
	Alias alias(key,value,nested_command);
	aliases.push_back(alias);
}

char* transform_alias(char* name)
{

	string word(name);
	for (auto iter = aliasTable.begin(); iter != aliasTable.end(); iter++)
		if (iter->first == word)
			return transform_alias(strdup(iter->second.c_str()));
	
	return name;
}

bool wouldMakeInfiniteLoop(char* name, int depth)
{
	if (depth == 0)
		return true;

	string word(name);
	for (auto iter = aliasTable.begin(); iter != aliasTable.end(); iter++)
		if (iter->first == word)
			return wouldMakeInfiniteLoop(strdup(iter->second.c_str()),depth-1);

	return false;
	
}

void unAlias(std::vector<char*> args)
{
	if (args.size() > 1)
		{
			cout << "Too many arguments for unalias!" << endl;
			return;
		}
	aliasTable.erase(args[0]);
}

void execute_command(Command cmd)
{
	print_command(cmd);
	
	std::string name(cmd.command_name);

	if (name == "alias")
		doAlias(name,cmd.args);

	if (name == "unalias")
		unAlias(cmd.args);

	if (name == "setenv")
	{
		setEnvironmentVariable(cmd.args);
	}

	

	for (int i = 0; i < cmd.args.size(); i++)
		cout << "arg before " << i << ": " << cmd.args[i] << endl;
	
	for (int i = 0; i < cmd.args.size(); i++)
	{
		//cmd.args[i] = transform_env_variable(cmd.args[i]);
		cmd.args[i] = do_parse(cmd.args[i]);
	}

	for (int i = 0; i < cmd.args.size(); i++)
		cout << "arg after" << i << ": " << cmd.args[i] << endl;

	if (std::find(built_in_cmds.begin(), built_in_cmds.end(), name) != built_in_cmds.end()) {
		execute_built_in(name,cmd.args);
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
	}

	// Fork child process
	pid_t pid;
	pid = fork();

	if (pid < 0)
	{
		std::cout << "Unable to fork new process" << std::endl;
		return;
	}
	// Child process
	else if (pid == 0)
	{
		int n = cmd.args.size() + 2;
		char* args[n];

		args[0] = cmd.command_path;

		for(int i = 1; i <= cmd.args.size(); i++) {
			args[i] = cmd.args[i-1];
		}

		args[n-1] = NULL;

		// Execute command
		execv(args[0], args);
		
		exit(0);
	}
	// Parent Process
	else {
		wait(0);
	}
	return;

	// (not yet) Check I/O File for redirection (<, >, |)
	// Build up pipeline (create and setup pipe)
	// End points (using pipe and dup)

	// Process in background (if & present)

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
*/