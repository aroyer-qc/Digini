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

#define VT100_TOKEN(x)                  myVT100_Terminal.x

/// This enum is a list of all menu page that exist in the VT100_MENU_DEF.
#define EXPAND_VT100_MENU_AS_ENUM(MENU_ID)  CAT(MENU_ID, _ID),

/// This create list of pointer on each menu (see Note 1)
#define EXPAND_VT100_MENU_AS_DATA(NAME)     {CAT(VT100_Terminal::m_, NAME), CAT(NAME, _NB_OF_ITEMS)},


/// This will create a enum list of all sub menu items contains in each menu page.
#define EXPAND_VT100_MENU_ITEM_AS_ENUM(MENU, MEMBER_OF, ITEM_ID, CALLBACK, NAVIGATE_TO, LABEL)            WHEN(EQUAL(MENU, MEMBER_OF))(MEMBER_OF ## _ ## ITEM_ID,)
#define EXPAND_AS_MENU_ENUMS_ITEM(NAME)			   			  \
enum CAT(NAME, _ItemID_e)                                     \
{                                                             \
	VT100_MENU_TREE_DEF(EXPAND_VT100_MENU_ITEM_AS_ENUM, NAME) \
    CAT(NAME, _NB_OF_ITEMS)                                   \
};

/// This create the class member structure declaration containing sub item information for each menu.
#define EXPAND_VT100_MENU_AS_STRUCT_VARIABLE_MEMBER(NAME)                                                  static const VT100_MenuDef_t CAT(m_, NAME)[NAME ## _NB_OF_ITEMS];

/// (Note 1) This create the class member structure containing actual sub item information for each menu.
//#define EXPAND_VT100_MENU_AS_MEMBER_VARIABLE_DATA(MENU, MEMBER_OF, ITEM_ID, CALLBACK, NAVIGATE_TO, LABEL) WHEN(EQUAL(MENU, MEMBER_OF))({LABEL, IF_THEN(EVAL(EQUAL(nullptr, CALLBACK)), nullptr, VT100_TOKEN(CALLBACK)), CAT(NAVIGATE_TO, _ID)},)
#define EXPAND_VT100_MENU_AS_MEMBER_VARIABLE_DATA(MENU, MEMBER_OF, ITEM_ID, CALLBACK, NAVIGATE_TO, LABEL) WHEN(EQUAL(MENU, MEMBER_OF))({LABEL, (VT100_TOKEN(CALLBACK)), CAT(NAVIGATE_TO, _ID)},)
#define EXPAND_VT100_AS_MENU_MEMBER_VARIABLE_DATA(NAME)                        \
const VT100_MenuDef_t VT100_Terminal::m_ ## NAME[NAME ## _NB_OF_ITEMS] =           \
{                                                                              \
    VT100_MENU_TREE_DEF(EXPAND_VT100_MENU_AS_MEMBER_VARIABLE_DATA, NAME)       \
};

/// this automatically create all the method declaration in the class for each callback
#define EXPAND_VT100_MENU_CALLBACK(NAME)                    VT100_InputType_e NAME(uint8_t Input, VT100_CallBackType_e Type);
#define EXPAND_VT100_MENU_CALLBACK_INTERFACE(NAME)          virtual VT100_InputType_e NAME(uint8_t Input, VT100_CallBackType_e Type) = 0;

//-------------------------------------------------------------------------------------------------
