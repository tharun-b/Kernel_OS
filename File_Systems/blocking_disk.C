/*
 File        : blocking_disk.c

 Author      : Tharun Battula
 Modified    : 30/04/2016

 Description : Blockind Disk as per required assignment instructions
 This related SimpleDisk.C functionality

 */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/
#include "assert.H"
#include "utils.H"
#include "console.H"
#include "blocking_disk.H"
#include "Scheduler.H"
#include "thread.H"

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/
#define DEBUG_FLAG 0
/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/
/* Extern definition of the pointer to the Scheduler Class to call the functions */
extern Scheduler *SYSTEM_SCHEDULER;

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

BlockingDisk::BlockingDisk(DISK_ID _disk_id, unsigned int _size) :
				SimpleDisk(_disk_id, _size), disk_id(_disk_id) {
	lock = 0;
	current_status.block_no = 0;
	current_status.thread = 0;
	current_status.operation = READ;
}

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

/* ISSUE OPERATION of blocking disk */
void BlockingDisk::issue_operation(DISK_OPERATION _op,
		unsigned long _block_no) {

	outportb(0x1F1, 0x00); /* send NULL to port 0x1F1         */
	outportb(0x1F2, 0x01); /* send sector count to port 0X1F2 */
	outportb(0x1F3, (unsigned char) _block_no);
	/* send low 8 bits of block number */
	outportb(0x1F4, (unsigned char) (_block_no >> 8));
	/* send next 8 bits of block number */
	outportb(0x1F5, (unsigned char) (_block_no >> 16));
	/* send next 8 bits of block number */
	outportb(0x1F6,
			((unsigned char) (_block_no >> 24) & 0x0F) | 0xE0 | (disk_id << 4));
	/* send drive indicator, some bits,
	 highest 4 bits of block no */

	outportb(0x1F7, (_op == READ) ? 0x20 : 0x30);
	current_status.block_no = _block_no;
	current_status.operation = _op;
}

void BlockingDisk::read(unsigned long _block_no, unsigned char * _buf) {
	/* Reads 512 Bytes in the given block of the given disk drive and copies them
	 to the given buffer. No error check! */

	//Check if the current disk is performing an operation
	/* operating -> enter for que
	 *  or there is queue -> enter for que
	 */

#if DEBUG_FLAG
	Console::puts("\n BLOCK NUMBER = ");
	Console::putui(_block_no);

	Console::puts("\n");

#endif

	if ((is_op_disk()!=0)/*|| !disk_ready_Que.isEmpty()*/)
	{
#if	DEBUG_FLAG
		Console::puts("\nEntering Queue\n");
#endif
		struct disk_operation new_op;
		new_op.operation = READ;
		new_op.block_no = _block_no;
		new_op.thread = Thread::CurrentThread();

		if(machine_interrupts_enabled())
		{
			machine_disable_interrupts();

		}
		disk_ready_Que.enqueue(&new_op);

		if(!machine_interrupts_enabled())
		{
			machine_enable_interrupts();
		}
		//Console::puts("\nThread and op is queue");
		//  for(;;);
		SYSTEM_SCHEDULER->resume(Thread::CurrentThread());
		SYSTEM_SCHEDULER->yield();
	} else
	{
		if (disk_ready_Que.isEmpty())
		{
			if(machine_interrupts_enabled())
			{
				machine_disable_interrupts();
			}
			set_op_disk(Thread::CurrentThread());
			issue_operation(READ, _block_no);

			if(!machine_interrupts_enabled())
			{
				machine_enable_interrupts();
			}

			//  Console::puts("\nThread is queue");
			// Enqueue the current thread in the device_queue while the disk move its head to the required sector
		} else
		{
			Console::puts("\nError in queue setup");
			assert(false);
		}
	}
	while (1) {
#if DEBUG_FLAG
		Console::puts("Control lock = \t");
		Console::putui(current_status.block_no);
		Console::puts("\t");
		Console::putui(_block_no);
		Console::puts("\t");
		Console::putui((unsigned int)lock);
		Console::puts("\t");
		Console::putui((unsigned int)Thread::CurrentThread());
		Console::puts("\n");

#endif
		if ((lock == (unsigned int)Thread::CurrentThread()))
		{
			wait_until_ready();

			if(machine_interrupts_enabled())
			{
				machine_disable_interrupts();

			}

#if DEBUG_FLAG
		Console::puts("Read lock = \t");
		Console::putui(current_status.block_no);
		Console::puts("\t");
		Console::putui(_block_no);
		Console::puts("\t");
		Console::putui((unsigned int)lock);
		Console::puts("\t");
		Console::putui((unsigned int)Thread::CurrentThread());
		Console::puts("\n");

#endif
			{ /* Reading The information */
				int i;
				unsigned short tmpw;
				for (i = 0; i < 256; i++) {
					tmpw = inportw(0x1F0);
					_buf[i * 2] = (unsigned char) tmpw;
					_buf[i * 2 + 1] = (unsigned char) (tmpw >> 8);
				}
			}
			reset_op_disk(); /* Setting Op to false */

			if(!machine_interrupts_enabled())
			{
				machine_enable_interrupts();
			}
#if DEBUG_FLAG
			Console::puts("Read Done\n");
#endif
			choose_next_disk_op();
			return;
		}
		else
		{
			if(lock ==0)
			{
				choose_next_disk_op();
			}
			else
			{
				SYSTEM_SCHEDULER->resume(Thread::CurrentThread());
				SYSTEM_SCHEDULER->yield();
			}

		}
	}

}

void BlockingDisk::write(unsigned long _block_no, unsigned char * _buf) {
	/* Writes 512 Bytes from the buffer to the given block on the given disk drive. */

	//Check if the current disk is performing an operation
	// If the queue is not empty =>  threads waiting
	if ((is_op_disk()!=0) /*|| !disk_ready_Que.isEmpty()*/) {
#if DEBUG_FLAG
		Console::puts("\nEntering Queue for write\n");
#endif
		disk_op quing_op;
		quing_op.operation = WRITE;
		quing_op.block_no = _block_no;
		quing_op.thread = (Thread::CurrentThread());

		if(machine_interrupts_enabled())
		{
			machine_disable_interrupts();
		}
		disk_ready_Que.enqueue(&quing_op);
		if(!machine_interrupts_enabled())
		{
			machine_enable_interrupts();
		}


		SYSTEM_SCHEDULER->resume(Thread::CurrentThread());
		SYSTEM_SCHEDULER->yield();
	} else {  /* If free Issue */
		if (disk_ready_Que.isEmpty()) {
			if(machine_interrupts_enabled())
			{
				machine_disable_interrupts();

			}
			set_op_disk(Thread::CurrentThread());
			issue_operation(WRITE, _block_no);
			if(!machine_interrupts_enabled())
			{
				machine_enable_interrupts();
			}
		} else { /* Some thing Wron */
			Console::puts("\nError in queue setup\n");
			assert(false);
		}
	}

	while (1) {
#if DEBUG_FLAG
		Console::puts("Control lock  \t");
		Console::putui(current_status.block_no);
		Console::puts("\t");
		Console::putui(_block_no);
		Console::puts("\t");
		Console::putui((unsigned int)lock);
		Console::puts("\t");
		Console::putui((unsigned int)Thread::CurrentThread());
		Console::puts("\n");
#endif
		if ((lock == (unsigned int)Thread::CurrentThread()))
		{
			wait_until_ready();
			/* write data to port */
			if(machine_interrupts_enabled())
			{
				machine_disable_interrupts();

			}
			{
#if DEBUG_FLAG
				Console::puts("write lock = \t");
				Console::putui(current_status.block_no);
				Console::puts("\t");
				Console::putui(_block_no);
				Console::puts("\t");
				Console::putui((unsigned int)lock);
				Console::puts("\t");
				Console::putui((unsigned int)Thread::CurrentThread());
				Console::puts("\n");

#endif
				int i;
				unsigned short tmpw;
				for (i = 0; i < 256; i++) {
					tmpw = _buf[2 * i] | (_buf[2 * i + 1] << 8);
					outportw(0x1F0, tmpw);
				}
			}
			reset_op_disk();
			if(!machine_interrupts_enabled())
			{
				machine_enable_interrupts();
			};

			choose_next_disk_op();

#if DEBUG_FLAG
			Console::puts(" Write Done\n");
#endif
			return;
		}
		else
		{
			if(lock == 0)
			{
				choose_next_disk_op();
			}
			else
			{
				SYSTEM_SCHEDULER->resume(Thread::CurrentThread());
				SYSTEM_SCHEDULER->yield();
			}

		}
	}
}

void BlockingDisk::choose_next_disk_op() {

	if(!disk_ready_Que.isEmpty())
	{
		if(machine_interrupts_enabled())
		{
			machine_disable_interrupts();

		}

		disk_op *op = disk_ready_Que.dequeue();
		set_op_disk(op->thread);
		issue_operation(op->operation, op->block_no);

		if(!machine_interrupts_enabled())
		{
			machine_enable_interrupts();
		}

#if DEBUG_FLAG
		Console::puts("Next Guy ->\t");
		Console::putui((unsigned int)op->thread);
		Console::puts("\n");
#endif
	}
}

void BlockingDisk::set_op_disk(Thread *t) {
	lock = ((unsigned int)t);
}

int BlockingDisk::is_op_disk() {
	return lock;
}

void BlockingDisk::reset_op_disk() {
	lock = 0;
}

void BlockingDisk::wait_until_ready() {

	{
		while (!is_ready()) {
#if DEBUG_FLAG
			Console::puts("=> ");
#endif
			SYSTEM_SCHEDULER->resume(Thread::CurrentThread());
			SYSTEM_SCHEDULER->yield();
		}
#if DEBUG_FLAG
		Console::puts("...........");
#endif
	};
}
