#if (defined (STM32F100xB) || \
     defined (STM32F101x6) || defined (STM32F101xB) || \
     defined (STM32F102x6) || defined (STM32F102xB) || \
     defined (STM32F103x6) || defined (STM32F103xB))

#define DMA2_SUPPORT        DEF_DISABLED

#endif

#if (defined (STM32F100xE) || \
     defined (STM32F101xE) || defined (STM32F101xG) || \
     defined (STM32F103xE) || defined (STM32F103xG) || \
     defined (STM32F105xC) || defined (STM32F107xC))

#define DMA2_SUPPORT        DEF_ENABLED

#endif

// Add CPU spec as project need then

#if defined (STM32F103xB)

#define NUMBER_OF_IO_PORT               6

#endif

#if (defined (STM32F103xE) || defined (STM32F103xG))

#define NUMBER_OF_IO_PORT               8

#endif
