//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_i2c_STMPE811.h
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

#pragma once

//-------------------------------------------------------------------------------------------------

#if (USE_I2C_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

// ---- Global Interrupts definitions ----
#define IRQ_FLAG_GPIO               0x80
#define IRQ_FLAG_ADC                0x40
#define IRQ_FLAG_TEMP               0x20
#define IRQ_FLAG_FE                 0x10
#define IRQ_FLAG_FF                 0x08
#define IRQ_FLAG_FOV                0x04
#define IRQ_FLAG_FTH                0x02
#define IRQ_FLAG_TOUCH              0x01
#define IRQ_FLAG_ALL                0xFF


//-------------------------------------------------------------------------------------------------
// typedef struct(s)
//-------------------------------------------------------------------------------------------------

/* NOT USED???
struct TOUCH_State_t
{
  uint16_t  X;
  uint16_t  Y;
  uint16_t  Z;
  uint8_t   TouchDetected;
}
*/

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

// TODO: Mutex is not yet integrated ( do i need to? )

class TouchDriver
{
    public:

                            TouchDriver             (I2C_Driver* pI2C);  // use ID
                            ~TouchDriver            ();

        bool                Initialize              ();
        void                ProcessTouch            ();
        void                FifoReset               ();
        void                SetTouchCorrection      (LinearEquation_t X_Correction, LinearEquation_t Y_Correction);
        void                EnableTouchCorrection   (bool Enable);
        void                TS_GetXY                (uint16_t *X, uint16_t *Y);
        uint16_t            GetY                    ();
        void                SetIRQ_Enable           (bool State);
        bool                GetIRQ_Status           (uint8_t IRQ_Flag);
        void                ClearIRQ_Status         (uint8_t IRQ_Flag);
        bool                IsOperational           ();

    private:

        I2C_Driver*         m_pI2C;
        I2C_DeviceInfo_t*   m_pDevice;
        uint16_t            m_IntX;
        uint16_t            m_IntY;
        uint16_t            m_X;
        uint16_t            m_Y;
        LinearEquation_t    m_X_Correction;
        LinearEquation_t    m_Y_Correction;
        bool                m_IsCalibrated;
        bool                m_IsCorrectionEnable;
};

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

#define __CLASS_STMP811__
#include "device_var.h"
#undef  __CLASS_STMP811__

//-------------------------------------------------------------------------------------------------

#else // (USE_I2C_DRIVER == DEF_ENABLED)

#pragma message("DIGINI driver for I2C must be enable and configure to use this device driver")

#endif // (USE_I2C_DRIVER == DEF_ENABLED)

