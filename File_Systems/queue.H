/*
 File: queue.C

 Author: Tharun Battula
 Department of ECE
 Texas A&M University
 Date  : 4/15/2016

 This code is for generic implementation of Queue Data structure
 to be initialized on different data types, So it is  of type Template data structure

 This is developed in order to use it for future for further usage for different
 types of projects involving queue.
 */
#ifndef QUEUE_H
#define QUEUE_H

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/
#include "utils.H"
#include "thread.H"
#include "console.H"
/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

#define MAX_QUEUE_SIZE 100

/*Implementation of Queue for Scheduler/Controller of Ready Status*/

template<typename T>
class Queue {
private:

	int head, tail;
	T list[MAX_QUEUE_SIZE];

public:
	Queue();
	int isEmpty();
	int isFull();
	void enqueue(T th);
	T dequeue();

};
#include "queue.C"
#endif
