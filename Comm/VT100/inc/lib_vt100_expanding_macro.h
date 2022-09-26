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
// Define(s)
//-------------------------------------------------------------------------------------------------


#include "lib_advanced_macro.h"

#define EXPAND_VT100_MENU_AS_ENUMS(NAME) NAME,
#define EXPAND_VT100_MENU_ITEM_AS_ENUM(MENU, MEMBER_OF, ITEM_ID, CALLBACK, NAVIGATE_TO, LABEL)      WHEN(EQUAL(MENU, MEMBER_OF))(ITEM_ID,)
#define EXPAND_VT100_MENU_ITEM_AS_FLAG_ENUM(MENU, MEMBER_OF, ITEM_ID, CALLBACK, NAVIGATE_TO, LABEL) WHEN(EQUAL(MENU, MEMBER_OF))(ITEM_ID##_FLAG = (1 << abs(CAT(MENU, ItemID_e)::ITEM_ID - 1)),)
#define EXPAND_VT100_MENU_ITEM_AS_DATA(MENU, MEMBER_OF, ITEM_ID, CALLBACK, NAVIGATE_TO, LABEL)      WHEN(EQUAL(MENU, MEMBER_OF))({ITEM_ID, CALLBACK, NAVIGATE_TO},)

/*
    { LBL_MAIN_MENU, nullptr,                       VT100_MENU_NONE   },
struct VT100_MenuDef_t
{
    Label_e           Label;
    VT100_InputType_e (*Callback)(uint8_t, VT100_CallBackType_e);
    VT100_Menu_e      NextMenu;
};
*/

#define EXPAND_VT100_AS_MENU_ENUMS(NAME)                      \
enum CAT(NAME, Item_e)                                        \
{                                                             \
	VT100_MENU_TREE_DEF(EXPAND_VT100_MENU_ITEM_AS_ENUM, NAME) \
	CAT(NAME, _NB_OF_ITEM)                                    \
};


//enum CAT(NAME, ItemFlag_e)                                  \
//{                                                           \
//    CON_MENU_DEF(EXPAND_VT100_MENU_ITEM_AS_FLAG_ENUM, NAME) \
//};

#define EXPAND_VT100_AS_MENU_DATA(NAME)                     \
const VT100_MenuDef_t CAT(NAME, ItemFlag_e)              \
{                                                           \
    VT100_MENU_TREE_DEF(EXPAND_VT100_MENU_ITEM_AS_DATA, NAME)      \
};




#define TRUNCATE_VT100

#define EXPAND_VT100_MENU_AS_STRUCT_VARIABLE_MEMBER(NAME)   static const VT100_MenuDef_t CAT(m_, NAME) [];
#define EXPAND_VT100_MENU_CALLBACK(NAME)                    VT100_InputType_e NAME (uint8_t Input, VT100_CallBackType_e Type);

//-------------------------------------------------------------------------------------------------
