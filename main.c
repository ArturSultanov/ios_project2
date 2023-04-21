#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

int main(int argc, char* argv[]){
    int id = fork();
    int n;
    if (id == 0){
        n = 1;
    }
    else{
        n = 6;
    }

    printf ("Current ID: %d, parent ID: %d\n",
        getpid(), getppid());

    int res = wait(NULL);
    if (id != 0){
        if ( res == -1 ){
            printf("No children to wait for");
        }
        else{
            printf("%d child finished exicution\n", res);
        }
    }
    int i;
    for (i = n; i < n + 5; i++)
    {
        printf ("%d ", i); 
        fflush (stdout);
    }
    printf("\n");
    return 0;
}
