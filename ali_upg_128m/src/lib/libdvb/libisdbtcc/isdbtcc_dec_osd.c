/****************************************************************************
*  ALi Corporation, All Rights Reserved. 2010 Copyright (C)
*  File: isdbtcc_dec_osd.c
*  Description: control cc display functions
*  History:
*      Date            Author             Version   Comment
*      ====            ======         =======   =======
*  1.  2010.04.26  Vicky Hsu            0.1.000       create for isdbtcc
****************************************************************************/
/*******************
* INCLUDE FILES    *
********************/
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <hld/osd/osddrv.h>
#include <hld/hld_dev.h>
#include <hld/isdbtcc/isdbtcc_dec.h>
#include <hld/dis/vpo.h>
#include <api/libisdbtcc/lib_isdbtcc.h>
#include <api/libisdbtcc/isdbtcc_osd.h>
#include "isdbtcc_dec_osd.h"
/*******************
* EXPORTED DATA    *
********************/

/*******************
* LOCAL MACROS     *
********************/
#define DBG_CS    1
/*******************
* STATIC DATA      *
********************/
extern struct isdbtcc_config_par g_isdbtcc_config_par;
static BOOL gf_cc_show=FALSE;
static BOOL gf_cc_pre_show=FALSE;
static BOOL gf_cc_enbale_show=FALSE;
//Local CC_OSD status: TRUE: ready and ok to use, FASLE: need to call isdbtcc_osd_enter() first
static BOOL gf_in_subt_decoding=FALSE;
static UINT8 gb_cur_region= 0;    // region id of close caption , create via isdbtcc_osd_enter()
static VSCR g_vscr ;            // virtual screen for close caption
static struct osd_device *g_osd_dev_local = NULL;

extern UINT16 gw_for_x;
extern UINT16 gw_for_y;
extern UINT16 gw_area_x;
extern UINT16 gw_area_y;
extern UINT16 gw_disp_x;
extern UINT16 gw_disp_y;
extern UINT16 gw_char_x;
extern UINT16 gw_char_y;
extern UINT16 gw_space_x;
extern UINT16 gw_space_y;

extern UINT8 gbx2;    //Max X
extern UINT8 gby2;        //Max Y
extern UINT8 gbx;        //Active X
extern UINT8 gby;        //Active Y
extern UINT8 gb_fcol;    //Foreground Color
extern UINT8 gb_bcol;    //Background Color
extern UINT8 gb_rcol;    //Raster Color

static BOOL gf_ycbcr=FALSE;
/*******************
* LOCAL TYPEDEFS   *
********************/
//#define FONTSIZE        FONT_18X18
/**********************
* FUNCTION PROTOTYPES *
***********************/
void icc_fill_char(UINT16 x, UINT16 y,UINT16 w,UINT16 h,UINT8 bg_color);
const osd_scale_param m_osd_cc_scale_map_540[] =
{
    {PAL,             8,15,6,16,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 576}*/},
    {PAL_N,         8,15,6,16,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 576}*/},

    {NTSC,         8,9,6,8,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480}*/},
    {NTSC_443,     8,9,6,8,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480}*/},
    {PAL_60,         8,9,6,8,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480}*/},
    {PAL_M,         8,9,6,8,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480}*/},

    {LINE_720_25,     3,3,4,4,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1280, 720 }*/},
    {LINE_720_30,     3,3,4,4,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1280, 720}*/},

    {LINE_1080_25, 1,1,2,2,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1920, 1080}*/},
    {LINE_1080_30, 1,1,2,2,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1920, 1080}*/},
    {LINE_1080_50, 1,1,2,2,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1920, 1080}*/},
    {LINE_1080_60, 1,1,2,2,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1920, 1080}*/},
    {LINE_1080_24, 1,1,2,2,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1920, 1080}*/},
};

const osd_scale_param m_osd_cc_scale_map_480[] =
{
    {PAL,             1,5,1,6,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 576}*/},
    {PAL_N,         1,5,1,6,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 576}*/},

    {NTSC,         1,1,1,1,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480}*/},
    {NTSC_443,     1,1,1,1,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480}*/},
    {PAL_60,         1,1,1,1,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480}*/},
    {PAL_M,         1,1,1,1,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 720, 480}*/},

    {LINE_720_25,     9,2,16,3,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1280, 720 }*/},
    {LINE_720_30,     9,2,16,3,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1280, 720}*/},

    {LINE_1080_25, 3,4,8,9,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1920, 1080}*/},
    {LINE_1080_30, 3,4,8,9,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1920, 1080}*/},
    {LINE_1080_50, 3,4,8,9,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1920, 1080}*/},
    {LINE_1080_60, 3,4,8,9,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1920, 1080}*/},
    {LINE_1080_24, 3,4,8,9,/*{UI_SCREEN_WIDTH, UI_SCREEN_HEIGHT, 1920, 1080}*/},
};
static pcosd_scale_param icc_get_scale_param(enum tvsystem e_tvmode)
{
    UINT32 i,size_540,size_480,size=0;
    const osd_scale_param  *p_ui_map=(void*)NULL;
    size_540  = ARRAY_SIZE(m_osd_cc_scale_map_540);
    size_480  = ARRAY_SIZE(m_osd_cc_scale_map_480);

    switch(gw_for_y)
    {
        case 540:
              size = size_540;
              p_ui_map = m_osd_cc_scale_map_540;
    break;
        case 480:
              size = size_480;
              p_ui_map = m_osd_cc_scale_map_480;
    break;
    }

    for (i = 0; i < size; i++)
    {
        if (p_ui_map[i].tv_sys == e_tvmode)
        {
            return &(p_ui_map[i]);
        }
    }
    return &(p_ui_map[0]);
}

/********************************************
* NAME: cc_osd_enter
*       init cc osd
*
* Returns : void
* Parameter                     Flow    Description
* ------------------------------------------------------------------------------
* None
* ------------------------------------------------------------------------------
* Additional information:

******************************************/
void isdbtcc_osd_enter(void)
{
        struct osdpara    t_open_para;
    struct osdrect    t_open_rect;
    struct osdrect  r;
    enum tvsystem tvsys=PAL;
    RET_CODE t_ret;
    UINT16 b_areax=0,b_areay=0;
    UINT8 b_rcol=gb_rcol;

    gf_cc_enbale_show=TRUE;
    if((0>=gw_area_x) ||(0>=gw_area_y))
        goto LEAVE;
    if(TRUE==gf_cc_show)
        goto LEAVE;

    if(TRUE==gf_cc_pre_show)
        goto LEAVE;
    else
        gf_cc_pre_show=TRUE;

    while(gf_in_subt_decoding == TRUE)
        {
              os_task_sleep(1);
        }
    vpo_ioctl((struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS), VPO_IO_GET_OUT_MODE, (UINT32)(&tvsys));
    t_open_para.e_mode= OSD_256_COLOR;
    t_open_para.u_galpha_enable= FALSE;
    t_open_para.u_galpha= 0x0f;
    t_open_para.u_pallette_sel = 0;
    g_osd_dev_local = (struct osd_device *)dev_get_by_id(HLD_DEV_TYPE_OSD, g_isdbtcc_config_par.isdbtcc_osd_layer_id);
    if(NULL==g_osd_dev_local)    {ICC_PRINTF("Assert due to null osd_dev!\n");}

    #if 1
        t_ret=osddrv_close((HANDLE)g_osd_dev_local);
            if(RET_SUCCESS!=t_ret)    {ICC_PRINTF("Fail to Close osd_dev.\n");}
        osddrv_show_on_off((HANDLE)g_osd_dev_local, OSDDRV_OFF);
        t_ret=osddrv_open((HANDLE)g_osd_dev_local, &t_open_para);
            if(RET_SUCCESS!=t_ret)    {ICC_PRINTF("Fail to Open osd_dev.\n");}
        //osal_task_sleep(20);
    #else
    osddrv_show_on_off((HANDLE)g_osd_dev, OSDDRV_OFF);
    #endif

    b_areax=gw_for_x;
    b_areay=gw_for_y;
    t_open_rect.u_width = b_areax;
    t_open_rect.u_height = b_areay;
    t_open_rect.u_left = 0;//(1280-bAreax)>>1;
    t_open_rect.u_top = 0;//(720-bAreay);
    ICC_PRINTF("\nTVSys(%d),",tvsys);
    //ICC_PRINTF("[%d,%d,%d,%d]\n",bAreax,bAreay,tOpenRect.uLeft,tOpenRect.uTop);

    t_ret=osddrv_create_region((HANDLE)g_osd_dev_local, gb_cur_region, &(t_open_rect), &t_open_para);
        if(RET_SUCCESS!=t_ret)    {ICC_PRINTF("Fail to Create Region.\n");}

    // ((struct osd_s3601_private*)gOsdDev->priv)->surface   // need to check
    t_ret=osddrv_set_pallette((HANDLE)g_osd_dev_local,(UINT8*)gb_pallette, CC_COLOR_NUMBER, OSDDRV_YCBCR);
        if(RET_SUCCESS!=t_ret)    {ICC_PRINTF("Fail to Set Pallette.\n");}

    g_vscr.v_r.u_left = 0;
    g_vscr.v_r.u_top = 0;
    g_vscr.v_r.u_width = t_open_rect.u_width;
    g_vscr.v_r.u_height = t_open_rect.u_height;


    #if 0
    extern void* heap_ptr;
    ICC_PRINTF("15__Heap_PTR_in_isdbtcc_osd_enter,%x\n",heap_ptr);
    #endif

    #if 1
        g_vscr.lpb_scr = g_isdbtcc_config_par.g_buf_addr;
        MEMSET(g_vscr.lpb_scr,b_rcol,g_isdbtcc_config_par.g_buf_len);
    #else
    UINT8* b_vscr_buf;
    b_vscr_buf = (UINT8 *)MALLOC((UINT32)(t_open_rect.u_width)*(t_open_rect.u_height));
    if(NULL==b_vscr_buf)    {ICC_PRINTF("\nOh!Oh! Fail to alloc Disp Mem for CC!!!!\n");}
    g_vscr.lpb_scr = b_vscr_buf;
    MEMSET(b_vscr_buf,b_rcol,(UINT32)(t_open_rect.u_width)*(t_open_rect.u_height));
    #endif

    r.u_left = 0;
    r.u_top = 0;
    r.u_width =t_open_rect.u_width;
    r.u_height =t_open_rect.u_height;
    //tRet=OSDDrv_RegionFill((HANDLE)gOsdDev, gbCurRegion, &r, CC_TRANSPARENT);
    t_ret=osddrv_region_fill((HANDLE)g_osd_dev_local, gb_cur_region, &r, b_rcol);
    if(RET_SUCCESS!=t_ret)    {ICC_PRINTF("Fail to Fill Region with %d Color.\n",b_rcol);}
    //libc_printf("\ntvsys(%d),eOSDSys(%d)",tvsys,eOSDSys);

    enum osdsys  e_osdsys;
#ifdef HDTV_SUPPORT
    if(tvsys == NTSC||tvsys == PAL_M||tvsys == PAL_60||tvsys == NTSC_443||tvsys == LINE_720_30||tvsys == LINE_1080_30)
        e_osdsys = OSD_NTSC;
    else
        e_osdsys = OSD_PAL;
#else
    if(tvsys == NTSC||tvsys == PAL_M||tvsys == PAL_60||tvsys == NTSC_443)
        e_osdsys = OSD_NTSC;
    else
        e_osdsys = OSD_PAL;
#endif

//#ifdef DUAL_VIDEO_OUTPUT
#if defined(DUAL_VIDEO_OUTPUT) || defined(DUAL_VIDEO_OUTPUT_USE_VCAP)
    osd_clut_ycbcr_out clut_ycbcr_out = {0,0,0,0};
    if( (NTSC==tvsys)||  (NTSC_443==tvsys) ||(PAL_M==tvsys)||(PAL_N==tvsys)||(PAL_60==tvsys)||(PAL==tvsys))
    {
        clut_ycbcr_out.layer = 0; // set layer number
        clut_ycbcr_out.enable = 1;
        clut_ycbcr_out.no_temp_buf = 1;
        osddrv_io_ctl((HANDLE)g_osd_dev_local, OSD_IO_SET_YCBCR_OUTPUT, (UINT32)&clut_ycbcr_out);
        gf_ycbcr=TRUE;
    }

    if(    (gw_for_y==540)||(gw_for_y==480)    )
        {t_ret=osddrv_scale((HANDLE)g_osd_dev_local,OSD_SCALE_WITH_PARAM,(UINT32)icc_get_scale_param(tvsys));}
    else
    {
        t_ret=osddrv_scale((HANDLE)g_osd_dev_local,OSD_VSCALE_TTX_SUBT,(UINT32)(&e_osdsys));
        ICC_PRINTF("\nNeed to Add Scale with Unexpected Format XY (i.e Neithor 960x540 Nor 720x480 !!!!!!!");
    }

    #if 1
    if( (NTSC==tvsys)||  (NTSC_443==tvsys) ||(PAL_M==tvsys)||(PAL_N==tvsys)||(PAL_60==tvsys)||(PAL==tvsys))
    {
        t_ret=osddrv_scale((HANDLE)g_osd_dev_local,     OSD_SET_SCALE_MODE, OSD_SCALE_FILTER);
    }
    #endif

    if(RET_SUCCESS!=t_ret)    {ICC_PRINTF("Fail to Scale.\n");}
#endif
    t_ret=osddrv_show_on_off((HANDLE)g_osd_dev_local, OSDDRV_ON);
    ICC_PRINTF(",ENTER CC OSD ");
    if(RET_SUCCESS!=t_ret)    {ICC_PRINTF("Fai to Show On.\n");}
    gf_cc_show = TRUE;
    gf_cc_pre_show=FALSE;
LEAVE:
    return;
}
/********************************************
* NAME: cc_osd_leave
*       destroy cc osd and return menu osd
*
* Returns : void
* Parameter                     Flow    Description
* ------------------------------------------------------------------------------
* None
* ------------------------------------------------------------------------------
* Additional information:
******************************************/

void isdbtcc_osd_leave()
{
    enum tvsystem tvsys;
    gf_cc_enbale_show=FALSE;

    if(FALSE==gf_cc_show)
        goto LEAVE;

    while(gf_in_subt_decoding == TRUE)
        {
              os_task_sleep(1);
        }

    vpo_ioctl((struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS), VPO_IO_GET_OUT_MODE, (UINT32)(&tvsys));
    g_osd_dev_local = (struct osd_device *)dev_get_by_id(HLD_DEV_TYPE_OSD, g_isdbtcc_config_par.isdbtcc_osd_layer_id);
    if(NULL!=g_osd_dev_local)
    {
        if(TRUE==gf_ycbcr)
        {
            osd_clut_ycbcr_out clut_ycbcr_out = {0,0,0,0};
            clut_ycbcr_out.layer = 0; // set layer number
            clut_ycbcr_out.enable = 0;
            clut_ycbcr_out.no_temp_buf = 1;    //notice, be careful to restore original value before call isdbtcc_osd_enter()
            osddrv_io_ctl((HANDLE)g_osd_dev_local, OSD_IO_SET_YCBCR_OUTPUT, (UINT32)&clut_ycbcr_out);
            gf_ycbcr=FALSE;
        }
        osddrv_delete_region((HANDLE)g_osd_dev_local, gb_cur_region);
        osddrv_close((HANDLE)g_osd_dev_local);
        ICC_PRINTF(",Leave CC OSD ");
    }
    if(NULL!=g_vscr.lpb_scr)
        {FREE(g_vscr.lpb_scr);}
    gf_cc_show = FALSE;
LEAVE:
return;
}
/********************************************
* NAME: CC_DrawChar1
*       draw cc osd
*
* Returns : void
* Parameter                     Flow    Description
* ------------------------------------------------------------------------------
* UINT16 x                    In    pos_x of cc char    (input range as 0~31)
* UINT16 y                    In    pos_y of cc char    (input range as 0~14)
* UINT8 fg_color                In    fg color of cc char
* UINT8 bg_color                In    bg color of cc char
* UINT16 character                In    cc char
* ------------------------------------------------------------------------------
* Additional information:
******************************************/
void icc_draw_char1(UINT16 x, UINT16 y, UINT8 fg_color, UINT8 bg_color,UINT16 character)
{
    //Target
    struct osdrect r;                // rectangle info  to display
    UINT8* p_data;
    //Temp
    UINT8 i=0, j=0;
    UINT16 w_add=0;
    UINT8 data1=0;                //temp data to store data as fg or bg color
    #define CHARMASK_BITORDER        5
    #define CHARMASK_BITS            32
    UINT32 charmask;            // 32bits, one by one bit check
    UINT16 bit_count;            // sum bits check num
    const UINT32*  char_addr=NULL;        // font data start points
    //Font Info
    UINT8 cc_actual_width=0;
    UINT8 cc_col=0;
    //========
    UINT16 wy,wx;


	UINT8 bwidth=CC_FONT_W;//gw_char_x+gw_space_x;
	UINT8 bfonthight=CC_FONT_H;//gw_char_y;
	UINT8 bheight=gw_area_y/MAX_CC_ROWS;//gw_char_y+gw_space_y;
    UINT8 bxend=gbx2;
    UINT8 byend=gby2;
    UINT16 w_left=gw_disp_x;
    UINT16 w_top=gw_disp_y;

	if((0xf<bg_color)||(0xf<fg_color))
	{
		return;
	}
    if(TRUE==gf_cc_enbale_show)
    {
        if(TRUE!=gf_cc_show)
        {
            if(TRUE!=gf_cc_pre_show)
            {
                isdbtcc_osd_enter();
            }
            else
            {
                goto LEAVE;
            }
        }
    }
    else
    {
        goto LEAVE;
    }

ENTER_SUBT_OSD_API();
//Correct para if out of range
    y = (y>byend)?byend:y;
    x = (x>bxend)?bxend:x;

    wy=y*bheight+w_top;
    wx=x*bwidth+w_left;

//Invoke corresponging font data
    for(i=0;i<CC_FONT_NUM;i++)
    {
        if(CC_FPOR[i]==character)     {j=i;break;}
    }
    if(CC_FONT_NUM==i)
    {
        ICC_PRINTF("No Found Font data(%x)\n",character);
        LEAVE_SUBT_OSD_API();
        icc_fill_char(x, y, 1, 1,bg_color);
        return;
    }

    for(i=0;i<=j;i++)
    {
        cc_actual_width=cc_fpor_width[i];
        cc_col=((UINT16)cc_actual_width*bfonthight)>>CHARMASK_BITORDER;
        if(((UINT16)cc_actual_width*bfonthight)%CHARMASK_BITS)
            cc_col++;
        if(i==j)
            break;
        w_add+=cc_col;
    }
    char_addr=&cc_fpor_data[w_add];

    p_data = g_vscr.lpb_scr+(UINT32)g_vscr.v_r.u_width*wy+wx;
    for(i=0;i<bheight;i++)
    {
        MEMSET(p_data+g_vscr.v_r.u_width*i,bg_color,bwidth);
    }
    if(character < 0x20)
        {goto LEAVE ;}

    r.u_left = wx;
    r.u_top = wy;
    r.u_width = bwidth;
    r.u_height = bheight;

    p_data = g_vscr.lpb_scr+(UINT32)g_vscr.v_r.u_width*wy+wx;
    bit_count = 0;

    //vertical normal , horizine normal
         for(i = 0 ;i < cc_col;i++)
         {
             charmask = 0x80000000;
        for(j = 0 ;j < 32 ;j++)
        {
            if(bit_count >= (UINT16)cc_actual_width * bfonthight)    //char draw finished
                break;

            if( (bit_count!=0) && (bit_count %cc_actual_width ==0))    //one row finished, change to next
                {p_data += (g_vscr.v_r.u_width-cc_actual_width);}

            if(*char_addr & charmask)
                {data1 = fg_color;    }
            else                
                {data1 = bg_color;    }

            *p_data = data1;
            p_data++;
            bit_count++;
            charmask >>=1;
        }
        char_addr ++;
         }
    // write Vscr data to osd frame buffer
    #if (DBG_CS==1)
    osddrv_region_write((HANDLE)g_osd_dev_local, gb_cur_region, &g_vscr, &r);
    #endif
LEAVE:
    LEAVE_SUBT_OSD_API();
}

/********************************************
* NAME: CC_FillChar
*       fill a rectanger with one color
*
* Returns : void
* Parameter                     Flow    Description
* ------------------------------------------------------------------------------
* UINT16 x                    In    start pos_x of char
* UINT16 y                    In    start pos_y of char
* UINT16 w                    In    width (number of cc char)
* UINT16 h                    In    high (number of cc char)
* UINT8 bg_color                In    bg color
* ------------------------------------------------------------------------------
* Additional information:
******************************************/
void icc_fill_char(UINT16 x, UINT16 y,UINT16 w,UINT16 h,UINT8 bg_color)
{
    UINT32 i;
    struct osdrect r;
    UINT8* p_data;
	
	UINT8 bwidth=CC_FONT_W;//gw_char_x+gw_space_x;
	UINT8 bheight=gw_area_y/MAX_CC_ROWS;;//gw_char_y+gw_space_y;
    UINT8 bxend=gbx2;
    UINT8 byend=gby2;
    UINT16 w_left=gw_disp_x;
    UINT16 w_top=gw_disp_y;
    UINT16 wy,wx;
	if(0xf<bg_color)
	{
		return;
	}
    if(TRUE==gf_cc_enbale_show)
    {
        if(TRUE!=gf_cc_show)
        {
            if(TRUE!=gf_cc_pre_show)
            {
                isdbtcc_osd_enter();
            }
            else
            {
                goto LEAVE;
            }
        }
    }
    else
    {
        goto LEAVE;
    }

    ENTER_SUBT_OSD_API();

    x = (x>bxend)?0:x;
    y = (y>byend)?byend:y;
    w = (w+x)>(bxend+1) ? ((bxend+1) -x) : w;
    h = (h+y)>(byend+1) ? ((byend+1) -y) : h;

    wy=y*bheight+w_top;
    wx=x*bwidth+w_left;

    r.u_left = wx;
    r.u_top = wy ;
    r.u_width = w*bwidth;
    r.u_height = h*bheight;

    p_data = g_vscr.lpb_scr+g_vscr.v_r.u_width*wy +wx;
    for(i=0;i<h*bheight;i++)
    {
        MEMSET(p_data+g_vscr.v_r.u_width*i,bg_color,w*bwidth);
    }

    #if (DBG_CS==1)
    osddrv_region_fill((HANDLE)g_osd_dev_local, gb_cur_region, &r,bg_color);
    #endif

    LEAVE:
    LEAVE_SUBT_OSD_API();
}

 void icc_clear_screen(UINT16 w, UINT16 h,UINT8 bg_color)
{
    UINT32 i;
    struct osdrect r;
    UINT8* p_data;
	
	UINT8 bwidth=CC_FONT_W;//gw_char_x+gw_space_x;
	UINT8 bheight=gw_area_y/MAX_CC_ROWS;;//gw_char_y+gw_space_y;
    UINT16 w_left=gw_disp_x;
    UINT16 w_top=gw_disp_y;
	if((w>MAX_CC_COLS)||(h>MAX_CC_ROWS)||(0x8!=bg_color))
	{
		return;
	}
    if(TRUE==gf_cc_enbale_show)
    {
        if(TRUE!=gf_cc_show)
        {
            if(TRUE!=gf_cc_pre_show)
            {
                isdbtcc_osd_enter();
            }
            else
            {
                goto LEAVE;
            }
        }
    }
    else
    {
        goto LEAVE;
    }

    ENTER_SUBT_OSD_API();
    p_data = g_vscr.lpb_scr+g_vscr.v_r.u_width*w_top +w_left;
    for(i=0;i<h*bheight;i++)
    {
        MEMSET(p_data+g_vscr.v_r.u_width*i,bg_color,w*bwidth);
    }

    #if (DBG_CS==1)
    r.u_left = w_left;
    r.u_top = w_top ;
    r.u_width = w*bwidth;
    r.u_height = h*bheight;
    osddrv_region_fill((HANDLE)g_osd_dev_local, gb_cur_region, &r,bg_color);
    #endif

    LEAVE:
    LEAVE_SUBT_OSD_API();
}

 void icc_update_screen(void)
 {
     #if (DBG_CS==1)
        return;
    #elif (DBG_CS==2)
    struct osdrect r;
    r.u_left = 0;
    r.u_top = 0;
    r.u_width = gw_for_x;
    r.u_height = gw_for_y;
    osddrv_region_write((HANDLE)g_osd_dev_local, gb_cur_region, &g_vscr, &r);
    #endif
 }
