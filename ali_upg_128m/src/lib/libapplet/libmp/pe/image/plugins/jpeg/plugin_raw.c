#include <types.h>
#include <sys_config.h>
#include <retcode.h>
#include <api/libc/string.h>
#include <api/libc/printf.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>
#include <hld/dis/vpo.h>
#include <hld/decv/vdec_driver.h>
#include <hld/decv/decv.h>

#include <api/libimagedec/imagedec.h>
#include <api/libmp/pe.h>
#include "../../image_engine.h"
#include "../../plugin.h"
#include <api/libfs2/stdio.h>
#include <api/libmp/imgdec_common.h>
#include <api/libmp/imgdec_engine_raw.h>

#if 0
#define IMAGE_RAW_DEBUG      libc_printf
#else
#define IMAGE_RAW_DEBUG(...) do{}while(0)
#endif

#define JPEG_RAW_HDLE_MAX 5

typedef struct jpeg_raw_hdle_s
{
   BOOL                    used;
   imagedec_id             imgdec_id;
   enum vp_display_layer   disp_layer;
   mp_disphdle_config_t    disp_config;
   mp_disphdle_frmdata_t   disp_data;
   imgengi_rawconf_file_t  init_file;
   mp_maphdle_frm_yc_t    init_frm;
   imgengi_rawconf_mem_t   init_mem;
} jpeg_raw_hdle_t;

jpeg_raw_hdle_t jpeg_raw_hdles[JPEG_RAW_HDLE_MAX];


static UINT8 *img_file_addr; //start address of the buffer to restore the image file
static UINT32 img_file_size;  // image file size
static INT32 img_input_pos = 0;

static INT32 img_read_data(file_h fh,BYTE *buf, UINT32 size)
{
	UINT32 read_pos;
	UINT32 read_len;

	read_pos = img_input_pos+size;
	if(read_pos>img_file_size)
	{
		read_len = img_file_size-img_input_pos;
		MEMCPY((void *)buf, (void *)(img_file_addr+img_input_pos), read_len);
		img_input_pos += read_len;
		return read_len;
	}
	else
	{
		MEMCPY((void *)buf, (void *)(img_file_addr+img_input_pos), size);
		img_input_pos+=size;
		return size;
	}
}

static int img_seek_data(UINT32 fh,long offset, UINT32 origin)
{
	if(origin == 0)
	{
		img_input_pos = (UINT32)offset;
		return TRUE;
	}
	return FALSE;
}

int jpeg_raw_alloc(int id)
{
    IMAGE_RAW_DEBUG("%s %d.%d\n", __FUNCTION__, __LINE__, id);

    if ((id < 0) || (id > JPEG_RAW_HDLE_MAX))
        return RET_FAILURE;

    //if (jpeg_raw_hdles[id].used == TRUE)
    //    return RET_FAILURE;

    jpeg_raw_hdle_t *rawhdle = &jpeg_raw_hdles[id];
    memset(rawhdle, 0 ,sizeof(jpeg_raw_hdle_t));

    rawhdle->used = TRUE;

    rawhdle->disp_data.ctrl.layer = VPO_LAYER_MAIN;
    rawhdle->disp_config.src_rect.u_start_x = 0;
    rawhdle->disp_config.src_rect.u_start_y = 0;
    rawhdle->disp_config.src_rect.u_width  = 720;
    rawhdle->disp_config.src_rect.u_height = 576;

    rawhdle->disp_config.dst_rect.u_start_x = 0;
    rawhdle->disp_config.dst_rect.u_start_y = 0;
    rawhdle->disp_config.dst_rect.u_width  = 720;
    rawhdle->disp_config.dst_rect.u_height = 576;
    rawhdle->disp_layer = VPO_LAYER_MAIN;

    return 0;
}

int jpeg_raw_free(int id)
{

    if ((id < 0) || (id > JPEG_RAW_HDLE_MAX))
        return RET_FAILURE;

    jpeg_raw_hdles[id].used = FALSE;

    IMAGE_RAW_DEBUG("%s %d.\n", __FUNCTION__, __LINE__);
    return 0;
}

int jpeg_raw_initconf(int id, void* config)
{

    if ((id < 0) || (id > JPEG_RAW_HDLE_MAX))
        return RET_FAILURE;

    imgengi_raw_conf_t* init_conf;
    init_conf = (imgengi_raw_conf_t*)config;
    jpeg_raw_hdle_t *rawhdle;
    rawhdle = &jpeg_raw_hdles[id];


    IMAGE_RAW_DEBUG("%s init_conf->type=0x%x.\n", __FUNCTION__, init_conf->type);

    if (init_conf->type == IMGENGI_RAW_INITCONF_PECFG) {
        //memcpy(&rawhdle->init_pecfg, &init_conf->data.init_pecfg, sizeof(struct pe_image_cfg));
        return RET_FAILURE;
    } else if (init_conf->type == IMGENGI_RAW_INITCONF_FILE) {
        memcpy(&rawhdle->init_file, &init_conf->data.init_file, sizeof(imgengi_rawconf_file_t));
    } else if (init_conf->type == IMGENGI_RAW_INITCONF_YUVFRM) {
        memcpy(&rawhdle->init_frm, &init_conf->data.init_frm, sizeof(mp_maphdle_frm_yc_t));
    } else if (init_conf->type == IMGENGI_RAW_INITCONF_MEM) {
        memcpy(&rawhdle->init_mem, &init_conf->data.init_mem, sizeof(imgengi_rawconf_mem_t));
    } else {
        return RET_FAILURE;
    }

    return RET_SUCCESS;
}

int jpeg_raw_init(int id)
{
    if ((id < 0) || (id > JPEG_RAW_HDLE_MAX))
        return RET_FAILURE;

    jpeg_raw_hdle_t *rawhdle;
    rawhdle = &jpeg_raw_hdles[id];

    /* initialize the decode buffer, and image file operations */
    imagedec_id imgdec_id = 0;

    imagedec_init_config img_init_info;
    MEMSET(&img_init_info, 0, sizeof(imagedec_init_config));
    img_init_info.decoder_buf     = (UINT8 *)(rawhdle->init_mem.buffer);
    img_init_info.decoder_buf_len = rawhdle->init_mem.buflen;
    img_init_info.frm_y_addr  = (UINT32)rawhdle->init_frm.image_y_addr;
    img_init_info.frm_y_size  = rawhdle->init_frm.image_y_addr_len;
    img_init_info.frm_c_addr  = (UINT32)rawhdle->init_frm.image_c_addr;
    img_init_info.frm_c_size  = rawhdle->init_frm.image_c_addr_len;
    img_init_info.frm2_y_addr = (UINT32)rawhdle->init_frm.image_y_addr;
    img_init_info.frm2_y_size = rawhdle->init_frm.image_y_addr_len;
    img_init_info.frm2_c_addr = (UINT32)rawhdle->init_frm.image_c_addr;
    img_init_info.frm2_c_size = rawhdle->init_frm.image_c_addr_len;
    img_file_addr = (UINT8 *)(rawhdle->init_file.bufffile);
    img_file_size = rawhdle->init_file.bufffile_len;
    img_init_info.fread_callback = img_read_data;
    img_init_info.fseek_callback = img_seek_data;

    imgdec_id = imagedec_init(&img_init_info);
    if (0 == imgdec_id)
    {
        return -1;
    }
    rawhdle->imgdec_id = imgdec_id;

    IMAGE_RAW_DEBUG("%s %d.\n", __FUNCTION__, __LINE__);
    return 0;
}
int jpeg_raw_decconf(int id, void* config)
{
    if ((id < 0) || (id > JPEG_RAW_HDLE_MAX))
        return RET_FAILURE;

    IMAGE_RAW_DEBUG("%s %d.\n", __FUNCTION__, __LINE__);
    return 0;
}
extern int mp_dispoper_dedrv_readout_frm(int id, int de_idx, mp_disphdle_frmyc_t* frmyc);
int jpeg_raw_decode(int id)
{
    if ((id < 0) || (id > JPEG_RAW_HDLE_MAX))
        return RET_FAILURE;

    BOOL ret_code = FALSE;
    int  ret = RET_SUCCESS;

    jpeg_raw_hdle_t *rawhdle;
    rawhdle = &jpeg_raw_hdles[id];
    imagedec_id imgdec_id = 0;
    imgdec_id = rawhdle->imgdec_id;

    /* configure the decode system. */
    imagedec_mode_par dec_par;
    MEMSET((void *)&dec_par, 0, sizeof(imagedec_mode_par));
    dec_par.vp_close_control = 0;
    dec_par.pro_show = 1;

    dec_par.src_rect.u_start_x = 0;
    dec_par.src_rect.u_start_y = 0;
    dec_par.src_rect.u_width = 720;
    dec_par.src_rect.u_height = 576;

    dec_par.dis_rect.u_start_x = 0;
    dec_par.dis_rect.u_start_y = 0;
    dec_par.dis_rect.u_width = 720;
    dec_par.dis_rect.u_height= 576;

    dec_par.show_mode = M_NORMAL;
    imagedec_set_mode(imgdec_id, IMAGEDEC_FULL_SRN, &dec_par);

    struct rect dst_rect;
    dst_rect.u_start_x = 0;
    dst_rect.u_start_y = 0;
    dst_rect.u_width = 720;
    dst_rect.u_height = 2880;

    imagedec_ioctl(imgdec_id, IMAGEDEC_IO_CMD_EXPAND, (UINT32)&dst_rect);
    imagedec_ioctl(imgdec_id, IMAGEDEC_IO_CMD_NODISP, (UINT32)TRUE);

    /* decode. */
    ret_code = imagedec_decode(imgdec_id, 1);
    if (ret_code == TRUE) {
        mp_dispoper_dedrv_readout_frm(0, 0, &jpeg_raw_hdles[id].disp_data.out);
    } else {
        ret = RET_FAILURE;
    }

    imagedec_ioctl(imgdec_id, IMAGEDEC_IO_CMD_NODISP, (UINT32)FALSE);
    imagedec_ioctl(imgdec_id, IMAGEDEC_IO_CMD_EXPAND, (UINT32)NULL);

    if (imagedec_stop(imgdec_id) != TRUE)
        return RET_FAILURE;
    
    if (imagedec_release(imgdec_id) != TRUE)
        return RET_FAILURE;

    IMAGE_RAW_DEBUG("%s %d. %d %d\n", __FUNCTION__, __LINE__, id, imgdec_id);
    return ret;
}
int jpeg_raw_dispconf(int id, void* config)
{
    if ((id < 0) || (id > JPEG_RAW_HDLE_MAX))
        return RET_FAILURE;

    imgengi_raw_conf_t* disp_conf;
    disp_conf = (imgengi_raw_conf_t*)config;
    jpeg_raw_hdle_t *rawhdle;
    rawhdle = &jpeg_raw_hdles[id];

    if (disp_conf->type == IMGENGI_RAW_DISPCONF_RESET) {
        return RET_FAILURE;
    } else if (disp_conf->type == IMGENGI_RAW_DISPCONF_LAYER) {
        rawhdle->disp_layer = disp_conf->data.disp_layer;
    } else if (disp_conf->type == IMGENGI_RAW_DISPCONF_SRCRECT) {
        return RET_FAILURE;
    } else if (disp_conf->type == IMGENGI_RAW_DISPCONF_DSTRECT) {
        rawhdle->disp_config.dst_rect.u_start_x = disp_conf->data.disp_dstrect.u_start_x;
        rawhdle->disp_config.dst_rect.u_start_y = disp_conf->data.disp_dstrect.u_start_y;
        rawhdle->disp_config.dst_rect.u_width  = disp_conf->data.disp_dstrect.u_width;
        rawhdle->disp_config.dst_rect.u_height = disp_conf->data.disp_dstrect.u_height;
    } else {
        return RET_FAILURE;
    }

    IMAGE_RAW_DEBUG("%s %d.\n", __FUNCTION__, __LINE__);
    return 0;
}
int jpeg_raw_display(int id)
{
    if ((id < 0) || (id > JPEG_RAW_HDLE_MAX))
        return RET_FAILURE;

    jpeg_raw_hdle_t *rawhdle;
    rawhdle = &jpeg_raw_hdles[id];
    mp_disphdle_frmyc_t   *frmyuv;
    frmyuv  = &rawhdle->disp_data.out;

    maphdle_id maphdle_yuv;
    mp_maphdle_construct(&maphdle_yuv, MP_MAPHDLE_MPEG2_32x16);

    mp_mapoper_initconf_t init_config;

    init_config.info_image.width  = frmyuv->videoinfo_width;
    init_config.info_image.height = frmyuv->videoinfo_height;
    init_config.initflag |= MP_MAPCOMMON_INITCONF_INFOIMAGE;

    init_config.info_canvas.width  = frmyuv->videoinfo_width;
    init_config.info_canvas.height = frmyuv->videoinfo_height;
    init_config.info_canvas.stride = (frmyuv->videoinfo_stride)*32;
    init_config.info_canvas.sample_format = frmyuv->videoinfo_sample_format;
    init_config.initflag |= MP_MAPCOMMON_INITCONF_INFOCANVAS;

    init_config.mem_frm.yc.image_y_addr = (char *)frmyuv->frm_y_addr;
    init_config.mem_frm.yc.image_c_addr = (char *)frmyuv->frm_c_addr;
    init_config.mem_frm.yc.image_y_addr_len = frmyuv->frm_y_len;
    init_config.mem_frm.yc.image_c_addr_len = frmyuv->frm_c_len;
    init_config.initflag |= MP_MAPCOMMON_INITCONF_MEMFRM;

    mp_maphdle_initconfig(maphdle_yuv, &init_config);
    mp_maphdle_open(maphdle_yuv, 0xff);

    mp_dispoper_type_t disphdle_type;
    if (rawhdle->disp_layer == VPO_LAYER_MAIN) {
        disphdle_type = MP_DISPHDL_DE_MAIN;
    } else {
        disphdle_type = MP_DISPHDL_DE_AUX;
    }

    disphdle_id disphdl_de;
    mp_disphdle_construct(&disphdl_de, disphdle_type);
    mp_disphdle_open(disphdl_de);
    mp_disphdle_bind(disphdl_de, maphdle_yuv);
    mp_disphdle_config(disphdl_de, &rawhdle->disp_config);
    mp_disphdle_run(disphdl_de);

    mp_maphdle_close(maphdle_yuv);
    mp_maphdle_destruct(maphdle_yuv);

    IMAGE_RAW_DEBUG("%s %d.\n", __FUNCTION__, __LINE__);
    return 0;
}

int jpeg_raw_abort(int id)
{
    if ((id < 0) || (id > JPEG_RAW_HDLE_MAX))
        return RET_FAILURE;

#if 0
    imagedec_id imgdec_id = 0;
    imgdec_id = jpeg_raw_hdles[id].imgdec_id;


    imagedec_ioctl(imgdec_id, IMAGEDEC_IO_CMD_NODISP, (UINT32)FALSE);
    imagedec_ioctl(imgdec_id, IMAGEDEC_IO_CMD_EXPAND, (UINT32)NULL);

    if (imagedec_stop(imgdec_id) != TRUE)
        return -1;
    
    if (imagedec_release(imgdec_id) != TRUE)
        return -1;
#endif

    IMAGE_RAW_DEBUG("%s %d.\n", __FUNCTION__, __LINE__);
    return 0;
}

const plugin_image_raw jpeg_raw =
{
    .raw_alloc = jpeg_raw_alloc,
    .raw_free = jpeg_raw_free,
    .raw_initconf = jpeg_raw_initconf,
    .raw_init = jpeg_raw_init,
    .raw_decconf = jpeg_raw_decconf,
    .raw_decode = jpeg_raw_decode,
    .raw_dispconf = jpeg_raw_dispconf,
    .raw_display = jpeg_raw_display,
    .raw_abort = jpeg_raw_abort,
	.raw_info = NULL,
};

