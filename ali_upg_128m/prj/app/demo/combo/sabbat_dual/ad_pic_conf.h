#include <sys_config.h>


#ifdef PIP_PNG_GIF


#ifdef  _RD_DEBUG_
#define AD_PIC_DEBUG
#define AD_PIC_FILE_DEBUG
#endif

#define ENABLE_PE_CACHE

#if 0
#define SUPPORT_DEBUG_USING_OWN_PICTURE

#define SUPPORT_AD_PIC_SRC
#define SUPPORT_AD_PIC_TIMER
#define SUPPORT_AD_PIC_UIINFO
#else
#define SUPPORT_AD_PIC_UIINFO
#endif


#endif
