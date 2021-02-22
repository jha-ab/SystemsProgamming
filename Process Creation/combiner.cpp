#include <iostream>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

using namespace std;

int main(int argc, char** argv)
{
    int pipe_fd[2];
    
    int pid1,pid2;
    if(pipe(pipe_fd)==-1)
    {
        exit(-1);
    }
    pid1=fork();
    if(pid1==-1)
    {
        exit(1);
    }
    switch(pid1)
    {
        case 0:
            close(pipe_fd[0]);
            dup2(pipe_fd[1],1);
            close(pipe_fd[1]);
            execlp("./mapper.o","mapper.o",argv[1],NULL);
            //sleep(2000);
            //wait(NULL);
            exit(-1);
            break;
        default:
            pid2=fork();
            if(pid2==-1)
            {
                 exit(1);
            }
            if(pid2==0)
            {
                close(pipe_fd[1]);
                dup2(pipe_fd[0],0);
                close(pipe_fd[0]);
                sleep(3);
                execlp("./reducer.o","reducer.o","Mapper_Output.txt",NULL);                
                wait(NULL);
                exit(-1);
            }
            else
            {
                
                close(pipe_fd[0]);
                close(pipe_fd[1]);
                wait(NULL);
                wait(NULL);
            }
            break;
    }
    return 0;
}