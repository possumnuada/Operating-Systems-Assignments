# Kapish
## CSC 360 - Operating Systems Assignment 1
### By Possum Nuada

## Resources
Instructions from "UVic CSC 360, Assignment 1: kapish?" (kapish.pdf)

I used code from the following sources:
* Reading a File Line By Line  - https://www.daniweb.com/programming/software-development/code/216411/reading-a-file-line-by-line
* Tutorial - Write a Shell in C - https://brennan.io/2015/01/16/write-a-shell-in-c/
* Printing all environment variables in C / C++ - https://stackoverflow.com/questions/2085302/printing-all-environment-variables-in-c-c

## Using kapish
1. Run the command "make" while in the same directory as kapish.c and makefile.
2. If you would like to run a kapish script, add a file named ".kapishrc" to the same directory.
   Each command should be on its own line.
3. Run the command "./kapish".
4. Enter commands
 * See built-in commands.
 * Or enter a program name to execute it.
5. Enter "exit" or Control-D to exit.

## Kapish Built-In Commands
### help
This command provides instructions and lists the built-in commands.

### setenv var [value]
This command sets the given variable var with the given value. If the variable does not exist, it will be created.

### unsetenv var
This command destroys the environment variable var.

### cd [dir]
This command changes kapish's working directory to dir, or to home the directory dir is omitted.

### exit
This command causes kapish to exit.

### printenv
This command prints all of the environment variables.

## Known Bugs
* Control-C is not handled in any special way.
