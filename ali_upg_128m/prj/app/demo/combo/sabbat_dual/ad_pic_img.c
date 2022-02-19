/***************************************************************************************************
 * brief:     feature support , osd display picture 
 * detail:    big file read, decode file and fix the window 
 *            size, and give the examle
 * author:    saicheong.yuen
 * date:      2014-7-14
 ***************************************************************************************************/

#include <sys_config.h>

#ifdef PIP_PNG_GIF

#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <hld/ge/ge.h>
#include <hld/osd/osddrv_dev.h>
#include <hld/dis/vpo.h>
#include <api/libfs2/stdio.h>

#include <api/libimagedec/imagedec.h>
#include <api/libmp/gif.h>
#include <api/libmp/png.h>
#include <hld/osd/osddrv.h>

/* "ad_pic_conf.h" must be front, such as here, follow ad_pic_*.h */
#include "ad_pic_conf.h"

#include "ad_pic_file.h"
#include "ad_pic_src.h"
#include "win_com_popup.h"
#include "ap_ctrl_display.h"
#include <api/libmp/imgdec_common.h>

#ifdef  AD_PIC_DEBUG
#define AD_PIC_PRINTF	libc_printf
#else
#define AD_PIC_PRINTF(...)	do{}while(0)
#endif

extern BOOL check_usb_item_is_active();
extern int mp_decmem_construct(memhdle_id* memhdl, mp_memoper_type_t type, void* buffer, int buflen);
extern int gif_dec_ad(char* filename, img_dec_dispconfig_t* dispconfig, memhdle_id *memhdl_ptr);
extern int mp_decmem_open(memhdle_id memhdl);
extern int mp_filehdle_construct3(filehdle_id *fh, mp_fileoper_type_t type, memhdle_id memhdle);

/***************************************************************************************************
 * brief:     jpg/bmp file decoder operation
 * detail:    methods:
 *            1. ad_pic_jpgbmp_draw
 *            2. ad_pic_jpgbmp_decode
 *            3. ad_pic_jpgbmp_close
 *            4. ad_pic_jpgbmp_ge_display
 *            5. ad_pic_jpgbmp_ge_undisplay
 *            6. ad_pic_jpgbmp_osdlayer1_display
 *            7. ad_pic_jpgbmp_osdlayer1_display
 * author:    saicheong.yuen
 * date:      2014-9-26
 *            2014-11-14 
 ***************************************************************************************************/

/* low osd callback function, just update the picture rectangle area. */
static void ad_pic_jpgbmp_update_cb(void *handle, p_imagedec_out_image pimage, struct rect *rect)
{
    struct rect *dst_rect;
    dst_rect = (struct rect*) handle;
    dst_rect->u_start_x = rect->u_start_x;
    dst_rect->u_start_y = rect->u_start_y;
    dst_rect->u_width  = rect->u_width;
    dst_rect->u_height = rect->u_height;	
}

/* decode the jpeg bmp file and buffer it. */
static int ad_pic_jpgbmp_decode(file_h fh, UINT8 *deccode_buf, INT32 deccode_buflen, UINT8 *bitmap_buf, INT32 bitmap_buflen, struct rect *src_rect, struct rect *dst_rect)
{
    BOOL    ret = 0;
    INT32   bitmap_pic_len = 0;

    if ((NULL == deccode_buf) || (NULL == bitmap_buf) || (NULL == src_rect) || (NULL == dst_rect))
    {
        AD_PIC_PRINTF("%s() pointer parameter is NULL!\n", __FUNCTION__);
        return RET_FAILURE;
    }

    UINT32 chip_id   = 0;
    INT32  bytedepth = 4;
    chip_id = sys_ic_get_chip_id();
    if (ALI_S3281 == chip_id)
    {
        bytedepth = 2;
    }

    bitmap_pic_len = (INT32)((INT32)src_rect->u_width * (INT32)src_rect->u_height) * bytedepth;
    if (bitmap_buflen < bitmap_pic_len)
    {
        AD_PIC_PRINTF("%s() buffer size if too small, may lead to problem!\n", __FUNCTION__);
        return RET_FAILURE;
    }

    imagedec_id id = 0;

    /* initialize the decode buffer, and image file operations */
    imagedec_init_config img_init_info;
    MEMSET(&img_init_info,0,sizeof(imagedec_init_config));
    img_init_info.decoder_buf = deccode_buf;
    img_init_info.decoder_buf_len = deccode_buflen;
    img_init_info.fread_callback = (void *)ad_pic_file_read;
    img_init_info.fseek_callback = ad_pic_file_seek;

    id = imagedec_init(&img_init_info);
    AD_PIC_PRINTF("%s() imagedec_init id = (%d)!\n", __FUNCTION__, id);
    if (0 == id)
    {
        AD_PIC_PRINTF("%s() imagedec_init failed(%d)!\n", __FUNCTION__, id);
        return RET_FAILURE;
    }

    /* configure the decoder. */
    ret = imagedec_ioctl(id, IMAGEDEC_IO_CMD_OPEN_HW_ACCE, 0);
    if (TRUE != ret)
    {
        AD_PIC_PRINTF("%s() imagedec_ioctl(IMAGEDEC_IO_CMD_OPEN_HW_ACCE) failed!\n", __FUNCTION__);
        return RET_FAILURE;
    }

    /* configure the decode system. */
    imagedec_mode_par dec_par;
    MEMSET((void *)&dec_par,0,sizeof(imagedec_mode_par));
    dec_par.vp_close_control = 0;
    dec_par.pro_show = 1;
    dec_par.dis_rect.u_start_x = 0;
    dec_par.dis_rect.u_start_y = 0;
    dec_par.dis_rect.u_width = 720;
    dec_par.dis_rect.u_height = 576;
    dec_par.src_rect.u_start_x = 0;
    dec_par.src_rect.u_start_y = 0;
    dec_par.src_rect.u_width = 720;
    dec_par.src_rect.u_height = 576;
    imagedec_set_mode(id, 1, &dec_par);

    /* configure the osd callback and output parameters. */
    imagedec_osd_config config;
    MEMSET((void *)&config, 0, sizeof(imagedec_osd_config));
    config.blt = ad_pic_jpgbmp_update_cb;
    config.handle = ((void*)dst_rect);
    config.pic_frmt = 0;
    imagedec_osd_init(&config);

    /* configure the osd input parameters and buffer. */
    imagedec_osd_io_cmd cmd;
    MEMSET((void *)&cmd,0,sizeof(imagedec_osd_io_cmd));
    cmd.on = 1;
    cmd.bitmap_start = (UINT32)bitmap_buf;
    cmd.bitmap_size = bitmap_buflen;
    cmd.dis_rect.u_start_x = src_rect->u_start_x;
    cmd.dis_rect.u_start_y = src_rect->u_start_y;
    cmd.dis_rect.u_width  = src_rect->u_width;
    cmd.dis_rect.u_height = src_rect->u_height;

    imagedec_ioctl(id, IMAGEDEC_IO_CMD_OSD_SHOW, (UINT32)&cmd);

    /* decode. */
    ret = imagedec_decode(id, fh);
    if (ret != TRUE)
        AD_PIC_PRINTF("\n Decode Image Unsuccessfully");

    imagedec_ioctl(id, IMAGEDEC_IO_CMD_OSD_SHOW, (UINT32)NULL);

    /* recycle the resouce. */
    if (imagedec_stop(id) != TRUE)
        AD_PIC_PRINTF("\n Stop Image decoding unsuccessfully!\n");
    
    if (imagedec_release(id) != TRUE)
        AD_PIC_PRINTF("\n Release Image decoder unsuccessfully!\n");   

    /* return. */
    if (ret != TRUE)
        return RET_FAILURE;
    else
        return RET_SUCCESS;
}

/* release the source */
static int ad_pic_jpgbmp_close()
{
    return RET_SUCCESS;
}

/* display the image stored in the buffer. */
static int ad_pic_jpgbmp_ge_display(UINT8 *buf, struct osdrect *rect, UINT16 pixel_pitch)
{
    ge_rect_t dst_rect, src_rect;
    struct ge_device *ge_dev;
    INT32 ret = 0;
    ge_operating_entity entity;
    UINT32 layer_id = 0;
    UINT32 region_id =0;
    UINT32 cmd_hdl = 0;

    if (buf == NULL)
    {
        AD_PIC_PRINTF("%s() para buf illegal!\n", __FUNCTION__);
        return RET_FAILURE;
    }

    ge_dev = (struct ge_device *)dev_get_by_id(HLD_DEV_TYPE_GE, 0);
    ge_cmd_list_hdl cmd_list = ge_cmd_list_create(ge_dev, 10);
    if (ge_dev == NULL)
    {
        AD_PIC_PRINTF("%s() ge device find failed!\n", __FUNCTION__);
        return RET_FAILURE;
    }
    
    src_rect.left = rect->u_left;
    src_rect.top = rect->u_top;
    src_rect.width = rect->u_width;
    src_rect.height = rect->u_height;
    
    dst_rect.left = rect->u_left;
    dst_rect.top = rect->u_top+100;
    dst_rect.width = rect->u_width;
    dst_rect.height = rect->u_height;
    AD_PIC_PRINTF("left: %d, top: %d, width: %d, height: %d\n", dst_rect.left, dst_rect.top,
        dst_rect.width, dst_rect.height);

    osal_cache_flush(buf, src_rect.width * src_rect.height * 4 );
    ge_cmd_list_new(ge_dev,cmd_list,GE_COMPILE_AND_EXECUTE);
    ge_gma_set_region_to_cmd_list(ge_dev,layer_id,region_id,cmd_list);

    entity.color_format = GE_PF_ARGB8888;
    entity.base_address = (UINT32)buf;
    entity.data_decoder = GE_DECODER_DISABLE;
    entity.pixel_pitch = src_rect.width;
    entity.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;

    cmd_hdl = ge_cmd_begin(ge_dev,cmd_list,GE_DRAW_BITMAP);             
    ge_set_operating_entity(ge_dev,cmd_hdl,GE_PTN,&entity); 
    ge_set_clut_rgb_order(ge_dev,cmd_hdl,GE_RGB_ORDER_ARGB);
    ge_set_xy(ge_dev,cmd_hdl,GE_DST,dst_rect.left,dst_rect.top);
    ge_set_xy(ge_dev,cmd_hdl,GE_SRC,dst_rect.left,dst_rect.top);
    ge_set_xy(ge_dev,cmd_hdl,GE_PTN,src_rect.left,src_rect.top);                
    ge_set_wh(ge_dev,cmd_hdl,GE_DST_PTN,dst_rect.width,dst_rect.height);

    ge_cmd_end(ge_dev,cmd_hdl);
    ge_cmd_list_destroy(ge_dev, cmd_list);
    return ret;
}

static int ad_pic_jpgbmp_ge_undisplay()
{
    osd_clear_screen();

    return RET_SUCCESS;
}

/***************************************************************************************************
 * brief:     jpg/bmp file decoder operation for 3281 32M
 * detail:    methods:
 *            1. ad_pic_jpgbmp_display_osdlayer1
 *            2. ad_pic_jpgbmp_undisplay_osdlayer1
 * author:    saicheong.yuen
 * date:      2014-11-14
 ***************************************************************************************************/
static int ad_pic_jpgbmp_osdlayer1_display(UINT8 *buf, struct osdrect *rect, UINT16 pixel_pitch)
{
    struct osd_device *osd_dev;
   	struct osdrect region_rect;
   	struct osdrect dest_rect;
    struct osdrect region_pos;
   	struct osdpara init_param;
    UINT32 region_id =0;
	VSCR   source;
    INT32  ret;

    MEMSET((void *)&source,0,sizeof(VSCR));

    source.v_r.u_left = rect->u_left;
	source.v_r.u_top = rect->u_top;
    source.v_r.u_width = rect->u_width;
    source.v_r.u_height = rect->u_height;
    source.lpb_scr = (UINT8 *)buf;

    dest_rect.u_left = rect->u_left;
	dest_rect.u_top = rect->u_top+100;
    dest_rect.u_width = rect->u_width;
    dest_rect.u_height = rect->u_height;

#ifdef OSD3_ENABLE
    osd_dev = (struct osd_device *)dev_get_by_id(HLD_DEV_TYPE_OSD, 2);
#else
    osd_dev = (struct osd_device *)dev_get_by_id(HLD_DEV_TYPE_OSD, 1);
#endif
    if (osd_dev == NULL)
    {
         AD_PIC_PRINTF("%s() get osd dev failed!\n", __FUNCTION__);
         return RET_FAILURE;
    }

    osddrv_close((HANDLE)osd_dev);

    init_param.e_mode = OSD_HD_ARGB1555;
    init_param.u_galpha = 0x7f;
    init_param.u_galpha_enable = 0;
    init_param.u_pallette_sel = 0;
 
    ret = osddrv_open((HANDLE)osd_dev, &init_param);
    if (ret != RET_SUCCESS)
    {
     AD_PIC_PRINTF("%s() osd open failed!\n", __FUNCTION__);
     return RET_FAILURE;
    }

    osal_task_sleep(20);

    osddrv_show_on_off((HANDLE)osd_dev, FALSE);

    osddrv_io_ctl((HANDLE)osd_dev, OSD_IO_SET_TRANS_COLOR, 0x0000);
    osddrv_delete_region((HANDLE)osd_dev, region_id);

    region_rect.u_left = 0;
	region_rect.u_top = 0;
    region_rect.u_width = 112;//208;//304;
    region_rect.u_height = 112;//160;//208;

    ret = osddrv_create_region((HANDLE)osd_dev, region_id, &region_rect, NULL);
    if (ret != RET_SUCCESS)
    {
        AD_PIC_PRINTF("%s() osd create_region failed!\n", __FUNCTION__);
        return RET_FAILURE;
    }

    region_pos.u_top = 50;
    region_pos.u_left = 100;
    region_pos.u_width = region_rect.u_width;
    region_pos.u_height = region_rect.u_height;
    osddrv_set_region_pos((HANDLE)osd_dev, region_id, &region_pos);

    osddrv_region_fill((HANDLE)osd_dev, region_id, &region_rect, 0x0000);

	ret = osddrv_region_write((HANDLE)osd_dev, region_id, &source, &dest_rect);

    osddrv_show_on_off((HANDLE)osd_dev, TRUE);

    return RET_SUCCESS;
}


static int ad_pic_jpgbmp_osdlayer1_undisplay()
{
    struct osd_device *osd_dev;
   	//struct osdrect region_rect;
    UINT32 region_id =0;
    //INT32 ret;

#ifdef OSD3_ENABLE
    osd_dev = (struct osd_device *)dev_get_by_id(HLD_DEV_TYPE_OSD, 2);
#else
    osd_dev = (struct osd_device *)dev_get_by_id(HLD_DEV_TYPE_OSD, 1);
#endif

    if (osd_dev == NULL)
    {
         AD_PIC_PRINTF("%s() get osd dev failed!\n", __FUNCTION__);
         return RET_FAILURE;
    }


    osal_task_sleep(20);

    osddrv_show_on_off((HANDLE)osd_dev, FALSE);
    osddrv_delete_region((HANDLE)osd_dev, region_id);
    osddrv_show_on_off((HANDLE)osd_dev, TRUE);
    osddrv_close((HANDLE)osd_dev);

    return RET_SUCCESS;
}


/***************************************************************************************************
 * brief:     jpg/bmp file decoder operation for 3281 64M
 * detail:    methods:
 *            1. ad_pic_jpgbmp_display_osdlayer1
 *            2. ad_pic_jpgbmp_undisplay_osdlayer1
 * author:    saicheong.yuen
 * date:      2014-11-14
 ***************************************************************************************************/
static int ad_pic_jpgbmp_osdlayer0_display(UINT8 *buf, struct osdrect *rect, UINT16 pixel_pitch)
{
    struct osd_device *osd_dev;
   	//struct osdrect region_rect;
   	struct osdrect dest_rect;
    //struct osdrect region_pos;
   	//struct osdpara init_param;
    UINT32 region_id = 0;
	VSCR   source;
    __MAYBE_UNUSED__ INT32  ret;

    osd_dev = (struct osd_device *)dev_get_by_id(HLD_DEV_TYPE_OSD, 0);
    if (osd_dev == NULL)
    {
         AD_PIC_PRINTF("%s() get osd dev failed!\n", __FUNCTION__);
         return RET_FAILURE;
    }

    osal_task_sleep(20);

    osddrv_show_on_off((HANDLE)osd_dev, FALSE);

    osddrv_io_ctl((HANDLE)osd_dev, OSD_IO_SET_TRANS_COLOR, 0x0000);


    MEMSET((void *)&source,0,sizeof(VSCR));

    source.v_r.u_left = rect->u_left;
	source.v_r.u_top = rect->u_top;
    source.v_r.u_width = rect->u_width;
    source.v_r.u_height = rect->u_height;
    source.lpb_scr = (UINT8 *)buf;

    dest_rect.u_left = 100;//rect->u_left;
	dest_rect.u_top = rect->u_top+100;
    dest_rect.u_width = rect->u_width;
    dest_rect.u_height = rect->u_height;

    ret = osddrv_region_write_by_surface((HANDLE)osd_dev, region_id, (UINT8 *)buf, &dest_rect, rect, (rect->u_width<<1));

    osddrv_show_on_off((HANDLE)osd_dev, TRUE);

    return RET_SUCCESS;
}


static int ad_pic_jpgbmp_osdlayer0_undisplay()
{
    struct osd_device *osd_dev;
   	struct osdrect region_rect;
    UINT32 region_id = 0;
    //INT32 ret;


    osd_dev = (struct osd_device *)dev_get_by_id(HLD_DEV_TYPE_OSD, 0);
    if (osd_dev == NULL)
    {
         AD_PIC_PRINTF("%s() get osd dev failed!\n", __FUNCTION__);
         return RET_FAILURE;
    }

    region_rect.u_left = 100;
	region_rect.u_top = 0+100;
    region_rect.u_width = 300;
    region_rect.u_height = 200;


    osal_task_sleep(20);

    osddrv_show_on_off((HANDLE)osd_dev, FALSE);
    
    osddrv_region_fill((HANDLE)osd_dev, region_id, &region_rect, 0x0000);
    //osddrv_delete_region((HANDLE)osd_dev, region_id);
    osddrv_show_on_off((HANDLE)osd_dev, TRUE);
    //osddrv_close((HANDLE)osd_dev);

    return RET_SUCCESS;
}

/***************************************************************************************************
 * brief:     gif file decoder operation
 * detail:    methods:
 *            1. ad_pic_gif_draw
 *            2. ad_pic_gif_decode
 *            3. ad_pic_gif_close
 *            4. ad_pic_gif_ge_display
 *            5. ad_pic_gif_ge_undisplay
 * author:    saicheong.yuen
 * date:      2014-9-26
 ***************************************************************************************************/

char* g_file_name;
/* decode the gif file and display it. */
static int ad_pic_gif_decode(file_h fh, UINT8 *deccode_buf, INT32 deccode_buflen, UINT8 *bitmap_buf, INT32 bitmap_buflen, struct rect *src_rect, struct rect *dst_rect)
{
    int ret = 0;
    memhdle_id memhdl;
    char* filename = g_file_name;
    img_dec_dispconfig_t dec_config;
    dec_config.disp_layer_id = 1;
    dec_config.gma_disp_config.disp_layer_id = 0;
    dec_config.gma_disp_config.src_rect.u_start_x = 0;
    dec_config.gma_disp_config.src_rect.u_start_y = 0;
    dec_config.gma_disp_config.dst_rect.u_start_x = 0;
    dec_config.gma_disp_config.dst_rect.u_start_y = 100;
    mp_decmem_construct(&memhdl, MP_MEMHDLE_ADVERT, deccode_buf, deccode_buflen);
    ret = gif_dec_ad(filename, &dec_config, &memhdl);
    if(ret)
    {
        return RET_FAILURE;
    }

    return RET_SUCCESS;
}

/* release the source */
static int ad_pic_gif_close()
{
    return RET_SUCCESS;
}

/* resersed */
static int ad_pic_gif_ge_display(UINT8 *buf, struct osdrect *rect, UINT16 pixel_pitch)
{
    return RET_SUCCESS;
}

/* resersed */
static int ad_pic_gif_ge_undisplay()
{
    osd_clear_screen();

    return RET_SUCCESS;
}

/***************************************************************************************************
 * brief:     png file decoder operation
 * detail:    methods:
 *            1. ad_pic_png_draw
 *            2. ad_pic_png_decode
 *            3. ad_pic_png_close
 *            4. ad_pic_png_ge_display
 *            5. ad_pic_png_ge_undisplay
 * author:    saicheong.yuen
 * date:      2014-9-26
 ***************************************************************************************************/


typedef struct ad_png_ctrl_s
{
    char* filename;
    memhdle_id    memhdl;
    filehdle_id   fhdl;
    dechdle_id    dechdl;
    maphdle_id    maphdle_argb;
    //disphdle_id   disphdl_ge;
} ad_png_ctrl_t;

ad_png_ctrl_t  g_png_ctrl;


static int ad_pic_png_decode(file_h fh, UINT8 *deccode_buf, INT32 deccode_buflen, UINT8 *bitmap_buf, INT32 bitmap_buflen, struct rect *src_rect, struct rect *dst_rect)
{
    int   ret;
    char* filename = g_file_name;

    filehdle_id          fhdl;
    dechdle_id           dechdl;

    mp_decmem_construct(&g_png_ctrl.memhdl, MP_MEMHDLE_ADVERT, deccode_buf, deccode_buflen);
    //mp_decmem_construct(&g_png_ctrl.memhdl, MP_MEMHDLE_ADVERT,  __MM_MP_BUFFER_ADDR & ~(7<<28), __MM_MP_BUFFER_LEN);
    memhdle_id    memhdl;
    memhdl = g_png_ctrl.memhdl;
    mp_decmem_open(memhdl);
    /***************************************************************
     * first step: only decode the information from the file.
     * include follow operation:
     *   1) initialize file handle and decoder handle
     *   2) bind them together
     *   3) decode the file first section, the file information
     ***************************************************************/

    /* step1.1:  initialize file handle */
    //mp_filehdle_construct(&fhdl, MP_FILEHDLE_STD);
    //mp_filehdle_construct(&fhdl, MP_FILEHDLE_PECACHE);
    //mp_filehdle_construct(&fhdl, MP_FILEHDLE_MEMLOAD);
    mp_filehdle_construct3(&fhdl, MP_FILEHDLE_PECACHE, memhdl);
    ret = mp_filehdle_open(fhdl, filename);
    if (ret != RET_SUCCESS) {
        libc_printf("osd show png can't be open!!!!\n");
        mp_filehdle_destruct(fhdl); 
        return RET_FAILURE;
    }

    /* step1.2:  initialize decoder handle */
    mp_imgdechdle_construct(&dechdl, MP_DECHDLE_PNG_SW);
    mp_imgdechdle_open(dechdl, memhdl);

    /* step2:  bind them together */
    mp_imgdechdle_filebind_beforeparse(dechdl, fhdl);

    /* step3:  decode the file information */
    mp_imgdechdle_parseinfo(dechdl);

    /***************************************************************
     * second step: decode the file, to the mapping.
     * include follow operation:
     *   1) 
     *   2) initialize mapping(rgb) handle
     *   3) bind them (the mapping handle and decoder handle) together
     *   4) decode to the mapping
     *   5) release the file handle and decode handle
     ***************************************************************/

    /* step1:  */
    __MAYBE_UNUSED__ int canvas_width;
    __MAYBE_UNUSED__ int canvas_height;

    canvas_width  = 1280;
    canvas_height = 720;


    /* step1:  */
    int width;
    int height;
    mp_imgdechdle_originsizeget(dechdl, &width, &height);

    if ((width > 1280) ||  (height >360)){
        mp_imgdechdle_close(dechdl);
        mp_imgdechdle_destruct(dechdl);
        mp_filehdle_close(fhdl);
        mp_filehdle_destruct(fhdl); 

        return RET_FAILURE;
    }

    /* step2: initialize mapping handle */
    maphdle_id maphdle_argb;
    mp_maphdle_construct(&maphdle_argb, MP_MAPHDLE_ARGB8888);
    mp_mapoper_initconf_t init_config;

    init_config.initflag = 0;
    
    init_config.info_image.width  = width;
    init_config.info_image.height = height;
    init_config.initflag |= MP_MAPCOMMON_INITCONF_INFOIMAGE;
    
    init_config.info_canvas.width  = width;
    init_config.info_canvas.height = height;
    init_config.initflag |= MP_MAPCOMMON_INITCONF_INFOCANVAS;

    //init_config.mem_frm.argb.image_addr     = __MM_FB1_Y_START_ADDR;
    //init_config.mem_frm.argb.image_addr_len = __MM_FB1_Y_LEN;

    init_config.mem_frm.argb.image_addr     = (char *)bitmap_buf; // & ~(7<<28);
    init_config.mem_frm.argb.image_addr_len = bitmap_buflen;

    mp_maphdle_initconfig(maphdle_argb, &init_config);
    mp_maphdle_open(maphdle_argb, memhdl);

    /* step3: bind them together */
    mp_imgdechdle_mapbind_beforedecode(dechdl, maphdle_argb);   
    mp_imgdechdle_decodepreconfig(dechdl, 0);

    /* step4: decode to the mapping */
    mp_imgdechdle_decode(dechdl);

    /* step5: release the file handle and decode handle */
    mp_imgdechdle_close(dechdl);
    mp_imgdechdle_destruct(dechdl);
    mp_filehdle_close(fhdl);
    mp_filehdle_destruct(fhdl); 


    g_png_ctrl.filename = filename;
    g_png_ctrl.fhdl     = fhdl;
    g_png_ctrl.dechdl   = dechdl;
    g_png_ctrl.maphdle_argb = maphdle_argb;

    return RET_SUCCESS;
}
static int ad_pic_png_close()
{
    return RET_SUCCESS;
}

static int ad_pic_png_ge_display(UINT8 *buf, struct osdrect *rect, UINT16 pixel_pitch)
{
    __MAYBE_UNUSED__ filehdle_id          fhdl;
    __MAYBE_UNUSED__ dechdle_id           dechdl;
    __MAYBE_UNUSED__ maphdle_id           maphdle_argb;

    fhdl         = g_png_ctrl.fhdl;
    dechdl       = g_png_ctrl.dechdl;
    maphdle_argb = g_png_ctrl.maphdle_argb;


    /***************************************************************
     * third step: using ge to display the mapping
     * include follow operation:
     *   1) 
     ***************************************************************/

    mp_maphdle_imginfo_t imginfo;
    mp_maphdle_get_imageinfo(maphdle_argb, &imginfo);

    mp_maphdle_frm_argb_t imgbuf;
    mp_maphdle_get_imageaddr_argb(maphdle_argb, &imgbuf);


#if 0
    disphdle_id          disphdl_ge;
    mp_disphdle_construct(&disphdl_ge, MP_DISPHDL_GE);
    mp_disphdle_open(disphdl_ge);
    mp_disphdle_bind(disphdl_ge, maphdle_argb);
    mp_disphdle_config(disphdl_ge, NULL);
    mp_disphdle_run(disphdl_ge);
    mp_disphdle_destruct(disphdl_ge);
#endif

    struct osdrect dst_rect;
    dst_rect.u_left  = 0;
    dst_rect.u_top = 0;
    dst_rect.u_width  = imginfo.width;
    dst_rect.u_height = imginfo.height;
    

    ad_pic_jpgbmp_ge_display((UINT8 *)(imgbuf.image_addr), &dst_rect, imginfo.width);


    /***************************************************************
     * third step: release the mapping handle and display handle
     * include follow operation:
     *   1) release the mapping handle
     *   2) release the display handle
     ***************************************************************/
    mp_maphdle_close(maphdle_argb);
    mp_maphdle_destruct(maphdle_argb);

    return RET_SUCCESS;
}

static int ad_pic_png_ge_undisplay()
{
    osd_clear_screen();

    return RET_SUCCESS;
}


typedef struct ad_pic_decoder_s { 
    int (*decode) (file_h fh, UINT8 *deccode_buf, INT32 deccode_buflen, UINT8 *bitmap_buf, INT32 bitmap_buflen, struct rect *src_rect, struct rect *dst_rect);
    int (*close)();
    int (*display)(UINT8 *buf, struct osdrect *rect, UINT16 pixel_pitch);
    int (*undisplay)();
} ad_pic_decoder_t;


ad_pic_decoder_t ad_pic_jpgbmp_decoder_m3281_32m = 
{
    ad_pic_jpgbmp_decode,
    ad_pic_jpgbmp_close,
    ad_pic_jpgbmp_osdlayer1_display,
    ad_pic_jpgbmp_osdlayer1_undisplay
};

ad_pic_decoder_t ad_pic_jpgbmp_decoder_m3281_64m = 
{
    ad_pic_jpgbmp_decode,
    ad_pic_jpgbmp_close,
    ad_pic_jpgbmp_osdlayer0_display,
    ad_pic_jpgbmp_osdlayer0_undisplay
};

ad_pic_decoder_t ad_pic_jpgbmp_decoder = 
{
    ad_pic_jpgbmp_decode,
    ad_pic_jpgbmp_close,
    ad_pic_jpgbmp_ge_display,
    ad_pic_jpgbmp_ge_undisplay
};

ad_pic_decoder_t ad_pic_gif_decoder = 
{
    ad_pic_gif_decode,
    ad_pic_gif_close,
    ad_pic_gif_ge_display,
    ad_pic_gif_ge_undisplay
};

ad_pic_decoder_t ad_pic_png_decoder = 
{
    ad_pic_png_decode,
    ad_pic_png_close,
    ad_pic_png_ge_display,
    ad_pic_png_ge_undisplay
};

ad_pic_decoder_t *ad_pic_decoder;

static int app_ad_pic_getdecoder(char *filename)
{

	char *ext;
    UINT32 chip_id = 0;

    chip_id = sys_ic_get_chip_id();

    if (ALI_S3281 == chip_id)
    {
        ext = strrchr(filename, '.');
        if((!strncasecmp(ext, ".jpg", 4)) || (!strncasecmp(ext, ".JPG", 4)))
        {
#ifdef OSD_SHOW_JPG_FOR_32M
            ad_pic_decoder = &ad_pic_jpgbmp_decoder_m3281_32m;
#else
            ad_pic_decoder = &ad_pic_jpgbmp_decoder_m3281_64m;
#endif
            AD_PIC_PRINTF("selet the jpeg decoder!\n");
            return RET_SUCCESS;    
        }
        if((!strncasecmp(ext, ".jpeg", 5)) || (!strncasecmp(ext, ".JPEG", 5)))
        {
#ifdef OSD_SHOW_JPG_FOR_32M
            ad_pic_decoder = &ad_pic_jpgbmp_decoder_m3281_32m;
#else
            ad_pic_decoder = &ad_pic_jpgbmp_decoder_m3281_64m;
#endif
            AD_PIC_PRINTF("selet the jpeg decoder!\n");
            return RET_SUCCESS;        
        }

        AD_PIC_PRINTF("ERR: selet the none decoder!\n");
        return RET_FAILURE;
    }

    ext = strrchr(filename, '.');
	if (ext)
	{
		if((!strncasecmp(ext, ".png", 4)) || (!strncasecmp(ext, ".PNG", 4)))
		{  
		    ad_pic_decoder = &ad_pic_png_decoder;
            AD_PIC_PRINTF("selet the png decoder!\n");
			return RET_SUCCESS;
		}
		if((!strncasecmp(ext, ".gif", 4)) || (!strncasecmp(ext, ".GIF", 4)))
		{       
		    ad_pic_decoder = &ad_pic_gif_decoder;
            AD_PIC_PRINTF("selet the gif decoder!\n");
			return RET_SUCCESS;
		}
        if((!strncasecmp(ext, ".jpg", 4)) || (!strncasecmp(ext, ".JPG", 4)))
        {         
            ad_pic_decoder = &ad_pic_jpgbmp_decoder;
            AD_PIC_PRINTF("selet the jpg decoder!\n");
            return RET_SUCCESS;;
        }
        if((!strncasecmp(ext, ".jpeg", 5)) || (!strncasecmp(ext, ".JPEG", 5)))
        {
            ad_pic_decoder = &ad_pic_jpgbmp_decoder;
            AD_PIC_PRINTF("selet the jpeg decoder!\n");
            return RET_SUCCESS;
        }
        if((!strncasecmp(ext, ".bmp", 4)) || (!strncasecmp(ext, ".bmp", 4)))
        {
            ad_pic_decoder = &ad_pic_jpgbmp_decoder;
            AD_PIC_PRINTF("selet the bmp decoder!\n");
            return RET_SUCCESS;
        }
	}
    return RET_FAILURE;
}

/***************************************************************************************************
 * brief:     the example to use api to show pic.
 * detail:    the example to use api to show pic.
 * author:    saicheong.yuen
 * date:      2014-9-26
 ***************************************************************************************************/
 
typedef struct app_ad_pic_data_s
{
    file_h         hdl_curr;
    UINT8          *deccode_buf;
    INT32          deccode_buflen;
    UINT8          *bitmap_buf;
    INT32          bitmap_buflen;
    struct rect    src_rect;
    struct rect    dst_rect;    
} app_ad_pic_data_t;

/* allocation resouce and configure. decode the image file and buffer it. */
static int app_ad_pic_decode(app_ad_pic_data_t *ppic_img_data, const char* file_name)
{
    int         ret = RET_SUCCESS;
    UINT8       *decode_buf = NULL;
    INT32       deccode_buflen = 0;
    UINT8       *picture_buf = NULL;
    INT32       picture_buflen = 0;
    UINT8       *fileoper_buf = NULL;
    INT32       fileoper_buflen = 0;
    BOOL        usbfile_filled = TRUE;

    g_file_name = (char *)file_name;

    /* reserved the buffer for latter use */
    ad_pic_bufmgr_init();

    /* check if file exist at rom, get it by addr and size */
    ad_pic_dbase_getimg_byname(file_name, (char **)(&fileoper_buf), (int *)(&fileoper_buflen));

    /* bindle the file handle and the file from rom */
    if (fileoper_buf != NULL)
    {
        /* get the file handle. */
        ret = ad_pic_bufffile_init(&(ppic_img_data->hdl_curr), (const char *)fileoper_buf, fileoper_buflen);
        if (ret != RET_SUCCESS)
        {
             AD_PIC_PRINTF("%s: (buf)file can't be opened!\n", __FUNCTION__);
            return RET_FAILURE;
        }
        usbfile_filled = FALSE;
    }

    /* bindle the file handle and the file from usb */

	char *ext;
	ext = strrchr(g_file_name, '.');
    if((!strncasecmp(ext, ".png", 4)) || (!strncasecmp(ext, ".PNG", 4))) {
        usbfile_filled = FALSE;
        if (check_usb_item_is_active() == FALSE)
        {
            AD_PIC_PRINTF("%s() : no usb!\n", __FUNCTION__);
            return RET_FAILURE;
        }
    }

    if (usbfile_filled == TRUE)
    {
#if 1
        /* check if the usb inserted */
        if (check_usb_item_is_active() == FALSE)
        {
            AD_PIC_PRINTF("%s() : no usb!\n", __FUNCTION__);
            return RET_FAILURE;
        }
#endif
        /* get the file handle. */
        ret = ad_pic_file_init(&(ppic_img_data->hdl_curr), file_name);
        if (RET_SUCCESS != ret)
        {
            AD_PIC_PRINTF("%s: file can't be opened!\n", __FUNCTION__);
            return RET_FAILURE;  
        }
    }

    ret = app_ad_pic_getdecoder((char *)file_name);
    if (RET_SUCCESS != ret)
    {
        AD_PIC_PRINTF("%s: decoder can't be found!\n", __FUNCTION__);
        return RET_FAILURE;  
    }

    /* configure the picture size and location. define the display area. */
#ifdef OSD_SHOW_JPG_FOR_32M
    ppic_img_data->src_rect.u_start_x = 0;
    ppic_img_data->src_rect.u_start_y = 0;
    ppic_img_data->src_rect.u_width  = 112;
    ppic_img_data->src_rect.u_height = 112;
#else
    ppic_img_data->src_rect.u_start_x = 0;
    ppic_img_data->src_rect.u_start_y = 0;
    ppic_img_data->src_rect.u_width  = 300;
    ppic_img_data->src_rect.u_height = 200;
#endif

    /* the final picture size will fill dst_rect. fix the display area. */
    MEMSET((void *)&(ppic_img_data->dst_rect), 0, sizeof(struct osdrect));


    ad_pic_bufmgr_get_decodebuffer(&decode_buf, &deccode_buflen);
    ad_pic_bufmgr_get_picturebuffer(&picture_buf, &picture_buflen);

    /* decode buffer allocation. */
    ppic_img_data->deccode_buf = decode_buf;
    ppic_img_data->deccode_buflen = deccode_buflen;


    /* picture buffer allocation. */
    ppic_img_data->bitmap_buf = picture_buf;
    ppic_img_data->bitmap_buflen= picture_buflen;

    /* decode the image file */
    ret = ad_pic_decoder->decode(ppic_img_data->hdl_curr, ppic_img_data->deccode_buf, ppic_img_data->deccode_buflen, 
                ppic_img_data->bitmap_buf, ppic_img_data->bitmap_buflen, &ppic_img_data->src_rect, &ppic_img_data->dst_rect);
    if (RET_SUCCESS != ret)
    {
        ad_pic_decoder->close();
        if (usbfile_filled == TRUE)
            ad_pic_file_exit(ppic_img_data->hdl_curr);
        AD_PIC_PRINTF("%s: decode image file failured!\n", __FUNCTION__);
        return RET_FAILURE;
    }

    //ad_pic_bufffile_exit(ppic_img_data->hdl_curr);
    if (usbfile_filled == TRUE)
    {
        ret = ad_pic_file_exit(ppic_img_data->hdl_curr);
        if (RET_SUCCESS != ret)
        {
            ad_pic_decoder->close();
            AD_PIC_PRINTF("%s: file close error!\n", __FUNCTION__);
            return RET_FAILURE;
        }
    }

    AD_PIC_PRINTF("%s(): decode finish!\n", __FUNCTION__);

    return RET_SUCCESS;
}


/* display the picture. refer the file_name as the path. */
static int app_ad_pic_display(app_ad_pic_data_t *ppic_img_data)
{
    struct osdrect draw_rect;
    __MAYBE_UNUSED__ int    ret = RET_SUCCESS;

    /* draw the picture by using ge */

    draw_rect.u_left   = ppic_img_data->dst_rect.u_start_x;
    draw_rect.u_top    = ppic_img_data->dst_rect.u_start_y;
    draw_rect.u_width  = ppic_img_data->dst_rect.u_width;
    draw_rect.u_height = ppic_img_data->dst_rect.u_height;
    
    ret = ad_pic_decoder->display(ppic_img_data->bitmap_buf, &draw_rect, draw_rect.u_width);

    return RET_SUCCESS;
}

/* undisplay the picture. and recycle the resouces */
static int app_ad_pic_undisplay(app_ad_pic_data_t *ppic_img_data)
{
    int ret = RET_SUCCESS;

    ret = ad_pic_decoder->close();
    if (ret != RET_SUCCESS)
    {
        AD_PIC_PRINTF("%s() : decoder close failed!\n", __FUNCTION__);
    }

    ret = ad_pic_decoder->undisplay();
    if (ret != RET_SUCCESS)
    {
        AD_PIC_PRINTF("%s() : decoder undisplay failed!\n", __FUNCTION__);
    }

    return ret;
}


/***************************************************************************************************
 * brief:     application, display the picture by using reference path. 
 * detail:    application, display the picture by using reference path. 
 * author:    saicheong.yuen
 * date:      2014-9-26
 ***************************************************************************************************/

#ifdef SUPPORT_DEBUG_USING_OWN_PICTURE
/* image file path */
const char* app_ad_pic_jpg_item[] = {
//"/mnt/uda1/jpg/1.JPG",
//"/mnt/uda1/jpg/2.JPG",
"img_jpg1.jpg",
"img_jpg2.jpg",
"/mnt/uda1/jpg/3.JPG",
"/mnt/uda1/jpg/4.JPG",
"/mnt/uda1/jpg/5.JPG"
};

/* image file path */
const char* app_ad_pic_bmp_item[] = {
"/mnt/uda1/bmp/1.BMP",
"/mnt/uda1/bmp/2.BMP",
"/mnt/uda1/bmp/3.BMP",
"/mnt/uda1/bmp/4.BMP",
"/mnt/uda1/bmp/5.BMP"
};

/* image file path */
const char* app_ad_pic_gif_item[] =
{
"img_gif1.gif",
"img_gif2.gif",
"img_gif3.gif",
"img_gif4.gif",
//"img_gif5.gif"
"/mnt/uda1/gif/2.gif"
};

/* image file path */
const char* app_ad_pic_png_item[] = 
{
"/mnt/uda1/png/1.png",
"/mnt/uda1/png/2.png",
"/mnt/uda1/png/3.png",
"/mnt/uda1/png/4.png",
"/mnt/uda1/png/5.png"
};
#else

/* image file path */
const char* app_ad_pic_jpg_item[] = {
"/mnt/uda1/jpg/1.jpg",
"/mnt/uda1/jpg/2.jpg",
"/mnt/uda1/jpg/3.jpg",
"/mnt/uda1/jpg/4.jpg",
"/mnt/uda1/jpg/5.jpg"
};

/* image file path */
const char* app_ad_pic_bmp_item[] = {
"/mnt/uda1/bmp/1.bmp",
"/mnt/uda1/bmp/2.bmp",
"/mnt/uda1/bmp/3.bmp",
"/mnt/uda1/bmp/4.bmp",
"/mnt/uda1/bmp/5.bmp"
};

/* image file path */
const char* app_ad_pic_gif_item[] =
{
"/mnt/uda1/gif/1.gif",
"/mnt/uda1/gif/2.gif",
"/mnt/uda1/gif/3.gif",
"/mnt/uda1/gif/4.gif",
"/mnt/uda1/gif/5.gif"
};

/* image file path */
const char* app_ad_pic_png_item[] = 
{
"/mnt/uda1/png/1.png",
"/mnt/uda1/png/2.png",
"/mnt/uda1/png/3.png",
"/mnt/uda1/png/4.png",
"/mnt/uda1/png/5.png"
};

#endif

const char** app_ad_pic_category[] =
{
    (const char**)(&app_ad_pic_jpg_item),
    (const char**)(&app_ad_pic_bmp_item),
    (const char**)(&app_ad_pic_gif_item),
    (const char**)(&app_ad_pic_png_item)
};

const char* app_ad_pic_category_name[] =
{
    "category: JPEG ",
    "category: BMP",
    "category: GIF",
    "category: PNG"
};


char** app_ad_pic_path = (char **)(&app_ad_pic_jpg_item);


typedef struct app_ad_pic_handle_tag{
    int category;
    int item;
    int select_new;
    int display;
} app_ad_pic_handle_t;

typedef struct app_ad_pic_op_tag {
    int (*decode) (app_ad_pic_data_t *ppic_img_data, const char* file_name);
    int (*display) (app_ad_pic_data_t *ppic_img_data);
    int (*undisplay) (app_ad_pic_data_t *ppic_img_data);
} app_ad_pic_op_t;

app_ad_pic_op_t app_ad_pic_op_simple = {
    app_ad_pic_decode,
    app_ad_pic_display,
    app_ad_pic_undisplay
};

app_ad_pic_handle_t      g_app_ad_pic_handle = {0, 0, 0, 0};
app_ad_pic_op_t         *g_app_ad_pic_op = NULL;
app_ad_pic_data_t        g_app_ad_pic_data;


#ifdef SUPPORT_AD_PIC_UIINFO
/* popup menu: show infomation */
static int app_ad_pic_popup_show(char *info)
{
    //return RET_SUCCESS;
    UINT8 back_save;
    win_compopup_smsg_restoreback();
    win_compopup_init(WIN_POPUP_TYPE_SMSG);
    
    win_compopup_set_msg(info, NULL, 0);
    win_compopup_set_frame(0, 0, 500, 100);
    win_compopup_open_ext(&back_save);
    return RET_SUCCESS;
}

/* popup menu: clean infomation */
static int app_ad_pic_popup_clean(UINT32 delay)
{
    //return RET_SUCCESS;
    osal_task_sleep(delay);
    win_compopup_smsg_restoreback();
    return RET_SUCCESS;
}
#else
static int app_ad_pic_popup_show(char *info)
{
    return RET_SUCCESS;
}
static int app_ad_pic_popup_clean(UINT32 delay)
{
    return RET_SUCCESS;
}
#endif


/* select the file category */
static int app_ad_pic_select_category()
{
    int mod = 4;

    g_app_ad_pic_handle.category++;
    g_app_ad_pic_handle.category %= mod;

    g_app_ad_pic_handle.item = 0;
    g_app_ad_pic_handle.select_new = 1;


    app_ad_pic_path = (char **)(app_ad_pic_category[g_app_ad_pic_handle.category]);

    //if(g_app_ad_pic_handle.display == TRUE)
    //    g_app_ad_pic_op->undisplay(&g_app_ad_pic_data);

    app_ad_pic_popup_show((char *)(app_ad_pic_category_name[g_app_ad_pic_handle.category]));
    app_ad_pic_popup_clean(1000);

    return RET_SUCCESS;
}

/* select the file item */
static int app_ad_pic_select_item()
{
    int mod = 5;
    //mod = sizeof(app_ad_pic_path) / sizeof(const char*);
    char info[64];

    g_app_ad_pic_handle.item++;
    g_app_ad_pic_handle.item %= mod;
    g_app_ad_pic_handle.select_new = 1;

    AD_PIC_PRINTF("%s(): index=%d path=%s....\n", __FUNCTION__, g_app_ad_pic_handle.item, app_ad_pic_path[g_app_ad_pic_handle.item]);

    sprintf(info, "item: %s", app_ad_pic_path[g_app_ad_pic_handle.item]);

    //if(g_app_ad_pic_handle.display == TRUE)
    //    g_app_ad_pic_op->undisplay(&g_app_ad_pic_data);

    app_ad_pic_popup_show(info);
    app_ad_pic_popup_clean(1000);
    return RET_SUCCESS;
}

/* display and undisplay the picture */
static int app_ad_pic_action()
{
    int ret = RET_SUCCESS;

    /* initialize the display/undisplay operation */
    if (NULL == g_app_ad_pic_op)
    {
        g_app_ad_pic_op=&app_ad_pic_op_simple;
    }

    /* when the osd is cleaned, just display the picture */
    if (FALSE == g_app_ad_pic_handle.display)
    {
        app_ad_pic_popup_show("waiting..");
        //if (g_app_ad_pic_handle.category >= 2)
        //    app_ad_pic_popup_clean(0);
        ret = g_app_ad_pic_op->decode(&g_app_ad_pic_data, app_ad_pic_path[g_app_ad_pic_handle.item]);
        if (RET_SUCCESS != ret)
        {
            app_ad_pic_popup_show("ERROR!");
            app_ad_pic_popup_clean(1000);
            return ret;
        }
        //if (g_app_ad_pic_handle.category < 2)
            app_ad_pic_popup_clean(0);
        ret = g_app_ad_pic_op->display(&g_app_ad_pic_data);
        if (RET_SUCCESS != ret)
        {
            app_ad_pic_popup_show("ERROR!");
            app_ad_pic_popup_clean(1000);
            return ret;
        }
        g_app_ad_pic_handle.select_new = 0;
        g_app_ad_pic_handle.display = TRUE;
        return RET_SUCCESS;
    }

    /* the picture is displayed. when other picture isn't selected, so undisplay and clear osd. */
    if (g_app_ad_pic_handle.select_new == 0)
    {
        app_ad_pic_popup_show("prepare to clear!");
        app_ad_pic_popup_clean(1000);
        ret = g_app_ad_pic_op->undisplay(&g_app_ad_pic_data);
        if (RET_SUCCESS != ret)
        {
            return ret;
        }
        g_app_ad_pic_handle.display = FALSE;
        return RET_SUCCESS;
    /* the picture is displayed. when other picture is selected, so display other picture. */
    } else {
        ret = g_app_ad_pic_op->undisplay(&g_app_ad_pic_data);
        if (RET_SUCCESS != ret)
        {
            return ret;
        }
        g_app_ad_pic_handle.display = FALSE;
        app_ad_pic_popup_show("waiting..");
        //if (g_app_ad_pic_handle.category >= 2)
        //    app_ad_pic_popup_clean(0);
        ret = g_app_ad_pic_op->decode(&g_app_ad_pic_data, app_ad_pic_path[g_app_ad_pic_handle.item]);
        if (RET_SUCCESS != ret)
        {
            app_ad_pic_popup_show("ERROR!");
            app_ad_pic_popup_clean(1000);
            return ret;
        }
        //if (g_app_ad_pic_handle.category < 2)
            app_ad_pic_popup_clean(0);
        ret = g_app_ad_pic_op->display(&g_app_ad_pic_data);
        if (RET_SUCCESS != ret)
        {
            app_ad_pic_popup_show("ERROR!");
            app_ad_pic_popup_clean(1000);
            return ret;
        }
        
        //g_app_ad_pic_handle.pre = g_app_ad_pic_handle.curr;
        g_app_ad_pic_handle.select_new = 0;
        g_app_ad_pic_handle.display = TRUE;
        return RET_SUCCESS;
    }

    return RET_SUCCESS;
}


/***************************************************************************************************
 * brief:     application, display the picture by using reference path. 
 * detail:    application, display the picture by using reference path. 
 * author:    robin.zhai
 *            saicheong.yuen
 * date:      2014-12-8
 ***************************************************************************************************/
#ifdef SUPPORT_AD_PIC_TIMER

#include "copper_common/com_api.h"

static void app_ad_pic_timer_select(void)
{
    static int item = 0;

    g_app_ad_pic_handle.category = 0;
    g_app_ad_pic_handle.item = item;
    g_app_ad_pic_handle.select_new = 1;
    app_ad_pic_path = app_ad_pic_category[g_app_ad_pic_handle.category];

    item++;
    item = (item % 2);
}

static void app_ad_pic_timer_event(UINT32 vkey)
{
    app_ad_pic_timer_select();

    UINT32 hkey;
	ap_vk_to_hk(0, V_KEY_MOVE, &hkey);
	ap_send_msg(CTRL_MSG_SUBTYPE_KEY, hkey, TRUE);
}

static int app_ad_pic_timer_onoff()
{
    static ID timer_id = OSAL_INVALID_ID;

    if (OSAL_INVALID_ID != timer_id)
    {
        osal_timer_activate(timer_id, TCY_OFF);

        if(OSAL_INVALID_ID != timer_id)
            osal_timer_delete(timer_id);

        timer_id = OSAL_INVALID_ID;

        UINT32 hkey;
    	ap_vk_to_hk(0, V_KEY_MOVE, &hkey);
    	ap_send_msg(CTRL_MSG_SUBTYPE_KEY, hkey, TRUE);
    }
    else
    {
        OSAL_T_CTIM  timer_data;

        MEMSET(&timer_data, 0, sizeof(OSAL_T_CTIM));
    	timer_data.callback = app_ad_pic_timer_event;
    	timer_data.type  = TIMER_CYCLIC;
    	timer_data.time  = 1000;
    	timer_data.param = 0;

        timer_id = osal_timer_create(&timer_data);

        if (OSAL_INVALID_ID == timer_id)
        {
             return RET_FAILURE;
        }

        osal_timer_activate(timer_id, TCY_ON);
    }

    return RET_SUCCESS;
}
#endif

static int app_ad_pic_initialized = 0;

/* process key press!! */
int app_ad_pic_processkey(UINT32 vkey)
{
    int ret = RET_SUCCESS;

    /* initialize the display/undisplay operation */
    if (app_ad_pic_initialized == 0)
    {
        if (NULL == g_app_ad_pic_op)
        {
            g_app_ad_pic_op=&app_ad_pic_op_simple;
        }
        ad_pic_bufmgr_init();
        app_ad_pic_initialized = 1;
    }

    switch(vkey)
    {
        case V_KEY_RED:
            app_ad_pic_select_category();
            break;
        case V_KEY_SWAP:
            app_ad_pic_select_item();
            break;
        case V_KEY_MOVE:
            if(sys_data_get_tms())
            {
                UINT8 back_save = 0;
                
                win_compopup_init(WIN_POPUP_TYPE_SMSG);    
                win_compopup_set_msg("OSD Show Picture: Please disable TMS firstly !!!", NULL, 0);                
                win_compopup_set_frame(50, 50, 500, 200);               
                win_compopup_open_ext(&back_save);
                osal_task_sleep(2000);
                win_compopup_smsg_restoreback();
            }
            else
            {
                app_ad_pic_action();
            }
            break;
#ifdef SUPPORT_AD_PIC_TIMER
        case V_KEY_BLUE:
            app_ad_pic_timer_onoff();
            break;
#endif
        default:
            if (g_app_ad_pic_handle.display == TRUE)
            {
                if (NULL == g_app_ad_pic_op)
                {
                    return RET_FAILURE;
                }
                app_ad_pic_popup_show("prepare to clear!");
                ret = g_app_ad_pic_op->undisplay(&g_app_ad_pic_data);
                if (RET_SUCCESS != ret)
                {
                    return ret;
                }
                app_ad_pic_popup_clean(0);
                g_app_ad_pic_handle.display = FALSE;
            }
            break;
    }
    return RET_SUCCESS;
}

#include <api/libpub/lib_hde.h>
#include <api/libchunk/chunk.h>

int ad_pic_jpegbmp_logoshow(UINT32 logo_id)
{
    //RET_CODE ret_code = RET_FAILURE;
    UINT32   addr= 0;
    UINT32   len = 0;
    file_h   fh;
//    UINT8    *bitmap_buf;
//    INT32    bitmap_buflen;
    UINT8    *deccode_buf;
    INT32    deccode_buflen;
    UINT8    *fileoper_buf;
    INT32    fileoper_buflen;
    int      ret = RET_SUCCESS;
    BOOL     usbfile_filled = FALSE;

    struct sto_device *sto_flash_dev = NULL;
    CHUNK_HEADER chuck_hdr;

    ad_pic_bufmgr_init_logo();
    ad_pic_bufmgr_get_fileoperbuffer(&fileoper_buf, &fileoper_buflen);
    ad_pic_bufmgr_get_decodebuffer(&deccode_buf, &deccode_buflen);
//    ad_pic_bufmgr_get_picturebuffer(&bitmap_buf, &bitmap_buflen);

	#if 0
    //if ((g_app_ad_pic_handle.category == 0) && (g_app_ad_pic_handle.item != 0))
    if ((g_app_ad_pic_handle.category == 0) && (check_usb_item_is_active() == TRUE))
    {
        /* check if the usb inserted */
        if (check_usb_item_is_active() == FALSE)
        {
            AD_PIC_PRINTF("%s() : no usb!\n", __FUNCTION__);
        }
        else
        {
            FILE  *fp = NULL;
            fp = fopen(app_ad_pic_path[g_app_ad_pic_handle.item], "rb");
            len = fread(fileoper_buf, 1, fileoper_buflen, fp);
            AD_PIC_PRINTF("INFO: %s() read file:%s, len = %d\n", __FUNCTION__, app_ad_pic_path[g_app_ad_pic_handle.item], len);
            if ((len > 0) && (len < fileoper_buflen))
            {
                usbfile_filled = TRUE;
                osal_cache_flush(fileoper_buf, len);
            }
            fclose(fp);
        }
    }
	#endif
	
    if(usbfile_filled != TRUE)
    {
        if(0 == sto_get_chunk_header(logo_id, &chuck_hdr))
        {
            AD_PIC_PRINTF("%s() : flash error!\n", __FUNCTION__);
            return RET_FAILURE;
        }
        sto_flash_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
        addr = (UINT32)sto_chunk_goto(&logo_id, 0xFFFFFFFF, 1) + CHUNK_HEADER_SIZE;
        len = chuck_hdr.len - CHUNK_HEADER_SIZE + CHUNK_NAME;

        sto_get_data(sto_flash_dev, (UINT8 *)fileoper_buf, addr, len);
        osal_cache_flush(fileoper_buf, len);         
    }

    ret = ad_pic_bufffile_init(&fh, (const char*)fileoper_buf, len);
    if (ret != RET_SUCCESS)
    {
        return RET_FAILURE;
    }

    imagedec_id id = 0;

    /* initialize the decode buffer, and image file operations */
    imagedec_init_config img_init_info;
    MEMSET(&img_init_info, 0, sizeof(imagedec_init_config));
    #if 1
    img_init_info.frm_y_addr = __MM_FB_START_ADDR;
    img_init_info.frm_y_size = __MM_FB0_Y_LEN;
    img_init_info.frm_c_addr = img_init_info.frm_y_addr+img_init_info.frm_y_size;
    img_init_info.frm_c_size = __MM_FB0_C_LEN;
    
    img_init_info.frm2_y_addr = img_init_info.frm_c_addr+img_init_info.frm_c_size;
    img_init_info.frm2_y_size = __MM_FB1_Y_LEN;
    img_init_info.frm2_c_addr = img_init_info.frm2_y_addr+img_init_info.frm2_y_size;
    img_init_info.frm2_c_size = __MM_FB1_C_LEN;
	#endif
    img_init_info.decoder_buf = deccode_buf;
    img_init_info.decoder_buf_len = (UINT32)deccode_buflen;
    img_init_info.fread_callback = (void *)ad_pic_file_read;
    img_init_info.fseek_callback = ad_pic_file_seek;

    id = imagedec_init(&img_init_info);
    AD_PIC_PRINTF("%s() imagedec_init id = (%d)!\n", __FUNCTION__, id);
    if (0 == id)
    {
        AD_PIC_PRINTF("%s() imagedec_init failed(%d)!\n", __FUNCTION__, id);
        goto JPEG_LOGO_RELEASE;
    }


    struct rect dst_rect;

    if(VIEW_MODE_PREVIEW == hde_get_mode())
    {
        hde_get_preview_win(&dst_rect);
    }
    else
    {
        dst_rect.u_start_x = 0;
        dst_rect.u_start_y = 0;
        dst_rect.u_width  = 720;
        dst_rect.u_height = 2880;
    }


    imagedec_ioctl(id, IMAGEDEC_IO_CMD_EXPAND, (UINT32)&dst_rect);


    /* configure the decode system. */
    imagedec_mode_par dec_par;
    MEMSET((void *)&dec_par, 0, sizeof(imagedec_mode_par));
    dec_par.vp_close_control = 0;
    dec_par.pro_show = 1;
    dec_par.dis_rect.u_start_x = 0;
    dec_par.dis_rect.u_start_y = 0;
    dec_par.dis_rect.u_width = 720;
    dec_par.dis_rect.u_height = 576;
    dec_par.src_rect.u_start_x = 0;
    dec_par.src_rect.u_start_y = 0;
    dec_par.src_rect.u_width = 720;
    dec_par.src_rect.u_height = 576;
    dec_par.show_mode = M_NORMAL;
    imagedec_set_mode(id, IMAGEDEC_FULL_SRN, &dec_par);

    /* decode. */
    ret = imagedec_decode(id, fh);
    if (ret != TRUE)
        AD_PIC_PRINTF("\n Decode Image Unsuccessfully");


    imagedec_ioctl(id, IMAGEDEC_IO_CMD_EXPAND, (UINT32)NULL);

JPEG_LOGO_RELEASE:
    /* recycle the resouce. */
    if (imagedec_stop(id) != TRUE)
        AD_PIC_PRINTF("\n Stop Image decoding unsuccessfully!\n");
    
    if (imagedec_release(id) != TRUE)
        AD_PIC_PRINTF("\n Release Image decoder unsuccessfully!\n");   


    ad_pic_bufffile_exit(fh);


    return ret;
}

#endif

#if 0
#include <api/libmp/pe.h>

#if 1
struct image_info_pe
{
    unsigned long    fsize;
    unsigned long    width;
    unsigned long    height;
    unsigned long    bbp;
};
#endif

#include "still_1280_720.h"
int ad_pic_jpegbmp_opentv()
{
    struct pe_image_cfg img_init_info;
    BOOL ret = 0;
    imagedec_id id = 0;

    osd_clear_screen();


    MEMSET(&img_init_info,0,sizeof(struct pe_image_cfg));
    img_init_info.frm_y_size = __MM_FB0_Y_LEN;
    img_init_info.frm_y_addr = __MM_FB0_Y_START_ADDR;
    img_init_info.frm_c_size = __MM_FB0_C_LEN;
    img_init_info.frm_c_addr = __MM_FB0_C_START_ADDR;
    img_init_info.frm2_y_size = __MM_FB1_Y_LEN;
    img_init_info.frm2_y_addr = __MM_FB1_Y_START_ADDR;
    img_init_info.frm2_c_size = __MM_FB1_C_LEN;
    img_init_info.frm2_c_addr = __MM_FB1_C_START_ADDR;
    img_init_info.frm3_y_size = __MM_FB2_Y_LEN;
    img_init_info.frm3_y_addr = __MM_FB2_Y_START_ADDR;
    img_init_info.frm3_c_size = __MM_FB2_C_LEN;
    img_init_info.frm3_c_addr = __MM_FB2_C_START_ADDR;
    img_init_info.frm_mb_type = 0;
    //img_init_info.decoder_buf = (UINT8 *)((__MM_MP_BUFFER_ADDR) & ~(3<<29));//__MM_IMG_DEC_ADDR;
    img_init_info.decoder_buf = (UINT8 *)(__MM_MP_BUFFER_ADDR + 0x00800000);
    img_init_info.decoder_buf_len = __MM_MP_BUFFER_LEN - 0x00800000;
    //img_init_info.mp_cb = mp_cb;

    img_init_info.frm4_y_size = 0x00400000;
    img_init_info.frm4_y_addr = __MM_MP_BUFFER_ADDR;
    img_init_info.frm4_c_size = 0x00400000;
    img_init_info.frm4_c_addr = __MM_MP_BUFFER_ADDR + 0x00400000;


    libc_printf("img_init_info.decoder_buf = 0x%x\n", img_init_info.decoder_buf);        
    id = image_engine_init_opentv(&img_init_info);//c tree use. pe.c is different
    ret = imagedec_ioctl(id, IMAGEDEC_IO_CMD_OPEN_TV, 1);


#if 0
    struct image_config cfg;
    MEMSET(&cfg, 0, sizeof(cfg));

    cfg.decode_mode = IMAGEDEC_FULL_SRN;
    cfg.dest_top = 0;
    cfg.dest_left = 0;
    cfg.dest_width= 720;
    cfg.dest_height = 576;
    cfg.rotate = (rotate < 4) ? rotate :  ANG_ORI;

    ret = image_engine_decode_opentv("x.jpg", &STILL_still_1280_720, sizeof(STILL_still_1280_720), 0, &cfg);
#endif




    ret = image_decode_opentv("x.jpg", &STILL_still_1280_720, sizeof(STILL_still_1280_720), 
        IMAGEDEC_FULL_SRN, 0, 0, 720, 576, ANG_ORI, 0);

    osal_task_sleep(1000);

    int buffer;
    buffer = image_engine_get_decoded_buffer();

#if 0
    struct image_info_pe imginfo;
    image_engine_get_info_opentv("x.jpg", &imginfo);
#endif

    libc_printf("%s buf = 0x%x\n", __FUNCTION__, buffer);   

    struct rect src_rect;
    struct rect dst_rect;

    src_rect.u_start_x = 0;
    src_rect.u_start_y = 0;
    src_rect.u_width   = 720;
    src_rect.u_height  = 2880;

    dst_rect.u_start_x = 0;
    dst_rect.u_start_y = 0;
    dst_rect.u_width   = 720;
    dst_rect.u_height  = 2880;


    st_image_buffer imgbuf;
    imgbuf.buffer = buffer;
    //imgbuf.width  = imginfo.width;
    //imgbuf.height = imginfo.height;

    imgbuf.width  = 1280;
    imgbuf.height = 720;


    image_engine_display_opentv("x.jpg", 1, &imgbuf, &src_rect, &dst_rect);

}
#endif

