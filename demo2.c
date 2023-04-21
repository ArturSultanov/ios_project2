#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

int main(int argc, char* argv[]){
    int id1 = fork();
    int id2 = fork();

    if (id1 == 0)
    {
        if (id2 == 0)
        {
            printf("We are process y \n");
        }
        else{
            printf("We are process x \n");
        }
    }
    else{
        if (id2 == 0)
        {
            printf("We are process z \n");
        }
        else{
            printf("We are process perant \n");
        }   

    }
    
    while (wait (NULL) != - 1 || errno != ECHILD){
        printf("Waited for a child to finishin\n");
    }

}
