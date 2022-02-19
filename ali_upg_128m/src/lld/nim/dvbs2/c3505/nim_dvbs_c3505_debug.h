/*****************************************************************************
*    Copyright (C)2015 Ali Corporation. All Rights Reserved.
*
*    File:	This file contains C3505 DVBS2 debug function  
*           
*
*    Description:    
*    History: 'refer to update_history.txt'
*******************************************************************************/

#ifndef __LLD_NIM_DVBS_C3505_DEBUG_H__
#define __LLD_NIM_DVBS_C3505_DEBUG_H__


void nim_c3505_cr_adaptive_monitor(struct nim_device *dev);
void nim_c3505_cr_new_adaptive_monitor(struct nim_device *dev);

INT32 nim_c3505_cr_sw_adaptive(struct nim_device *dev);
INT32 nim_c3505_debug_intask(struct nim_device *dev);
INT32 nim_c3505_mon_signal(struct nim_device *dev);
INT32 nim_c3505_mon_reg(struct nim_device *dev);
INT32 nim_c3505_mon_lock_status(struct nim_device *dev);


#endif	// __LLD_NIM_DVBS_DEBUG_H__ */
