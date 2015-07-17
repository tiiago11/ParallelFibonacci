/*
Fibonacci algorithm - a parallel multithreaded approach.
Author: Tiago Augusto Engel (tengel@inf.ufsm.br)
Refer to LICENSE.md
*/

#pragma once
#include "Chrono.h"
#include <process.h>
#include <time.h>

//#define DEBUG // add debug tracing
/*
Generic AtomicRingBuffer class. thread-safe;
*/
template<class T> class AtomicRingBuffer
{
public:
	AtomicRingBuffer(int length);
	T get() volatile;
	bool add(T item) volatile;
	bool isEmpty() volatile;
	bool isFull() volatile;
	~AtomicRingBuffer();
private:
	volatile int head;
	volatile int tail;
	volatile int length;
	volatile T* ring;
	volatile unsigned int occupance;
	HANDLE mutex;
	unsigned int nextPosition(unsigned int pos) volatile;
	// as they're gonna be called from inside the critical zone, we don't need to lock these functions
	bool _nonBlock_isFull() volatile;
	bool _nonBlock_isEmpty() volatile;
};

/*
	Constructor
	by default, the buffer length is 256
	*/
template<class T>
AtomicRingBuffer<T>::AtomicRingBuffer(int length = 256)
{
	this->length = length;
	this->head = this->tail = this->occupance = 0;
	this->ring = new T[length];
	mutex = CreateMutex(NULL, FALSE, NULL);
}
/*
	Destructor: release the data
	*/
template<class T>
AtomicRingBuffer<T>::~AtomicRingBuffer(){
	delete ring;
	CloseHandle(mutex);
}

/*
	Returns the current element (equivalent to RemoveFromRing)
	*/
template<class T>
T AtomicRingBuffer<T>::get() volatile {
	WaitForSingleObject(mutex, INFINITE);
	if (!_nonBlock_isEmpty())
	{
		T aux = ring[tail];
		ring[tail] = 0;
		tail = nextPosition(tail);
		this->occupance--;
#ifdef DEBUG
		printf(" %ld removed from ", aux);
		printf(" %d \n", tail);
#endif // DEBUG
		ReleaseMutex(mutex);
		return aux;
	}
	ReleaseMutex(mutex);
	return 0;
}

/*
	Add a new item to the queue. If the ring is full, then return false;
	*/
template<class T>
bool AtomicRingBuffer<T>::add(T val) volatile {
	WaitForSingleObject(mutex, INFINITE);
	if (!_nonBlock_isFull())
	{
		ring[head] = val;
#ifdef DEBUG
		printf(" %d added in ", val);
		printf(" %d \n", head);
#endif // DEBUG
		head = nextPosition(head);
		this->occupance++;
		ReleaseMutex(mutex);
		return true;
	}
	ReleaseMutex(mutex);
	return false;
}

/*
	returns whether the ring is full. As this is public, we need to ensure
	*/
template<class T>
bool AtomicRingBuffer<T>::isFull() volatile {
	WaitForSingleObject(mutex, INFINITE);
	bool res = (this->occupance == this->length) ? true : false;
	ReleaseMutex(mutex);
	return res;
}
// non-blocking
template<class T>
bool AtomicRingBuffer<T>::_nonBlock_isFull() volatile {
	return (this->occupance == this->length);
}
/*
	returns whether the ring is empty
	*/
template<class T>
bool AtomicRingBuffer<T>::isEmpty() volatile {
	WaitForSingleObject(mutex, INFINITE);
	bool res = (this->occupance == 0) ? true : false;
	ReleaseMutex(mutex);
	return res;
}

//non blocking
template<class T>
bool AtomicRingBuffer<T>::_nonBlock_isEmpty() volatile {
	return (this->occupance == 0);
}

/*
	calculates the element after pos in the ring
	*/
template<class T>
unsigned int AtomicRingBuffer<T>::nextPosition(unsigned int pos) volatile {
	if (pos == (this->length - 1))
	{
		return 0;
	}
	return (pos + 1);
}