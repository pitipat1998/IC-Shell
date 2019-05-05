#include <signal.h>
#include <stdio.h>
void *Signal(int signum, void *handler)
{
    struct sigaction action, old_action;

    action.sa_handler = handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_RESTART;

    if(sigaction(signum, &action, &old_action) < 0)
        perror("***Error: sigaction error\n");
    return (old_action.sa_handler);
}
