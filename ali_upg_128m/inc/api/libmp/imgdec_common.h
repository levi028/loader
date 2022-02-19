#ifndef	__LIB_IMGDEC_COMMON_H__
#define	__LIB_IMGDEC_COMMON_H__

#include <mediatypes.h>

typedef unsigned long filehdle_id;
typedef unsigned long maphdle_id;
typedef unsigned long dechdle_id;
typedef unsigned long disphdle_id;
typedef unsigned long memhdle_id;
typedef unsigned long scalehdle_id;


typedef enum MP_MEMHDLE_E
{
    MP_MEMHDLE_MEDIAPLAY,
    MP_MEMHDLE_ADVERT
} mp_memoper_type_t;



/***************************************************************************************************
 * brief:     
 * detail:    
 * author:    saicheong.yuen
 * date:      2015-7-15
 ***************************************************************************************************/
typedef enum MP_FILEHDLE_E
{
    MP_FILEHDLE_STD,
    MP_FILEHDLE_PECACHE,
    MP_FILEHDLE_MEMLOAD,
    MP_FILEHDLE_BUFF,
} mp_fileoper_type_t;

int mp_filehdle_open(filehdle_id fh, const char* filename);
int mp_filehdle_close(filehdle_id fh);
int mp_filehdle_read(filehdle_id fh, unsigned char *buf, UINT32 size);
int mp_filehdle_seek(filehdle_id fh, long offset, UINT32 origin);
int mp_filehdle_tell(filehdle_id fh);
int mp_filehdle_construct(filehdle_id *fh, mp_fileoper_type_t type);
int mp_filehdle_construct2(filehdle_id *fh, mp_fileoper_type_t type, char* bufffile, UINT32 bufffile_len);
int mp_filehdle_destruct(filehdle_id fh);


/***************************************************************************************************
 * brief:     
 * detail:    
 * author:    saicheong.yuen
 * date:      2015-7-15
 ***************************************************************************************************/
typedef enum MP_IMGDECHDLE_E
{
    MP_DECHDLE_PNG_SW,
    MP_DECHDLE_PNG_HWACC,
} mp_imgdecoper_type_t;

int mp_imgdechdle_open(dechdle_id dechdl, memhdle_id memhdl);
int mp_imgdechdle_close(dechdle_id dechdl);

int mp_imgdechdle_filebind_beforeparse(dechdle_id dechdl, filehdle_id fhdl);
int mp_imgdechdle_parseinfo(dechdle_id dechdl);
int mp_imgdechdle_originsizeget(dechdle_id dechdl, int* width, int* height);
int mp_imgdechdle_outsizeget(dechdle_id dechdl, int* width, int* height);
int mp_imgdechdle_mapbind_beforedecode(dechdle_id dechdl, maphdle_id mhdl);
int mp_imgdechdle_decodepreconfig(dechdle_id dechdl, int flag);
int mp_imgdechdle_decode(dechdle_id dechdl);

int mp_imgdechdle_construct(dechdle_id* dechdl, mp_imgdecoper_type_t type);
int mp_imgdechdle_destruct(dechdle_id dechdl);


/***************************************************************************************************
 * brief:     
 * detail:    
 * author:    saicheong.yuen
 * date:      2015-7-15
 ***************************************************************************************************/
typedef enum MP_MAPHDLE_E
{
    MP_MAPHDLE_ARGB8888,
    MP_MAPHDLE_ARGB1555,
    MP_MAPHDLE_CLUT8,
    MP_MAPHDLE_MPEG2_32x16,
    MP_MAPHDLE_UNKNOWN
} mp_mapoper_type_t;

typedef enum MP_SCALEHDLE_E
{
    MP_SCALEHDLE_ARGB8888,
} mp_scaleoper_type_t;
#define MP_MAPCOMMON_INITCONF_INFOIMAGE     0x00000001
#define MP_MAPCOMMON_INITCONF_INFOCANVAS    0x00000002
#define MP_MAPCOMMON_INITCONF_INOFPALLETE   0x00000004
#define MP_MAPCOMMON_INITCONF_MEMFRM        0x00000010


typedef struct mp_maphdle_imginfo_s
{
    int width;
    int height;
} mp_maphdle_imginfo_t;

typedef struct mp_maphdle_canvasinfo_s
{
    int width;
    int height;
    int stride;
    unsigned char sample_format;
} mp_maphdle_canvasinfo_t;

typedef struct mp_maphdle_frm_yuv_s
{
    char*  image_y_addr;
    int    image_y_addr_len;
    char*  image_c_addr;
    int    image_c_addr_len;
} mp_maphdle_frm_yuv_t, mp_maphdle_frm_yc_t;

typedef struct mp_maphdle_frm_argb_s
{
    char*  image_addr;
    int    image_addr_len;
} mp_maphdle_frm_argb_t, mp_maphdle_frm_rgb_t;

typedef struct mp_maphdle_frm_s
{
    mp_maphdle_frm_yuv_t     yuv;
    mp_maphdle_frm_argb_t    argb;
    mp_maphdle_frm_rgb_t     rgb;
	mp_maphdle_frm_yc_t      yc;
} mp_maphdle_frm_t;

typedef struct mp_maphdle_palleteinfo_s
{
    int           num;
    unsigned int* data_ptr;
} mp_maphdle_palleteinfo_t;
typedef struct mp_mapoper_initconf_s
{
    int initflag;
    mp_maphdle_imginfo_t    info_image;
    mp_maphdle_canvasinfo_t info_canvas;
    mp_maphdle_frm_t        mem_frm;
    mp_maphdle_palleteinfo_t info_pallete;
} mp_mapoper_initconf_t;

typedef struct mp_scalehdle_imginfo_s
{
    unsigned int width;
    unsigned int height;
}mp_scalehdle_imginfo_t;

typedef struct mp_scaleoper_initconf_s
{
    int  initflag;
    char *image_addr;
    int  image_addr_len;
    mp_scalehdle_imginfo_t info_src;
    mp_scalehdle_imginfo_t info_dest;
} mp_scaleoper_initconf_t;
#define MP_MAPHDL_FILLCONFIG_INDEX_BLACK       0x00000000
#define MP_MAPHDL_FILLCONFIG_INDEX_DEEPBLUE    0x00000001
typedef struct mp_mapoper_fillconfig_s
{
    int flag;
    union
    {
        int color_index;
    } data;
    struct rect region;
} mp_mapoper_fillconfig_t;
int mp_maphdle_trans_mpeg2mapping_line(maphdle_id maphdle_out, maphdle_id maphdle_in, int line, int line_start, int pixel_num);

int mp_maphdle_horline_read(maphdle_id maphdle, int line, int line_start, char* pixel_data, int pixel_num);
int mp_maphdle_horline_write(maphdle_id maphdle, int line, int line_start, char* pixel_data, int pixel_num);
int mp_maphdle_verline_write(maphdle_id maphdle, int line, int line_start, char* pixel_data, int pixel_num);
int mp_maphdle_fill(maphdle_id maphdle, mp_mapoper_fillconfig_t *fill_config);
int mp_maphdle_get_imageinfo(maphdle_id maphdle, mp_maphdle_imginfo_t* imginfo);
int mp_maphdle_flush_imageaddr(maphdle_id maphdle);
int mp_maphdle_initconfig(maphdle_id maphdle, mp_mapoper_initconf_t* init_config);
int mp_maphdle_get_imageaddr_yuv(maphdle_id maphdle, mp_maphdle_frm_yuv_t* imgbuf);
int mp_maphdle_get_imageaddr_argb(maphdle_id maphdle, mp_maphdle_frm_argb_t* imgbuf);
int mp_maphdle_open(maphdle_id maphdle, memhdle_id memhdle);
int mp_maphdle_close(maphdle_id maphdle);
int mp_maphdle_construct(maphdle_id *maphdle, mp_mapoper_type_t type);
int mp_maphdle_destruct(maphdle_id maphdle);



/***************************************************************************************************
 * brief:     
 * detail:    
 * author:    saicheong.yuen
 * date:      2015-7-15
 ***************************************************************************************************/
typedef enum MP_DISPHDLE_E
{
    MP_DISPHDL_DE_MAIN = 0,
    MP_DISPHDL_DE_AUX,
    MP_DISPHDL_GE,
    MP_DISPHDL_OSD
} mp_dispoper_type_t;


typedef struct mp_disphdle_config_s
{
    struct rect        src_rect;
    struct rect        dst_rect;
	unsigned char      disp_layer_id;/*0 for 1st layer,1 for 2st layer*/
    unsigned char      image_count;
    unsigned char      disposal_method;
} mp_disphdle_config_t;

typedef struct img_dec_dispconfig_s
{
    struct rect           src_rect;
    struct rect           dst_rect;
    mp_disphdle_config_t  de_disp_config;
    mp_disphdle_config_t  gma_disp_config;
    unsigned char	      decode_mode;
    unsigned char         disp_layer_id;/*0 for de,1 for gma*/
} img_dec_dispconfig_t;


int mp_disphdle_open(disphdle_id disphdl);
int mp_disphdle_close(disphdle_id disphdl);

int mp_disphdle_bind(disphdle_id disphdl, maphdle_id mhdl);
int mp_disphdle_config(disphdle_id disphdl, mp_disphdle_config_t* config);
int mp_disphdle_run(disphdle_id disphdl);

int mp_disphdle_construct(disphdle_id *disphdl, mp_dispoper_type_t type);
int mp_disphdle_destruct(disphdle_id disphdl);




#if 1

typedef struct mp_disphdle_frmyuv_s
{
    UINT32  videoinfo_aspect_ratio;
    int     videoinfo_width;
    int     videoinfo_height;
    int     videoinfo_stride;
    UINT8   videoinfo_sample_format;
    UINT8  *frm_y_addr;
    UINT32  frm_y_len;
    UINT8  *frm_c_addr;
    UINT32  frm_c_len;
} mp_disphdle_frmyuv_t,mp_disphdle_frmyc_t;


typedef struct mp_disphdle_frmctrl_s
{
    UINT8  layer;
    UINT8  bvp_act_flag;
    struct rect        src_rect;
    struct rect        dst_rect;
} mp_disphdle_frmctrl_t;


typedef struct mp_disphdle_frmdata_s
{
    mp_disphdle_frmctrl_t ctrl;
    mp_disphdle_frmyuv_t  out;
    mp_disphdle_frmyuv_t  sd_out;
} mp_disphdle_frmdata_t;


#endif



typedef unsigned long proghdle_id;


#define MP_PROGHDLE_TRUNK_START   0
#define MP_PROGHDLE_TRUNK_RESET   1
#define MP_PROGHDLE_TRUNK_PARSE   2
#define MP_PROGHDLE_TRUNK_CHECK   3
#define MP_PROGHDLE_TRUNK_DECODE  4
#define MP_PROGHDLE_TRUNK_TRANS   5
#define MP_PROGHDLE_TRUNK_DISP    6
#define MP_PROGHDLE_TRUNK_END     7

typedef struct mp_proghdle_statinfo_s
{
    int percent;
    int error;
} mp_proghdle_statinfo_t;

typedef struct mp_scaleconfig_s
{
    struct rect   dst_rect;
    unsigned char scale_flag;
} mp_scaleconfig_t;

int mp_maphdle_argb32_to_mpeg2(maphdle_id *maphdle_yc, mp_mapoper_initconf_t *frm_info, mp_scaleconfig_t *scale_config);

#endif


