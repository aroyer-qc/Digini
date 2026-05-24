//-------------------------------------------------------------------------------------------------
//
//  File :  lib_basic_printf.cpp
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

#include "./lib_digini.h"

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_GRAFX == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Private function(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//   Function Name: Draw
//
//   Parameter(s):  Box_t*                          pBox
//                  const char* or const wchar_t*   pFormat
//                  ...                             list of argument
//   Return Value:  size_t                          Size
//
//   Description:   Print a formatted text on the display. It will be auto center in both X and Y
//                  according to X size, and Y size, this function support multi-line.
//
//   note:          Overloaded function
//                  Maximum of lines define by DIGINI_MAX_PRINT_NUMBER_OF_LINE.
//                  Maximum of characters define by DIGINI_MAX_PRINT_SIZE.
//
//-------------------------------------------------------------------------------------------------
size_t GPrintf::Draw(Box_t* pBox, const char* pFormat, ...)
{
    va_list     vaArg;
    uint16_t    Size;

    va_start(vaArg, pFormat);
    Size = this->Draw(pBox, pFormat, vaArg);
    va_end(vaArg);

    return Size;
}


//-------------------------------------------------------------------------------------------------
//
//   Function Name: Draw
//
//   Parameter(s):  Box_t*        pBox
//                  const char*   pFormat
//                  va_list       vaArg              list of argument
//   Return Value:  size_t
//
//   Description:   Print a formatted text on the display. It will be auto center in both X and Y
//                  according to X size, and Y size, this function support multi-line.
//
//   note:          Maximum of lines define by DIGINI_MAX_PRINT_NUMBER_OF_LINE.
//                  Maximum of characters define by DIGINI_MAX_PRINT_SIZE.
//
//-------------------------------------------------------------------------------------------------
size_t GPrintf::Draw(Box_t* pBox, const char* pFormat, va_list vaArg)
{
    size_t Size = 0;

    if(pBox == nullptr)
    {
        return 0;
    }

    if(BoxValid(pBox) != true)
    {
        return 0;
    }

    m_Size = LIB_vsnprintf(&m_String[0], DIGINI_MAX_PRINT_SIZE, pFormat, vaArg);

    if(m_Size != 0)
    {
        m_pLocalBox = pBox;
        PutString();
        Size = m_Size;
    }

    return Size;
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: PutString
//
//   Parameter(s):  None
//   Return Value:  size_t           Size
//
//   Description:   Print a formatted text on the display. It will be auto center in both X and Y
//                  according to X size, and Y size, this function support multi-line.
//
//   note:          Maximum of 8 lines per print and also 128 characters MAX.
//
//                  ASCII_CARRIAGE_RETURN       new line:       0x0D
//                  ASCII_DEVICE_CTRL1          Color change:   0x11 + RRGGBB in ascii (single)
//                  ASCII_DEVICE_CTRL2          Color change:   0x12 + RRGGBB in ascii (change)
//                  ASCII_DEVICE_CTRL3          Font change:    0x13 + FF  (single)
//                  ASCII_DEVICE_CTRL4          Font change:    0x14 + FF  (change)
//
//          TODO (Alain#2#).. if Bearing + size is bigger than the font size.. need to add it to total size of box.. to erase properly the font
//          so .Size.Width + Bearing > Total size... also check for the negative size (if size expand negatively)
//
//
//-------------------------------------------------------------------------------------------------
size_t GPrintf::PutString(void)
{
    uint32_t   KeepDrawingColor = DisplayLayer::GetColor();
    FontInfo_t FontInfo;

    // Parse features and compute layout
    m_pFontUsedInString  = (Font_e*)    pMemoryPool->Alloc(sizeof(Font_e) * m_Size);
    m_pColorUsedInString = (uint32_t*)  pMemoryPool->Alloc(sizeof(uint32_t) * m_Size);

    ParseFeature();
    ParseString();

    m_pMovingUsedFontPtr  = m_pFontUsedInString;
    m_pMovingUsedColorPtr = m_pColorUsedInString;

    // Load first font info
    DB_Central.Get(&FontInfo, GFX_FONT_INFO, *m_pFontUsedInString, 0);

    // Unified print engine
    uint16_t totalLines = 1;

  #if (GRAFX_USE_MULTI_LINE == DEF_ENABLED)
    totalLines = m_Line;
  #endif

    m_Position.Y = m_pLocalBox->Pos.Y + m_OffsetJustY;

    for(uint16_t line = 0; line < totalLines; line++)
    {
        // Compute X justification
        m_Position.X = m_pLocalBox->Pos.X + m_OffsetJustX;

      #if (GRAFX_USE_MULTI_LINE == DEF_ENABLED)
        switch(GetXY_Justification() & _X_LINE_JUSTIFICATION)
        {
            case _X_LINE_CENTER:
			{
                m_Position.X += ((m_BoxSizeX - m_SubLineSizePixX[line]) >> 1);
			}
            break;

            case _X_LINE_RIGHT:
			{
                m_Position.X += (m_BoxSizeX - m_SubLineSizePixX[line]);
			}
            break;
        }
      #endif

        // Number of characters in this line
        uint16_t charCount =
      #if (GRAFX_USE_MULTI_LINE == DEF_ENABLED)
        m_SubLineSizeChar[line];
      #else
        m_Size;
      #endif

        // Pointer to the string for this line
        const char* pLineString =
      #if (GRAFX_USE_MULTI_LINE == DEF_ENABLED)
        m_pSubLineString[line];
      #else
        m_pString;
      #endif

        // Print each character
        for(uint16_t j = 0; j < charCount; j++)
        {
          #if (GRAFX_USE_ROM_DATABASE == DEF_ENABLED)
            uint8_t Character = uint32_t(pLineString[j]) - uint32_t(FontInfo.FirstCaracter);                      // Get the offset for this font

            if(Character <= (FontInfo.LastCaracter - FontInfo.FirstCaracter))
            {
                if(FontInfo.pLookUpTable != nullptr)
                {
                    Character = FontInfo.pLookUpTable[Character];
                }

                const FontDescriptor_t* AddresstDescriptor = FontInfo.pDescriptor + Character;
                memcpy(&m_FontDescriptor, (void*)AddresstDescriptor, sizeof(FontDescriptor_t));

                m_CorrectedPos.X = m_Position.X + m_FontDescriptor.LeftBearing;
                m_CorrectedPos.Y = m_Position.Y + m_FontDescriptor.OffsetY;

                if(pLineString[j] != ' ')
                //if(m_FontDescriptor.pAddress != 0) //need fix in GUI_Builder
                {
                    DisplayLayer::SetTextColor(*m_pMovingUsedColorPtr);
                    PrintFont(&m_FontDescriptor, &m_CorrectedPos);
                }
            }
          #else // RAM database
			DB_Central.Get(&m_FontDescriptor, GFX_FONT_DESC_INFO, *m_pMovingUsedFontPtr, pLineString[j]);
			m_CorrectedPos.X = m_Position.X + m_FontDescriptor.LeftBearing;
			m_CorrectedPos.Y = m_Position.Y + m_FontDescriptor.OffsetY;

			if(m_FontDescriptor.pAddress != 0)
			{
			    DisplayLayer::SetTextColor(*m_pMovingUsedColorPtr);
			    PrintFont(&m_FontDescriptor, &m_CorrectedPos);
			}
	      #endif

          #if (GRAFX_PAINT_BOX_DEBUG == DEF_ENABLED)
            if((m_FontDescriptor.WidthPixel != 0) && (m_FontDescriptor.HeightPixel != 0))
            {
                uint32_t Color = DisplayLayer::GetColor();
                DisplayLayer::SetColor(GRAFX_PAINT_BOX_DEBUG_COLOR);
                DrawBox(m_Position.X,
                        m_CorrectedPos.Y,
                        m_FontDescriptor.Width,
                        m_FontDescriptor.HeightPixel,
                        1);
                DisplayLayer::SetColor(BLUE);
                DrawBox(m_CorrectedPos.X,
                        m_CorrectedPos.Y,
                        m_FontDescriptor.WidthPixel,
                        m_FontDescriptor.HeightPixel;
                        1);
                DisplayLayer::SetColor(Color);
            }
          #endif

            m_Position.X += m_FontDescriptor.HorizontalAdvance;
            IncrementFeaturePointer();
        }

        // Next line
        m_Position.Y += (FontInfo.Height + FontInfo.Interline);
        IncrementFeaturePointer();
    }

    // Cleanup
    pMemoryPool->Free((void**)&m_pColorUsedInString);
    pMemoryPool->Free((void**)&m_pFontUsedInString);
    DisplayLayer::SetColor(KeepDrawingColor);

    return 0;
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: ParseString
//
//   Parameter(s):  None
//
//   Return Value:  void
//
//   Description:   Sub service function for to parse printf for box size, position, justification
//
//   note:
//
//-------------------------------------------------------------------------------------------------
void GPrintf::ParseString(void)
{
    uint16_t    j;
    FontInfo_t  FontInfo;

    //***********************************************************************
    //
    // Find out how many segment there is to print
    // and get pointer and size on each segment
    //
  #if (GRAFX_USE_MULTI_LINE == DEF_ENABLED)
    m_pSubLineString[0] = &m_String[0];
    uint8_t  k = 0;

    for(uint16_t i = 1; i < DIGINI_MAX_PRINT_NUMBER_OF_LINE; i++)
    {
        m_pSubLineString[i] = LIB_strnchr(&m_String[k], m_Size, '\n');

        if(m_pSubLineString[i] == nullptr)
        {
            m_SubLineSizeChar[i - 1] = (uint8_t)strnlen(&m_String[k], m_Size);              // Get size for the last string
            m_Line = i;
            break;
        }
        m_SubLineSizeChar[i - 1] = (uint8_t)(m_pSubLineString[i] - m_pSubLineString[i - 1]);// Calculate size in character for each string
        m_pSubLineString[i]++;                                                              // Put pointer on beginning of the string
        k = (uint8_t)(m_pSubLineString[i] - m_pSubLineString[0]);                           // Get position for the next search point
    }
  #endif

    //***********************************************************************
    //
    // Find beginning and end off each segment from an absolute position
    // and get the minimum Y size and maximum Y size
    //
    m_BoxSizeX = 0;
    m_BoxSizeY = 0;

    m_pMovingUsedFontPtr = m_pFontUsedInString;                                             // Reload pointer on memory block

    // Get the Font height for this particular font
    DB_Central.Get(&FontInfo, GFX_FONT_INFO, *m_pMovingUsedFontPtr, 0);

  #if (GRAFX_USE_MULTI_LINE == DEF_ENABLED)

    //**********************************************************************
    // Parse all line
    for(uint16_t i = 0; i < m_Line; i++)
    {
        // Preinit all Parameter for this line
        m_MinY[i]            = 0xFF;
        m_SubLineSizePixX[i] = 0;

        for(j = 0; j < m_SubLineSizeChar[i]; j++, m_pMovingUsedFontPtr++)                   // Parse all character in this line
        {
            if(*(m_pSubLineString[i] + j) != ASCII_CARRIAGE_RETURN)                         // Do all normal font parsing
            {
// TODO find the use of this.. probably on multi line print (* see. old version)                char LoadChar = *(m_pSubLineString[i] + j);  // why it is working... ghost ???

              #ifdef GFX_ROM_DBASE_DEF
                //DB_Central.Get(&FontInfo, GFX_FONT_INFO, 0, LoadChar);           // need the font number here
                memcpy(&m_FontDescriptor, FontInfo.pDescriptor, sizeof(FontDescriptor_t));
              #else
                //DB_Central.Get(&m_FontDescriptor, GFX_FONT_DESC_INFO, *m_pMovingUsedFontPtr, LoadChar);
              #endif
              //  *m_pMovingUsedFontPtr = m_FontDescriptor[LoadChar];  TODO fix

                m_SubLineSizePixX[i] += (/*m_FontDescriptor.LeftBearing  +*/                    // Calculate total X size
                                         m_FontDescriptor.HorizontalAdvance       /* +
                                         m_FontDescriptor.RightBearing*/);
            }
        }

        m_BoxSizeY += FontInfo.Height;                                                      // Add this line to the total
        if(i != (m_Line - 1))
        {
            m_BoxSizeY += FontInfo.Interline;                                               // Add the interline of the previous line but not on the last one
        }

        if(m_BoxSizeX < m_SubLineSizePixX[i])                                               // Adjust the box size in X if this line is longer
        {
            m_BoxSizeX = m_SubLineSizePixX[i];
        }

        m_pMovingUsedFontPtr++;
    }

  #else //  (GRAFX_USE_MULTI_LINE == DEF_ENABLED)

    //**********************************************************************
    // Parse the line
    size_t lineSizeChar = m_Size;
    size_t lineSizePixX = 0;

    for(j = 0; j < lineSizeChar; j++, m_pMovingUsedFontPtr++)
    {
        char LoadChar = m_String[j];

        if(LoadChar != ASCII_CARRIAGE_RETURN)
        {
          #ifdef GFX_ROM_DBASE_DEF
            FontInfo_t FontInfo;

            DB_Central.Get(&FontInfo, GFX_FONT_INFO, *m_pMovingUsedFontPtr, LoadChar);           // TODO validate
            memcpy(&m_FontDescriptor, FontInfo.pDescriptor, sizeof(FontDescriptor_t));
          #else
            DB_Central.Get(&m_FontDescriptor, GFX_FONT_DESC_INFO, *m_pMovingUsedFontPtr, LoadChar);
          #endif
          //  *m_pMovingUsedFontPtr = m_FontDescriptor[LoadChar];  TODO fix

            lineSizePixX += m_FontDescriptor.HorizontalAdvance;
        }
    }

    // Final box size
    m_BoxSizeX = (uint16_t)lineSizePixX;
    m_BoxSizeY = FontInfo.Height;

  #endif // (GRAFX_USE_MULTI_LINE == DEF_ENABLED)

    //**********************************************************************
    //
    // Pre-calculate global justification
    //
    m_OffsetJustX = 0;

    switch(GetXY_Justification() & _X_JUSTIFICATION)
    {
        case _X_CENTER:  m_OffsetJustX = (m_pLocalBox->Size.Width - m_BoxSizeX) >> 1;   break;
        case _X_RIGHT:   m_OffsetJustX =  m_pLocalBox->Size.Width - m_BoxSizeX;         break;
    }

    m_OffsetJustY = 0;
    switch(GetXY_Justification() & _Y_JUSTIFICATION)
    {
        case _Y_CENTER: m_OffsetJustY = (m_pLocalBox->Size.Height - m_BoxSizeY) >> 1;   break;
        case _Y_BOTTOM: m_OffsetJustY =  m_pLocalBox->Size.Height - m_BoxSizeY;         break;
    }
}

//-------------------------------------------------------------------------------------------------
//
//   Function Name: ParseFeature
//
//   Parameter(s):  None
//   Return Value:  void
//
//   Description:   Sub service function for font and color change
//
//   note:
//
//-------------------------------------------------------------------------------------------------
void GPrintf::ParseFeature(void)
{
    uint16_t i;
    bool     BackPtrCtrl;

    i = 0;
    BackPtrCtrl = false;

    m_pMovingUsedFontPtr  = m_pFontUsedInString;
    m_pMovingUsedColorPtr = m_pColorUsedInString;

    while(i < m_Size)
    {
        if(BackPtrCtrl == true)
        {
            BackPtrCtrl  = false;
        }
        else
        {
            *m_pMovingUsedFontPtr  = FontDefault.Get();
            *m_pMovingUsedColorPtr = DisplayLayer::GetTextColor();
        }

        // Color override
        if((m_String[i] == ASCII_SINGLE_COLOR_OVERRIDE) || (m_String[i] == ASCII_COLOR_OVERRIDE))
        {
            *m_pMovingUsedColorPtr = (uint32_t)LIB_6AscHex(&m_String[i + 1]) | 0xFF000000;      // Extract the color from 6 Ascii character
            if(m_String[i] == ASCII_COLOR_OVERRIDE) DisplayLayer::SetTextColor(*m_pMovingUsedColorPtr);
            strncpy(&m_String[i], &m_String[i + 7], ((m_Size - 7) + 1) - i);                    // Strip font info from string
            m_Size -= 7;                                                                        // Remove it from the size also
            BackPtrCtrl = true;
        }

        // Font override
        if((m_String[i] == ASCII_SINGLE_FONT_OVERRIDE) || (m_String[i] == ASCII_FONT_OVERRIDE))
        {
            *m_pMovingUsedFontPtr = Font_e(LIB_2AscHex(&m_String[i + 1]));                      // Extract the font from 2 Ascii character
            if(m_String[i] == ASCII_FONT_OVERRIDE) FontDefault.Set(*m_pMovingUsedFontPtr);
            strncpy(&m_String[i], &m_String[i + 3], ((m_Size - 3) + 1) - i);                    // Strip font info from string
            m_Size -= 3;                                                                        // Remove it from the size also
            BackPtrCtrl = true;
        }

        // No features found, then advance pointer
        if(BackPtrCtrl != true)
        {
            i++;
            IncrementFeaturePointer();
        }
    }
}


//-------------------------------------------------------------------------------------------------
//
//   Function Name: IncrementFeaturePointer
//
//   Parameter(s):  None
//   Return Value:  None
//
//   Description:   Increment all pointer for string feature
//
//   note:
//
//-------------------------------------------------------------------------------------------------
void GPrintf::IncrementFeaturePointer(void)
{
    m_pMovingUsedFontPtr++;
    m_pMovingUsedColorPtr++;
}


//-------------------------------------------------------------------------------------------------
#endif // DIGINI_USE_GRAFX
