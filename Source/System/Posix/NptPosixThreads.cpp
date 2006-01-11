/*****************************************************************
|
|      Neptune - Threads :: Posix Implementation
|
|      (c) 2001-2002 Gilles Boccon-Gibod
|      Author: Gilles Boccon-Gibod (bok@bok.net)
|
 ****************************************************************/

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include <pthread.h>
#include <unistd.h>

#include "NptConfig.h"
#include "NptTypes.h"
#include "NptThreads.h"
#include "NptDebug.h"

/*----------------------------------------------------------------------
|       NPT_PosixMutex
+---------------------------------------------------------------------*/
class NPT_PosixMutex : public NPT_MutexInterface
{
public:
    // methods
             NPT_PosixMutex();
    virtual ~NPT_PosixMutex();

    // NPT_Mutex methods
    virtual NPT_Result Lock();
    virtual NPT_Result Unlock();

private:
    // members
    pthread_mutex_t m_Mutex;
};

/*----------------------------------------------------------------------
|       NPT_PosixMutex::NPT_PosixMutex
+---------------------------------------------------------------------*/
NPT_PosixMutex::NPT_PosixMutex()
{
    pthread_mutex_init(&m_Mutex, NULL);
}

/*----------------------------------------------------------------------
|       NPT_PosixMutex::~NPT_PosixMutex
+---------------------------------------------------------------------*/
NPT_PosixMutex::~NPT_PosixMutex()
{
    pthread_mutex_destroy(&m_Mutex);
}

/*----------------------------------------------------------------------
|       NPT_PosixMutex::Lock
+---------------------------------------------------------------------*/
NPT_Result
NPT_PosixMutex::Lock()
{
    pthread_mutex_lock(&m_Mutex);
    return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|       NPT_PosixMutex::Unlock
+---------------------------------------------------------------------*/
NPT_Result
NPT_PosixMutex::Unlock()
{
    pthread_mutex_unlock(&m_Mutex);
    return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|       NPT_Mutex::NPT_Mutex
+---------------------------------------------------------------------*/
NPT_Mutex::NPT_Mutex()
{
    m_Delegate = new NPT_PosixMutex();
}

/*----------------------------------------------------------------------
|       NPT_PosixSharedVariable
+---------------------------------------------------------------------*/
class NPT_PosixSharedVariable : public NPT_SharedVariableInterface
{
 public:
    // methods
               NPT_PosixSharedVariable(NPT_Integer value);
              ~NPT_PosixSharedVariable();
    NPT_Result SetValue(NPT_Integer value);
    NPT_Result GetValue(NPT_Integer& value);
    NPT_Result WaitUntilEquals(NPT_Integer value);
    NPT_Result WaitWhileEquals(NPT_Integer value);

 private:
    // members
    volatile NPT_Integer m_Value;
    pthread_mutex_t      m_Mutex;
    pthread_cond_t       m_Condition;
};

/*----------------------------------------------------------------------
|       NPT_PosixSharedVariable::NPT_PosixSharedVariable
+---------------------------------------------------------------------*/
NPT_PosixSharedVariable::NPT_PosixSharedVariable(NPT_Integer value) : 
    m_Value(value)
{
    pthread_mutex_init(&m_Mutex, NULL);
    pthread_cond_init(&m_Condition, NULL);
}

/*----------------------------------------------------------------------
|       NPT_PosixSharedVariable::~NPT_PosixSharedVariable
+---------------------------------------------------------------------*/
NPT_PosixSharedVariable::~NPT_PosixSharedVariable()
{
    pthread_cond_destroy(&m_Condition);
    pthread_mutex_destroy(&m_Mutex);
}

/*----------------------------------------------------------------------
|       NPT_PosixSharedVariable::SetValue
+---------------------------------------------------------------------*/
NPT_Result
NPT_PosixSharedVariable::SetValue(NPT_Integer value)
{
    pthread_mutex_lock(&m_Mutex);
    m_Value = value;
    pthread_cond_signal(&m_Condition);
    pthread_mutex_unlock(&m_Mutex);
    
    return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|       NPT_PosixSharedVariable::GetValue
+---------------------------------------------------------------------*/
NPT_Result
NPT_PosixSharedVariable::GetValue(NPT_Integer& value)
{
    // reading an integer should be atomic on most platforms
    value = m_Value;

    return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|       NPT_PosixSharedVariable::WaitUntilEquals
+---------------------------------------------------------------------*/
NPT_Result
NPT_PosixSharedVariable::WaitUntilEquals(NPT_Integer value)
{
    pthread_mutex_lock(&m_Mutex);
    while (value != m_Value) {
        pthread_cond_wait(&m_Condition, &m_Mutex);
    }
    pthread_mutex_unlock(&m_Mutex);
    
    return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|       NPT_PosixSharedVariable::WaitWhileEquals
+---------------------------------------------------------------------*/
NPT_Result
NPT_PosixSharedVariable::WaitWhileEquals(NPT_Integer value)
{
    pthread_mutex_lock(&m_Mutex);
    while (value == m_Value) {
        pthread_cond_wait(&m_Condition, &m_Mutex);
    }
    pthread_mutex_unlock(&m_Mutex);
    
    return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|       NPT_SharedVariable::NPT_SharedVariable
+---------------------------------------------------------------------*/
NPT_SharedVariable::NPT_SharedVariable(NPT_Integer value)
{
    m_Delegate = new NPT_PosixSharedVariable(value);
}

/*----------------------------------------------------------------------
|       NPT_PosixAtomicVariable
+---------------------------------------------------------------------*/
class NPT_PosixAtomicVariable : public NPT_AtomicVariableInterface
{
 public:
    // methods
                NPT_PosixAtomicVariable(NPT_Integer value);
               ~NPT_PosixAtomicVariable();
    NPT_Integer Increment(); 
    NPT_Integer Decrement();
    NPT_Integer GetValue();
    void        SetValue(NPT_Integer value);

 private:
    // members
    volatile NPT_Integer m_Value;
    pthread_mutex_t      m_Mutex;
};

/*----------------------------------------------------------------------
|       NPT_PosixAtomicVariable::NPT_PosixAtomicVariable
+---------------------------------------------------------------------*/
NPT_PosixAtomicVariable::NPT_PosixAtomicVariable(NPT_Integer value) : 
    m_Value(value)
{
    pthread_mutex_init(&m_Mutex, NULL);
}

/*----------------------------------------------------------------------
|       NPT_PosixAtomicVariable::~NPT_PosixAtomicVariable
+---------------------------------------------------------------------*/
NPT_PosixAtomicVariable::~NPT_PosixAtomicVariable()
{
    pthread_mutex_destroy(&m_Mutex);
}

/*----------------------------------------------------------------------
|       NPT_PosixAtomicVariable::Increment
+---------------------------------------------------------------------*/
NPT_Integer
NPT_PosixAtomicVariable::Increment()
{
    NPT_Integer value;

    pthread_mutex_lock(&m_Mutex);
    value = ++m_Value;
    pthread_mutex_unlock(&m_Mutex);
    
    return value;
}

/*----------------------------------------------------------------------
|       NPT_PosixAtomicVariable::Decrement
+---------------------------------------------------------------------*/
NPT_Integer
NPT_PosixAtomicVariable::Decrement()
{
    NPT_Integer value;

    pthread_mutex_lock(&m_Mutex);
    value = --m_Value;
    pthread_mutex_unlock(&m_Mutex);
    
    return value;
}

/*----------------------------------------------------------------------
|       NPT_PosixAtomicVariable::GetValue
+---------------------------------------------------------------------*/
NPT_Integer
NPT_PosixAtomicVariable::GetValue()
{
    return m_Value;
}

/*----------------------------------------------------------------------
|       NPT_PosixAtomicVariable::SetValue
+---------------------------------------------------------------------*/
void
NPT_PosixAtomicVariable::SetValue(NPT_Integer value)
{
    pthread_mutex_lock(&m_Mutex);
    m_Value = value;
    pthread_mutex_unlock(&m_Mutex);
}

/*----------------------------------------------------------------------
|       NPT_AtomicVariable::NPT_AtomicVariable
+---------------------------------------------------------------------*/
NPT_AtomicVariable::NPT_AtomicVariable(NPT_Integer value)
{
    m_Delegate = new NPT_PosixAtomicVariable(value);
}

/*----------------------------------------------------------------------
|       NPT_PosixThread
+---------------------------------------------------------------------*/
class NPT_PosixThread : public NPT_ThreadInterface
{
 public:
    // methods
                NPT_PosixThread(NPT_Thread*   delegator,
                                NPT_Runnable& target,
                                bool          detached);
               ~NPT_PosixThread();
    NPT_Result  Start(); 
    NPT_Result  Wait();
    NPT_Result  Terminate();

 private:
    // methods
    static void* EntryPoint(void* argument);

    // NPT_Runnable methods
    void Run();

    // members
    NPT_Thread*    m_Delegator;
    NPT_Runnable&  m_Target;
    bool           m_Detached;
    pthread_t      m_ThreadId;
    bool           m_Joined;
    NPT_PosixMutex m_JoinLock;
};

/*----------------------------------------------------------------------
|       NPT_PosixThread::NPT_PosixThread
+---------------------------------------------------------------------*/
NPT_PosixThread::NPT_PosixThread(NPT_Thread*   delegator,
                                 NPT_Runnable& target,
                                 bool          detached) : 
    m_Delegator(delegator),
    m_Target(target),
    m_Detached(detached),
    m_ThreadId(0),
    m_Joined(false)
{
    NPT_Debug(":: NPT_PosixThread::NPT_PosixThread\n");
}

/*----------------------------------------------------------------------
|       NPT_PosixThread::~NPT_PosixThread
+---------------------------------------------------------------------*/
NPT_PosixThread::~NPT_PosixThread()
{
    NPT_Debug(":: NPT_PosixThread::~NPT_PosixThread %d\n", m_ThreadId);

    if (!m_Detached) {
        // we're not detached, and not in the Run() method, so we need to 
        // wait until the thread is done
        Wait();
    }
}

/*----------------------------------------------------------------------
|       NPT_PosixThread::Terminate
+---------------------------------------------------------------------*/
NPT_Result
NPT_PosixThread::Terminate()
{
    NPT_Debug(":: NPT_PosixThread::Terminate %d\n", m_ThreadId);

    // if we're detached, we need to delete ourselves
    if (m_Detached) {
        delete m_Delegator;
    }

    return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|       NPT_PosixThread::EntryPoint
+---------------------------------------------------------------------*/
void*
NPT_PosixThread::EntryPoint(void* argument)
{
    NPT_PosixThread* thread = reinterpret_cast<NPT_PosixThread*>(argument);

    NPT_Debug(":: NPT_PosixThread::EntryPoint - in =======================\n");

    // run the thread 
    thread->Run();
    
    NPT_Debug(":: NPT_PosixThread::EntryPoint - out ======================\n");

    // we're done with the thread object
    thread->Terminate();

    // done
    return NULL;
}

/*----------------------------------------------------------------------
|       NPT_PosixThread::Start
+---------------------------------------------------------------------*/
NPT_Result
NPT_PosixThread::Start()
{
    NPT_Debug(":: NPT_PosixThread::Start - creating thread\n");

    // create the native thread
    int result = pthread_create(&m_ThreadId, NULL, EntryPoint, 
                                reinterpret_cast<void*>(this));
    NPT_Debug(":: NPT_PosixThread::Start - id = %d, res=%d\n", 
              m_ThreadId, result);
    if (result) {
        // failed
        return NPT_FAILURE;
    } else {
        // detach the thread if we're not joinable
        if (m_Detached) {
            pthread_detach(m_ThreadId);
        }
        return NPT_SUCCESS;
    }
}

/*----------------------------------------------------------------------
|       NPT_PosixThread::Run
+---------------------------------------------------------------------*/
void
NPT_PosixThread::Run()
{
    m_Target.Run();
}

/*----------------------------------------------------------------------
|       NPT_PosixThread::Wait
+---------------------------------------------------------------------*/
NPT_Result
NPT_PosixThread::Wait()
{
    void* return_value;
    int   result;

    NPT_Debug(":: NPT_PosixThread::Wait - waiting for id %d\n", m_ThreadId);

    // check that we're not detached
    if (m_ThreadId == 0 || m_Detached) {
        return NPT_FAILURE;
    }

    // wait for the thread to finish
    m_JoinLock.Lock();
    if (m_Joined) {
        NPT_Debug(":: NPT_PosixThread::Wait - %d already joined\n", 
                  m_ThreadId);
        result = NPT_SUCCESS;
    } else {
        NPT_Debug(":: NPT_PosixThread::Wait - joining thread id %d\n", 
                  m_ThreadId);
        result = pthread_join(m_ThreadId, &return_value);
        m_Joined = true;
    }
    m_JoinLock.Unlock();
    if (result != 0) {
        return NPT_FAILURE;
    } else {
        return NPT_SUCCESS;
    }
}

/*----------------------------------------------------------------------
|       NPT_Thread::NPT_Thread
+---------------------------------------------------------------------*/
NPT_Thread::NPT_Thread(bool detached)
{
    m_Delegate = new NPT_PosixThread(this, *this, detached);
}

/*----------------------------------------------------------------------
|       NPT_Thread::NPT_Thread
+---------------------------------------------------------------------*/
NPT_Thread::NPT_Thread(NPT_Runnable& target, bool detached)
{
    m_Delegate = new NPT_PosixThread(this, target, detached);
}














