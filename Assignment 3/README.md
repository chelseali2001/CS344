Name: Chelsea Li 
ID: 933294417
Date: 02/08/2021

How to compile program: gcc --std=gnu99 -o smallsh lichel_program3.c
How to run program: smallsh

Notes: Structure for sigaction, handling fork() with a switch statement, and checking the input/output files, and executing a command are based off class materials.

Instructions:
1. User will be presented with ": " as a prompt for each command line.
2. If '&' is entered at the end of the command line, the command will be executed in the background.
3. Entering > or < will redirect the standard input or output.
4. Entering '#' or a blank line will be ignored.
5. Entering "$$" will be converted to the process ID of the smallsh.
6. Entering "exit" will exit the shell.
7. Entering "cd" by itself will change the current directory to the home directory.
8. Entering "cd" and the name of a directory will change the current directory to that directory. (Program also checks if the directory is valid)
9. Entering "status" will print out either the exit status or the terminating signal of the last foreground process ran by your shell.
10. The shell supports any other commands and checks if they are valid commands.
11. When Ctrl-C or Ctrl-Z is entered, it'll be ignored (unless in foreground-mode) and a new line will be printed.
