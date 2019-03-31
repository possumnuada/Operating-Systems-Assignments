#include <stdio.h>
#include <string.h>
#include "Disk_Library.h"

#define DISK_PATH "vdisk"

void initLLFS(){
    char zeros[2097152];
    memset(zeros, 0, 2097152);  // Set array of all zeros
    zeros[513] = 63;
    char freeBlocks[512];       // Make block of all 1s except for first 10 bits
    memset(freeBlocks, 255, 512);
    freeBlocks[0] = 0; //00000000
    freeBlocks[1] = 63; //00111111
    FILE * file = fopen(DISK_PATH, "w");
    //size_t bytes = 
    fwrite(zeros, 1, sizeof(zeros), file);
    // printf("The number of bytes written was: %ld\n", bytes);
    fseek(file, 512, SEEK_SET);
    fwrite(freeBlocks, 1, 512, file);
    fclose(file);
}

int wblock(char* block, int blockNum){
    if(blockNum > 4095 || blockNum < 0){
        printf("Bock size out of range.");
        return -1;
    }
    // printf("Size of block: %ld \n",sizeof(block));
    // if(sizeof(block)!=512){
    //     printf("Error: Tried to write a block that is not 512 bytes.\nBlock: %s\n", block);
    //     return -1;
    // }
    FILE * file = fopen(DISK_PATH, "r+");
    fseek(file, blockNum*512, SEEK_SET);
    size_t bytes = fwrite(block, 1, 512, file);
    fclose(file);
    return bytes;
}

void * rblock(char* block, int blockNum){
    if(blockNum > 4095 || blockNum < 0){
        printf("Bock size out of range.");
        return NULL;
    }
    // printf("Size of block: %ld \n",sizeof(block));
    // if(sizeof(block)!=512){
    //     printf("Error: Tried to read into a block that is not 512 bytes.\n");
    //     return -1;
    // }
    FILE * file = fopen(DISK_PATH, "r+");
    fseek(file, blockNum*512, SEEK_SET);
    fread(block, 1, 512, file);
    fclose(file);
    // printf("%s\n", block);
    void *p = block; 
    return p;
}



// int main(){
//     initLLFS();
//     char block[512];
//     memset(block, 0, 512);
//     strcpy(block, "abcdefg");
//     wblock(block, 3);
//     strcpy(block, "*************");
//     wblock(block, 13);
//     strcpy(block, "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^");
//     wblock(block, 40);
//     char block1[512];
//     void * p = rblock(block1, 3);
//     for (int i = 0; i < 10; i++) {
//         printf("%02x\n", ((unsigned char *) p) [i]);
//     }
// }