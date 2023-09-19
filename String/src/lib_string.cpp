//-------------------------------------------------------------------------------------------------
//
//  File : lib_sprintf.cpp
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

#include "lib_digini.h"

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define LIB_OPT_PAD_ZERO            128         // value in bit Position for padding with zero
#define LIB_OPT_PAD_LEFT            64          // value in bit Position for Padding flag
#define LIB_OPT_LOWERCASE           32          // value in bit Position for base
#define LIB_OPT_SIGN_NEGATIVE       16          // value in bit Position for sign
#define LIB_OPT_BASE_HEXA           8           // value in bit Position for base

#define LIB_putchar(s,c)                      {*((char*)s) = (char)(c);}

//-------------------------------------------------------------------------------------------------
// enum(s)
//-------------------------------------------------------------------------------------------------

enum VarLength_e
{
    LIB_VAR_16,
    LIB_VAR_32,
    LIB_VAR_64,
};

//-------------------------------------------------------------------------------------------------
// Forward declaration
//-------------------------------------------------------------------------------------------------

static size_t LIB_vsnformat  (char* pOut, size_t Size, const char* pFormat, va_list Arg);
static size_t LIB_printi     (char* pOut, int64_t Value, size_t Width, uint8_t Option);
static size_t LIB_prints     (char* pOut, char* pString, size_t Width, uint8_t Option);
static size_t LIB_str2str    (char* pOut, const char* pString);

//-------------------------------------------------------------------------------------------------
// Private const(s)
//-------------------------------------------------------------------------------------------------

static const char StrNULL[7] = "{null}";

//-------------------------------------------------------------------------------------------------
// Private function(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//   Function Name: LIB_prints
//
//   Parameter(s):  char*   pOut        Output string pointer
//                  char*   pString     String to print
//                  size_t  Width       Width in the print string
//                  uint8_t Option      Padding, Space or 0
//   Return Value:  size_t
//
//   Description:   Service function to print string for printf and sprintf
//
//-------------------------------------------------------------------------------------------------
static size_t LIB_prints(char* pOut, char* pString, size_t Width, uint8_t Option)
{
    size_t  Counter;
    size_t  Len;
    char    PadChar;

    Counter = 0;
    PadChar = ' ';

    if(Width > 0)                                           // If different than '0' -> there is padding
    {
        Len = strnlen(pString, 255);

        // Check if length of the string is bigger or equal than the width of padding
        if(Len >= Width)               Width   = 0;         // No padding necessary, the number is bigger than the padding space
        else                           Width  -= Len;       // Remove unnecessary padding because of the length of the number
        if(Option & LIB_OPT_PAD_ZERO)  PadChar = '0';       // If padding is zero, than set the variable

        if(Option & LIB_OPT_PAD_LEFT)
        {
            for(; Width > 0; Width--)
            {
                LIB_putchar(&pOut[Counter], PadChar);
                Counter++;
            }
        }
    }

    Counter += LIB_str2str(&pOut[Counter], pString);        // Print character until EOL

    for(; Width > 0; Width--)                               // Print padding character if any
    {
        LIB_putchar(&pOut[Counter], PadChar);
        Counter++;
    }

    return Counter;
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: LIB_printi
//
//   Parameter(s):  char*       pOut            Output string pointer
//                  int32_t     Value           Value to print
//                  size_t      Width           Width in the print string
//                  uint8_t     Option          Padding, Space or 0, Upper or lowercase
//   Return Value:  size_t                      Number of character printed
//
//   Description:   Service function to print integer for printf and sprintf
//
//-------------------------------------------------------------------------------------------------
static size_t LIB_printi(char* pOut, int64_t Value, size_t Width, uint8_t Option)
{
    size_t      Counter;
    uint64_t    _Value;
    char        PrintBuffer[STR_MAX_PRINT_VALUE];
    char*       pString;
    bool        Negative;
    char        Temp;
  #ifdef STR_USE_HEX_SUPPORT
    uint8_t     LetterBase;
    uint8_t     Base;
  #endif

    Counter    = 0;
    Negative   = false;
    _Value     = Value;

  #ifdef STR_USE_HEX_SUPPORT
    LetterBase = 'A' + (Option & LIB_OPT_LOWERCASE);
    Base       = (Option & LIB_OPT_BASE_HEXA) ? 16 : 10;
  #endif

    if(Value == 0)
    {
        PrintBuffer[0] = '0';
        PrintBuffer[1] = '\0';
        return LIB_prints(pOut, PrintBuffer, Width, Option);
    }

  #ifdef STR_USE_HEX_SUPPORT
    if((Option & LIB_OPT_SIGN_NEGATIVE) && (Base == 10) && (Value < 0))
  #else
    if((Option & LIB_OPT_SIGN_NEGATIVE) && (Value < 0))
  #endif
    {
        Negative = true;
        _Value   = -Value;
    }

    pString = PrintBuffer + (STR_MAX_PRINT_VALUE - 1);
    *pString = '\0';

    while(_Value)
    {
      #ifdef STR_USE_HEX_SUPPORT

        Temp = _Value % Base;

        if(Temp >= 10)
        {
            Temp += (LetterBase -'0' - 10);
        }

        *--pString = Temp + '0';
        _Value = _Value / Base;

      #else

        Temp = _Value % 10;
        *--pString = Temp + '0';
        _Value = _Value / 10;

      #endif
    }

    if(Negative == true)
    {
        if((Width != 0) && (Option & LIB_OPT_PAD_ZERO))
        {
            LIB_putchar(pOut, '-');
            Counter++;
            Width--;
        }
        else
        {
            *--pString = '-';
        }
    }

    return Counter + LIB_prints(pOut, pString, Width, Option);
}


//-------------------------------------------------------------------------------------------------
//
//   Function Name: LIB_str2str
//
//   Parameter(s):  char*       pOut        Output string pointer
//                  const char* pFormat     Formatted string
//   Return Value:  size_t      Size
//
//   Description:   Transfer the string to the other string until EOL
//
//-------------------------------------------------------------------------------------------------

static size_t LIB_str2str(char* pOut, const char* pString)
{
    size_t Counter = 0;

    for(; *pString; pString++)                                  // Print caracter until EOL
    {
        LIB_putchar(&pOut[Counter], (wchar_t)*pString);
        Counter++;
    }
    LIB_putchar(&pOut[Counter], '\0');

    return Counter;
}


//-------------------------------------------------------------------------------------------------
//
//   Function Name: LIB_vsnprintf
//
//   Parameter(s):  char*           pOut        Output string pointer
//                  size_t          Size        Maximum character to print
//                  const char*     pFormat     Formatted string
//                  va_list         va          list of argument
//   Return Value:  size_t                      Number of character printed
//
//   Description:   Print a formatted text in a buffer
//
//                  will support the following Parameter
//
//                  %%               Print symbol %
//                  %s               Print a string
//                  %0               pad with zero
//                  %1 to %255       number of padding space
//                  %01 to %0255     number of padding 0
//                  %c               special character
//                  %l               for long support               ( valid only for d,u,x,X )
//                  %ll              for longlong (64 Bits) support ( valid only for d,u,x,X ) Ex. %llu
//                  %u               unsigned integer
//                  %d               integer
//                  %X               hexadecimal in upper case
//
//
//                   note: this is the order in which they must be use
//
//                   ex.  %05lX   pad with ZERO +
//                                5 padding character +
//                                it will be a long +
//                                it's a HEX printed in uppercase
//
//                   ex. %6ld     normal padding ( no Parameter ) +
//                                padding with space ( no Parameter ) +
//                                6  padding space on the left +
//                                it will be a long +
//                                it is a decimal value
//
//-------------------------------------------------------------------------------------------------
size_t LIB_vsnprintf(char* pOut, size_t Size, const char* pFormat, va_list va)
{

    uint32_t    Counter;
    size_t      Width;
    VarLength_e SizeVar;
    int64_t     n;
    uint8_t     Option;
    char*       s;
    char        scr[2];

    if(*pFormat == ASCII_SUBSTITUTION)
    {
       return LIB_vsnformat(pOut, Size, pFormat, va);
    }

    Counter = 0;

    for(; (*pFormat != '\0') && (Counter < Size); pFormat++)
    {
        if(*pFormat == '%')
        {
            pFormat++;
            Width   = 0;
            Option  = LIB_OPT_PAD_LEFT;
            SizeVar = LIB_VAR_16;

            if(*pFormat == '%')
            {
                LIB_putchar(&pOut[Counter], *(char*)pFormat);
                Counter++;
            }

            if(*pFormat == 's')
            {
                s = va_arg(va, char *);
                Counter += LIB_prints(&pOut[Counter], s ? s : (char*)"(null)", Width, Option);
                continue;
            }

            if(*pFormat == 'c')
            {
                scr[0] = (uint8_t)va_arg(va, int);
                scr[1] = '\0';
                Counter += LIB_prints(&pOut[Counter], scr, Width, Option);
                continue;
            }

            while(*pFormat == '0')
            {
                pFormat++;
                Option |= LIB_OPT_PAD_ZERO;
            }

            for(; (*pFormat >= '0') && (*pFormat <= '9'); pFormat++)
            {
                Width *= 10;
                Width += *pFormat - '0';
            }

            if(*pFormat == 'l')
            {
                SizeVar = LIB_VAR_32;
                pFormat++;
                if(*pFormat == 'l')
                {
                    SizeVar = LIB_VAR_64;
                    pFormat++;
                }
            }

            if(*pFormat == 'd')
            {
                switch(SizeVar)
                {
                    case LIB_VAR_16: n = (int16_t)va_arg(va, int32_t);    break;
                    case LIB_VAR_32: n = (int32_t)va_arg(va, int32_t);    break;
                    case LIB_VAR_64: n = (int64_t)va_arg(va, int64_t);    break;
                }
                Counter += LIB_printi(&pOut[Counter], n, Width, Option | LIB_OPT_SIGN_NEGATIVE | LIB_OPT_LOWERCASE);
                continue;
            }

            switch(SizeVar)
            {
                case LIB_VAR_16: n = (uint16_t)va_arg(va, uint32_t);    break;
                case LIB_VAR_32: n = (uint32_t)va_arg(va, uint32_t);    break;
                case LIB_VAR_64: n = (uint64_t)va_arg(va, uint64_t);    break;
            }

          #ifdef STR_USE_HEX_SUPPORT
            if(*pFormat == 'x')
            {
                Counter += LIB_printi(&pOut[Counter], n, Width, Option | LIB_OPT_BASE_HEXA | LIB_OPT_LOWERCASE);
                continue;
            }

            if(*pFormat == 'X')
            {
                Counter += LIB_printi(&pOut[Counter], n, Width, Option | LIB_OPT_BASE_HEXA);
                continue;
            }
          #endif

            if(*pFormat == 'u')
            {
                Counter += LIB_printi(&pOut[Counter], n, Width, Option | LIB_OPT_LOWERCASE);
                continue;
            }
        }
        else
        {
            LIB_putchar(&pOut[Counter], *pFormat);
            Counter++;
        }
    }

    LIB_putchar(&pOut[Counter], '\0');

    return Counter;
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: LIB_vsnformat
//
//   Parameter(s):  char*       pOut        Pointer on output string
//                  size_t      Size        Maximum character to print
//                  const char* pFormat     Formatted string
//                  va_list     va          list of argument
//   Return Value:  size_t      Size
//
//   Description:   Special printf that allow argument to be located anywhere in string according
//                  to the position override
//
//   Note(s):       Not for 16 bits CPU, need to do different variables for va_arg()
//                  ex. #define VAR_CHAR ..., VAR_UINT16_T ..., etc..
//-------------------------------------------------------------------------------------------------
size_t LIB_vsnformat(char* pOut, size_t Size, const char* pFormat, va_list va)
{
    STR_Format_t* pFmt;
    size_t        PointerCounter = 0;
    char*         pFmtPtr;
    char*         pDestination;
    char*         pWorkFormat;              // Copy of the format to work on
    char*         pFormatPtr;
    uint32_t      Counter;


    if((pFmt = (STR_Format_t*)pMemoryPool->AllocAndSet(sizeof(STR_Format_t), 0xFF)) == nullptr)
    {
        return PointerCounter;
    }

    if((pWorkFormat = (char*)pMemoryPool->AllocAndClear(DIGINI_MAX_PRINT_SIZE)) == nullptr)         // Get memory to work this printf
    {
        pMemoryPool->Free((void**)&pFmt);
        return PointerCounter;
    }

    pFormatPtr = (char*)pFormat + 1;
    strncpy(pWorkFormat, pFormatPtr, DIGINI_MAX_PRINT_SIZE);                                        // Copy from possible const location to RAM
    pFmtPtr    = pWorkFormat;
    Counter    = 0;

    // This first loop is to capture each position
    for(; *pFmtPtr != '\0';)
    {
        // ************************************************************************************************************************
        // Process Position Override

        if(*pFmtPtr == '$')                                                                         // Process each position override
        {
            if(*(pFmtPtr + 1) != '$')
            {
                *pFmtPtr = (uint8_t)ASCII_SUBSTITUTION;
                pFmtPtr++;
                pFormatPtr++;
                pDestination = pFmtPtr;

                if((*pFmtPtr >= '0') && (*pFmtPtr <= '9'))                                          // Get position from 0 to 9
                {
                    pFmt->Position[Counter] = (uint8_t)(*pFmtPtr - '0');
                    pFmtPtr++;
                    pFormatPtr++;
                }
                else if((*pFmtPtr >= 'A') && (*pFmtPtr <= 'F'))                                     // Get position from A to F
                {
                    pFmt->Position[Counter] = (uint8_t)(*pFmtPtr - ('A' - 10));
                    pFmtPtr++;
                    pFormatPtr++;
                }
                else // so no value.. just print it
                {
                    pDestination--;
                }

                memcpy(pDestination, pFmtPtr, strlen(pFmtPtr) + 1);                                 // Strip '$' and position override from string
                pFmtPtr = pDestination;

                // ****************************************************************************************************************
                // Organize Formatting Information

                if(*pFmtPtr == '%')
                {
                    pFormatPtr++;
                    pFmt->pSwitchArg[pFmt->Position[Counter]] = pFormatPtr;
                    pFmt->pListArg[pFmt->Position[Counter]]   = va;
                    Counter++;
                    pFmtPtr++;

                    if(*pFmtPtr == 's')
                    {
                        va_arg(va, char*);
                        pFmtPtr++;
                        pFormatPtr++;
                    }
                    else if(*pFmtPtr == 'c')
                    {
                        va_arg(va, int);
                        pFmtPtr++;
                        pFormatPtr++;
                    }
                    else
                    {
                        while(*pFmtPtr == '0')
                        {
                            pFmtPtr++;
                            pFormatPtr++;
                        }

                        if((*pFmtPtr >= '1') && (*pFmtPtr <= '9'))
                        {
                            pFmtPtr++;
                            pFormatPtr++;
                        }

                        if(*pFmtPtr == 'l')
                        {
                            pFmt->SizeVar = LIB_VAR_32;
                            pFmtPtr++;
                            pFormatPtr++;
                        }
                        else
                        {
                            pFmt->SizeVar  = LIB_VAR_16;
                        }

                        if(*pFmtPtr == 'd')
                        {
                            (pFmt->SizeVar == LIB_VAR_16) ? va_arg(va, int) : va_arg(va, long);
                            pFmtPtr++;
                            pFormatPtr++;
                        }
                        else if((*pFmtPtr == 'X') || (*pFmtPtr == 'u'))
                        {
                            (pFmt->SizeVar == LIB_VAR_16) ? va_arg(va, int) : va_arg(va, uint32_t);
                            pFmtPtr++;
                            pFormatPtr++;
                        }
                    }
                }

                memcpy(pDestination, pFmtPtr, strlen(pFmtPtr) + 1);                                 // Strip any formatting information
                pFmtPtr = pDestination;
            }
            else //  Strip '$' and position override from string and skip good one
            {
                pFmtPtr++;
                memcpy(pDestination, pFmtPtr, strlen(pFmtPtr) + 1);
                pFmtPtr = pDestination + 1;
                pFormatPtr += 2;
            }
        }
        else
        {
            if(*pFmtPtr == '%')
            {
                if(*(pFmtPtr + 1) == '%') // if 2 consecutives % are found one is stripped... only one will also work
                {
                    memcpy(pFmtPtr, pFmtPtr + 1, strlen(pFmtPtr) + 1);
                    pFormatPtr++;
                }
            }

            pFmtPtr++;        // Skip to next normal character to print
            pFormatPtr++;
        }
    }

    // ****************************************************************************************************************************
    // Do the actual formatting according to correct argument placement

    Counter = 0;

    for(; (*pWorkFormat != '\0') && (PointerCounter < Size); pWorkFormat++)
    {
        if(*pWorkFormat == (uint8_t)ASCII_SUBSTITUTION)
        {
            pFmt->Width   = 0;
            pFmt->Option  = LIB_OPT_PAD_LEFT;
            pFmt->SizeVar = LIB_VAR_16;
            pFormatPtr    = pFmt->pSwitchArg[Counter];

            if(pFormatPtr != nullptr)
            {
                if(*pFormatPtr == 's')
                {
                    pFmt->s = va_arg(pFmt->pListArg[Counter], char*);
                    Counter++;
                    PointerCounter += LIB_prints(&pOut[PointerCounter], pFmt->s ? pFmt->s : (char*)StrNULL, pFmt->Width, pFmt->Option);
                    continue;
                }

                if(*pFormatPtr == 'c')
                {
                    pFmt->scr[0] = va_arg(pFmt->pListArg[Counter], int);
                    pFmt->scr[1] = '\0';
                    Counter++;
                    PointerCounter += LIB_prints(&pOut[PointerCounter], pFmt->scr, pFmt->Width, pFmt->Option);
                    continue;
                }

                while(*pFormatPtr == '0')
                {
                    pFmt->Option |= LIB_OPT_PAD_ZERO;
                    pFormatPtr++;
                }

                for(; ((*pFormatPtr >= '0') && (*pFormatPtr <= '9')); pFormatPtr++)
                {
                    pFmt->Width *= 10;
                    pFmt->Width += *pFormatPtr - '0';
                }

                if(*pFormatPtr == 'l')
                {
                    pFmt->SizeVar = LIB_VAR_32;
                    pFormatPtr++;
                }

                if(*pFormatPtr == 'd')
                {
                    if(pFmt->SizeVar == LIB_VAR_16)   pFmt->n = va_arg(pFmt->pListArg[Counter], int);
                    else                              pFmt->n = va_arg(pFmt->pListArg[Counter], long);

                    Counter++;
                    PointerCounter += LIB_printi(&pOut[PointerCounter], pFmt->n, pFmt->Width, pFmt->Option | LIB_OPT_SIGN_NEGATIVE | LIB_OPT_LOWERCASE);
                    continue;
                }

                if(*pFormatPtr == 'X')
                {
                    if(pFmt->SizeVar == LIB_VAR_16)  pFmt->n = va_arg(pFmt->pListArg[Counter], int);
                    else                             pFmt->n = va_arg(pFmt->pListArg[Counter], uint32_t);

                    Counter++;
                    PointerCounter += LIB_printi(&pOut[PointerCounter], pFmt->n, pFmt->Width, pFmt->Option | LIB_OPT_BASE_HEXA);
                    continue;
                }

                if(*pFormatPtr == 'u')
                {
                    if(pFmt->SizeVar == LIB_VAR_16)   pFmt->n = va_arg(pFmt->pListArg[Counter], int);
                    else                              pFmt->n = va_arg(pFmt->pListArg[Counter], uint32_t);

                    Counter++;
                    PointerCounter += LIB_printi(&pOut[PointerCounter], pFmt->n, pFmt->Width, pFmt->Option | LIB_OPT_LOWERCASE);
                    continue;
                }
            }
        }
        else
        {
            LIB_putchar(&pOut[PointerCounter++], *pWorkFormat);
        }
    }

    if(pOut)
    {
        pOut[PointerCounter] = '\0';
    }

    pMemoryPool->Free((void**)&pWorkFormat);
    pMemoryPool->Free((void**)&pFmt);

    return PointerCounter;
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: LIB_strnstrip
//
//   Parameter(s):  char*   pString
//                  size_t  nSize
//   Return Value:  None
//
//   Description:   Strip the trailing space starting at nSize
//
//-------------------------------------------------------------------------------------------------
void LIB_strnstrip(char* pString, size_t nSize)
{
    char* End;

    End = pString + (nSize - 1);
    while((End >= pString))
    {
        if(*End == ' ')
        {
            *End = '\0';
        }
        else
        {
            return;
        }
        End--;
    }
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: LIB_strnchr
//
//   Parameter(s):  const char*      pStr        Destination buffer
//                  size_t           Size        Size max to scan
//                  char             Chr         Character to found
//   Return Value:  const char*      pPtr        Pointer if found or nullptr
//
//   Description:   Locate first occurence of caracter in a string up to the EOL
//                  EOL is part of the string and can be use to find the end of string
//
//-------------------------------------------------------------------------------------------------
char* LIB_strnchr(char* pStr, size_t Size, char Chr)
{
    size_t Count = 0;


    if(Size > DIGINI_MAX_PRINT_SIZE)
    {
        Size = DIGINI_MAX_PRINT_SIZE;
    }

    while((*pStr != '\0') && (*pStr != Chr) && (Count < Size))
    {
        pStr++;
        Count++;
    }

    if((*pStr == '\0') && (Chr != '\0'))
    {
        pStr = nullptr;
    }
    return pStr;
}

//-------------------------------------------------------------------------------------------------
