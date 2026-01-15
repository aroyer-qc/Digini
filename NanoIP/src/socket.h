//=================================================================================================
//
//  File : socket.h
//
//  By   : Alain Royer
//
//  On   : Spetember 15, 2010
//
//-------------------------------------------------------------------------------------------------

#ifndef __SOCKET_H__
#define __SOCKET_H__

#ifdef SOCKET_GLOBAL
	#define	SOCKET_EXTERN
#else	
	#define	SOCKET_EXTERN extern
#endif

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------
	
#include <stddef.h>
#include <ip_cfg.h>
#include <bsp.h>
#include <mem.h>
#include <ip.h>
#include <tcp.h>
// ...

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define SOCK_TIME_OUT			4					// 4 * 30 Seconds chunk = 2 Minutes


//...

//-------------------------------------------------------------------------------------------------
// macro(s)
//-------------------------------------------------------------------------------------------------

// ...

//-------------------------------------------------------------------------------------------------
// Enum(s)
//-------------------------------------------------------------------------------------------------
	
typedef enum
{
	TCP_SOCKET_SYN_RECEIVE,
	TCP_SOCKET_LISTENING,
	TCP_SOCKET_CLOSE_WAIT,
	TCP_SOCKET_CLOSE,
}SocketState_e;

// ...

//-------------------------------------------------------------------------------------------------
// Type definition(s) and structure(s)
//-------------------------------------------------------------------------------------------------

typedef struct
{
	LinkList_t		List;										// Link to other PortInfo_t
	WORD			wNumber;									// Listen to this port
	WORD			wProtocol;                                  // this is the protocol attach to this port
	WORD			wBindSocketCount;
	void 			(*pFunction)(void*, BYTE*, WORD);			// function bind to this port
} PortInfo_t;

typedef struct
{
	LinkList_t				List;
	PortInfo_t*				pPortInfo;						   // The incoming pair is link to this PortInfo_t
	BYTE				    MAC[6];
	WORD					wClientPort;                       // Return answers to this client port
	UDWORD					ClientIP;                          // at this IP
	UDWORD					SequenceNumber;
	UDWORD					AckNumber;
    BYTE					byConnectionState;
	BYTE					byTimer;
	struct
	{
		WORD	wUnacknowledge;
		WORD	wNext;
		WORD	wWindow;
	}Send;
	struct
	{
		WORD	wNext;
		WORD	wWindow;
	}Receive;
} SocketInfo_t;

// ...

//-------------------------------------------------------------------------------------------------
// Global variable(s) and constant(s)
//-------------------------------------------------------------------------------------------------

SOCKET_EXTERN PortInfo_t*			SOCK_pPortList;
SOCKET_EXTERN SocketInfo_t* 		SOCK_pSocketList;

SOCKET_EXTERN WORD					SOCK_wPortCount;				// keep count of how many listening socket are active
SOCKET_EXTERN WORD					SOCK_wSocketCount;				// keep count of how many connection socket are active
		
// ...

//-------------------------------------------------------------------------------------------------
// Function prototype(s)
//-------------------------------------------------------------------------------------------------
 
BYTE 					SOCK_Init				(void);
void 			 		SOCK_OpenPort			(WORD wPort, WORD wProtocol, void* (*pFunction)(SocketInfo_t*, BYTE*, WORD));
BYTE 			 		SOCK_ClosePort			(WORD wPort, WORD wProtocol);
PortInfo_t* 			SOCK_ValidPort			(WORD wPort, WORD wProtocol);
SocketInfo_t*			SOCK_OpenSocket			(PortInfo_t* pPort, IP_PacketMsg_t* pRX, BYTE* pError);
BYTE	 				SOCK_CloseSocket		(SocketInfo_t* pSocket);
SocketInfo_t* 			SOCK_LookupSocket		(IP_PacketMsg_t* pRX, BYTE* pError);



//void 					SOCK_Listen				(void);
//void 					SOCK_Accept				(void);
//void					SOCK_Send				(void);
//void					SOCK_Recv				(void);



#endif //__SOCKET_H__
//EOF

