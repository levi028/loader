#ifndef __LIB_IMGDEC_ENGINE_RAW_H__
#define __LIB_IMGDEC_ENGINE_RAW_H__

#include <api/libmp/imgdec_common.h>



#define IMGENGI_RAW_INITCONF          0x00010000
#define IMGENGI_RAW_INITCONF_PECFG    (IMGENGI_RAW_INITCONF | 0x0001)
#define IMGENGI_RAW_INITCONF_FILE     (IMGENGI_RAW_INITCONF | 0x0002)
#define IMGENGI_RAW_INITCONF_MEM      (IMGENGI_RAW_INITCONF | 0x0003)
#define IMGENGI_RAW_INITCONF_YUVFRM   (IMGENGI_RAW_INITCONF | 0x0004)




#define IMGENGI_RAW_DECCONF          0x00020000
#define IMGENGI_RAW_DECCONF_PECFG    1
#define IMGENGI_RAW_DECCONF_FILE     2


#define IMGENGI_RAW_DISPCONF          0x00030000
#define IMGENGI_RAW_DISPCONF_RESET    (IMGENGI_RAW_DISPCONF | 0x0001)
#define IMGENGI_RAW_DISPCONF_LAYER    (IMGENGI_RAW_DISPCONF | 0x0002)
#define IMGENGI_RAW_DISPCONF_SRCRECT  (IMGENGI_RAW_DISPCONF | 0x0003)
#define IMGENGI_RAW_DISPCONF_DSTRECT  (IMGENGI_RAW_DISPCONF | 0x0004)

typedef struct imgengi_rawconf_mem_s
{
    mp_memoper_type_t type;
    UINT8*            buffer;
    int               buflen;
} imgengi_rawconf_mem_t;


typedef struct imgengi_rawconf_file_s
{
    mp_fileoper_type_t type;
    const char*        filename;
    char*              bufffile;
    UINT32             bufffile_len;
} imgengi_rawconf_file_t;

#if 0
typedef struct imgengi_raw_initconf_s
{
    int   type;
    union {
        struct pe_image_cfg           pe_cfg;
    } data;
} imgengi_raw_initconf_t;
#endif

typedef struct imgengi_raw_conf_s
{
    int   type;
    union {
        enum  vp_display_layer disp_layer;
        struct rect            disp_srcrect;
        struct rect            disp_dstrect;
        imgengi_rawconf_file_t   init_file;
        mp_maphdle_frm_yuv_t     init_frm;
        imgengi_rawconf_mem_t    init_mem;
	    mp_maphdle_frm_yc_t      init_frmyc;
    } data;
} imgengi_raw_conf_t;

int image_engine_raw_alloc(int id);
int image_engine_raw_free(int id);
int image_engine_raw_initconf(int id, void* config);
int image_engine_raw_init(int id);
int image_engine_raw_decconf(int id, void* config);
int image_engine_raw_decode(int id);
int image_engine_raw_dispconf(int id, void* config);
int image_engine_raw_display(int id);
int image_engine_raw_abort(int id);

#endif

