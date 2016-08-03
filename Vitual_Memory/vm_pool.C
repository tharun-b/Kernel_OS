/*
    File: vm_pool.C

    Author: Tharun Battula
            Department of Computer Science
            Texas A&M University
    Date  : 10/26/2010

    Description: Management of the Virtual Memory Pool


*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/
#define DEBUG_ENABLE 0
#define MEMORY_PROFILE 0

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/
#include "page_table.H"
#include "vm_pool.H"
#include "console.H"
/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* Forward declaration of class PageTable */

/*--------------------------------------------------------------------------*/
/* V M  P o o l  */
/*--------------------------------------------------------------------------*/

   VMPool::VMPool(unsigned long _base_address,
          unsigned long _size,
          FramePool *_frame_pool,
          PageTable *_page_table):base_address(_base_address),
          page_table(_page_table),frame_pool(_frame_pool)
   {
	   /* Allocation of a frame page for list holding*/
/* Tried to allocate within the VMPool but it leads to chicken and egg problem */
	   /* Start Using virtual memory as if it is allocated */

	   alloc_list = (l_list_elem*)(_page_table->allocate_kernel_frame()*PageTable::PAGE_SIZE);
	   avail_list  = (l_list_elem *)(_page_table->allocate_kernel_frame()*PageTable::PAGE_SIZE);

	   _page_table->register_vmpool(this);

	     //Zero the frame obtained
	     /* Check if this causes any issues */
#if DEBUG_ENABLE

	     /* Checks for checking the frames regularity */
	     this->is_legitimate((unsigned long)alloc_info_frame);

	     Console::puts("\nAlloc data frame structure:");
	     Console::putui((unsigned int)alloc_info_frame);
	     Console::puts("\nFree data frame structure:");
	     Console::putui((unsigned int)free_info_frame);

	     Console::putui(alloc_info_frame[0]);
	     Console::putui(free_info_frame[0]);
	     Console::puts("\n=============\n");

#endif
#if 1
	     memset(alloc_list, 0, PageTable::PAGE_SIZE);
	     memset(avail_list, 0, PageTable::PAGE_SIZE);
#endif

	     //Size in number of pages
	     size = _size / PageTable::PAGE_SIZE;

	     //Entering the first entry in free list
	     avail_list[0].size = (size-2);
	     avail_list[0].address = (base_address+ 2*PageTable::PAGE_SIZE);
	     avail_list[0].next = NULL;

	     alloc_list[0].size = 2;
	     alloc_list[0].address = base_address;
	     alloc_list[0].next = NULL;

#if DEBUG_ENABLE
	     Console::putui(this->is_legitimate((unsigned long)alloc_info_frame));
	     Console::putui(this->is_legitimate((unsigned long)free_info_frame));

	     for(;;);
#endif
   }
   /* Initializes the data structures needed for the management of this
    * virtual-memory pool.
    * _base_address is the logical start address of the pool.
    * _size is the size of the pool in bytes.
    * _frame_pool points to the frame pool that provides the virtual
    * memory pool with physical memory frames.
    * _page_table points to the page table that maps the logical memory
    * references to physical addresses. */

   unsigned long VMPool::allocate(unsigned long _size)
   {
	      struct l_list_elem *ptr_avail = avail_list;
	      struct l_list_elem *ptr_alloc = alloc_list;

	      /* in the free list search for free page */
	      /* Getting the size in frame numbers
	       * Internal fragmentation allowed
	       * Getting ceiling function by getting to the next number
	       */
#if MEMORY_PROFILE
			  {
	    	  unsigned long total_mem = 0 ;
	    	  ptr_avail = avail_list;
	    	  while(ptr_avail!=NULL)
	    	  {
	    		  total_mem+=ptr_avail->size;
	    		  ptr_avail=ptr_avail->next;

	    	  }
	    	  Console::puts("[");
	    	  Console::putui(total_mem);
	    	  total_mem = 0;

	    	  ptr_avail = alloc_list;
	    	  while(ptr_avail!=NULL)
	    	  {
	    		  total_mem+=ptr_avail->size;
	    		  ptr_avail=ptr_avail->next;

	    	  }
	    	  Console::putui(total_mem);
	    	  Console::puts("]\t");


	    	  ptr_avail = avail_list;
			  }
#endif

			  /* Rounding off to nearest */
	      _size = (_size+ (PageTable::PAGE_SIZE-1))/PageTable::PAGE_SIZE;

	      if(_size > this->size)
	      {
	    	  return 0;
	      }

	      unsigned long ret_addr_allocated = 0 ;
	   /* Logic for no free page found*/
	      while(ptr_avail != NULL)
	      {
	    	  if(ptr_avail->size < _size)/* needed size more, Move on */
	    	  {
	    		  ptr_avail = ptr_avail->next;
	    	  }
/*	    	  else if(ptr_avail->size == _size)  Exact Size Match, Allocate
	    	  {
	    		  ptr_avail->size = 0;
	    		  ret_addr_allocated = ptr_avail->address;
	    		  break;
	    	  }*/
	    	  else /*  More Size available, Allocate */
	    	  {
	    		  ptr_avail->size    -= _size;
	    		  ret_addr_allocated = ptr_avail->address;
	    		  ptr_avail->address += _size * PageTable::PAGE_SIZE;
	    		  break;
	    	  }
	      }

	      /* No frame allocated so far */
	      if(ptr_avail == NULL) /* Exhausted the free frame list */
	      {
#if 1
	    	  Console::puts("No free space! Please free space before allocation in this pool\n");
		      return 0 ;
#endif
	      }
	      else /* Break happened in the prev while loop */
	      {/* Allocated frame is done */
	    	  /* First free void , mostly hits tail */
	    	  while((ptr_alloc->size != 0) && (ptr_alloc->next != NULL ))
	    	  {
	    		  ptr_alloc = ptr_alloc->next;
	    	  }

	    	  if((ptr_alloc->next == NULL)&&(ptr_alloc->size != 0))
	    	  {
	    		  /* if the current list size exceeds ,allocate new page */
	    		  if (((ptr_alloc - alloc_list) % PageTable::PAGE_SIZE) + sizeof(l_list_elem) > PageTable::PAGE_SIZE)  //overflow.
	    		  {
	    			  /* recursive call make sure it doesnt cross limits */
//	    			  unsigned long new_frame = this->allocate(PageTable::PAGE_SIZE);
#if 1
	    			  unsigned long new_frame = page_table->allocate_kernel_frame()* PageTable::PAGE_SIZE;
#endif
	    			  ptr_alloc->next = (l_list_elem *)new_frame;
	    			  /* Allocation for next record if required*/
	    		  }
	    		  else
	    		  {
	    			  ptr_alloc->next = ptr_alloc + sizeof(struct l_list_elem);
	    			  /* Pointer Updated to next record*/

	    		  }
	    		  ptr_alloc  = ptr_alloc->next;  /* Next record data written*/
	    	  }

	    	  ptr_alloc->next = NULL;
	    	  ptr_alloc->size = _size;
	    	  ptr_alloc->address = ret_addr_allocated;

	    	  return ret_addr_allocated;
	      }

	      return 0 ;
   }
   /* Allocates a region of _size bytes of memory from the virtual
    * memory pool. If successful, returns the virtual address of the
    * start of the allocated region of memory. If fails, returns 0. */

   void VMPool::release(unsigned long _start_address)
   {
	   struct l_list_elem *ptr_alloc = alloc_list;
	   struct l_list_elem *ptr_avail = avail_list;
#if MEMORY_PROFILE
			  {
	    	  unsigned long total_mem = 0 ;
	    	  ptr_avail = avail_list;
	    	  while(ptr_avail!=NULL)
	    	  {
	    		  total_mem+=ptr_avail->size;
	    		  ptr_avail=ptr_avail->next;
	    	  }
	    	  Console::puts("[");
	    	  Console::putui(total_mem);
	    	  total_mem = 0;
	    	  ptr_avail = alloc_list;
	    	  while(ptr_avail!=NULL)
	    	  {
	    		  total_mem+=ptr_avail->size;
	    		  ptr_avail=ptr_avail->next;

	    	  }
	    	  Console::putui(total_mem);
	    	  Console::puts("]\t");

	    	  ptr_avail = avail_list;
			  }
#endif

	   unsigned int size;
	   while(ptr_alloc != NULL)
	   {
		   /* Check List with same start address */
		   if((ptr_alloc->address == _start_address) && (ptr_alloc->size != 0))
		   {
			   size = ptr_alloc->size;
			   ptr_alloc->size = 0;

#if 1 /* Immediate free of pages*/
			   // Can be delayed with proper mechanism
			   /* Calling Release on the frames to be freed */
			   {
				   for(int j=0 ; j < ptr_alloc->size; j++)
				   {
					   page_table->free_page(j+(_start_address/PageTable::PAGE_SIZE));
				   }

				   /* To Flush the TLB */
				    page_table->load();

			   }
#endif

			   break;
		   }
		   ptr_alloc = ptr_alloc->next;
	   }

	   if(ptr_alloc == NULL)
		   Console::puts("Invalid start address!");
	   else
	   {
		   while(ptr_avail->next != NULL && (ptr_avail->size != 0))
		   {
			   ptr_avail = ptr_avail->next;
		   }

		   if((ptr_avail->size != 0) && (ptr_avail->next == NULL))
		   {
			   /* New free element allocate */
			   if(((ptr_alloc - alloc_list) % PageTable::PAGE_SIZE) + sizeof(l_list_elem) > PageTable::PAGE_SIZE)  //overflow
			   {
				   /* recursive call make sure it doesnt cross limits */
//				   unsigned long new_frame = this->allocate(PageTable::PAGE_SIZE);
#if 1
				   unsigned long new_frame = page_table->allocate_kernel_frame()* PageTable::PAGE_SIZE;
#endif
				   ptr_avail->next = (l_list_elem *)new_frame;
			   }
			   else
			   {	/* Set the pointer */
				   ptr_avail->next = ptr_avail + sizeof(struct l_list_elem);
			   }

			   /* Move the pointer */
			   ptr_avail = ptr_avail->next;
		   }

		   ptr_avail->size = size;
		   ptr_avail->address = _start_address;
		   ptr_avail->next = NULL;
	   }


#if MEMORY_PROFILE
			  {
	    	  unsigned long total_mem = 0 ;
	    	  ptr_avail = avail_list;
	    	  while(ptr_avail!=NULL)
	    	  {
	    		  total_mem+=ptr_avail->size;
	    		  ptr_avail=ptr_avail->next;

	    	  }
	    	  Console::puts("[");
	    	  Console::putui(total_mem);
	    	  total_mem = 0;
	    	  ptr_avail = alloc_list;
	    	  while(ptr_avail!=NULL)
	    	  {
	    		  total_mem+=ptr_avail->size;
	    		  ptr_avail=ptr_avail->next;
	    	  }
	    	  Console::putui(total_mem);
	    	  Console::puts("]\t");

	    	  ptr_avail = avail_list;
			  }
#endif
   }
   /* Releases a region of previously allocated memory. The region
    * is identified by its start address, which was returned when the
    * region was allocated. */

   BOOLEAN VMPool::is_legitimate(unsigned long _address)
   {
	   struct l_list_elem *alloc_ptr = this->alloc_list;
	   unsigned long base_addre,size;
	   while(alloc_ptr)
	   {
		   base_addre = alloc_ptr->address;
		   size = alloc_ptr->size;
		   if(( base_addre <= _address) && (_address < (base_addre + size* PageTable::PAGE_SIZE)))
			   return TRUE;
		   else
			   alloc_ptr =  alloc_ptr->next;
	   }
	   return FALSE;
   }
   /* Returns FALSE if the address is not valid. An address is not valid
    * if it is not part of a region that is currently allocated. */

