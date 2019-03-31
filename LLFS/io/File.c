#include <stdio.h>
#include <string.h>
#include "../disk/Disk_Library.h"



int main(){

    printf("Hello\n");
    initLLFS();
    char block[512];
    memset(block, 0, 512);
    strcpy(block, "abcdefg");
    int bytes = wblock(block, 3);
    printf("The number of bytes written was: %d\n", bytes);
    char block1[512];
    void * p = rblock(block1, 3);
    for (int i = 0; i < 600; i++) {
        printf("%02x ", ((unsigned char *) p) [i]);
        if(i%100 == 0) printf("\n");
    }
    printf("\n");
}