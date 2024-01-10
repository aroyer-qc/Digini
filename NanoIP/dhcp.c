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
//-------------------------------------------------------------------------------------------------
void DHCP_Init(void* pQ)
{

    DHCP_State = DHCP_STATE_INITIAL;
    DHCP_pQ    = pQ;
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
//-------------------------------------------------------------------------------------------------
bool DHCP_Start(void)
{
    bool Status = false;

    DHCP_State = DHCP_STATE_INITIAL;

    if(DHCP_OST_Discover  != TIME_TIMER_NULL) TIMER_Stop(&DHCP_OST_Discover);
    if(DHCP_OST_T1_Lease  != TIME_TIMER_NULL) TIMER_Stop(&DHCP_OST_T1_Lease);
    if(DHCP_OST_T2_Rebind != TIME_TIMER_NULL) TIMER_Stop(&DHCP_OST_T2_Rebind);

    IP_Status.b.IP_IsValid        = false;
    IP_DHCP_GatewayIP             = IP_ADDR(0,0,0,0);
    IP_DHCP_SubnetMask            = IP_ADDR(0,0,0,0);
    IP_DHCP_IP                    = IP_ADDR(0,0,0,0);
    IP_DHCP_DNS_IP                = IP_ADDR(0,0,0,0);
    DHCP_Xid                      = GET_Random();

    sipr(IP_DHCP_IP);

    if(SOCK_Socket(IP_SOCKET_DHCP, Sn_MR_UDP, DHCP_CLIENT_PORT, 0x00) == true)
    {
        Status = true;
    }

    return Status;
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
//-------------------------------------------------------------------------------------------------
bool DHCP_Process(MSG_t* pMsg)
{
    IP_Address_t ServerAddr;
    uint16_t     ServerPort;
    DHCP_Msg_t*  pRX             = nullptr;
    bool         Status          = false;

    if(pMsg != nullptr)
    {
        switch(pMsg->Action)
        {
            case DHCP_MSG_ACTION_TIME_OUT:
            {
                    DHCP_State = DHCP_STATE_INITIAL;
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

    if(DHCP_Mode == true)
    {
        if(DHCP_State == DHCP_STATE_INITIAL)
        {
            Status = DHCP_Discover();
        }
        else
        {
            if(sock_sr_read(DHCP_SOCKET) == SOCK_CLOSED)
            {
               if(DHCP_State != DHCP_STATE_BOUND)
               {
                   DHCP_State = DHCP_STATE_INITIAL;                          // reset DHCP state machine
               }
            }
            else
            {
                if(SOCK_GetRX_RSR(DHCP_SOCKET) > 0)
                {
                    pRX = (DHCP_Msg_t*)pMemory->AllocAndClear(sizeof(DHCP_Msg_t));
                    
                    if(pRX != nullptr)
                    {
                        SOCK_ReceivedFrom(DHCP_SOCKET, (uint8_t*)pRX, sizeof(DHCP_Msg_t), &ServerAddr, &ServerPort);

                        if((pRX->Op          == DHCP_BOOT_REPLY) &&
                           (pRX->MagicCookie == DHCP_MAGIC_COOKIE) &&
                           (pRX->X_ID        == htonl(DHCP_Xid)))
                        {
                            DHCP_ParseOption(pRX);

                            switch(DHCP_Options.Type)
                            {
                                case DHCP_OFFER:
                                {
                                    if(DHCP_State == DHCP_STATE_DISCOVER)
                                    {
                                        if(TIMER_Restart(&DHCP_OST_Discover) == TIME_TIMER_IS_RETRIGGED)
                                        {
                                            DHCP_ParseOffer(pRX);
                                            DHCP_Request();
                                            DHCP_State = DHCP_STATE_OFFER_RECEIVED;
                                          #if (IP_DBG_DHCP == DEF_ENABLED)
                                            DBG_Printf("DHCP offer received and Request is sent\n");
                                          #endif
                                        }
                                    }
                                    break;
                                }

                                case DHCP_ACK:
                                {
                                    if(DHCP_State == DHCP_STATE_OFFER_RECEIVED)
                                    {
                                        if(TIMER_Stop(&DHCP_OST_Discover) == TIME_TIMER_IS_RELEASED)
                                        {
                                            DHCP_IsBound();
                                            SOCK_Close(DHCP_SOCKET);

                                          #if (IP_DBG_DHCP == DEF_ENABLED)
                                            DBG_Printf("DHCP ACK received and interface is bound\n");
                                            DBG_Printf("Host IP           %d.%d.%d.%d\n",   uint8_t(IP_HostAddr           >> 24), uint8_t(IP_HostAddr           >> 16), uint8_t(IP_HostAddr           >> 8), uint8_t(IP_HostAddr));
                                            DBG_Printf("SubNet mask IP    %d.%d.%d.%d\n",   uint8_t(IP_SubnetMaskAddr     >> 24), uint8_t(IP_SubnetMaskAddr     >> 16), uint8_t(IP_SubnetMaskAddr     >> 8), uint8_t(IP_SubnetMaskAddr));
                                            DBG_Printf("Default router IP %d.%d.%d.%d\n\n", uint8_t(IP_DefaultGatewayAddr >> 24), uint8_t(IP_DefaultGatewayAddr >> 16), uint8_t(IP_DefaultGatewayAddr >> 8), uint8_t(IP_DefaultGatewayAddr));
                                          #endif
                                        }
                                    }
                                    else if(DHCP_State == DHCP_STATE_BOUND)
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
                                    DHCP_State = DHCP_STATE_INITIAL;                          // reset DHCP state machine
                                    TIMER_Stop(&DHCP_OST_T1_Lease);                           // stop T1 & T2
                                    TIMER_Stop(&DHCP_OST_T2_Rebind);
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

        if(DHCP_State == DHCP_STATE_BOUND)
        {
            Status = true;
        }
    }

    return Status;       // Static IP mode
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
    uint8_t      Options;
    DHCP_Msg_t*  pTX         = nullptr;
    IP_Address_t IP_Address;
    size_t       Len;
    bool         Status;

    W5100_Init();               // This will start fresh
    Status = DHCP_Start();

    if(Status == true)
    {
        pTX = (DHCP_Msg_t*)pMemory->AllocAndClear(sizeof(DHCP_Msg_t));

        if(pTX != nullptr)
        {
            //Setup Options
            Options = (DHCP_PUT_OPTION_CLIENT_IDENTIFIER | DHCP_PUT_OPTION_HOST_NAME | DHCP_PUT_OPTION_PL_DISCOVER);
            Len     = DHCP_PutOption(&pTX->Options[0], Options, DHCP_DISCOVER);
            DHCP_PutHeader(pTX);

            // Send broadcasting packet
            IP_Address = IP_ADDR(255,255,255,255);

            Len = SOCK_SendTo(DHCP_SOCKET,
                              (uint8_t*)pTX,
                              uint16_t((sizeof(DHCP_Msg_t) - DHCP_OPTION_IN_PACKET_SIZE) + wLen),
                              IP_Address,
                              DHCP_SERVER_PORT);
            if(Len == 0)
            {
              Status = false;
              #if (IP_DBG_DHCP == DEF_ENABLED)
                DBG_Printf("DHCP : Fatal Error(0).");
              #endif
            }
            else
            {

              #if (IP_DBG_DHCP == DEF_ENABLED)
                DBG_Printf("DHCP Discover sent\n");
              #endif

                DHCP_State = DHCP_STATE_DISCOVER;

                TIMER_Start(&DHCP_OST_Discover,
                            uint32_t(DHCP_OST_TIMEOUT),
                            DHCP_pQ,
                            IP_MSG_TYPE_DHCP_MANAGEMENT,
                            DHCP_MSG_ACTION_TIME_OUT);
            }

            MemPut(&pTX);
        }
        else
        {
            Status = false;
        }
    }

    return Status;
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
    uint8_t         Options;
    DHCP_Msg_t*     pTX         = nullptr;
    IP_Address_t    IP_Address;
    size_t          Len;
    bool            Status     = true;

    pTX = (DHCP_Msg_t*)pMemory->AllocAndClear(sizeof(DHCP_Msg_t));

    if(pTX != nullptr)
    {
        //Setup Options and dynamic part of DHCP
        Options = (DHCP_PUT_OPTION_CLIENT_IDENTIFIER | DHCP_PUT_OPTION_HOST_NAME | DHCP_PUT_OPTION_PL_REQUEST);

        if(DHCP_State < DHCP_STATE_BOUND)
        {
            Options |= DHCP_PUT_OPTION_REQUESTED_CLIENT_IP;
        }
        else
        {
            Options |= DHCP_PUT_OPTION_SERVER_IP;
        }

        Len = DHCP_PutOption(&pTX->Options[0], Options, DHCP_REQUEST);
        DHCP_PutHeader(pTX);

        // Send broadcasting packet
        if(DHCP_State < DHCP_STATE_BOUND)
        {
        // this is the global one i think check previous version
            IP_Address = IP_ADDRESS(255,255,255,255);
        }
        else
        {
            IP_Address = IP_DHCP_IP;
        }

        Len = SOCK_SendTo(DHCP_SOCKET,
                           (uint8_t*)pTX,
                           uint16_t((sizeof(DHCP_Msg_t) - DHCP_OPTION_IN_PACKET_SIZE) + Len),
                           IP_Address,
                           DHCP_SERVER_PORT);
        if(Len == 0)
        {
          Status = false;
          #if (IP_DBG_DHCP == DEF_ENABLED)
            DBG_Printf("DHCP : Fatal Error(0).");
          #endif
        }

        MemPut(&pTX);
    }
    else
    {
       Status = false;
    }

    return Status;
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
//-------------------------------------------------------------------------------------------------
void DHCP_ParseOffer(DHCP_Msg_t* pRX)
{
    DHCP_Options.ClientIP = ntohl(pRX->YourIP_Addr);
    DHCP_Options.ServerIP = ntohl(pRX->ServerIP_Addr);
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
//-------------------------------------------------------------------------------------------------
void DHCP_IsBound(void)
{
    IP_DHCP_IP         = DHCP_Options.ClientIP;
    IP_DHCP_SubnetMask = DHCP_Options.SubnetMaskIP;
    IP_DHCP_GatewayIP  = DHCP_Options.GatewayIP;
    IP_DHCP_DNS_IP     = DHCP_Options.DNS_ServerIP;

    gar(IP_DHCP_GatewayIP);
    subr(IP_DHCP_SubnetMask);
    sipr(IP_DHCP_IP);

    DHCP_State             = DHCP_STATE_BOUND;
    IP_Status.b.IP_IsValid = true;

    TIMER_Stop(&DHCP_OST_T1_Lease);                               // make sure timer T1 & T2 are stop
    TIMER_Stop(&DHCP_OST_T2_Rebind);

    TIMER_Start(&DHCP_OST_T1_Lease,
                DHCP_Options.LeaseTime >> 1,
                DHCP_pQ,
                IP_MSG_TYPE_DHCP_MANAGEMENT,
                DHCP_MSG_ACTION_LEASE_RENEWAL);                     // Start the renewal timer for 50% of the total lease

    TIMER_Start(&DHCP_OST_T2_Rebind,
                (DHCP_Options.LeaseTime >> 1) + (DHCP_Options.LeaseTime >> 2) + (DHCP_Options.LeaseTime >> 4),
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
    uint8_t* pPtr;

    pPtr = (uint8_t*)&pRX->Options;


    while((*pPtr != DHCP_OPTION_END_OF_FIELD) && (*pPtr != DHCP_OPTION_PADDING))
    {
        switch(*pPtr)
        {
            case DHCP_OPTION_SUBNET_MASK:
                DHCP_Options.SubnetMaskIP = ntohl(*(uint32_t*)(pPtr + 2));
                break;

            case DHCP_OPTION_GATEWAY:
                DHCP_Options.GatewayIP = ntohl(*(uint32_t*)(pPtr + 2));
                break;

            case DHCP_OPTION_DNS_SERVER:
                DHCP_Options.DNS_ServerIP = ntohl(*(uint32_t*)(pPtr + 2));
                break;

            case DHCP_OPTION_CLIENT_IP:
                DHCP_Options.ClientIP = ntohl(*(uint32_t*)(pPtr + 2));
                break;

            case DHCP_OPTION_LEASE_TIME:
                DHCP_Options.LeaseTime = ntohl(*(uint32_t*)(pPtr + 2));
                break;

            case DHCP_OPTION_MESSAGE_TYPE:
                DHCP_Options.Type = *(pPtr + 2);
                break;

            case DHCP_OPTION_SERVER_IP:
                DHCP_Options.ServerIP = ntohl(*(uint32_t*)(pPtr + 2));
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
//  Parameter(s):   uint8_t*   pPtr         Pointer on option field in packet
//                  uint8_t    Options      Option(s) in Bit position to put in packet
//                  uint8_t    Message
//  Return:         size_t     Len          Lenght of the option(s)
//
//  Description:    Put option specify the flag
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
size_t DHCP_PutOption(uint8_t* pPtr, uint8_t Options, uint8_t Message)
{
    size_t     Len;
    uint8_t*   pStart;

    pStart = pPtr;

    *pPtr++ = DHCP_OPTION_MESSAGE_TYPE;
    *pPtr++ = 1;
    *pPtr++ = Message;

    if(Options & DHCP_PUT_OPTION_CLIENT_IDENTIFIER)
    {
        *pPtr++ = DHCP_OPTION_CLIENT_IDENTIFIER;
        *pPtr++ = 7;
        *pPtr++ = 1;
        LIB_memcpy(pPtr, &IP_MAC, 6);
        pPtr += 6;
    }

    if(Options & DHCP_PUT_OPTION_PL_DISCOVER)
    {
        LIB_memcpy(pPtr, DHCP_OPL_Discover, sizeof(DHCP_OPL_Discover));
        pPtr += sizeof(DHCP_OPL_Discover);
    }

    if(byOptions & DHCP_PUT_OPTION_PL_REQUEST)
    {
        LIB_memcpy(pPtr, DHCP_OPL_Request, sizeof(DHCP_OPL_Request));
        pPtr += sizeof(DHCP_OPL_Request);
    }

    if(Options & DHCP_PUT_OPTION_REQUESTED_CLIENT_IP)
    {
        *pPtr++ = DHCP_OPTION_CLIENT_IP;
        *pPtr++ = 4;
        *((int32_t*)pPtr) = htonl(DHCP_Options.ClientIP);
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
            *pPtr    = (uint8_t)((IP_MAC.[i] >> 4) + 0x30);
            *pPtr   += (uint8_t)((*pPtr > 0x39) ? 7 : 0);
            pPtr++;
            *pPtr    = (uint8_t)((IP_MAC.[i] & 0x0F) + 0x30);
            *pPtr   += (uint8_t)((*pPtr > 0x39) ? 7 : 0);
            pPtr++;
        }
    }

    if(Options & DHCP_PUT_OPTION_SERVER_IP)
    {
        *pPtr++ = DHCP_OPTION_SERVER_IP;
        *pPtr++ = 4;
        *((uint32_t*)pPtr) = htonl(DHCP_Options.ServerIP);
        pPtr += 4;
    }

    *pPtr++ = DHCP_OPTION_END_OF_FIELD;

    return uint16_t(pPtr - pStart);
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
    pTX->Op             = DHCP_BOOT_REQUEST;
    pTX->Htype          = DHCP_HARWARE_TYPE_ETHERNET_100;
    pTX->Hlen           = DHCP_HARWARE_ADDRESS_LENGHT;
    pTX->X_ID           = htonl(DHCP_Xid);
    pTX->MagicCookie = DHCP_MAGIC_COOKIE;

    if(DHCP_State < DHCP_STATE_BOUND)
    {
        pTX->Flags  = htons(DHCP_FLAGS_BROADCAST);
    }
    else
    {
        pTX->Flags = 0;     // For Unicast
        pTX->ClientIP_Addr = htonl(IP_DHCP_IP);
    }

    LIB_memcpy(pTX->ClientHardware, IP_MAC., 6);
}

//-------------------------------------------------------------------------------------------------


