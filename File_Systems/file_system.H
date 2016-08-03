/* 
    File: file_system.H

    Author: R. Bettati
            Department of Computer Science
            Texas A&M University
    Date  : 10/04/05

    Description: File System.
    

*/

#ifndef _FILE_SYSTEM_H_                   // include file only once
#define _FILE_SYSTEM_H_

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

#define MB * (0x1 << 20)
#define KB * (0x1 << 10)
#define MAX_DISK_SIZE ((10 MB)) 
#define BLOCK_SIZE 512
#define MAX_NUMBER_OF_BLOCKS ( (MAX_DISK_SIZE) / (BLOCK_SIZE) )

#define IS_SET(var, pos) ((var) & (1<<(pos)))
#define BIT_SET(var, pos) ((var) |= (1<<(pos)))
#define BIT_TOGGLE(var, pos) ((var) ^= (1<<(pos)))


/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "utils.H"
#include "simple_disk.H"
#include "blocking_disk.H"
/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/
typedef struct iNode
{
    unsigned int file_id;
    unsigned int file_size;
    unsigned int block_no[10];
    unsigned int no_of_blks_used;
}iNode_t;

/*--------------------------------------------------------------------------*/
/* FORWARD DECLARATIONS */ 
/*--------------------------------------------------------------------------*/

class FileSystem;

/*--------------------------------------------------------------------------*/
/* F i l e */
/*--------------------------------------------------------------------------*/

class File {
    friend class FileSystem;
private: 
     /* -- DEFINE YOUR FILE DATA STRUCTURES HERE. */
     
    unsigned long cur_blk_for_read;
    unsigned int  ind_of_cur_block;
    unsigned int cur_pos_in_block;
    unsigned long first_blk;

    unsigned int * file_blocks;
    unsigned int file_size;
    FileSystem   * file_system;
    unsigned int   file_id;

     /* -- You may want to store other information, such as 
             .. position in the file
             .. cached block(s)
             .. etc.
      */

public:

    File();
    /* Constructor for the file handle. Set the 'current
       position' to be at the beginning of the file. */

    unsigned int Read(unsigned int _n, unsigned char * _buf);
    /* Read _n characters from the file starting at the 
       current location and copy them in _buf.
       Return the number of characters read. */

    unsigned int Write(unsigned int _n, unsigned char * _buf);
    /* Write _n characters to the file starting at the current 
       location, if we run past the end of file, we increase 
       the size of the file as needed. 
     */

    void Reset();
    /* Set the 'current position' at the beginning of the file. */

    void Rewrite();
    /* Erase the content of the file. Return any freed blocks. 
       Note: This function does not delete the file! It just erases its
       content. */

    BOOLEAN EoF();
    /* Is the current location for the file at the end of the file? */

    /* Helper functions */
    void Print_file_information();

};

/*--------------------------------------------------------------------------*/
/* F i l e S y s t e m  */
/*--------------------------------------------------------------------------*/

class FileSystem {

    friend class File;

private:
     /* -- DEFINE YOUR FILE SYSTEM DATA STRUCTURES HERE. */
     
     static unsigned int size;
     static BOOLEAN is_mounted;
     static unsigned long * free_blk_bit_map;
     static unsigned long no_of_blks;
     static unsigned long no_of_inodes;
     static unsigned long inode_info_blocks;
     static BlockingDisk * disk;

     
public:

   FileSystem();   
   /* Just initializes local data structures. Does not connect to disk yet. */

   BOOLEAN Mount(BlockingDisk * _disk);
   /* Associates the file system with a disk. We limit ourselves to at most one
      file system per disk. Returns TRUE if 'Mount' operation successful (i.e. there
      is indeed a file system on the disk. */

   static BOOLEAN Format(BlockingDisk * _disk, unsigned int _size);
   /* Wipes any file system from the given disk and installs a new, empty, file
      system that supports up to _size Byte. */

   BOOLEAN LookupFile(int _file_id, File * _file);
   /* Find file with given id in file system. If found, initialize the file 
      object and return TRUE. Otherwise, return FALSE. */

   BOOLEAN CreateFile(int _file_id);
   /* Create file with given id in the file system. If file exists already,
      abort and return FALSE. Otherwise, return TRUE. */

   BOOLEAN DeleteFile(int _file_id);
   /* Delete file with given id in the file system and free any disk block
      occupied by the file. */

   /* Helper functions */
   unsigned long get_next_free_block_num();


   // Returns the array with the block numbers of the file if the file is found
   // Returns NULL otherwise
   unsigned int* GetFileBlocks(int _file_id);

   /* freeing up memory of block */
   void release_blk(unsigned long block_no);

   /* Debug Prints for file systems*/
     void print_file_system_details();


   // Update INODE with new file size
   void inode_increase_size(File *_file, int file_id, int file_size_increase);

   // Update INODE with new block number
   void inode_add_new_block_num(int file_id, int block_no);

};

#endif
