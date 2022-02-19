#include <sys_config.h>
#include <osal/osal.h>
#include <basic_types.h>
#include <mediatypes.h>

#include <api/libc/printf.h>
#include <api/libc/alloc.h>
#include <api/libc/string.h>

#include <hld/hld_dev.h>
#include <hld/dis/vpo.h>

#include <hld/isdbtcc/isdbtcc_dec.h>

#include <math.h>

/////////////////////
#include "isdbtcc_dec_buffer.h"
#include <api/libisdbtcc/lib_isdbtcc.h>
///////////////////////////////////////////////////////////////
    #define CC_US        0x1F    //dat unit separator code
    #define CC_I1        0x3B
    #define CC_IE        0x20
BOOL f_control,f_got_bgcolor;
enum GROUP_ID
{
    CC_GP_MAN_A=0x00,
    CC_GP_STA_A1=0x01,
    CC_GP_STA_A2,
    CC_GP_STA_A3,
    CC_GP_STA_A4,

    CC_GP_MAN_B=0x20,
    CC_GP_STA_B1=0x21,
    CC_GP_STA_B2,
    CC_GP_STA_B3,
    CC_GP_STA_B4
};
enum TIME_CONTROL_MODE
{
    CC_TMD_FREE=0x00,
    CC_TMD_REAL_TIME,
    CC_TMD_OFFSET_TIME,
    CC_TMD_RES,
};

enum DISPLAY_MODE
{
    CC_DF_H_STAND=0x00,
    CC_DF_V_STAND=0x01,
    CC_DF_H_HIGH=0x02,
    CC_DF_V_HIGH=0x03,
    CC_DF_H_WESTERN_LANG=0x04,
    CC_DF_H_1920X1080=0x06,
    cc_df_v_1920x1080=0x07,
    CC_DF_H_960X540=0x08,
    CC_DF_V_960X540=0x09,
    CC_DF_H_1280X720=0x0C,
    CC_DF_V_1280X720=0x0D,
    CC_DF_H_720X480=0x0A,
    CC_DF_V_720X480=0x0B
};

enum CSI_DISPLAY_MODE
{
    CC_CSI_H_STAND=0x00,
// Editing
    CC_CSI_V_STAND=0x01,
    CC_CSI_H_HIGH=0x02,
    CC_CSI_V_HIGH=0x03,
    CC_CSI_H_WESTERN_LANG=0x04,
    CC_CSI_H_1920X1080=0x05,
    cc_csi_v_1920x1080=0x06,
    CC_CSI_H_960X540=0x07,
    CC_CSI_V_960X540=0x08,
    CC_CSI_H_720X480=0x09,
    CC_CSI_V_720X480=0x0A,
    CC_CSI_H_1280X720=0x0B,
    CC_CSI_V_1280X720=0x0C
};

enum CHAR_CODING
{
    CC_TCS_8BIT=0x00,
    CC_TCS_UCS=0x01,
    CC_TCS_RES1=0x02,
    CC_TCS_RES2=0x03
};

enum ROLL_UP_MODE
{
    CC_ROLL_NO=0x00,
    CC_ROLL_UP=0x01,
    CC_ROLL_RES1=0x02,
    CC_ROLL_RES2=0x03
};
enum DISPLAY_FORMAT
{
    CC_DMF_AUTO_WHEN_REC=0x0C,
    CC_DMF_NO_WHEN_REC=0x0D,
    CC_DMF_SEL_WHEN_REC=0x0E
};

enum DATA_UNIT_TYPE
{
    CC_TYPE_BODY=0x20,
    CC_TYPE_GEOM=0x28,
    CC_TYPE_SOUND=0x2C,
    CC_TYPE_1BDRCS=0x30,
    CC_TYPE_2BDRCS=0x31,
    CC_TYPE_COLOR_MAP=0x34,
    CC_TYPE_BITMAP=0x35
};

enum CC_CMLA
{
    CC_CMLA_BLACK=0,        //0x00,
    CC_CMLA_RED,            //0x01,
    CC_CMLA_GREEN,        //0x02,
    CC_CMLA_YELLOW,        //0x03,
    CC_CMLA_BLUE,            //0x04,
    CC_CMLA_MAGENTA,        //0x05,
    CC_CMLA_CYAN,            //0x06,
    CC_CMLA_WHITE,        //0x07,

    CC_CMLA_TRANSPARENT,    //0x08,

    CC_CMLA_HALF_RED,            //0x09,
    CC_CMLA_HALF_GREEN,        //0x0A,
    CC_CMLA_HALF_YELLOW,        //0x0B,
    CC_CMLA_HALF_BLUE,        //0x0C,
    CC_CMLA_HALF_MAGENTA,    //0x0D,
    CC_CMLA_HALF_CYAN,        //0x0E,
    CC_CMLA_HALF_WHITE,        //0x0F,
};

#define CC_INIT_C0    0x00
#define CC_END_C0    0x20
#define CC_INIT_C1    0x80
#define CC_END_C1    0x9F
#define CC_INIT_GL    0x21
#define CC_END_GL    0x7E
#define CC_INIT_GR    0xA1
#define CC_END_GR    0xFE

enum CMD_SEQ
{
    CC_CMD_C0=0,
    CC_CMD_C1=1,
    CC_CMD_CSI=2,
    CC_CMD_G3=3,
    CC_CMD_GR=4
};
#define CC_CMD_DBINIT    CC_CMD_C0
#define CC_CMD_DBEND    CC_CMD_GR

enum CHAR_C0
{
    CC_C0_NULL=0x00,
    CC_C0_BEL=0x07,
    CC_C0_APB=0x08,
    CC_C0_APF=0x09,
    CC_C0_APD=0x0A,
    CC_C0_APU=0x0B,
    CC_C0_CS=0x0C,
    CC_C0_APR=0x0D,
    CC_C0_LS1=0x0E,
    CC_C0_LS0=0x0F,
    CC_C0_PAPF=0x16,
    CC_C0_CAN=0x18,
    CC_C0_SS2=0x19,
    CC_C0_ESC=0x1B,
    CC_C0_APS=0x1C,
    CC_C0_SS3=0x1D,
    CC_C0_RS=0x1E,
    CC_C0_US=0x1F,
    CC_C0_SP=0x20
};

enum CHAR_C1
{
    CC_C1_BKF=0x80,
    CC_C1_RDF=0x81,
    CC_C1_GRF=0x82,
    CC_C1_YLF=0x83,
    CC_C1_BLF=0x84,
    CC_C1_MGF=0x85,
    CC_C1_CNF=0x86,
    CC_C1_WHF=0x87,
    CC_C1_SSZ=0x88,
    CC_C1_MSZ=0x89,
    CC_C1_NSZ=0x8A,
    CC_C1_SZX=0x8B,

    CC_C1_COL=0x90,
    CC_C1_FLC=0x91,
    CC_C1_CDC=0x92,
    CC_C1_POL=0x93,
    CC_C1_WMM=0x94,
    CC_C1_MACRO=0x95,

    CC_C1_HLC=0x97,
    CC_C1_RPC=0x98,
    CC_C1_SPL=0x99,
    CC_C1_STL=0x9A,
    CC_C1_CSI=0x9B,
    CC_C1_TIME=0x9D
};

enum CHAR_CSI
{
    CC_CSI_GSM=0x42,

    CC_CSI_SWF=0x53,
    CC_CSI_CCC=0x54,

    CC_CSI_SDF=0x56,
    CC_CSI_SSM=0x57,
    CC_CSI_SHS=0x58,
    CC_CSI_SVS=0x59,

    CC_CSI_PLD=0x5B,
    CC_CSI_PLU=0x5C,
    CC_CSI_GAA=0x5D,
    CC_CSI_SRC=0x5E,
    CC_CSI_SDP=0x5F,

    CC_CSI_ACPS=0x61,
    CC_CSI_TCC=0x62,
    CC_CSI_ORN=0x63,
    CC_CSI_MDF=0x64,
    CC_CSI_CFS=0x65,
    CC_CSI_XCS=0x66,

    CC_CSI_PRA=0x68,
    CC_CSI_ACS=0x69,

    CC_CSI_RCS=0x6E,
    CC_CSI_SCS=0x6F
};

enum CC_CHAR_G3
{
    CC_G3_NOTE=0x21,
    CC_G3_1DIV4=0x35,
    CC_G3_1DIV2=0x36,
    CC_G3_3DIV4=0x37,

    CC_G3_TM=0x47,
    CC_G3_1DIV8=0x48,
    CC_G3_3DIV8=0x49,
    CC_G3_5DIV8=0x4A,
    CC_G3_7DIV8=0x4B

};

enum CC_FONT_SIZE
{
    CC_FONT_SMALL=0x00,
    CC_FONT_MID=0x01,
    CC_FONT_NOR=0x02
};

struct st_cc_grp
{
    UINT8 b_id;
    UINT32 dw_size;
    UINT8 b_tmd;
    UINT32 dw_pts;
    void* p_grp;
};

struct st_cc_mang
{
    UINT8 b_num;
    UINT8 b_tag;
    UINT8 b_dmf;
    UINT8 b_lan[3];
    UINT8 b_format;
    UINT8 b_tcs;
    UINT8 b_rool_up;
    UINT32 dw_len;
};

struct st_cc_sta
{
    UINT32 dw_len;
    UINT8 b_type;
    UINT32 dw_size;
};
struct st_cc_mang mang1;
struct st_cc_sta sta1;
UINT8 b_grp_data[502];
//////////////////////////////////////////////////////////////
extern UINT32 stc_delay;

struct ccdec_data_hdr *ccdec_bs_hdr_buf;

struct ccdec_data_hdr p_ccdec_init_hdr;
struct ccdec_data_hdr *p_ccdec_wr_hdr;
struct ccdec_data_hdr *p_ccdec_rd_hdr;

UINT8 ccidentifier_detected;
UINT8 ccdec_stuffing_flag;
UINT32    u_ccdec_remain_size;
UINT8*    pu_ccdec_start_point ;

BOOL gcc_task_open=FALSE;
BOOL gcc_pktremain=FALSE;
static BOOL isdbtcc_dec_stopped = FALSE;//get sdec status
////////
static char cb_ccdec_name[HLD_MAX_NAME_SIZE] = "ISDBT_CC_0";

OSAL_ID gccdec_tsk_id, gccdec_flag_id;
////////////////////////////
UINT8  isdbtcc_dec_tsk_qtm;
UINT8  isdbtcc_dec_tsk_pri;
UINT8  isdbtcc_transparent_color;
static UINT32 gdw_ccsize=0;
static UINT32 gdw_ccleft_size=0;
static UINT32    dw_remain_cnt=0;
static struct st_cc_grp grp1;

#define CRC16_SIZE    2
#define PES_HEAD_SIZE    3
#define DATA_GRP_SIZE    5
#if 0
UINT8 *isdbtcc_dec_temp_buf;
UINT32 isdbtcc_dec_temp_buf_len;
#endif

extern UINT32 isdbtcc_dec_bs_hdr_buf_len;
extern UINT8* isdbtcc_dec_bs_buf;
extern UINT8* isdbtcc_dec_bs_end;
extern UINT32 isdbtcc_dec_bs_buf_len;
extern UINT8     isdbtcc_transparent_color;

extern UINT8  isdbtcc_dec_tsk_qtm;
extern UINT8  isdbtcc_dec_tsk_pri;

UINT16 gw_for_x=0;
UINT16 gw_for_y=0;
UINT16 gw_area_x=0;
UINT16 gw_area_y=0;
UINT16 gw_disp_x=0;
UINT16 gw_disp_y=0;
UINT16 gw_char_x=0;
UINT16 gw_char_y=0;
UINT16 gw_space_x=0;
UINT16 gw_space_y=0;

UINT8 gbx2=0;    //Max X
UINT8 gby2=0;        //Max Y
UINT8 gbx=0;        //Active X
UINT8 gby=0;        //Active Y
UINT8 gb_fcol=0;    //Foreground Color
UINT8 gb_bcol=0;    //Background Color
UINT8 gb_rcol=0;    //Raster Color
extern void icc_draw_char1(UINT16 x, UINT16 y, UINT8 fg_color, UINT8 bg_color,UINT16 character);
extern void icc_fill_char(UINT16 x, UINT16 y,UINT16 w,UINT16 h,UINT8 bg_color);
extern void icc_clear_screen(UINT16 w, UINT16 h,UINT8 bg_color);
int parse_mang(struct st_cc_grp *grp, UINT8 *pbcnt,UINT8 *gpbpkt);

extern void icc_update_screen(void);
struct isdbtcc_priv * gp_ccdec_priv;


void font_size_change(void)
{
   UINT16 char_y;
   if((gw_char_y != 18) && (gw_char_y > 0)) //Font only supports 18bit height
   {
        char_y = gw_char_y;
        gw_char_x = gw_char_x * 18 / char_y;
        gw_space_x = gw_space_x * 18 / char_y;
        gw_char_y = 18;//gwCharY * 18 / charY;
        gw_space_y = gw_space_y * 18 / char_y;
        gbx2=gw_area_x/(gw_char_x+gw_space_x);
        if(gbx2) {gbx2--;}
            gby2=gw_area_y/(gw_char_y+gw_space_y);
        if(gby2) {gby2--;}
   }
}


//local func
void isdbtcc_sw_init(void)
{
    p_ccdec_init_hdr.u_data_size = 0;
    p_ccdec_rd_hdr = &p_ccdec_init_hdr;
    p_ccdec_wr_hdr = &p_ccdec_init_hdr;
    isdbtcc_dec_sbf_create();
    isdbtcc_dec_hdr_buf_create();
}
//End of local func
/////////////////////////////
// param1 as the point of sdec_device
// param2 is not used
void isdbtcc_parsing_task(UINT32 param1,__MAYBE_UNUSED__ UINT32 param2)
{
    OSAL_ER result=0;
    UINT32 flgptn=0 ;
    //UINT32 u_size;
    //UINT8 * pu_data;

//    #define WATCH_TIME
	param2=0;

#ifdef WATCH_TIME
    static UINT32 dw_cur_tick=0,dw_last_tick=0;
#endif
    struct isdbtcc_priv* priv =(struct isdbtcc_priv*)(((struct sdec_device*)param1)->priv);
    while(1) // run forever when task exist.
    {
#ifdef WATCH_TIME
        dw_cur_tick=osal_get_tick()/1000;
        if(dw_last_tick!=dw_cur_tick)
        {
            dw_last_tick=dw_cur_tick;
            ICC_PRINTF("ICCTick=%d\n",dw_last_tick);
        }
#endif

        if(priv->status == ISDBTCC_DEC_STATE_IDLE)
        {
            result = osal_flag_wait(&flgptn,gccdec_flag_id, ISDBTCC_DEC_CMD_START/*|ISDBTCC_DEC_CMD_SHOWON|ISDBTCC_DEC_CMD_SHOWOFF*/, OSAL_TWF_ORW, OSAL_WAIT_FOREVER_TIME);
            if(flgptn&ISDBTCC_DEC_CMD_START)
            {
                osal_flag_clear(gccdec_flag_id, ISDBTCC_DEC_CMD_START);
                isdbtcc_sw_init();
                priv->status = ISDBTCC_DEC_STATE_PLAY;
                osal_flag_set(gccdec_flag_id,ISDBTCC_DEC_MSG_START_OK);

            }
        }
        else if(priv->status == ISDBTCC_DEC_STATE_PLAY)
        {
            result = osal_flag_wait(&flgptn,gccdec_flag_id, ISDBTCC_DEC_CMD_STOP|/*ISDBTCC_DEC_CMD_SHOWON|ISDBTCC_DEC_CMD_SHOWOFF|*/ISDBTCC_DEC_MSG_DATA_AVAILABLE, OSAL_TWF_ORW, 100);

            if(OSAL_E_TIMEOUT==result)
            {
                //ICC_PRINTF("isdbtcc task running!\n");
                if(priv->stream_parse_cb)
                {
                    priv->stream_parse_cb();
                    osal_task_sleep(10);
                }
                     continue;
            }
            if(flgptn&ISDBTCC_DEC_CMD_STOP)
            {
                osal_flag_clear(gccdec_flag_id, ISDBTCC_DEC_CMD_STOP|ISDBTCC_DEC_CMD_START|ISDBTCC_DEC_MSG_DATA_AVAILABLE);
                osal_flag_set(gccdec_flag_id,ISDBTCC_DEC_MSG_STOP_OK);
                priv->status = ISDBTCC_DEC_STATE_IDLE;
                continue;
            }

            if(flgptn&ISDBTCC_DEC_MSG_DATA_AVAILABLE)
            {
                osal_flag_clear(gccdec_flag_id, ISDBTCC_DEC_MSG_DATA_AVAILABLE);
                if(priv->stream_parse_cb)
                {
                    priv->stream_parse_cb();
                    osal_task_sleep(10);
                }
            }
        }
    }//end main while loop
}

BOOL get_isdbtcc_dec_status()
{
    return FALSE;
}

//#define DBG_DECFLOW
//Start of isdbtcc_dec.c
INT32 isdbtcc_dec_open(struct sdec_device *dev)
{
    OSAL_T_CTSK t_ctsk;
	if(NULL==dev)
	{
		return RET_FAILURE;
	}
    struct isdbtcc_priv * priv = (struct isdbtcc_priv*)(dev->priv) ;
#ifdef     DBG_DECFLOW
    ICC_PRINTF("\nICC OPEN-start");
#endif
    priv->status = ISDBTCC_DEC_STATE_IDLE;
    gccdec_flag_id=osal_flag_create(0x00000000);
    if(OSAL_INVALID_ID==gccdec_flag_id)
    {
      //error handle?
    }
    t_ctsk.task = (FP)isdbtcc_parsing_task;
    t_ctsk.para1 = (UINT32)dev;
    t_ctsk.para2 = 0;
    t_ctsk.stksz = 0x2800;//5K //0x1400;    //0x0F00;//5k
    t_ctsk.quantum = 4;
    t_ctsk.itskpri = OSAL_PRI_NORMAL;
#ifdef _TDS_INFO_MONITOR
    t_ctsk.name[0] = 'I';
    t_ctsk.name[1] = 'C';
    t_ctsk.name[2] = 'C';
#endif
    gccdec_tsk_id = osal_task_create(&t_ctsk);
    if(OSAL_INVALID_ID==gccdec_tsk_id)
    {/* error handle?*/    }
    else
    {    gcc_task_open=TRUE;    }

#ifdef     DBG_DECFLOW
    ICC_PRINTF(",end");
#endif
    return RET_SUCCESS;
}

INT32  isdbtcc_dec_close(__MAYBE_UNUSED__ struct sdec_device *dev)
{
    //struct isdbtcc_priv * priv = (struct isdbtcc_priv*)(dev->priv) ;
#ifdef     DBG_DECFLOW
    ICC_PRINTF("\nICC CLOSE-start");
#endif

	dev=NULL;

    gcc_task_open=FALSE;
    if(OSAL_E_OK!=osal_task_delete(gccdec_tsk_id))
    {
        // error handle?
    }
    if(OSAL_E_OK!=osal_flag_delete(gccdec_flag_id))
    {
        // error handle?
    }
#ifdef     DBG_DECFLOW
    ICC_PRINTF(",end");
#endif
    return RET_SUCCESS;
}
INT32  isdbtcc_dec_start(struct sdec_device *dev, __MAYBE_UNUSED__ UINT16 w1,\
    __MAYBE_UNUSED__ UINT16 w2)
{
	if(NULL==dev)
	{
		return RET_FAILURE;
	}
	w1=0;
	w2=0;
    struct isdbtcc_priv * priv = (struct isdbtcc_priv*)(dev->priv) ;
    UINT32 flgptn ;
    OSAL_ER result;
#ifdef     DBG_DECFLOW
    ICC_PRINTF("\nICC START-start");
#endif
    if(priv->status == ISDBTCC_DEC_STATE_IDLE)
    {
        osal_flag_set(gccdec_flag_id, ISDBTCC_DEC_CMD_START);
        flgptn = 0;
        result = osal_flag_wait(&flgptn,gccdec_flag_id, ISDBTCC_DEC_MSG_START_OK, OSAL_TWF_ORW, OSAL_WAIT_FOREVER_TIME);
        if(OSAL_E_OK!=result)
        {
            return !RET_SUCCESS;
        }
        osal_flag_clear(gccdec_flag_id, ISDBTCC_DEC_MSG_START_OK);
    }
    isdbtcc_dec_stopped = FALSE;
#ifdef     DBG_DECFLOW
    ICC_PRINTF(",end");
#endif
    return RET_SUCCESS;
}
INT32  isdbtcc_dec_stop(struct sdec_device *dev)
{
	if(NULL==dev)
	{
		return RET_FAILURE;
	}
      struct isdbtcc_priv * priv = (struct isdbtcc_priv*)(dev->priv) ;
      UINT32 flgptn ;
      OSAL_ER result;
#ifdef     DBG_DECFLOW
    ICC_PRINTF("\nICC STOP-start");
#endif
    if(priv->status == ISDBTCC_DEC_STATE_PLAY)
    {
        osal_flag_set(gccdec_flag_id, ISDBTCC_DEC_CMD_STOP);
        flgptn = 0;
        result = osal_flag_wait(&flgptn,gccdec_flag_id, ISDBTCC_DEC_MSG_STOP_OK, OSAL_TWF_ORW, OSAL_WAIT_FOREVER_TIME);
        if(OSAL_E_OK!=result)
        {
                 //SDEC_PRINTF("Wait SDEC_MSG_START_OK failed with err code %s\n",
                 //result == OSAL_E_TIMEOUT?"OSAL_E_TIMEOUT":"OSAL_E_FAIL");
                 return !RET_SUCCESS;
        }
        osal_flag_clear(gccdec_flag_id, ISDBTCC_DEC_MSG_STOP_OK);

    }
#ifdef     DBG_DECFLOW
    ICC_PRINTF(",end");
#endif
    isdbtcc_dec_stopped = TRUE;
       return RET_SUCCESS;
}
INT32  isdbtcc_dec_pause(struct sdec_device *dev)
{
	if(NULL==dev)
	{
		return RET_FAILURE;
	}
      struct isdbtcc_priv * priv = (struct isdbtcc_priv*)(dev->priv) ;
      UINT32 flgptn ;
      OSAL_ER result;
#ifdef     DBG_DECFLOW
    ICC_PRINTF("\nICC PAUSE-start");
#endif
    if(priv->status == ISDBTCC_DEC_STATE_PLAY)
    {
        osal_flag_set(gccdec_flag_id, ISDBTCC_DEC_CMD_PAUSE);
        flgptn = 0;
        result = osal_flag_wait(&flgptn,gccdec_flag_id, ISDBTCC_DEC_MSG_PAUSE_OK, OSAL_TWF_ORW, OSAL_WAIT_FOREVER_TIME);
        if(OSAL_E_OK!=result)
        {
                 return !RET_SUCCESS;
        }
        osal_flag_clear(gccdec_flag_id, ISDBTCC_DEC_MSG_PAUSE_OK);
    }
#ifdef     DBG_DECFLOW
    ICC_PRINTF(",end");
#endif
       return RET_SUCCESS;
}
/*
INT32  isdbtcc_dec_showonoff(struct sdec_device *dev,BOOL bOn)
{
      struct isdbtcc_priv * priv = (struct isdbtcc_priv*)(dev->priv) ;
      UINT32 flgptn ;
      OSAL_ER result;

    if(bOn == TRUE)
    {
        osal_flag_set(gccdec_flag_id, ISDBTCC_DEC_CMD_SHOWON);
        flgptn = 0;
        result = osal_flag_wait(&flgptn,gccdec_flag_id, ISDBTCC_DEC_MSG_SHOWON_OK, OSAL_TWF_ORW, OSAL_WAIT_FOREVER_TIME);
        if(OSAL_E_OK!=result)
        {
                 return !RET_SUCCESS;
        }
        osal_flag_clear(gccdec_flag_id, ISDBTCC_DEC_MSG_SHOWON_OK);
    }
    else
    {
        osal_flag_set(gccdec_flag_id, ISDBTCC_DEC_CMD_SHOWOFF);
        flgptn = 0;
        result = osal_flag_wait(&flgptn,gccdec_flag_id, ISDBTCC_DEC_MSG_SHOWOFF_OK, OSAL_TWF_ORW, OSAL_WAIT_FOREVER_TIME);
        if(OSAL_E_OK!=result)
        {
                 return !RET_SUCCESS;
        }
        osal_flag_clear(gccdec_flag_id, ISDBTCC_DEC_MSG_SHOWOFF_OK);
    }
       return RET_SUCCESS;
}
*/
///////////////////////////////
extern BOOL g_is_hdvideo;
INT32 isdbtcc_dec_request_write(
    __MAYBE_UNUSED__ struct sdec_device *dev,
    UINT32 u_size_requested,
    struct control_block* p_tdata_ctrl_blk,
    UINT8** ppu_data,
    UINT32* pu_size_got)
{
    UINT32 u_req_size;
    UINT32 u_hdr_ptr;
    static UINT8 last_stcid=0xff;
    static UINT32 last_pts = 0;
	
	dev=NULL;
	
	if(NULL==pu_size_got)
	{
		return RET_FAILURE;
	}
    //struct isdbtcc_priv * priv = (struct isdbtcc_priv*)(dev->priv) ;
    u_size_requested= (u_size_requested>PKT_SIZE)?PKT_SIZE:(u_size_requested);
    u_req_size= u_size_requested;
    if(RET_FAILURE==isdbtcc_dec_sbf_wr_req(&u_req_size, ppu_data))
    {
        ICC_PRINTF("No BS buf!\n");
        #if 1
        static struct sdec_device* tempdev;
        tempdev=(struct sdec_device*)dev_get_by_name("ISDBT_CC_0");
        /* by saicheong for secure coding, BD-PB-NP-1 */
        if (NULL == tempdev)
        {
            return RET_FAILURE;
        }

        isdbtcc_dec_stop(tempdev);
        isdbtcc_dec_start(tempdev,0,0);
        #endif
        return RET_FAILURE;
    }
    if(RET_FAILURE==isdbtcc_dec_hdr_buf_wr_req(&u_hdr_ptr))
    {
        //error handle
        ICC_PRINTF("No Hdr buf!\n");
        #if 1
        static struct sdec_device* tempdev;
        tempdev=(struct sdec_device*)dev_get_by_name("ISDBT_CC_0");
        /* by saicheong for secure coding, BD-PB-NP-1 */
        if (NULL == tempdev)
        {
            return RET_FAILURE;
        }
        
        isdbtcc_dec_stop(tempdev);
        isdbtcc_dec_start(tempdev,0,0);
        #endif
        return RET_FAILURE;
    }
    p_ccdec_wr_hdr = &ccdec_bs_hdr_buf[u_hdr_ptr];
    p_ccdec_wr_hdr->pu_start_point = *ppu_data;
    p_ccdec_wr_hdr->u_data_size = 0;

    if (NULL == p_tdata_ctrl_blk)
        p_ccdec_wr_hdr->b_info_valid = FALSE;
    else
    {
        p_ccdec_wr_hdr->b_info_valid = TRUE;
        if(p_tdata_ctrl_blk->data_continue == 0x00)
        {
            p_ccdec_wr_hdr->u_pes_start =0x01;
        }
        else
        {
            p_ccdec_wr_hdr->u_pes_start =0x00;
        }
        if(1==p_tdata_ctrl_blk->pts_valid)
        {
            last_stcid = p_tdata_ctrl_blk->stc_id;
            last_pts = p_tdata_ctrl_blk->pts;
        }
        else
        {
            last_pts=0;
        }
        p_ccdec_wr_hdr->u_stc_id = last_stcid;
        p_ccdec_wr_hdr->u_pts = last_pts;

        /* by saicheong for secure coding, BD-PB-NP-1 */
        p_tdata_ctrl_blk->instant_update = 1;
    }
    /* by saicheong for secure coding, BD-PB-NP-1, deleted */
    //p_tdata_ctrl_blk->instant_update = 1;
    *pu_size_got = (u_req_size>u_size_requested? u_size_requested : u_req_size);
    return RET_SUCCESS;
///////////////////////////////
}
void isdbtcc_dec_update_write(__MAYBE_UNUSED__ struct sdec_device *dev,UINT32 u_size)
{
    //UINT32 stc_msb32;
    //struct isdbtcc_priv * priv = (struct isdbtcc_priv*)(dev->priv) ;
	dev=NULL;
    if(NULL==p_ccdec_wr_hdr)
        return;
    if(!u_size)
        return ;
    isdbtcc_dec_sbf_wr_update(u_size);
    p_ccdec_wr_hdr->u_data_size += u_size;
    isdbtcc_dec_hdr_buf_wr_update();
    osal_flag_set(gccdec_flag_id,ISDBTCC_DEC_MSG_DATA_AVAILABLE);
    return ;
}


__ATTRIBUTE_RAM_
// isdbtcc_dectask.c
#if 1
//get number from char
UINT32 cal_val(UINT8* pbtemp, UINT8 bnum)
{
    UINT32 dw_val=0;
    UINT8 bi;
	if((NULL!=pbtemp)&&((0<bnum)&&(bnum<0xff)))
	{
    	for(bi=0;bi<bnum;bi++)
    	{
        	if(bi)
            	dw_val<<=8;
        	dw_val+=(pbtemp[bi]);
    	}
    
	}
	return dw_val;
}

BOOL cc_find_stat(void)
{
    UINT8 b_id;
    UINT8* pu_data=p_ccdec_rd_hdr->pu_start_point;

    b_id=(pu_data[3])>>2;
    switch(b_id)
    {
        case CC_GP_STA_A1:
        case CC_GP_STA_A2:
        case CC_GP_STA_A3:
        case CC_GP_STA_A4:
        case CC_GP_STA_B1:
        case CC_GP_STA_B2:
        case CC_GP_STA_B3:
        case CC_GP_STA_B4:
            return TRUE;
    }
    return FALSE;
}

//Parse PES Data
UINT16 get_val(UINT16 w_val,UINT8 b_char)
{
	UINT16 value=0;
	if((0xffff!=w_val)&&(0xff!=b_char))
	{
    	value=(w_val*10)+(unsigned int)(b_char-'0');
	}
	return  value;
}

void get_setting(UINT8* b_dat ,UINT8 b_sets,UINT16* w_v1,UINT16* w_v2)
{
    UINT8 bi=0;
    UINT16 w1=0;
	if((NULL==b_dat)||(NULL==w_v1)||(NULL==w_v2)||(0xff<=b_sets))
	{
		return;
	}
    for(bi=1;bi<b_dat[0];bi++)
    {
        if((CC_I1==b_dat[bi]) ||(CC_IE==b_dat[bi]))
        {
            bi++;
            break;
        }
        w1=get_val(w1,b_dat[bi]);
    }
    *w_v1=w1;

    if(b_sets>=2)
    {
        w1=0;
        for(;bi<b_dat[0];bi++)
        {
            if((CC_I1==b_dat[bi]) ||(CC_IE==b_dat[bi]))
            {
                bi++;
                break;
            }
            w1=get_val(w1,b_dat[bi]);
        }
        *w_v2=w1;
    }
}
void get_setting_ext(UINT8* b_dat ,UINT8 b_sets,UINT16* w_v1)
{
    UINT8 bi=0;
    UINT16 w1=0;
	if((NULL==b_dat)||(NULL==w_v1)||(0xff<=b_sets))
	{
		return;
	}
    for(bi=1;bi<b_dat[0];bi++)
    {
        if((CC_I1==b_dat[bi]) ||(CC_IE==b_dat[bi]))
        {
            bi++;
            break;
        }
        w1=get_val(w1,b_dat[bi]);
    }
    *w_v1=w1;

    if(b_sets>=2)
    {
        w1=0;
        for(;bi<b_dat[0];bi++)
        {
            if((CC_I1==b_dat[bi]) ||(CC_IE==b_dat[bi]))
            {
                bi++;
                break;
            }
            w1=get_val(w1,b_dat[bi]);
        }
        
    }
}

void update_xy(UINT8 bcmd)
{
    switch(bcmd)
    {
        case CC_C0_APB:
            if(gbx>0)
			{    
				gbx--;
			}
            else            
			{    
				if(gby>0)    
				{
					gby--;
					gbx=gbx2;
				}    
			}
        break;
        case CC_C0_APF:
            if(gbx<gbx2)    
			{    
				gbx++;
			}
            else            
			{    
				if(gby<gby2)    
				{
					gbx++;
					gbx=0;
				}    
			}
        break;
        case CC_C0_APD:
            if(gby<gby2)    
			{    
				gby++;
			}
            else            
			{    
				gby=0;    
			}
        break;
        case CC_C0_APU:
            if(gby>0)    
			{    
				gby--;
			}
            else            
			{    
				gby=gby2;    
			}
        break;
        case CC_C0_APR:
            gbx=0;
            if(gby<gby2)    
			{
				gby++;
			}
        break;
    }
}

void update_col(UINT8 bval)
{
    switch(bval)
    {
        case 0x48: gb_fcol=CC_CMLA_TRANSPARENT;        
		break;
        case 0x49: gb_fcol=CC_CMLA_HALF_RED;             
		break;
        case 0x4A: gb_fcol=CC_CMLA_HALF_GREEN;         
		break;
        case 0x4B: gb_fcol=CC_CMLA_HALF_YELLOW;        
		break;
        case 0x4C: gb_fcol=CC_CMLA_HALF_BLUE;         
		break;
        case 0x4D: gb_fcol=CC_CMLA_HALF_MAGENTA;     
		break;
        case 0x4E: gb_fcol=CC_CMLA_HALF_CYAN;         
		break;
        case 0x4F: gb_fcol=CC_CMLA_HALF_WHITE;         
		break;

        case 0x50: gb_bcol=CC_CMLA_BLACK;    
		break;
        case 0x51: gb_bcol=CC_CMLA_RED;         
		break;
        case 0x52: gb_bcol=CC_CMLA_GREEN;     
		break;
        case 0x53: gb_bcol=CC_CMLA_YELLOW;    
		break;
        case 0x54: gb_bcol=CC_CMLA_BLUE;      
		break;
        case 0x55: gb_bcol=CC_CMLA_MAGENTA;    
		break;
        case 0x56: gb_bcol=CC_CMLA_CYAN;     
		break;
        case 0x57: gb_bcol=CC_CMLA_WHITE;     
		break;

        case 0x58: gb_bcol=CC_CMLA_TRANSPARENT;     
		break;

        case 0x59: gb_bcol=CC_CMLA_HALF_RED;         
		break;
        case 0x5A: gb_bcol=CC_CMLA_HALF_GREEN;     
		break;
        case 0x5B: gb_bcol=CC_CMLA_HALF_YELLOW;    
		break;
        case 0x5C: gb_bcol=CC_CMLA_HALF_BLUE;     
		break;
        case 0x5D: gb_bcol=CC_CMLA_HALF_MAGENTA;
		break;
        case 0x5E: gb_bcol=CC_CMLA_HALF_CYAN;     
		break;        
        case 0x5F: gb_bcol=CC_CMLA_HALF_WHITE;     
		break;
        default:
            ICC_PRINTF("\nOh! Oh! Special Col control (%x) via C1_COL ",bval);
            break;
    }
}

void dec_excsi(UINT16* pwcnt, UINT8 *gpbpkt )
{
    UINT8 bi,btemp;
    UINT8 b_arr[20];
    UINT16 wtemp;

	if((NULL==gpbpkt)||(NULL==pwcnt))
	{
		return ;
	}
    b_arr[0]=0;
    for(bi=1;bi<20;bi++)
    {
        (*pwcnt)++;
        b_arr[bi]=gpbpkt[*pwcnt];
        if(CC_IE==b_arr[bi])
        {
            b_arr[0]=bi+2;
            (*pwcnt)++;
            b_arr[bi+1]=gpbpkt[*pwcnt];
            break;
        }
    }
	/* by wen for cpptest BD-PB-ARRAY-1 */
    if(b_arr[0] == 0)
    	return;
    btemp=b_arr[ b_arr[0]-1 ];
    switch(btemp)
    {
        case CC_CSI_SWF:
            //get_setting(b_arr, 1, &wtemp,(UINT16*)NULL);
            get_setting_ext(b_arr, 1, &wtemp);
            if(wtemp==7)
            {
                gw_for_x=960;
                gw_for_y=540;
            }
            break;
        case CC_CSI_SDP:
            get_setting(b_arr, 2, &gw_disp_x, &gw_disp_y);
            break;
        case CC_CSI_SDF:
            get_setting(b_arr, 2, &gw_area_x, &gw_area_y);
            break;
        case CC_CSI_SSM:
            get_setting(b_arr, 2, &gw_char_x, &gw_char_y);
            break;
        case CC_CSI_SHS:
            //get_setting(b_arr, 1, &gw_space_x,(UINT16*)NULL);
            get_setting_ext(b_arr, 1, &gw_space_x);
            break;                                            
        case CC_CSI_SVS:
            get_setting_ext(b_arr, 1, &gw_space_y);
            break;
        case CC_CSI_RCS:
            get_setting_ext(b_arr,1,&wtemp);
            if(wtemp==8)
                gb_rcol=CC_CMLA_TRANSPARENT;
            break;
        default:
            ICC_PRINTF("\nOh!Oh! Extended CSI (%x)",btemp);
            break;
    }
}
int dec_sta(UINT8 *gpbpkt)
{
    UINT16 wcnt=2,wlen=0;
    UINT8 bval=0;
    UINT16 wval=0;
    //UINT16 wtemp=0;

    //UINT8 bi,b_num,btemp;
    //UINT8 b_set_bgcolor=0;
    //UINT8 b_arr[20];
    static BOOL f_cs=FALSE;
    BOOL f_got_data=FALSE;
/*
     gbx=0;        //Active X
     gby=0;        //Active Y
     gbFcol=CC_CMLA_TRANSPARENT;    //Foreground Color
     gbBcol=CC_CMLA_TRANSPARENT;    //Background Color
     gbRcol=CC_CMLA_TRANSPARENT;    //Raster Color
*/
	if(NULL==gpbpkt)
	{
		return FALSE;
	}
    wcnt++;
    sta1.dw_len=cal_val((UINT8*) gpbpkt+wcnt, 3);
    wcnt+=3;
    if(CC_US!=gpbpkt[wcnt])
    {
        ICC_PRINTF("CC_US_ERR(%d->%x)\n",wcnt,gpbpkt[wcnt]);
        return FALSE;
    }
    sta1.b_type=gpbpkt[++wcnt];
    if(CC_TYPE_BODY!=sta1.b_type)
    {
        ICC_PRINTF("CC_TYPE_ERR(%d->%x)\n",wcnt,gpbpkt[wcnt]);
        return FALSE;
    }
    wcnt++;
    sta1.dw_size=cal_val((UINT8*) gpbpkt+wcnt, 3);
    wcnt+=3;
    wlen=wcnt+sta1.dw_size;
    //ICC_PRINTF("[CC Data]");
    while(wcnt<=wlen)
    {
        if(wcnt==wlen)//reach 1st byte of CRC16 , no need to parse
        {    break;    }

        bval=gpbpkt[wcnt];
        //ICC_PRINTF("(%d,%d-%x)",gbx,gby,bval);
        //Analyze char //C0,C1,CSI then Char of CC  (refer to GL.GR,G3)
        //C0 range
        if(CC_END_C0>=bval)
        {
            switch(bval)
            {
                case CC_C0_BEL:
                case CC_C0_LS1:
                case CC_C0_LS0:
                case CC_C0_SS2:
                case CC_C0_ESC:
                case CC_C0_RS:
                case CC_C0_US:
                    ICC_PRINTF("\nOh!Oh! C0 Char(%x)",bval);
                    break;
                case CC_C0_NULL:    break;    // no need handle
                case CC_C0_APB:
                case CC_C0_APF:
                case CC_C0_APD:
                case CC_C0_APU:
                case CC_C0_APR:
                    update_xy(bval);
                    break;
                case CC_C0_CS:
                    if(FALSE==f_cs)
                    {
                        if((gbx2>0) &&(gby2>0))
                        {
                            //ICC_FillChar(0, 0, gbx2+1, gby2+1, gbRcol);
                            icc_clear_screen(gbx2+1, gby2+1,gb_rcol);
                            f_cs=TRUE;
                            f_got_data=TRUE;
                        }
                    }
                    break;
                case CC_C0_CAN:
                    icc_fill_char(gbx,gby,gby2-gby+1,1,gb_bcol);
                    f_got_data=TRUE;
                    break;
                case CC_C0_SP:
                    if(gbx<=gbx2)    //patch due to one cc.ts has more space char (out of max x)
                    {
                        icc_fill_char(gbx,gby,1,1,gb_bcol);
                        f_got_data=TRUE;
                        gbx++;
                    }
                    break;
                case CC_C0_PAPF:
                    wcnt++;        
					bval=gpbpkt[wcnt]& 0x3F;
                    gbx+=bval;
                    while(gbx>gbx2)
                    {
                        gbx-=gbx2+1;
                        if(gby<gby2)        {gby++;}
                        else                {gby2=0;}
                    }
                    break;
                case CC_C0_APS:
                    wcnt++;        
					gby=gpbpkt[wcnt]& 0x3F;
                    wcnt++;        
					gbx=gpbpkt[wcnt]& 0x3F;
                    break;
                case CC_C0_SS3:
                    wcnt++;
                    bval=gpbpkt[wcnt];
                    wval=0x1d;    
					wval<<=8;
                    wval|=bval;
                    font_size_change();//Font only supports 18bit height
                    icc_draw_char1(gbx,gby,gb_fcol,gb_bcol,wval);
                    f_got_data=TRUE;
                    gbx++;
                    break;
            }
        }
        //C1 range
        else if ((CC_INIT_C1<=bval) && (CC_END_C1>=bval))
        {
            switch(bval)
            {
                //(need to discard P2)
                case CC_C1_TIME:// Time control
                wcnt++;
				break;
                // (need to discard P1)
                case CC_C1_SZX:        //Special size control , such as tiny , double , special
                case CC_C1_FLC:        // Flash control
                case CC_C1_CDC:        // Conceal control
                case CC_C1_WMM:    // Writng mode modification
                case CC_C1_POL:        // Polarity control
                case CC_C1_MACRO:    // Macro command
                case CC_C1_HLC:        // Highlight control
                case CC_C1_RPC:        // Repeat
                wcnt++;
				break;
                // (nothing to discard)
                case CC_C1_SPL:  //stop lining
                case CC_C1_STL:    //start lining (composition of moasaic A and B)
                    ICC_PRINTF("\nOh!Oh! C1 Char(%x)",bval);
                    break;
                case CC_C1_BKF:
                case CC_C1_RDF:
                case CC_C1_GRF:
                case CC_C1_YLF:
                case CC_C1_BLF:
                case CC_C1_MGF:
                case CC_C1_CNF:
                case CC_C1_WHF:
                    gb_fcol=bval-0x80;    //    CC_CMLA_BLACK as 0x00
                    break;
                case CC_C1_SSZ:
                    gw_char_x>>=1;    
					gw_space_x>>=1;
                    gw_char_y>>=1;    
					gw_space_y>>=1;
                    gbx2=gw_area_x/(gw_char_x+gw_space_x);
                    if(gbx2) {gbx2--;}
                    gby2=gw_area_y/(gw_char_y+gw_space_y);
                    if(gby2) {gby2--;}
                    //gwSpaceY>>=1;
                    //ICC_PRINTF("For(%d,%d)Area(%d,%d)Font(%d,%d)Space(%d,%d)End(%d,%d)\n",gwForX,gwForY,gwAreaX,gwAreaY,gwCharX,gwCharY,gwSpaceX,gwSpaceY,gbx2,gby2);
                    //ICC_PRINTF("For(%d,%d)Area(%d,%d)End(%d,%d) ",gwForX,gwForY,gwAreaX,gwAreaY,gbx2,gby2);
                    break;
                case CC_C1_MSZ:
                    gw_char_x>>=1;    
					gw_space_x>>=1;
                    if(gbx2) {gbx2--;}
                    gby2=gw_area_y/(gw_char_y+gw_space_y);
                    if(gby2) {gby2--;}
                    gw_space_y>>=1;
                    break;
                case CC_C1_NSZ:
                    gbx2=gw_area_x/(gw_char_x+gw_space_x);
                    if(gbx2) {gbx2--;}
                    gby2=gw_area_y/(gw_char_y+gw_space_y);
                    if(gby2) {gby2--;}
                    gw_space_y>>=1;
                    break; // no need handl, normal x y size
                case CC_C1_COL:
                    wcnt++;
                    bval=gpbpkt[wcnt];
                    update_col(bval);
                    break;
                case CC_C1_CSI:
                    dec_excsi(&wcnt,gpbpkt);
                    break;
            }
        }
        //GL+GR range
        else if(    ((CC_INIT_GL<=bval)&&(CC_END_GL>=bval))||    ((CC_INIT_GR<=bval)&&(CC_END_GR>=bval)))
        {
            if(gbx>gbx2)
            {    ICC_PRINTF("Out of X rang (%d)",gbx);}
            if(gby>gby2)
            {    ICC_PRINTF("Out of Y rang (%d)",gby);}
             font_size_change();//Font only supports 18bit height
            icc_draw_char1(gbx, gby, gb_fcol, gb_bcol, bval);
            gbx++;
            f_cs=FALSE;
            f_got_data=TRUE;
        }
        wcnt++;
    }
    if(TRUE==f_got_data)
        {icc_update_screen();}
    return TRUE;
}

void cc_display_stream()
{
    UINT8* pu_data;
    UINT8 bcnt;

    pu_data=b_grp_data;
    bcnt=2;
    switch(grp1.b_id)
    {
        case CC_GP_MAN_A:
        case CC_GP_MAN_B:
            if(FALSE!=parse_mang(&grp1,&bcnt,pu_data))    //Analyze management
            {
                #if 1
                    //ICC_PRINTF(",CC_Mang[%c %c %c]",Mang1.bLan[0],Mang1.bLan[1],Mang1.bLan[2]);
                    //USB_PRINTF("CM(%d)\n",wCmd);
                #else
                    USB_PRINTF("Man[%d] Num(%d)",w_cmd,mang1.b_num);
                    USB_PRINTF(",Tag(%d)",mang1.b_tag);
                    USB_PRINTF(",Format(%d)",mang1.b_format);
                    USB_PRINTF(",TCS(%d)",mang1.b_tcs);
                    USB_PRINTF(",ROL(%d)",mang1.b_rool_up);
                    USB_PRINTF(",LEN(%d)",mang1.dw_len);
                    USB_PRINTF(",Lan(%c %c  %c)\n",mang1.b_lan[0],mang1.b_lan[1],mang1.b_lan[2]);
                #endif
            }
            break;
        case CC_GP_STA_A1:
        case CC_GP_STA_A2:
        case CC_GP_STA_A3:
        case CC_GP_STA_A4:
        case CC_GP_STA_B1:
        case CC_GP_STA_B2:
        case CC_GP_STA_B3:
        case CC_GP_STA_B4:
        dec_sta(pu_data);
        break;

        default:
                ICC_PRINTF("Err Grp ID(%x)",grp1.b_id);// unknow value
                break;
    }
}

void cc_stream_dec(void)
{
    UINT8 bcnt;//,blen;
    UINT8* pu_data=p_ccdec_rd_hdr->pu_start_point;
    UINT32 dw_cur_size=0;
    UINT32 dw_valid_size=0;

    gcc_pktremain=FALSE;
    gdw_ccleft_size=0;
    gdw_ccsize=0;

    f_control=TRUE;
    f_got_bgcolor=FALSE;
    bcnt=3;
//    blen=0;
    grp1.b_id=0;
    grp1.dw_size=0;

    grp1.b_id=(pu_data[bcnt])>>2;
    bcnt+=3;
    grp1.dw_size=cal_val( (UINT8*) (pu_data+bcnt), 2);

    //ICC_PRINTF("\n PKT SIZE(%d),Grp Size(%d)",pCCDecRdHdr->uDataSize,Grp->dwSize);
    dw_valid_size=p_ccdec_rd_hdr->u_data_size-DATA_GRP_SIZE-PES_HEAD_SIZE;
    if((dw_valid_size)<(grp1.dw_size+CRC16_SIZE))
    {
        gdw_ccsize=grp1.dw_size+CRC16_SIZE;
        gdw_ccleft_size=gdw_ccsize-dw_valid_size;
        if(gdw_ccleft_size<CRC16_SIZE)
        {
            gdw_ccleft_size=0;
            gcc_pktremain=FALSE;
            dw_cur_size=grp1.dw_size;
        }
        else
        {
            gcc_pktremain=TRUE;
            dw_cur_size=dw_valid_size;
        }
        if(gcc_pktremain)
        {    ICC_PRINTF(",gcc_pktremain(%d)",gcc_pktremain);}
    }
    else
        dw_cur_size=grp1.dw_size;


    bcnt+=2;
    grp1.b_tmd=(pu_data[bcnt])>>6;
    if(CC_TMD_OFFSET_TIME==grp1.b_tmd)
        bcnt+=5;
    pu_data+=bcnt;
    bcnt=0;
    MEMSET(b_grp_data, 0x00, sizeof(b_grp_data));
    MEMCPY(b_grp_data+2,pu_data,dw_cur_size);
    *((UINT16*)b_grp_data)=(UINT16)dw_cur_size;
    if(FALSE==gcc_pktremain)
        cc_display_stream();
}

BOOL cc_parse_remain()
{
    UINT8* pu_data=p_ccdec_rd_hdr->pu_start_point;
    UINT32 dw_size=p_ccdec_rd_hdr->u_data_size;
    UINT32 dw_cur_size=0;
    //UINT8 bi;
    UINT16 w_offset=0;

    if(FALSE==gcc_pktremain)
        {goto ERR;}

    if(gdw_ccleft_size>dw_size)
    {
        dw_cur_size=dw_size;
        gdw_ccleft_size-=dw_size;
    }
    else
    {
        dw_cur_size=gdw_ccleft_size-CRC16_SIZE;
        gdw_ccleft_size=0;
    }

    if(gdw_ccleft_size<CRC16_SIZE)
    {
        gdw_ccleft_size=0;
        gcc_pktremain=FALSE;
    }

    //ICC_PRINTF("\n --PKT SIZE(%d),Remain Size(%d)",pCCDecRdHdr->uDataSize,dwCurSize);
    dw_remain_cnt++;
    w_offset=*((UINT16*)b_grp_data);
    MEMCPY(b_grp_data+2+w_offset,pu_data,dw_cur_size);
    if(FALSE==gcc_pktremain)
        cc_display_stream();
    /*
    ICC_PRINTF("\n[%d-(%d,%d)]",dwRemainCnt,dwCurSize,gdwCCLeftSize);
    for(bi=0;bi<dwCurSize;bi++)
    {
        ICC_PRINTF("%c",puData[bi]);
    }
    */
    return TRUE;
ERR:
    gcc_pktremain=FALSE;
    gdw_ccleft_size=0;
    return FALSE;
}
BOOL cc_display_entry_bl(void)
{
    #define INVALID_STC    0xffffffff
    #define TIME_AHEAD    45000    // 1sec    //    90000    // 2sec    //
    volatile UINT32 dwstc =0;
    UINT32 sync_delay = 0;
    dwstc=INVALID_STC;
    if( 0xFF != p_ccdec_rd_hdr->u_stc_id )
    {
        get_stc((UINT32 *)&dwstc, p_ccdec_rd_hdr->u_stc_id);
    }
    if(INVALID_STC!=dwstc)
    {
        if( (dwstc+45*32) < p_ccdec_rd_hdr->u_pts )
        {
            osal_task_sleep(1);
            get_stc((UINT32 *)&dwstc, p_ccdec_rd_hdr->u_stc_id);
            while(((dwstc+45*32) < p_ccdec_rd_hdr->u_pts) && (sync_delay<1000))
            {
                osal_task_sleep(1);
                get_stc((UINT32 *)&dwstc, p_ccdec_rd_hdr->u_stc_id);
                sync_delay++;
            }
            //if(sync_delay==1000)
              //  return FALSE;
        }
        // else cc data coming late, show it directly
    }
    if(gcc_task_open)
        cc_stream_dec();    //reach near time
    return TRUE;

}
void cc_stream_parse_bl(void)
{
    #define PES_HDR_DID    0x80
    #define PES_HDR_SID    0xFF
    #define PES_HDR_RES    0xF0
    //static UINT8 f_osd_drv_done=FALSE;
    //UINT8 sync_byte;
    //UINT8 segment_type;
    //UINT16 page_id;
    //UINT16 segment_length;

    UINT32 u_size;
    UINT8 * pu_data=NULL;

    UINT32 u_hdr_ptr;
    //UINT8 *tmpdata, tmpsync_byte, tmpsegment_type;
    //UINT32 tmpsize;
    //UINT16 tmppage_id, tmpsegment_length;
    //BOOL stuffing_has_valid_data = FALSE;
    //static UINT16 w_scnt=0;
    //static UINT16 w_mcnt=0;

    //ICC_PRINTF("1.CCParse,");

//    #define WATCH_PARSE_TIME

#ifdef WATCH_PARSE_TIME
    static UINT32 dw_curp_tick=0,dw_lastp_tick=0;
#endif

    while(1)
    {
#ifdef WATCH_PARSE_TIME
        dw_curp_tick=osal_get_tick()/1000;
        if(dw_lastp_tick!=dw_curp_tick)
        {
            dw_lastp_tick=dw_curp_tick;
            //ICC_PRINTF("PTick=%d\n",dwLastpTick);
            ICC_PRINTF("PTick=%d,size=%d\n",dw_lastp_tick,p_ccdec_rd_hdr->u_data_size);
        }
#endif


        if(p_ccdec_rd_hdr->u_data_size== 0)
        {
            if( RET_SUCCESS==isdbtcc_dec_hdr_buf_rd_req(&u_hdr_ptr))
            {
                p_ccdec_rd_hdr = &ccdec_bs_hdr_buf[u_hdr_ptr];
                isdbtcc_dec_hdr_buf_rd_update();
            }
            else
            {
                //ICC_PRINTF(",2.EOF Hd read");
                break; // not enough block data
            }
        }
        u_size = p_ccdec_rd_hdr->u_data_size;
        if(isdbtcc_dec_sbf_rd_req(&u_size,&pu_data) != RET_SUCCESS)
        {
                //ICC_PRINTF(",3.EOF Data read");
                break;//not enough data
        }
        else
        {
            if(pu_data!= p_ccdec_rd_hdr->pu_start_point)
                {CCDEC_ASSERT(FALSE);}

            if(p_ccdec_rd_hdr->u_pes_start == 1)  // skip 2 byte (data_identifier & sbutitle_stream_id)
            {
                //ICC_PRINTF("1.First(%d)",uSize);
                if((PES_HDR_DID==pu_data[0])&&(PES_HDR_SID==pu_data[1])&&(PES_HDR_RES==pu_data[2]))
                {
                    //if(TRUE==cc_find_stat())
                    if(1)
                    {
                        if(TRUE==cc_display_entry_bl())
                        {
                            isdbtcc_dec_sbf_rd_update(p_ccdec_rd_hdr->u_data_size);
                            p_ccdec_rd_hdr->u_data_size = 0;
                        }
                        break;
                    }
                    /*
                    else
                    {    //Mang
                        isdbtcc_dec_sbf_rd_update(pCCDecRdHdr->uDataSize);
                        pCCDecRdHdr->uDataSize = 0;
                    }
                    */
                }
                else
                {
                    //ICC_PRINTF(",4.Not CC pes!");
                    isdbtcc_dec_sbf_rd_update(p_ccdec_rd_hdr->u_data_size);
                    p_ccdec_rd_hdr->u_data_size = 0;
                }
                continue;
            }
            else
            {
                //ICC_PRINTF(",2.Remain(%d)",uSize);
                //ICC_PRINTF(",5.Non PES Start");
                cc_parse_remain();
                isdbtcc_dec_sbf_rd_update(p_ccdec_rd_hdr->u_data_size);
                p_ccdec_rd_hdr->u_data_size = 0;
            }
        }//sbf_rd_req(SDEC_DATA_BUF_ID) RET_SUCCESS
    }// end while loop for each update write
    //ICC_PRINTF("6.End !!");
}


#endif
void isdbtcc_disply_init(struct sdec_device *dev)
{
	if(NULL==dev)
	{
		return;
	}
    struct isdbtcc_priv* priv=(struct isdbtcc_priv*)(dev->priv);
    if(priv)
    {
        osal_interrupt_disable();
        priv->stream_parse_cb = cc_stream_parse_bl;
        priv->display_entry_cb = cc_display_entry_bl;
        osal_interrupt_enable();
    }
}

INT32 isdbtcc_dec_attach(struct sdec_feature_config * cfg_param)
{
    //return RET_FAILURE; //debugging

    if(!cfg_param)
        return RET_FAILURE;

    struct sdec_device *dev;
    struct isdbtcc_priv *tp;
    void *priv_mem;
    BOOL bl_addr_range_legal = TRUE;

	#if (defined(_M3715C_) || defined(_M3823C_)) 
	cfg_param->bs_buf =(UINT8*)__MM_ISDBTCC_BS_START_ADDR;
	cfg_param->bs_buf_len = __MM_ISDBTCC_BS_LEN;
	#endif
    /* Addr legal check */
    bl_addr_range_legal  = osal_check_address_range_legal(cfg_param->temp_buf, cfg_param->temp_buf_len);
    bl_addr_range_legal &= osal_check_address_range_legal(cfg_param->bs_buf, cfg_param->bs_buf_len);
    bl_addr_range_legal &= osal_check_address_range_legal((struct ccdec_data_hdr *)(cfg_param->bs_hdr_buf), cfg_param->bs_hdr_buf_len);
    bl_addr_range_legal &= osal_check_address_range_legal(cfg_param->pixel_buf, cfg_param->pixel_buf_len);
    bl_addr_range_legal &= osal_check_address_range_legal(cfg_param->sdec_hw_buf, cfg_param->sdec_hw_buf_len);
    if (FALSE == bl_addr_range_legal)
    {
        ICC_PRINTF("%s:%d Addr Illegal!\n",__func__,__LINE__);
        return RET_FAILURE;
    }

    dev = (struct sdec_device *)dev_alloc(cb_ccdec_name,HLD_DEV_TYPE_SDEC,sizeof(struct sdec_device));
    if (NULL==dev )
    {
        return RET_FAILURE;
    }
    /* Alloc structure space of private */
    priv_mem = (void *)MALLOC(sizeof(struct isdbtcc_priv));
    if (NULL==priv_mem )
    {
        dev_free(dev);
        return RET_FAILURE;
    }
    MEMSET(priv_mem, 0, sizeof(struct isdbtcc_priv));
    dev->priv =  tp = priv_mem;
       dev->next = NULL;
       dev->flags = 0;
    /* Function point init */
    dev->init = NULL;
    dev->open = isdbtcc_dec_open;
    dev->close = isdbtcc_dec_close;
    dev->request_write = isdbtcc_dec_request_write;
    dev->update_write = isdbtcc_dec_update_write;
    //dev->ioctl = isdbtcc_dec_ioctl;
    dev->start = isdbtcc_dec_start;
    dev->stop = isdbtcc_dec_stop;
    dev->pause = isdbtcc_dec_pause;
//    dev->showonoff = isdbtcc_dec_showonoff;
//    tp->isdbtcc_draw_char = cfg_param->isdbtcc_draw_char;
//    tp->isdbtcc_region_is_created = cfg_param->isdbtcc_region_is_created;
//    tp->isdbtcc_create_region = cfg_param->isdbtcc_create_region;
//    tp->isdbtcc_region_show = cfg_param->isdbtcc_region_show;
//    tp->isdbtcc_delete_region = cfg_param->isdbtcc_delete_region;
//    tp->isdbtcc_get_region_addr = cfg_param->isdbtcc_get_region_addr;

#if 1
    #if 0
    isdbtcc_dec_temp_buf_len = cfg_param->temp_buf_len;// > segment len
    isdbtcc_dec_temp_buf = cfg_param->temp_buf;
    if(!isdbtcc_dec_temp_buf)
    {
        isdbtcc_dec_temp_buf = (UINT8 *)MALLOC(isdbtcc_dec_temp_buf_len);
        if(NULL==isdbtcc_dec_temp_buf)
            CCDEC_ASSERT(0);
    }
    #endif
    isdbtcc_dec_bs_hdr_buf_len = cfg_param->bs_hdr_buf_len;
    ccdec_bs_hdr_buf = (struct ccdec_data_hdr *)cfg_param->bs_hdr_buf;
    if(!ccdec_bs_hdr_buf)
    {
        ccdec_bs_hdr_buf = (struct ccdec_data_hdr *)MALLOC(sizeof(struct ccdec_data_hdr)*isdbtcc_dec_bs_hdr_buf_len);
        if(NULL==ccdec_bs_hdr_buf)
        {    
        	//CCDEC_ASSERT(0);
        	ICC_PRINTF("ccdec_bs_hdr_buf malloc fail.\n");
        }
    }

    isdbtcc_dec_bs_buf_len = cfg_param->bs_buf_len;
    isdbtcc_dec_bs_buf = cfg_param->bs_buf;
    if(!isdbtcc_dec_bs_buf)
    {
        isdbtcc_dec_bs_buf = (UINT8 *)MALLOC(isdbtcc_dec_bs_buf_len);
        if(NULL==isdbtcc_dec_bs_buf)
        {    
        	//CCDEC_ASSERT(0);  
        	ICC_PRINTF("isdbtcc_dec_bs_buf malloc fail.\n");
		}
    }
    isdbtcc_dec_bs_end = isdbtcc_dec_bs_buf + isdbtcc_dec_bs_buf_len;

    isdbtcc_dec_tsk_qtm = cfg_param->tsk_qtm;
    isdbtcc_dec_tsk_pri = cfg_param->tsk_pri;

    isdbtcc_transparent_color = cfg_param->transparent_color;
#endif
    /* Add this device to queue */
    if (dev_register(dev) != RET_SUCCESS)
    {
        FREE(priv_mem);
        dev_free(dev);
        return RET_FAILURE;
    }
    gp_ccdec_priv = tp;

    return RET_SUCCESS;
}

int parse_mang(struct st_cc_grp *grp, UINT8 *pbcnt,UINT8 *gpbpkt)
{
	if((NULL==grp)||(NULL==pbcnt)||(NULL==gpbpkt))
	{
		return FALSE;	
	}
    UINT8 bi;
    UINT8 bcnt=*pbcnt;
    grp->p_grp=&mang1;
    mang1.b_num=gpbpkt[++bcnt];
    mang1.b_tag=gpbpkt[++bcnt] >>5;
    mang1.b_dmf=gpbpkt[bcnt] >>5;
    if(    (CC_DMF_AUTO_WHEN_REC==mang1.b_dmf) ||
        (CC_DMF_NO_WHEN_REC==mang1.b_dmf)    ||
        (CC_DMF_SEL_WHEN_REC==mang1.b_dmf)    )
    {
        bcnt+=1;
    }
    for(bi=0;bi<3;bi++)
    {
        mang1.b_lan[bi]=gpbpkt[++bcnt];
        //USB_PRINTF("%c",Mang1.bLan[bi]);
    }
        //USB_PRINTF("\n");
        mang1.b_format=gpbpkt[++bcnt]>>4;
        mang1.b_tcs=(gpbpkt[bcnt]>>2)&0x03;
        mang1.b_rool_up=gpbpkt[bcnt]&0x03;
        bcnt++;
        mang1.dw_len    =cal_val((UINT8*) gpbpkt+bcnt, 3);
        bcnt+=3;
        bcnt+=mang1.dw_len; // be careful about overflow
        bcnt+=2; // reach last 2B CRC
    *pbcnt=bcnt;
    return TRUE;
}
