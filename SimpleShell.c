#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_ARGS 510
#define MAX_COMMANDS 510
#define MAX_COMMAND_LENGTH 510

//declaration on the functions
//struct for the variables
typedef struct variable{
    char* name;
    char* val;
}vars;

// Variable to store the last child process ID
pid_t last_pid;

int execute(char* command, char** args);
void freeMemory( char**, char** ,vars* , char*);
void read_input(char* input);
void parse_input_space(char* cmd, char** args, int* num_args);
void parse_input_by_symbol(char* input, char** args, int* num_args);
int execute_command(char** args, int background, int , char* output_file, char** pipe_args);
void handle_signal(int signum);

// Function prototype
//void handle_signal(int signum);
void handle_signal(int signum) {
    if (signum == SIGTSTP) { // Ctrl+Z signal
        if (last_pid != 0) {
            kill(last_pid, SIGSTOP); // Suspend the last command
        }
    }
}

//a function that read input from the user
void read_input(char* input){
    fgets(input, 520, stdin);
    if (input[strlen(input) - 1] == '\n') {
        input[strlen(input) - 1] = '\0'; // Remove trailing newline
    }
}

// a function that take a string and parse it by space
void parse_input_space(char* cmd, char** args, int* num_args) {
    int numArgsOfEcho = 0;
    args[1] = (char*)malloc(sizeof(char) * 510);
    //if the command is echo
    if (strstr(cmd, "echo") != NULL){
        args[0] = "echo";
        int iterateOnCmd = 0;
        //skip first spaces
        while (cmd[iterateOnCmd] == ' '){
            iterateOnCmd++;
        }
        //counting args
        while (cmd[iterateOnCmd] != '\0') {
            if(cmd[iterateOnCmd] == '"'){
                iterateOnCmd++;
                while (cmd[iterateOnCmd] != '"'){
                    iterateOnCmd++;
                }
                iterateOnCmd++;
            }
            if(cmd[iterateOnCmd] == '\''){
                iterateOnCmd++;
                while (cmd[iterateOnCmd] != '\''){
                    iterateOnCmd++;
                }
                iterateOnCmd++;
            }
            if(cmd[iterateOnCmd] != ' ' && cmd[iterateOnCmd] != '"' && cmd[iterateOnCmd] != '\'' && cmd[iterateOnCmd] != '\0'){
                while (cmd[iterateOnCmd] != ' ' && cmd[iterateOnCmd] != '"' && cmd[iterateOnCmd] != '\'' && cmd[iterateOnCmd] != '\0'){
                    iterateOnCmd++;
                }
            } if(cmd[iterateOnCmd] == ' ' || cmd[iterateOnCmd] == '\0'){
                (*num_args)++;
                iterateOnCmd++;
            }
        }
        //i = location after echo and one space
        int i = 5;
        //skip first spaces
        while (cmd[i] == ' '){
            i++;
        }
        //copy the output
        int k = 0;
        while (cmd[i] != '\0') {
            if(cmd[i] == '"'){
                i++;//skip '"'
                while (cmd[i] != '"'){
                    args[1][k] = cmd[i];
                    k++;
                    i++;
                }
                i++;//skip '"'
            }
            else if(cmd[i] == '\''){
                i++;//skip ' ' '
                while (cmd[i] != '\''){
                    args[1][k] = cmd[i];
                    k++;
                    i++;
                }
                i++;//skip ' ' '
            }
            else {
                args[1][k] = cmd[i];
                i++;
                k++;
            }
        }
        args[1][k+1] = '\0';
        return;
    }

    *num_args = 0; // Initialize num_args to 0
    char* token = strtok(cmd, " "); // Tokenize cmd string using space as delimiter
    while (token != NULL && *num_args < MAX_ARGS - 1) { // Loop until end of tokens or maximum args reached
        args[*num_args] = token; // Store token in args array at current num_args index
        (*num_args)++; // Increment num_args to indicate a token has been parsed
        token = strtok(NULL, " "); // Get next token using strtok with NULL as cmd string
    }
    args[*num_args] = NULL; // Set the last argument to NULL to indicate end of argument list

    // Error handling: If too many arguments are parsed, set the last element of args to NULL
    if (*num_args >= MAX_ARGS - 1) {
        args[MAX_ARGS - 1] = NULL;
    }
}

// a function that take a string and parse it by symbol ';'
void parse_input_by_symbol(char* input, char** args, int* num_args) {
    char* a;
    for (int i = 0; input[i] != '\0'; ++i) {
        if(input[i] == 'e' && input[i + 1] == 'c' && input[i + 2] == 'h' && input[i + 3] == 'o') {
            i += 3;
            while (input[i] != '"' && input[i] != '\0'){
                i++;
            }
            if(input[i] == '"') {
                i++;
                while (input[i] != '"'){
                    if(input[i] == ';'){
                        a = &input[i];
                    }
                    i++;
                }
            }
        }
    }
    if(a != NULL){
        *a = '`';
    }

    *num_args = 0; // Initialize num_args to 0
    char* token = strtok(input, ";"); // Tokenize input string using space as delimiter
    while (token != NULL && *num_args < MAX_ARGS - 1) { // Loop until end of tokens or maximum args reached
        args[*num_args] = token; // Store token in args array at current num_args index
        (*num_args)++; // Increment num_args to indicate a token has been parsed
        token = strtok(NULL, ";"); // Get next token using strtok with NULL as input string
    }
    args[*num_args] = NULL; // Set the last argument to NULL to indicate end of argument list

    if(a != NULL){
        for (int i = 0; args[i] != NULL; ++i) {
            for (int j = 0; args[i][j] != '\0'; ++j) {
                if(args[i][j] == '`'){
                    args[i][j] = ';';
                }
            }
        }
    }
    // Error handling: If too many arguments are parsed, set the last element of args to NULL
    if (*num_args >= MAX_ARGS - 1) {
        args[MAX_ARGS - 1] = NULL;
    }
}

//a function for executing the commands
int execute_command(char** args, int background, int is_file, char* output_file, char** pipe_args) {
    int pipefd[2];
    pid_t pid;

    // Create a pipe if there are pipe arguments
    if (pipe_args != NULL) {
        if (pipe(pipefd) == -1) {
            printf("ERR\n");
            return -1;
        }
    }

    pid = fork(); // Fork a child process
    int fd = -1;
    if (pid == -1) {
        printf("ERR\n");
        return -1;
    }
    else if (pid == 0) {
        // Child process
        if(is_file == 1) {
            fd = open("temp", O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
            dup2(fd, STDOUT_FILENO);
            if (fd < 0) {
                //freeMemory()
                printf("ERR");
            }
        }
        // Redirect input if there are pipe arguments
        if (pipe_args != NULL) {
            close(pipefd[0]);
            dup2(pipefd[1], STDOUT_FILENO);
            close(pipefd[1]);
        }
        execute(args[0], args);
    } else {
        // Parent process
        close(fd);
        if (background == 1) {
            // Do not wait for child to terminate if command is run in the background
            return 0;
        } else {
            // Wait for the child process to finish or be suspended
            last_pid = pid;
            int status;
            waitpid(pid, &status, WUNTRACED);

            // Check if the child process was terminated by a signal
            if (WIFSIGNALED(status)) {
                return -1;
            } else if (WIFEXITED(status)) {
                return WEXITSTATUS(status);
            } else {
                return -1;
            }
        }
    }
    return 0;
}

//free memory function
void freeMemory(char** commands, char** args, vars* vars, char* input) {
    free(input);

    // Free the memory for each row of the commands array
    for (int i = 0; commands[i] != NULL; i++) {
        free(commands[i]);
    }
    // Free the memory for the rows of the commands
    free(commands);

    // Free the memory for each row of the args array
    for (int i = 0; args[i] != NULL; i++) {
        free(args[i]);
    }
    free(args);

    // Free the memory for each member of the struct
    for (int i = 0; vars[i].name != NULL && vars[i].val != NULL; i++) {
        free(vars[i].name);
        free(vars[i].val);
    }
    free(vars);
}

int execute(char* command, char** args){
    if (strcmp(args[0], "cd") == 0){//if the next command is cd
        printf("cd not supported\n");
        return 0;
    }
    if (execvp(args[0], args) == -1) {
        printf("ERR\n");
        return -1;
    }
    return 0;
}

int main() {
    // Set the signal handler for SIGINT to prevent the shell from exiting when the user presses Ctrl+Z
    signal(SIGTSTP, handle_signal);

    //variables declaration
    int background[510];
    for (int i = 0; i < 510; ++i) {
        background[i] = 0;
    }
    int is_file;
    char* output_file;
    char** pipe_args[1000][1000];
    char** commands;
    char** args;
    vars* vars;
    char cwd[510]; // Buffer to store current working directory
    int numArgs = 0; //use for the parse
    int emptyInputCount = 0; //to count how many times the user gave an empty input
    int totalNumOfCmd = 0; //amount of arguments per command
    int totalNumOfArgs = 0; // total arguments

    //initiate variables
    char* input = (char *)malloc(sizeof (char) * 1024); //the input from user
    if(input == NULL){
        printf("ERR\n");
        return -1;
    }

    //find the current location for the first print
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        printf("ERR\n");
        free(input);
        return 1;
    }
    printf("#cmd:0|#args:0@%s>",cwd);

    while (1) {
        //Read input from the user
        read_input(input);
        //if the input is longer then 510 chars
        if(strlen((input)) > 510){
            printf("ERR\n");
            //find the current location
            if (getcwd(cwd, sizeof(cwd)) == NULL) {
                printf("ERR\n");
                freeMemory(commands, args, vars, input);
                return 1;
            }
            printf("#cmd:0|#args:0@%s>",cwd);
            continue;
        }

        // Exit on empty input after 3 times in a row
        if (strlen(input) == 0) {
            emptyInputCount++;
            if(emptyInputCount == 3){
                freeMemory(commands, args, vars, input);
                return -1;
            }
            printf("\n#cmd:%d|#args:%d@%s>", totalNumOfCmd, totalNumOfArgs, cwd);
            continue;
        }
        else{
            emptyInputCount = 0;
        }

        //find the size of the arrays
        int echoSize = 3,commandSize = 3, argSize = 3, varSize = 3;
        for (int i = 0; input[i] != '\0'; ++i) {
            if(input[i] == 'e' && input[i + 1] == 'c' && input[i + 2] == 'h' && input[i + 3] == 'o'){
                echoSize++;
            }
            if(input[i] == ';'){
                commandSize ++;
            }
            if(input[i] == ' '){
                argSize++;
            }
            if(input[i] == '='){
                varSize++;
            }
        }

        // Allocate memory
        commands = (char**)malloc(sizeof(char*) * commandSize);
        if(commands == NULL){
            printf("ERR\n");
            freeMemory(commands, args, vars, input);
            return -1;
        }
        for (int i = 0; i < commandSize; ++i) {
            commands[i] = (char *)malloc(sizeof(char) * 510);
            if(commands[i] == NULL){
                printf("ERR\n");
                freeMemory(commands, args, vars, input);
                return -1;
            }
        }
        args = (char**)malloc(3 * sizeof(char*) * argSize);
        if(args == NULL){
            printf("ERR\n");
            freeMemory(commands, args, vars, input);
            return -1;
        }
        for (int i = 0; i < argSize; ++i) {
            args[i] = (char*)malloc(sizeof(char) * 510);
            if(args[i] == NULL){
                printf("ERR\n");
                freeMemory(commands, args, vars, input);
                return -1;
            }
        }
        if(vars == NULL) {
            vars = malloc(sizeof(*vars) * varSize);
        }
        if (vars == NULL) {
            printf("ERR\n");
            freeMemory(commands, args, vars, input);
            return -1;
        }
        for (int i = 0; i < varSize; i++) {
            if(vars[i].name == NULL) {
                vars[i].name = malloc(sizeof(char) * 510);
                if (vars[i].name == NULL) {
                    printf("ERR\n");
                    freeMemory(commands, args, vars, input);
                    return -1;
                }
                vars[i].val = malloc(sizeof(char) * 510);
                if (vars[i].val == NULL) {
                    printf("ERR\n");
                    freeMemory(commands, args, vars, input);
                    return -1;
                }
            }
        }

        numArgs = 0;
        // Parse input into commands by symbol ";"
        parse_input_by_symbol(input, commands, &numArgs);
        //adding the args to the total num
        totalNumOfCmd += numArgs;
        for (int i = 0; commands[i] != NULL; ++i) {
            if(commands[i][strlen(commands[i]) - 1] == '&'){
                background[i] = 1;
                commands[i][strlen(commands[i]) - 1] = '\0';
            }
        }

        int currVar = 0;
        int currCmd = 0;
        //saves the variables
        while (commands[currCmd] != NULL){
            int varDontExist = 0;
            char *eq = strchr(commands[currCmd], '=');
            char *echoStr = strstr(commands[currCmd], "echo");
            char *dollar = strchr(commands[currCmd], '$');
            //if in the same command there is "=" and "echo" and in that order, then it's a regular command
            if((eq != NULL && echoStr != NULL && eq > echoStr) || (eq == NULL)){
                //if there is a value
                if(dollar != NULL){
                    //get the var name to see if he exists
                    char nameTemp[510];
                    //initialize to '\0'
                    for (int i = 0; i < strlen(nameTemp); ++i) {
                        nameTemp[i] = '\0';
                    }
                    //copy the name
                    for (int i = 0; *(dollar + i + 1) != '\0' && *(dollar + i + 1) != ' ' && *(dollar + i + 1) != '"'; i++) {
                        nameTemp[i] = *(dollar + i + 1);//copy the name of the var
                    }
                    varDontExist = 0;
                    //check if the variable exist (yet)
                    for (int i = 0; vars[i].name != NULL; ++i) {
                        //if the var exist
                        if(strstr(vars[i].name, nameTemp) != NULL || (strlen(vars[i].name) == 1 && strlen(nameTemp) == 1 && vars[i].name[0] ==  nameTemp[0])){
                            varDontExist = 1;
                            currVar = i;
                            char commandTemp[510];
                            int iterateOnCmd = 0;
                            int iterateOnCmdTemp = 0;
                            //copy the command until the dollar sing
                            while (commands[currCmd][iterateOnCmd] != *(dollar)){
                                commandTemp[iterateOnCmd] = commands[currCmd][iterateOnCmd];
                                iterateOnCmd++;
                            }
                            //cmdLocationAfterVarName is the location of the command after the name
                            int cmdLocationAfterVarName = iterateOnCmd + (int)strlen(nameTemp) + 1;
                            //copy the value
                            for (int k = 0; vars[currVar].val[k] != '\0'; ++k,iterateOnCmd++) {
                                commandTemp[iterateOnCmd] = vars[currVar].val[k];
                            }
                            //updating locations
                            iterateOnCmdTemp = iterateOnCmd;
                            iterateOnCmd = cmdLocationAfterVarName;
                            //copy the rest of the command
                            while (commands[currCmd][iterateOnCmd] != '\0'){
                                commandTemp[iterateOnCmdTemp] = commands[currCmd][iterateOnCmd];
                                iterateOnCmd++;
                                iterateOnCmdTemp++;
                            }
                            commandTemp[iterateOnCmdTemp] = '\0';

                            int iteratorForCoping = 0;
                            //chang the command
                            while(commandTemp[iteratorForCoping] != '\0') {
                                commands[currCmd][iteratorForCoping] = commandTemp[iteratorForCoping];
                                iteratorForCoping++;
                            }
                            commands[currCmd][iteratorForCoping] = '\0';
                        }
                    }
                    //check for more values
                    char* dollar2 = strchr(commands[currCmd], '$');
                    if(dollar2 != NULL){
                        if(varDontExist == 0){
                            currCmd++;
                            continue;
                        }
                        continue;
                    }
                    else{
                        currCmd++;
                        continue;
                    }
                }
                else {//id dollar == null
                    currCmd++;
                    continue;
                }
            }
                //if it's a variable setting command
            else if(eq != NULL){
                if(vars[currVar].name == NULL){
                    vars[currVar].name = malloc(sizeof(char ) * 510);
                    vars[currVar].val = malloc(sizeof(char ) * 510);
                }
                //pointer to the command that's the name
                vars[currVar].name = commands[currCmd];
                //pointer to the command that's the value
                vars[currVar].val = eq + 1;
                //split the command into 2 strings name and command
                *eq = '\0';
                currVar++;
                currCmd++;
            } else{
                currCmd++;
            }
        }

        //find the current location
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            printf("ERR\n");
            freeMemory(commands, args, vars, input);
            return 1;
        }

        //execute the commands
        int numOfExecuteCmd = 0;

        last_pid = 0;
        //while there are commands
        while (commands[numOfExecuteCmd] != NULL) {
            //if it's a command with pipe
            if (strchr(commands[numOfExecuteCmd], '|') != 0) {
                int backgroundPipe[510];
                for (int i = 0; i < 510; ++i) {
                    backgroundPipe[i] = 0;
                }
                // Parse commandsOfPipe string into an array of commandsPipe
                char *commandsPipe[510];
                char *commandsOfPipe = strtok(commands[numOfExecuteCmd], "|");
                int numCommandsPipe = 0;
                while (commandsOfPipe != NULL && numCommandsPipe < MAX_COMMANDS) {
                    commandsPipe[numCommandsPipe++] = commandsOfPipe;
                    commandsOfPipe = strtok(NULL, "|");
                }

                for (int i = 0; commandsPipe[i] != NULL; ++i) {
                    if (commandsPipe[i][strlen(commandsPipe[i]) - 1] == '&') {
                        backgroundPipe[i] = 1;
                        commandsPipe[i][strlen(commandsPipe[i]) - 1] = '\0';
                    }
                }

                // Set up initial pipe
                int pipefd[2];
                if (pipe(pipefd) == -1) {
                    printf("ERR\n");
                    freeMemory(commands, args, vars, input);
                    return -1;
                }

                // Fork child processes for each commandsOfPipe and connect pipes
                int i;
                for (i = 0; i < numCommandsPipe; i++) {
                    int new_pipefd[2];
                    if (i < numCommandsPipe - 1 && pipe(new_pipefd) == -1) {
                        printf("ERR\n");
                        freeMemory(commands, args, vars, input);
                        return -1;
                    }

                    pid_t pid = fork();
                    if (pid == -1) {
                        printf("ERR\n");
                        freeMemory(commands, args, vars, input);
                        return -1;
                    }
                        //child process
                    else if (pid == 0) {
                        if (i > 0) {
                            // Connect stdin to previous commandsOfPipe's pipe
                            if (dup2(pipefd[0], STDIN_FILENO) == -1) {
                                printf("ERR\n");
                                freeMemory(commands, args, vars, input);
                                return -1;
                            }
                            close(pipefd[0]);
                            close(pipefd[1]);
                        }
                        //if it's not the last command
                        if (i < numCommandsPipe - 1) {
                            // Connect stdout to next commandsOfPipe's pipe
                            if (dup2(new_pipefd[1], STDOUT_FILENO) == -1) {
                                printf("ERR\n");
                                freeMemory(commands, args, vars, input);
                                return -1;
                            }
                            close(new_pipefd[0]);
                            close(new_pipefd[1]);
                        }

                        // Execute commandsOfPipe
                        char *argsPipe[MAX_COMMAND_LENGTH / 2 + 1];
                        char *arg = strtok(commandsPipe[i], " ");
                        int num_args = 0;
                        while (arg != NULL && num_args < MAX_COMMAND_LENGTH / 2 + 1) {
                            argsPipe[num_args++] = arg;
                            arg = strtok(NULL, " ");
                        }
                        argsPipe[num_args] = NULL;

                        if (strchr(argsPipe[strlen(*argsPipe - 1)], '&') != 0) {
                            argsPipe[strlen(*argsPipe - 1)][0] = '\0';
                            backgroundPipe[i] = 1;
                        }

                        if (execute(argsPipe[0], argsPipe) == -1) {
                            printf("ERR");
                            numOfExecuteCmd++;
                            continue;
                        }

                        printf("ERR");
                        freeMemory(commands, args, vars, input);
                        return -1;
                    } else {
                        // Parent process
                        if (i > 0) {
                            close(pipefd[0]);
                            close(pipefd[1]);
                        }
                        pipefd[0] = new_pipefd[0];
                        pipefd[1] = new_pipefd[1];
                        if (backgroundPipe[i] == 0) {
                            wait(NULL);
                        }
                    }
                }

                // Wait for all child processes to complete
                for (i = 0; i < numCommandsPipe; i++) {
                    if (backgroundPipe[i] == 0) {
                        wait(NULL);
                    }
                }
                numOfExecuteCmd++;
                continue;
            }

            is_file = 0;
            char *redirect = strchr(commands[numOfExecuteCmd], '>');
            // Check if the command contains '>'
            if (redirect != NULL) {
                is_file = 1;
                *redirect = ' ';

                // Get the filename after '>'
                if(*(redirect + 1) != '\0') {
                    output_file = redirect + 1;
                } else{
                    output_file = redirect;
                }

                // Remove any leading whitespace from the filename
                while (*output_file == ' ') {
                    output_file++;
                }
            }
            else {
                output_file = NULL;
            }

            numArgs=0;
            // Parse input into arguments by space (handles echo)
            parse_input_space(commands[numOfExecuteCmd], args, &numArgs);

            //if there is more than 10 args
            if(numArgs > 10){
                printf("more the 10 arguments\n");
                numOfExecuteCmd++;
                continue;
            }
            else {
                totalNumOfArgs += numArgs;
            }

            // Check if the command is "bg"
            if (strcmp(commands[numOfExecuteCmd], "bg") == 0) {
                if (last_pid != 0) {
                    // Resume the last suspended command
                    kill(last_pid, SIGCONT);
                }
                numOfExecuteCmd++;
                continue;
            }

            //if there are command
            if (numArgs > 0) {
                numArgs = 0;

                //if the command is a variable setting
                int isVariable = 0;
                for (int l = 0; vars[l].name != NULL; ++l) {
                    if(strcmp(args[0], vars[l].name) == 0){
                        isVariable = 1;
                        break;
                    }
                }
                if(isVariable == 1){
                    isVariable = 0;
                    numOfExecuteCmd++;
                    continue;
                }

                //execute the command
                int status = execute_command(args, background[numOfExecuteCmd], is_file, output_file, **pipe_args); // Execute the command
                if (status == -1) {
                    printf("ERR");
                    numOfExecuteCmd++;
                    continue;
                } else{
                    totalNumOfArgs += numArgs;
                }
            }
            numOfExecuteCmd++;
        }
        //find the current location
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("ERR");
            // Call freeMemory() function to free memory before exiting with specific exit code
            freeMemory(commands, args, vars, input);
            return 1;
        }
        printf("#cmd:%d|#args:%d@%s>", totalNumOfCmd, totalNumOfArgs, cwd);
    }
    return 0;
}
