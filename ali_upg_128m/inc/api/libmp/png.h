#ifndef _PNG_
#define _PNG_
#include <sys_config.h>
#include <basic_types.h>
#include <mediatypes.h>

#include <api/libmp/imgdec_common.h>
#ifdef __cplusplus
extern "C"
{
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct png_dec_initconfig_s
{
	UINT32 frm1_y_addr;
	UINT32 frm1_y_size;
	UINT32 frm1_c_addr;
	UINT32 frm1_c_size;

	UINT32 frm2_y_addr;
	UINT32 frm2_y_size;
	UINT32 frm2_c_addr;
	UINT32 frm2_c_size;

    UINT32 mem_alloc;
    UINT32 mem_size;  


	UINT32 (*status)(void *value);
	UINT32 frm3_y_addr;
	UINT32 frm3_y_size;
	UINT32 frm3_c_addr;
	UINT32 frm3_c_size;
	UINT32 frm4_y_addr;
	UINT32 frm4_y_size;
	UINT32 frm4_c_addr;
	UINT32 frm4_c_size;
} png_dec_initconfig_t;


typedef struct png_dec_dispconfig_s
{
    struct rect        src_rect;
    struct rect        dst_rect;
    mp_disphdle_config_t de_disp_config;
    unsigned char	   decode_mode;
} png_dec_dispconfig_t;


#ifdef __cplusplus
}
#endif

#endif

