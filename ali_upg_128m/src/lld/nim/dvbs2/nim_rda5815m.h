/*****************************************************************************
*    Copyright (C) 2010 ALi Corp. All Rights Reserved.
*    
*    Company confidential and Properietary information.       
*    This information may not be disclosed to unauthorized  
*    individual.    
*    File: nim_rda5815m.h
*   
*    Description: 
*    
*    History: 
*    Date                         Athor        Version                 Reason
*    ========       ========     ========       ========
*    2013/8/26               Roman         V1.0
*        
*****************************************************************************/
#ifndef __NIM_RDA5815M_H
#define __NIM_RDA5815M_H

#include <types.h>
#include <hld/nim/nim_tuner.h>

#ifdef __cplusplus
extern "C"
{
#endif

INT32 nim_rda5815m_init(UINT32* tuner_id, struct QPSK_TUNER_CONFIG_EXT * ptrTuner_Config);
INT32 nim_rda5815m_control(UINT32 tuner_id, UINT32 freq, UINT32 sym);
INT32 nim_rda5815m_status(UINT32 tuner_id, UINT8 *lock);

#ifdef __cplusplus
}
#endif

#endif  /* __NIM_RDA5815M_H */


