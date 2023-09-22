#include "lib_digini.h"

extern "C" void  lwipPlatformDiag(const char* pFormat, ...)
{
    va_list vaArg;

    va_start(vaArg, pFormat);
    DEBUG_PrintSerialLog(CON_DEBUG_LEVEL_LWIP, pFormat, vaArg);
    va_end(vaArg);
}
