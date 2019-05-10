#include <ds.h>

#define PROC_FOREGROUND 1
#define PROC_BACKGROUND 0

#define PROC_RUNNING    0
#define PROC_EXITED     1 
#define PROC_TERMINATED 2 
#define PROC_STOPPED    3 
#define PROC_CONTINUED  4 

process *create_process(char *cmd, int argc, char **argv, int exec_mode, int in_fd, int out_fd);
void add_process(process *p);
int launch_process(process *p);
void fg_exit(int wstatus);

int remove_process_by_id(int id);
int remove_process_by_pid(pid_t pid);
void free_process(process *p);

int process_is_done_by_id(int id);
int process_is_done_by_pid(pid_t id);

process *find_process_by_id(int id);
process *find_process_by_pid(pid_t pid);

void set_process_status(process *p, int status);
void set_process_status_by_id(int id, int status);
void set_process_status_by_pid(pid_t pid, int status);

void print_all_processes();
void print_process_status(process *p);
void print_process_status_by_id(int id);
void print_process_status_by_pid(pid_t pid);

