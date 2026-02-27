//-------------------------------------------------------------------------------------------------
//
//  File : TCP_interface.h
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
// class definition(s)
//-------------------------------------------------------------------------------------------------

class TCP_Socket
{
    public:

        virtual                 ~TCP_Socket     (){}
    
        virtual size_t          Send            (const uint8_t* pBuffer, size_t Length)         = 0;
        virtual size_t          Receive         (uint8_t* pBuffer, size_t MaxLength)            = 0;
        virtual void            Close           (void)                                          = 0;
        virtual bool            IsConnected     (void) const                                    = 0;
        virtual TCP_State_e     GetState        (void) const                                    = 0;
};  
    
class TCP_Manager   
{   
    public: 
    
        virtual                 ~TCP_Manager    (){}
      #if (IP_USE_TCP_CLIENT == DEF_ENABLED)    
        virtual TCP_Socket*     Connect         (const IP_Address_t& ServerIP, uint16_t Port)   = 0;
      #endif    
    
      #if (IP_USE_TCP_SERVER == DEF_ENABLED)    
        //SystemState_e   EnterListen             (Socket* pSocket, uint16_t Backlog)           = 0;
        //void            Close                   (Socket* pSocket)                             = 0;
      #endif    
    
        virtual void            Process         (void)                                          = 0;
        virtual void            ProcessSegment  (IP_PacketMsg_t* pPacket)                       = 0;
        virtual bool            SendSegment     (TCP_Socket* pSocket, const uint8_t* pPayload,
                                                 size_t Length, uint8_t Flags,
                                                 bool Retransmit = false)                       = 0;
};

//-------------------------------------------------------------------------------------------------

