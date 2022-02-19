#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#ifdef __cplusplus
extern "C"
{
#endif

//******** Release Mode definition *******//
//#define _DEB                 1

#ifdef _ALI_SYSDBG_ENABLE_
#define _DEB                1
#endif
// Include configuration file for current build.
#include "config.h"
#include <asm/chip.h>

#ifdef __cplusplus
}
#endif

#endif

