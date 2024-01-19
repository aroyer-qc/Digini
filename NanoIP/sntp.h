//-------------------------------------------------------------------------------------------------
//
//  File :  sntp.h
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2010-2024 Alain Royer.
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
// Include file(s)
//-------------------------------------------------------------------------------------------------


#ifdef SNTP_GLOBAL
    #define SNTP_EXTERN
    #define SNTP_PRIVATE
#else
    #define SNTP_EXTERN extern
#endif

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define SNTP_MODE_CLIENT                    3
#define SNTP_VERSION_4                      4

#define SNTP_PORT                           htons(123)

#define SNTP_UNIX_START                     2208988800UL                // January 1, 1970
#define SNTP_TIME_START                     3471292800UL                // January 1, 2010

#define SNTP_OPTIONS_IN_PACKET_SIZE         160

#define SNTP_MSG_ACTION_TIME_OUT            0

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

struct SNTP_Msg_t
{
    union
    {
        struct
        {
            uint8_t    MODE    :3;
            uint8_t    VN      :3;
            uint8_t    LI      :2;
        } s;
        uint8_t by;
    } Flags_1;

    uint8_t    Stratus;
    uint8_t    Poll;
    uint8_t    Precision;

    uint32_t   RootDelay;
    uint32_t   RootDispersion;
    uint32_t   ReferenceID;
    uint32_t   RefTimeStampSecond;
    uint32_t   RefTimeStampFraction;
    uint32_t   OriTimeStampSecond;
    uint32_t   OriTimeStampFraction;
    uint32_t   RcvTimeStampSecond;
    uint32_t   RcvTimeStampFraction;
    uint32_t   TxmTimeStampSecond;
    uint32_t   TxmTimeStampFraction;
    uint8_t    Data[SNTP_OPTIONS_IN_PACKET_SIZE];
};

//-------------------------------------------------------------------------------------------------
// class
//-------------------------------------------------------------------------------------------------

class NetSNTP
{
    public:
   
        void            Initialize      (void* pQ);
        uint32_t        Request         (SOCKET SocketNumber, uint8_t* pDomainName1, uint8_t* pDomainName2, uint8_t* pError);
    
    private:

        void            Reply           (SOCKET SocketNumber);


    nOS_Timer           m_pResync;
    TickCount_t         m_Seconds;
    OS_EVENT*           m_pQ;

    
}

//-------------------------------------------------------------------------------------------------

