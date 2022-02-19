#include <osal/osal.h>
#include <basic_types.h>

#include <hld/hld_dev.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/sdec/sdec.h>

#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libtsi/si_types.h>
//#include <api/libtsi/si_table.h>
////////////ISDBT//////////////////////////
#include <api/libtsi/si_descriptor.h>
#include <api/libisdbtcc/lib_isdbtcc.h>

#ifdef LIB_TSI3_FULL
#include <api/libtsi/si_section.h>
#include <api/libsi/desc_loop_parser.h>
#include <api/libsi/sie_monitor.h>
#endif

#if (!defined(SEE_CPU)) && (ISDBT_CC == 1) 

// need to enable ISDBT_CC 1

struct t_isdbtcc_lang g_isdbtcc_lang[ISDBTCC_LANG_NUM];
UINT8 g_cc_lang_num =0;        // num of total cc lang (valid value as 0-8)
UINT8 g_cur_lang = 0xff;        //cur cc lang (valid value as 0-7)
static UINT16 g_current_pid =0x1fff;
BOOL g_isdbtcc_enable = FALSE;
BOOL g_pvrisdbtcc_enable = FALSE;
//monitor cc via dmx regiser already. TRUE: Enable, FALSE: need to call isdbtcc_enable()
BOOL g_chk_data_comp=FALSE;
//point out - could do isdbtcc_data_comp_desc if matched data_comp_descriptor
//otherwise, no matched data comp (valid range as 0x30-0x37)
static UINT16 dmx_index = 0;
static UINT8 g_temp_com=0;


#ifdef PVR_DYNAMIC_PID_MONITOR_SUPPORT
UINT8 rec_monitor_id_isdbtcc_lang_num[2][2]={{0,0},{0,0}};
struct t_isdbtcc_lang rec_monitor_id_isdbtcc_lang[2][2][ISDBTCC_LANG_NUM];
BOOL g_rec_chk_data_comp=FALSE;
static UINT8 g_rec_temp_com=0;
#endif

#define DATA_CON_COMP_ID     0x08
#define COMP_PRIMARY_ES    0x30
#define ARIB_DMF_VAL        0x03
#define ARIB_TIMING            0x01
#define ARIB_LANG_NUM        0x01
#define ARIB_LANG_TAG        0x00

static BOOL g_is_isdbtcc_available = FALSE;

BOOL isdbtcc_is_available()
{
    return g_is_isdbtcc_available;
}

#ifdef LIB_TSI3_FULL
struct desc_table isdbtcc_desc_info[] = {
    {    STREAM_IDENTIFIER_DESCRIPTOR,0,(desc_parser_t)isdbtcc_stream_iden_desc},
    {    ISDBT_DATA_COMPONENT_DESCRIPTOR,0,(desc_parser_t)isdbtcc_data_comp_desc}
};

INT32 isdbtcc_callback(UINT8 *buff, INT32 len, UINT32 param)
{
    INT32 prog_info_length;
    INT32 es_info_length;
    INT32 i;
    UINT16 es_pid;

    struct pmt_section *pms = (struct pmt_section *)buff;
    struct pmt_stream_info *stream;

    prog_info_length = SI_MERGE_HL8(pms->program_info_length);
    g_is_isdbtcc_available = FALSE;
    for(i=sizeof(struct pmt_section)+prog_info_length-4; i<len-4; i+=es_info_length+sizeof(struct pmt_stream_info))
    {
        stream = (struct pmt_stream_info *)(buff+i);
        es_info_length = SI_MERGE_HL8(stream->es_info_length);
        es_pid = SI_MERGE_HL8(stream->elementary_pid);
        if(PRIVATE_DATA_STREAM==stream->stream_type)
            desc_loop_parser(stream->descriptor, es_info_length, isdbtcc_desc_info, 2, NULL, (void *)&es_pid);
    }
    return SUCCESS;
}

INT32 isdbtcc_unregister(UINT32 monitor_id)
{
    UINT16 stuff_pid = 0x1FFF;
    struct sim_cb_param param;
    if( sim_get_monitor_param(monitor_id, &param) == SUCCESS)
    {
        if(sim_unregister_scb(monitor_id, isdbtcc_callback)!= SUCCESS)
        {
            ICC_PRINTF("%s : Error: isdbtcc fail to unmount to monitor[%d]!\n",__FUNCTION__,monitor_id);
            return ERR_FAILUE;
        }
    }

    ICC_PRINTF("%s : isdbtcc  success to unmount to monitor[%d]!\n",__FUNCTION__,monitor_id);
    isdbtcc_stream_iden_desc(STREAM_IDENTIFIER_DESCRIPTOR, 0, NULL, (void *)&stuff_pid);
    return SUCCESS;
}

static UINT8 isdbtcc_cur_monitor_id = 0xff;
#ifdef NEW_TIMER_REC
static UINT32 isdbtcc_living_monitor_id;
UINT32 g_isdbtcc_is_bg_record = FALSE;

void isdbtcc_set_is_bg_record(BOOL flag)
{
	g_isdbtcc_is_bg_record = flag;
}

static void isdbtcc_set_monitor_id(monitor_id)
{
	isdbtcc_living_monitor_id = monitor_id;
}

UINT32 isdbtcc_get_monitor_id(void)
{
	return isdbtcc_living_monitor_id;
}
#endif
INT32 isdbtcc_register(UINT32 monitor_id)
{
    struct sim_cb_param param;
    if( sim_get_monitor_param(monitor_id, &param) != SUCCESS)
    {
        ICC_PRINTF("%s : Error1: isdbtcc fail to mount to monitor[%d]!\n",__FUNCTION__,monitor_id);
        return ERR_FAILUE;
    }

    isdbtcc_unregister(isdbtcc_cur_monitor_id);

    if(sim_register_scb(monitor_id, isdbtcc_callback, NULL)==SUCCESS)
    {
        dmx_index = param.dmx_idx;
	 #ifdef NEW_TIMER_REC
	 isdbtcc_set_monitor_id(isdbtcc_cur_monitor_id);
	 #endif
        isdbtcc_cur_monitor_id = monitor_id;
        ICC_PRINTF("%s : isdbtcc success to mount to monitor[%d]!\n",__FUNCTION__,monitor_id);
        return SUCCESS;
    }
    ICC_PRINTF("%s : Error2: isdbtcc fail to mount to monitor[%d]!\n",__FUNCTION__,monitor_id);
    return ERR_FAILUE;
}

#endif

static UINT8 cc_dmx_used[3] = {0,0,0};
static RET_CODE cc_unreg_all_dmx()
{
    static struct dmx_device *dmx_dev;
    RET_CODE reg_code = RET_SUCCESS;
    UINT8 i = 0;
    for(i=0; i<3; i++)
    {
        if(cc_dmx_used[i])
        {
            dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, i);
            reg_code = dmx_unregister_service(dmx_dev,ISDBTCC_DMX_SERV_ID);
            if(reg_code != RET_SUCCESS)
            {
                USB_PRINTF("cc unreg dmx %d failed\n",i);
                return !RET_SUCCESS;
            }
            else
            {
                //USB_PRINTF("cc unreg dmx %d\n",i);
            }
            cc_dmx_used[i]=0;
        }
    }

    return reg_code;
}

/////stream_identifier_descriptor/////////
INT32 isdbtcc_stream_iden_desc(UINT8 tag, INT32 desc_length, UINT8 *desc, void *param)
{
    UINT16 pid = *((UINT16*)param);
    UINT8 b_com;

    if(pid <0x20 || pid == 0x1fff)
    {
        #ifdef NEW_TIMER_REC
	 if(g_isdbtcc_is_bg_record == FALSE)
	 #endif
	 {
        static struct sdec_device *ccdec_dev;
        ccdec_dev = (struct sdec_device *)dev_get_by_name("ISDBT_CC_0");
        g_current_pid = 0x1fff;
        g_cc_lang_num = 0;
        g_cur_lang = 0xff;
        if(g_isdbtcc_enable == TRUE)
        {
            sdec_stop(ccdec_dev);
            cc_unreg_all_dmx();
        }
        g_chk_data_comp = FALSE;
	 }
        return SI_SUCCESS;
    }


    if((STREAM_IDENTIFIER_DESCRIPTOR==tag) && (1==desc_length))
    {
        b_com=desc[0];
        if(        (COMP_PRIMARY_ES<=b_com)
            &&     ((COMP_PRIMARY_ES+ISDBTCC_LANG_NUM-1)>=b_com) )
        {
            g_temp_com=b_com;
            g_chk_data_comp=TRUE;    //case as (1). New Com (2).duplicate com
        }
    }
    return SI_SUCCESS;
}

#ifdef PVR_DYNAMIC_PID_MONITOR_SUPPORT
INT32 rec_isdbtcc_stream_iden_desc(UINT16 dmx_id,UINT8 prog_index,UINT8 tag, INT32 desc_length, UINT8 *desc, void *param)
{
    UINT8 lang_idx;
    UINT16 pid = *((UINT16*)param);
    UINT8 b_com;

    //rec_monitor_id_isdbtcc_lang_num[dmx_id][prog_index] = 0;

    if(pid <0x20 || pid == 0x1fff)
    {
        rec_monitor_id_isdbtcc_lang_num[dmx_id][prog_index] = 0;
        return SI_SUCCESS;
    }

    if((STREAM_IDENTIFIER_DESCRIPTOR==tag) && (1==desc_length))
    {
        b_com=desc[0];
        if(        (COMP_PRIMARY_ES<=b_com)
            &&     ((COMP_PRIMARY_ES+ISDBTCC_LANG_NUM-1)>=b_com) )
        {
            g_rec_temp_com=b_com;
            g_rec_chk_data_comp=TRUE;    //case as (1). New Com (2).duplicate com
        }
    }

        if(rec_monitor_id_isdbtcc_lang_num[dmx_id][prog_index]>=ISDBTCC_LANG_NUM)
            return SI_SUCCESS;
    return SI_SUCCESS;

}
INT32 rec_isdbtcc_data_comp_desc(UINT16 dmx_id,UINT8 prog_index,UINT8 tag, INT32 desc_length, UINT8 *desc, void *param)
{
    UINT16 pid = *((UINT16*)param);
    UINT8 bcnt=0;
    UINT16 w_comp_id=0;    //data_component_id (Const as 0x0008)
    UINT8 b_dmf;            // recommand to be 0x0011 (Auto display)
    UINT8 b_timing;        // recommand to be 0x01 (Program synchronous)

    UINT8 b_index;
    BOOL f_new_com=TRUE;

    if(g_rec_chk_data_comp)
    {
        w_comp_id=desc[bcnt];
        w_comp_id=(w_comp_id<<8)|desc[++bcnt];
        if(DATA_CON_COMP_ID!=w_comp_id)    //unsupport DATA_CON_COMP_ID
        {
            g_rec_chk_data_comp=FALSE;
            return SI_SUCCESS;
        }
        b_dmf=((desc[++bcnt]) &0xF0)>>4;
        b_timing=(desc[bcnt])&0x03;

        //check if exist com
        for(b_index=0;b_index<rec_monitor_id_isdbtcc_lang_num[dmx_id][prog_index];b_index++ )
        {
            if(rec_monitor_id_isdbtcc_lang[dmx_id][prog_index][b_index].es_com==g_rec_temp_com)
            {
                f_new_com=FALSE;
                // need to check pid
                if(rec_monitor_id_isdbtcc_lang[dmx_id][prog_index][b_index].pid!=pid)
                {
                    ICC_PRINTF("\n REC ES COM(%d)PID change to (%d)\n",rec_monitor_id_isdbtcc_lang[dmx_id][prog_index][b_index].pid,pid);
                    // need to update monitor pid
                    rec_monitor_id_isdbtcc_lang[dmx_id][prog_index][b_index].b_dmf= b_dmf;
                    rec_monitor_id_isdbtcc_lang[dmx_id][prog_index][b_index].b_timing= b_timing;
                    rec_monitor_id_isdbtcc_lang[dmx_id][prog_index][b_index].pid=pid;
                }
            }
        }

        if(TRUE==f_new_com)
        {
            rec_monitor_id_isdbtcc_lang[dmx_id][prog_index][rec_monitor_id_isdbtcc_lang_num[dmx_id][prog_index]].es_com= g_rec_temp_com;
            rec_monitor_id_isdbtcc_lang[dmx_id][prog_index][rec_monitor_id_isdbtcc_lang_num[dmx_id][prog_index]].b_dmf= b_dmf;
            rec_monitor_id_isdbtcc_lang[dmx_id][prog_index][rec_monitor_id_isdbtcc_lang_num[dmx_id][prog_index]].b_timing= b_timing;
            rec_monitor_id_isdbtcc_lang[dmx_id][prog_index][rec_monitor_id_isdbtcc_lang_num[dmx_id][prog_index]].pid= pid;
            rec_monitor_id_isdbtcc_lang_num[dmx_id][prog_index]++;
            ICC_PRINTF("\nRec Lang Num=%d,receive=%x",rec_monitor_id_isdbtcc_lang_num[dmx_id][prog_index],g_rec_temp_com);
            if(rec_monitor_id_isdbtcc_lang_num[dmx_id][prog_index]==1)
            {
                //isdbtcc_set_language(0);
            }
        }

        g_rec_temp_com=0;
        g_rec_chk_data_comp=FALSE;

    }
    return SI_SUCCESS;
}
#endif


INT32 isdbtcc_data_comp_desc(UINT8 tag, INT32 desc_length, UINT8 *desc, void *param)
{
    UINT16 pid = *((UINT16*)param);
    UINT8 bcnt=0;
    UINT16 w_comp_id=0;    //data_component_id (Const as 0x0008)
    UINT8 b_dmf;            // recommand to be 0x0011 (Auto display)
    UINT8 b_timing;        // recommand to be 0x01 (Program synchronous)

    UINT8 b_index;
    BOOL f_new_com=TRUE;

    g_is_isdbtcc_available = TRUE;
    if(g_chk_data_comp)
    {
        w_comp_id=desc[bcnt];
        w_comp_id=(w_comp_id<<8)|desc[++bcnt];
        if(DATA_CON_COMP_ID!=w_comp_id)    //unsupport DATA_CON_COMP_ID
        {
            g_chk_data_comp=FALSE;
            return SI_SUCCESS;
        }
        b_dmf=((desc[++bcnt]) &0xF0)>>4;
        b_timing=(desc[bcnt])&0x03;

        //check if exist com
        for(b_index=0;b_index<g_cc_lang_num;b_index++ )
        {
            if(g_isdbtcc_lang[b_index].es_com==g_temp_com)
            {
                f_new_com=FALSE;
                // need to check pid
                if(g_isdbtcc_lang[b_index].pid!=pid)
                {
                    // need to update monitor pid
                    ICC_PRINTF("\n ES COM(%d)PID change to (%d)\n",g_isdbtcc_lang[b_index].pid,pid);
                }
            }
        }

        if(TRUE==f_new_com)
        {
            g_isdbtcc_lang[g_cc_lang_num].es_com= g_temp_com;
            g_isdbtcc_lang[g_cc_lang_num].b_dmf= b_dmf;
            g_isdbtcc_lang[g_cc_lang_num].b_timing= b_timing;
            g_isdbtcc_lang[g_cc_lang_num].pid= pid;
            g_cc_lang_num++;

            #if 0    //self debug code only, to check CC menu display (8 CC Service + 1 CC OFF) only
            unsigned char bi;
            for(bi=0;bi<7;bi++)
            {
                g_isdbtcc_lang[bi+1]=g_isdbtcc_lang[0];
                g_isdbtcc_lang[bi+1].pid=100+bi+1;
            }
            g_cc_lang_num=8;
            #endif
            ICC_PRINTF("\nLang Num=%d,receive=%x",g_cc_lang_num,g_temp_com);
            if(g_cc_lang_num==1)
                isdbtcc_set_language(0);
        }

        g_temp_com=0;
        g_chk_data_comp=FALSE;

    }
    return SI_SUCCESS;
}

INT32 isdbtcc_eit_data_content_desc(UINT8* desc_buf, UINT16 buf_len)
{
    UINT16 w_comp_id=0;    //data_component_id (Const as 0x0008)
    UINT8 b_com=0;        //component_tag
    UINT8 b_num=0;        //num languages (recommand to be 1)
    UINT8 b_tag=0;        //language tag (recommand to be 0)
    UINT8 b_dmf=0;        //display mode (recommand to be 0x0011- auto display)
    UINT8 b_err=0;
    UINT8 *p = desc_buf;
    UINT8 b_index;

    p += 2;
    w_comp_id=p[0];
    w_comp_id= (w_comp_id << 8)|p[1];
    if (DATA_CON_COMP_ID!=w_comp_id)    //incorrect data component ID
        return SI_SUCCESS;
    p+=2;
    b_com=*p;            //Range as  0x30~0x37. and Primary ES=0x30
    if((COMP_PRIMARY_ES>b_com) ||((COMP_PRIMARY_ES+ISDBTCC_LANG_NUM-1)<b_com) )
        return SI_SUCCESS;    //out of valid range
    p+=2;
    b_num=*p;
    if(ARIB_LANG_NUM!=b_num)    //only one languaue per ES
        b_err|=0x04;
    p+=1;
    b_tag=(0xE0&*p)>>5;     //b7-b5
    b_dmf=(0x0F&*p);        //b3-b0
    if(ARIB_LANG_TAG!=b_tag)    // 1st language
        b_err|=0x08;
    if(ARIB_DMF_VAL!=b_dmf)    // DMF as Auto display
        b_err|=0x10;
    p+=1;
    for(b_index=0;b_index<g_cc_lang_num;b_index++ )
    {
        if(g_isdbtcc_lang[b_index].es_com==b_com)    //same comp
        {
            MEMCPY(g_isdbtcc_lang[b_index].lang,p,3);    //fill lang code
            ICC_PRINTF("\nEIT_Com(%d)=[%c %c %c]",b_com,p[0],p[1],p[2]);
        }
    }

    if(0==b_err)
        return SI_SUCCESS;
    else
        return SI_SUCCESS;
}


void isdbtcc_get_language(struct t_isdbtcc_lang** list ,UINT8* num)
{
    *list = g_isdbtcc_lang;
    *num = g_cc_lang_num;
}

#ifdef PVR_DYNAMIC_PID_MONITOR_SUPPORT
void rec_monitor_isdbtcc_get_language(UINT16 dmx_id,UINT8 prog_index,struct t_isdbtcc_lang** list ,UINT8* num)
{
    *num = rec_monitor_id_isdbtcc_lang_num[dmx_id][prog_index];
    *list = rec_monitor_id_isdbtcc_lang[dmx_id][prog_index];
}
#endif

UINT8 isdbtcc_get_cur_language(void)
{
    return g_cur_lang;
}

INT32 isdbtcc_set_language(UINT8 lang)
{
    struct register_service cc_serv;
    static struct sdec_device *ccdec_dev= NULL;
    struct dmx_device *dmx_dev = NULL;
    RET_CODE reg_code = RET_SUCCESS;

    ccdec_dev = (struct sdec_device *)dev_get_by_name("ISDBT_CC_0");
    dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, dmx_index);

    ICC_PRINTF("\nSet Lang (%d)",lang);

    if(lang>=g_cc_lang_num)
        return ERR_FAILUE;

    if(g_isdbtcc_enable == TRUE)
    {
        sdec_stop(ccdec_dev);
    }
    if(g_isdbtcc_lang[lang].pid != g_current_pid)
    {
        g_current_pid = g_isdbtcc_lang[lang].pid;
        if(g_isdbtcc_enable == TRUE)
        {
            //change monitor pid for new lang selected
            cc_unreg_all_dmx();
            cc_serv.device = ccdec_dev;
            cc_serv.request_write = (request_write)sdec_request_write;
            cc_serv.update_write = (update_write)sdec_update_write;
            cc_serv.service_pid = g_current_pid;
            reg_code = dmx_register_service(dmx_dev,ISDBTCC_DMX_SERV_ID,&cc_serv);
            if(reg_code == RET_SUCCESS)
            {
                cc_dmx_used[dmx_index] = 1;
                //USB_PRINTF("CC_set_lan reg %d\n",dmx_index);
            }
            else
            {
                //USB_PRINTF("CC_set_lan reg Fail%d\n",dmx_index);
            }
        }
    }
    if(g_isdbtcc_enable == TRUE)
    {
        sdec_start(ccdec_dev,0,0);
    }
    g_cur_lang = lang;

    return SUCCESS;
}

INT32 isdbtcc_show_onoff(BOOL b_on)
{    
    if(b_on == TRUE)
    {
#ifdef _MHEG5_SUPPORT_
        tm_mheg5stop_2();
#endif
        ICC_PRINTF("\nShow ON");
		g_is_isdbtcc_available=TRUE;
        osd_isdbtcc_enter();
    }
    else
    {
#ifdef _MHEG5_SUPPORT_
        tm_mheg5start_2();
#endif
        ICC_PRINTF("\nShow OFF");
		g_is_isdbtcc_available=FALSE;
        osd_isdbtcc_leave();
    }

    return 0;
}

BOOL isdbtcc_check_enable()
{
    return g_isdbtcc_enable ;
}

void isdbtcc_pvr_set_language(struct t_isdbtcc_lang* list ,UINT8 num)
{
    UINT8 i = 0;
    int n_size = sizeof(struct t_isdbtcc_lang);
    g_cc_lang_num = num;
    ICC_PRINTF("%s : num[%d]!\n",__FUNCTION__,num);
    for(i=0; i<num; i++)
    {
        MEMCPY(&(g_isdbtcc_lang[i]),&(list[i]),n_size);
    }
}
INT32 isdbtcc_pvr_enable(BOOL enable,UINT16 dmx_id)
{
    struct register_service cc_serv;
    static struct sdec_device *ccdec_dev;
    struct dmx_device *dmx_dev = NULL;
       RET_CODE reg_code = RET_SUCCESS;
    ccdec_dev = (struct sdec_device *)dev_get_by_name("ISDBT_CC_0");
    dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, dmx_id);
    ICC_PRINTF("%s : PVR isdbtcc Enable[%d] DMX[%d]!\n",__FUNCTION__,enable,dmx_id);
    if(enable == TRUE)
    {
        dmx_index = dmx_id;
        if(g_cc_lang_num>0 && g_current_pid != 0x1fff)
        {
            dmx_unregister_service(dmx_dev,ISDBTCC_DMX_SERV_ID);
            cc_serv.device = ccdec_dev;
            cc_serv.request_write = (request_write)sdec_request_write;
            cc_serv.update_write = (update_write)sdec_update_write;
            cc_serv.service_pid = g_current_pid;
                   reg_code=dmx_register_service(dmx_dev,ISDBTCC_DMX_SERV_ID,&cc_serv);
                   if(reg_code == RET_SUCCESS)
                   {
                           cc_dmx_used[dmx_index] = 1;
                ICC_PRINTF("%s : subt success to start dmx[%d]!\n",__FUNCTION__,dmx_index);
                   }
            else
            {
                ICC_PRINTF("%s : Error: subt fail to start dmx[%d]!\n",__FUNCTION__,dmx_index);
            }
            sdec_start(ccdec_dev,0,0);
        }
        g_pvrisdbtcc_enable = TRUE;
    }
    else
    {
        if(g_pvrisdbtcc_enable)
            dmx_index = 0;
        dmx_unregister_service(dmx_dev,ISDBTCC_DMX_SERV_ID);
        sdec_stop(ccdec_dev);
        g_current_pid = 0x1fff;
        g_pvrisdbtcc_enable = FALSE;
        g_cc_lang_num = 0;
    }
    return SUCCESS;
}

void isdbtcc_reg_callback(ISDBTCC_EVENT_CALLBACK callback)
{

}

void isdbtcc_unreg_callback()
{

}

INT32 isdbtcc_enable(BOOL enable)
{
    struct register_service cc_serv;
    static struct sdec_device *ccdec_dev;
    struct dmx_device *dmx_dev = NULL;
    RET_CODE reg_code = RET_SUCCESS;

    if(enable)
    {    ICC_PRINTF("CC Enable!!");}
    else
    {    ICC_PRINTF("CC Disable!!");}

    #if 1
    ccdec_dev = (struct sdec_device *)dev_get_by_name("ISDBT_CC_0");
    dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, dmx_index);
    if(enable == TRUE)
    {
        if(g_cc_lang_num>0 && g_current_pid != 0x1fff)
        {
            cc_unreg_all_dmx();
            cc_serv.device = ccdec_dev;
            cc_serv.request_write = (request_write)sdec_request_write;
            cc_serv.update_write = (update_write)sdec_update_write;
            cc_serv.service_pid =g_current_pid;
            reg_code = dmx_register_service(dmx_dev,ISDBTCC_DMX_SERV_ID,&cc_serv);
            if(reg_code == RET_SUCCESS)
            {
                cc_dmx_used[dmx_index] = 1;
                //ICC_PRINTF("subt_enable reg %d\n",dmx_index);
            }
            else
            {
                //ICc_PRINTF("subt_enable reg %d failed\n",dmx_index);
            }
            sdec_start(ccdec_dev,0,0);
        }
        g_isdbtcc_enable = TRUE;
    }
    else
    {
        sdec_stop(ccdec_dev);
        cc_unreg_all_dmx();
        g_current_pid = 0x1fff;
        g_isdbtcc_enable = FALSE;
    }
    #endif
    return SUCCESS;
}

#endif

