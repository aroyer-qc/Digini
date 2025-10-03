//-------------------------------------------------------------------------------------------------
//
//  File : automat_fct_def.h
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
// Define(s)
//-------------------------------------------------------------------------------------------------

#define FTC_AUTO_DEF(X_IO) \
/*         ENUM ID of the FCT,  FCT Ascii value,       Class object size,             */  \
    X_FCT( FCT_TIMER,           '@',                   uint8_t(sizeof(FCT_Timer))         \
    X_FCT( FCT_COMPARATOR,      'A',                   uint8_t(sizeof(FCT_Comparator),    \
    X_FCT( FCT_TIME_COUNTER,    'C',                   uint8_t(sizeof(FCT_TimeCounter))   \
    X_FCT( FCT_COUNT_DOWN,      'D',                   uint8_t(sizeof(FCT_CountDown))     \
    X_FCT( FCT_EVENT_COUNTER,   'E',                   uint8_t(sizeof(FCT_EventCounter))  \
    X_FCT( FCT_FLIP_FLOP,       'F',                   uint8_t(sizeof(FCT_FlipFlop))      \
    X_FCT( FCT_GATE,            'G',                   uint8_t(sizeof(FCT_Gate))          \
    X_FCT( FCT_INPUT,           'I',                   uint8_t(sizeof(FCT_Input))         \
    X_FCT( FCT_OUTPUT,          'O',                   uint8_t(sizeof(FCT_Output))        \
    X_FCT( FCT_PRIORITY,        'P',                   uint8_t(sizeof(FCT_Priority))      \
    X_FCT( FCT_SELECTOR,        'S',                   uint8_t(sizeof(FCT_Selector))      \
    X_FCT( FCT_TOGGLE,          'T',                   uint8_t(sizeof(FCT_Toggle))        \
    X_FCT( FCT_RANDOM_CYCLE,    'c',                   uint8_t(sizeof(FCT_RandomCycle))   \
    X_FCT( FCT_SCHEDULE_DAY,    'd',                   uint8_t(sizeof(FCT_ScheduleDay))   \
    X_FCT( FCT_PID              'p',                   uint8_t(sizeof(FCT_Pid))           \
    X_FCT( FCT_SUNRISE,         'r',                   uint8_t(sizeof(FCT_SunRise))       \
    X_FCT( FCT_SUNSET,          's',                   uint8_t(sizeof(FCT_SunSet))        \
    X_FCT( FCT_SCHEDULE_TIME,   't',                   uint8_t(sizeof(FCT_ScheduleTime))  \

//-------------------------------------------------------------------------------------------------

#if 0

A channel is a global ID for each input or output channel in the big scenario picture.
A channel is written by only one FCT_Block or formula.
A channel can read by multiples FCT_Blocks or formulas.
A channel is resident inside a module on a system.

Every Channel can have an override Channel associated to it to activate or not the output ( 0 = formula or block, 1 = default value)
if not CHANNEL_NO_OVERRIDE.


THIS NEED TBD: if to complicated or if global system program the broadcast of the value because it knows!!
If a channel is needed by a module, but the channel is not resident in the same module, the module must
send a request on the external bus so it can subscribe to that channel.

as long a Channel is unavailable, formula or FCT_Block using them must be invalidated and/or reverting to default value or last valid value.
this will be configuration


Each function must have this set of status bit 

Flag.IsInitialized
Flag.IsValid
Flag.Error, bit fields

Operator are directly processed by the controller ( <>=+-!^&*/|?: )

Formula has a configurable number of recursivity

Formula has this format:

1=(p(2,3,4,"configuration") * 1000),              
5=6<7,          if input 6 is lower than input 7 than result is 1 otherwise it is 0
5=(6<7)?6:7,    if input 6 is lower than input 7 than result is value in 6 otherwise it is value of 7
5=(6<7)?8:9,    if input 6 is lower than input 7 than result is value in 8 otherwise it is value of 9


Each function must have this set of variables
INPUT   channels        ( type of channel is according to range??  (maybe make this universal) )
OUTPUT  channels        ( type of channel is according to range??  (maybe make this universal) )

Each function might have multiple I/O channels according to the needs

Each function class must have a function:

    Run     manage the function according to state of the controller   exemple :  Controller.Run(CREATE);
                                                                                  Controller.Run(REFRESH);
                                                                                  Controller.Run(UPDATE);
                                                                                  Controller.Run(FINALIZE);
    
    Create      Allocated ressources. and init flags.
    Refresh     Run the function and update output.
    Update      Update the parameters of the function.
    finalize

Each function class are accessed via an interface.

External channel data might be coming from different source, Serial, ethernet, mqtt, upd, custom protocol, bacnet, WIFI,
sky is the limit. so using interface to access driver, automation core should not know the medium.

#endif

#if 0
Random timeslot module:     This module is use for example to simulate presence in a home controlling light.
    Must have a minimum / Maximum time range configuration.             On time.     60 Seconds to eights hours.
    Must have a minimum / Maximum time range before next activation.    Off time.
    Must have an enable time range (for day/night use)
    TBD Maybe have a Time overrange allowance.. ( ex. allow ON time to go up to x seconds(minute hour) passed permitted enabled time range.
#endif




#if 0

MQTT bus functionnality
CRC for ALL;
ReqID.. must be present on important message, if no answers in x time, request is resend up to x time.

Master module must create MQTT subscription for broadcast. every other module must subscribe

Request a channel   this will set a transmit the change on channel up to 1 hour.
REQ:    Request broadcast for a channel  REQSUB
ANS:    one module should respond OK if it own the channel
notes: no need for module ID

Refresh for channel
REQ:    StayAlive for a channel          ALIVE      (if module detect ALIVE for same channel it will reset its own timeout ... traffic control) 
ANS:    one module should respond OK if it own the channel and reset is timeout.
notes: no need for module ID

Presence for module
CAST:   module signify his presence  MODID = module ID, HI
// no answers

Channel value
CAST:   Module send the channel      CHVAL = value; 

#endif
