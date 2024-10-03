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

#if (DIGINI_USE_COMM_AS_A_TASK == DEF_ENABLED)

nOS_Thread ClassTaskCOMM::m_Handle;
nOS_Stack  ClassTaskCOMM::m_Stack[TASK_COMM_STACK_SIZE];

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

#endif

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
nOS_Error ClassTaskCOMM::Initialize(void)
{
    nOS_Error Error = NOS_OK;

    // Uart console Command Line and VT100 terminal
    myConsole.Initialize(CON_USE_COMM_UART);

  #if (DIGINI_USE_CMD_LINE == DEF_ENABLED)
    myCommandLine.Initialize(&myConsole);
    myConsole.GiveControlToChildProcess(&myCommandLine);            // Hijack the console if there is a CLI
  #endif

  #if (DIGINI_USE_VT100_MENU == DEF_ENABLED)
    myVT100.Initialize(&myConsole);

   #if (DIGINI_USE_CMD_LINE == DEF_DISABLED)
    myConsole.GiveControlToChildProcess(&myVT100);                  // Hijack the console for VT100 if there is no CLI
   #endif

  #endif

  #if (DIGINI_USE_COMM_AS_A_TASK == DEF_ENABLED)

   #if (DIGINI_USE_STACKTISTIC == DEF_ENABLED)
    myStacktistic.Register(&m_Stack[0], TASK_COMM_STACK_SIZE, "Task COMM");
   #endif

    Error = nOS_ThreadCreate(&m_Handle,
                             ClassTaskCOMM_Wrapper,
                             this,
                             &m_Stack[0],
                             TASK_COMM_STACK_SIZE,
                             TASK_COMM_PRIO);
  #endif

    return Error;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Process
//
//  Parameter(s):   void
//  Return:         void
//
//  Description:    main() loop of COMM
//
//-------------------------------------------------------------------------------------------------
#if (DIGINI_USE_COMM_AS_A_TASK == DEF_ENABLED)
void ClassTaskCOMM::Run(void)
{
    for(;;)
    {
        // wake the task if we have job to do...  flag! for RX and for print

        // TODO improve sleep or yield and adjust the priority for this task... it is working nicely in process mode

        //nOS_Yield();
        myConsole.Process();
       // nOS_Sleep(1);
    }
}
#endif

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_COMM_MODULE == DEF_ENABLED) && (DIGINI_USE_CONSOLE == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------

