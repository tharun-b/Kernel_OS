/*
 File: page_table.C

 Author: Tharun Battula
 Department of Electrical Engineering
 Texas A&M University
 Date  : 01/30/2016

 Description: Page Table Management

 */

#include "interrupts.H"
#include "frame_pool.H"
#include "console.H"
#include "page_table.H"
#include "paging_low.H"

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/
/* -- (none) -- */
#define DEBUG_ENABLE 0
/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* P A G E - T A B L E  */
/*--------------------------------------------------------------------------*/
 FramePool* PageTable::kernel_mem_pool =  0 ;
 FramePool* PageTable::process_mem_pool = 0;
 PageTable* PageTable::current_page_table = 0 ;
 unsigned long PageTable::shared_size =  (1<<22) ;
 unsigned int PageTable::paging_enabled = FALSE;
/* Set the global parameters for the paging subsystem. */

  void PageTable::init_paging(FramePool * _kernel_mem_pool,
                          FramePool * _process_mem_pool,
                          const unsigned long _shared_size)
  {
	  kernel_mem_pool = _kernel_mem_pool;
	  process_mem_pool = _process_mem_pool;
	  shared_size = _shared_size;
	  paging_enabled = FALSE;
  }

  /* Initializes a page table with a given location for the directory and the
     page table proper.
     NOTE: The PageTable object still needs to be stored somewhere! Probably it is best
           to have it on the stack, as there is no memory manager yet...
     NOTE2: It may also be simpler to create the first page table *before* paging
           has been enabled.
  */
  PageTable::PageTable(){
	  int i ;
	  page_directory = (unsigned long *)(kernel_mem_pool->get_frame() << 12);
	  if(page_directory ==0 )
	  {
		  Console::puts("Physical frame not available at  Page Directory request \n");
	  }
	  unsigned long *page_table_first = (unsigned long *)(kernel_mem_pool->get_frame() << 12);
	  if(page_directory ==0 )
	  {
		  Console::puts("Physical frame not available at first page table request \n");
	  }
	  unsigned long address = 0 ;
	  /* Set flags */
	  /*
	   * Setting up the Direct Mapped memory by creating page table
	   * Flag 3 to set R/W to 1  and present to 1
	   *
	   */
	  for ( i = 0; i < ENTRIES_PER_PAGE; ++i)
	  {
		  page_table_first[i] = (address|3);
		  address += (1<<12);
	  }

	  /*
	   * First Page entry of the Page Directory is Page Table that
	   * is pointing to the Direct Mapped memory of 0-4MB
	   */
	  page_directory[0] = (((unsigned long)page_table_first) | 3);
	  //attribute set to: supervisor level, read/write, present(011 in binary)
	  /*
	   * Setting the rest of page directory entries to not available
	   */
#if 1 /* Recursive Page Trick Enabled
 	 	 Other trial to Index entire memory like in linux */
	  // Trial Did not work - Can be investigated in future

	  for(i=(ENTRIES_PER_PAGE-2); i>0; i--)
	  {
		  page_directory[i] = 2;
		  // attribute set to: supervisor level, read/write, not present(010 in binary)
	  };
	  /* Main Step in the Recursive Page Lookup */
	  page_directory[ENTRIES_PER_PAGE-1] = ((unsigned long)page_directory|3);

#else
	  for(i=1; i<ENTRIES_PER_PAGE; i++)
	  {
		  int j = 0 ;
		  unsigned long *page_DE_i = (unsigned long *)(process_mem_pool->get_frame() << 12);
		  if(page_DE_i ==0)
		  {
			  break;
		  }
		  else
		  {
			  page_directory[i] = ((unsigned long)page_DE_i|3);
			  unsigned long *page_table_E;

			  for( j =0 ; j<1024; j++)
			  {
				  page_table_E = (unsigned long *)(process_mem_pool->get_frame() << 12);
				  if(page_table_E ==0)
				  {
					  break;
				  }
				  else
				  {
					  page_DE_i[j] = ((unsigned long)page_table_E|3);

				  }

			  }
			  for(;j<1024; j++)
			  {
				  page_DE_i[j] = 2;
			  }
			  if(page_table_E)
			  {
				  break;
			  }
		  }
		  // attribute set to: supervisor level, read/write, not present(010 in binary)
	  };

	  for(; i<1024; i++)
	  {
  		  page_directory[i] = 2;
	  }
#endif


#if DEBUG_ENABLE
	  Console::putui(shared_size>>20);
	  Console::putui(paging_enabled);
	  Console::puts("\n");
#endif


#if DEBUG_ENABLE
	  Console::putui((unsigned long)page_directory>>12);
	  Console::putui((unsigned long)page_table_first>>12);
	  Console::puts("\n");

#endif

  }



 /* Makes the given page table the current table. This must be done once during
     system startup and whenever the address space is switched (e.g. during
     process switching). */
  void PageTable::load(){

	  current_page_table = this;
	  // write_cr3, read_cr3, write_cr0, and read_cr0 all come from the assembly functions
	  write_cr3((unsigned long)page_directory); // put that page directory address into CR3

#if DEBUG_ENABLE

	  Console::putui(shared_size>>20);	  Console::puts("\t = shared size in MB \n");
	  Console::putui(paging_enabled);
	  Console::puts("\n");

#endif

  }


  /* Enable paging on the CPU. Typically, a CPU start with paging disabled, and
     memory is accessed by addressing physical memory directly. After paging is
     enabled, memory is addressed logically. */
  void PageTable::enable_paging(){

	  write_cr0(read_cr0() | 0x80000000); // set the paging bit in CR0 to 1
	  paging_enabled = TRUE;
  }

  /* The page fault handler. */
  void PageTable::handle_fault(REGS * _r){
	  //  Console::putui();
	  /* Since it Has the Fault Handle Address */
	  unsigned long fault_address = read_cr2();

	  /*
	  Bit 0 (P) is the Present flag.
	  Bit 1 (R/W) is the Read/Write flag.
	  Bit 2 (U/S) is the User/Supervisor flag.*/


	  //TODO: Optimize
#if DEBUG_ENABLE
	  Console::putui(fault_address);
#endif
	  unsigned int dir_index = fault_address>>22 ;
	  unsigned int table_index = ((fault_address>>12) & 0x3FF);
	  unsigned long *page_dir = (unsigned long *)read_cr3();
	  unsigned long *page_table = NULL ;
	  unsigned long *recursive_page_table = NULL;

	  unsigned int error = (_r->err_code &7);

	  if(!(error&1)) /* Page Fault Occured due to No Memory Miss for the Virtual Page */
	  {

#if DEBUG_ENABLE
	  Console::putui(dir_index);
	  Console::putui(table_index);
#endif
	  /* It means that the entry is not present */
	  if((page_dir[dir_index]&1) == 0)
	  {
		  page_table = (unsigned long *)(process_mem_pool->get_frame()<<12);
		  if(page_table ==0)
		  {
			  Console::puts("No More Physical Frames Available for mapping\n");
		  }
		  /* Supervisor level, read/write , present
		   * Because the directory is maintained by Super user */
		  page_dir[dir_index] = (unsigned long)page_table|3;
		  recursive_page_table = (unsigned long *)(0xFFC00000 +(dir_index<<12));
		 // memset(recursive_page_table,0,(1<<12));

		  /* User access level, read/write, not present */
		  for(int i = 0; i<ENTRIES_PER_PAGE; i++)
		  {
			  recursive_page_table[i] = 2;
		  }
#if DEBUG_ENABLE
		  Console::puts("NDE"); /* No-New Directory Entry */
#endif

	  }

	  page_table = (unsigned long *)((page_dir[dir_index]>>12)<<12);
	  recursive_page_table = (unsigned long *)(0xFFC00000 +(dir_index<<12));
	  /* Page Table Exists Now */
	  if((recursive_page_table[table_index]&1)== 0)/* If inner page doesnt hold anything */
	  {
		  unsigned long *new_pte_frame = (unsigned long *)(process_mem_pool->get_frame()<<12);
		  //memset(PTE_address,0,(1<<12));
		  if(new_pte_frame ==0)
		  {
			  Console::puts("No More Physical Frames Available for mapping\n");
		  }
		  //TODO: What is the user access 1 or 3?
		  /* User access level, read/write, present */

		  recursive_page_table[table_index] = (unsigned long)new_pte_frame|1;

#if DEBUG_ENABLE
		  Console::puts("NTE"); /* New Table Entry */
#endif
	  }
#if DEBUG_ENABLE

	  else
	  {
		  /* This Will be Optimized Out */

		  /*Console::putui(dir_index);
			  Console::putui(table_index);
			  Console::putui((unsigned long)page_dir[dir_index]);
			  Console::putui((unsigned long)page_table);*/

		  Console::puts("NmmF");  /* Not a memory miss fault */
		  Console::putui(recursive_page_table[table_index]);
	  }
#endif


	  }
	  else
	  {

		  /*
		  US RW  P - Description
		  0  0  0 - Supervisory process tried to read a non-present page entry
		  0  0  1 - Supervisory process tried to read a page and caused a protection fault
		  0  1  0 - Supervisory process tried to write to a non-present page entry
		  0  1  1 - Supervisory process tried to write a page and caused a protection fault
		  1  0  0 - User process tried to read a non-present page entry
		  1  0  1 - User process tried to read a page and caused a protection fault
		  1  1  0 - User process tried to write to a non-present page entry
		  1  1  1 - User process tried to write a page and caused a protection fault*/

		  switch (_r->err_code) {
		  case 1:
			  Console::puts("Supervisory process tried to read a page and caused a protection fault\n");
			  break;
		  case 3:
			  Console::puts("Supervisory process tried to write a page and caused a protection fault\n");

			  break;
		  case 5:
			  Console::puts("User process tried to read a page and caused a protection fault\n");

			  break;
		  case 7:
			  Console::puts("User process tried to write a page and caused a protection fault\n");

			  break;
		  default:
			  break;
		  }
	  }



  }



