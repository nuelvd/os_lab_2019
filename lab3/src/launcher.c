#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
    if (argc<2) {
        fprintf(stderr, "Using %s [args...]\n", argv[0]);
        return 1;
    }
    pid_t pid = fork();
    
    if (pid<0) {
        perror("fork");
        return 1;
    }

    if (pid==0) {
        execvp(argv[1], &argv[1]);
    }
    int status;
    if (waitpid(pid,&status, 0)==-1){
        perror("waitpid");
        return 1;
    }
    if (WIFEXITED(status)) {
        printf("Дочерний процесс завершился\n");
    }
    return 0;
}
