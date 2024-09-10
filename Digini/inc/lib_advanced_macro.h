//-------------------------------------------------------------------------------------------------
//
//  File : lib_advanced_macro.h
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

#define STRING(X)                       #X
#define STRINGIFY(X)                    STRING(X)

// Defines used for macro unrolling
// These defines are also used to insert "logic" to emulate if-else statements
//#define ABS(X)                          (((X) < 0) ? (-X) : (X))
#define PRIMITIVE_CAT(a, ...)           a ## __VA_ARGS__
#define CAT(a, ...)                     PRIMITIVE_CAT(a, __VA_ARGS__)

#define IIF(c)                          PRIMITIVE_CAT(IIF_, c)
#define IIF_0(t, ...)                   __VA_ARGS__
#define IIF_1(t, ...)                   t
#define COMPL(b)                        PRIMITIVE_CAT(COMPL_, b)
#define COMPL_0                         1
#define COMPL_1                         0

#define BITAND(x)                       PRIMITIVE_CAT(BITAND_, x)
#define BITAND_0(y)                     0
#define BITAND_1(y)                     y

#define EVAL(...)                       EVAL1(EVAL1(EVAL1(__VA_ARGS__)))
#define EVAL1(...)                      EVAL2(EVAL2(EVAL2(__VA_ARGS__)))
#define EVAL2(...)                      EVAL3(EVAL3(EVAL3(__VA_ARGS__)))
#define EVAL3(...)                      EVAL4(EVAL4(EVAL4(__VA_ARGS__)))
#define EVAL4(...)                      EVAL5(EVAL5(EVAL5(__VA_ARGS__)))
#define EVAL5(...)                      EVAL6(EVAL6(EVAL6(__VA_ARGS__)))
#define EVAL6(...)                      __VA_ARGS__

#define CHECK_N(x, n, ...)              n
#define CHECK(...)                      CHECK_N(__VA_ARGS__, 0,)
#define PROBE(x)                        x, 1,
#define IS_PAREN(x)                     CHECK(IS_PAREN_PROBE x)
#define IS_PAREN_PROBE(...)             PROBE(~)
#define NOT(x)                          CHECK(PRIMITIVE_CAT(NOT_, x))
#define NOT_0                           PROBE(~)
#define BOOL(x)                         COMPL(NOT(x))
#define IF(c)                           IIF(BOOL(c))
#define EAT(...)
#define EXPAND(...)                     __VA_ARGS__
#define WHEN(c)                         IF(c)(EXPAND, EAT)
#define IF_USE(cond, ...)               WHEN(cond)(__VA_ARGS__)

#define EMPTY()
#define DEFER(id)                       id EMPTY()
#define OBSTRUCT(...)                   __VA_ARGS__ DEFER(EMPTY)()

#define PRIMITIVE_COMPARE(x, y)         IS_PAREN                                \
                                        (                                       \
                                            COMPARE_ ## x ( COMPARE_ ## y) (()) \
                                        )

#define IS_COMPARABLE(...)              IS_PAREN(PRIMITIVE_CAT(COMPARE_, __VA_ARGS__)(()))

#define NOT_EQUAL(x, y)                 IIF(BITAND(IS_COMPARABLE(x))(IS_COMPARABLE(y)) ) \
                                        (                                                \
                                            PRIMITIVE_COMPARE,                           \
                                            EAT                                          \
                                        )(x, y)

#define EQUAL(x, y)                     COMPL(NOT_EQUAL(x, y))

// General macros to add to library
// If more arguments are required to be counted in macro,
// simply add more elements in the macro below and the same with the ARG_COUNT macro
// IMPORTANT: LAST_ARGUMENT must contains 2 more value than ARG_COUNT
#define LAST_ARGUMENT(_1, _2, _3, _4, _5, _6, _7, _8, _9, \
    _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, \
    _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, \
    _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, \
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, \
    _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, \
    _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, \
    _70, _71, _72, _73, _74, _75, _76, _77, _78, _79, \
    _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, \
    _90, _91, _92, _93, _94, _95, _96, _97, _98, _99, \
    _100, _101, ...) _101
// Note: With GCC, the preprocessor eliminates the comma in
//   , ## __VA_ARGS__
// IF __VA_ARGS__ is empty. (https://gcc.gnu.org/onlinedocs/cpp/Variadic-Macros.html)
// This is necessary to correctly count the arguments
//
#define ARG_COUNT(...) LAST_ARGUMENT(not_used, ## __VA_ARGS__, \
    99, 98, 97, 96, 95, 94, 93, 92, 91, 90, \
    89, 88, 87, 86, 85, 84, 83, 82, 81, 80, \
    79, 78, 77, 76, 75, 74, 73, 72, 71, 70, \
    69, 68, 67, 66, 65, 64, 63, 62, 61, 60, \
    59, 58, 57, 56, 55, 54, 53, 52, 51, 50, \
    49, 48, 47, 46, 45, 44, 43, 42, 41, 40, \
    39, 38, 37, 36, 35, 34, 33, 32, 31, 30, \
    29, 28, 27, 26, 25, 24, 23, 22, 21, 20, \
    19, 18, 17, 16, 15, 14, 13, 12, 11, 10, \
    9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

//-------------------------------------------------------------------------------------------------
