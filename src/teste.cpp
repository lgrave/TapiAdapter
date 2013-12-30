#include "safewindows.h"

//#include "Log.h"
//#include "Settings.h"

class Thread
{
public:
    Thread ( DWORD (WINAPI * pFun) (void* arg), void* pArg)
    {
        _handle = CreateThread (
            0, // Security attributes
            0, // Stack size
            pFun,
            pArg,
            CREATE_SUSPENDED,
            &_tid);
    }
    ~Thread () { CloseHandle (_handle); }
    void Resume () { ResumeThread (_handle); }
    void WaitForDeath ()
    {
        WaitForSingleObject (_handle, 2000);
    }
private:
    HANDLE _handle;
    DWORD  _tid;     // thread id
};


class ActiveObject
{
public:
    ActiveObject ();
    virtual ~ActiveObject () {}
    void Kill ();

protected:
    virtual void InitThread () = 0;
    virtual void Run () = 0;
    virtual void FlushThread () = 0;

    static DWORD WINAPI ThreadEntry (void *pArg);

    int             _isDying;
    Thread          _thread;
};

// The constructor of the derived class
// should call
//    _thread.Resume ();
// at the end of construction

ActiveObject::ActiveObject ()
: _isDying (0),
#pragma warning(disable: 4355) // 'this' used before initialized
  _thread (ThreadEntry, this)
#pragma warning(default: 4355)
{
}

void ActiveObject::Kill ()
{
    _isDying++;
    FlushThread ();
    // Let's make sure it's gone
    _thread.WaitForDeath ();
}

DWORD WINAPI ActiveObject::ThreadEntry (void* pArg)
{
    ActiveObject * pActive = (ActiveObject *) pArg;
    pActive->InitThread ();
    pActive->Run ();
    return 0;
}


class Mutex
{
    friend class Lock;
public:
    Mutex () { InitializeCriticalSection (& _critSection); }
    ~Mutex () { DeleteCriticalSection (& _critSection); }
private:
    void Acquire ()
    {
        EnterCriticalSection (& _critSection);
    }
    void Release ()
    {
        LeaveCriticalSection (& _critSection);
    }

    CRITICAL_SECTION _critSection;
};

class Lock
{
public:
    // Acquire the state of the semaphore
    Lock ( Mutex & mutex )
        : _mutex(mutex)
    {
        _mutex.Acquire();
    }
    // Release the state of the semaphore
    ~Lock ()
    {
        _mutex.Release();
    }
private:
    Mutex & _mutex;
};

class Event
{
public:
    Event ()
    {
        // start in non-signaled state (red light)
        // auto reset after every Wait
        _handle = CreateEvent (0, FALSE, FALSE, 0);
    }

    ~Event ()
    {
        CloseHandle (_handle);
    }

    // put into signaled state
    void Release () { SetEvent (_handle); }
    void Wait ()
    {
        // Wait until event is in signaled (green) state
        WaitForSingleObject (_handle, INFINITE);
    }
    operator HANDLE () { return _handle; }
private:
    HANDLE _handle;
};


class FolderWatcher : public ActiveObject
{
public:
    FolderWatcher (char const * folder, HWND hwnd)
        : _notifySource (folder),
          _hwndNotifySink (hwnd)
    {
        strcpy (_folder, folder);
        _thread.Resume ();
    }
    ~FolderWatcher ()
    {
        Kill ();
    }

private:
    void InitThread () {}
    void Loop ();
    void FlushThread () {}

    FolderChangeEvent _notifySource;
    HWND _hwndNotifySink;
    char _folder [MAX_PATH];
};


UINT const WM_FOLDER_CHANGE = WM_USER;

void FolderWatcher::Loop ()
{
    for (;;)
    {
        // Wait for change notification
        DWORD waitStatus = WaitForSingleObject (_notifySource, INFINITE);
        if (WAIT_OBJECT_0 == waitStatus)
        {
            // If folder changed
            if (_isDying)
                return;

            PostMessage (_hwndNotifySink,
                          WM_FOLDER_CHANGE,
                          0,
                          (LPARAM) _folder);

            // Continue change notification
            if (!_notifySource.ContinueNotification ())
            {
                // Error: Continuation failed
                return;
            }
        }
        else
        {
            // Error: Wait failed
            return;
        }
    }
}




int main()
{
}