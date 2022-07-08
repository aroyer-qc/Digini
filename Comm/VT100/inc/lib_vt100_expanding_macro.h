//-------------------------------------------------------------------------------------------------
//
//  File : lib_vt100_expand_macro.h
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

#pragma once

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------


// TODO need an expand for this
/*
static const VT100_MenuDef_t VT100_Terminal::m_MENU_Select          [];
static const VT100_MenuDef_t VT100_Terminal::m_MENU_Main            [];
static const VT100_MenuDef_t VT100_Terminal::m_MENU_Boot            [];
static const VT100_MenuDef_t VT100_Terminal::m_MENU_Debug           [];
static const VT100_MenuDef_t VT100_Terminal::m_MENU_Test            [];
static const VT100_MenuDef_t VT100_Terminal::m_MENU_Setting         [];
static const VT100_MenuDef_t VT100_Terminal::m_MENU_SettingMisc     [];
static const VT100_MenuDef_t VT100_Terminal::m_MENU_SettingTimeDate [];
*/

#define EXPAND_MENU_ITEM_AS_ENUM(MENU_SELECT, MENU, ITEM_ID, CALLBACK, NAVIGATE, LABEL) WHEN(EQUAL(MENU_SELECT, MENU)) (ITEM_ID,)
#define EXPAND_MENU_ITEM_AS_FLAG_ENUM(MENU_SELECT, MENU, ITEM_ID, CALLBACK, NAVIGATE, LABEL) WHEN(EQUAL(MENU_SELECT, MENU)) (ITEM_ID##_FLAG = (1 << abs(CAT(MENU, ItemID_e)::ITEM_ID - 1)),)

#define EXPAND_AS_MENU_ENUMS(NAME)                     \
enum CAT(NAME, ItemID_e)                               \
{                                                      \
	CON_MENU_DEF(EXPAND_MENU_ITEM_AS_ENUM , NAME)      \
};                                                     \
                                                       \
enum CAT(NAME, ItemFlag_e)                             \
{                                                      \
    CON_MENU_DEF(EXPAND_MENU_ITEM_AS_FLAG_ENUM, NAME)  \
};

//#define EXPAND_VT100_MENU_AS_ENUM(NAME, STRUCT)         NAME,
//#define EXPAND_VT100_MENU_AS_MENU_DATA(NAME, STRUCT)    STRUCT,
//#define EXPAND_VT100_MENU_AS_CALLBACK(NAME, FUNCTION)   VT100_InputType_e FUNCTION(uint8_t Input, VT100_CallBackType_e Type);

//-------------------------------------------------------------------------------------------------
