//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_http.cpp
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
//  Description:
//
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "./lib_digini.h"

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_ETHERNET == DEF_ENABLED) && (IP_USE_HTTP == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//  Name:           TaskMQTT_ClientWrapper
//
//  Parameter(s):   void* pvParameters
//  Return:         void
//
//  Description:    main() for the MQTT_Client
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
//extern "C" void TaskMQTT_ClientWrapper(void* pvParameters)
//{
//    (static_cast<MQTT_Client*>(pvParameters))->Run();
//}

//-------------------------------------------------------------------------------------------------
//  Start listening
//-------------------------------------------------------------------------------------------------
bool HTTP_Server::Begin(uint16_t Port, HTTP_Handler* pHandler)
{
    if(pHandler == nullptr)
    {
        return false;
    }

    m_pHandler = pHandler;

    // Create listening socket
    m_pListenSocket = m_TCP.EnterListen(nullptr, Port);

    if(m_pListenSocket == nullptr)
    {
        return false;
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//  Stop server
//-------------------------------------------------------------------------------------------------
void HTTP_Server::Stop(void)
{
    if(m_pListenSocket != nullptr)
    {
        m_TCP.Close(m_pListenSocket);
        m_pListenSocket = nullptr;
    }

    if(m_pClientSocket != nullptr)
    {
        m_pClientSocket->Close();
        m_pClientSocket = nullptr;
    }
}

//-------------------------------------------------------------------------------------------------
//  Main loop
//-------------------------------------------------------------------------------------------------
void HTTP_Server::Process(void)
{
    AcceptIfNeeded();

    if(m_pClientSocket != nullptr)
    {
        HandleRX(m_pClientSocket);
    }
}

//-------------------------------------------------------------------------------------------------
//  Accept a client if none is connected
//-------------------------------------------------------------------------------------------------
void HTTP_Server::AcceptIfNeeded(void)
{
    if(m_pClientSocket != nullptr)
    {
        return;
    }

    // TCP_ManagerSystem will assign a socket when a SYN arrives
    // We simply check if a new established socket exists
    if(m_TCP.m_pClientSocket != nullptr)
    {
        m_pClientSocket = m_TCP.m_pClientSocket;
        m_pClientSocket->SetEventHandler(this);
    }
}

//-------------------------------------------------------------------------------------------------
//  TCP event callback
//-------------------------------------------------------------------------------------------------
void HTTP_Server::OnSocketEvent(TCP_Socket* pSocket, SocketEvent_e Event)
{
    switch(Event)
    {
        case SOCKET_EVENT_RX_READY:
            HandleRX(pSocket);
            break;

        case SOCKET_EVENT_CLOSED:
        case SOCKET_EVENT_ERROR:
            pSocket->Close();
            if(pSocket == m_pClientSocket)
            {
                m_pClientSocket = nullptr;
            }
            break;

        default:
            break;
    }
}

//-------------------------------------------------------------------------------------------------
//  Handle incoming data
//-------------------------------------------------------------------------------------------------
void HTTP_Server::HandleRX(TCP_Socket* pClient)
{
    if(pClient == nullptr)
    {
        return;
    }

    size_t Received = pClient->Receive(m_RX_Buffer, sizeof(m_RX_Buffer));
    if(Received == 0)
    {
        return;
    }

    m_RX_Length = Received;

    const char* pMethod = nullptr;
    const char* pPath   = nullptr;
    const uint8_t* pBody = nullptr;
    size_t BodyLen = 0;

    if(parseRequest(m_RX_Buffer, m_RX_Length, pMethod, pPath, pBody, BodyLen))
    {
        m_pHandler->OnRequest(this, pClient, pMethod, pPath, pBody, BodyLen);
    }
}

//-------------------------------------------------------------------------------------------------
//  Minimal HTTP request parser (GET /path HTTP/1.1)
//-------------------------------------------------------------------------------------------------
bool HTTP_Server::ParseRequest(uint8_t* pData, size_t Len,
                               const char*& pMethod,
                               const char*& pPath,
                               const uint8_t*& pBody,
                               size_t& BodyLen)
{
    pMethod = nullptr;
    pPath   = nullptr;
    pBody   = nullptr;
    BodyLen = 0;

    // Ensure null-terminated for strstr/strchr
    pData[Len] = '\0';

    // Parse method
    char* p = (char*)pData;
    char* space1 = strchr(p, ' ');
    if(space1 == nullptr)
    {
        return false;
    }

    *space1 = '\0';
    pMethod = p;

    // Parse path
    char* space2 = strchr(space1 + 1, ' ');
    if(space2 == nullptr)
    {
        return false;
    }

    *space2 = '\0';
    pPath = space1 + 1;

    // Find body (after blank line)
    char* bodyStart = strstr(space2 + 1, "\r\n\r\n");
    if(bodyStart != nullptr)
    {
        bodyStart += 4;
        pBody = (uint8_t*)bodyStart;
        BodyLen = Len - (bodyStart - (char*)pData);
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//  Send HTTP response
//-------------------------------------------------------------------------------------------------

void HTTP_Server::Send(TCP_Socket* pClient,
                       int StatusCode,
                       const char* pContentType,
                       const uint8_t* pBody,
                       size_t BodyLen)
{
    if(pClient == nullptr)
    {
        return;
    }

    char header[256];
    int headerLen = snprintf(header, sizeof(header),
                             "HTTP/1.1 %d OK\r\n"
                             "Content-Type: %s\r\n"
                             "Content-Length: %u\r\n"
                             "Connection: close\r\n"
                             "\r\n",
                             StatusCode,
                             pContentType,
                             (unsigned)BodyLen);

    pClient->Send((uint8_t*)header, headerLen);

    if(BodyLen > 0)
    {
        pClient->Send(pBody, BodyLen);
    }

    pClient->Close();
    if(pClient == m_pClientSocket)
    {
        m_pClientSocket = nullptr;
    }
}
//---------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_ETHERNET == DEF_ENABLED) && (IP_USE_HTTP == DEF_ENABLED)
