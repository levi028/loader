#include <types.h>
#include <sys_config.h>
#include <retcode.h>
#include <api/libc/string.h>
#include <api/libc/printf.h>
#include <osal/osal.h>
#include <api/libmp/pe.h>
#include "image_engine.h"
#include "plugin.h"

#ifdef _BUILD_OTA_E_
#if defined(SEE_CPU)
//for ota see link
void lib_pe_image_engine_callee(__attribute__((unused)) UINT8 *msg)
{
}

#else
//for ota main link
int image_engine_init(struct pe_image_cfg *pe_image_cfg)
{
    return 0;
}
int image_engine_cleanup()
{
    return 0;
}
int image_engine_rotate(unsigned char rotate_angle)
{
    return 0;
}
int image_engine_decode(char *filename, struct image_config *cfg)
{
    return 0;
}
int image_engine_show()
{
    return 0;
}
int image_engine_abort()
{
    return 0;
}
int image_engine_zoom(struct rect *dst_rect, struct rect *src_rect)
{
    return 0;
}
int image_engine_get_info(char *filename, struct image_info *info)
{
    return 0;
}
BOOL imagedec_ioctl(imagedec_id id,UINT32 cmd, UINT32 para)
{
    return TRUE;
}
int image_engine_2d_to_3d_swap(int type)
{
    return 0;
}

int image_engine_3d_user_option(struct image_3d_para *para)
{
    return 0;
}
int image_engine_set_disp_param(int display_type, struct image_3d_para *para)
{
    return 0;
}

int image_engine_decode_opentv(char *filename, UINT32 file_addr, UINT32 file_len, int scale_coef, struct image_config *cfg)
{
    return 0;
}


int image_engine_raw_alloc(int id)
{
    return 0;
}
int image_engine_raw_free(int id)
{
    return 0;
}
int image_engine_raw_initconf(int id, void* config)
{
    return 0;
}
int image_engine_raw_init(int id)
{
    return 0;
}
int image_engine_raw_decconf(int id, void* config)
{
    return 0;
}
int image_engine_raw_decode(int id)
{
    return 0;
}
int image_engine_raw_dispconf(int id, void* config)
{
    return 0;
}
int image_engine_raw_display(int id)
{
    return 0;
}
int image_engine_raw_abort(int id)
{
    return 0;
}

#endif
#endif
