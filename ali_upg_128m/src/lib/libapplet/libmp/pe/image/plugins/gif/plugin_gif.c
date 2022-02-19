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

#include <api/libmp/pe.h>
#include <api/libmp/gif.h>
#include "../../image_engine.h"
#include "../../plugin.h"
#include <api/libfs2/stdio.h>

#if 0
#define PLUGIN_GIF_PRINT  libc_printf
#else
#define PLUGIN_GIF_PRINT(...)    do{}while(0)
#endif

extern int gif_dec_init(struct pe_image_cfg *pe_image_cfg);
extern int gif_dec_mp(char* filename, img_dec_dispconfig_t* dispconfig);
extern void gif_dec_abort(void);
extern int gif_dec_infoget(char *filename, struct image_info *info);

static OSAL_ID g_gif_mutex = OSAL_INVALID_ID;
#define ENTER_GIF_API()  osal_mutex_lock(g_gif_mutex, OSAL_WAIT_FOREVER_TIME)
#define LEAVE_GIF_API()  osal_mutex_unlock(g_gif_mutex)

typedef struct plugin_gif_mgr_s
{
    char        dec_filename[256];
    img_dec_dispconfig_t dec_config;
	OSAL_ID	             tsk_id;
	mp_callback_func     cb;
} plugin_gif_mgr_t;

plugin_gif_mgr_t plugin_gif_mgr;
//{
//    {0}//{"no"}, {0}, {0}, {OSAL_INVALID_ID}, {NULL}
//};

static int plugin_gif_isourfile(char *filename, void *args)
{
#ifndef	 _USE_32M_MEM_
    char *ext;
	ext = strrchr(filename, '.');
	if (ext)
	{
		if((!strncasecmp(ext, ".gif", 4)))
		{
			return IMAGE_IS_OUR_FILE;
		}
	}
#endif

	return IMAGE_NOT_OUR_FILE;
}

static int plugin_gif_sysinit(struct pe_image_cfg *pe_image_cfg)
{
    PLUGIN_GIF_PRINT("%s %d\n", __FUNCTION__, __LINE__);

	if(g_gif_mutex == OSAL_INVALID_ID)
	{
        g_gif_mutex = osal_mutex_create();
        if(g_gif_mutex == OSAL_INVALID_ID)
	    {
    		PLUGIN_GIF_PRINT("create gif mutex fail\n");
    		return INNO_ERROR_OPEN_HANDLE;
        }
	}

    gif_dec_init(pe_image_cfg);

    return RET_SUCCESS;
}

static int plugin_gif_syscleanup(void)
{
    PLUGIN_GIF_PRINT("%s %d\n", __FUNCTION__, __LINE__);

    return RET_SUCCESS;
}

int plugin_gif_task(UINT32 para1,UINT32 para2)
{
	int ret = RET_SUCCESS;

    ENTER_GIF_API();
    ret = gif_dec_mp(plugin_gif_mgr.dec_filename, &plugin_gif_mgr.dec_config);
    LEAVE_GIF_API();

	if (ret != RET_SUCCESS)
	{
		return IMAGE_DEC_E_FAIL;
	}

    return 0;
}

static int plugin_gif_decode(char *filename, void *args)
{
    char *ext;

    ext = strrchr(filename, '.');
    if (ext)
    {
        if (strncasecmp(ext, ".gif", 4))
        {
            return IMAGE_DEC_E_FAIL;
        }
    }

    ENTER_GIF_API();
    struct image_config *cfg = (struct image_config*)args;

    img_dec_dispconfig_t dec_config;

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

        dec_config.disp_layer_id = 1;
        dec_config.gma_disp_config.disp_layer_id = 0;
        dec_config.gma_disp_config.src_rect.u_start_x = 0;
        dec_config.gma_disp_config.src_rect.u_start_y = 0;
        dec_config.gma_disp_config.src_rect.u_width   = 0;
        dec_config.gma_disp_config.src_rect.u_height  = 0;
        dec_config.gma_disp_config.dst_rect.u_start_x = 0;
        dec_config.gma_disp_config.dst_rect.u_start_y = 0;
        dec_config.gma_disp_config.dst_rect.u_width   = 0;
        dec_config.gma_disp_config.dst_rect.u_height  = 0;
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
        dec_config.disp_layer_id = 0;
        dec_config.de_disp_config.disp_layer_id = 0;
        dec_config.gma_disp_config.disp_layer_id = 0;
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
        dec_config.disp_layer_id = 0;
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
        dec_config.disp_layer_id = 1;
        dec_config.gma_disp_config.disp_layer_id = 0;
    }

	strncpy((char *)plugin_gif_mgr.dec_filename, filename, 256);
	MEMCPY(&plugin_gif_mgr.dec_config, &dec_config, sizeof(img_dec_dispconfig_t));

	//create the decode task
	OSAL_T_CTSK t_ctsk;
	t_ctsk.itskpri = OSAL_PRI_HIGH;
	t_ctsk.stksz = 0xa000;
	t_ctsk.quantum = 10;
	t_ctsk.para1 = (UINT32)filename;
	t_ctsk.para2 = (UINT32)NULL;
	t_ctsk.name[0] = 'G';
	t_ctsk.name[1] = 'I';
	t_ctsk.name[2] = 'F';
	t_ctsk.task = (void *)plugin_gif_task;
	plugin_gif_mgr.tsk_id = osal_task_create(&t_ctsk);
	if(OSAL_INVALID_ID == plugin_gif_mgr.tsk_id)
	{
        LEAVE_GIF_API();
		return IMAGE_DEC_E_OPEN_FAIL;
	}

    LEAVE_GIF_API();
    PLUGIN_GIF_PRINT("gif_dec_decoder task creat!\n");
    return IMAGE_DEC_E_OK;
}

static int plugin_gif_abort(void)
{
    PLUGIN_GIF_PRINT("%s %d\n", __FUNCTION__, __LINE__);

    gif_dec_abort();

    return RET_SUCCESS;
}

static int plugin_gif_getinfo(char *filename, struct image_info *info)
{
    int ret = RET_FAILURE;

    if((NULL == filename) || (NULL == info))
    {
        return ret;
    }

    MEMSET(info, 0, sizeof(struct image_info));

    ENTER_GIF_API();

    ret = gif_dec_infoget(filename, info);

    LEAVE_GIF_API();

    return ret;
}


image_plugin gif_plugin =
{
	.handle = 0,
	.filename = NULL,
	.description = NULL,
	.init = plugin_gif_sysinit,
	.about = NULL,
	.configure = NULL,
	.is_our_file = plugin_gif_isourfile,
	.decode = plugin_gif_decode,
	.show = NULL,
	.abort = plugin_gif_abort,
	.cleanup = plugin_gif_syscleanup,
	.get_info = plugin_gif_getinfo,
	.zoom = NULL,
};


