//-------------------------------------------------------------------------------------------------
//
//  File :  lib_class_http.h
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
//                  - 
//                  - 
//                  - 
//
//-------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_ETHERNET == DEF_ENABLED) && (IP_USE_HTTP == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define HTTP_LISTEN_PORT         HTONS(80)


//-------------------------------------------------------------------------------------------------
// Enum(s)
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// Class definition(s)
//-------------------------------------------------------------------------------------------------

class HTTP_Server;

class HTTP_Handler
{
	public:

		virtual 	   ~HTTP_Handler() {}

		// Called when a full HTTP request has been parsed
		virtual void 			OnRequest					(HTTP_Server* pServer, TCP_Socket* pClient, const char* pMethod, const char* pPath, const uint8_t* pBody, size_t BodyLen) 		= 0;
};

class HTTP_Server : public TCP_SocketEventHandler
{
	public:

		HTTP_Server(NetworkContext* pContext, TCP_Manager& TCP)
			: m_pContext(pContext)
			, m_TCP(TCP)
		{}

		bool    				Begin						(uint16_t Port, HTTP_Handler* pHandler);
		void    				Stop						(void);

		void    				Process						(void);                                  // Called from main loop

		// Send a simple HTTP response (Arduino-style)
		void    				Send						(TCP_Socket* pClient, int StatusCode, const char* pContentType,	const uint8_t* pBody, size_t BodyLen);

		// TCP event callback
		void    				OnSocketEvent				(TCP_Socket* pSocket, SocketEvent_e Event) override;

	private:
	
		void    				AcceptIfNeeded				(void);
		void    				HandleRX					(TCP_Socket* pClient);
		bool    				ParseRequest				(uint8_t* pData, size_t Len, const char*& pMethod, const char*& pPath, const uint8_t*& pBody, size_t& BodyLen);

		NetworkContext*     	m_pContext;
		TCP_Manager&        	m_TCP;
		HTTP_Handler*       	m_pHandler = nullptr;

		Socket*             	m_pListenSocket = nullptr;

		// Later: support multiple clients
		TCP_Socket*         	m_pClientSocket = nullptr;

		// Temporary RX buffer
		uint8_t             	m_RX_Buffer[512];
		size_t              	m_RX_Length = 0;
};

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_ETHERNET == DEF_ENABLED) && (IP_USE_HTTP == DEF_ENABLED)
