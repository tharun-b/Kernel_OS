/*
 File: frame_pool.C

 Author: Tharun Battula
 Department of Electrical Engineering
 Texas A&M University
 Date  : 01/30/2016

 Description: Management of the Free-Frame Pool.


 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

//#define MB * (0x1 << 20)
//#define KB * (0x1 << 10)
//#define KERNEL_POOL_START_FRAME ((2 MB) / (4 KB))
//#define KERNEL_POOL_SIZE ((2 MB) / (4 KB))
//#define PROCESS_POOL_START_FRAME ((4 MB) / (4 KB))
//#define PROCESS_POOL_SIZE ((28 MB) / (4 KB))
///* definition of the kernel and process memory pools */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/
#include "interrupts.H"
#include "frame_pool.H"
#include "console.H"
/*--------------------------------------------------------------------------*/
/* Function Definitions */
/*--------------------------------------------------------------------------*/

/* Allocates a frame from the frame pool. If successful, returns the frame
 * number of the frame. If fails, returns 0. */

FramePool::FramePool(unsigned long _base_frame_no, unsigned long _nframes,
		unsigned long _info_frame_no) {

	base_frame_num = _base_frame_no;
	num_frames = _nframes;
	info_frame_num = _info_frame_no;
	unsigned int char_4_fr_count = ((num_frames + 3) >> 2);

	if (info_frame_num == 0) {
		/* Need to add a frame for storing the info
		 * and declaring inaccessible
		 */
		bit_map_ptr = (unsigned char *) (base_frame_num * (FRAME_SIZE));
		memset(bit_map_ptr, 0, FRAME_SIZE); /* Bitmap is within single frame */
		info_frame_num = get_frame();

	} else {
		/* Pool has already available info bit map table */
		bit_map_ptr = (unsigned char *) (info_frame_num * (FRAME_SIZE));
		 /* Taking Ceiling of the number */
		/* TODO:
		 * Little fragmentation, as it expects pool number of frames to be divisible by 4
		 * Further can be improved if FramePool declarations are frequent*/

		for (int i = 0; i < ((num_frames + 3) >> 2); i++) {
			bit_map_ptr[i] = 0; /* Setting All pages to unused */
		}
	}

#if DEBUG_ENABLE
		Console::puts(" base_frame_num =  ");
		Console::puti(base_frame_num);

		Console::puts("\t info frame =  ");
		Console::puti(info_frame_num);

		Console::puts("\t num frame =  ");
		Console::puti(num_frames);
#endif

}
;

unsigned long FramePool::get_frame() {
	unsigned int cur_char_idx = 0;
	unsigned int frame_sub_index = 0;

	while (((bit_map_ptr[cur_char_idx] & 85) == 85)
			|| ((bit_map_ptr[cur_char_idx] & 170) == 170))
	{/* 85 = 0x55 = 01010101 All the bits are allocated and accessible
	 170 = 0xAA = 10101010 All the bits are inaccessbile */
		if (cur_char_idx >= ((num_frames + 3) >> 2))
		{
#if DEBUG_ENABLE /* Prints For Debugging */
			Console::puti(char_4_fr_count);
			Console::puts(" ");
			Console::puti(cur_char_idx);
			Console::puts(
					"All Frames are allocated or inaccessible, Time to move frames to disk\n");
#endif
			return 0;
		}
		++cur_char_idx;
	}
	{
		/* cur_idx - Holds frame with 00 */
		unsigned char pos_eval = 1;
		unsigned char char_with_slot = ((bit_map_ptr[cur_char_idx] | 170) ^ 255);

		while ((char_with_slot & pos_eval) == 0) {
			frame_sub_index++;
			pos_eval = pos_eval << 2;
		}

		/* set the frame status to 01 */
		bit_map_ptr[cur_char_idx] += pos_eval;

#if DEBUG_ENABLE /* Prints For Debugging */
		Console::putui(bit_map_ptr[cur_char_idx]);
#endif

	}

#if DEBUG_ENABLE /* Prints For Debugging */
	{
		for(int i = 0; i<(num_frames>>2); i++)
		{
			if(bit_map_ptr[base_frame_num+i]==0) { break;}
			Console::puti(bit_map_ptr[base_frame_num+i]);
			Console::puts(" ,");
		};
		Console::puts("\t");
	}
#endif
	/* local index + micro index + global index */
	return (base_frame_num + (cur_char_idx << 2) + frame_sub_index);

};

/* Mark the area of physical memory as inaccessible. The arguments have the
 * same semanticas as in the constructor.
 */
void FramePool::mark_inaccessible(unsigned long _base_frame_no,
		unsigned long _nframes) {
	/* Relative index of frames dat structure in char array */
	unsigned int index = ((_base_frame_no-base_frame_num) >> 2);
	unsigned int temp = 2; /* temp used for setting inaccessible feature */
	temp = temp << ((_base_frame_no-base_frame_num) - (index << 2));

#if DEBUG_ENABLE /* Prints For Debugging */
	{
		Console::putui((_base_frame_no-base_frame_num));
		Console::puti(bit_map_ptr[index]);
		Console::puts("->");
	}
#endif

	for (int i = 0; i < _nframes; i++) {
		bit_map_ptr[index] |= temp; /* Setting Inaccessible flag */
		if (temp >= 128) {
			temp = 2;
			index++;
		} else {
			temp <<= 2;
		}
	}

#if DEBUG_ENABLE /* Prints For Debugging */
	{
		Console::putui((_base_frame_no-base_frame_num));
		Console::puti(bit_map_ptr[((_base_frame_no+_nframes-base_frame_num) >> 2)]);
		Console::puts("\t");
	}
#endif

};

/* Releases frame back to the given frame pool.
 The frame is identified by the frame number.
 NOTE: This function is static because there may be more than one frame pool
 defined in the system, and it is unclear which one this frame belongs to.
 This function must first identify the correct frame pool and then call the frame
 pool's release_frame function. */

/* -- WE DO THE FOLLOWING TO KEEP THE COMPILER HAPPY. */

void FramePool::release_frame(unsigned long _frame_no) {

	int frame_offset = (_frame_no - base_frame_num); /* Frame number with respect to pool requested */
	unsigned int shift_req = (frame_offset - ((frame_offset>>2)<<2))<<1;
	/* The sub index < 8 for shifting in the Char data structure  */

	unsigned char fr_4_data = bit_map_ptr[(frame_offset>>2)];
	/* The current 4 frames info stored in char structure  */


#if DEBUG_ENABLE /* Prints For Debugging */
	{
		Console::putui(frame_offset);
		Console::puti(bit_map_ptr[(frame_offset>>2)]);
		Console::puts("   ");
	}
#endif
	/* Checking the range of Frame */
	if ((frame_offset < 0)
			|| (frame_offset > num_frames))
	{
		Console::puts(
				"release_frame: Frame is out of range of this pool\n");
		return;
	}

	/* Checking the Accessibility of Frame */
	if(((fr_4_data>>shift_req) & 3) > 1)
	{
		Console::puts(
				"release_frame: Frame Inaccessible\n");
		return;
	}
	else
	{/* Releasing the frame by setting char data to 0 */
		bit_map_ptr[(frame_offset>>2)] = fr_4_data & (~(3<<shift_req));
	}

#if DEBUG_ENABLE /* Prints For Debugging */
	{
		Console::putui(frame_offset);
		Console::puti(bit_map_ptr[(frame_offset>>2)]);
		Console::puts("   ");
	}
#endif

	return;
}

