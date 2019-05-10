#define _POSIX_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <stddef.h>

#include <cmdp.h>
#include <psignal.h>
#include <process.h>

/****************************************************
 ********************* MACROS ***********************
 ****************************************************/
#define MAXLINE 1024
#define MAXSIZE 64

//Global variable
pid_t shell_pgid;
extern process *process_head;
extern process *process_tail;
extern int latest_fg_status;

//Shell related
static void init_shell();

//Signal Handlers
void child_handler(int sig);
void sig_handler(int sig);

//Processing Commands
static void eval(char *cmd);
static int builtin_command(int bg, int argc, char **argv);
static void execute_external_command(int bg, char *cmd, int argc, char **argv);

//Process Manipulation
void put_process_in_foreground(process *p, int cont);
void put_process_in_background(process *p, int cont);
void wait_for_process(process *p);
static void child_notification(process *p, int wstatus);

//Builtin functions
void exit_shell();
void echo_latest_process();
void jobs();
void foreground_process(char *arg);
void background_process(char *arg);

int main()
{
    init_shell();
    while (1){
        char cmd[MAXLINE];
        printf("\033[1;37;41m" "icsh>" "\033[m" " ");
        fgets(cmd, MAXLINE, stdin);        
        if(feof(stdin)){
            exit_shell();
        }
        eval(cmd);
    }
    return 0;
}

void child_handler(int sig)
{
    pid_t pid;
    int wstatus;
    while((pid = waitpid(-1, &wstatus, WNOHANG|WUNTRACED)) > 0){
        process *cp = find_process_by_pid(pid);
        child_notification(cp, wstatus);
    }
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
        Signal(SIGINT, SIG_IGN);
        Signal(SIGTSTP, SIG_IGN);
        Signal(SIGQUIT, SIG_IGN);
        Signal(SIGTTIN, SIG_IGN);
        Signal(SIGTTOU, SIG_IGN);

        shell_pgid = getpid();
        if(setpgid(shell_pgid, shell_pgid) < 0){
            perror("setpgid failed");
            exit(1);
        }

        tcsetpgrp(0, shell_pgid);
}

void eval(char *cmd)
{
    int argc;
    char *argv[MAXSIZE];
    char buf[MAXLINE];
    int bg;

    strcpy(buf, cmd);
    bg = parse(buf, argv, &argc);
    if (argv[0] == NULL) {
        return;
    }

    if(!builtin_command(bg, argc, argv)){
        execute_external_command(bg, cmd, argc, argv);
    }
}

int builtin_command(int bg, int argc,  char **argv)
{
    if (!strcmp(argv[0], "exit")){
        exit_shell();
        return 1;
    }
    if (!strcmp(argv[0], "&")){
        return 1;
    }
    if (!strcmp(argv[0], "jobs")){
        jobs();
        return 1;
    }
    if (argc >= 2 && !strcmp(argv[0], "echo") && !strcmp(argv[1], "$?")){
        echo_latest_process();
        return 1;
    }
    if (argc >= 2 && !strcmp(argv[0], "fg") && *argv[1] == '%' && strlen(argv[1]) > 1){
        foreground_process(argv[1]);
        return 1;
    }
    if (argc >= 2 && !strcmp(argv[0], "bg") && *argv[1] == '%' && strlen(argv[1]) > 1){
        background_process(argv[1]);
        return 1;
    }
    return 0;
}

void wait_for_process(process *p)
{
    int wstatus;
            
    waitpid(-p->pid, &wstatus, WUNTRACED); 

    if (WIFEXITED(wstatus)){
        set_process_status(p, PROC_EXITED);
        if(p->exec_mode == PROC_FOREGROUND)
            fg_exit(WEXITSTATUS(wstatus));
        remove_process_by_pid(p->pid);
    }
    else if (WIFSIGNALED(wstatus)){
        set_process_status(p, PROC_TERMINATED);
        if(p->exec_mode == PROC_FOREGROUND)
            fg_exit(WTERMSIG(wstatus));
        remove_process_by_pid(p->pid);
    }
    else if (WIFSTOPPED(wstatus)){
        tcsetpgrp(0, p->pid);
        set_process_status(p, PROC_STOPPED);
        print_process_status(p);
    }
}

void put_process_in_foreground(process *p, int cont)
{

    if (cont)
    {
        if (kill (-p->pid, SIGCONT) < 0)
            perror ("***ERROR: kill (SIGCONT)\n");
        p->status = PROC_CONTINUED;
    }

    p->exec_mode = PROC_FOREGROUND;
    p->status = PROC_RUNNING;

    tcsetpgrp(0, p->pid);
    wait_for_process(p);
    tcsetpgrp(0, shell_pgid);

}

void put_process_in_background(process *p, int cont)
{
    if (cont)
    {
        if (kill (-p->pid, SIGCONT) < 0)
            perror ("***ERROR: kill (SIGCONT)\n");
        p->status = PROC_CONTINUED;
    }
    p->exec_mode = PROC_BACKGROUND;
    print_process_status(p);
    p->status = PROC_RUNNING;
}

void execute_external_command(int bg, char *buf, int argc, char **argv)
{
    char *cmd = (char *) malloc(sizeof(char) * MAXLINE);
    strcpy(cmd, buf);
    pid_t pid;
    sigset_t mask;
    process *p = create_process(cmd, argc, argv, bg);
    
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &mask, NULL);

    pid = fork();
    if (pid < 0){
        free(cmd);
        perror("***ERROR: fork failed\n");
        exit(1);
    }
    else if (pid == 0){
        sigprocmask(SIG_UNBLOCK, &mask, NULL);
        Signal(SIGINT, SIG_DFL);
        Signal(SIGQUIT, SIG_DFL);
        Signal(SIGTSTP, SIG_DFL);
        Signal(SIGTTIN, SIG_DFL);
        Signal(SIGTTOU, SIG_DFL);
        Signal(SIGCHLD, SIG_DFL);

        if(execvp(argv[0], argv) < 0){
            free(cmd);
            printf("***ERROR: exec failed\n");
            exit(1);
        }
    }
    else{
        setpgid(pid, pid);
        p->pid = pid;
        add_process(p);
        sigprocmask(SIG_UNBLOCK, &mask, NULL);
        if(!bg){
            put_process_in_foreground(p, 0);    
        }
        else{
            put_process_in_background(p, 0);    
        }
    }

}

static void child_notification(process *p, int wstatus)
{
    if (WIFEXITED(wstatus)){
        set_process_status(p, PROC_EXITED);
        if(p->status == PROC_FOREGROUND)
            fg_exit(WEXITSTATUS(wstatus));        
        print_process_status(p);
        remove_process_by_pid(p->pid);
    }
    else if (WIFSIGNALED(wstatus)){
        set_process_status(p, PROC_TERMINATED);
        if(p->status == PROC_FOREGROUND)
            fg_exit(WTERMSIG(wstatus));        
        print_process_status(p);
        remove_process_by_pid(p->pid);
    }
    else if (WIFSTOPPED(wstatus)){
        tcsetpgrp(0, shell_pgid);
        set_process_status(p, PROC_STOPPED);
        print_process_status(p);
    }
}

void exit_shell()
{
    printf("\n");
    for(process *p = process_head; p; p=p->next){
        kill(p->pid, SIGKILL);
    }
    exit(0);
}

void echo_latest_process()
{
    if(latest_fg_status == -1) return;
    printf("%d\n", latest_fg_status);
}

void jobs()
{
    print_all_processes();
}

static int get_argument_id(char *arg)
{
    char *c = arg;
    return atoi(++c);
}

void foreground_process(char *arg)
{
    int id = get_argument_id(arg); 
    process *p = find_process_by_id(id);
    if(!p){
        perror("***Error: process not found\n");
    }
    put_process_in_foreground(p, 1);
}

void background_process(char *arg)
{
    int id = get_argument_id(arg); 
    process *p = find_process_by_id(id);
    if(!p){
        perror("***Error:  process not found\n");
    }
    put_process_in_background(p, 1);
}
