/* 
    Author: Tharun Battula
			Based on code by:
			R. Bettati
            Department of Computer Science
            Texas A&M University
			
			A thread scheduler.

*/
/*--------------------------------------------------------------------------*/
/* DEFINES 
/*--------------------------------------------------------------------------*/
#define DEBUG_ENABLE 0
/*--------------------------------------------------------------------------*/
/* INCLUDES 
/*--------------------------------------------------------------------------*/

/* Can be emabled based on necessity */
#include "assert.H"
#include "utils.H"
#include "console.H"
//#include "frame_pool.H"
//#include "threads_low.H"
#include "thread.H"
#include "Scheduler.H"
/*--------------------------------------------------------------------------*/
/* SCHEDULER
/*--------------------------------------------------------------------------*/


/* The scheduler may need private members... */


   Scheduler::Scheduler(){
	   thread_ready_queue = new Thread*[MAX_NUM_THREADS];
	   queue_length = 0 ;
	   head_of_queue = 0 ;

   }
   /* Setup the scheduler. This sets up the ready queue, for example.
      If the scheduler implements some sort of round-robin scheme, then the 
      end_of_quantum handler is installed here as well. */

   void Scheduler::yield()
   {
	   if(queue_length)
	   {
		   int rnext_thread= head_of_queue;
		   head_of_queue++; /* remove head_of_queue and move pointer*/
		   if(head_of_queue >=MAX_NUM_THREADS) /* Loop Unrolling */
		   {
			   head_of_queue-=MAX_NUM_THREADS;
		   }
		   queue_length--;
#if DEBUG_ENABLE
		   Console::puts("head_of_queue T@\t");
		   Console::putui(head_of_queue);
		   Console::puts("\tRunning\n");
#endif
		   Thread::dispatch_to(thread_ready_queue[rnext_thread]);

	   }
	   else
	   {
		   Console::puts("No threads in ready queue\n");
	   }

	   /* Enabling interrupts to make thread switching proper and scheduler process atomicity */
	   if(not(Machine::interrupts_enabled()))
		   Machine::enable_interrupts();
	   return;
   }
   /* Called by the currently running thread in order to give up the CPU. 
      The scheduler selects the next thread from the ready queue to load onto 
      the CPU, and calls the dispatcher function defined in 'threads.h' to
      do the context switch. */

   void Scheduler::resume(Thread * _thread)
   {
	   /* Disabling interrupts to avoid thread switching and scheduler process atomicity */
	   if(Machine::interrupts_enabled())
		   Machine::disable_interrupts();

	   /*  Enqueue , loop rolling for the queue*/
	   thread_ready_queue[(head_of_queue+queue_length)%MAX_NUM_THREADS] = _thread;
	   queue_length++;

#if DEBUG_ENABLE
	   Console::puts("Re added\t");
	   Console::putui((head_of_queue+queue_length)%MAX_NUM_THREADS);
	   Console::puts("\t");
#endif


   }
   /* Add the given thread to the ready queue of the scheduler. This is called
      for threads that were waiting for an event to happen, or that have 
      to give up the CPU in response to a preemption. */

   void Scheduler::add(Thread * _thread)
   {
	   if(queue_length >= MAX_NUM_THREADS) /* CHeck on array size*/
	   {
		   Console::puts("Exceeded Max number of threads(128)\n");
	   }
	   /*  Enqueue , loop rolling for the queue*/
	   thread_ready_queue[(head_of_queue+queue_length)%MAX_NUM_THREADS] = _thread;
	   queue_length++;

#if DEBUG_ENABLE
	   Console::puts("Added \t");
	   Console::putui((head_of_queue+queue_length)%MAX_NUM_THREADS);
	   Console::puts("\t");
#endif

   }
   /* Make the given thread runnable by the scheduler. This function is called
	  typically after thread creation. Depending on the
      implementation, this may not entail more than simply adding the 
      thread to the ready queue (see scheduler_resume). */

   void Scheduler::terminate(Thread * _thread){

	   delete _thread->get_stack();  /* delete stack created by Kernel.C*/
	   delete _thread; /* deleete the thread itself */
	   this->yield(); /* Dequeue and Dispatch */

   }
   /* Remove the given thread from the scheduler in preparation for destruction
      of the thread. */

	
