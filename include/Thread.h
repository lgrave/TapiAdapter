//
//	Last Modified: $Date: 2010-07-22 10:16:37 $
//
//	$Log: Thread.h,v $
//	Revision 1.1.2.1  2010-07-22 10:16:37  lgrave
//	RT8: TapiAdapter: threads for reception/execution of paralel commands
//
//
//


/* Written by Jae An   (jae_young_an@yahoo.com)

 */

#ifndef _THREAD_H
#define _THREAD_H

//#include <pthread.h>
#include "safewindows.h"

class Thread
{
public:
	Thread(): thread_t(NULL) {};
	~Thread();
	void start(); // Start the thread of this object
	void wait(); // Wait until this thread is terminated. When already terminated, return immediately
	virtual void run()=0;
private:
	//pthread_t thread_t; 
	HANDLE thread_t;
};

class Mutex
{
public:
	Mutex();
	~Mutex();
	void lock(); // Enter critical region
	void unlock(); // Exit the critical region
private:
	//struct pthread_mutex mutex_t;    
	HANDLE mutex_t;
	CRITICAL_SECTION _critSection;
};

#endif
