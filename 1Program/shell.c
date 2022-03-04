#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <stdbool.h>

#define MAX_LINE 80 /* The maximum length command */

int main() {

  char userInput[MAX_LINE/2 + 1]; // user input buffer
  char history[MAX_LINE/2 + 1] = "\0"; // history buffer
  char *cmd[MAX_LINE/2 + 1]; // command array to be used in execvp
  const char DELIM[2] = " "; // char that separates user input
  const char *WRITE = "w";
  const char *READ = "r";

  while(true) {
    printf("osh>"); // prompt user

    for (int i = 0; i < strlen(userInput); i++) { // null out userInput buffer
      userInput[i] = '\0';
    }
    fgets(userInput, MAX_LINE/2, stdin); // recieve input from user
    userInput[strlen(userInput) - 1] = '\0'; // switch '\n' to '\0'

    if (strcmp(userInput,"exit") == 0) { // check for exit
      return 0;
    }

    int rtf = fork(); // fork

    if (rtf == -1) { // if fork failed
      printf("Could not fork.");
    }

    /**********************************Child***********************************/
    if (rtf == 0) {
      if (strcmp(userInput,"!!") == 0) { // if userInput "!!"
        if (strlen(history) == 0) { // if nothing in history
          printf("No commands in history.\n");
          exit(0); // end process
        }
        strcpy(userInput, history); // copy history into userInput
      }

      char *token; // used to store individual tokens from user input
      token = strtok(userInput, DELIM); // get first token
      cmd[0] = token; // add first token to command array

      char *fileName; // used to store text file for redirecting input/output

      /* Get other tokens and add to command array. Last token adden to cmd
      will be null which is what is wanted for execvp. */
      int i = 1;
      while (token != NULL) {
        token = strtok(NULL, DELIM); // get token

        /* If token is not NULL or token is not "&" and token is not ">" and
        token is not "<" and token is not "|". Must include the Null part
        because strcmp does not work properlly with NULL. */
        if (token == NULL || strcmp(token,"&") != 0 && strcmp(token,">") != 0
            && strcmp(token,"<") != 0 && strcmp(token,"|") != 0) {
          cmd[i++] = token; // add token to command array
        }

        else if (strcmp(token,">") == 0) { // redirect output
          fileName = strtok(NULL, DELIM); // get file name
          FILE *file = fopen(fileName, WRITE); // open file to write
          dup2(fileno(file), STDOUT_FILENO); // duplicate file to stdout
        }

        else if (strcmp(token,"<") == 0) { // redirect input
          fileName = strtok(NULL, DELIM); // get file name
          FILE *file = fopen(fileName, READ); // open file to read
          if (file == NULL) { // could not find file
            printf("Requested file not found.\n");
            exit(0); // end process
          }
          dup2(fileno(file), STDIN_FILENO); // duplicate file to stdin
        }

        else if (strcmp(token,"|") == 0) { // pipe
          int pipeFD[2]; // set up pipe
          pipe(pipeFD); // pipe

          int rtf2 = fork(); // fork

          /***************************Child's Child****************************/
          if (rtf2 == 0) {
            close(pipeFD[0]); // close read side of pipe
            dup2(pipeFD[1], STDOUT_FILENO); // duplicate write pipe to stdout

            cmd[i] = NULL; // end command array with NULL

            /* run exec, last string of cmd must be NULL */
            int rc2 = execvp(cmd[0], cmd);

            if (rc2 == -1) { // if exec failed
              fprintf(stderr, "Cannot exec: %s.\n", cmd[0]);
              exit(0); // end process
            }
          }

          /*******************************Child********************************/
          close(pipeFD[1]); // close write side of pipe
          dup2(pipeFD[0], STDIN_FILENO); // duplicate read pipe to stdin

          /* Get tokens and add to command array. */
          int j = 0;
          while (token != NULL) {
            token = strtok(NULL, DELIM); // get token

            /* If token is not NULL or token is not "&" */
            if (token == NULL || strcmp(token,"&") != 0) {
              cmd[j++] = token; // add token to child command array
            }
          }
        }

        else {} // token is "&" and do nothing
      }

      int rc = execvp(cmd[0], cmd); // run exec, last string of cmd must be NULL
      if (rc == -1) { // if exec failed
        printf("Cannot exec: %s.\n", cmd[0]);
        exit(0); // end process
      }
    }

    /**********************************Parent**********************************/
    else {
      /* If user input not "!!", copy userInput into history. This also allows
      "!!" to be run multiple times in a row because it won't write over the
      history if "!!" is called. */
      if (strcmp(userInput,"!!") != 0) {
        strcpy(history, userInput);
      }
      else { // if userInput "!!"
        strcpy(userInput, history); // copy history into userInput
      }

      if (userInput[strlen(userInput) - 1] != '&') { // if last char not '&'
        waitpid(rtf, NULL, 0); // wait for child to finish
      }
      else { // if last char '&'
        continue;
      }
    }
  }
  return 0;
}
