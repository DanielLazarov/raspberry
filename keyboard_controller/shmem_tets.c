#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <sys/mman.h>

int main()
{
    pid_t childPID;
    
    //Shared variable
    int *number = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
               MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    childPID = fork();

    if (childPID < 0)
    {
        write(1, "Error creating child\n", 21);
        exit(1);// Error
    }
    else if (childPID == 0)
    {
        //CHILD
        sleep(5);
        write(1, "CHILD\n", 6);
    }
    else
    {
        sleep(1);
        //PARENT
        write(1, "PARENT\n", 7);
    }
   
    //Free shared variable 
    munmap(number, sizeof(int));
}
