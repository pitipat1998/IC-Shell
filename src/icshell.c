#include <stdio.h>
#include <cmdp.h>

/****************************************************
 ********************* MACROS ***********************
 ****************************************************/
#define MAXLINE 1024
#define MAXSIZE 64

int main(){
    char cmd[MAXLINE];
    char *argv[MAXSIZE];
    while (1){
        printf("icsh> ");
        fgets(cmd, MAXLINE, stdin);        
        eval(cmd, argv);
    }
    return 0;
}
