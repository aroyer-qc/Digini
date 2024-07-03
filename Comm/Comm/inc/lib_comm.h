//-------------------------------------------------------------------------------------------------
//
//  File : lib_comm.h
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

#pragma once

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_COMM_MODULE == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Global Macro
//-------------------------------------------------------------------------------------------------

#ifdef TASK_COMM_GLOBAL
    #define TASK_COMM_EXTERN
#else
    #define TASK_COMM_EXTERN extern
#endif

//-------------------------------------------------------------------------------------------------
// Class definition(s)
//-------------------------------------------------------------------------------------------------

class ClassTaskCOMM
{
    public:

        nOS_Error       Initialize         (void);

      #if (DIGINI_USE_COMM_AS_A_TASK == DEF_ENABLED)
        void            Run                (void);                              // Task
      #else
        void            Process            (void)   { myConsole.Process(); }    // Process
      #endif

    private:

      #if (DIGINI_USE_COMM_AS_A_TASK == DEF_ENABLED)
        static nOS_Thread      m_Handle;
        static nOS_Stack       m_Stack[TASK_COMM_STACK_SIZE];
      #endif
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

// Only one instance for now

TASK_COMM_EXTERN class ClassTaskCOMM  TaskCOMM;

#ifdef TASK_COMM_GLOBAL
                 class ClassTaskCOMM* pTaskCOMM = &TaskCOMM;
#else
    extern       class ClassTaskCOMM* pTaskCOMM;
#endif

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_COMM_MODULE == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
