/*
    File: frame_pool.H

    Author: Tharun Battula
            Department of Electrical Engineering
            Texas A&M University
    Date  : 01/30/2016

    Description: Management of the Free-Frame Pool.


*/

#ifndef _FRAME_POOL_H_                   // include file only once
#define _FRAME_POOL_H_

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/
#define DEBUG_ENABLE 0
#define FRAME_SIZE (1<<12)
/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* F r a m e   P o o l  */
/*--------------------------------------------------------------------------*/

class FramePool {

private:
     /* -- DEFINE YOUR FRAME POOL DATA STRUCTURE(s) HERE. */
	unsigned long base_frame_num, num_frames, info_frame_num;
	/* Base_frame_number = is the frame number at the start of the physical memory
	      region that this frame pool manages.
	 num_frames = is the number of frames in the physical memory region that this
	      frame pool manages
	 info_frame_num = is the numbered address in the kernel memory of where bitmap info of the pool
	  is located

	  These get initialized in the constructor of the class*/

	unsigned char *bit_map_ptr;
	/* Bit Map Table Points to Frame location with status of the pool's framses*/


public:



	//FramePool();

   FramePool(unsigned long _base_frame_no,
             unsigned long _nframes,
             unsigned long _info_frame_no);
   /* Initializes the data structures needed for the management of this
      frame pool. This function must be called before the paging system
      is initialized.
      _base_frame_no is the frame number at the start of the physical memory
      region that this frame pool manages.
      _nframes is the number of frames in the physical memory region that this
      frame pool manages.
      e.g. If _base_frame_no is 16 and _nframes is 4, this frame pool manages
      physical frames numbered 16, 17, 18 and 19
      _info_frame_no is the frame number (within the directly mapped region) of
      the frame0 that should be used to store the management information of the
      frame pool. However, if _info_frame_no is 0, the frame pool is free to
      choose any frame from the pool to store management information.
      */

   unsigned long get_frame();
   /* Allocates a frame from the frame pool. If successful, returns the frame
    * number of the frame. If fails, returns 0. */
   /* If get_frame() runs out of memory of accessible region
    * it will return 0
    */

   void mark_inaccessible(unsigned long _base_frame_no,
                          unsigned long _nframes);
   /* Mark the area of physical memory as inaccessible. The arguments have the
    * same semanticas as in the constructor.
    */

   void release_frame(unsigned long _frame_no);

   /* Releases frame back to the given frame pool.
      The frame is identified by the frame number.
    * If this function asks for releasing inaccesible frame -
    	* It will not free it,
    	* It prints out warning on the screen for Log purposes.
	* If this function asks for releasing out of range frame -
    	* It will not free it.
    	* It prints out the warning on the screen for Log purposes.

      NOTE: This function is changed Non static Hence use only object instances
      * It is good coding practice to keep the information within the pool structure
      * Avoiding Static/Global variables
      * Making Static required extra memory for storing every instancee
      * Design becomes complex to maintain the extra memory
      * If Linked list is used, looking inside linked lists take more time in design also caching
      * It is easier implementation
      */
};
#endif
