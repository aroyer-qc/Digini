//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_keyswitch.h
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2022 Alain Royer.
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

// Note(s)  When configuring a clock/data quadrature encoder only choose falling or rising for ISR.
//          If encoder is not incrementing in the right direction, then change only clock polarity.
//
//-------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "./lib_digini.h"
#include "device_cfg.h"

//-------------------------------------------------------------------------------------------------

#ifdef KEYSWICH_GLOBAL
    #define KEYSWICH_EXTERN
#else
    #define KEYSWICH_EXTERN extern
#endif

//-------------------------------------------------------------------------------------------------
// Expand macro(s)
//-------------------------------------------------------------------------------------------------

#define EXPAND_X_KEYSWITCH_AS_ENUM(ENUM_ID, IO_KEYSWITCH) ENUM_ID,

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

#ifdef QUAD_ENCODER_DEF
enum KEYSwitchID_e
{
    KEYSWITCH_NONE = -1,
    KEYSWITCH_DEF(EXPAND_X_KEYSWITCH_AS_ENUM)
    NB_OF_KEYSWITCH,
};

enum KEYSWITCH_Change_e
{
    KEY_PRESSED,
    KEY_RELEASED,
    KEY_SUPERKEY,
};

struct KEYSWITCH_Info_t
{
    IO_IrqID_e IO_KeySwitch;
};

typedef void (* KEYSWITCH_CallBack_t) (KEYSWITCH_Change_e Change);

//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class KEY_Switch
{
    public:

                                KEY_Switch              (KeySwitchID_e KeyID);

        void                    RegisterChangeCallback  (void* pCallback);
        void                    Enable                  (void);
        void                    Disable                 (void);

        void                    KeySwitchISR            (void);

    private:

                KEYSWITCH_Info_t*     m_pKeySwitchInfo;
                KEYSWITCH_CallBack_t  m_pCallback;
        static  KEYSWITCH_Info_t      m_KeySwitchInfo[NB_OF_KEYSWITCH];
};

//-------------------------------------------------------------------------------------------------

#endif



