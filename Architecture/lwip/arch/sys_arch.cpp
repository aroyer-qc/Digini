//-------------------------------------------------------------------------------------------------
//
//  File :sys_arch.cpp
//
// This file is custom file, part for the lwIP TCP/IP stack.
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2023 Alain Royer.
// Email: aroyer.qc@gmail.com
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software
// and associated documentation files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or
// substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
// INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
// AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "lib_digini.h"

//-------------------------------------------------------------------------------------------------

extern "C" {

//-------------------------------------------------------------------------------------------------
//
// Name:           sys_mbox_new
//
// Parameter(s):   sys_mbox_t* pMailBox         Handle of mailbox
//                 int         Size             Size of elements in the mailbox
//
// Return:         err_t                        ERR_OK if mailbox is created, else ERR_MEM if not
//
// Description:    Creates a new mailbox
//
//-------------------------------------------------------------------------------------------------
err_t sys_mbox_new(sys_mbox_t* pMailBox, int Size)
{
    void* pBuffer;

    pBuffer = (void*)pMemoryPool->Alloc(Size * sizeof(void *));

    if(pBuffer != nullptr)
    {
        if(nOS_QueueCreate(pMailBox, pBuffer, (uint8_t)sizeof(void *), Size) == NOS_OK)
        {
            SYS_STATS_INC_USED(mbox);
            return ERR_OK;
        }

        pMemoryPool->Free((void**)&pBuffer);
    }

    return ERR_MEM;
}

//-------------------------------------------------------------------------------------------------
//
// Name:           sys_mbox_free
//
// Parameter(s):   sys_mbox_t*  mbox         Handle of mailbox
// Return:         None
//
// Description:    Deallocates a mailbox. If there are messages still present in the mailbox when
//                 the mailbox is deallocated, it is an indication of a programming error in lwIP
//                 and the developer should be notified.
//
//-------------------------------------------------------------------------------------------------
void sys_mbox_free(sys_mbox_t* pMailBox)
{
    nOS_StatusReg  sr;
    bool           MessagesWaiting;

	MessagesWaiting = nOS_QueueIsEmpty(pMailBox);

	#if SYS_STATS
	{
		if(MessagesWaiting != true)
		{
			SYS_STATS_INC(mbox.err);
		}

		SYS_STATS_DEC(mbox.used);
	}
	#endif // SYS_STATS

    nOS_EnterCritical(sr);
    pMemoryPool->Free((void**)&pMailBox->buffer);
    nOS_QueueDelete(pMailBox);
    nOS_LeaveCritical(sr);
}

//-------------------------------------------------------------------------------------------------
//
// Name:           sys_mbox_post
//
// Parameter(s):   sys_mbox_t*  pMailBox            Handle of mailbox
//                 void*        pMessageToPost      Pointer to data to post
// Return:         None
//
// Description:    Post the "msg" to the mailbox.
//-------------------------------------------------------------------------------------------------
void sys_mbox_post(sys_mbox_t* pMailBox, void* pMessageToPost)
{
	while(nOS_QueueWrite(pMailBox, &pMessageToPost, NOS_WAIT_INFINITE) != NOS_OK) {};
}

//-------------------------------------------------------------------------------------------------
//
// Name:           sys_mbox_trypost
//
// Parameter(s):   sys_mbox_t*  pMailBox            Handle of mailbox
//                 void *       pMessageToPost      Pointer to data to post
// Return:         err_t                            ERR_OK if message posted, else ERR_MEM if not.
//
// Description:    Try to post the "msg" to the mailbox.  Returns immediately with error if cannot.
//-------------------------------------------------------------------------------------------------
#define sys_mbox_trypost_fromisr sys_mbox_trypost
err_t sys_mbox_trypost(sys_mbox_t* pMailBox, void* pMessageToPost)
{
    err_t Error = ERR_OK;

    // nOS can write in ISR. just wrap ISR with NOS_ISR() when using nOS function
	if(nOS_QueueWrite(pMailBox, pMessageToPost, NOS_NO_WAIT) != NOS_OK)
	{
		// The queue was already full.
		Error = ERR_MEM;
		SYS_STATS_INC(mbox.err);
	}

	return Error;
}

//-------------------------------------------------------------------------------------------------
//
// Name:           sys_arch_mbox_fetch
//
// Parameter(s):   sys_mbox_t*  pMailBox         Handle of mailbox
//                 void**       ppBuffer         Pointer to pointer to msg received
//                 u32_t        TimeOut          Number of milliseconds until timeout
// Return:         u32_t                         SYS_ARCH_TIMEOUT if timeout, else number of
//                                               milliseconds until received.
//
// Description:    Locks the thread until a message arrives in the mailbox.
//
// Note(s):        Does not block the thread longer than "timeout" milliseconds (similar to the
//                 sys_arch_sem_wait() function). The "msg" argument is a result parameter that is
//                 set by the function (i.e., by doing "*msg = ptr"). The "msg" parameter maybe
//                 NULL to indicate that the message should be dropped. The return values are the
//                 same as for the sys_arch_sem_wait() function: Number of milliseconds spent
//                 waiting or SYS_ARCH_TIMEOUT if there was a timeout. Note that a function with a
//                 similar name, sys_mbox_fetch(), is implemented by lwIP.
//-------------------------------------------------------------------------------------------------
u32_t sys_arch_mbox_fetch(sys_mbox_t* pMailBox, void** ppBuffer, u32_t TimeOut)
{
    void*           Dummy;
    nOS_TickCounter TickStart;
    nOS_TickCounter TickEnd;
    nOS_TickCounter TickElapse;
    u32_t           Return;

	TickStart = GetTick();

	if(NULL == ppBuffer)
	{
		ppBuffer = &Dummy;
	}

	if(TimeOut != 0)
	{
        if(nOS_QueueRead(pMailBox, &(*ppBuffer), TimeOut) == NOS_OK)
		{
			TickEnd = GetTick();
			TickElapse = (TickEnd - TickStart);

			Return = TickElapse;
		}
		else
		{
			// Timed out.
			*ppBuffer = nullptr;
			Return    = SYS_ARCH_TIMEOUT;
		}
	}
	else
	{
		while(nOS_QueueRead(pMailBox, &(*ppBuffer), NOS_WAIT_INFINITE) != NOS_OK){};
		TickEnd    = GetTick();
		TickElapse = (TickEnd - TickStart);

		if(TickElapse == 0)
		{
			TickElapse = 1;
		}

		Return = TickElapse;
	}

	return Return;
}

//-------------------------------------------------------------------------------------------------
//
// Name:           sys_arch_mbox_tryfetch
//
// Parameter(s):   sys_mbox_t*  pMailBox      Handle of mailbox
//                 void**       ppBuffer      Pointer to buffer received
//
// Return:         u32_t                   -  SYS_MBOX_EMPTY if no messages. Otherwise, ERR_OK.
//
// Description:    Similar to sys_arch_mbox_fetch, but if message is not ready immediately, will
//                 return with SYS_MBOX_EMPTY.  On success, ERR_OK is returned.
//-------------------------------------------------------------------------------------------------
u32_t sys_arch_mbox_tryfetch(sys_mbox_t* pMailBox, void** ppBuffer)
{
    void *Dummy;
    u32_t Return = ERR_OK;

	if(ppBuffer == nullptr)
	{
		ppBuffer = &Dummy;
	}

    // nOS can read in ISR. just wrap ISR with NOS_ISR() when using nOS function
    if(nOS_QueueRead(pMailBox, &(*ppBuffer), NOS_NO_WAIT) != NOS_OK)
	{
		Return = SYS_MBOX_EMPTY;
	}
	else
    {
        __asm("nop");
    }

	return Return;
}

//-------------------------------------------------------------------------------------------------
//
// Name:           sys_sem_new
//
// Parameter(s):   sys_sem_t*  pSemaphore     Handle of semaphore
//                  u8_t       Count         Initial Count of semaphore (1 or 0)
//
// Return:         err_t       ERR_OK if semaphore is created, else ERR_MEM if not
//
// Description:    Creates and returns a new semaphore.
//
// Note(s):        The "Count" argument specifies the initial state of the semaphore.
//                 NOTE: Currently this routine only creates counts of 1 or 0
//-------------------------------------------------------------------------------------------------
err_t sys_sem_new(sys_sem_t* pSemaphore, u8_t Count)
{
    if(nOS_SemCreate(pSemaphore, 0, Count) == NOS_OK)
    {
        if(Count == 0)
        {
            nOS_SemTake(pSemaphore, NOS_NO_WAIT);
        }

        SYS_STATS_INC_USED(sem);
        return ERR_OK;
    }

    SYS_STATS_INC(sem.err);
	return ERR_MEM;
}

//-------------------------------------------------------------------------------------------------
//
// Name:           sys_arch_sem_wait
//
// Parameter(s):   sys_sem_t*   pSemaphore     Semaphore to wait on
//                 u32_t        TimeOut        Number of milliseconds until timeout
//
// Return:         u32_t                    Time elapsed or SYS_ARCH_TIMEOUT
//
// Description:    Blocks the thread while waiting for the semaphore to be signaled.
//
// Note(s):        If the "timeout" argument is non-zero, the thread should only be blocked for
//                 the specified time (measured in milliseconds). If the timeout argument is
//                 non-zero, the return value is the number of milliseconds spent waiting for the
//                 semaphore to be signaled. If the semaphore was not signaled within the specified
//                 time, the return value is SYS_ARCH_TIMEOUT. If the thread didn't have to wait for
//                 the semaphore (i.e., it was already signaled), the function may return zero.
//                 Notice that lwIP implements a function with a similar name, sys_sem_wait(),
//                 that uses the sys_arch_sem_wait() function.
//-------------------------------------------------------------------------------------------------
u32_t sys_arch_sem_wait(sys_sem_t* pSemaphore, u32_t TimeOut)
{
    nOS_TickCounter TickStart;
    nOS_TickCounter TickEnd;
    nOS_TickCounter TickElapse;
    u32_t           Return;

	TickStart = GetTick();

	if(TimeOut != 0)
	{
		if(nOS_SemTake(pSemaphore, TimeOut) == NOS_OK)
		{
			TickEnd = GetTick();
			TickElapse = (TickEnd - TickStart);
			Return = TickElapse;
		}
		else
		{
			Return = SYS_ARCH_TIMEOUT;
		}
	}
	else
	{
		while(nOS_SemTake(pSemaphore, NOS_WAIT_INFINITE) != NOS_OK);
		TickEnd = GetTick();
		TickElapse = (TickEnd - TickStart);

		if(TickElapse == 0)
		{
			TickElapse = 1;
		}

		Return = TickElapse;
	}

	return Return;
}

//-------------------------------------------------------------------------------------------------
//
// Name:           sys_sem_signal
//
// Parameter(s):   sys_sem_t*   pSemaphore
//
// Return:         None
//
// Description:    Signals (releases) a semaphore
//-------------------------------------------------------------------------------------------------
void sys_sem_signal(sys_sem_t* pSemaphore)
{
	nOS_SemGive(pSemaphore);
}

//-------------------------------------------------------------------------------------------------
//
// Name:           sys_sem_free
//
// Parameter(s):   sys_sem_t*   pSemaphore
//
// Return:         None
//
// Description:    Deallocates a semaphore
//-------------------------------------------------------------------------------------------------
void sys_sem_free(sys_sem_t* pSemaphore)
{
	SYS_STATS_DEC(sem.used);
	nOS_SemDelete(pSemaphore);
}

//-------------------------------------------------------------------------------------------------
//
// Name:           sys_mutex_new
//
// Parameter(s):   sys_mutex_t*     pMutex         Handle of mutex
//
// Return:         err_t                           ERR_OK if mutex is created, else ERR_MEM if not.
//
// Description:    Create a new mutex
//-------------------------------------------------------------------------------------------------
err_t sys_mutex_new(sys_mutex_t* pMutex)
{
	if(nOS_MutexCreate(pMutex, NOS_MUTEX_NORMAL, NOS_MUTEX_PRIO_INHERIT) == NOS_OK)
	{
        SYS_STATS_INC_USED(mutex);
        return ERR_OK;
	}

    SYS_STATS_INC(mutex.err);
	return ERR_MEM;
}

//-------------------------------------------------------------------------------------------------
//
// Name:           sys_mutex_lock
//
// Parameter(s):   sys_mutex_t*     pMutex              Handle of mutex
// Return:         None
//
// Description:    Lock a mutex
//-------------------------------------------------------------------------------------------------
void sys_mutex_lock(sys_mutex_t* pMutex)
{
    while(nOS_MutexLock(pMutex, NOS_WAIT_INFINITE) != NOS_OK){};
}

//-------------------------------------------------------------------------------------------------
//
// Name:           sys_mutex_unlock
//
// Parameter(s):   sys_mutex_t*     pMutex              Handle of mutex
// Return:         None
//
// Description:    Unlock a mutex
//-------------------------------------------------------------------------------------------------
void sys_mutex_unlock(sys_mutex_t* pMutex)
{
    nOS_MutexUnlock(pMutex);
}

//-------------------------------------------------------------------------------------------------
//
// Name:           sys_mutex_free
//
// Parameter(s):   sys_mutex_t*     pMutex              Handle of mutex
// Return:         None
//
// Description:    Delete a mutex
//-------------------------------------------------------------------------------------------------
void sys_mutex_free(sys_mutex_t* pMutex)
{
	SYS_STATS_DEC(mutex.used);
	nOS_MutexDelete(pMutex);
}

//-------------------------------------------------------------------------------------------------
//
// Name:           sys_init
//
// Parameter(s):   None
// Return:         None
//
// Description:    Initialize sys arch
//-------------------------------------------------------------------------------------------------
static sys_mutex_t sys_arch_mutex;
void sys_init(void)
{
    sys_mutex_new(&sys_arch_mutex);
}

//-------------------------------------------------------------------------------------------------
//
// Name:           sys_now
//
// Parameter(s):   None
// Return:         u32_t
//
// Description:    Return actual systick
//-------------------------------------------------------------------------------------------------
u32_t sys_now(void)
{
	return u32_t(GetTick());
}

//-------------------------------------------------------------------------------------------------
//
// Name:           sys_thread_new
//
// Parameter(s):   char*    pName                   Name of the thread
//                 void     (*Thread)(void* pArg)   Thread-function.
//                 void*    pArg                    parameter passed to 'thread'
//                 int      StackSize               stack size in bytes for the new thread.
//                 int      Priority                priority of the new thread
// Return:         sys_thread_t
//
// Description:    Creates a new thread
//-------------------------------------------------------------------------------------------------
sys_thread_t sys_thread_new(const char* pName, void(*Thread)(void* Parameters), void* pArg, int StackSize, int Priority)
{
    nOS_Stack*    pBuffer;
    nOS_Thread*   pThread;

   #if (NOS_CONFIG_THREAD_NAME_ENABLE == 0)
     VAR_UNUSED(pName);
   #endif

    if((pThread = (nOS_Thread*)pMemoryPool->Alloc(sizeof(nOS_Thread))) != nullptr)
    {
        if((pBuffer = (nOS_Stack*)pMemoryPool->Alloc(StackSize)) !=  nullptr)
        {
            if(nOS_ThreadCreate(pThread, Thread, pArg, pBuffer, StackSize / sizeof(nOS_Stack), Priority
                              #if (NOS_CONFIG_THREAD_NAME_ENABLE > 0)
                              , pName
                              #endif
                                ) != NOS_OK)
            {
                pMemoryPool->Free((void**)&pBuffer);
                pMemoryPool->Free((void**)&pThread);
            }
          #if (DIGINI_USE_STACKTISTIC == DEF_ENABLED)
            else
            {
               myStacktistic.Register(pBuffer, StackSize, pName);
            }
          #endif
        }
        else
        {
            pMemoryPool->Free((void**)&pThread);
        }
    }

    return *pThread;
}

//-------------------------------------------------------------------------------------------------
//
// Name:           sys_arch_protect
//
// Parameter(s):   None
//
// Return:         sys_prot_t               Previous protection level (not used here)
//
// Description:    This optional function does a "fast" critical region protection and returns the
//                 previous protection level. This function is only called during very short
//                 critical regions. An embedded system which supports ISR-based drivers might want
//                 to implement this function by disabling interrupts. Task-based systems might
//                 want to implement this by using a mutex or disabling tasking. This function
//                 should support recursive calls from the same task or interrupt. In other words,
//                 sys_arch_protect() could be called while already protected. In that case the
//                 return value indicates that it is already protected.
//-------------------------------------------------------------------------------------------------
sys_prot_t sys_arch_protect(void)
{
    sys_mutex_lock(&sys_arch_mutex);
	return 0;
}

//-------------------------------------------------------------------------------------------------
//
// Name:           sys_arch_unprotect
//
// Parameter(s):   sys_prot_t   Prot        protection level (not used here)
//
// Return:         None
//
// Description:    See the documentation for sys_arch_protect() for more information. This function
//                 is only required if your port is supporting an operating system.
//-------------------------------------------------------------------------------------------------
void sys_arch_unprotect(sys_prot_t Prot)
{
	VAR_UNUSED(Prot);
	sys_mutex_unlock(&sys_arch_mutex);
}

//-------------------------------------------------------------------------------------------------
//
// Name:           sys_assert
//
// Parameter(s):   char*    pMessage
//
// Return:         None
//
// Description:    Prints an assertion messages and aborts execution.
//-------------------------------------------------------------------------------------------------
void sys_assert(const char* pMessage)
{
    DEBUG_PrintSerialLog(CON_DEBUG_LEVEL_ETHERNET, pMessage);

	for(;;)
	{
	}
}

//-------------------------------------------------------------------------------------------------

}   // extern "C"

//-------------------------------------------------------------------------------------------------

