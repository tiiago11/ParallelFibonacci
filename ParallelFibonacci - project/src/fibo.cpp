/*
	Fibonacci algorithm - a parallel multithreaded approach.
	Author: Tiago Augusto Engel (tengel@inf.ufsm.br)
	Refer to LICENSE.md
	*/
// #define DEBUG

#include "Chrono.h"
#include <process.h>
#include <time.h>
#include "AtomicCounter.h"
#include "AtomicRingBuffer.h"

#define nbThreads 10
#define ringSize (256)

typedef unsigned long long ull;

int notEnd = 1;
const ull minimumJobSize = 28;

AtomicCounter runningJobsCounter;
AtomicRingBuffer<ull> ringBuffer(ringSize);

struct ThreadArgs
{
	int id;
	ull returnValue;
}threadArguments[nbThreads];

/*
	sequential fibonacci
	*/
ull fibo(ull n)
{
	if (n <= 1ull)
		return n;
	else
		return
		fibo(n - 1ull) + fibo(n - 2ull);
}

/*
	Retrieve one job value from pool
	*/
ull GetJobFromPool()
{
	ull result;
	result = ringBuffer.get();
	if (result == 0){
		if (runningJobsCounter.get() == 0) // queue is empty and no jobs running
			notEnd = 0; // finish
	}
	else{
		runningJobsCounter.increment();
	}
	return result;
}

/*
	add one job value to pool
	*/
bool AddJobToPool(ull job)
{
	return (ringBuffer.add(job));
}

/*
	Ideally, the ring methods would be encapsulated inside a class, and would be accessed by
	public methods. The concurrency would be implemented then inside this class, and wouldn't be visible from
	outside.
	-PROBLEM
	However, there are some problems that must be dealt in order to have a full encapsulated implementation.
	In the current implementation, the runningJobs variable is been manipulated inside the critical zones, if we encapsultate the ring
	functionality then there must be created some mechanism to control the access to runningJobs. Otherwise,
	there is a great risk of some racing condition as the variable is been manipulated in
	several processes. Furthermore, this variable is critical for the program, as it holds the counter of the number of threads
	running.
	-SOLUTION
	I have made the encapsulation of the ring functionalities (see AtomicRingBuffer.h) and now the class is fully encapsulated and
	generic (can hold any data type).
	To solve the counter problem, I created a simple AtomicCounter class that controls the access to the variable, thus encapsulating
	the atomic functionalities.
	*/

void JobCompleted()
{
	runningJobsCounter.decrement();
}

void ThreadI(void *arg)
{
	int id = ((ThreadArgs *)arg)->id;
	((ThreadArgs *)arg)->returnValue = 0ull;
	while (notEnd)
	{
		ull job = GetJobFromPool();
		if (job > 0) //A job has been retrieved from pool. There cannot exist such a job (0) in the pool by definition.
		{
			if ((job < minimumJobSize)) //Job is too small. Synchronizing may add too much overhead
				((ThreadArgs *)arg)->returnValue += fibo(job);
			else //Job is too large. add more jobs
			{
				if (!AddJobToPool(job - 1))
					((ThreadArgs *)arg)->returnValue += fibo(job - 1);
				if (!AddJobToPool(job - 2))
					((ThreadArgs *)arg)->returnValue += fibo(job - 2);
			}
			JobCompleted();
		}
	}
}

ull fiboMultithreaded(ull n)
{
	ull result = 0ull;
	//initialization of variables does not need protection as one thread only is running at thsi point.
	notEnd = 1;
	AddJobToPool(n);
	for (int i = 0; i < nbThreads; i++)
	{
		threadArguments[i].id = i;
		if (i < nbThreads - 1)
			_beginthread(ThreadI, 0, &threadArguments[i]);
		else
			ThreadI(&threadArguments[i]); //last thread should be the current one!
	}
	for (int i = 0; i < nbThreads; i++)
		result += threadArguments[i].returnValue;
	return result;
}

int main(int argc, char* argv[])
{
	ull val;
	const ull startValue = 46ull;
	Chrono c; //only work for 64 bit version ?
	val = fiboMultithreaded(startValue);
	c.PrintElapsedTime("Fibo time multithreaded (s):");
	printf("Final value %u\n", val);
	c.InitChrono();
	val = fibo(startValue); //funnily enough, the Visual C compiler produce far less optimal code if this line is de-activated
	c.PrintElapsedTime("Fibo time singlethreaded (s):");
	printf("Final value  %u\n", val);

	system("pause");
	return 0;
}

