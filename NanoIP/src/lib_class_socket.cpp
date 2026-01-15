//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_socket.c
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2024 Alain Royer.
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


// it was a work in progress.. to be continued


//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "./lib_digini.h"

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

//#define TCP_DEFAULT_WINDOW_SIZE         (4 * 1460)   // 5840 bytes
//#define TCP_TX_BUFFER_SIZE              (4 * 1460)   // 5840 bytes
//#define TCP_RX_BUFFER_SIZE              (4 * 1460)   // 5840 bytes
//#define UDP_RX_BUFFER_SIZE              2048
//#define RAW_RX_BUFFER_SIZE              1536

//-------------------------------------------------------------------------------------------------

Socket* SocketManager::AllocSocket(SocketType_e Type)
{
    for(uint16_t i = 0; i < SOCKET_MAX_COUNT; i++)
    {
        if(SocketManager::m_SocketInUse[i] == false)
        {
            SocketManager::m_SocketInUse[i] = true;
            Socket* pSock = &SocketManager::m_SocketTable[i];
            memset(pSock, 0, sizeof(Socket));
            pSock->m_SocketID = i;
            pSock->Create(Type);
            return pSock;
        }
    }

    return nullptr;
}

void SocketManager::FreeSocket(Socket** ppSocket)
{
    if((ppSocket == nullptr) || (*ppSocket == nullptr))
    {
        return;
    }

    Socket* pSock = *ppSocket;
    uint16_t id = pSock->m_SocketID;

    pSock->Close();
    SocketManager::m_SocketInUse[id] = false;
    *ppSocket = nullptr;
}

SystemState_e Socket::SendTo(uint8_t* pData, size_t Length, SocketInfo_t* pDestInfo, size_t* pBytesSent)
{
    if(m_Type != SOCKET_TYPE_DGRAM)
    {
        return SYS_INVALID_STATE;
    }

    UDP_Socket_t* pUdp = m_Protocol.pUDP;
    return UDP_Send(pUdp, pData, Length, pDestInfo, pBytesSent);
}

SystemState_e Socket::RecvFrom(uint8_t* pBuffer, size_t BufferSize, SocketInfo_t* pSrcInfo, size_t* pBytesReceived)
{
    if(m_Type != SOCKET_TYPE_DGRAM)
    {
        return SYS_INVALID_STATE;
    }

    UDP_Socket_t* pUdp = m_Protocol.pUDP;
    IP_PacketMsg_t* pMsg = nullptr;

    if(nOS_QueueRead(&pUdp->RxQueue, &pMsg, m_TimeoutMs) != NOS_OK)
    {
        return SYS_TIMEOUT;
    }

    // Copy payload out of pMsg into pBuffer, fill pSrcInfo from IP/UDP headers
    // ...

    pMemoryPool->Free((void**)&pMsg->pPacket);
    pMemoryPool->Free((void**)&pMsg);
    *pBytesReceived = payloadLen;
    return SYS_READY;
}

SystemState_e Socket::Send(uint8_t* pData, size_t Length, size_t* pBytesSent)
{
    if(m_Type != SOCKET_TYPE_STREAM)
    {
        return SYS_INVALID_STATE;
    }

    TCP_Socket_t* pTcp = m_Protocol.pTCP;
    return TCP_Send(pTcp, pData, Length, pBytesSent);
}

SystemState_e Socket::SendTo(uint8_t* pData, size_t Length, SocketInfo_t* pDestInfo, size_t* pBytesSent)
{
    if(m_Type != SOCKET_TYPE_DGRAM)
    {
        return SYS_INVALID_STATE;
    }

    UDP_Socket_t* pUdp = m_Protocol.pUDP;
    return UDP_Send(pUdp, pData, Length, pDestInfo, pBytesSent);
}

SystemState_e Socket::Recv(uint8_t* pBuffer, size_t BufferSize, size_t* pBytesReceived)
{
    if(m_Type != SOCKET_TYPE_STREAM)
    {
        return SYS_INVALID_STATE;
    }

    TCP_Socket_t* pTcp = m_Protocol.pTCP;
    IP_PacketMsg_t* pMsg = nullptr;

    if(nOS_QueueRead(&pTcp->RxQueue, &pMsg, m_TimeoutMs) != NOS_OK)
    {
        return SYS_TIMEOUT;
    }

    TCP_Header_t* pTCP = &pMsg->pPacket->TCP_Frame.Header;
    IP_Header_t*  pIP  = &pMsg->pPacket->TCP_Frame.IP_Header;

    size_t headerLen = (pTCP->DataOffset >> 4) * 4;
    size_t ipLen     = ntohs(pIP->Length);
    size_t dataLen   = ipLen - sizeof(IP_IP_Header_t) - headerLen;

    if(dataLen > BufferSize)
    {
        dataLen = BufferSize;
    }

    uint8_t* pPayload = (uint8_t*)((uint8_t*)pTCP + headerLen);
    memcpy(pBuffer, pPayload, dataLen);

    // Free segment
    pMemoryPool->Free((void**)&pMsg->pPacket);
    pMemoryPool->Free((void**)&pMsg);

    *pBytesReceived = dataLen;
    return SYS_READY;
}

SystemState_e Socket::RecvFrom(uint8_t* pBuffer, size_t BufferSize, SocketInfo_t* pSrcInfo, size_t* pBytesReceived)
{
    if(m_Type != SOCKET_TYPE_DGRAM)
    {
        return SYS_INVALID_STATE;
    }

    UDP_Socket_t* pUdp = m_Protocol.pUDP;
    IP_PacketMsg_t* pMsg = nullptr;

    if(nOS_QueueRead(&pUdp->RxQueue, &pMsg, m_TimeoutMs) != NOS_OK)
    {
        return SYS_TIMEOUT;
    }

    UDP_Header_t* pUDP = &pMsg->pPacket->UDP_Frame.Header;
    IP_Header_t*  pIP  = &pMsg->pPacket->UDP_Frame.IP_Header;

    size_t udpLen = ntohs(pUDP->Length);
    size_t payloadLen = udpLen - sizeof(UDP_Header_t);

    if(payloadLen > BufferSize)
    {
        payloadLen = BufferSize;
    }

    uint8_t* pPayload = (uint8_t*)(pUDP + 1);
    memcpy(pBuffer, pPayload, payloadLen);

    if(pSrcInfo != nullptr)
    {
        pSrcInfo->Address = pIP->SrcIP_Addr;
        pSrcInfo->Port    = ntohs(pUDP->SrcPort);
        // Fill MacAddress if you store it, etc.
    }

    // Free packet now that we copied data
    pMemoryPool->Free((void**)&pMsg->pPacket);
    pMemoryPool->Free((void**)&pMsg);

    *pBytesReceived = payloadLen;
    return SYS_READY;
}

