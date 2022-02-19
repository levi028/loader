#ifndef __LLD_DVBS_C3505_H__
#define __LLD_DVBS_C3505_H__




#include "nim_dvbs_c3505_tds.h"
#include "nim_dvbs_c3505_tp_scan.h"
#include "nim_dvbs_c3505_hw_cfg.h"
#include "nim_dvbs_c3505_monitor.h"
#include "nim_dvbs_c3505_data_cap.h"
#include "nim_dvbs_c3505_private.h"
#include "nim_dvbs_c3505_debug.h"

// added by gloria 
#define ERR_CRC_HEADER    43
#define nim_reg_write		nim_c3505_reg_write
#define nim_reg_read		nim_c3505_reg_read

#define C3505_ERR_I2C_NO_ACK	ERR_I2C_NO_ACK
#define T_CTSK					OSAL_T_CTSK

//#define AUTOSCAN_SPEEDUP
#define RS_LIMITED 1000

/*****************************************************
**************** Custom Spec Macro **********************
******************************************************/

// For india VDL skew dish PK test
//#define INDIA_VDL_SKEW_LNB
#ifdef INDIA_VDL_SKEW_LNB
    #define _INDIA_VDL_8PSK_R35_IMPULSE_
    #define _INDIA_VDL_8PSK_R34_IMPULSE_
#endif


// For Brazil Century skew dish PK test, Patch2/3 can be a general patch.
//#define ASKEY_PATCH1 // For Askey Skew Dish PK Test, Patch1 only for TP1 3700/V/15000 8PSK 5_6 15MRs, it can improve PN performance
#define ASKEY_PATCH2 // For Askey Skew Dish PK Test, Patch2 for dvb-s low symbol rate TP, it can improve PN performance
#define ASKEY_PATCH3 // For Askey Skew Dish PK Test, Patch3 For all TP, it can improve AWGN performance


/*****************************************************
************************ End **************************
******************************************************/


#define FAST_LNB_DETEC


//extern INT32 snr_initial_en;//Seen, 1 means first snr estimation done, set 1 in channel change
INT32 snr_initial_en;//Seen, 1 means first snr estimation done, set 1 in channel change

#endif /* __LLD_DVBS_C3505_H__ */

