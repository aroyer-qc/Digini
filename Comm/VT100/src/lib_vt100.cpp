//-------------------------------------------------------------------------------------------------
//
//  File : lib_vt100.cpp
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
#define VT100_ESCAPE                                    255

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
//  Return:         None
//
//  Description:    Initialize VT100_terminal
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
nOS_Error VT100_Terminal::Initialize(Console* pConsole)
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
   // m_BackFromEdition         = false;
    m_FlushNextEntry          = false;
    m_ValidateInput           = false;
    m_LogsAreMuted            = true;

    Error = nOS_TimerCreate(&m_EscapeTimer, EscapeCallback, this, VT100_ESCAPE_TIME_OUT, NOS_TIMER_ONE_SHOT);
  #if (VT100_USER_CALLBACK_INITIALIZE == DEF_ENABLED)
    CallbackInitialize();                       // User callback specific initialization
  #endif
    ClearConfigFLag();
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
    const VT100_MenuDef_t* pMenu = nullptr;
    TickCount_t            Delay;

    ProcessRX();

    if(m_IsItInitialized == false)
    {
        m_IsItInitialized = true;
        InMenuPrintf(VT100_LBL_RESET_TERMINAL);
        Delay = GetTick();
        while(TickHasTimeOut(Delay, 10) == false){};
      #if (VT100_USE_STANDARD_MENU_STATIC_INFO == DEF_ENABLED) || (VT100_USE_USER_MENU_STATIC_INFO == DEF_ENABLED)
        PrintMenuStaticInfo();
      #endif
        GoToMenu(VT100_STARTUP_MENU_ID_CFG);
    }

    // Input mode: Decimal/hexadecimal and string input mode.
    //      there is no need to execute further in those mode
    else if(m_InputDecimalMode == true)     {  InputDecimal();  }
    else if(m_InputStringMode  == true)     {  InputString();   }

    // Menu display mode and refreshDisplay the menu and process callback
    else
    {
        if(m_Input < m_ItemsQts)
        {
            pMenu = &m_Menu[m_MenuID].pDefinition[m_Input];                 // Get pointer on the menu from actual or new menu
        }
        else if(m_Input == VT100_ESCAPE)
        {
            m_Input = 0;
            pMenu   = &m_Menu[m_MenuID].pDefinition[0];                     // Get pointer on the previous menu
        }
        else // It was an invalid entry (should not happened) do nothing
        {
            m_Input         = 0;
            m_ValidateInput = false;
        }

        if(m_ValidateInput == true)
        {
            m_ValidateInput = false;

            // Validate the range for the menu
            if(m_InputType == VT100_INPUT_MENU_CHOICE)
            {
                if(pMenu->NextMenu == VT100_MENU_NONE)
                {
                    m_IsItInitialized = false;
                    ClearScreenWindow(0, 0, VT100_X_SIZE, VT100_Y_SIZE);            // Clear screen
                    m_pConsole->ReleaseControl();
                }
                else if(m_MenuID != pMenu->NextMenu)                                // If new menu selection, draw this new menu
                {
                    FinalizeAllItems();                                             // Finalize all items in previous menu
                    ClearConfigFLag();                                              // make sure all flag are initialize for the new menu
                    GoToMenu(pMenu->NextMenu);                                      // display new menu and initialize all items in new menu
                }
                else
                {
                    CallBack(pMenu->pCallback, VT100_CALLBACK_ON_INPUT, m_Input);   // This is an input for menu with dynamic information to change
                }
            }
        }
        else if (pMenu != nullptr)
        {
            if(m_RefreshOnce == true)
            {
                m_RefreshOnce = false;
                CallBack(pMenu->pCallback, VT100_CALLBACK_REFRESH_ONCE, 0);
            }

            CallBack(pMenu->pCallback, VT100_CALLBACK_REFRESH, 0);
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
                    if(Data != VT100_ESCAPE)
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
                    //m_BackFromEdition  = true;
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
                else if(Data == VT100_ESCAPE)
                {
                    m_ID = VT100_INPUT_INVALID_ID;
                    GoToMenu(m_MenuID);
                }
            }
            break;

            case VT100_INPUT_STRING:
            {
                if(Data == ASCII_CARRIAGE_RETURN)
                {
                    //m_BackFromEdition = true;
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
                else if(Data == VT100_ESCAPE)
                {
                    m_ID = VT100_INPUT_INVALID_ID;
                    GoToMenu(m_MenuID);
                }
            }
            break;

            case VT100_INPUT_ESCAPE:
            {
                if(Data == VT100_ESCAPE)
                {
                    GoToMenu(m_MenuID);
                }
            }
            break;

        //    case VT100_INPUT_ESCAPE_TO_CONTINUE:
        //    {
        //        if(Data == VT100_ESCAPE)
        //        {
        //            m_ValidateInput = true;
        //            m_InputType     = VT100_INPUT_MENU_CHOICE;
        //        }
        //    }
        //    break;

            default: break;
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           FlushAllItems
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Flush all items in the menu
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void VT100_Terminal::FinalizeAllItems(void)
{
    const VT100_MenuDef_t* pMenu;
    uint8_t                Items;

    for(Items = 0; Items < m_Menu[m_MenuID].Size; Items++)
    {
        pMenu = &m_Menu[m_MenuID].pDefinition[Items];
        CallBack(pMenu->pCallback, VT100_CALLBACK_ON_FLUSH, Items);
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
    m_MenuID      = MenuID;
    m_InputType   = VT100_INPUT_MENU_CHOICE;
    m_Input       = 0;
    m_RefreshOnce = true;
    DisplayMenu();
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ClearConfigFLag
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Clear the new configuration flag
//
//  Note(s):        The next new menu access will also get the fresh 'save configuration'
//
//-------------------------------------------------------------------------------------------------
void VT100_Terminal::ClearConfigFLag(void)
{
    for(int i = 0; i < CONFIG_FLAG_SIZE; i++)
    {
        m_ConfigFlag[i] = 0;
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           DisplayMenu
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Display selected menu
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void VT100_Terminal::DisplayMenu(void)
{
    const VT100_MenuDef_t*  pMenu;
    uint8_t                 Items;
    char                    ItemsChar;

    pMenu = nullptr;
    ClearScreenWindow(0, 4, VT100_X_SIZE, VT100_Y_SIZE);     // Clear screen bellow header
    m_PosY_SaveLabel = 9;
   SetCursorPosition(0, 6);                              // Reposition cursor to print menu
  #if (VT100_USE_COLOR == DEF_ENABLED)
    SetForeColor(VT100_COLOR_YELLOW);
  #endif
    m_ItemsQts = m_Menu[m_MenuID].Size;

    if(m_ItemsQts > 1)
    {
        // Print all items in the menu list
        for(Items = 0; Items < m_ItemsQts; Items++)
        {
            //pPreviousMenu = pMenu;
            pMenu = &m_Menu[m_MenuID].pDefinition[Items];

            if(Items != 0)
            {
                ItemsChar  = (char)Items;
                ItemsChar += (ItemsChar >= 10) ? ('a' - 10) : '0';
                MenuSelectItems(ItemsChar);
                if(pMenu->Label == VT100_LBL_SAVE_CONFIGURATION)
                {
                  #if (VT100_USE_COLOR == DEF_ENABLED)
                    SetForeColor(VT100_COLOR_BLUE);
                  #endif
                }

                InMenuPrintf(pMenu->Label);
                m_PosY_SaveLabel++;

              #if (VT100_USE_COLOR == DEF_ENABLED)
                SetForeColor(VT100_COLOR_YELLOW);
              #endif
            }
            else
            {
                InMenuPrintf(1, 6, pMenu->Label);
                InMenuPrintf(2, 8, VT100_LBL_SELECT);
            }
            // CallBack(pMenu->pCallback, VT100_CALLBACK_ON_INIT, Items);
        }

        MenuSelectItems('0');
        InMenuPrintf(VT100_LBL_QUIT);
        ItemsChar  = (char)(Items - 1);
        ItemsChar += (ItemsChar >= 10) ? ('a' - 10) : '0';
        InMenuPrintf(VT100_LBL_ENTER_SELECTION, ItemsChar);
    }
    else
    {
        // There is nothing to draw if it has only one item ( it is a redirection menu )
        CallBack(m_Menu[m_MenuID].pDefinition[0].pCallback, VT100_CALLBACK_INIT, 0);
    }
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
    InMenuPrintf(VT100_LBL_RESET_TERMINAL);
    nOS_Sleep(100);                                                 // Terminal need time to reset
    InMenuPrintf(VT100_LBL_HIDE_CURSOR);
    InMenuPrintf(VT100_LBL_CLEAR_SCREEN);
  #if (VT100_USE_COLOR == DEF_ENABLED)
    SetColor(VT100_COLOR_WHITE, VT100_COLOR_BLUE);
  #endif
    InMenuPrintf(VT100_LBL_LINE_SEPARATOR);
    pString  = myLabel.GetPointer(VT100_LBL_LINE_SEPARATOR);
    SizeLine = VT100_X_SIZE;
    pString  = myLabel.GetPointer(VT100_LBL_MENU_TITLE);
    SizeTitle = strlen(pString);
    SizeLine -= SizeTitle;
    RepeatChar(' ', SizeLine / 2);
    InMenuPrintf(VT100_LBL_MENU_TITLE);
    RepeatChar(' ', (SizeLine / 2) + (SizeLine % 2));
    InMenuPrintf(LBL_LINEFEED);
    InMenuPrintf(VT100_LBL_LINE_SEPARATOR);
  #if (VT100_USE_COLOR == DEF_ENABLED)
    SetColor(VT100_COLOR_YELLOW, VT100_COLOR_BLACK);
  #endif
    InMenuPrintf(LBL_DOUBLE_LINEFEED);
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
    InMenuPrintf(VT100_LBL_ERASE_FROM_CURSOR_N_CHAR, 2);
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
        InMenuPrintf(PosX, y, VT100_LBL_ERASE_FROM_CURSOR_N_CHAR, int(SizeX));
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
    InMenuPrintf(LBL_STRING, "\r  (");

  #if (VT100_USE_COLOR == DEF_ENABLED)
    SetForeColor(VT100_COLOR_CYAN);
  #endif

    if(ItemsChar == '0') InMenuPrintf(LBL_STRING, "ESC");
    else                 InMenuPrintf(LBL_CHAR, ItemsChar);

  #if (VT100_USE_COLOR == DEF_ENABLED)
    SetForeColor(VT100_COLOR_YELLOW);
  #endif

    if(ItemsChar == '0') InMenuPrintf(LBL_STRING, ") ");
    else                 InMenuPrintf(LBL_STRING, ")   ");
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
    Escape = VT100_ESCAPE;
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
        SetColor(VT100_COLOR_BLACK, ((m_Value >= m_Minimum) && (m_Value <= m_Maximum)) ? VT100_COLOR_GREEN : VT100_COLOR_RED);
      #endif

        InMenuPrintf(m_PosX + 36, m_PosY + 3, LBL_CHAR, ' ');

        switch(m_Divider)
        {
            case 10:   InMenuPrintf(VT100_LBL_INT_TO_DIVIDE_BY_10,   m_Value / m_Divider, abs(m_Value % m_Divider)); break;
            case 100:  InMenuPrintf(VT100_LBL_INT_TO_DIVIDE_BY_100,  m_Value / m_Divider, abs(m_Value % m_Divider)); break;
            case 1000: InMenuPrintf(VT100_LBL_INT_TO_DIVIDE_BY_1000, m_Value / m_Divider, abs(m_Value % m_Divider)); break;
            default:   InMenuPrintf(VT100_LBL_INT_NO_DIVIDE,         m_Value);                                       break;
        }

        InMenuPrintf(VT100_LBL_WHITE_MOVE_CURSOR_2_TO_LEFT);
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
        m_RefreshInputPtr = m_InputPtr;
        InMenuPrintf(m_PosX + 2, m_PosY + 3, VT100_LBL_STRING_AND_ONE_SPACE, m_pString);
        InMenuPrintf(VT100_LBL_MOVE_LEFT_CURSOR);
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
void VT100_Terminal::UpdateSaveLabel(VT100_Color_e Color)
#endif
{
    SetForeColor(Color);
    InMenuPrintf(9, m_PosY_SaveLabel, VT100_LBL_SAVE_CONFIGURATION);
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
//                  Label_e      Label       ID of the label to print in the box.
//  Return:         None
//
//  Description:    Prepare decimal Input value process with min/max value and an ID to tell user
//                  callback what was the request.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void VT100_Terminal::SetDecimalInput(uint8_t PosX, uint8_t PosY, int32_t Minimum, int32_t Maximum, int32_t Value, uint16_t Divider, uint8_t ID, Label_e Label)
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
  #if (VT100_USE_COLOR == DEF_ENABLED)
    DrawBox(PosX, PosY, 56, 8, VT100_COLOR_WHITE);
  #else
    DrawBox(PosX, PosY, 56, 8);
  #endif

    // Write input information
    SetForeColor(VT100_COLOR_CYAN);
    InMenuPrintf(PosX + 2,  PosY + 1, VT100_LBL_MINIMUM);
    InMenuPrintf(LBL_STRING, " : ");

    switch(Divider)
    {
        case 10:  InMenuPrintf(VT100_LBL_INT_TO_DIVIDE_BY_10,   Minimum / Divider,  abs(Minimum % Divider)); break;
        case 100: InMenuPrintf(VT100_LBL_INT_TO_DIVIDE_BY_100,  Minimum / Divider,  abs(Minimum % Divider)); break;
        case 1000:InMenuPrintf(VT100_LBL_INT_TO_DIVIDE_BY_1000, Minimum / Divider,  abs(Minimum % Divider)); break;
        default:  InMenuPrintf(VT100_LBL_INT_NO_DIVIDE,         Minimum);                                    break;
    }


    InMenuPrintf(PosX + 24, PosY + 1, VT100_LBL_MAXIMUM);
    InMenuPrintf(LBL_STRING, " : ");

    switch(Divider)
    {
        case 10:  InMenuPrintf(VT100_LBL_INT_TO_DIVIDE_BY_10,   Maximum / Divider,  abs(Maximum % Divider)); break;
        case 100: InMenuPrintf(VT100_LBL_INT_TO_DIVIDE_BY_100,  Maximum / Divider,  abs(Maximum % Divider)); break;
        case 1000:InMenuPrintf(VT100_LBL_INT_TO_DIVIDE_BY_1000, Maximum / Divider,  abs(Maximum % Divider)); break;
        default:  InMenuPrintf(VT100_LBL_INT_NO_DIVIDE,         Maximum);                                    break;
    }

    // Print type of input
  #if (VT100_USE_COLOR == DEF_ENABLED)
    SetForeColor(VT100_COLOR_YELLOW);
  #endif
    //InMenuPrintf(PosX + 2, PosY + 3, LBL_STRING, pMsg);

    // Add 'how to' info
    InMenuPrintf(PosX + 2,  PosY + 5, VT100_LBL_INPUT_VALIDATION);

    // And show cursor
    InMenuPrintf(VT100_LBL_SHOW_CURSOR);
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
//                  Label_e     Label       ID of the label to print in the box.
//                  const char* pString     Actual string to edit.
//  Return:         None
//
//  Description:    Prepare Input string process with max size and an ID to tell user callback what
//                  was the request.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void VT100_Terminal::SetStringInput(uint8_t PosX, uint8_t PosY, int32_t Maximum, uint8_t ID, Label_e Label, const char* pString)
{
    m_Maximum          = Maximum;
    m_PosX             = PosX;
    m_PosY             = PosY;
    m_ID               = (int16_t)ID;
    m_InputStringMode  = true;
    m_IsItString       = true;

    // Draw the Box
  #if (VT100_USE_COLOR == DEF_ENABLED)
    DrawBox(PosX, PosY, 56, 8, VT100_COLOR_WHITE);
  #else
    DrawBox(PosX, PosY, 56, 8);
  #endif

    // Write input information
  #if (VT100_USE_COLOR == DEF_ENABLED)
    SetForeColor(VT100_COLOR_CYAN);
  #endif
    //InMenuPrintf(PosX + 2,  PosY + 1, Maximum, LBL_STRING, pMsg);

    // Add 'how to' info
    InMenuPrintf(PosX + 2,  PosY + 5, VT100_LBL_INPUT_VALIDATION);

    // Copy string
    if((m_pString = (char*)pMemoryPool->AllocAndSet(VT100_STRING_SZ + 1, 0)) != nullptr)
    {
        memcpy(m_pString, pString, VT100_STRING_SZ);
    }     // todo handle error....

    STR_strnstrip(m_pString, Maximum);                      // TODO Fix this
    m_InputPtr = strlen(m_pString);                         // Get string end pointer
    m_RefreshInputPtr = m_InputPtr + 1;                     // To force a refresh

    // And show cursor
    InMenuPrintf(VT100_LBL_SHOW_CURSOR);
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
//  Parameter(s):   Label_e     Label       ID of the label to with optional formatting.
//                  ...                     Parameter for formatting if any.
//
//  Return:         None
//
//  Description:    Send formatted string to console if menu system is active.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
size_t VT100_Terminal::InMenuPrintf(Label_e Label, ...)
{
    size_t  Size;
    va_list vaArg;

    va_start(vaArg, Label);
    Size = MenuPrintfCommon(Label, &vaArg);
    va_end(vaArg);
    return Size;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           InMenuPrintf
//
//  Parameter(s):   Label_e     Label       ID of the label to with optional formatting.
//                  ...                     Parameter for formatting if any.
//
//  Return:         None
//
//  Description:    Send formatted string to console if menu system is active.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
size_t VT100_Terminal::InMenuPrintf(uint8_t PosX, uint8_t PosY, Label_e Label, ...)
{
    size_t  Size;
    va_list vaArg;

    SetCursorPosition(PosX, PosY);
    va_start(vaArg, Label);
    Size = MenuPrintfCommon(Label, &vaArg);
    va_end(vaArg);
    return Size;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           MenuPrintfCommon
//
//  Parameter(s):   Label_e     Label       ID of the label to with optional formatting.
//                  va_list*    p_vaArg     Parameter from va_list.
//
//  Return:         None
//
//  Description:    Common function to format string and send it to console.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
size_t VT100_Terminal::MenuPrintfCommon(Label_e Label, va_list* p_vaArg)
{
    char*  pBuffer;
    size_t Size    = 0;

    if((pBuffer = (char*)pMemoryPool->Alloc(VT100_TERMINAL_SIZE)) != nullptr)
    {
        const char* pFormat = myLabel.GetPointer(Label);
        Size = STR_vsnformat(pBuffer, VT100_TERMINAL_SIZE, pFormat, *p_vaArg);
        m_pConsole->SendData((const uint8_t*)&pBuffer[0], &Size);
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
        InMenuPrintf(VT100_LBL_ATTRIBUTE, Attribute);
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
        InMenuPrintf(VT100_LBL_SAVE_ATTRIBUTE);
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
        InMenuPrintf(VT100_LBL_RESTORE_ATTRIBUTE);
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
        InMenuPrintf(VT100_LBL_SET_CURSOR, PosY, PosX);
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
        InMenuPrintf(VT100_LBL_SAVE_CURSOR);
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
        InMenuPrintf(VT100_LBL_RESTORE_CURSOR);
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
        InMenuPrintf(VT100_LBL_SCROLL_ZONE, FirstLine, LastLine);
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
    if(Invert == true) InMenuPrintf(VT100_LBL_BACK_WHITE_FORE_BLACK);
    else               InMenuPrintf(VT100_LBL_BACK_BLACK_FORE_WHITE);
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
void VT100_Terminal::DisplayTimeDateStamp(uint8_t PosX, uint8_t PosY, DateAndTime_t* pTimeDate)
{
    // TODO Should use register date time printing method..

   myVT100.InMenuPrintf(PosX, PosY, VT100_LBL_FULL_DATE, myLabel.GetPointer(Label_e((LIB_GetDayOfWeek(&pTimeDate->Date)) + (int(LBL_FIRST_WEEK_DAY)))),
                                                         myLabel.GetPointer(Label_e((pTimeDate->Date.Month - 1) + (int(LBL_FIRST_MONTH)))),
                                                         pTimeDate->Date.Day,
                                                         pTimeDate->Date.Year,
                                                         pTimeDate->Time.Hour,
                                                         pTimeDate->Time.Minute,
                                                         pTimeDate->Time.Second);
/*
    InMenuPrintf(VT100_LBL_TIME_DATE_STAMP, pTimeDate->Date.Year,
                                            pTimeDate->Date.Month,
                                            pTimeDate->Date.Day,
                                            pTimeDate->Time.Hour,
                                            pTimeDate->Time.Minute,
                                            pTimeDate->Time.Second);
*/
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
        InMenuPrintf(PosX, PosY++, LBL_STRING, "┌");

        for(uint8_t i = 0; i < (H_Size - 2); i++)
        {
            InMenuPrintf(LBL_STRING, "─");
        }

        InMenuPrintf(LBL_STRING, "┐");

        for(uint8_t i = 0; i < (V_Size - 2); i++)
        {
            InMenuPrintf(PosX, PosY, LBL_STRING, "│");

            // Erase inside
            for(uint8_t j = 0; j < (H_Size - 2); j++)
            {
                InMenuPrintf(LBL_CHAR, ' ');
            }

            InMenuPrintf(PosX + (H_Size - 1), PosY++, LBL_STRING, "│");
        }

        InMenuPrintf(PosX, PosY, LBL_STRING, "└");

        for(uint8_t i = 0; i < (H_Size - 2); i++)
        {
            InMenuPrintf(LBL_STRING, "─");
        }

        InMenuPrintf(LBL_STRING, "┘");
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
            InMenuPrintf(PosX, PosY++, LBL_STRING, "│");
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

        InMenuPrintf(LBL_CHAR, ' ');
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
//void VT100_Terminal::ForceMenuRefresh(void)
//{
    //m_ForceRefresh = true;
//}

//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_VT100_MENU == DEF_ENABLED)
