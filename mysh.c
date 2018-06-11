#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_ARGS 128

int run_command(char** args, int amp) { //amp = 0 when there is an ampersand, 1 otherwise
     
   
  //do nothing if the user doesn't give a command
     if (args[0]=='\0') { 
     }
     //if the command is 'cd', change directories with the chdir command
     else if (strcmp(args[0], "cd") == 0) {
       int rce = chdir(args[1]);
       if (rce == -1) {
         perror("chdir");
       }
     }
     //handle exit command
     else if(strcmp(args[0], "exit") == 0) {
       printf("\nShutting down...\n");
       exit(0);
       return 0;
     }
     //handle other commands
     else {
       int ret;
       ret = fork();
       if(ret < 0) { //failed to fork
         perror("fork");
       }
       else if(ret > 0) { // in the parent 
         int status;
         
         if(amp==0){ //ampersand with command
           return 0;
         }
         else { //no ampersand with command
           
           wait(&status);
           printf("Child process %d exited with status %d\n", ret, status);
           return 0;
         }
         
       }
       else { // in the child
         int ret_child = -1;
         if(amp==0) { //ampersand with command
           ret_child = fork();
         }
        
         if (ret_child > 0) { //in child-parent after fork
         //monitor life of child-child
           int status_child;
           wait(&status_child);
           printf("Child-child process %d exited with status %d\n", ret_child, status_child);
           return 1;
         }
         else { //in child-child after fork or in child if didn't fork
           int rc = execvp(args[0], args);//cmd,argv);
           if (rc == -1) {
             perror("execvp");
           }
           printf("should not print!!!");
         }
       }
     }




}

int main(int argc, char** argv) {
  while(1) {
    char* line = NULL;    // Pointer that will hold the line we read in
    size_t line_length;   // Space for the length of the line
    
    // Print the shell prompt
    printf("> ");
    // Get a line of stdin, storing the string pointer in line and length in line_length
    if(getline(&line, &line_length, stdin) == -1) {
      if(errno == EINVAL) {
        perror("Unable to read command line");
        exit(2);
      } else {
        // Must have been end of file (ctrl+D)
        printf("\nShutting down...\n");
        exit(0);
      }
    }
    
    //printf("Received command: <<%s>>\n", line);
    
    char* commands[50]; // individual commands from stdin
    char* and_commands[50]; //commands preceded or followed by &
    char* all_commands[100]; //commands/groups of commands seperated by semicolon
    //memset(all_commands, NULL, 101);
    char* token; //pointer to a token in stdin with semicolon as delimeter
    char* and_token; //pointer to a token in stdin with & as delimiter
    char* all_token; //
    int j = 0; //number of commands separated by semicolon
    int k = 0; //index for looping through semicolon separated commands
    int l = 0; //number of commands separated by ampersand
    int m = 0; //index for looping through ampersand separated commands

    /*tokenize stdin by semicolon and newline to get individual commands and
    their arguments */
    all_token = strtok(line, ";\n");
    //traverse all the tokens in stdin
    while (all_token != NULL) {
      all_commands[j++]=all_token; //add tokens to an array
      all_token = strtok(NULL, ";\n");
    }

    //loop through command tokens separated by semicolon
    for (k; k<j; k++) {

      if(strchr(all_commands[k], '&') != NULL ) { // if there is an ampersand (don't wait)
         memset(and_commands, '\0', 51);
         //traverse all the tokens with an ampersand before/after
         and_token = strtok(all_commands[k], "&");
         while (and_token != NULL) {
           and_commands[l++]=and_token; //add tokens to an array
           and_token = strtok(NULL, "&");
         }

         //loop through commands with ampersand before/after
        for (m; m<l; m++) {
          int i = 0;
          memset(commands, '\0', 51);
          token = strtok(and_commands[m], " \n"); //get each command and its arguments as tokens
          while (token != NULL) {
            commands[i++]=token;
            token = strtok(NULL, " \n");
          }
          commands[i] = NULL;
          if(run_command(commands, 0) != 0) { //run command; if returning from run_command as a child, don't continue
            break;
          }
        }
      }        
      else { //if no ampersand
        int i = 0;
        memset(commands, '\0', 51);
        token = strtok(all_commands[k], " \n"); //get each command and its arguments as tokens
        while (token != NULL) {
          commands[i++]=token;
          token = strtok(NULL, " \n");
        }
        commands[i] = NULL;

        run_command(commands, 1); //run command
      }

   }
   free(line);
 }
  
  return 0;
}
