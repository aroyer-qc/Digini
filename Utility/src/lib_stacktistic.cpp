//-------------------------------------------------------------------------------------------------
//
//  File : lib_stacktistic.cpp
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

#define STK_CHK_GLOBAL
#include "./lib_digini.h"
#undef  STK_CHK_GLOBAL

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_STACKTISTIC == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
// Name:           Constructor
//
// Parameter(s):   None
// Return:         None
//
// Description:    Initialize the stack monitoring tool.
//
//-------------------------------------------------------------------------------------------------
StackCheck::StackCheck()
{

    for(int i = 1; i < DIGINI_STACKTISTIC_NUMBER_OF_STACK; i++)
    {
        m_Size[i]         = 0;
        m_pStackBottom[i] = nullptr;
    }

    InitializePort();
}

//-------------------------------------------------------------------------------------------------
//
// Name:           Register
//
// Parameter(s):   pStack               Pointer on the stack to register.
//                 STackSz              Stack Size in multiple of 4 bytes.
//                 pStackName           Pointer on the stack name.
// Return:         StackID              Return the stackID of the registration.
//                                      -1 mean it was not register.
//
// Description:    Register a stack to be monitored by this utility
//
//-------------------------------------------------------------------------------------------------
int StackCheck::Register(const uint32_t* pStack, size_t STackSz, const char* pStackName)
{
    if(m_NumberOfStack < DIGINI_STACKTISTIC_NUMBER_OF_STACK)
    {
        #if (NOS_CONFIG_DEBUG == 0)
          memset((void*)pStack, LIB_CLASS_STACKTISTIC_WATER_MARK_CODE, STackSz * 4);
        #endif

        m_pStackBottom[m_NumberOfStack] = pStack;
        m_Size[m_NumberOfStack]         = STackSz;
        m_pStackName[m_NumberOfStack]   = pStackName;
        m_NumberOfStack++;
        return m_NumberOfStack - 1;
    }

    return -1;
}

//-------------------------------------------------------------------------------------------------
//
// Name:           GetStackName
//
// Parameter(s):   pStack               Number attributed to stack when it was register
// Return:         onst char*           Pointer on the name of the stack
//
// Description:    Return the pointer on the stack names.
//
//-------------------------------------------------------------------------------------------------
const char* StackCheck::GetStackName(int StackID)
{
    return m_pStackName[StackID];
}

//-------------------------------------------------------------------------------------------------
//
// Name:           GetUsage
//
// Parameter(s):   StackID              Number attributed to stack when it was register
// Return:         size_t               Used Size
//
// Description:    Return the use size of a specific stack.
//
//-------------------------------------------------------------------------------------------------
size_t StackCheck::GetUsage(int StackID)
{
    size_t    FreeSize = -1;
    uint32_t* pStack;

    if(StackID != -1)
    {
        FreeSize = 0;
        pStack = (uint32_t*)m_pStackBottom[StackID];

        // Use -1 here, because if we reach -1 we now know we busted the stack.
        while((FreeSize < m_Size[StackID]) && (*pStack == LIB_CLASS_STACKTISTIC_WATER_MARK))
        {
            FreeSize += 4;
            pStack   += 4;
        }
    }

    return m_Size[StackID] - FreeSize;
}

//-------------------------------------------------------------------------------------------------
//
// Name:           GetPercent
//
// Parameter(s):   StackID              Number attributed to stack when it was register
// Return:         int32_t              Percentage of the specific stack.
//
// Description:    Return the use size of a specific stack
//
//-------------------------------------------------------------------------------------------------
int32_t StackCheck::GetPercent(int StackID)
{
    int32_t Percent;

    Percent = int32_t(GetUsage(StackID));

    if(Percent != -1)
    {
        Percent = Percent * 100;
        Percent = (Percent / m_Size[StackID]) + ((((Percent * 10) % m_Size[StackID]) > 5) ? 1 : 0);      // Add 1 if .6% and more
    }

    return Percent;
}

//-------------------------------------------------------------------------------------------------
//
// Name:           GetNumberOfRegisterStack
//
// Parameter(s):   None
// Return:         int          Return the number of Stack that has been registered
//
// Description:    Return the use size of a specific stack
//
//-------------------------------------------------------------------------------------------------
int StackCheck::GetNumberOfRegisterStack(void)
{
    return m_NumberOfStack;
}

//-------------------------------------------------------------------------------------------------

#endif //  (DIGINI_USE_STACKTISTIC == DEF_ENABLED)
