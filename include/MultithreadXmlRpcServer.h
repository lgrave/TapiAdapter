//
//	Last Modified: $Date: 2010-07-22 10:16:36 $
//
//	$Log: MultithreadXmlRpcServer.h,v $
//	Revision 1.1.2.1  2010-07-22 10:16:36  lgrave
//	RT8: TapiAdapter: threads for reception/execution of paralel commands
//
//
//


/* Written by Jae An   (jae_young_an@yahoo.com)

 */

#ifndef _MULTI_THREAD_XMLRPCSERVER_H_
#define _MULTI_THREAD_XMLRPCSERVER_H_




#include <vector>
#include <Thread.h>

#include "XmlRpcServer.h"

#include "XmlRpcDispatch.h"



namespace XmlRpc {

    

  class WorkerThread : public Thread{

  public:

    WorkerThread() //:running(false)
    {
    	setRunning(false);
    }

    void addXmlRpcSource(XmlRpcSource* source,unsigned eventMask); // call this method before calling run

    virtual void run();

    bool isRunning()
    {
//    	mutex.lock();	
    	bool ret = running;
//    	mutex.unlock();	
    	return ret;
    }
    void setRunning(bool value)
    {
    	mutex.lock();	
    	running = value;
    	mutex.unlock();	
	}
	void lock() { mutex.lock(); }
	void unlock() { mutex.unlock(); }

  private:  

    volatile bool running;  
    Mutex mutex;

    XmlRpcDispatch dispatcher;  

  };









  //! Multi-threaded sever class to handle XML RPC requests

  class MultithreadXmlRpcServer : public XmlRpcServer {

	int MAX_THREAD_SIZE;    
	
  public:

    //! Create a server object.

    MultithreadXmlRpcServer(int n_threads);

    //! Destructor.

    virtual ~MultithreadXmlRpcServer();





  protected:



    //! Accept a client connection request

    virtual void acceptConnection();





    std::vector<WorkerThread> threads;

  };

  

} // namespace XmlRpc



#endif 
