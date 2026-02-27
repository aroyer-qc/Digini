//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_ethernet_interface.h
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

#pragma once

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class ETH_LinkDriver

{
    public:
    
        virtual bool                    Initialize              (void* pContext)                                                = 0;
        virtual bool                    SendFrame               (const uint8_t* pData, size_t Length)                           = 0;
        virtual bool                    ReceiveFrame            (uint8_t* pBuffer, size_t* pLength)                             = 0;
        virtual bool                    LinkIsUp                (void)                                                          = 0;
};

class ETH_MAC_DriverInterface //ETH_DriverInterface
{
    public:

        virtual SystemState_e           Initialize              (void* pContext, uint8_t PHY_Address)                           = 0;    // Initialize Ethernet MAC Device.
        virtual SystemState_e           InitializeInterface     (void)                                                          = 0;    // Post Initialize Ethernet DMA.

        virtual void                    Start                   (void)                                                          = 0;    // Start ETH module
        virtual SystemState_e           GetMacAddress           (      IP_MAC_Address_t* pMAC_Address)                          = 0;    // Get Ethernet MAC Address.
        virtual SystemState_e           SetMacAddress           (const IP_MAC_Address_t* pMAC_Address)                          = 0;    // Set Ethernet MAC Address.
        virtual SystemState_e           SetAddressFilter        (const IP_MAC_Address_t* pMAC_Address, uint32_t NbAddress)      = 0;    // Configure Address Filter.
        virtual SystemState_e           SendTX_Packet           (IP_PacketMsg_t** ppPacketMsg)                                  = 0;    // Send Ethernet frame.
        virtual SystemState_e           GetRX_Packet            (IP_PacketMsg_t** ppPacketMsg)                                  = 0;    // Read data of received Ethernet frame.
        virtual uint32_t                GetRX_FrameSize         (void)                                                          = 0;    // Get size of received Ethernet frame.
      #if (ETH_USE_TIME_STAMP == DEF_ENABLED)
        virtual SystemState_e           GetRX_FrameTime         (ETH_MacTime_t* pTime)                                          = 0;    // Get time of received Ethernet frame.
        virtual SystemState_e           GetTX_FrameTime         (ETH_MacTime_t* pTime)                                          = 0;    // Get time of transmitted Ethernet frame.
        virtual SystemState_e           ControlTimer            (ETH_ControlTimer_e Control, ETH_MacTime_t* pTime)              = 0;    // Control Precision Timer.
      #endif

        virtual SystemState_e           PHY_Read                (uint8_t PHY_Address, uint8_t RegisterAddress, uint16_t* pData) = 0; // Read Ethernet PHY Register through Management Interface.
        virtual SystemState_e           PHY_Write               (uint8_t PHY_Address, uint8_t RegisterAddress, uint16_t   Data) = 0; // Write Ethernet PHY Register through Management Interface.
};

class PHY_DriverInterface
{
    public:

        virtual SystemState_e           Initialize              (ETH_MAC_DriverInterface* pETH_Driver, uint32_t PHY_Address)    = 0;
        virtual SystemState_e           Uninitialize            (void)                                                          = 0;
        virtual SystemState_e           PowerControl            (ETH_PowerState_e state)                                        = 0;
        virtual SystemState_e           SetInterface            (ETH_MediaInterface_e Interface)                                = 0;
        virtual SystemState_e           SetMode                 (ETH_PHY_Mode_e Mode)                                           = 0;
        virtual ETH_LinkState_e         GetLinkState            (void)                                                          = 0;
        virtual ETH_LinkInfo_t          GetLinkInfo             (void)                                                          = 0;
        virtual uint8_t                 GetPHY_Address          (void)                                                          = 0;
      #if (ETH_USE_PHY_LINK_IRQ == DEF_ENABLED)
        virtual SystemState_e           SetLinkUpInterrupt      (bool State)                                                    = 0;
      #endif
};

class TCP_Socket
{
    public:

        virtual                         ~TCP_Socket             (){}

        virtual size_t                  Send                    (const uint8_t* pBuffer, size_t Length)                         = 0;
        virtual size_t                  Receive                 (uint8_t* pBuffer, size_t MaxLength)                            = 0;
        virtual void                    Close                   (void)                                                          = 0;
        virtual bool                    IsConnected             (void) const                                                    = 0;
        virtual TCP_State_e             GetState                (void) const                                                    = 0;
};

class TCP_Manager
{
    public:

        virtual                         ~TCP_Manager            (){}
      #if (IP_USE_TCP_CLIENT == DEF_ENABLED)
        virtual TCP_Socket*             Connect                 (const IP_Address_t& ServerIP, uint16_t Port)                   = 0;
      #endif

      #if (IP_USE_TCP_SERVER == DEF_ENABLED)
        //SystemState_e   EnterListen                     (Socket* pSocket, uint16_t Backlog)                       = 0;
        //void            Close                           (Socket* pSocket)                                         = 0;
      #endif

        virtual void                    Process                 (void)                                                          = 0;
        virtual void                    ProcessSegment          (IP_PacketMsg_t* pPacket)                                       = 0;
        virtual bool                    SendSegment             (TCP_Socket* pSocket, const uint8_t* pPayload, size_t Length, uint8_t Flags, bool Retransmit = false) = 0;
};

//-------------------------------------------------------------------------------------------------

