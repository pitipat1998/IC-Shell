#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

void execute(int bg, char **argv){
    //printf("bg=%d\n", bg);
    //for(int i=0; argv[i] != NULL; i++){
    //    printf("%s\n", argv[i]);
    //}
    pid_t pid;
    int wstatus;
    
    pid = fork();
    if (pid < 0){
        printf("*** ERROR: forking child process failed\n");
        exit(1);
    }
    else if (pid == 0){
        if(execvp(argv[0], argv) < 0){
            printf("*** ERROR: exec failed\n");
            exit(1);
        }
    }
    else{
        if(bg){
            waitpid(pid, &wstatus, WNOHANG | WUNTRACED); 
        }
        else{
            waitpid(pid, &wstatus, 0);
        }
    }
}
