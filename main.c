#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[]){
    int id = fork();
    printf("Hello World, parent\n");
    if (id == 0){
        printf("Hello World, child\n");
    }
    return 0;
}