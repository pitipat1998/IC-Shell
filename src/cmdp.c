#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

static char *trim(char *str)
{
    char *end;

    while (isspace((unsigned char) *str)) str++;

    if(!*str) 
        return str;
    
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char) *end)) end--;

    end[1] = '\0';
    return str;
}

int parse(char *cmd, char **argv)
{
    int argc;
    int bg;

    cmd[strcspn(cmd, "\n")] = 0;
    cmd = trim(cmd);

    argc = 0;
    char *pattern = " \t\n";
    char *p = strtok(cmd, pattern);
    while (p != NULL) {
        argv[argc++] = p;
        p = strtok(NULL, pattern);
    }
    for(int j=0; j<argc; j++){
        argv[j] = trim(argv[j]);
    }
    argv[argc] = NULL;

    if (argc == 0) return 1;

    if((bg = (*argv[argc-1] == '&')) != 0)
        argv[--argc] = NULL;

    return bg;
}
