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

#include <stdint.h>

#include "digini_cfg.h"

#include "lib_memory.h"
#include "lib_string.h"
#include "lib_typedef.h"

//-------------------------------------------------------------------------------------------------
// Private variable(s) or constant(s) or macro(s), do not put in header file (.h)
//-------------------------------------------------------------------------------------------------

#define     _LIB_putchar_(s,c)                      {*((wchar_t*)s) = (wchar_t)(c);}
#define     STR_MAX_PRINT_VALUE                     256

//-------------------------------------------------------------------------------------------------
// struct or enum
//-------------------------------------------------------------------------------------------------

enum STR_Option_e
{
    STR_OPT_BASE_HEXA          = 8,         // value in bit Position for base
    STR_OPT_SIGN_NEGATIVE      = 16,     	// value in bit Position for sign
    STR_OPT_LOWERCASE          = 32,   	    // value in bit Position for base
    STR_OPT_PAD_LEFT           = 64,     	// value in bit Position for Padding flag
    STR_OPT_PAD_ZERO           = 128,	    // value in bit Position for padding with zero
};


enum STR_VarLength_e
{
    STR_VAR_16,
    STR_VAR_32,
    STR_VAR_64,
};


//-------------------------------------------------------------------------------------------------
// Private function(s), do not put in header file (.h)
//-------------------------------------------------------------------------------------------------

                  size_t _LIB_printi_     (wchar_t* pOut, int32_t lValue, uint8_t Base, uint8_t Sign, size_t Width, uint8_t Pad, uint8_t LetterBase);
template<class T> size_t _LIB_prints_     (wchar_t* pOutput, T* pStr, size_t Width, STR_Option_e Option);
                  size_t _LIB_sprintf_    (wchar_t* pOut, size_t Size, const wchar_t* pFormat, va_list vaArg);
                  size_t _LIB_str2str_    (wchar_t* pOut, const wchar_t* pString);
                  size_t _LIB_str2str_    (wchar_t* pOut, const char* pString);


//-------------------------------------------------------------------------------------------------
//
//   Function Name: LIB_snprintf
//
//   Parameter(s):  char* or wchar_t*       pOut        Output string pointer
//                  char* or wchar_t*       pFormat     Formatted string
//                  ...                                 list of argument
//   Return Value:  size_t                  Size
//
//   Description:   Print a formatted text to a string
//
//   note(s):       Overloaded function
//
//-------------------------------------------------------------------------------------------------
/**
  *--------------------------------------------------------------------------------------------------------------------
  *
  * @brief  Print a formatted text in a buffer from ...
  *
  * @param  pOut            Pointer on output string
  * @param  Size            Maximum character to print
  * @param  pFormat         Formatted string
  * @param  ...             variable list of argument
  *
  * @note           Will support the following Parameter
  *
  *                  %%               Print symbol %
  *                  %s               Print a string
  *                  %0               pad with zero
  *                  %1 to %255       number of padding space
  *                  %01 to %0255     number of padding 0
  *                  %c               special character
  *                  %l               for long support                  ( valid only for d,u,x,X )
  *                  %ll              for longlong (64 Bits) support    ( valid only for d,u,x,X )
  *                  %u               unsigned integer
  *                  %d               integer
  *                  %x               hexadecimal in lower case
  *                  %X               hexadecimal in upper case
  *
  *
  *                   note: this is the order in which they must be use
  *
  *                   ex.  %05lX   pad with ZERO +
  *                                5 padding character +
  *                                it will be a long +
  *                                it's a HEX printed in uppercase
  *
  *                   ex. %6ld     normal padding ( no Parameter ) +
  *                                padding with space ( no Parameter ) +
  *                                6  padding space on the left +
  *                                it will be a long +
  *                                it is a decimal value
  *
  *
  * @retval uint8_t         Number of character printed
  *
  *--------------------------------------------------------------------------------------------------------------------
  */

template<class T> size_t LIB_snprintf(T* pOut, size_t Size, const T* pFormat, ...)
{
    va_list vaArg;

    va_start(vaArg, (T*)pFormat);
    Size = LIB_vsnprintf(pOut, Size, pFormat, vaArg);
    va_end(vaArg);

    return Size;
}


/*
size_t LIB_snprintf(char* pOut, size_t Size, const char* pFormat, ...)
{
    va_list vaArg;

    va_start(vaArg, (const char*)pFormat);
    Size = LIB_vsnprintf(pOut, Size, pFormat, vaArg);
    va_end(vaArg);

    return Size;
}


size_t LIB_snprintf(wchar_t* pOut, size_t Size, const wchar_t* pFormat, ...)
{
    va_list vaArg;

    va_start(vaArg, (const wchar_t*)pFormat);
    Size = LIB_vsnprintf(pOut, Size, pFormat, vaArg);
    va_end(vaArg);

    return Size;
}
*/

//-------------------------------------------------------------------------------------------------
//
//   Function Name: LIB_vsnprintf
//
//   Parameter(s):  char* or wchar_t*   pOut        Output string pointer
//                  size_t              Size        Max size to print
//                  char* or wchar_t*   pFormat     Formatted string
//                  va_list             vaArg       list of argument
//
//   Return Value:  size_t        wSize
//
//   Description:   Print a formatted text to a string
//
//   note(s):       Overloaded function
//
//-------------------------------------------------------------------------------------------------
size_t LIB_vsnprintf(char* pOut, size_t Size, const char* pFormat, va_list vaArg)
{
    wchar_t*    pOutW;
    wchar_t*    pFormatW;
    uint16_t    i;
    uint16_t    _Size;

    if(Size > DIGINI_MAX_PRINT_SIZE)
    {
        Size = DIGINI_MAX_PRINT_SIZE;
    }


    pOutW     = (wchar_t*)pMemory->Alloc(Size * sizeof(wchar_t));
    pFormatW  = (wchar_t*)pMemory->Alloc(Size * sizeof(wchar_t));

    // Convert string to wchar_t type
    _Size = LIB_strnlen((char*)pFormat, Size);
    for(i = 0; i <= _Size; i++)
    {
      pFormatW[i] = (wchar_t)pFormat[i];
    }

    Size = _LIB_sprintf_(pOutW, Size, pFormatW, vaArg);

    // Convert output back to char type
    for(i = 0; i <= Size; i++)
    {
      pOut[i] = (char)pOutW[i];
    }

    pMemory->Free(&pFormatW);
    pMemory->Free(&pOutW);

    return Size;
}

size_t LIB_vsnprintf(wchar_t* pOut, size_t Size, const wchar_t* pFormat, va_list vaArg)
{
    if(Size > DIGINI_MAX_PRINT_SIZE)
    {
        Size = DIGINI_MAX_PRINT_SIZE;
    }

    Size = _LIB_sprintf_(pOut, Size, pFormat, vaArg);

    return Size;
}


//-------------------------------------------------------------------------------------------------
//
//   Function Name: _LIB_str2str_
//
//   Parameter(s):  wchar_t*                        pOut        Output string pointer
//                  const char* or const wchar_t*   pFormat     Formatted string
//   Return Value:  size_t                          Size
//
//   Description:   Transfert the string to the other string until EOL
//
//-------------------------------------------------------------------------------------------------
size_t _LIB_str2str_(wchar_t* pOut, const wchar_t* pString)
{
    size_t Counter = 0;

    for(; *pString; pString++)                                  // Print caracter until EOL
    {
        _LIB_putchar_(&pOut[Counter], (wchar_t)*pString);
        Counter++;
    }

    return Counter;
}

size_t _LIB_str2str_(wchar_t* pOut, const char* pString)
{
    size_t Counter = 0;

    for(; *pString; pString++)                                  // Print caracter until EOL
    {
        _LIB_putchar_(&pOut[Counter], (wchar_t)*pString);
        Counter++;
    }

    return Counter;
}


//-------------------------------------------------------------------------------------------------
//
//   Function Name: _LIB_prints_
//
//   Parameter(s):  wchar_t* or char*       pOut        Output string pointer
//                  wchar_t* or char*       pString     String to print
//                  size_t                  Width       Width in the print string
//                  STR_Option_e            Option         Padding, Space or 0
//   Return Value:  void
//
//   Description:   Service function to print string for printf and sprintf
//
//-------------------------------------------------------------------------------------------------
template<class T> size_t _LIB_prints_(T* pOut, T* pString, size_t Width, STR_Option_e Option)
{
    size_t          Counter;
    wchar_t         PadChar;
    size_t          Len;

    Counter = 0;
    PadChar = ' ';

    if(Width > 0)                                                       // if different than '0' -> there is padding
    {
        Len = LIB_strnlen((wchar_t*)pString, DIGINI_MAX_PRINT_SIZE);

        if(Len >= Width)                                                // check if length of the string is bigger or equal than the width of padding
        {
            Width = 0;                                                  // no padding necessary, the number is bigger than the padding space
        }
        else
        {
            Width -= Len;                                               // remove unnecessary padding because of the length of the number
        }

        if(Option & LIB_PAD_ZERO)                                 // if padding is zero, than set the variable
        {
            PadChar = '0';
        }

        if(Option & LIB_PAD_LEFT)
        {
            for(; Width > 0; Width--)
            {
                _LIB_putchar_(&pOut[Counter], PadChar);
                Counter++;
            }
        }
    }

    Counter += _LIB_str2str_(&pOut[Counter], pString);                  // Print character until EOL

    for(; Width > 0; Width--)                                           // Print padding character if any
    {
        _LIB_putchar_(&pOut[Counter], PadChar);
        Counter++;
    }

    return Counter;
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: _LIB_printi_
//
//   Parameter(s):  wchar_t*    pOut            Pointer on output string
//                  int32_t      Value           Value to print
//                  size_t      Width           Width in the print string
//                  uint8_t     Pad             Padding, Space or 0
//   Return Value:  size_t                      Number of character printed
//
//   Description:   Service function to print integer for printf and sprintf
//
//-------------------------------------------------------------------------------------------------
size_t _LIB_printi_(char* pOut, int64_t Value, size_t Width, STR_Option_e Option)
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
    LetterBase = 'A' + (Option & STR_OPT_LOWERCASE);
    Base       = (Option & STR_OPT_BASE_HEXA) ? 16 : 10;
  #endif

    if(Value == 0)
    {
        PrintBuffer[0] = '0';
        PrintBuffer[1] = '\0';
        return _LIB_prints_(pOut, PrintBuffer, Width, Option);
    }

  #ifdef STR_USE_HEX_SUPPORT
    if((Option & STR_OPT_SIGN_NEGATIVE) && (Base == 10) && (Value < 0))
  #else
    if((Option & STR_OPT_SIGN_NEGATIVE) && (Value < 0))
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
        if((Width != 0) && (Option & STR_OPT_PAD_ZERO))
        {
            _LIB_putchar_(pOut, '-');
            Counter++;
            Width--;
        }
        else
        {
            *--pString = '-';
        }
    }

    return Counter + _LIB_prints_(pOut, pString, Width, Option);
}

/* Global Functions -------------------------------------------------------------------------------------------------*/


//-------------------------------------------------------------------------------------------------
//
//   Function Name: LIB_sprintf
//
//   Parameter(s):  wchar_t*        pOut        Output string pointer
//                  size_t          Size        Maximum character to print
//                  const wchar_t*  pFormat     Formatted string
//                  va_list         va          list of argument
//   Return Value:  size_t                      Number of character printed
//
//   Description:   Print a formatted text in a buffer
//
//                  will support the following Parameter
//
//                  %%               Print symbol %
//                  %s               Print a string
//                  %ws              Print a wide string
//                  %0               pad with zero
//                  %1 to %255       number of padding space
//                  %01 to %0255     number of padding 0
//                  %c               special caracter
//                  %l               for long support        ( valid only for d,u,x,X )
//                  %u               unsigned integer
//                  %d               integer
//                  %x               hexadecimal in lower case
//                  %X               hexadecimal in upper case
//
//
//                   note: this is the order in which they must be use
//
//                   ex.  %05lX   pad with ZERO +
//                                5 padding caracter +
//                                it will be a long +
//                                it's a HEX printed in uppercase
//
//                   ex. %6ld     normal padding ( no Parameter ) +
//                                padding with space ( no Parameter ) +
//                                6  padding space on the left +
//                                it will be a long +
//                                it is a decimal value
//
//
//-------------------------------------------------------------------------------------------------
size_t _LIB_sprintf_(wchar_t* pOut, size_t Size, const wchar_t* pFormat, va_list va)
{
    uint32_t            Counter;
    size_t              Width;
    STR_VarLength_e     SizeVar;
    int64_t             n;
    uint8_t             Option;
    char*               s;
    wchar_t*            ws;
    char                scr[2];
    bool                Wide;

    Counter = 0;
	Wide    = false;

    for(; (*pFormat != '\0') && (Counter < Size); pFormat++)
    {
        if(*pFormat == '%')
        {
            pFormat++;
            Width   = 0;
            Option  = STR_OPT_PAD_LEFT;
            SizeVar = STR_VAR_16;

            if(*pFormat == '%')
            {
                _LIB_putchar_(&pOut[Counter], *pFormat);
                Counter++;
            }

            if (*pFormat == 'w')
            {
                Wide = true;
                Counter++;
                pFormat++;
            }

            if(*pFormat == 's')
            {
                if(Wide == true)
                {
                    ws = va_arg(va, wchar_t *);
                    Counter += _LIB_prints_(&pOut[Counter], ws ? ws : _W("(null)"), Width, Option);
                }
                else
                {
                    s = va_arg(va, char *);
                    Counter += _LIB_prints_(&pOut[Counter], s ? s : "(null)", Width, Option);
                }
                continue;
            }

            if(*pFormat == 'c')
            {
                scr[0] = (uint8_t)va_arg(va, int);
                scr[1] = '\0';
                Counter += _LIB_prints_(&pOut[Counter], scr, Width, Option);
                continue;
            }

            while(*pFormat == '0')
            {
                pFormat++;
                Option |= LIB_PAD_ZERO;
            }

            for(; (*pFormat >= '0') && (*pFormat <= '9'); pFormat++)
            {
                Width *= 10;
                Width += *pFormat - '0';
            }

            if(*pFormat == 'l')
            {
                SizeVar = STR_VAR_32;
                pFormat++;
                if(*pFormat == 'l')
                {
                    SizeVar = STR_VAR_64;
                    pFormat++;
                }
            }

            if(*pFormat == 'd')
            {
                switch(SizeVar)
                {
                    case STR_VAR_16: n = (int16_t)va_arg(va, int32_t);    break;
                    case STR_VAR_32: n = (int32_t)va_arg(va, int32_t);    break;
                    case STR_VAR_64: n = (int64_t)va_arg(va, int64_t);    break;
                }
                Counter += _LIB_printi_(&pOut[Counter], n, Width, Option | STR_OPT_SIGN_NEGATIVE | STR_OPT_LOWERCASE);
                continue;
            }

            switch(SizeVar)
            {
                case STR_VAR_16: n = (uint16_t)va_arg(va, uint32_t);    break;
                case STR_VAR_32: n = (uint32_t)va_arg(va, uint32_t);    break;
                case STR_VAR_64: n = (uint64_t)va_arg(va, uint64_t);    break;
            }

          #ifdef STR_USE_HEX_SUPPORT
            if(*pFormat == 'x')
            {
                Counter += _LIB_printi_(&pOut[Counter], n, Width, Option | STR_OPT_BASE_HEXA | STR_OPT_LOWERCASE);
                continue;
            }

            if(*pFormat == 'X')
            {
                Counter += _LIB_printi_(&pOut[Counter], n, Width, Option | STR_OPT_BASE_HEXA);
                continue;
            }
          #endif

            if(*pFormat == 'u')
            {
                Counter += _LIB_printi_(&pOut[Counter], n, Width, Option | STR_OPT_LOWERCASE);
                continue;
            }
        }
        else
        {
            _LIB_putchar_(&pOut[Counter], *pFormat);
            Counter++;
        }
    }

    _LIB_putchar_(&pOut[Counter], '\0');

    return Counter;
}


//-------------------------------------------------------------------------------------------------
//
//   Function Name: LIB_sformat
//
//   Parameter(s):  wchar_t*        pOut            Output string pointer
//                  size_t          Size            Size limit of the pOut buffer
//                  const wchar_t*  pString         string to print
//                  uint8_t         Width           Width in the print string
//                  uint8_t         Pad             Padding, Space or 0
//   Return Value:  size_t          Size
//
//   Description:   Special printf that allow argument to be located anywhere in string according
//                  to the position override
//
//-------------------------------------------------------------------------------------------------
size_t LIB_vsnformat(wchar_t* pOut, size_t Size, const wchar_t* pFormat, va_list va)
{
/*
    LIB_sFormat* pFmt;
    size_t       PointerCounter = 0;

    pFmt = (LIB_sFormat*)pMemory->Alloc(sizeof(LIB_sFormat));
    pFmt->pFormat = (wchar_t*)pMemory->Alloc(DIGINI_MAX_PRINT_SIZE);                             // Get memory to work this printf

    LIB_strncpy(pFmt->pFormat, pFormat, DIGINI_MAX_PRINT_SIZE);                                        // Copy from possible const location to RAM
    pFmt->pFmtPtr    = pFmt->pFormat;
    pFmt->byCounter  = 0;

//    LIB_MemSetBYTE(&pFmt->byPosition[0], 0xFF, 16);

    // This first loop is to capture each position
    for(; *pFmt->pFmtPtr != 0; pFmt->pFmtPtr++)
    {
        if(*pFmt->pFmtPtr == '$')                                                                   // Process each position override
        {
            pFmt->pFmtPtr++;
            pFmt->pDestination = pFmt->pFmtPtr;

            if(*pFmt->pFmtPtr != '$')
            {
                *(pFmt->pFmtPtr - 1) = (uint8_t)ASCII_SUBSTITUTION;

                if((*pFmt->pFmtPtr >= '0') && (*pFmt->pFmtPtr <= '9'))                              // Get position from 0 to 9
                {
                    pFmt->byPosition[pFmt->byCounter++] = (uint8_t)(*pFmt->pFmtPtr - '0');
                    pFmt->pFmtPtr++;
                }
                else if((*pFmt->pFmtPtr >= 'A') && (*pFmt->pFmtPtr <= 'F'))                         // Get position from A to F
                {
                    pFmt->byPosition[pFmt->byCounter++] = (uint8_t)(*pFmt->pFmtPtr - ('A' - 10));
                    pFmt->pFmtPtr++;
                }
                else
                {
                    pFormat->pDestination--;
                }
                LIB_strncpy(pFmt->pDestination, pFmt->pFmtPtr, DIGINI_MAX_PRINT_SIZE);                 // Strip '$' and position override from string
                pFmt->pFmtPtr = pFmt->pDestination - 1;
            }
            else
            {
                pFmt->pFmtPtr++;
                LIB_strncpy(pFmt->pDestination, pFmt->pFmtPtr, DIGINI_MAX_PRINT_SIZE);                 // Strip '$' and position override from string
                pFmt->pFmtPtr = pFmt->pDestination;
            }
        }
    }

    pFmt->pFmtPtr    = pFmt->pFormat;
    pFmt->pFormatPtr = (char*)pFormat;
    pFmt->byCounter  = 0;

//    LIB_MemClrPOINTER(&pFmt->pSwitchArg[0], 16);


    // this second loop will strip any formatting info from the copy string
    // and get pointer on each formatting info in original string
    for(; *pFmt->pFmtPtr != 0; pFmt->pFmtPtr++, pFmt->pFormatPtr++)
    {
        if(*pFmt->pFmtPtr == '%')
        {
            pFmt->pDestination = pFmt->pFmtPtr;
            pFmt->pFmtPtr++;
            pFmt->pFormatPtr++;

            if(*pFmt->pFmtPtr == '%')
            {
                pFmt->pDestination++;
                pFmt->pFmtPtr++;
                pFmt->pFormatPtr++;
            }
            else
            {
                for(pFmt->i = 0; pFmt->i < 16; pFmt->i++)
                {
                    if(pFmt->byPosition[pFmt->i] == pFmt->byCounter)
                    {
                        pFmt->pSwitchArg[pFmt->i] = pFmt->pFormatPtr;
                        pFmt->pListArg[pFmt->i]   = va;
                        break;
                    }
                }
                pFmt->byCounter++;

                if(*pFmt->pFmtPtr == 's')
                {
                    va_arg(va, char*);
                    pFmt->pFmtPtr++;
                    pFmt->pFormatPtr++;
                }
                else if(*pFmt->pFmtPtr == 'c')
                {
                    va_arg(va, uint8_t);
                    pFmt->pFmtPtr++;
                    pFmt->pFormatPtr++;
                }
                else
                {
                    while(*pFmt->pFmtPtr == '0')
                    {
                        pFmt->pFmtPtr++;
                        pFmt->pFormatPtr++;
                    }

                    if((*pFmt->pFmtPtr >= '0') && (*pFmt->pFmtPtr <= '9'))
                    {
                        pFmt->pFmtPtr++;
                        pFmt->pFormatPtr++;
                    }

                    if(*pFmt->pFmtPtr == 'l')
                    {
                        pFmt->bySizeVar = LIB_VAR_LONG;
                        pFmt->pFmtPtr++;
                        pFmt->pFormatPtr++;
                    }
                    else
                    {
                        pFmt->bySizeVar  = LIB_VAR_INT;
                    }

                    if(*pFmt->pFmtPtr == 'd')
                    {
                        (pFmt->bySizeVar == LIB_VAR_INT) ? va_arg(va, short) : va_arg(va, long);
                        pFmt->pFmtPtr++;
                        pFmt->pFormatPtr++;
                    }

                    else if((*pFmt->pFmtPtr == 'X') || (*pFmt->pFmtPtr == 'u'))
                    {
                        (pFmt->bySizeVar == LIB_VAR_INT) ? va_arg(va, uint16_t) : va_arg(va, uint32_t);
                        pFmt->pFmtPtr++;
                        pFmt->pFormatPtr++;
                    }
                }
            }
            LIB_strncpy(pFmt->pDestination, pFmt->pFmtPtr);                                    // Strip any formatting information
            pFmt->pFmtPtr = pFmt->pDestination - 1;
            pFmt->pFormatPtr--;
        }
    }

    pFmt->pFmtPtr    = pFmt->pFmt;
    pFmt->pFormatPtr = (char*)pFormat;
    pFmt->byCounter  = 0;

    // Do the actual formatting according to correct argument placement
    for(; (*pFmt->pFmtPtr != 0) && (byOutCounter < bySize); pFmt->pFmtPtr++)
    {
        if(*pFmt->pFmtPtr == (uint8_t)ASCII_SUBSTITUTION)
        {
            pFmt->byWidth    = 0;
            pFmt->byPad      = LIB_PAD_LEFT;
            pFmt->bySizeVar  = LIB_VAR_INT;
            pFmt->pFormatPtr = pFmt->pSwitchArg[pFmt->byCounter];

            if(pFmt->pFormatPtr != nullptr)
            {
                if(*pFmt->pFormatPtr == 's')
                {
                    pFmt->s = va_arg(pFmt->pListArg[pFmt->byCounter], char*);
                    pFmt->byCounter++;
                    PointerCounter += LIB_prints(&pOut[PointerCounter], pFmt->s ? pFmt->s : (char*)pNULL_Str, pFmt->byWidth, pFmt->byPad);
                    pFormat->pFormatPtr++;
                    continue;
                }

                if(*pFormat->pFormatPtr == 'c')
                pFmt
                    pFmt->scr[0] = va_arg(VPf->pListArg[pFmt->byCounter], char);
                    pFmt->scr[1] = '\0';
                    pFmt->byCounter++;
                    PointerCounter += LIB_prints(&pOut[PointerCounter], pFmt->scr, pFmt->byWidth, pFmt->byPad);
                    pFmt->pFormatPtr++;
                    continue;
                }

                while(*pFmt->pFormatPtr == '0')
                {
                    pFmt->byPad |= LIB_PAD_ZERO;
                    pFmt->pFormatPtr++;
                }

                for(; *pFmt->pFormatPtr >= '0' && *pFmt->pFormatPtr <= '9'; pFmt->pFormatPtr++)
                {
                    pFmt->byWidth *= 10;
                    pFmt->byWidth += *pFmt->pFormatPtr - '0';
                }

                if(*pFmt->pFormatPtr == 'l')
                {
                    pFmt->bySizeVar = LIB_VAR_LONG;
                    pFmt->pFormatPtr++;
                }

                if(*pFmt->pFormatPtr == 'd')
                {
                    if(pFmt->bySizeVar == LIB_VAR_INT)   pFmt->n = va_arg(pFmt->pListArg[pFmt->byCounter], short);
                    else                                pFmt->n = va_arg(pFmt->pListArg[pFmt->byCounter], long);

                    pFmt->byCounter++;
                    PointerCounter += _LIB_printi_(&pOut[PointerCounter], pFmt->n, 10, 1, pFmt->byWidth, pFmt->byPad, 'a');
                    pFmt->pFormatPtr++;
                    continue;
                }

                if(*pFmt->pFormatPtr == 'X')
                {
                    if(pFmt->bySizeVar == LIB_VAR_INT)   pFmt->n = va_arg(pFmt->pListArg[pFmt->byCounter], uint16_t);
                    else                                pFmt->n = va_arg(pFmt->pListArg[pFmt->byCounter], uint32_t);

                    pFmt->byCounter++;
                    PointerCounter += _LIB_printi_(&pOut[PointerCounter], pFmt->n, 16, 0, pFmt->byWidth, pFmt->byPad, 'A');
                    pFmt->pFormatPtr++;
                    continue;
                }

                if(*pFmt->pFormatPtr == 'u')
                {
                    if(pFmt->bySizeVar == LIB_VAR_INT)   pFmt->n = va_arg(pFmt->pListArg[pFmt->byCounter], uint16_t);
                    else                                pFmt->n = va_arg(pFmt->pListArg[pFmt->byCounter], uint32_t);

                    pFmt->byCounter++;
                    PointerCounter += _LIB_printi_(&pOut[PointerCounter], pFmt->n, 10, 0, pFmt->byWidth, pFmt->byPad, 'a');
                    pFmt->pFormatPtr++;
                    continue;
                }
            }
        }
        else
        {
            LIB_PrintChar(&pOut[PointerCounter++], *pFmt->pFmtPtr);
        }
    }

    if(pOut)
    {
        pOut[PointerCounter] = '\0';
    }

    pMemory->Free(&pFmt->pFmt);
    pMemory->Free(&pFmt);
    return PointerCounter;
*/return 0;
}


