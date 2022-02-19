#ifndef __AP_CONTROL_MHEG5_H_
#define __AP_CONTROL_MHEG5_H_
#include <sys_config.h>
#include <basic_types.h>
#include <api/libtsi/db_3l.h>
#include "./copper_common/com_api.h"
#ifdef __cplusplus
extern "C"
{
#endif

// CI+ MHEG5 browser
typedef struct
{
    UINT8   initial_object_name_length;    //length of Initial Object Name
    UINT8   *p_initial_object_name;        // Initial Object Name
    UINT32  initial_object_length;        //length of Initial Object
    UINT8   *p_initial_object;            // Initial Object
} app_init_object_t;

#ifdef CI_PLUS_SUPPORT
extern UINT8    g_mheg5_app_domain_identifier[];
#endif

INT32           mheg_monitor_start(void);
void            mheg_monitor_stop(void);
BOOL            is_ciplus_menu_exist(void);
void            ciplus_browser_enter(void);
void            ciplus_browser_exit(void);
void            ciplus_reenter_browser(void);
#ifdef __cplusplus
}
#endif
#endif
