#include <sys/types.h>
#include <unistd.h>

//struct _job;

//struct _job
//{
//    int id;
//    char *cmd;
//    struct _process *root;
//    int pcount;
//    pid_t pgid;
//    int exec_mode;
//    struct _job *prev;
//    struct _job *next;
//};

typedef struct process
{
    int id;
    char *cmd;
    int argc;
    char **argv;
    int exec_mode;
    pid_t pid;
    int status;
    struct process *prev;
    struct process *next;
} process;

//typedef _job job;

