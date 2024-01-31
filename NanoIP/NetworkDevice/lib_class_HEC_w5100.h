//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_HEC_W5100.h
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

#if (USE_ETH_DRIVER == DEF_ENABLED)


Driver to build !!!!

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------



TODO
//  General Register
#pragma ADDRESS         mr_addr             030000H         // Gateway IP Register
#pragma ADDRESS         gar_addr            030001H         // Gateway IP Register
#pragma ADDRESS         subr_addr           030005H         // Subnet Mask Register
#pragma ADDRESS         shar_addr           030009H         // Source MAC Register
#pragma ADDRESS         sipr_addr           03000FH         // Source IP Register
#pragma ADDRESS         isr_addr            030015H         // Interrupt Status Register
#pragma ADDRESS         imr_addr            030016H         // Interrupt Mask Register
#pragma ADDRESS         rtr_addr            030017H         // Timeout Register (1 is 100us)
#pragma ADDRESS         rcr_addr            030019H         // Retry Count Reigster
#pragma ADDRESS         rmsr_addr           03001AH         // Receive memory Size Register
#pragma ADDRESS         tmsr_addr           03001BH         // Transmit memory Size Register
#pragma ADDRESS         patr_addr           03001CH         // Authentication Type Register in PPPoE mode
#pragma ADDRESS         ptimer_addr         030028H
#pragma ADDRESS         pmagic_addr         030029H
#pragma ADDRESS         uipr_addr           03002AH         // Unreachable IP Register in UDP mode
#pragma ADDRESS         uport_addr          03002EH         // Unreachable Port Register in UDP mode

//  Socket Register
#pragma ADDRESS         sock_mr_addr        030400H         // Socket Mode Register
#pragma ADDRESS         sock_cr_addr        030401H         // Socket Command Register
#pragma ADDRESS         sock_ir_addr        030402H         // Socket Interrupt Register
#pragma ADDRESS         sock_sr_addr        030403H         // Socket Status Register
#pragma ADDRESS         sock_sportr_addr    030404H         // Socket Source Port Register
#pragma ADDRESS         sock_dhar_addr      030406H         // Socket Destination Hardware Address Register
#pragma ADDRESS         sock_dipr_addr      03040CH         // Socket Destination IP Address Register
#pragma ADDRESS         sock_dportr_addr    030410H         // Socket Destination Port Register
#pragma ADDRESS         sock_mssr_addr      030412H         // Socket Maximum Segment Size Register
#pragma ADDRESS         sock_protor_addr    030414H         // Socket Protocol Register
#pragma ADDRESS         sock_tosr_addr      030415H         // Socket Type Of Service Register
#pragma ADDRESS         sock_ttlr_addr      030416H         // Socket Time To Live Register
#pragma ADDRESS         sock_tx_fsr_addr    030420H         // Socket TX Free Size Register
#pragma ADDRESS         sock_tx_rpr_addr    030422H         // Socket TX Read Pointer Register
#pragma ADDRESS         sock_tx_wpr_addr    030424H         // Socket TX Write Pointer Register
#pragma ADDRESS         sock_rx_rsr_addr    030426H         // Socket RX Received Size Register
#pragma ADDRESS         sock_rx_rpr_addr    030428H         // Socket RX Read Pointer Register

#define W5100_NUMBER_OF_SOCKET          4
#define W5100_MEMORY_TX_BASE            (DWORD)0x34000
#define W5100_MEMORY_RX_BASE            (DWORD)0x36000

//------------------------------------------------------
//  General Register
//------------------------------------------------------
//  Mode Register
//------------------------------------------------------
#define     mr                          mr_addr.by
#define     mr_ind                      mr_addr.b.b0
#define     mr_ai                       mr_addr.b.b1
#define     mr_lb                       mr_addr.b.b2
#define     mr_pppoe                    mr_addr.b.b3
#define     mr_pb                       mr_addr.b.b4
#define     mr_reset                    mr_addr.b.b7

// MODE register values
#define     MR_IND                      0x01                                // < Enable indirect mode
#define     MR_AI                       0x02                                // < Auto-increment in indirect mode
#define     MR_LB                       0x04                                // < Little or big endian selector in indirect mode
#define     MR_PPPOE                    0x08                                // < Enable pppoe
#define     MR_PB                       0x10                                // < Ping block
#define     MR_RST                      0x80                                // < Reset

//------------------------------------------------------
//  Gateway IP Address Register
//------------------------------------------------------
#define     gar(v)                      gar_addr.dw = LIB_SwapDW(v)
#define     gar0                        gar_addr.by.by3
#define     gar1                        gar_addr.by.by2
#define     gar2                        gar_addr.by.by1
#define     gar3                        gar_addr.by.by0

//------------------------------------------------------
//  Subnet Mask Register
//------------------------------------------------------
#define     subr(v)                     subr_addr.dw = LIB_SwapDW(v)
#define     subr0                       subr_addr.by.by3
#define     subr1                       subr_addr.by.by2
#define     subr2                       subr_addr.by.by1
#define     subr3                       subr_addr.by.by0

//------------------------------------------------------
//  Source MAC Address Register
//------------------------------------------------------
#define     shar                        shar_addr.ba[0]
#define     shar0                       shar_addr.ba[0]
#define     shar1                       shar_addr.ba[1]
#define     shar2                       shar_addr.ba[2]
#define     shar3                       shar_addr.ba[3]
#define     shar4                       shar_addr.ba[4]
#define     shar5                       shar_addr.ba[5]

//------------------------------------------------------
//  Source IP Address Register
//------------------------------------------------------
#define     sipr(v)                     sipr_addr.dw = LIB_SwapDW(v)
#define     sipr0                       sipr_addr.by.by3
#define     sipr1                       sipr_addr.by.by2
#define     sipr2                       sipr_addr.by.by1
#define     sipr3                       sipr_addr.by.by0

//------------------------------------------------------
//  Interrupt Register
//------------------------------------------------------
#define     isr                         isr_addr.by
#define     isr_s0                      isr_addr.b.b0
#define     isr_s1                      isr_addr.b.b1
#define     isr_s2                      isr_addr.b.b2
#define     isr_s3                      isr_addr.b.b3
#define     isr_pppoe                   isr_addr.b.b5
#define     isr_unreacheable            isr_addr.b.b6
#define     isr_conflict                isr_addr.b.b7

// IR register values
#define     ISR_SOCK(ch)                (0x01 << ch)                        // < Check socket interrupt
#define     ISR_PPPoE                   0x20                                // < Get the PPPoE close message
#define     ISR_UNREACH                 0x40                                // < Get the destination unreachable message in UDP sending
#define     ISR_CONFLICT                0x80                                // < Check ip conflict

//------------------------------------------------------
//  Interrupt Mask Register
//------------------------------------------------------
#define     imr                         imr_addr.by
#define     imr_s0                      imr_addr.b.b0
#define     imr_s1                      imr_addr.b.b1
#define     imr_s2                      imr_addr.b.b2
#define     imr_s3                      imr_addr.b.b3
#define     imr_pppoe                   imr_addr.b.b5
#define     imr_unreacheable            imr_addr.b.b6
#define     imr_conflict                imr_addr.b.b7

//------------------------------------------------------
//  Retry Time-value Register
//------------------------------------------------------
#define     rtr_write(v)                rtr_addr = LIB_SwapW(v)
#define     rtr_read()                  LIB_SwapW(rtr_addr)

//------------------------------------------------------
//  Retry Count Register
//------------------------------------------------------
#define     rcr                         rcr_addr

//------------------------------------------------------
//  RX Memory Size Register
//------------------------------------------------------
#define     rmsr                        rmsr_addr

//------------------------------------------------------
//  TX Memory Size Register
//------------------------------------------------------
#define     tmsr                        tmsr_addr

//------------------------------------------------------
//  Authentication Type in PPPoE mode
//------------------------------------------------------
#define     patr                        LIB_SwapW(patr_addr)

//------------------------------------------------------
//  PPP Link Control Protocol Request Timer Register
//------------------------------------------------------
#define     ptimer                      ptimer_addr

//------------------------------------------------------
//  PPP Link Control Protocol Magic Number Register
//------------------------------------------------------
#define     pmagic                      pmagic_addr

//------------------------------------------------------
//  Unreachable IP Address Register
//------------------------------------------------------
#define     uipr(v)                     LIB_SwapDW(uipr_addr.dw)
#define     uipr0                       uipr_addr.by.by3
#define     uipr1                       uipr_addr.by.by2
#define     uipr2                       uipr_addr.by.by1
#define     uipr3                       uipr_addr.by.by0

//------------------------------------------------------
//  Unreachable Port Register
//------------------------------------------------------
#define     uport                       LIB_SwapW(uport_addr)

//------------------------------------------------------
//  Socket Register
//------------------------------------------------------
//  Socket Mode Register
//------------------------------------------------------
#define     sock_mr_write(ch,v)         *(far BYTE*)((DWORD)&sock_mr_addr + ((DWORD)(ch) * (DWORD)0x100)) = (v)
#define     sock_mr_read(ch)            *(far BYTE*)((DWORD)&sock_mr_addr + ((DWORD)(ch) * (DWORD)0x100))
#define     sock_mr_p0(ch)              *(far UBYTE*)((DWORD)&sock_mr_addr + ((DWORD)(ch) * (DWORD)0x100))->b.b0
#define     sock_mr_p1(ch)              *(far UBYTE*)((DWORD)&sock_mr_addr + ((DWORD)(ch) * (DWORD)0x100))->b.b1
#define     sock_mr_p2(ch)              *(far UBYTE*)((DWORD)&sock_mr_addr + ((DWORD)(ch) * (DWORD)0x100))->b.b2
#define     sock_mr_p3(ch)              *(far UBYTE*)((DWORD)&sock_mr_addr + ((DWORD)(ch) * (DWORD)0x100))->b.b3
#define     sock_mr_nd_mc(ch)           *(far UBYTE*)((DWORD)&sock_mr_addr + ((DWORD)(ch) * (DWORD)0x100))->b.b5
#define     sock_mr_multi(ch)           *(far UBYTE*)((DWORD)&sock_mr_addr + ((DWORD)(ch) * (DWORD)0x100))->b.b7

// Sn_MR values
#define     Sn_MR_CLOSE                 0x00                                // < Unused socket
#define     Sn_MR_TCP                   0x01                                // < TCP
#define     Sn_MR_UDP                   0x02                                // < UDP
#define     Sn_MR_IPRAW                 0x03                                // < IP LAYER RAW SOCK
#define     Sn_MR_MACRAW                0x04                                // < MAC LAYER RAW SOCK
#define     Sn_MR_PPPOE                 0x05                                // < PPPoE
#define     Sn_MR_ND                    0x20                                // < No Delayed Ack(TCP) flag
#define     Sn_MR_MULTI                 0x80                                // < Support multicating


//------------------------------------------------------
//  Socket Command Register
//------------------------------------------------------
#define     sock_cr_write(ch,v)         *(far BYTE*)((DWORD)&sock_cr_addr + ((DWORD)(ch) * (DWORD)0x100)) = (v)
#define     sock_cr_read(ch)            *(far BYTE*)((DWORD)&sock_cr_addr + ((DWORD)(ch) * (DWORD)0x100))

// Sn_CR values
#define     Sn_CR_OPEN                  0x01                                // < Initialize or open socket
#define     Sn_CR_LISTEN                0x02                                // < Wait connection request in tcp mode(Server mode)
#define     Sn_CR_CONNECT               0x04                                // < Send connection request in tcp mode(Client mode)
#define     Sn_CR_DISCON                0x08                                // < Send closing reqeuset in tcp mode
#define     Sn_CR_CLOSE                 0x10                                // < Close socket
#define     Sn_CR_SEND                  0x20                                // < Updata txbuf pointer, send data
#define     Sn_CR_SEND_MAC              0x21                                // < Send data with MAC address, so without ARP process
#define     Sn_CR_SEND_KEEP             0x22                                // < Send keep alive message
#define     Sn_CR_RECV                  0x40                                // < Update rxbuf pointer, recv data

//------------------------------------------------------
//  Socket Interrupt Register
//------------------------------------------------------
#define     sock_ir_write(ch,v)         *(far BYTE*)((DWORD)&sock_ir_addr + ((DWORD)(ch) * (DWORD)0x100)) = (v)
#define     sock_ir_read(ch)            *(far BYTE*)((DWORD)&sock_ir_addr + ((DWORD)(ch) * (DWORD)0x100))
#define     sock_ir_connect(ch)         *(far UBYTE*)((DWORD)&sock_ir_addr + ((DWORD)(ch) * (DWORD)0x100))->b.b0
#define     sock_ir_disconnect(ch)      *(far UBYTE*)((DWORD)&sock_ir_addr + ((DWORD)(ch) * (DWORD)0x100))->b.b1
#define     sock_ir_received(ch)        *(far UBYTE*)((DWORD)&sock_ir_addr + ((DWORD)(ch) * (DWORD)0x100))->b.b2
#define     sock_ir_timeout(ch)         *(far UBYTE*)((DWORD)&sock_ir_addr + ((DWORD)(ch) * (DWORD)0x100))->b.b3
#define     sock_ir_send_ok(ch)         *(far UBYTE*)((DWORD)&sock_ir_addr + ((DWORD)(ch) * (DWORD)0x100))->b.b4

// Sn_IR values
#define     Sn_IR_CON                   (BYTE)0x01                           // < Established connection
#define     Sn_IR_DISCON                (BYTE)0x02                           // < Closed socket
#define     Sn_IR_RECV                  (BYTE)0x04                           // < Receiving data
#define     Sn_IR_TIMEOUT               (BYTE)0x08                           // < Assert timeout
#define     Sn_IR_SEND_OK               (BYTE)0x10                           // < Complete sending

//------------------------------------------------------
//  Socket Status Register
//------------------------------------------------------
#define     sock_sr_write(ch,v)         *(far BYTE*)((DWORD)&sock_sr_addr + ((DWORD)(ch) * (DWORD)0x100)) = (v)
#define     sock_sr_read(ch)            *(far BYTE*)((DWORD)&sock_sr_addr + ((DWORD)(ch) * (DWORD)0x100))

// Sn_SR values
#define SOCK_CLOSED                     0x00                                // < Closed
#define SOCK_INIT                       0x13                                // < Init state
#define SOCK_LISTEN                     0x14                                // < Listen state
#define SOCK_SYNSENT                    0x15                                // < Connection state
#define SOCK_SYNRECV                    0x16                                // < Connection state
#define SOCK_ESTABLISHED                0x17                                // < Success to connect
#define SOCK_FIN_WAIT                   0x18                                // < Closing state
#define SOCK_CLOSING                    0x1A                                // < Closing state
#define SOCK_TIME_WAIT                  0x1B                                // < Closing state
#define SOCK_CLOSE_WAIT                 0x1C                                // < Closing state
#define SOCK_LAST_ACK                   0x1D                                // < Closing state
#define SOCK_UDP                        0x22                                // < UDP socket
#define SOCK_IPRAW                      0x32                                // < IP raw mode socket
#define SOCK_MACRAW                     0x42                                // < MAC raw mode socket
#define SOCK_PPPOE                      0x5F                                // < PPPoE socket

//------------------------------------------------------
//  Socket Source Port Register
//------------------------------------------------------
#define     sock_sportr_write(ch,v)     *(far WORD*)((DWORD)&sock_sportr_addr + ((DWORD)(ch) * (DWORD)0x100)) = LIB_SwapW(v)
#define     sock_sportr_read(ch)        LIB_SwapW(*(far WORD*)((DWORD)&sock_sportr_addr + ((DWORD)(ch) * (DWORD)0x100)))

//------------------------------------------------------
//  Socket Destination Hardware Address Register
//------------------------------------------------------
#define     sock_dhar0(ch)              *(far MAC*)((DWORD)&sock_dhar_addr + ((DWORD)(ch) * (DWORD)0x100))->ba[0]
#define     sock_dhar1(ch)              *(far MAC*)((DWORD)&sock_dhar_addr + ((DWORD)(ch) * (DWORD)0x100))->ba[1]
#define     sock_dhar2(ch)              *(far MAC*)((DWORD)&sock_dhar_addr + ((DWORD)(ch) * (DWORD)0x100))->ba[2]
#define     sock_dhar3(ch)              *(far MAC*)((DWORD)&sock_dhar_addr + ((DWORD)(ch) * (DWORD)0x100))->ba[3]
#define     sock_dhar4(ch)              *(far MAC*)((DWORD)&sock_dhar_addr + ((DWORD)(ch) * (DWORD)0x100))->ba[4]
#define     sock_dhar5(ch)              *(far MAC*)((DWORD)&sock_dhar_addr + ((DWORD)(ch) * (DWORD)0x100))->ba[5]

//------------------------------------------------------
//  Socket Destination IP Address Register
//------------------------------------------------------
#define     sock_dipr_write(ch,v)       *(far DWORD*)((DWORD)&sock_dipr_addr + ((DWORD)(ch) * (DWORD)0x100)) = LIB_SwapDW(v)
#define     sock_dipr_read(ch)          *(far DWORD*)((DWORD)&sock_dipr_addr + ((DWORD)(ch) * (DWORD)0x100))
#define     sock_dipr0(ch)              *((far UDWORD*)((DWORD)&sock_dipr_addr + ((DWORD)(ch) * (DWORD)0x100)))->by3
#define     sock_dipr1(ch)              *((far UDWORD*)((DWORD)&sock_dipr_addr + ((DWORD)(ch) * (DWORD)0x100)))->by2
#define     sock_dipr2(ch)              *((far UDWORD*)((DWORD)&sock_dipr_addr + ((DWORD)(ch) * (DWORD)0x100)))->by1
#define     sock_dipr3(ch)              *((far UDWORD*)((DWORD)&sock_dipr_addr + ((DWORD)(ch) * (DWORD)0x100)))->by0

//------------------------------------------------------
//  Socket Destination Port Register
//------------------------------------------------------
#define     sock_dportr_write(ch,v)     *(far WORD*)((DWORD)&sock_dportr_addr + ((DWORD)(ch) * (DWORD)0x100)) = LIB_SwapW(v)
#define     sock_dportr_read(ch)        LIB_SwapW(*(far WORD*)((DWORD)&sock_dportr_addr + ((DWORD)(ch) * (DWORD)0x100)))

//------------------------------------------------------
//  Socket Maximum Segment Size Register
//------------------------------------------------------
#define     sock_mssr_write(ch,v)       *(far WORD*)((DWORD)&sock_mssr_addr + ((DWORD)(ch) * (DWORD)0x100)) = LIB_SwapW(v)
#define     sock_mssr_read(ch)          LIB_SwapW(*(far WORD*)((DWORD)&sock_mssr_addr + ((DWORD)(ch) * (DWORD)0x100)))

//------------------------------------------------------
//  Socket Protocol Register
//------------------------------------------------------
#define     sock_protor_write(ch,v)     *(far WORD*)((DWORD)&sock_protor_addr + ((DWORD)(ch) * (DWORD)0x100)) = LIB_SwapW(v)
#define     sock_protor_read(ch)        LIB_SwapW*(far WORD*)((DWORD)&sock_protor_addr + ((DWORD)(ch) * (DWORD)0x100)))

// IP PROTOCOL
#define     IPPROTO_IP                  0                                   // < Dummy for IP
#define     IPPROTO_ICMP                1                                   // < Control message protocol
#define     IPPROTO_IGMP                2                                   // < Internet group management protocol
#define     IPPROTO_GGP                 3                                   // < Gateway^2 (deprecated)
#define     IPPROTO_TCP                 6                                   // < TCP
#define     IPPROTO_PUP                 12                                  // < PUP
#define     IPPROTO_UDP                 17                                  // < UDP
#define     IPPROTO_IDP                 22                                  // < XNS idp
#define     IPPROTO_ND                  77                                  // < UNOFFICIAL net disk protocol
#define     IPPROTO_RAW                 255                                 // < Raw IP packet

//------------------------------------------------------
//  Socket Type Of Service Register (TOS)
//------------------------------------------------------
#define     sock_tosr_write(ch,v)       *(far BYTE*)((DWORD)&sock_tosr_addr + ((DWORD)(ch) * (DWORD)0x100)) = (v)
#define     sock_tosr_read(ch)          *(far BYTE*)((DWORD)&sock_tosr_addr + ((DWORD)(ch) * (DWORD)0x100))

//------------------------------------------------------
//  Socket Time To Live Register (TTL)
//------------------------------------------------------
#define     sock_ttlr_write(ch,v)       *(far BYTE*)((DWORD)&sock_ttlr_addr + ((DWORD)(ch) * (DWORD)0x100)) = (v)
#define     sock_ttlr_read(ch)          *(far BYTE*)((DWORD)&sock_ttlr_addr + ((DWORD)(ch) * (DWORD)0x100))

//------------------------------------------------------
//  Socket TX Free Size Register
//------------------------------------------------------
#define     sock_tx_fsr_write(ch,v)     *(far WORD*)((DWORD)&sock_tx_fsr_addr + ((DWORD)(ch) * (DWORD)0x100)) = LIB_SwapW(v)
#define     sock_tx_fsr_read(ch)        LIB_SwapW(*(far WORD*)((DWORD)&sock_tx_fsr_addr + ((DWORD)(ch) * (DWORD)0x100)))

//------------------------------------------------------
//  Socket TX Memory Read Pointer Address Register
//------------------------------------------------------
#define     sock_tx_rpr_write(ch,v)     *(far WORD*)((DWORD)&sock_tx_rpr_addr + ((DWORD)(ch) * (DWORD)0x100)) = LIB_SwapW(v)
#define     sock_tx_rpr_read(ch)        LIB_SwapW(*(far WORD*)((DWORD)&sock_tx_rpr_addr + ((DWORD)(ch) * (DWORD)0x100)))

//------------------------------------------------------
//  Socket TX Memory Write Pointer Address Register
//------------------------------------------------------
#define     sock_tx_wpr_write(ch,v)     *(far WORD*)((DWORD)&sock_tx_wpr_addr + ((DWORD)(ch) * (DWORD)0x100)) = LIB_SwapW(v)
#define     sock_tx_wpr_read(ch)        LIB_SwapW(*(far WORD*)((DWORD)&sock_tx_wpr_addr + ((DWORD)(ch) * (DWORD)0x100)))

//------------------------------------------------------
//  Socket RX Received Size Register
//------------------------------------------------------
#define     sock_rx_rsr_write(ch,v)     *(far WORD*)((DWORD)&sock_rx_rsr_addr + ((DWORD)(ch) * (DWORD)0x100)) = LIB_SwapW(v)
#define     sock_rx_rsr_read(ch)        LIB_SwapW(*(far WORD*)((DWORD)&sock_rx_rsr_addr + ((DWORD)(ch) * (DWORD)0x100)))

//------------------------------------------------------
//  Socket RX Memory Read Pointer Address Register
//------------------------------------------------------
#define     sock_rx_rpr_write(ch,v)     *(far WORD*)((DWORD)&sock_rx_rpr_addr + ((DWORD)(ch) * (DWORD)0x100)) = LIB_SwapW(v)
#define     sock_rx_rpr_read(ch)        LIB_SwapW(*(far WORD*)((DWORD)&sock_rx_rpr_addr + ((DWORD)(ch) * (DWORD)0x100)))



//------------------------------------------------------
//  Socket to Stack definition
//------------------------------------------------------
#define 	NIC_SOCKET_OPEN				Sn_CR_OPEN
#define		NIC_SOCKET_CLOSE			Sn_CR_CLOSE
#define		NIC_SOCKET_LISTEN			Sn_CR_LISTEN
#define		NIC_SOCKET_CONNECT			Sn_CR_CONNECT
#define		NIC_SOCKET_DISCONNECT		Sn_CR_DISCON


//-------------------------------------------------------------------------------------------------
// macro(s)
//-------------------------------------------------------------------------------------------------

#define W5100_ProcessCmdAndWait(sock,cmd)       {sock_cr_write(sock, cmd); while(sock_cr_read(SocketNumber) != 0);}

#define NIC_Init								W5100_Init
#define NIC_ProcessCommandAndWait				W5100_ProcessCmdAndWait
#define NIC_SocketMode							sock_mr_write
#define NIC_SourcePort							sock_sportr_write
#define NIC_DestinationIP						sock_dipr_write
#define NIC_DestinationPort						sock_dportr_write
#define NIC_GetTX_BufferSize					W5100_GetTX_FSR
#define NIC_GetRX_BufferSize					W5100_GetRX_RSR

//-------------------------------------------------------------------------------------------------
// Class definition(s)
//-------------------------------------------------------------------------------------------------

class HEC_W5100_Driver : public NET_DriverInterface
{
    public:

                                    HEC_W5100_Driver                (uint32_t PHY_Address);

        SystemState_e               Initialize                      (ETH_Driver* pDriver);
        SystemState_e               Uninitialize                    (void);
        SystemState_e               PowerControl                    (ETH_PowerState_e state);
        SystemState_e               SetInterface                    (ETH_MediaInterface_e Interface);
        SystemState_e               SetMode                         (ETH_PHY_Mode_e Mode);
        ETH_LinkState_e             GetLinkState                    (void);
        ETH_LinkInfo_t              GetLinkInfo                     (void);
        uint8_t                     GetPHY_Address                  (void)          { return m_PHY_Address; }
      #if (ETH_USE_PHY_LINK_IRQ == DEF_ENABLED)
        SystemState_e               SetLinkUpInterrupt              (bool State)    { VAR_UNUSED(State); return SYS_READY; };         // No interrupt line for link status on this PHY
      #endif



        // from old driver
        void       Init                     (void);
        void       ProcessTX_Data           (Socket_t SocketNumber, uint8_t* pData, size_t Length);
        void       ProcessRX_Data           (Socket_t SocketNumber, uint8_t* pData, size_t Length);
        void       ReadData                 (Socket_t SocketNumber, uint16_t BufferPtr, uint8_t* pData, size_t Length);
        void       SetSocket_tMemorySize    (uint8_t TX_Size, uint8_t RX_Size);
        uint16_t   GetTX_FSR      		    (Socket_t SocketNumber);
        uint16_t   GetRX_RSR      		    (Socket_t SocketNumber);

    private:

        // Ethernet PHY control structure
        uint32_t                    m_PHY_Address;
        ETH_Driver*                 m_pETH_Driver;                  // Pointer on the class ETH_Driver
        uint16_t                    m_BCR_Register;                 // BCR register value
        ETH_State_e                 m_Flags;                        // Control flags



//from old driver

    
        void    WriteData         (Socket_t SocketNumber, uint8_t* pData, uint16_t BufferPtr, size_t Length);
        void    WriteBuffer       (uint8_t* pSrc, uint8_t* pDst, size_t Length);
        void    ReadBuffer        (uint8_t* pSrc, uint8_t* pDst, size_t Length);

    volatile uint8_t        mr_addr;                        //  Mode Register
    volatile uint32_t       gar_addr;                       //  Gateway IP Address Register
    volatile uint32_t       subr_addr;                      //  Subnet Mask Register
    volatile MAC_t          shar_addr;                      //  Source MAC Address Register
    volatile uint32_t       sipr_addr;                      //  Source IP Address Register
    volatile uint8_t        isr_addr;                       //  Interrupt Register
    volatile uint8_t        imr_addr;                       //  Interrupt Mask Register
    volatile uint16_t       rtr_addr;                       //  Retry Time-value Register
    volatile uint8_t        rcr_addr;                       //  Retry Count Register
    volatile uint8_t        rmsr_addr;                      //  RX Memory Size Register
    volatile uint8_t        tmsr_addr;                      //  TX Memory Size Register
    volatile uint16_t       patr_addr;                      //  Authentication Type in PPPoE mode
    volatile uint8_t        ptimer_addr;                    //  PPP Link Control Protocol Request Timer Register
    volatile uint8_t        pmagic_addr;                    //  PPP Link Control Protocol Magic Number Register
    volatile uint32_t       uipr_addr;                      //  Unreachable IP Address Register
    volatile uint16_t       uport_addr;                     //  Unreachable Port Register
    volatile uint8_t        sock_mr_addr;                   //  Socket_t Mode Register
    volatile uint8_t        sock_cr_addr;                   //  Socket_t Command Register
    volatile uint8_t        sock_ir_addr;                   //  Socket_t Interrupt Register
    volatile uint8_t        sock_sr_addr;                   //  Socket_t Status Register
    volatile uint16_t       sock_sportr_addr;               //  Socket_t Source Port Register
    volatile MAC_t          sock_dhar_addr;                 //  Socket_t Destination Hardware Address Register
    volatile uint32_t       sock_dipr_addr;                 //  Socket_t Destination IP Address Register
    volatile uint16_t       sock_dportr_addr;               //  Socket_t Destination Port Register
    volatile uint16_t       sock_mssr_addr;                 //  Socket_t Maximum Segment Size Register
    volatile uint16_t       sock_protor_addr;               //  Socket_t Protocol Register
    volatile uint8_t        sock_tosr_addr;                 //  Socket_t Type Of Service Register (TOS)
    volatile uint8_t        sock_ttlr_addr;                 //  Socket_t Time To Live Register (TTL)
    volatile uint16_t       sock_tx_fsr_addr;               //  Socket_t TX Free Size Register
    volatile uint16_t       sock_tx_rpr_addr;               //  Socket_t TX Memory Read Pointer Address Register
    volatile uint16_t       sock_tx_wpr_addr;               //  Socket_t TX Memory Write Pointer Address Register
    volatile uint16_t       sock_rx_rsr_addr;               //  Socket_t RX Received Size Register
    volatile uint16_t       sock_rx_rpr_addr;               //  Socket_t RX Memory Read Pointer Address Register

    uint8_t    m_Socket_tIntStatus [W5100_NUMBER_OF_SOCKET];
    uint32_t   m_TX_BaseAddress    [W5100_NUMBER_OF_SOCKET];
    uint16_t   m_TX_Size           [W5100_NUMBER_OF_SOCKET];
    uint16_t   m_TX_Mask           [W5100_NUMBER_OF_SOCKET];
    uint32_t   m_RX_BaseAddress    [W5100_NUMBER_OF_SOCKET];
    uint16_t   m_RX_Size           [W5100_NUMBER_OF_SOCKET];
    uint16_t   m_RX_Mask           [W5100_NUMBER_OF_SOCKET];
};

//-------------------------------------------------------------------------------------------------

#endif // (USE_ETH_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
