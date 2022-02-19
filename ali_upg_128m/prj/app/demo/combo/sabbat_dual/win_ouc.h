/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_ota_set.h
*
*    Description:   The setting of OTA upgrade
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_OUC_H__
#define _WIN_OUC_H__

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct
{
    UINT32 magic;
    UINT32 upgrade_mode;
}OUC_CFG,*POUC_CFG;

#define OUC_MAGIC       0x4F554300
#define OUC_TYPE_OTA    1
#define OUC_TYPE_USB    2

#define OUC_CFG_OFFSET  0x1000



#ifdef __cplusplus
 }
#endif

#endif //_WIN_OUC_H__

