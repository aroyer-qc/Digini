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
#define EVAL5(...)                      __VA_ARGS__

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

//-------------------------------------------------------------------------------------------------
