//-------------------------------------------------------------------------------------------------
//
//  File : dhcp.cpp
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
//  DHCP - Dynamic Host Configuration Protocol
//
//
//  UDP Ports 67 and 68
//      Common Use
//      Port 67 Bootps
//      Port 68 Bootpc
//
//          Typically this traffic is related to normal DHCP operation.
//          DHCP (Dynamic Host Configuration Protocol) is how your computer gets its unique
//          IP address. When a system starts up on a network it must first request an IP address
//          (assume it is not using a static IP address), and it does this by broadcasting a
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
//          as a request, followed by a reply
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

#include <include.h>

//-------------------------------------------------------------------------------------------------
// 
//-------------------------------------------------------------------------------------------------

const uint8_tNetDHCP::OPL_Discover[8] = // OPL stand for option list
{ 
    55,        // Parameter list
    6,         // Size
    1,         // Subnet Mask
    3,         // Gateway
    6,         // DNS Server
    15,        // Domain Name
    58,        // DHCP T1 Value
    59};       // DHCP T2 Value

const uint8_t NetDHCP::OPL_Request[10] =
{
    55,        // Parameter list
    8,         // Size
    1,         // Subnet Mask
    3,         // Gateway
    6,         // DNS Server
    15,        // Domain Name
    58,        // DHCP T1 Value
    59,        // DHCP T2 Value
    31,        // Perform Router Discovery
    33};       // Static Route

//-------------------------------------------------------------------------------------------------
//
//  Name:           Initialize
//
//  Parameter(s):   void* pQ
//  Return:         void
//
//  Description:    Initialize the DHCP Client
//
//-------------------------------------------------------------------------------------------------
void NetDHCP::Initialize(void* pQ)
{
    nOS_Error;

    m_State = DHCP_STATE_INITIAL;
    m_pQ    = pQ;

    Error = nOS_TimerCreate(&m_TimerDiscover,  nullptr, nullptr, DHCP_MSG_ACTION_TIME_OUT, NOS_TIMER_ONE_SHOT);
    Error = nOS_TimerCreate(&m_TimerT1_Lease,  nullptr, nullptr, 0, NOS_TIMER_ONE_SHOT);
    Error = nOS_TimerCreate(&m_TimerT2_Rebind, nullptr, nullptr, 0, NOS_TIMER_ONE_SHOT);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Start
//
//  Parameter(s):   None
//  Return:         bool
//
//  Description:    Start the DHCP Client
//
//-------------------------------------------------------------------------------------------------
bool NetDHCP::Start(void)
{
    bool Status = false;

    m_State = DHCP_STATE_INITIAL;

    if(nOS_TimerIsRunning(m_TimerDiscover)  == true) nOS_TimerStop(&m_TimerDiscover,  true);
    if(nOS_TimerIsRunning(m_TimerT1_Lease)  == true) nOS_TimerStop(&m_TimerT1_Lease,  true);
    if(nOS_TimerIsRunning(m_TimerT2_Rebind) == true) nOS_TimerStop(&m_TimerT2_Rebind, true);

    IpIP->SetIP_Valid(false);
    IP_DHCP_GatewayIP   = IP_ADDRESS(0,0,0,0);
    IP_DHCP_SubnetMask  = IP_ADDRESS(0,0,0,0);
    IP_DHCP_IP          = IP_ADDRESS(0,0,0,0);
    IP_DHCP_DNS_IP      = IP_ADDRESS(0,0,0,0);
    m_Xid               = GET_Random();

    sipr(IP_DHCP_IP);           // w5100 stuff

    if(SOCK_Socket(IP_SOCKET_DHCP, Sn_MR_UDP, DHCP_CLIENT_PORT, 0x00) == true)
    {
        Status = true;
    }

    return Status;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Process
//
//  Parameter(s):   void
//  Return:         bool
//
//  Description:    Process the DHCP function
//
//-------------------------------------------------------------------------------------------------
bool NetDHCP::Process(DHCP_Msg_t* pMsg)
{
    IP_Address_t ServerAddress;
    IP_Port_t    ServerPort;
    DHCP_Msg_t*  pRX             = nullptr;
    bool         Status          = false;

    if(pMsg != nullptr)
    {
        switch(pMsg->Action)
        {
            case DHCP_MSG_ACTION_TIME_OUT:
            {
                    m_State = DHCP_STATE_INITIAL;
                    SOCK_Close(IP_SOCKET_DHCP);

                    // Should close all socket.
                  #if (IP_DBG_DHCP == DEF_ENABLED)
                    DBG_Printf("DHCP request time out, DHCP will restart\n");
                  #endif
            }
            break;

            case DHCP_MSG_ACTION_LEASE_RENEWAL:
            {
                if(SOCK_Socket(IP_SOCKET_DHCP, Sn_MR_UDP, DHCP_CLIENT_PORT, 0x00) == true)
                {
                    Request();
                  #if (IP_DBG_DHCP == DEF_ENABLED)
                    DBG_Printf("DHCP request send for lease renewal\n");
                  #endif
                }
            }
            break;

            case DHCP_MSG_ACTION_REBIND:
            {
            }
            break;
        }
        
        pMemory->Free((void**)&pMsg);
    }

    if(m_Mode == true)
    {
        if(m_State == DHCP_STATE_INITIAL)
        {
            Status = Discover();
        }
        else
        {
            if(sock_sr_read(DHCP_SOCKET) == SOCK_CLOSED)
            {
               if(m_State != DHCP_STATE_BOUND)
               {
                   m_State = DHCP_STATE_INITIAL;                          // reset DHCP state machine
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

                            switch(m_Options.Type)
                            {
                                case DHCP_OPTION_OFFER:
                                {
                                    if(m_State == DHCP_STATE_DISCOVER)
                                    {
                                        if(nOS_TimerIsRunning(&m_TimerDiscover) == true)
                                        {
                                            ParseOffer(pRX);
                                            Request();
                                            m_State = DHCP_STATE_OFFER_RECEIVED;
                                          #if (IP_DBG_DHCP == DEF_ENABLED)
                                            DBG_Printf("DHCP offer received and Request is sent\n");
                                          #endif
                                        }

                                        nOS_TimerRestart(&m_TimerDiscover, DHCP_MSG_ACTION_TIME_OUT);
                                    }
                                }
                                break;

                                case DHCP_OPTION_ACK:
                                {
                                    if(m_State == DHCP_STATE_OFFER_RECEIVED)
                                    {
                                        nOS_TimerStop(&m_TimerDiscover, true);
                                        IsBound();
                                        SOCK_Close(DHCP_SOCKET);

                                      #if (IP_DBG_DHCP == DEF_ENABLED)
                                        DBG_Printf("DHCP ACK received and interface is bound\n");
                                        DBG_Printf("Host IP           %d.%d.%d.%d\n",   uint8_t(IP_HostAddress           >> 24), uint8_t(IP_HostAddress           >> 16), uint8_t(IP_HostAddress           >> 8), uint8_t(IP_HostAddress));
                                        DBG_Printf("SubNet mask IP    %d.%d.%d.%d\n",   uint8_t(IP_SubnetMaskAddress     >> 24), uint8_t(IP_SubnetMaskAddress     >> 16), uint8_t(IP_SubnetMaskAddress     >> 8), uint8_t(IP_SubnetMaskAddress));
                                        DBG_Printf("Default router IP %d.%d.%d.%d\n\n", uint8_t(IP_DefaultGatewayAddress >> 24), uint8_t(IP_DefaultGatewayAddress >> 16), uint8_t(IP_DefaultGatewayAddress >> 8), uint8_t(IP_DefaultGatewayAddress));
                                      #endif
                                    }
                                    else if(m_State == DHCP_STATE_BOUND)
                                    {
                                        // check if in the ACK there is possibility of new value!
                                        IsBound();
                                        SOCK_Close(DHCP_SOCKET);

                                      #if (IP_DBG_DHCP == DEF_ENABLED)
                                        DBG_Printf("DHCP lease renewal is accepted\n");
                                      #endif
                                    }
                                }
                                break;

                                case DHCP_OPTION_NACK:
                                {
                                    m_State = DHCP_STATE_INITIAL;                          // reset DHCP state machine
                                    nOS_TimerStop(&m_TimerT1_Lease,  true);                // stop T1 & T2
                                    nOS_TimerStop(&m_TimerT2_Rebind, true);
                                    SOCK_Close(DHCP_SOCKET);

                                  #if (IP_DBG_DHCP == DEF_ENABLED)
                                    DBG_Printf("DHCP request NACK received\n");
                                  #endif
                                }
                                break;
                            }
                        }
                        
                        pMemory->Free((void**)&pRX);
                    }
                }
            }
        }

        if(m_State == DHCP_STATE_BOUND)
        {
            Status = true;
        }
    }

    return Status;       // Static IP mode
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Discover
//
//  Parameter(s):   None
//  Return:         bool
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
bool NetDHCP::Discover(void)
{
    uint8_t      Options;
    DHCP_Msg_t*  pTX         = nullptr;
    IP_Address_t IP_Address;
    size_t       Length;
    bool         Status;

    W5100_Init();               // This will start fresh
    Status = Start();

    if(Status == true)
    {
        pTX = (DHCP_Msg_t*)pMemory->AllocAndClear(sizeof(DHCP_Msg_t));

        if(pTX != nullptr)
        {
            //Setup Options
            Options = (DHCP_PUT_OPTION_CLIENT_IDENTIFIER | DHCP_PUT_OPTION_HOST_NAME | DHCP_PUT_OPTION_PL_DISCOVER);
            Length  = PutOption(&pTX->Options[0], Options, DHCP_OPTION_DISCOVER);
            PutHeader(pTX);

            // Send broadcasting packet
            IP_Address = IP_ADDRESS(255,255,255,255);

            Length = SOCK_SendTo(DHCP_SOCKET,
                              (uint8_t*)pTX,
                              (sizeof(DHCP_Msg_t) - DHCP_OPTION_IN_PACKET_SIZE) + Length),
                              IP_Address,
                              DHCP_SERVER_PORT);
            if(Length == 0)
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

                m_State = DHCP_STATE_DISCOVER;

                nOS_TimerStart(&m_TimerDiscover);
                /*
                TIMER_Start(,
                            uint32_t(m_OST_TIMEOUT),
                            m_pQ,
                            IP_MSG_TYPE_DHCP_MANAGEMENT,
                            DHCP_MSG_ACTION_TIME_OUT);
                            */
            }

            pMemory->Free((void**)&pTX);
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
//  Name:           Request
//
//  Parameter(s):   void
//  Return:         bool
//
//  Description:    Send back and Ack
//
//  Note(s):        this command is use after an offer and we it is time for lease renewal
//
//-------------------------------------------------------------------------------------------------
bool NetDHCP::Request(void)
{
    uint8_t         Options;
    DHCP_Msg_t*     pTX         = nullptr;
    IP_Address_t    IP_Address;
    size_t          Length;
    bool            Status     = true;

    pTX = (DHCP_Msg_t*)pMemory->AllocAndClear(sizeof(DHCP_Msg_t));

    if(pTX != nullptr)
    {
        //Setup Options and dynamic part of DHCP
        Options = (DHCP_PUT_OPTION_CLIENT_IDENTIFIER | DHCP_PUT_OPTION_HOST_NAME | DHCP_PUT_OPTION_PL_REQUEST);

        if(m_State < DHCP_STATE_BOUND)
        {
            Options |= DHCP_PUT_OPTION_REQUESTED_CLIENT_IP;
        }
        else
        {
            Options |= DHCP_PUT_OPTION_SERVER_IP;
        }

        Length = PutOption(&pTX->Options[0], Options, DHCP_OPTION_REQUEST);
        PutHeader(pTX);

        // Send broadcasting packet
        if(m_State < DHCP_STATE_BOUND)
        {
        // this is the global one i think check previous version
            IP_Address = IP_ADDRESS(255,255,255,255);
        }
        else
        {
            IP_Address = IP_DHCP_IP;
        }

        Length = SOCK_SendTo(DHCP_SOCKET,
                           (uint8_t*)pTX,
                           sizeof(DHCP_Msg_t) - DHCP_OPTION_IN_PACKET_SIZE) + Length,
                           IP_Address,
                           DHCP_SERVER_PORT);
        if(Length == 0)
        {
          Status = false;
          #if (IP_DBG_DHCP == DEF_ENABLED)
            DBG_Printf("DHCP : Fatal Error(0).");
          #endif
        }

        pMemory->Free((void**)&pTX);
    }
    else
    {
       Status = false;
    }

    return Status;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ParseOffer
//
//  Parameter(s):   DHCP_Msg_t*   pRX
//  Return:         void
//
//  Description:    Get info from the first offer
//
//-------------------------------------------------------------------------------------------------
void NetDHCP::ParseOffer(DHCP_Msg_t* pRX)
{
    m_Options.ClientIP = ntohl(pRX->YourIP_Address);
    m_Options.ServerIP = ntohl(pRX->ServerIP_Address);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           IsBound
//
//  Parameter(s):   void
//  Return:         void
//
//  Description:    Process ACK or NACK
//
//-------------------------------------------------------------------------------------------------
void NetDHCP::IsBound(void)
{
    IP_DHCP_IP         = m_Options.ClientIP;
    IP_DHCP_SubnetMask = m_Options.SubnetMaskIP;
    IP_DHCP_GatewayIP  = m_Options.GatewayIP;
    IP_DHCP_DNS_IP     = m_Options.DNS_ServerIP;

    gar(IP_DHCP_GatewayIP);           // w5100 stuff
    subr(IP_DHCP_SubnetMask);           // w5100 stuff
    sipr(IP_DHCP_IP);           // w5100 stuff

    m_State    = DHCP_STATE_BOUND;
    pIP->SetIP_Valid(true);

    nOS_TimerStop(&m_TimerT1_Lease,  true);                               // make sure timer T1 & T2 are stop
    nOS_TimerStop(&m_TimerT2_Rebind, true);

    nOS_TimerStart(&m_TimerT1_Lease);  // maybe i need restart
    /*
    TIMER_Start(&m_TimerT1_Lease,
                m_Options.LeaseTime >> 1,
                m_pQ,
                IP_MSG_TYPE_DHCP_MANAGEMENT,
                DHCP_MSG_ACTION_LEASE_RENEWAL);                     // Start the renewal timer for 50% of the total lease
*/
    nOS_TimerStart(&m_TimerT2_Rebind);  // maybe i need restart
  /*
    TIMER_Start(&m_TimerT2_Rebind,
                (m_Options.LeaseTime >> 1) + (m_Options.LeaseTime >> 2) + (m_Options.LeaseTime >> 4),
                m_pQ,
                IP_MSG_TYPE_DHCP_MANAGEMENT,
                DHCP_MSG_ACTION_REBIND);                            // Start the rebind timer for 87% of the total lease
*/
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ParseOption
//
//  Parameter(s):   DHCP_Msg_t* pRX
//  Return:         void
//
//  Description:    Process the Option field and extract any relevant info for us
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void NetDHCP::ParseOption(DHCP_Msg_t* pRX)
{
    uint8_t*  pPtr;
    uint32_t  Value;

    pPtr = (uint8_t*)&pRX->Options;


    while((*pPtr != DHCP_OPTION_END_OF_FIELD) && (*pPtr != DHCP_OPTION_PADDING))
    {
        Value = *(uint32_t*)(pPtr + 2);
        
        switch(*pPtr)
        {
            case DHCP_OPTION_SUBNET_MASK:  { m_Options.SubnetMaskIP = ntohl(Value); } break;
            case DHCP_OPTION_GATEWAY:      { m_Options.GatewayIP    = ntohl(Value); } break;
            case DHCP_OPTION_DNS_SERVER:   { m_Options.DNS_ServerIP = ntohl(Value); } break;
            case DHCP_OPTION_CLIENT_IP:    { m_Options.ClientIP     = ntohl(Value); } break;
            case DHCP_OPTION_LEASE_TIME:   { m_Options.LeaseTime    = ntohl(Value); } break;
            case DHCP_OPTION_MESSAGE_TYPE: { m_Options.Type         = *(pPtr + 2);  } break;
            case DHCP_OPTION_SERVER_IP:    { m_Options.ServerIP     = ntohl(Value); } break;
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
//  Name:           PutOption
//
//  Parameter(s):   uint8_t*   pPtr         Pointer on option field in packet
//                  uint8_t    Options      Option(s) in Bit position to put in packet
//                  uint8_t    Message
//  Return:         size_t     Length       Length of the option(s)
//
//  Description:    Put option specify the flag
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
size_t NetDHCP::PutOption(uint8_t* pPtr, uint8_t Options, uint8_t Message)
{
    size_t     Length;
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
        memcpy(pPtr, &IP_MAC, 6);
        pPtr += 6;
    }

    if(Options & DHCP_PUT_OPTION_PL_DISCOVER)
    {
        memcpy(pPtr, m_OPL_Discover, sizeof(m_OPL_Discover));
        pPtr += sizeof(m_OPL_Discover);
    }

    if(Options & DHCP_PUT_OPTION_PL_REQUEST)
    {
        memcpy(pPtr, m_OPL_Request, sizeof(m_OPL_Request));
        pPtr += sizeof(m_OPL_Request);
    }

    if(Options & DHCP_PUT_OPTION_REQUESTED_CLIENT_IP)
    {
        *pPtr++ = DHCP_OPTION_CLIENT_IP;
        *pPtr++ = 4;
        *((int32_t*)pPtr) = htonl(m_Options.ClientIP);
        pPtr += 4;
    }

    if(Options & DHCP_PUT_OPTION_HOST_NAME)
    {
        uint8_t i;

        *pPtr++ = DHCP_OPTION_HOST_NAME;

        *pPtr++ = (uint8_t)(strlen(IP_HOST_NAME) + 9);              // length of hostname + 6
        strcpy(pPtr, IP_HOST_NAME);
        pPtr += strlen(IP_HOST_NAME);

        for(i = 3; i < 6; i++)                                      // Add last 3 uint8_t of the MAC in HEXA -> Ascii at the end of label
        {
            *pPtr++  = '_';
            *pPtr    = (uint8_t)((IP_MAC[i] >> 4) + 0x30);
            *pPtr   += (uint8_t)((*pPtr > 0x39) ? 7 : 0);
            pPtr++;
            *pPtr    = (uint8_t)((IP_MAC[i] & 0x0F) + 0x30);
            *pPtr   += (uint8_t)((*pPtr > 0x39) ? 7 : 0);
            pPtr++;
        }
    }

    if(Options & DHCP_PUT_OPTION_SERVER_IP)
    {
        *pPtr++ = DHCP_OPTION_SERVER_IP;
        *pPtr++ = 4;
        *((uint32_t*)pPtr) = htonl(m_Options.ServerIP);
        pPtr += 4;
    }

    *pPtr++ = DHCP_OPTION_END_OF_FIELD;

    return uint16_t(pPtr - pStart);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           PutHeader
//
//  Parameter(s):   DHCP_Msg_t*     pTX
//  Return:         void
//
//  Description:    Put in header everything static
//
//  Requirement:    All other data must be already in the header
//
//-------------------------------------------------------------------------------------------------
void NetDHCP::PutHeader(DHCP_Msg_t* pTX)
{
    // Setup DHCP header
    pTX->Op          = DHCP_BOOT_REQUEST;
    pTX->H_Type      = DHCP_HARDWARE_TYPE_ETHERNET_100;
    pTX->H_Length    = DHCP_HARDWARE_ADDRESS_LENGHT;
    pTX->X_ID        = htonl(DHCP_Xid);
    pTX->MagicCookie = DHCP_MAGIC_COOKIE;

    if(m_State < DHCP_STATE_BOUND)
    {
        pTX->Flags  = htons(DHCP_FLAGS_BROADCAST);
    }
    else
    {
        pTX->Flags = 0;     // For Unicast
        pTX->ClientIP_Address = htonl(IP_DHCP_IP);
    }

    memcpy(pTX->ClientHardware, IP_MAC, 6);
}

//-------------------------------------------------------------------------------------------------


