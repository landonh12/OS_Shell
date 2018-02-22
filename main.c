// C Program to design a shell in Linux
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<readline/readline.h>
#include<readline/history.h>
#include<fcntl.h>

#define MAXCOM 1000 // max number of letters to be supported
#define MAXLIST 100 // max number of commands to be supported
 
// Clearing the shell using escape sequences
#define clear() printf("\033[H\033[J")

int offset = 0;
 
// Function to take input
char* takeInput(void)
{
    char* buf;
 
    buf = readline("$ ");
    if (strlen(buf) != 0) {
        add_history(buf);
        return buf;
    } else {
        return NULL;
    }
}
 
// Function to print Current Directory.
void printDir()
{
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("\n%s:", cwd);
}

void sigint(int signo) {
    signal(SIGINT,sigint);
    printf("CHILD: I have received a SIGINT\n");
    return;
}
 
// Function where the system command is executed
void execArgs(char** parsed)
{
    struct sigaction act;
    act.sa_handler = sigint;

    // Forking a child
    pid_t pid = fork(); 
 
    if (pid == -1) {
        printf("\nFailed forking child..");
        return;
    } else if (pid == 0) {
        if (execvp(parsed[0], parsed) < 0) {
            printf("\nCould not execute command..");
        }
        exit(0);
    } else {
        // waiting for child to terminate
        sigaction(SIGINT, &act, NULL);
        wait(NULL); 
        return;
    }
}
 
// Function where the piped system commands is executed
void execArgsPiped(char** parsed, char** parsedfunc)
{
    // 0 is read end, 1 is write end
    int pipefd[2]; 
    pid_t p1, p2;
 
    if (pipe(pipefd) < 0) {
        printf("\nPipe could not be initialized");
        return;
    }
    p1 = fork();
    if (p1 < 0) {
        printf("\nCould not fork");
        return;
    }
 
    if (p1 == 0) {
        // Child 1 executing..
        // It only needs to write at the write end
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
 
        if (execvp(parsed[0], parsed) < 0) {
            printf("\nCould not execute command 1..");
            exit(0);
        }
    } else {
        // Parent executing
        p2 = fork();
 
        if (p2 < 0) {
            printf("\nCould not fork");
            return;
        }
 
        // Child 2 executing..
        // It only needs to read at the read end
        if (p2 == 0) {
            close(pipefd[1]);
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[0]);
            if (execvp(parsedfunc[0], parsedfunc) < 0) {
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

/*
 *
 *
 */
void execArgsOutRedir(char** parsed, char** parsedfunc) {

    pid_t pid;
    struct sigaction act;
    act.sa_handler = sigint;
    int fd;
    
    pid = fork();
    if(pid < 0) {
        printf("\nCould not fork");
        return;
        // Error forking
    } else if(pid == 0) {
        // Child
        if((fd = open(parsedfunc[strlen(*parsedfunc)], O_CREAT|O_TRUNC|O_WRONLY, 0644)) < 0) {
            perror(parsedfunc[i]);
            exit(1);
        }
        fflush(0);
        dup2(fd, STDOUT_FILENO);
        if(execvp(parsedfunc[0], parsedfunc) < 0) {
            printf("\nCould not execute command..");
            exit(0);
        }
    } else {
        // Parent
        sigaction(SIGINT, &act, NULL);
        wait(NULL);
        return;
    }

}

void execArgsAppend(char** parsed, char** parsedfunc) {

}

void execArgsInRedir(char** parsed, char** parsedfunc) {

}
 
// Help command builtin
void openHelp()
{
    puts("\n***WELCOME TO MY SHELL HELP***"
        "\nCopyright @ Suprotik Dey"
        "\n-Use the shell at your own risk..."
        "\nList of Commands supported:"
        "\n>cd"
        "\n>ls"
        "\n>exit"
        "\n>all other general commands available in UNIX shell"
        "\n>pipe handling"
        "\n>improper space handling");
 
    return;
}
 
// Function to execute builtin commands
int ownCmdHandler(char** parsed)
{
    int NoOfOwnCmds = 5, i, switchOwnArg = 0;
    char* ListOfOwnCmds[NoOfOwnCmds];
    char* username;
    char* hist;
    int k = 0;
    int j = 0;

    ListOfOwnCmds[0] = "exit";
    ListOfOwnCmds[1] = "cd";
    ListOfOwnCmds[2] = "help";
    ListOfOwnCmds[3] = "hello";
    ListOfOwnCmds[4] = "history";
 
    for (i = 0; i < NoOfOwnCmds; i++) {
        if (strcmp(parsed[0], ListOfOwnCmds[i]) == 0) {
            switchOwnArg = i + 1;
            break;
        }
    }
 
    switch (switchOwnArg) {
    case 1:
        printf("\nGoodbye\n");
        exit(0);
    case 2:
        if(parsed[1] == NULL) {
            chdir(getenv("HOME"));
        }
        chdir(parsed[1]);
        return 1;
    case 3:
        openHelp();
        return 1;
    case 4:
        username = getenv("USER");
        printf("\nHello %s.\nMind that this is "
            "not a place to play around."
            "\nUse help to know more..\n",
            username);
        return 1;
    case 5:
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
    default:
        break;
    }
 
    return 0;
}
 
// function for finding cmd func
int parseFunc(char** args, char** parsed)
{
    int i = 0;
    int flag = 0;
    
    while(args[i] != NULL) {
        if(strcmp(args[i], "|") == 0) {
            flag = 1;
            i++;
            parsed[i] = args[i];
        } else if(strcmp(args[i], ">") == 0) {
            flag = 2;
            i++;
            parsed[i] = args[i];
        } else if(strcmp(args[i], ">>") == 0) {
            flag = 3;
            i++;
            parsed[i] = args[i];
        } else if(strcmp(args[i], "<") == 0) {
            flag = 4;
            i++;
            parsed[i] = args[i];
        } else {
            parsed[i] = args[i];
        }
        i++;
    }
    
    return flag + 1;
}

/*
 *
 *
 */
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"
char **lsh_split_line(char *line)
{
    int bufsize = LSH_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;
    
    if (!tokens) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }
    
    token = strtok(line, LSH_TOK_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;
        
        if (position >= bufsize) {
            bufsize += LSH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        
        token = strtok(NULL, LSH_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}
 
int main()
{
    int execFlag = 0;
    char *inputString, **inputArgs;
    char *parsedArgs[MAXLIST];
    while (1) {
        // Ignore SIGINT
        signal(SIGINT, SIG_IGN);
        // print shell line
        printDir(); 
        // take input
        inputString = takeInput();
        if (inputString == NULL)
            continue;
        // process
        inputArgs = lsh_split_line(inputString);
        execFlag = parseFunc(inputArgs, parsedArgs);
        // execflag returns zero if there is no command
        // or it is a builtin command,
        // 1 if it is a simple command
        // 2 if it is including a pipe.
        // 3 if it includes output redirection.
        // 4 if it includes append.
        // 5 if it includes input redirection.
        
        // execute
        if(execFlag == 1) {
            if(!ownCmdHandler(parsedArgs)) {
                execArgs(parsedArgs);
            }
        }
 
        if(execFlag == 2)
            execArgsPiped(inputArgs, parsedArgs);

        if(execFlag == 3)
            execArgsOutRedir(inputArgs, parsedArgs);

        if(execFlag == 4)
            execArgsAppend(inputArgs, parsedArgs);

        if(execFlag == 5)
            execArgsInRedir(inputArgs, parsedArgs);

        offset++;
    }
    return 0;
}
