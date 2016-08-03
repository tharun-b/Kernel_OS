/*
 File: queue.C

 Author: Tharun Battula
 Department of ECE
 Texas A&M University
 Date  : 4/15/2016

 This code is for generic implementation of Queue Data structure
 to be initialized on different data types, So it is  of type Template data structure

 This is developed in order to use it for future for furthur usage for different
 types of prjects inolving queue.

 The implementation is made based on using an array of maximum size of 100 queue elements
 Then based on the queue requests it loops along the array

 */
#include "queue.H"

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

template<typename T>
Queue<T>::Queue() :
		head(-1), tail(-1) {
}

template<typename T>
int Queue<T>::isEmpty() {
	return (tail == -1);
}

template<typename T>
int Queue<T>::isFull() {
	return (((tail + 1) % MAX_QUEUE_SIZE) == head);
}

template<typename T>
void Queue<T>::enqueue(T t) {

	if (!this->isFull()) {
		if (this->isEmpty())
			++head;

		tail = (tail + 1) % MAX_QUEUE_SIZE;
		list[tail] = t;
	} else {
		Console::puts("\n ReadyQueue Full!!");
		for (;;)
			;
	}
}

template<typename T>
T Queue<T>::dequeue() {
	if (!this->isEmpty()) {
		T t = list[head];

		if (head == tail) {
			tail = -1;
			head = -1;
		} else
			head = (head + 1) % MAX_QUEUE_SIZE;

		return t;
	} else {
		Console::puts("\nReadyQueue Empty!!");
		return NULL;
	}
}
