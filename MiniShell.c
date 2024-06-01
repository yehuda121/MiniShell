#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_INPUT_LENGTH 1000
#define MAX_ARGS 500
//declaration on the functions
void freeMemory(char** echo, int echoNum, char** commands, int numOfCmd, char** args, int numArgs);
void read_input(char* input);
void parse_input_space(char* input, char** args, int* num_args);
void parse_input_by_symbol(char* input, char** args, int* num_args);
int execute_command(char** args);



//a function that read input from the user
void read_input(char* input){
    fgets(input, MAX_INPUT_LENGTH, stdin);
    if (input[strlen(input) - 1] == '\n') {
        input[strlen(input) - 1] = '\0'; // Remove trailing newline
    }
}

// a function that take a string and parse it by space
void parse_input_space(char* input, char** args, int* num_args) {
    *num_args = 0; // Initialize num_args to 0
    char* token = strtok(input, " "); // Tokenize input string using space as delimiter
    while (token != NULL && *num_args < MAX_ARGS - 1) { // Loop until end of tokens or maximum args reached
        args[*num_args] = token; // Store token in args array at current num_args index
        (*num_args)++; // Increment num_args to indicate a token has been parsed
        token = strtok(NULL, " "); // Get next token using strtok with NULL as input string
    }
    args[*num_args] = NULL; // Set the last argument to NULL to indicate end of argument list

    // Error handling: If too many arguments are parsed, set the last element of args to NULL
    if (*num_args >= MAX_ARGS - 1) {
        args[MAX_ARGS - 1] = NULL;
    }
}

// a function that take a string and parse it by space
void parse_input_by_symbol(char* input, char** args, int* num_args) {
    *num_args = 0; // Initialize num_args to 0
    char* token = strtok(input, ";"); // Tokenize input string using space as delimiter
    while (token != NULL && *num_args < MAX_ARGS - 1) { // Loop until end of tokens or maximum args reached
        args[*num_args] = token; // Store token in args array at current num_args index
        (*num_args)++; // Increment num_args to indicate a token has been parsed
        token = strtok(NULL, ";"); // Get next token using strtok with NULL as input string
    }
    args[*num_args] = NULL; // Set the last argument to NULL to indicate end of argument list

    // Error handling: If too many arguments are parsed, set the last element of args to NULL
    if (*num_args >= MAX_ARGS - 1) {
        args[MAX_ARGS - 1] = NULL;
    }
}

//executing the commands
int execute_command(char** args) {
    pid_t pid = fork(); // Fork a child process

    if (pid == -1) {
        perror("ERR");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) {
            perror("ERR");////////////////////////////////////////////////////////////////
            exit(EXIT_FAILURE);
        }
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0); // Wait for child to terminate
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else {
            return -1;
        }
    }

    return 0;
}

void freeMemory(char** echo, int echoNum, char** commands, int numOfCmd, char** args, int numArgs) {
    // Free memory for echo array
    for (int i = 0; i < echoNum; i++) {
        free(echo[i]);
    }
    free(echo);

    // Free memory for commands array
    for (int i = 0; i < numOfCmd; i++) {
        free(commands[i]);
    }
    free(commands);

    // Free memory for args array
    for (int i = 0; i < numArgs; i++) {
        free(args[i]);
    }
    free(args);
}

//struct for the variables
typedef struct variable{
    char* name;
    char* val;
}vars;



int main() {
    char input[MAX_INPUT_LENGTH]; //input from user
    char* commands[500]; //divide the input in to commands by the symbol ;
    char* args[MAX_ARGS]; //every command divide to arguments to be executed
    char echo[500][510]; //if echo command been used
    int echoNum = 0; //to execute the echo in the right order
    int numArgs = 0; //use for the parse
    int emptyInputCount = 0; //to count how many times the user gave an empty input
    int numOfCmd = 0; //amaunt of arguments per command
    int numOfTotalArgs = 0; // total arguments
    char cwd[3000]; // Buffer to store current working directory
    struct variable vars[510];

    //find the current location
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd() error");
        return 1;
    }
    printf("#cmd:0|#args:0@%s>",cwd);

    while (1) {
        //Read input from the user
        read_input(input);
        //if the input is longer then 510 chars
        if(strlen((input)) > 510){
            perror("ERR");
            continue;
        }

        // Exit on empty input after 3 times
        if (strlen(input) == 0) {
            emptyInputCount++;
            if(emptyInputCount == 3){
                // Call freeMemory() function to free memory before exiting with specific exit code
                freeMemory(echo, echoNum, commands, numOfCmd, args, numArgs);
                exit(1);
            }
            printf("\n#cmd:%d|#args:%d@%s>", numOfCmd, numOfTotalArgs, cwd);
            continue;
        }
        else{
            emptyInputCount = 0;
        }

        int iterator = 0;
        int j = 0;
        int flag = 0;
        echoNum = 0;
        // Iterate on the input for echo command
        while (input[iterator] != '\0') {
            // If the command is echo
            if (input[iterator] == 'e' && input[iterator + 1] == 'c' && input[iterator + 2] == 'h' && input[iterator + 3] == 'o') {
                iterator += 4; // Skip past "echo"
                flag = 0;
                while (input[iterator] != '\0' && flag == 0) {
                    //skip the first spaces
                    if (input[iterator] == ' ') {
                        iterator++;
                        continue;
                    }
                    // If the text starts with double quote
                    if (input[iterator] == '"') {
                        iterator++;
                        // Copy the text until the next quote or reaching maximum length
                        while (input[iterator] != '"' && input[iterator] != '\0' && j < 510 && echoNum < 300) {
                            echo[echoNum][j] = input[iterator];
                            j++;
                            iterator++;
                        }
                        flag = 1;
                        echo[echoNum][j] = '\0'; // Null-terminate the extracted substring
                        echoNum++;
                        j = 0;
                    }
                        // If the text starts with single quote
                    else if (input[iterator] == '\'') {
                        iterator++;
                        // Copy the text until the next single quote or reaching maximum length
                        while (input[iterator] != '\'' && input[iterator] != '\0' && j < 510 - 1 && echoNum < 300) {
                            echo[echoNum][j] = input[iterator];
                            j++;
                            iterator++;
                        }
                        flag = 1;
                        echo[echoNum][j] = '\0'; // Null-terminate the extracted substring
                        echoNum++;
                        j = 0;
                    }
                    else {
                        while (input[iterator] == ' ') {
                            iterator++;
                        }
                        // Copy the text until the next semicolon or end of string or reaching maximum length
                        while (input[iterator] != ';' && input[iterator] != '\0' && j < 300) {
                            echo[echoNum][j] = input[iterator];
                            j++;
                            iterator++;
                        }
                        flag = 1;
                        echo[echoNum][j] = '\0'; // Null-terminate the extracted substring
                        echoNum++;
                        j = 0;
                    }
                }
            }
            else {
                iterator++; // Move to the next character in the input string
            }
        }
        echoNum = 0; //to count again for execute

        // Parse input into arguments by symbol ";"
        parse_input_by_symbol(input, commands, &numArgs);
        numOfCmd = numArgs;
        int lastVar = 0;
        int k = 0;

        //saves the variables
        while (commands[k] != NULL){
            char * eq = strchr(commands[k], '='), *echoStr = strstr(commands[k], "echo");
            char * dolar = strchr(commands[k], '$');
            //if in the same command there is "=" and "echo" and in that order, then its a regular command
            if((eq != NULL && echoStr != NULL && eq > echoStr) || (eq == NULL)){
                //if there is a value
                if(dolar != NULL){
                    char nameTemp[510];
                    for (int i = 0; *(dolar + i + 1) != '\0' && *(dolar + i + 1) != ' '; ++i) {
                        nameTemp[i] = *(dolar + i + 1);//copy the name of the var
                    }
                    //check if the variable exist yet
                    for (int i = 0; vars[i].name != NULL; ++i) {
                        if(strstr(vars[i].name, nameTemp) != NULL){//the var exist
                            lastVar=i;
                            char temp[510];
                            int z = 0;
                            while (commands[k][z] != '\0') {
                                while (commands[k][z] != *(dolar)){
                                    temp[z] = commands[k][z];
                                    z++;
                                }
                                z++; // skip the dollar
                                int r = 0;
                                while (vars[lastVar].val[r] != '\0') {
                                    temp[z] = vars[lastVar].val[r];
                                    r++;
                                }
                                for (int l = 0;nameTemp[l] != '\0'; ++l) {
                                    z ++;//skip the var name
                                }

                                temp[z] = commands[k][z];
                            }
                            temp[z+1] = '\0';
                            //copy temp to input
                            int l = 0;
                            while (temp[l] != '\0'){
                                if(commands[k][l] == temp[l]){
                                    continue;
                                }
                                commands[k][l] = temp[l];
                                l++;
                            }
                            commands[l+1] = '\0';
                        }
                    }
                }
                k++;
            }
            else if(eq != NULL){
                vars[lastVar].name = commands[k];
                vars[lastVar].val = eq + 1;
                *eq = '\0';
                lastVar++;
                k++;
            } else{
                k++;
            }
        }
//        printf("after    ");
//        o = 0;
//        while (commands[o] != NULL) {
//            puts(input);
//            o++;
//        }
//        printf("struc:");
//        for (int i = 0; vars[i].val != NULL; ++i) {
//            puts(vars[i].name);
//            puts(vars[i].val);
//        }


        //find the current location
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("ERR");
            return 1;
        }

        //execute the commands
        int i = 0;
        //numOfTotalArgs = 0;
        flag = 0;
        while (commands[i] != NULL){//while there are more arguments
            numArgs = 0;
            parse_input_space(commands[i], args, &numArgs);
            if(numArgs > 10){
                printf("more the 10 args\n");
                i++;
                continue;
            }
            else if (numArgs > 0) {
                if (strcmp(args[0], "cd") == 0){//if the next command is cd
                    printf("cd not supported\n");
                    i++;
                    numOfCmd --;
                    continue;
                }
                if (strcmp(args[0], "echo") == 0){
                    //counting args
                    int l = 0;
                    while (echo[echoNum][l] != '\0') {
                        if(echo[echoNum][l] == ' '){
                            l++;
                            continue;
                        }
                        if(echo[echoNum][l] == '"'){
                            l++;
                            while (echo[echoNum][l] != '"'){
                                l++;
                            }
                            l++;
                        } else if(echo[echoNum][l] == '\''){
                            l++;
                            while (echo[echoNum][l] != '\''){
                                l++;
                            }
                            l++;
                        } else if(echo[echoNum][l] != ' ' && echo[echoNum][l] != '"' && echo[echoNum][l] != '\'' && echo[echoNum][l] != '\0'){
                            while (echo[echoNum][l] != ' ' && echo[echoNum][l] != '"' && echo[echoNum][l] != '\'' && echo[echoNum][l] != '\0'){
                                l++;
                            }
                        } if(echo[echoNum][l] == ' ' || echo[echoNum][l] == '\0'){
                            numOfTotalArgs++;
                        }
                    }
                    puts(echo[echoNum]);
                    echo[echoNum][0] = '\0';
                    echoNum++;
                    i++;
                    continue;
                }

                //if the command is variable setting
                int contin = 0;
                for (int l = 0; vars[l].name != NULL; ++l) {
                    if(strcmp(args[0], vars[l].name) == 0){
                        contin = 1;
                        break;
                    }
                }
                if(contin == 1){
                    contin = 0;
                    i++;
                    continue;
                }


                int status = execute_command(args); // Execute the command
                if (status == -1) {
                    printf("Failed to execute command\n");
                } else{
                    numOfTotalArgs += numArgs;
                }
            }
            i++;
        }
        //find the current location
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("getcwd() error");
            // Call freeMemory() function to free memory before exiting with specific exit code
            freeMemory(echo, echoNum, commands, numOfCmd, args, numArgs);
            return 1;
        }
        printf("#cmd:%d|#args:%d@%s>", numOfCmd, numOfTotalArgs, cwd);
    }
    // Call freeMemory() function to free memory before exiting with specific exit code
    freeMemory(echo, echoNum, commands, numOfCmd, args, numArgs);
    return 0;
}
