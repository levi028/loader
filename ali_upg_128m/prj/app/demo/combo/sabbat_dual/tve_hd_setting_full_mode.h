/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: tve_hd_setting_full_mode.h
*
*    Description:
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef TVE_ADJUST_TABLE_H
#define TVE_ADJUST_TABLE_H

#ifdef TVE_USE_FULL_CURRENT_MODE
#include <hld/dis/vpo.h>

/*************************** 576I.txt *******************************/
T_TVE_ADJUST_ELEMENT table_576i[TVE_ADJ_FIELD_NUM] =
{
    {TVE_COMPOSITE_Y_DELAY          ,   0x3},
    {TVE_COMPOSITE_C_DELAY          ,   0x5},
    {TVE_COMPOSITE_LUMA_LEVEL       ,   0x50},
    {TVE_COMPOSITE_CHRMA_LEVEL      ,   0x64},
    {TVE_COMPOSITE_SYNC_DELAY       ,   0x7},
    {TVE_COMPOSITE_SYNC_LEVEL       ,   0x4},
    {TVE_COMPOSITE_FILTER_C_ENALBE  ,   0x0},
    {TVE_COMPOSITE_FILTER_Y_ENALBE  ,   0x1},
    {TVE_COMPOSITE_PEDESTAL_LEVEL   ,   0x0},


    {TVE_COMPONENT_IS_PAL           ,   0x1},
    {TVE_COMPONENT_PAL_MODE         ,   0x0},
    {TVE_COMPONENT_ALL_SMOOTH_ENABLE,   0x0},
    {TVE_COMPONENT_BTB_ENALBE       ,   0x0},
    {TVE_COMPONENT_INSERT0_ONOFF    ,   0x0},
    {TVE_COMPONENT_DAC_UPSAMPLEN    ,   0x1},
    {TVE_COMPONENT_Y_DELAY          ,   0x0},
    {TVE_COMPONENT_CB_DELAY         ,   0x0},
    {TVE_COMPONENT_CR_DELAY         ,   0x0},
    {TVE_COMPONENT_LUM_LEVEL        ,   0x51},
    {TVE_COMPONENT_CHRMA_LEVEL      ,   0x50},
    {TVE_COMPONENT_PEDESTAL_LEVEL   ,   0xb},
    {TVE_COMPONENT_UV_SYNC_ONOFF    ,   0x0},
    {TVE_COMPONENT_SYNC_DELAY       ,   0x7},
    {TVE_COMPONENT_SYNC_LEVEL       ,   0x3},
    {TVE_COMPONENT_R_SYNC_ONOFF     ,   0x0},
    {TVE_COMPONENT_G_SYNC_ONOFF     ,   0x0},
    {TVE_COMPONENT_B_SYNC_ONOFF     ,   0x0},
    {TVE_COMPONENT_RGB_R_LEVEL      ,   0x46},
    {TVE_COMPONENT_RGB_G_LEVEL      ,   0x46},
    {TVE_COMPONENT_RGB_B_LEVEL      ,   0x46},
    {TVE_COMPONENT_FILTER_Y_ENALBE  ,   0x1},
    {TVE_COMPONENT_FILTER_C_ENALBE  ,   0x1},
    {TVE_COMPONENT_PEDESTAL_ONOFF   ,   0x0},
    {TVE_COMPONENT_PED_RGB_YPBPR_ENABLE,0x0},
    {TVE_COMPONENT_PED_ADJUST       ,   0x0},
    {TVE_COMPONENT_G2Y              ,   0x12a},
    {TVE_COMPONENT_G2U              ,   0x64},
    {TVE_COMPONENT_G2V              ,   0xd0},
    {TVE_COMPONENT_B2U              ,   0x205},
    {TVE_COMPONENT_R2V              ,   0x198},


    {TVE_BURST_POS_ENABLE           ,   0x1},
    {TVE_BURST_LEVEL_ENABLE         ,   0x1},
    {TVE_BURST_CB_LEVEL             ,   0x96},
    {TVE_BURST_CR_LEVEL             ,   0x6a},
    {TVE_BURST_START_POS            ,   0x55},
    {TVE_BURST_END_POS              ,   0x74},
    {TVE_BURST_SET_FREQ_MODE        ,   0x0},
    {TVE_BURST_FREQ_SIGN            ,   0x0},
    {TVE_BURST_PHASE_COMPENSATION   ,   0x70a0},
    {TVE_BURST_FREQ_RESPONSE        ,   0x0},


    {TVE_ASYNC_FIFO                 ,   0x2},
    {TVE_CAV_SYNC_HIGH              ,   0x2},
    {TVE_SYNC_HIGH_WIDTH            ,   0x7},
    {TVE_SYNC_LOW_WIDTH             ,   0xd},


    {TVE_VIDEO_DAC_FS               ,   0x0},
    {TVE_SECAM_PRE_COEFFA3A2        ,   0x0},
    {TVE_SECAM_PRE_COEFFB1A4        ,   0x0},
    {TVE_SECAM_PRE_COEFFB3B2        ,   0x0},
    {TVE_SECAM_F0CB_CENTER          ,   0x0},
    {TVE_SECAM_F0CR_CENTER          ,   0x0},
    {TVE_SECAM_FM_KCBCR_AJUST       ,   0x0},
    {TVE_SECAM_CONTROL              ,   0x0},
    {TVE_SECAM_NOTCH_COEFB1         ,   0x0},
    {TVE_SECAM_NOTCH_COEFB2B3       ,   0x0},
    {TVE_SECAM_NOTCH_COEFA2A3       ,   0x0},


};


/*************************** 480I.txt *******************************/
T_TVE_ADJUST_ELEMENT table_480i[TVE_ADJ_FIELD_NUM] =
{
    {TVE_COMPOSITE_Y_DELAY          ,   0x0},
    {TVE_COMPOSITE_C_DELAY          ,   0x2},
    {TVE_COMPOSITE_LUMA_LEVEL       ,   0x4c},
    {TVE_COMPOSITE_CHRMA_LEVEL      ,   0x5e},
    {TVE_COMPOSITE_SYNC_DELAY       ,   0x1},
    {TVE_COMPOSITE_SYNC_LEVEL       ,   0x7},
    {TVE_COMPOSITE_FILTER_C_ENALBE  ,   0x0},
    {TVE_COMPOSITE_FILTER_Y_ENALBE  ,   0x1},
    {TVE_COMPOSITE_PEDESTAL_LEVEL   ,   0x0},


    {TVE_COMPONENT_IS_PAL           ,   0x0},
    {TVE_COMPONENT_PAL_MODE         ,   0x0},
    {TVE_COMPONENT_ALL_SMOOTH_ENABLE,   0x0},
    {TVE_COMPONENT_BTB_ENALBE       ,   0x0},
    {TVE_COMPONENT_INSERT0_ONOFF    ,   0x0},
    {TVE_COMPONENT_DAC_UPSAMPLEN    ,   0x1},
    {TVE_COMPONENT_Y_DELAY          ,   0x0},
    {TVE_COMPONENT_CB_DELAY         ,   0x0},
    {TVE_COMPONENT_CR_DELAY         ,   0x0},
    {TVE_COMPONENT_LUM_LEVEL        ,   0x51},
    {TVE_COMPONENT_CHRMA_LEVEL      ,   0x50},
    {TVE_COMPONENT_PEDESTAL_LEVEL   ,   0xb},
    {TVE_COMPONENT_UV_SYNC_ONOFF    ,   0x0},
    {TVE_COMPONENT_SYNC_DELAY       ,   0x7},
    {TVE_COMPONENT_SYNC_LEVEL       ,   0x3},
    {TVE_COMPONENT_R_SYNC_ONOFF     ,   0x0},
    {TVE_COMPONENT_G_SYNC_ONOFF     ,   0x0},
    {TVE_COMPONENT_B_SYNC_ONOFF     ,   0x0},
    {TVE_COMPONENT_RGB_R_LEVEL      ,   0x46},
    {TVE_COMPONENT_RGB_G_LEVEL      ,   0x46},
    {TVE_COMPONENT_RGB_B_LEVEL      ,   0x46},
    {TVE_COMPONENT_FILTER_Y_ENALBE  ,   0x1},
    {TVE_COMPONENT_FILTER_C_ENALBE  ,   0x1},
    {TVE_COMPONENT_PEDESTAL_ONOFF   ,   0x1},
    {TVE_COMPONENT_PED_RGB_YPBPR_ENABLE,0x1},
    {TVE_COMPONENT_PED_ADJUST       ,   0xa7},
    {TVE_COMPONENT_G2Y              ,   0x12a},
    {TVE_COMPONENT_G2U              ,   0x64},
    {TVE_COMPONENT_G2V              ,   0xd0},
    {TVE_COMPONENT_B2U              ,   0x205},
    {TVE_COMPONENT_R2V              ,   0x198},


    {TVE_BURST_POS_ENABLE           ,   0x0},
    {TVE_BURST_LEVEL_ENABLE         ,   0x1},
    {TVE_BURST_CB_LEVEL             ,   0xd4},
    {TVE_BURST_CR_LEVEL             ,   0x0},
    {TVE_BURST_START_POS            ,   0x55},
    {TVE_BURST_END_POS              ,   0x74},
    {TVE_BURST_SET_FREQ_MODE        ,   0x0},
    {TVE_BURST_FREQ_SIGN            ,   0x0},
    {TVE_BURST_PHASE_COMPENSATION   ,   0x3220},
    {TVE_BURST_FREQ_RESPONSE        ,   0x0},


    {TVE_ASYNC_FIFO                 ,   0x2},
    {TVE_CAV_SYNC_HIGH              ,   0x2},
    {TVE_SYNC_HIGH_WIDTH            ,   0x7},
    {TVE_SYNC_LOW_WIDTH             ,   0xd},


    {TVE_VIDEO_DAC_FS               ,   0x0},
    {TVE_SECAM_PRE_COEFFA3A2        ,   0x0},
    {TVE_SECAM_PRE_COEFFB1A4        ,   0x0},
    {TVE_SECAM_PRE_COEFFB3B2        ,   0x0},
    {TVE_SECAM_F0CB_CENTER          ,   0x0},
    {TVE_SECAM_F0CR_CENTER          ,   0x0},
    {TVE_SECAM_FM_KCBCR_AJUST       ,   0x0},
    {TVE_SECAM_CONTROL              ,   0x0},
    {TVE_SECAM_NOTCH_COEFB1         ,   0x0},
    {TVE_SECAM_NOTCH_COEFB2B3       ,   0x0},
    {TVE_SECAM_NOTCH_COEFA2A3       ,   0x0},


};


/*************************** 576P.txt *******************************/
T_TVE_ADJUST_ELEMENT table_576p[TVE_ADJ_FIELD_NUM] =
{
    {TVE_COMPOSITE_Y_DELAY          ,   0x0},
    {TVE_COMPOSITE_C_DELAY          ,   0x2},
    {TVE_COMPOSITE_LUMA_LEVEL       ,   0x50},
    {TVE_COMPOSITE_CHRMA_LEVEL      ,   0x68},
    {TVE_COMPOSITE_SYNC_DELAY       ,   0x0},
    {TVE_COMPOSITE_SYNC_LEVEL       ,   0x4},
    {TVE_COMPOSITE_FILTER_C_ENALBE  ,   0x0},
    {TVE_COMPOSITE_FILTER_Y_ENALBE  ,   0x1},
    {TVE_COMPOSITE_PEDESTAL_LEVEL   ,   0x0},


    {TVE_COMPONENT_IS_PAL           ,   0x0},
    {TVE_COMPONENT_PAL_MODE         ,   0x0},
    {TVE_COMPONENT_ALL_SMOOTH_ENABLE,   0x0},
    {TVE_COMPONENT_BTB_ENALBE       ,   0x0},
    {TVE_COMPONENT_INSERT0_ONOFF    ,   0x0},
    {TVE_COMPONENT_DAC_UPSAMPLEN    ,   0x1},
    {TVE_COMPONENT_Y_DELAY          ,   0x1},
    {TVE_COMPONENT_CB_DELAY         ,   0x0},
    {TVE_COMPONENT_CR_DELAY         ,   0x0},
    {TVE_COMPONENT_LUM_LEVEL        ,   0x52},
    {TVE_COMPONENT_CHRMA_LEVEL      ,   0x51},
    {TVE_COMPONENT_PEDESTAL_LEVEL   ,   0xa},
    {TVE_COMPONENT_UV_SYNC_ONOFF    ,   0x0},
    {TVE_COMPONENT_SYNC_DELAY       ,   0x7},
    {TVE_COMPONENT_SYNC_LEVEL       ,   0x2},
    {TVE_COMPONENT_R_SYNC_ONOFF     ,   0x0},
    {TVE_COMPONENT_G_SYNC_ONOFF     ,   0x0},
    {TVE_COMPONENT_B_SYNC_ONOFF     ,   0x0},
    {TVE_COMPONENT_RGB_R_LEVEL      ,   0x46},
    {TVE_COMPONENT_RGB_G_LEVEL      ,   0x46},
    {TVE_COMPONENT_RGB_B_LEVEL      ,   0x46},
    {TVE_COMPONENT_FILTER_Y_ENALBE  ,   0x1},
    {TVE_COMPONENT_FILTER_C_ENALBE  ,   0x1},
    {TVE_COMPONENT_PEDESTAL_ONOFF   ,   0x0},
    {TVE_COMPONENT_PED_RGB_YPBPR_ENABLE,0x0},
    {TVE_COMPONENT_PED_ADJUST       ,   0x0},
    {TVE_COMPONENT_G2Y              ,   0x12a},
    {TVE_COMPONENT_G2U              ,   0x64},
    {TVE_COMPONENT_G2V              ,   0xd0},
    {TVE_COMPONENT_B2U              ,   0x205},
    {TVE_COMPONENT_R2V              ,   0x198},


    {TVE_BURST_POS_ENABLE           ,   0x0},
    {TVE_BURST_LEVEL_ENABLE         ,   0x0},
    {TVE_BURST_CB_LEVEL             ,   0xd4},
    {TVE_BURST_CR_LEVEL             ,   0x0},
    {TVE_BURST_START_POS            ,   0x0},
    {TVE_BURST_END_POS              ,   0x0},
    {TVE_BURST_SET_FREQ_MODE        ,   0x0},
    {TVE_BURST_FREQ_SIGN            ,   0x0},
    {TVE_BURST_PHASE_COMPENSATION   ,   0x0},
    {TVE_BURST_FREQ_RESPONSE        ,   0x0},


    {TVE_ASYNC_FIFO                 ,   0x2},
    {TVE_CAV_SYNC_HIGH              ,   0x2},
    {TVE_SYNC_HIGH_WIDTH            ,   0x7},
    {TVE_SYNC_LOW_WIDTH             ,   0xd},


    {TVE_VIDEO_DAC_FS               ,   0x0},
    {TVE_SECAM_PRE_COEFFA3A2        ,   0x0},
    {TVE_SECAM_PRE_COEFFB1A4        ,   0x0},
    {TVE_SECAM_PRE_COEFFB3B2        ,   0x0},
    {TVE_SECAM_F0CB_CENTER          ,   0x0},
    {TVE_SECAM_F0CR_CENTER          ,   0x0},
    {TVE_SECAM_FM_KCBCR_AJUST       ,   0x0},
    {TVE_SECAM_CONTROL              ,   0x0},
    {TVE_SECAM_NOTCH_COEFB1         ,   0x0},
    {TVE_SECAM_NOTCH_COEFB2B3       ,   0x0},
    {TVE_SECAM_NOTCH_COEFA2A3       ,   0x0},


};


/*************************** 480P.txt *******************************/
T_TVE_ADJUST_ELEMENT table_480p[TVE_ADJ_FIELD_NUM] =
{
    {TVE_COMPOSITE_Y_DELAY          ,   0x0},
    {TVE_COMPOSITE_C_DELAY          ,   0x2},
    {TVE_COMPOSITE_LUMA_LEVEL       ,   0x50},
    {TVE_COMPOSITE_CHRMA_LEVEL      ,   0x68},
    {TVE_COMPOSITE_SYNC_DELAY       ,   0x0},
    {TVE_COMPOSITE_SYNC_LEVEL       ,   0x4},
    {TVE_COMPOSITE_FILTER_C_ENALBE  ,   0x0},
    {TVE_COMPOSITE_FILTER_Y_ENALBE  ,   0x1},
    {TVE_COMPOSITE_PEDESTAL_LEVEL   ,   0x0},


    {TVE_COMPONENT_IS_PAL           ,   0x0},
    {TVE_COMPONENT_PAL_MODE         ,   0x0},
    {TVE_COMPONENT_ALL_SMOOTH_ENABLE,   0x0},
    {TVE_COMPONENT_BTB_ENALBE       ,   0x0},
    {TVE_COMPONENT_INSERT0_ONOFF    ,   0x0},
    {TVE_COMPONENT_DAC_UPSAMPLEN    ,   0x1},
    {TVE_COMPONENT_Y_DELAY          ,   0x1},
    {TVE_COMPONENT_CB_DELAY         ,   0x0},
    {TVE_COMPONENT_CR_DELAY         ,   0x0},
    {TVE_COMPONENT_LUM_LEVEL        ,   0x52},
    {TVE_COMPONENT_CHRMA_LEVEL      ,   0x51},
    {TVE_COMPONENT_PEDESTAL_LEVEL   ,   0xa},
    {TVE_COMPONENT_UV_SYNC_ONOFF    ,   0x0},
    {TVE_COMPONENT_SYNC_DELAY       ,   0x7},
    {TVE_COMPONENT_SYNC_LEVEL       ,   0x2},
    {TVE_COMPONENT_R_SYNC_ONOFF     ,   0x0},
    {TVE_COMPONENT_G_SYNC_ONOFF     ,   0x0},
    {TVE_COMPONENT_B_SYNC_ONOFF     ,   0x0},
    {TVE_COMPONENT_RGB_R_LEVEL      ,   0x46},
    {TVE_COMPONENT_RGB_G_LEVEL      ,   0x46},
    {TVE_COMPONENT_RGB_B_LEVEL      ,   0x46},
    {TVE_COMPONENT_FILTER_Y_ENALBE  ,   0x1},
    {TVE_COMPONENT_FILTER_C_ENALBE  ,   0x1},
    {TVE_COMPONENT_PEDESTAL_ONOFF   ,   0x0},
    {TVE_COMPONENT_PED_RGB_YPBPR_ENABLE,0x0},
    {TVE_COMPONENT_PED_ADJUST       ,   0x0},
    {TVE_COMPONENT_G2Y              ,   0x12a},
    {TVE_COMPONENT_G2U              ,   0x64},
    {TVE_COMPONENT_G2V              ,   0xd0},
    {TVE_COMPONENT_B2U              ,   0x205},
    {TVE_COMPONENT_R2V              ,   0x198},


    {TVE_BURST_POS_ENABLE           ,   0x0},
    {TVE_BURST_LEVEL_ENABLE         ,   0x0},
    {TVE_BURST_CB_LEVEL             ,   0xd4},
    {TVE_BURST_CR_LEVEL             ,   0x0},
    {TVE_BURST_START_POS            ,   0x0},
    {TVE_BURST_END_POS              ,   0x0},
    {TVE_BURST_SET_FREQ_MODE        ,   0x0},
    {TVE_BURST_FREQ_SIGN            ,   0x0},
    {TVE_BURST_PHASE_COMPENSATION   ,   0x0},
    {TVE_BURST_FREQ_RESPONSE        ,   0x0},


    {TVE_ASYNC_FIFO                 ,   0x2},
    {TVE_CAV_SYNC_HIGH              ,   0x2},
    {TVE_SYNC_HIGH_WIDTH            ,   0x7},
    {TVE_SYNC_LOW_WIDTH             ,   0xd},


    {TVE_VIDEO_DAC_FS               ,   0x0},
    {TVE_SECAM_PRE_COEFFA3A2        ,   0x0},
    {TVE_SECAM_PRE_COEFFB1A4        ,   0x0},
    {TVE_SECAM_PRE_COEFFB3B2        ,   0x0},
    {TVE_SECAM_F0CB_CENTER          ,   0x0},
    {TVE_SECAM_F0CR_CENTER          ,   0x0},
    {TVE_SECAM_FM_KCBCR_AJUST       ,   0x0},
    {TVE_SECAM_CONTROL              ,   0x0},
    {TVE_SECAM_NOTCH_COEFB1         ,   0x0},
    {TVE_SECAM_NOTCH_COEFB2B3       ,   0x0},
    {TVE_SECAM_NOTCH_COEFA2A3       ,   0x0},


};


/*************************** 720P_50.txt *******************************/
T_TVE_ADJUST_ELEMENT table_720p_50[TVE_ADJ_FIELD_NUM] =
{
    {TVE_COMPOSITE_Y_DELAY          ,   0x0},
    {TVE_COMPOSITE_C_DELAY          ,   0x2},
    {TVE_COMPOSITE_LUMA_LEVEL       ,   0x50},
    {TVE_COMPOSITE_CHRMA_LEVEL      ,   0x68},
    {TVE_COMPOSITE_SYNC_DELAY       ,   0x0},
    {TVE_COMPOSITE_SYNC_LEVEL       ,   0x4},
    {TVE_COMPOSITE_FILTER_C_ENALBE  ,   0x0},
    {TVE_COMPOSITE_FILTER_Y_ENALBE  ,   0x1},
    {TVE_COMPOSITE_PEDESTAL_LEVEL   ,   0x0},


    {TVE_COMPONENT_IS_PAL           ,   0x0},
    {TVE_COMPONENT_PAL_MODE         ,   0x0},
    {TVE_COMPONENT_ALL_SMOOTH_ENABLE,   0x0},
    {TVE_COMPONENT_BTB_ENALBE       ,   0x0},
    {TVE_COMPONENT_INSERT0_ONOFF    ,   0x0},
    {TVE_COMPONENT_DAC_UPSAMPLEN    ,   0x1},
    {TVE_COMPONENT_Y_DELAY          ,   0x6},
    {TVE_COMPONENT_CB_DELAY         ,   0x5},
    {TVE_COMPONENT_CR_DELAY         ,   0x5},
    {TVE_COMPONENT_LUM_LEVEL        ,   0x52},
    {TVE_COMPONENT_CHRMA_LEVEL      ,   0x51},
    {TVE_COMPONENT_PEDESTAL_LEVEL   ,   0x0},
    {TVE_COMPONENT_UV_SYNC_ONOFF    ,   0x0},
    {TVE_COMPONENT_SYNC_DELAY       ,   0x0},
    {TVE_COMPONENT_SYNC_LEVEL       ,   0x2},
    {TVE_COMPONENT_R_SYNC_ONOFF     ,   0x0},
    {TVE_COMPONENT_G_SYNC_ONOFF     ,   0x0},
    {TVE_COMPONENT_B_SYNC_ONOFF     ,   0x0},
    {TVE_COMPONENT_RGB_R_LEVEL      ,   0x46},
    {TVE_COMPONENT_RGB_G_LEVEL      ,   0x46},
    {TVE_COMPONENT_RGB_B_LEVEL      ,   0x46},
    {TVE_COMPONENT_FILTER_Y_ENALBE  ,   0x1},
    {TVE_COMPONENT_FILTER_C_ENALBE  ,   0x1},
    {TVE_COMPONENT_PEDESTAL_ONOFF   ,   0x0},
    {TVE_COMPONENT_PED_RGB_YPBPR_ENABLE,0x0},
    {TVE_COMPONENT_PED_ADJUST       ,   0x0},
    {TVE_COMPONENT_G2Y              ,   0x12a},
    {TVE_COMPONENT_G2U              ,   0x36},
    {TVE_COMPONENT_G2V              ,   0x88},
    {TVE_COMPONENT_B2U              ,   0x21d},
    {TVE_COMPONENT_R2V              ,   0x1cb},


    {TVE_BURST_POS_ENABLE           ,   0x0},
    {TVE_BURST_LEVEL_ENABLE         ,   0x0},
    {TVE_BURST_CB_LEVEL             ,   0xd4},
    {TVE_BURST_CR_LEVEL             ,   0x0},
    {TVE_BURST_START_POS            ,   0x0},
    {TVE_BURST_END_POS              ,   0x0},
    {TVE_BURST_SET_FREQ_MODE        ,   0x0},
    {TVE_BURST_FREQ_SIGN            ,   0x0},
    {TVE_BURST_PHASE_COMPENSATION   ,   0x0},
    {TVE_BURST_FREQ_RESPONSE        ,   0x0},


    {TVE_ASYNC_FIFO                 ,   0x4},
    {TVE_CAV_SYNC_HIGH              ,   0x2},
    {TVE_SYNC_HIGH_WIDTH            ,   0x7},
    {TVE_SYNC_LOW_WIDTH             ,   0xd},


    {TVE_VIDEO_DAC_FS               ,   0x0},
    {TVE_SECAM_PRE_COEFFA3A2        ,   0x0},
    {TVE_SECAM_PRE_COEFFB1A4        ,   0x0},
    {TVE_SECAM_PRE_COEFFB3B2        ,   0x0},
    {TVE_SECAM_F0CB_CENTER          ,   0x0},
    {TVE_SECAM_F0CR_CENTER          ,   0x0},
    {TVE_SECAM_FM_KCBCR_AJUST       ,   0x0},
    {TVE_SECAM_CONTROL              ,   0x0},
    {TVE_SECAM_NOTCH_COEFB1         ,   0x0},
    {TVE_SECAM_NOTCH_COEFB2B3       ,   0x0},
    {TVE_SECAM_NOTCH_COEFA2A3       ,   0x0},


};


/*************************** 720P_60.txt *******************************/
T_TVE_ADJUST_ELEMENT table_720p_60[TVE_ADJ_FIELD_NUM] =
{
    {TVE_COMPOSITE_Y_DELAY          ,   0x0},
    {TVE_COMPOSITE_C_DELAY          ,   0x2},
    {TVE_COMPOSITE_LUMA_LEVEL       ,   0x50},
    {TVE_COMPOSITE_CHRMA_LEVEL      ,   0x68},
    {TVE_COMPOSITE_SYNC_DELAY       ,   0x0},
    {TVE_COMPOSITE_SYNC_LEVEL       ,   0x4},
    {TVE_COMPOSITE_FILTER_C_ENALBE  ,   0x0},
    {TVE_COMPOSITE_FILTER_Y_ENALBE  ,   0x1},
    {TVE_COMPOSITE_PEDESTAL_LEVEL   ,   0x0},


    {TVE_COMPONENT_IS_PAL           ,   0x0},
    {TVE_COMPONENT_PAL_MODE         ,   0x0},
    {TVE_COMPONENT_ALL_SMOOTH_ENABLE,   0x0},
    {TVE_COMPONENT_BTB_ENALBE       ,   0x0},
    {TVE_COMPONENT_INSERT0_ONOFF    ,   0x0},
    {TVE_COMPONENT_DAC_UPSAMPLEN    ,   0x1},
    {TVE_COMPONENT_Y_DELAY          ,   0x7},
    {TVE_COMPONENT_CB_DELAY         ,   0x6},
    {TVE_COMPONENT_CR_DELAY         ,   0x6},
    {TVE_COMPONENT_LUM_LEVEL        ,   0x52},
    {TVE_COMPONENT_CHRMA_LEVEL      ,   0x51},
    {TVE_COMPONENT_PEDESTAL_LEVEL   ,   0x0},
    {TVE_COMPONENT_UV_SYNC_ONOFF    ,   0x0},
    {TVE_COMPONENT_SYNC_DELAY       ,   0x4},
    {TVE_COMPONENT_SYNC_LEVEL       ,   0x2},
    {TVE_COMPONENT_R_SYNC_ONOFF     ,   0x0},
    {TVE_COMPONENT_G_SYNC_ONOFF     ,   0x0},
    {TVE_COMPONENT_B_SYNC_ONOFF     ,   0x0},
    {TVE_COMPONENT_RGB_R_LEVEL      ,   0x46},
    {TVE_COMPONENT_RGB_G_LEVEL      ,   0x46},
    {TVE_COMPONENT_RGB_B_LEVEL      ,   0x46},
    {TVE_COMPONENT_FILTER_Y_ENALBE  ,   0x1},
    {TVE_COMPONENT_FILTER_C_ENALBE  ,   0x1},
    {TVE_COMPONENT_PEDESTAL_ONOFF   ,   0x0},
    {TVE_COMPONENT_PED_RGB_YPBPR_ENABLE,0x0},
    {TVE_COMPONENT_PED_ADJUST       ,   0x0},
    {TVE_COMPONENT_G2Y              ,   0x12a},
    {TVE_COMPONENT_G2U              ,   0x36},
    {TVE_COMPONENT_G2V              ,   0x88},
    {TVE_COMPONENT_B2U              ,   0x21d},
    {TVE_COMPONENT_R2V              ,   0x1cb},


    {TVE_BURST_POS_ENABLE           ,   0x0},
    {TVE_BURST_LEVEL_ENABLE         ,   0x0},
    {TVE_BURST_CB_LEVEL             ,   0xd4},
    {TVE_BURST_CR_LEVEL             ,   0x0},
    {TVE_BURST_START_POS            ,   0x0},
    {TVE_BURST_END_POS              ,   0x0},
    {TVE_BURST_SET_FREQ_MODE        ,   0x0},
    {TVE_BURST_FREQ_SIGN            ,   0x0},
    {TVE_BURST_PHASE_COMPENSATION   ,   0x0},
    {TVE_BURST_FREQ_RESPONSE        ,   0x0},


    {TVE_ASYNC_FIFO                 ,   0x4},
    {TVE_CAV_SYNC_HIGH              ,   0x2},
    {TVE_SYNC_HIGH_WIDTH            ,   0x7},
    {TVE_SYNC_LOW_WIDTH             ,   0xd},


    {TVE_VIDEO_DAC_FS               ,   0x0},
    {TVE_SECAM_PRE_COEFFA3A2        ,   0x0},
    {TVE_SECAM_PRE_COEFFB1A4        ,   0x0},
    {TVE_SECAM_PRE_COEFFB3B2        ,   0x0},
    {TVE_SECAM_F0CB_CENTER          ,   0x0},
    {TVE_SECAM_F0CR_CENTER          ,   0x0},
    {TVE_SECAM_FM_KCBCR_AJUST       ,   0x0},
    {TVE_SECAM_CONTROL              ,   0x0},
    {TVE_SECAM_NOTCH_COEFB1         ,   0x0},
    {TVE_SECAM_NOTCH_COEFB2B3       ,   0x0},
    {TVE_SECAM_NOTCH_COEFA2A3       ,   0x0},


};


/*************************** 1080I_25.txt *******************************/
T_TVE_ADJUST_ELEMENT table_1080i_25[TVE_ADJ_FIELD_NUM] =
{
    {TVE_COMPOSITE_Y_DELAY          ,   0x0},
    {TVE_COMPOSITE_C_DELAY          ,   0x2},
    {TVE_COMPOSITE_LUMA_LEVEL       ,   0x50},
    {TVE_COMPOSITE_CHRMA_LEVEL      ,   0x68},
    {TVE_COMPOSITE_SYNC_DELAY       ,   0x0},
    {TVE_COMPOSITE_SYNC_LEVEL       ,   0x4},
    {TVE_COMPOSITE_FILTER_C_ENALBE  ,   0x0},
    {TVE_COMPOSITE_FILTER_Y_ENALBE  ,   0x1},
    {TVE_COMPOSITE_PEDESTAL_LEVEL   ,   0x0},


    {TVE_COMPONENT_IS_PAL           ,   0x0},
    {TVE_COMPONENT_PAL_MODE         ,   0x0},
    {TVE_COMPONENT_ALL_SMOOTH_ENABLE,   0x0},
    {TVE_COMPONENT_BTB_ENALBE       ,   0x0},
    {TVE_COMPONENT_INSERT0_ONOFF    ,   0x0},
    {TVE_COMPONENT_DAC_UPSAMPLEN    ,   0x1},
    {TVE_COMPONENT_Y_DELAY          ,   0x6},
    {TVE_COMPONENT_CB_DELAY         ,   0x5},
    {TVE_COMPONENT_CR_DELAY         ,   0x5},
    {TVE_COMPONENT_LUM_LEVEL        ,   0x52},
    {TVE_COMPONENT_CHRMA_LEVEL      ,   0x51},
    {TVE_COMPONENT_PEDESTAL_LEVEL   ,   0x0},
    {TVE_COMPONENT_UV_SYNC_ONOFF    ,   0x0},
    {TVE_COMPONENT_SYNC_DELAY       ,   0x5},
    {TVE_COMPONENT_SYNC_LEVEL       ,   0x3},
    {TVE_COMPONENT_R_SYNC_ONOFF     ,   0x0},
    {TVE_COMPONENT_G_SYNC_ONOFF     ,   0x0},
    {TVE_COMPONENT_B_SYNC_ONOFF     ,   0x0},
    {TVE_COMPONENT_RGB_R_LEVEL      ,   0x50},
    {TVE_COMPONENT_RGB_G_LEVEL      ,   0x50},
    {TVE_COMPONENT_RGB_B_LEVEL      ,   0x50},
    {TVE_COMPONENT_FILTER_Y_ENALBE  ,   0x1},
    {TVE_COMPONENT_FILTER_C_ENALBE  ,   0x1},
    {TVE_COMPONENT_PEDESTAL_ONOFF   ,   0x0},
    {TVE_COMPONENT_PED_RGB_YPBPR_ENABLE,0x0},
    {TVE_COMPONENT_PED_ADJUST       ,   0x0},
    {TVE_COMPONENT_G2Y              ,   0x12a},
    {TVE_COMPONENT_G2U              ,   0x36},
    {TVE_COMPONENT_G2V              ,   0x88},
    {TVE_COMPONENT_B2U              ,   0x21d},
    {TVE_COMPONENT_R2V              ,   0x1cb},


    {TVE_BURST_POS_ENABLE           ,   0x0},
    {TVE_BURST_LEVEL_ENABLE         ,   0x0},
    {TVE_BURST_CB_LEVEL             ,   0xd4},
    {TVE_BURST_CR_LEVEL             ,   0x0},
    {TVE_BURST_START_POS            ,   0x0},
    {TVE_BURST_END_POS              ,   0x0},
    {TVE_BURST_SET_FREQ_MODE        ,   0x0},
    {TVE_BURST_FREQ_SIGN            ,   0x0},
    {TVE_BURST_PHASE_COMPENSATION   ,   0x0},
    {TVE_BURST_FREQ_RESPONSE        ,   0x0},


    {TVE_ASYNC_FIFO                 ,   0x4},
    {TVE_CAV_SYNC_HIGH              ,   0x1},
    {TVE_SYNC_HIGH_WIDTH            ,   0x6},
    {TVE_SYNC_LOW_WIDTH             ,   0xd},


    {TVE_VIDEO_DAC_FS               ,   0x0},
    {TVE_SECAM_PRE_COEFFA3A2        ,   0x0},
    {TVE_SECAM_PRE_COEFFB1A4        ,   0x0},
    {TVE_SECAM_PRE_COEFFB3B2        ,   0x0},
    {TVE_SECAM_F0CB_CENTER          ,   0x0},
    {TVE_SECAM_F0CR_CENTER          ,   0x0},
    {TVE_SECAM_FM_KCBCR_AJUST       ,   0x0},
    {TVE_SECAM_CONTROL              ,   0x0},
    {TVE_SECAM_NOTCH_COEFB1         ,   0x0},
    {TVE_SECAM_NOTCH_COEFB2B3       ,   0x0},
    {TVE_SECAM_NOTCH_COEFA2A3       ,   0x0},


};


/*************************** 1080I_30.txt *******************************/
T_TVE_ADJUST_ELEMENT table_1080i_30[TVE_ADJ_FIELD_NUM] =
{
    {TVE_COMPOSITE_Y_DELAY          ,   0x0},
    {TVE_COMPOSITE_C_DELAY          ,   0x2},
    {TVE_COMPOSITE_LUMA_LEVEL       ,   0x50},
    {TVE_COMPOSITE_CHRMA_LEVEL      ,   0x68},
    {TVE_COMPOSITE_SYNC_DELAY       ,   0x0},
    {TVE_COMPOSITE_SYNC_LEVEL       ,   0x4},
    {TVE_COMPOSITE_FILTER_C_ENALBE  ,   0x0},
    {TVE_COMPOSITE_FILTER_Y_ENALBE  ,   0x1},
    {TVE_COMPOSITE_PEDESTAL_LEVEL   ,   0x0},


    {TVE_COMPONENT_IS_PAL           ,   0x0},
    {TVE_COMPONENT_PAL_MODE         ,   0x0},
    {TVE_COMPONENT_ALL_SMOOTH_ENABLE,   0x0},
    {TVE_COMPONENT_BTB_ENALBE       ,   0x0},
    {TVE_COMPONENT_INSERT0_ONOFF    ,   0x0},
    {TVE_COMPONENT_DAC_UPSAMPLEN    ,   0x1},
    {TVE_COMPONENT_Y_DELAY          ,   0x6},
    {TVE_COMPONENT_CB_DELAY         ,   0x5},
    {TVE_COMPONENT_CR_DELAY         ,   0x5},
    {TVE_COMPONENT_LUM_LEVEL        ,   0x52},
    {TVE_COMPONENT_CHRMA_LEVEL      ,   0x51},
    {TVE_COMPONENT_PEDESTAL_LEVEL   ,   0x0},
    {TVE_COMPONENT_UV_SYNC_ONOFF    ,   0x0},
    {TVE_COMPONENT_SYNC_DELAY       ,   0x6},
    {TVE_COMPONENT_SYNC_LEVEL       ,   0x3},
    {TVE_COMPONENT_R_SYNC_ONOFF     ,   0x0},
    {TVE_COMPONENT_G_SYNC_ONOFF     ,   0x0},
    {TVE_COMPONENT_B_SYNC_ONOFF     ,   0x0},
    {TVE_COMPONENT_RGB_R_LEVEL      ,   0x50},
    {TVE_COMPONENT_RGB_G_LEVEL      ,   0x50},
    {TVE_COMPONENT_RGB_B_LEVEL      ,   0x50},
    {TVE_COMPONENT_FILTER_Y_ENALBE  ,   0x1},
    {TVE_COMPONENT_FILTER_C_ENALBE  ,   0x1},
    {TVE_COMPONENT_PEDESTAL_ONOFF   ,   0x0},
    {TVE_COMPONENT_PED_RGB_YPBPR_ENABLE,0x0},
    {TVE_COMPONENT_PED_ADJUST       ,   0x0},
    {TVE_COMPONENT_G2Y              ,   0x12a},
    {TVE_COMPONENT_G2U              ,   0x36},
    {TVE_COMPONENT_G2V              ,   0x88},
    {TVE_COMPONENT_B2U              ,   0x21d},
    {TVE_COMPONENT_R2V              ,   0x1cb},


    {TVE_BURST_POS_ENABLE           ,   0x0},
    {TVE_BURST_LEVEL_ENABLE         ,   0x0},
    {TVE_BURST_CB_LEVEL             ,   0xd4},
    {TVE_BURST_CR_LEVEL             ,   0x0},
    {TVE_BURST_START_POS            ,   0x0},
    {TVE_BURST_END_POS              ,   0x0},
    {TVE_BURST_SET_FREQ_MODE        ,   0x0},
    {TVE_BURST_FREQ_SIGN            ,   0x0},
    {TVE_BURST_PHASE_COMPENSATION   ,   0x0},
    {TVE_BURST_FREQ_RESPONSE        ,   0x0},


    {TVE_ASYNC_FIFO                 ,   0x4},
    {TVE_CAV_SYNC_HIGH              ,   0x1},
    {TVE_SYNC_HIGH_WIDTH            ,   0x6},
    {TVE_SYNC_LOW_WIDTH             ,   0xd},


    {TVE_VIDEO_DAC_FS               ,   0x0},
    {TVE_SECAM_PRE_COEFFA3A2        ,   0x0},
    {TVE_SECAM_PRE_COEFFB1A4        ,   0x0},
    {TVE_SECAM_PRE_COEFFB3B2        ,   0x0},
    {TVE_SECAM_F0CB_CENTER          ,   0x0},
    {TVE_SECAM_F0CR_CENTER          ,   0x0},
    {TVE_SECAM_FM_KCBCR_AJUST       ,   0x0},
    {TVE_SECAM_CONTROL              ,   0x0},
    {TVE_SECAM_NOTCH_COEFB1         ,   0x0},
    {TVE_SECAM_NOTCH_COEFB2B3       ,   0x0},
    {TVE_SECAM_NOTCH_COEFA2A3       ,   0x0},


};


/*************************** 1080P_24.txt *******************************/
T_TVE_ADJUST_ELEMENT table_1080p_24[TVE_ADJ_FIELD_NUM] =
{
    {TVE_COMPOSITE_Y_DELAY          ,   0x0},
    {TVE_COMPOSITE_C_DELAY          ,   0x2},
    {TVE_COMPOSITE_LUMA_LEVEL       ,   0x50},
    {TVE_COMPOSITE_CHRMA_LEVEL      ,   0x68},
    {TVE_COMPOSITE_SYNC_DELAY       ,   0x0},
    {TVE_COMPOSITE_SYNC_LEVEL       ,   0x4},
    {TVE_COMPOSITE_FILTER_C_ENALBE  ,   0x0},
    {TVE_COMPOSITE_FILTER_Y_ENALBE  ,   0x1},
    {TVE_COMPOSITE_PEDESTAL_LEVEL   ,   0x0},


    {TVE_COMPONENT_IS_PAL           ,   0x0},
    {TVE_COMPONENT_PAL_MODE         ,   0x0},
    {TVE_COMPONENT_ALL_SMOOTH_ENABLE,   0x0},
    {TVE_COMPONENT_BTB_ENALBE       ,   0x0},
    {TVE_COMPONENT_INSERT0_ONOFF    ,   0x0},
    {TVE_COMPONENT_DAC_UPSAMPLEN    ,   0x1},
    {TVE_COMPONENT_Y_DELAY          ,   0x6},
    {TVE_COMPONENT_CB_DELAY         ,   0x6},
    {TVE_COMPONENT_CR_DELAY         ,   0x6},
    {TVE_COMPONENT_LUM_LEVEL        ,   0x52},
    {TVE_COMPONENT_CHRMA_LEVEL      ,   0x51},
    {TVE_COMPONENT_PEDESTAL_LEVEL   ,   0x0},
    {TVE_COMPONENT_UV_SYNC_ONOFF    ,   0x0},
    {TVE_COMPONENT_SYNC_DELAY       ,   0x5},
    {TVE_COMPONENT_SYNC_LEVEL       ,   0x3},
    {TVE_COMPONENT_R_SYNC_ONOFF     ,   0x0},
    {TVE_COMPONENT_G_SYNC_ONOFF     ,   0x0},
    {TVE_COMPONENT_B_SYNC_ONOFF     ,   0x0},
    {TVE_COMPONENT_RGB_R_LEVEL      ,   0x50},
    {TVE_COMPONENT_RGB_G_LEVEL      ,   0x50},
    {TVE_COMPONENT_RGB_B_LEVEL      ,   0x50},
    {TVE_COMPONENT_FILTER_Y_ENALBE  ,   0x1},
    {TVE_COMPONENT_FILTER_C_ENALBE  ,   0x1},
    {TVE_COMPONENT_PEDESTAL_ONOFF   ,   0x0},
    {TVE_COMPONENT_PED_RGB_YPBPR_ENABLE,0x0},
    {TVE_COMPONENT_PED_ADJUST       ,   0x0},
    {TVE_COMPONENT_G2Y              ,   0x12a},
    {TVE_COMPONENT_G2U              ,   0x36},
    {TVE_COMPONENT_G2V              ,   0x88},
    {TVE_COMPONENT_B2U              ,   0x21d},
    {TVE_COMPONENT_R2V              ,   0x1cb},


    {TVE_BURST_POS_ENABLE           ,   0x0},
    {TVE_BURST_LEVEL_ENABLE         ,   0x0},
    {TVE_BURST_CB_LEVEL             ,   0xd4},
    {TVE_BURST_CR_LEVEL             ,   0x0},
    {TVE_BURST_START_POS            ,   0x0},
    {TVE_BURST_END_POS              ,   0x0},
    {TVE_BURST_SET_FREQ_MODE        ,   0x0},
    {TVE_BURST_FREQ_SIGN            ,   0x0},
    {TVE_BURST_PHASE_COMPENSATION   ,   0x0},
    {TVE_BURST_FREQ_RESPONSE        ,   0x0},


    {TVE_ASYNC_FIFO                 ,   0x4},
    {TVE_CAV_SYNC_HIGH              ,   0x1},
    {TVE_SYNC_HIGH_WIDTH            ,   0x6},
    {TVE_SYNC_LOW_WIDTH             ,   0xd},


    {TVE_VIDEO_DAC_FS               ,   0x0},
    {TVE_SECAM_PRE_COEFFA3A2        ,   0x0},
    {TVE_SECAM_PRE_COEFFB1A4        ,   0x0},
    {TVE_SECAM_PRE_COEFFB3B2        ,   0x0},
    {TVE_SECAM_F0CB_CENTER          ,   0x0},
    {TVE_SECAM_F0CR_CENTER          ,   0x0},
    {TVE_SECAM_FM_KCBCR_AJUST       ,   0x0},
    {TVE_SECAM_CONTROL              ,   0x0},
    {TVE_SECAM_NOTCH_COEFB1         ,   0x0},
    {TVE_SECAM_NOTCH_COEFB2B3       ,   0x0},
    {TVE_SECAM_NOTCH_COEFA2A3       ,   0x0},


};


/*************************** 1080P_25.txt *******************************/
T_TVE_ADJUST_ELEMENT table_1080p_25[TVE_ADJ_FIELD_NUM] =
{
    {TVE_COMPOSITE_Y_DELAY          ,   0x0},
    {TVE_COMPOSITE_C_DELAY          ,   0x2},
    {TVE_COMPOSITE_LUMA_LEVEL       ,   0x50},
    {TVE_COMPOSITE_CHRMA_LEVEL      ,   0x68},
    {TVE_COMPOSITE_SYNC_DELAY       ,   0x0},
    {TVE_COMPOSITE_SYNC_LEVEL       ,   0x4},
    {TVE_COMPOSITE_FILTER_C_ENALBE  ,   0x0},
    {TVE_COMPOSITE_FILTER_Y_ENALBE  ,   0x1},
    {TVE_COMPOSITE_PEDESTAL_LEVEL   ,   0x0},


    {TVE_COMPONENT_IS_PAL           ,   0x0},
    {TVE_COMPONENT_PAL_MODE         ,   0x0},
    {TVE_COMPONENT_ALL_SMOOTH_ENABLE,   0x0},
    {TVE_COMPONENT_BTB_ENALBE       ,   0x0},
    {TVE_COMPONENT_INSERT0_ONOFF    ,   0x0},
    {TVE_COMPONENT_DAC_UPSAMPLEN    ,   0x1},
    {TVE_COMPONENT_Y_DELAY          ,   0x6},
    {TVE_COMPONENT_CB_DELAY         ,   0x6},
    {TVE_COMPONENT_CR_DELAY         ,   0x6},
    {TVE_COMPONENT_LUM_LEVEL        ,   0x52},
    {TVE_COMPONENT_CHRMA_LEVEL      ,   0x51},
    {TVE_COMPONENT_PEDESTAL_LEVEL   ,   0x0},
    {TVE_COMPONENT_UV_SYNC_ONOFF    ,   0x0},
    {TVE_COMPONENT_SYNC_DELAY       ,   0x5},
    {TVE_COMPONENT_SYNC_LEVEL       ,   0x3},
    {TVE_COMPONENT_R_SYNC_ONOFF     ,   0x0},
    {TVE_COMPONENT_G_SYNC_ONOFF     ,   0x0},
    {TVE_COMPONENT_B_SYNC_ONOFF     ,   0x0},
    {TVE_COMPONENT_RGB_R_LEVEL      ,   0x50},
    {TVE_COMPONENT_RGB_G_LEVEL      ,   0x50},
    {TVE_COMPONENT_RGB_B_LEVEL      ,   0x50},
    {TVE_COMPONENT_FILTER_Y_ENALBE  ,   0x1},
    {TVE_COMPONENT_FILTER_C_ENALBE  ,   0x1},
    {TVE_COMPONENT_PEDESTAL_ONOFF   ,   0x0},
    {TVE_COMPONENT_PED_RGB_YPBPR_ENABLE,0x0},
    {TVE_COMPONENT_PED_ADJUST       ,   0x0},
    {TVE_COMPONENT_G2Y              ,   0x12a},
    {TVE_COMPONENT_G2U              ,   0x36},
    {TVE_COMPONENT_G2V              ,   0x88},
    {TVE_COMPONENT_B2U              ,   0x21d},
    {TVE_COMPONENT_R2V              ,   0x1cb},


    {TVE_BURST_POS_ENABLE           ,   0x0},
    {TVE_BURST_LEVEL_ENABLE         ,   0x0},
    {TVE_BURST_CB_LEVEL             ,   0xd4},
    {TVE_BURST_CR_LEVEL             ,   0x0},
    {TVE_BURST_START_POS            ,   0x0},
    {TVE_BURST_END_POS              ,   0x0},
    {TVE_BURST_SET_FREQ_MODE        ,   0x0},
    {TVE_BURST_FREQ_SIGN            ,   0x0},
    {TVE_BURST_PHASE_COMPENSATION   ,   0x0},
    {TVE_BURST_FREQ_RESPONSE        ,   0x0},


    {TVE_ASYNC_FIFO                 ,   0x4},
    {TVE_CAV_SYNC_HIGH              ,   0x1},
    {TVE_SYNC_HIGH_WIDTH            ,   0x6},
    {TVE_SYNC_LOW_WIDTH             ,   0xd},


    {TVE_VIDEO_DAC_FS               ,   0x0},
    {TVE_SECAM_PRE_COEFFA3A2        ,   0x0},
    {TVE_SECAM_PRE_COEFFB1A4        ,   0x0},
    {TVE_SECAM_PRE_COEFFB3B2        ,   0x0},
    {TVE_SECAM_F0CB_CENTER          ,   0x0},
    {TVE_SECAM_F0CR_CENTER          ,   0x0},
    {TVE_SECAM_FM_KCBCR_AJUST       ,   0x0},
    {TVE_SECAM_CONTROL              ,   0x0},
    {TVE_SECAM_NOTCH_COEFB1         ,   0x0},
    {TVE_SECAM_NOTCH_COEFB2B3       ,   0x0},
    {TVE_SECAM_NOTCH_COEFA2A3       ,   0x0},


};


/*************************** 1080P_30.txt *******************************/
T_TVE_ADJUST_ELEMENT table_1080p_30[TVE_ADJ_FIELD_NUM] =
{
    {TVE_COMPOSITE_Y_DELAY          ,   0x0},
    {TVE_COMPOSITE_C_DELAY          ,   0x2},
    {TVE_COMPOSITE_LUMA_LEVEL       ,   0x50},
    {TVE_COMPOSITE_CHRMA_LEVEL      ,   0x68},
    {TVE_COMPOSITE_SYNC_DELAY       ,   0x0},
    {TVE_COMPOSITE_SYNC_LEVEL       ,   0x4},
    {TVE_COMPOSITE_FILTER_C_ENALBE  ,   0x0},
    {TVE_COMPOSITE_FILTER_Y_ENALBE  ,   0x1},
    {TVE_COMPOSITE_PEDESTAL_LEVEL   ,   0x0},


    {TVE_COMPONENT_IS_PAL           ,   0x0},
    {TVE_COMPONENT_PAL_MODE         ,   0x0},
    {TVE_COMPONENT_ALL_SMOOTH_ENABLE,   0x0},
    {TVE_COMPONENT_BTB_ENALBE       ,   0x0},
    {TVE_COMPONENT_INSERT0_ONOFF    ,   0x0},
    {TVE_COMPONENT_DAC_UPSAMPLEN    ,   0x1},
    {TVE_COMPONENT_Y_DELAY          ,   0x6},
    {TVE_COMPONENT_CB_DELAY         ,   0x6},
    {TVE_COMPONENT_CR_DELAY         ,   0x6},
    {TVE_COMPONENT_LUM_LEVEL        ,   0x52},
    {TVE_COMPONENT_CHRMA_LEVEL      ,   0x51},
    {TVE_COMPONENT_PEDESTAL_LEVEL   ,   0x0},
    {TVE_COMPONENT_UV_SYNC_ONOFF    ,   0x0},
    {TVE_COMPONENT_SYNC_DELAY       ,   0x6},
    {TVE_COMPONENT_SYNC_LEVEL       ,   0x3},
    {TVE_COMPONENT_R_SYNC_ONOFF     ,   0x0},
    {TVE_COMPONENT_G_SYNC_ONOFF     ,   0x0},
    {TVE_COMPONENT_B_SYNC_ONOFF     ,   0x0},
    {TVE_COMPONENT_RGB_R_LEVEL      ,   0x50},
    {TVE_COMPONENT_RGB_G_LEVEL      ,   0x50},
    {TVE_COMPONENT_RGB_B_LEVEL      ,   0x50},
    {TVE_COMPONENT_FILTER_Y_ENALBE  ,   0x1},
    {TVE_COMPONENT_FILTER_C_ENALBE  ,   0x1},
    {TVE_COMPONENT_PEDESTAL_ONOFF   ,   0x0},
    {TVE_COMPONENT_PED_RGB_YPBPR_ENABLE,0x0},
    {TVE_COMPONENT_PED_ADJUST       ,   0x0},
    {TVE_COMPONENT_G2Y              ,   0x12a},
    {TVE_COMPONENT_G2U              ,   0x36},
    {TVE_COMPONENT_G2V              ,   0x88},
    {TVE_COMPONENT_B2U              ,   0x21d},
    {TVE_COMPONENT_R2V              ,   0x1cb},


    {TVE_BURST_POS_ENABLE           ,   0x0},
    {TVE_BURST_LEVEL_ENABLE         ,   0x0},
    {TVE_BURST_CB_LEVEL             ,   0xd4},
    {TVE_BURST_CR_LEVEL             ,   0x0},
    {TVE_BURST_START_POS            ,   0x0},
    {TVE_BURST_END_POS              ,   0x0},
    {TVE_BURST_SET_FREQ_MODE        ,   0x0},
    {TVE_BURST_FREQ_SIGN            ,   0x0},
    {TVE_BURST_PHASE_COMPENSATION   ,   0x0},
    {TVE_BURST_FREQ_RESPONSE        ,   0x0},


    {TVE_ASYNC_FIFO                 ,   0x4},
    {TVE_CAV_SYNC_HIGH              ,   0x1},
    {TVE_SYNC_HIGH_WIDTH            ,   0x6},
    {TVE_SYNC_LOW_WIDTH             ,   0xd},


    {TVE_VIDEO_DAC_FS               ,   0x0},
    {TVE_SECAM_PRE_COEFFA3A2        ,   0x0},
    {TVE_SECAM_PRE_COEFFB1A4        ,   0x0},
    {TVE_SECAM_PRE_COEFFB3B2        ,   0x0},
    {TVE_SECAM_F0CB_CENTER          ,   0x0},
    {TVE_SECAM_F0CR_CENTER          ,   0x0},
    {TVE_SECAM_FM_KCBCR_AJUST       ,   0x0},
    {TVE_SECAM_CONTROL              ,   0x0},
    {TVE_SECAM_NOTCH_COEFB1         ,   0x0},
    {TVE_SECAM_NOTCH_COEFB2B3       ,   0x0},
    {TVE_SECAM_NOTCH_COEFA2A3       ,   0x0},


};


/*************************** 1152I_25.txt *******************************/
T_TVE_ADJUST_ELEMENT table_1152i_25[TVE_ADJ_FIELD_NUM] =
{
    {TVE_COMPOSITE_Y_DELAY          ,   0x0},
    {TVE_COMPOSITE_C_DELAY          ,   0x2},
    {TVE_COMPOSITE_LUMA_LEVEL       ,   0x50},
    {TVE_COMPOSITE_CHRMA_LEVEL      ,   0x68},
    {TVE_COMPOSITE_SYNC_DELAY       ,   0x0},
    {TVE_COMPOSITE_SYNC_LEVEL       ,   0x4},
    {TVE_COMPOSITE_FILTER_C_ENALBE  ,   0x0},
    {TVE_COMPOSITE_FILTER_Y_ENALBE  ,   0x1},
    {TVE_COMPOSITE_PEDESTAL_LEVEL   ,   0x0},


    {TVE_COMPONENT_IS_PAL           ,   0x0},
    {TVE_COMPONENT_PAL_MODE         ,   0x0},
    {TVE_COMPONENT_ALL_SMOOTH_ENABLE,   0x0},
    {TVE_COMPONENT_BTB_ENALBE       ,   0x0},
    {TVE_COMPONENT_INSERT0_ONOFF    ,   0x0},
    {TVE_COMPONENT_DAC_UPSAMPLEN    ,   0x1},
    {TVE_COMPONENT_Y_DELAY          ,   0x6},
    {TVE_COMPONENT_CB_DELAY         ,   0x6},
    {TVE_COMPONENT_CR_DELAY         ,   0x6},
    {TVE_COMPONENT_LUM_LEVEL        ,   0x52},
    {TVE_COMPONENT_CHRMA_LEVEL      ,   0x51},
    {TVE_COMPONENT_PEDESTAL_LEVEL   ,   0x0},
    {TVE_COMPONENT_UV_SYNC_ONOFF    ,   0x0},
    {TVE_COMPONENT_SYNC_DELAY       ,   0x6},
    {TVE_COMPONENT_SYNC_LEVEL       ,   0x3},
    {TVE_COMPONENT_R_SYNC_ONOFF     ,   0x0},
    {TVE_COMPONENT_G_SYNC_ONOFF     ,   0x0},
    {TVE_COMPONENT_B_SYNC_ONOFF     ,   0x0},
    {TVE_COMPONENT_RGB_R_LEVEL      ,   0x50},
    {TVE_COMPONENT_RGB_G_LEVEL      ,   0x50},
    {TVE_COMPONENT_RGB_B_LEVEL      ,   0x50},
    {TVE_COMPONENT_FILTER_Y_ENALBE  ,   0x1},
    {TVE_COMPONENT_FILTER_C_ENALBE  ,   0x1},
    {TVE_COMPONENT_PEDESTAL_ONOFF   ,   0x0},
    {TVE_COMPONENT_PED_RGB_YPBPR_ENABLE,0x0},
    {TVE_COMPONENT_PED_ADJUST       ,   0x0},
    {TVE_COMPONENT_G2Y              ,   0x12a},
    {TVE_COMPONENT_G2U              ,   0x36},
    {TVE_COMPONENT_G2V              ,   0x88},
    {TVE_COMPONENT_B2U              ,   0x21d},
    {TVE_COMPONENT_R2V              ,   0x1cb},


    {TVE_BURST_POS_ENABLE           ,   0x0},
    {TVE_BURST_LEVEL_ENABLE         ,   0x0},
    {TVE_BURST_CB_LEVEL             ,   0xd4},
    {TVE_BURST_CR_LEVEL             ,   0x0},
    {TVE_BURST_START_POS            ,   0x0},
    {TVE_BURST_END_POS              ,   0x0},
    {TVE_BURST_SET_FREQ_MODE        ,   0x0},
    {TVE_BURST_FREQ_SIGN            ,   0x0},
    {TVE_BURST_PHASE_COMPENSATION   ,   0x0},
    {TVE_BURST_FREQ_RESPONSE        ,   0x0},


    {TVE_ASYNC_FIFO                 ,   0x4},
    {TVE_CAV_SYNC_HIGH              ,   0x1},
    {TVE_SYNC_HIGH_WIDTH            ,   0x6},
    {TVE_SYNC_LOW_WIDTH             ,   0xd},


    {TVE_VIDEO_DAC_FS               ,   0x0},
    {TVE_SECAM_PRE_COEFFA3A2        ,   0x0},
    {TVE_SECAM_PRE_COEFFB1A4        ,   0x0},
    {TVE_SECAM_PRE_COEFFB3B2        ,   0x0},
    {TVE_SECAM_F0CB_CENTER          ,   0x0},
    {TVE_SECAM_F0CR_CENTER          ,   0x0},
    {TVE_SECAM_FM_KCBCR_AJUST       ,   0x0},
    {TVE_SECAM_CONTROL              ,   0x0},
    {TVE_SECAM_NOTCH_COEFB1         ,   0x0},
    {TVE_SECAM_NOTCH_COEFB2B3       ,   0x0},
    {TVE_SECAM_NOTCH_COEFA2A3       ,   0x0},


};


/*************************** 1080IASS.txt *******************************/
T_TVE_ADJUST_ELEMENT table_1080iass[TVE_ADJ_FIELD_NUM] =
{
    {TVE_COMPOSITE_Y_DELAY          ,   0x0},
    {TVE_COMPOSITE_C_DELAY          ,   0x2},
    {TVE_COMPOSITE_LUMA_LEVEL       ,   0x50},
    {TVE_COMPOSITE_CHRMA_LEVEL      ,   0x68},
    {TVE_COMPOSITE_SYNC_DELAY       ,   0x0},
    {TVE_COMPOSITE_SYNC_LEVEL       ,   0x4},
    {TVE_COMPOSITE_FILTER_C_ENALBE  ,   0x0},
    {TVE_COMPOSITE_FILTER_Y_ENALBE  ,   0x1},
    {TVE_COMPOSITE_PEDESTAL_LEVEL   ,   0x0},


    {TVE_COMPONENT_IS_PAL           ,   0x0},
    {TVE_COMPONENT_PAL_MODE         ,   0x0},
    {TVE_COMPONENT_ALL_SMOOTH_ENABLE,   0x0},
    {TVE_COMPONENT_BTB_ENALBE       ,   0x0},
    {TVE_COMPONENT_INSERT0_ONOFF    ,   0x0},
    {TVE_COMPONENT_DAC_UPSAMPLEN    ,   0x1},
    {TVE_COMPONENT_Y_DELAY          ,   0x6},
    {TVE_COMPONENT_CB_DELAY         ,   0x6},
    {TVE_COMPONENT_CR_DELAY         ,   0x6},
    {TVE_COMPONENT_LUM_LEVEL        ,   0x52},
    {TVE_COMPONENT_CHRMA_LEVEL      ,   0x51},
    {TVE_COMPONENT_PEDESTAL_LEVEL   ,   0x0},
    {TVE_COMPONENT_UV_SYNC_ONOFF    ,   0x0},
    {TVE_COMPONENT_SYNC_DELAY       ,   0x6},
    {TVE_COMPONENT_SYNC_LEVEL       ,   0x3},
    {TVE_COMPONENT_R_SYNC_ONOFF     ,   0x0},
    {TVE_COMPONENT_G_SYNC_ONOFF     ,   0x0},
    {TVE_COMPONENT_B_SYNC_ONOFF     ,   0x0},
    {TVE_COMPONENT_RGB_R_LEVEL      ,   0x50},
    {TVE_COMPONENT_RGB_G_LEVEL      ,   0x50},
    {TVE_COMPONENT_RGB_B_LEVEL      ,   0x50},
    {TVE_COMPONENT_FILTER_Y_ENALBE  ,   0x1},
    {TVE_COMPONENT_FILTER_C_ENALBE  ,   0x1},
    {TVE_COMPONENT_PEDESTAL_ONOFF   ,   0x0},
    {TVE_COMPONENT_PED_RGB_YPBPR_ENABLE,0x0},
    {TVE_COMPONENT_PED_ADJUST       ,   0x0},
    {TVE_COMPONENT_G2Y              ,   0x12a},
    {TVE_COMPONENT_G2U              ,   0x36},
    {TVE_COMPONENT_G2V              ,   0x88},
    {TVE_COMPONENT_B2U              ,   0x21d},
    {TVE_COMPONENT_R2V              ,   0x1cb},


    {TVE_BURST_POS_ENABLE           ,   0x0},
    {TVE_BURST_LEVEL_ENABLE         ,   0x0},
    {TVE_BURST_CB_LEVEL             ,   0xd4},
    {TVE_BURST_CR_LEVEL             ,   0x0},
    {TVE_BURST_START_POS            ,   0x0},
    {TVE_BURST_END_POS              ,   0x0},
    {TVE_BURST_SET_FREQ_MODE        ,   0x0},
    {TVE_BURST_FREQ_SIGN            ,   0x0},
    {TVE_BURST_PHASE_COMPENSATION   ,   0x0},
    {TVE_BURST_FREQ_RESPONSE        ,   0x0},


    {TVE_ASYNC_FIFO                 ,   0x4},
    {TVE_CAV_SYNC_HIGH              ,   0x1},
    {TVE_SYNC_HIGH_WIDTH            ,   0x6},
    {TVE_SYNC_LOW_WIDTH             ,   0xd},


    {TVE_VIDEO_DAC_FS               ,   0x0},
    {TVE_SECAM_PRE_COEFFA3A2        ,   0x0},
    {TVE_SECAM_PRE_COEFFB1A4        ,   0x0},
    {TVE_SECAM_PRE_COEFFB3B2        ,   0x0},
    {TVE_SECAM_F0CB_CENTER          ,   0x0},
    {TVE_SECAM_F0CR_CENTER          ,   0x0},
    {TVE_SECAM_FM_KCBCR_AJUST       ,   0x0},
    {TVE_SECAM_CONTROL              ,   0x0},
    {TVE_SECAM_NOTCH_COEFB1         ,   0x0},
    {TVE_SECAM_NOTCH_COEFB2B3       ,   0x0},
    {TVE_SECAM_NOTCH_COEFA2A3       ,   0x0},


};


/*************************** 1080P_50.txt *******************************/
T_TVE_ADJUST_ELEMENT table_1080p_50[TVE_ADJ_FIELD_NUM] =
{
    {TVE_COMPOSITE_Y_DELAY          ,   0x0},
    {TVE_COMPOSITE_C_DELAY          ,   0x2},
    {TVE_COMPOSITE_LUMA_LEVEL       ,   0x50},
    {TVE_COMPOSITE_CHRMA_LEVEL      ,   0x68},
    {TVE_COMPOSITE_SYNC_DELAY       ,   0x0},
    {TVE_COMPOSITE_SYNC_LEVEL       ,   0x4},
    {TVE_COMPOSITE_FILTER_C_ENALBE  ,   0x0},
    {TVE_COMPOSITE_FILTER_Y_ENALBE  ,   0x1},
    {TVE_COMPOSITE_PEDESTAL_LEVEL   ,   0x0},


    {TVE_COMPONENT_IS_PAL           ,   0x0},
    {TVE_COMPONENT_PAL_MODE         ,   0x0},
    {TVE_COMPONENT_ALL_SMOOTH_ENABLE,   0x0},
    {TVE_COMPONENT_BTB_ENALBE       ,   0x0},
    {TVE_COMPONENT_INSERT0_ONOFF    ,   0x0},
    {TVE_COMPONENT_DAC_UPSAMPLEN    ,   0x0},
    {TVE_COMPONENT_Y_DELAY          ,   0x0},
    {TVE_COMPONENT_CB_DELAY         ,   0x2},
    {TVE_COMPONENT_CR_DELAY         ,   0x2},
    {TVE_COMPONENT_LUM_LEVEL        ,   0x52},
    {TVE_COMPONENT_CHRMA_LEVEL      ,   0x51},
    {TVE_COMPONENT_PEDESTAL_LEVEL   ,   0x0},
    {TVE_COMPONENT_UV_SYNC_ONOFF    ,   0x0},
    {TVE_COMPONENT_SYNC_DELAY       ,   0x0},
    {TVE_COMPONENT_SYNC_LEVEL       ,   0x3},
    {TVE_COMPONENT_R_SYNC_ONOFF     ,   0x0},
    {TVE_COMPONENT_G_SYNC_ONOFF     ,   0x0},
    {TVE_COMPONENT_B_SYNC_ONOFF     ,   0x0},
    {TVE_COMPONENT_RGB_R_LEVEL      ,   0x50},
    {TVE_COMPONENT_RGB_G_LEVEL      ,   0x50},
    {TVE_COMPONENT_RGB_B_LEVEL      ,   0x50},
    {TVE_COMPONENT_FILTER_Y_ENALBE  ,   0x1},
    {TVE_COMPONENT_FILTER_C_ENALBE  ,   0x1},
    {TVE_COMPONENT_PEDESTAL_ONOFF   ,   0x0},
    {TVE_COMPONENT_PED_RGB_YPBPR_ENABLE,0x0},
    {TVE_COMPONENT_PED_ADJUST       ,   0x0},
    {TVE_COMPONENT_G2Y              ,   0x12a},
    {TVE_COMPONENT_G2U              ,   0x36},
    {TVE_COMPONENT_G2V              ,   0x88},
    {TVE_COMPONENT_B2U              ,   0x21d},
    {TVE_COMPONENT_R2V              ,   0x1cb},


    {TVE_BURST_POS_ENABLE           ,   0x0},
    {TVE_BURST_LEVEL_ENABLE         ,   0x0},
    {TVE_BURST_CB_LEVEL             ,   0xd4},
    {TVE_BURST_CR_LEVEL             ,   0x0},
    {TVE_BURST_START_POS            ,   0x0},
    {TVE_BURST_END_POS              ,   0x0},
    {TVE_BURST_SET_FREQ_MODE        ,   0x0},
    {TVE_BURST_FREQ_SIGN            ,   0x0},
    {TVE_BURST_PHASE_COMPENSATION   ,   0x0},
    {TVE_BURST_FREQ_RESPONSE        ,   0x0},


    {TVE_ASYNC_FIFO                 ,   0x2},
    {TVE_CAV_SYNC_HIGH              ,   0x1},
    {TVE_SYNC_HIGH_WIDTH            ,   0x0},
    {TVE_SYNC_LOW_WIDTH             ,   0xd},


    {TVE_VIDEO_DAC_FS               ,   0x0},
    {TVE_SECAM_PRE_COEFFA3A2        ,   0x0},
    {TVE_SECAM_PRE_COEFFB1A4        ,   0x0},
    {TVE_SECAM_PRE_COEFFB3B2        ,   0x0},
    {TVE_SECAM_F0CB_CENTER          ,   0x0},
    {TVE_SECAM_F0CR_CENTER          ,   0x0},
    {TVE_SECAM_FM_KCBCR_AJUST       ,   0x0},
    {TVE_SECAM_CONTROL              ,   0x0},
    {TVE_SECAM_NOTCH_COEFB1         ,   0x0},
    {TVE_SECAM_NOTCH_COEFB2B3       ,   0x0},
    {TVE_SECAM_NOTCH_COEFA2A3       ,   0x0},


};


/*************************** 1080P_60.txt *******************************/
T_TVE_ADJUST_ELEMENT table_1080p_60[TVE_ADJ_FIELD_NUM] =
{
    {TVE_COMPOSITE_Y_DELAY          ,   0x0},
    {TVE_COMPOSITE_C_DELAY          ,   0x2},
    {TVE_COMPOSITE_LUMA_LEVEL       ,   0x50},
    {TVE_COMPOSITE_CHRMA_LEVEL      ,   0x68},
    {TVE_COMPOSITE_SYNC_DELAY       ,   0x0},
    {TVE_COMPOSITE_SYNC_LEVEL       ,   0x4},
    {TVE_COMPOSITE_FILTER_C_ENALBE  ,   0x0},
    {TVE_COMPOSITE_FILTER_Y_ENALBE  ,   0x1},
    {TVE_COMPOSITE_PEDESTAL_LEVEL   ,   0x0},


    {TVE_COMPONENT_IS_PAL           ,   0x0},
    {TVE_COMPONENT_PAL_MODE         ,   0x0},
    {TVE_COMPONENT_ALL_SMOOTH_ENABLE,   0x0},
    {TVE_COMPONENT_BTB_ENALBE       ,   0x0},
    {TVE_COMPONENT_INSERT0_ONOFF    ,   0x0},
    {TVE_COMPONENT_DAC_UPSAMPLEN    ,   0x0},
    {TVE_COMPONENT_Y_DELAY          ,   0x4},
    {TVE_COMPONENT_CB_DELAY         ,   0x6},
    {TVE_COMPONENT_CR_DELAY         ,   0x6},
    {TVE_COMPONENT_LUM_LEVEL        ,   0x52},
    {TVE_COMPONENT_CHRMA_LEVEL      ,   0x51},
    {TVE_COMPONENT_PEDESTAL_LEVEL   ,   0x0},
    {TVE_COMPONENT_UV_SYNC_ONOFF    ,   0x0},
    {TVE_COMPONENT_SYNC_DELAY       ,   0x0},
    {TVE_COMPONENT_SYNC_LEVEL       ,   0x3},
    {TVE_COMPONENT_R_SYNC_ONOFF     ,   0x0},
    {TVE_COMPONENT_G_SYNC_ONOFF     ,   0x0},
    {TVE_COMPONENT_B_SYNC_ONOFF     ,   0x0},
    {TVE_COMPONENT_RGB_R_LEVEL      ,   0x50},
    {TVE_COMPONENT_RGB_G_LEVEL      ,   0x50},
    {TVE_COMPONENT_RGB_B_LEVEL      ,   0x50},
    {TVE_COMPONENT_FILTER_Y_ENALBE  ,   0x1},
    {TVE_COMPONENT_FILTER_C_ENALBE  ,   0x1},
    {TVE_COMPONENT_PEDESTAL_ONOFF   ,   0x0},
    {TVE_COMPONENT_PED_RGB_YPBPR_ENABLE,0x0},
    {TVE_COMPONENT_PED_ADJUST       ,   0x0},
    {TVE_COMPONENT_G2Y              ,   0x12a},
    {TVE_COMPONENT_G2U              ,   0x36},
    {TVE_COMPONENT_G2V              ,   0x88},
    {TVE_COMPONENT_B2U              ,   0x21d},
    {TVE_COMPONENT_R2V              ,   0x1cb},


    {TVE_BURST_POS_ENABLE           ,   0x0},
    {TVE_BURST_LEVEL_ENABLE         ,   0x0},
    {TVE_BURST_CB_LEVEL             ,   0xd4},
    {TVE_BURST_CR_LEVEL             ,   0x0},
    {TVE_BURST_START_POS            ,   0x0},
    {TVE_BURST_END_POS              ,   0x0},
    {TVE_BURST_SET_FREQ_MODE        ,   0x0},
    {TVE_BURST_FREQ_SIGN            ,   0x0},
    {TVE_BURST_PHASE_COMPENSATION   ,   0x0},
    {TVE_BURST_FREQ_RESPONSE        ,   0x0},


    {TVE_ASYNC_FIFO                 ,   0x2},
    {TVE_CAV_SYNC_HIGH              ,   0x1},
    {TVE_SYNC_HIGH_WIDTH            ,   0xb},
    {TVE_SYNC_LOW_WIDTH             ,   0xb},


    {TVE_VIDEO_DAC_FS               ,   0x0},
    {TVE_SECAM_PRE_COEFFA3A2        ,   0x0},
    {TVE_SECAM_PRE_COEFFB1A4        ,   0x0},
    {TVE_SECAM_PRE_COEFFB3B2        ,   0x0},
    {TVE_SECAM_F0CB_CENTER          ,   0x0},
    {TVE_SECAM_F0CR_CENTER          ,   0x0},
    {TVE_SECAM_FM_KCBCR_AJUST       ,   0x0},
    {TVE_SECAM_CONTROL              ,   0x0},
    {TVE_SECAM_NOTCH_COEFB1         ,   0x0},
    {TVE_SECAM_NOTCH_COEFB2B3       ,   0x0},
    {TVE_SECAM_NOTCH_COEFA2A3       ,   0x0},


};

//if have no the tv sys ,pls set null,can't delete it.
T_TVE_ADJUST_TABLE tve_table_total[TVE_SYS_NUM] =
{
        {SYS_576I        ,table_576i        },
        {SYS_480I        ,table_480i        },
        {SYS_576P        ,table_576p        },
        {SYS_480P        ,table_480p        },
        {SYS_720P_50    ,table_720p_50  },   
        {SYS_720P_60    ,table_720p_60    },
        {SYS_1080I_25   ,table_1080i_25 },
        {SYS_1080I_30   ,table_1080i_30 },
        {SYS_1080P_24   ,table_1080p_24 },
        {SYS_1080P_25   ,table_1080p_25 },
        {SYS_1080P_30   ,table_1080p_30 },
        {SYS_1152I_25   ,table_1152i_25 },
        {SYS_1080IASS   ,table_1080iass },
        {SYS_1080P_50   ,table_1080p_50 },
        {SYS_1080P_60   ,table_1080p_60 },
};
#endif
#endif
