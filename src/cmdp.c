#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <process.h>

static char *trim(char *str){
    char *end;

    while (isspace((unsigned char) *str)) 
        str++;

    if(!*str) return str;

    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char) *end)) 
        end--;

    end[1] = '\0';

    return str;
}

static int parse(char *cmd, char **argv){
    cmd[strcspn(cmd, "\n")] = 0;
    cmd = trim(cmd);

    int i = 0;
    char *pattern = " \t\n";
    char *p = strtok(cmd, pattern);
    while (p != NULL) {
        argv[i++] = p;
        p = strtok(NULL, pattern);
    }

    for(int j=0; j<i; j++){
        argv[j] = trim(argv[j]);
    }

    argv[i] = NULL;
    return i;
}

void eval(char *cmd, char **argv){
    int bg = 0;
    int asize = parse(cmd, argv);
    if (!strcmp(argv[0], "exit") || feof(stdin))
        exit(0);
    if (!strcmp(argv[asize-1], "&")){
        bg = 1;
        argv[asize-1] = NULL;
    }
    execute(bg, argv);
}