#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

// Static variable used to keep track of the foreground-mode status.
int foreground = 0;

/* Our signal handler for SIGINT */
void handle_SIGINT(int signo) {
    // When Ctrl-C is entered, it is ignored and a new line is printed.
    write(STDOUT_FILENO, "\n", 39);
    fflush(stdout);
}

/* Our signal handler for SIGTSTP */
void handle_SIGTSTP(int signo) {
    // Entering Ctrl-Z will be ignored and will print whether or not the system will enter or exit foreground-only mode.
    if (foreground == 0) {
        // If the process is not in foreground-mode, then the process will switch to foreground-mode.
        foreground = 1;
        write(STDOUT_FILENO, "\nEntering foreground-only mode (& is now ignored)\n", 39);
        fflush(stdout);
    } else if (foreground == 1) {
        // If the process is in foreground-mode, then the process will leave it.
        foreground = 0;
        write(STDOUT_FILENO, "\nExiting foreground-only mode.\n", 39);
        fflush(stdout);
    }
}

/*
 * This function should print out the terminating signal or the exit value
 * (which represents status of the current process).
 *
 * Params:
 *   childExitStatus - the exit status of the process.
 */
void getStatus(int childExitStatus) {
  if (!WIFEXITED(childExitStatus)) // If process is terminated normally, print terminating signal.
      printf("terminated by signal %d\n", childExitStatus);
  else // If process was not terminated, print exit value. 
      printf("exit value %d\n", WEXITSTATUS(childExitStatus));
      
  fflush(stdout);
}

/*
 * This function should free all memory.
 *
 * Params:
 *   commands - the command line.
 *   count - the number of words in the command line.
 */
void freeMemory(char* commands[2048], int count) {
    // Iterating through the list and deleting memory
    for (int x = 0; x < count; x++)
        free(commands[x]);
}

int main() {
    char input[2048], save[512], *token;
    int file = 0, childExitStatus = 0, background = 0;
    pid_t spawnpid = -5;

    // Initialize SIGINT_action struct to be empty.
    struct sigaction SIGINT_action = { 0 };
    // Fill out the SIGINT_action struct.
    // Register handle_SIGINT as the signal handler.
    SIGINT_action.sa_handler = handle_SIGINT;
    // Block all catchable signals while handle_SIGINT is running.
    sigfillset(&SIGINT_action.sa_mask);
    // No flags set.
    SIGINT_action.sa_flags = 0;
    // Install our signal handler.
    sigaction(SIGINT, &SIGINT_action, NULL);

    // Initialize SIGTSTP_action struct to be empty.
    struct sigaction SIGTSTP_action = { 0 };
    // Fill out the SIGTSTP_action struct.
    // Register handle_SIGTSTP as the signal handler.
    SIGTSTP_action.sa_handler = handle_SIGTSTP;
    // Block all catchable signals while handle_SIGTSTP is running.
    sigfillset(&SIGTSTP_action.sa_mask);
    // No flags set.
    SIGTSTP_action.sa_flags = 0;
    // Install our signal handler.
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);

    // Infinite oop will continue to run until the user enters "exit".
    while (1) {
        int count = 0;
        char* inFile = NULL, * outFile = NULL, * commands[2048];

        // Enter command.
        printf(": ");
        fflush(stdout);
        fgets(input, 512, stdin);

        token = strtok(input, " ");

        // Iterating through the command line.
        while (token != NULL) {
            if (strcmp(token, "<") == 0) { // Collect input file.
                token = strtok(NULL, " \n");
                inFile = strdup(token);
            } else if (strcmp(token, ">") == 0) { // Collect output file.
                token = strtok(NULL, " \n");
                outFile = strdup(token);
            } else { // Collect all other info.
                sscanf(token, "%s", save);
                
                // Replace "$$" with process ID.
                if (strstr(token, "$$") != NULL) {
                  char newVal[512] = "", pidNum[10];
                  int x = 0, index = 0, pid = getpid();
                
                  sprintf(pidNum, "%d", pid); // Convert pid datatype from int to char.
                
                  // Iterating through the command.
                  while (x < strlen(save)) {
                      // If '$' is encountered.
                      if (save[x] == '$') {
                          // If another '$' is encountered, replace with process ID.
                          if (x + 1 == strlen(save) || save[x + 1] != '$') {
                              newVal[index] = '$';
                          } else { // If the next char is not another '$' then leave the chars as it is.
                              strcat(newVal, pidNum);
                              index += strlen(pidNum) - 1;
                              x++;
                          }
                      } else {
                          newVal[index] = save[x]; // Leave the character as it is if it's not "$$".
                      }
                
                      index++;
                      x++;
                  }
                
                  strcpy(save, newVal);
                } 
                
                commands[count] = strdup(save);
                count++;
            }
            
            // If '&' occurs in the middle of the command, it'll be treated as a normal text.
            // The next if statement will check if '&' occurs at the end of the command.
            token = strtok(NULL, " ");
        }

        // Checking for "&" at the end command.
        if (strcmp(commands[count - 1], "&") == 0) {
            // If '&' at the end of the command, the background process is entered.
            commands[count - 1] = NULL;
            background = 1;
        } else {
            // If '&' is not at the end, then background process will not be entered.
            commands[count] = NULL;
            background = 0;
        }

        if (strcmp(commands[0], "exit") == 0) { // If the user enters "exit" the program will stop.
            freeMemory(commands, count); // Free all memory
            exit(0);
        } else if (strcmp(commands[0], "cd") == 0) { // If the user enters "cd" the program will change directories.
            if (count == 2) { // If a directory is given, check if it exists.
                if (chdir(commands[1]) != 0) { // If the directory doesn't exist.
                    printf("%s: no such file or directory\n", commands[1]);
                    fflush(stdout);
                } else { // Program changes to the directory to the given one.
                    chdir(commands[1]);
                }
            } else { // If the directory is not given, go to main/home directory.                                                                       
                chdir(getenv("HOME"));
            }
        } else if (strcmp(commands[0], "status") == 0) { // If the user enters "status" the program will print out the process status.
            getStatus(childExitStatus);
        } else if (input[0] != '\n' && commands[0][0] != '#') { // If the user enters any other command (entering a new line or '#' will be ignored).
            spawnpid = fork();

            // If fork is successful, the value of spawnpid will be 0 in the child, the child's pid in the parent.
            switch (spawnpid) {
              case -1:
                  // Code in this branch will be exected by the parent when fork() fails and the creation of child process fails as well.              
                  printf("fork() failed!\n");
                  fflush(stdout);
                  childExitStatus = 1;
                  break;
              case 0:
                  // spawnpid is 0. This means the child will execute the code in this branch.
                  // If background is not in progress or foreground mode is in progess, the program will enable Ctrl-C.                                 
                  if (background == 0 || foreground == 1) {
                      SIGINT_action.sa_handler = SIG_DFL;
                      sigaction(SIGINT, &SIGINT_action, NULL);
                  }
  
                  // Check if there's an input file.
                  if (inFile != NULL) {
                      file = open(inFile, O_RDONLY);
  
                      // Checking if the file can be opened.
                      if (file == -1) {
                          // If not, error message is printed and exit status is set to 1.
                          printf("cannot open %s for input\n", inFile);
                          fflush(stdout);
                          exit(1);
                      }
  
                      // Checking if the file can be duplicated.
                      if (dup2(file, 0) == -1) {
                          // If not, error message is printed and exit status is set to 1.
                          printf("dup2\n");
                          fflush(stdout);
                          exit(1);
                      }
  
                      close(file);
                  }
  
                  // Check if there's an output file.
                  if (outFile != NULL) {
                      file = open(outFile, O_WRONLY | O_CREAT | O_TRUNC, 0640);
  
                      // Checking if the file can be opened.                                                      
                      if (file == -1) {
                          // If not, error message is printed and exit status is set to 1.
                          printf("cannot open %s for output\n", outFile);
                          fflush(stdout);
                          exit(1);
                      }
  
                      // Checking if the file can be duplicated.
                      if (dup2(file, 1) == -1) {
                          // If not, error message is printed and exit status is set to 1.
                          printf("dup2\n");
                          fflush(stdout);
                          exit(1);
                      }
  
                      close(file);
                  }
                  
                  // Check if background is in progress
                  if (background == 1) {
                    // Check if there's an input file.
                    if (inFile != NULL) {
                        // In the background process, standard input should be redirected to "/dev/null".
                        file = open("/dev/null", O_RDONLY);
    
                        // Checking if the file can be opened.
                        if (file == -1) {
                            // If not, error message is printed and exit status is set to 1.
                            printf("cannot open %s for input\n", inFile);
                            fflush(stdout);
                            exit(1);
                        }
    
                        // Checking if the file can be duplicated.
                        if (dup2(file, 0) == -1) {
                            // If not, error message is printed and exit status is set to 1.
                            printf("dup2\n");
                            fflush(stdout);
                            exit(1);
                        }
    
                        close(file);
                    }
    
                    // Check if there's an output file.
                    if (outFile != NULL) {
                        // In the background process, standard output should be redirected to "/dev/null".
                        file = open("/dev/null", O_WRONLY | O_CREAT | O_TRUNC, 0640);
    
                        // Checking if the file can be opened.                                                      
                        if (file == -1) {
                            // If not, error message is printed and exit status is set to 1.
                            printf("cannot open %s for output\n", outFile);
                            fflush(stdout);
                            exit(1);
                        }
    
                        // Checking if the file can be duplicated.
                        if (dup2(file, 1) == -1) {
                            // If not, error message is printed and exit status is set to 1.
                            printf("dup2\n");
                            fflush(stdout);
                            exit(1);
                        }
    
                        close(file);
                    }
                  }
  
                  // Running the command and checking if it's a valid command.
                  if (execvp(commands[0], commands) < 0) {
                      // If execvp returns a value less than 0, then the user didn't enter a valid command.
                      // Error statement is then printed and exit status is set to 1.
                      printf("%s: no such file or directory\n", commands[0]);
                      fflush(stdout);
                      exit(1);
                  }
                  
                  break;
              default: // Wait for the child process to end or print out background pin.   
                  // If not in background mode or if in foreground-mode, the program will wait for child process to finish.                             
                  if (background == 0 || foreground == 1) { 
                      waitpid(spawnpid, &childExitStatus, 0);
                  // If in background process or not in foreground-mode, the program will print out the background pin.
                  } else if (background == 1 || foreground == 0) {
                      printf("background pid is %d\n", spawnpid);
                      fflush(stdout);
                  }
                  
                  break;
              }
        }
        
        // Check for zombie-children.
        spawnpid = waitpid(-1, &childExitStatus, WNOHANG); 

        // Check for any existing zombie-children (or completed background processes).
        while (spawnpid > 0) { // If spawnpid is less than 0 then there are zombie-children.
            printf("background pid %d is done: ", spawnpid);
            fflush(stdout);
            getStatus(childExitStatus); // Printing the status of the child process.
            spawnpid = waitpid(-1, &childExitStatus, WNOHANG); // Check for zombie-children.
        }

        // Freeing memory.
        freeMemory(commands, count);
    }
    
    return 0;
}
