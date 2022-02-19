/*****************************************************************************
*    Copyrights(C) 2005 ALi Corp. All Rights Reserved.
*
*    Company confidential and Properietary information.
*    This information may not be disclosed to unauthorized
*    individual.
*
*    File:    imagedec.h
*
*    Description:
*
*    History:
*           Date            Athor        Version          Reason
*        ============    =============    =========    =================
*     1.  2007.4.29        john li        1.0.0          Create file.
*****************************************************************************/

#ifndef _IMAGEDEC_HEADER_
#define _IMAGEDEC_HEADER_
#ifdef __cplusplus
extern "C"
{
#endif
#include <sys_config.h>
#include <basic_types.h>
#include <mediatypes.h>

#define JPGD_GRAYSCALE 0
#define JPGD_YH1V1     1 //4:4:4
#define JPGD_YH2V1     2 //4:2:2
#define JPGD_YH1V2     3 //
#define JPGD_YH2V2     4 //4:2:0
#define JPGD_YH4V1     5 //4:1:1

#define IMAGEDEC_MAX_INSTANCE 3

/*status control wait flag ptn*/
#define IMAGEDEC_STOP_PTN            0x00000001
#define IMAGEDEC_DECD_PTN            0x00000002
#define IMAGEDEC_FILL_IN_BUF_PTN   0x00000004

#define IMAGEDEC_TIME_OUT_DECD    (20000)
#define IMAGEDEC_TIME_OUT_STOP    (1000)
#define IMAGEDEC_TIME_OUT_READ    OSAL_WAIT_FOREVER_TIME

/*imagedec io cmd list*/
#define IMAGEDEC_IO_CMD_OPEN_HW_ACCE        0x01
#define IMAGEDEC_IO_CMD_FILL_LOGO           0x02
#define IMAGEDEC_IO_CMD_CLEAN_FRM           0x03
#define IMAGEDEC_IO_CMD_OSD_SHOW            0x04
#define IMAGEDEC_IO_CMD_COPY_FRM            0x05
#define IMAGEDEC_IO_CMD_DIS_FRM_INFO        0x06
#define IMAGEDEC_IO_CMD_NEED_LOGO           0x07
#define IMAGEDEC_IO_CMD_CFG_DEO_FRM         0x08
#define IMAGEDEC_IO_CMD_NEED_DELAY          0x09
#define IMAGEDEC_IO_CMD_OPEN_TV             0x0A
#define IMAGEDEC_IO_CMD_EXPAND              0x0B
#define IMAGEDEC_IO_CMD_NODISP              0x0C   //!< io control command, jpeg decode but no display

#define IMAGEDEC_SHOW_MODE_PAR_LEN     (4)

enum IMAGE_JPG_CODING_TYPE
{
    SEQUENTIAL_JPEG,
    PROGRESSIVE_JPEG
};

enum FRM_UNIT_TYPE
{
    FRM_UNIT_36_MP2,            // for 3602 series 32*16 memory mapping mode
    FRM_UNIT_27                // for 27   series 16*16 memory mapping mode
};

enum IMAGE_DIS_MODE
{
    IMAGEDEC_FULL_SRN = 1,
    IMAGEDEC_REAL_SIZE,
    IMAGEDEC_THUMBNAIL,
    IMAGEDEC_AUTO,
    IMAGEDEC_SIZEDEFINE,
    IMAGEDEC_MULTI_PIC,
    RESERVED_MODE,
};
enum IMAGE_ANGLE
{
    ANG_ORI,
    ANG_90_A ,    /*Along the clock with 90*/
    ANG_180,
    ANG_90_C,    /*Counter the clock with 90*/
};

enum IMAGE_SHOW_MODE
{
    M_NORMAL,
    M_SHUTTERS,
    M_BRUSH,
    M_SLIDE,
    M_RANDOM,
    M_FADE,
};

enum IMAGE_DEC_ERROR
{
    IMG_ER_FILE = 1,
    IMG_ER_MEM,
};

/*every image decoder with a device id*/
typedef unsigned long imagedec_id;
/*handle point to the jpeg file*/
typedef unsigned long file_h;

typedef enum
{
    NO_SUPPORT,
    JPEG,
    bitmap,
    RESERVED_TYPE,
}imagedec_file_format;

typedef struct image_info_t
{
    imagedec_file_format file_type;        //only support jpeg and BMP
    int precision;        // bit count of pixel
    int image_width;
    int image_height;
    int scan_type;
    int coding_type;            //progressive or sequential
    int coding_progress;        //for slide show solution know the progress of the decoding. (value>>16) is 0.x of the work
    UINT8  density_unit;
    UINT16 density_x;
    UINT16 density_y;
    int reserved;
}image_info, *p_image_info;

typedef struct imagedec_init_config_t
{
    /*dst frame info*/
    UINT32 frm_y_addr;
    UINT32 frm_y_size;
    UINT32 frm_c_addr;
    UINT32 frm_c_size;
    UINT32 frm2_y_addr;
    UINT32 frm2_y_size;
    UINT32 frm2_c_addr;
    UINT32 frm2_c_size;
    UINT32 frm_mb_type;/*reserved for 36XX series with different MB width. 33XX : 16  36XX : 32*/

    /*buf for dec internal usage*/
    UINT8 *decoder_buf;
    UINT32 decoder_buf_len;

    /*file operation callback function*/
    INT32 (*fread_callback)(file_h fh,UINT8 *buf, UINT32 size);
    BOOL (*fseek_callback)(file_h fh,INT32 offset, UINT32 origin);
    int (*ftell_callback)(file_h fh);
    /*external status info callback function*/
    UINT32 (*imagedec_status)(void *value);

    //backup the old frame buffer
    UINT32 frm3_y_addr;
    UINT32 frm3_y_size;
    UINT32 frm3_c_addr;
    UINT32 frm3_c_size;

    UINT32 frm4_y_addr;
    UINT32 frm4_y_size;
    UINT32 frm4_c_addr;
    UINT32 frm4_c_size;
  void (*ali2dto3d_callback)(unsigned char *pic_out_buf, unsigned char *pic_in_buf,
                    int pic_width, int pic_height, int output_format,
                    int user_depth, int user_shift, int user_vip);
}imagedec_init_config , *p_imagedec_init_config;

typedef struct imagedec_out_image_t
{
    UINT32 start_buf;
    UINT16 width;
    UINT16 height;
    UINT32 bpp;        // bits per pixel
    UINT32 stride;     // bytes of every line
}imagedec_out_image,*p_imagedec_out_image;

typedef struct imagedec_osd_config_t
{
    void *handle;
    UINT32 pic_frmt;
    void (*blt)(void *,p_imagedec_out_image,struct rect *);
}imagedec_osd_config,*p_imagedec_osd_config;

typedef struct imagedec_osd_io_cmd_t
{
    UINT32 on:1;
    UINT32 res:31;

    struct rect dis_rect;
    UINT32 bitmap_start; // restore the decompressed picture data
    UINT32 bitmap_size;
}imagedec_osd_io_cmd,*p_imagedec_osd_io_cmd;

typedef struct imagedec_io_cmd_copy_frm_t
{
    void *logo_buf;
    UINT32 y_buf_size;
    UINT32 c_buf_size;
    UINT16 width;
    UINT16 height;
}imagedec_io_cmd_copy_frm,*p_imagedec_io_cmd_copy_frm;

typedef struct imagedec_io_cmd_frm_info_t
{
    UINT32 y_buf;
    UINT32 c_buf;
    UINT32 y_size;
    UINT32 c_size;
}imagedec_io_cmd_frm_info,*p_imagedec_io_cmd_frm_info;

typedef struct imagedec_show_shutters_t
{
    UINT8 direction; //0 : horizontal 1: vertical
    UINT8 type;
    UINT16 time; // ms
}imagedec_show_shutters,*p_imagedec_show_shutters;

typedef struct imagedec_show_brush_t
{
    UINT8 direction; //0 : from left to right 1: from top to bottom
    UINT8 type;
    UINT16 time; // ms
}imagedec_show_brush,*p_imagedec_show_brush;

typedef struct imagedec_show_slide_t
{
    UINT8 direction; //0 : from left to right 1: from top to bottom
    UINT8 type;
    UINT16 time; // ms
}imagedec_show_slide,*p_imagedec_show_slide;

typedef struct imagedec_show_random_t
{
    UINT8 type;    // 0: random block operation
    UINT8 res;
    UINT16 time; // ms
}imagedec_show_show_random,*p_imagedec_show_random;

typedef struct imagedec_show_fade_t
{
    UINT8 type;
    UINT8 res;
    UINT16 time; // ms
}imagedec_show_fade,*p_imagedec_show_fade;

typedef struct imagedec_mode_par_t
{
    struct rect src_rect;    /*source rectangule, only used in Real_size mode*/
    struct rect dis_rect;    /*display rectangule,only used in ThumbNail mode*/
    BOOL pro_show;    /*whether show the part of the pic when decoding it*/
    BOOL vp_close_control;    /*whether close vp once*/
    enum IMAGE_SHOW_MODE show_mode;
    UINT8 *show_mode_par;
    UINT8    need_logo;
    BOOL file_type;  /*whether local file or not: 0 -- local, 1 -- other*/
    UINT32 reserved;
}imagedec_mode_par,*p_imagedec_mode_par;

typedef struct image_display_par_t
{
    UINT8 *y;
    UINT32 y_len;
    UINT8 *c;
    UINT32 c_len;

    UINT32 width;
    UINT32 height;

    int scantype;
}image_display_par, *p_image_display_par;

imagedec_id imagedec_init(p_imagedec_init_config pconfig);
BOOL imagedec_release(imagedec_id id);
BOOL imagedec_getinfo(imagedec_id id,file_h fh,p_image_info p_info);
BOOL imagedec_decode(imagedec_id id,file_h fh);
BOOL imagedec_stop(imagedec_id id);
BOOL imagedec_set_mode(imagedec_id id, enum IMAGE_DIS_MODE mode,p_imagedec_mode_par p_par);
BOOL imagedec_dis_next_pic(imagedec_id id,UINT32 mode,UINT32 time_out);/*only active when pro_show is FALSE*/
BOOL imagedec_zoom(imagedec_id id,struct rect dis_rect,struct rect src_rect);
BOOL imagedec_rotate(imagedec_id id,enum IMAGE_ANGLE angle,UINT32 mode);
BOOL imagedec_ioctl(imagedec_id id,UINT32 cmd, UINT32 para);
BOOL imagedec_display(imagedec_id id, p_image_display_par p_disinfo);
INT32 imagedec_check_fileheader(imagedec_id id, file_h fh);
BOOL imagedec_ioctl_ex(imagedec_id id,UINT32 cmd, UINT32 para);

void imagedec_osd_init(p_imagedec_osd_config config);

int imagedec_de_config_imagetoplay();
int imagedec_de_config_videotoplay();

void imagedec_set_disp_param(imagedec_id id, int display_type, int output_format,
                              int user_depth, int user_shift, int user_vip);
BOOL imagedec_3d_user_option(imagedec_id id, int output_format, int user_depth, int user_shift, int user_vip);
BOOL imagedec_swap(imagedec_id id, int show_type);

#ifdef __cplusplus
 }
#endif
#endif

