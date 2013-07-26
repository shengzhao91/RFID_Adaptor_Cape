#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
 
int foo(char *adr[])
{
        pid_t pid;
 
        pid=fork();
        if (pid==0)
        {
                if (execv("/home/root/BBB_SPI/hello",adr)<0)
                        return -1;
                else
                        return 1;
        }
        else if(pid>0)
                return 2;
        else
                return 0;
}
 
int main(int argc,char *argv[])
{
        if (foo(argv)<=0)
                perror("foo");
        return 0;
}
