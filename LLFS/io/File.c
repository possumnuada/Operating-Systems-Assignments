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

void printCacheBlock(int block);
void printNode(struct Node * node);
short charToShort(char * c);
char isFlagSet(char byte, char index);
void cacheInit();
int inCache(int inode);
struct Node * newNode(int value);
void getBlocks(char * inode, short * blocks);
char findFile(char * path);
char findInDir(char * file, int dir);
int makeInode();
int findFreeInode();

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

void printNode(struct Node * node){
    printf("Node value: %d\n", node->value);
}
//*****************************************************************

//***************** General Small Helper Functions ****************

short charToShort(char * c){
    short s = (c[0]<<8) + c[1];
    return s;
}

// Returns 0 is byte not set not 0 is byte is set
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

    rblock(cache[freeblockvector], 1);                  // Read in free block vector
    freeCache[freeblockvector] = 1;                   // Set cache block to in use
    rblock(cache[inodemap], 2);                         // Read in inode map
    freeCache[inodemap] = 1;                          // Set cache block to in use
    char rootinodeblock[2];                             // Get block number for inode 0
    rootinodeblock[0] = cache[inodemap][1];
    rootinodeblock[1] = cache[inodemap][2];
    rblock(cache[rootinode], charToShort(rootinodeblock)); // Read in root directory inode (inode 0)
    freeCache[rootinode] = 1;                            // Set cache block to in use
    printCacheBlock(rootinode);


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
        printCacheBlock(3+i);
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

// not used yet
// int getNodeVal(struct Node * node){
//     return node->value;
// }

int pullInodeIntoCache(char inode){

}

// Returns the number of a free cache block if there is one
// Returns -1 if there isn't one 
// TODO: Implement freeing cache blocks 
int findFreeCacheBlock(){
    for(int i = 0 ; i < NUM_CACHE_BLOCKS ; i++){
        if(freeCache[i] == 0) return i;
    }
    return -1;  // Free cache blocks instead 
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
// Return
// -1 - file not found
// inode number - file found
char findFile(char * path){
    char * token;
    const char slash[2] = "/";
    int dir = 0;
    char inode;

    token = strtok(path, slash);     // Should be 'root'
    if(strncmp(token, "root", 5)!=0){
        printf("Path must start with 'root', your path started with %s\n", token);
        return -10;
    }

    while(token != NULL){
        printf("Token: %s\n", token);
        inode = findInDir(token, dir);
        token = strtok(NULL, path);

        if(token == NULL){      // If last item in path
            if(inode == -1){        // Is not found
                return -1;        
            }else{                  // Is found
                return inode;
            }
        }else{                  // If not last item in path
            if(inode == -1){        // And not found
                printf("There is no directory %s in path %s", token, path); // Print error
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
// Directory is passed in as the open file index in the cachedFiles array 
// TODO: Make it so that all the directory blocks pointed to by that inode are checked
//       right now only one block is checked
char findInDir(char * file, int dir){
    int block = cachedFiles[dir]->next->value;
    char name[31];
    printCacheBlock(block);
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
//***********************************************************************************

// Puts and inode into cache
// Updates the inode map in cache
// Parameters:
// Type - (file/directory) (0/1)
// Returns the inode number if successful or -1 if not successful
int makeInode(char type){
    int inodeNum = findFreeInode();
    if(inodeNum == -1){
        printf("Error: max number of files reached\n");
        return -1;
    }

    char inode[32];
    memset(inode, 0, 32);
    inode[1] = (type<<1);
    int freeCacheBlock = findFreeCacheBlock();

    // Put into cache
    // 
}

int findFreeInode(){
    for(int i = 10 ; i < NUM_INODES ; i++){
        char flags = cache[inodemap][i*3];
        if(isFlagSet(flags, 0) != 0){
            return i;
        }
    }
    return -1;
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

void mkfile(char * name, char * content){
    int inode_num = makeInode(0);


}

void unlink(){
    
}

void mkdir(){

}

void rmdir(){

}
//*************************************************************************************


int main(){

    initLLFS();
    char block[512];
    memset(block, 0, 512);
    strcpy(block+1, "a");
    wblock(block, 11);
    cacheInit();

    char path[225] = "a";
    findFile(path);
    // char block[512];
    // memset(block, 0, 512);
    // strcpy(block, "abcdefg");
    // block[400] = 64;
    // int bytes = wblock(block, 3);
    // printf("The number of bytes written was: %d\n", bytes);
    // char block1[512];
    // void * p = rblock(block1, 3);
    // for (int i = 0; i < 600; i++) {
    //     printf("%02x ", ((unsigned char *) p) [i]);
    //     if(i%100 == 0) printf("\n");
    // }
    // printf("\n");

}