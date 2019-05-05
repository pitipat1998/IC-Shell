#define _POSIX_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

#include <cmdp.h>
#include <psignal.h>

/****************************************************
 ********************* MACROS ***********************
 ****************************************************/
#define MAXLINE 1024
#define MAXSIZE 64

typedef struct process
{
    struct process *next;
    char **argv;
    pid_t pid;
    int status;
} process;

pid_t shell_pgid;
process *phead = NULL;

void child_handler(int sig);
void sig_handler(int sig);
static void init_shell();
static void eval(char *cmd);
static int builtin_command(char **argv);
static void execute(int bg, char **argv);
static int child_notification(process *cp);
static process *find_process(pid_t pid);


int main()
{
    init_shell();
    while (1){
        char cmd[MAXLINE];
        printf("icsh> ");
        fgets(cmd, MAXLINE, stdin);        
        if(feof(stdin)){
            exit(0);
        }
        eval(cmd);
    }
    return 0;
}

void child_handler(int sig)
{
    pid_t pid;
    int wstatus;
    while((pid = waitpid(-1, &wstatus, WUNTRACED | WNOHANG)) > 0){
        //process *cp = find_process(pid);
        //int stp = child_notification(cp);
        //if(stp)
        //    break;
    }
    kill(shell_pgid, SIGCONT);
}

void sig_handler(int sig)
{
    switch(sig){
        case SIGCHLD:
            child_handler(sig);            
            break;
    }
}

void init_shell()
{
        struct sigaction action;
        action.sa_handler = sig_handler;
        action.sa_flags = SA_RESTART;
        sigfillset(&action.sa_mask);

        Signal(SIGCHLD, sig_handler);
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

void eval(char *cmd)
{
    char *argv[MAXSIZE];
    char buf[MAXLINE];
    int bg;

    strcpy(buf, cmd);
    bg = parse(buf, argv);
    if (argv[0] == NULL) return;

    if(!builtin_command(argv)){
        execute(bg, argv);
    }
}

int builtin_command(char **argv)
{
    if (!strcmp(argv[0], "exit")){
        exit(0);
    }
    if (!strcmp(argv[0], "&")){
        return 1;
    }
    return 0;
}

void execute(int bg, char **argv)
{
    pid_t pid;
    process cp;
    cp.next = phead;
    cp.status = -1;
    cp.argv = argv;
    phead = &cp;

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

        if(execvp(argv[0], argv) < 0){
            printf("***ERROR: exec failed\n");
            exit(1);
        }
    }
    else{
        setpgid(pid, pid);
        printf("PID=%d\n", pid);
        cp.pid = pid;
        if(!bg){
            tcsetpgrp(0, pid);
            pause();
            //waitpid(-pid, &cp.status, WUNTRACED); 
            //child_notification(&cp);
            tcsetpgrp(0, shell_pgid);
        }
    }
}

int child_notification(process *cp)
{
    if (WIFEXITED(cp->status)){
        printf("%d exited, status=%d\n", cp->pid, WEXITSTATUS(cp->status));
    }
    else if (WIFSIGNALED(cp->status)){
        printf("%d killed by signal %d\n", cp->pid, WTERMSIG(cp->status));
    }
    else if (WIFSTOPPED(cp->status)){
        printf("%d stopped by signal %d\n", cp->pid, WSTOPSIG(cp->status));
        return 0;
    }
    else if (WIFCONTINUED(cp->status)){
        printf("%d continued\n", cp->pid);
    }
    return -1;
}

static process *find_process(pid_t pid)
{
    process *p = phead;
    while(p){
        if(p->pid == pid)
            return p;
        printf("pid=%d, p->pid=%d\n", pid, p->pid);
        p = p->next; 
    }
    return NULL;
}
