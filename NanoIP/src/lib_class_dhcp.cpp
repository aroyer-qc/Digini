//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_dhcp.cpp
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

#include "./lib_digini.h"

//-------------------------------------------------------------------------------------------------

#if (IP_USE_DHCP == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// const(s)
//-------------------------------------------------------------------------------------------------

const uint8_t NetDHCP::m_OPL_Discover[8] = // OPL stand for option list
{
    55,        // Parameter list
    6,         // Size
    1,         // Subnet Mask
    3,         // Gateway
    6,         // DNS Server
    15,        // Domain Name
    58,        // DHCP T1 Value
    59         // DHCP T2 Value
};

const uint8_t NetDHCP::m_OPL_Request[10] =
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
    33         // Static Route
};


// temporary
//-------------------------------------------------------------------------------------------------
// DHCP Options Structure
//-------------------------------------------------------------------------------------------------
struct DHCP_Options_t
{
    IP_Address_t    ClientIP;
    IP_Address_t    ServerIP;
    IP_Address_t    SubnetMaskIP;
    IP_Address_t    GatewayIP;
    IP_Address_t    DNS_ServerIP;
    uint32_t        LeaseTime;
    uint8_t         Type;
};

//-------------------------------------------------------------------------------------------------
//
//  Name:           Initialize
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Initialize the DHCP Client
//
//-------------------------------------------------------------------------------------------------
void NetDHCP::Initialize(void)
{
    nOS_Error Error;

    m_Mode    = DHCP_IS_ON;            // This is the default value for DHCP
    m_State   = DHCP_STATE_INITIAL;
    m_pSocket = nullptr;
    m_XID     = RNG_GetRandom();

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
    SystemState_e State;
    SocketInfo_t  LocalAddress;

    m_State = DHCP_STATE_INITIAL;                                       // Reset DHCP state

    if(nOS_TimerIsRunning(&m_TimerDiscover)  == true) nOS_TimerStop(&m_TimerDiscover,  true);
    if(nOS_TimerIsRunning(&m_TimerT1_Lease)  == true) nOS_TimerStop(&m_TimerT1_Lease,  true);
    if(nOS_TimerIsRunning(&m_TimerT2_Rebind) == true) nOS_TimerStop(&m_TimerT2_Rebind, true);

    // Reset DHCP-related context information
    m_Context.SetIP_Valid(false);
    m_Context.SetDHCP_GatewayIP(IP_ADDRESS(0,0,0,0));
    m_Context.SetDHCP_SubnetMask(IP_ADDRESS(0,0,0,0));
    m_Context.SetDHCP_IP(IP_ADDRESS(0,0,0,0));
    m_Context.SetDHCP_DNS_IP(IP_ADDRESS(0,0,0,0));
    m_XID = RNG_GetRandom();                                            // Generate a new transaction ID

    if(m_pSocket != nullptr)                                            // Close existant socket if already open
    {
        Socket::FreeSocket(&m_pSocket);
    }

    m_pSocket = Socket::AllocSocket(SOCKET_TYPE_UDP);                   // Create a new for socket UDP

    if(m_pSocket == nullptr)
    {
        return false;
    }

    bool NonBlocking = true;                                            // Configure in non blocking mode
    m_pSocket->SetOption(SOCKET_OPT_NON_BLOCKING, &NonBlocking, sizeof(bool));

    // Bind sur le port DHCP client
    LocalAddress.Address = IP_ADDRESS(0,0,0,0);                         // ANY address
    LocalAddress.Port    = DHCP_CLIENT_PORT;
    State = m_pSocket->Bind(&LocalAddr);

    if(State != SYS_READY)
    {
        Socket::FreeSocket(&m_pSocket);
        return false;
    }

    return true;
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
    bool          Status = false;
    DHCP_Msg_t*   pRX    = nullptr;
    size_t        RxSize = 0;
    SystemState_e Error;

    // Handle external DHCP actions (timeouts, renewal, rebind)
    if(pMsg != nullptr)
    {
        switch(pMsg->Action)
        {
            case DHCP_MSG_ACTION_TIME_OUT:
            {
                m_State = DHCP_STATE_INITIAL;                                           // Reset DHCP state machine

                if(m_pSocket != nullptr)                                                // Close existing socket if any
                {
                    Socket::FreeSocket(&m_pSocket);
                }

              #if (IP_DBG_DHCP == DEF_ENABLED)
                DBG_Printf("DHCP timeout, restarting DHCP\n");
              #endif
            }
            break;

            case DHCP_MSG_ACTION_LEASE_RENEWAL:
            {
                if(Start() == true)                                                     // Restart DHCP using the existing lease information
                {
                    Request();

                  #if (IP_DBG_DHCP == DEF_ENABLED)
                    DBG_Printf("DHCP renewal request sent\n");
                  #endif
                }
            }
            break;

            case DHCP_MSG_ACTION_REBIND:
            {
                // Rebind logic can be added later
            }
            break;
        }

        pMemoryPool->Free((void**)&pMsg);                                               // Free the action message
    }

    if(m_Mode == false)                                                                 // DHCP disabled → nothing to do
    {
        return false;
    }

    if(m_State == DHCP_STATE_INITIAL)                                                   // If in INITIAL state, send a DHCP DISCOVER
    {
        Status = Discover();
    }
    else
    {
        if((m_pSocket == nullptr) || (m_pSocket->GetState() == SOCKET_STATE_CLOSED))    // Validate socket state
        {
            if(m_State != DHCP_STATE_BOUND)                                             // If not bound yet, restart DHCP
            {
                m_State = DHCP_STATE_INITIAL;
            }
        }
        else
        {
            // Try to receive a DHCP message (non-blocking)
            SocketInfo_t ServerInfo;
            pRX = (DHCP_Msg_t*)pMemoryPool->Alloc(sizeof(DHCP_Msg_t), MEM_DBG_DHCP_RX);

            if(pRX != nullptr)
            {
                Error = m_pSocket->RecvFrom((uint8_t*)pRX, sizeof(DHCP_Msg_t), &ServerInfo, &RxSize);

                // Validate received DHCP message
                if((Error == SYS_READY) && (RxSize >= sizeof(DHCP_Header_t)))
                {
                    if( (pRX->Op          == DHCP_BOOT_REPLY) &&
                        (pRX->MagicCookie == DHCP_MAGIC_COOKIE) &&
                        (pRX->X_ID        == htonl(m_XID)) )
                    {
                        // Parse DHCP options
                        ParseOption(pRX);

                        // Process DHCP message type
                        switch(m_Options.Type)
                        {
                            //-------------------------------------------------------------------------
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
                                        DBG_Printf("DHCP OFFER received, REQUEST sent\n");
                                      #endif
                                    }

                                    // Restart discover timer while waiting for ACK
                                    nOS_TimerRestart(&m_TimerDiscover, DHCP_MSG_ACTION_TIME_OUT);
                                }
                            }
                            break;

                            //-------------------------------------------------------------------------
                            case DHCP_OPTION_ACK:
                            {
                                if(m_State == DHCP_STATE_OFFER_RECEIVED)
                                {
                                    // Stop discover timeout
                                    nOS_TimerStop(&m_TimerDiscover, true);

                                    // Apply lease parameters
                                    IsBound();

                                    // Close DHCP socket
                                    if(m_pSocket != nullptr)
                                    {
                                        m_pSocket->Close();
                                    }

                                  #if (IP_DBG_DHCP == DEF_ENABLED)
                                    DBG_Printf("DHCP ACK received, interface is now bound\n");
                                  #endif
                                }
                                else if(m_State == DHCP_STATE_BOUND)
                                {
                                    // Lease renewal accepted
                                    IsBound();

                                  #if (IP_DBG_DHCP == DEF_ENABLED)
                                    DBG_Printf("DHCP lease renewal accepted\n");
                                  #endif
                                }
                            }
                            break;

                            //-------------------------------------------------------------------------
                            case DHCP_OPTION_NACK:
                            {
                                // Reset DHCP state machine
                                m_State = DHCP_STATE_INITIAL;

                                // Stop lease timers
                                nOS_TimerStop(&m_TimerT1_Lease,  true);
                                nOS_TimerStop(&m_TimerT2_Rebind, true);

                                // Close socket
                                if(m_pSocket != nullptr)
                                {
                                    m_pSocket->Close();
                                }

                              #if (IP_DBG_DHCP == DEF_ENABLED)
                                DBG_Printf("DHCP NACK received\n");
                              #endif
                            }
                            break;

                            default:
                                // Unsupported or irrelevant option type
                            break;
                        }
                    }
                }

                // Free RX buffer
                pMemoryPool->Free((void**)&pRX);
            }
        }
    }

    // DHCP completed successfully when in BOUND state
    if(m_State == DHCP_STATE_BOUND)
    {
        Status = true;
    }

    return Status;
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
    uint8_t       Options;
    DHCP_Msg_t*   pTX     = nullptr;
    size_t        Length  = 0;
    bool          Status  = false;

    Status = Start();                                                                           // Restart DHCP state and recreate socket
    
    if(Status == false)
    {
        return false;
    }

    // Allocate DHCP transmit buffer
    pTX = (DHCP_Msg_t*)pMemoryPool->AllocAndClear(sizeof(DHCP_Msg_t), MEM_DBG_DHCP_TX);
    
    if(pTX == nullptr)
    {
        return false;
    }

    // Build DHCP options for DISCOVER
    Options = (DHCP_PUT_OPTION_CLIENT_IDENTIFIER |
               DHCP_PUT_OPTION_HOST_NAME         |
               DHCP_PUT_OPTION_PL_DISCOVER);

    Length = PutOption(&pTX->Options[0], Options, DHCP_OPTION_DISCOVER);
    PutHeader(pTX);                                                                             // Build DHCP header (Op, HTYPE, HLEN, XID, CHADDR, etc.)
    size_t PacketLength = (sizeof(DHCP_Msg_t) - DHCP_OPTION_IN_PACKET_SIZE) + Length;           // Compute total packet length (header + options)

    // Destination: broadcast IP
    SocketInfo_t Dest;
    Dest.Address = IP_ADDRESS(255,255,255,255);
    Dest.Port    = DHCP_SERVER_PORT;

    // Send DHCP DISCOVER (non-blocking)
    size_t BytesSent = 0;
    SystemState_e Error = m_pSocket->SendTo((uint8_t*)pTX, PacketLength, &Dest, &BytesSent);

    if((Error != SYS_READY) || (BytesSent == 0))
    {
        Status = false;

      #if (IP_DBG_DHCP == DEF_ENABLED)
        DBG_Printf("DHCP: Fatal error while sending DISCOVER\n");
      #endif
    }
    else
    {
      #if (IP_DBG_DHCP == DEF_ENABLED)
        DBG_Printf("DHCP DISCOVER sent\n");
      #endif

        m_State = DHCP_STATE_DISCOVER;                                                          // Update DHCP state
        nOS_TimerStart(&m_TimerDiscover);                                                       // Start timeout timer for OFFER
    }

    pMemoryPool->Free((void**)&pTX);                                                            // Free TX buffer

    return Status;
    
    
    #if 0
    uint8_t      Options;
    DHCP_Msg_t*  pTX         = nullptr;
    //IP_Address_t IP_Address;
    size_t       Length;
    bool         Status;

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
#endif
}
we are here with the IA

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
    m_DHCP_IP         = m_Options.ClientIP;
    m_DHCP_SubnetMask = m_Options.SubnetMaskIP;
    m_DHCP_GatewayIP  = m_Options.GatewayIP;
    m_DHCP_DNS_IP     = m_Options.DNS_ServerIP;

    //gar(IP_DHCP_GatewayIP);           // w5100 stuff
    //subr(IP_DHCP_SubnetMask);           // w5100 stuff
    //sipr(IP_DHCP_IP);           // w5100 stuff

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
        memcpy(pPtr, &IP_MAC, IP_MAC_ADDRESS_SIZE);
        pPtr += IP_MAC_ADDRESS_SIZE;
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
        pPtr += sizeof(int32_t);
    }

    if(Options & DHCP_PUT_OPTION_HOST_NAME)
    {
        uint8_t i;

        *pPtr++ = DHCP_OPTION_HOST_NAME;

        *pPtr++ = (uint8_t)(strlen(IP_HOST_NAME) + 9);              // length of hostname + 9
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
        pPtr += sizeof(uint32_t);
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
    pTX->X_ID        = htonl(DHCP_XID);
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

    memcpy(pTX->ClientHardware, IP_MAC, IP_MAC_ADDRESS_SIZE);
}

//-------------------------------------------------------------------------------------------------

#endif // (IP_USE_DHCP == DEF_ENABLED)


