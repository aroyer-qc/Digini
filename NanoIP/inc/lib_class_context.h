//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_context.h
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2026 Alain Royer.
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

#if (DIGINI_USE_ETHERNET == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Expand macro(s)
//-------------------------------------------------------------------------------------------------

  #define EXPAND_X_IF_AS_ENUM(ENUM_ID, HOST_NAME, STK_VAR, PROTOCOL_FLAG, DEFAULT_STATIC_IP, DEFAULT_GATEWAY, DEFAULT_SUBNET, DEFAULT_STATIC_DNS, MAC_ADDRESS, ETH_DRIVER, PHY_DRIVER, PHY_ADDRESS) ENUM_ID,
  #define EXPAND_X_IF_AS_STRUCT_DATA(ENUM_ID, HOST_NAME, STK_VAR, PROTOCOL_FLAG, DEFAULT_STATIC_IP, DEFAULT_GATEWAY, DEFAULT_SUBNET, DEFAULT_STATIC_DNS,   MAC_ADDRESS, ETH_DRIVER, PHY_DRIVER, PHY_ADDRESS) \
                                            { HOST_NAME, STK_VAR, PROTOCOL_FLAG, DEFAULT_STATIC_IP, DEFAULT_GATEWAY, DEFAULT_SUBNET, DEFAULT_STATIC_DNS, { MAC_ADDRESS, ETH_DRIVER, PHY_DRIVER, PHY_ADDRESS } },

  #define EXPAND_X_IF_AS_STACK_DECLARATION(ENUM_ID, HOST_NAME, STK_VAR, PROTOCOL_FLAG, DEFAULT_STATIC_IP, DEFAULT_GATEWAY, DEFAULT_SUBNET, DEFAULT_STATIC_DNS,   MAC_ADDRESS, ETH_DRIVER, PHY_DRIVER, PHY_ADDRESS) \
                                                     nOS_Stack STK_VAR[TASK_IP_MANAGER_STACK_SIZE]  NOS_STACK_LOCATION;

//-------------------------------------------------------------------------------------------------
// Enum(s)
//-------------------------------------------------------------------------------------------------

enum IF_ID_e
{
    IF_ETH_DEF(EXPAND_X_IF_AS_ENUM)
    IP_NUMBER_OF_INTERFACE,
};

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

typedef SystemState_e (*SendCallback_t)(void* pContext, IP_PacketMsg_t** ppPacketMsg);

//-------------------------------------------------------------------------------------------------
// Class
//-------------------------------------------------------------------------------------------------

class NetworkContext
{
    public:

        void                Initialize              (IF_ID_e If_ID);


        ETH_LinkState_e     GetLinkState            (void)          const                       { return m_LinkState;                                                           }
        ETH_LinkSpeed_e     GetLinkSpeed            (void)          const                       { return m_LinkSpeed;                                                           }
        bool                GetLinkChange           (void)                                      { return m_LinkChange;                                                          }
        const char*         GetHostName             (void)                                      { return m_Config[m_IF_ID].pHostName;                                           }
        nOS_Stack*          GetIP_Stack             (void)                                      { return m_Config[m_IF_ID].pStack;                                              }

        void                SetLinkState            (ETH_LinkState_e State)                     { m_LinkState = State;                                                          }
        void                SetLinkSpeed            (ETH_LinkSpeed_e Speed)                     { m_LinkSpeed = Speed;                                                          }
        void                SetLinkChange           (bool State)                                { m_LinkChange = State;                                                         }
		void                SetHostName             (const char* pHostName)                     { m_pHostName = pHostName;                                                      }

        bool                IsIP_Valid              (void)                                      { return m_IP_Valid;                                                            }
        void                SetIP_Valid             (bool State)                                { m_IP_Valid = State;                                                           }

        bool                IsEthernetReady         (void);

      //#if (IP_USE_ARP == DEF_ENABLED)
        ARP_Manager&        GetARP                  (void)                                      { return m_ARP;                                                                 }
      //#endif
      #if (IP_USE_DHCP == DEF_ENABLED)
        DHCPv4_Manager&     GetDHCP                 (void)                                      { return m_DHCP;                                                                }
      #endif
      #if (IP_USE_DNS == DEF_ENABLED)
        DNS_Manager&        GetDNS                  (void)                                      { return m_DNS;                                                                 }
      #endif
      #if (IP_USE_ICMP == DEF_ENABLED)
        ICMP_Manager&       GetICMP                 (void)                                      { return m_ICMP;                                                                }
      #endif
      #if (IP_USE_NTP == DEF_ENABLED)
        NTP_Manager&        GetNTP                  (void)                                      { return m_NTP;                                                                 }
      #endif
      #if (IP_USE_SNTP == DEF_ENABLED)
        SNTP_Manager&       GetSNTP                 (void)                                      { return m_SNTP;                                                                }
      #endif
      #if (IP_USE_RAW == DEF_ENABLED)
        RAW_Manager&        GetRAW                  (void)                                      { return m_RAW;                                                                 }
      #endif
      #if (IP_USE_TCP_SERVER == DEF_ENABLED) || (IP_USE_TCP_CLIENT == DEF_ENABLED)
        TCP_Manager&        GetTCP                  (void)                                      { return m_TCP;                                                                 }
      #endif
      #if (IP_USE_UDP == DEF_ENABLED)
        UDP_Manager&        GetUDP                  (void)                                      { return m_UDP;                                                                 }
      #endif
        SocketManager&      GetSocketManager        (void)                                      { return m_SocketManager;                                                       }

      #if (IP_USE_DHCP == DEF_ENABLED)
        bool                IsDHCP_Enable           (void)                                      { return m_DHCP.IsItEnabled();                                                  }
        IP_Address_t        GetDHCP_IP              (void)                                      { return m_DHCP.GetIP();                                                        }
        IP_Address_t        GetDHCP_GatewayIP       (void)                                      { return m_DHCP.GetGatewayIP();                                                 }
        IP_Address_t        GetDHCP_SubnetMask      (void)                                      { return m_DHCP.GetSubnetMask();                                                }
        IP_Address_t        GetDHCP_ServerIP        (void)                                      { return m_DHCP.GetServerIP();                                                  }
        IP_Address_t        GetDHCP_DNS_IP          (void)                                      { return m_DHCP.GetDNS_IP();                                                    }

        IP_Address_t        GetActiveIP             (void)                                      { return (IsDHCP_Enable() == true) ? GetDHCP_IP()         : m_StaticIP;         }
        IP_Address_t        GetActiveGatewayIP      (void)                                      { return (IsDHCP_Enable() == true) ? GetDHCP_GatewayIP()  : m_StaticGatewayIP;  }
        IP_Address_t        GetActiveSubnetMask     (void)                                      { return (IsDHCP_Enable() == true) ? GetDHCP_SubnetMask() : m_StaticSubnetMask; }
        IP_Address_t        GetActiveDNS_IP         (void)                                      { return (IsDHCP_Enable() == true) ? GetDHCP_DNS_IP()     : m_StaticDNS_IP;     }
      #else
        IP_Address_t        GetActiveIP             (void)                                      { return m_StaticIP;                                                            }
        IP_Address_t        GetActiveGatewayIP      (void)                                      { return m_StaticGatewayIP;                                                     }
        IP_Address_t        GetActiveSubnetMask     (void)                                      { return m_StaticSubnetMask;                                                    }
        IP_Address_t        GetActiveDNS_IP         (void)                                      { return m_StaticDNS_IP;                                                        }
      #endif

        IP_Manager*         GetIP_Manager           (void)                                      { return &m_IP_Manager;                                                         }
        IP_Address_t        GetStaticGatewayIP      (void)                                      { return m_StaticGatewayIP;                                                     }
        IP_Address_t        GetStaticSubnetMask     (void)                                      { return m_StaticSubnetMask;                                                    }
        IP_Address_t        GetStaticIP             (void)                                      { return m_StaticIP;                                                            }
        IP_Address_t        GetStaticDNS_IP         (void)                                      { return m_StaticDNS_IP;                                                        }
        void                GetMAC_Address          (IP_MAC_Address_t* pMAC)                    { memcpy(pMAC, &m_MAC_Address, IP_MAC_ADDRESS_SIZE);                            }
        uint16_t            GetMTU                  (void)                                      { return m_MTU;                                                                 }

        void                SetStaticGatewayIP      (IP_Address_t GatewayIP)                    { m_StaticGatewayIP = GatewayIP;                                                }
        void                SetStaticSubnetMask     (IP_Address_t SubnetMask)                   { m_StaticSubnetMask = SubnetMask;                                              }
        void                SetStaticIP             (IP_Address_t StaticIP)                     { m_StaticIP = StaticIP;                                                        }
        void                SetStaticDNS_IP         (IP_Address_t DNS_IP)                       { m_StaticDNS_IP = DNS_IP;                                                      }
        void                SetMAC_Address          (const IP_MAC_Address_t* pMAC)              { memcpy(&m_MAC_Address, pMAC, IP_MAC_ADDRESS_SIZE);                            }
        void                SetMTU                  (uint16_t MTU)                              { m_MTU = MTU;                                                                  }

        bool                IsItMyMAC_Address       (const IP_MAC_Address_t* pMAC)              { return memcmp(pMAC->Byte, m_MAC_Address.Byte, IP_MAC_ADDRESS_SIZE) == 0;      }

        SystemState_e       InitializeMsgQ          (void);
        nOS_Queue*          GetMsgQ                 (void)                                      { return &m_Q_Msg;                                                              }

        void                RegisterSendCallback    (SendCallback_t callback, void* pContext)   { m_SendCallback = callback; m_SendContext = pContext;                          }
        SystemState_e       SendPacket              (IP_PacketMsg_t* pMsg);

    private:

        IP_Manager                      m_IP_Manager;
        ARP_Manager                     m_ARP;                                  // Address Resolution Protocol
        SocketManager                   m_SocketManager;                        // Socket Manager

      #if (IP_USE_ICMP == DEF_ENABLED)
        ICMP_Manager                    m_ICMP;                                 // Internet Control Message Protocol
      #endif

      #if (IP_USE_DHCP == DEF_ENABLED)
        DHCPv4_Manager                  m_DHCP;                                 // Dynamic Host Control Protocol. Need UDP
      #endif

      #if (IP_USE_DNS == DEF_ENABLED)
        DNS_Manager                     m_DNS;                                  // Domain name system Protocol. Need UDP
      #endif

      #if (IP_USE_NTP == DEF_ENABLED)
        NTP_Manager                     m_NTP;                                  // Network Time Protocol
      #endif

      #if (IP_USE_SNTP == DEF_ENABLED)
        SNTP_Manager                    m_SNTP;                                 // Simple Network Time Protocol
      #endif

      #if (IP_USE_RAW == DEF_ENABLED)
        RAW_Manager                     m_RAW;                                  // RAW Manager
      #endif


      #if (IP_USE_TCP_SERVER == DEF_ENABLED) || (IP_USE_TCP_CLIENT == DEF_ENABLED)
        TCP_Manager                     m_TCP;                                  // Transport Control Protocol Cleint Side
      #endif

      #if (IP_USE_UDP == DEF_ENABLED)
        UDP_Manager                     m_UDP;                                  // User Datagram Protocol
      #endif

        IF_ID_e                         m_IF_ID;
        SendCallback_t                  m_SendCallback = nullptr;
        void*                           m_SendContext  = nullptr;

        bool                            m_LinkChange   = false;
        ETH_LinkState_e                 m_LinkState    = ETH_LINK_DOWN;
        ETH_LinkSpeed_e                 m_LinkSpeed    = ETH_PHY_SPEED_10M;
        bool                            m_IP_Valid     = false;
        uint16_t                        m_MTU;

        IP_ETH_Config_t*                m_pEthernetIF;                          // Ethernet Configuration

        // There is default in config.. but it can be changed
        IP_Address_t                    m_StaticGatewayIP;                      // Gateway IP Address from server
        IP_Address_t                    m_StaticSubnetMask;                     // Subnet Mask from server
        IP_Address_t                    m_StaticIP;                             // IP Address from server
        IP_Address_t                    m_StaticDNS_IP;                         // DNS Server IP Address from server
        IP_MAC_Address_t                m_MAC_Address;
        const char*                     m_pHostName;



        nOS_Queue                       m_Q_Msg;
        IP_PacketMsg_t*                 m_ArrayPacketPtr[IP_PACKET_Q_SIZE];

        ETH_IF_Driver                   m_IF_Driver;
        static const IP_Config_t        m_Config[IP_NUMBER_OF_INTERFACE];
};

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_ETHERNET == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------


