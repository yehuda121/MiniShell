# MiniShell.c
A simple command-line interpreter written in C.

# MiniShell
MiniShell is a simple command-line interpreter written in C. It supports basic command execution, variable assignment, and an echo command. MiniShell reads user input, parses it into commands and arguments, and executes these commands. The shell also handles user-defined variables and provides a simple prompt indicating the current working directory.

## Features
- Execute standard shell commands.
- Set and use variables.
- Echo strings.
- Simple prompt showing the current working directory.

## Usage
### Compilation
To compile the MiniShell, use the following command:
gcc -o minishell minishell.c

### Running MiniShell
To start MiniShell, simply run the compiled executable:
./minishell

### Commands
MiniShell supports the execution of standard shell commands, setting and using variables, and echoing strings. Below are some examples of how to use MiniShell:

#### Executing Commands
# Running a simple command
ls -l

#### Setting and Using Variables
# Setting a variable
name=John
# Using a variable
echo $name

#### Echoing Strings
# Echoing a string with double quotes
echo "Hello, World"
# Echoing a string with single quotes
echo 'Hello, World'
# Echoing a string without quotes
echo Hello World

## Limitations
- The `cd` command is not supported.
- Commands with more than 10 arguments are not supported.
- Exiting the shell after three consecutive empty inputs.

## Exit
To exit MiniShell, simply type `exit` or press `Ctrl+C` or type three consecutive empty inputs.
