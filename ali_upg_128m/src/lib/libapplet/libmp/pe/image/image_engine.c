#include <types.h>
#include <sys_config.h>
#include <retcode.h>
#include <api/libc/string.h>
#include <api/libc/printf.h>
#include <osal/osal.h>
#include <api/libmp/pe.h>
#include <hld/dis/vpo.h>
#include <hld/decv/decv.h>
#include "image_engine.h"
#include "plugin.h"

extern image_plugin jpeg_plugin;
extern image_plugin png_plugin;
extern image_plugin gif_plugin;

#ifdef SEE_CPU
void hld_cpu_img_cb(unsigned long type, unsigned long param);
#endif

static image_plugin* plugins[] =
{
#ifndef _DISABLE_MEDIA_PLAYER_    
    &jpeg_plugin,
    &gif_plugin,
    &png_plugin,
#endif    
    NULL,
};

#define IMAGE_STATE_STOP        0
#define IMAGE_STATE_DECODE        1
#define IMAGE_STATE_SHOW        2

struct image_plugin_data
{
    unsigned long    state;
    image_plugin     *plugin;
};

struct image_plugin_data image_plugin_data;

#if 0
static image_plugin *get_current_plugin(void)
{
    return image_plugin_data.plugin;
}
#endif
OSAL_ID flag_image;

static mp_callback_func func_mp_cb;
int image_engine_init(struct pe_image_cfg *pe_image_cfg)
{
    image_plugin *plugin = NULL;
    image_plugin **head = NULL;
    BOOL bl_addr_range_legal = TRUE;

    /* Addr legal check */
    if(NULL != pe_image_cfg)
    {
        bl_addr_range_legal  = osal_check_address_range_legal((void *)pe_image_cfg->frm_y_addr, pe_image_cfg->frm_y_size);
        bl_addr_range_legal &= osal_check_address_range_legal((void *)pe_image_cfg->frm_c_addr, pe_image_cfg->frm_c_size);
        bl_addr_range_legal &= osal_check_address_range_legal((void *)pe_image_cfg->frm2_y_addr, pe_image_cfg->frm2_y_size);
        bl_addr_range_legal &= osal_check_address_range_legal((void *)pe_image_cfg->frm2_c_addr, pe_image_cfg->frm2_c_size);
        bl_addr_range_legal &= osal_check_address_range_legal((void *)pe_image_cfg->frm3_y_addr, pe_image_cfg->frm3_y_size);
        bl_addr_range_legal &= osal_check_address_range_legal((void *)pe_image_cfg->frm3_c_addr, pe_image_cfg->frm3_c_size);
        bl_addr_range_legal &= osal_check_address_range_legal((void *)pe_image_cfg->frm4_y_addr, pe_image_cfg->frm4_y_size);
        bl_addr_range_legal &= osal_check_address_range_legal((void *)pe_image_cfg->frm4_c_addr, pe_image_cfg->frm4_c_size);
        bl_addr_range_legal &= osal_check_address_range_legal((void *)pe_image_cfg->decoder_buf, pe_image_cfg->decoder_buf_len);
        if (FALSE == bl_addr_range_legal)
        {
            libc_printf("%s:%d Addr Illegal!\n",__func__,__LINE__);
            return -1;
        }
#ifdef SEE_CPU
    if(pe_image_cfg->mp_cb)
        func_mp_cb = hld_cpu_img_cb;
    else
#endif
    {
        func_mp_cb = pe_image_cfg->mp_cb;
    }
    /* by wen for secure coding, BD-PB-NP-1 */
    }

    for(head = plugins;(plugin = (*head)); head++)
    {
        if(plugin->init)
            plugin->init(pe_image_cfg);

    }

    return 0;

}

int image_engine_cleanup()
{
    image_plugin *plugin = NULL;
    image_plugin **head = plugins;

    for(head = plugins;(plugin = (*head)); head++)
    {
        if(plugin->cleanup)
            plugin->cleanup();

    }

    return 0;
}

int image_engine_rotate(unsigned char rotate_angle)
{
    image_plugin *plugin = NULL;
    image_plugin **head = NULL;
    int ret = -1;

    for(head = plugins; (plugin = (*head)); head ++)
    {
        if(plugin->rotate)
            ret = plugin->rotate(rotate_angle);
    }

    return ret;
}

int image_engine_decode(char *filename, struct image_config *cfg)
{
    int ret = -1;
    BOOL try_image_decoder = TRUE;
    image_plugin *plugin = NULL;
    image_plugin **head = plugins;
    struct image_display_t *dis = (struct image_display_t *)cfg;

    for(head = plugins;(plugin = (*head)); head++)
    {
        if(plugin->is_our_file(filename, dis))
        {
            try_image_decoder = FALSE;
            break;
        }
    }

    if(try_image_decoder) goto LBL_OTHER_IMAGE_DECODER;

    if(NULL == plugin)
    {
        return -1;
    }

    MEMSET(&image_plugin_data,     0, sizeof(struct image_plugin_data));
    image_plugin_data.plugin = plugin;

    if((image_plugin_data.plugin) && (image_plugin_data.plugin->decode))
    {
         if (3 == strlen(filename)  && !strcasecmp(filename, "YUV"))
         {
             dis->mp_cb = func_mp_cb;
             ret = plugin->decode(filename, dis);
         }
        else
        {
            cfg->mp_cb = func_mp_cb;
            ret = plugin->decode(filename, cfg);
        }
    }
    if(ret == IMAGE_DEC_E_OPEN_FAIL)
        return -1;

    if (ret == IMAGE_DEC_E_OK)
        image_plugin_data.state = IMAGE_STATE_DECODE;
    else
    {
        try_image_decoder = TRUE;
        goto LBL_OTHER_IMAGE_DECODER;
    }

    return ret;

LBL_OTHER_IMAGE_DECODER:

    head = plugins;
    while( NULL != *head && try_image_decoder)
    {
        plugin = *head;

        if( plugin == NULL )
            return -1;

        MEMSET(&image_plugin_data,     0, sizeof(struct image_plugin_data));
        image_plugin_data.plugin = plugin;

        if((image_plugin_data.plugin) && (image_plugin_data.plugin->decode))
        {

            cfg->mp_cb = func_mp_cb;
            ret = plugin->decode(filename, cfg);
        }

        if(ret == 0)
        {
            image_plugin_data.state = IMAGE_STATE_DECODE;
            try_image_decoder = FALSE;
            break;
        }
        head++;
    }

    return ret;
}

int image_engine_show()
{
    if(image_plugin_data.state != IMAGE_STATE_DECODE)
    {
        return -1;
    }
    if((image_plugin_data.plugin) && (image_plugin_data.plugin->show))
    {
        image_plugin_data.plugin->show();
    }
    image_plugin_data.state = IMAGE_STATE_SHOW;
    return 0;
}

int image_engine_abort()
{
/*    if(image_plugin_data.state != IMAGE_STATE_DECODE)
    {
        return -1;
    }
*/    if((image_plugin_data.plugin) && (image_plugin_data.plugin->abort))
    {
        image_plugin_data.plugin->abort();
    }
    image_plugin_data.state = IMAGE_STATE_STOP;
    return 0;

}
int image_engine_2d_to_3d_swap(int type)
{
    if((image_plugin_data.plugin) && (image_plugin_data.plugin->swap))
    {
        image_plugin_data.plugin->swap(type);
    }

    return 0;
}


int image_engine_3d_user_option(struct image_3d_para *para)
{
    int output_format = para->output_format;
    int user_depth = para->user_depth;
    int user_shift = para->user_shift;
    int user_vip = para->user_vip;
    if((image_plugin_data.plugin) && (image_plugin_data.plugin->option))
    {
        image_plugin_data.plugin->option(output_format, user_depth, user_shift, user_vip);
    }

    return 0;
}

int image_engine_set_disp_param(int display_type, struct image_3d_para *para)
{
    int output_format = para->output_format;
    int user_depth = para->user_depth;
    int user_shift = para->user_shift;
    int user_vip = para->user_vip;
    if((image_plugin_data.plugin) && (image_plugin_data.plugin->option))
    {
        image_plugin_data.plugin->set_param(display_type, output_format, user_depth, user_shift, user_vip);
    }

    return 0;
}


int image_engine_zoom(struct rect *dst_rect, struct rect *src_rect)
{
    if((image_plugin_data.plugin) && (image_plugin_data.plugin->zoom))
    {
        image_plugin_data.plugin->zoom(dst_rect, src_rect);
    }
    return 0;
}

int image_engine_get_info(char *filename, struct image_info *info)
{
    int ret = -1;
    image_plugin *plugin = NULL;
    image_plugin **head = plugins;

    for(head = plugins;(plugin = (*head)); head++)
    {
        if(plugin->is_our_file(filename, NULL))
            break;

    }

    if(NULL == plugin)
    {
        return -1;
    }

    if(plugin->get_info)
    {
        ret  = plugin->get_info(filename, info);
    }

    return ret;
}

/***************************************************************************************************
 * brief:     opentv feature support
 * detail:    methods:
 *            1. image_engine_init_opentv
 *            2. image_engine_decode_opentv
 *            3. image_engine_display_opentv
 *            4. image_engine_get_info_opentv
 *            5. image_engine_get_decoded_buffer
 * author:    saicheong.yuen
 * date:      2014-11-26
 ***************************************************************************************************/

struct image_plugin_data image_opentv_plugin;


int image_engine_init_opentv(struct pe_image_cfg *pe_image_cfg)
{
	image_plugin *plugin;
	image_plugin **head;

#ifdef SEE_CPU
    if(pe_image_cfg->mp_cb)
        func_mp_cb = hld_cpu_img_cb;
    else
#endif
    {
	    func_mp_cb = pe_image_cfg->mp_cb;
    }


	for(head = plugins;(plugin = (*head)); head++)
	{
		if(plugin->opentv.init_opentv)
			return plugin->opentv.init_opentv(pe_image_cfg);

	}	
    return 0;
}



int image_engine_decode_opentv(char *filename, UINT32 file_addr, UINT32 file_len, int scale_coef, struct image_config *cfg)
{
	int ret = -1;
	BOOL try_image_decoder = TRUE;
	image_plugin *plugin;
	image_plugin **head = plugins;
	struct image_display_t *dis = (struct image_display_t *)cfg;

	for(head = plugins;(plugin = (*head)); head++)
	{
		if(plugin->is_our_file(filename, NULL))
		{
			try_image_decoder = FALSE;
			break;
		}
	}

	if(try_image_decoder) goto LBL_OTHER_IMAGE_DECODER;

	if(plugin == NULL)
		return -1;

	MEMSET(&image_opentv_plugin, 0, sizeof(struct image_plugin_data));
	image_opentv_plugin.plugin = plugin;

	if((image_opentv_plugin.plugin) && (image_opentv_plugin.plugin->opentv.decode_opentv))
	{
	 	if (3 == strlen(filename)  && !strcasecmp(filename, "YUV"))
	 	{
	 		dis->mp_cb = func_mp_cb;
	 		ret = plugin->opentv.decode_opentv(filename, file_addr, file_len, scale_coef, dis);
	 	}
		else
		{
			cfg->mp_cb = func_mp_cb;
			ret = plugin->opentv.decode_opentv(filename, file_addr, file_len, scale_coef, cfg);
		}
	}
	if(ret == IMAGE_DEC_E_OPEN_FAIL)
        return -1;

    if (ret == IMAGE_DEC_E_OK)
		image_opentv_plugin.state = IMAGE_STATE_DECODE;
	else
	{
		try_image_decoder = TRUE;
		goto LBL_OTHER_IMAGE_DECODER;
	}

	return ret;

LBL_OTHER_IMAGE_DECODER:

	head = plugins;
	while( NULL != *head && try_image_decoder)
	{
		plugin = *head;

		if( plugin == NULL )
			return -1;

		MEMSET(&image_opentv_plugin, 	0, sizeof(struct image_plugin_data));
		image_opentv_plugin.plugin = plugin;

		if((image_opentv_plugin.plugin) && (image_opentv_plugin.plugin->opentv.decode_opentv))
		{

			cfg->mp_cb = func_mp_cb;
			ret = plugin->opentv.decode_opentv(filename, file_addr, file_len, scale_coef, cfg);
		}

		if(ret == 0)
		{
			image_opentv_plugin.state = IMAGE_STATE_DECODE;
			try_image_decoder = FALSE;
			break;
		}
		head++;
	}

	return ret;
}

int image_engine_display_opentv(char *filename,int layer,st_image_buffer *image_buffer, struct rect *src_rect, struct rect *dst_rect)   
{
    
//    int ret = -1;
    image_plugin *plugin = NULL;
    image_plugin **head = plugins;

    
    if((image_opentv_plugin.plugin) && (image_opentv_plugin.plugin->opentv.display_buffer))
    {
        return image_opentv_plugin.plugin->opentv.display_buffer(image_buffer, layer, src_rect, dst_rect);
    }
    else
    { 

        for(head = plugins;(plugin = (*head)); head++)
        {
            if(plugin->is_our_file(filename, NULL))
                break;

        }

        if(NULL == plugin)
        {
            return -1;
        } 

    	MEMSET(&image_opentv_plugin, 0, sizeof(struct image_plugin_data));
    	image_opentv_plugin.plugin = plugin;

        
        return image_opentv_plugin.plugin->opentv.display_buffer(image_buffer, layer, src_rect, dst_rect);
    }

    return 0;
}

int image_engine_get_info_opentv(char *filename, struct image_info *info)
{
    int ret = -1;
    image_plugin *plugin = NULL;
    image_plugin **head = plugins;

    for(head = plugins;(plugin = (*head)); head++)
    {
        if(plugin->is_our_file(filename, NULL))
            break;

    }

    if(NULL == plugin)
    {
        return -1;
    }

    if(plugin->opentv.get_info_opentv)
    {
        ret = plugin->opentv.get_info_opentv(filename, info);
    }

    return ret;
    
}

int image_engine_get_decoded_buffer()
{
    if((image_opentv_plugin.plugin) && (image_opentv_plugin.plugin->opentv.get_decoded_buffer))
    {
        return image_opentv_plugin.plugin->opentv.get_decoded_buffer();
    }

    return 0;
}

int image_engine_abort_opentv()
{
    if((image_opentv_plugin.plugin) && (image_opentv_plugin.plugin->opentv.get_decoded_buffer))
    {
        return image_opentv_plugin.plugin->opentv.abort_opentv();
    }

    return 0;
}


#if 0
#define IMAGE_ENGINE_DEBUG libc_printf
#else
#define IMAGE_ENGINE_DEBUG(...) do{}while(0)
#endif

int image_engine_raw_alloc(int id)
{
    IMAGE_ENGINE_DEBUG("%s %d.\n", __FUNCTION__, __LINE__);
	image_plugin *plugin;
    plugin = plugins[0];

    plugin_image_raw* raw;
    raw = plugin->raw;

    if (raw == NULL)
        return RET_FAILURE;

    return raw->raw_alloc(id);
}
int image_engine_raw_free(int id)
{
    IMAGE_ENGINE_DEBUG("%s %d.\n", __FUNCTION__, __LINE__);
	image_plugin *plugin;
    plugin = plugins[0];

    plugin_image_raw* raw;
    raw = plugin->raw;
    if (raw == NULL)
        return RET_FAILURE;

    return raw->raw_free(id);
}
int image_engine_raw_initconf(int id, void* config)
{
    IMAGE_ENGINE_DEBUG("%s %d.\n", __FUNCTION__, __LINE__);
	image_plugin *plugin;
    plugin = plugins[0];

    plugin_image_raw* raw;
    raw = plugin->raw;
    if (raw == NULL)
        return RET_FAILURE;

    return raw->raw_initconf(id, config);
}
int image_engine_raw_init(int id)
{
    IMAGE_ENGINE_DEBUG("%s %d.\n", __FUNCTION__, __LINE__);
	image_plugin *plugin;
    plugin = plugins[0];

    plugin_image_raw* raw;
    raw = plugin->raw;
    if (raw == NULL)
        return RET_FAILURE;

    return raw->raw_init(id);
}
int image_engine_raw_decconf(int id, void* config)
{
    IMAGE_ENGINE_DEBUG("%s %d.\n", __FUNCTION__, __LINE__);
	image_plugin *plugin;
    plugin = plugins[0];

    plugin_image_raw* raw;
    raw = plugin->raw;
    if (raw == NULL)
        return RET_FAILURE;

    if(MPEG2_DECODER != get_current_decoder())
        video_decoder_select(MPEG2_DECODER, TRUE);
    return raw->raw_decconf(id, config);
}
int image_engine_raw_decode(int id)
{
    IMAGE_ENGINE_DEBUG("%s %d.\n", __FUNCTION__, __LINE__);
	image_plugin *plugin;
    plugin = plugins[0];

    plugin_image_raw* raw;
    raw = plugin->raw;
    if (raw == NULL)
        return RET_FAILURE;

    return raw->raw_decode(id);
}
int image_engine_raw_dispconf(int id, void* config)
{
    IMAGE_ENGINE_DEBUG("%s %d.\n", __FUNCTION__, __LINE__);
	image_plugin *plugin;
    plugin = plugins[0];

    plugin_image_raw* raw;
    raw = plugin->raw;
    if (raw == NULL)
        return RET_FAILURE;

    return raw->raw_dispconf(id, config);
}
int image_engine_raw_display(int id)
{
    IMAGE_ENGINE_DEBUG("%s %d.\n", __FUNCTION__, __LINE__);
	image_plugin *plugin;
    plugin = plugins[0];

    plugin_image_raw* raw;
    raw = plugin->raw;
    if (raw == NULL)
        return RET_FAILURE;

    return raw->raw_display(id);
}
int image_engine_raw_abort(int id)
{
    IMAGE_ENGINE_DEBUG("%s %d.\n", __FUNCTION__, __LINE__);
	image_plugin *plugin;
    plugin = plugins[0];

    plugin_image_raw* raw;
    raw = plugin->raw;
    if (raw == NULL)
        return RET_FAILURE;

    return raw->raw_abort(id);
}


