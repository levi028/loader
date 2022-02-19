/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: lib_device_manage.h
*
* Description:
*     ali STB device(NIM) manage unit
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _LIB_DEVICE_MANAGE_H_
#define _LIB_DEVICE_MANAGE_H_

#include <sys_config.h>
#include <api/libpub/lib_frontend.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************
* macro define
********************************************************/
#define DEV_STATUS_NULL           0x00000000

//attribute, byte0:play or record; byte1~3 reserve
#define DEV_FRONTEND_STATUS_PLAY          0x00000001
#define DEV_FRONTEND_STATUS_RECORD      0x00000002

/*CI*/

/*video decoder*/
//type
#define DEV_VDEC_TYPE_SD            0x00000100
#define DEV_VDEC_TYPE_HD            0x00000200
//attribute,byte0:output type; byte1~3 reserve
#define DEV_VDEC_STATUS_MP            0x00000001
#define DEV_VDEC_STATUS_PIP           0x00000002


/*audio decoder*/

/*sound device*/

/*dmx device*/
#define DEV_DMX_STATUS_PLAY          0x00000001
#define DEV_DMX_STATUS_RECORD      0x00000002

#ifdef PUB_PORTING_PUB29
/* For 2 tuner PVR */
#define NIM_STAT_IDLE         0
#define NIM_STAT_PLAY         (1 << 1)
#define NIM_STAT_REC         (1 << 2)
#define NIM_STAT_ERROR         (1 << 3)

//for build
struct nim_lnb_info
{
    UINT8     tuner_type;            //0: empty, 1: fixed, 2: motor

    UINT8    positioner_type;    //0:no positioner, 1:1.2 positioner, 2:1.3 USALS
    //UINT32  longitude;
    //UINT32    latitude;

    //0: not DiSEqC LNB,
    //1:Universal Local freq DiSEqC LNB, 2:DiSEqC LNB Hi local freq,
    //3:DiSEqC LNB lo local freq    UINT8      lnb_type;
    UINT32    lnb_high;
    UINT32    lnb_low;
    UINT8   lnb_power_off;
    /*add for Unicable LNB*/
    UINT8   unicable_pos;   //position:A/B(value:0/1)
    UINT8   unicable_ub;    //user band(slot) or SCR channel:1~8(value:0~7)
    UINT16  unicable_freq;  //centre frequency of user band:1210,1420,...

};

#endif


/*******************************************************
* struct and define
********************************************************/
struct nim_config
{
    struct ft_antenna antenna;
    union ft_xpond xpond;
};

/*
struct dmx_config
{
    UINT32 tp_id;
     struct dmx_sim dmx_sim_info[DEV_DMX_SIM_CNT];
};
*/

typedef BOOL (* device_match_func)(void* dev_handle);

/*******************************************************
* API
********************************************************/

/* Select NIM Device from Device List extend API */
struct nim_device *dev_select_nim_ext(UINT32 sub_type, UINT32 status,device_match_func match_func);

/* Select NIM Device from Device List */
struct nim_device *dev_select_nim(UINT32 sub_type, UINT32 status, struct nim_config *config);

/* Select Video Decoder Device from Device List */
struct vdec_device *dev_select_vdec(UINT32 sub_type, UINT32 status, UINT32 param);

/* Select Dmx Device from Device List */
struct dmx_device *dev_select_dmx(UINT32 sub_type, UINT32 status, UINT32 param);

/* Get Dmx Device Config Info */
INT32 dev_get_nim_config(struct nim_device *nim, UINT32 sub_type, struct nim_config *config);
/* Set Dmx Device Config Info */
INT32 dev_set_nim_config(struct nim_device *nim, UINT32 sub_type, struct nim_config *config);

/* Get Dmx Device Config Info */
INT32 dev_get_dmx_config(struct dmx_device *dmx, UINT32 sub_type, struct dmx_config *config);
/* Set Dmx Device Config Info */
INT32 dev_set_dmx_config(struct dmx_device *dmx, UINT32 sub_type, struct dmx_config *config);
/* Get Device Status */
INT32 dev_get_device_status(UINT32 hld_type, UINT32 sub_type, void *dev, UINT32 *status);
/* Set Device Status */
INT32 dev_set_device_status(UINT32 hld_type, UINT32 sub_type, void *dev, UINT32 status);
/* Get Number of one special type Device*/
UINT8 dev_get_device_cnt(UINT32 hld_type, UINT32 sub_type);
/* Register one special type Device */
INT32 dev_register_device(void *dev, UINT32 hld_type,UINT8 id,UINT32 sub_type);
/* Get special Device sub type*/
UINT32 dev_get_device_sub_type(void *dev, UINT32 hld_type);

/* Get the NIM id of playing channel*/
UINT8 lib_nimng_get_nim_play();



#ifdef __cplusplus
}
#endif

#endif

