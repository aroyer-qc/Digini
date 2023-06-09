//-------------------------------------------------------------------------------------------------
//
//  File : lib_grafx_link.h
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

#pragma once

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_GRAFX == DEF_ENABLED)


//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

#include "menu_link_cfg.h"
#include "menu_link_list_cfg.h"

//-------------------------------------------------------------------------------------------------
// Expand macro(s)
//-------------------------------------------------------------------------------------------------

#define EXPAND_X_LINK_AS_ENUM(ENUM_ID, OBJECT) ENUM_ID,
#define EXPAND_X_LINK_AS_OBJECT(ENUM_ID, OBJECT) OBJECT,
#define EXPAND_X_LINK_LIST_AS_ENUM(ENUM_ID, ID1, ID2, ID3, ID4, ID5, ID6, ID7, ID8) ENUM_ID,
#define EXPAND_X_LINK_LIST_AS_OBJECT(ENUM_ID, ID1, ID2, ID3, ID4, ID5, ID6, ID7, ID8) \
                                             {ID1, ID2, ID3, ID4, ID5, ID6, ID7, ID8},

//-------------------------------------------------------------------------------------------------

#ifdef LINK_DEF

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------


enum Link_e
{
  #ifdef LINK_DEF
    LINK_DEF(EXPAND_X_LINK_AS_ENUM)
  #endif
    NB_LINK_CONST,
    PREVIOUS_LINK = 0xFFFD,
    REMOVE_WIDGET = 0xFFFE,
    INVALID_LINK  = 0xFFFF,
};


struct PageWidget_t
{
    Widget_e ID;
    Link_e   Link;
};

#ifdef LINK_LIST_DEF
enum LinkList_e
{
    LINK_LIST_DEF(EXPAND_X_LINK_LIST_AS_ENUM)
    NB_LINK_LIST_CONST,
    INVALID_LINK_LIST = 0xFFFF,
};

struct LinkList_t
{
     Link_e Link[8];
};
#endif // LINK_LIST_DEF

//-------------------------------------------------------------------------------------------------
// Post include file(s)
//-------------------------------------------------------------------------------------------------

#include "menu_cfg.h"

//-------------------------------------------------------------------------------------------------
// Const(s)
//-------------------------------------------------------------------------------------------------

#ifdef GFX_GLOBAL
  const PageWidget_t* PageWidget[NB_LINK_CONST] =
  {
      LINK_DEF(EXPAND_X_LINK_AS_OBJECT)
  };
#else
  extern const PageWidget_t* PageWidget[NB_LINK_CONST];
#endif // GFX_GLOBAL


#ifdef LINK_LIST_DEF
 #ifdef GFX_GLOBAL
  LinkList_t LinkList[NB_LINK_LIST_CONST] =
  {
      LINK_LIST_DEF(EXPAND_X_LINK_LIST_AS_OBJECT)
  };
 #else
  extern LinkList_t LinkList[NB_LINK_LIST_CONST];
 #endif
#endif // LINK_LIST_DEF

//-------------------------------------------------------------------------------------------------

#endif // LINK_DEF

#endif // (DIGINI_USE_GRAFX == DEF_ENABLED)