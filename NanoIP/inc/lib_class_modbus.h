//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_modbus.h
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

//------ Note(s) ----------------------------------------------------------------------------------
//
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "./lib_digini.h"

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_ETHERNET == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
//
//  Name:           Initialize
//
//  Parameter(s):   NetworkContext&  Context        Reference on the context
//
//  Return:         None
//
//  Description:
//
//-------------------------------------------------------------------------------------------------
void MODBUS_Manager::Initialize(NetworkContext* pContext)
{
    m_pContext = pContext;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Process
//
//  Parameter(s):   
//  Return:         None
//
//  Description:    
//
//-------------------------------------------------------------------------------------------------
void MODBUS_Manager::Process(IP_PacketMsg_t* pMsg)
{
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Send
//
//  Parameter(s):   
//                  
//
//  Return:         
//                  
//                  
//  Description:
//
//-------------------------------------------------------------------------------------------------

class ModbusTcpTransport : public IModbusTransport
{
public:
    ModbusTcpTransport(nanoIP_socket* s)
        : sock(s)
    {}

    int send(const uint8_t* data, uint16_t len) override;
    int recv(uint8_t* buffer, uint16_t maxLen, uint32_t timeoutMs) override;

private:
    nanoIP_socket* sock;
};



//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_ETHERNET == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
