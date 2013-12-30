//
//	Last Modified: $Date: 2010-07-22 10:16:38 $
//
//	$Log: Thread.cpp,v $
//	Revision 1.1.2.1  2010-07-22 10:16:38  lgrave
//	RT8: TapiAdapter: threads for reception/execution of paralel commands
//
//
//


/* Written by Jae An   (jae_young_an@yahoo.com)
 */

#include <stdio.h>
#include <stdlib.h>
//#include <pthread.h>
#include "Thread.h"
#include "..\src\Log.h"

#define TS(x)	theLog->Debug(format("Thread %ld: %s", GetCurrentThreadId(), #x))

void executeThread(Thread * threadPtr)
{
	xBEGIN
	TS(executeThread);
	threadPtr->run();
	xEND
}

void Thread::start()
{
	xBEGIN
	TS(start);
    //int rs;
    DWORD  threadId;
	// TODO
    //rs = pthread_create(&thread_t,NULL,(void *(*)(void *))executeThread,(void*)this);
_x
	ASS(thread_t == 0)
    thread_t = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)executeThread, this, 0, &threadId);
    ASS(thread_t != 0)
	xEND
}

void Thread::wait()
{
	xBEGIN
	TS(wait);
    //int rs;
    DWORD rs;
	// TODO
    //rs = pthread_join(thread_t,NULL);
_x
	ASS(thread_t != 0)
    rs = WaitForSingleObject(thread_t, INFINITE);
	xEND
}

Thread::~Thread()
{
	xBEGIN
	TS(~Thread);
	CloseHandle(thread_t);
_x
	thread_t = NULL;
	xEND
}

// TODO
//struct pthread_mutex mutex_init = PTHREAD_MUTEX_INITIALIZER;
//HANDLE mutex_init = NULL;

Mutex::Mutex()
{
	xBEGIN
	TS(Mutex);
    //mutex_t = mutex_init;
    InitializeCriticalSection (& _critSection);
    //mutex_t = CreateMutex(NULL, FALSE, NULL);
    //ASS(mutex_t != 0)
	xEND
}

Mutex::~Mutex()
{
	xBEGIN
	TS(~Mutex);
	DeleteCriticalSection (& _critSection);
//	CloseHandle(mutex_t);
//	mutex_t = NULL;
	xEND
}

void Mutex::lock() // Enter critical region
{
	xBEGIN
	TS(lock);
    //pthread_mutex_lock(&mutex_t);
    EnterCriticalSection (& _critSection);
//     ASS(mutex_t != 0)
//     switch(WaitForSingleObject(mutex_t, 2000))   //INFINITE
//     {
//     	case WAIT_ABANDONED: theLog->Debug("WAIT_ABANDONED"); break;
//     	case WAIT_OBJECT_0: theLog->Debug("WAIT_OBJECT_0"); break;
//     	case WAIT_TIMEOUT: theLog->Debug("WAIT_TIMEOUT"); break;
//     	case WAIT_FAILED: theLog->Debug(format("WAIT_FAILED %ld", GetLastError())); break;
//     }
	xEND
}

void Mutex::unlock() // Exit the critical region
{
	xBEGIN
	TS(unlock);
    //pthread_mutex_unlock(&mutex_t);
    LeaveCriticalSection (& _critSection);
//     ASS(mutex_t != 0)
//     ReleaseMutex(mutex_t);
	xEND
}
