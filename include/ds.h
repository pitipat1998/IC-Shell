#include <sys/types.h>
#include <unistd.h>

typedef struct process
{
    int id;
    char *cmd;
    int argc;
    char **argv;
    int exec_mode;
    pid_t pid;
    int status;
    int in_fd;
    int out_fd;
    struct process *prev;
    struct process *next;
} process;


