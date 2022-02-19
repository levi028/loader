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

extern void jhconvs_buffer_init(imagedec_id id);
extern void jhconvs_buffer_release(void);

typedef enum _jpgmessage_type
{
    JPG_CMD_NONE=0,
    JPG_CMD_START,
    JPG_CMD_STOP,
}jpgmessage_type;

#define ENABLE_PE_CACHE
#ifdef     ENABLE_PE_CACHE
#define    JPEG_BLOCK_SIZE        (320 * 1024)
#define JPEG_CACHE_SIZE        (JPEG_BLOCK_SIZE * 2)
static int jpg_cache_id = -1;
static UINT8 *jpg_cache_buffer = NULL;
#endif

#define  JPE_FILE_NAME_LENGTH  5
#define  BMP_FILE_NAME_LENGTH  4
#define  YUV_LENGTH            3
#define  HTTP_LENGTH           7
//BOOL jpeg_decoder_task_not_over;        // added for bug fixing : avoid running of two decoder task at one time.
struct jpeg_file
{
    imagedec_id                id;             //image id for decoder
    imagedec_init_config     init_conf;        //configurefor decoder

    FILE                    *fp;            //file pointer for JPEG
    unsigned long            task_terminated;
    mp_callback_func        mp_cb;            //callback function for mp
    OSAL_ID                    lock;
    jpgmessage_type          command;// command & status, add this to distinguish different state and can get stop cmd.
};

static struct jpeg_file jpeg_file;
static struct image_display_t dpy;

static INT32 jpg_read_data(UINT32 fh,BYTE *buf, UINT32 size)
{
    int ret = 0;

    if (0 == size)
    {
        return 0;
    }
#ifdef ENABLE_PE_CACHE
    if(jpg_cache_id >= 0)
    {
        // In network case, maybe network temp disconnect when reading data,
        // and cause pe cache read return 0. here wait data until user stop.
        do
        {
            ret = pe_cache_read(jpg_cache_id, buf, size, 0xFFFFFFFF);
            if (ret > 0)
            {
                break;
            }
            if (pe_cache_check_eof(jpg_cache_id))
            {
                break;
            }
            osal_task_sleep(100);
        } while((0 == ret) && (jpeg_file.command != JPG_CMD_STOP));

        return ret;
    }
#endif

#ifndef ENABLE_PE_CACHE
    FILE *fp = (FILE*)fh;
    int byte_read;

    if(NULL == fp)
    {
        return -1;
    }

    byte_read = fread(buf, size, 1, fp);

    return ((byte_read < 0) ? 0 : byte_read) ;
#endif
    return 0;
}

static int jpg_seek_data(UINT32 fh,long offset, UINT32 origin)
{
#ifdef ENABLE_PE_CACHE
    if(jpg_cache_id >= 0)
    {
        pe_cache_seek(jpg_cache_id, (off_t)offset, origin);
        return 0;
    }
#endif

#ifndef ENABLE_PE_CACHE
    FILE *fp = (FILE*)fh;

    if(NULL == fp)
    {
        return -1;
    }

    fseek(fp, (off_t)offset, origin);
#endif
    return 0;
}

static UINT32 jpeg_process(void *value)
{
    unsigned long process = *(unsigned long*)value;
    unsigned long err = process >> 24;

    process = (process * 100) >> 16;

    if(process > 100)
    {
        process = 100;
    }

    osal_mutex_lock(jpeg_file.lock, TMO_FEVR);
    if(jpeg_file.mp_cb)
    {
        if(0 == err)
        {
            jpeg_file.mp_cb(MP_IMAGE_DECODE_PROCESS, process);
        }
        else
        {
            jpeg_file.mp_cb(MP_IMAGE_DECODER_ERROR, err);
        }
    }
    osal_mutex_unlock(jpeg_file.lock);

    return 0;
}


static int jpeg_init(struct pe_image_cfg *pe_image_cfg)
{
    struct vpo_device *pvpo_sd = NULL;

    MEMSET(&jpeg_file, 0, sizeof(jpeg_file));

    jpeg_file.init_conf.frm_y_size = pe_image_cfg->frm_y_size;
    jpeg_file.init_conf.frm_y_addr = pe_image_cfg->frm_y_addr;
    jpeg_file.init_conf.frm_c_size = pe_image_cfg->frm_c_size;
    jpeg_file.init_conf.frm_c_addr = pe_image_cfg->frm_c_addr;
    jpeg_file.init_conf.frm2_y_size = pe_image_cfg->frm2_y_size;
    jpeg_file.init_conf.frm2_y_addr = pe_image_cfg->frm2_y_addr;
    jpeg_file.init_conf.frm2_c_size = pe_image_cfg->frm2_c_size;
    jpeg_file.init_conf.frm2_c_addr = pe_image_cfg->frm2_c_addr;
    jpeg_file.init_conf.decoder_buf = pe_image_cfg->decoder_buf;
    jpeg_file.init_conf.frm3_y_size = pe_image_cfg->frm3_y_size;
    jpeg_file.init_conf.frm3_y_addr = pe_image_cfg->frm3_y_addr;
    jpeg_file.init_conf.frm3_c_size = pe_image_cfg->frm3_c_size;
    jpeg_file.init_conf.frm3_c_addr = pe_image_cfg->frm3_c_addr;
    pvpo_sd = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 1);
    if(NULL != pvpo_sd)
    {
        jpeg_file.init_conf.frm4_y_size = pe_image_cfg->frm4_y_size;
        jpeg_file.init_conf.frm4_y_addr = pe_image_cfg->frm4_y_addr;
        jpeg_file.init_conf.frm4_c_size = pe_image_cfg->frm4_c_size;
        jpeg_file.init_conf.frm4_c_addr = pe_image_cfg->frm4_c_addr;
    }
    jpeg_file.init_conf.decoder_buf_len = pe_image_cfg->decoder_buf_len;
    jpeg_file.init_conf.fread_callback = jpg_read_data;
    jpeg_file.init_conf.fseek_callback = jpg_seek_data;
    jpeg_file.init_conf.imagedec_status = jpeg_process;
    jpeg_file.init_conf.ali2dto3d_callback = pe_image_cfg->ali_pic_2dto3d;

    if(sys_ic_get_chip_id()==ALI_S3602 || sys_ic_get_chip_id()==ALI_S3602F ||\
        sys_ic_get_chip_id()==ALI_S3811 || sys_ic_get_chip_id()>=ALI_C3701)
    {
        jpeg_file.init_conf.frm_mb_type = 1;
    }
    else
    {
        jpeg_file.init_conf.frm_mb_type = 0;
    }

#ifdef ENABLE_PE_CACHE
    jpeg_file.init_conf.decoder_buf_len -= (JPEG_CACHE_SIZE + 0x200);
    if(0 != pe_image_cfg->frm4_y_addr) //AS Project
    {
        jpg_cache_buffer = (unsigned char *)((((unsigned long)pe_image_cfg->frm4_y_addr)+0xff) & 0xffffff00);
    }
    else
    {
        jpg_cache_buffer = (unsigned char *)((((unsigned long)jpeg_file.init_conf.decoder_buf)+0xff) & 0xffffff00);
    }
    jpg_cache_buffer = (unsigned char *)(((unsigned long)jpg_cache_buffer & 0x0fffffff) | 0x80000000);
    jpeg_file.init_conf.decoder_buf = (unsigned char *)(((unsigned long)jpeg_file.init_conf.decoder_buf + \
                                                           JPEG_CACHE_SIZE + 0x100 + 0x100)&0xffffff00);
#endif

    jpeg_file.id = imagedec_init(&jpeg_file.init_conf);
    jpeg_file.lock = osal_mutex_create();

    return 0;

}

static int jpeg_cleanup(void)
{
    if(jpeg_file.id > 0)
    {
        imagedec_release(jpeg_file.id);
        jpeg_file.id = 0;
        osal_mutex_delete(jpeg_file.lock);
        return 0;
    }
    return -1;
}

static int jpeg_is_our_file(char *filename, void *args)
{
    char *ext = NULL;
    struct image_config *cfg = (struct image_config*)args;

#ifndef     _USE_32M_MEM_
    if (strrchr(filename, '.') && cfg)
    {
        extern int g_image_slide_flag;
        if(!strncasecmp(strrchr(filename, '.'), ".png", 4) && (cfg->decode_mode == IMAGEDEC_FULL_SRN))
        {
            g_image_slide_flag = 1;
        }
        if(g_image_slide_flag)
        {
            vpo_win_onoff_ext((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), FALSE, VPO_LAYER_MAIN);
        }
    }

    if(YUV_LENGTH == strlen(filename) && !strncasecmp(filename, "YUV", YUV_LENGTH))
    {
        return IMAGE_IS_OUR_FILE;
    }

    ext = strrchr(filename, '.');
    if (ext)
    {
        if((!strncasecmp(ext, ".jpg", BMP_FILE_NAME_LENGTH)) || \
          (!strncasecmp(ext, ".jpeg", JPE_FILE_NAME_LENGTH)) || \
          (!strncasecmp(ext, ".bmp", BMP_FILE_NAME_LENGTH)))
        {
            return IMAGE_IS_OUR_FILE;
        }
    }
#endif
    return IMAGE_NOT_OUR_FILE;
}

//enum Output_Frame_Ret_Code imagedec_mp_request(void *dev, void *pinfo);
//BOOL imagedec_mp_release(void *dev,UINT8 utop_idx,UINT8 frm_array_idx);



static int jpeg_rotate(unsigned char rotate)
{
    int ret = -1;

    //imagedec_de_config_imagetoplay();

    if(imagedec_rotate(jpeg_file.id, rotate, 0))
    {
        ret = 1;
    }

    //imagedec_de_config_videotoplay();

    return ret;

}

static void jpeg_decoder_task(UINT32 para1,UINT32 para2)
{
    struct jpeg_file *pjpeg_file = (struct jpeg_file*)para1;
    struct image_display_t *dispy = (struct image_display_t*)para2;
    int ret = 0;
    image_display_par dis_par;

    pjpeg_file->task_terminated = 1;
//    jpeg_decoder_task_not_over = TRUE;      // initialized to TRUE, decoder task is not over
    //add for display
    if((sys_ic_get_chip_id()==ALI_S3602F) && ((FILE *)-100 ==pjpeg_file->fp))
    {
        MEMSET(&dis_par, 0, sizeof(dis_par));

        dis_par.y = dispy->y_addr;

        dis_par.y_len = dispy->y_len;
        dis_par.c = dispy->c_addr;
        dis_par.c_len = dispy->c_len;
        dis_par.width = dispy->width;
        dis_par.height = dispy->height;
        dis_par.scantype = dispy->sample_format;

        imagedec_display(pjpeg_file->id, &dis_par);
    }
    else
    {
#ifdef ENABLE_PE_CACHE
    pjpeg_file->fp = (FILE *)1;            // avoid imagedec_decode return FALSE without FS API
#endif
    jpeg_file.command = JPG_CMD_START;
    jhconvs_buffer_init(pjpeg_file->id);
	ret = imagedec_decode(pjpeg_file->id, (UINT32)pjpeg_file->fp);
    jhconvs_buffer_release();

#ifdef ENABLE_PE_CACHE
    pe_cache_close(jpg_cache_id);
    jpg_cache_id = -1;
#endif

#ifndef ENABLE_PE_CACHE
    fclose(pjpeg_file->fp);
#endif
    }
    pjpeg_file->fp = NULL;

    //jpeg_decoder_task_vposet();
    //imagedec_de_config_videotoplay();
    
    pjpeg_file->task_terminated = 0;

    osal_mutex_lock(jpeg_file.lock, TMO_FEVR);
    if(pjpeg_file->mp_cb)
    {
		if(ret != 1)
			pjpeg_file->mp_cb(MP_IMAGE_DECODER_ERROR, 0);
		else
        	pjpeg_file->mp_cb(MP_IMAGE_PLAYBACK_END, 0);
    }
    osal_mutex_unlock(jpeg_file.lock);

}

static int jpeg_decode_yuv(void *args)
{
    OSAL_T_CTSK t_ctsk;
    OSAL_ID    tsk_id = 0;
    struct image_display_t *display = (struct image_display_t *)args;;
    imagedec_mode_par mode_par;
    imagedec_mode_par *par = NULL;
    struct vpo_io_get_info dis_info;

    if(args == NULL)
        return IMAGE_DEC_E_FAIL;
    
    jpeg_file.fp = (FILE *)-100;
    MEMSET(&mode_par, 0, sizeof(mode_par));
    par = &mode_par;
    dpy.y_addr = display->y_addr;
    dpy.y_len = display->y_len;
    dpy.c_addr = display->c_addr;
    dpy.c_len = display->c_len;
    dpy.width = display->width;
    dpy.height = display->height;
    dpy.sample_format = display->sample_format;
    if(sys_ic_get_chip_id()==ALI_S3602 || sys_ic_get_chip_id()==ALI_S3602F)
    {
        vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), \
                                 VPO_IO_GET_INFO, (UINT32) &dis_info);
    }
    jpeg_file.mp_cb = display->mp_cb;
    jpeg_file.command = JPG_CMD_NONE;
    if(NULL == display)
    {
        par->pro_show = 1;
        par->dis_rect.u_start_x = display->dest_left;
        par->dis_rect.u_start_y = display->dest_top;
        par->dis_rect.u_width = display->dest_width;
        par->dis_rect.u_height = display->dest_height;
        par->src_rect.u_start_x = display->dest_left;
        par->src_rect.u_start_y = display->dest_top;
        par->src_rect.u_width = display->dest_width;
        par->src_rect.u_height = display->dest_height;
    }
    else if( IMAGEDEC_REAL_SIZE == display->decode_mode)
    {
        par->pro_show = 1;
        par->dis_rect.u_start_x = 0;
        par->dis_rect.u_start_y = 0;
        if(sys_ic_get_chip_id()==ALI_S3602 || sys_ic_get_chip_id()==ALI_S3602F)
        {
            par->src_rect.u_start_x = 0;
            par->src_rect.u_start_y = 0;
            switch(dis_info.tvsys)
            {
                case LINE_720_25:
                case LINE_720_30:
                    par->dis_rect.u_width = 1280;
                    par->dis_rect.u_height = 720;
                    par->src_rect.u_width = 1280;
                    par->src_rect.u_height = 720;
                    break;
                case LINE_1080_25:
                case LINE_1080_30:
                    case LINE_1080_50:
                    case LINE_1080_60:
                    case LINE_1080_24:
                    case LINE_1080_ASS:
                    case LINE_1152_ASS:
                    par->dis_rect.u_width = 1920;
                    par->dis_rect.u_height = 1080;
                    par->src_rect.u_width = 1920;
                    par->src_rect.u_height = 1080;
                    break;
                default:
                    par->dis_rect.u_width = 720;
                    par->dis_rect.u_height = 576;
                    par->src_rect.u_width = 720;
                    par->src_rect.u_height = 576;
                break;
            }
        }
        else
        {
            par->dis_rect.u_width = 720;
            par->dis_rect.u_height = 576;
            par->src_rect.u_start_x = display->src_left;
            par->src_rect.u_start_y = display->src_top;
            par->src_rect.u_width = display->src_width;
            par->src_rect.u_height = display->src_height;
        }
    }
    else
    {
        par->pro_show = 1;
        if(sys_ic_get_chip_id()==ALI_S3602 || sys_ic_get_chip_id()==ALI_S3602F)
        {
            par->dis_rect.u_start_x = 0;
            par->dis_rect.u_start_y = 0;
            par->src_rect.u_start_x = 0;
            par->src_rect.u_start_y = 0;
            switch(dis_info.tvsys)
            {
                case LINE_720_25:
                case LINE_720_30:
                    par->dis_rect.u_width = 1280;
                    par->dis_rect.u_height = 720;
                    par->src_rect.u_width = 1280;
                    par->src_rect.u_height = 720;
                    break;
                case LINE_1080_25:
                case LINE_1080_30:
                case LINE_1080_50:
                case LINE_1080_60:
                case LINE_1080_24:
                case LINE_1080_ASS:
                case LINE_1152_ASS:
                    par->dis_rect.u_width = 1920;
                    par->dis_rect.u_height = 1080;
                    par->dis_rect.u_width = 1920;
                    par->dis_rect.u_height = 1080;
                    break;
                default:
                    par->dis_rect.u_width = 720;
                    par->dis_rect.u_height = 576;
                    par->src_rect.u_width = 720;
                    par->src_rect.u_height = 576;
                    break;
            }
        }
        else
        {
            par->dis_rect.u_start_x = display->dest_left;
            par->dis_rect.u_start_y = display->dest_top;
            par->dis_rect.u_width = display->dest_width;
            par->dis_rect.u_height = display->dest_height;
            par->src_rect.u_start_x = display->dest_left;
            par->src_rect.u_start_y = display->dest_top;
            par->src_rect.u_width = display->dest_width;
            par->src_rect.u_height = display->dest_height;
        }
    }
    if(NULL == display->effect)
    {
        par->show_mode = M_NORMAL;
    }
    else
    {
        par->show_mode = display->effect->mode;
        par->show_mode_par = (UINT8 *)&display->effect->mode_param;
    }
    if(sys_ic_get_chip_id()==ALI_S3602 || sys_ic_get_chip_id()==ALI_S3602F)
    {
        imagedec_set_mode(jpeg_file.id, display->decode_mode, par);
    }
    else
    {
        imagedec_rotate(jpeg_file.id, display->rotate, 1);
        imagedec_set_mode(jpeg_file.id, display->decode_mode, par);
    }

    //jpeg_decode_yuv_vposet();
    //imagedec_de_config_imagetoplay();

    t_ctsk.itskpri = OSAL_PRI_NORMAL;
    t_ctsk.stksz = 0x6000;
    t_ctsk.quantum = 10;
    t_ctsk.para1 = (UINT32)&jpeg_file;
    t_ctsk.para2 = (UINT32)&dpy;
    t_ctsk.name[0] = 'J';
    t_ctsk.name[1] = 'P';
    t_ctsk.name[2] = 'G';
    t_ctsk.task = jpeg_decoder_task;
    tsk_id = osal_task_create(&t_ctsk);
    if(OSAL_INVALID_ID == tsk_id)
    {
        return IMAGE_DEC_E_OPEN_FAIL;
    }
    return IMAGE_DEC_E_OK;
}

static int jpeg_decode_normal(char *filename, void *args)
{
    OSAL_T_CTSK t_ctsk;
    OSAL_ID    tsk_id = 0;
    struct image_config *cfg = (struct image_config*)args;
    imagedec_mode_par mode_par;
    imagedec_mode_par *par = NULL;
    struct vpo_io_get_info dis_info;
    int ret = IMAGE_DEC_E_OK;

#ifndef ENABLE_PE_CACHE
    FILE *fp = fopen(filename, "rb");
    if(NULL == fp)
    {
        return IMAGE_DEC_E_OPEN_FAIL;
    }
    jpeg_file.fp = fp;
    jpeg_file.command = JPG_CMD_NONE;
#endif

    MEMSET(&mode_par, 0, sizeof(mode_par));
    par = &mode_par;
    if(!MEMCMP(filename, "http://", HTTP_LENGTH))
    {
        par->file_type = TRUE;
    }
    else
    {
        par->file_type = FALSE;
    }
    if(sys_ic_get_chip_id()==ALI_S3602 || sys_ic_get_chip_id()==ALI_S3602F ||\
         sys_ic_get_chip_id()==ALI_S3811 || sys_ic_get_chip_id()>=ALI_C3701)
    {
        vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_GET_INFO, (UINT32) &dis_info);
    }

    jpeg_file.mp_cb = cfg->mp_cb;
    jpeg_file.command = JPG_CMD_NONE;
    if((cfg->decode_mode == IMAGEDEC_THUMBNAIL) || cfg->decode_mode == IMAGEDEC_MULTI_PIC)
    {
        par->pro_show = 1;
        par->dis_rect.u_start_x = cfg->dest_left;
        par->dis_rect.u_start_y = cfg->dest_top;
        par->dis_rect.u_width = cfg->dest_width + 4;
        par->dis_rect.u_height = cfg->dest_height + 4;
        par->src_rect.u_start_x = 0;
        par->src_rect.u_start_y = 0;
        par->src_rect.u_width = 720;
        par->src_rect.u_height = 576;
    }
    else if(cfg->decode_mode == IMAGEDEC_SIZEDEFINE)
    {
        par->pro_show = 1;
        par->dis_rect.u_start_x = cfg->dest_left;
        par->dis_rect.u_start_y = cfg->dest_top;
        par->dis_rect.u_width = cfg->dest_width;
        par->dis_rect.u_height = cfg->dest_height;
        par->src_rect.u_start_x = 0;
        par->src_rect.u_start_y = 0;
        par->src_rect.u_width = 720;
        par->src_rect.u_height = 576;
    }
    else if(cfg->decode_mode == IMAGEDEC_REAL_SIZE)
    {
        par->pro_show = 1;
        par->dis_rect.u_start_x = 0;
        par->dis_rect.u_start_y = 0;

        if(sys_ic_get_chip_id()==ALI_S3602 || sys_ic_get_chip_id()==ALI_S3602F || \
             sys_ic_get_chip_id()==ALI_S3811 || sys_ic_get_chip_id()>=ALI_C3701)
        {
            par->src_rect.u_start_x = 0;
            par->src_rect.u_start_y = 0;
            switch(dis_info.tvsys)
            {
                case LINE_720_25:
                case LINE_720_30:
                    par->dis_rect.u_width = 1280;
                    par->dis_rect.u_height = 720;
                    par->src_rect.u_width = 1280;
                    par->src_rect.u_height = 720;
                    break;
                case LINE_1080_25:
                case LINE_1080_30:
                case LINE_1080_50:
                case LINE_1080_60:
                case LINE_1080_24:
                case LINE_1080_ASS:
                case LINE_1152_ASS:
                    par->dis_rect.u_width = 1920;
                    par->dis_rect.u_height = 1080;
                    par->src_rect.u_width = 1920;
                    par->src_rect.u_height = 1080;
                    break;
                default:
                    par->dis_rect.u_width = 720;
                    par->dis_rect.u_height = 576;
                    par->src_rect.u_width = 720;
                    par->src_rect.u_height = 576;
                    break;
            }
        }
        else
        {
            par->dis_rect.u_width = 720;
            par->dis_rect.u_height = 576;
            par->src_rect.u_start_x = cfg->src_left;
            par->src_rect.u_start_y = cfg->src_top;
            par->src_rect.u_width = cfg->src_width;
            par->src_rect.u_height = cfg->src_height;
        }
    }
    else
    {//default as full screen
        par->pro_show = 1;
        if(sys_ic_get_chip_id()==ALI_S3602 || sys_ic_get_chip_id()==ALI_S3602F || \
               sys_ic_get_chip_id()==ALI_S3811 || sys_ic_get_chip_id()>=ALI_C3701)
        {
            par->dis_rect.u_start_x = 0;
            par->dis_rect.u_start_y = 0;
            par->src_rect.u_start_x = 0;
            par->src_rect.u_start_y = 0;
            switch(dis_info.tvsys)
            {
                case LINE_720_25:
                case LINE_720_30:
                    par->dis_rect.u_width = 1280;
                    par->dis_rect.u_height = 720;
                    par->src_rect.u_width = 1280;
                    par->src_rect.u_height = 720;
                    break;
                case LINE_1080_25:
                case LINE_1080_30:
                case LINE_1080_50:
                case LINE_1080_60:
                case LINE_1080_24:
                case LINE_1080_ASS:
                case LINE_1152_ASS:
                    par->dis_rect.u_width = 1920;
                    par->dis_rect.u_height = 1080;
                    par->dis_rect.u_width = 1920;
                    par->dis_rect.u_height = 1080;
                    break;
                default:
                    par->dis_rect.u_width = 720;
                    par->dis_rect.u_height = 576;
                    par->src_rect.u_width = 720;
                    par->src_rect.u_height = 576;
                    break;
            }
        }
        else
        {
            par->dis_rect.u_start_x = cfg->dest_left;
            par->dis_rect.u_start_y = cfg->dest_top;
            par->dis_rect.u_width = cfg->dest_width;
            par->dis_rect.u_height = cfg->dest_height;
            par->src_rect.u_start_x = cfg->dest_left;
            par->src_rect.u_start_y = cfg->dest_top;
            par->src_rect.u_width = cfg->dest_width;
            par->src_rect.u_height = cfg->dest_height;
        }
    }
    if(NULL == cfg->effect)
    {
        par->show_mode = M_NORMAL;
    }
    else
    {
        par->show_mode = cfg->effect->mode;
        par->show_mode_par = (UINT8 *)&cfg->effect->mode_param;
    }
    if(sys_ic_get_chip_id()==ALI_S3602 || sys_ic_get_chip_id()==ALI_S3602F || \
        sys_ic_get_chip_id()==ALI_S3811 || sys_ic_get_chip_id()>=ALI_C3701)
    {
        imagedec_set_mode(jpeg_file.id, cfg->decode_mode, par);
    }
    else
    {
        imagedec_rotate(jpeg_file.id, cfg->rotate, 1);
        imagedec_set_mode(jpeg_file.id, cfg->decode_mode, par);
    }
    //ret = jpeg_decode_normal_vposet();
    //imagedec_de_config_imagetoplay();

#ifdef ENABLE_PE_CACHE
    jpg_cache_id = pe_cache_open(filename, jpg_cache_buffer, JPEG_CACHE_SIZE, JPEG_BLOCK_SIZE);
    if (jpg_cache_id < 0)
    {
        pe_cache_close(jpg_cache_id);
        jpg_cache_id = -1;
        return IMAGE_DEC_E_OPEN_FAIL;
    }
#endif
    
    osal_task_sleep(20);
    
#ifdef ENABLE_PE_CACHE
    jpeg_file.fp = (FILE *)1;            // avoid imagedec_decode return FALSE without FS API
#endif
    if (!imagedec_check_fileheader(jpeg_file.id, (file_h)jpeg_file.fp))
    {
#ifdef ENABLE_PE_CACHE
        pe_cache_close(jpg_cache_id);
        jpg_cache_id = -1;
#endif

#ifndef ENABLE_PE_CACHE
        fclose(jpeg_file.fp);
#endif
        jpeg_file.fp = NULL;
        return IMAGE_DEC_E_FAIL;
    }

    
    //create the decode task
    t_ctsk.itskpri = OSAL_PRI_NORMAL;
    t_ctsk.stksz = 0x6000;
    t_ctsk.quantum = 10;
    t_ctsk.para1 = (UINT32)&jpeg_file;
    t_ctsk.para2 = (UINT32)NULL;
    t_ctsk.name[0] = 'J';
    t_ctsk.name[1] = 'P';
    t_ctsk.name[2] = 'G';
    t_ctsk.task = jpeg_decoder_task;
    tsk_id = osal_task_create(&t_ctsk);
    if(OSAL_INVALID_ID == tsk_id)
    {
        ret = IMAGE_DEC_E_OPEN_FAIL;
    }
    return ret;
}

static int jpeg_decode(char *filename, void *args)
{
    int ret = 0;
    if(YUV_LENGTH == strlen(filename) && !strcasecmp(filename, "YUV"))
    {
        ret = jpeg_decode_yuv(args);
    }
    else
    {
        ret = jpeg_decode_normal(filename, args);
    }
    return ret;
}

static int jpeg_show(void)
{
    return imagedec_dis_next_pic(jpeg_file.id, 0, OSAL_WAIT_FOREVER_TIME);
}

static int jpeg_abort(void)
{
    if(jpeg_file.id > 0)
    {
        osal_mutex_lock(jpeg_file.lock, TMO_FEVR);
        jpeg_file.mp_cb = NULL;
        osal_mutex_unlock(jpeg_file.lock);

        imagedec_stop(jpeg_file.id);
        jpeg_file.command = JPG_CMD_STOP;

        while(1 == jpeg_file.task_terminated)
        {
            osal_task_sleep(10);
        }
        return 0;
    }
    return -1;
}

static int jpeg_get_info(char *filename, struct image_info *info)
{
    image_info ifo;
    BOOL ret = 0;
    FILE *fp = NULL;

#ifndef ENABLE_PE_CACHE
    fp = fopen(filename, "rb");
    if(NULL == fp)
    {
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    info->fsize = ftell(fp);
#else
    jpg_cache_id = pe_cache_open(filename, jpg_cache_buffer, JPEG_CACHE_SIZE, JPEG_BLOCK_SIZE);
    if(jpg_cache_id < 0)
    {
        pe_cache_close(jpg_cache_id);
        jpg_cache_id = -1;
        return -1;
    }

    pe_cache_seek(jpg_cache_id, 0, SEEK_END);
    info->fsize = pe_cache_tell(jpg_cache_id);
#endif

    ret = imagedec_getinfo(jpeg_file.id, (UINT32)fp, &ifo);

#ifndef ENABLE_PE_CACHE
    fclose(fp);
#else
    pe_cache_close(jpg_cache_id);
    jpg_cache_id = -1;
#endif

    if(ret)
    {
        info->height = ifo.image_height;
        info->width = ifo.image_width;
        info->bbp = ifo.precision;
        return 0;
    }
    return -1;

}

static int jpeg_zoom(struct rect *dst_rect, struct rect *src_rest)
{
    return imagedec_zoom(jpeg_file.id, *dst_rect, *src_rest);
}


static int jpeg_swap(int type)
{
    return imagedec_swap(jpeg_file.id, type);
}

static int jpeg_option(int output_format, int user_depth, int user_shift, int user_vip)
{
    return imagedec_3d_user_option(jpeg_file.id, output_format, user_depth, user_shift, user_vip);
}

static void jpeg_set_disp_param(int display_type, int output_format, int user_depth, int user_shift, int user_vip)
{
    imagedec_set_disp_param(jpeg_file.id, display_type, output_format, user_depth, user_shift, user_vip);
}

/***************************************************************************************************
 * brief:     opentv system decoder operation
 * detail:    methods:
 *            1. jpeg_opentv_init
 *            2. jpeg_opentv_decode
 *            3. jpeg_opentv_display
 *            4. jpeg_opentv_get_info
 *            5. jpeg_opentv_get_decoded_buffer
 * author:    lily.zhang         add
 *            saicheong.yuen     mod   2014-12-16
 * date:      2014-10-18
 ***************************************************************************************************/

static UINT8 *img_file_addr; //start address of the buffer to restore the image file
static UINT32 img_file_size;  // image file size
static INT32 img_input_pos = 0;

static INT32 jpeg_opentv_img_read_data(file_h fh,UINT8 *buf, UINT32 size)
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

static int jpeg_opentv_img_seek_data(UINT32 fh,long offset, UINT32 origin)
{
	if(origin == 0)
	{
		img_input_pos = (UINT32)offset;
		return TRUE;
	}
	return FALSE;
}

static struct jpeg_file jpeg_opentv_file;

void jpeg_opentv_decoder_task(UINT32 para1,UINT32 para2)
{
	struct jpeg_file *pjpeg_file = (struct jpeg_file*)para1;
//	struct image_display_t *dispy = (struct image_display_t*)para2;
//	int ret;
 
	pjpeg_file->task_terminated = 1;

    jpeg_opentv_file.command = JPG_CMD_START;
    pjpeg_file->fp = (FILE*)0xFF;
	imagedec_decode(pjpeg_file->id, (UINT32)pjpeg_file->fp);

	pjpeg_file->fp = NULL;
    pjpeg_file->task_terminated = 0;

	osal_mutex_lock(jpeg_opentv_file.lock, TMO_FEVR);
	if(pjpeg_file->mp_cb)
	{
		pjpeg_file->mp_cb(MP_IMAGE_PLAYBACK_END, 0);
	}
	osal_mutex_unlock(jpeg_opentv_file.lock);

}

static int jpeg_opentv_decode(char *filename, UINT32 file_addr, UINT32 file_len, int scale_coef, void *args)
{
	OSAL_T_CTSK t_ctsk;
	OSAL_ID	tsk_id;
	struct image_config *cfg;
//	struct image_display_t *display;
	imagedec_mode_par Par, *pPar;
	struct vpo_io_get_info dis_info;
    img_input_pos = 0;
	img_file_addr = (UINT8 *)file_addr;
	img_file_size = file_len;

    if(args == NULL)
        return IMAGE_DEC_E_FAIL;
    
	//set the decode parameter
	cfg = (struct image_config*)args;

	MEMSET(&Par, 0, sizeof(Par));
	pPar = &Par;

	if(!MEMCMP(filename, "http://", 7))
    {
		pPar->file_type = TRUE;
	} else
	{
		pPar->file_type = FALSE;
	}

	if(sys_ic_get_chip_id()==ALI_S3602 || sys_ic_get_chip_id()==ALI_S3602F ||sys_ic_get_chip_id()==ALI_S3811 || sys_ic_get_chip_id()>=ALI_C3701)
	{
		vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_GET_INFO, (UINT32) &dis_info);
	}

	jpeg_opentv_file.mp_cb = cfg->mp_cb;
    jpeg_opentv_file.command = JPG_CMD_NONE;

	if(cfg == NULL)
	{
	#if 0
		//as full screen
		pPar->pro_show = 1;
		pPar->dis_rect.u_start_x = cfg->dest_left;
		pPar->dis_rect.u_start_y = cfg->dest_top;
		pPar->dis_rect.u_width = cfg->dest_width;
		pPar->dis_rect.u_height = cfg->dest_height;

		pPar->src_rect.u_start_x = cfg->dest_left;
		pPar->src_rect.u_start_y = cfg->dest_top;
		pPar->src_rect.u_width = cfg->dest_width;
		pPar->src_rect.u_height = cfg->dest_height;
    #endif
    	pPar->pro_show = 1;
		pPar->dis_rect.u_start_x = 0;
		pPar->dis_rect.u_start_y = 0;
		pPar->dis_rect.u_width = 720;
		pPar->dis_rect.u_height = 576;

		pPar->src_rect.u_start_x = 0;
		pPar->src_rect.u_start_y = 0;
		pPar->src_rect.u_width = 720;
		pPar->src_rect.u_height = 576;
	}
	else if((cfg->decode_mode == IMAGEDEC_THUMBNAIL) || cfg->decode_mode == IMAGEDEC_MULTI_PIC)
	{
		pPar->pro_show = 1;
		pPar->dis_rect.u_start_x = cfg->dest_left;
		pPar->dis_rect.u_start_y = cfg->dest_top;
		pPar->dis_rect.u_width = cfg->dest_width + 4;
		pPar->dis_rect.u_height = cfg->dest_height + 4;

		pPar->src_rect.u_start_x = 0;
		pPar->src_rect.u_start_y = 0;
		pPar->src_rect.u_width = 720;
		pPar->src_rect.u_height = 576;

	}
	else if(cfg->decode_mode == IMAGEDEC_SIZEDEFINE)
	{
		pPar->pro_show = 1;
		pPar->dis_rect.u_start_x = cfg->dest_left;
		pPar->dis_rect.u_start_y = cfg->dest_top;
		pPar->dis_rect.u_width = cfg->dest_width;
		pPar->dis_rect.u_height = cfg->dest_height;

		pPar->src_rect.u_start_x = 0;
		pPar->src_rect.u_start_y = 0;
		pPar->src_rect.u_width = 720;
		pPar->src_rect.u_height = 576;

	}
	else if(cfg->decode_mode == IMAGEDEC_REAL_SIZE)
	{
		pPar->pro_show = 1;
		pPar->dis_rect.u_start_x = 0;
		pPar->dis_rect.u_start_y = 0;

		if(sys_ic_get_chip_id()==ALI_S3602 || sys_ic_get_chip_id()==ALI_S3602F || sys_ic_get_chip_id()==ALI_S3811 || sys_ic_get_chip_id()>=ALI_C3701)
		{
			pPar->src_rect.u_start_x = 0;
			pPar->src_rect.u_start_y = 0;
			switch(dis_info.tvsys)
			{
				case LINE_720_25:
				case LINE_720_30:
					pPar->dis_rect.u_width = 1280;
					pPar->dis_rect.u_height = 720;

					pPar->src_rect.u_width = 1280;
					pPar->src_rect.u_height = 720;
					break;
				case LINE_1080_25:
				case LINE_1080_30:
                case LINE_1080_50:
                case LINE_1080_60:
                case LINE_1080_24:
                case LINE_1080_ASS:
                case LINE_1152_ASS:
					pPar->dis_rect.u_width = 1920;
					pPar->dis_rect.u_height = 1080;

					pPar->src_rect.u_width = 1920;
					pPar->src_rect.u_height = 1080;
					break;
				default:
					pPar->dis_rect.u_width = 720;
					pPar->dis_rect.u_height = 576;

					pPar->src_rect.u_width = 720;
					pPar->src_rect.u_height = 576;
					break;
			}
		}
		else
		{
			pPar->dis_rect.u_width  = 720;
			pPar->dis_rect.u_height = 576;

			pPar->src_rect.u_start_x = cfg->src_left;
			pPar->src_rect.u_start_y = cfg->src_top;
			pPar->src_rect.u_width  = cfg->src_width;
			pPar->src_rect.u_height = cfg->src_height;
		}
	}
	else
	{
		//default as full screen
		pPar->pro_show = 1;
		if(sys_ic_get_chip_id()==ALI_S3602 || sys_ic_get_chip_id()==ALI_S3602F || sys_ic_get_chip_id()==ALI_S3811 || sys_ic_get_chip_id()>=ALI_C3701)
		{

			pPar->dis_rect.u_start_x = 0;
			pPar->dis_rect.u_start_y = 0;

			pPar->src_rect.u_start_x = 0;
			pPar->src_rect.u_start_y = 0;
			switch(dis_info.tvsys)
			{
				case LINE_720_25:
				case LINE_720_30:
					pPar->dis_rect.u_width = 1280;
					pPar->dis_rect.u_height = 720;

					pPar->src_rect.u_width = 1280;
					pPar->src_rect.u_height = 720;
					break;
				case LINE_1080_25:
				case LINE_1080_30:
                case LINE_1080_50:
                case LINE_1080_60:
                case LINE_1080_24:
                case LINE_1080_ASS:
                case LINE_1152_ASS:
					pPar->dis_rect.u_width = 1920;
					pPar->dis_rect.u_height = 1080;

					pPar->dis_rect.u_width = 1920;
					pPar->dis_rect.u_height = 1080;
					break;
				default:
					pPar->dis_rect.u_width = 720;
					pPar->dis_rect.u_height = 576;

					pPar->src_rect.u_width = 720;
					pPar->src_rect.u_height = 576;
					break;
			}
		}
		else
		{
			pPar->dis_rect.u_start_x = cfg->dest_left;
			pPar->dis_rect.u_start_y = cfg->dest_top;
			pPar->dis_rect.u_width = cfg->dest_width;
			pPar->dis_rect.u_height = cfg->dest_height;

			pPar->src_rect.u_start_x = cfg->dest_left;
			pPar->src_rect.u_start_y = cfg->dest_top;
			pPar->src_rect.u_width = cfg->dest_width;
			pPar->src_rect.u_height = cfg->dest_height;
		}
	}
    cfg->effect = NULL;
	if(cfg->effect == NULL)
	{
		pPar->show_mode = M_NORMAL;
	}
	else
	{
		pPar->show_mode = cfg->effect->mode;
		pPar->show_mode_par = (UINT8 *)&cfg->effect->mode_param;
	}
	if(sys_ic_get_chip_id()==ALI_S3602 || sys_ic_get_chip_id()==ALI_S3602F || sys_ic_get_chip_id()==ALI_S3811 || sys_ic_get_chip_id()>=ALI_C3701)
	{
		imagedec_set_mode(jpeg_opentv_file.id, cfg->decode_mode, pPar);
	}
	else
	{
		imagedec_rotate(jpeg_opentv_file.id, cfg->rotate, 1);
		imagedec_set_mode(jpeg_opentv_file.id, cfg->decode_mode, pPar);
	}

	osal_task_sleep(20);
    jpeg_opentv_file.fp = (FILE*)0xff;
	if( !imagedec_check_fileheader(jpeg_opentv_file.id, (file_h)jpeg_opentv_file.fp) )
	{
    	jpeg_opentv_file.fp = NULL;
		return IMAGE_DEC_E_FAIL;
	}
	//create the decode task
	//t_ctsk.itskpri = OSAL_PRI_NORMAL;
    t_ctsk.itskpri = OSAL_PRI_HIGH;
	t_ctsk.stksz = 0x6000;
	t_ctsk.quantum = 10;
	t_ctsk.para1 = (UINT32)&jpeg_opentv_file;
	t_ctsk.para2 = (UINT32)scale_coef;
	t_ctsk.name[0] = 'O';
	t_ctsk.name[1] = 'T';
	t_ctsk.name[2] = 'V';
	t_ctsk.task = jpeg_opentv_decoder_task;
	tsk_id = osal_task_create(&t_ctsk);
	if(OSAL_INVALID_ID == tsk_id)
	{
		return IMAGE_DEC_E_OPEN_FAIL;
	}

	return IMAGE_DEC_E_OK;
}

extern int imagedec_opentv_display(imagedec_id id, p_image_display_par pdisinfo, int layer, struct rect *src_rect, struct rect *dst_rect);
static int jpeg_opentv_display(st_image_buffer *image_buffer, int layer, struct rect *src_rect, struct rect *dst_rect)
{
    image_display_par dis_par;
    
    MEMSET(&dis_par, 0, sizeof(dis_par));

	dis_par.y = (UINT8 *)image_buffer->buffer;
	dis_par.width = image_buffer->width;
	dis_par.height = image_buffer->height;	
    imagedec_opentv_display(jpeg_opentv_file.id, &dis_par, layer, src_rect, dst_rect);
    osal_task_sleep(100);
    return 0;
}

static int jpeg_opentv_init(struct pe_image_cfg *pe_image_cfg)
{
//	struct vpo_device *pvpo_sd;

	MEMSET(&jpeg_opentv_file, 0, sizeof(jpeg_opentv_file));

	jpeg_opentv_file.init_conf.frm_y_size = pe_image_cfg->frm_y_size;
	jpeg_opentv_file.init_conf.frm_y_addr = pe_image_cfg->frm_y_addr;
	jpeg_opentv_file.init_conf.frm_c_size = pe_image_cfg->frm_c_size;
	jpeg_opentv_file.init_conf.frm_c_addr = pe_image_cfg->frm_c_addr;
	jpeg_opentv_file.init_conf.frm2_y_size = pe_image_cfg->frm2_y_size;
	jpeg_opentv_file.init_conf.frm2_y_addr = pe_image_cfg->frm2_y_addr;
	jpeg_opentv_file.init_conf.frm2_c_size = pe_image_cfg->frm2_c_size;
	jpeg_opentv_file.init_conf.frm2_c_addr = pe_image_cfg->frm2_c_addr;
	jpeg_opentv_file.init_conf.decoder_buf = pe_image_cfg->decoder_buf;
	jpeg_opentv_file.init_conf.frm3_y_size = pe_image_cfg->frm3_y_size;
	jpeg_opentv_file.init_conf.frm3_y_addr = pe_image_cfg->frm3_y_addr;
	jpeg_opentv_file.init_conf.frm3_c_size = pe_image_cfg->frm3_c_size;
	jpeg_opentv_file.init_conf.frm3_c_addr = pe_image_cfg->frm3_c_addr;
#if 0
	pvpo_sd = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 1);
	if(NULL != pvpo_sd)
	{
		jpeg_file.init_conf.frm4_y_size = pe_image_cfg->frm4_y_size;
		jpeg_file.init_conf.frm4_y_addr = pe_image_cfg->frm4_y_addr;
		jpeg_file.init_conf.frm4_c_size = pe_image_cfg->frm4_c_size;
		jpeg_file.init_conf.frm4_c_addr = pe_image_cfg->frm4_c_addr;
	}
#endif
    jpeg_opentv_file.init_conf.frm4_y_size = pe_image_cfg->frm4_y_size;
    jpeg_opentv_file.init_conf.frm4_y_addr = pe_image_cfg->frm4_y_addr;
    jpeg_opentv_file.init_conf.frm4_c_size = pe_image_cfg->frm4_c_size;
    jpeg_opentv_file.init_conf.frm4_c_addr = pe_image_cfg->frm4_c_addr;


	jpeg_opentv_file.init_conf.decoder_buf_len = pe_image_cfg->decoder_buf_len;
	jpeg_opentv_file.init_conf.fread_callback = jpeg_opentv_img_read_data;
	jpeg_opentv_file.init_conf.fseek_callback = jpeg_opentv_img_seek_data;
	jpeg_opentv_file.init_conf.imagedec_status = jpeg_process;
	jpeg_opentv_file.init_conf.decoder_buf = (unsigned char *)(((unsigned long)jpeg_opentv_file.init_conf.decoder_buf + 0x100)&0xffffff00);
    jpeg_opentv_file.init_conf.ali2dto3d_callback = pe_image_cfg->ali_pic_2dto3d;

	if(sys_ic_get_chip_id()==ALI_S3602 || sys_ic_get_chip_id()==ALI_S3602F ||sys_ic_get_chip_id()==ALI_S3811 || sys_ic_get_chip_id()>=ALI_C3701)
		jpeg_opentv_file.init_conf.frm_mb_type = 1;
	else
		jpeg_opentv_file.init_conf.frm_mb_type = 0;

	jpeg_opentv_file.id = imagedec_init(&jpeg_opentv_file.init_conf);
	jpeg_opentv_file.lock = osal_mutex_create();
    //libc_printf("jpeg_file:id %d\n", jpeg_file.id);
    return jpeg_opentv_file.id;
}

static int jpeg_opentv_get_info(char *filename, struct image_info *info)
{
    image_info ifo;
    BOOL ret = 0;
    FILE *fp = NULL;

    jpeg_opentv_img_seek_data(0xff, 0, 0);

    ret = imagedec_getinfo(jpeg_opentv_file.id, (UINT32)fp, &ifo);

    if(ret)
    {
        info->height = ifo.image_height;
        info->width = ifo.image_width;
        info->bbp = ifo.precision;
        return ret;
    }
    return -1;

}

extern int imagedec_opentv_get_decoded_buffer();
static int jpeg_opentv_get_decoded_buffer()
{
    UINT32 buffer_addr;
    buffer_addr = imagedec_opentv_get_decoded_buffer();
    return buffer_addr;
    
}

static int jpeg_opentv_abort()
{
    if(jpeg_opentv_file.id > 0)
    {
        osal_mutex_lock(jpeg_opentv_file.lock, TMO_FEVR);
        jpeg_opentv_file.mp_cb = NULL;
        osal_mutex_unlock(jpeg_opentv_file.lock);

        imagedec_stop(jpeg_opentv_file.id);
        jpeg_opentv_file.command = JPG_CMD_STOP;

        while(1 == jpeg_opentv_file.task_terminated)
        {
            osal_task_sleep(10);
        }
        return 0;
    }
    return -1;

    return RET_SUCCESS;
}

extern const plugin_image_raw jpeg_raw;

image_plugin jpeg_plugin =
{
    .handle = 0,
    .filename = NULL,
    .description = NULL,
    .init = jpeg_init,
    .about = NULL,
    .configure = NULL,
    .is_our_file = jpeg_is_our_file,
    .decode = jpeg_decode,
    .show = jpeg_show,
    .abort = jpeg_abort,
    .cleanup = jpeg_cleanup,
    .get_info = jpeg_get_info,
    .rotate = jpeg_rotate,
    .zoom = jpeg_zoom,
    .swap = jpeg_swap,
    .option = jpeg_option,
    .set_param = jpeg_set_disp_param,
	.opentv =
    {
        .init_opentv = jpeg_opentv_init,
	    .decode_opentv = jpeg_opentv_decode,
	    .display_buffer = jpeg_opentv_display,
	    .get_info_opentv = jpeg_opentv_get_info,
	    .get_decoded_buffer = jpeg_opentv_get_decoded_buffer,
	    .abort_opentv = jpeg_opentv_abort,
    },
    .raw = (plugin_image_raw*)&jpeg_raw,
};

