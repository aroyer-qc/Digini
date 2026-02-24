//-------------------------------------------------------------------------------------------------
//
//  File :  lib_class_arp.h
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
// Define(s)
//-------------------------------------------------------------------------------------------------

#define ARP_REQUEST                     HTONS(1)
#define ARP_REPLY                       HTONS(2)
#define ARP_HARDWARE_TYPE_ETHERNET      HTONS(1)

#define ARP_PENDING_QUEUE_SIZE          4
#define ARP_SECURE_MODE   				DEF_ENABLED  //move to ip_cfg.h

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

struct ARP_PendingEntry_t
{
    IP_Address_t      IP;
    IP_PacketMsg_t*   pMsg;
    ARP_State_e       State;
};

//-------------------------------------------------------------------------------------------------
// Function prototype(s)
//-------------------------------------------------------------------------------------------------

class ARP_Manager
{
    public:

        SystemState_e       Initialize              (NetworkContext& Context);
        void                ProcessIP               (IP_PacketMsg_t* pRX);
        void                ProcessARP              (IP_PacketMsg_t* pRX);
        void                ProcessOut              (void);
        bool                Resolve                 (IP_Address_t IP, IP_MAC_Address_t* pMAC, IP_PacketMsg_t* pMsg);
        void                TimerCallBack	    	(void);

        // For ARP Timer callback
        IP_Address_t        GetIP_Address           (void)                      { return m_IP_Address;         }
        ARP_TableEntry_t*   GetTableEntryPointer    (int Entry)                 { return &m_TableEntry[(Entry < IP_ARP_TABLE_SIZE) ? Entry : 0]; }
        ARP_PendingEntry_t* GetPendingEntryByOffset (int Offset);
        void                OnPendingTimeOut        (int LogicalIndex);

	  #if (ARP_SECURE_MODE == DEF_ENABLED)
        bool 				IsPendingARP_Request	(IP_Address_t IP);
      #endif

    private:

        void                UpdateEntry				(IP_Address_t IP_Address, IP_MAC_Address_t* pMAC_Adress);
        void                FillCommon              (ARP_Frame_t* pARP, uint16_t Type);
        bool                ItHasEntry              (IP_Address_t IP);
        bool                IsItMatchingMAC         (IP_Address_t IP, IP_MAC_Address_t* pMAC);

        NetworkContext*     m_pContext;
        IP_Address_t        m_IP_Address;
        ARP_TableEntry_t    m_TableEntry[IP_ARP_TABLE_SIZE];

        ARP_PendingEntry_t  m_PendingQueue[ARP_PENDING_QUEUE_SIZE];
        int                 m_PendingOldest;                        // Points to oldest non-EMPTY entry

        nOS_Timer           m_Timer;                                // Pointer on the OS timer
};

//-------------------------------------------------------------------------------------------------
