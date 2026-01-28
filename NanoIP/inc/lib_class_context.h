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
// Typedef(s)
//-------------------------------------------------------------------------------------------------

typedef SystemState_e (*SendCallback_t)(void* Context, IP_PacketMsg_t** ppPacketMsg);

//-------------------------------------------------------------------------------------------------
// Class
//-------------------------------------------------------------------------------------------------

class NetworkContext
{
    public:

        ETH_LinkState_e     GetLinkState            (void)          const                       { return m_LinkState;                                                       }
        void                SetLinkState            (ETH_LinkState_e State)                     { m_LinkState = State;                                                      }

        ETH_LinkSpeed_e     GetLinkSpeed            (void)          const                       { return m_LinkSpeed;                                                       }
        void                SetLinkSpeed            (ETH_LinkSpeed_e Speed)                     { m_LinkSpeed = Speed;                                                      }

        bool                GetLinkChange           (void)                                      { return m_LinkChange;                                                      }
        void                SetLinkChange           (bool State)                                { m_LinkChange = State;                                                     }

        bool                IsIP_Valid              (void)                                      { return m_IP_Valid;                                                        }
        void                SetIP_Valid             (bool State)                                { m_IP_Valid = State;                                                       }

        void                SetHostName             (const char* pHostName)                     { m_pHostName = pHostName;                                                  }
        const char*         GetHostName             (void)                                      { return m_pHostName;                                                       }


      #if (IP_USE_DHCP == DEF_ENABLED)

        bool                IsDHCP_Enable           (void)                                      { return m_DHCP_Enable;                                                     }
        void                SetDHCP_Enable          (bool State)                                { m_DHCP_Enable = State;                                                    }

        IP_Address_t        GetDHCP_GatewayIP       (void)                                      { return m_DHCP_GatewayIP;                                                  }
        IP_Address_t        GetDHCP_SubnetMask      (void)                                      { return m_DHCP_SubnetMask;                                                 }
        IP_Address_t        GetDHCP_ServerIP        (void)                                      { return m_DHCP_ServerIP;                                                   }
        IP_Address_t        GetDHCP_IP              (void)                                      { return m_DHCP_IP;                                                         }
        IP_Address_t        GetDHCP_DNS_IP          (void)                                      { return m_DHCP_DNS_IP;                                                     }

        void                SetDHCP_GatewayIP       (IP_Address_t GatewayIP)                    { m_DHCP_GatewayIP = GatewayIP;                                             }
        void                SetDHCP_SubnetMask      (IP_Address_t SubnetMask)                   { m_DHCP_SubnetMask = SubnetMask;                                           }
        void                SetDHCP_ServerIP        (IP_Address_t ServerIP)                     { m_DHCP_ServerIP = ServerIP;                                               }
        void                SetDHCP_IP              (IP_Address_t DHCP_IP)                      { m_DHCP_IP = DHCP_IP;                                                      }
        void                SetDHCP_DNS_IP          (IP_Address_t DHCP_DNS_IP)                  { m_DHCP_DNS_IP = DHCP_DNS_IP;                                              }

        IP_Address_t        GetActiveGatewayIP      (void)                                      { return (m_DHCP_Enable == true) ? m_DHCP_GatewayIP  : m_StaticGatewayIP;   }
        IP_Address_t        GetActiveSubnetMask     (void)                                      { return (m_DHCP_Enable == true) ? m_DHCP_SubnetMask : m_StaticSubnetMask;  }
        IP_Address_t        GetActiveIP             (void)                                      { return (m_DHCP_Enable == true) ? m_DHCP_IP         : m_StaticIP;          }
        IP_Address_t        GetActiveDNS_IP         (void)                                      { return (m_DHCP_Enable == true) ? m_DHCP_DNS_IP     : m_StaticDNS_IP;      }

      #endif

        class IP_Manager*   GetIP_Manager           (void)                                      { return m_IP_Manager;                                                      }
        void                SetIP_Manager           (IP_Manager* Manager)                       { m_IP_Manager = Manager;                                                   }

        IP_Address_t        GetStaticGatewayIP      (void)                                      { return m_StaticGatewayIP;                                                 }
        void                SetStaticGatewayIP      (IP_Address_t GatewayIP)                    { m_StaticGatewayIP = GatewayIP;                                            }

        IP_Address_t        GetStaticSubnetMask     (void)                                      { return m_StaticSubnetMask;                                                }
        void                SetStaticSubnetMask     (IP_Address_t SubnetMask)                   { m_StaticSubnetMask = SubnetMask;                                          }

        IP_Address_t        GetStaticIP             (void)                                      { return m_StaticIP;                                                        }
        void                SetStaticIP             (IP_Address_t StaticIP)                     { m_StaticIP = StaticIP;                                                    }

        IP_Address_t        GetStaticDNS_IP         (void)                                      { return m_StaticDNS_IP;                                                    }
        void                SetStaticDNS_IP         (IP_Address_t DNS_IP)                       { m_StaticDNS_IP = DNS_IP;                                                  }

        void                GetMAC_Address          (IP_MAC_Address_t* pMAC_Address)            { memcpy(pMAC_Address, &m_MAC_Address, IP_MAC_ADDRESS_SIZE);                }
        void                SetMAC_Address          (const IP_MAC_Address_t* pMAC_Address)      { memcpy(&m_MAC_Address, pMAC_Address, IP_MAC_ADDRESS_SIZE);                }

        uint16_t            GetMTU                  (void)                                      { return m_MTU;                                                             }
        void                SetMTU                  (uint16_t MTU)                              { m_MTU = MTU;                                                              }


        SystemState_e       InitializeMsgQ          (void);
        nOS_Queue*          GetMsgQ                 (void)                                      { return &m_Q_Msg;                                                          }



        void                RegisterSendCallback    (SendCallback_t callback, void* pContext)   { m_SendCallback = callback; m_SendContext = pContext;                      }
        SystemState_e       SendPacket              (IP_PacketMsg_t* pMsg);

    private:

        IP_Manager*         m_IP_Manager   = nullptr;
        SendCallback_t      m_SendCallback = nullptr;
        void*               m_SendContext  = nullptr;

        bool                m_LinkChange   = false;
        ETH_LinkState_e     m_LinkState    = ETH_LINK_DOWN;
        ETH_LinkSpeed_e     m_LinkSpeed    = ETH_PHY_SPEED_10M;
        bool                m_IP_Valid     = false;
        uint16_t            m_MTU;


      #if (IP_USE_DHCP == DEF_ENABLED)
        bool                m_DHCP_Enable;

        IP_Address_t        m_DHCP_GatewayIP;                       // Gateway IP Address from server
        IP_Address_t        m_DHCP_SubnetMask;                      // Subnet Mask from server
        IP_Address_t        m_DHCP_ServerIP;                        // Server IP
        IP_Address_t        m_DHCP_IP;                              // IP Address from server
        IP_Address_t        m_DHCP_DNS_IP;                          // DNS Server IP Address from server
        const char*         m_pHostName;

      #endif

        IP_Address_t        m_StaticGatewayIP;                      // Gateway IP Address from server
        IP_Address_t        m_StaticSubnetMask;                     // Subnet Mask from server
        IP_Address_t        m_StaticIP;                             // IP Address from server
        IP_Address_t        m_StaticDNS_IP;                         // DNS Server IP Address from server

        IP_MAC_Address_t    m_MAC_Address;
        nOS_Queue           m_Q_Msg;
        IP_PacketMsg_t*     m_ArrayPacketPtr[IP_MANAGER_PACKET_Q_SIZE];
};

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_ETHERNET == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------


