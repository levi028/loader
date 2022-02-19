#ifndef __USB_DEV_TPL_H__
#define __USB_DEV_TPL_H__
#include <sys_config.h>
#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
        {
            UINT16 vids;
            UINT16 pids;
            char *device_class;
            char *device_model;
        }USB_DEVICE_TPL;

#ifdef USB_LOGO_TEST
extern USB_DEVICE_TPL usb_tpls[];
extern UINT32 usb_tpl_arry_cnt;
#endif

typedef struct
    {
        UINT8 devclass_id;
        char *devclss_strs;
    }USB_DEVCLASS_TBL;

#ifdef __cplusplus
}
#endif

#endif

