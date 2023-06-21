//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_phy_LAN8742A.cpp
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

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "lib_digini.h"

//-------------------------------------------------------------------------------------------------

#if (USE_ETH_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
//  Function:       Constructor
//
//  Parameter(s):   PHY_Address
//
//  Description:    Set PHY Adress for this driver.
//
//-------------------------------------------------------------------------------------------------
PHY_LAN8742A_Driver::PHY_LAN8742A_Driver(uint32_t PHY_Address)
{
    m_PHY_Address = PHY_Address;
    Uninitialize();
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       Uninitialize
//
//  Parameter(s):   Ethernet driver class pointer
//  Return:         SystemState_e   State of function.
//
//  Description:    Initializee PHY Device.
//
//-------------------------------------------------------------------------------------------------
SystemState_e PHY_LAN8742A_Driver::Initialize(ETH_Driver* pDriver)
{

    if(pDriver == nullptr)
    {
        return SYS_INVALID_PARAMETER;
    }

    if((m_Flags & ETH_INITIALIZED) == 0)
    {
        m_pETH_Driver  = pDriver;               // Register ETH_Driver
        m_BCR_Register = 0;
        m_Flags        = ETH_INITIALIZED;
    }

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       Uninitialize
//
//  Parameter(s):   None
//  Return:         SystemState_e   State of function.
//
//  Description:    De-initialize PHY Device.
//
//-------------------------------------------------------------------------------------------------
SystemState_e PHY_LAN8742A_Driver::Uninitialize(void)
{
    m_pETH_Driver  = nullptr;
    m_BCR_Register = 0;
    m_Flags        = ETH_STATE_UNKNOWN;

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       PowerControl
//
//  Parameter(s):   State
//  Return:         SystemState_e   State of function.
//
//  Description:    Control Ethernet PHY Device Power.
//
//-------------------------------------------------------------------------------------------------
SystemState_e PHY_LAN8742A_Driver::PowerControl(ETH_PowerState_e PowerState)
{
    SystemState_e State;
    uint16_t      Value;

    switch(uint32_t(PowerState))
    {
        case uint32_t(ETH_POWER_OFF):
        {
            if((m_Flags & ETH_INITIALIZED) != 0)
            {
                m_Flags        = ETH_INITIALIZED;
                m_BCR_Register = BCR_POWER_DOWN;
                State = m_pETH_Driver->PHY_Write(m_PHY_Address, REG_BCR, m_BCR_Register);
            }
            else
            {
                // Initialize must provide register access function pointers
                State = SYS_ERROR;
            }
        }
        break;

        case uint32_t(ETH_POWER_FULL):
        {
            if((m_Flags & ETH_INITIALIZED) == 0)
            {
                State = SYS_ERROR;
            }
            else if((m_Flags & ETH_POWERED_ON) != 0)
            {
                State= SYS_READY;
            }
            else
            {
                // Check Device Identification.
                m_pETH_Driver->PHY_Read(m_PHY_Address, REG_PHY_ID_1, &Value);

                if(Value != PHY_ID_1)
                {
                    // Invalid PHY ID
                    State = SYS_UNSUPPORTED_FEATURE;
                }
                else
                {
                    m_pETH_Driver->PHY_Read(m_PHY_Address, REG_PHY_ID_2, &Value);

                    if((Value & 0xFFF0) != PHY_ID_2)
                    {
                        State = SYS_UNSUPPORTED_FEATURE;                         // Invalid PHY ID
                    }
                    else
                    {
                        m_BCR_Register = 0;

                        if((State = m_pETH_Driver->PHY_Write(m_PHY_Address, REG_BCR, m_BCR_Register)) == SYS_READY)
                        {
                            m_Flags = ETH_INITIALIZED_AND_POWERED_ON;
                            State   = SYS_READY;
                        }
                    }
                }
            }
        }
        break;

      //case uint32_t(ETH_POWER_LOW):
        default:
        {
            State = SYS_UNSUPPORTED_FEATURE;
        }
        break;
    }

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       SetInterface
//
//  Parameter(s):   Interface       ETH_INRTERFACE_RMII or ETH_INTERFACE_MII or ETH_INTERFACE_SII
//  Return:         SystemState_e   State of function.
//
//  Description:    Set Ethernet PHY Device Operation Mode.
//
//  Note(s):        LAN8742A has no interface to set. Compatibility to generic driver.
//
//-------------------------------------------------------------------------------------------------
SystemState_e PHY_LAN8742A_Driver::SetInterface(ETH_MediaInterface_e Interface)
{

    if((m_Flags & ETH_POWERED_ON) == 0)
    {
        return SYS_ERROR;
    }

    switch(uint32_t(Interface))
    {
        case uint32_t(ETH_INTERFACE_RMII):  break;          // Nothing to do on the driver
     // case uint32_t(ETH_INTERFACE_MII):
     // case uint32_t(ETH_INTERFACE_SII):
        default:                            return SYS_UNSUPPORTED_FEATURE;
    }

    return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       SetMode
//
//  Parameter(s):   Mode
//  Return:         SystemState_e   State of function.
//
//  Description:    Set Ethernet PHY Device Operation Mode.
//
//-------------------------------------------------------------------------------------------------
SystemState_e PHY_LAN8742A_Driver::SetMode(ETH_PHY_Mode_e Mode)
{
    uint16_t Value;

    if((m_Flags & ETH_POWERED_ON) == 0)
    {
        return SYS_ERROR;
    }

    Value = m_BCR_Register & BCR_POWER_DOWN;

    switch(uint32_t(Mode) & ETH_PHY_MODE_SPEED_MASK)
    {
        case uint32_t(ETH_PHY_MODE_SPEED_10M):                                      break;
        case uint32_t(ETH_PHY_MODE_SPEED_100M):    Value |= BCR_SPEED_SEL;          break;
        //default:                                   return SYS_UNSUPPORTED_FEATURE;
  }

    switch(Mode & ETH_PHY_MODE_DUPLEX_MASK)
    {
        case uint32_t(ETH_PHY_MODE_DUPLEX_HALF):                                    break;
        case uint32_t(ETH_PHY_MODE_DUPLEX_FULL):   Value |=  BCR_DUPLEX;            break;
    }

    if(Mode & ETH_PHY_MODE_AUTO_NEGOTIATE)       { Value |= BCR_AUTO_NEG_EN;  }
    if(Mode & ETH_PHY_MODE_LOOPBACK)             { Value |= BCR_LOOPBACK;     }
    if(Mode & ETH_PHY_MODE_ISOLATE)              { Value |= BCR_ISOLATE;      }

    m_BCR_Register = Value;

    return m_pETH_Driver->PHY_Write(m_PHY_Address, REG_BCR, m_BCR_Register);
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       GetLinkState
//
//  Parameter(s):   None
//  Return:         ETH_LinkState_e
//
//  Description:    Get Ethernet Link state.
//
//-------------------------------------------------------------------------------------------------
ETH_LinkState_e PHY_LAN8742A_Driver::GetLinkState(void)
{
    ETH_LinkState_e State;
    uint16_t        Value = 0;

    if(m_Flags & ETH_POWERED_ON)
    {
        m_pETH_Driver->PHY_Read(m_PHY_Address, REG_BSR, &Value);
    }

    State = (Value & BSR_LINK_STAT) ? ETH_LINK_UP : ETH_LINK_DOWN;

    return State;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       GetLinkInfo
//
//  Parameter(s):   None
//  Return:         ETH_LinkInfo_t
//
//  Description:    Get Ethernet Link Info.
//
//-------------------------------------------------------------------------------------------------
ETH_LinkInfo_t PHY_LAN8742A_Driver::GetLinkInfo(void)
{
    ETH_LinkInfo_t Info;
    uint16_t       Value = 0;

    if(m_Flags & ETH_POWERED_ON)
    {
        m_pETH_Driver->PHY_Read(m_PHY_Address, REG_PSCS, &Value);
    }

    Info.Speed  = (Value & PSCS_SPEED)  ? ETH_PHY_SPEED_10M   : ETH_PHY_SPEED_100M;
    Info.Duplex = (Value & PSCS_DUPLEX) ? ETH_PHY_FULL_DUPLEX : ETH_PHY_HALF_DUPLEX;

    return Info;
}

//-------------------------------------------------------------------------------------------------

#endif // (USE_ETH_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
