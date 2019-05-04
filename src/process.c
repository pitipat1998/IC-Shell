#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>
#include <icshell.h>

void execute(int bg, char **argv){
    pid_t pid;
    int wstatus;
    
    pid = fork();
    if (pid < 0){
        perror("***ERROR: fork failed\n");
        exit(1);
    }
    else if (pid == 0){
        setpgid(0,0);
        if(!bg)
            tcsetpgrp(0, getpid());
        signal(SIGINT, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        signal(SIGTTIN, SIG_DFL);
        signal(SIGTTOU, SIG_DFL);
        signal(SIGCHLD, SIG_DFL);

        if(execvp(argv[0], argv) < 0){
            printf("***ERROR: exec failed\n");
            exit(1);
        }
    }
    else{
        setpgid(pid, pid);
        if(bg){
            waitpid(-pid, &wstatus, WNOHANG | WUNTRACED); 
        }
        if(!bg){
            tcsetpgrp(0, pid);
            waitpid(-pid, &wstatus, WUNTRACED /*| WCONTINUED*/); 
           //if (WIFEXITED(wstatus)){
           //    printf("exited, status=%d\n", WEXITSTATUS(wstatus));
           //}
           //else if (WIFSIGNALED(wstatus)){
           //    printf("killed by signal %d\n", WTERMSIG(wstatus));
           //}
           //else if (WIFSTOPPED(wstatus)){
           //    printf("stopped by signal %d\n", WSTOPSIG(wstatus));
           //}
           ///*
           //else if (WIFCONTINUED(wstatus)){
           //    printf("continued\n");
           //}
           //*/
            tcsetpgrp(shell_terminal, shell_pgid);
        }
    }
}
