# Description
`SimpleShell` is a basic Unix shell implementation that supports command execution, background processes, piping, variable management, and signal handling for job control. It reads commands from the user, parses them, executes them, and handles special built-in commands like `echo` and variable assignments.

### README.md
## Features
- **Command Execution**: Execute standard Unix commands.
- **Background Processes**: Run processes in the background using `&`.
- **Piping**: Use pipes (`|`) to connect the output of one command to the input of another.
- **Variable Management**: Set and use shell variables.
- **Signal Handling**: Manage signals such as SIGTSTP (Ctrl+Z) to suspend processes.
- **Built-in Commands**: Supports built-in commands like `echo`.

## Compilation
To compile the SimpleShell program, use the following command:
gcc -o SimpleShell SimpleShell.c

## Usage
Run the compiled `SimpleShell` executable:
./SimpleShell

### Examples
- **Execute a command**:
    ls -l
- **Run a command in the background**:
    sleep 10 &
- **Use piping**:
    ls -l | grep txt
- **Set and use a variable**:
    MY_VAR=Hello
    echo $MY_VAR
- **Suspend the last foreground process with Ctrl+Z**.

## Signal Handling
SimpleShell handles SIGTSTP (Ctrl+Z) to suspend the last running foreground process. When you press Ctrl+Z, the currently running command will be suspended.

## Limitations
- **No `cd` support**: The `cd` command is not supported and will display a message indicating this.
