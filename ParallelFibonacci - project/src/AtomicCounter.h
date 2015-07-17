/*
Fibonacci algorithm - a parallel multithreaded approach.
Author: Tiago Augusto Engel (tengel@inf.ufsm.br)
Refer to LICENSE.md
*/

#include "Chrono.h"
#include <process.h>
#include <time.h>

/*
	Implements a simple atomic counter
	*/
class AtomicCounter {

public:
	AtomicCounter();
	~AtomicCounter();
	int get() volatile;
	void increment() volatile;
	void decrement() volatile;
private:
	volatile int counter;
	HANDLE mutex;
};

AtomicCounter::AtomicCounter(){
	mutex = CreateMutex(NULL, FALSE, NULL);
	counter = 0;
}

AtomicCounter::~AtomicCounter(){
	CloseHandle(mutex);
}

void AtomicCounter::increment() volatile {
	WaitForSingleObject(mutex, INFINITE);
	counter++;
	ReleaseMutex(mutex);
}

void AtomicCounter::decrement() volatile {
	WaitForSingleObject(mutex, INFINITE);
	counter--;
	ReleaseMutex(mutex);
}
// read is non-blocking
int AtomicCounter::get() volatile {
	return counter;
}