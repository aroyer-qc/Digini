//-------------------------------------------------------------------------------------------------
//
//  File : dhcp.c
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

//------ Note(s) ----------------------------------------------------------------------------------
//
//  UDP Ports 67 and 68
//      Common Use
//      Port 67 Bootps
//      Port 68 Bootpc
//
//          Typically this traffic is related to normal DHCP operation.
//          DHCP (Dynamic Host Configuration Protocol) is how your computer gets its unique
//          IP address. When a system starts up on a network it must first request an IP address
//          (assume it is not using a static IP address), and it does this  broadcasting a
//          request to the DHCP server:
//
//          UDP 0.0.0.0:68 -> 255.255.255.255:67
//
//          Since the requesting system doesn't have an IP address (why it is asking) it uses
//          0.0.0.0 and since its new to the network it doesn't know where the DHCP server is, so
//          it broadcasts the request to the entire network (255.255.255.255).
//
//          The DHCP server then responds with something like:
//
//          UDP 192.168.1.1:67 -> 255.255.255.255:68
//
//          This is typically a DHCP offer.  NOTE it has to be broadcasted (255.255.255.255)
//          as the requesting system doesn't yet have an IP address (its contained in the offer).
//          The data in this transmission contains the IP and other network configuration
//          information that the requesting system needs to connect to the network (lease time,
//          Subnet Mask, etc).
//
//          Sometimes you will see something like:
//
//          UDP 192.168.1.101:67 -> 192.168.1.1:68
//
//          as a request, followed  a reply
//
//          UDP 192.168.1.1:68 -> 192.168.1.101:67
//
//          These are typically IP renewal requests, where a system has an IP address and is asking
//          to renew it (ie get the lease extended), or if its not possible to renew the IP address
//          to receive a new IP address from the DHCP server. Since the requesting system knows
//          where the DHCP server is and it already has a current IP address the requests don't
//          need to use 0.0.0.0 and 255.255.255.255.
//
//
//  DHCP Message type to put or receive in option no 53.
//
//   1 = DHCP Discover message (DHCPDiscover).
//   2 = DHCP Offer message (DHCPOffer).
//   3 = DHCP Request message (DHCPRequest).
//   4 = DHCP Decline message (DHCPDecline).
//   5 = DHCP Acknowledgment message (DHCPAck).
//   6 = DHCP Negative Acknowledgment message (DHCPNack).
//   7 = DHCP Release message (DHCPRelease).
//   8 = DHCP Informational message (DHCPInform).
//
//  DISCOVER EXAMPLE
//      DHCP: DHCP Message Type         = 53 -> 1 DHCP Discover
//      DHCP: Parameter Request List    = 55 -> (Length: 3) 01  Subnet Mask
//                                                          03  Gateway
//                                                          06  DNS server
//      DHCP: End of field              = 255
//
//
//  OFFER EXAMPLE
//      DHCP: DHCP Message Type         = 53 -> 2 DHCP Offer
//      DHCP: Subnet Mask               = 01 -> 255.255.240.0
//      DHCP: Renewal Time Value (T1)   = 58 -> 8 Days,  0:00:00
//      DHCP: Rebinding Time Value (T2) = 59 -> 14 Days,  0:00:00
//      DHCP: IP Address Lease Time     = 51 -> 16 Days,  0:00:00
//      DHCP: DNS Server Identifier     = 06 -> 157.54.48.151
//      DHCP: Gateway (Router)          = 03 -> 157.54.48.1
//      DHCP: End of field              = 255
//
//
//  REQUEST EXAMPLE
//      DHCP: DHCP Message Type         = 53 -> 3 DHCP Request
//      DHCP: Parameter Request List    = 55 -> (Length: 3) 01  Subnet Mask
//                                                          03  Gateway
//                                                          06  DNS server
//      DHCP: Requested Address         = 50 -> 157.54.50.5
//      DHCP: Server Identifier         = 06 -> 157.54.48.151
//      DHCP: End of field              = 255
//
//
//  ACK EXAMPLE
//      DHCP: DHCP Message Type         = 53 -> 5 DHCP ACK
//      DHCP: Subnet Mask               = 01 -> 255.255.240.0
//      DHCP: Renewal Time Value (T1)   = 58 -> 8 Days,  0:00:00
//      DHCP: Rebinding Time Value (T2) = 59 -> 14 Days,  0:00:00
//      DHCP: IP Address Lease Time     = 51 -> 16 Days,  0:00:00
//      DHCP: DNS Server Identifier     = 06 -> 157.54.48.151
//      DHCP: Gateway (Router)          = 03 -> 157.54.48.1
//      DHCP: End of field              = 255
//
//
//  format option field
//
//  code + size + data[]                example 55 3 1 3 6
//
//*************************************************************************************************

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#define DHCP_GLOBAL
#include <include.h>

//-------------------------------------------------------------------------------------------------
//
//  Name:           DHCP_Init
//
//  Parameter(s):   void* pQ
//  Return:         void
//
//  Description:    Initialize the DHCP Client
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void DHCP_Init(void* pQ)
{

    DHCP_byState = DHCP_STATE_INITIAL;
    DHCP_pQ      = pQ;
}
//-------------------------------------------------------------------------------------------------
//
//  Name:           DHCP_Start
//
//  Parameter(s):   none
//  Return:         void
//
//  Description:    Start the DHCP Client
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
bool DHCP_Start(void)
{
    bool bStatus;


    DHCP_byState                    = DHCP_STATE_INITIAL;

    if(DHCP_byOST_Discover  != TIME_TIMER_nullptr) TIMER_Stop(&DHCP_byOST_Discover);
    if(DHCP_byOST_T1_Lease  != TIME_TIMER_nullptr) TIMER_Stop(&DHCP_byOST_T1_Lease);
    if(DHCP_byOST_T2_Rebind != TIME_TIMER_nullptr) TIMER_Stop(&DHCP_byOST_T2_Rebind);

    IP_Status.b.IP_IsValid           = NO;
    IP_DHCP_GatewayIP.dw             = 0x00000000;
    IP_DHCP_SubnetMask.dw            = 0x00000000;
    IP_DHCP_IP.dw                    = 0x00000000;
    IP_DHCP_DNS_IP.dw                = 0x00000000;
    DHCP_Xid.dw                      = TIME_Random(0xFFFFFFFF);

    sipr(IP_DHCP_IP.dw);

    if(SOCK_Socket(IP_SOCKET_DHCP, Sn_MR_UDP, DHCP_CLIENT_PORT, 0x00) == true)
    {
        bStatus = true;
    }
    else
    {
        bStatus = false;
    }

    return(bStatus);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DHCP_Process
//
//  Parameter(s):   void
//  Return:         void
//
//  Description:    Process the DHCP function
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
bool DHCP_Process(MSG_t* pMsg)
{
    uint8_t        Error;
    uint32_t      ServerAddr;
    int16_t        wServerPort;
    DHCP_Msg_t* pRX             = nullptr;
    bool        bStatus         = true;

    if(pMsg != nullptr)
    {
        switch(pMsg->byAction)
        {
            case DHCP_MSG_ACTION_TIME_OUT:
            {
                    DHCP_byState = DHCP_STATE_INITIAL;
                    SOCK_Close(IP_SOCKET_DHCP);

                    // Should close all socket.
                  #if (IP_DBG_DHCP == DEF_ENABLED)
                    DBG_Printf("DHCP request time out, DHCP will restart\n");
                  #endif
                break;
            }

            case DHCP_MSG_ACTION_LEASE_RENEWAL:
            {
                if(SOCK_Socket(IP_SOCKET_DHCP, Sn_MR_UDP, DHCP_CLIENT_PORT, 0x00) == true)
                {
                    DHCP_Request();
                  #if (IP_DBG_DHCP == DEF_ENABLED)
                    DBG_Printf("DHCP request send for lease renewal\n");
                  #endif
                }
                break;
            }

            case DHCP_MSG_ACTION_REBIND:
            {
                break;
            }
        }
        MemPut(&pMsg);
    }

    if(IP_Flag.b.DHCP_Mode == YES)
    {
        if(DHCP_byState == DHCP_STATE_INITIAL)
        {
            bStatus = DHCP_Discover();
        }
        else
        {
            if(sock_sr_read(DHCP_SOCKET) == SOCK_CLOSED)
            {
               if(DHCP_byState != DHCP_STATE_BOUND)
               {
                   DHCP_byState = DHCP_STATE_INITIAL;                          // reset DHCP state machine
               }
            }
            else
            {
                if(SOCK_GetRX_RSR(DHCP_SOCKET) > 0)
                {
                    pRX = MemGetAndClear(sizeof(DHCP_Msg_t), &Error);
                    if(pRX != nullptr)
                    {
                        SOCK_ReceivedFrom(DHCP_SOCKET, (uint8_t*)pRX, sizeof(DHCP_Msg_t), &ServerAddr.dw, &wServerPort);

                        if((pRX->byOp           == DHCP_BOOT_REPLY) &&
                           (pRX->MagicCookie.dw == DHCP_MAGIC_COOKIE) &&
                           (pRX->dwX_ID         == htonl(DHCP_Xid.dw)))
                        {
                            DHCP_ParseOption(pRX);

                            switch(DHCP_Options.byType)
                            {
                                case DHCP_OFFER:
                                {
                                    if(DHCP_byState == DHCP_STATE_DISCOVER)
                                    {
                                        if(TIMER_Restart(&DHCP_byOST_Discover) == TIME_TIMER_IS_RETRIGGED)
                                        {
                                            DHCP_ParseOffer(pRX);
                                            DHCP_Request();
                                            DHCP_byState = DHCP_STATE_OFFER_RECEIVED;
                                          #if (IP_DBG_DHCP == DEF_ENABLED)
                                            DBG_Printf("DHCP offer received and Request is sent\n");
                                          #endif
                                        }
                                    }
                                    break;
                                }

                                case DHCP_ACK:
                                {
                                    if(DHCP_byState == DHCP_STATE_OFFER_RECEIVED)
                                    {
                                        if(TIMER_Stop(&DHCP_byOST_Discover) == TIME_TIMER_IS_RELEASED)
                                        {
                                            DHCP_IsBound();
                                            SOCK_Close(DHCP_SOCKET);

                                          #if (IP_DBG_DHCP == DEF_ENABLED)
                                            DBG_Printf("DHCP ACK received and interface is bound\n");
                                            DBG_Printf("Host IP           %d.%d.%d.%d\n", IP_HostAddr.by.by0, IP_HostAddr.by.by1, IP_HostAddr.by.by2, IP_HostAddr.by.by3);
                                            DBG_Printf("SubNet mask IP    %d.%d.%d.%d\n", IP_SubnetMaskAddr.by.by0, IP_SubnetMaskAddr.by.by1, IP_SubnetMaskAddr.by.by2, IP_SubnetMaskAddr.by.by3);
                                            DBG_Printf("Default router IP %d.%d.%d.%d\n\n", IP_DefaultGatewayAddr.by.by0, IP_DefaultGatewayAddr.by.by1, IP_DefaultGatewayAddr.by.by2, IP_DefaultGatewayAddr.by.by3);
                                          #endif
                                        }
                                    }
                                    else if(DHCP_byState == DHCP_STATE_BOUND)
                                    {
                                        // check if in the ACK there is possibility of new value!
                                        DHCP_IsBound();
                                        SOCK_Close(DHCP_SOCKET);

                                      #if (IP_DBG_DHCP == DEF_ENABLED)
                                        DBG_Printf("DHCP lease renewal is accepted\n");
                                      #endif
                                    }
                                    break;
                                }

                                case DHCP_NACK:
                                {
                                    DHCP_byState = DHCP_STATE_INITIAL;                          // reset DHCP state machine
                                    TIMER_Stop(&DHCP_byOST_T1_Lease);                           // stop T1 & T2
                                    TIMER_Stop(&DHCP_byOST_T2_Rebind);
                                    SOCK_Close(DHCP_SOCKET);

                                  #if (IP_DBG_DHCP == DEF_ENABLED)
                                    DBG_Printf("DHCP request NACK received\n");
                                  #endif
                                    break;
                                }
                            }
                        }
                        MemPut(&pRX);
                    }
                }
            }
        }

        if(DHCP_byState == DHCP_STATE_BOUND)
        {
            bStatus = true;
        }
        else
        {
            bStatus = false;
        }
    }

    return(bStatus);       // Static IP mode
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DHCP_Discover
//
//  Parameter(s):   none
//  Return:         IP_PacketMsg_t*
//
//  Description:    Send the Discover message
//
//  Note(s):        Parameter to be zero's are not set because packet is already clear
//
//                  IP Frame Checksum      and
//                  IP Frame Time To Live  and
//                  IP Frame Protocol      must be set after UDP checksum is calculated
//
//-------------------------------------------------------------------------------------------------
bool DHCP_Discover(void)
{
    uint8_t                    Options;
    uint8_t                    Error;
    DHCP_Msg_t*             pTX         = nullptr;
    int32_t                   dwIP;
    int16_t                    wLen;
    bool                    bStatus;

    W5100_Init();               // This will start fresh
    bStatus = DHCP_Start();

    if(bStatus == true)
    {
        pTX = MemGetAndClear(sizeof(DHCP_Msg_t), &Error);

        if(pTX != nullptr)
        {
            //Setup Options
            Options = (DHCP_PUT_OPTION_CLIENT_IDENTIFIER | DHCP_PUT_OPTION_HOST_NAME | DHCP_PUT_OPTION_PL_DISCOVER);
            wLen      = DHCP_PutOption(&pTX->baOptions[0], Options, DHCP_DISCOVER);
            DHCP_PutHeader(pTX);

            // Send broadcasting packet
            dwIP = 0xFFFFFFFF;

            wLen = SOCK_SendTo(DHCP_SOCKET,
                               (uint8_t*)pTX,
                               (int16_t)((sizeof(DHCP_Msg_t) - DHCP_OPTION_IN_PACKET_SIZE) + wLen),
                               dwIP,
                               DHCP_SERVER_PORT);
            if(wLen == 0)
            {
              bStatus = false;
              #if (IP_DBG_DHCP == DEF_ENABLED)
                DBG_Printf("DHCP : Fatal Error(0).");
              #endif
            }
            else
            {

              #if (IP_DBG_DHCP == DEF_ENABLED)
                DBG_Printf("DHCP Discover sent\n");
              #endif

                DHCP_byState = DHCP_STATE_DISCOVER;

                TIMER_Start(&DHCP_byOST_Discover,
                            (int32_t)DHCP_OST_TIMEOUT,
                            DHCP_pQ,
                            IP_MSG_TYPE_DHCP_MANAGEMENT,
                            DHCP_MSG_ACTION_TIME_OUT);
            }

            MemPut(&pTX);
        }
        else
        {
            bStatus = false;
        }
    }

    return(bStatus);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DHCP_Request
//
//  Parameter(s):   void
//  Return:         none
//
//  Description:    Send back and Ack
//
//  Note(s):        this command is use after an offer and we it is time for lease renewal
//
//-------------------------------------------------------------------------------------------------
bool DHCP_Request(void)
{
    uint8_t            Options;
    uint8_t            Error;
    DHCP_Msg_t*     pTX         = nullptr;
    uint32_t          IP;
    int16_t            wLen;
    bool            bStatus     = true;

    pTX = MemGetAndClear(sizeof(DHCP_Msg_t), &Error);

    if(pTX != nullptr)
    {
        //Setup Options and dynamic part of DHCP
        Options = (DHCP_PUT_OPTION_CLIENT_IDENTIFIER | DHCP_PUT_OPTION_HOST_NAME | DHCP_PUT_OPTION_PL_REQUEST);

        if(DHCP_byState < DHCP_STATE_BOUND)
        {
            Options |= DHCP_PUT_OPTION_REQUESTED_CLIENT_IP;
        }
        else
        {
            Options |= DHCP_PUT_OPTION_SERVER_IP;
        }

        wLen = DHCP_PutOption(&pTX->baOptions[0], Options, DHCP_REQUEST);
        DHCP_PutHeader(pTX);

        // Send broadcasting packet
        if(DHCP_byState < DHCP_STATE_BOUND)
        {
            IP.dw = 0xFFFFFFFF;
        }
        else
        {
            IP.dw = IP_DHCP_IP.dw;
        }

        wLen = SOCK_SendTo(DHCP_SOCKET,
                           (uint8_t*)pTX,
                           (int16_t)((sizeof(DHCP_Msg_t) - DHCP_OPTION_IN_PACKET_SIZE) + wLen),
                           IP.dw,
                           DHCP_SERVER_PORT);
        if(wLen == 0)
        {
          bStatus = false;
          #if (IP_DBG_DHCP == DEF_ENABLED)
            DBG_Printf("DHCP : Fatal Error(0).");
          #endif
        }

        MemPut(&pTX);
    }
    else
    {
       bStatus = false;
    }

    return(bStatus);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DHCP_ParseOffer
//
//  Parameter(s):   IP_PacketMsg_t*   pRX
//  Return:         none
//
//  Description:    Get info from the first offer
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void DHCP_ParseOffer(DHCP_Msg_t* pRX)
{
    DHCP_Options.dwClientIP = ntohl(pRX->dwYourIP_Addr);
    DHCP_Options.dwServerIP = ntohl(pRX->dwServerIP_Addr);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DHCP_IsBound
//
//  Parameter(s):   void
//  Return:         void
//
//  Description:    Process ACK or NACK
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void DHCP_IsBound(void)
{
    IP_DHCP_IP.dw               = DHCP_Options.dwClientIP;
    IP_DHCP_SubnetMask.dw       = DHCP_Options.dwSubnetMaskIP;
    IP_DHCP_GatewayIP.dw        = DHCP_Options.dwGatewayIP;
    IP_DHCP_DNS_IP.dw           = DHCP_Options.dwDNS_ServerIP;

    gar(IP_DHCP_GatewayIP.dw);
    subr(IP_DHCP_SubnetMask.dw);
    sipr(IP_DHCP_IP.dw);

    DHCP_byState                = DHCP_STATE_BOUND;
    IP_Status.b.IP_IsValid      = YES;

    TIMER_Stop(&DHCP_byOST_T1_Lease);                               // make sure timer T1 & T2 are stop
    TIMER_Stop(&DHCP_byOST_T2_Rebind);

    TIMER_Start(&DHCP_byOST_T1_Lease,
                DHCP_Options.dwLeaseTime >> 1,
                DHCP_pQ,
                IP_MSG_TYPE_DHCP_MANAGEMENT,
                DHCP_MSG_ACTION_LEASE_RENEWAL);                     // Start the renewal timer for 50% of the total lease

    TIMER_Start(&DHCP_byOST_T2_Rebind,
                (DHCP_Options.dwLeaseTime >> 1) + (DHCP_Options.dwLeaseTime >> 2) + (DHCP_Options.dwLeaseTime >> 4),
                DHCP_pQ,
                IP_MSG_TYPE_DHCP_MANAGEMENT,
                DHCP_MSG_ACTION_REBIND);                            // Start the rebind timer for 87% of the total lease
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DHCP_ParseOption
//
//  Parameter(s):   none
//  Return:         void
//
//  Description:    Process the Option field and extract any relevant info for us
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void DHCP_ParseOption(DHCP_Msg_t* pRX)
{
    uint8_t*   pPtr;

    pPtr = (uint8_t*)&pRX->baOptions;


    while((*pPtr != DHCP_OPTION_END_OF_FIELD) && (*pPtr != DHCP_OPTION_PADDING))
    {
        switch(*pPtr)
        {
            case DHCP_OPTION_SUBNET_MASK:
                DHCP_Options.dwSubnetMaskIP = ntohl(*(int32_t*)(pPtr + 2));
                break;

            case DHCP_OPTION_GATEWAY:
                DHCP_Options.dwGatewayIP = ntohl(*(int32_t*)(pPtr + 2));
                break;

            case DHCP_OPTION_DNS_SERVER:
                DHCP_Options.dwDNS_ServerIP = ntohl(*(int32_t*)(pPtr + 2));
                break;

            case DHCP_OPTION_CLIENT_IP:
                DHCP_Options.dwClientIP = ntohl(*(int32_t*)(pPtr + 2));
                break;

            case DHCP_OPTION_LEASE_TIME:
                DHCP_Options.dwLeaseTime = ntohl(*(int32_t*)(pPtr + 2));
                break;

            case DHCP_OPTION_MESSAGE_TYPE:
                DHCP_Options.byType = *(pPtr + 2);
                break;

            case DHCP_OPTION_SERVER_IP:
                DHCP_Options.dwServerIP = ntohl(*(int32_t*)(pPtr + 2));
                break;
        }

        if(*pPtr == DHCP_OPTION_PADDING)
        {
            pPtr++;
        }
        else
        {
            pPtr += pPtr[1] + 2;
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DHCP_PutOption
//
//  Parameter(s):   uint8_t*   pPtr        Pointer on option field in packet
//                  uint8_t    Options   Option(s) in Bit position to put in packet
//  Return:         int16_t    wLen        Lenght of the option(s)
//
//  Description:    Put option specify  the flag
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
int16_t DHCP_PutOption(uint8_t* pPtr, uint8_t Options, uint8_t Message)
{
    int16_t    wLen;
    uint8_t*   pStart;

    pStart = pPtr;

    *pPtr++ = DHCP_OPTION_MESSAGE_TYPE;
    *pPtr++ = 1;
    *pPtr++ = Message;

    if(byOptions & DHCP_PUT_OPTION_CLIENT_IDENTIFIER)
    {
        *pPtr++ = DHCP_OPTION_CLIENT_IDENTIFIER;
        *pPtr++ = 7;
        *pPtr++ = 1;
        LIB_memcpy(pPtr, &IP_MAC, 6);
        pPtr += 6;
    }

    if(byOptions & DHCP_PUT_OPTION_PL_DISCOVER)
    {
        LIB_memcpy(pPtr, DHCP_baOPL_Discover, sizeof(DHCP_baOPL_Discover));
        pPtr += sizeof(DHCP_baOPL_Discover);
    }

    if(byOptions & DHCP_PUT_OPTION_PL_REQUEST)
    {
        LIB_memcpy(pPtr, DHCP_baOPL_Request, sizeof(DHCP_baOPL_Request));
        pPtr += sizeof(DHCP_baOPL_Request);
    }

    if(byOptions & DHCP_PUT_OPTION_REQUESTED_CLIENT_IP)
    {
        *pPtr++ = DHCP_OPTION_CLIENT_IP;
        *pPtr++ = 4;
        *((int32_t*)pPtr) = htonl(DHCP_Options.dwClientIP);
        pPtr += 4;
    }

    if(byOptions & DHCP_PUT_OPTION_HOST_NAME)
    {
        uint8_t i;

        *pPtr++ = DHCP_OPTION_HOST_NAME;

        *pPtr++ = (uint8_t)(LIB_strlen(IP_HOST_NAME) + 9);              // length of hostname + 6
        LIB_strcpy(pPtr, IP_HOST_NAME);
        pPtr += LIB_strlen(IP_HOST_NAME);

        for(i = 3; i < 6; i++)                              // Add last 3 uint8_t of the MAC in HEXA -> Ascii at the end of label
        {
            *pPtr++  = '_';
            *pPtr    = (uint8_t)((IP_MAC.ba[i] >> 4) + 0x30);
            *pPtr   += (uint8_t)((*pPtr > 0x39) ? 7 : 0);
            pPtr++;
            *pPtr    = (uint8_t)((IP_MAC.ba[i] & 0x0F) + 0x30);
            *pPtr   += (uint8_t)((*pPtr > 0x39) ? 7 : 0);
            pPtr++;
        }
    }

    if(byOptions & DHCP_PUT_OPTION_SERVER_IP)
    {
        *pPtr++ = DHCP_OPTION_SERVER_IP;
        *pPtr++ = 4;
        *((int32_t*)pPtr) = htonl(DHCP_Options.dwServerIP);
        pPtr += 4;
    }

    *pPtr++ = DHCP_OPTION_END_OF_FIELD;

    return((int16_t)(pPtr - pStart));
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DHCP_PutHeader
//
//  Parameter(s):   IP_PacketMsg_t*     pTX
//  Return:         void
//
//  Description:    Put in header everything static
//
//  Requirement:    All other data must be already in the header
//
//-------------------------------------------------------------------------------------------------
void DHCP_PutHeader(DHCP_Msg_t* pTX)
{
    // Setup DHCP header
    pTX->byOp            = DHCP_BOOT_REQUEST;
    pTX->byHtype         = DHCP_HARWARE_TYPE_ETHERNET_100;
    pTX->byHlen          = DHCP_HARWARE_ADDRESS_LENGHT;
    pTX->dwX_ID          = htonl(DHCP_Xid.dw);
    pTX->MagicCookie.dw  = DHCP_MAGIC_COOKIE;

    if(DHCP_byState < DHCP_STATE_BOUND)
    {
        pTX->wFlags  = htons(DHCP_FLAGS_BROADCAST);
    }
    else
    {
        pTX->wFlags = 0;     // For Unicast
        pTX->dwClientIP_Addr = htonl(IP_DHCP_IP.dw);
    }

    LIB_memcpy(pTX->baClientHardware, IP_MAC.ba, 6);
}

//-------------------------------------------------------------------------------------------------


