#include <stdio.h>
#include <string.h>
#include "Disk_Library.h"

#define DISK_PATH "vdisk"

void initLLFS(){
    // Clear vdisk - set to all 0s
    char zeros[2097152];
    memset(zeros, 0, 2097152);  // Set array of all zeros
    FILE * file = fopen(DISK_PATH, "w");
    fwrite(zeros, 1, sizeof(zeros), file);

    // Initialize superblock - block 0

    // Initialize free block vector - block 1
    char freeBlocks[512];       
    memset(freeBlocks, 255, 512);
    freeBlocks[0] = 0; //00000000
    freeBlocks[1] = 15; //00001111 - first 10 blocks reserved, next 2 used for root
    fseek(file, 512, SEEK_SET);
    fwrite(freeBlocks, 1, 512, file);

    // Initialize inode map - block 2
    // 1st byte | 2nd and 3rd byte
    // ---------|-------------------
    // flags    | inode block
    // flags (0/1)
    // | 0 | 0 | 0 | 0 | 0 | 0 | file/directory | in use/free |
    char inode0map[3];
    inode0map[0] = 3;       // flags - 00000011
    inode0map[1] = 0;       // 2 and 3 inode block
    inode0map[2] = 10;      // block 10 first free block
    fseek(file, 512*2, SEEK_SET);
    fwrite(inode0map, 1, 3, file);

    // Initialize root inode - block 10
    char inode0[32];
    // flag bit at 0th posiiton (0/1) (file/directory)
    inode0[7] = 1;

    // first data block is block 11
    inode0[9] = 11; 

    fseek(file, 512*10, SEEK_SET);
    fwrite(inode0, 1, 32, file);

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