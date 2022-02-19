/*****************************************************************************
*    Copyright (C)2006 Ali Corporation. All Rights Reserved.
*
*    File:    nim_sharp6158.h
*
*    Description:    Header file in LLD.
*    History:
*           Date            Athor        Version          Reason
*       ============    =============   =========   =================
*   1.  4.27.2006        Joey Gao        Ver 0.1       Create file.
*   
*****************************************************************************/

#ifndef __LLD_NIM_CXD2834_H__
#define __LLD_NIM_CXD2834_H__

#include <sys_config.h>
#include <types.h>
#include <retcode.h>
#include <hld/nim/nim_dev.h>
#include <hld/nim/nim_tuner.h>

typedef struct AGC_Table
{
	UINT32 AGC;
	UINT32 SignalStrength;
}AGC_Table_t;

static INT32 nim_cxd2837_ioctl(struct nim_device *dev, INT32 cmd, UINT32 param);

INT32 nim_cxd2837_attach(char *name, PCOFDM_TUNER_CONFIG_API pConfig);
INT32 nim_cxd2837_get_lock(struct nim_device *dev, UINT8 *lock);

#endif  /* __LLD_NIM_CXD2834_H__ */

