# Little Log File System 
## By Possum Nuada


## Resources 

* Log-Structured File Systems - https://youtu.be/KTCkW_6zz2k
* How To Create Files Of A Certain Size In Linux - https://www.ostechnix.com/create-files-certain-size-linux/ 
* C library function - fopen() - https://www.tutorialspoint.com/c_standard_library/c_function_fopen.htm
* C library function - fwrite() - https://www.tutorialspoint.com/c_standard_library/c_function_fwrite.htm
* C library function - fseek() - https://www.tutorialspoint.com/c_standard_library/c_function_fseek.htm 
* Write byte 0 in a binary file in C - https://stackoverflow.com/questions/37556516/write-byte-0-in-a-binary-file-in-c
* C library function - memset() - https://www.tutorialspoint.com/c_standard_library/c_function_memset.htm 
* C library function - fread() - https://www.tutorialspoint.com/c_standard_library/c_function_fread.htm 
* print bits of a void pointer - https://stackoverflow.com/questions/14656375/print-bits-of-a-void-pointer
* Compiling, linking, Makefile, header files - https://www.gribblelab.org/CBootCamp/12_Compiling_linking_Makefile_header_files.html 
* USING AND LINKING LIBRARY CODE - https://www.cs.swarthmore.edu/~newhall/unixhelp/howto_C_libraries.html 
* C library function - strcpy() - https://www.tutorialspoint.com/c_standard_library/c_function_strcpy.htm
* C library function - strcat() - https://www.tutorialspoint.com/c_standard_library/c_function_strcat.htm 
* Convert 2 bytes into an integer - https://stackoverflow.com/questions/17071458/convert-2-bytes-into-an-integer 
* C library function - malloc() - https://www.tutorialspoint.com/c_standard_library/c_function_malloc.htm

## Inodes
Each inode is 32 bytes long and is stored in a block by itself. I decided have an inode map and to store it in one block and to make each inode map entry consists of 3 bytes, see below for more details. Since a block is 512 bytes and 512/3 is 170.6, there can be at most 170 inodes.
**First 4 bytes** - Size of file in bytes
**Next 4 bytes** - flags (0/1)
..| 0 | 0 | 0 | 0 | 0 | 0 | file/directory | in use/free |
**Next 2 bytes, multiplied by 10** - block number for first 10 blocks
**Next 2 bytes** - single-indirect block number
**Last 2 bytes** - double-indirect block number


### Inode Map
Each entry in the inode map contains 3 bytes.
* **First byte** - flags (0/1) - info about the inode 
..| 0 | 0 | 0 | 0 | 0 | 0 | file/directory | in use/free |
* **Second and thrid bytes** - inode location block number 
The inode map entry for inode x is stored at byte x*3 in the inode map.

**Root Directory**
The root directory is always inode 0.




## Open Files
When a file is opened it is assigned an integer to reference the file. The integer is the index in the files array of struct fileNodes. The value of the node in the files array is the cache block number where the inode for that file is stored. The next node value is the first cache block that stores the first data block for that file. The value of the next node after that is the second data block of that file and so on. 

When a file is written to, the entire file must be passed to the write function. The write function sequentially checks each section of 512 bytes to see if it matches the cached block. If a block does not match, the block is written to cache. 