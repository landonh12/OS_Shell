/*
 *	       File: shell.c
 *	     Author: Landon Haugh (lrh282)
 *	Description: Custom shell program for Operating Systems I
 *				 with Thomas Ritter. This shell supports 
 * 				 input/output redirection, append, piping,
 * 				 background processing, SIGINT ingoring,
 * 				 and builtin commands (cd, history, exit).
 *
 *	Due Date: 2-27-18
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <fcntl.h>

/*
 * Function: 	void readCommand(void)
 * Description: Returns a char* buffer from user input.
 */
char* readCommand() {
	char* buf;
	buf = readline("\n$ ");
	if (strlen(buf) != 0) {
		add_history(buf);
		return buf;
	} else {
		return NULL;
	}	
}

/*
 * Function:    char** splitLine(char*)
 * Description: Returns a char** array formed from tokenizing
 * 				a char* array.
 */
#define BUFSIZE 64
char** splitLine(char* line) {
	
	char* token;
	char** args = malloc(BUFSIZE * sizeof(char*));
	int i = 0;
	
	token = strtok(line, " ");
	while(token != NULL) {
		args[i] = token;
		token = strtok(NULL, " ");
		i++;
	}
	
	return args;
	
}

/*
 * Function:	int* parseCommand(char**)
 * Description: Returns an int* array of flags
 *				stating whether or not the command
 *				contains command functions
 * 				(I/O redirection, bg processing, etc)
 */
char** findFunctions(char** args) {
	
	char** flag = malloc(5 * sizeof(char));								// Not sure why but I have to malloc this
	int i = 0;
	int j = 0;
	
	while(args[i] != NULL) {							// Loop through the args array and check for functions
														// Store function flags in an int* array
		if(strcmp(args[i], ">") == 0) {
			flag[j] = args[i];
			j++;
		}
		else if(strcmp(args[i], "<") == 0) {
			flag[j] = args[i];
			j++;
		}
		else if(strcmp(args[i], ">>") == 0) {
			flag[j] = args[i];
			j++;		
		}
		else if(strcmp(args[i], "|") == 0) {
			flag[j] = args[i];
			j++;
		}
		i++;
		
	}
	
	return flag;
	
}

int executeCommand(char** cmd, char* file, char* flag) {
	
	pid_t pid;
	int fdin, fdout;
	
	pid = fork();
	if(pid < 0) {
		printf("\nCould not fork");
		return 1;
		// Error forking
	} else if(pid == 0) {
		// Child
		if(strcmp(flag, ">") == 0) {
			if((fdout = open(file, O_CREAT|O_TRUNC|O_WRONLY, 0644)) < 0) {
				perror(file);
				exit(1);
			}
			fflush(0);
			dup2(fdout, STDOUT_FILENO);
		} if(strcmp(flag, ">>") == 0) {
			if((fdout = open(file, O_CREAT|O_WRONLY|O_APPEND, 0644)) < 0) {
				perror(file);
				exit(1);
			}
			fflush(0);
			dup2(fdout, STDOUT_FILENO);
		} if(strcmp(flag, "<") == 0) {
			if((fdin = open(file, O_RDONLY)) < 0) {
				perror(file);
				exit(1);
			}
			fflush(0);
			dup2(fdin, STDIN_FILENO);
		}
		if(execvp(cmd[0], cmd) < 0) {
			printf("\nCould not execute command..");
			exit(0);
		}
	} else {
		// Parent
		wait(NULL);
		return 1;
	}
	
}

/*
 * Function:	int launchCommand(char**, int*)
 * Description:	Launches commands using the proper
 *				run flags; parses args and runs
 *				commands based on order
 */
int launchCommand(char** args, char** flag) {
	
	int i = 0;
	int j = 0;
	int k = 0;
	char** cmd = malloc(BUFSIZE * sizeof(char*));
	char* file;
	
	if(flag[0] == NULL) {
		executeCommand(args, NULL, NULL);
	}
	
	while(flag[i] != NULL) {
		while(strcmp(args[j], flag[i]) != 0) {
			j++;
		}
		if(strcmp(args[j], flag[i]) == 0) {
			for(k = 0; k < j; k++) {
				cmd[k] = args[k];
			}
			file = args[k + 1];
			executeCommand(cmd, file, flag[i]);
		}
		i++;
	}

	return 1;
}

/*
 * Function:	int main()
 * Description:	Main function. Contains the
 *				main while loop responsible for
 *				parsing commands input by the user.
 */
int main() {
	
	char* line;
	char** args;
	char** flag;
	
	while(1) {
		line = readCommand();
		args = splitLine(line);
		flag = findFunctions(args);
		launchCommand(args, flag);
	}
	return 0;
}
