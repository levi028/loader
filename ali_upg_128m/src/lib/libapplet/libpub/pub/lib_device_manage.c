/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: lib_device_manage.c
*
* Description:
*     ali STB device(NIM) manage unit
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#include <basic_types.h>
#include <retcode.h>
#include <mediatypes.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/nim/nim_dev.h>
#include <hld/nim/nim.h>
#include <api/libnim/lib_nim.h>
#include <api/libpub/lib_device_manage.h>


/*******************************************************
* macro define
********************************************************/
#define DEV_FRONTEND_MAX_CNT    2
#define DEV_VDEC_MAX_CNT        2
#define DEV_DMX_MAX_CNT         DMX_HW_SW_REAL_NUM
#define DEV_MAX_CNT         (DEV_FRONTEND_MAX_CNT+DEV_VDEC_MAX_CNT+DEV_DMX_MAX_CNT)

#define MAX_NIM_COUNT 2

/*device sub type define*/
#define DEV_MANAG_TYPE_MASK 0x0000FF00
#define DEV_MANAG_TYPE_NULL 0x00

#define ENTER_DEV_MUTEX()  osal_semaphore_capture(dev_manag_sema_id,TMO_FEVR);
#define LEAVE_DEV_MUTEX()  osal_semaphore_release(dev_manag_sema_id)

#ifdef PUB_PORTING_PUB29

#define GET_VALID_TUNER(s_node) \
    ((s_node).tuner1_valid == 1 ? 1 : ((s_node).tuner2_valid == 1 ? 2 : 0))
#endif


/*******************************************************
* structure and enum ...
********************************************************/
struct dev_manag_dev
{
        void *dev_handle;
        /*device type HLD_DEV_TYPE*/
        UINT32 hld_type;
        /*lib device manage defined subtype*/
        UINT32 sub_type;
        /*device manage status*/
        UINT32 status;
        /*device config param or pointer to param struct*/
        UINT32 config;
};


/*******************************************************
* global and local variable
********************************************************/
static UINT16 dev_manag_sema_id = OSAL_INVALID_ID;

static struct dev_manag_dev dev_manag_list[DEV_MAX_CNT];
static struct nim_config dev_nim_info[DEV_FRONTEND_MAX_CNT];
static struct dmx_config dev_dmx_info[DEV_DMX_MAX_CNT];


/*******************************************************
* local function declare
********************************************************/
#if(SYS_PROJECT_FE == PROJECT_FE_DVBT) || (defined(PORTING_ATSC))
static UINT8 nim_mng_get_rec_id(UINT8 nim_stat);
#endif

/*******************************************************
* external API
********************************************************/
/* Select NIM Device from Device List */
struct nim_device *dev_select_nim_ext(UINT32 sub_type, UINT32 status, device_match_func match_func)
{
    UINT8 i = 0;
    struct dev_manag_dev *dev_list = NULL;
    struct nim_device *ret_dev = NULL;

    ENTER_DEV_MUTEX();
    dev_list = &dev_manag_list[0];
    for(i=0; i <DEV_MAX_CNT; i++)
    {
        if((HLD_DEV_TYPE_NIM==dev_list[i].hld_type)
            &&(dev_list[i].sub_type==sub_type)
            &&(dev_list[i].status&status))
        {
            //Check whether NIM Config Info Match or not
            if((match_func != NULL)&&(TRUE==match_func(dev_list[i].dev_handle)))
            {
                ret_dev = (struct nim_device*)dev_list[i].dev_handle;
            }
            else if(NULL == match_func)
            {
                ret_dev = (struct nim_device*)dev_list[i].dev_handle;
            }
            break;
        }
    }
    LEAVE_DEV_MUTEX();
    return ret_dev;
}

/* Select NIM Device from Device List */
struct nim_device *dev_select_nim(UINT32 sub_type, UINT32 status, struct nim_config *config)
{
    UINT8 i = 0;
    struct dev_manag_dev *dev_list = NULL;
    struct nim_device *ret_dev = NULL;
    struct nim_config *pnim_info = NULL;

    ENTER_DEV_MUTEX();
    dev_list = &dev_manag_list[0];
    for(i=0; i < DEV_MAX_CNT; i++)
    {
        if((HLD_DEV_TYPE_NIM==dev_list[i].hld_type)&&(dev_list[i].sub_type==sub_type)&&(dev_list[i].status&status))
        {
            //Check whether NIM Config Info Match or not
            if((config!=NULL)&&(dev_list[i].config!=0))
            {
                pnim_info = (struct nim_config *)(dev_list[i].config);
                if(0==MEMCMP(config, pnim_info, sizeof(struct nim_config)))
                {
                    ret_dev = (struct nim_device*)dev_list[i].dev_handle;
                }
            }
            else
            {
                ret_dev = (struct nim_device*)dev_list[i].dev_handle;
            }
            break;
        }
    }
    LEAVE_DEV_MUTEX();
    return ret_dev;
}

#if 0
/* Select Video Decoder Device from Device List */
struct vdec_device *dev_select_vdec(UINT32 sub_type, UINT32 status, UINT32 param)
{
    return NULL;
}

/* Select Dmx Device from Device List */
struct dmx_device *dev_select_dmx(UINT32 sub_type, UINT32 status, UINT32 param)
{
    return NULL;
}
#endif

/* Get NIM Device Config Info */
INT32 dev_get_nim_config(struct nim_device *nim, UINT32 sub_type, struct nim_config *config)
{
    INT32 ret = RET_FAILURE;
    UINT8 i = 0;
    struct dev_manag_dev *dev_list = NULL;

    ENTER_DEV_MUTEX();
    dev_list = &dev_manag_list[0];
    for(i=0; i < DEV_MAX_CNT; i++)
    {
        if((HLD_DEV_TYPE_NIM==dev_list[i].hld_type)&&(sub_type==dev_list[i].sub_type) \
                &&(nim==(struct nim_device *)dev_list[i].dev_handle))
        {
            if((config!=NULL)&&(dev_list[i].config!=0))
             {
                MEMCPY(config,(UINT8 *)dev_list[i].config, sizeof(struct nim_config));
            }
            ret = RET_SUCCESS;
            break;
        }
    }
    LEAVE_DEV_MUTEX();
    return ret;
}

/* Set NIM Device Config Info */
INT32 dev_set_nim_config(struct nim_device *nim, UINT32 sub_type, struct nim_config *config)
{
    INT32 ret = RET_FAILURE;
    UINT8 i = 0;
    struct dev_manag_dev *dev_list = NULL;

    ENTER_DEV_MUTEX();
    dev_list = &dev_manag_list[0];
    for(i=0; i < DEV_MAX_CNT; i++)
    {
        if((HLD_DEV_TYPE_NIM==dev_list[i].hld_type)&&(sub_type==dev_list[i].sub_type)
                &&(nim==(struct nim_device *)dev_list[i].dev_handle))
        {
            if((config!=NULL)&&(dev_list[i].config!=0))
            {
                MEMCPY((UINT8 *)dev_list[i].config, config,sizeof(struct nim_config));
            }
            ret = RET_SUCCESS;
            break;
        }
    }
    LEAVE_DEV_MUTEX();
    return ret;
}

/* Get Dmx Device Config Info */
INT32 dev_get_dmx_config(struct dmx_device *dmx, UINT32 sub_type, struct dmx_config *config)
{
    INT32 ret = RET_FAILURE;
    UINT8 i = 0;
    struct dev_manag_dev *dev_list = NULL;

    ENTER_DEV_MUTEX();
    dev_list = &dev_manag_list[0];
    for(i=0; i < DEV_MAX_CNT; i++)
    {
        if((HLD_DEV_TYPE_DMX==dev_list[i].hld_type)&&(sub_type==dev_list[i].sub_type)
                &&(dmx==(struct dmx_device *)dev_list[i].dev_handle))
        {
            if((config!=NULL)&&(dev_list[i].config!=0))
            {
                _ts_route_get_dmx_config(dmx, (struct dmx_config*)dev_list[i].config);
                MEMCPY(config,(UINT8 *)dev_list[i].config, sizeof(struct dmx_config));
            }
            ret = RET_SUCCESS;
            break;
        }
    }
    LEAVE_DEV_MUTEX();
    return ret;
}
#if 0
/* Set Dmx Device Config Info */
INT32 dev_set_dmx_config(struct dmx_device *dmx, UINT32 sub_type, struct dmx_config *config)
{
        INT32 ret = -1;
        UINT8 i;
        struct dev_manag_dev *dev_list = NULL;

        ENTER_DEV_MUTEX();
        dev_list = &dev_manag_list[0];
        for(i=0; i < DEV_MAX_CNT; i++)
        {
            if((dev_list[i].hld_type==HLD_DEV_TYPE_DMX)&&(sub_type==dev_list[i].sub_type)
                    &&(dmx==(struct dmx_device *)dev_list[i].dev_handle))
            {
                    if((config!=NULL)&&(dev_list[i].config!=0))
                        MEMCPY((UINT8 *)dev_list[i].config, config,sizeof(struct dmx_config));
                    ret = SUCCESS;
                    break;
            }
        }
        LEAVE_DEV_MUTEX();
        return ret;
}
#endif
/* Get Device Status */
INT32 dev_get_device_status(UINT32 hld_type, UINT32 sub_type, void *dev, UINT32 *status)
{
    INT32 ret = RET_FAILURE;
    UINT8 i = 0;
    struct dev_manag_dev *dev_list = NULL;

    if ((NULL==dev) || (NULL==status))
    {
        return RET_FAILURE;
    }
    ENTER_DEV_MUTEX();
    dev_list = &dev_manag_list[0];
    for(i=0; i < DEV_MAX_CNT; i++)
    {
        if((dev_list[i].hld_type==hld_type)&&(sub_type==dev_list[i].sub_type)
                &&(dev==dev_list[i].dev_handle))
        {
            _ts_route_get_dev_state(hld_type, dev, &dev_list[i].status);
            *status = dev_list[i].status;
            ret = RET_SUCCESS;
            break;
        }
    }
    LEAVE_DEV_MUTEX();
    return ret;
}
#if 0
/* Set Device Status */
INT32 dev_set_device_status(UINT32 hld_type, UINT32 sub_type, void *dev, UINT32 status)
{
        INT32 ret = -1;
        UINT8 i;
        struct dev_manag_dev *dev_list = NULL;

        ENTER_DEV_MUTEX();
        dev_list = &dev_manag_list[0];
        for(i=0; i < DEV_MAX_CNT; i++)
        {
            if((dev_list[i].hld_type==hld_type)&&(sub_type==dev_list[i].sub_type)
                    &&(dev==dev_list[i].dev_handle))
            {
                    dev_list[i].status = status;
                    ret = SUCCESS;
                    break;
            }
        }
        LEAVE_DEV_MUTEX();
        return ret;
}
#endif
/* Get Number of one special type Device*/
UINT8 dev_get_device_cnt(UINT32 hld_type, UINT32 sub_type)
{
    UINT16 i = 0;
    UINT8 cnt = 0;
    struct dev_manag_dev *dev_list = NULL;

    ENTER_DEV_MUTEX();
    dev_list = &dev_manag_list[0];
    for(i=0;i < DEV_MAX_CNT; i++)
    {
        if((dev_list[i].hld_type==hld_type)&&(dev_list[i].sub_type==sub_type)
                &&(dev_list[i].dev_handle!=NULL))
        {
            cnt++;
        }
    }
    LEAVE_DEV_MUTEX();
    return cnt;
}

/* Register one special type Device */
INT32 dev_register_device(void *dev, UINT32 hld_type,UINT8 id,UINT32 sub_type)
{
    UINT8 i = 0;
    INT32 ret = RET_FAILURE;
    struct dev_manag_dev *dev_list = NULL;

	//Device Config Info Initialization
    if(OSAL_INVALID_ID == dev_manag_sema_id)
    {
        dev_manag_sema_id = osal_semaphore_create(1);
        MEMSET(dev_manag_list, 0, sizeof(dev_manag_list));
        MEMSET(dev_nim_info, 0, sizeof(dev_nim_info));
        MEMSET(dev_dmx_info, 0, sizeof(dev_dmx_info));
    }
    if((NULL==dev) || (OSAL_INVALID_ID == dev_manag_sema_id) ||(HLD_DEV_TYPE_INVALID ==hld_type)
    	||(DEV_MAX_CNT < id)||(UINT_MAX == sub_type))
    {
        return ret;
    }

    ENTER_DEV_MUTEX();
    dev_list = &dev_manag_list[0];
    for(i=0;i < DEV_MAX_CNT; i++)
    {
        if((0 == dev_list[i].hld_type)&&(0 == dev_list[i].sub_type)
                &&(NULL == dev_list[i].dev_handle))
        {
            dev_list[i].dev_handle = dev;
            dev_list[i].hld_type = hld_type;
            dev_list[i].sub_type = sub_type;
            dev_list[i].status = DEV_STATUS_NULL;
            if(HLD_DEV_TYPE_NIM == hld_type)
            {
                dev_list[i].config = (UINT32)&dev_nim_info[id];
            }
            else if(HLD_DEV_TYPE_DMX == hld_type)
            {
                dev_list[i].config = (UINT32)&dev_dmx_info[id];
            }
            ret = RET_SUCCESS;
            break;
        }
    }
    LEAVE_DEV_MUTEX();
    return ret;
}

UINT32 dev_get_device_sub_type(void *dev, UINT32 hld_type)
{
    UINT8 i = 0;
    UINT32 sub_type = 0;
    struct dev_manag_dev *dev_list = NULL;

    if(NULL == dev)
    {
        return 0;
    }
    ENTER_DEV_MUTEX();
    dev_list = &dev_manag_list[0];
    for(i=0;i < DEV_MAX_CNT; i++)
    {
        if((dev_list[i].hld_type==hld_type) && (dev_list[i].dev_handle == dev))
        {
            sub_type = dev_list[i].sub_type;
            break;
        }
    }
    LEAVE_DEV_MUTEX();
    return sub_type;
}

/*************************************************************************************
*
*                       Porting API For PUB29
*
*************************************************************************************/
#ifdef PUB_PORTING_PUB29


struct nim_dev_status
{
    UINT8   nim_dev_stat;
    UINT32  prog_id_play;
#if(SYS_PROJECT_FE != PROJECT_FE_DVBT) && (!defined(PORTING_ATSC))
    UINT32  prog_id_record;
#else
    UINT32  prog_id_record[2];
#endif
};

struct _nim_dev_info
{
    struct nim_dev_status   nim_stat;
    struct nim_lnb_info     lnb_info;
};

struct nim_dev_list
{
    UINT8       nim_dev_ns;
    struct _nim_dev_info nim_info[MAX_NIM_COUNT];
};


static struct nim_dev_list  nim_list;

/*************************************************************************************
*           Internal Function
*************************************************************************************/

static UINT8 nim_mng_get_id(UINT8 nim_stat)
{
    UINT8 i = 0;

    for(i = 0; i < nim_list.nim_dev_ns; i++)
    {
        if(nim_list.nim_info[i].nim_stat.nim_dev_stat & nim_stat)
        {
            return (i + 1);
        }
    }
    return 0;

}

/*************************************************************************************
*           External Function
*************************************************************************************/

INT32 lib_nimng_init(UINT8 antenna_connect_type)
{
    INT32 res = RET_FAILURE;

    MEMSET(&nim_list, 0, sizeof(struct nim_dev_list));
#if(SYS_PROJECT_FE != PROJECT_FE_DVBT) && (!defined(PORTING_ATSC))
    if(antenna_connect_type > 0)
    {
        nim_list.nim_dev_ns = 2;
    }
    else
    {
        nim_list.nim_dev_ns = 1;
    }
#else
    nim_list.nim_dev_ns = antenna_connect_type;
#endif

    return nim_list.nim_dev_ns;

}

UINT8 lib_nimng_get_lnb_ns()
{
    return nim_list.nim_dev_ns;
}

UINT8 lib_nimng_get_nim_play()
{
    UINT8 nim_id = 0;

    nim_id = nim_mng_get_id(NIM_STAT_PLAY);
#if(SYS_PROJECT_FE == PROJECT_FE_DVBT) || (defined(PORTING_ATSC))
    if(0 == nim_id)
    {
        nim_id=1;
    }
#endif
    return nim_id;
}

UINT8 lib_nimng_get_nim_rec()
{
    UINT8 nim_id = 0;

#if(SYS_PROJECT_FE != PROJECT_FE_DVBT) && (!defined(PORTING_ATSC))
    nim_id = nim_mng_get_id(NIM_STAT_REC);
#else
    nim_id = nim_mng_get_rec_id(NIM_STAT_REC);
#endif
    return nim_id;
}

UINT8 lib_nimng_request_play(UINT32 prog_id)
{
    S_NODE s_node;
    T_NODE t_node;
    P_NODE p_node, p_r_node;
    UINT8 nim_id_play = 0;
    UINT8 nim_id_rec = 0;
    UINT8 nim_ret = 0;
    UINT8 i = 0;
    UINT8 j = 0;

    MEMSET(&s_node, 0, sizeof(S_NODE));
    MEMSET(&t_node, 0, sizeof(T_NODE));
    MEMSET(&p_node, 0, sizeof(P_NODE));
    //get the sat and TP id
    if(DB_SUCCES != get_prog_by_id(prog_id, &p_node))
    {
        //NIM_MANAGE_DEBUG("p_node fail \n");
        return 0;
    }
#if(SYS_PROJECT_FE != PROJECT_FE_DVBT) && (!defined(PORTING_ATSC))
    if(DB_SUCCES != get_sat_by_id(p_node.sat_id, &s_node))
    {
        return 0;
    }
#else
    if(DB_SUCCES != get_tp_by_id(p_node.tp_id, &t_node))
    {
        //NIM_MANAGE_DEBUG("t_node fail \n");
        return 0;
    }
#endif
    nim_id_play = nim_mng_get_id(NIM_STAT_PLAY);

#if(SYS_PROJECT_FE != PROJECT_FE_DVBT) && (!defined(PORTING_ATSC))
    nim_id_rec  = nim_mng_get_id(NIM_STAT_REC);

    //the prog is only at a tuner
    if(nim_list.nim_dev_ns < MAX_NIM_COUNT)
    {
        nim_ret = 1;
    }
    else if(1 == (s_node.tuner1_valid + s_node.tuner2_valid))
    {
        nim_ret = GET_VALID_TUNER(s_node);

    }
    else if((1 == s_node.tuner1_valid) && (1 == s_node.tuner2_valid))
    {
        nim_ret = (nim_id_rec > 0) ? (3 - nim_id_rec) : 1;
    }

    if(nim_ret > 0)
    {
        //if the same nim
        if(nim_ret == nim_id_rec)
        {
            if(DB_SUCCES == get_prog_by_id(nim_list.nim_info[nim_ret - 1].nim_stat.prog_id_record, &p_r_node))
            {
                if(p_r_node.tp_id != p_node.tp_id)
                {
                    nim_ret = 0;
                }
            }
        }
    }

    //update the nim status
    if(nim_ret > 0)
    {
        if(nim_id_play > 0)
        {
            nim_list.nim_info[nim_id_play - 1].nim_stat.prog_id_play = 0;
            nim_list.nim_info[nim_id_play - 1].nim_stat.nim_dev_stat &= ~NIM_STAT_PLAY;
        }

        nim_list.nim_info[nim_ret - 1].nim_stat.prog_id_play = prog_id;
        nim_list.nim_info[nim_ret - 1].nim_stat.nim_dev_stat |= NIM_STAT_PLAY;

    }

    //libc_printf("%s: nim id = %d, prog_id = x%x\n", __FUNCTION__, nim_ret, prog_id);

    return nim_ret;
#else
    nim_id_rec  = lib_nimng_get_nim_rec(NIM_STAT_REC);

    if(1 == lib_nimng_get_lnb_ns())
    {
        nim_id_play = 1;
    }
    else if(0 == nim_id_play)
    {
        nim_id_play = 1;
    }
    else
    {
        switch(nim_id_rec)
        {
            case 0 : //rec_nim_num = 0  // no tuner is recording
                nim_id_play=1;
                break;
            case 1 :
            case 2 :// if tuner1 is recording , play is another
                for(i=0; i<MAX_NIM_COUNT; i++)
                {
                    if(DB_SUCCES == get_prog_by_id(nim_list.nim_info[nim_id_rec - 1].nim_stat.prog_id_record[i],
                    &p_r_node))
                    {
                        if(p_r_node.tp_id != p_node.tp_id)
                        {
                            nim_id_play = (nim_id_rec == 1) ? 2 : 1;
                        }
                        else if((nim_list.nim_info[nim_id_rec - 1].nim_stat.prog_id_record[i]!=0)
                                &&(nim_list.nim_info[nim_id_rec - 1].nim_stat.prog_id_record[i]!=prog_id))
                        {
                            nim_id_play = (nim_id_rec == 1) ? 2 : 1;
                        }
                    }
                }
                break;
            case 3 :
                // Check every prog_id_record of nim_info
                for(i=0; i<MAX_NIM_COUNT; i++)
                {
                    for(j=0; j<MAX_NIM_COUNT; j++)
                    {
                        if(DB_SUCCES == get_prog_by_id(nim_list.nim_info[i].nim_stat.prog_id_record[j], &p_r_node))
                        {
                            if(p_r_node.tp_id == p_node.tp_id)
                            {
                                nim_id_play=i+1;
                                break;
                            }
                        }
                    }
                    }
                break;
            default:
                break;
        }
    }

    if(lib_nimng_get_lnb_ns()>1)
    {
        nim_list.nim_info[((nim_id_play == 1) ? 2 : 1) - 1].nim_stat.prog_id_play = 0;
        nim_list.nim_info[((nim_id_play == 1) ? 2 : 1) - 1].nim_stat.nim_dev_stat &= ~NIM_STAT_PLAY;
    }
    else
    {
        if(nim_id_rec != 0)
        {
            // check p_node of 2 recorders @ same nim_id
            for(i=0; i<MAX_NIM_COUNT;i++)
            {
                if(DB_SUCCES == get_prog_by_id(nim_list.nim_info[nim_id_rec - 1].nim_stat.prog_id_record[i], &p_r_node))
                {
                    if(p_r_node.tp_id != p_node.tp_id)
                    {
                        nim_id_play = 0xff;
                        return nim_id_play;
                    }
                }
            }
        }
    }
    nim_list.nim_info[nim_id_play - 1].nim_stat.prog_id_play = prog_id;
    nim_list.nim_info[nim_id_play - 1].nim_stat.nim_dev_stat |= NIM_STAT_PLAY;

    return nim_id_play;

#endif

}


UINT8 lib_nimng_request_rec(UINT32 prog_id)
{
    S_NODE s_node;
    T_NODE t_node;
    P_NODE p_node;
    P_NODE p_r_node;
    UINT8 nim_id_play = 0;
    UINT8 nim_id_rec = 0;
    UINT8 nim_ret = 0;

    MEMSET(&s_node, 0, sizeof(S_NODE));
    MEMSET(&t_node, 0, sizeof(T_NODE));
    MEMSET(&p_node, 0, sizeof(P_NODE));
    MEMSET(&p_r_node, 0, sizeof(P_NODE));
    //get the sat and TP id
    if(DB_SUCCES != get_prog_by_id(prog_id, &p_node))
    {
        return 0;
    }
#if(SYS_PROJECT_FE != PROJECT_FE_DVBT) && (!defined(PORTING_ATSC))
    if(DB_SUCCES != get_sat_by_id(p_node.sat_id, &s_node))
    {
        return 0;
    }
#else
    if(DB_SUCCES != get_tp_by_id(p_node.tp_id, &t_node))
    {
        return 0;
    }
#endif
    //get_tp_by_id(prog_id, &t_node);

    nim_id_play = nim_mng_get_id(NIM_STAT_PLAY);
#if(SYS_PROJECT_FE != PROJECT_FE_DVBT) && (!defined(PORTING_ATSC))
    nim_id_rec  = nim_mng_get_id(NIM_STAT_REC);


    //the prog is only at a tuner
    if(nim_list.nim_dev_ns < MAX_NIM_COUNT)
    {
        nim_ret = 1;
    }

    else if(1 == (s_node.tuner1_valid + s_node.tuner2_valid))
    {
        nim_ret = GET_VALID_TUNER(s_node);

    }
    //two same tuner
    else if((1 == s_node.tuner1_valid) && (1 == s_node.tuner2_valid))
    {
        nim_ret = (nim_id_play > 0) ? (3 - nim_id_play) : 2;
    }


    if(nim_id_rec > 0)
    {
        nim_list.nim_info[nim_id_rec - 1].nim_stat.prog_id_play = 0;
        nim_list.nim_info[nim_id_rec - 1].nim_stat.nim_dev_stat &= ~NIM_STAT_REC;
    }

    if(nim_ret > 0)
    {
        nim_list.nim_info[nim_ret - 1].nim_stat.prog_id_record = prog_id;
        nim_list.nim_info[nim_ret - 1].nim_stat.nim_dev_stat |= NIM_STAT_REC;

    }
    return nim_ret;
#else

    nim_id_rec  = lib_nimng_get_nim_rec(NIM_STAT_REC);

    if(1 == lib_nimng_get_lnb_ns())
    {
        nim_ret=2;//nim_ret=nim_id_play;
    }
    else if(0 == nim_id_rec)
    {
        nim_ret=2;//nim_ret=nim_id_play;
    }
    else
    {
        switch(nim_id_rec)
        {
            case 1 :
            case 2 :
                if(DB_SUCCES == get_prog_by_id(nim_list.nim_info[nim_id_rec-1].nim_stat.prog_id_record[0], &p_r_node)||
                    DB_SUCCES == get_prog_by_id(nim_list.nim_info[nim_id_rec-1].nim_stat.prog_id_record[1], &p_r_node))
                {
                    if(p_r_node.tp_id != p_node.tp_id)
                    {
                        nim_ret = (nim_id_rec == 1) ? 2 : 1;
                    }
                    else
                    {
                        nim_ret = nim_id_rec;
                    }
                }
                break;
            case 3 :
                nim_ret=0;
                break;
        }
    }


    if(nim_ret > 0)
    {
        if(0 == nim_list.nim_info[nim_ret - 1].nim_stat.prog_id_record[0])
        {
            nim_list.nim_info[nim_ret - 1].nim_stat.prog_id_record[0] = prog_id;
        }
        else if ((nim_list.nim_info[nim_ret - 1].nim_stat.prog_id_record[0]!=prog_id)
               &&(nim_list.nim_info[nim_ret - 1].nim_stat.prog_id_record[1]==0))
        {
            nim_list.nim_info[nim_ret - 1].nim_stat.prog_id_record[1] = prog_id;
        }
        nim_list.nim_info[nim_ret - 1].nim_stat.nim_dev_stat |= NIM_STAT_REC;

    }
    return nim_ret;

#endif


}
//just for building, which is for -S project
void lib_nimg_set_lnb_info(UINT8 lnb_id, struct nim_lnb_info *lnb_info)
{
    //MEMCPY(&(nim_list.nim_info[lnb_id - 1].lnb_info), lnb_info, sizeof(struct nim_lnb_info));
}


#if(SYS_PROJECT_FE == PROJECT_FE_DVBT) || (defined(PORTING_ATSC))
DB_TP_ID lib_nimng_get_tp_id( UINT8 id)
{
    S_NODE s_node;
    UINT8 nim_id_rec;
    UINT8 nim_ret = 0;
    UINT32 prog_id=0;
    P_NODE p_node;
    //get the sat and TP id

    if (1 == (nim_list.nim_info[id - 1].nim_stat.nim_dev_stat >> 2))
    {
        if(nim_list.nim_info[id - 1].nim_stat.prog_id_record[0]!=0)
        {
            prog_id=nim_list.nim_info[id - 1].nim_stat.prog_id_record[0];
        }
        else
        {
            prog_id=nim_list.nim_info[id - 1].nim_stat.prog_id_record[1];
        }
    }
    else if(1 == (nim_list.nim_info[id - 1].nim_stat.nim_dev_stat >> 1))
    {
        prog_id=nim_list.nim_info[id - 1].nim_stat.prog_id_play;
    }

    if(DB_SUCCES != get_prog_by_id(prog_id, &p_node))
    {
        //NIM_MANAGE_DEBUG("p_node fail \n");
        return 0;
    }

    return p_node.tp_id;
}

UINT32 *lib_nimmng_get_rec_prog_id( UINT8 id)
{
    return nim_list.nim_info[id-1].nim_stat.prog_id_record;
}

/*
    rec_nim_num = 0  // no tuner is recording
    rec_nim_num = 1  // tuner1  is recording
    rec_nim_num = 2  // tuner2  is recording
    rec_nim_num = 3   // tuner1 and tuner2  are recording
*/
static UINT8 nim_mng_get_rec_id(UINT8 nim_stat)
{
    UINT8 i;
    UINT8 rec_nim_num=0;

    for(i = 0; i < nim_list.nim_dev_ns; i++)
    {
        if(1 == lib_nimng_get_lnb_ns())
        {
            i =1;// fixed dmx to record
            if(nim_list.nim_info[i].nim_stat.nim_dev_stat & nim_stat)
            {
                rec_nim_num=(rec_nim_num+i+1);
            }
        }
        else
        {
            if(nim_list.nim_info[i].nim_stat.nim_dev_stat & nim_stat)
            {
                rec_nim_num=(rec_nim_num+i+1);
            }
        }
    }

    return rec_nim_num;

}

UINT8 lib_nimng_release_rec_channel(UINT32 prog_id)
{
    UINT8 nim_id_play, nim_id_rec;
    nim_id_play = nim_mng_get_id(NIM_STAT_PLAY);
    nim_id_rec  = lib_nimng_get_nim_rec(NIM_STAT_REC);
    UINT8 rec_states=0xff;
    if((nim_id_rec > 0)&&(prog_id >0))
    {
        switch(nim_id_rec)
        {
            case 1 :
            case 2 :
                if(nim_list.nim_info[nim_id_rec - 1].nim_stat.prog_id_record[0]==prog_id)
                {
                    nim_list.nim_info[nim_id_rec - 1].nim_stat.prog_id_record[0]= 0;
                }
                else if(nim_list.nim_info[nim_id_rec - 1].nim_stat.prog_id_record[1]==prog_id)
                {
                    nim_list.nim_info[nim_id_rec - 1].nim_stat.prog_id_record[1]= 0;
                }

                if((0 == nim_list.nim_info[nim_id_rec - 1].nim_stat.prog_id_record[0]) \
                    && (0 == nim_list.nim_info[nim_id_rec - 1].nim_stat.prog_id_record[1]))
                {
                    rec_states=nim_id_rec;
                    nim_list.nim_info[nim_id_rec - 1].nim_stat.nim_dev_stat &= ~NIM_STAT_REC;
                }
                break;
            case 3 :
                //check tuner1
                if(nim_list.nim_info[0].nim_stat.prog_id_record[0]==prog_id)
                {
                    nim_list.nim_info[0].nim_stat.prog_id_record[0]= 0;
                }
                else if(nim_list.nim_info[0].nim_stat.prog_id_record[1]==prog_id)
                {
                    nim_list.nim_info[0].nim_stat.prog_id_record[1]= 0;
                }
                if ((nim_list.nim_info[0].nim_stat.prog_id_record[0]== 0)
                  &&(nim_list.nim_info[0].nim_stat.prog_id_record[1]== 0))
                {
                    rec_states=1;
                    nim_list.nim_info[0].nim_stat.nim_dev_stat &= ~NIM_STAT_REC;
                }

                //check tuner2
                if(nim_list.nim_info[1].nim_stat.prog_id_record[0]==prog_id)
                {
                    nim_list.nim_info[1].nim_stat.prog_id_record[0]= 0;
                }
                else if(nim_list.nim_info[1].nim_stat.prog_id_record[1]==prog_id)
                {
                    nim_list.nim_info[1].nim_stat.prog_id_record[1]= 0;
                }
                if((0 == nim_list.nim_info[1].nim_stat.prog_id_record[0]) \
                    &&(0 == nim_list.nim_info[1].nim_stat.prog_id_record[1]))
                {
                    rec_states=2;
                    nim_list.nim_info[1].nim_stat.nim_dev_stat &= ~NIM_STAT_REC;
                }

                break;
            default:
                break;
        }
    }   
    else
    {
        rec_states=0xff;
    }

    return rec_states;
}

#endif

#endif



