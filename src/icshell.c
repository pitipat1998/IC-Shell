#define _POSIX_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

/****************************************************
 ********************* MACROS ***********************
 ****************************************************/
#define MAXLINE 1024
#define MAXSIZE 64

pid_t shell_pgid;
char cmd[MAXLINE];
char *argv[MAXSIZE];

int child_notification(pid_t pid, int wstatus);

void execute(int bg, char **argv){
    pid_t pid;
    int wstatus;
    
    pid = fork();
    if (pid < 0){
        perror("***ERROR: fork failed\n");
        exit(1);
    }
    else if (pid == 0){
        signal(SIGINT, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        signal(SIGTTIN, SIG_DFL);
        signal(SIGTTOU, SIG_DFL);
        signal(SIGCHLD, SIG_DFL);
        setpgid(0,0);
        if(!bg)
            tcsetpgrp(0, getpid());

        printf("handle PID=%d\n", getpid());
        if(execvp(argv[0], argv) < 0){
            printf("***ERROR: exec failed\n");
            exit(1);
        }
    }
    else{
        setpgid(pid, pid);
        if(!bg){
            tcsetpgrp(0, pid);
            waitpid(-pid, &wstatus, WUNTRACED); 
            child_notification(pid, wstatus);
            tcsetpgrp(0, shell_pgid);
        }
    }
}
static char *trim(char *str){

    char *end;

    while (isspace((unsigned char) *str)) str++;

    if(!*str) 
        return str;
    
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char) *end)) end--;

    end[1] = '\0';
    return str;
}

static int parse(char *cmd, char **argv){
    cmd[strcspn(cmd, "\n")] = 0;
    cmd = trim(cmd);

    int i = 0;
    char *pattern = " \t\n";
    char *p = strtok(cmd, pattern);
    while (p != NULL) {
        argv[i++] = p;
        p = strtok(NULL, pattern);
    }

    for(int j=0; j<i; j++){
        argv[j] = trim(argv[j]);
    }

    argv[i] = '\0';
    return i;
}

void eval(char *cmd, char **argv){
    int bg = 0;
    int asize = parse(cmd, argv);
    if (argv[0] == NULL) return;
    if (!strcmp(argv[0], "exit")){
        exit(0);
    }
    if (!strcmp(argv[asize-1], "&")){
        bg = 1;
        argv[asize-1] = '\0';
    }
    execute(bg, argv);
}

int child_notification(pid_t pid, int wstatus){
    if (WIFEXITED(wstatus)){
        printf("%d exited, status=%d\n", pid, WEXITSTATUS(wstatus));
    }
    else if (WIFSIGNALED(wstatus)){
        printf("%d killed by signal %d\n", pid, WTERMSIG(wstatus));
    }
    else if (WIFSTOPPED(wstatus)){
        printf("%d stopped by signal %d\n", pid, WSTOPSIG(wstatus));
        return 0;
    }
    else if (WIFCONTINUED(wstatus)){
        printf("%d continued\n", pid);
    }
    return -1;
}

void sig_handler(int sig){
    pid_t pid;
    int wstatus;
    switch(sig){
        case SIGCHLD:
            while((pid = waitpid(-1, &wstatus, 0)) > 0){
                int stp = child_notification(pid, wstatus);
                if(stp)
                    break;
            }
            break;
    }
}

void init_shell(){
        struct sigaction action;
        action.sa_handler = sig_handler;
        action.sa_flags = SA_RESTART;
        sigfillset(&action.sa_mask);

        if(sigaction(SIGCHLD, &action, NULL))
            perror("***ERROR: SIGCHLD handler\n");
        signal(SIGINT, SIG_IGN);
        signal(SIGTSTP, SIG_IGN);
        signal(SIGQUIT, SIG_IGN);
        signal(SIGTTIN, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);

        shell_pgid = getpid();
        if(setpgid(shell_pgid, shell_pgid) < 0){
            perror("setpgid failed");
            exit(1);
        }

        tcsetpgrp(0, shell_pgid);
}

int main(){
    init_shell();
    while (1){
        printf("icsh> ");
        fgets(cmd, MAXLINE, stdin);        
        if(feof(stdin)){
            exit(0);
        }
        eval(cmd, argv);
    }
    return 0;
}
