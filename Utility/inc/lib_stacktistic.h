//-------------------------------------------------------------------------------------------------
//
//  File : lib_stacktistic.h
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

#pragma once

//-------------------------------------------------------------------------------------------------

class StackCheck
{
    public:

                    StackCheck                  ();
        void        InitializePort              (void);

        int         Register                    (const uint32_t* pStack, size_t STackSz, const char* pStackName);
        size_t      GetUsage                    (int StackID);
        int32_t     GetPercent                  (int StackID);
        const char* GetStackName                (int StackID);
        int         GetNumberOfRegisterStack    (void);

    private:

        const uint32_t*     m_pStackBottom [DIGINI_STACKTISTIC_NUMBER_OF_STACK];
        size_t              m_Size         [DIGINI_STACKTISTIC_NUMBER_OF_STACK];
        const char*         m_pStackName   [DIGINI_STACKTISTIC_NUMBER_OF_STACK];
        int                 m_NumberOfStack;
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#ifdef STK_CHK_GLOBAL

class StackCheck            myStacktistic;

#else

extern class StackCheck     myStacktistic;

#endif // STK_CHK_GLOBAL

//-------------------------------------------------------------------------------------------------
