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

#if (USE_UART_DRIVER != DEF_ENABLED)  // and console
  #error USE_UART_DRIVER must be define DEF_ENABLED
#endif

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define VT100_FIFO_PARSER_RX_SZ                         1000
#define VT100_TERMINAL_SIZE                             256
#define VT100_REFRESH_TIME_OUT                          10
#define VT100_ESCAPE_TIME_OUT                           3
#define VT100_INPUT_INVALID_ID                          -1
#define VT100_LIMIT_DECIMAL_EDIT                        100000000   // Edition of decimal value limited to 100 Millions
#define VT100_TRAP_REAL_ESCAPE                          255

#define VT100_STARTUP_MENU_ID_CFG                       CAT(VT100_STARTUP_MENU_CFG, _ID)

//-------------------------------------------------------------------------------------------------
// Const(s)
//-------------------------------------------------------------------------------------------------

VT100_MENU_DEF(EXPAND_VT100_AS_MENU_MEMBER_VARIABLE_DATA)

const VT100_MenuObject_t VT100_Terminal::m_Menu[NUMBER_OF_MENU] =
{
    VT100_MENU_DEF(EXPAND_VT100_MENU_AS_DATA)
};

inline bool ConvertToValue(uint8_t* pData);

bool ConvertToValue(uint8_t* pData)
{
    if((*pData >= 'a') && (*pData <= 'z'))
    {
        *pData = (*pData - 'a') + 10;
    }
    else if((*pData >= 'A') && (*pData <= 'Z'))
    {
        *pData = (*pData - 'A') + 10;
    }
    else if((*pData >= '1') && (*pData <= '9'))
    {
        *pData -= '0';
    }
    else
    {
        return false;
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Initialize
//
//  Parameter(s):   Console*    pConsole                   Pointer on the console class object
//  Return:         const char* pHeader
//
//  Description:    Initialize VT100_terminal
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
nOS_Error VT100_Terminal::Initialize(Console* pConsole, const char* pHeader)
{
    nOS_Error Error = NOS_OK;

    // Should read future configuration for muting
    m_pConsole                = pConsole;
    m_IsItInitialized         = false;
    m_InputCount              = 0;
    m_MenuID                  = VT100_MENU_NONE;
    m_ItemsQts                = 0;
    m_BypassPrintf            = false;
    m_ID                      = -1;
    m_InputDecimalMode        = false;
    m_InEscapeSequence        = false;
    m_IsDisplayLock           = false;
    m_BackFromEdition         = false;
    m_FlushNextEntry          = false;
    m_ValidateInput           = false;
    m_ForceRefresh            = false;
    m_LogsAreMuted            = true;

    m_SetMenuCursorPosX       = 0;
    m_SetMenuCursorPosY       = 0;

    Error = nOS_TimerCreate(&m_EscapeTimer, EscapeCallback, this, VT100_ESCAPE_TIME_OUT, NOS_TIMER_ONE_SHOT);
  #if (VT100_USER_CALLBACK_INITIALIZE == DEF_ENABLED)
    CallbackInitialize();                       // User callback specific initialization
  #endif
    ClearConfigFLag();
    ClearGenericString();
    return Error;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Process
//
//  Parameter(s):   void
//  Return:         None
//
//  Description:    Console process
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void VT100_Terminal::IF_Process(void)
{
    const VT100_MenuDef_t* pMenu;
    uint8_t                Items;
   // uint8_t                ItemsQts;
    TickCount_t            Delay;

    ProcessRX();

    // Send the reset terminal string
    if(m_IsItInitialized == false)
    {
        m_IsItInitialized = true;
        InMenuPrintf(VT100_SZ_NONE, VT100_LBL_RESET_TERMINAL);
        Delay = GetTick();
        while(TickHasTimeOut(Delay, 10) == false){};
      #if (VT100_USE_STANDARD_MENU_STATIC_INFO == DEF_ENABLED)
        PrintMenuStaticInfo();
      #endif
      #if (VT100_USE_USER_MENU_STATIC_INFO == DEF_ENABLED)
        PrintUserMenuStaticInfo();
      #endif
        GoToMenu(VT100_STARTUP_MENU_ID_CFG);
    }
    else if(m_InputDecimalMode == true)     {  InputDecimal();  }
    else if(m_InputStringMode  == true)     {  InputString();   }
    else  // Display the menu and process callback
    {
        if(m_ForceRefresh == true)
        {
            m_ForceRefresh = false;
            DisplayMenu(m_MenuID);
        }

        pMenu = &m_Menu[m_MenuID].pDefinition[m_Input];

        // An entry have been detected, do job accordingly
        if(m_ValidateInput == true)
        {
            m_ValidateInput = false;

            if(m_InputType == VT100_INPUT_MENU_CHOICE)
            {
                // Validate the range for the menu
                if(m_Input < m_ItemsQts)
                {
                    // If new menu selection, draw this new menu
                    if(m_MenuID != pMenu->NextMenu)
                    {
                        m_FlushMenuID = m_MenuID;
                        m_MenuID      = pMenu->NextMenu;

                        // Call the destructor for each callback, if any
                        if((m_FlushMenuID != VT100_MENU_NONE) &&
                           (m_FlushMenuID != m_MenuID))
                        {
                            m_BackFromEdition = false;

                            for(Items = 0; Items < m_Menu[m_FlushMenuID].Size; Items++)
                            {
                                pMenu = &m_Menu[m_FlushMenuID].pDefinition[Items];
                                CallBack(pMenu->pCallback, VT100_CALLBACK_FLUSH, Items);
                            }

                            ClearConfigFLag();
                            ClearGenericString();
                            GoToMenu(m_MenuID);
                        }
                    }
                    else
                    {
                        // If selection has a callback, call it and react to it's configuration for key input
                        CallbackMethod_t pCallback = m_Menu[m_MenuID].pDefinition[m_Input].pCallback;
                        m_InputType = CallBack(pCallback, VT100_CALLBACK_ON_INPUT, m_Input);
                    }

                    // Job is already done, so no refresh
                    if(m_InputType == VT100_INPUT_MENU_CHOICE)
                    {
                        ClearInputMenuSelection();
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
            if(m_MenuID != VT100_MENU_NONE)
            {
                CallBack(pMenu->pCallback, VT100_CALLBACK_REFRESH, 0);
            }
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ProcessRX
//
//  Parameter(s):   None
//
//  Return:         None
//
//  Description:    Here we read the character from the console.
//
//  Note(s):        This is a state machine to handle incoming character.
//
//
//
//-------------------------------------------------------------------------------------------------
void VT100_Terminal::ProcessRX(void)
{
    uint8_t Data;

    if(m_pConsole->ReadyRead() == true)
    {
        m_pConsole->Read(&Data, 1);

        // We flush this entry as it follow escape sequence... don't know why !!!
        if(m_FlushNextEntry == true)
        {
            m_FlushNextEntry = false;
            return;
        }

        if(Data == ASCII_ESCAPE)
        {
            nOS_TimerRestart(&m_EscapeTimer, VT100_ESCAPE_TIME_OUT);
            m_InEscapeSequence = true;
            m_Value            = m_OldValue;
            return;
        }

        if(m_InEscapeSequence == true)
        {
            nOS_TimerStop(&m_EscapeTimer, true);
            m_InEscapeSequence = false;
            m_ValidateInput    = false;
            m_Input            = 0;
            m_FlushNextEntry   = true;
        }

        switch(m_InputType)
        {
            case VT100_INPUT_MENU_CHOICE:
            {
                m_InputCount++;
                m_ValidateInput = true;

                if(ConvertToValue(&Data) != true)
                {
                    if(Data == VT100_TRAP_REAL_ESCAPE)
                    {
                        Data = 0;
                    }
                    else
                    {
                        m_Input         = 0;
                        m_InputCount    = 0;
                        m_ValidateInput = false;
                        return;
                    }
                }

                m_Input = Data;
            }
            break;

            case VT100_INPUT_DECIMAL:
            {
                if(Data == ASCII_CARRIAGE_RETURN)
                {
                    m_BackFromEdition  = true;
                    m_InputDecimalMode = false;
                    GoToMenu(m_MenuID);
                }
                else if(Data == ASCII_BACKSPACE)
                {
                    m_Value /= 10;
                }
                else if((Data >= '0') && (Data <= '9'))
                {
                    if(((abs(m_Value * 10)) + (Data - '0')) <= VT100_LIMIT_DECIMAL_EDIT)
                    {
                        m_InputCount++;
                        m_Value *= 10;
                        if(m_Value < 0) m_Value -= Data - '0';
                        else            m_Value += Data - '0';
                    }
                }
                else if(((Data == '-') && (m_Value > 0)) ||
                        ((Data == '+') && (m_Value < 0)))
                {
                   m_Value = -m_Value;
                }
                else if(Data == VT100_TRAP_REAL_ESCAPE)
                {
                    m_ID = VT100_INPUT_INVALID_ID;
                    GoToMenu(m_MenuID);  //?? pas sure
                }
            }
            break;

            case VT100_INPUT_STRING:
            {
                if(Data == ASCII_CARRIAGE_RETURN)
                {
                    m_BackFromEdition = true;
                    m_InputStringMode = false;
                    GoToMenu(m_MenuID);
                }
                else if(Data == ASCII_BACKSPACE)
                {
                    if((m_InputPtr >= 0) && (m_InputPtr <= m_Maximum))
                    {
                        if(m_InputPtr > 0)
                        {
                            m_InputPtr--;
                            m_pString[m_InputPtr] = '\0';
                        }
                    }
                }
                else if((Data >= ' ') && (Data <= '~'))
                {
                    if(m_InputPtr < m_Maximum)
                    {
                        m_pString[m_InputPtr] = Data;
                        m_InputPtr++;
                    }
                }
                else if(Data == VT100_TRAP_REAL_ESCAPE)
                {
                    m_ID = VT100_INPUT_INVALID_ID;
                    GoToMenu(m_MenuID);
                }
            }
            break;

            case VT100_INPUT_ESCAPE:
            {
                if(Data == VT100_TRAP_REAL_ESCAPE)
                {
                    GoToMenu(m_MenuID);
                }
            }
            break;

            case VT100_INPUT_ESCAPE_TO_CONTINUE:
            {
                if(Data == VT100_TRAP_REAL_ESCAPE)
                {
                    m_ValidateInput = true;
                    m_InputType     = VT100_INPUT_MENU_CHOICE;
                }
            }
            break;

            default: break;
        }
    }
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
void VT100_Terminal::GoToMenu(VT100_Menu_e MenuID)
{
    m_FlushMenuID = m_MenuID;
    m_MenuID      = MenuID;
    m_InputType   = VT100_INPUT_MENU_CHOICE;
    m_Input       = 0;
    m_ItemsQts    = DisplayMenu(m_MenuID);
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
//  Parameter(s):   VT100_Menu_e    ID of the MENU
//  Return:         None
//
//  Description:    Display selected menu
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
uint8_t VT100_Terminal::DisplayMenu(VT100_Menu_e MenuID)
{
    const VT100_MenuDef_t*  pMenu;
    const VT100_MenuDef_t*  pPreviousMenu;
    uint8_t                 ItemsQts;
    uint8_t                 Items;
    char                    ItemsChar;

    pMenu = nullptr;

    if(m_MenuID != VT100_MENU_NONE)
    {
        SetCursorPosition(m_SetMenuCursorPosX, m_SetMenuCursorPosY);

        ItemsQts = m_Menu[MenuID].Size;

        if(ItemsQts > 1)
        {
            // Print all items in the menu list
            for(Items = 0; Items < ItemsQts; Items++)
            {
                pPreviousMenu = pMenu;
                pMenu         = &m_Menu[MenuID].pDefinition[Items];

                if(Items != 0)
                {
                    ItemsChar  = (char)Items;
                    ItemsChar += (ItemsChar >= 10) ? ('a' - 10) : '0';
                    MenuSelectItems(ItemsChar);
                }

                InMenuPrintf(VT100_SZ_NONE, pMenu->Label);

                if(Items == 0)
                {
                    InMenuPrintf(VT100_SZ_NONE, VT100_LBL_SELECT);
                    if(pMenu != pPreviousMenu)
                    {
                        CallBack(pMenu->pCallback, VT100_CALLBACK_INIT, 0);
                    }
                }
                else
                {
                    CallBack(pMenu->pCallback, VT100_CALLBACK_INIT, Items);
                }
            }

            MenuSelectItems('0');
            InMenuPrintf(VT100_SZ_NONE, VT100_LBL_QUIT);
            ItemsChar  = (char)(Items - 1);
            ItemsChar += (ItemsChar >= 10) ? ('a' - 10) : '0';
            InMenuPrintf(VT100_SZ_NONE, VT100_LBL_ENTER_SELECTION, ItemsChar);

            // need to Calculated Actual X and Y position of cursor
            //SetCursorPosition();

            return ItemsQts;
        }

        // There is nothing to draw if it has only one item ( it is a redirection menu )
        CallBack(m_Menu[m_MenuID].pDefinition[0].pCallback, VT100_CALLBACK_INIT, 0);

        return 0;
    }

    InMenuPrintf(VT100_SZ_NONE, VT100_LBL_CLEAR_SCREEN);
  #if (VT100_USE_COLOR == DEF_ENABLED)
    SetForeColor(VT100_COLOR_WHITE);
  #endif
    m_BypassPrintf = false;
    return 0;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           PrintMenuStaticInfo
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Display standard Digini menu header
//
//-------------------------------------------------------------------------------------------------
#if (VT100_USE_STANDARD_MENU_STATIC_INFO == DEF_ENABLED)
void VT100_Terminal::PrintMenuStaticInfo(void)
{
    const char* pString;
    size_t      SizeLine;
    size_t      SizeTitle;

    m_BypassPrintf = true;
    InMenuPrintf(VT100_SZ_NONE, VT100_LBL_RESET_TERMINAL);
    nOS_Sleep(100);                                                 // Terminal need time to reset
    InMenuPrintf(VT100_SZ_NONE, VT100_LBL_HIDE_CURSOR);
    InMenuPrintf(VT100_SZ_NONE, VT100_LBL_CLEAR_SCREEN);
  #if (VT100_USE_COLOR == DEF_ENABLED)
    SetColor(VT100_COLOR_WHITE, VT100_COLOR_BLUE);
  #endif
    InMenuPrintf(VT100_SZ_NONE, VT100_LBL_LINE_SEPARATOR);
    pString  = myLabel.GetPointer(VT100_LBL_LINE_SEPARATOR);
    SizeLine = strlen(pString) - 1;
    pString  = myLabel.GetPointer(LBL_VT100_MENU_TITLE);
     SizeTitle = strlen(pString);
    SizeLine -= SizeTitle;
    RepeatChar(' ', SizeLine / 2);
    InMenuPrintf(VT100_SZ_NONE, LBL_VT100_MENU_TITLE);
    RepeatChar(' ', (SizeLine / 2) + (SizeLine % 2));
    InMenuPrintf(VT100_SZ_NONE, LBL_LINEFEED);
    InMenuPrintf(VT100_SZ_NONE, VT100_LBL_LINE_SEPARATOR);
  #if (VT100_USE_COLOR == DEF_ENABLED)
    SetColor(VT100_COLOR_YELLOW, VT100_COLOR_BLACK);
  #endif
    InMenuPrintf(VT100_SZ_NONE, LBL_DOUBLE_LINEFEED);

    m_SetMenuCursorPosY = 8;
}
#endif

//-------------------------------------------------------------------------------------------------
//
//  Name:           RepeatChar
//
//  Parameter(s):   uint8_t             Char to print multiple time
//                  size_t              How many time to repeat print of the char
//  Return:         None
//
//  Description:    Print a char multiple time
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void VT100_Terminal::RepeatChar(uint8_t Char, size_t Count)
{
    char* pBuffer;

    if((pBuffer = (char*)pMemoryPool->AllocAndSet(Count + 1, Char)) != nullptr)
    {
        pBuffer[Count] = '\0';
        m_pConsole->SendData((const uint8_t*)pBuffer, &Count);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ClearInputMenuSelection
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Flush Input line after selection and input was processed
//
//-------------------------------------------------------------------------------------------------
void VT100_Terminal::ClearInputMenuSelection(void)
{
    SetCursorPosition(m_LastSetCursorPosX, m_LastSetCursorPosY);
    InMenuPrintf(VT100_SZ_NONE, VT100_LBL_ERASE_FROM_CURSOR_N_CHAR, 2);
    SetCursorPosition(m_LastSetCursorPosX, m_LastSetCursorPosY);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ClearScreenWindows
//
//  Parameter(s):   uint8_t PosX
//                  uint8_t PosY
//                  uint8_t SizeX
//                  uint8_t SizeY
//  Return:         None
//
//  Description:    Clear the portion of the screen specified by parameter
//
//-------------------------------------------------------------------------------------------------
void VT100_Terminal::ClearScreenWindow(uint8_t PosX, uint8_t PosY, uint8_t SizeX, uint8_t SizeY)
{
    for(int y = PosY; y < (PosY + SizeY); y++)
    {
        SetCursorPosition(PosX, y);
        InMenuPrintf(VT100_SZ_NONE, VT100_LBL_ERASE_FROM_CURSOR_N_CHAR, int(SizeX));
    }
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
    InMenuPrintf(VT100_SZ_NONE, LBL_STRING, "\r  (");
  #if (VT100_USE_COLOR == DEF_ENABLED)
    SetForeColor(VT100_COLOR_CYAN);
  #endif
    if(ItemsChar == '0') InMenuPrintf(VT100_SZ_NONE, LBL_STRING, "ESC");
    else                 InMenuPrintf(VT100_SZ_NONE, LBL_CHAR, ItemsChar);
  #if (VT100_USE_COLOR == DEF_ENABLED)
    SetForeColor(VT100_COLOR_YELLOW);
  #endif
    if(ItemsChar == '0') InMenuPrintf(VT100_SZ_NONE, LBL_STRING, ") ");
    else                 InMenuPrintf(VT100_SZ_NONE, LBL_STRING, ")   ");
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CallBack
//
//  Parameter(s):   CallbackMethod_t                Item menu callback
//                  CON_CallBackType_e              Type of callback (Tell callback what to do)
//                  uint8_t                         Item selection in the menu
//  Return:         CON_InputType_e
//
//  Description:    Call the callback
//
//  Note(s):        Push and restore the attribute
//
//-------------------------------------------------------------------------------------------------
VT100_InputType_e VT100_Terminal::CallBack(CallbackMethod_t pCallback, VT100_CallBackType_e Type, uint8_t Item)
{
    VT100_InputType_e InputType;

    InputType = VT100_INPUT_MENU_CHOICE;

    if(pCallback != nullptr)
    {
        SaveAttribute();
        InputType = pCallback(Item, Type);
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
    nOS_StatusReg   sr;
    VT100_Terminal* This;
    uint8_t         Escape;

    VAR_UNUSED(pTimer);
    This = (VT100_Terminal*)pArg;

    nOS_EnterCritical(sr);
    This->m_InEscapeSequence = false;
    This->m_InputDecimalMode = false;
    This->m_InputStringMode  = false;
    Escape = VT100_TRAP_REAL_ESCAPE;
    This->m_pConsole->Write(&Escape, 1);
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

      #if (VT100_USE_COLOR == DEF_ENABLED)
        if((m_Value >= m_Minimum) && (m_Value <= m_Maximum))
        {
            SetColor(VT100_COLOR_BLACK, VT100_COLOR_GREEN);
        }
        else
        {
            SetColor(VT100_COLOR_BLACK, VT100_COLOR_RED);
        }
      #endif

        SetCursorPosition(m_PosX + 36, m_PosY + 3);
        InMenuPrintf(VT100_SZ_NONE, LBL_CHAR, ' ');

        switch(m_Divider)
        {
            case 10:   InMenuPrintf(VT100_SZ_NONE, VT100_LBL_INT_TO_DIVIDE_BY_10,   m_Value / m_Divider, abs(m_Value % m_Divider)); break;
            case 100:  InMenuPrintf(VT100_SZ_NONE, VT100_LBL_INT_TO_DIVIDE_BY_100,  m_Value / m_Divider, abs(m_Value % m_Divider)); break;
            case 1000: InMenuPrintf(VT100_SZ_NONE, VT100_LBL_INT_TO_DIVIDE_BY_1000, m_Value / m_Divider, abs(m_Value % m_Divider)); break;
            default:   InMenuPrintf(VT100_SZ_NONE, VT100_LBL_INT_NO_DIVIDE,         m_Value);                                       break;
        }

        InMenuPrintf(VT100_SZ_NONE, VT100_LBL_WHITE_MOVE_CURSOR_2_TO_LEFT);
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
        InMenuPrintf(VT100_SZ_NONE, VT100_LBL_STRING_AND_ONE_SPACE, m_pString);
        InMenuPrintf(VT100_SZ_NONE, VT100_LBL_MOVE_LEFT_CURSOR);
    }
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
#if (VT100_USE_COLOR == DEF_ENABLED)
void VT100_Terminal::PrintSaveLabel(uint8_t PosX, uint8_t PosY, VT100_Color_e Color)
#else
void VT100_Terminal::PrintSaveLabel(uint8_t PosX, uint8_t PosY)
#endif
{
  #if (VT100_USE_COLOR == DEF_ENABLED)
    SetForeColor(Color);
  #endif
    SetCursorPosition(PosX, PosY);
    InMenuPrintf(VT100_SZ_NONE, VT100_LBL_SAVE_CONFIGURATION);
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
  #if (VT100_USE_COLOR == DEF_ENABLED)
    SetForeColor(VT100_COLOR_CYAN);
  #endif
    SetCursorPosition(PosX + 2,  PosY + 1);
    InMenuPrintf(VT100_SZ_NONE, VT100_LBL_MINIMUM);
    InMenuPrintf(VT100_SZ_NONE, LBL_STRING, " : ");

    switch(Divider)
    {
        case 10:  InMenuPrintf(VT100_SZ_NONE, VT100_LBL_INT_TO_DIVIDE_BY_10,   Minimum / Divider,  abs(Minimum % Divider)); break;
        case 100: InMenuPrintf(VT100_SZ_NONE, VT100_LBL_INT_TO_DIVIDE_BY_100,  Minimum / Divider,  abs(Minimum % Divider)); break;
        case 1000:InMenuPrintf(VT100_SZ_NONE, VT100_LBL_INT_TO_DIVIDE_BY_1000, Minimum / Divider,  abs(Minimum % Divider)); break;
        default:  InMenuPrintf(VT100_SZ_NONE, VT100_LBL_INT_NO_DIVIDE,         Minimum);                                    break;
    }


    SetCursorPosition(PosX + 24, PosY + 1);
    InMenuPrintf(VT100_SZ_NONE, VT100_LBL_MAXIMUM);
    InMenuPrintf(VT100_SZ_NONE, LBL_STRING, " : ");

    switch(Divider)
    {
        case 10:  InMenuPrintf(VT100_SZ_NONE, VT100_LBL_INT_TO_DIVIDE_BY_10,   Maximum / Divider,  abs(Maximum % Divider)); break;
        case 100: InMenuPrintf(VT100_SZ_NONE, VT100_LBL_INT_TO_DIVIDE_BY_100,  Maximum / Divider,  abs(Maximum % Divider)); break;
        case 1000:InMenuPrintf(VT100_SZ_NONE, VT100_LBL_INT_TO_DIVIDE_BY_1000, Maximum / Divider,  abs(Maximum % Divider)); break;
        default:  InMenuPrintf(VT100_SZ_NONE, VT100_LBL_INT_NO_DIVIDE,         Maximum);                                    break;
    }

    // Print type of input
  #if (VT100_USE_COLOR == DEF_ENABLED)
    SetForeColor(VT100_COLOR_YELLOW);
  #endif
    SetCursorPosition(PosX + 2, PosY + 3);
    InMenuPrintf(VT100_SZ_NONE, LBL_STRING, pMsg);

    // Add 'how to' info
    SetCursorPosition(PosX + 2,  PosY + 5);
    InMenuPrintf(VT100_SZ_NONE, VT100_LBL_INPUT_VALIDATION);

    // And show cursor
    InMenuPrintf(VT100_SZ_NONE, VT100_LBL_SHOW_CURSOR);
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
  #if (VT100_USE_COLOR == DEF_ENABLED)
    SetForeColor(VT100_COLOR_CYAN);
  #endif
    SetCursorPosition(PosX + 2,  PosY + 1);
    InMenuPrintf(Maximum, LBL_STRING, pMsg);

    // Add 'how to' info
    SetCursorPosition(PosX + 2,  PosY + 5);
    InMenuPrintf(VT100_SZ_NONE, VT100_LBL_INPUT_VALIDATION);

    // Copy string
    if((m_pString = (char*)pMemoryPool->AllocAndSet(VT100_STRING_SZ + 1, 0)) != nullptr)
    {
        memcpy(m_pString, pString, VT100_STRING_SZ);
    }     // todo handle error....

 //   STR_strnstrip(m_pString, Maximum);                 // TODO Fix this
    m_InputPtr = strlen(m_pString);                          // Get string end pointer
    m_RefreshInputPtr = m_InputPtr + 1;                     // To force a refresh

    // And show cursor
    InMenuPrintf(VT100_SZ_NONE, VT100_LBL_SHOW_CURSOR);
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
            if(strlen(m_pString) <= size_t(m_Maximum))
            {
                ID = m_ID;
                memcpy(pString, m_pString, VT100_STRING_SZ);
                pMemoryPool->Free((void**)&m_pString);
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
size_t VT100_Terminal::InMenuPrintf(int nSize, Label_e Label, ...)
{
    va_list     vaArg;
    char*       pBuffer;
    size_t      Size    = 0;
    const char* pFormat = myLabel.GetPointer(Label);

    if((pBuffer = (char*)pMemoryPool->Alloc(VT100_TERMINAL_SIZE)) != nullptr)
    {
        va_start(vaArg, Label);
        Size = STR_vsnprintf(pBuffer, ((nSize == VT100_SZ_NONE) ? VT100_TERMINAL_SIZE : nSize), pFormat, vaArg);
        m_pConsole->SendData((const uint8_t*)&pBuffer[0], &Size);
        va_end(vaArg);
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
//  Return:         size_t              Number of character printed.
//
//  Description:    Send formatted string to console if menu system is active.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
/*
size_t VT100_Terminal::LoggingPrintf(CLI_DebugLevel_e Level, const char* pFormat, ...)
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
            if((pBuffer = (char*)pMemoryPool->Alloc(VT100_TERMINAL_SIZE)) == nullptr)
            {
                va_start(vaArg, (const char*)pFormat);
                Size = STR_vsnprintf(pBuffer, VT100_TERMINAL_SIZE, pFormat, vaArg);
                while(m_pUartDriver->IsItBusy() == true){};
                m_pUartDriver->SendData((const uint8_t*)&pBuffer[0], &Size, pBuffer);
                UART_Write(UART_CONSOLE, pBuffer, Size);
                m_pUartDriver->SendData("\r", &Size);



                va_end(vaArg);
                pMemoryPool->Free((void**)&pBuffer);
            }
        }
    }

    return Size;
}
*/
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
        InMenuPrintf(VT100_SZ_NONE, VT100_LBL_ATTRIBUTE, Attribute);
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
        InMenuPrintf(VT100_SZ_NONE, VT100_LBL_SAVE_ATTRIBUTE);
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
        InMenuPrintf(VT100_SZ_NONE, VT100_LBL_RESTORE_ATTRIBUTE);
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
        InMenuPrintf(VT100_SZ_NONE, VT100_LBL_SET_CURSOR, PosY, PosX);
        m_LastSetCursorPosX = PosX;
        m_LastSetCursorPosY = PosY;
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
        InMenuPrintf(VT100_SZ_NONE, VT100_LBL_SAVE_CURSOR);
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
        InMenuPrintf(VT100_SZ_NONE, VT100_LBL_RESTORE_CURSOR);
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
        InMenuPrintf(VT100_SZ_NONE, VT100_LBL_SCROLL_ZONE, FirstLine, LastLine);
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
#if (VT100_USE_COLOR == DEF_ENABLED)
void VT100_Terminal::SetColor(VT100_Color_e ForeColor, VT100_Color_e BackColor)
{
    if(m_IsDisplayLock == false)
    {
        SetAttribute(VT100_Attribute_e(int(ForeColor) + VT100_OFFSET_COLOR_FOREGROUND));
        SetAttribute(VT100_Attribute_e(int(BackColor) + VT100_OFFSET_COLOR_BACKGROUND));
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
#ifndef VT100_USE_COLOR
void VT100_Terminal::InvertMono(bool Invert)
{
    if(Invert == true) InMenuPrintf(VT100_SZ_NONE, VT100_LBL_BACK_WHITE_FORE_BLACK);
    else               InMenuPrintf(VT100_SZ_NONE, VT100_LBL_BACK_BLACK_FORE_WHITE);
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
    InMenuPrintf(VT100_SZ_NONE, VT100_LBL_TIME_DATE_STAMP, pTimeDate->year,
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
      #if (VT100_USE_COLOR == DEF_ENABLED)
        SetForeColor(ForeColor);
      #endif
        SetCursorPosition(PosX, PosY++);
        InMenuPrintf(VT100_SZ_NONE, LBL_STRING, "╔");

        for(uint8_t i = 0; i < (H_Size - 2); i++)
        {
            InMenuPrintf(VT100_SZ_NONE, LBL_STRING, "�?");
        }

        InMenuPrintf(VT100_SZ_NONE, LBL_STRING, "╗");

        for(uint8_t i = 0; i < (V_Size - 2); i++)
        {
            SetCursorPosition(PosX, PosY);

            InMenuPrintf(VT100_SZ_NONE, LBL_STRING, "║");

            // Erase inside
            for(uint8_t j = 0; j < (H_Size - 2); j++)
            {
                InMenuPrintf(VT100_SZ_NONE, LBL_CHAR, ' ');
            }

            SetCursorPosition(PosX + (H_Size - 1), PosY++);
            InMenuPrintf(VT100_SZ_NONE, LBL_STRING, "║");
        }

        SetCursorPosition(PosX, PosY);
        InMenuPrintf(VT100_SZ_NONE, LBL_STRING, "╚");

        for(uint8_t i = 0; i < (H_Size - 2); i++)
        {
            InMenuPrintf(VT100_SZ_NONE, LBL_STRING, "�?");
        }
        InMenuPrintf(VT100_SZ_NONE, LBL_STRING, "�?");
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
      #if (VT100_USE_COLOR == DEF_ENABLED)
        SetForeColor(ForeColor);
      #endif

        for(uint8_t i = 0; i < V_Size; i++)
        {
            SetCursorPosition(PosX, PosY++);
            InMenuPrintf(VT100_SZ_NONE, LBL_STRING, "║");
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Bargraph
//
//  Parameter(s):   uint8_t         PosX        X Position on screen.
//                  uint8_t         PosY        Y Position On screen.
//              **  VT100_Color_e   Color       Color of the bargraph.
//                  uint8_t         Value       Actual value.
//                  uint8_t         Max         Maximum value.
//                  uint8_t         Size        Size in character.
//
//
//  Return:         None
//
//  Description:    Print the bargraph according to value and maximum at specified location.
//
//  Note(s):        ** This parameter exist only if VT100_USE_COLOR is defined
//
//-------------------------------------------------------------------------------------------------
#if (VT100_USE_COLOR == DEF_ENABLED)
void VT100_Terminal::Bargraph(uint8_t PosX, uint8_t PosY, VT100_Color_e Color, uint8_t Value, uint8_t Max, uint8_t Size)
#else
void VT100_Terminal::Bargraph(uint8_t PosX, uint8_t PosY, uint8_t Value, uint8_t Max, uint8_t Size)
#endif
{
    int i;

    SetCursorPosition(PosX, PosY);

    for(i = 0; i < Size; i++)
    {
      #if (VT100_USE_COLOR == DEF_ENABLED)
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

        InMenuPrintf(VT100_SZ_NONE, LBL_CHAR, ' ');
    }

  #if (VT100_USE_COLOR == DEF_ENABLED)
    SetAttribute(VT100_Attribute_e(VT100_COLOR_BLACK + VT100_OFFSET_COLOR_BACKGROUND));
  #else
    InvertMono(false);
  #endif
}

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

    if(m_pConsole->At(0) == '"')
    {
        m_pConsole->Flush(1);

        for(size_t i = 0; i <= Size; i++)
        {
            if(m_pConsole->Read(&Character, 1) == 1)
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
//  Name:           ForceMenuRefresh
//
//  Parameter(s):   None
//
//  Return:         None
//
//  Description:    This function force a refresh of the menu.
//
//  Note(s):        TODO add F5 key detection for a refresh
//
//-------------------------------------------------------------------------------------------------
void VT100_Terminal::ForceMenuRefresh(void)
{
    m_ForceRefresh = true;
}

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_VT100_MENU == DEF_ENABLED)
