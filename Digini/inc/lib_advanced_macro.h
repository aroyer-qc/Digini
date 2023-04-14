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

#define INT32_CAST(X)                   ((int32_t) X)
#define ROW_COUNT(TABLE)                ((sizeof(TABLE)) / (sizeof(TABLE[0])))
#define STRING(X)                       #X
#define STRINGIFY(X)                    STRING(X)
#define PASTER(x,y)                     x ## _ ## y

// Token pasting for significant message:
#define COMPILE_TIME_ASSERT3(X,L)       STATIC_ASSERT(X,static_assertion_at_line_##L)
#define COMPILE_TIME_ASSERT2(X,L)       COMPILE_TIME_ASSERT3(X,L)
#define COMPILE_TIME_ASSERT(X)          COMPILE_TIME_ASSERT2(X,__LINE__)

// Defines used for macro unrolling
// These defines are also used to insert "logic" to emulate if-else statements
#define PRIMITIVE_CAT(a, ...)           a ## __VA_ARGS__
#define CAT(a, ...)                     PRIMITIVE_CAT(a, __VA_ARGS__)
#define DEC(x)                          PRIMITIVE_CAT(DEC_, x)

#define IIF(c)                          PRIMITIVE_CAT(IIF_, c)
#define IIF_0(t, ...)                   __VA_ARGS__
#define IIF_1(t, ...)                   t
#define COMPL(b)                        PRIMITIVE_CAT(COMPL_, b)
#define COMPL_0                         1
#define COMPL_1                         0

#define BITAND(x)                       PRIMITIVE_CAT(BITAND_, x)
#define BITAND_0(y)                     0
#define BITAND_1(y)                     y

#define INC(x)                          PRIMITIVE_CAT(INC_, x)
#define INC_0                           1
#define INC_1                           2
#define INC_2                           3
#define INC_3                           4
#define INC_4                           5
#define INC_5                           6
#define INC_6                           7
#define INC_7                           8
#define INC_8                           9
#define INC_9                           10
#define INC_10                          11
#define INC_11                          12
#define INC_12                          13
#define INC_13                          14
#define INC_14                          15
#define INC_15                          16
#define INC_16                          17
#define INC_17                          18
#define INC_18                          19
#define INC_19                          20
#define INC_20                          21
#define INC_21                          22
#define INC_22                          23
#define INC_23                          24
#define INC_24                          25
#define INC_25                          26
#define INC_26                          27
#define INC_27                          28
#define INC_28                          29
#define INC_29                          30
#define INC_30                          31
#define INC_31                          32
#define INC_32                          33
#define INC_33                          34
#define INC_34                          35
#define INC_35                          36
#define INC_36                          37
#define INC_37                          38
#define INC_38                          39
#define INC_39                          40
#define INC_40                          41
#define INC_41                          42

#define DEC(x)                          PRIMITIVE_CAT(DEC_, x)
#define DEC_0                           0
#define DEC_1                           0
#define DEC_2                           1
#define DEC_3                           2
#define DEC_4                           3
#define DEC_5                           4
#define DEC_6                           5
#define DEC_7                           6
#define DEC_8                           7
#define DEC_9                           8
#define DEC_10                          9
#define DEC_11                          10
#define DEC_12                          11
#define DEC_13                          12
#define DEC_14                          13
#define DEC_15                          14
#define DEC_16                          15
#define DEC_17                          16
#define DEC_18                          17
#define DEC_19                          18
#define DEC_20                          19
#define DEC_21                          20
#define DEC_22                          21
#define DEC_23                          22
#define DEC_24                          23
#define DEC_25                          24
#define DEC_26                          25
#define DEC_27                          26
#define DEC_28                          27
#define DEC_29                          28
#define DEC_30                          29
#define DEC_31                          30
#define DEC_32                          31
#define DEC_33                          32
#define DEC_34                          33
#define DEC_35                          34
#define DEC_36                          35
#define DEC_37                          36
#define DEC_38                          37
#define DEC_39                          38
#define DEC_40                          39
#define DEC_41                          40
#define DEC_42                          41
#define DEC_43                          42

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
#define EMPTY()
#define DEFER(id)                       id EMPTY()
#define OBSTRUCT(...)                   __VA_ARGS__ DEFER(EMPTY)()
#define EVAL(...)                       EVAL1(EVAL1(EVAL1(__VA_ARGS__)))
#define EVAL1(...)                      EVAL2(EVAL2(EVAL2(__VA_ARGS__)))
#define EVAL2(...)                      EVAL3(EVAL3(EVAL3(__VA_ARGS__)))
#define EVAL3(...)                      EVAL4(EVAL4(EVAL4(__VA_ARGS__)))
#define EVAL4(...)                      EVAL5(EVAL5(EVAL5(__VA_ARGS__)))
#define EVAL5(...)                      __VA_ARGS__
#define IF_USE(cond, ...)               WHEN(cond)(__VA_ARGS__)

#define REPEAT(count, macro, ...)       WHEN(count)                            \
                                        (                                      \
                                            OBSTRUCT(REPEAT_INDIRECT) ()       \
                                            (                                  \
                                                DEC(count), macro, __VA_ARGS__ \
                                            )                                  \
                                            OBSTRUCT(macro)                    \
                                            (                                  \
                                                DEC(count), __VA_ARGS__        \
                                            )                                  \
                                        )

#define REPEAT_INDIRECT()               REPEAT

#define PRIMITIVE_COMPARE(x, y)         IS_PAREN                                \
                                        (                                       \
                                            COMPARE_ ## x ( COMPARE_ ## y) (()) \
                                        )

#define IS_COMPARABLE(...)              IS_PAREN(PRIMITIVE_CAT(COMPARE_, __VA_ARGS__)(()))

#define NOT_EQUAL(x, y)                 IIF(BITAND(IS_COMPARABLE(x))(IS_COMPARABLE(y)) ) \
                                        (                                                \
                                            PRIMITIVE_COMPARE,                           \
                                            1 EAT                                        \
                                        )(x, y)

#define EQUAL(x, y)                     COMPL(NOT_EQUAL(x, y))

#define OFFSET_BITS(x, ...)             | (1 << x)
#define GENERATE_BITMASK(LEN, SHIFT)    (((0) EVAL(REPEAT(LEN, OFFSET_BITS))) << SHIFT)

//-------------------------------------------------------------------------------------------------
