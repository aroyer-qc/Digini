//-------------------------------------------------------------------------------------------------
//
//  File : lib_comm.cpp
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2024 Alain Royer.
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
//
//  This is the idle task
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#define TASK_COMM_GLOBAL
#include "./lib_digini.h"
#undef  TASK_COMM_GLOBAL

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_COMM_MODULE == DEF_ENABLED) && (DIGINI_USE_CONSOLE == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
//   Static Variables
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//  Name:           ClassTaskCOMM_Wrapper
//
//  Parameter(s):   void* pvParameters
//  Return:         void
//
//  Description:    main() for the ClassTaskCOMM_Wrapper
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
extern "C" void ClassTaskCOMM_Wrapper(void* pvParameters)
{
    (static_cast<ClassTaskCOMM*>(pvParameters))->Run();
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Initialize
//
//  Parameter(s):   void
//  Return:         nOS_Error
//
//  Description:    Initialize
//
//-------------------------------------------------------------------------------------------------
nOS_Error ClassTaskCOMM::Initialize(Console* pConsole, UART_Driver* pUart, const char* pTaskName)
{
    nOS_Error Error = NOS_OK;

    m_pConsole = pConsole;

    // Uart console Command Line and VT100 terminal
    m_pConsole->Initialize(pUart);

    Error = nOS_ThreadCreate(&m_Handle,
                             ClassTaskCOMM_Wrapper,
                             this,
                             &m_Stack[0],
                             TASK_COMM_STACK_SIZE,
                             TASK_COMM_PRIO,
                             pTaskName);

    return Error;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Run
//
//  Parameter(s):   void
//  Return:         void
//
//  Description:    main() loop of COMM
//
//-------------------------------------------------------------------------------------------------
void ClassTaskCOMM::Run(void)
{
// TODO better this... only for test... need to be completed
//const SystemDebugLevel_e DebugLvl = SystemDebugLevel_e(SYS_DEBUG_LEVEL_ETHERNET);// | SYS_DEBUG_LEVEL_MEMORY_POOL);
//myConsole.SetDebugLevel(DebugLvl);

    for(;;)
    {
        m_pConsole->Process();
        nOS_Yield();
    }
}

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_COMM_MODULE == DEF_ENABLED) && (DIGINI_USE_CONSOLE == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------

