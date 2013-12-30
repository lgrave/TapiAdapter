//
//	Last Modified: $Date: 2010-07-22 10:16:38 $
//
//	$Log: MultithreadXmlRpcServer.cpp,v $
//	Revision 1.1.2.1  2010-07-22 10:16:38  lgrave
//	RT8: TapiAdapter: threads for reception/execution of paralel commands
//
//
//

/* Written by Jae An   (jae_young_an@yahoo.com)
 */

#include <vector>

#include "XmlRpcServer.h"

#include "XmlRpcServerConnection.h"
#include "XmlRpcServerMethod.h"
#include "XmlRpcSocket.h"
#include "XmlRpcUtil.h"
#include "XmlRpcException.h"
#include "MultithreadXmlRpcServer.h"
#include "..\src\Log.h"



using namespace XmlRpc;




// call this method before calling run

void WorkerThread::addXmlRpcSource(XmlRpcSource* source,unsigned eventMask)

{
	xBEGIN
    XmlRpcUtil::log(2, "WorkerThread::AddXmlRpcSource: adding source fd=%d",source->getfd());
_x

    dispatcher.addSource(source,eventMask);
	xEND
} 



void WorkerThread::run()

{
	xBEGIN

    //running = true;
	setRunning(true);
    

_x
    XmlRpcUtil::log(2, "WorkerThread::run: calling dispather.work");

_x
    dispatcher.work(-1.0);

    

_x
    XmlRpcUtil::log(2, "WorkerThread::run: calling dispather.clear");

_x
    dispatcher.clear(); // close socket and others ...   

    

_x
    //running = false;
    setRunning(false);
	xEND

}

MultithreadXmlRpcServer::MultithreadXmlRpcServer(int n_threads):XmlRpcServer(), MAX_THREAD_SIZE(n_threads)
{
	xBEGIN
	//MAX_THREAD_SIZE = IniReadInt(TA_SETTINGS_COMMANDS, TA_SETTINGS_COMMANDS_THREADS);
	threads.resize(MAX_THREAD_SIZE);
	xEND
}



// Wait until all the worker threads are done.

MultithreadXmlRpcServer::~MultithreadXmlRpcServer()

{
	xBEGIN

    for (int i=0;i<MAX_THREAD_SIZE;i++) 

    {

_x
        if (threads[i].isRunning()) threads[i].wait(); 

    }

	xEND
}







// Accept a client connection request and create a connection to

// handle method calls from the client.

void MultithreadXmlRpcServer::acceptConnection()

{
	xBEGIN

  int s = XmlRpcSocket::accept(this->getfd());

_x
  XmlRpcUtil::log(2, "MultithreadXmlRpcServer::acceptConnection: socket %d", s);

_x
  if (s < 0)

  {

    //this->close();

_x
    XmlRpcUtil::error("MultithreadXmlRpcServer::acceptConnection: Could not accept connection (%s).", XmlRpcSocket::getErrorMsg().c_str());
_x

  }

  else  // Notify the dispatcher to listen for input on this source when we are in work()

  {

_x
    XmlRpcUtil::log(2, "MultithreadXmlRpcServer::acceptConnection: creating a connection");

    

    // Find out any vacant slot
_x

    for (int i=0; i < MAX_THREAD_SIZE; i++) 

    {

_x
		if (!threads[i].isRunning()) 

        {

_x
			threads[i].lock();
_x
            threads[i].addXmlRpcSource(this->createConnection(s), XmlRpcDispatch::ReadableEvent);

_x
            XmlRpcUtil::log(2, "MultithreadXmlRpcServer::acceptConnection: using thread %d",i);

_x
            threads[i].start(); // Let the thread run!

_x
			threads[i].unlock();
_x
            return; // break out

        }
//		threads[i].unlock();
    }



    // Uh oh.. all threads are busy... 

    // Just close the connection so that the rejected client would try again.

_x
    XmlRpcSocket::close(s);

_x
    XmlRpcUtil::error("MultithreadXmlRpcServer::acceptConnection: All threads are busy. Rejected a client");
_x

  }
	xEND

}
