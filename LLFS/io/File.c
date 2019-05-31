#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../disk/Disk_Library.h"

#define NUM_INODES 170
#define NUM_CACHE_BLOCKS 50

char cache[NUM_CACHE_BLOCKS][512];
char freeCache[NUM_CACHE_BLOCKS]; // 0 if free

// Array of pointers to linked lists
// First value is cache block where inode is stored
// Second value is cache block where first data block is stored and so on
struct Node * cachedFiles[NUM_INODES]; 
char cache_initialized = 0;

// Cache block 
int freeblockvector = 0;
int inodemap = 1;
int rootinode = 2;

// Disk
int lastUsedDiskBlock = 11;  // Starting at 0

void printCacheBlock(int block);
void printNode(struct Node * node);
short charToShort(char * c);
char isFlagSet(char byte, char index);
void cacheInit();
int inCache(int inode);
struct Node * newNode(int value);
void writeAllToDisk();
void writeFileToDisk(int inode);
int findFreeDiskBlock(int start);
void markDiskBlockUsed(int blockNum);
void getBlocks(char * inode, short * blocks);
char traversePath(char * path);
char findInDir(char * file, int dir);
char findFreeInDir(int dir);
int makeInode(char type);
int findFreeInode();
void updateInodeWithBlockNum(int inode, int dataBlockNum, unsigned short diskBlock);

void open(char * path);

struct Node{
    // char * name;
    int value;
    struct Node * next;
};

//****************** Print Helper Functions **********************

// Prints 50 bytes per line 
void printCacheBlock(int block){
    printf("Cache block %d:\n", block);
    for( int i = 0 ; i< 512 ; i++){
        if(i!=0 && i%50 == 0) printf("\n");
        printf("%x ", (unsigned char)cache[block][i]);
    }
    printf("\n");
}

// Prints 50 bytes per line 
void hexDump(char * print, int len){
    printf("Hex Dump\n");
    for( int i = 0 ; i< len ; i++){
        if(i!=0 && i%50 == 0) printf("\n");
        printf("%x ", (unsigned char)print[i]);
    }
    printf("\n");
}

void printNode(struct Node * node){
    printf("Node value: %d\n", node->value);
}
//*****************************************************************

//***************** General Small Helper Functions ****************

short charToShort(char * c){
    short s = (c[0]<<8) + c[1];
    return s;
}

// Returns 0 if bit not set, and returns not 0 if bit is set
char isFlagSet(char byte, char index){
    return byte & (1<<index);
}
//*******************************************************************

//****************** Cache Helper Functions *************************

// reads in the required data structures adn root directory from disk and initializes cacheFiles 
void cacheInit(){

    for(int i = 0; i<NUM_INODES ; i++){     // Allocate a struct Node for each element of cachedFiles array
        cachedFiles[i] = newNode(-1);
    }
    for(int i = 0 ; i<NUM_CACHE_BLOCKS; i++){   // Set all cache to 0
        memset(cache[i], 0, 512);       
    }

    rblock(cache[freeblockvector], 1);                  // Read in free block vector
    freeCache[freeblockvector] = 1;                     // Set cache block to in use
    rblock(cache[inodemap], 2);                         // Read in inode map
    freeCache[inodemap] = 1;                            // Set cache block to in use
    char rootinodeblock[2];                             // Get block number for inode 0
    rootinodeblock[0] = cache[inodemap][1];
    rootinodeblock[1] = cache[inodemap][2];
    rblock(cache[rootinode], charToShort(rootinodeblock));  // Read in root directory inode (inode 0)
    freeCache[rootinode] = 1;                               // Set cache block to in use
    // printCacheBlock(rootinode);


    cachedFiles[0] = malloc(sizeof(struct Node));  // Set as open file
    cachedFiles[0]->value = 2;
    // cachedFiles[0]->name = "root";
    printNode(cachedFiles[0]);

    short rootinodeblocks[10];
    getBlocks(cache[rootinode], rootinodeblocks);
    int i = 0;
    struct Node * node = cachedFiles[0];
    while(rootinodeblocks[i]!=0){
        rblock(cache[3+i], rootinodeblocks[i]);
        freeCache[3+i] = 1;
        // printCacheBlock(3+i);
        node->next = newNode(3+i);
        node = node->next;
        printNode(node);        
        i++;
    }
    cache_initialized = 1;
}

// Checks if a file is in the cache
// Returns the cache block that the inode is in if it's in the cache
// Returns -1 if is not in the cache
int inCache(int inode){
    return cachedFiles[inode]->value;
}

// Allocates space for a new node and returns the node
// Sets the value of the node to the given value 
// Sets the node's next pointer to NULL
struct Node * newNode(int value){
    struct Node * node = malloc(sizeof(struct Node));
    if(node == NULL) return NULL;
    node->value = value;
    node->next = NULL;
    // node->name = NULL;
    return node;
}

int pullInodeIntoCache(char inode){
    return -1;
}

// Returns the number of a free cache block if there is one
// Returns -1 if there isn't one 
// TODO: Implement freeing cache blocks 
int findFreeCacheBlock(){
    for(int i = 0 ; i < NUM_CACHE_BLOCKS ; i++){
    
        if(freeCache[i] == 0) return i;
    }
    return -1;  // Free cache blocks instead of returning -1
}


// Must pass in a pointer that points to 512 bytes
int writeCacheBlock(int blockNum, char * blockContents){
    printf("Block contents: %s\n", blockContents);
    // hexDump(blockContents, 512);
    if(blockNum<0 || blockNum>=NUM_CACHE_BLOCKS){
        printf("There is not cache block %d", blockNum);
        return -1;
    }

    for(int i = 0; i<512 ; i++){    // Copy each byte over
        cache[blockNum][i] = blockContents[i];
    }
    freeCache[blockNum] = 1;
    return 1;
}

void writeAllToDisk(){
    // Loop through inode map
    // Check for in use inodes that have been modified
    printf("Writing all modified files to disk\n");
    for(int i = 0 ; i < NUM_INODES ; i++){
        char flags = cache[inodemap][i*3];
        if(isFlagSet(flags, 0) != 0 && isFlagSet(flags, 2) != 0){   // if in use and modified
           writeFileToDisk(i); 
        }
    }
}

void writeFileToDisk(int inode){
    // Write each data block to disk 
        // Mark the old block as free in free block vector
        // Update the inode with the new block number
    printf("Writing file with inode number %d to disk\n", inode);
    int inodeCacheBlock = cachedFiles[inode]->value;
    if(inodeCacheBlock == -1){
        printf("inode %d not in cache\n", inode);
    }
    struct Node * curNode = cachedFiles[inode]->next;
    int curCacheDataBlock = curNode->value;
    int dataBlockNum = 0;
    int diskBlock = lastUsedDiskBlock;
    do{
        diskBlock = findFreeDiskBlock(diskBlock);
        markDiskBlockUsed(diskBlock);
        wblock(cache[curCacheDataBlock], diskBlock);
        updateInodeWithBlockNum(inode, dataBlockNum, diskBlock);
        curNode = curNode->next;

    } while (curNode!=NULL);
    lastUsedDiskBlock = diskBlock;



    // Mark inode as not modified in inode map and update location 
}

int findFreeDiskBlock(int start){
    for(int i = start; i<4096 ; i++){
        if(isFlagSet(cache[freeblockvector][i/8], 128>>(i%8)) == 1){ // 1000 0000 >> i%8
            return i;
        } 
    }
    printf("Error: No more free disk blocks!\n");
    return -1;
}

// Sets the given block to used in the freeblock vector
void markDiskBlockUsed(int blockNum){
    unsigned char blockByte = cache[freeblockvector][blockNum/8];
    blockByte = blockByte - (128 >> (blockNum%8));
    cache[freeblockvector][blockNum/8] = blockByte;
}

//*****************************************************************


//*************** Directory Helper Functions *********************

// Stores block number for first 10 blocks for the given inode in blocks pointer
// TODO: return all the blocks 
void getBlocks(char * inode, short * blocks){
    char toshort[2];
    for(int i = 0; i < 20 ; i+=2){
        toshort[0] = inode[8+i];
        toshort[1] = inode[9+i];
        blocks[i] = charToShort(toshort);
        printf("Block %d at location %u\n", i/2, blocks[i]);
        if(blocks[i] == 0) return;
    }
}


// Traverses directories looking for the last directory or file in the path
// Tokenize path string 
// Find file 
// Return:
// * -1 - file not found
// * inode number - file found
char traversePath(char * pathString){
    char path[200];                 // Copy string into char array
    strncpy(path, pathString, 200);
    printf("Traversing path: %s\n", path);
    
    char * token;
    const char slash[2] = "/";
    int inode = 0;      // Start with root

    token = strtok(path, slash);     // Should be 'root'
    printf("Token: %s\n", token);
    if(strncmp(token, "root", strlen(token))!=0){
        printf("Path must start with 'root', your path started with %s\n", token);
        return -10;
    }
    token = strtok(NULL, path);     // Should not be null
    printf("next token: %s\n", token);
    if(token == NULL){
        printf("Not much to traverse, only the root directory was passed in\n");
        return 0;
    }

    while(token != NULL){           // Look for token in dir with number inode
        printf("Token: %s\n", token);
        inode = findInDir(token, inode);
        token = strtok(NULL, path);

        if(token == NULL){      // If last item in path
            if(inode == -1){        // Is not found
                printf("There is no directory '%s' in path '%s'\n", token, path); // Print error
                return -1;        
            }else{                  // Is found
                return inode;
            }
        }else{                  // If not last item in path
            if(inode == -1){        // And not found
                printf("There is no directory '%s' in path '%s'\n", token, path); // Print error
                return -1;
            }else{                  // And found
                if(inCache(inode) == 0){    // Check if file in cache
                        // if not, pull dir into cache
                }                           
            }                                                
        }

        
    }
    
    return -1;
}

// Checks for the given filename in the given directory (passed in by inode number)
// Returns the inode number of the file if found
// Returns -1 if not found
// Directory is passed in as the inode number
// TODO: Make it so that all the directory blocks pointed to by that inode are checked
//       right now only one block is checked
char findInDir(char * file, int dir){
    printf("Finding file %s in directory with inode number %d\n", file, dir);
    int block = cachedFiles[dir]->next->value;
    char name[31];
    // printCacheBlock(block);
    printf("block: %d\n", block);
    for(int i = 0; i < 16 ; i++){       // Loop through dir entries in block
        if(cache[block][1+i*32] == 0){  // If there are no more file names
            return -1;
        }
        strncpy(name, cache[block]+1+i*32, 31); // Copy filename into name buffer
        // printf("name: %s\n", name);
        if(strncmp(name, file, 31) == 0){       // If filename matched given filename
            return cache[block][0+i*32];        // Return inode number 
            // printf("strings equal\n");
        } 
    }
    return -1;
}

// Takes in inode number of a dir
char addToDir(int dir, char * name, int inodeNum){
    struct Node * cur = cachedFiles[dir]->next;
    while(cur->next!=NULL){
        cur = cur->next;
    }
    int block = cur->value;
    for(char i = 0; i < 16 ; i++){   // Loop through last dir looking for free space
        if(cache[block][i*32] == 0){
            cache[block][i*32] = inodeNum;
            strncpy(cache[block]+i*32+1, name, 31);
            // printCacheBlock(block);
            return 1;
        }
    }
    // TODO - Implement adding another block to the directory 
    return -1;
}
//***********************************************************************************

// Puts and inode into cache
// Updates the inode map in cache
// Parameters:
// Type - (file/directory) (0/1)
// Returns the inode number if successful or -1 if not successful
int makeInode(char type){
    printf("Making inode\n");
    int inodeNum = findFreeInode();
    if(inodeNum == -1){
        printf("Error: max number of files reached\n");
        return -1;
    }

    char inode[512];
    memset(inode, 0, 512);
    inode[7] = (type<<1) + 1;   // Flag type and in use
    int freeCacheBlock = findFreeCacheBlock();
    printf("Free cache block found: %d\nFor inode #%d\n", freeCacheBlock, inodeNum);
    writeCacheBlock(freeCacheBlock, inode);
    // Mark not free in inode map // something wrong here
    // cache[inodemap][(freeCacheBlock-1)/8] = 128>>(freeCacheBlock%8);  
    cache[inodemap][inodeNum*3] = 4 + (type<<1) + 1; 

    // Add to page table 
    cachedFiles[inodeNum]->value = freeCacheBlock;
    return inodeNum;
}

int findFreeInode(){
    // printCacheBlock(inodemap);
    for(int i = 0 ; i < NUM_INODES ; i++){
        char flags = cache[inodemap][i*3];
        if(isFlagSet(flags, 0) == 0){
            printf("inode %d being used\n", i );
            return i;
 
        }
    }
    printf("Free inode not found");
    return -1;
}

void updateInodeWithBlockNum(int inode, int dataBlockNum, unsigned short diskBlock){
    int cacheBlock = cachedFiles[inode]->value;
    if(dataBlockNum<10){ // If it is a direct data block (0 through 9)
        cache[cacheBlock][8 + dataBlockNum*2] = (65280 & diskBlock) >> 8; // ((1111 1111  0000 0000) & diskblock) >> 8
        cache[cacheBlock][9 + dataBlockNum*2] = 255 & diskBlock; // 1111 1111 & diskBlock
    }else{
        // TODO: implement single and double indirect block updating
    }
}

//********************** User Facing Functions **************************************

// Opens a file or creates a file if it doesn't exist
void open(char * path){
    if(cache_initialized == 0){
        cacheInit();
    }
    // Look for file
    // If file exists
        // Pull it into cache
    // Else
        printf("The file %s does not exist, please use the command mkfile to create the file", path);

}


int mkfile(char * path, char * name, char * content){
    if(cache_initialized == 0){
        cacheInit();
    }   
    printf("Making File: '%s', with path: '%s'\n", name, path);
    
    // Return the inode number for the last directory in the path
    int dir = traversePath(path);
    if(dir == -1) return -1;
    // Check for file to make sure no duplicate
    int inodeNum = findInDir(name, dir);
    if(inodeNum != -1){
        printf("The file '%s' already exists in the directory '%s'.\n", name, path);
        return -1; 
    }

    cache[inodemap][dir*3] = (cache[inodemap][dir*3] | 4); // Mark modified

    // Make a new inode 
    inodeNum = makeInode(0);
    // printCacheBlock(cachedFiles[inodeNum]->value);

    // Add inode to last dir in path
    addToDir(dir, name, inodeNum);

    // Get head of list of cache blocks for new file
    struct Node * curNode = cachedFiles[inodeNum];
    // Loop through content 512 bytes at a time 
    for(int i = 0 ; i < strlen(content)/512 + 1 ; i++){
        // Write in cache and add to cachedFile[inodeNum] list
        int blockNum = findFreeCacheBlock();
        if(i == strlen(content)/512){
            char block[512];
            memset(block, 0, 512);
            strcpy(block, content + i*512);
            writeCacheBlock(blockNum, block);
            printCacheBlock(blockNum);
        }else{
            writeCacheBlock(blockNum, content + i*512);
            printCacheBlock(blockNum);
        }
        curNode->next = newNode(blockNum);
        curNode = curNode->next;
    }
    return 1;


}

void unlink(){
    if(cache_initialized == 0){
        cacheInit();
    }
    
}

void mkdir(){
    if(cache_initialized == 0){
        cacheInit();
    }

}

void rmdir(){
    if(cache_initialized == 0){
        cacheInit();
    }

}
//*************************************************************************************


int main(){

    initLLFS();

    cacheInit();

    mkfile("/root/", "Hello", "snfsdkfnlsdkfnskdlnflsdkfnlknsdfkndsfknsdflknsdklf");
    char block[514];
    memset(block, 57, 512);
    block[512] = 55;
    block[513] = 0;
    mkfile("/root/", "Hello1", block);
    writeAllToDisk();
}