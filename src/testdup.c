#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

int main(){
    int fd[2];
    char *args[] = {"ls", "-la", NULL};
    int in_fd = 0, out_fd = 1, wstatus;
    pid_t pid;

    out_fd = open("out", O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
    dup2(0, 0);
    dup2(out_fd, 1);

    close(out_fd);
    pid = fork();
    if(pid < 0)
        perror("***ERROR: fork failed\n");
    else if (pid == 0){
        if(execvp("ls", args)< 0){
            perror("***ERROR: exec failed\n");
        }
    }
    else{
        waitpid(pid, &wstatus, 0);
    }
    
    
}
