#ifndef    _ISDBTCC_OSD_H_
#define  _ISDBTCC_OSD_H_
#include <hld/osd/osddrv.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define ISDBTCC_WIDTH    640//640//608//608//720
#define ISDBTCC_HEIGHT 540//520//520//480//416//576
#define ISDBTCC_HOR_OFFSET ((720-SUBT_WIDTH)/2)
#define ISDBTCC_VER_OFFSET ((576-SUBT_HEIGHT)/2)

#define ISDBTCC_TRANSPARENT_COLOR    OSD_TRANSPARENT_COLOR//0//0xff

struct isdbtcc_config_par
{
    UINT8 *g_buf_addr;
    UINT32 g_buf_len;

    UINT16 max_isdbtcc_height;
    UINT8 isdbtcc_osd_layer_id;
    UINT16 isdbtcc_width;
    UINT16 isdbtcc_height;
    UINT16 isdbtcc_hor_offset;
    UINT16 isdbtcc_ver_offset;

    void (*osd_isdbtcc_enter)();
    void (*osd_isdbtcc_leave)();
};

//void osd_isdbtcc_enter();
//void osd_isdbtcc_leave();
//void lib_isdbtcc_init();
void lib_isdbtcc_attach(struct isdbtcc_config_par *pcc_config_par);

#ifdef __cplusplus
}
#endif

#endif
