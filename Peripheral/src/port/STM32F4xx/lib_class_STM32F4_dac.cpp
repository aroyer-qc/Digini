//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F4_dac.cpp
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2020 Alain Royer.
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

#define DAC_DRIVER_GLOBAL
#include <lib_digini.h>
#undef  DAC_DRIVER_GLOBAL

//-------------------------------------------------------------------------------------------------

#if (USE_DAC_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

//  CR register Mask
#define CR_CLEAR_MASK                   0x00000FFE

// DHR registers offsets
#define DHR12R1_OFFSET                  0x00000008
#define DHR12R2_OFFSET                  0x00000014
#define DHR12RD_OFFSET                  0x00000020

// DOR register offset
#define DOR_OFFSET                      0x0000002C

#define DAC_STANDARD_CONFIGURATION      ( DMA_CHANNEL_7               | \
                                          DMA_PERIPHERAL_TO_MEMORY    | \
                                          DMA_PERIPHERAL_NO_INCREMENT | \
                                          DMA_MEMORY_INCREMENT        | \
                                          DMA_PERIPHERAL_BURST_SINGLE | \
                                          DMA_MEMORY_BURST_SINGLE     | \
                                          DMA_PRIORITY_LEVEL_MEDIUM)


//-------------------------------------------------------------------------------------------------
// Const(s)
//-------------------------------------------------------------------------------------------------

#if (CFG_DAC_DRIVER_SUPPORT_DMA_CHANNEL_1 ==  DEF_ENABLED)
// DMA Channel 1
const DMA_Info_t DAC_Driver::m_DMA_InfoChannel_1 =
{
    DAC_STANDARD_CONFIGURATION | CFG_DAC_CH1_DMA_MODE | CFG_DAC_CH1_DMA_PERIPHERAL_SIZE | CFG_DAC_CH1_DMA_MEMORY_SIZE,
    DMA_HIFCR_CTCIF5,
    DMA1_Stream5,
    DMA1_Stream5_IRQn,
};
#endif

#if (CFG_DAC_DRIVER_SUPPORT_DMA_CHANNEL_2 ==  DEF_ENABLED)
// DMA Channel 2
const DMA_Info_t DAC_Driver::m_DMA_InfoChannel_2 =
{
    DAC_STANDARD_CONFIGURATION | CFG_DAC_CH2_DMA_MODE | CFG_DAC_CH2_DMA_PERIPHERAL_SIZE | CFG_DAC_CH2_DMA_MEMORY_SIZE,
    DMA_HIFCR_CTCIF6,
    DMA1_Stream6,
    DMA1_Stream6_IRQn,
};
#endif

//-------------------------------------------------------------------------------------------------
//
//  Function:       Initialize
//
//  Parameter(s):   None
//  Return:         SystemState_e
//
//  Description:    Initialize the DAC peripheral.
//
//  Note(s):        Clock configuration is done in lib_STM32F4_system_clock.c via clock_cfg.h
//
//-------------------------------------------------------------------------------------------------
SystemState_e DAC_Driver::Initialize(void)
{
    uint32_t Mask = CR_CLEAR_MASK;
    uint32_t Config;

	SET_BIT(RCC->APB1ENR, RCC_APB1ENR_DACEN);

  #if (CFG_DAC_DRIVER_CHANNEL_1 == DEF_ENABLED)
    Config = CFG_DAC_CH1_TRIGGER                        |
             CFG_DAC_CH1_WAVE_GENERATION                |
             CFG_DAC_CH1_LFSR_UNMASK_TRIANGLE_AMPLITUDE |
             CFG_DAC_CH1_OUTPUT_BUFFER;
    MODIFY_REG(DAC->CR, Mask << DAC_CHANNEL_1, Config);

   #if (CFG_DAC_DRIVER_SUPPORT_DMA_CHANNEL_1 ==  DEF_ENABLED)
        m_DMA_Channel_1.Initialize(&m_DMA_InfoChannel_1);
        m_DMA_Channel_1.SetDestination((void*)CFG_DAC_CH1_DATA_HOLDING_REGISTER);
   #endif
  #endif

  #if (CFG_DAC_DRIVER_CHANNEL_2 == DEF_ENABLED)
    MODIFY_REG(DAC->CR, Mask << DAC_CHANNEL_2, CFG_DAC_CH2_TRIGGER                        |
                                               CFG_DAC_CH2_WAVE_GENERATION                |
                                               CFG_DAC_CH2_LFSR_UNMASK_TRIANGLE_AMPLITUDE |
                                               CFG_DAC_CH2_OUTPUT_BUFFER);

   #if (CFG_DAC_DRIVER_SUPPORT_DMA_CHANNEL_2 ==  DEF_ENABLED)
        m_DMA_Channel_2.Initialize(&m_DMA_InfoChannel_2);
        m_DMA_Channel_2.SetDestination((void*)CFG_DAC_CH2_DATA_HOLDING_REGISTER);
   #endif
  #endif

	return SYS_READY;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       SetChannel_1
//
//  Parameter(s):   DAC_Align   - Alignment selector:       DAC_12_BITS_LEFT
//                                                          DAC_12_BITS_RIGHT
//                                                          DAC_8_BITS_RIGHT
//                  Data        - Data value to write into DAC Channel 1
//
//  Return:         None
//
//  Description:    Writes the specified value into the DAC Channel 1 data holding register.
//                  The target register is computed from the DAC base address, the
//                  channel-specific offset, and the selected alignment mode.
//
//-------------------------------------------------------------------------------------------------
void DAC_Driver::SetChannel_1(DAC_Alignment_e DAC_Align, uint16_t Data)
{
    volatile uint32_t Register;

    Register = DAC_BASE + DHR12R1_OFFSET + uint32_t(DAC_Align);
    *(volatile uint32_t *)Register = Data;
}

//-------------------------------------------------------------------------------------------------
//
//  Function:       SetChannel_2
//
//  Parameter(s):   DAC_Align   - Alignment selector:       DAC_12_BITS_LEFT
//                                                          DAC_12_BITS_RIGHT
//                                                          DAC_8_BITS_RIGHT
//                  Data        - Data value to write into DAC Channel 2
//
//  Return:         None
//
//  Description:    Writes the specified value into the DAC Channel 1 data holding register.
//                  The target register is computed from the DAC base address, the
//                  channel-specific offset, and the selected alignment mode.
//
//-------------------------------------------------------------------------------------------------
void DAC_Driver::SetChannel_2(DAC_Alignment_e DAC_Align, uint16_t Data)
{
    volatile uint32_t Register;

    Register = DAC_BASE + DHR12R2_OFFSET + DAC_Align;
    *(volatile uint32_t *)Register = Data;
}

//-------------------------------------------------------------------------------------------------
//
//     Function:        SetDualChannelData
//
//     Parameter(s):    DAC_Align   - Alignment selector:   DAC_12_BITS_LEFT
//                                                          DAC_12_BITS_RIGHT
//                                                          DAC_8_BITS_RIGHT
//                      Data2       - Data for DAC Channel 2
//                      Data1       - Data for DAC Channel 1
//
//     Return:          None
//
//     Description:     Writes dual-channel data into the DAC data holding register.
//                      The function packs both channel values according to the
//                      selected alignment and updates the appropriate DHRxRD register.
//
//-------------------------------------------------------------------------------------------------
void DAC_Driver::SetDualChannelData(DAC_Alignment_e DAC_Align, uint16_t Data2, uint16_t Data1)
{
    volatile uint32_t   Register;
    uint32_t            Data = 0;

      // Calculate and set dual DAC data holding register value
      if(DAC_Align == DAC_8_BITS_RIGHT)
      {
            Data = uint32_t(Data2 << 8) | uint32_t(Data1);
      }
      else
      {
            Data = uint32_t(Data2 << 16) | uint32_t(Data1);
      }

      Register = DAC_BASE + DHR12RD_OFFSET + DAC_Align;
      *(volatile uint32_t *)Register = Data;
}

//-------------------------------------------------------------------------------------------------
//
//     Function:        GetDataOutputValue
//
//     Parameter(s):    DAC_Channel_e   Channel     The selected DAC channel.
//     Return:          None
//
//     Description:     Returns the last data output value of the selected DAC channel.
//
//-------------------------------------------------------------------------------------------------
uint16_t DAC_Driver::GetDataOutputValue(DAC_Channel_e Channel)
{
    volatile uint32_t Register;

    Register = DAC_BASE + DOR_OFFSET + (uint32_t(Channel) >> 2);
    return (uint16_t) (*(volatile uint32_t*) Register);
}

//-------------------------------------------------------------------------------------------------
//
//     Function:        GetFlagStatus
//
//     Parameter(s):    Channel     Selected DAC channel (DAC_Channel_e)
//                      Flag        DAC status flag to evaluate
//
//     Return:          bool        true if the specified flag is set, otherwise false
//
//     Description:     Checks whether the specified DAC status flag is active for the
//                      selected channel. The flag value is shifted according to the
//                      channel index and evaluated against the DAC status register.
//
//-------------------------------------------------------------------------------------------------
bool DAC_Driver::GetFlagStatus(DAC_Channel_e Channel, uint32_t Flag)
{
    if((DAC->SR & (Flag << uint32_t(Channel))) != 0)
    {
        return true;
    }

    return false;
}

//-------------------------------------------------------------------------------------------------
//
//     Function:        GetIT_Status
//
//     Parameter(s):    Channel     - Selected DAC channel (DAC_Channel_e)
//                      IT_Source   - DAC interrupt source to evaluate
//
//     Return:          bool        - true if the interrupt is enabled and active, otherwise false
//
//     Description:     Checks whether the specified DAC interrupt is both enabled and pending.
//                      The interrupt source is shifted according to the selected channel and
//                      evaluated against the DAC control and status registers.
//
//-------------------------------------------------------------------------------------------------
bool DAC_Driver::GetIT_Status(DAC_Channel_e Channel, uint32_t IT_Source)
{
    uint32_t EnableStatus;


    //Get the DAC_IT enable bit status
    EnableStatus = (DAC->CR & (IT_Source << uint32_t(Channel)));

    // Check the status of the specified DAC interrupt
    if(((DAC->SR & (IT_Source << uint32_t(Channel))) != 0) && (EnableStatus != 0))
    {
        return true;
    }

    return  false;
}

//-------------------------------------------------------------------------------------------------
//
//     Function:        DMA_Config
//
//     Parameter(s):    DAC_Channel_e Channel       Selected DAC channel
//                      void*         pBuffer       Buffer on the data to send to DAC
//                      size_t        Length        Size of the data to transfer
//
//     Return:          None
//
//     Description:     Set Source and Length for the DMA transfer.
//
//-------------------------------------------------------------------------------------------------
#if (CFG_DAC_DRIVER_SUPPORT_DMA_CHANNEL_1 ==  DEF_ENABLED) || (CFG_DAC_DRIVER_SUPPORT_DMA_CHANNEL_2 ==  DEF_ENABLED)
void DAC_Driver::DMA_Config(DAC_Channel_e Channel, void* pBuffer, size_t Length)
{
  #if (CFG_DAC_DRIVER_SUPPORT_DMA_CHANNEL_1 ==  DEF_ENABLED)
    if(Channel == DAC_CHANNEL_1)
    {
        m_DMA_Channel_1.SetSource(pBuffer);
        m_DMA_Channel_1.SetLength(Length);
    }
  #endif

  #if (CFG_DAC_DRIVER_SUPPORT_DMA_CHANNEL_2 ==  DEF_ENABLED)
    if(Channel == DAC_CHANNEL_2)
    {
        m_DMA_Channel_2.SetSource(pBuffer);
        m_DMA_Channel_2.SetLength(Length);
    }
  #endif
}
#endif

//-------------------------------------------------------------------------------------------------

#endif // (USE_DAC_DRIVER == DEF_ENABLED)

