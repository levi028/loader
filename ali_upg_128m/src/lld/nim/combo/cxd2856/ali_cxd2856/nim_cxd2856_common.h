#ifndef NIM_CXD2856_COMMMON_H
#define NIM_CXD2856_COMMMON_H
#include "basic_types.h"
#include <osal/osal.h>
#include <hld/nim/nim_dev.h>
#include <hld/nim/nim_tuner.h>

#include "../sony_integ.h"
#include "../sony_dtv.h"
#include "../sony_demod.h"
#include "../sony_common.h"
#include "../i2c/sony_i2c.h"
#include "../dvb_terr/sony_demod_dvbt.h"
#include "../dvb_terr/sony_demod_dvbt2.h"
#include "../dvb_terr/sony_demod_dvbt2_monitor.h"
#include "../dvb_terr/sony_demod_dvbt_monitor.h"
#include "../dvb_terr/sony_dvbt.h"
#include "../dvb_terr/sony_dvbt2.h"
#include "../dvb_terr/sony_integ_dvbt2.h"
#include "../dvb_terr/sony_integ_dvbt.h"
#include "../dvb_terr/sony_integ_dvbt_t2.h"
#include "../isdb_terr/sony_demod_isdbt.h"
#include "../isdb_terr/sony_integ_isdbt.h"
#include "../lib/sony_math.h"
#include "../lib/sony_stdlib.h"

#include "nim_cxd2856_debug.h"
#include "nim_cxd2856_monitor.h"

#ifdef SONY_DEMOD_SUPPORT_DVBT2
#include ".././dvb_terr/sony_dvbt2.h"
#endif

#ifdef SONY_DEMOD_SUPPORT_ISDBT
#include ".././isdb_terr/sony_isdbt.h"
#endif



#define INVALID_VALUE 0XFFFFFFF //when need return a invalid value,can use it,NOTE: 7 byte not 8byte

/*this file is created because of the letter capitalization and lowercase problems
   the file:"nim_cxd2856_define.h" don't need to be included in s-tree */
//#include "nim_cxd2856_define.h" 

//void cxd2837_log_i2c(sony_demod_t* param, UINT8 err, UINT8 write, UINT8 slv_addr, UINT8 *data, int len);

sony_result_t nim_cxd2856_i2c_CommonReadRegister(sony_i2c_t* pI2c, uint8_t deviceAddress, uint8_t subAddress, uint8_t* pData, uint32_t size);
sony_result_t nim_cxd2856_i2c_CommonWriteRegister(sony_i2c_t* pI2c, uint8_t deviceAddress, uint8_t subAddress, const uint8_t* pData, uint32_t size);
sony_result_t nim_cxd2856_i2c_CommonWriteOneRegister(sony_i2c_t* pI2c, uint8_t deviceAddress, uint8_t subAddress, uint8_t data);
sony_result_t nim_cxd2856_TunerGateway(void* nim_dev_priv, UINT8	tuner_address , UINT8* wdata , int wlen , UINT8* rdata , int rlen);

#endif 
