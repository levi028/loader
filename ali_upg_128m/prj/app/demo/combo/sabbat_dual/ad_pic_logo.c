#include <sys_config.h>

#if 0
#ifdef PIP_PNG_GIF

#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <hld/ge/ge.h>
#include <hld/osd/osddrv_dev.h>
#include <hld/dis/vpo.h>
#include <hld/decv/decv.h>
#include <api/libfs2/stdio.h>

#include <api/libimagedec/imagedec.h>
#include <api/libmp/gif.h>
#include <api/libmp/png.h>
#include <hld/osd/osddrv.h>
#include <api/libmp/pe.h>
#include <api/libmp/imgdec_engine_raw.h>
#include <api/libmp/imgdec_common.h>

#include "still_1280_720.h"
#include "still_2_1280.h"


int adpic_logoshow_main()
{
    int      ret = RET_SUCCESS;

    int id = 1;
    imgengi_raw_conf_t config;
 
    image_engine_raw_alloc(id);

    config.type = IMGENGI_RAW_INITCONF_FILE;
    config.data.init_file.type      = MP_FILEHDLE_BUFF;
    config.data.init_file.bufffile     = &STILL_boot_1280;
    config.data.init_file.bufffile_len = sizeof(STILL_boot_1280);
    //config.data.init_file.bufffile     = &STILL_still_1280_720;
    //config.data.init_file.bufffile_len = sizeof(STILL_still_1280_720);

    image_engine_raw_initconf(id, &config);

    config.type = IMGENGI_RAW_INITCONF_MEM;
    config.data.init_mem.type       = MP_MEMHDLE_ADVERT;
    config.data.init_mem.buffer     = (UINT8*)__MM_PVR_VOB_BUFFER_ADDR;
    config.data.init_mem.buflen     = __MM_PVR_VOB_BUFFER_LEN;
    image_engine_raw_initconf(id, &config);

    config.type = IMGENGI_RAW_INITCONF_YUVFRM;
    config.data.init_frm.image_y_addr      = __MM_FB0_Y_START_ADDR;
    config.data.init_frm.image_y_addr_len  = __MM_FB0_Y_LEN;
    config.data.init_frm.image_c_addr      = __MM_FB0_C_START_ADDR;
    config.data.init_frm.image_c_addr_len  = __MM_FB0_C_LEN;
    image_engine_raw_initconf(id, &config);

    image_engine_raw_init(id);

    image_engine_raw_decconf(id, &config);
    image_engine_raw_decode(id);
    osal_task_sleep(150);
#if 1
    config.type = IMGENGI_RAW_DISPCONF_DSTRECT;
    config.data.disp_dstrect.u_start_x = 0;
    config.data.disp_dstrect.u_start_y = 0;
    config.data.disp_dstrect.u_width  = 540;
    config.data.disp_dstrect.u_height = 432;
    image_engine_raw_dispconf(id, &config);
    
    image_engine_raw_display(id);
#endif
    image_engine_raw_abort(id);
    image_engine_raw_free(id);


    return ret;
}

int adpic_logoshow_aux()
{
    int      ret = RET_SUCCESS;

    int id = 2;
    imgengi_raw_conf_t config;
 
    image_engine_raw_alloc(id);

    config.type = IMGENGI_RAW_INITCONF_FILE;
    config.data.init_file.type      = MP_FILEHDLE_BUFF;
    config.data.init_file.bufffile     = &STILL_still_1280_720;
    config.data.init_file.bufffile_len = sizeof(STILL_still_1280_720);
    image_engine_raw_initconf(id, &config);

    config.type = IMGENGI_RAW_INITCONF_MEM;
    config.data.init_mem.type       = MP_MEMHDLE_ADVERT;
    config.data.init_mem.buffer     = (UINT8*)__MM_PVR_VOB_BUFFER_ADDR;
    config.data.init_mem.buflen     = __MM_PVR_VOB_BUFFER_LEN;
    image_engine_raw_initconf(id, &config);

    config.type = IMGENGI_RAW_INITCONF_YUVFRM;
    config.data.init_frm.image_y_addr      = __MM_FB1_Y_START_ADDR;
    config.data.init_frm.image_y_addr_len  = __MM_FB1_Y_LEN;
    config.data.init_frm.image_c_addr      = __MM_FB1_C_START_ADDR;
    config.data.init_frm.image_c_addr_len  = __MM_FB1_C_LEN;

    image_engine_raw_initconf(id, &config);

    image_engine_raw_init(id);

    image_engine_raw_decconf(id, &config);
    image_engine_raw_decode(id);
    osal_task_sleep(150);
#if 1
    config.type = IMGENGI_RAW_DISPCONF_LAYER;
    config.data.disp_layer = VPO_LAYER_AUXP;
    image_engine_raw_dispconf(id, &config);
#endif
#if 1
    config.type = IMGENGI_RAW_DISPCONF_DSTRECT;
    config.data.disp_dstrect.u_start_x = 180;
    config.data.disp_dstrect.u_start_y = 144;
    config.data.disp_dstrect.u_width  = 540;
    config.data.disp_dstrect.u_height = 432;
    image_engine_raw_dispconf(id, &config);
#endif
    image_engine_raw_display(id);

    image_engine_raw_abort(id);
    image_engine_raw_free(id);

    
    return ret;
}

int ad_pic_jpegbmp_logoshow_dual_exam1()
{
    int      ret = RET_SUCCESS;


    osd_clear_screen();
    struct vpo_device*   vpo_dev = NULL;
    vpo_dev    = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0);
    vpo_ioctl(vpo_dev, VPO_IO_SET_LAYER_ORDER, AUXP_MP_GMAS_GMAF);
    //vpo_ioctl(vpo_dev, VPO_IO_SET_LAYER_ORDER, MP_AUXP_GMAF_GMAS);

    adpic_logoshow_main();
    adpic_logoshow_aux();

    return ret;
}

int adpic_logodec_pic1()
{
    int      ret = RET_SUCCESS;

    int id = 1;
    imgengi_raw_conf_t config;
 
    image_engine_raw_alloc(id);

    config.type = IMGENGI_RAW_INITCONF_FILE;
    config.data.init_file.type      = MP_FILEHDLE_BUFF;
    config.data.init_file.bufffile     = &STILL_boot_1280;
    config.data.init_file.bufffile_len = sizeof(STILL_boot_1280);
    image_engine_raw_initconf(id, &config);

    config.type = IMGENGI_RAW_INITCONF_MEM;
    config.data.init_mem.type       = MP_MEMHDLE_ADVERT;
    config.data.init_mem.buffer     = (UINT8*)__MM_PVR_VOB_BUFFER_ADDR;
    config.data.init_mem.buflen     = __MM_PVR_VOB_BUFFER_LEN;
    image_engine_raw_initconf(id, &config);

    config.type = IMGENGI_RAW_INITCONF_YUVFRM;
    config.data.init_frm.image_y_addr      = __MM_FB2_Y_START_ADDR;
    config.data.init_frm.image_y_addr_len  = __MM_FB2_Y_LEN;
    config.data.init_frm.image_c_addr      = __MM_FB2_C_START_ADDR;
    config.data.init_frm.image_c_addr_len  = __MM_FB2_C_LEN;
    image_engine_raw_initconf(id, &config);

    image_engine_raw_init(id);

    image_engine_raw_decconf(id, &config);
    image_engine_raw_decode(id);

    return ret;

}

int adpic_logodec_pic2()
{
    int      ret = RET_SUCCESS;

    int id = 2;
    imgengi_raw_conf_t config;
 
    image_engine_raw_alloc(id);

    config.type = IMGENGI_RAW_INITCONF_FILE;
    config.data.init_file.type      = MP_FILEHDLE_BUFF;
    config.data.init_file.bufffile     = &STILL_still_1280_720;
    config.data.init_file.bufffile_len = sizeof(STILL_still_1280_720);
    image_engine_raw_initconf(id, &config);

    config.type = IMGENGI_RAW_INITCONF_MEM;
    config.data.init_mem.type       = MP_MEMHDLE_ADVERT;
    config.data.init_mem.buffer     = (UINT8*)__MM_PVR_VOB_BUFFER_ADDR;
    config.data.init_mem.buflen     = __MM_PVR_VOB_BUFFER_LEN;
    image_engine_raw_initconf(id, &config);

    config.type = IMGENGI_RAW_INITCONF_YUVFRM;
#if 0
    config.data.init_frm.image_y_addr      = __MM_FB1_Y_START_ADDR;
    config.data.init_frm.image_y_addr_len  = __MM_FB1_Y_LEN;
    config.data.init_frm.image_c_addr      = __MM_FB1_C_START_ADDR;
    config.data.init_frm.image_c_addr_len  = __MM_FB1_C_LEN;
#endif
    config.data.init_frm.image_y_addr      = __MM_FB3_Y_START_ADDR;
    config.data.init_frm.image_y_addr_len  = __MM_FB1_Y_LEN;
    config.data.init_frm.image_c_addr      = __MM_FB4_C_START_ADDR;
    config.data.init_frm.image_c_addr_len  = __MM_FB1_C_LEN;

    image_engine_raw_initconf(id, &config);

    image_engine_raw_init(id);

    image_engine_raw_decconf(id, &config);
    image_engine_raw_decode(id);

    return ret;
}

int adpic_logodisp_pic1()
{
    int      ret = RET_SUCCESS;

    int id = 1;
    imgengi_raw_conf_t config;

    config.type = IMGENGI_RAW_DISPCONF_LAYER;
    config.data.disp_layer = VPO_LAYER_MAIN;
    image_engine_raw_dispconf(id, &config);


    config.type = IMGENGI_RAW_DISPCONF_DSTRECT;
    config.data.disp_dstrect.u_start_x = 0;
    config.data.disp_dstrect.u_start_y = 0;
    config.data.disp_dstrect.u_width  = 540;
    config.data.disp_dstrect.u_height = 432;
    image_engine_raw_dispconf(id, &config);
    
    image_engine_raw_display(id);

    image_engine_raw_abort(id);
    image_engine_raw_free(id);


    return ret;
}

int adpic_logodisp_pic2()
{
    int      ret = RET_SUCCESS;

    int id = 2;
    imgengi_raw_conf_t config;

    config.type = IMGENGI_RAW_DISPCONF_LAYER;
    config.data.disp_layer = VPO_LAYER_AUXP;
    image_engine_raw_dispconf(id, &config);

    config.type = IMGENGI_RAW_DISPCONF_DSTRECT;
    config.data.disp_dstrect.u_start_x = 180;
    config.data.disp_dstrect.u_start_y = 144;
    config.data.disp_dstrect.u_width  = 540;
    config.data.disp_dstrect.u_height = 432;
    image_engine_raw_dispconf(id, &config);
    
    image_engine_raw_display(id);

    image_engine_raw_abort(id);
    image_engine_raw_free(id);

    return ret;
}


int ad_pic_jpegbmp_logoshow_dual_exam2()
{

    int      ret = RET_SUCCESS;


    osd_clear_screen();
    struct vpo_device*   vpo_dev = NULL;
    vpo_dev    = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0);
    //vpo_ioctl(vpo_dev, VPO_IO_SET_LAYER_ORDER, AUXP_MP_GMAS_GMAF);
    //vpo_win_onoff_ext(vpo_dev, TRUE, VPO_LAYER_MAIN);
    //vpo_win_onoff_ext(vpo_dev, TRUE, VPO_LAYER_AUXP);


    adpic_logodec_pic1();
    adpic_logodec_pic2();
    adpic_logodisp_pic2();
    adpic_logodisp_pic1();


    return RET_SUCCESS;
}

int ad_pic_jpegbmp_logoshow_dual_exam3_task()
{
    adpic_logodec_pic2();

#if 1
    int id   = 2;
    int incr_w = 10;
    int incr_h = 40;

    imgengi_raw_conf_t config;
    
    config.type = IMGENGI_RAW_DISPCONF_LAYER;
    config.data.disp_layer = VPO_LAYER_AUXP;
    image_engine_raw_dispconf(id, &config);
    image_engine_raw_display(id);

#if 0
    config.type = IMGENGI_RAW_DISPCONF_DSTRECT;
    config.data.disp_dstrect.uStartX = 0;
    config.data.disp_dstrect.uStartY = 0;
    config.data.disp_dstrect.uWidth  = 360;
    config.data.disp_dstrect.uHeight = 288;
    image_engine_raw_dispconf(id, &config);
#endif

    struct vpo_device*   vpo_dev = NULL;
    vpo_dev    = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0);
    //vpo_ioctl(vpo_dev, VPO_IO_SET_LAYER_ORDER, AUXP_MP_GMAS_GMAF);


    struct rect srcrect;
    struct rect dstrect;
    srcrect.u_start_x = 0;
    srcrect.u_start_y = 0;
    srcrect.u_width  = 720;
    srcrect.u_height = 576*5;

    dstrect.u_start_x = 0;
    dstrect.u_start_y = 0;
    dstrect.u_width  = 360;
    dstrect.u_height = 288*5;

    
    vpo_zoom_ext(vpo_dev, &srcrect, &dstrect, VPO_LAYER_AUXP);

    while(1) {
        vpo_zoom_ext(vpo_dev, &srcrect, &dstrect, VPO_LAYER_AUXP);
        osal_task_sleep(300);

        if (dstrect.u_width >= 720) {
            incr_w = -10;
            incr_h = -40;
        }
        if (dstrect.u_width <= 360) {
            incr_w = 10;
            incr_h = 40;
        }
        dstrect.u_width += incr_w;
        dstrect.u_height+= incr_h;

    }


#if 0
    while(1) {
        image_engine_raw_display(id);
        osal_task_sleep(500);
        config.data.disp_dstrect.uWidth += incr_w;
        config.data.disp_dstrect.uHeight+= incr_h;
        image_engine_raw_dispconf(id, &config);
 
        if (config.data.disp_dstrect.uWidth >= 720) {
            incr_w = -10;
            incr_h = -8;
        }
        if (config.data.disp_dstrect.uWidth <= 360) {
            incr_w = 10;
            incr_h = 8;
        } 
    }
#endif
    image_engine_raw_abort(id);
    image_engine_raw_free(id);
 #endif
    return RET_SUCCESS;

}


int ad_pic_jpegbmp_logoshow_dual_exam3()
{

    OSAL_T_CTSK t_ctsk;
    OSAL_ID tsk_id;

    t_ctsk.itskpri = OSAL_PRI_NORMAL;
    t_ctsk.stksz = 0x6000;
    t_ctsk.quantum = 10;
    t_ctsk.para1 = (UINT32)NULL;
    t_ctsk.para2 = (UINT32)NULL;
    t_ctsk.name[0] = 'E';
    t_ctsk.name[1] = 'X';
    t_ctsk.name[2] = '3';
    t_ctsk.task = ad_pic_jpegbmp_logoshow_dual_exam3_task;
    tsk_id = osal_task_create(&t_ctsk);
    if (OSAL_INVALID_ID == tsk_id)
    {
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

int ad_pic_jpegbmp_logoshow_dual()
{
    ad_pic_jpegbmp_logoshow_dual_exam1();
    //ad_pic_jpegbmp_logoshow_dual_exam2();
    //ad_pic_jpegbmp_logoshow_dual_exam3();

    //osd_clear_screen();

    //adpic_logoshow_main();

    //adpic_logoshow_aux();


    return RET_SUCCESS;
}
#endif
#endif

#ifdef APP_PIC_BG_AUX

#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <hld/ge/ge.h>
#include <hld/osd/osddrv_dev.h>
#include <hld/dis/vpo.h>
#include <hld/decv/decv.h>
#include <api/libfs2/stdio.h>

#include <api/libimagedec/imagedec.h>
#include <api/libmp/gif.h>
#include <api/libmp/png.h>
#include <hld/osd/osddrv.h>
#include <api/libmp/pe.h>
#include <api/libmp/imgdec_engine_raw.h>
#include <api/libmp/imgdec_common.h>


int ad_pic_backgroud_active()
{
    struct vpo_device*   vpo_dev = NULL;
    vpo_dev    = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0);
    vpo_ioctl(vpo_dev, VPO_IO_SET_LAYER_ORDER, AUXP_MP_GMAS_GMAF);


    int      ret = RET_SUCCESS;

    int id = 1;
    imgengi_raw_conf_t config;
 
    image_engine_raw_alloc(id);

    config.type = IMGENGI_RAW_INITCONF_FILE;
    config.data.init_file.type      = MP_FILEHDLE_BUFF;
    //config.data.init_file.bufffile     = &STILL_still_1280_720;
    //config.data.init_file.bufffile_len = sizeof(STILL_still_1280_720);
    ad_pic_dbase2_getimg_byname("STILL_still_1280_720.jpg",
        &config.data.init_file.bufffile, &config.data.init_file.bufffile_len);
    
    image_engine_raw_initconf(id, &config);

    config.type = IMGENGI_RAW_INITCONF_MEM;
    config.data.init_mem.type       = MP_MEMHDLE_ADVERT;
    //config.data.init_mem.buffer     = (UINT8*)__MM_PVR_VOB_BUFFER_ADDR;
    //config.data.init_mem.buflen     = __MM_PVR_VOB_BUFFER_LEN;

    config.data.init_mem.buffer     = (UINT8*)__MM_MP_BUFFER_ADDR;
    config.data.init_mem.buflen     = __MM_MP_BUFFER_LEN;
    image_engine_raw_initconf(id, &config);

    config.type = IMGENGI_RAW_INITCONF_YUVFRM;
#if 0
    config.data.init_frm.image_y_addr      = __MM_FB1_Y_START_ADDR;
    config.data.init_frm.image_y_addr_len  = __MM_FB1_Y_LEN;
    config.data.init_frm.image_c_addr      = __MM_FB1_C_START_ADDR;
    config.data.init_frm.image_c_addr_len  = __MM_FB1_C_LEN;
#endif
    config.data.init_frm.image_y_addr      = __MM_FB3_Y_START_ADDR;
    config.data.init_frm.image_y_addr_len  = __MM_FB1_Y_LEN;
    config.data.init_frm.image_c_addr      = __MM_FB4_C_START_ADDR;
    config.data.init_frm.image_c_addr_len  = __MM_FB1_C_LEN;

    image_engine_raw_initconf(id, &config);

    image_engine_raw_init(id);

    image_engine_raw_decconf(id, &config);
    image_engine_raw_decode(id);
    osal_task_sleep(150);

    config.type = IMGENGI_RAW_DISPCONF_LAYER;
    config.data.disp_layer = VPO_LAYER_AUXP;
    image_engine_raw_dispconf(id, &config);
    
    image_engine_raw_display(id);

    image_engine_raw_abort(id);
    image_engine_raw_free(id);


    struct rect srcrect;
    struct rect dstrect;
    srcrect.u_start_x = 0;
    srcrect.u_start_y = 0;
    srcrect.u_width  = 720;
    srcrect.u_height = 576*5;

    dstrect.u_start_x = 180;
    dstrect.u_start_y = 36*5;
    dstrect.u_width  = 360;
    dstrect.u_height = 288*5;

    vpo_zoom(vpo_dev, &srcrect, &dstrect);

    return RET_SUCCESS;
}

int ad_pic_backgroud_deactive()
{
    struct vpo_device*   vpo_dev = NULL;
    vpo_dev    = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0);
    vpo_ioctl(vpo_dev, VPO_IO_SET_LAYER_ORDER, MP_GMAF_GMAS_AUXP);

    vpo_win_onoff_ext(vpo_dev, FALSE, VPO_LAYER_AUXP);


    struct rect srcrect;
    struct rect dstrect;
    srcrect.u_start_x = 0;
    srcrect.u_start_y = 0;
    srcrect.u_width  = 720;
    srcrect.u_height = 576*5;

    dstrect.u_start_x = 0;
    dstrect.u_start_y = 0;
    dstrect.u_width  = 720;
    dstrect.u_height = 576*5;

    vpo_zoom(vpo_dev, &srcrect, &dstrect);

    return RET_SUCCESS;
}

#include "win_com_popup.h"

BOOL g_backgroud_jpg = FALSE;

int app_ad_pic_processkey2(UINT32 vkey)
{
    switch(vkey) {
        case V_KEY_MOVE:
            if (FALSE == g_backgroud_jpg) {
                ad_pic_backgroud_active();
                g_backgroud_jpg = TRUE;
            } else if (TRUE == g_backgroud_jpg) {
                ad_pic_backgroud_deactive();
                g_backgroud_jpg = FALSE;
            }
            break;
        default:
            break;
    }

    return RET_SUCCESS;
}


#endif


