//-------------------------------------------------------------------------------------------------
//
//  File :sys_arch.c
//
// This file is custom file, part for the lwIP TCP/IP stack.
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2020 Alain Royer.
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

#include <stdio.h>

//------------------------- System architecture includes ------------------------------------------

#include "arch/sys_arch.h"

//------------------------- lwIP includes ---------------------------------------------------------

#include "lwip/opt.h"
#include "lwip/debug.h"
#include "lwip/def.h"
#include "lwip/sys.h"
#include "lwip/mem.h"
#include "lwip/stats.h"

//-------------------- Digini Error Systems includes ----------------------------------------------

#include "lib_typedef.h"
#include "lib_macro.h"

//-------------------------------------------------------------------------------------------------
//
// Name:           sys_mbox_new
// Parameter(s):   int size                -- Size of elements in the mailbox
// Return:         sys_mbox_t              -- Handle to new mailbox
//
// Description:    Creates a new mailbox
//
//-------------------------------------------------------------------------------------------------
err_t sys_mbox_new(sys_mbox_t *pxMailBox, int iSize)
{
    void* pBuffer;

    pBuffer = (void*)pMemoryPool->Alloc(iSize * sizeof(void *);

    if(pBuffer !=  NULL)
    {
        if(nOS_QueueCreate(pxMailBox, pBuffer, (uint8_t)sizeof(void *), iSize) != NOS_OK)
        {
            pMemoryPool->Free(&pBuffer);
            return ERR_MEM;
        }

		SYS_STATS_INC_USED(mbox);
        return ERR_OK;
    }

    return ERR_MEM;
}

//-------------------------------------------------------------------------------------------------
//
// Name:           sys_mbox_free
// Parameter(s):   sys_mbox_t   mbox         -- Handle of mailbox
// Return:         None
//
// Description:    Deallocates a mailbox. If there are messages still present in the mailbox when
//                 the mailbox is deallocated, it is an indication of a programming error in lwIP
//                 and the developer should be notified.
//
//-------------------------------------------------------------------------------------------------
void sys_mbox_free(sys_mbox_t *pxMailBox)
{
    nOS_StatusReg  sr;
    bool           MessagesWaiting;

	MessagesWaiting = nOS_QueueIsEmpty(pxMailBox);
	//configASSERT((MessagesWaiting == true));

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
    free(&pxMailBox->buffer);
    nOS_QueueDelete(pxMailBox);
    nOS_LeaveCritical(sr);
}

//-------------------------------------------------------------------------------------------------
//
// Name:           sys_mbox_post
// Parameter(s):   sys_mbox_t   mbox            -- Handle of mailbox
//                 void*        data            -- Pointer to data to post
// Return:         None
//
// Description:    Post the "msg" to the mailbox.
//-------------------------------------------------------------------------------------------------
void sys_mbox_post(sys_mbox_t *pxMailBox, void *pxMessageToPost)
{
	while(nOS_QueueWrite(pxMailBox, &pxMessageToPost, NOS_WAIT_INFINITE) != NOS_OK);
}

//-------------------------------------------------------------------------------------------------
//
// Name:           sys_mbox_trypost
// Parameter(s):   sys_mbox_t   mbox            -- Handle of mailbox
//                 void *       data            -- Pointer to data to post
// Return:         err_t                        -- ERR_OK if message posted, else ERR_MEM if not.
//
// Description:    Try to post the "msg" to the mailbox.  Returns immediately with error if cannot.
//-------------------------------------------------------------------------------------------------
err_t sys_mbox_trypost(sys_mbox_t *pxMailBox, void *pxMessageToPost)
{
    err_t Error;

    // nOS can write in ISR. just wrap ISR with NOS_ISR() when using nOS function
	if(nOS_QueueWrite(pxMailBox, pxMessageToPost, NOS_NO_WAIT) == NOS_OK)
	{
		Error = ERR_OK;
	}
	else
	{
		// The queue was already full.
		Error = ERR_MEM;
		SYS_STATS_INC(mbox.err);
	}

	return Error;
}

err_t sys_mbox_trypost_fromisr(sys_mbox_t *q, void *msg)
{
    return sys_mbox_trypost(q, msg);
}

//-------------------------------------------------------------------------------------------------
//
// Name:           sys_arch_mbox_fetch
// Parameter(s):   sys_mbox_t   mbox            -- Handle of mailbox
//                 void**       msg             -- Pointer to pointer to msg received
//                 u32_t        timeout         -- Number of milliseconds until timeout
// Return:         u32_t                        -- SYS_ARCH_TIMEOUT if timeout, else number of
//                                                 milliseconds until received.
//
// Description:    locks the thread until a message arrives in the mailbox, but does not block the
//                 thread longer than "timeout" milliseconds (similar to the sys_arch_sem_wait()
//                 function). The "msg" argument is a result parameter that is set by the function
//                 (i.e., by doing "*msg = ptr"). The "msg" parameter maybe NULL to indicate that
//                 the message should be dropped.
//                 The return values are the same as for the sys_arch_sem_wait() function:
//                 Number of milliseconds spent waiting or SYS_ARCH_TIMEOUT if there was a timeout.
//                 Note that a function with a similar name, sys_mbox_fetch(), is implemented by
//                 lwIP.
//-------------------------------------------------------------------------------------------------
u32_t sys_arch_mbox_fetch(sys_mbox_t *pxMailBox, void **pBuffer, u32_t ulTimeOut)
{
    void *Dummy;
    nOS_TickCounter TickStart;
    nOS_TickCounter TickEnd;
    nOS_TickCounter TickElapse;
    u32_t           Return;

	TickStart = GetTick();

	if(NULL == pBuffer)
	{
		pBuffer = &Dummy;
	}

	if(ulTimeOut != 0)
	{
		if(nOS_QueueRead(pxMailBox, &(*pBuffer), ulTimeOut / NOS_CONFIG_TICKS_PER_SECOND) == NOS_OK)
		{
			TickEnd = GetTick();
			TickElapse = (TickEnd - TickStart) * NOS_CONFIG_TICKS_PER_SECOND;

			Return = TickElapse;
		}
		else
		{
			/* Timed out. */
			*pBuffer = NULL;
			Return   = SYS_ARCH_TIMEOUT;
		}
	}
	else
	{
		while(nOS_QueueRead(pxMailBox, &(*pBuffer), NOS_WAIT_INFINITE) != NOS_OK);
		TickEnd = GetTick();
		TickElapse = (TickEnd - TickStart) * NOS_CONFIG_TICKS_PER_SECOND;

		if(TickElapse == 0)
		{
			TickElapse = 1;
		}

		Return = TickElapse;
	}

	return Return;
}

/*---------------------------------------------------------------------------*
 * Routine:  sys_arch_mbox_tryfetch
 *---------------------------------------------------------------------------*
 * Description:
 *      Similar to sys_arch_mbox_fetch, but if message is not ready
 *      immediately, we'll return with SYS_MBOX_EMPTY.  On success, 0 is
 *      returned.
 * Inputs:
 *      sys_mbox_t mbox         -- Handle of mailbox
 *      void **msg              -- Pointer to pointer to msg received
 * Outputs:
 *      u32_t                   -- SYS_MBOX_EMPTY if no messages.  Otherwise,
 *                                  return ERR_OK.
 *---------------------------------------------------------------------------*/
u32_t sys_arch_mbox_tryfetch( sys_mbox_t *pxMailBox, void **ppvBuffer )
{
    void *Dummy;
    u32_t Return;

	if(ppvBuffer== NULL)
	{
		ppvBuffer = &Dummy;
	}

    // nOS can read in ISR. just wrap ISR with NOS_ISR() when using nOS function
    if(nOS_QueueRead(pxMailBox, &( *ppvBuffer ), 0 ) == NOS_OK)
	{
		Return = ERR_OK;
	}
	else
	{
		Return = SYS_MBOX_EMPTY;
	}

	return Return;
}

/*---------------------------------------------------------------------------*
 * Routine:  sys_sem_new
 *---------------------------------------------------------------------------*
 * Description:
 *      Creates and returns a new semaphore. The "ucCount" argument specifies
 *      the initial state of the semaphore.
 *      NOTE: Currently this routine only creates counts of 1 or 0
 * Inputs:
 *      sys_mbox_t mbox         -- Handle of mailbox
 *      u8_t ucCount            -- Initial ucCount of semaphore (1 or 0)
 * Outputs:
 *      sys_sem_t               -- Created semaphore or 0 if could not create.
 *---------------------------------------------------------------------------*/
err_t sys_sem_new(sys_sem_t *pxSemaphore, u8_t ucCount)
{
    err_t xReturn = ERR_MEM;

	if(nOS_SemCreate(pxSemaphore, 0, 1) == NOS_OK)
	{
		if(ucCount == 0)
		{
			nOS_SemTake(pxSemaphore, NOS_NO_WAIT);
		}

		xReturn = ERR_OK;
		SYS_STATS_INC_USED( sem );
	}
	else
	{
		SYS_STATS_INC( sem.err );
	}

	return xReturn;
}

/*---------------------------------------------------------------------------*
 * Routine:  sys_arch_sem_wait
 *---------------------------------------------------------------------------*
 * Description:
 *      Blocks the thread while waiting for the semaphore to be
 *      signaled. If the "timeout" argument is non-zero, the thread should
 *      only be blocked for the specified time (measured in
 *      milliseconds).
 *
 *      If the timeout argument is non-zero, the return value is the number of
 *      milliseconds spent waiting for the semaphore to be signaled. If the
 *      semaphore wasn't signaled within the specified time, the return value is
 *      SYS_ARCH_TIMEOUT. If the thread didn't have to wait for the semaphore
 *      (i.e., it was already signaled), the function may return zero.
 *
 *      Notice that lwIP implements a function with a similar name,
 *      sys_sem_wait(), that uses the sys_arch_sem_wait() function.
 * Inputs:
 *      sys_sem_t sem           -- Semaphore to wait on
 *      u32_t timeout           -- Number of milliseconds until timeout
 * Outputs:
 *      u32_t                   -- Time elapsed or SYS_ARCH_TIMEOUT.
 *---------------------------------------------------------------------------*/
u32_t sys_arch_sem_wait( sys_sem_t *pxSemaphore, u32_t ulTimeout)
{
    nOS_TickCounter TickStart;
    nOS_TickCounter TickEnd;
    nOS_TickCounter TickElapse;
    unsigned long ulReturn;

	TickStart = GetTick();

	if(ulTimeout != 0)
	{
		if(nOS_SemTake(pxSemaphore, ulTimeout / NOS_CONFIG_TICKS_PER_SECOND ) == NOS_OK)
		{
			TickEnd = GetTick();
			TickElapse = (TickEnd - TickStart) * NOS_CONFIG_TICKS_PER_SECOND;
			ulReturn = TickElapse;
		}
		else
		{
			ulReturn = SYS_ARCH_TIMEOUT;
		}
	}
	else
	{
		while(nOS_SemTake(pxSemaphore, NOS_WAIT_INFINITE) != NOS_OK);
		TickEnd = GetTick();
		TickElapse = (TickEnd - TickStart) * NOS_CONFIG_TICKS_PER_SECOND;

		if(TickElapse == 0)
		{
			TickElapse = 1;
		}

		ulReturn = TickElapse;
	}

	return ulReturn;
}

/*---------------------------------------------------------------------------*
 * Routine:  sys_sem_signal
 *---------------------------------------------------------------------------*
 * Description:
 *      Signals (releases) a semaphore
 * Inputs:
 *      sys_sem_t sem           -- Semaphore to signal
 *---------------------------------------------------------------------------*/
void sys_sem_signal( sys_sem_t *pxSemaphore )
{
	nOS_SemGive(pxSemaphore);
}

/*---------------------------------------------------------------------------*
 * Routine:  sys_sem_free
 *---------------------------------------------------------------------------*
 * Description:
 *      Deallocates a semaphore
 * Inputs:
 *      sys_sem_t sem           Semaphore to free
 *---------------------------------------------------------------------------*/
void sys_sem_free(sys_sem_t *pxSemaphore)
{
	SYS_STATS_DEC(sem.used);
	nOS_SemDelete(pxSemaphore);
}


//-------------------------------------------------------------------------------------------------
//
// Name:           sys_mutex_new
// Parameter(s):   sys_mutex_t   mbox           Handle of mutex
//
// Return:         err_t                        ERR_OK if mutex is created, else ERR_MEM if not.
//
// Description:    Create a new mutex
//-------------------------------------------------------------------------------------------------
err_t sys_mutex_new(sys_mutex_t *pxMutex)
{
    err_t Error = ERR_MEM;

	if(nOS_MutexCreate(pxMutex, NOS_MUTEX_RECURSIVE, NOS_MUTEX_PRIO_INHERIT) == NOS_OK)
	{
		Error = ERR_OK;
		SYS_STATS_INC_USED(mutex);
	}
	else
	{
		SYS_STATS_INC(mutex.err);
	}

	return Error;
}

//-------------------------------------------------------------------------------------------------
//
// Name:           sys_mutex_lock
// Parameter(s):   sys_mutex_t   Mutex            Handle of mutex
// Return:         None
//
// Description:    Lock a mutex
//-------------------------------------------------------------------------------------------------
void sys_mutex_lock(sys_mutex_t* Mutex)
{
    while(nOS_MutexLock(Mutex, NOS_WAIT_INFINITE) != NOS_OK){};
}

//-------------------------------------------------------------------------------------------------
//
// Name:           sys_mutex_unlock
// Parameter(s):   sys_mutex_t   Mutex            Handle of mutex
// Return:         None
//
// Description:    Unlock a mutex
//-------------------------------------------------------------------------------------------------
void sys_mutex_unlock(sys_mutex_t *Mutex)
{
    nOS_MutexUnlock(Mutex);
}

//-------------------------------------------------------------------------------------------------
//
// Name:           sys_mutex_free
// Parameter(s):   sys_mutex_t   mbox            -- Handle of mutex
// Return:         None
//
// Description:    Delete a mutex
//-------------------------------------------------------------------------------------------------
void sys_mutex_free(sys_mutex_t* Mutex)
{
	SYS_STATS_DEC(mutex.used);
	nOS_MutexDelete(Mutex);
}

//-------------------------------------------------------------------------------------------------
//
// Name:           sys_init
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
// Parameter(s):   None
// Return:         None
//
// Description:    Return actual systick
//-------------------------------------------------------------------------------------------------
u32_t sys_now(void)
{
	return (u32_t)GetTick();
}

//-------------------------------------------------------------------------------------------------
//
// Name:           sys_thread_new
// Parameter(s):   char*    Name                    Name of the thread
//                 void     (*Thread)(void* arg)    Thread-function.
//                 void*    Arg                     parameter passed to 'thread'
//                 int      StackSize               stack size in bytes for the new thread.
//                 int      Priority                priority of the new thread
// Return:         sys_thread_t
//
// Description:    Creates a new thread
//-------------------------------------------------------------------------------------------------
sys_thread_t sys_thread_new(const char* Name, void(*Thread)(void* Parameters), void* Arg, int StackSize, int Priority)
{
    nOS_Stack*    pBuffer;
    nOS_Thread*   pThread;

   #if (NOS_CONFIG_THREAD_NAME_ENABLE == 0)
     VAR_UNUSED(Name);
   #endif


   pBuffer = (void*);

    if(pBuffer !=  NULL)
    {


    if((pThread = (nOS_Thread*)pMemoryPool->Alloc(sizeof(nOS_Thread)) != NULL)
    {
        if((pBuffer = (nOS_Stack*)pMemoryPool->Alloc(StackSize) !=  NULL)
        {
            if(nOS_ThreadCreate(pThread, Thread, Arg, pBuffer, StackSize, Priority
                              #if (NOS_CONFIG_THREAD_NAME_ENABLE > 0)
                              , Name
                              #endif
                                ) != NOS_OK)
            {
                pMemoryPool-Free(&pBuffer);
                pMemoryPool-Free(&pThread);
            }
        }
        else
        {
            pMemoryPool-Free(&pThread);
            
        }
    }

            return *pThread;

}

/*---------------------------------------------------------------------------*
 * Routine:  sys_arch_protect
 *---------------------------------------------------------------------------*
 * Description:
 *      This optional function does a "fast" critical region protection and
 *      returns the previous protection level. This function is only called
 *      during very short critical regions. An embedded system which supports
 *      ISR-based drivers might want to implement this function by disabling
 *      interrupts. Task-based systems might want to implement this by using
 *      a mutex or disabling tasking. This function should support recursive
 *      calls from the same task or interrupt. In other words,
 *      sys_arch_protect() could be called while already protected. In
 *      that case the return value indicates that it is already protected.
 *
 *      sys_arch_protect() is only required if your port is supporting an
 *      operating system.
 * Outputs:
 *      sys_prot_t              -- Previous protection level (not used here)
 *---------------------------------------------------------------------------*/
sys_prot_t sys_arch_protect(void)
{
    sys_mutex_lock(&sys_arch_mutex);
	return 0;
}

/*---------------------------------------------------------------------------*
 * Routine:  sys_arch_unprotect
 *---------------------------------------------------------------------------*
 * Description:
 *      See the documentation for
 *      sys_arch_protect() for more information. This function is only
 *      required if your port is supporting an operating system.
 * Inputs:
 *      sys_prot_t
 *---------------------------------------------------------------------------*/
void sys_arch_unprotect(sys_prot_t Value)
{
	VAR_UNUSED(Value);
	sys_mutex_unlock(&sys_arch_mutex);
}

/*
 * Prints an assertion messages and aborts execution.
 */
void sys_assert(const char *Message)
{
	(void) Message;

	for (;;)
	{
	}
}


