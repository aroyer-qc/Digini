//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_PHY_LAN8742A.h
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2023 Alain Royer.
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
#if (USE_ETH_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

// Basic Registers
#define REG_BCR                     0           // Basic Control Register
#define REG_BSR                     1           // Basic Status Register

// Extended Registers
#define REG_PHY_ID_1                2           // PHY Identifier 1
#define REG_PHY_ID_2                3           // PHY Identifier 2
#define REG_ANAR                    4           // Auto-Negotiation Advertisement
#define REG_ANLPAR                  5           // Auto-Negotiation Link Partner Ability
#define REG_ANER                    6           // Auto-Negotiation Expansion Register
#define REG_ANEG_NP_TX              7           // Auto-Negotiation Next Page TX
#define REG_ANEG_NP_RX              8           // Auto-Negotiation Next Page RX
#define REG_MMD_ACCESS_CTRL         13          // MMD Access Control
#define REG_MMD_ACCESS_AD           14          // MMD Access Address/Data

// Vendor-specific Registers
#define REG_MCSR                    17          // Mode Control/Status Register
#define REG_SPEC_MODE               18          // Special Modes Register
#define REG_TDR_PAT_DEL             24          // TDR Patterns/Delay Control Register
#define REG_TDR_CTRL_STAT           25          // TDR Control/Status Register
#define REG_SEC                     26          // System Error Counter Register
#define REG_SC_SI                   27          // Specifal Control/Status Indication
#define REG_CABLE_LEN               28          // Cable Length Register
#define REG_ISF                     29          // Interrupt Source Flag Register
#define REG_IM                      30          // Interrupt Mask Register
#define REG_PSCS                    31          // PHY Special Ctrl/Status Register

// Basic Control Register
#define BCR_RESET                   0x8000      // Software Reset
#define BCR_LOOPBACK                0x4000      // Loopback mode
#define BCR_SPEED_SEL               0x2000      // Speed Select (0 = 10Mb/s, 1 = 100Mb/s)
#define BCR_AUTO_NEG_EN             0x1000      // Auto Negotiation Enable
#define BCR_POWER_DOWN              0x0800      // Power Down (0 = ?? , 1 = power down mode)
#define BCR_ISOLATE                 0x0400      // Isolate Media interface
#define BCR_REST_ANEG               0x0200      // Restart Auto Negotiation
#define BCR_DUPLEX                  0x0100      // Duplex Mode (0 = half duplex, 1 = Full duplex)
#define BCR_COL_TEST                0x0080      // Collision Test Enable (0 = disable,

// Basic Status Register
#define BSR_100B_T4                 0x8000      // 100BASE-T4 Capable
#define BSR_100B_TX_FD              0x4000      // 100BASE-TX Full Duplex Capable
#define BSR_100B_TX_HD              0x2000      // 100BASE-TX Half Duplex Capable
#define BSR_10B_T_FD                0x1000      // 10BASE-T Full Duplex Capable
#define BSR_10B_T_HD                0x0800      // 10BASE-T Half Duplex Capable
#define BSR_100B_T2_FD              0x0400      // 1000BASE-T2 Full Duplex Capable
#define BSR_100B_T2_HD              0x0200      // 1000BASE-T2 Half Duplex Capable
#define BSR_EXTENDED_STAT           0x0100      // Extended Status in register 15
#define BSR_AUTO_NEGO_COMPLETE      0x0020      // Auto Negotiation Complete
#define BSR_REM_FAULT               0x0010      // Remote Fault
#define BSR_ANEG_ABIL               0x0008      // Auto Negotiation Ability
#define BSR_LINK_STAT               0x0004      // Link Status (1=link us up)
#define BSR_JABBER_DET              0x0002      // Jabber Detect
#define BSR_EXT_CAPAB               0x0001      // Extended Capabilities

// PHY Identifier Registers
#define PHY_ID_1                    0x0007      // LAN8742A Device IdentifierMSB
#define PHY_ID_2                    0xC130      // LAN8742A Device IdentifierLSB

// PHY Special Control/Status Register
#define PSCS_AUTODONE               0x1000      // Auto-negotiation is done
#define PSCS_DUPLEX                 0x0010      // Duplex Status (1=Full duplex)
#define PSCS_SPEED                  0x0008      // Speed10 Status (1=10MBit/s)

//#define GEN_PHY_CR1                 REG_PSCS
// PHY Control 1 Register Bitmasks
#define PHY_CR1_PAUSE_ENABLE        0x0200      // Enable Pause (Flow Control)
#define PHY_CR1_LINK_STATUS         0x0100      // Link Status
#define PHY_CR1_POLARITY_STATUS     0x0080      // Polarity Status
#define PHY_CR1_MDI_STATE           0x0020      // MDI/MDI-X State
#define PHY_CR1_ENERGY_DETECT       0x0010      // Energy Detect
#define PHY_CR1_PHY_ISOLATE         0x0008      // PHY Isolate
#define PHY_CR1_OPERATION_MODE      0x0007      // Operation Mode Indication
#define PHY_CR1_MODE_100_BASE       0x0008      // 100Base-TX bitmask
#define PHY_CR1_MODE_FULL_DUPLEX    0x0010      // Full-duplex bitmask

//#define GEN_PHY_CR2

#define PHY_TIMEOUT                 2

//-------------------------------------------------------------------------------------------------
// Class definition(s)
//-------------------------------------------------------------------------------------------------

class PHY_LAN8742A_Driver : public PHY_DriverInterface
{
    public:

        SystemState_e               Initialize                      (ETH_DriverInterface* pETH_Driver, uint32_t PHY_Address);
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

    private:

        // Ethernet PHY control structure
        ETH_DriverInterface*        m_pETH_Driver;                  // Pointer on the class ETH_Driver
        uint32_t                    m_PHY_Address;
        bool                        m_IsItInitialize;
        uint16_t                    m_BCR_Register;                 // BCR register value
        ETH_State_e                 m_Flags;                        // Control flags
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#ifdef LIB_PHY_8742A_GLOBAL

class        PHY_LAN8742A_Driver    myPHY_Driver;

#else
extern class PHY_LAN8742A_Driver    myPHY_Driver;
#endif

//-------------------------------------------------------------------------------------------------

#else // (USE_ETH_DRIVER == DEF_ENABLED)

#pragma message("DIGINI driver for ETHERNET must be enable and configure to use this device driver")

#endif // (USE_ETH_DRIVER == DEF_ENABLED)

#endif // (DIGINI_USE_ETHERNET == DEF_ENABLED)
//-------------------------------------------------------------------------------------------------



