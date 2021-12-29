//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32F7_spi.h
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
//
//  Notes: Callback for TickHook will be reinitialize when LockToDevice() or UnlockFromDevice()
//         is called
//
//-------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------

#if (USE_SPI_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//  typedef(s)
//-------------------------------------------------------------------------------------------------

enum SPI_ID_e
{
    #if (SPI_DRIVER_SUPPORT_SPI1 == DEF_ENABLED)
		DRIVER_SPI1_ID,
	#endif

    #if (SPI_DRIVER_SUPPORT_SPI2 == DEF_ENABLED)
		DRIVER_SPI2_ID,
	#endif

    #if (SPI_DRIVER_SUPPORT_SPI3 == DEF_ENABLED)
		DRIVER_SPI3_ID,
	#endif

    #if (SPI_DRIVER_SUPPORT_SPI4 == DEF_ENABLED)
		DRIVER_SPI4_ID,
	#endif

    #if (SPI_DRIVER_SUPPORT_SPI5 == DEF_ENABLED)
		DRIVER_SPI5_ID,
	#endif

    #if (SPI_DRIVER_SUPPORT_SPI6 == DEF_ENABLED)
		DRIVER_SPI6_ID,
	#endif

    NB_OF_SPI_DRIVER,
};

//-------------------------------------------------------------------------------------------------
// typedef struct(s) and enum(s)
//-------------------------------------------------------------------------------------------------

struct SPI_PortInfo_t
{
    SPI_ID_e            SPI_ID;
    SPI_TypeDef*        pSPIx;
    IO_ID_e             PinCLK;
    IO_ID_e             PinMOSI;
    IO_ID_e             PinMISO;
    IO_ID_e             PinNSS;
    uint32_t            Clock;
    uint16_t            portConfig;
    uint8_t             PreempPrio;
    IRQn_Type           IRQn_Channel;
    uint16_t            IRQ_Source;
    void                (*CallBackISR)();

  #if (SPI_DRIVER_SUPPORT_DMA == DEF_ENABLED)
    DMA_TypeDef*        pDMA;
    uint32_t            DMA_ChannelRX;
    uint32_t            DMA_Flag_TC_RX;
    DMA_Stream_TypeDef* pDMA_RX;

    uint32_t            DMA_ChannelTX;
    uint32_t            DMA_Flag_TC_TX;
    DMA_Stream_TypeDef* pDMA_TX;
  #endif // (SPI_DRIVER_SUPPORT_DMA == DEF_ENABLED)
};

struct SPI_DeviceInfo_t
{
    uint32_t            SlowSpeed;
    uint32_t            FastSpeed;
    uint8_t             TimeOut;
    uint32_t            ChipSelectClock;
    GPIO_TypeDef*       pChipSelect;
    uint16_t            ChipSelectPin;
};

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class SPI_Driver
{
    public:
                        SPI_Driver              (SPI_ID_e SPI_ID);

        void            Initialize              (void);
        SystemState_e   GetStatus               (void);
        SystemState_e   LockToDevice            (SPI_DeviceInfo_t* pDevice);                                             // Set SPI to this device and lock
        SystemState_e   UnlockFromDevice        (SPI_DeviceInfo_t* pDevice);                                             // Unlock SPI from device

        // Read function (overloaded)
        SystemState_e   Read                    (uint8_t* pBuffer, size_t Size);
        SystemState_e   Read                    (uint8_t*  Data);
        SystemState_e   Read                    (uint16_t* Data);
        SystemState_e   Read                    (uint32_t* Data);
        SystemState_e   Read                    (uint8_t* pBuffer, size_t Size, SPI_DeviceInfo_t* pDevice);
        SystemState_e   Read                    (uint8_t*  pData, SPI_DeviceInfo_t* pDevice);
        SystemState_e   Read                    (uint16_t* pData, SPI_DeviceInfo_t* pDevice);
        SystemState_e   Read                    (uint32_t* pData, SPI_DeviceInfo_t* pDevice);

        // Write function (overloaded)
        SystemState_e   Write                   (const uint8_t* pBuffer, size_t Size);
        SystemState_e   Write                   (uint8_t  Data);
        SystemState_e   Write                   (uint16_t Data);
        SystemState_e   Write                   (uint32_t Data);
        SystemState_e   Write                   (const uint8_t* pBuffer, size_t Size, SPI_DeviceInfo_t* pDevice);
        SystemState_e   Write                   (uint8_t  Data, SPI_DeviceInfo_t* pDevice);
        SystemState_e   Write                   (uint16_t Data, SPI_DeviceInfo_t* pDevice);
        SystemState_e   Write                   (uint32_t Data, SPI_DeviceInfo_t* pDevice);

        void            IRQHandler              (void);
        SystemState_e   Request                 (AccessRequest_e Request, uint8_t *pBuffer, size_t Size);
        void            Config                  (uint32_t Speed);
        //void            TickHook                (void);
        // void            RegisterDriverTick      (void (*CallBackTick)());
        SystemState_e   WaitReady               (void);

    private:

        uint16_t        GetPrescalerFromSpeed   (uint32_t speed);
        void            Lock                    (void);
        void            Unlock                  (void);

        SPI_Info_t*                             m_pInfo;
        SPI_DeviceInfo_t*                       m_pDevice;
        nOS_Mutex                               m_Mutex;
        AccessRequest_e                         m_Request;
        uint32_t                                m_SlowSpeed;
        uint32_t                                m_FastSpeed;
        volatile size_t                         m_Size;
        volatile SystemState_e		            m_State;
        volatile uint8_t                        m_Timeout;
        void                                    (*m_pCallBackTick)();

};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// constant data
//-------------------------------------------------------------------------------------------------

#include "spi_var.h"

//-------------------------------------------------------------------------------------------------

#endif // (USE_SPI_DRIVER == DEF_ENABLED)
