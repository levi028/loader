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
#include <hld/ge/ge.h>
#include <hld/osd/osddrv.h>

#include <api/libimagedec/imagedec.h>
#include <api/libmp/png.h>
#include <api/libmp/pe.h>
#include "../../image_engine.h"
#include "../../plugin.h"
#include <api/libfs2/stdio.h>
#include <api/libmp/imgdec_common.h>

//mp_callback_func        mp_png_cb; 

#if 0
#define PLUGIN_PNG_PRINT  libc_printf
#else
#define PLUGIN_PNG_PRINT(...)    do{}while(0)
#endif

typedef struct plugin_png_mgr_s
{
    char                 dec_filename[256];
    png_dec_dispconfig_t dec_config;
	int                  tsk_flag;
	OSAL_ID	             tsk_id;
	mp_callback_func     cb; 
} plugin_png_mgr_t;

plugin_png_mgr_t plugin_png_mgr;
//{
//    {0}//{"no"}, {0}, {0}, {OSAL_INVALID_ID}, {NULL}
//};

typedef struct mp_playhdle_info_s
{
    int img_width;
    int img_height;
    int img_bpp;
    int img_fsize;
} mp_playhdle_info_t;

extern int png_dec_init(png_dec_initconfig_t *cfg);
extern int png_dec_filename(char* filename, png_dec_dispconfig_t* dispconfig);
extern int png_abort();
extern int png_cleanup();
extern int png_dec_infoget(char* filename, mp_playhdle_info_t* play_info);
extern int png_dec_zoom(struct rect *dst_rect, struct rect *src_rect);

static int plugin_png_isourfile(char *filename, void *args)
{
	char *ext;
    plugin_png_mgr.tsk_flag = 0;
#ifndef	 _USE_32M_MEM_	
	ext = strrchr(filename, '.');
	if (ext)
	{
		if((!strncasecmp(ext, ".png", 4)))
		{
			return IMAGE_IS_OUR_FILE;
		}
	}
#endif
	return IMAGE_NOT_OUR_FILE;
}


static UINT32 _plugin_png_status(void *value)
{
    mp_proghdle_statinfo_t* statinfo;

	statinfo = (mp_proghdle_statinfo_t*) value;



	if (plugin_png_mgr.cb != NULL)
	{
		plugin_png_mgr.cb(MP_IMAGE_DECODE_PROCESS, statinfo->percent);
        //plugin_png_mgr.cb(MP_IMAGE_DECODER_ERROR, err);
	}
    return 0;
}


static int plugin_png_sysinit(struct pe_image_cfg *pe_image_cfg)
{
    PLUGIN_PNG_PRINT("%s %d\n", __FUNCTION__, __LINE__);

    png_dec_initconfig_t init_config;

    init_config.mem_alloc = (UINT32)pe_image_cfg->decoder_buf;
    init_config.mem_size  = pe_image_cfg->decoder_buf_len;

    init_config.frm1_y_addr = pe_image_cfg->frm_y_addr;
    init_config.frm1_y_size = pe_image_cfg->frm_y_size;
    init_config.frm1_c_addr = pe_image_cfg->frm_c_addr;
    init_config.frm1_c_size = pe_image_cfg->frm_c_size;

    init_config.frm2_y_addr = pe_image_cfg->frm2_y_addr;
    init_config.frm2_y_size = pe_image_cfg->frm2_y_size;
    init_config.frm2_c_addr = pe_image_cfg->frm2_c_addr;
    init_config.frm2_c_size = pe_image_cfg->frm2_c_size;

    init_config.frm3_y_addr = pe_image_cfg->frm3_y_addr;
    init_config.frm3_y_size = pe_image_cfg->frm3_y_size;
    init_config.frm3_c_addr = pe_image_cfg->frm3_c_addr;
    init_config.frm3_c_size = pe_image_cfg->frm3_c_size;
    init_config.frm4_y_addr = pe_image_cfg->frm4_y_addr;
    init_config.frm4_y_size = pe_image_cfg->frm4_y_size;
    init_config.frm4_c_addr = pe_image_cfg->frm4_c_addr;
    init_config.frm4_c_size = pe_image_cfg->frm4_c_size;
	init_config.status      = _plugin_png_status;

    png_dec_init((png_dec_initconfig_t *)&init_config);

    //mp_png_cb = pe_image_cfg->mp_cb;

    return RET_SUCCESS;
}

void plugin_png_task(UINT32 para1,UINT32 para2)
{
	int ret = RET_SUCCESS;

    ret = png_dec_filename(plugin_png_mgr.dec_filename, &plugin_png_mgr.dec_config);
	plugin_png_mgr.tsk_flag = 0;
	if (ret != RET_SUCCESS)
	{
		return ;//IMAGE_DEC_E_FAIL;
	}
 
	if (plugin_png_mgr.cb != NULL)
        plugin_png_mgr.cb(MP_IMAGE_PLAYBACK_END, 0);

    return;
}



static int plugin_png_decode(char *filename, void *args)
{
//	int ret = RET_SUCCESS;

	if (plugin_png_mgr.tsk_flag == 1)
		return IMAGE_DEC_E_OPEN_FAIL;

	plugin_png_mgr.tsk_flag = 1;


    PLUGIN_PNG_PRINT("%s %d\n", __FUNCTION__, __LINE__);\

    char *ext;

    ext = strrchr(filename, '.');
    if (ext)
    {
        if (strncasecmp(ext, ".png", 4))
        {
            return IMAGE_DEC_E_FAIL;
        }
    }

    struct image_config *cfg = (struct image_config*)args;

    png_dec_dispconfig_t dec_config;
    dec_config.decode_mode = cfg->decode_mode;
    if ((cfg->decode_mode == IMAGEDEC_FULL_SRN))
    {
        dec_config.de_disp_config.src_rect.u_start_x = 0;
        dec_config.de_disp_config.src_rect.u_start_y = 0;
        dec_config.de_disp_config.src_rect.u_width  = 720;
        dec_config.de_disp_config.src_rect.u_height = 576;
        dec_config.de_disp_config.dst_rect.u_start_x = cfg->dest_left;
        dec_config.de_disp_config.dst_rect.u_start_y = cfg->dest_top;
        dec_config.de_disp_config.dst_rect.u_width   = cfg->dest_width;
        dec_config.de_disp_config.dst_rect.u_height = cfg->dest_height;
        dec_config.src_rect.u_start_x = cfg->dest_left;
        dec_config.src_rect.u_start_y = cfg->dest_top;
        dec_config.src_rect.u_width  = cfg->dest_width;
        dec_config.src_rect.u_height = cfg->dest_height;
	    dec_config.dst_rect.u_start_x = cfg->dest_left;
	    dec_config.dst_rect.u_start_y = cfg->dest_top;
	    dec_config.dst_rect.u_width   = cfg->dest_width;
	    dec_config.dst_rect.u_height  = cfg->dest_height;
        vpo_win_onoff_ext((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), FALSE, VPO_LAYER_MAIN);
    }
    else if ((cfg->decode_mode == IMAGEDEC_THUMBNAIL))
    {
        dec_config.de_disp_config.src_rect.u_start_x = cfg->dest_left;
        dec_config.de_disp_config.src_rect.u_start_y = cfg->dest_top;
        dec_config.de_disp_config.src_rect.u_width  = cfg->dest_width;
        dec_config.de_disp_config.src_rect.u_height = cfg->dest_height;
        dec_config.de_disp_config.dst_rect.u_start_x = cfg->dest_left;
        dec_config.de_disp_config.dst_rect.u_start_y = cfg->dest_top;
        dec_config.de_disp_config.dst_rect.u_width  = cfg->dest_width;
        dec_config.de_disp_config.dst_rect.u_height = cfg->dest_height;
        dec_config.src_rect.u_start_x = cfg->dest_left;
        dec_config.src_rect.u_start_y = cfg->dest_top;
        dec_config.src_rect.u_width  = cfg->dest_width;
        dec_config.src_rect.u_height = cfg->dest_height;
        dec_config.dst_rect.u_start_x = cfg->dest_left;
    	dec_config.dst_rect.u_start_y = cfg->dest_top;
    	dec_config.dst_rect.u_width   = cfg->dest_width;
    	dec_config.dst_rect.u_height  = cfg->dest_height;
    }
    else if ((cfg->decode_mode == IMAGEDEC_MULTI_PIC))
    {
        dec_config.de_disp_config.src_rect.u_start_x = 0;
        dec_config.de_disp_config.src_rect.u_start_y = 0;
        dec_config.de_disp_config.src_rect.u_width  = 720;
        dec_config.de_disp_config.src_rect.u_height = 576;
        dec_config.de_disp_config.dst_rect.u_start_x = 0;
        dec_config.de_disp_config.dst_rect.u_start_y = 0;
        dec_config.de_disp_config.dst_rect.u_width   = 720;
        dec_config.de_disp_config.dst_rect.u_height = 576;
        dec_config.src_rect.u_start_x = cfg->dest_left;
        dec_config.src_rect.u_start_y = cfg->dest_top;
        dec_config.src_rect.u_width  = cfg->dest_width;
        dec_config.src_rect.u_height = cfg->dest_height;
        dec_config.dst_rect.u_start_x = cfg->dest_left;
        dec_config.dst_rect.u_start_y = cfg->dest_top;
        dec_config.dst_rect.u_width  = cfg->dest_width;
        dec_config.dst_rect.u_height = cfg->dest_height;
    }
    else
    {
        dec_config.de_disp_config.src_rect.u_start_x = 0;
        dec_config.de_disp_config.src_rect.u_start_y = 0;
        dec_config.de_disp_config.src_rect.u_width  = 720;
        dec_config.de_disp_config.src_rect.u_height = 576;
        dec_config.de_disp_config.dst_rect.u_start_x = cfg->dest_left;
        dec_config.de_disp_config.dst_rect.u_start_y = cfg->dest_top;
        dec_config.de_disp_config.dst_rect.u_width   = cfg->dest_width;
        dec_config.de_disp_config.dst_rect.u_height = cfg->dest_height;
        dec_config.src_rect.u_start_x = cfg->dest_left;
        dec_config.src_rect.u_start_y = cfg->dest_top;
        dec_config.src_rect.u_width  = cfg->dest_width;
        dec_config.src_rect.u_height = cfg->dest_height;
        dec_config.dst_rect.u_start_x = cfg->dest_left;
        dec_config.dst_rect.u_start_y = cfg->dest_top;
        dec_config.dst_rect.u_width  = cfg->dest_width;
        dec_config.dst_rect.u_height = cfg->dest_height;
    }

	//memset(plugin_png_mgr.dec_filename, 0, 128);
	memcpy(plugin_png_mgr.dec_filename, filename, 256);
	memcpy(&plugin_png_mgr.dec_config, &dec_config, sizeof(png_dec_dispconfig_t));

	plugin_png_mgr.cb = cfg->mp_cb;
#if 1

	OSAL_T_CTSK t_ctsk;

	//create the decode task
	t_ctsk.itskpri = OSAL_PRI_HIGH;
	t_ctsk.stksz = 0x6000;
	t_ctsk.quantum = 10;
	t_ctsk.para1 = (UINT32)filename;
	t_ctsk.para2 = (UINT32)NULL;
	t_ctsk.name[0] = 'P';
	t_ctsk.name[1] = 'N';
	t_ctsk.name[2] = 'G';
	t_ctsk.task = plugin_png_task;
	plugin_png_mgr.tsk_id = osal_task_create(&t_ctsk);
	if(OSAL_INVALID_ID == plugin_png_mgr.tsk_id)
	{
		return IMAGE_DEC_E_OPEN_FAIL;
	}
#endif
    return IMAGE_DEC_E_OK;
}

static int plugin_png_abort(void)
{
    PLUGIN_PNG_PRINT("%s %d\n", __FUNCTION__, __LINE__);

    png_abort();

    return RET_SUCCESS;
}

static int plugin_png_cleanup(void)
{
    PLUGIN_PNG_PRINT("%s %d\n", __FUNCTION__, __LINE__);

    png_cleanup();

    return RET_SUCCESS;
}


/*  the struct mp_playhdle_info_t need to remove */


static int plugin_png_infoget(char *filename, struct image_info *info)
{

	if (plugin_png_mgr.tsk_flag == 1)
		return IMAGE_DEC_E_FAIL;

	plugin_png_mgr.tsk_flag = 1;


    PLUGIN_PNG_PRINT("%s %d\n", __FUNCTION__, __LINE__);
    int ret = RET_FAILURE;

    MEMSET(info, 0, sizeof(struct image_info));
    mp_playhdle_info_t  play_info;

    ret = png_dec_infoget(filename, &play_info);
    if (ret == RET_SUCCESS) {
        info->width  = play_info.img_width;
        info->height = play_info.img_height;
        info->bbp    = play_info.img_bpp;
        info->fsize  = play_info.img_fsize;
    }

    plugin_png_mgr.tsk_flag = 0;

    return ret;
}

static int plugin_png_zoom(struct rect *dst_rect, struct rect *src_rest)
{
    png_dec_zoom(dst_rect, src_rest);

    return RET_SUCCESS;
}


#if 1
image_plugin png_plugin =
{
	.handle      = 0,
	.filename    = NULL,
	.description = NULL,
	.init        = plugin_png_sysinit,
	.about       = NULL,
	.configure   = NULL,
	.is_our_file = plugin_png_isourfile,
	.decode      = plugin_png_decode,
	.show        = NULL,
	.abort       = plugin_png_abort,
	.cleanup     = plugin_png_cleanup,
	.get_info    = plugin_png_infoget,
	.rotate      = NULL,
	.zoom        = plugin_png_zoom,
};
#endif

