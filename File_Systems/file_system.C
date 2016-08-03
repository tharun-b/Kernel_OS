/*
    File: file_system.C

    Author: Tharun Battula
            Department of ELectrical ENgineering
            Texas A&M University
    Date  : 5/3/16


    This file has file_system class and file class
   It contains main implementaion of file management according to the classe
   it has file functionality
   and closely with disk to read and write
   inode implementation

*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/
#include "file_system.H"
#include "console.H"
#define DEBUG_FLAG 0 /* To enable and disable Debug flags */
/*
 * File class function definitions
 */
File::File()
{
    /*
     * Initialize file attributes
     */
    cur_pos_in_block = 0;
    cur_blk_for_read = -1;
    first_blk = -1;
    file_size = 0;
    file_id = -1;
    ind_of_cur_block = 1;
    file_blocks = NULL;
}

unsigned int File::Read(unsigned int n, unsigned char * buffer)
{
    if (cur_blk_for_read == -1 || first_blk == -1)
    {
        Console::puts("File has not been initialized\n");
        return 0;
    }
#if 0
    Console::putui(cur_pos_in_block);
    // cur_blk_for_read =0;
    Console::putui(cur_blk_for_read);
#endif
    file_blocks = file_system->GetFileBlocks(file_id);
    int number_of_char_read = 0;
    cur_blk_for_read = file_blocks[ind_of_cur_block - 1];
    unsigned char block_data[BLOCK_SIZE];
    file_system->disk->read(cur_blk_for_read, block_data);
#if DEBUG_FLAG
            Console::puts("Readen0 \n");
            for(int j = 0 ; j<BLOCK_SIZE;j++)
            {
            	Console::putch(block_data[j]);
            	Console::puts(",");
            }
            Console::puts("cur_blk_for_read \n");
            Print_file_information();


#endif

    while(n > 0 && !EoF())
    {
        buffer[number_of_char_read] = block_data[cur_pos_in_block];
        number_of_char_read++;
        cur_pos_in_block++;
        if (cur_pos_in_block >= BLOCK_SIZE)
        {
            cur_pos_in_block = 0;
            ind_of_cur_block++;
            if (ind_of_cur_block >= 10)
            /*
             * A file can contain only 10 blocks
             */
            {
                return number_of_char_read;
            }
            file_blocks = file_system->GetFileBlocks(file_id);
            cur_blk_for_read = file_blocks[ind_of_cur_block - 1];
            file_system->disk->read(cur_blk_for_read, block_data);
#if DEBUG_FLAG
            Console::puts("Readen \n");
            for(int j = 0 ; j<BLOCK_SIZE;j++)
            {
            	Console::putch(block_data[j]);
            	Console::puts(",");
            }
            Console::puts("Extra \n");
#endif
        }
        n--;
    }
    return number_of_char_read;
}

unsigned int File::Write(unsigned int _n_size, unsigned char * buffer)
{
    if (cur_blk_for_read == -1 || first_blk == -1)
    {
        Console::puts("File has not been initialized\n");
        return 0;
    }
#if DEBUG_FLAG
    Print_file_information();
#endif
    int position  = cur_pos_in_block;
    int block_to_write = cur_blk_for_read;
    int char_written = 0;
    unsigned char curr_block_data[BLOCK_SIZE];
    file_system->disk->read(block_to_write, curr_block_data);
    unsigned int blk_index = ind_of_cur_block;
    while (char_written < _n_size || buffer[char_written] == '\0')
    {
        curr_block_data[position] = buffer[char_written];
        position++;
        char_written++;
        if(position >= BLOCK_SIZE)
        {
        	position = 0;
        	blk_index++;
        	file_system->disk->write(block_to_write, curr_block_data);
#if DEBUG_FLAG
        	Console::puts("Written1 \n");
        	for(int j = 0 ; j<BLOCK_SIZE;j++)
        	{
        		Console::putch(curr_block_data[j]);
        		Console::puts(",");
        	}
        	Console::puts("Inside loop \n");
#endif
            file_blocks = file_system->GetFileBlocks(file_id);

        	if(file_blocks[blk_index-1] != 0 )
        	{
                block_to_write = file_blocks[blk_index-1];
        		file_system->disk->read(block_to_write, curr_block_data);
        	}
        	else
        	{
        		int new_block_number = file_system->get_next_free_block_num();
        		file_system->inode_add_new_block_num(file_id, new_block_number);
        	//	file_system->UpdateINodeWithNewFileSize(this, file_id, BLOCK_SIZE - cur_pos_in_block);
        		block_to_write = new_block_number;
        		file_system->disk->read(block_to_write, curr_block_data);
        	}
        }
    }
    if(file_size < (((ind_of_cur_block-1)*BLOCK_SIZE) + cur_pos_in_block +_n_size))
    {
    	file_system->inode_increase_size(this, file_id, (((ind_of_cur_block-1)*BLOCK_SIZE) + cur_pos_in_block +_n_size)-file_size);
    }

    // indicates EOF
    curr_block_data[position++] = -1;
    file_system->disk->write(block_to_write, curr_block_data);

#if DEBUG_FLAG
    Console::puts("Written1 \n");
    for(int j = 0 ; j<BLOCK_SIZE;j++)
    {
    	Console::putch(curr_block_data[j]);
    	Console::puts(",");
    }
    Console::puts("Outside \n");

#endif
    return char_written;
}

void File::Reset()
{
    cur_pos_in_block = 0;
    cur_blk_for_read = first_blk;
    ind_of_cur_block =1;
#if DEBUG_FLAG
    Print_file_information();
#endif
    /* Private Variables
    unsigned int cur_pos_in_block;
       unsigned long first_blk;
       unsigned long cur_blk_for_read;
       unsigned int  ind_of_cur_block;
       unsigned int * file_blocks;
       unsigned int file_size;
       FileSystem   * file_system;
       unsigned int   file_id;*/
}

void File::Print_file_information()
{

    Console::puts("\nFile ID: ");
    Console::puti(file_id);
    Console::puts("\n");

    Console::puts("File Size : ");
    Console::puti(file_size);
    Console::puts("\n");


    Console::puts("Starting Block : ");
    Console::puti(first_blk);
    Console::puts("\n");

    Console::puts("Current Block : ");
    Console::puti(cur_blk_for_read);
    Console::puts("\n");

    Console::puts("Current Position : ");
    Console::puti(cur_pos_in_block);
    Console::puts("\n\n");

    Console::puts("Current Block Index");
    Console::puti(ind_of_cur_block);
    Console::puts("\n");
}

void File::Rewrite()
{
    unsigned char temp_bufer[BLOCK_SIZE];
    /* The first buffer */
    memset(temp_bufer, 0, BLOCK_SIZE);
    temp_bufer[0] = -1;
    file_system->disk->write(file_blocks[0],temp_bufer);

    /* The rest of the bufdr*/
    for (int i = 1; i < 10; i++)
    {
        if (file_blocks[i] != 0)
        {
            file_system->disk->write(file_blocks[i], temp_bufer);
            file_system->release_blk(file_blocks[i]);
            file_blocks[i] = 0;
        }
        else
        {
            break;
        }
    }
    file_size = 0;
    Reset();

}

BOOLEAN File::EoF()
{

    unsigned char block_data[BLOCK_SIZE];
    if((BLOCK_SIZE* (ind_of_cur_block-1)) +cur_pos_in_block == file_size)
    {
    	return TRUE;
    }

    file_system->disk->read(cur_blk_for_read, block_data);
    if(block_data[cur_pos_in_block] == -1)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}



/*
 * Filesystem function defintions
 */

/*
 * Defining static variables
 */

BlockingDisk * FileSystem::disk;
unsigned int FileSystem::size;
unsigned long FileSystem::no_of_blks;
unsigned long FileSystem::no_of_inodes;
unsigned long FileSystem::inode_info_blocks;
BOOLEAN FileSystem::is_mounted;
unsigned long* FileSystem::free_blk_bit_map;


FileSystem::FileSystem()
{
    FileSystem::disk = NULL;
    is_mounted = FALSE;
    size = 0;
}



BOOLEAN FileSystem::Format(BlockingDisk *_disk, unsigned int size)
{
    if (size > MAX_DISK_SIZE)
    {
        Console::puts("File system size cannot exceed maximum disk size! Exiting\n");
        return FALSE;
    }
    FileSystem::disk = _disk;
    FileSystem::size = size;
    FileSystem::no_of_blks = (size+(BLOCK_SIZE-1) )/ BLOCK_SIZE;
    memset(free_blk_bit_map, 0, ((FileSystem::no_of_blks)/32) * sizeof(unsigned long));

    unsigned char buffer[BLOCK_SIZE];
    memset(buffer, 0, BLOCK_SIZE);

#if DEBUG_FLAG
    Console::puts("\n no of blocks: ");
       Console::puti(no_of_blks);
       Console::puts("\n");

       Console::puts("inodes : ");
       Console::puti(no_of_inodes);
       Console::puts("\n");


       Console::puts(" inode mgmt blocks: ");
       Console::puti(inode_info_blocks);
       Console::puts("\n");

       Console::puts("size: ");
       Console::puti(size);
       Console::puts("\n");

       Console::puts("free block map : ");
       Console::puti((int)free_blk_bit_map);
       Console::puts("\n\n");

#endif

    for(int i = 0 ; i < FileSystem::no_of_blks; i++)
    {
#if DEBUG_FLAG
    	Console::puts("\t");
    	Console::putui(i);
    	Console::puts("\t");

#endif
        FileSystem::disk->write(i, buffer);
    }

    // This is a design choice.
    FileSystem::no_of_inodes = (FileSystem::no_of_blks +9)/ 10;

    /*
     * We require some blocks to store the inode information. 
     * We are using the first few blocks in the filesystem for the 
     * management information of inodes
     */
    FileSystem::inode_info_blocks = (((no_of_inodes * sizeof(iNode_t)))+(BLOCK_SIZE-1))/ BLOCK_SIZE ;
    
    int number_of_full_frames = inode_info_blocks / 32;
    int left_over = inode_info_blocks % 32;
    
    int i = 0;
    for (i = 0; i < number_of_full_frames; i++)
        free_blk_bit_map[i] = 0xFFFFFFFF;

    for (int j = 0; j < left_over; j++)
        free_blk_bit_map[i] = BIT_SET(free_blk_bit_map[i], j);

#if DEBUG_FLAG

    Console::puts("\n no of blocks: ");
       Console::puti(no_of_blks);
       Console::puts("\n");

       Console::puts("inodes : ");
       Console::puti(no_of_inodes);
       Console::puts("\n");


       Console::puts(" inode mgmt blocks: ");
       Console::puti(inode_info_blocks);
       Console::puts("\n");

       Console::puts("size: ");
       Console::puti(size);
       Console::puts("\n");

       Console::puts("free block map : ");
       Console::puti((int)free_blk_bit_map);
       Console::puts("\n\n");
#endif


    return TRUE;
}

BOOLEAN FileSystem::Mount(BlockingDisk *_disk)
{
    if (_disk && !is_mounted)
    {
        FileSystem::is_mounted = TRUE;
        FileSystem::disk = _disk;
        return TRUE;
#if DEBUG_FLAG
    	Console::puts("Mounting Done ");
#endif
    }
    else
        return FALSE;
}

BOOLEAN FileSystem::LookupFile(int file_id, File *file)
{
#if DEBUG_FLAG
	print_file_system_details();
#endif
	unsigned char buffer[BLOCK_SIZE];
    for (int i = 0; i < inode_info_blocks; i++)
    {
        disk->read(i, buffer);
#if DEBUG_FLAG
        Console::puts("FInished reading for lookup\n");
#endif
        iNode_t *inode_list = (iNode_t*) buffer;
        int no_of_inodes_in_list = BLOCK_SIZE/sizeof(iNode_t);
        for (int j = 0; j < no_of_inodes_in_list; j++)
        {
            if (inode_list[j].file_id == file_id)
            {
                //Initialize the passed file object
                file->cur_pos_in_block = 0;
                file->file_size = 0;
                file->ind_of_cur_block = 1;

                file->first_blk = inode_list[j].block_no[0];
                file->cur_blk_for_read  = inode_list[j].block_no[0];
                file->file_id = file_id;
                file->file_system = this;
                file->file_blocks = GetFileBlocks(file_id); 

                return TRUE;
            }
        }
    }
    return FALSE;
}

BOOLEAN FileSystem::CreateFile(int file_id)
{
    if(!is_mounted || file_id == 0)
    {
        Console::puts("File system not mounted or invalid file ID!! Returning\n");
        return FALSE;
    }

    File *file;
    if(LookupFile(file_id, file))
    {
        Console::puts("File already exists! Not creating again\n");
        return FALSE;
    }
    
    unsigned char buffer[BLOCK_SIZE];
    for(int i = 0; i < inode_info_blocks; i++)
    {
        disk->read(i, buffer); 
        iNode_t *inode_list = (iNode_t*)buffer;
        int no_of_inodes_in_list = BLOCK_SIZE/sizeof(iNode_t);
        for (int j = 0; j < no_of_inodes_in_list; j++)
        {
            if (inode_list[j].file_id == 0)
            {
                inode_list[j].file_id   = file_id;
                inode_list[j].file_size = 0;
                inode_list[j].block_no[0] = get_next_free_block_num();
                inode_list[j].no_of_blks_used = 0;
                disk->write(i, buffer);
                return TRUE;
            }
        }
    }
    return FALSE;
}



BOOLEAN FileSystem::DeleteFile(int file_id)
{
    if(!is_mounted || file_id == 0)
    {
        Console::puts("File system not mounted or invalid file ID!! Returning\n");
        return FALSE;
    }

    unsigned char buffer[BLOCK_SIZE];
    for(int i = 0; i < inode_info_blocks; i++)
    {
        disk->read(i, buffer); 
        iNode_t *inode_list = (iNode_t*)buffer;
        int no_of_inodes_in_list = BLOCK_SIZE/sizeof(iNode_t);
        for (int j = 0; j < no_of_inodes_in_list; j++)
        {
            if (inode_list[j].file_id == file_id)
            {
                inode_list[j].file_id   = 0;
                inode_list[j].file_size = 0;
                for (int k = 0; k < 10; k++)
                {
                    if (inode_list[j].block_no[k])
                        release_blk(inode_list[j].block_no[k]);
                    inode_list[j].block_no[k] = 0;
                }
                inode_list[j].no_of_blks_used = 0;
                disk->write(i, buffer);
                return TRUE;
            }
        }
    }
    return FALSE;
}

unsigned long FileSystem::get_next_free_block_num()
{
    BOOLEAN block_found = false;
    int i, j;
    unsigned long block_number;
    for ( i = 0 ; i < no_of_blks / 32; i++)
    {
        if (free_blk_bit_map[i] != 0xFFFFFFFF)
        {
            for ( j = 0; j <= 31; j++ )
            {
                if (IS_SET(free_blk_bit_map[i], j))
                    continue;
                else
                {
                    free_blk_bit_map[i] = BIT_TOGGLE(free_blk_bit_map[i], j);
                    block_found = true;
                    break;
                }
            }
            if (block_found)
                break;
        }
    }
    if (block_found)
    {
        block_number = j + i*32;
        return block_number;
    }
    else
    {
        return 0;
    }
} 


unsigned int* FileSystem::GetFileBlocks(int file_id)
{
    unsigned char buffer[BLOCK_SIZE];
    for (int i = 0; i < inode_info_blocks; i++)
    {
        disk->read(i, buffer);

        iNode_t *inode_list = (iNode_t*) buffer;
        int no_of_inodes_in_list = BLOCK_SIZE/sizeof(iNode_t);
        for (int j = 0; j < no_of_inodes_in_list; j++)
        {
            if (inode_list[j].file_id == file_id)
            {
                return inode_list[j].block_no;
            }
        }
    }
    return NULL;
   
}



void FileSystem::inode_increase_size(File *_file, int file_id, int file_size_increase)
{
    unsigned char buffer[BLOCK_SIZE];
    for (int i = 0; i < inode_info_blocks; i++)
    {
        disk->read(i, buffer);
        iNode_t *inode_list = (iNode_t*) buffer;
        int no_of_inodes_in_list = BLOCK_SIZE/sizeof(iNode_t);
        for (int j = 0; j < no_of_inodes_in_list; j++)
        {
            if (inode_list[j].file_id == file_id)
            {
                inode_list[j].file_size += file_size_increase;
                _file->file_size = inode_list[j].file_size;
                disk->write(i, buffer);
            }
        }
    }
}


void FileSystem::print_file_system_details()
{

	Console::puts(" inode info blocks: ");
	Console::puti(inode_info_blocks);
	Console::puts("\n");

	Console::puts("\n no of blocks: ");
	Console::puti(no_of_blks);
	Console::puts("\n");

	Console::puts("information nodes count : ");
	Console::puti(no_of_inodes);
	Console::puts("\n");

	Console::puts("free block map : ");
	Console::puti((int)free_blk_bit_map);
	Console::puts("\n\n");
	Console::puts("size: ");
	Console::puti(size);
	Console::puts("\n");

}

void FileSystem::inode_add_new_block_num(int file_id, int block_no)
{
    unsigned char buffer[BLOCK_SIZE];
    for (int i = 0; i < inode_info_blocks; i++)
    {
        disk->read(i, buffer);
        iNode_t *inode_list = (iNode_t*) buffer;
        int no_of_inodes_in_list = BLOCK_SIZE/sizeof(iNode_t);
        for (int j = 0; j < no_of_inodes_in_list; j++)
        {
            if (inode_list[j].file_id == file_id)
            {
                inode_list[j].no_of_blks_used++;
                inode_list[j].block_no[inode_list[j].no_of_blks_used] = block_no;
                disk->write(i, buffer);
            }
        }
    }
}

void FileSystem::release_blk(unsigned long block_no)
{
    int bit_position = block_no % 32;
    int index_of_frame = block_no / 32;
    if (IS_SET (free_blk_bit_map[index_of_frame], bit_position))
    {
        free_blk_bit_map[index_of_frame] = BIT_TOGGLE(free_blk_bit_map[index_of_frame], bit_position);
    }
}
