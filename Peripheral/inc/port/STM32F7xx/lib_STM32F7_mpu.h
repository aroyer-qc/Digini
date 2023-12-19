//-------------------------------------------------------------------------------------------------
//
//  File : lib_STM32F7_mpu.h
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

#pragma once

//-------------------------------------------------------------------------------------------------

#if (DIGINI_MPU_DRIVER == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define MPU_REGION_ENABLE                   (uint32_t(0x00000001))

// MPU Instruction Access
#define MPU_INSTRUCTION_ACCESS_ENABLE       (uint32_t(0x00000000))
#define MPU_INSTRUCTION_ACCESS_DISABLE      (uint32_t(0x10000000))

// MPU Access Shareable
#define MPU_ACCESS_SHAREABLE                (uint32_t(0x00040000))
#define MPU_ACCESS_NOT_SHAREABLE            (uint32_t(0x00000000))

// MPU Access Cacheable
#define MPU_ACCESS_CACHEABLE                (uint32_t(0x00020000))
#define MPU_ACCESS_NOT_CACHEABLE            (uint32_t(0x00000000))

// MPU Access Bufferable
#define MPU_ACCESS_BUFFERABLE               (uint32_t(0x00010000))
#define MPU_ACCESS_NOT_BUFFERABLE           (uint32_t(0x00000000))

// MPU TEX Levels
#define MPU_TEX_LEVEL_0                     (uint32_t(0x00000000))
#define MPU_TEX_LEVEL_1                     (uint32_t(0x00080000))
#define MPU_TEX_LEVEL_2                     (uint32_t(0x00100000))

// MPU Region Size
#define MPU_REGION_32B                      (uint32_t(0x00000008))
#define MPU_REGION_64B                      (uint32_t(0x0000000A))
#define MPU_REGION_128B                     (uint32_t(0x0000000C))
#define MPU_REGION_256B                     (uint32_t(0x0000000E))
#define MPU_REGION_512B                     (uint32_t(0x00000010))
#define MPU_REGION_1KB                      (uint32_t(0x00000012))
#define MPU_REGION_2KB                      (uint32_t(0x00000014))
#define MPU_REGION_4KB                      (uint32_t(0x00000016))
#define MPU_REGION_8KB                      (uint32_t(0x00000018))
#define MPU_REGION_16KB                     (uint32_t(0x0000001A))
#define MPU_REGION_32KB                     (uint32_t(0x0000001C))
#define MPU_REGION_64KB                     (uint32_t(0x0000001E))
#define MPU_REGION_128KB                    (uint32_t(0x00000020))
#define MPU_REGION_256KB                    (uint32_t(0x00000022))
#define MPU_REGION_512KB                    (uint32_t(0x00000024))
#define MPU_REGION_1MB                      (uint32_t(0x00000026))
#define MPU_REGION_2MB                      (uint32_t(0x00000028))
#define MPU_REGION_4MB                      (uint32_t(0x0000002A))
#define MPU_REGION_8MB                      (uint32_t(0x0000002C))
#define MPU_REGION_16MB                     (uint32_t(0x0000002E))
#define MPU_REGION_32MB                     (uint32_t(0x00000030))
#define MPU_REGION_64MB                     (uint32_t(0x00000032))
#define MPU_REGION_128MB                    (uint32_t(0x00000034))
#define MPU_REGION_256MB                    (uint32_t(0x00000036))
#define MPU_REGION_512MB                    (uint32_t(0x00000038))
#define MPU_REGION_1GB                      (uint32_t(0x0000003A))
#define MPU_REGION_2GB                      (uint32_t(0x0000003C))
#define MPU_REGION_4GB                      (uint32_t(0x0000003E))

// MPU Region Permission Attributes
#define MPU_REGION_NO_ACCESS                (uint32_t(0x00000000))
#define MPU_REGION_PRIV_RW                  (uint32_t(0x01000000))
#define MPU_REGION_PRIV_RW_URO              (uint32_t(0x02000000))
#define MPU_REGION_FULL_ACCESS              (uint32_t(0x03000000))
#define MPU_REGION_PRIV_RO                  (uint32_t(0x05000000))
#define MPU_REGION_PRIV_RO_URO              (uint32_t(0x06000000))

// MPU Region Number
#define MPU_REGION_0                        (uint8_t(0x0))
#define MPU_REGION_1                        (uint8_t(0x1))
#define MPU_REGION_2                        (uint8_t(0x2))
#define MPU_REGION_3                        (uint8_t(0x3))
#define MPU_REGION_4                        (uint8_t(0x4))
#define MPU_REGION_5                        (uint8_t(0x5))
#define MPU_REGION_6                        (uint8_t(0x6))
#define MPU_REGION_7                        (uint8_t(0x7))

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

//  MPU HFNMI and PRIVILEGED Access control
enum MPU_Priviledge_e
{
    MPU_HFNMI_PRIVDEF_NONE    =   0,
    MPU_HARDFAULT_NMI         =   2,
    MPU_PRIVILEGED_DEFAULT    =   4,
    MPU_HFNMI_PRIVDEF         =   6
};

//-------------------------------------------------------------------------------------------------
// Function prototype(s)
//-------------------------------------------------------------------------------------------------

void  MPU_Initialize  (MPU_Priviledge_e Priviledge);

//-------------------------------------------------------------------------------------------------

#endif
