/****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.

   *    File: atsc_cc_init.c

   *    Description: PSIP parsing engine to monitor and parse tables in streams
   *  History:
   *      Date         Author          Version   Comment
   *      ====         ======         =======   =======
   *  1.  ----     Steve Lee         0.1.000       Initial
   *  2.  2007.        HB Chen           0.2.000       modify, rename
   *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
         KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
         IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
         PARTICULAR PURPOSE.
   *****************************************************************************/

/*******************
* INCLUDE FILES    *
********************/
#include <sys_config.h>
#include <osal/osal.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/alloc.h>
#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <hld/decv/decv.h>
#include <hld/vbi/vbi.h>
#include <hld/vbi/vbi_dev.h>
#include <hld/dis/vpo.h>
#include <api/libclosecaption/atsc_cc_init.h>//vbi_m3327_cc_init.h
#include <api/libclosecaption/atsc_dtvcc_dec.h>
#include <api/libclosecaption/closecaption_osd.h> //vicky20110128
#include "lib_closecaption_internal.h"
#if 1//(SYS_CHIP_MODULE!=ALI_M3327C)&&((SYS_PROJECT_FE == PROJECT_FE_ATSC)||(SYS_PROJECT_FE == PROJECT_FE_DVBS))




#ifdef SEE_CPU   //compiler to seeboot_type

BOOL g_cc_by_vbi = FALSE;
BOOL g_disable_vbioutput=FALSE;

//EIA-608  CC Control commands 
#define CMD_CS 0x2c94;   //erase display memory, clear screen
#define CMD_CB 0xae94;   //erase non displayed memory, clear buffer
#define CMD_CL 0xa494;	  //delete to end of row, clear line 
#define CMD_DB 0x2f94;	  //end of caption, display buffer
/********************************************
* NAME: disable_vbioutput
*       set vbi output or not
*
* Returns : none
* Parameter                     Flow    Description
*
* ------------------------------------------------------------------------------
* BOOL bOn                  IN  Disable vbi output. TRUE: Disable, False: vbi output
* ------------------------------------------------------------------------------
* Additional information:
******************************************/
void disable_vbioutput(BOOL b_on)
{
    g_disable_vbioutput=b_on;
}

void set_cc_by_vbi(BOOL b_on)
{
    g_cc_by_vbi = b_on;
}

static void atsc_cc_output_proc(UINT8 field_parity , UINT16 data)
{
    struct vpo_io_cc cc;

    MEMSET(&cc,0,sizeof(struct vpo_io_cc));
    cc.field_parity = field_parity;
    cc.data = data;
    vpo_ioctl((struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS), VPO_IO_WRITE_CC, (UINT32)&cc);
}



static void clear_cc_data()
{
	struct vpo_io_cc cc_odd,cc_even;

	MEMSET(&cc_odd,0,sizeof(struct vpo_io_cc));
	MEMSET(&cc_even,0,sizeof(struct vpo_io_cc));

	//step1: erase display memory, clear screen	
    cc_odd.field_parity = 0;
    cc_odd.data = CMD_CS;   
    cc_even.field_parity = 1;
    cc_even.data = CMD_CS;	
    vpo_ioctl((struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS), VPO_IO_WRITE_CC, (UINT32)&cc_odd);
	vpo_ioctl((struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS), VPO_IO_WRITE_CC, (UINT32)&cc_even);

	//step2: erase non displayed memory, clear buffer		 
	cc_odd.field_parity = 0;
	cc_odd.data = CMD_CB;      	 
	cc_even.field_parity = 1;
	cc_even.data = CMD_CB;
	vpo_ioctl((struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS), VPO_IO_WRITE_CC, (UINT32)&cc_odd);
	vpo_ioctl((struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS), VPO_IO_WRITE_CC, (UINT32)&cc_even);

	//step3: delete to end of row, clear line		
	cc_odd.field_parity = 0;
	cc_odd.data = CMD_CL;		 		
	cc_even.field_parity = 1;
	cc_even.data = CMD_CL;
	vpo_ioctl((struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS), VPO_IO_WRITE_CC, (UINT32)&cc_odd);
	vpo_ioctl((struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS), VPO_IO_WRITE_CC, (UINT32)&cc_even);

	//step4: end of caption, display buffer
	cc_odd.field_parity = 0;    
    cc_odd.data = CMD_DB;		
    cc_even.field_parity = 1;
    cc_even.data = CMD_DB;
    vpo_ioctl((struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS), VPO_IO_WRITE_CC, (UINT32)&cc_odd);
	vpo_ioctl((struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS), VPO_IO_WRITE_CC, (UINT32)&cc_even);		
}
/********************************************
* NAME: vbi_cc_stop
*    
*
* Returns : void
*
* Parameter                     Flow    Description
* ------------------------------------------------------------------------------
* struct vbi_device *dev        IN  vbi dev
* ------------------------------------------------------------------------------
* Additional information:
******************************************/

INT32 vbi_cc_stop(struct vbi_device *dev)
{
    if(NULL == dev)
    {
        libc_printf("%s(),line %d\n",__FUNCTION__,__LINE__);
        return RET_FAILURE;
    }
	clear_cc_data();

    return RET_SUCCESS;
}

/********************************************
* NAME: vbi_cc_close
*    
*
* Returns : void
*
* Parameter                     Flow    Description
* ------------------------------------------------------------------------------
* struct vbi_device *dev        IN  vbi dev
* ------------------------------------------------------------------------------
* Additional information:
******************************************/
INT32 vbi_cc_close(struct vbi_device *dev)
{	
    if(NULL == dev)
    {
        libc_printf("%s(),line %d\n",__FUNCTION__,__LINE__);
        return RET_FAILURE;
    }

    return RET_SUCCESS;
}


/********************************************
* ------------------------------------------------------------------------------
* UINT8 FieldPolar              IN      no use
* ------------------------------------------------------------------------------
* Additional information:
******************************************/
//see
static void atsc_user_data_cc_see(__attribute__((unused)) UINT8 field_polar)
{
    UINT8 i=0;
    UINT8 j=0;
    UINT8 cc_cnt=0;
    UINT8 pariety_check_1=0;
    UINT8 pariety_check_2=0;
    UINT8 rd_idx = 0;
    enum tvsystem tvsys= NTSC;
    struct vdec_device *vdec_dev = NULL;
    UINT8 (*cc_user_data_inner)[USER_DATA_LENGTH_MAX] = NULL;

    struct vdec_cc_user_data_info cc_user_data;
    MEMSET(&cc_user_data,0,sizeof(struct vdec_cc_user_data_info));
    vdec_dev = (struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0);
    #ifndef SUPPORT_PAL
    vdec_io_control(vdec_dev, VDEC_IO_GET_CC_USER_DATA_INFO, &cc_user_data);
    cc_user_data_inner = cc_user_data.array;
    if(cc_user_data.array_cnt>0)
    #else
    UINT8 b_run=1;
    vpo_ioctl((struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS), VPO_IO_GET_OUT_MODE, (UINT32)(&tvsys));
    if(!((NTSC == tvsys)||(NTSC_443 == tvsys)))
    {
        b_run=2; //need to run twice.
        //libc_printf("PAL");
    }
   vdec_io_control(vdec_dev, VDEC_IO_GET_CC_USER_DATA_INFO, (UINT32)&cc_user_data);
    while( b_run && (cc_user_data.array_cnt>0))
    #endif
    {
        #ifdef SUPPORT_PAL
        b_run--;
        vdec_io_control(vdec_dev, VDEC_IO_GET_CC_USER_DATA_INFO, (UINT32)&cc_user_data);
        cc_user_data_inner = (UINT8 (*)[USER_DATA_LENGTH_MAX])cc_user_data.array;
        #endif
        rd_idx = g_user_data_array_rd;
        if((CC_INDICATOR_1== cc_user_data_inner[rd_idx][0])        //'G'
            &&(CC_INDICATOR_2== cc_user_data_inner[rd_idx][1])     //'A'
            &&(CC_INDICATOR_3==cc_user_data_inner[rd_idx][2])      //'9'
            &&(CC_INDICATOR_4==cc_user_data_inner[rd_idx][3])      //'4'
            &&(CC_DATA_INDICATOR==cc_user_data_inner[rd_idx][4])      //cc indicator
            &&(CC_DATA_FLAG==(cc_user_data_inner[rd_idx][5]&0x40)))       //process_cc_data_flag
        {
           // for(cc_cnt = 0;cc_cnt < 14;++cc_cnt)
            //libc_printf("%2x ",cc_user_data_inner[rd_idx][cc_cnt]);
            //     libc_printf("\n");
            cc_cnt = cc_user_data_inner[rd_idx][5]&0x1f;
            for(i=0;i<cc_cnt;i++)
            {
                if(0x04==(cc_user_data_inner[rd_idx][i*3+7]&0x04)  )//cc_valid == 1
                {
                    pariety_check_1=0;
                    pariety_check_2=0;
                    if( 0x00== (cc_user_data_inner[rd_idx][i*3+7]&0x03))//NTSC_CC_FIELD_1
                    {
                        for(j=0;j<8;j++)//pariety check
                        {
                            if( 0x01 == (((cc_user_data_inner[rd_idx][i*3+8])>>j)&0x01)) pariety_check_1 += 1;
                            if( 0x01 == (((cc_user_data_inner[rd_idx][i*3+9])>>j)&0x01)) pariety_check_2 += 1;
                            //libc_printf("%3x - %3x\n",(g_user_data[rd_idx][i*3+8]),(g_user_data[rd_idx][i*3+9]));
                        }
                        if( (pariety_check_1 % 2)==0 || (pariety_check_2 % 2)==0 )
                        {
                            continue;
                        }
                        //libc_printf("Test1 (%d)(%d)",ccg_vbi27_pconfig_par->cc_by_vbi,g_disable_vbioutput);
                        if((TRUE == g_cc_by_vbi) && (g_disable_vbioutput==FALSE))
                        {
                            atsc_cc_output_proc(1,(cc_user_data_inner[rd_idx][i*3+9]<<8)|
                                cc_user_data_inner[rd_idx][i*3+8]); // 1:top, 0:bot
                        }
                        
                    }
                    else if(0x01 == (cc_user_data_inner[rd_idx][i*3+7]&0x03))//NTSC_CC_FIELD_2
                    {
                        for(j=0;j<8;j++)//pariety check
                        {
                            if( 0x01 == (((cc_user_data_inner[rd_idx][i*3+8])>>j)&0x01))
                            {
                                pariety_check_1 += 1;
                            }
                            if( 0x01 == (((cc_user_data_inner[rd_idx][i*3+9])>>j)&0x01))
                            {
                                pariety_check_2 += 1;
                            }
                        }
                        if( (pariety_check_1 % 2)==0 || (pariety_check_2 % 2)==0 )
                        {
                            continue;
                        }
                        if((TRUE == g_cc_by_vbi) && (g_disable_vbioutput==FALSE))
                        {
                            atsc_cc_output_proc(0,(cc_user_data_inner[rd_idx][i*3+9]<<8)|
                                cc_user_data_inner[rd_idx][i*3+8]); // 1:top, 0:bot
                            //libc_printf("<%x,%x>",g_user_data[rd_idx][i*3+9],g_user_data[rd_idx][i*3+8]);
                        }
                    }
                }
            }
        }
        cc_user_data.array_cnt--;
        vdec_io_control(vdec_dev, VDEC_IO_SET_CC_USER_DATA_INFO, (UINT32)&cc_user_data);
        g_user_data_array_rd = (g_user_data_array_rd==USER_DATA_ARRAY_MAX-1)?0:g_user_data_array_rd+1;
    }
    //
}

/********************************************
* NAME: RequestVBIData
*       callback func to: 1 vbi output, 2 push user data for cc and dtvcc using
*
* Returns : void
*
* Parameter                     Flow    Description
* ------------------------------------------------------------------------------
* UINT8 FieldPolar              IN      no use
* ------------------------------------------------------------------------------
* Additional information:
******************************************/

void cc_request_vbidata(UINT8 field_polar)
{
    if(TRUE == g_cc_by_vbi)
        atsc_user_data_cc_see(field_polar);
}
/********************************************
* NAME: vbi_m3327_setoutput
*       set callback func
*
* Returns : void
*
* Parameter                     Flow    Description
* ------------------------------------------------------------------------------
* struct vbi_device *dev        IN  no use
* T_VBIRequest *pVBIRequest IN/OUT  callback point
* ------------------------------------------------------------------------------
* Additional information:
******************************************/
void ccvbi_m3327_setoutput(__attribute__((unused)) struct vbi_device *dev,t_vbirequest *p_vbirequest)
{
    if(NULL == p_vbirequest)
    {
        //libc_printf("%s(),line %d\n",__FUNCTION__,__LINE__);
        return;
    }
    *p_vbirequest = cc_request_vbidata;
}



#else

UINT16 *ccg_vbi27_dtvcc_by_osd=NULL;
UINT8 *ccg_vbi27_cc_by_osd = NULL;
BOOL ccg_disable_process_cc=FALSE;

BOOL clean_cc_buffer=FALSE;
BOOL b_ccrecv_pac=FALSE;
UINT32 ccvbi_rating=0;
UINT32 ccpre_vbi_rating=0;
BOOL ccvbi_cc=FALSE;
UINT8 ccvbi_cgmsa = 0;                  //for cgmsa
static struct vbi_m3327_private *g_vbi_priv;
struct vbi_config_par *ccg_vbi27_pconfig_par = NULL;
char ccvbi_m3327_name[HLD_MAX_NAME_SIZE] = "CCVBI_M3327_0";
UINT8 *ccg_vbi27_ttx_by_osd = NULL;
static char *g_vbi_m3327_name = NULL;
struct atsc_cc_config_par g_atsc_cc_config_par;
BOOL gf_ccattach=FALSE;
static BOOL XDS=FALSE;
extern void dtvccparsing_task(UINT32 param1,UINT32 param2);

#ifdef  CC_BY_OSD
//static OSAL_ID vbidec_tsk_id = 0;
//static OSAL_ID vbidec_flag_id = 0;
#endif

void lib_atsc_cc_attach(struct atsc_cc_config_par *pcc_config_par)
{
    BOOL bl_addr_range_legal = TRUE;

    if(NULL == pcc_config_par)
    {
        return;//VBI_PRINTF("Oh!Oh!, Forgot to set atsc_cc_config_par: ex Bitstream /Draw Area Buf Addr, OSD Layer!!\n");
    }

    if((1!= pcc_config_par->osd_layer_id)||(576!=pcc_config_par->cc_width)\
        ||(390!=pcc_config_par->cc_height)||(500!=pcc_config_par->w_polling_time)\
        ||(4!=pcc_config_par->b_ccscan)||(4!=pcc_config_par->b_dtvccscan))
    {
        return;
    }
    /* Addr legal check */
    bl_addr_range_legal  = osal_check_address_range_legal(pcc_config_par->pb_buf_addr, pcc_config_par->pb_buf_len);
    bl_addr_range_legal &= osal_check_address_range_legal(pcc_config_par->bs_buf_addr, pcc_config_par->bs_buf_len);
    if (FALSE == bl_addr_range_legal)
    {
        //libc_printf("%s:%d Addr Illegal!\n",__func__,__LINE__);
        return;
    }

    MEMCPY(&g_atsc_cc_config_par, pcc_config_par, sizeof(struct atsc_cc_config_par));
    gf_ccattach=TRUE;
    VBI_PRINTF("%s() Done !!\n",__FUNCTION__);

}

/********************************************
* NAME: vbi_cc_init
*       init cc parameters and set callback fucn of vpo_isr to excute push user data for cc using
*
* Returns : BOOL
*              1        : RET_FAILURE
*              0        : RET_SUCCESS
* Parameter                     Flow    Description
* ------------------------------------------------------------------------------
* None
* ------------------------------------------------------------------------------
* Additional information:
******************************************/
__ATTRIBUTE_REUSE_
INT32 vbi_cc_init(void)
{
    struct vbi_device *dev = NULL;
    struct vbi_m3327_private *tp = NULL;
    void *priv_mem = NULL;

    //error handle while "never call lib_atsc_cc_attach() previously".
    if(!gf_ccattach)
    {
        VBI_PRINTF("Oh!Oh!no call lib_atsc_cc_attach() first. Use Default Parameter");
        g_atsc_cc_config_par.bs_buf_len=0x1c00;
        g_atsc_cc_config_par.pb_buf_len=0x45780;
        g_atsc_cc_config_par.osd_layer_id=1;
        g_atsc_cc_config_par.cc_width=576;
        g_atsc_cc_config_par.cc_height=390;
        g_atsc_cc_config_par.w_polling_time=500;
        g_atsc_cc_config_par.b_ccscan=4;
        g_atsc_cc_config_par.b_dtvccscan=4;
        g_atsc_cc_config_par.bs_buf_addr=(UINT8*)MALLOC(g_atsc_cc_config_par.bs_buf_len);
        g_atsc_cc_config_par.pb_buf_addr=(UINT8*)MALLOC(g_atsc_cc_config_par.pb_buf_len);
        if(g_atsc_cc_config_par.bs_buf_addr==NULL)
        {
        	  
            VBI_PRINTF("Fail to alloc mem for CC BitStream\n");
            return RET_FAILURE;
        }
        if(g_atsc_cc_config_par.pb_buf_addr==NULL)
        {	
            VBI_PRINTF("Fail to alloc mem for CC Picture buffer\n");
            return RET_FAILURE;
        }
    }
    g_vbi_m3327_name = (char *)ccvbi_m3327_name;

    dev = (struct vbi_device *)dev_alloc(g_vbi_m3327_name,HLD_DEV_TYPE_VBI,sizeof(struct vbi_device));
    if ( NULL == dev)
    {
        VBI_PRINTF("Error: Alloc video vbiplay device error!\n");
        return RET_FAILURE;
    }
    /* Alloc structure space of private */
    priv_mem = (void *)MALLOC(sizeof(struct vbi_m3327_private));
    if (NULL == priv_mem)
    {
        dev_free(dev);
        dev = NULL;
        VBI_PRINTF("Alloc vbiplay device prive memory error!/n");
        return RET_FAILURE;
    }
    MEMSET(priv_mem, 0, sizeof(struct vbi_m3327_private));
    g_vbi_priv = dev->priv =  tp = (struct vbi_m3327_private *)priv_mem;
    ccg_vbi27_pconfig_par = &(g_vbi_priv->config_par);
    ccg_vbi27_ttx_by_osd = &(g_vbi_priv->ttx_by_osd);
    ccg_vbi27_cc_by_osd = &(g_vbi_priv->cc_by_osd);
#ifdef CC_BY_OSD
    ccg_vbi27_dtvcc_by_osd=&(g_vbi_priv->dtvcc_by_osd);//hbchen
#endif
    /*according the macro , config the VBI driver*/
  //vbi_data_array = (struct vbi_data_array_t *)MALLOC((VBI_QUEUE_DEPTH)*sizeof(struct vbi_data_array_t));
  //MEMSET(vbi_data_array, 0, (VBI_QUEUE_DEPTH)*sizeof(struct vbi_m3327_private));


    *ccg_vbi27_ttx_by_osd = FALSE;
    g_vbi_priv->init_ttx_decoder = NULL;

#ifdef  CC_BY_OSD
    *ccg_vbi27_cc_by_osd = TRUE;
    g_vbi_priv->init_cc_decoder = vbi_m3327_init_cc_decoder;
    g_vbi_priv->vbi_line21_push_by_cc = vbi_line21_push;
    g_vbi_priv->vbi_line21_push_by_dtvcc = vbi_line21_push_dtvcc;//xing for DTVCC
#else
    *ccg_vbi27_cc_by_osd = FALSE;
#endif


#ifdef  CC_BY_VBI
    ccg_vbi27_pconfig_par->cc_by_vbi = TRUE;
#else
    ccg_vbi27_pconfig_par->cc_by_vbi = FALSE;
#endif

    set_cc_by_vbi(ccg_vbi27_pconfig_par->cc_by_vbi);
    //dev->setoutput = vbi_m3327_setoutput;

    dev->next = NULL;
    dev->flags = 0;
    /* Function point init */
    dev->init = vbi_cc_init;
    dev->open = vbi_cc_open;
    dev->ioctl = NULL;
    dev->close = vbi_cc_close;
    dev->request_write = NULL;
    dev->update_write = NULL;
    dev->setoutput = ccvbi_m3327_setoutput;
    dev->start = NULL;
    dev->stop = vbi_cc_stop;;

    /* Add this device to queue */
    if(dev_register(dev) != RET_SUCCESS)
    {
        VBI_PRINTF("Error: Register vbiplay device error!\n");
        FREE(priv_mem);
        dev_free(dev);
        priv_mem = NULL;
        dev = NULL;
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}


/********************************************
* NAME: atsc_user_data_cc_main
*       callback func to: push user data for cc and dtvcc using
*
* Returns : void
*
* Parameter                     Flow    Description
* ------------------------------------------------------------------------------
* UINT8 FieldPolar              IN      no use
* ------------------------------------------------------------------------------
* Additional information:
******************************************/

void atsc_user_data_cc_main(UINT32 uparam1, UINT32 uparam2)
{
    UINT8 i=0;
    UINT8 j=0;
    UINT8 cc_cnt=0;
    UINT8 byte1=0;
    UINT8 byte2=0;
    UINT8 pariety_check_1=0;
    UINT8 pariety_check_2=0;
    UINT8 cc_field=0;
    BOOL push_1=FALSE;
    BOOL push_2=FALSE;
    BOOL b_cc_valid=FALSE;
    struct user_data_pram atsc_cc_user_data;    
    struct user_data_pram atsc_cc_user_data_tmp;
	
    MEMSET(&atsc_cc_user_data,0,sizeof(struct user_data_pram));
    MEMCPY(&atsc_cc_user_data,(struct user_data_pram *)uparam1,sizeof(struct user_data_pram));

	/* contain the new vdec cc header: 00 00 00 01 06 */
    if(((VDEC_CC_HEARDER_1== atsc_cc_user_data.user_data[0])
		&&(VDEC_CC_HEARDER_2== atsc_cc_user_data.user_data[1])
		&&(VDEC_CC_HEARDER_3== atsc_cc_user_data.user_data[2])
		&&(VDEC_CC_HEARDER_4== atsc_cc_user_data.user_data[3])
		&&(VDEC_CC_HEARDER_5== atsc_cc_user_data.user_data[4]))
		&&((CC_INDICATOR_1== atsc_cc_user_data.user_data[5])        //'G'
        &&(CC_INDICATOR_2== atsc_cc_user_data.user_data[6])     //'A'
        &&(CC_INDICATOR_3==atsc_cc_user_data.user_data[7])      //'9'
        &&(CC_INDICATOR_4==atsc_cc_user_data.user_data[8])      //'4'
        &&(CC_DATA_INDICATOR==atsc_cc_user_data.user_data[9])      //cc indicator
        &&(CC_DATA_FLAG==(atsc_cc_user_data.user_data[10]&0x40))))       //process_cc_data_flag
    {
		MEMSET((UINT8 *)&atsc_cc_user_data_tmp.user_data[0],0,sizeof(atsc_cc_user_data_tmp.user_data));
		MEMCPY((UINT8 *)&atsc_cc_user_data_tmp.user_data[0],(UINT8 *)&atsc_cc_user_data.user_data[5],(sizeof(atsc_cc_user_data_tmp.user_data)-5));
		MEMSET((UINT8 *)&atsc_cc_user_data.user_data[0],0,sizeof(atsc_cc_user_data.user_data));
		MEMCPY((UINT8 *)&atsc_cc_user_data.user_data[0],(UINT8 *)&atsc_cc_user_data_tmp.user_data[0],(sizeof(atsc_cc_user_data.user_data)-5));

		atsc_cc_user_data.user_data_size -= 5;

		b_cc_valid = TRUE;
	}
	
	/* contain the new vdec(used for h.265) cc header: 00 00 01 4E */
	else if(((VDECH265_CC_HEARDER_1== atsc_cc_user_data.user_data[0])
		&&(VDECH265_CC_HEARDER_2== atsc_cc_user_data.user_data[1])
		&&(VDECH265_CC_HEARDER_3== atsc_cc_user_data.user_data[2])
		&&(VDECH265_CC_HEARDER_4== atsc_cc_user_data.user_data[3]))
		&&((CC_INDICATOR_1== atsc_cc_user_data.user_data[4])		//'G'
		&&(CC_INDICATOR_2== atsc_cc_user_data.user_data[5]) 	//'A'
		&&(CC_INDICATOR_3==atsc_cc_user_data.user_data[6])		//'9'
		&&(CC_INDICATOR_4==atsc_cc_user_data.user_data[7])		//'4'
		&&(CC_DATA_INDICATOR==atsc_cc_user_data.user_data[8])	   //cc indicator
		&&(CC_DATA_FLAG==(atsc_cc_user_data.user_data[9]&0x40))))		 //process_cc_data_flag
	{
		MEMSET((UINT8 *)&atsc_cc_user_data_tmp.user_data[0],0,sizeof(atsc_cc_user_data_tmp.user_data));
		MEMCPY((UINT8 *)&atsc_cc_user_data_tmp.user_data[0],(UINT8 *)&atsc_cc_user_data.user_data[4],(sizeof(atsc_cc_user_data_tmp.user_data)-4));
		MEMSET((UINT8 *)&atsc_cc_user_data.user_data[0],0,sizeof(atsc_cc_user_data.user_data));
		MEMCPY((UINT8 *)&atsc_cc_user_data.user_data[0],(UINT8 *)&atsc_cc_user_data_tmp.user_data[0],(sizeof(atsc_cc_user_data.user_data)-4));
	
		atsc_cc_user_data.user_data_size -= 4;
	
		b_cc_valid = TRUE;
	}
	/* the old cc header, not contain the 00 00 00 01 06*/
	else if(((CC_INDICATOR_1== atsc_cc_user_data.user_data[0])        //'G'
        &&(CC_INDICATOR_2== atsc_cc_user_data.user_data[1])     //'A'
        &&(CC_INDICATOR_3==atsc_cc_user_data.user_data[2])      //'9'
        &&(CC_INDICATOR_4==atsc_cc_user_data.user_data[3])      //'4'
        &&(CC_DATA_INDICATOR==atsc_cc_user_data.user_data[4])      //cc indicator
        &&(CC_DATA_FLAG==(atsc_cc_user_data.user_data[5]&0x40))))
	{
		b_cc_valid = TRUE;
	}
	else /* invalid cc data */
	{
		return;
	}
	
	if(b_cc_valid)
    {
        //for(cc_cnt = 0;cc_cnt < 14;++cc_cnt)
        //    libc_printf("%2x ",atsc_cc_user_data.user_data[cc_cnt]);
        //libc_printf("\n");
        cc_cnt = atsc_cc_user_data.user_data[5]&0x1f;
        for(i=0;i<cc_cnt;i++)
        {
            if(0x04==(atsc_cc_user_data.user_data[i*3+7]&0x04)  )//cc_valid == 1
            {
                pariety_check_1=0;pariety_check_2=0;
                byte1=(atsc_cc_user_data.user_data[i*3+8])&0x7F;
                byte2=(atsc_cc_user_data.user_data[i*3+9])&0x7F;
                #if 1 //for cgmsa
                if((0x01==byte1 && 0x05==byte2)||(0x01==byte1 && 0x08==byte2)||
                    (0x09==byte1 &&0x01==byte2) || (0x09==byte1 &&0x02==byte2) \
                    || (0x0a==byte1 && 0x01==byte2) || (0x0a==byte1 &&0x02==byte2) )
                    //v-chip ,CGMS-A, puplic service & puplic service cont.
                #else
                if((0x01==byte1 && 0x05==byte2) ||(0x09==byte1 && 0x01==byte2) || (0x09==byte1 &&0x02==byte2) \
                    || (0x0a==byte1 && 0x01==byte2) ||
                    (0x0a==byte1 &&0x02==byte2) ) //v-chip , puplic service & puplic service cont.
                #endif
                {
                    XDS=TRUE;
                }
                if((byte1>=0x10&&byte1<=0x1f)&&(!(byte2>0x2f&&byte2<0x40))&&
                    byte2>=0x20&&byte2<=0x7f)//control data
                {
                    XDS=FALSE;
                }
                if(((byte1>=0x10)&&(byte1<=0x1f))&&(!((byte2>0x2f)&&(byte2<0x40)))&&
                    (byte2>=0x20)&&(byte2<=0x7f)&&(!(((0x12==byte1)||(0x13==byte1)||
                    (0x1A==byte1)||(0x1B==byte1))&&((byte2>0x1f)&&(byte2<0x40)))))
                {
                    ccvbi_cc=TRUE;
                }
                push_1 = ((*ccg_vbi27_cc_by_osd==0x00 )|| (*ccg_vbi27_cc_by_osd==0x01) ||
                (*ccg_vbi27_cc_by_osd==0x02)||( *ccg_vbi27_cc_by_osd==0x05)||( *ccg_vbi27_cc_by_osd==0x06));
                push_2 = ( XDS ||  (*ccg_vbi27_cc_by_osd==0x00) || (*ccg_vbi27_cc_by_osd==0x03) ||
                    (*ccg_vbi27_cc_by_osd==0x04)||(*ccg_vbi27_cc_by_osd==0x07)||(*ccg_vbi27_cc_by_osd==0x08));

                if( 0x00== (atsc_cc_user_data.user_data[i*3+7]&0x03))//NTSC_CC_FIELD_1
                {
                    for(j=0;j<8;j++)//pariety check
                    {
                        if( 0x01 == (((atsc_cc_user_data.user_data[i*3+8])>>j)&0x01)) pariety_check_1 += 1;
                        if( 0x01 == (((atsc_cc_user_data.user_data[i*3+9])>>j)&0x01)) pariety_check_2 += 1;
                        //libc_printf("%3x - %3x\n",(g_user_data[rd_idx][i*3+8]),(g_user_data[rd_idx][i*3+9]));
                    }
                    if( (pariety_check_1 % 2)==0 || (pariety_check_2 % 2)==0 )
                    {
                        continue;
                    }
                    cc_field=1;
                    //libc_printf("Test1 (%d)(%d)",ccg_vbi27_pconfig_par->cc_by_vbi,g_disable_vbioutput);
                    
                    if(ccg_disable_process_cc==TRUE)
                    {
                        //soc_printf("ccrecv_PAC=%d\n",ccrecv_PAC);
                        b_ccrecv_pac=FALSE;
                        //MEMSET(g_user_data[rd_idx],0x00,USER_DATA_LENGTH_MAX);
                        //return;
                    }
                    if(push_1 )//cc1 cc2 tt1 tt2
                    {
                        if(NULL != g_vbi_priv->vbi_line21_push_by_cc)
                        {
                            cc_push_field(cc_field);
                            g_vbi_priv->vbi_line21_push_by_cc((byte1<<8)|byte2);
                        }
                    }
                }
                else if(0x01 == (atsc_cc_user_data.user_data[i*3+7]&0x03))//NTSC_CC_FIELD_2
                {
                    for(j=0;j<8;j++)//pariety check
                    {
                        if( 0x01 == (((atsc_cc_user_data.user_data[i*3+8])>>j)&0x01))
                        {
                            pariety_check_1 += 1;
                        }
                        if( 0x01 == (((atsc_cc_user_data.user_data[i*3+9])>>j)&0x01))
                        {
                            pariety_check_2 += 1;
                        }
                    }
                    if( (pariety_check_1 % 2)==0 || (pariety_check_2 % 2)==0 )
                    {
                        continue;
                    }

                    cc_field=2;
                    
                    if(ccg_disable_process_cc==TRUE)
                    {
                        b_ccrecv_pac=FALSE;
                        
                    }

                    if(push_2 )//cc3 cc4 tt3 tt4
                    {
                        if(NULL != g_vbi_priv->vbi_line21_push_by_cc)
                        {
                            cc_push_field(cc_field);
                            g_vbi_priv->vbi_line21_push_by_cc((byte1<<8)|byte2);
                        }
                    }
                }
                #ifdef CC_MONITOR_CS
                else if((0x03==(atsc_cc_user_data.user_data[i*3+7]&0x03))||(0x02==(atsc_cc_user_data.user_data[i*3+7]&0x03)))
                #else
                else if(((0x03== (atsc_cc_user_data.user_data[i*3+7]&0x03))||(0x02==(atsc_cc_user_data.user_data[i*3+7]&0x03)))&&
                        (g_dtv_cc_service !=0) && (0x00==*ccg_vbi27_cc_by_osd))
                #endif
                {
                    if(NULL != g_vbi_priv->vbi_line21_push_by_dtvcc)
                    {
                        if( 0x03== (atsc_cc_user_data.user_data[i*3+7]&0x03))
                        {
                            //soc_printf("\n==================start packet =================\n");
                            g_vbi_priv->vbi_line21_push_by_dtvcc((0xff<<8)|0xff );
                        }
                        //soc_printf("s => ,%x,%x\n",g_user_data[rd_idx][i*3+8],g_user_data[rd_idx][i*3+9] );
                        g_vbi_priv->vbi_line21_push_by_dtvcc(
                        (((atsc_cc_user_data.user_data[i*3+8])&0XFF)<<8)|((atsc_cc_user_data.user_data[i*3+9])&0XFF));
                     }
                }
                if(CC_XDS_INVALID_FLAG==byte1) XDS = FALSE;
            }
        }
    }
}



/********************************************
* NAME: vbi_cc_open
*       init cc task
*
* Returns : void
*
* Parameter                     Flow    Description
* ------------------------------------------------------------------------------
* struct vbi_device *dev        IN  vbi dev
* ------------------------------------------------------------------------------
* Additional information:
******************************************/
INT32 vbi_cc_open(struct vbi_device *dev)
{
    if(NULL == dev)
    {
        //libc_printf("%s(),line %d\n",__FUNCTION__,__LINE__);
        return RET_FAILURE;
    }

    struct vbi_m3327_private *tp = dev->priv;

    if(NULL != tp->init_cc_decoder)
    {
        tp->init_cc_decoder(dev);
    }

    return RET_SUCCESS;
}
#ifdef  CC_BY_OSD
/********************************************
* NAME: CCParsing_Task
*       cc task
*
* Returns : void
*
* Parameter                     Flow    Description
* ------------------------------------------------------------------------------
* UINT32 param1             IN  no use
* UINT32 param2             IN  no use
* ------------------------------------------------------------------------------
* Additional information:
* EAS will disable cc_parsing and cc_parsing_dtvcc
* cc_parsing_dtvcc will disable if enable cc decode
******************************************/
void ccparsing_task(UINT32 param1,UINT32 param2)
{
    dtvcc_init();
    while(1) // run forever when task exist.
    {
        osal_task_sleep(5); //for atsc,speed up cc_parsing()

        eas_text_scroll();

        //if(*ccg_vbi27_cc_by_osd != 0)
        cc_parsing();// can't turn off this parsing for parsing v-chip data for rating lock setting

    }//end main while loop
}

/********************************************
* NAME: vbi_m3327_init_cc_decoder
*       init atsc closecaption decoder
*
* Returns : void
*
* Parameter                     Flow    Description
* ------------------------------------------------------------------------------
* struct vbi_device *dev        IN  vbi dev
* ------------------------------------------------------------------------------
* Additional information:
******************************************/

OSAL_ID cc_tsk_id;
OSAL_ID dtvcc_tsk_id;

void vbi_m3327_init_cc_decoder(struct vbi_device *dev)
{
	OSAL_T_CTSK t_ctsk;
	OSAL_T_CTSK t_dtcctsk;

    MEMSET(&t_ctsk, 0 ,sizeof(OSAL_T_CTSK));
    t_ctsk.task = (FP)ccparsing_task;
    t_ctsk.para1 = (UINT32)dev;
    t_ctsk.para2 = 0;
    //t_ctsk.stksz = 0x0C00;
    t_ctsk.stksz = 0x0F00;//5k
    t_ctsk.quantum = 10;
    t_ctsk.itskpri = OSAL_PRI_NORMAL;
#ifdef _TDS_INFO_MONITOR
    t_ctsk.name[0] = 'C';
    t_ctsk.name[1] = 'C';
    t_ctsk.name[2] = 'T';
#endif
	cc_tsk_id = osal_task_create(&t_ctsk);
	if(OSAL_INVALID_ID==cc_tsk_id)
	{
	     VBI_PRINTF("Can not create CCParsing_Task!!!\n");
		 VBI_ASSERT(0);
	}

	/* DTVCCParsing_Task */
	t_dtcctsk.task = (FP)dtvccparsing_task;
	t_dtcctsk.para1 = (UINT32)dev;
	t_dtcctsk.para2 = 0;
	t_dtcctsk.stksz = 0x0F00;//5k
	t_dtcctsk.quantum = 10;
	t_dtcctsk.itskpri = OSAL_PRI_NORMAL;
#ifdef _TDS_INFO_MONITOR
	t_dtcctsk.name[0] = 'D';
	t_dtcctsk.name[1] = 'T';
	t_dtcctsk.name[2] = 'V';
#endif
	dtvcc_tsk_id = osal_task_create(&t_dtcctsk);
	if(OSAL_INVALID_ID==dtvcc_tsk_id)
	{
		 VBI_PRINTF("Can not create DTVCCParsing_Task!!!\n");
		 VBI_ASSERT(0);
	}

}
/********************************************
* NAME: vbi_enable_cc_by_osd
*       enable cc by osd
*
* Returns : void
*
* Parameter                     Flow    Description
* ------------------------------------------------------------------------------
* struct vbi_device *dev        IN  vbi dev
* ------------------------------------------------------------------------------
* Additional information:
* no use now, when compile atsc project, vbi_cc_init will set these parameters.
******************************************/
void vbi_enable_cc_by_osd(struct vbi_device*dev)
{
    struct vbi_m3327_private *priv = NULL;

    if(NULL == dev)
    {
        libc_printf("%s(),line %d\n",__FUNCTION__,__LINE__);
        return;
    }
    priv = (struct vbi_m3327_private *)(dev->priv);
    priv->cc_by_osd = TRUE;
    priv->init_cc_decoder = vbi_m3327_init_cc_decoder;
    priv->vbi_line21_push_by_cc = vbi_line21_push;
    priv->vbi_line21_push_by_dtvcc = vbi_line21_push_dtvcc;//for DTVCC
}
#endif
#endif
#endif

