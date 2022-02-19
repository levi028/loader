/*****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.

   *    File: vbi_m3327.c

   *    Description:define the operation used by low layer VBI
   *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
         KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
         IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
         PARTICULAR PURPOSE.
   *****************************************************************************/

#include <sys_config.h>
#include <osal/osal.h>
#include <basic_types.h>
#include <mediatypes.h>

#include <api/libc/printf.h>
#include <api/libc/alloc.h>
#include <api/libc/string.h>
//#include <api/libttx/lib_ttx.h>


#include <hld/hld_dev.h>
#include <hld/vbi/vbi.h>
#include <hld/vbi/vbi_dev.h>
#include <hld/dis/vpo.h>
#include <hal/hal_common.h>

#include "vbi_buffer.h"
#include "vbi_m3327.h"
#include "vbi_m3327_internal.h"

//#include "../../../dis/m3327/tv_enc.h"

#define    TTX_PACKET_MAX_LEN 256
/*config par*/
struct vbi_config_par *g_vbi27_pconfig_par = NULL;
UINT8 *g_vbi27_ttx_by_osd = NULL;
//#ifdef ENABLE_ATSC_USER_DATA
UINT8 *g_vbi27_cc_by_osd = NULL;
//========================
UINT16 *g_vbi27_dtvcc_by_osd=NULL;
char vbi_m3327_name[HLD_MAX_NAME_SIZE]= "VBI_M3327_0";

#ifndef SEE_CPU 

static OSAL_ID vbidec_tsk_id = OSAL_INVALID_ID;
static OSAL_ID vbidec_flag_id = OSAL_INVALID_ID;
static BOOL enable_vbi = TRUE;
static UINT8 last_stcid =0xff;  //only use in function below
static UINT32 vbi_last_pts = 0;

#endif

//BOOL g_stop_cc=FALSE;
//UINT8 cc_field=0;
//#endif //vicky20101229 dbg
//========================

//char vbi_m3327_name[HLD_MAX_NAME_SIZE] = "VBI_M3327_0";
struct vbi_m3327_private *g_vbi_priv= NULL;
#ifdef SEE_CPU
struct vbi_device *g_vbi_device = NULL;
#endif

/*variable & array for TTX decoder*/
#ifdef TTX_BY_OSD
t_ttxdec_cbfunc p_cbpage_update = NULL;
UINT8 *g_ttx_p26_nation = NULL;
#endif
struct vbidata_hdr *vbi_data_hdr = NULL;

//change to dynamic alloc to aviod gp overflow


volatile UINT32 *vbi_queue_cnt = NULL;
volatile UINT16 *vbi_queue_rd = NULL;
volatile UINT16 *vbi_queue_wr = NULL;


#define QUEUE_FULL(rd,wr) ((wr)+1==(rd))
#define QUEUE_EMPTY(rd,wr) ((rd)==(wr))
#define QUEUE_NEXT(p)  ((p)+1==VBI_QUEUE_DEPTH?0:((p)+1))

#ifdef SEE_CPU   //compiler to see


struct vbi_data_array_t *g_see_vbi_data_array = NULL;

//static volatile UINT16 g_see_vbi_queue_cnt = 0;
static UINT32 g_vbi_status = 0;
static BOOL g_ttx_by_vbi = FALSE;
static UINT16 g_ttx_tve_id = 0;
void vbi_set_vbi_data_addr(struct vbi_data_array_t *p_vbi_data_array,UINT32 *cnt_add )
{
    g_see_vbi_data_array = p_vbi_data_array;
    vbi_queue_cnt = cnt_add;
	vbi_queue_rd = (UINT16 *)vbi_queue_cnt;
	vbi_queue_wr = (UINT16 *)(vbi_queue_rd+1);
}

void vbi_queue_set_rd(UINT16 rd)
{  
    *vbi_queue_rd=rd;
}



void set_vbi_status_to_see(UINT32 status)
{
    g_vbi_status = status;  
}

void set_ttx_by_vbi(BOOL b_on)
{
    g_ttx_by_vbi = b_on;
}

void set_tve_id(UINT16 id)
{
    g_ttx_tve_id = id;
}

static void vbi_output_proc(UINT8 line_addr, UINT8 addr, UINT8 data)
{
    struct tve_device *ptve_dev = NULL;
    struct vpo_io_ttx ttx;
    RET_CODE func_ret = 0;

    MEMSET(&ttx,0,sizeof(struct vpo_io_ttx));

    ptve_dev = (struct tve_device *)dev_get_by_id(HLD_DEV_TYPE_TVE, g_ttx_tve_id);
    if (ptve_dev)
    {
        func_ret = tvenc_write_ttx(ptve_dev, line_addr, addr, data);
		
		
		func_ret=(func_ret==RET_SUCCESS)?RET_SUCCESS:RET_FAILURE;
		
    }
    else
    {
        ttx.line_addr = line_addr;
        ttx.addr = addr;
        ttx.data = data;
        func_ret = vpo_ioctl((struct vpo_device *)dev_get_by_type
                  (NULL, HLD_DEV_TYPE_DIS), VPO_IO_WRITE_TTX, (UINT32)&ttx);
		
		func_ret=(func_ret==RET_SUCCESS)?RET_SUCCESS:RET_FAILURE;
		
        
    }
}

static void vbi_cc_output_proc(UINT8 field_parity , UINT16 data)
{
    struct vpo_io_cc cc;

    MEMSET(&cc,0,sizeof(struct vpo_io_cc));

    RET_CODE func_ret = 0;

    cc.field_parity = field_parity;
    cc.data= data;
    func_ret = vpo_ioctl((struct vpo_device *)dev_get_by_type
             (NULL, HLD_DEV_TYPE_DIS), VPO_IO_WRITE_CC, (UINT32)&cc);
	func_ret=(func_ret==RET_SUCCESS)?RET_SUCCESS:RET_FAILURE;
}

static void vbi_wss_proc(UINT8 data)
{
     RET_CODE func_ret = 0;

    func_ret = vpo_ioctl((struct vpo_device *)dev_get_by_type
            (NULL, HLD_DEV_TYPE_DIS), VPO_IO_WRITE_WSS, data);
	func_ret=(func_ret==RET_SUCCESS)?RET_SUCCESS:RET_FAILURE;
}

static void set_vbi_startline(UINT8 line)
{
    RET_CODE func_ret = 0;

    func_ret = vpo_ioctl((struct vpo_device *)dev_get_by_type
            (NULL, HLD_DEV_TYPE_DIS), VPO_IO_SET_VBI_START_LINE, line);
	
	func_ret=(func_ret==RET_SUCCESS)?RET_SUCCESS:RET_FAILURE;
}

static void clear_ttx_data(void)
{
    UINT32 i = 0;
    UINT32 j = 0;

    for(j=0;j<TTX_BUF_DEPTH;j++)
    {
        for(i=0;i<45;i++)
        {
            vbi_output_proc(j+TTX_START_LINE, i, 0x00);
        }
    }

}

void vbi_m3327_setoutput(__attribute__((unused)) struct vbi_device *dev,t_vbirequest *p_vbi_request)
{
    *p_vbi_request = request_vbidata;
}

static INLINE void vbi_queue_get(void)
{
    
    *vbi_queue_rd=QUEUE_NEXT(*vbi_queue_rd);
  
}

static UINT16 vbi_queue_rd_ptr(void)
{
   
    return *vbi_queue_rd;
}

static BOOL vbi_queue_is_empty(void)
{
   
   return QUEUE_EMPTY(*vbi_queue_rd,*vbi_queue_wr);
}


static BOOL send_line_data
       (UINT8 line_id,UINT8 *p_data, UINT8 unit_id,UINT8 unit_len)
{//FieldPolar 1: TOP (7-22), 0 : BOT (320-335)
    UINT32 i = 0;
    UINT8 mychar = 0;
    UINT8 field_polar = 0;
    //UINT8 line_offset = 0;
    UINT8 frame_code = 0;
    UINT16 myword = 0;
    UINT16 byte_addr=0;
    UINT16 line_addr = 0;

    if(unit_len != 0)
    {
        field_polar = ((*p_data)&0x20)>>5;
        //line_offset = *p_data&0x1F;//cloud
        frame_code = *(p_data+1);

        line_addr = line_id;//line_id+6;
        p_data++;
        switch(unit_id)
        {
            case 0x02:
            case 0x03:
                if(TRUE == g_ttx_by_vbi)
                {
                    if(unit_len>1)
                    {
                        p_data++;//frame_code
                        vbi_output_proc(line_addr,byte_addr++, 0xAA);//clock
                        vbi_output_proc(line_addr,byte_addr++, 0xAA);
                        //vbi_output_proc(line_addr,byte_addr++, 0xE4);
                        //frame code
                        vbi_output_proc(line_addr,byte_addr++, frame_code);
                        //frame code
                        for(i=0;i<(UINT32)(unit_len-2);i++)
                        {
                            vbi_output_proc(line_addr,byte_addr++, *p_data++);
                        }
                    }
                }
                return TRUE;

            case 0xC3:
                if(TRUE == g_vbi27_pconfig_par->vps_by_vbi)
                {
                    //VBI_PRINTF("0xc3\n");
                    p_data+=(unit_len-1);
                }
            break;

            case 0xC4:
                if(TRUE == g_vbi27_pconfig_par->wss_by_vbi)
                {
                    //VBI_PRINTF("0xc4\n");
                    if(unit_len>TTX_NORMAL_2)
                    {
                        mychar = *p_data++; //first byte
                        myword = mychar+(*p_data++<<8);
                        vbi_wss_proc(myword);
                    }
                    else
                    {
                        p_data++;
                    }
                }
            break;

            case 0xC5:
                if(TRUE == g_vbi27_pconfig_par->cc_by_vbi)
                {
                    //VBI_PRINTF("0xc5\n");
                    if(unit_len>TTX_NORMAL_2)
                    {
                        mychar = *p_data++; //first byte
                        myword = mychar+(*p_data++<<8);
                        vbi_cc_output_proc(field_polar,myword);
                    }
                    else
                    {
                        p_data++;
                    }
                }

            break;

            case 0xC6:
                //VBI_PRINTF("0xc6\n");
                p_data+=(unit_len-1);
            break;

            default:
                //VBI_PRINTF("%x\n",unit_id);
                p_data+=(unit_len-1);
            break;
        }
    }
    return FALSE;
}
static struct vbi_data_array_t temp_vbi_array[TTX_BUF_DEPTH];

static UINT8 got_line_cnt=0;


static UINT8 last_line_offset=0;

static UINT8 line_data_value[TTX_BUF_DEPTH]={0}; 

static void feed_data_hsr(UINT8 field_polar)
{


	UINT8 i=0;
	
    UINT16 rd = 0;
   
    UINT8 send_line_cnt=0;
	
    UINT8 max_line_num = 0;
	
    UINT8 line_offset=0;
     
   //UINT8 real_field = 0;   
	
    struct vpo_io_get_info dis_info;
   
	UINT8 start_line =0;
	
	//UINT8 field =0;
	
	UINT8 ntsc = 0;
	
	BOOL line_sent_finished=FALSE;
	

	start_line = TTX_START_LINE;
	//field= !(field_polar&1);
    ntsc=field_polar>>4;

	
	MEMSET(&dis_info,0,sizeof(struct vpo_io_get_info));
	

    if(0xFF==field_polar)
    {
        while(TRUE != vbi_queue_is_empty())
        {
            vbi_queue_get();
        }
        return;
    }

   
	if(0 == ntsc)
		max_line_num = 16;
	else
		max_line_num = 13;
	

    if(TRUE == vbi_queue_is_empty())
    {
    	//libc_printf("%d\n",__LINE__);
        return;
    }
    
    vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_GET_INFO, (UINT32) &dis_info);
    if((dis_info.tvsys == NTSC)        ||
       (dis_info.tvsys == LINE_720_30) ||
       (dis_info.tvsys == LINE_1080_30)||
       (dis_info.tvsys == LINE_1080_60))
    {
        
        if(g_ttx_tve_id == 0)
        {
            start_line = TTX_START_LINE_NTSC_HD;
        }
		else
			start_line = TTX_START_LINE;
		
    }
	if((ntsc==0)&&(g_ttx_tve_id == 0))
		start_line=TTX_START_LINE+1;
	

    set_vbi_startline(start_line-1);
   
	if(TRUE == g_ttx_by_vbi)
    {
        clear_ttx_data();
    }
  	while(1)
    {  
    
		if(TRUE == vbi_queue_is_empty())				
		{	
			
			//libc_printf("\n*************************************\n");
			break;
		}
		
		if(send_line_cnt<=max_line_num)
		{	
		
    		
					
    		rd = vbi_queue_rd_ptr();			
    					
    			
    		line_offset = *(g_see_vbi_data_array[rd].vbi_data)&0x1F;
    		//real_field = (*(g_see_vbi_data_array[rd].vbi_data)&0x20)>>5;
    		
    		//libc_printf("now:%d-last:%d\n",line_offset,last_line_offset);
    		
    		if((line_offset<TTX_NORMAL_7) || (line_offset>TTX_NORMAL_22))
            {
                        
    		    vbi_queue_get();					
    		    continue;
            } 
    						
         	if((last_line_offset!=0)&&(last_line_offset>=line_offset))
         	{
         		//libc_printf("last%d-now%d\n",last_line_offset,line_offset);
         		last_line_offset=0;
    			line_sent_finished=TRUE;
         		//libc_printf("\n&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&\n");
         		break;
         	}
    		line_sent_finished=FALSE;
			
    		last_line_offset=line_offset;
			
    		
			MEMCPY(&(temp_vbi_array[got_line_cnt]),&(g_see_vbi_data_array[rd]),sizeof(struct vbi_data_array_t));

			
    		line_data_value[got_line_cnt] = line_offset;
			
    		got_line_cnt++;

			//libc_printf("got_line_cnt %d\n",got_line_cnt);
    		
    		vbi_queue_get();
    				
            send_line_cnt=line_offset-7+1;                  
           
		
		}
		else
		{
			break;
		}
			
    }  
	if(line_sent_finished==TRUE)
	{
		
		for(i=0;i<got_line_cnt;i++)
		{
			send_line_data(line_data_value[i],temp_vbi_array[i].vbi_data,temp_vbi_array[i].unit_id,temp_vbi_array[i].unit_len);
			//libc_printf("send line %d\n",line_data_value[i]);
		}
		got_line_cnt=0;
		
	}
	
    return;

}

void request_vbidata(UINT8 field_polar)
{
    if((TRUE == g_ttx_by_vbi) || (TRUE == g_vbi27_pconfig_par->cc_by_vbi))
    {
        if(VBI_STATE_PLAY == g_vbi_status)
        {
            feed_data_hsr(field_polar);
        }
    }
}

#else   //compiler to main




static INLINE void vbi_queue_init(void)
{
    *vbi_queue_wr = 0;
    
    *vbi_queue_cnt = 0;
	
    vbi_queue_set_rd(0);  
}

//void vbi_see_set_quene_cnt(UINT32 cnt)

static BOOL vbi_queue_is_full(void)
{
    return QUEUE_FULL(*vbi_queue_rd,*vbi_queue_wr);
}

static UINT16 vbi_queue_wr_ptr(void)
{
    return *vbi_queue_wr;
}

static  void vbi_queue_put(void)
{
    *vbi_queue_wr=QUEUE_NEXT(*vbi_queue_wr);
}

void enable_vbi_transfer(BOOL enable)//cloud
{
    enable_vbi = enable;
}



static UINT8 do_line_data(UINT8 *p_data ,UINT32 size, UINT32 *len)
{ //field_polar 1: TOP (7-22), 0 : BOT (320-335)
    UINT8 field_polar = 0;
    UINT8 unit_id = 0;
    UINT32 unit_len = 0;
//    UINT8 line_offset = 0;
    UINT16 wr = 0;

    *len = 0;

    unit_id = *p_data++;

    if(size <= TTX_NORMAL_2)
    {
        return RET_NO_DATA;    //not enough data
    }

    unit_len = *p_data++;

    if(size < unit_len+TTX_NORMAL_2)
    {
        return RET_NO_DATA;//not enough data
    }


    field_polar = (*p_data&0x20)>>5;
//    line_offset = *p_data&0x1F;//cloud

    //p_data has 2 bytes offset
//    if(TRUE == vbi_queue_is_full())
//    {
//        return RET_QUEUE_FULL;
//    }
//    else
//    {
        if((TRUE == g_vbi27_pconfig_par->cc_by_vbi) || (TRUE == g_vbi27_pconfig_par->ttx_by_vbi))
        {
            //if(unit_id != 0xff && unit_len <= 44 && enable_vbi == TRUE)//cloud
            if((vbi_queue_is_full()!=TRUE)&&(((TTX_NORMAL_2 == unit_id)||(TTX_NORMAL_3 == unit_id))
                && (TTX_NORMAL_44==unit_len)) && ( TRUE== enable_vbi))
            {
                wr = vbi_queue_wr_ptr();

                MEMCPY(vbi_data_array[wr].vbi_data,p_data,unit_len);
				
				osal_cache_flush(vbi_data_array[wr].vbi_data,unit_len);
				
                vbi_data_array[wr].vbi_field = field_polar;
                vbi_data_array[wr].unit_id = unit_id;
                vbi_data_array[wr].unit_len = unit_len;

                vbi_queue_put();
            }
        }
#ifdef TTX_BY_OSD
        if(TRUE == (*g_vbi27_ttx_by_osd))
        {
            if(( (0x02== unit_id)|| (0x03==unit_id) ) && (TTX_NORMAL_44==unit_len))
            {
                parse_ttx_data(p_data+2); // unit_len == 44
            }
        }
#endif
//    }

    *len = (unit_len+2);
    return RET_OK;//find OK

}


//static UINT8 lang_char[256];

static void vbi_m3327_software_init(struct vbi_m3327_private *priv)
{
    INT32 i = 0;
//    INT32 func_ret = RET_SUCCESS;

    priv->u_not_enough_data_flag = 0;
    vbi_sbf_create();
    vbi_hdr_buf_create();
#ifdef TTX_BY_OSD
    pbf_create();
#endif
    for(i=0;i<VBI_BS_HDR_LEN;i++)
    {
        vbi_data_hdr[i].b_info_valid = FALSE;
        vbi_data_hdr[i].u_data_size = 0;
    }
    priv->init_hdr.u_data_size = 0;
    priv->p_vbiwr_hdr = &priv->init_hdr;
    priv->p_vbird_hdr = &priv->init_hdr;


    vbi_queue_init();
#ifdef TTX_BY_OSD
    if(TRUE == (*g_vbi27_ttx_by_osd))
    {
        if(NULL != priv->init_ttx_decoder)
        {
            priv->init_ttx_decoder();
        }
    }
#endif
}

static void parse_vbi_stream(struct vbi_m3327_private *priv)
{
    UINT8 ret = 0;

    UINT32 u_hdr_ptr = 0;
    UINT32 u_size = 0;
    UINT8 *pu_data = NULL;

    UINT32 len = 0;

    while(1)
    {
        if(0 == priv->p_vbird_hdr->u_data_size)
        {
            if(RET_SUCCESS == vbi_hdr_buf_rd_req(&u_hdr_ptr))
            {
                priv->p_vbird_hdr = &vbi_data_hdr[u_hdr_ptr];
                // to do : judge pts sync
                vbi_hdr_buf_rd_update();
            }
            else
            {
                //soc_printf("Exit1\n");
                return;//goto ERR_POINT;//return RET_FAILURE;
            }
        }
        if(1 ==priv->u_not_enough_data_flag)
        {
            if(1 == priv->p_vbird_hdr->u_pes_start)
            {
                vbi_sbf_rd_update(priv->u_remain_data_size);
                VBI_PRINTF("5. rd upd \n");
            }
            else if(priv->pu_remain_data_ptr > priv->p_vbird_hdr->pu_start_point)
            {
                VBI_ASSERT(priv->u_remain_data_size<=LOOPBACK_SIZE);
                //vbi_sbf_rd_update(priv->uRemainDataSize);
                //VBI_PRINTF("4. rd upd = %d \n",priv->uRemainDataSize);
                vbi_sbf_loopback(priv->pu_remain_data_ptr,priv->u_remain_data_size,priv->p_vbird_hdr->pu_start_point);
                priv->p_vbird_hdr->u_data_size += priv->u_remain_data_size;
                priv->p_vbird_hdr->pu_start_point -= priv->u_remain_data_size;
            }
            else
            {
                priv->p_vbird_hdr->u_data_size += priv->u_remain_data_size;
                priv->p_vbird_hdr->pu_start_point = priv->pu_remain_data_ptr;
            }
            priv->u_not_enough_data_flag = 0;
        }
        u_size = priv->p_vbird_hdr->u_data_size;
        if(vbi_sbf_rd_req(&u_size,&pu_data) != RET_SUCCESS)
        {
            VBI_PRINTF("No Buffer Data\n");
            VBI_ASSERT(FALSE);
            return;
            //goto ERR_POINT;//return RET_FAILURE;
        }
        else
        {
            VBI_ASSERT(pu_data== priv->p_vbird_hdr->pu_start_point);

            if(1 == priv->p_vbird_hdr->u_pes_start)
            {
                if(0x10 != *pu_data)
                {
                    VBI_PRINTF("this is not a VBI pes!\n");
                    vbi_sbf_rd_update(u_size);
                    priv->p_vbird_hdr->u_data_size = 0;
                    continue;
                    //ASSERT(FALSE);//break;
                }
                VBI_PRINTF("1. rd upd = 1 \n");
                vbi_sbf_rd_update(1);

                priv->p_vbird_hdr->u_data_size -= 1;
                priv->p_vbird_hdr->pu_start_point +=1;
                priv->p_vbird_hdr->u_pes_start = 0;

                continue;
            }

            while(RET_OK == (ret=do_line_data(pu_data, u_size,&len)))
            {
                VBI_PRINTF("2. rd upd = %d \n",len);
                vbi_sbf_rd_update(len);
                priv->p_vbird_hdr->u_data_size -= len;
                priv->p_vbird_hdr->pu_start_point += len;

                pu_data += len;
                u_size -= len;
            }
            if (RET_NO_DATA == ret)//not enough data
            {
                VBI_PRINTF("not enough data\n");
                priv->u_remain_data_size = u_size;
                priv->pu_remain_data_ptr = pu_data;
                priv->u_not_enough_data_flag = 1;
                priv->p_vbird_hdr->u_data_size = 0;
                continue;
            }
            else if(RET_QUEUE_FULL == ret)
            {
                //soc_printf("Exit2\n");
                return;
                //goto ERR_POINT;
            }

        }
    }
//ERR_POINT:
    return;

}

void vbidec_task(UINT32 param1,UINT32 param2)
{
    //OSAL_ER ret;
    OSAL_ER result = param2;
    UINT32 flgptn = 0 ;
    struct vbi_m3327_private *priv = NULL;

    priv = (struct vbi_m3327_private*)(((struct vbi_device*)param1)->priv) ;

    while(1) // run forever when task exist.
    {
        if(VBI_STATE_IDLE == priv->status)
        {
            result = osal_flag_wait(&flgptn,vbidec_flag_id, VBI_CMD_START,OSAL_TWF_ORW,OSAL_WAIT_FOREVER_TIME);

            if(flgptn&VBI_CMD_START)
            {
                VBI_PRINTF("ttx_start_flag!\n");
                vbi_m3327_software_init(priv);
                osal_flag_clear(vbidec_flag_id, VBI_CMD_STOP|VBI_CMD_START|
                               VBI_MSG_DATA_AVAILABLE);
                priv->status = VBI_STATE_PLAY;
                set_vbi_status_to_see(VBI_STATE_PLAY);
                osal_flag_set(vbidec_flag_id,VBI_MSG_START_OK);
            }
        }
        if(VBI_STATE_PLAY == priv->status)
        {
            result = osal_flag_wait(&flgptn,vbidec_flag_id, VBI_CMD_STOP|VBI_MSG_DATA_AVAILABLE, OSAL_TWF_ORW, 20);
            if(E_TIMEOUT == result)
            {
                parse_vbi_stream(priv);
            }
            else if(flgptn&VBI_CMD_STOP)
            {
                VBI_PRINTF("ttx_stop_flag!\n");
                priv->status = VBI_STATE_IDLE;
                set_vbi_status_to_see(VBI_STATE_IDLE);
                osal_flag_clear(vbidec_flag_id, VBI_CMD_STOP|VBI_CMD_START|VBI_MSG_DATA_AVAILABLE);
                #ifdef SUPPORT_TTX_SUBT_TIMER_CLEAN
                // Delete working ttx_subt timer when ttx task need to stop
                if(ttx_subt_alarm_id != OSAL_INVALID_ID)
                {
                    //libc_printf("delte timer forec\n");
                    osal_timer_delete(ttx_subt_alarm_id);
                    ttx_subt_alarm_id = OSAL_INVALID_ID;
                }
                #endif
                flgptn = 0;
                osal_flag_set(vbidec_flag_id,VBI_MSG_STOP_OK);          
            }
            else if(flgptn&VBI_MSG_DATA_AVAILABLE)
            {
                //VBI_PRINTF("update flag received!\n");
                osal_flag_clear(vbidec_flag_id, VBI_MSG_DATA_AVAILABLE);
                parse_vbi_stream(priv);
            }
        }


    }//end main while loop


}
__ATTRIBUTE_REUSE_
INT32 vbi_m3327_open(struct vbi_device *dev)
{
    struct vbi_m3327_private *priv = (struct vbi_m3327_private *)(dev->priv);
    OSAL_T_CTSK t_ctsk;

    MEMSET(&t_ctsk,0,sizeof(OSAL_T_CTSK));
    vbidec_flag_id=osal_flag_create(0x00000000);

    if(OSAL_INVALID_ID==vbidec_flag_id)
    {
         VBI_PRINTF("Create flag of ttxdec  failed!\n");
         VBI_ASSERT(0);
    }

    t_ctsk.task = (FP)vbidec_task;
    t_ctsk.para1 = (UINT32)dev;
    t_ctsk.para2 = 0;
    t_ctsk.stksz = 0x0C00;
    t_ctsk.quantum = 10;
    t_ctsk.itskpri = OSAL_PRI_HIGH; //OSAL_PRI_NORMAL; //OSAL_PRI_HIGH;

#ifdef _TDS_INFO_MONITOR
    t_ctsk.name[0] = 'V';
    t_ctsk.name[1] = 'B';
    t_ctsk.name[2] = '0';
#endif
    vbidec_tsk_id = osal_task_create(&t_ctsk);
    if(OSAL_INVALID_ID==vbidec_tsk_id)
    {
         VBI_PRINTF("Can not create VBIDec_Task!!!\n");
         VBI_ASSERT(0);
    }

    if(NULL != priv->init_cc_decoder)
    {
        priv->init_cc_decoder(dev);
    }

    return RET_SUCCESS;
}

INT32 vbi_m3327_close(struct vbi_device *dev)
{
    if(NULL == dev)
    {
        return !RET_SUCCESS;
    }
    if(OSAL_E_OK!=osal_task_delete(vbidec_tsk_id))
    {
        while(1)
        {
            VBI_PRINTF("Delete ttxdec task of %s failed!\n");
            osal_task_sleep(2000);
        }
    }

    if(OSAL_E_OK!=osal_flag_delete(vbidec_flag_id))
    {
        while(1)
        {
            VBI_PRINTF("Delete ttxdec flag of %s failed!\n");
            osal_task_sleep(2000);
        }
    }
    /* clear vbi data when close*/

    return RET_SUCCESS;
}

INT32  vbi_m3327_start(struct vbi_device *dev,t_ttxdec_cbfunc p_cb_func)
{
    struct vbi_m3327_private *priv = (struct vbi_m3327_private*)(dev->priv) ;
    UINT32 flgptn = 0 ;
    OSAL_ER result = 0;

    if(VBI_STATE_IDLE == priv->status)
    {
#ifdef TTX_BY_OSD
         //MEMSET(g_ttx_p26_nation, 0xff, 25*40*400);
        if(TRUE == g_vbi27_pconfig_par->parse_packet26_enable)
        {
            MEMSET(g_ttx_p26_nation, 0x00, 25*40*400);//cloud
        }
#endif
        osal_flag_set(vbidec_flag_id, VBI_CMD_START);
        flgptn = 0;
        result = osal_flag_wait(&flgptn,vbidec_flag_id, VBI_MSG_START_OK,
                  OSAL_TWF_ORW, OSAL_WAIT_FOREVER_TIME);
        if(OSAL_E_OK!=result)
        {
            VBI_PRINTF("Wait VBI_MSG_START_OK failed with err code %s\n",
                        (result == OSAL_E_TIMEOUT)?"OSAL_E_TIMEOUT":"OSAL_E_FAIL");
            return !RET_SUCCESS;
        }
        osal_flag_clear(vbidec_flag_id, VBI_MSG_START_OK);

#ifdef TTX_BY_OSD
        if(TRUE == (*g_vbi27_ttx_by_osd))
        {
#ifdef SEE_CPU
            p_cbpage_update = ttx_eng_update_page_cb;
            g_vbi_device = dev;
            p_cb_func = ttx_eng_update_page_cb;
#else
            p_cbpage_update = p_cb_func;
#endif
        }
#endif
    }

    return RET_SUCCESS;
}
INT32  vbi_m3327_stop(struct vbi_device *dev)
{
      struct vbi_m3327_private *priv = (struct vbi_m3327_private*)(dev->priv) ;
      UINT32 flgptn = 0 ;
      OSAL_ER result = 0;

      if(VBI_STATE_PLAY == priv->status)
      {
        osal_flag_set(vbidec_flag_id, VBI_CMD_STOP);
        flgptn = 0;
        result = osal_flag_wait(&flgptn,vbidec_flag_id, VBI_MSG_STOP_OK,
                 OSAL_TWF_ORW, OSAL_WAIT_FOREVER_TIME);
        if(OSAL_E_OK!=result)
        {
            VBI_PRINTF("Wait VBI_MSG_START_OK failed with err code %s\n",
            result == OSAL_E_TIMEOUT?"OSAL_E_TIMEOUT":"OSAL_E_FAIL");
            return !RET_SUCCESS;
        }
        osal_flag_clear(vbidec_flag_id, VBI_MSG_STOP_OK);
#ifdef TTX_BY_OSD
        if(TRUE == *g_vbi27_ttx_by_osd)
        {
            p_cbpage_update = NULL;
        }
#endif
      }
       return RET_SUCCESS;
}
/*
void vbi_register_cb(struct vbi_device *dev,t_VBIDecCBFunc p_cb_func)
{
    pCBPageUpdate = p_cb_func;
}
*/

INT32 vbi_m3327_request_write(struct vbi_device *dev,UINT32 u_size_requested,
   struct control_block *pt_data_ctrl_blk,UINT8 **ppu_data,UINT32 *pu_size_got)
{
    UINT32 u_req_size = 0;
    UINT32 u_vbi_req_hdr_wr_ptr = 0;
    //static UINT8 last_stcid =0xff;
   // static UINT32 last_pts = 0;
    struct vbi_m3327_private *priv = (struct vbi_m3327_private*)(dev->priv) ;

    if(VBI_STATE_IDLE == priv->status)
    {
        VBI_PRINTF("VBI_STATE_IDLE!\n");
        return RET_STA_ERR;
    }

    if(0)//(g_is_hdvideo)
    {
        VBI_PRINTF("VBI_HD_ABORD!\n");
        return RET_STA_ERR;
    }

    u_size_requested= (u_size_requested>552)?552:(u_size_requested);

    u_req_size= u_size_requested;
    //VBI_PRINTF("\nD WR REQ(%x):\n",u_size_requested);
    if(RET_FAILURE == vbi_sbf_wr_req(&u_req_size, ppu_data))
    {
        VBI_PRINTF("SBF request write FAILURE.");
        return RET_FAILURE;
    }
    //VBI_PRINTF("size= %x ,wpt = %x\n",u_req_size,*ppu_data);

    if(RET_FAILURE == vbi_hdr_buf_wr_req(&u_vbi_req_hdr_wr_ptr))
    {
        VBI_PRINTF("HDR request write FAILURE.");
        return RET_FAILURE;
    }
    priv->p_vbiwr_hdr = &vbi_data_hdr[u_vbi_req_hdr_wr_ptr];
    priv->p_vbiwr_hdr->pu_start_point = *ppu_data;
    priv->p_vbiwr_hdr->u_data_size = 0;

    if (NULL == pt_data_ctrl_blk)
    {
        priv->p_vbiwr_hdr->b_info_valid = FALSE;
    }
    else
    {
        priv->p_vbiwr_hdr->b_info_valid = TRUE;
        priv->p_vbiwr_hdr->u_pes_start = (0x00 == pt_data_ctrl_blk->data_continue)?0x01:0x00;

        if(1 == pt_data_ctrl_blk->pts_valid)
        {
            last_stcid = pt_data_ctrl_blk->stc_id;
            vbi_last_pts = pt_data_ctrl_blk->pts;
        }
        priv->p_vbiwr_hdr->u_stc_id = last_stcid;
        priv->p_vbiwr_hdr->u_pts = vbi_last_pts;
        pt_data_ctrl_blk->instant_update = 1;
    }


    *pu_size_got = (u_req_size>u_size_requested? u_size_requested : u_req_size);
    return RET_SUCCESS;

}

void vbi_m3327_update_write(struct vbi_device *dev,UINT32 u_data_size)
{
    //Not more than 1 time update .
    struct vbi_m3327_private *priv = (struct vbi_m3327_private*)(dev->priv);

    if(VBI_STATE_IDLE == priv->status)
    {
        VBI_PRINTF("VBI_STATE_IDLE!\n");
        return ;
    }

    if(0)//(g_is_hdvideo)
    {
        VBI_PRINTF("VBI_HD_ABORD!\n");
        return;
    }

    if(!u_data_size)
    {
        return ;
    }
    if(NULL == priv->p_vbiwr_hdr)
    {
        return;
    }

     vbi_sbf_wr_update(u_data_size);

    //VBI_PRINTF("wr upd = %d\n",u_data_size);

    priv->p_vbiwr_hdr->u_data_size += u_data_size;
    vbi_hdr_buf_wr_update();
    priv->p_vbiwr_hdr=NULL;

    osal_flag_set(vbidec_flag_id,VBI_MSG_DATA_AVAILABLE);
    return ;
}

RET_CODE vbi_m3327_ioctl(struct vbi_device *dev,UINT32 cmd,UINT32 param)
{
    RET_CODE result=RET_SUCCESS ;
//    INT32 func_ret = SUCCESS;
    __MAYBE_UNUSED__ struct ttx_page_info *ttx_page_update = NULL;

    if((NULL == dev)||(0xffffffff == param))
    {
        return !RET_SUCCESS;
    }
    //struct vbi_m3327_private *priv = (struct vbi_m3327_private *)(dev->priv);
    switch(cmd)
    {
#ifdef TTX_BY_OSD
    case IO_VBI_WORK_MODE_SEPERATE_TTX_SUBT:
        ttx_eng_set_seperate_ttxsubt_mode(dev, param);
        break;
    case IO_VBI_WORK_MODE_HISTORY :
        ttx_eng_set_ttx_history_mode(dev, param);
        break;
    case IO_VBI_ENGINE_OPEN:
        ttx_eng_open(dev, (param>>16)&0xff, param&0xffff);
        break;
    case IO_VBI_ENGINE_CLOSE:
        ttx_eng_close(dev);
        break;
    case IO_VBI_ENGINE_UPDATE_PAGE:
        ttx_eng_update_page(dev, (param>>8)&0xffff, param&0xff);
        break;
    case IO_VBI_ENGINE_SHOW_ON_OFF:
        ttx_eng_show_onoff(dev, param);
        break;
    case IO_VBI_ENGINE_SEND_KEY:
        ttx_eng_send_key(dev, param);
        break;
    case IO_VBI_ENGINE_GET_STATE:
        *(UINT8 *)(param) = ttx_eng_get_state(dev);
        break;
    case IO_VBI_ENGINE_UPDATE_INIT_PAGE:
        {
            ttx_page_update =(struct ttx_page_info *)(param);
            ttx_eng_update_init_page(dev, ttx_page_update->num, ttx_page_update->page_addr);
        }
        break;
    case IO_VBI_ENGINE_UPDATE_SUBT_PAGE:
        {
            ttx_page_update = (struct ttx_page_info *)(param);
            ttx_eng_update_subt_page(dev, ttx_page_update->num, ttx_page_update->page_addr);
        }
        break;
    case IO_VBI_ENGINE_SET_CUR_LANGUAGE:
        ttx_eng_set_cur_language(param);
        break;
    case IO_TTX_USER_DSG_FONT:
        ttx_eng_set_ttx_dsg_font(param);
        break;
#endif
    case IO_VBI_SELECT_OUTPUT_DEVICE:
        if (g_vbi_priv)
        {
            g_vbi_priv->tve_dev_id = param & 1;
        }
		else
		{
			return !RET_SUCCESS;
		}
        set_tve_id(g_vbi_priv->tve_dev_id);
        break;
    default:
        result=!RET_SUCCESS ;
        break ;
    }
    return result;
}

#endif

