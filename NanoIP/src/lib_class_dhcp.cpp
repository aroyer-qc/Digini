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
void NetDHCP::Initialize(NetworkContext* pContext)
{
    nOS_Error Error;

    m_pContext = pContext;
    m_Mode     = DHCP_IS_ON;            // This is the default value for DHCP
    m_State    = DHCP_STATE_INITIAL;
    m_pSocket  = nullptr;
    m_XID      = RNG_GetRandom();

    Error = nOS_TimerCreate(&m_TimerDiscover,  nullptr, nullptr, DHCP_MSG_ACTION_TIME_OUT, NOS_TIMER_ONE_SHOT);
    Error = nOS_TimerCreate(&m_TimerT1_Lease,  nullptr, nullptr, 0, NOS_TIMER_ONE_SHOT);
    Error = nOS_TimerCreate(&m_TimerT2_Rebind, nullptr, nullptr, 0, NOS_TIMER_ONE_SHOT);

    VAR_UNUSED(Error); // TODO Manage error
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
    m_pContext->SetIP_Valid(false);
    m_pContext->SetDHCP_GatewayIP(IP_ADDRESS(0,0,0,0));
    m_pContext->SetDHCP_SubnetMask(IP_ADDRESS(0,0,0,0));
    m_pContext->SetDHCP_IP(IP_ADDRESS(0,0,0,0));
    m_pContext->SetDHCP_DNS_IP(IP_ADDRESS(0,0,0,0));
    m_XID = RNG_GetRandom();                                            // Generate a new transaction ID

    if(m_pSocket != nullptr)                                            // Close existent socket if already open
    {
        m_pContext->GetIP_Manager()->GetSocketManager()->FreeSocket(&m_pSocket);
    }

    m_pSocket = m_pContext->GetIP_Manager()->GetSocketManager()->AllocSocket(SOCKET_TYPE_DATAGRAM);    // Create a new for socket UDP

    if(m_pSocket == nullptr)
    {
        return false;
    }

    bool NonBlocking = true;                                            // Configure in non blocking mode
    m_pSocket->SetOption(SOCKET_OPT_NON_BLOCKING, &NonBlocking, sizeof(bool));

    // Bind on DHCP client port
    LocalAddress.Address = IP_ADDRESS(0,0,0,0);                         // ANY address
    LocalAddress.Port    = DHCP_CLIENT_PORT;
    State                = m_pSocket->Bind(LocalAddress.Port);

    if(State != SYS_READY)
    {
        m_pContext->GetIP_Manager()->GetSocketManager()->FreeSocket(&m_pSocket);
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
bool NetDHCP::Process(void)
{
    //--------------------------------------------------------------------------
    // 1) Handle internal timers (non-blocking)
    //--------------------------------------------------------------------------

    if(nOS_TimerIsRunning(&m_TimerDiscover) == false)
    {
        // Retry DISCOVER if still not bound
        if((m_State == DHCP_STATE_INITIAL) || (m_State == DHCP_STATE_SELECTING))
        {
            Discover();
        }
    }

    if(nOS_TimerIsRunning(&m_TimerT1_Lease) == false)
    {
        // T1: Renew
        if (m_State == DHCP_STATE_BOUND)
        {
            m_State = DHCP_STATE_RENEWING;
            Request();
        }
    }

    if(nOS_TimerIsRunning(&m_TimerT2_Rebind) == false)
    {
        // T2: Rebind
        if (m_State == DHCP_STATE_RENEWING)
        {
            m_State = DHCP_STATE_REBINDING;
            Request();
        }
    }

    // Poll DHCP socket for incoming packets (non-blocking)
    size_t          BytesReceived = 0;
    SocketInfo_t    SourceInfo;

    // Allocate a buffer from your pool
    uint8_t* pBuffer = (uint8_t*)pMemoryPool->Alloc(DHCP_PACKET_SIZE, MEM_DBG_DHCP);

    if(pBuffer != nullptr)
    {
        // Receive directly into the packet buffer
        SystemState_e State = m_pSocket->RecvFrom((uint8_t*)pBuffer,        // Buffer
                                                  DHCP_PACKET_SIZE,         // Buffer size
                                                  &SourceInfo,              // Optional source info
                                                  &BytesReceived);          // Number of bytes received

        if((State == SYS_READY) && (BytesReceived > 0))
        {
            DHCP_Msg_t* pRX = (DHCP_Msg_t*)pBuffer;

            if (pRX->MagicCookie == DHCP_MAGIC_COOKIE)
            {
                ParseOption(pRX);

                switch (m_Options.Type)
                {
                    case DHCP_OPTION_OFFER:
                    {
                        ParseOffer(pRX);
                        m_State = DHCP_STATE_REQUESTING;
                        Request();
                    }
                    break;

                    case DHCP_OPTION_ACK:
                    {
                        IsBound();
                    }
                    break;

                    case DHCP_OPTION_NACK:
                    {
                        m_State = DHCP_STATE_INITIAL;
                        Discover();
                    }
                    break;
                }
            }
        }

        // Always free the buffer
        pMemoryPool->Free((void**)&pBuffer);
    }

    //--------------------------------------------------------------------------
    // 3) Return true if we are bound
    //--------------------------------------------------------------------------

    return (m_State == DHCP_STATE_BOUND);
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
    bool          Status;

    Status = Start();                                                                           // Restart DHCP state and recreate socket

    if(Status == false)
    {
        return false;
    }

    // Allocate DHCP transmit buffer
    pTX = (DHCP_Msg_t*)pMemoryPool->AllocAndClear(sizeof(DHCP_Msg_t), MEM_DBG_DHCPTX);

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

        m_State = DHCP_STATE_SELECTING;                                                         // Update DHCP state
        nOS_TimerStart(&m_TimerDiscover);                                                       // Start timeout timer for OFFER
    }

    pMemoryPool->Free((void**)&pTX);                                                            // Free TX buffer

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
    uint8_t       Options;
    DHCP_Msg_t*   pTX     = nullptr;
    size_t        Length  = 0;
    bool          Status  = true;

    // Allocate DHCP transmit buffer
    pTX = (DHCP_Msg_t*)pMemoryPool->AllocAndClear(sizeof(DHCP_Msg_t), MEM_DBG_DHCPTX);

    if(pTX == nullptr)
    {
        return false;
    }

    // Build DHCP options for REQUEST
    Options = (DHCP_PUT_OPTION_CLIENT_IDENTIFIER |
               DHCP_PUT_OPTION_HOST_NAME         |
               DHCP_PUT_OPTION_PL_REQUEST);

    // Before BOUND → include Requested IP
    // After BOUND  → include Server Identifier
    if(m_State < DHCP_STATE_BOUND)
    {
        Options |= DHCP_PUT_OPTION_REQUESTED_CLIENT_IP;
    }
    else
    {
        Options |= DHCP_PUT_OPTION_SERVER_IP;
    }

    Length = PutOption(&pTX->Options[0], Options, DHCP_OPTION_REQUEST);

    // Build DHCP header (Op, HTYPE, HLEN, XID, CHADDR, etc.)
    PutHeader(pTX);

    // Compute total packet length
    size_t PacketLength = (sizeof(DHCP_Msg_t) - DHCP_OPTION_IN_PACKET_SIZE) + Length;

    // Select destination: broadcast for initial REQUEST, unicast for renewal
    SocketInfo_t Dest;

    if(m_State < DHCP_STATE_BOUND)
    {
        // Initial REQUEST -> broadcast
        Dest.Address = IP_ADDRESS(255,255,255,255);
    }
    else
    {
        // Renewal REQUEST -> unicast to DHCP server
        Dest.Address = m_pContext->GetDHCP_ServerIP();
    }

    Dest.Port = DHCP_SERVER_PORT;

    // Send DHCP REQUEST
    size_t BytesSent = 0;
    SystemState_e Error = m_pSocket->SendTo((uint8_t*)pTX,
                                            PacketLength,
                                            &Dest,
                                            &BytesSent);

    if((Error != SYS_READY) || (BytesSent == 0))
    {
        Status = false;

      #if (IP_DBG_DHCP == DEF_ENABLED)
        DBG_Printf("DHCP: Fatal error while sending REQUEST\n");
      #endif
    }
    else
    {
      #if (IP_DBG_DHCP == DEF_ENABLED)
        DBG_Printf("DHCP REQUEST sent\n");
      #endif
    }

    // Free TX buffer
    pMemoryPool->Free((void**)&pTX);

    return Status;
}

//-------------------------------------------------------------------------------------------------
// Name:            ParseOffer
//
// Parameter(s):    DHCP_Msg_t* pRX
// Return:          void
//
// Description:     Extract offered client IP and DHCP server IP from the OFFER message.
//                  This function assumes the message has already been validated.
//-------------------------------------------------------------------------------------------------
void NetDHCP::ParseOffer(DHCP_Msg_t* pRX)
{
    // Offered IP address for this client
    m_Options.ClientIP = ntohl(pRX->YourIP_Address);

    // DHCP server identifier (may be zero if not provided)
    m_Options.ServerIP = ntohl(pRX->ServerIP_Address);

#if (IP_DBG_DHCP == DEF_ENABLED)
    DBG_Printf("DHCP OFFER: Client IP = %d.%d.%d.%d, Server IP = %d.%d.%d.%d\n",
               uint8_t(m_Options.ClientIP >> 24),
               uint8_t(m_Options.ClientIP >> 16),
               uint8_t(m_Options.ClientIP >> 8),
               uint8_t(m_Options.ClientIP),
               uint8_t(m_Options.ServerIP >> 24),
               uint8_t(m_Options.ServerIP >> 16),
               uint8_t(m_Options.ServerIP >> 8),
               uint8_t(m_Options.ServerIP));
#endif
}

//-------------------------------------------------------------------------------------------------
// Name:            IsBound
//
// Parameter(s):    None
// Return:          void
//
// Description:     Apply DHCP lease parameters to the interface and transition to BOUND state.
//                  This function assumes that m_Options has been filled by ParseOption().
//-------------------------------------------------------------------------------------------------
void NetDHCP::IsBound(void)
{
    // Update interface context
    m_pContext->SetDHCP_IP(m_Options.ClientIP);
    m_pContext->SetDHCP_SubnetMask(m_Options.SubnetMaskIP);
    m_pContext->SetDHCP_GatewayIP(m_Options.GatewayIP);
    m_pContext->SetDHCP_DNS_IP(m_Options.DNS_ServerIP);
    m_pContext->SetDHCP_ServerIP(m_Options.ServerIP);

    // Mark interface as valid
    m_pContext->SetIP_Valid(true);

    // Update DHCP state
    m_State = DHCP_STATE_BOUND;

    // Restart lease timers (T1 and T2)
    nOS_TimerStop(&m_TimerT1_Lease,  true);
    nOS_TimerStop(&m_TimerT2_Rebind, true);

    nOS_TimerStart(&m_TimerT1_Lease);
    nOS_TimerStart(&m_TimerT2_Rebind);

#if (IP_DBG_DHCP == DEF_ENABLED)
    DBG_Printf("DHCP BOUND: IP=%d.%d.%d.%d  MASK=%d.%d.%d.%d  GW=%d.%d.%d.%d\n",
               uint8_t(m_DHCP_IP >> 24),         uint8_t(m_DHCP_IP >> 16),
               uint8_t(m_DHCP_IP >> 8),          uint8_t(m_DHCP_IP),
               uint8_t(m_DHCP_SubnetMask >> 24), uint8_t(m_DHCP_SubnetMask >> 16),
               uint8_t(m_DHCP_SubnetMask >> 8),  uint8_t(m_DHCP_SubnetMask),
               uint8_t(m_DHCP_GatewayIP >> 24),  uint8_t(m_DHCP_GatewayIP >> 16),
               uint8_t(m_DHCP_GatewayIP >> 8),   uint8_t(m_DHCP_GatewayIP));
#endif
}

//-------------------------------------------------------------------------------------------------
// Name:            ParseOption
//
// Parameter(s):    DHCP_Msg_t* pRX
// Return:          void
//
// Description:     Parse DHCP options from the received message. This function assumes that the
//                  DHCP header and magic cookie have already been validated.
//-------------------------------------------------------------------------------------------------
void NetDHCP::ParseOption(DHCP_Msg_t* pRX)
{
    uint8_t* pPtr = (uint8_t*)&pRX->Options;
    uint8_t  Code;
    uint8_t  Len;

    // Iterate through all DHCP options until END option is reached
    while(true)
    {
        Code = *pPtr;

        // End of options field
        if(Code == DHCP_OPTION_END_OF_FIELD)
        {
            break;
        }

        // Padding: skip one byte
        if(Code == DHCP_OPTION_PADDING)
        {
            pPtr++;
            continue;
        }

        // Read option length
        Len = *(pPtr + 1);

        // Safety: avoid malformed packets
        if(Len == 0)
        {
            pPtr += 2;
            continue;
        }

        // Pointer to option data
        uint8_t* pData = pPtr + 2;

        // Process known options
        switch(Code)
        {
            case DHCP_OPTION_SUBNET_MASK:
            {
                if(Len >= 4)
                {
                    m_Options.SubnetMaskIP = ntohl(*(uint32_t*)pData);
                }
            }
            break;

            case DHCP_OPTION_GATEWAY:
            {
                if(Len >= 4)
                {
                    m_Options.GatewayIP = ntohl(*(uint32_t*)pData);
                }
            }
            break;

            case DHCP_OPTION_DNS_SERVER:
            {
                if(Len >= 4)
                {
                    m_Options.DNS_ServerIP = ntohl(*(uint32_t*)pData);
                }
            }
            break;

            case DHCP_OPTION_CLIENT_IP:
            {
                if(Len >= 4)
                {
                    m_Options.ClientIP = ntohl(*(uint32_t*)pData);
                }
            }
            break;

            case DHCP_OPTION_LEASE_TIME:
            {
                if(Len >= 4)
                {
                    m_Options.LeaseTime = ntohl(*(uint32_t*)pData);
                }
            }
            break;

            case DHCP_OPTION_MESSAGE_TYPE:
            {
                if(Len >= 1)
                {
                    m_Options.Type = pData[0];
                }
            }
            break;

            case DHCP_OPTION_SERVER_IP:
            {
                if(Len >= 4)
                {
                    m_Options.ServerIP = ntohl(*(uint32_t*)pData);
                }
            }
            break;

            default:
                // Unknown option -> ignore
                break;
        }

        // Move to next option (code + length + data)
        pPtr += (2 + Len);
    }
}

//-------------------------------------------------------------------------------------------------
// Name:            PutOption
//
// Parameter(s):    uint8_t* pPtr      - Pointer to DHCP options buffer
//                  uint8_t  Options   - Bitmask of options to include
//                  uint8_t  Message   - DHCP message type (DISCOVER, REQUEST, etc.)
//
// Return:          Total number of bytes written to the options buffer
//
// Description:     Build DHCP options field based on the requested option flags.
//                  This function does not perform bounds checking; caller must ensure
//                  the buffer is large enough.
//-------------------------------------------------------------------------------------------------
size_t NetDHCP::PutOption(uint8_t* pPtr, uint8_t Options, uint8_t Message)
{
    uint8_t* pStart = pPtr;

    // DHCP Message Type
    *pPtr++ = DHCP_OPTION_MESSAGE_TYPE;
    *pPtr++ = 1;
    *pPtr++ = Message;

    // Client Identifier (Type 1 = Ethernet + MAC address)
    if(Options & DHCP_PUT_OPTION_CLIENT_IDENTIFIER)
    {
        *pPtr++ = DHCP_OPTION_CLIENT_IDENTIFIER;
        *pPtr++ = 1 + IP_MAC_ADDRESS_SIZE;   // Type + MAC
        *pPtr++ = 1;                         // Hardware type = Ethernet
        m_pContext->GetMAC_Address((IP_MAC_Address_t*)pPtr);
        pPtr += IP_MAC_ADDRESS_SIZE;
    }

    // Predefined option list for DISCOVER
    if(Options & DHCP_PUT_OPTION_PL_DISCOVER)
    {
        memcpy(pPtr, m_OPL_Discover, sizeof(m_OPL_Discover));
        pPtr += sizeof(m_OPL_Discover);
    }

    // Predefined option list for REQUEST
    if(Options & DHCP_PUT_OPTION_PL_REQUEST)
    {
        memcpy(pPtr, m_OPL_Request, sizeof(m_OPL_Request));
        pPtr += sizeof(m_OPL_Request);
    }

    // Requested Client IP (used in initial REQUEST)
    if(Options & DHCP_PUT_OPTION_REQUESTED_CLIENT_IP)
    {
        *pPtr++ = DHCP_OPTION_CLIENT_IP;
        *pPtr++ = 4;

        uint32_t ip = htonl(m_Options.ClientIP);
        memcpy(pPtr, &ip, sizeof(ip));
        pPtr += sizeof(ip);
    }

    // Host Name (hostname + "_" + last 3 bytes of MAC in hex)
    if(Options & DHCP_PUT_OPTION_HOST_NAME)
    {
        const char* Host = m_pContext->GetHostName();
        size_t HostLen   = strlen(Host);

        // Hostname + 3 separators + 6 hex chars
        size_t ExtraLen = 3 + (3 * 2);
        size_t TotalLen = HostLen + ExtraLen;

        *pPtr++ = DHCP_OPTION_HOST_NAME;
        *pPtr++ = (uint8_t)TotalLen;

        // Copy hostname
        memcpy(pPtr, Host, HostLen);
        pPtr += HostLen;

        // Append "_XX_XX_XX" based on last 3 MAC bytes
        IP_MAC_Address_t MAC_Address;
        m_pContext->GetMAC_Address(&MAC_Address);

        for(uint8_t i = 3; i < 6; i++)
        {
            *pPtr++ = '_';

            uint8_t hi = (MAC_Address.Byte[i] >> 4) & 0x0F;
            uint8_t lo = (MAC_Address.Byte[i] & 0x0F);

            *pPtr++ = (hi < 10) ? ('0' + hi) : ('A' + (hi - 10));
            *pPtr++ = (lo < 10) ? ('0' + lo) : ('A' + (lo - 10));
        }
    }

    // Server Identifier (used in renewal REQUEST)
    if(Options & DHCP_PUT_OPTION_SERVER_IP)
    {
        *pPtr++ = DHCP_OPTION_SERVER_IP;
        *pPtr++ = 4;

        uint32_t ip = htonl(m_Options.ServerIP);
        memcpy(pPtr, &ip, sizeof(ip));
        pPtr += sizeof(ip);
    }

    // End of DHCP options
    *pPtr++ = DHCP_OPTION_END_OF_FIELD;

    return (size_t)(pPtr - pStart);
}

//-------------------------------------------------------------------------------------------------
// Name:            PutHeader
//
// Parameter(s):    DHCP_Msg_t* pTX
// Return:          void
//
// Description:     Build the fixed DHCP header fields for DISCOVER/REQUEST messages.
//                  This function assumes that m_XID and m_Context are already initialized.
//-------------------------------------------------------------------------------------------------
void NetDHCP::PutHeader(DHCP_Msg_t* pTX)
{
    // Clear all header fields (safety)
    memset(pTX, 0, sizeof(DHCP_Msg_t));

    // DHCP fixed header
    pTX->Op          = DHCP_BOOT_REQUEST;
    pTX->H_Type      = DHCP_HARDWARE_TYPE_ETHERNET;     // Always 1 for Ethernet
    pTX->H_Length    = IP_MAC_ADDRESS_SIZE;             // MAC address length
    pTX->Hops        = 0;
    pTX->X_ID        = htonl(m_XID);
    pTX->Seconds     = 0;
    pTX->MagicCookie = DHCP_MAGIC_COOKIE;

    // Broadcast flag for initial DISCOVER/REQUEST
    if (m_State < DHCP_STATE_BOUND)
    {
        pTX->Flags = htons(DHCP_FLAGS_BROADCAST);
    }
    else
    {
        // Unicast renewal: include client IP
        pTX->Flags = 0;

        uint32_t ip = htonl(m_pContext->GetDHCP_IP());
        memcpy(&pTX->ClientIP_Address, &ip, sizeof(ip));
    }

    // Client hardware address (MAC)
    m_pContext->GetMAC_Address((IP_MAC_Address_t*)pTX->ClientHardware);
}

//-------------------------------------------------------------------------------------------------

#endif // (IP_USE_DHCP == DEF_ENABLED)


