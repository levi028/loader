
#ifdef OSD_16BIT_SUPPORT
    #ifndef SD_UI
    #include "winstyle_16bit.h"
    #else
    #include "winstyle_sd_16bit.h"
    #endif
#else
    #ifdef SD_UI
    #include "winstyle1_sd_8bit.h"
    #else
    #include "winstyle1.h"
    #endif
#endif


static WINSTYLE *winstyle_array[] =
{
#ifndef OSD_16BIT_SUPPORT
    #ifdef SD_UI
    (WINSTYLE*)w_shstyle1_sd_8bit,
    #else
    (WINSTYLE*)w_shstyle1,
    #endif
#else
    #ifndef SD_UI
    (WINSTYLE*)w_shstyle16
    #else
    (WINSTYLE*)w_shstyle_sd_16bit,
    #endif
#endif
};

