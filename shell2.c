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

int executeCommand(char** cmd1, char** cmd2, char* infile1, char* infile2,
					char* outfile1, char* outfile2, char* appendfile1,
					char* appendfile2) {
	
	pid_t pid1, pid2;
	int fdin, fdout;
	int pipefd[2];
	int i = 0;
	int j = 0;
	int k = 0;
	char* hist = "\0";
	int offset = 0;
	
	if(strcmp(cmd1[0], "cd") == 0) {
		if(cmd1[1] == NULL) {
			chdir(getenv("HOME"));
		} else {
			chdir(cmd1[1]);
		}
		return 1;
	} else if(strcmp(cmd1[0], "history") == 0) {
		k = where_history();
		k += offset;
		j = k;
		i = 0;
		while(k > j - 10) {
			if(history_get(k) != NULL) {
				hist = history_get(k)->line;
				printf("%d: %s\n", i, hist);
			}
			i++;
			k--;
		}
		return 1;
	}
	
	if(cmd2[0] == NULL) {	
		pid1 = fork();
		if(pid1 < 0) {
			printf("\nCould not fork");
			return 1;
			// Error forking
		} else if(pid1 == 0) {
			if(strcmp(outfile1, "\0") != 0) {
				if((fdout = open(outfile1, O_CREAT|O_TRUNC|O_WRONLY, 0644)) < 0) {
					perror(outfile1);
					exit(1);
				}
				fflush(0);
				dup2(fdout, STDOUT_FILENO);
				close(fdout);
			} if(strcmp(appendfile1, "\0") != 0) {
				if((fdout = open(appendfile1, O_CREAT|O_WRONLY|O_APPEND, 0644)) < 0) {
					perror(appendfile1);
					exit(1);
				}
				fflush(0);
				dup2(fdout, STDOUT_FILENO);
				close(fdout);
			} if(strcmp(infile1, "\0") != 0) {
				if((fdin = open(infile1, O_RDONLY)) < 0) {
					perror(infile1);
					exit(1);
				}
				fflush(0);
				dup2(fdin, STDIN_FILENO);
				close(fdin);
			}
			if(execvp(cmd1[0], cmd1) < 0) {
				printf("\nCould not execute command..");
				exit(0);
			}
	
		} else {
			// Parent
			wait(NULL);
			return 1;
		}
	} else {
		
		if (pipe(pipefd) < 0) {
			printf("\nPipe could not be initialized");
			return 0;
		}
		
		pid1 = fork();
		
		if (pid1 < 0) {
			printf("\nCould not fork");
			return 0;
		}
	 
		if (pid1 == 0) {
			// Child 1 executing..
			// It only needs to write at the write end
			close(pipefd[0]);
			dup2(pipefd[1], STDOUT_FILENO);
			close(pipefd[1]);
			
			if(strcmp(outfile1, "\0") != 0) {
				if((fdout = open(outfile1, O_CREAT|O_TRUNC|O_WRONLY, 0644)) < 0) {
					perror(outfile1);
					exit(1);
				}
				fflush(0);
				dup2(fdout, STDOUT_FILENO);
				close(fdout);
			} if(strcmp(appendfile1, "\0") != 0) {
				if((fdout = open(appendfile1, O_CREAT|O_WRONLY|O_APPEND, 0644)) < 0) {
					perror(appendfile1);
					exit(1);
				}
				fflush(0);
				dup2(fdout, STDOUT_FILENO);
				close(fdout);
			} if(strcmp(infile1, "\0") != 0) {
				if((fdin = open(infile1, O_RDONLY)) < 0) {
					perror(infile1);
					exit(1);
				}
				fflush(0);
				dup2(fdin, STDIN_FILENO);
				close(fdin);
			}
	 
			if (execvp(cmd1[0], cmd1) < 0) {
				printf("\nCould not execute command 1..");
				exit(0);
			}
			
		} else {
			// Parent executing
			pid2 = fork();
	 
			if (pid2 < 0) {
				printf("\nCould not fork");
				return 0;
			}
	 
			// Child 2 executing..
			// It only needs to read at the read end
			if (pid2 == 0) {
				close(pipefd[1]);
				dup2(pipefd[0], STDIN_FILENO);
				close(pipefd[0]);
				
				if(strcmp(outfile2, "\0") != 0) {
					if((fdout = open(outfile2, O_CREAT|O_TRUNC|O_WRONLY, 0644)) < 0) {
						perror(outfile1);
						exit(1);
					}
					fflush(0);
					dup2(fdout, STDOUT_FILENO);
					close(fdout);
				} if(strcmp(appendfile2, "\0") != 0) {
					if((fdout = open(appendfile2, O_CREAT|O_WRONLY|O_APPEND, 0644)) < 0) {
						perror(appendfile1);
						exit(1);
					}
					fflush(0);
					dup2(fdout, STDOUT_FILENO);
					close(fdout);
				} if(strcmp(infile2, "\0") != 0) {
					if((fdin = open(infile2, O_RDONLY)) < 0) {
						perror(infile1);
						exit(1);
					}
					fflush(0);
					dup2(fdin, STDIN_FILENO);
					close(fdin);
				}
				
				if (execvp(cmd2[0], cmd2) < 0) {
					printf("\nCould not execute command 2..");
					exit(0);
				}
			} else {
				// parent executing, waiting for two children
				wait(NULL);
				wait(NULL);
			}
		}
	}
}

/*
 * Function:	int launchCommand(char**)
 * Description:	Launches commands using the proper
 *				run flags; parses args and runs
 *				commands based on order
 */
int launchCommand(char** args) {
	
	int i = 0;
	int cmd1index = 0;
	int cmd2index = 0;
	char** cmd1 = malloc(BUFSIZE * sizeof(char*));
	char** cmd2 = malloc(BUFSIZE * sizeof(char*));
	char* infile1 = "\0";
	char* outfile1 = "\0";
	char* appendfile1 = "\0";
	char* infile2 = "\0";
	char* outfile2 = "\0";
	char* appendfile2 = "\0";
	int outflag = 0;
	int inflag = 0;
	int appendflag = 0;
	int pipeflag = 0;
	
	while(args[i] != NULL) {
		if(outflag == 0 && inflag == 0 && appendflag == 0) {
			if(strcmp(args[i], ">") == 0) {
				outflag = 1;
			} else if(strcmp(args[i], "<") == 0) {
				inflag = 1;
			} else if(strcmp(args[i], ">>") == 0) {
				appendflag = 1;	
			}
		}
	    if(strcmp(args[i],  "|") == 0) {
			pipeflag = 1;
			i++;
		}
		if(pipeflag == 0) {
			if(outflag == 1) {
				i++;
				outfile1 = args[i];
				outflag = 0;
			} else if(inflag == 1) {
				i++;
				infile1 = args[i];
				inflag = 0;
			} else if(appendflag == 1) {
				i++;
				appendfile1 = args[i];
				appendflag = 0;
			} else {
				cmd1[cmd1index] = args[i];
				cmd1index++;
			}
		} else if(pipeflag == 1) {
			if(outflag == 1) {
				i++;
				outfile2 = args[i];
				outflag = 0;
			} else if(inflag == 1) {
				i++;
				infile2 = args[i];
				inflag = 0;
			} else if(appendflag == 1) {
				i++;
				appendfile2 = args[i];
				appendflag = 0;
			} else {
				cmd2[cmd2index] = args[i];
				cmd2index++;
			}
		}
	i++;
	}
	
	executeCommand(cmd1, cmd2, infile1, infile2, outfile1, outfile2, appendfile1, appendfile2);

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
		if(line == NULL) {
			continue;
		}
		args = splitLine(line);
		launchCommand(args);
	}
	return 0;
}