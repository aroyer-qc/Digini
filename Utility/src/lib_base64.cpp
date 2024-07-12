//-------------------------------------------------------------------------------------------------
//
//  File : lib_Base64.cpp
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

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "./lib_digini.h"

//-------------------------------------------------------------------------------------------------
// const(s)
//-------------------------------------------------------------------------------------------------

const char Base64[] = {"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"};

//-------------------------------------------------------------------------------------------------
//
//   Function Name: LIB_EncodeBase64
//
//   Parameter(s):  const uint8_t*      Pointer on data stream to encode
//   Return:        char*               Base 64 encoded string
//
//   Description:   Encode data stream into base64 string
//
//   Note:          Caller must free the memory of the returned pointer
//
//-------------------------------------------------------------------------------------------------
char* LIB_EncodeBase64(const uint8_t* pSource)
{
    size_t   PreNeededChar;
    size_t   TotalNeededChar;
    size_t   SizeSrc;
    char*    pPtrEncode;
    char*    pEncode;
    uint8_t* pPtrSrc;
    size_t   Counter = 0;
    uint32_t BitArray = 0;
    size_t   Offset = 0;

    SizeSrc       = strlen(pSource);
    PreNeededChar = ((SizeSrc * 4) / 3);
    TotalNeededChar = (PreNeededChar + 4) - (PreNeededChar % 4) + 1;
    pEncode       = (char*)pMemoryPool->Alloc((PreNeededChar + 4) - (PreNeededChar % 4)));
    pPtrEncode    = pEncode;
    pPtrSrc       = (uint8_t*)pSource;

    while(SizeSrc-- != 0)
    {
        Offset    = 16 - ((Counter % 3) * 8);
        BitArray += (uint32_t(*pPtrSrc) << Offset);
        pPtrSrc++;

        if(Offset == 16)
        {
            *pPtrEncode = Base64[(BitArray >> 18) & 0x3F];
        }

        if(Offset == 8)
        {
            *pPtrEncode = Base64[(BitArray >> 12) & 0x3F];
        }

        if((Offset == 0) && (Counter != 3))
        {
            *pPtrEncode = Base64[(BitArray >> 6) & 0x3F];
            pPtrEncode++;
            *pPtrEncode = Base64[BitArray & 0x3F];
            BitArray = 0;
        }

        pPtrEncode++;
        Counter++;
    }

    if(Offset == 16)
    {
        *pPtrEncode = Base64[(BitArray >> 12) & 0x3F];
        pPtrEncode++;
        *pPtrEncode = '=';
        pPtrEncode++;
        *pPtrEncode = '=';
        pPtrEncode++;
    }

    if(Offset == 8)
    {
        *pPtrEncode = Base64[(BitArray >> 6) & 0x3F];
        pPtrEncode++;
        *pPtrEncode = '=';
        pPtrEncode++;
    }

    return pEncode;
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: LIB_DecodeBase64
//
//   Parameter(s):  const char*         Base 64 encoded string
//   Return:        uint8_t*            Pointer on decoded data stream
//
//   Description:   Decode base64 into data stream
//
//   Note:          Caller must free the memory of the returned pointer
//
//-------------------------------------------------------------------------------------------------
uint8_t* LIB_DecodeBase64(const char* pSource)
{
    size_t   SizeDecode;
    size_t   SizeSrc;
    char*    pPtrSrc;
    uint8_t* pPtrDecode;
    size_t   Offset   = 0;
    size_t   Counter  = 0;
    uint32_t BitArray = 0;
    uint8_t* pDecode;

    SizeSrc    = strlen(pSource);
    SizeDecode = ((SizeSrc * 3) / 4) + 1;
    pDecode    = pMemoryPool->AllocAndClear(SizeDecode);
    pPtrDecode = pDecode;
    pPtrSrc    = (char*)pSource;

    while(SizeSrc-- != 0)
    {
        char Char = *pPtrSrc;

        if(Char != '=')
        {
            char* Pos1 = (char*)Base64;
            char* Pos2 = strchr(Base64, Char);
            size_t   Pos  = size_t(Pos2 - Pos1);

            if(Offset != Pos)
            {
                size_t const Offset = 18 - ((Counter % 4) * 6);
                BitArray += (uint32_t(Pos) << Offset);

                if(Offset == 12)
                {
                    *pPtrDecode = uint8_t(BitArray >> 16);
                    pPtrDecode++;
                }

                if(Offset == 6)
                {
                    *pPtrDecode = uint8_t(BitArray >> 8);
                    pPtrDecode++;
                }

                if((Offset == 0) && (Counter != 4))
                {
                    *pPtrDecode = uint8_t(BitArray);
                    pPtrDecode++;
                    BitArray = 0;
                }

                pPtrSrc++;
            }
            else if(*pPtrSrc != '=')
            {
                pMemoryPool->Free(&pDecode);
                return nullptr;;
            }

            Counter++;
        }
    }

    return pDecode;
}

//-------------------------------------------------------------------------------------------------
