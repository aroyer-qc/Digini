//-------------------------------------------------------------------------------------------------
//
//  File :  lib_class_dns.h
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2010-2024 Alain Royer.
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
// Include file(s)
//-------------------------------------------------------------------------------------------------

#ifdef DNS_GLOBAL
    #define DNS_EXTERN
    #define DNS_PRIVATE
#else
    #define DNS_EXTERN extern
#endif

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define DNS_MAX_DNSMSG_SIZE         	512            		// Maximum size of DNS message
#define DNS_MAX_QNAME_LEN           	128                 // Maximum size of QNAME

#define DNS_MAX_DOMAIN_NAME_LENGHT  	128                 // Maximum size of domain name

#define DNS_NO_IP                   	0

// Flag QR
#define DNS_QR_QUERY                	0
#define DNS_QR_RESPONSE             	1

// Flag OPCODE
#define DNS_OPCODE_STANDARD_QUERY   	0                	// Standard query (IQUERY)  Name -> IP
#define DNS_OPCODE_INVERSE_QUERY    	1                   // Inverse query (IQUERY)   IP -> Name
#define DNS_OPCODE_STATUS           	2                   // Server status request

//Flag RD
#define DNS_RD_NO_RECURSION         	0
#define DNS_RD_RECURSION            	1

// QCLASS
#define DNS_CLASS_IN                	1                   // The Internet

#define DNS_PORT                 		htons(53)


// TYPE
#define DNS_TYPE_A                      1   				// The ARPA Internet
#define DNS_TYPE_NS                     2   				// An authoritative name server
#define DNS_TYPE_MD                     3   				// A mail destination (Obsolete - use MX)
#define DNS_TYPE_MF                     4   				// A mail forwarder (Obsolete - use MX)
#define DNS_TYPE_CNAME                  5   				// The canonical name for an alias
#define DNS_TYPE_SOA                    6   				// Marks the start of a zone of authority
#define DNS_TYPE_MB                     7   				// A mailbox domain name (EXPERIMENTAL)
#define DNS_TYPE_MG                     8   				// A mail group member (EXPERIMENTAL)
#define DNS_TYPE_MR                     9   				// A mail rename domain name (EXPERIMENTAL)
#define DNS_TYPE_NULL                   10  				// A null RR (EXPERIMENTAL)
#define DNS_TYPE_WKS                    11  				// A well known service description
#define DNS_TYPE_PTR                    12  				// A domain name pointer
#define DNS_TYPE_HINFO                  13  				// Host information
#define DNS_TYPE_MINFO                  14  				// Mailbox or mail list information
#define DNS_TYPE_MX                     15  				// Mail exchange
#define DNS_TYPE_TXT                    16  				// Text strings

//-------------------------------------------------------------------------------------------------
// Type definition(s) and structure(s)
//-------------------------------------------------------------------------------------------------

typedef struct
{
    uint16_t ID;
    union
    {
        struct
        {
            uint8_t    RD      :1;
            uint8_t    TC      :1;
            uint8_t    AA      :1;
            uint8_t    OPCODE  :4;
            uint8_t    QR      :1;
        } s;
        uint8_t by;
    } Flags_1;

    union
    {
        struct
        {
            uint8_t    RCODE   :4;
            uint8_t    Z       :3;
            uint8_t    RA      :1;
        } s;
        uint8_t by;
    } Flags_2;

    uint16_t QD_Count;
    uint16_t AN_Count;
    uint16_t NS_Count;
    uint16_t AR_Count;
    uint8_t  Data[500];
} DNS_Msg_t;


//-------------------------------------------------------------------------------------------------
// 
//-------------------------------------------------------------------------------------------------

class NetDNS
{
    public:

        IP_Address_t    Query           (Socket_t SocketNumber, uint8_t* pDomainName, uint8_t* pError);

    private:

        IP_Address_t    DNS_Response    (Socket_t SocketNumber);

        uint16_t        m_ID;
};


//-------------------------------------------------------------------------------------------------

