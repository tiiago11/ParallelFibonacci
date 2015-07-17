/*
Fibonacci algorithm - a parallel multithreaded approach.
Author: Tiago Augusto Engel (tengel@inf.ufsm.br)
Refer to LICENSE.md
*/

#pragma once

//#define DEBUG
/*
	Generic RingBuffer class. Non-blocking, not thread-safe;
	*/
template<class T> class RingBuffer
{
public:
	RingBuffer(int length);
	T get() volatile; /// = RemoveFromRing
	bool add(T item) volatile;
	bool isEmpty() volatile;
	bool isFull() volatile;
	~RingBuffer();
private:
	volatile int head;
	volatile int tail;
	volatile int length;
	volatile T* ring;
	volatile unsigned int occupance;

	unsigned int nextPosition(unsigned int pos) volatile;
};

/*
	by default, the buffer length is 256
	*/
template<class T>
RingBuffer<T>::RingBuffer(int length = 256)
{
	this->length = length;
	this->head = this->tail = this->occupance = 0;
	this->ring = new T[length];
}

template<class T>
RingBuffer<T>::~RingBuffer(){
	delete ring;
}

/*
	Returns the current element (equivalent to RemoveFromRing)
	*/
template<class T>
T RingBuffer<T>::get() volatile {
	if (!isEmpty())
	{
		T aux = ring[tail];
		ring[tail] = 0;
		tail = nextPosition(tail);
		this->occupance--;
#ifdef DEBUG
		printf(" %ld removed from ", aux);
		printf(" %d \n", tail);
#endif // DEBUG
		return aux;
	}
	return 0;
}

/*
	Add a new item to the queue. If the ring is full, then return false;
	*/
template<class T>
bool RingBuffer<T>::add(T val) volatile {
	if (!isFull())
	{
		ring[head] = val;
#ifdef DEBUG
		printf(" %d added in ", val);
		printf(" %d \n", head);
#endif // DEBUG
		head = nextPosition(head);
		this->occupance++;
		return true;
	}
	return false;
}

/*
	whether the ring is full
	*/
template<class T>
bool RingBuffer<T>::isFull() volatile {
	return (this->occupance == this->length);
}

/*
	whether the ring is empty
	*/
template<class T>
bool RingBuffer<T>::isEmpty() volatile {
	return (this->occupance == 0);
}

/*
	calculates the element after pos in the ring
	*/
template<class T>
unsigned int RingBuffer<T>::nextPosition(unsigned int pos) volatile {
	if (pos == (this->length - 1))
	{
		return 0;
	}
	return (pos + 1);
}