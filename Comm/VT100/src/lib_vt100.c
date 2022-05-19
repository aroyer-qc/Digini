//-------------------------------------------------------------------------------------------------
//
//  File : vt100.c
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2021 Alain Royer.
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
#include "vt100.h"
#undef  VT100_GLOBAL
#define VT100_CONST_GLOBAL
#include "vt100_cfg.h"
#undef  VT100_CONST_GLOBAL
#include "uart.h"
#include "label.h"
#include "strfct.h"
//#include "cli_command.h"
#include "strfct.h"

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define CLI_CHAR_ENTER                              0x0D
#define CLI_CHAR_ESCAPE                             0x1B
#define CLI_CHAR_BACKSPACE                          0x08
#define CON_TERMINAL_SIZE                           256
#define CON_REFRESH_TIME_OUT                        10
#define CON_ESCAPE_TIME_OUT                         3
#define CON_INPUT_INVALID_ID                        -1
#define CON_LIMIT_DECIMAL_EDIT                      100000000   // Edition of decimal value limited to 100 Millions
#define TRAP_REAL_ESCAPE                            255
#define CON_MAX_TAG_SZ                              4
#define CON_PASSWORD_SZ                             16
#define CON_CMD_AT_TIME_OUT                         2000        // user has 2 seconds between character input while inputing an AT command

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

typedef struct
{
    const VT100_MenuDef_t*    pMenu;
    size_t                    pMenuSize;
} VT100_MenuObject_t;

//-------------------------------------------------------------------------------------------------
// Const(s)
//-------------------------------------------------------------------------------------------------

static const VT100_MenuObject_t VT100_Menu[NUMBER_OF_MENU] =
{
    VT100_MENU_DEF(EXPAND_VT100_MENU_AS_MENU_DATA)
};

//-------------------------------------------------------------------------------------------------
// Variable(s)
//-------------------------------------------------------------------------------------------------

static bool                 VT100_RefreshMenu;
static VT100_Menu_e         VT100_MenuID;
static VT100_Menu_e         VT100_FlushMenuID;
static VT100_InputType_e    VT100_InputType;
static uint64_t             VT100_Input;
static uint64_t             VT100_InputCount;
static bool                 VT100_ValidateInput;
static uint8_t              VT100_ItemsQts;
static bool                 VT100_BypassPrintf;
static bool                 VT100_LogsAreMuted;
static nOS_Timer            VT100_EscapeTimer;
static bool                 CON_IsDisplayLock;
static bool                 VT100_FlushNextEntry;
static char*                VT100_pHeadLabel;
static char*                VT100_pDescription;
static bool                 CON_ForceRefresh;

// Input string or decimal service
static int32_t              VT100_Minimum;
static int32_t              VT100_Maximum;
static uint8_t              VT100_PosX;
static uint8_t              VT100_PosY;
static int16_t              VT100_ID;
static int32_t              VT100_Value;
static int32_t              VT100_RefreshValue;
static int32_t              VT100_OldValue;
static uint16_t             VT100_Divider;
static bool                 VT100_InputDecimalMode;
volatile static bool        VT100_InEscapeSequence;
static int32_t              VT100_InputPtr;
static int32_t              VT100_RefreshInputPtr;
static char                 CON_String[CON_STRING_SZ + 1];
static bool                 VT100_InputStringMode;
static bool                 VT100_IsItString;

       #ifdef CONSOLE_USE_PASSWORD
         static char                 CLI_CMD_Password[CLI_PASSWORD_SZ];
       #endif

       #ifdef CONSOLE_USE_TAG
         static char                 CLI_Tag[CON_MAX_TAG_SZ];
       #endif

//-------------------------------------------------------------------------------------------------
// Private(s) Prototype(s)
//-------------------------------------------------------------------------------------------------

static void             CON_Process             (void *arg);

// CONSOLE VT100 Specific
static uint8_t          VT100_DisplayMenu         (VT100_Menu_e MenuID);
static void             VT100_MenuSelectItems     (char ItemsChar);
static VT100_InputType_e  VT100_CallBack            (VT100_InputType_e (*Callback)(uint8_t, VT100_CallBackType_e), VT100_CallBackType_e Type, uint8_t Item);
static void             VT100_EscapeCallback      (nOS_Timer* pTimer, void* pArg);
static void             VT100_InputString         (void);
static void             VT100_InputDecimal        (void);
static void             VT100_ClearConfigFLag     (void);
static void             VT100_ClearGenericString  (void);

// CLI Specific
static void             CLI_AT_Parser           (void);
static void             CLI_ParseFIFO           (uint8_t Data);

//-------------------------------------------------------------------------------------------------
// Private(s) Function(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//  Name:           CON_Process
//
//  Parameter(s):   void
//  Return:         None
//
//  Description:    Console task
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
static void CON_Process(void)
{
    const CON_MenuDef_t*  pMenu;
    uint8_t               Items;
    uint8_t               ItemsQts;
    uint32_t              Delay;
//------------ move this to init
    pMenu = NULL;

    CON_ForceRefresh = false;
    // Should read futur configuration for muting
    CON_LogsAreMuted = false;

    CON_InMenuPrintf(CON_SZ_NONE, CON_LBL_RESET_TERMINAL);
    Delay = GET_START_TIME();
    while(HAS_TIMED_OUT(Delay, 100) == false);
//--------------------


    if(FIFO_ReadyRead(&CON_FIFO_ParserRX) == true)
    {
        if(FIFO_At(&CON_FIFO_ParserRX, 0) == TRAP_REAL_ESCAPE)
        {
            FIFO_Flush(&CON_FIFO_ParserRX, 1);
            CON_GoToMenu((CON_MenuID == CON_NO_MENU) ? CON_MENU_MAIN : CON_MenuID);
        }
        else
        {
            CON_AT_Parser();
        }
    }

    if(CON_AT_ParserInfo.Step != AT_STEP_WAITING)
    {
        if(HAS_TIMED_OUT(CON_AT_ParserInfo.TimeOut, CON_CMD_AT_TIME_OUT) == true)
        {
            FIFO_Flush(&CON_FIFO_ParserRX, CON_FIFO_PARSER_RX_SZ);
            CON_InMenuPrintf(CON_SZ_NONE, "$ERROR, Command Timeout\r\n");
            CON_AT_ParserInfo.Step = AT_STEP_WAITING;
        }
    }

    if((CON_InputDecimalMode == false) && (CON_InputStringMode == false))
    {
        // Display the menu
        if(CON_RefreshMenu == true)
        {
            // Call the destructor for each callback, if any
            if((CON_FlushMenuID != CON_NO_MENU) &&
               (CON_FlushMenuID != CON_MenuID))
            {
                CON_BackFromEdition = false;
                CON_ClearConfigFLag();
                CON_ClearGenericString();

                ItemsQts = CON_Menu[CON_FlushMenuID].pMenuSize;

                for(Items = 0; Items < ItemsQts; Items++)
                {
                    pMenu = &CON_Menu[CON_FlushMenuID].pMenu[Items];
                    CON_CallBack(pMenu->Callback, CON_CALLBACK_FLUSH, Items);
                }
            }

            CON_RefreshMenu = false;
            CON_ItemsQts    = CON_DisplayMenu(CON_MenuID);
            CON_Input       = 0;
        }

        if ( CON_ForceRefresh )
        {
            CON_ItemsQts    = CON_DisplayMenu(CON_MenuID);
            CON_Input       = 0;
            CON_ForceRefresh = false;
        }

        pMenu = &CON_Menu[CON_MenuID].pMenu[CON_Input];

        // An entry have been detected, do job accordingly
        if(CON_ValidateInput == true)
        {
            CON_ValidateInput = false;

            if(CON_InputType == CON_INPUT_MENU_CHOICE)
            {
                // Validate the range for the menu
                if(CON_Input < CON_ItemsQts)
                {
                    // If new menu selection, draw this new menu
                    if(CON_MenuID != pMenu->NextMenu)
                    {
                        CON_FlushMenuID = CON_MenuID;
                        CON_MenuID      = pMenu->NextMenu;
                        CON_RefreshMenu = true;
                    }

                    // If selection has a callback, call it and react to it's configuration for key input
                    CON_InputType = CON_CallBack(pMenu->Callback, CON_CALLBACK_ON_INPUT, CON_Input);

                    // Job is already done, so no refresh
                    if(CON_InputType == CON_INPUT_MENU_CHOICE)
                    {
                        CON_Input = 0;
                    }
                }
                else
                {
                    CON_Input = 0;
                }
            }

            CON_InputCount = 0;
        }
        else
        {
            // Still in a callback mode
            if(CON_MenuID != CON_NO_MENU)
            {
                CON_CallBack(pMenu->Callback, CON_CALLBACK_REFRESH, 0);
            }
        }
    }
    else // Input decimal or string mode
    {
        if(CON_InputDecimalMode == true)   { CON_InputDecimal(); }
        if(CON_InputStringMode  == true)   { CON_InputString();  }
    }

}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CON_ClearConfigFLag
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
static void CON_ClearConfigFLag(void)
{
    for(int i = 0; i < CONFIG_FLAG_SIZE; i++)
    {
        CON_NewConfigFlag[i] = 0;
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CON_ClearGenericString
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    lear the generic string used in configuration menu
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
static void CON_ClearGenericString(void)
{
    memset(&CON_GenericString, '\0', sizeof(CON_GenericString));
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CON_DisplayMenu
//
//  Parameter(s):   CON_Menu_e      ID of the MENU
//  Return:         uint8_t         Number of items in the menu
//
//  Description:    Display selected menu
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
static uint8_t CON_DisplayMenu(CON_Menu_e MenuID)
{
    const char*             pLabel;
    const CON_MenuDef_t*    pMenu;
    const CON_MenuDef_t*    pPreviousMenu;
    uint8_t                 ItemsQts;
    uint8_t                 Items;
    char                    ItemsChar;

    pMenu = NULL;

    if(MenuID != CON_NO_MENU)
    {
        ItemsQts = CON_Menu[MenuID].pMenuSize;

        if(ItemsQts > 1)
        {
            CON_BypassPrintf = true;
            CON_InMenuPrintf(CON_SZ_NONE, CON_LBL_RESET_TERMINAL);
            nOS_Sleep(100);                                                 // Terminal need time to reset
            CON_InMenuPrintf(CON_SZ_NONE, CON_LBL_HIDE_CURSOR);
            CON_InMenuPrintf(CON_SZ_NONE, CON_LBL_CLEAR_SCREEN);
            CON_pHeadLabel = (char*)LABEL_pStr[LBL_MENU_TITLE];
            CON_SetColor(VT100_COLOR_WHITE, VT100_COLOR_BLUE);
            CON_InMenuPrintf(CON_SZ_NONE, CON_LBL_LINE_SEPARATOR);
            CON_InMenuPrintf(CON_SZ_NONE, CON_pHeadLabel);
            CON_InMenuPrintf(CON_SZ_NONE, CON_LBL_LINE_SEPARATOR);
            CON_SetColor(VT100_COLOR_YELLOW, VT100_COLOR_BLACK);
            CON_InMenuPrintf(CON_SZ_NONE, "\r\n\r\n");

            // Print all items in the menu list
            for(Items = 0; Items < ItemsQts; Items++)
            {
                pPreviousMenu = pMenu;
                pMenu         = &CON_Menu[MenuID].pMenu[Items];

                if(Items != 0)
                {
                    ItemsChar  = (char)Items;
                    ItemsChar += (ItemsChar >= 10) ? ('a' - 10) : '0';
                    CON_MenuSelectItems(ItemsChar);
                }

                pLabel = LABEL_pStr[pMenu->Label];
                CON_InMenuPrintf(CON_SZ_NONE, pLabel);

                if(Items == 0)
                {
                    CON_InMenuPrintf(CON_SZ_NONE, CON_LBL_SELECT);
                    if(pMenu != pPreviousMenu)
                    {
                        CON_CallBack(pMenu->Callback, CON_CALLBACK_INIT, 0);
                    }
                }
                else
                {
                    CON_CallBack(pMenu->Callback, CON_CALLBACK_INIT, Items);
                }
            }

            CON_MenuSelectItems('0');
            CON_InMenuPrintf(CON_SZ_NONE, CON_LBL_QUIT);
            ItemsChar  = (char)(Items - 1);
            ItemsChar += (ItemsChar >= 10) ? ('a' - 10) : '0';
            CON_InMenuPrintf(CON_SZ_NONE, CON_LBL_ENTER_SELECTION, ItemsChar);
            return ItemsQts;
        }

        // There is nothing to draw if it has only one item ( it is a redirection menu )
        CON_CallBack(CON_Menu[CON_MenuID].pMenu[0].Callback, CON_CALLBACK_INIT, 0);

        return 0;
    }

    CON_InMenuPrintf(CON_SZ_NONE, CON_LBL_CLEAR_SCREEN);
    CON_SetForeColor(VT100_COLOR_WHITE);
    CON_InputType    = CON_INPUT_CLI;
    CON_BypassPrintf = false;
    return 0;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CON_MenuSelectItems
//
//  Parameter(s):   ItemsChar   print (x)  where x is the character represented by ItemsChar
//  Return:         None
//
//  Description:    Menu Selection items print
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
static void CON_MenuSelectItems(char ItemsChar)
{
    CON_InMenuPrintf(CON_SZ_NONE, "  (");
    CON_SetForeColor(VT100_COLOR_CYAN);
    if(ItemsChar == '0') CON_InMenuPrintf(CON_SZ_NONE, "ESC");
    else                 CON_InMenuPrintf(CON_SZ_NONE, "%c", ItemsChar);
    CON_SetForeColor(VT100_COLOR_YELLOW);
    if(ItemsChar == '0') CON_InMenuPrintf(CON_SZ_NONE, ") ");
    else                 CON_InMenuPrintf(CON_SZ_NONE, ")   ");
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CON_CallBack
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
static CON_InputType_e CON_CallBack(CON_InputType_e (*Callback)(uint8_t, CON_CallBackType_e), CON_CallBackType_e Type, uint8_t Item)
{
    CON_InputType_e InputType;

    InputType = CON_INPUT_MENU_CHOICE;

    if(Callback != NULL)
    {
        CON_SaveAttribute();
        InputType = Callback(Item, Type);
        CON_RestoreAttribute();
    }

    return InputType;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CON_EscapeCallback
//
//  Parameter(s):   pTimer              Pointer on the timer that initiate this call, Not used
//                  arg                 NULL in this case
//  Return:         None
//
//  Description:    CallBack after we received a ESCAPE from terminal.
//                  If we enter here, then ESCAPE was sent alone.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
static void CON_EscapeCallback(nOS_Timer* pTimer, void* pArg)
{
    nOS_StatusReg sr;

    VAR_UNUSED(pTimer);
    VAR_UNUSED(pArg);

    nOS_EnterCritical(sr);
    CON_InEscapeSequence = false;
    CON_InputDecimalMode = false;
    CON_InputStringMode  = false;
    CON_RX_Callback(TRAP_REAL_ESCAPE);
    nOS_LeaveCritical(sr);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CON_InputDecimal
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Function that handle the decimal input mode.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
static void CON_InputDecimal(void)
{
    if(CON_RefreshValue != CON_Value)
    {
        CON_RefreshValue = CON_Value;

        if((CON_Value >= CON_Minimum) && (CON_Value <= CON_Maximum))
        {
            CON_SetColor(VT100_COLOR_BLACK, VT100_COLOR_GREEN);
        }
        else
        {
            CON_SetColor(VT100_COLOR_BLACK, VT100_COLOR_RED);
        }

        CON_SetCursorPosition(CON_PosX + 36, CON_PosY + 3);

        switch(CON_Divider)
        {
            case 10:   CON_InMenuPrintf(CON_SZ_NONE, " %ld.%d \e[40m \e[2D",   CON_Value / CON_Divider, abs(CON_Value % CON_Divider)); break;
            case 100:  CON_InMenuPrintf(CON_SZ_NONE, " %ld.%02d \e[40m \e[2D", CON_Value / CON_Divider, abs(CON_Value % CON_Divider)); break;
            case 1000: CON_InMenuPrintf(CON_SZ_NONE, " %ld.%03d \e[40m \e[2D", CON_Value / CON_Divider, abs(CON_Value % CON_Divider)); break;
            default:   CON_InMenuPrintf(CON_SZ_NONE, " %ld \e[40m \e[2D",      CON_Value);                                             break;
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CON_InputString
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Function that handle the string input mode.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
static void CON_InputString(void)
{
    if(CON_RefreshInputPtr != CON_InputPtr)
    {
        CON_SetCursorPosition(CON_PosX + 2, CON_PosY + 3);
        CON_RefreshInputPtr = CON_InputPtr;
        CON_InMenuPrintf(CON_SZ_NONE, "%s \e[D", CON_String);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CON_AT_Parser
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Preprocessing line of data to trap ERROR or OK or continue for data
//
//  Note(s):        We let the callback decide if structure pCommandPool should be put in waiting
//                  for process to complete.
//
//-------------------------------------------------------------------------------------------------
static void CON_AT_Parser(void)
{
    int16_t          CmdNameSz;
    CON_CmdName_e    Cmd;
    CON_AT_Error_e   Error;

  #ifdef CONSOLE_TAG_SUPPORT
    uint16_t         Size;
    char             ReadChar;
    int              TagCount;
  #endif

    if(CON_AT_ParserInfo.Step == AT_STEP_CMD_VALID)
    {
        CmdNameSz = CON_AT_ParserInfo.CmdNameSz;
        FIFO_ToUpper(&CON_FIFO_ParserRX, CmdNameSz);
        Error = CON_AT_INVALID_COMMAND;

        // Process the valid input by iterating through valid command list
        for(Cmd = FIRST_CON_COMMAND; Cmd < NUMBER_OF_CON_CMD; Cmd++)
        {
            // First size must match
            if(CmdNameSz == CON_CmdStrSz[Cmd])
            {
                // Check Command name
                if(FIFO_Memncmp(&CON_FIFO_ParserRX, CON_pCmdStr[Cmd], CmdNameSz) == true)
                {
                    // Flush the command name from the Fifo
                    FIFO_Flush(&CON_FIFO_ParserRX, CmdNameSz);

                    // Command is valid

                  #ifdef CONSOLE_TAG_SUPPORT
                    // TODO TAG support is not converted YET
                    // TAG handling
                    if(FIFO_At(&CON_FIFO_ParserRX, Size) == '+')
                    {
                        TagCount = 0;
                        Size     = 0;
                        memset(&CON_Tag[0], '\0', CON_MAX_TAG_SZ);

                        FIFO_Flush(&CON_FIFO_ParserRX, 1);

                        for(int i = 0; (i < CON_MAX_TAG_SZ) && (TagCount == i); i++)
                        {
                            if((ReadChar = FIFO_At(&CON_FIFO_ParserRX, Size + i)) != '=')
                            {
                                TagCount++;
                                HOST_API_Tag[Cmd] = ReadChar;
                            }
                        }

                        CmdNameSz += TagCount;

                        // If next character is not '=', it is also a TAG Error
                        if(FIFO_At(&CON_FIFO_ParserRX, CmdNameSz) != '=')
                        {
                            STR_memset(&HOST_API_Tag[0], '\0', CON_MAX_TAG_SZ);
                            CON_SendAnswer(Cmd, CON_TAG_ERROR, NULL);
                            FIFO_Flush(&CON_FIFO_ParserRX, Size);
                            return;
                        }
                    }
                  #endif

                    // Callback to the command handler
                    Error = CON_AT_Function[Cmd]();
                    CON_SendAnswer(Cmd, Error, NULL);
                }
            }
        }

        if(Error == CON_AT_INVALID_COMMAND)
        {
            CON_InMenuPrintf(CON_SZ_NONE, "$ERROR, Command Invalid\r\n");
        }

        FIFO_Flush(&CON_FIFO_ParserRX, CON_FIFO_PARSER_RX_SZ);      // flush completely the FIFO
        CON_AT_ParserInfo.Step = AT_STEP_WAITING;
    }
    else if(CON_AT_ParserInfo.Step == AT_STEP_CMD_MALFORMED)
    {
        CON_InMenuPrintf(CON_SZ_NONE, "$ERROR, Malformed Packet\r\n");
        CON_AT_ParserInfo.Step = AT_STEP_WAITING;
    }
    else if(CON_AT_ParserInfo.Step == AT_STEP_CMD_BUFFER_OVERFLOW)
    {
        CON_InMenuPrintf(CON_SZ_NONE, "$ERROR, Buffer Overflow\r\n");
        CON_AT_ParserInfo.Step = AT_STEP_WAITING;
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CON_HandleCmdPassword
//
//  Parameter(s):   CON_AT_Error_e*     pError  Type of error on the password
//  Return:         CON_CmdRequest_e    AT_REQUEST_PASSWORD_OK
//                                      AT_REQUEST_ERROR
//
//  Description:    Check if password is valid
//
//  Note(s):        if a password exist this function will check:
//                     - if not valid it return false,
//                     - otherwise it return true if valid or not set
//
//-------------------------------------------------------------------------------------------------
#ifdef CONSOLE_USE_PASSWORD
static CON_CmdRequest_e CON_HandleCmdPassword(CON_AT_Error_e* pError)
{
    *pError = HOST_API_AT_OK;

    if(CON_CMD_Password[0] == '\0')
    {
        return AT_REQUEST_PASSWORD_OK;
    }

    /// Need a password
    for(int i = 0; i <= sizeof(CON_CMD_Password); i++)
    {
        // We need a comma somewhere span in the size of the Command password range
        if(FIFO_At(&CON_FIFO_ParserRX, i) == COMMA)
        {
            // Check if password match with internal password
            if(FIFO_Memncmp(&CON_FIFO_ParserRX, &CON_CMD_Password[0], i) == true)
            {
                if(CON_CMD_Password[i] == '\0')
                {
                    i++;
                    FIFO_Flush(&CON_FIFO_ParserRX, i);
                    return AT_REQUEST_PASSWORD_OK;
                }
                else
                {
                    *pError = CON_AT_INVALID_PASSWORD;
                    return AT_REQUEST_ERROR;
                }
            }
            else
            {
                *pError = CON_AT_INVALID_PASSWORD;
                return AT_REQUEST_ERROR;
            }
        }
    }

    *pError = CON_AT_INVALID_PASSWORD;
    return AT_REQUEST_ERROR;
}
#endif

//-------------------------------------------------------------------------------------------------
//
//  Name:           CON_ParseFIFO
//
//  Parameter(s):   uint8_t Data
//  Return:         None
//
//  Description:    Parse the fifo
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
static void CON_ParseFIFO(uint8_t Data)
{
    if(CON_AT_ParserInfo.Step != AT_STEP_CMD_VALID)
    {
        /// **************************************************
        /// Waiting to process incoming AT Command
        if(CON_AT_ParserInfo.Step != AT_STEP_GETTING_DATA)
        {
            if(((char)Data == '$') && (CON_AT_ParserInfo.Step == AT_STEP_T_DETECTED))      // we receive more data than AT header so, this accelerate process in IRQ
            {
                CON_AT_ParserInfo.Step       = AT_STEP_GETTING_DATA;
                CON_AT_ParserInfo.ReadCmd    = false;
                CON_AT_ParserInfo.PlainCmd   = true;
                CON_AT_ParserInfo.CmdNameSz  = 0;
              #ifdef CONSOLE_TAG_SUPPORT
                CON_AT_ParserInfo.TagSz      = 0;
                CON_AT_ParserInfo.TagPresent = false;
              #endif
            }
            else if((((char)Data == 'a') || ((char)Data == 'A')) && (CON_AT_ParserInfo.Step == AT_STEP_WAITING))
            {
                CON_AT_ParserInfo.Step = AT_STEP_A_DETECTED;
            }
            else if((((char)Data == 't') || ((char)Data == 'T')) && (CON_AT_ParserInfo.Step == AT_STEP_A_DETECTED))
            {
                CON_AT_ParserInfo.Step = AT_STEP_T_DETECTED;
            }
            else
            {
                CON_AT_ParserInfo.Step = AT_STEP_WAITING;
            }

            CON_AT_ParserInfo.RX_Offset = 0;
        }
        /// **************************************************
        /// We are receiving a AT command
        ///
        /// Note: we do a basic parsing of the command
        else
        {
            if((char)Data == '=')
            {
              CON_AT_ParserInfo.PlainCmd = false;

              #ifdef CONSOLE_TAG_SUPPORT
                if(CON_AT_ParserInfo.TagPresent == true)    // if a command tag exist this is the tag size if valid in size
                {
                    if(((CON_AT_ParserInfo.RX_Offset - CON_AT_ParserInfo.CmdNameSz) < CON_MAX_TAG_SZ) &&
                        (CON_AT_ParserInfo.RX_Offset != CON_AT_ParserInfo.CmdNameSz))
                    {
                        CON_AT_ParserInfo.TagSz = CON_AT_ParserInfo.RX_Offset;
                    }
                    else
                    {
                        CON_AT_ParserInfo.Step = AT_STEP_CMD_MALFORMED;
                    }
                }
                else                                        // If no Tag exist this is the command size
              #endif
                {
                    if(CON_AT_ParserInfo.RX_Offset != 0)
                    {
                        CON_AT_ParserInfo.CmdNameSz = CON_AT_ParserInfo.RX_Offset;
                    }
                    else
                    {
                        CON_AT_ParserInfo.Step = AT_STEP_CMD_MALFORMED;
                    }
                }
            }
            else if((char)Data == '?')
            {
                // '?' must follow '='
                if(CON_AT_ParserInfo.CmdNameSz == CON_AT_ParserInfo.RX_Offset)
                {
                    CON_AT_ParserInfo.CmdNameSz = CON_AT_ParserInfo.RX_Offset;
                    CON_AT_ParserInfo.ReadCmd   = true;
                }
                else // Malformed packet
                {
                    CON_AT_ParserInfo.Step = AT_STEP_CMD_MALFORMED;
                }
            }
          #ifdef CONSOLE_TAG_SUPPORT
            else if((char)Data == '+')
            {
                if((CON_AT_ParserInfo.RX_Offset != 0)
                {
                    CON_AT_ParserInfo.CmdNameSz  = CON_AT_ParserInfo.RX_Offset;
                    CON_AT_ParserInfo.TagPresent = true;
                }
                else
                {
                    CON_AT_ParserInfo.Step = AT_STEP_CMD_MALFORMED;
                }
            }
          #endif
            else if((char)Data == '\r')
            {
                if(CON_AT_ParserInfo.CmdNameSz == 0)
                {
                    CON_AT_ParserInfo.CmdNameSz = CON_AT_ParserInfo.RX_Offset;
                }

                CON_AT_ParserInfo.DataSz = CON_AT_ParserInfo.RX_Offset;
                CON_AT_ParserInfo.Step   = AT_STEP_CMD_VALID;
            }
            else if((char)Data == '\b')
            {
                CON_AT_ParserInfo.RX_Offset--;

              #ifdef CONSOLE_TAG_SUPPORT
                if(CON_AT_ParserInfo.TagPresent == true)
                {
                    if(CON_AT_ParserInfo.TagSz == CON_AT_ParserInfo.RX_Offset)
                    {
                        CON_AT_ParserInfo.TagSz = 0;
                    }
                    else if(CON_AT_ParserInfo.CmdNameSz == CON_AT_ParserInfo.RX_Offset)
                    {
                        CON_AT_ParserInfo.TagPresent = false;
                        CON_AT_ParserInfo.CmdNameSz  = 0;
                    }
                }
              #else
                if(CON_AT_ParserInfo.CmdNameSz == CON_AT_ParserInfo.RX_Offset)
                {
                    CON_AT_ParserInfo.CmdNameSz = 0;
                }
              #endif

              FIFO_HeadBackward(&CON_FIFO_ParserRX, 1);
            }
            else
            {
                // Write data into the Fifo buffer

                // Check if we can write into the Fifo
                if(FIFO_Write(&CON_FIFO_ParserRX, (const void*)&Data, 1) != 1)
                {
                    // We have overrun the Fifo buffer
                    CON_AT_ParserInfo.Step = AT_STEP_CMD_BUFFER_OVERFLOW;
                }
                else
                {
                    CON_AT_ParserInfo.RX_Offset++;
                }
            }

            // An Error occurred then we flush the Fifo
            if((CON_AT_ParserInfo.Step != AT_STEP_GETTING_DATA) &&
               (CON_AT_ParserInfo.Step != AT_STEP_CMD_VALID))
            {
                FIFO_Flush(&CON_FIFO_ParserRX, CON_FIFO_PARSER_RX_SZ);
                // Parser will send Error and reset the Step for next acquisition cycle
            }
        }
    }

    if(CON_AT_ParserInfo.Step != AT_STEP_WAITING)
    {
        CON_AT_ParserInfo.TimeOut = GET_START_TIME();
    }
}

//-------------------------------------------------------------------------------------------------
// Public(s) Function(s)
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//
//  Name:           CON_Initialize
//
//  Parameter(s):   const char* pHeadLabel
//  Return:         const char* pDescription
//
//  Description:    Initialize console
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void CON_Initialize(const char* pHeadLabel, const char* pDescription)
{
    UART_Initialize(UART_CONSOLE);
    CON_pHeadLabel            = (char*)pHeadLabel;
    CON_pDescription          = (char*)pDescription;
    CON_RefreshMenu           = false;
    CON_MenuID                = CON_NO_MENU;
    CON_FlushMenuID           = CON_NO_MENU;
    CON_InputType             = CON_INPUT_CLI;
    CON_Input                 = 0;
    CON_InputCount            = 0;
    CON_ValidateInput         = false;
    CON_ItemsQts              = 0;
    CON_BypassPrintf          = false;
    CON_ID                    = -1;
    CON_InputDecimalMode      = false;
    CON_InEscapeSequence      = false;
    CON_IsDisplayLock         = false;
    CON_BackFromEdition       = false;
    CON_FlushNextEntry        = false;
    CON_LogsAreMuted          = true;
    CON_String[CON_STRING_SZ] = '\0';

    nOS_TimerCreate(&CON_EscapeTimer, CON_EscapeCallback, NULL, CON_ESCAPE_TIME_OUT, NOS_TIMER_ONE_SHOT);

    nOS_ThreadCreate(&CON_ThreadHandle, CON_Task, NULL, CON_Stack, CON_STACK_SIZE, CON_TASK_PRIO);
    CON_CallbackInitialize();
    CON_CLI_Initialize();
    CON_AT_ParserInfo.RX_Offset = 0;
    CON_AT_ParserInfo.Step      = AT_STEP_WAITING;

    FIFO_Init(&CON_FIFO_ParserRX, &CON_BufferParserRX[0], CON_FIFO_PARSER_RX_SZ);
    CON_ClearConfigFLag();
    CON_ClearGenericString();
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CON_DisplayDescription
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:     Clear the screen and Display the header line
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void CON_DisplayDescription(void)
{
    CON_InMenuPrintf(CON_SZ_NONE, CON_LBL_CLEAR_SCREEN);
    CON_SetCursorPosition(10, 1);
    CON_InMenuPrintf(CON_SZ_NONE, "%s\r\n\r\n\r\n", CON_pDescription);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CON_PrintSaveLabel
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
void CON_PrintSaveLabel(uint8_t PosX, uint8_t PosY, VT100_Color_e Color)
#else
void CON_PrintSaveLabel(uint8_t PosX, uint8_t PosY)
#endif
{
    CON_SetForeColor(Color);
    CON_SetCursorPosition(PosX, PosY);
    CON_InMenuPrintf(CON_SZ_NONE, CON_LBL_SAVE_CONFIGURATION);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CON_SetDecimalInput
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
void CON_SetDecimalInput(uint8_t PosX, uint8_t PosY, int32_t Minimum, int32_t Maximum, int32_t Value, uint16_t Divider, uint8_t ID, const char* pMsg)
{
    CON_Minimum          = Minimum;
    CON_Maximum          = Maximum;
    CON_PosX             = PosX;
    CON_PosY             = PosY;
    CON_ID               = (int16_t)ID;
    CON_Value            = Value;
    CON_RefreshValue     = Value + 1;       // To force the first refresh
    CON_OldValue         = Value;
    CON_Divider          = Divider;
    CON_InputDecimalMode = true;
    CON_IsItString       = false;

    // Draw the Box
    CON_DrawBox(PosX, PosY, 48, 8, VT100_COLOR_WHITE);

    // Write input information
    CON_SetForeColor(VT100_COLOR_CYAN);
    CON_SetCursorPosition(PosX + 2,  PosY + 1);

    switch(Divider)
    {
        case 10:  CON_InMenuPrintf(CON_SZ_NONE, "%s : %ld.%d",   CON_LBL_MINIMUM, Minimum / Divider,  abs(Minimum % Divider)); break;
        case 100: CON_InMenuPrintf(CON_SZ_NONE, "%s : %ld.%02d", CON_LBL_MINIMUM, Minimum / Divider,  abs(Minimum % Divider)); break;
        case 1000:CON_InMenuPrintf(CON_SZ_NONE, "%s : %ld.%03d", CON_LBL_MINIMUM, Minimum / Divider,  abs(Minimum % Divider)); break;
        default:  CON_InMenuPrintf(CON_SZ_NONE, "%s : %ld",      CON_LBL_MINIMUM, Minimum);                                    break;
    }

    CON_SetCursorPosition(PosX + 24, PosY + 1);

    switch(Divider)
    {
        case 10:  CON_InMenuPrintf(CON_SZ_NONE, "%s : %ld.%d",   CON_LBL_MAXIMUM, Maximum / Divider,  abs(Maximum % Divider)); break;
        case 100: CON_InMenuPrintf(CON_SZ_NONE, "%s : %ld.%02d", CON_LBL_MAXIMUM, Maximum / Divider,  abs(Maximum % Divider)); break;
        case 1000:CON_InMenuPrintf(CON_SZ_NONE, "%s : %ld.%03d", CON_LBL_MAXIMUM, Maximum / Divider,  abs(Maximum % Divider)); break;
        default:  CON_InMenuPrintf(CON_SZ_NONE, "%s : %ld",      CON_LBL_MAXIMUM, Maximum);                                    break;
    }

    // Print type of input
    CON_SetForeColor(VT100_COLOR_YELLOW);
    CON_SetCursorPosition(PosX + 2, PosY + 3);
    CON_InMenuPrintf(CON_SZ_NONE, "%s", pMsg);

    // Add 'how to' info
    CON_SetCursorPosition(PosX + 2,  PosY + 5);
    CON_InMenuPrintf(CON_SZ_NONE, CON_LBL_INPUT_VALIDATION);

    // And show cursor
    CON_InMenuPrintf(CON_SZ_NONE, CON_LBL_SHOW_CURSOR);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CON_GetDecimalInputValue
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
void CON_GetDecimalInputValue(uint32_t* pValue, uint8_t* pID)
{
    uint8_t  ID    = 0;
    uint32_t Value = CON_OldValue;

    if(CON_ID != CON_INPUT_INVALID_ID)
    {
        if(CON_IsItString == false)
        {
            if((CON_Value >= CON_Minimum) && (CON_Value <= CON_Maximum))
            {
                ID    = CON_ID;
                Value = CON_Value;
            }
            CON_InputDecimalMode = false;
            CON_ID = CON_INPUT_INVALID_ID;   // We can read this only once
        }
    }

    *pID    = (uint8_t)ID;
    *pValue = Value;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CON_SetStringInput
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
void CON_SetStringInput(uint8_t PosX, uint8_t PosY, int32_t Maximum, uint8_t ID, const char* pMsg, const char* pString)
{
    CON_Maximum          = Maximum;
    CON_PosX             = PosX;
    CON_PosY             = PosY;
    CON_ID               = (int16_t)ID;
    CON_InputStringMode  = true;
    CON_IsItString       = true;

    // Draw the Box
    CON_DrawBox(PosX, PosY, 46, 7, VT100_COLOR_WHITE);

    // Write input information
    CON_SetForeColor(VT100_COLOR_CYAN);
    CON_SetCursorPosition(PosX + 2,  PosY + 1);
    CON_InMenuPrintf(Maximum, "%s", pMsg);

    // Add 'how to' info
    CON_SetCursorPosition(PosX + 2,  PosY + 5);
    CON_InMenuPrintf(CON_SZ_NONE, CON_LBL_INPUT_VALIDATION);

    // Copy string
    memcpy(CON_String, pString, CON_STRING_SZ);
    STR_strnstrip(CON_String, Maximum);
    CON_InputPtr = strlen(CON_String);                          // Get string end pointer
    CON_RefreshInputPtr = CON_InputPtr + 1;                     // To force a refresh

    // And show cursor
    CON_InMenuPrintf(CON_SZ_NONE, CON_LBL_SHOW_CURSOR);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CON_GetStringInput
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
void CON_GetStringInput(char* pString, uint8_t* pID)
{
    uint8_t ID;

    ID = 0;

    if(CON_ID != CON_INPUT_INVALID_ID)
    {
        if(CON_IsItString == true)
        {
            if(strlen(CON_String) <= CON_Maximum)
            {
                ID = CON_ID;
                memcpy(pString, CON_String, CON_STRING_SZ);
            }
            CON_InputStringMode = false;
            CON_ID = CON_INPUT_INVALID_ID;             // We can read this only once
        }
    }

    *pID = (uint8_t)ID;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CON_InMenuPrintf
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
size_t CON_InMenuPrintf(int nSize, const char* pFormat, ...)
{
    va_list vaArg;
    char*   Buffer;
    size_t  Size    = 0;

    if((pBuffer = nOS_MemAlloc(&BSP_MemBlock, NOS_NO_WAIT) != NULL)
    {
        va_start(vaArg, (const char*)pFormat);
        Size = STR_vsnprintf(pBuffer, ((nSize == CON_SZ_NONE) ? CON_TERMINAL_SIZE : nSize), pFormat, vaArg);
        UART_Write(UART_CONSOLE, &Buffer[0], Size);
        va_end(vaArg);
        nOS_MemFree(&BSP_MemBlock, (void*)Buffer);
    }

    return Size;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CON_LoggingPrintf
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
size_t CON_LoggingPrintf(CON_DebugLevel_e Level, const char* pFormat, ...)
{
    va_list          vaArg;
    char*            pBuffer;
    CON_DebugLevel_e DebugLevel;
    size_t           Size = 0;

    if((CON_BypassPrintf == false) && (CON_LogsAreMuted == false))
    {
        //// SYS_Read(SYS_DEBUG_LEVEL, MAIN_ACU, 0, &DebugLevel, NULL);
        if((DebugLevel & Level) != 0)
        {
            if((pBuffer = nOS_MemAlloc(&BSP_MemBlock, NOS_NO_WAIT) != NULL)
            {
                va_start(vaArg, (const char*)pFormat);
                Size = STR_vsnprintf(pBuffer, CON_TERMINAL_SIZE, pFormat, vaArg);
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
//  Name:           CON_GoToMenu
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
void CON_GoToMenu(CON_Menu_e MenuID)
{
    CON_RefreshMenu = true;
    CON_FlushMenuID = CON_MenuID;
    CON_MenuID      = MenuID;
    CON_InputType   = CON_INPUT_MENU_CHOICE;
    CON_Input       = 0;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CON_ConsoleState
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
bool CON_ConsoleState(void)
{
    return (CON_MenuID == CON_NO_MENU) ? false : true;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CON_SetConsoleMuteLogs
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
void CON_SetConsoleMuteLogs(bool Mute)
{
    CON_LogsAreMuted = Mute;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CON_SetAttribute
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
void CON_SetAttribute(CON_VT100_Attribute_e Attribute)
{
    if(CON_IsDisplayLock == false)
    {
        CON_InMenuPrintf(CON_SZ_NONE, CON_LBL_ATTRIBUTE, Attribute);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CON_SaveAttribute
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
void CON_SaveAttribute(void)
{
    if(CON_IsDisplayLock == false)
    {
        CON_InMenuPrintf(CON_SZ_NONE, CON_LBL_SAVE_ATTRIBUTE);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CON_RestoreAttribute
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
void CON_RestoreAttribute(void)
{
    if(CON_IsDisplayLock == false)
    {
        CON_InMenuPrintf(CON_SZ_NONE, CON_LBL_RESTORE_ATTRIBUTE);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CON_SetCursorPosition
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
void CON_SetCursorPosition(uint8_t PosX, uint8_t PosY)
{
    if(CON_IsDisplayLock == false)
    {
        CON_InMenuPrintf(CON_SZ_NONE, CON_LBL_SET_CURSOR, PosY, PosX);
    }
}


//-------------------------------------------------------------------------------------------------
//
//  Name:           CON_SaveCursorPosition
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
void CON_SaveCursorPosition(void)
{
    if(CON_IsDisplayLock == false)
    {
        CON_InMenuPrintf(CON_SZ_NONE, CON_LBL_SAVE_CURSOR);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CON_RestoreCursorPosition
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
void CON_RestoreCursorPosition(void)
{
    if(CON_IsDisplayLock == false)
    {
        CON_InMenuPrintf(CON_SZ_NONE, CON_LBL_RESTORE_CURSOR);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CON_SetScrollZone
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
void CON_SetScrollZone(uint8_t FirstLine, uint8_t LastLine)
{
    if(CON_IsDisplayLock == false)
    {
        CON_InMenuPrintf(CON_SZ_NONE, CON_LBL_SCROLL_ZONE, FirstLine, LastLine);
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CON_SetColor
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
void CON_SetColor(VT100_Color_e ForeColor, VT100_Color_e BackColor)
{
    if(CON_IsDisplayLock == false)
    {
        CON_SetAttribute((CON_VT100_Attribute_e)ForeColor + CON_OFFSET_COLOR_FOREGROUND);
        CON_SetAttribute((CON_VT100_Attribute_e)BackColor + CON_OFFSET_COLOR_BACKGROUND);
    }
}
#endif

//-------------------------------------------------------------------------------------------------
//
//  Name:           CON_InvertMono
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
void CON_InvertMono(bool Invert)
{
    if(Invert == true) CON_InMenuPrintf(CON_SZ_NONE, "\e[30m\e[47m");
    else               CON_InMenuPrintf(CON_SZ_NONE, "\e[37m\e[40m");
}
#endif

//-------------------------------------------------------------------------------------------------
//
//  Name:           CON_LockDisplay
//
//  Parameter(s):   bool       State        if true, Display is loccked.
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
void CON_LockDisplay(bool State)
{
    CON_IsDisplayLock = State;
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
void CON_DisplayTimeDateStamp(nOS_TimeDate* pTimeDate)
{
    CON_InMenuPrintf(CON_SZ_NONE, CON_LBL_TIME_DATE_STAMP, pTimeDate->year,
                                                           pTimeDate->month,
                                                           pTimeDate->day,
                                                           pTimeDate->hour,
                                                           pTimeDate->minute,
                                                           pTimeDate->second);
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CON_DrawBox
//
//  Parameter(s):   uint8_t         PosX                Position x on screen.
//                  uint8_t         PosY                Position y on screen.
//                  uint8_t         H_Size              horizontal size in of the box.
//                  uint8_t         V_Size              vertical size of the box.
//                  VT100_Color_e     ForeColor           Color of the box.
//
//  Return:         None
//
//  Description:    Draw a box at specified attribute.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void CON_DrawBox(uint8_t PosX, uint8_t PosY, uint8_t H_Size, uint8_t V_Size, VT100_Color_e ForeColor)
{
    if(CON_IsDisplayLock == false)
    {
        CON_SetForeColor(ForeColor);
        CON_SetCursorPosition(PosX, PosY++);
        CON_InMenuPrintf(CON_SZ_NONE, "╔");

        for(uint8_t i = 0; i < (H_Size - 2); i++)
        {
            CON_InMenuPrintf(CON_SZ_NONE, "�?");
        }

        CON_InMenuPrintf(CON_SZ_NONE, "╗");

        for(uint8_t i = 0; i < (V_Size - 2); i++)
        {
            CON_SetCursorPosition(PosX, PosY);

            CON_InMenuPrintf(CON_SZ_NONE, "║");

            // Erase inside
            for(uint8_t j = 0; j < (H_Size - 2); j++)
            {
                CON_InMenuPrintf(CON_SZ_NONE, " ");
            }

            CON_SetCursorPosition(PosX + (H_Size - 1), PosY++);
            CON_InMenuPrintf(CON_SZ_NONE, "║");
        }

        CON_SetCursorPosition(PosX, PosY);
        CON_InMenuPrintf(CON_SZ_NONE, "╚");

        for(uint8_t i = 0; i < (H_Size - 2); i++)
        {
            CON_InMenuPrintf(CON_SZ_NONE, "�?");
        }
        CON_InMenuPrintf(CON_SZ_NONE, "�?");
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CON_DrawBox
//
//  Parameter(s):   uint8_t         PosX                Position X on screen.
//                  uint8_t         PosY                Position Y on screen.
//                  uint8_t         V_Size              vertical size of line.
//                  VT100_Color_e     ForeColor           Color of the box.
//
//  Return:         None
//
//  Description:    Draw a line at specified attribute.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void CON_DrawVline(uint8_t PosX, uint8_t PosY, uint8_t V_Size, VT100_Color_e ForeColor)
{
    if(CON_IsDisplayLock == false)
    {
        CON_SetForeColor(ForeColor);

        for(uint8_t i = 0; i < V_Size; i++)
        {
            CON_SetCursorPosition(PosX, PosY++);
            CON_InMenuPrintf(CON_SZ_NONE, "║");
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CON_Bargraph
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
void CON_Bargraph(uint8_t PosX, uint8_t PosY, VT100_Color_e Color, uint8_t Value, uint8_t Max, uint8_t Size)
#else
void CON_Bargraph(uint8_t PosX, uint8_t PosY, uint8_t Value, uint8_t Max, uint8_t Size)
#endif
{
    int i;

    CON_SetCursorPosition(PosX, PosY);

    for(i = 0; i < Size; i++)
    {
      #ifdef CONSOLE_USE_COLOR
        if(i < ((Value / (Max / Size))))
        {
            CON_SetBackColor(Color);
        }
        else
        {
            CON_SetBackColor(VT100_COLOR_BLACK);
        }
      #else
        CON_InvertMono((i < ((Value / (Max / Size)))));
      #endif

        CON_InMenuPrintf(CON_SZ_NONE, " ");
    }

  #ifdef CONSOLE_USE_COLOR
    CON_SetAttribute(VT100_COLOR_BLACK + CON_OFFSET_COLOR_BACKGROUND);
  #else
    CON_InvertMono(false);
  #endif
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CON_CLI_CmdCONSOLE
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
CON_AT_Error_e CON_CLI_CmdCONSOLE(void)
{
    CON_GoToMenu((CON_MenuID == CON_NO_MENU) ? CON_MENU_MAIN : CON_MenuID);
    return CON_AT_OK_SILENT;
}
#endif

//-------------------------------------------------------------------------------------------------
//
//  Name:           CON_SendAnswer
//
//  Parameter(s):   CON_CmdName_e   CommandID           Command identification.
//                  CON_AT_Error_e  Error               Error code.
//                  const char*     Answer              Specific error string if not NULL.
//
//  Return:         None
//
//  Description:    Send error line to terminal.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
void CON_SendAnswer(CON_CmdName_e Cmd, CON_AT_Error_e Error, const char* Answer)
{
    const char* pMsg1 = "NACK";

    const char* pMsg2 = "";

    switch(Error)   // print error message
    {
        case CON_AT_OK:                     { pMsg1 = "OK";                 break; }
        case CON_AT_OK_READ:                { pMsg1 = "";
                                              pMsg2 = Answer;
                                              break;
                                            }

      #ifdef CONSOLE_USE_EXTENDED_ERROR
        case CON_AT_OK_DENIED:              { pMsg2 = ",Denied";            break; }
        case CON_AT_CMD_NO_READ_SUPPORT:    { pMsg2 = ",No Read Support";   break; }
        case CON_AT_CMD_NO_WRITE_SUPPORT:   { pMsg2 = ",No Write Support";  break; }
        case CON_AT_INVALID_PARAMETER:      { pMsg2 = ",Invalid Parameter"; break; }
        case CON_AT_TAG_FORMAT_INVALID:     { pMsg2 = ",TAG Format Invalid";break; }
        case CON_AT_INVALID_PASSWORD:       { pMsg2 = ",Password Invalid";  break; }
        case CON_AT_TAG_ERROR:              { pMsg2 = ",TAG Error";         break; }
      #else
        case CON_AT_OK_DENIED:
        case CON_AT_CMD_NO_READ_SUPPORT:
        case CON_AT_CMD_NO_WRITE_SUPPORT:
        case CON_AT_INVALID_PARAMETER:
        case CON_AT_TAG_FORMAT_INVALID:
        case CON_AT_INVALID_PASSWORD:
        case CON_AT_TAG_ERROR:              { break; }
      #endif

        case CON_AT_OK_SILENT:           // We return nothing.. it is silent
        case CON_AT_INVALID_COMMAND:     // <- This case cannot append here
                                            { pMsg1 = "";                   break; }
    }

    if(Error != CON_AT_OK_SILENT)
    {
      #ifdef TAG_SUPPORT
        if((Size = strnlen(&CON_Tag[0], MAX_TAG_SZ)) != 0)
        {
            CON_InMenuPrintf(CON_SZ_NONE, "$%s+%s=%s%s\r\n", CON_pCmdStr[Cmd], &CON_Tag[0], pMsg1, pMsg2);
        }
        else
      #endif
        {
            CON_InMenuPrintf(CON_SZ_NONE, "$%s=%s%s\r\n", CON_pCmdStr[Cmd], pMsg1, pMsg2);
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CON_GetString
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
bool CON_GetString(char* pBuffer, size_t Size)
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
//  Name:           CON_CheckForComma
//
//  Parameter(s):   None
//
//  Return:         true or false
//
//  Description:    Check if next character is a comma, and extract it if true, also adjust the
//                  size in command pool structure.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
bool CON_CheckForComma(void)
{
    if(FIFO_At(&CON_FIFO_ParserRX, 0) == ',')
    {
        FIFO_Flush(&CON_FIFO_ParserRX, 1);
        return true;
    }

    return false;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CON_GetAtoi
//
//  Parameter(s):   int32_t*    pValue          Value will be returned here.
//                  int32_t     Min             Minimum accepted value.
//                  int32_t     Max             Maximum accepted value.
//                  int8_t      Base            Base of the extraction.
//                                                  - DECIMAL_BASE
//                                                  - HEXADECIMAL_BASE
//
//  Return:         true or false
//
//  Description:    Get the integer value from the FIFO.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
bool CON_GetAtoi(int32_t* pValue, int32_t Min, int32_t Max, uint8_t Base)
{
    uint32_t Size;
    bool     Result = false;

    Size = FIFO_Atoi(&CON_FIFO_ParserRX, pValue, Base);

    if(Size != 0)
    {
        if((*pValue >= Min) && (*pValue <= Max))
        {
            Result = true;
        }
    }

    return Result;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CON_CheckForEOL
//
//  Parameter(s):   None
//
//  Return:         true or false
//
//  Description:    Check to be sure there no more parameter on the command line.
//
//  Note(s):
//
//-------------------------------------------------------------------------------------------------
bool CON_CheckForEOL(void)
{
    if(FIFO_ReadyRead(&CON_FIFO_ParserRX) == false)
    {
        return true;
    }

    return false;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CON_RX_Callback
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
void CON_RX_Callback(uint8_t Data)
{
    if(CON_FlushNextEntry == true)
    {
        CON_FlushNextEntry = false;
        return;
    }

    if(CON_InEscapeSequence == true)
    {
        CON_InEscapeSequence = false;
        nOS_TimerStop(&CON_EscapeTimer, true);
        CON_ValidateInput  = false;
        CON_Input          = 0;
        CON_FlushNextEntry = true;
    }

    if(Data == CON_CHAR_ESCAPE)
    {
        nOS_TimerRestart(&CON_EscapeTimer, CON_ESCAPE_TIME_OUT);
        CON_InEscapeSequence = true;
        CON_Value            = CON_OldValue;
        return;
    }

    switch(CON_InputType)
    {
        case CON_INPUT_MENU_CHOICE:
        {
            CON_InputCount++;
            CON_ValidateInput = true;

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
                CON_Input         = 0;
                CON_InputCount    = 0;
                CON_ValidateInput = false;
                return;
            }
            if(Data == TRAP_REAL_ESCAPE)
            {
                Data = 0;
            }
            CON_Input = Data;
            break;
        }

        case CON_INPUT_DECIMAL:
        {
            if(Data == CON_CHAR_ENTER)
            {
                CON_BackFromEdition  = true;
                CON_InputDecimalMode = false;
                CON_GoToMenu(CON_MenuID);
            }
            else if(Data == CON_CHAR_BACKSPACE)
            {
                CON_Value /= 10;
            }
            else if((Data >= '0') && (Data <= '9'))
            {
                if(((abs(CON_Value * 10)) + (Data - '0')) <= CON_LIMIT_DECIMAL_EDIT)
                {
                    CON_InputCount++;
                    CON_Value *= 10;
                    if(CON_Value < 0) CON_Value -= Data - '0';
                    else              CON_Value += Data - '0';
                }
            }
            else if(((Data == '-') && (CON_Value > 0)) ||
                    ((Data == '+') && (CON_Value < 0)))
            {
               CON_Value = -CON_Value;
            }
            else if(Data == TRAP_REAL_ESCAPE)
            {
                CON_ID = CON_INPUT_INVALID_ID;
                CON_GoToMenu(CON_MenuID);
            }

            break;
        }

        case CON_INPUT_STRING:
        {
            if(Data == CON_CHAR_ENTER)
            {
                CON_BackFromEdition = true;
                CON_InputStringMode = false;
                CON_GoToMenu(CON_MenuID);
            }
            else if(Data == CON_CHAR_BACKSPACE)
            {
                if((CON_InputPtr >= 0) && (CON_InputPtr <= CON_Maximum))
                {
                    if(CON_InputPtr > 0)
                    {
                        CON_InputPtr--;
                        CON_String[CON_InputPtr] = '\0';
                    }
                }
            }
            else if((Data >= ' ') && (Data <= '~'))
            {
                if(CON_InputPtr < CON_Maximum)
                {
                    CON_String[CON_InputPtr] = Data;
                    CON_InputPtr++;
                }
            }
            else if(Data == TRAP_REAL_ESCAPE)
            {
                CON_ID = CON_INPUT_INVALID_ID;
                CON_GoToMenu(CON_MenuID);
            }

            break;
        }

        case CON_INPUT_ESCAPE:
        {
            if(Data == TRAP_REAL_ESCAPE)
            {
                CON_GoToMenu(CON_MenuID);
            }
            break;
        }

        case CON_INPUT_CLI:
        {
            CON_ParseFIFO(Data);
            break;
        }

        case CON_INPUT_ESCAPE_TO_CONTINUE:
        {
            if(Data == TRAP_REAL_ESCAPE)
            {
                CON_ValidateInput = true;
                CON_InputType     = CON_INPUT_MENU_CHOICE;
            }
            break;
        }

        default: break;
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CON_ForceMenuRefresh
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
void CON_ForceMenuRefresh(void)
{
    CON_ForceRefresh = true;
}

//-------------------------------------------------------------------------------------------------

