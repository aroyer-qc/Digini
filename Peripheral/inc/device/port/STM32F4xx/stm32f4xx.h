#pragma once

#ifdef __cplusplus
 extern "C" {
#endif

#if !defined  (STM32F4)
#define STM32F4
#endif

// Use one of those define in your project

  // #define STM32F405xx           STM32F405RG, STM32F405VG and STM32F405ZG Devices
  // #define STM32F415xx           STM32F415RG, STM32F415VG and STM32F415ZG Devices
  // #define STM32F407xx           STM32F407VG, STM32F407VE, STM32F407ZG, STM32F407ZE, STM32F407IG, STM32F407IE Devices
  // #define STM32F417xx           STM32F417VG, STM32F417VE, STM32F417ZG, STM32F417ZE, STM32F417IG, STM32F417IE Devices
  // #define STM32F427xx           STM32F427VG, STM32F427VI, STM32F427ZG, STM32F427ZI, STM32F427IG, STM32F427II Devices
  // #define STM32F437xx           STM32F437VG, STM32F437VI, STM32F437ZG, STM32F437ZI, STM32F437IG, STM32F437II Devices
  // #define STM32F429xx           STM32F429VG, STM32F429VI, STM32F429ZG, STM32F429ZI, STM32F429BG, STM32F429BI, STM32F429NG, STM32F439NI, STM32F429IG, STM32F429II Devices
  // #define STM32F439xx           STM32F439VG, STM32F439VI, STM32F439ZG, STM32F439ZI, STM32F439BG, STM32F439BI, STM32F439NG, STM32F439NI, STM32F439IG, STM32F439II Devices
  // #define STM32F401xC           STM32F401CB, STM32F401CC, STM32F401RB, STM32F401RC, STM32F401VB, STM32F401VC Devices
  // #define STM32F401xE           STM32F401CD, STM32F401RD, STM32F401VD, STM32F401CE, STM32F401RE, STM32F401VE Devices
  // #define STM32F410Tx           STM32F410T8, STM32F410TB Devices
  // #define STM32F410Cx           STM32F410C8, STM32F410CB Devices
  // #define STM32F410Rx           STM32F410R8, STM32F410RB Devices
  // #define STM32F411xE           STM32F411CC, STM32F411RC, STM32F411VC, STM32F411CE, STM32F411RE, STM32F411VE Devices
  // #define STM32F446xx           STM32F446MC, STM32F446ME, STM32F446RC, STM32F446RE, STM32F446VC, STM32F446VE, STM32F446ZC, STM32F446ZE Devices
  // #define STM32F469xx           STM32F469AI, STM32F469II, STM32F469BI, STM32F469NI, STM32F469AG, STM32F469IG, STM32F469BG, STM32F469NG, STM32F469AE, STM32F469IE, STM32F469BE, STM32F469NE Devices
  // #define STM32F479xx           STM32F479AI, STM32F479II, STM32F479BI, STM32F479NI, STM32F479AG, STM32F479IG, STM32F479BG, STM32F479NG Devices
  // #define STM32F412Cx           STM32F412CEU, STM32F412CGU Devices
  // #define STM32F412Zx           STM32F412ZET, STM32F412ZGT, STM32F412ZEJ, STM32F412ZGJ Devices
  // #define STM32F412Vx           STM32F412VET, STM32F412VGT, STM32F412VEH, STM32F412VGH Devices
  // #define STM32F412Rx           STM32F412RET, STM32F412RGT, STM32F412REY, STM32F412RGY Devices
  // #define STM32F413xx           STM32F413CH, STM32F413MH, STM32F413RH, STM32F413VH, STM32F413ZH, STM32F413CG, STM32F413MG, STM32F413RG, STM32F413VG, STM32F413ZG Devices
  // #define STM32F423xx           STM32F423CH, STM32F423RH, STM32F423VH, STM32F423ZH Devices

#if defined(STM32F405xx)
  #include "stm32f405xx.h"
#elif defined(STM32F415xx)
  #include "stm32f415xx.h"
#elif defined(STM32F407xx)
  #include "stm32f407xx.h"
#elif defined(STM32F417xx)
  #include "stm32f417xx.h"
#elif defined(STM32F427xx)
  #include "stm32f427xx.h"
#elif defined(STM32F437xx)
  #include "stm32f437xx.h"
#elif defined(STM32F429xx)
  #include "stm32f429xx.h"
#elif defined(STM32F439xx)
  #include "stm32f439xx.h"
#elif defined(STM32F401xC)
  #include "stm32f401xc.h"
#elif defined(STM32F401xE)
  #include "stm32f401xe.h"
#elif defined(STM32F410Tx)
  #include "stm32f410tx.h"
#elif defined(STM32F410Cx)
  #include "stm32f410cx.h"
#elif defined(STM32F410Rx)
  #include "stm32f410rx.h"
#elif defined(STM32F411xE)
  #include "stm32f411xe.h"
#elif defined(STM32F446xx)
  #include "stm32f446xx.h"
#elif defined(STM32F469xx)
  #include "stm32f469xx.h"
#elif defined(STM32F479xx)
  #include "stm32f479xx.h"
#elif defined(STM32F412Cx)
  #include "stm32f412cx.h"
#elif defined(STM32F412Zx)
  #include "stm32f412zx.h"
#elif defined(STM32F412Rx)
  #include "stm32f412rx.h"
#elif defined(STM32F412Vx)
  #include "stm32f412vx.h"
#elif defined(STM32F413xx)
  #include "stm32f413xx.h"
#elif defined(STM32F423xx)
  #include "stm32f423xx.h"
#else
 #error "Please select first the target STM32F4xx device used in your application"
#endif

typedef enum
{
  RESET = 0U,
  SET = !RESET
} FlagStatus, ITStatus;

typedef enum
{
  DISABLE = 0U,
  ENABLE = !DISABLE
} FunctionalState;
#define IS_FUNCTIONAL_STATE(STATE) (((STATE) == DISABLE) || ((STATE) == ENABLE))

typedef enum
{
  SUCCESS = 0U,
  ERROR = !SUCCESS
} ErrorStatus;

#define SET_BIT(REG, BIT)     ((REG) |= (BIT))

#define CLEAR_BIT(REG, BIT)   ((REG) &= ~(BIT))

#define READ_BIT(REG, BIT)    ((REG) & (BIT))

#define CLEAR_REG(REG)        ((REG) = (0x0))

#define WRITE_REG(REG, VAL)   ((REG) = (VAL))

#define READ_REG(REG)         ((REG))

#define MODIFY_REG(REG, CLEARMASK, SETMASK)  WRITE_REG((REG), (((READ_REG(REG)) & (~(CLEARMASK))) | (SETMASK)))

#define POSITION_VAL(VAL)     (__CLZ(__RBIT(VAL)))

// Atomic 32-bit register access macro to set one or several bits
#define ATOMIC_SET_BIT(REG, BIT)                             \
  do {                                                       \
    uint32_t val;                                            \
    do {                                                     \
      val = __LDREXW((__IO uint32_t *)&(REG)) | (BIT);       \
    } while ((__STREXW(val,(__IO uint32_t *)&(REG))) != 0U); \
  } while(0)

// Atomic 32-bit register access macro to clear one or several bits
#define ATOMIC_CLEAR_BIT(REG, BIT)                           \
  do {                                                       \
    uint32_t val;                                            \
    do {                                                     \
      val = __LDREXW((__IO uint32_t *)&(REG)) & ~(BIT);      \
    } while ((__STREXW(val,(__IO uint32_t *)&(REG))) != 0U); \
  } while(0)

// Atomic 32-bit register access macro to clear and set one or several bits
#define ATOMIC_MODIFY_REG(REG, CLEARMSK, SETMASK)                          \
  do {                                                                     \
    uint32_t val;                                                          \
    do {                                                                   \
      val = (__LDREXW((__IO uint32_t *)&(REG)) & ~(CLEARMSK)) | (SETMASK); \
    } while ((__STREXW(val,(__IO uint32_t *)&(REG))) != 0U);               \
  } while(0)

// Atomic 16-bit register access macro to set one or several bits
#define ATOMIC_SETH_BIT(REG, BIT)                            \
  do {                                                       \
    uint16_t val;                                            \
    do {                                                     \
      val = __LDREXH((__IO uint16_t *)&(REG)) | (BIT);       \
    } while ((__STREXH(val,(__IO uint16_t *)&(REG))) != 0U); \
  } while(0)

// Atomic 16-bit register access macro to clear one or several bits
#define ATOMIC_CLEARH_BIT(REG, BIT)                          \
  do {                                                       \
    uint16_t val;                                            \
    do {                                                     \
      val = __LDREXH((__IO uint16_t *)&(REG)) & ~(BIT);      \
    } while ((__STREXH(val,(__IO uint16_t *)&(REG))) != 0U); \
  } while(0)

// Atomic 16-bit register access macro to clear and set one or several bits
#define ATOMIC_MODIFYH_REG(REG, CLEARMSK, SETMASK)                         \
  do {                                                                     \
    uint16_t val;                                                          \
    do {                                                                   \
      val = (__LDREXH((__IO uint16_t *)&(REG)) & ~(CLEARMSK)) | (SETMASK); \
    } while ((__STREXH(val,(__IO uint16_t *)&(REG))) != 0U);               \
  } while(0)

#if defined (USE_HAL_DRIVER)
 #include "stm32f4xx_hal.h"
#endif

#ifdef __cplusplus
}
#endif

