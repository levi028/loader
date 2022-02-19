/*****************************************************************************
*    Copyright (C)2016 Ali Corporation. All Rights Reserved.
*
*    File:    tun_r858.h
*
*    Description:    Header file for tuner.
*    History:
*              Date              Author          Version          Reason
*	    ========    =========  ========  =========
*	1.  20160513		Robin.gan	  Ver 1.0		 Create file.
*****************************************************************************/

#ifndef __LLD_TUN_R858_H__
#define __LLD_TUN_R858_H__

#include <types.h>
#include <hld/nim/nim_tuner.h>

#ifdef __cplusplus
extern "C"
{
#endif

INT32 tun_r858_init(UINT32 *tuner_id, struct QAM_TUNER_CONFIG_EXT *ptrTuner_Config);
INT32 tun_r858_control(UINT32 tuner_id, UINT32 freq, UINT32 sym, UINT8 AGC_Time_Const, UINT8 cmd);
INT32 tun_r858_status(UINT32 tuner_id, UINT8 *lock);
INT32 tun_r858_command(UINT32 tuner_id, INT32 cmd, UINT32 param);

#ifdef __cplusplus
}
#endif

#endif  /* __LLD_TUN_R858_H__ */


