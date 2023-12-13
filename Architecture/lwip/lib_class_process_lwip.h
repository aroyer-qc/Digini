//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_process_lwip.h
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
//
// Note(s)          This class is a special implementation for LWIP.    from observation:
//                  -   tcpip_thread is a task that never release control to a lower priority task.
//                  -   nOS has a big advantage, the main loop is the idle task, and is not loosing
//                      the memory allocated to the system stack.
//
//                  We want the lwip to run on the idle task, but nOS has no need to create this
//                  task. An adapted copy of the tcpip_init is perform in the init phase of this
//                  class, then an adapted copy of tcpip_thread is handle in the process of this
//                  class.
//                  NO_SYS is define at 0, because the class still need OS Support.
//
//                  Consider this an hybrid.
//
//-------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_ETHERNET == DEF_ENABLED)
#if (DIGINI_USE_LWIP == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------

class LWIP_Application
{
    public:

        nOS_Error           Initialize      (void);
        void                Process         (void);


    private:

        struct netif        m_NetIf;

        ip_addr_t           m_IP_Address;
        ip_addr_t           m_SubnetMask;
        ip_addr_t           m_GatewayIP;
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------


#ifdef LWIP_APP_GLOBAL
                 class LWIP_Application  LWIP_App;
                 class LWIP_Application* pLWIP_App = &LWIP_App;
#else
    extern       class LWIP_Application* pLWIP_App;
#endif

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_LWIP == DEF_ENABLED)
#endif // (DIGINI_USE_ETHERNET == DEF_ENABLED)

