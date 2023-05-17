//-------------------------------------------------------------------------------------------------
//
//  File : crc.cpp
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

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include <stdint.h>
#include "crc.h"

#if (DIGINI_USE_CRC == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// reference(s)
//-------------------------------------------------------------------------------------------------

// http://www.barrgroup.com/Embedded-Systems/How-To/CRC-Calculation-C-Code
// http://create.stephan-brumme.com/crc32/
// http://www.opensource.apple.com/source/xnu/xnu-1456.1.26/bsd/libkern/crc32.c

//-------------------------------------------------------------------------------------------------
// const(s)
//-------------------------------------------------------------------------------------------------

const CRC_Info_t CRC::m_MethodList[NUMBER_OF_CRC_METHOD] =
{
    CRC_METHOD_DEF(EXPAND_X_CRC_AS_CLASS_CONST)
};


// CRC 16 ANSI
#ifdef CRC_16_MODBUS_USE_TABLE

// CRC table for the CRC-16. The poly is 0x8005 (x^16 + x^15 + x^2 + 1)
static const uint16_t CRC_16_MODBUS_table[256] =
{
    0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
    0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
    0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
    0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
    0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
    0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
    0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
    0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
    0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
    0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
    0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
    0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
    0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
    0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
    0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
    0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
    0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
    0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
    0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
    0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
    0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
    0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
    0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
    0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
    0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
    0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
    0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
    0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
    0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
    0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
    0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
    0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};
#endif

// CRC 16 CCITT
#ifdef CRC_16_CCITT_USE_TABLE
// This mysterious table is just the CRC of each possible byte. It can be
// computed using the standard bit-at-a-time methods. The polynomial can
// be seen in entry 128, 0x8408. This corresponds to x^0 + x^5 + x^12.
// Add the implicit x^16, and you have the standard CRC-CCITT.
static const uint16_t CRC_16_CCITT_table[256] =
{
    0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
    0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
    0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
    0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
    0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
    0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
    0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
    0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
    0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
    0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
    0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
    0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
    0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
    0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
    0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
    0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
    0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
    0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
    0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
    0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
    0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
    0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
    0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
    0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
    0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
    0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
    0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
    0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
    0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
    0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
    0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
    0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};

#endif

// CRC 32 IEEE
#ifdef CRC_32_IEEE_USE_TABLE
static const uint32_t CRC_32_IEEE_table[256] =
{
    /* 0x00 */ 0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,    /* 0x04 */ 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
    /* 0x08 */ 0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,    /* 0x0C */ 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
    /* 0x10 */ 0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,    /* 0x14 */ 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
    /* 0x18 */ 0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,    /* 0x1C */ 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
    /* 0x20 */ 0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,    /* 0x24 */ 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
    /* 0x28 */ 0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,    /* 0x2C */ 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
    /* 0x30 */ 0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,    /* 0x34 */ 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
    /* 0x38 */ 0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,    /* 0x3C */ 0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
    /* 0x40 */ 0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,    /* 0x44 */ 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
    /* 0x48 */ 0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,    /* 0x4C */ 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
    /* 0x50 */ 0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,    /* 0x54 */ 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
    /* 0x58 */ 0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,    /* 0x5C */ 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
    /* 0x60 */ 0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,    /* 0x64 */ 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
    /* 0x68 */ 0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,    /* 0x6C */ 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
    /* 0x70 */ 0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,    /* 0x74 */ 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
    /* 0x78 */ 0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,    /* 0x7C */ 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
    /* 0x80 */ 0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,    /* 0x84 */ 0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
    /* 0x88 */ 0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,    /* 0x8C */ 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
    /* 0x90 */ 0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,    /* 0x94 */ 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
    /* 0x98 */ 0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,    /* 0x9C */ 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
    /* 0xA0 */ 0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,    /* 0xA4 */ 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
    /* 0xA8 */ 0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,    /* 0xAC */ 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
    /* 0xB0 */ 0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,    /* 0xB4 */ 0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
    /* 0xB8 */ 0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,    /* 0xBC */ 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
    /* 0xC0 */ 0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,    /* 0xC4 */ 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
    /* 0xC8 */ 0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,    /* 0xCC */ 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
    /* 0xD0 */ 0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,    /* 0xD4 */ 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
    /* 0xD8 */ 0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,    /* 0xDC */ 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
    /* 0xE0 */ 0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,    /* 0xE4 */ 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
    /* 0xE8 */ 0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,    /* 0xEC */ 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
    /* 0xF0 */ 0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,    /* 0xF4 */ 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
    /* 0xF8 */ 0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,    /* 0xFC */ 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D,
};

#endif

//-------------------------------------------------------------------------------------------------
//
//   Class: CRC
//
//   Description:   Class to handle CRC Calculation
//
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//   Constructor:   CRC
//
//   Parameter(s):  CrcType_e
//
//   Description:   Initializes the CRC_ID that define the method for calculating the CRC
//
//-------------------------------------------------------------------------------------------------
CRC::CRC(CRC_Type_e Type)
{
    m_Type    = Type;
//    m_Polynomial = Polynomial;
//    m_Xor        = Xor;
//    m_Width      = Width;
//    m_Mask       = 0xFFFFFFFF >> (32 - Width);
//    m_Remainder  = 0xFFFFFFFF;
}

//-------------------------------------------------------------------------------------------------
//
//   Destructor:    ~CRC
//
//   Parameter(s):  None
//
//   Description:   In some case there are resource to be freed
//
//-------------------------------------------------------------------------------------------------
CRC::~CRC()
{
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Start
//
//  Parameter(s):   CrcType
//  Return:         void
//
//  Description:    This function init the conversion with the CRC type selected.
//
//-------------------------------------------------------------------------------------------------
void CRC::Start(void)
{
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Done
//
//  Parameter(s):   void
//  Return:         uint32_t
//
//  Description:    This function return the CRC value.
//
//-------------------------------------------------------------------------------------------------
uint32_t CRC::Done(void)
{
    if(m_Type == CRC_NORMAL)
    {
        m_Remainder ^= m_Xor;
        m_Remainder &= m_Mask;
    }

    return m_Remainder;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Byte
//
//  Parameter(s):   uint8_t Byte
//  Return:         void
//
//  Description:    Find the CRC of a byte.
//
//-------------------------------------------------------------------------------------------------
void CRC::Byte(const uint8_t Byte)
{
    uint32_t TopBit;

    switch(m_CrcType)
    {
        case CRC_8:
        {
            #ifdef CRC_16_MODBUS_USE_TABLE
            m_Remainder = CRC_16_MODEBUS_table[(m_Remainder ^ Byte) & 0xFF] ^ (m_Remainder >> 8);
            #else
            m_Remainder = 0;
            #endif
            break;
        }
        case CRC_8_EBU:
        {
            #ifdef CRC_16_MODBUS_USE_TABLE
            #else
            m_Remainder = 0;
            #endif
            break;
        }
        case CRC_16_MODBUS:
        {
            #ifdef CRC_16_MODBUS_USE_TABLE
            m_Remainder = CRC_16_MODEBUS_table[(m_Remainder ^ Byte) & 0xFF] ^ (m_Remainder >> 8);
            #else
            m_Remainder = 0;
            #endif
            break;
        }

        case CRC_16_CCITT:
        {
            #ifdef CRC_16_CCITT_USE_TABLE
            m_Remainder = CRC_16_CCITT_table[(m_Remainder ^ Byte) & 0xFF] ^ (m_Remainder >> 8);
            #endif
            m_Remainder = 0;
            break;
        }

        case CRC_32_IEEE:
        {
            #ifdef CRC_32_IEEE_USE_TABLE
            m_Remainder = CRC_32_IEEE_table[(m_Remainder ^ Byte) & 0xFF] ^ (m_Remainder >> 8);
            #else
            m_Remainder = 0;
            #endif
            break;
        }

        case CRC_NORMAL:
        {
            m_Remainder    = m_Remainder ^ ((uint32_t)Byte << (m_Width - 8) );
            TopBit        = 0x1 << (m_Width - 1);

            for(uint32_t i = 8; i > 0; i-- )
            {
                if(m_Remainder & TopBit)
                {
                    m_Remainder <<= 1;
                    m_Remainder ^= m_Polynomial;
                }
                else
                {
                    m_Remainder <<= 1;
                }
            }
            break;
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Done
//
//  Parameter(s):   pBuffer
//                  Len
//  Return:         void
//
//  Description:    Find the CRC of a buffer of byte.
//
//-------------------------------------------------------------------------------------------------
void CRC::Buffer(const uint8_t *pBuffer, uint32_t Len)
{
    for(; Len > 0; Len--, pBuffer++)
    {
        Byte(*pBuffer);
    }
}



// TODO AR will use my own library instead of repeating stuff at the infinite
/**
  \fn          uint32_t crc32_8bit_rev (uint32_t crc32, uint8_t val)
  \brief       Calculate 32-bit CRC (Polynomial: 0x04C11DB7, data bit-reversed).
  \param[in]   crc32  CRC initial value
  \param[in]   val    Input value
  \return      Calculated CRC value
*/
static uint32_t crc32_8bit_rev (uint32_t crc32, uint8_t val)
{
    uint32_t n;

    crc32 ^= __RBIT(val);

    for(n = 8; n; n--)
    {
        if(crc32 & 0x80000000)
        {
            crc32 <<= 1;
            crc32  ^= 0x04C11DB7;
        }
        else
        {
            crc32 <<= 1;
        }
    }
    return crc32;
}

/**
  \fn          uint32_t crc32_data (const uint8_t *data, uint32_t len)
  \brief       Calculate standard 32-bit Ethernet CRC.
  \param[in]   data  Pointer to buffer containing the data
  \param[in]   len   Data length in bytes
  \return      Calculated CRC value
*/
static uint32_t crc32_data(const uint8_t *data, uint32_t len)
{
    uint32_t cnt;
    uint32_t crc;

    crc = 0xFFFFFFFF;

    for(cnt = len; cnt > 0; cnt--)
    {
        crc = crc32_8bit_rev (crc, *data++);
    }

    return crc ^ 0xFFFFFFFF;
}

//-------------------------------------------------------------------------------------------------

#endif

//-------------------------------------------------------------------------------------------------
