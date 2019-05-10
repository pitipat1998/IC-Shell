#include <stdlib.h>
#include <stdio.h>
#include <process.h>

//#define PROC_RUNNING    0
//#define PROC_EXITED     1
//#define PROC_TERMINATED 2
//#define PROC_STOPPED    3
//#define PROC_CONTINUED  4

process *process_head = NULL;
process *process_tail = NULL;
int latest_fg_status = -1;
char *process_status[5] = { "running", "exited", "terminated", "suspended", "continued" };

void print_process_status(process *p);

void free_process(process *p)
{
    free(p->cmd);
    free(p);
}

process *create_process(char *cmd, int argc, char **argv, int exec_mode)
{
    process *p = (process *) malloc(sizeof(process));
    p->id = -1;
    p->cmd = cmd;
    p->argc = argc;
    p->argv = argv;
    p->exec_mode = exec_mode;
    p->pid = PROC_RUNNING;
    p->status = 0;
    p->prev = NULL;
    p->next = NULL;
    return p;
}

void add_process(process *p)
{
    int i = 1;

    if(!process_head)
    {
        p->id = i;
        process_head = p;
        process_tail = p;
        return;
    }

    p->id = process_tail->id+1;
    p->prev = process_tail;
    process_tail->next = p;
    process_tail = p;
}

int launch_process(process *p)
{
    return 0;
}

static void remove_process(process *p)
{
    if(!p->prev)
    {
        if(!p->next)
        {
            process_head = NULL;
            process_tail = NULL;
        }
        else
        {
            process_head = p->next;
            p->next->prev = p->prev;
        }
    }
    else
    {
        if(!p->next)
        {
            process_tail = p->prev;
            p->prev->next = p->next;
        }
        else
        {
            p->next->prev = p->prev;         
            p->prev->next = p->next;
        }
    }
    free_process(p);
}

int remove_process_by_id(int id)
{
    process *p;
    
    for(p=process_head; p; p=p->next)
    {
        if(id == p->id)
        {
            remove_process(p);
            return 0;
        }
    }
    return -1;
}

int remove_process_by_pid(pid_t pid)
{
    process *p;
    
    for(p=process_head; p; p=p->next)
    {

        if(pid == p->pid)
        {
            remove_process(p);
            return 0;
        }
    }
    return -1;
}

void fg_exit(int wstatus)
{
    latest_fg_status = wstatus; 
}


int process_is_done_by_id(int id)
{
    process *p;
    
    for(p=process_head; p; p=p->next)
    {
        if(id == p->id)
        {
            if(p->status == PROC_EXITED ||
               p->status == PROC_TERMINATED) 
                return 1;
        }
    }
    return -1;

}

int process_is_done_by_pid(pid_t pid)
{
    process *p;
    
    for(p=process_head; p; p=p->next)
    {
        if(pid == p->pid)
        {
            if(p->status == PROC_EXITED ||
               p->status == PROC_TERMINATED) 
                return 1;
        }
    }
    return -1;

}

process *find_process_by_id(int id)
{
    process *p;
    
    for(p=process_head; p; p=p->next)
    {
        if(id == p->id)
            return p;             
    }
    return NULL;

}

process *find_process_by_pid(pid_t pid)
{
    process *p;
    
    for(p=process_head; p; p=p->next)
    {
        if(pid == p->pid)
            return p;             
    }
    return NULL;

}

void set_process_status(process *p, int status){
    p->status = status;
}

void set_process_status_by_id(int id, int status){
    set_process_status(find_process_by_id(id), status);
}

void set_process_status_by_pid(pid_t pid, int status){
    set_process_status(find_process_by_pid(pid), status);
}

void print_all_processes()
{
    for(process *p = process_head; p; p = p->next)
    {
        print_process_status(p);
    }
}

void print_process_status(process *p)
{
    if(!p) return;
    printf("[%d]\t %d \t %s \t %s\n", p->id, p->pid, process_status[p->status], p->cmd);
}

void print_process_status_by_id(int id){
    process *p = find_process_by_id(id);
    print_process_status(p);
}

void print_process_status_by_pid(pid_t pid){
    process *p = find_process_by_pid(pid);
    print_process_status(p);
}

