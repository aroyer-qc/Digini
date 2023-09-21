#include "lib_digini.h"

extern "C" void  lwipPlatformDiag(const char *fmt, ...)
{
    CDEBUG_PrintSerialLog(CON_DEBUG_LEVEL_LWIP, __VA_ARGS__);
}
