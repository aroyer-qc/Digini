//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_STM32L4_usb.h
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

#if (USE_USB_DRIVER == DEF_ENABLED)

#include <stdint.h>

#ifdef USB
#undef USB
#endif

#define USB_CONTROL_PACKET_SIZE 64
#define USB_CDC_PACKET_SIZE 64
#define USB_CDC_BUFFER_SIZE 512

//
//  class definition(s)
//
//-------------------------------------------------------------------------------------------------

class USB_STM32L4_Port
{
  public:

    USB_STM32L4_Port(void);
    virtual ~USB_STM32L4_Port(void) = default;

    bool        Initialize(void);
    void        Process(void);
    void        IRQHandler(void);
    bool        IsConfigured(void) const;

  protected:

    virtual void ResetClassState(void) = 0;
    virtual void ConfigureClassEndpoints(void) = 0;
    virtual bool GetClassDescriptor(uint16_t Value, uint16_t Requested) = 0;
    virtual bool HandleClassSetup(uint8_t Type, uint8_t Request, uint16_t Value,
                    uint16_t Index, uint16_t Length) = 0;
    virtual void CancelClassControl(void) = 0;
    virtual void HandleClassControlOut(const uint8_t* pData, uint16_t Length) = 0;
    virtual void HandleClassEndpointRx(uint8_t Endpoint) = 0;
    virtual void HandleClassEndpointTx(uint8_t Endpoint) = 0;
    virtual void HandleEndpointHaltCleared(uint8_t Endpoint) = 0;
    virtual bool IsEndpointAvailable(uint8_t Endpoint, bool In) const = 0;
    virtual uint8_t GetInterfaceCount(void) const = 0;
    virtual void ProcessClass(void) = 0;

    void        SetEndpointStatus(uint8_t Endpoint, uint16_t Mask, uint16_t Status);
    void        ClearEndpointEvent(uint8_t Endpoint, uint16_t Event);
    void        ConfigureEndpoint(uint8_t Endpoint, uint16_t Type, uint16_t TxAddress, uint16_t RxAddress);
    bool        IsEndpointTxReady(uint8_t Endpoint) const;
    bool        IsEndpointTxStalled(uint8_t Endpoint) const;
    bool        IsEndpointRxBlocked(uint8_t Endpoint) const;
    void        EnableEndpointRx(uint8_t Endpoint, uint16_t Capacity);
    uint16_t    GetEndpointRxLength(uint8_t Endpoint) const;
    void        ReadEndpointRx(uint8_t Endpoint, uint8_t* pData, uint16_t Length);
    void        WritePMA(uint16_t Address, const uint8_t* pData, uint16_t Length);
    void        ReadPMA(uint16_t Address, uint8_t* pData, uint16_t Length);
    void        Transmit(uint8_t Endpoint, const uint8_t* pData, uint16_t Length);
    void        ControlSend(const uint8_t* pData, uint16_t Length, uint16_t Requested);
    void        StallControl(void);

    volatile bool       m_Configured;
    volatile bool       m_Suspended;
    uint8_t             m_ControlBuffer[128];
    uint16_t            m_ControlRemaining;
    bool                m_ControlZLP;

  private:

    void        Reset(void);
    void        ConfigureEndpoints(void);
    void        Setup(const uint8_t* pSetup);
    void        ControlIn(void);

    const uint8_t*      m_pControlData;
    bool                m_AddressPending;
    uint8_t             m_PendingAddress;
    uint8_t             m_Configuration;
    bool                m_Initialized;
};

class USB_CDC_ACM_Driver : public USB_STM32L4_Port
{
  public:

    USB_CDC_ACM_Driver(void);

    bool        IsDTR(void) const;
    bool        IsRTS(void) const;
    uint32_t    GetBaudRate(void) const;
    uint16_t    Available(void) const;
    uint16_t    Read(uint8_t* pData, uint16_t Length);
    uint16_t    Write(const uint8_t* pData, uint16_t Length);

  protected:

    void        ResetClassState(void) override;
    void        ConfigureClassEndpoints(void) override;
    bool        GetClassDescriptor(uint16_t Value, uint16_t Requested) override;
    bool        HandleClassSetup(uint8_t Type, uint8_t Request, uint16_t Value,
                  uint16_t Index, uint16_t Length) override;
    void        CancelClassControl(void) override;
    void        HandleClassControlOut(const uint8_t* pData, uint16_t Length) override;
    void        HandleClassEndpointRx(uint8_t Endpoint) override;
    void        HandleClassEndpointTx(uint8_t Endpoint) override;
    void        HandleEndpointHaltCleared(uint8_t Endpoint) override;
    bool        IsEndpointAvailable(uint8_t Endpoint, bool In) const override;
    uint8_t     GetInterfaceCount(void) const override;
    void        ProcessClass(void) override;

  private:

    void        StartTransmit(void);
    void        Receive(void);

    volatile bool       m_TxBusy;
    volatile uint16_t   m_RxHead;
    volatile uint16_t   m_RxTail;
    volatile uint16_t   m_TxHead;
    volatile uint16_t   m_TxTail;
    volatile uint16_t   m_ControlLines;
    uint8_t             m_RxBuffer[USB_CDC_BUFFER_SIZE];
    uint8_t             m_TxBuffer[USB_CDC_BUFFER_SIZE];
    uint8_t             m_LineCoding[7];
    bool                m_TxZLP;
    uint16_t            m_TxPacketLength;
    bool                m_ExpectLineCoding;
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#ifdef USB_DRIVER_GLOBAL
USB_CDC_ACM_Driver USB;
#else // USB_DRIVER_GLOBAL
extern USB_CDC_ACM_Driver USB;
#endif // USB_DRIVER_GLOBAL

//-------------------------------------------------------------------------------------------------

#endif // (USE_USB_DRIVER == DEF_ENABLED)


