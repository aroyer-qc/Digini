//-------------------------------------------------------------------------------------------------
//
//  File : lib_vt100.cpp
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2022 Alain Royer.
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

#define VT100_GLOBAL
#include "lib_digini.h"
#undef  VT100_GLOBAL

//-------------------------------------------------------------------------------------------------
// Driver Check
//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_VT100_MENU == DEF_ENABLED)

#if (USE_UART_DRIVER != DEF_ENABLED)
  #error USE_UART_DRIVER must be define DEF_ENABLED
#endif

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------


#define VT100_TERMINAL_SIZE                              256
#define VT100_REFRESH_TIME_OUT                          10
#define VT100_ESCAPE_TIME_OUT                           3
#define VT100_INPUT_INVALID_ID                          -1
#define VT100_LIMIT_DECIMAL_EDIT                        100000000   // Edition of decimal value limited to 100 Millions
#define VT100_TRAP_REAL_ESCAPE                          255

//-------------------------------------------------------------------------------------------------
// Const(s)
//-------------------------------------------------------------------------------------------------

const VT100_MenuObject_t VT100_Terminal::m_Menu[NUMBER_OF_MENU] =
{
    VT100_MENU_DEF(EXPAND_VT100_MENU_AS_MENU_DATA)
};

#define VT100_CLASS_CONSTANT
#include "vt100_var.h"         // Project variable
#undef  VT100_CLASS_CONSTANT

//-------------------------------------------------------------------------------------------------
// Private(s) Function(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//  Name:           Initialize
//
//  Parameter(s):   const char* pHeadLabel
//  Return:         const char* pDescription
//
//  Description:    Initialize VT100_terminal
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void VT100_Terminal::Initialize(const char* pHeadLabel, const char* pDescription)
{
    m_pMenu = nullptr;

    m_ForceRefresh = false;
    // Should read futur configuration for muting
    m_LogsAreMuted = false;

    InMenuPrintf(CON_SZ_NONE, VT100_LBL_RESET_TERMINAL);
    Delay = GetTick();
    while(TickHasTimeOut(Delay, 100) == false){};

//    UART_Initialize(UART_CONSOLE);
    m_pHeadLabel            = (char*)pHeadLabel;
    m_pDescription          = (char*)pDescription;
    m_RefreshMenu           = false;
    m_MenuID                = VT100_NO_MENU;
    m_FlushMenuID           = VT100_NO_MENU;
    m_InputType             = VT100_INPUT_CLI;
    m_Input                 = 0;
    m_InputCount            = 0;
    m_ValidateInput         = false;
    m_ItemsQts              = 0;
    m_BypassPrintf          = false;
    m_ID                    = -1;
    m_InputDecimalMode      = false;
    m_InEscapeSequence      = false;
    //m_IsDisplayLock         = false;
    m_BackFromEdition       = false;
    m_FlushNextEntry        = false;
    m_LogsAreMuted          = true;
    m_String[CON_STRING_SZ] = '\0';

    nOS_TimerCreate(&m_EscapeTimer, EscapeCallback, nullptr, VT100_ESCAPE_TIME_OUT, NOS_TIMER_ONE_SHOT);

    nOS_ThreadCreate(&m_ThreadHandle, m_Task, nullptr, m_Stack, CON_STACK_SIZE, CON_TASK_PRIO);
    CallbackInitialize();
//    CLI_Initialize();

    FIFO_Init(&m_FIFO_ParserRX, &m_BufferParserRX[0], CON_FIFO_PARSER_RX_SZ);
    ClearConfigFLag();
    ClearGenericString();
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Process
//
//  Parameter(s):   void
//  Return:         None
//
//  Description:    Console task
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void VT100_Terminal::Process(void)
{
    const CON_MenuDef_t*  pMenu;
    uint8_t               Items;
    uint8_t               ItemsQts;
    uint32_t              Delay;

    if((m_InputDecimalMode == false) && (m_InputStringMode == false))
    {
        // Display the menu
        if(m_RefreshMenu == true)
        {
            // Call the destructor for each callback, if any
            if((m_FlushMenuID != CON_NO_MENU) &&
               (m_FlushMenuID != CON_MenuID))
            {
                m_BackFromEdition = false;
                ClearConfigFLag();
                ClearGenericString();

                ItemsQts = CON_Menu[m_FlushMenuID].pMenuSize;

                for(Items = 0; Items < ItemsQts; Items++)
                {
                    pMenu = &CON_Menu[m_FlushMenuID].pMenu[Items];
                    CallBack(pMenu->Callback, CON_CALLBACK_FLUSH, Items);
                }
            }

            m_RefreshMenu = false;
            m_ItemsQts    = m_DisplayMenu(m_MenuID);
            m_Input       = 0;
        }

        if ( CON_ForceRefresh )
        {
            m_ItemsQts     = m_DisplayMenu(m_MenuID);
            m_Input        = 0;
            m_ForceRefresh = false;
        }

        pMenu = &m_Menu[m_MenuID].pMenu[m_Input];

        // An entry have been detected, do job accordingly
        if(m_ValidateInput == true)
        {
            m_ValidateInput = false;

            if(m_InputType == CON_INPUT_MENU_CHOICE)
            {
                // Validate the range for the menu
                if(m_Input < m_ItemsQts)
                {
                    // If new menu selection, draw this new menu
                    if(m_MenuID != pMenu->NextMenu)
                    {
                        m_FlushMenuID = m_MenuID;
                        m_MenuID      = pMenu->NextMenu;
                        m_RefreshMenu = true;
                    }

                    // If selection has a callback, call it and react to it's configuration for key input
                    m_InputType = CallBack(pMenu->Callback, CON_CALLBACK_ON_INPUT, m_Input);

                    // Job is already done, so no refresh
                    if(m_InputType == CON_INPUT_MENU_CHOICE)
                    {
                        m_Input = 0;
                    }
                }
                else
                {
                    m_Input = 0;
                }
            }

            m_InputCount = 0;
        }
        else
        {
            // Still in a callback mode
            if(m_MenuID != CON_NO_MENU)
            {
                CallBack(pMenu->Callback, CON_CALLBACK_REFRESH, 0);
            }
        }
    }
    else // Input decimal or string mode
    {
        if(m_InputDecimalMode == true)   { InputDecimal(); }
        if(m_InputStringMode  == true)   { InputString();  }
    }

}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ClearConfigFLag
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Clear the new configuration flag so the next new menu access get the fresh
//                  'save configuration'
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void VT100_Terminal::ClearConfigFLag(void)
{
    for(int i = 0; i < CONFIG_FLAG_SIZE; i++)
    {
        m_NewConfigFlag[i] = 0;
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ClearGenericString
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Clear the generic string used in configuration menu
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void VT100_Terminal::ClearGenericString(void)
{
    memset(&m_GenericString, '\0', sizeof(m_GenericString));
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DisplayMenu
//
//  Parameter(s):   CON_Menu_e      ID of the MENU
//  Return:         uint8_t         Number of items in the menu
//
//  Description:    Display selected menu
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
uint8_t VT100_Terminal::DisplayMenu(VT100_Menu_e MenuID)
{
    const char*             pLabel;
    const VT100_MenuDef_t*  pMenu;
    const VT100_MenuDef_t*  pPreviousMenu;
    uint8_t                 ItemsQts;
    uint8_t                 Items;
    char                    ItemsChar;

    pMenu = nullptr;

    if(m_MenuID != VT100_NO_MENU)
    {
        ItemsQts = m_Menu[MenuID].pMenuSize;

        if(ItemsQts > 1)
        {
            m_BypassPrintf = true;
            InMenuPrintf(CON_SZ_NONE, VT100_LBL_RESET_TERMINAL);
            nOS_Sleep(100);                                                 // Terminal need time to reset
            InMenuPrintf(CON_SZ_NONE, VT100_LBL_HIDE_CURSOR);
            InMenuPrintf(CON_SZ_NONE, VT100_LBL_CLEAR_SCREEN);
            m_pHeadLabel = (char*)LABEL_pStr[LBL_MENU_TITLE];
            SetColor(VT100_COLOR_WHITE, VT100_COLOR_BLUE);
            InMenuPrintf(CON_SZ_NONE, VT100_LBL_LINE_SEPARATOR);
            InMenuPrintf(CON_SZ_NONE, m_pHeadLabel);
            InMenuPrintf(CON_SZ_NONE, VT100_LBL_LINE_SEPARATOR);
            SetColor(VT100_COLOR_YELLOW, VT100_COLOR_BLACK);
            InMenuPrintf(CON_SZ_NONE, "\r\n\r\n");

            // Print all items in the menu list
            for(Items = 0; Items < ItemsQts; Items++)
            {
                pPreviousMenu = pMenu;
                pMenu         = &m_Menu[MenuID].pMenu[Items];

                if(Items != 0)
                {
                    ItemsChar  = (char)Items;
                    ItemsChar += (ItemsChar >= 10) ? ('a' - 10) : '0';
                    MenuSelectItems(ItemsChar);
                }

                pLabel = LABEL_pStr[pMenu->Label];
                InMenuPrintf(CON_SZ_NONE, pLabel);

                if(Items == 0)
                {
                    InMenuPrintf(CON_SZ_NONE, VT100_LBL_SELECT);
                    if(pMenu != pPreviousMenu)
                    {
                        CallBack(pMenu->Callback, VT100_CALLBACK_INIT, 0);
                    }
                }
                else
                {
                    CallBack(pMenu->Callback, VT100_CALLBACK_INIT, Items);
                }
            }

            MenuSelectItems('0');
            InMenuPrintf(CON_SZ_NONE, VT100_LBL_QUIT);
            ItemsChar  = (char)(Items - 1);
            ItemsChar += (ItemsChar >= 10) ? ('a' - 10) : '0';
            InMenuPrintf(CON_SZ_NONE, VT100_LBL_ENTER_SELECTION, ItemsChar);
            return ItemsQts;
        }

        // There is nothing to draw if it has only one item ( it is a redirection menu )
        CallBack(m_Menu[m_MenuID].pMenu[0].Callback, VT100_CALLBACK_INIT, 0);

        return 0;
    }

    InMenuPrintf(CON_SZ_NONE, VT100_LBL_CLEAR_SCREEN);
    SetForeColor(VT100_COLOR_WHITE);
    m_InputType    = VT100_INPUT_CLI;
    m_BypassPrintf = false;
    return 0;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           MenuSelectItems
//
//  Parameter(s):   ItemsChar   print (x)  where x is the character represented by ItemsChar
//  Return:         None
//
//  Description:    Menu Selection items print
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void VT100_Terminal::MenuSelectItems(char ItemsChar)
{
    InMenuPrintf(CON_SZ_NONE, "  (");
    SetForeColor(VT100_COLOR_CYAN);
    if(ItemsChar == '0') InMenuPrintf(CON_SZ_NONE, "ESC");
    else                 InMenuPrintf(CON_SZ_NONE, "%c", ItemsChar);
    SetForeColor(VT100_COLOR_YELLOW);
    if(ItemsChar == '0') InMenuPrintf(CON_SZ_NONE, ") ");
    else                 InMenuPrintf(CON_SZ_NONE, ")   ");
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CallBack
//
//  Parameter(s):   CON_InputType_e (*Callback)(uint8_t, CON_CallBackType_e)    Item menu callback                                // todo used a typedef
//                  CON_CallBackType_e              Type of callback (Tell callback what to do)
//                  uint8_t                         Item selection in the menu
//  Return:         CON_InputType_e
//
//  Description:    Call the callback
//
//  Note(s):        Push and restore the attribute
//
//-------------------------------------------------------------------------------------------------
VT100_InputType_e VT100_Terminal::CallBack(VT100_InputType_e (*CallBack)(uint8_t, VT100_CallBackType_e), VT100_CallBackType_e Type, uint8_t Item)
{
    VT100_InputType_e InputType;

    InputType = CON_INPUT_MENU_CHOICE;

    if(Callback != nullptr)
    {
        SaveAttribute();
        InputType = Callback(Item, Type);
        RestoreAttribute();
    }

    return InputType;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           EscapeCallback
//
//  Parameter(s):   pTimer              Pointer on the timer that initiate this call, Not used
//                  arg                 nullptr in this case
//  Return:         None
//
//  Description:    CallBack after we received a ESCAPE from terminal.
//                  If we enter here, then ESCAPE was sent alone.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void VT100_Terminal::EscapeCallback(nOS_Timer* pTimer, void* pArg)
{
    nOS_StatusReg sr;

    VAR_UNUSED(pTimer);
    VAR_UNUSED(pArg);

    nOS_EnterCritical(sr);
    m_InEscapeSequence = false;
    m_InputDecimalMode = false;
    m_InputStringMode  = false;
    RX_Callback(VT100_TRAP_REAL_ESCAPE);
    nOS_LeaveCritical(sr);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           InputDecimal
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Function that handle the decimal input mode.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void VT100_Terminal::InputDecimal(void)
{
    if(m_RefreshValue != m_Value)
    {
        m_RefreshValue = m_Value;

        if((m_Value >= m_Minimum) && (m_Value <= m_Maximum))
        {
            SetColor(VT100_COLOR_BLACK, VT100_COLOR_GREEN);
        }
        else
        {
            SetColor(VT100_COLOR_BLACK, VT100_COLOR_RED);
        }

        SetCursorPosition(m_PosX + 36, m_PosY + 3);

        switch(m_Divider)
        {
            case 10:   InMenuPrintf(CON_SZ_NONE, " %ld.%d \033[40m \033[2D",   m_Value / m_Divider, abs(m_Value % m_Divider)); break;
            case 100:  InMenuPrintf(CON_SZ_NONE, " %ld.%02d \033[40m \033[2D", m_Value / m_Divider, abs(m_Value % m_Divider)); break;
            case 1000: InMenuPrintf(CON_SZ_NONE, " %ld.%03d \033[40m \033[2D", m_Value / m_Divider, abs(m_Value % m_Divider)); break;
            default:   InMenuPrintf(CON_SZ_NONE, " %ld \033[40m \033[2D",      m_Value);                                       break;
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           InputString
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Function that handle the string input mode.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void VT100_Terminal::InputString(void)
{
    if(m_RefreshInputPtr != m_InputPtr)
    {
        SetCursorPosition(m_PosX + 2, m_PosY + 3);
        m_RefreshInputPtr = m_InputPtr;
        InMenuPrintf(CON_SZ_NONE, "%s \033[D", m_String);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DisplayDescription
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:     Clear the screen and Display the header line
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void VT100_Terminal::DisplayDescription(void)
{
    InMenuPrintf(CON_SZ_NONE, CON_LBL_CLEAR_SCREEN);
    SetCursorPosition(10, 1);
    InMenuPrintf(CON_SZ_NONE, "%s\r\n\r\n\r\n", CON_pDescription);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           PrintSaveLabel
//
//  Parameter(s):   uint8_t      PosX           Horizontal position on terminal
//                  uint8_t      PosY           Vertical position on terminal
//                  VT100_Color_e  Color          Color of the 'Save Configuration'
//  Return:         None
//
//  Description:    Change the color for the save configuration label
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
#ifdef CONSOLE_USE_COLOR
void VT100_Terminal::PrintSaveLabel(uint8_t PosX, uint8_t PosY, VT100_Color_e Color)
#else
void VT100_Terminal::PrintSaveLabel(uint8_t PosX, uint8_t PosY)
#endif
{
    SetForeColor(Color);
    SetCursorPosition(PosX, PosY);
    InMenuPrintf(CON_SZ_NONE, VT100_LBL_SAVE_CONFIGURATION);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SetDecimalInput
//
//  Parameter(s):   uint8_t      PosX        Position X for the edit box.
//                  uint8_t      PosY        Position Y for the edit box.
//                  int32_t      Minimum     Minimum value accepted.
//                  int32_t      Maximum     Maximum value accepted.
//                  int32_t      Value       Actual value.
//                  uint8_t      Divider     Display fraction portion.
//                  uint8_t      ID          ID of the caller.
//                  const char*  pMsg        Message to print in the box.
//  Return:         None
//
//  Description:    Prepare decimal Input value process with min/max value and an ID to tell user
//                  callback what was the request.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void VT100_Terminal::SetDecimalInput(uint8_t PosX, uint8_t PosY, int32_t Minimum, int32_t Maximum, int32_t Value, uint16_t Divider, uint8_t ID, const char* pMsg)
{
    m_Minimum          = Minimum;
    m_Maximum          = Maximum;
    m_PosX             = PosX;
    m_PosY             = PosY;
    m_ID               = (int16_t)ID;
    m_Value            = Value;
    m_RefreshValue     = Value + 1;       // To force the first refresh
    m_OldValue         = Value;
    m_Divider          = Divider;
    m_InputDecimalMode = true;
    m_IsItString       = false;

    // Draw the Box
    DrawBox(PosX, PosY, 48, 8, VT100_COLOR_WHITE);

    // Write input information
    SetForeColor(VT100_COLOR_CYAN);
    SetCursorPosition(PosX + 2,  PosY + 1);

    switch(Divider)
    {
        case 10:  InMenuPrintf(CON_SZ_NONE, "%s : %ld.%d",   VT100_LBL_MINIMUM, Minimum / Divider,  abs(Minimum % Divider)); break;
        case 100: InMenuPrintf(CON_SZ_NONE, "%s : %ld.%02d", VT100_LBL_MINIMUM, Minimum / Divider,  abs(Minimum % Divider)); break;
        case 1000:InMenuPrintf(CON_SZ_NONE, "%s : %ld.%03d", VT100_LBL_MINIMUM, Minimum / Divider,  abs(Minimum % Divider)); break;
        default:  InMenuPrintf(CON_SZ_NONE, "%s : %ld",      VT100_LBL_MINIMUM, Minimum);                                    break;
    }

    SetCursorPosition(PosX + 24, PosY + 1);

    switch(Divider)
    {
        case 10:  InMenuPrintf(CON_SZ_NONE, "%s : %ld.%d",   VT100_LBL_MAXIMUM, Maximum / Divider,  abs(Maximum % Divider)); break;
        case 100: InMenuPrintf(CON_SZ_NONE, "%s : %ld.%02d", VT100_LBL_MAXIMUM, Maximum / Divider,  abs(Maximum % Divider)); break;
        case 1000:InMenuPrintf(CON_SZ_NONE, "%s : %ld.%03d", VT100_LBL_MAXIMUM, Maximum / Divider,  abs(Maximum % Divider)); break;
        default:  InMenuPrintf(CON_SZ_NONE, "%s : %ld",      VT100_LBL_MAXIMUM, Maximum);                                    break;
    }

    // Print type of input
    SetForeColor(VT100_COLOR_YELLOW);
    SetCursorPosition(PosX + 2, PosY + 3);
    InMenuPrintf(CON_SZ_NONE, "%s", pMsg);

    // Add 'how to' info
    SetCursorPosition(PosX + 2,  PosY + 5);
    InMenuPrintf(CON_SZ_NONE, VT100_LBL_INPUT_VALIDATION);

    // And show cursor
    InMenuPrintf(CON_SZ_NONE, VT100_LBL_SHOW_CURSOR);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           GetDecimalInputValue
//
//  Parameter(s):   uint32_t*    pValue      Original value.
//                  uint8_t*     pID         ID to return after edition.
//  Return:         None
//
//  Description:    Get the value entered while in input decimal mode.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void VT100_Terminal::GetDecimalInputValue(uint32_t* pValue, uint8_t* pID)
{
    uint8_t  ID    = 0;
    uint32_t Value = m_OldValue;

    if(m_ID != VT100_INPUT_INVALID_ID)
    {
        if(m_IsItString == false)
        {
            if((m_Value >= m_Minimum) && (m_Value <= m_Maximum))
            {
                ID    = m_ID;
                Value = m_Value;
            }
            m_InputDecimalMode = false;
            m_ID = VT100_INPUT_INVALID_ID;   // We can read this only once
        }
    }

    *pID    = (uint8_t)ID;
    *pValue = Value;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SetStringInput
//
//  Parameter(s):   uint8_t     PosX        Position X for the edit box.
//                  uint8_t     PosY        Position Y for the edit box.
//                  int32_t     Maximum     Maximum number of character in the string.
//                  uint8_t     ID          ID of the caller.
//                  const char* pMsg        Message to print in the box.
//                  const char* pString     Actual string to edit.
//  Return:         None
//
//  Description:    Prepare Input string process with max size and an ID to tell user callback what
//                  was the request.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void VT100_Terminal::SetStringInput(uint8_t PosX, uint8_t PosY, int32_t Maximum, uint8_t ID, const char* pMsg, const char* pString)
{
    m_Maximum          = Maximum;
    m_PosX             = PosX;
    m_PosY             = PosY;
    m_ID               = (int16_t)ID;
    m_InputStringMode  = true;
    m_IsItString       = true;

    // Draw the Box
    DrawBox(PosX, PosY, 46, 7, VT100_COLOR_WHITE);

    // Write input information
    SetForeColor(VT100_COLOR_CYAN);
    SetCursorPosition(PosX + 2,  PosY + 1);
    InMenuPrintf(Maximum, "%s", pMsg);

    // Add 'how to' info
    SetCursorPosition(PosX + 2,  PosY + 5);
    InMenuPrintf(CON_SZ_NONE, VT100_LBL_INPUT_VALIDATION);

    // Copy string
    m_memcpy(m_String, pString, VT100_STRING_SZ);
    STR_strnstrip(m_String, Maximum);
    m_InputPtr = strlen(m_String);                          // Get string end pointer
    m_RefreshInputPtr = m_InputPtr + 1;                     // To force a refresh

    // And show cursor
    InMenuPrintf(CON_SZ_NONE, VT100_LBL_SHOW_CURSOR);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           GetStringInput
//
//  Parameter(s):   uint32_t*   pString     Original string.
//                  uint8_t*    pID         ID to return after edition.
//  Return:         None
//
//  Description:    Get the string entered while in input string mode.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void VT100_Terminal::GetStringInput(char* pString, uint8_t* pID)
{
    uint8_t ID;

    ID = 0;

    if(m_ID != VT100_INPUT_INVALID_ID)
    {
        if(m_IsItString == true)
        {
            if(strlen(m_String) <= m_Maximum)
            {
                ID = m_ID;
                memcpy(pString, m_String, CON_STRING_SZ);
            }
            m_InputStringMode = false;
            m_ID = VT100_INPUT_INVALID_ID;             // We can read this only once
        }
    }

    *pID = (uint8_t)ID;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           InMenuPrintf
//
//  Parameter(s):   int         nSize       Max size of the string to print.
//                  const char* pFormat     Formatted string.
//                  ...                     Parameter if any.
//
//  Return:         None
//
//  Description:    Send formatted string to console if menu system is active.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
size_t VT100_Terminal::InMenuPrintf(int nSize, const char* pFormat, ...)
{
    va_list vaArg;
    char*   Buffer;
    size_t  Size    = 0;

    if((pBuffer = nOS_MemAlloc(&BSP_MemBlock, NOS_NO_WAIT) != nullptr)
    {
        va_start(vaArg, (const char*)pFormat);
        Size = STR_vsnprintf(pBuffer, ((nSize == CON_SZ_NONE) ? VT100_TERMINAL_SIZE : nSize), pFormat, vaArg);
        UART_Write(UART_CONSOLE, &Buffer[0], Size);
        va_end(vaArg);
        nOS_MemFree(&BSP_MemBlock, (void*)Buffer);
    }

    return Size;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           LoggingPrintf
//
//  Parameter(s):   CON_DebugLevel_e    Level       Level of printf logging.
//                  const char*         pFormat     Formatted string.
//                  ...                             Parameter if any.
//
//  Return:         size_t              Number of caracter printed.
//
//  Description:    Send formatted string to console if menu system is active.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
size_t VT100_Terminal::LoggingPrintf(CON_DebugLevel_e Level, const char* pFormat, ...)
{
    va_list          vaArg;
    char*            pBuffer;
    CON_DebugLevel_e DebugLevel;
    size_t           Size = 0;

    if((m_BypassPrintf == false) && (m_LogsAreMuted == false))
    {
        //// SYS_Read(SYS_DEBUG_LEVEL, MAIN_ACU, 0, &DebugLevel, nullptr);
        if((DebugLevel & Level) != 0)
        {
            if((pBuffer = nOS_MemAlloc(&BSP_MemBlock, NOS_NO_WAIT) != nullptr)
            {
                va_start(vaArg, (const char*)pFormat);
                Size = STR_vsnprintf(pBuffer, VT100_TERMINAL_SIZE, pFormat, vaArg);
                UART_Write(UART_CONSOLE, pBuffer, Size);
                UART_Write(UART_CONSOLE, "\r\n", 2);
                va_end(vaArg);
                nOS_MemFree(&BSP_MemBlock, (void*)pBuffer);
            }
        }
    }

    return Size;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           GoToMenu
//
//  Parameter(s):   CON_Menu_e  MenuID              Menu to go to.
//
//  Return:         None
//
//  Description:    Select a new menu to go.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void VT100_Terminal::GoToMenu(CON_Menu_e MenuID)
{
    m_RefreshMenu = true;
    m_FlushMenuID = m_MenuID;
    m_MenuID      = MenuID;
    m_InputType   = VT100_INPUT_MENU_CHOICE;
    m_Input       = 0;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ConsoleState
//
//  Parameter(s):   CON_Menu_e  MenuID              Menu to go to.
//
//  Return:         bool        - true        if console is active
//                              - false       if only in dump log mode
//
//  Description:    Return console state.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
bool VT100_Terminal::ConsoleState(void)
{
    return (m_MenuID == VT100_NO_MENU) ? false : true;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SetConsoleMuteLogs
//
//  Parameter(s):   bool        Mute        - if true, console is muted
//                                          - if false, console unmuted
//
//  Return:         None
//
//  Description:    Set the mute flag for the console logs.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void VT100_Terminal::SetConsoleMuteLogs(bool Mute)
{
    m_LogsAreMuted = Mute;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SetAttribute
//
//  Parameter(s):   CON_VT100_Attribute_e Attribute     See enum for list of attribute that can be
//                                                      set.
//
//  Return:         None
//
//  Description:    Set the color font and background.
//
//  Note(s):        Some attribute may not be supported by all terminal program.
//
//-------------------------------------------------------------------------------------------------
void VT100_Terminal::SetAttribute(VT100_Attribute_e Attribute)
{
    if(m_IsDisplayLock == false)
    {
        InMenuPrintf(CON_SZ_NONE, VT100_LBL_ATTRIBUTE, Attribute);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SaveAttribute
//
//  Parameter(s):   None
//
//  Return:         None
//
//  Description:    Set the attribute.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void VT100_Terminal::SaveAttribute(void)
{
    if(m_IsDisplayLock == false)
    {
        InMenuPrintf(CON_SZ_NONE, VT100_LBL_SAVE_ATTRIBUTE);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           RestoreAttribute
//
//  Parameter(s):   None
//
//  Return:         None
//
//  Description:    Restore the VT-100 attribute.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void VT100_Terminal::RestoreAttribute(void)
{
    if(m_IsDisplayLock == false)
    {
        InMenuPrintf(CON_SZ_NONE, VT100_LBL_RESTORE_ATTRIBUTE);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SetCursorPosition
//
//  Parameter(s):   uint8_t     PosX        Cursor position X.
//                  uint8_t     PosY        Cursor Position Y.
//
//  Return:         None
//
//  Description:    Set the position of the cursor.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void VT100_Terminal::SetCursorPosition(uint8_t PosX, uint8_t PosY)
{
    if(m_IsDisplayLock == false)
    {
        InMenuPrintf(CON_SZ_NONE, VT100_LBL_SET_CURSOR, PosY, PosX);
    }
}


//-------------------------------------------------------------------------------------------------
//
//  Name:           SaveCursorPosition
//
//  Parameter(s):   None
//
//  Return:         None
//
//  Description:    Save the VT-100 cursor position.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void VT100_Terminal::SaveCursorPosition(void)
{
    if(m_IsDisplayLock == false)
    {
        InMenuPrintf(CON_SZ_NONE, VT100_LBL_SAVE_CURSOR);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           RestoreCursorPosition
//
//  Parameter(s):   None
//
//  Return:         None
//
//  Description:    Restore the VT-100 cursor position.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void VT100_Terminal::RestoreCursorPosition(void)
{
    if(m_IsDisplayLock == false)
    {
        InMenuPrintf(CON_SZ_NONE, VT100_LBL_RESTORE_CURSOR);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SetScrollZone
//
//  Parameter(s):   uint8_t     FirstLine       First line of the scroll region
//                  uint8_t     LastLine        Last line of the scroll region
//
//  Return:         None
//
//  Description:    Set a new scroll zone for the terminal.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void VT100_Terminal::SetScrollZone(uint8_t FirstLine, uint8_t LastLine)
{
    if(m_IsDisplayLock == false)
    {
        InMenuPrintf(CON_SZ_NONE, VT100_LBL_SCROLL_ZONE, FirstLine, LastLine);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           SetColor
//
//  Parameter(s):   VT100_Color_e     ForeColor       Set foreground color.
//                  VT100_Color_e     BackColor       Set Background color.
//
//  Return:         None
//
//  Description:    Set the color for foreground and background.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
#ifdef CONSOLE_USE_COLOR
void VT100_Terminal::SetColor(VT100_Color_e ForeColor, VT100_Color_e BackColor)
{
    if(m_IsDisplayLock == false)
    {
        SetAttribute((VT100_Attribute_e)ForeColor + CON_OFFSET_COLOR_FOREGROUND);
        SetAttribute((VT100_Attribute_e)BackColor + CON_OFFSET_COLOR_BACKGROUND);
    }
}
#endif

//-------------------------------------------------------------------------------------------------
//
//  Name:           InvertMono
//
//  Parameter(s):   bool            Invert    if true, black on white.
//                                            if false, normal white on white.
//
//  Return:         None
//
//  Description:    Invert monochrome Black <-> White versus foreground and background.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
#ifndef CONSOLE_USE_COLOR
void VT100_Terminal::InvertMono(bool Invert)
{
    if(Invert == true) InMenuPrintf(CON_SZ_NONE, "\033[30m\033[47m");
    else               InMenuPrintf(CON_SZ_NONE, "\033[37m\033[40m");
}
#endif

//-------------------------------------------------------------------------------------------------
//
//  Name:           LockDisplay
//
//  Parameter(s):   bool       State        if true, Display is locked.
//                                          if false, Display active.
//
//  Return:         None
//
//  Description:    Prevent console from updating display.
//
//  Note(s):        It is the responsibility the user to release the lock, when exiting a console
//                  callback. Useful for logging to file, and prevent garbage to be visible in the
//                  file. also send the print stop label before exiting callback that use this
//                  feature.
//
//-------------------------------------------------------------------------------------------------
void VT100_Terminal::LockDisplay(bool State)
{
    m_IsDisplayLock = State;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CON_DisplayTimeDateStamp
//
//  Parameter(s):   pTimeDate       Pointer on the TimeDate Structure
//
//  Return:         None
//
//  Description:    Display time and date stamp on terminal.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void VT100_Terminal::DisplayTimeDateStamp(nOS_TimeDate* pTimeDate)
{
    InMenuPrintf(CON_SZ_NONE, VT100_LBL_TIME_DATE_STAMP, pTimeDate->year,
                                                         pTimeDate->month,
                                                         pTimeDate->day,
                                                         pTimeDate->hour,
                                                         pTimeDate->minute,
                                                         pTimeDate->second);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DrawBox
//
//  Parameter(s):   uint8_t         PosX                Position x on screen.
//                  uint8_t         PosY                Position y on screen.
//                  uint8_t         H_Size              horizontal size in of the box.
//                  uint8_t         V_Size              vertical size of the box.
//                  VT100_Color_e   ForeColor           Color of the box.
//
//  Return:         None
//
//  Description:    Draw a box at specified attribute.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void VT100_Terminal::DrawBox(uint8_t PosX, uint8_t PosY, uint8_t H_Size, uint8_t V_Size, VT100_Color_e ForeColor)
{
    if(m_IsDisplayLock == false)
    {
        SetForeColor(ForeColor);
        SetCursorPosition(PosX, PosY++);
        InMenuPrintf(CON_SZ_NONE, "╔");

        for(uint8_t i = 0; i < (H_Size - 2); i++)
        {
            InMenuPrintf(CON_SZ_NONE, "�?");
        }

        InMenuPrintf(CON_SZ_NONE, "╗");

        for(uint8_t i = 0; i < (V_Size - 2); i++)
        {
            SetCursorPosition(PosX, PosY);

            InMenuPrintf(CON_SZ_NONE, "║");

            // Erase inside
            for(uint8_t j = 0; j < (H_Size - 2); j++)
            {
                InMenuPrintf(CON_SZ_NONE, " ");
            }

            SetCursorPosition(PosX + (H_Size - 1), PosY++);
            InMenuPrintf(CON_SZ_NONE, "║");
        }

        SetCursorPosition(PosX, PosY);
        InMenuPrintf(CON_SZ_NONE, "╚");

        for(uint8_t i = 0; i < (H_Size - 2); i++)
        {
            InMenuPrintf(CON_SZ_NONE, "�?");
        }
        InMenuPrintf(CON_SZ_NONE, "�?");
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DrawVline
//
//  Parameter(s):   uint8_t         PosX                Position X on screen.
//                  uint8_t         PosY                Position Y on screen.
//                  uint8_t         V_Size              vertical size of line.
//                  VT100_Color_e   ForeColor           Color of the box.
//
//  Return:         None
//
//  Description:    Draw a line at specified attribute.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void VT100_Terminal::DrawVline(uint8_t PosX, uint8_t PosY, uint8_t V_Size, VT100_Color_e ForeColor)
{
    if(m_IsDisplayLock == false)
    {
        SetForeColor(ForeColor);

        for(uint8_t i = 0; i < V_Size; i++)
        {
            SetCursorPosition(PosX, PosY++);
            InMenuPrintf(CON_SZ_NONE, "║");
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Bargraph
//
//  Parameter(s):   uint8_t         PosX        X Position on screen.
//                  uint8_t         PosY        Y Position On screen.
//                   VT100_Color_e  Color       Color of the bargraph.
//                  uint8_t         Value       Actual value.
//                  uint8_t         Max         Maximum value.
//                  uint8_t         Size        Size in character.
//
//
//  Return:         None
//
//  Description:    Print the bargraph according to value and maximum at specified location.
//
//  Note(s):        ** This parameter exist only if CONSOLE_USE_COLOR is defined
//
//-------------------------------------------------------------------------------------------------
#ifdef CONSOLE_USE_COLOR
void VT100_Terminal::Bargraph(uint8_t PosX, uint8_t PosY, VT100_Color_e Color, uint8_t Value, uint8_t Max, uint8_t Size)
#else
void VT100_Terminal::Bargraph(uint8_t PosX, uint8_t PosY, uint8_t Value, uint8_t Max, uint8_t Size)
#endif
{
    int i;

    SetCursorPosition(PosX, PosY);

    for(i = 0; i < Size; i++)
    {
      #ifdef CONSOLE_USE_COLOR
        if(i < ((Value / (Max / Size))))
        {
            SetBackColor(Color);
        }
        else
        {
            SetBackColor(VT100_COLOR_BLACK);
        }
      #else
        InvertMono((i < ((Value / (Max / Size)))));
      #endif

        InMenuPrintf(CON_SZ_NONE, " ");
    }

  #ifdef CONSOLE_USE_COLOR
    SetAttribute(VT100_COLOR_BLACK + VT100_OFFSET_COLOR_BACKGROUND);
  #else
    InvertMono(false);
  #endif
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CLI_CmdCONSOLE
//
//  Parameter(s):   None
//
//  Return:         CON_AT_Error_e
//
//  Description:    AT Command to enter menu mode.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
#ifdef CONSOLE_USE_VT100_MENU
CON_AT_Error_eVT100_Terminal::CLI_CmdCONSOLE(void)
{
    GoToMenu((CON_MenuID == CON_NO_MENU) ? VT100_MENU_MAIN : m_MenuID);
    return CON_AT_OK_SILENT;
}
#endif

//-------------------------------------------------------------------------------------------------
//
//  Name:           GetString
//
//  Parameter(s):   char*   pBuffer     Pointer on the buffer to put the string
//                  size_t  Size        MAX size of the buffer
//
//  Return:         bool
//
//  Description:    Retrieve the string in the FIFO buffer. Must be located in quote, and inside
//                  the provided range
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
bool VT100_Terminal::GetString(char* pBuffer, size_t Size)
{
    char Character;

    if(FIFO_At(&CON_FIFO_ParserRX, 0) == '"')
    {
        FIFO_Flush(&CON_FIFO_ParserRX, 1);

        for(int i = 0; i <= Size; i++)
        {
            if(FIFO_Read(&CON_FIFO_ParserRX, &Character, 1) == 1)
            {
                if(Character == '"')
                {
                    if(i < Size)
                    {
                        *pBuffer++ = '\0';
                    }
                    return true;
                }

                *pBuffer++ = Character;
            }
        }
    }

    return false;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           RX_Callback
//
//  Parameter(s):   uint8_t     Data        First byte received
//
//  Return:         None
//
//  Description:    Here we received the character from the terminal.
//
//  Note(s):        This is a state machine to handle incoming caracter, according to state on menu
//                  or AT Sequence or input (string or decical/hexadecimal).
//
//-------------------------------------------------------------------------------------------------
void VT100_Terminal::RX_Callback(uint8_t Data)
{
    if(m_FlushNextEntry == true)
    {
        m_FlushNextEntry = false;
        return;
    }

    if(m_InEscapeSequence == true)
    {
        m_InEscapeSequence = false;
        nOS_TimerStop(&m_EscapeTimer, true);
        m_ValidateInput  = false;
        m_Input          = 0;
        m_FlushNextEntry = true;
    }

    if(Data == CON_CHAR_ESCAPE)
    {
        nOS_TimerRestart(&m_EscapeTimer, CON_ESCAPE_TIME_OUT);
        m_InEscapeSequence = true;
        m_Value            = m_OldValue;
        return;
    }

    switch(m_InputType)
    {
        case CON_INPUT_MENU_CHOICE:
        {
            m_InputCount++;
            m_ValidateInput = true;

            if((Data >= 'a') && (Data <= 'z'))
            {
                Data = (Data - 'a') + 10;
            }
            else if((Data >= 'A') && (Data <= 'Z'))
            {
                Data = (Data - 'A') + 10;
            }
            else if((Data >= '1') && (Data <= '9'))
            {
                Data = Data - '0';
            }
            else if(Data != TRAP_REAL_ESCAPE)
            {
                m_Input         = 0;
                m_InputCount    = 0;
                m_ValidateInput = false;
                return;
            }
            if(Data == TRAP_REAL_ESCAPE)
            {
                Data = 0;
            }
            m_Input = Data;
            break;
        }

        case CON_INPUT_DECIMAL:
        {
            if(Data == ASCII_CARRIAGE_RETURN)
            {
                m_BackFromEdition  = true;
                m_InputDecimalMode = false;
                GoToMenu(CON_MenuID);
            }
            else if(Data == ASCII_BACKSPACE)
            {
                m_Value /= 10;
            }
            else if((Data >= '0') && (Data <= '9'))
            {
                if(((abs(m_Value * 10)) + (Data - '0')) <= CON_LIMIT_DECIMAL_EDIT)
                {
                    m_InputCount++;
                    m_Value *= 10;
                    if(m_Value < 0) m_Value -= Data - '0';
                    else              m_Value += Data - '0';
                }
            }
            else if(((Data == '-') && (m_Value > 0)) ||
                    ((Data == '+') && (m_Value < 0)))
            {
               m_Value = -m_Value;
            }
            else if(Data == TRAP_REAL_ESCAPE)
            {
                m_ID = CON_INPUT_INVALID_ID;
                GoToMenu(CON_MenuID);
            }

            break;
        }

        case CON_INPUT_STRING:
        {
            if(Data == ASCII_CARRIAGE_RETURN)
            {
                m_BackFromEdition = true;
                m_InputStringMode = false;
                GoToMenu(CON_MenuID);
            }
            else if(Data == ASCII_BACKSPACE)
            {
                if((m_InputPtr >= 0) && (m_InputPtr <= m_Maximum))
                {
                    if(m_InputPtr > 0)
                    {
                        m_InputPtr--;
                        m_String[CON_InputPtr] = '\0';
                    }
                }
            }
            else if((Data >= ' ') && (Data <= '~'))
            {
                if(m_InputPtr < m_Maximum)
                {
                    m_String[CON_InputPtr] = Data;
                    m_InputPtr++;
                }
            }
            else if(Data == TRAP_REAL_ESCAPE)
            {
                m_ID = CON_INPUT_INVALID_ID;
                m_GoToMenu(CON_MenuID);
            }

            break;
        }

        case CON_INPUT_ESCAPE:
        {
            if(Data == TRAP_REAL_ESCAPE)
            {
                GoToMenu(CON_MenuID);
            }
            break;
        }

        case CON_INPUT_CLI:
        {
            ParseFIFO(Data);
            break;
        }

        case CON_INPUT_ESCAPE_TO_CONTINUE:
        {
            if(Data == TRAP_REAL_ESCAPE)
            {
                m_ValidateInput = true;
                m_InputType     = CON_INPUT_MENU_CHOICE;
            }
            break;
        }

        default: break;
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ForceMenuRefresh
//
//  Parameter(s):   None
//
//  Return:         None
//
//  Description:    This function force a refresh of the menu.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void VT100_Terminal::ForceMenuRefresh(void)
{
    m_ForceRefresh = true;
}

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_VT100_MENU == DEF_ENABLED)
