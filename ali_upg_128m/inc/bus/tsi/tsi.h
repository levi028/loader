 /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: tsi.h
*
*    Description: This file includes the defines and API of TSI module
                  (TSI,TS switch controller).
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef    __LLD_TSI_H__
#define __LLD_TSI_H__

#ifdef __cplusplus
extern "C"
{
#endif


#include <sys_config.h>
#include <basic_types.h>

/* For TS input interface */
#define TSI_SPI_0       0       /* The ID of SPI_TS 0 */
#define TSI_SPI_1       1       /* The ID of SPI_TS 1 */
#define TSI_SPI_2       2       /* The ID of SPI_ATA  */
#define TSI_SPI_3       3       /* The ID of SPI_DVBS2  */
#define TSI_SRC_REVD    3       /* The ID of reserved */
#define TSI_SSI_0       4       /* The ID of SSI 0 */
#define TSI_SSI_1       5       /* The ID of SSI 1 */
#define TSI_SSI_2       6       /* The ID of SSI 2 */
#define TSI_SSI_3       7       /* The ID of SSI 3 */
#define PARA_MODE_SRC   12      /* The mode is TSCI_PARALLEL_MODE,and sourc from CARD B*/
#define TSI_SSI2B_0     8       /* The ID of TSI_SSI  2Bit 0*/
#define TSI_SSI2B_1     9       /* The ID of TSI_SSI  2Bit 1*/
#define TSI_SSI2B_2     14      /* The ID of TSI_SSI  2Bit 2*/
#define TSI_SSI2B_3     15      /* The ID of TSI_SSI  2Bit 3*/
#define TSI_SSI4B_0     10      /* The ID of TSI_SSI  4Bit 0*/
#define TSI_SSI4B_1     11      /* The ID of TSI_SSI  4Bit 1*/

#define TSI_SSI_2_INTERNAL      12      /* The ID of SSI 2, Only used for C3603 internal */
#define TSI_SSI_3_INTERNAL      13      /* The ID of SSI 3, Only used for C3603 internal */
#define PARA_MODE_SRC_INTERNAL  6       /* The mode is TSCI_PARALLEL_MODE,and sourc from CARD B, Only used for C3603 internal*/

#define TSI_CI_SRC      7       /*for dmx 3 ,data from ci, ts select by tsa_source_sel*/

#if(SYS_CHIP_MODULE==ALI_S3602)
#define TSI_SPI_QPSK    0       /*SPI input from QPSK*/
#define TSI_SPI_EX1     0       /*1st SPI input from DVB-C/T NIM*/
#define TSI_SPI_TSG     2       /*SPI input from TSG*/
#else
#define TSI_SPI_QPSK    0       /*SPI input from QPSK*/
#define TSI_SPI_EX1     1       /*1st SPI input from DVB-C/T NIM*/
#define TSI_SPI_EX2     2       /*2nd SPI input from DVB-C/T NIM*/
#define TSI_SPI_TSG     3       /*SPI input from TSG*/
#endif
#define CI_PARA_MODE_SRC     6  /*CI_PARA_MODE_SOURCE can feed to DMX through TSB*/

/*TSI output*/
#define TSI_TS_A        0x01    /*TS_A is default output to DMX 0*/
#define TSI_TS_B        0x02    /*TS_B is default output to DMX 1*/
#define TSI_TS_C        0x04    /*TS_C is default output to DMX 2*/
#define TSI_TS_D        0x08    /*TS_D is default output to DMX 3*/

#define MODE_PARALLEL   1
#define MODE_CHAIN      0

/* For DeMUX output */
#define TSI_DMX_0       0x01    /* The ID of DMX 0 */
#define TSI_DMX_1       0x02    /* The ID of DMX 1 */
#define TSI_DMX_3       0x04    /* The ID of DMX 2 */
#define TSI_DMX_4       0x08    /* The ID of DMX 3 */

/* For the TSI attribute byte:
 * bit7: Enable flag: 0 for disable; 1 for enable.
 * bit5-6: Reserved.
 * bit4: CLOCK polarity: 0 for normal; 1 for invert.
 * bit3: SSI bit order: 0 for MSB first; 1 for LSB first.
 * bit2: ERROR polarity: 0 for active low; 1 for active high.
 * bit1: SYNC polarity: 0 for active low; 1 for active high.
 * bit0: VALID polarity: 0 for active low; 1 for active high.
 */

#if(SYS_CHIP_MODULE==ALI_S3602)

RET_CODE tsi_s3602_chg_tsiid(int raw_tsi, int chg_tsi, int chg_en);
void tsi_s3602_mode_set(int tsiid, UINT8 attrib);
void tsi_s3602_para_src_select(int tsiid, UINT32 card_id);
void tsi_s3602_parallel_mode_set(int para);
void tsi_s3602_select(int ts_id, int tsiid);
void tsi_s3602_dmx_src_select(int dmxid, int ts_id);
void tsi_s3602_bypass_ci(int dmxid, int slot_mask, BOOL bypass);
void tsi_s3602_check_dmx_src(int dmxid, UINT8 *p_ts_id, UINT8 *p_tsi_id, UINT8 *p_ci_mode);
void tsi_ci_select(UINT32 ci_enble, UINT32 ci_number);
void tsi_source_pin_mux_select(UINT32 enable, UINT32 source_sel);
void tsi_clock_select(UINT32 clock);
//void tsi_set_ssi_swap(UINT32 enable);
void tsi_set_ssi_swap(UINT32 enable, UINT8 tsiid);

void tsi_disable_error_signal(UINT8 tsiid);
void tsi_check_frontend(void);
BOOL tsi_frontend_is_invalide(void);


#define tsi_chg_tsiid(raw_tsi,chg_tsi,chg_en)    tsi_s3602_chg_tsiid(raw_tsi,chg_tsi,chg_en)
#define tsi_parallel_mode_set(para)          tsi_s3602_parallel_mode_set(para)
//select the source passes the Card B
#define tsi_para_src_select(tsiid,card_id)     tsi_s3602_para_src_select(tsiid,card_id)
#define tsi_mode_set(tsiid, attrib)        tsi_s3602_mode_set(tsiid, attrib)

/***********************************************************************************************
 * tsi_select(ts_id, tsiid)
 *
 *     This function select source for TS_A or TS_B. TS_A is default output to DMX 0,  TS_B is default output to DMX 1.
 * TS A is the source for Cam card A, TS B is the sorce for Cam card B.
 *
 * Parameters:
 * ts_id :  TSI_TS_A         // TS A
 *          TSI_TS_B         // TS B
 *
 * tsiid:     TSI_SPI_0       //1st SPI input from 1st DVB-C/TNIM
 *             TSI_SPI_1         //2nd SPI input from 2nd DVB-C/T NIM
 *             TSI_SSI_0         //1st SSI input from DVB-C/T NIM
 *             TSI_SSI_1         //2nd SSI input from DVB-C/T NIM
 *             ......
 *             TSI_SPI_TSG     //SPI input from TSG
 ***********************************************************************************************/
#define tsi_select(dmxid, tsiid)            tsi_s3602_select(dmxid, tsiid)

/***********************************************************************************************
 * tsi_dmx_src_select(dmxid, ts_id)
 *
 * This function select input TS for DMX_0 or DMX_1, the input TS id can be TSI_TS_A or TSI_TS_B
 *
 * Parameters:
 * dmxid :  TSI_DMX_0       // The ID of DMX 0
 *          TSI_DMX_1       // The ID of DMX 1
 *
 * ts_id :  TSI_TS_A         // TS A
 *          TSI_TS_B         // TS B
 ***********************************************************************************************/
#define tsi_dmx_src_select(dmxid, ts_id)     tsi_s3602_dmx_src_select(dmxid, ts_id)

/***********************************************************************************************
 * tsi_check_dmx_src(dmxid, p_ts_id, p_tsi_id, p_ci_mode)
 *
 * This function is used to check dmx's input source, input TS stream and CI mode.
 * The input TS id can be TSI_TS_A or TSI_TS_B, the input TS stream can be any TSI input stream.
 *
 * Input Parameters:
 * dmxid :  TSI_DMX_0       // The ID of DMX 0
 *          TSI_DMX_1       // The ID of DMX 1
 *
 * Output Parameters: ( the type of the following 3 parameters should be  UINT8*  )
 * p_ts_id :    TSI_TS_A     // TS A
 *                TSI_TS_B     // TS B
 *
 * p_tsi_id:    TSI_SPI_0   // 1st SPI input from 1st DVB-C/TNIM
 *                 TSI_SPI_1   // 2nd SPI input from 2nd DVB-C/T NIM
 *                 TSI_SSI_0   // 1st SSI input from DVB-C/T NIM
 *                 TSI_SSI_1   // 2nd SSI input from DVB-C/T NIM
 *                ......
 *                 TSI_SPI_TSG // SPI input from TSG
 *
 * p_ci_mode:    MODE_PARALLEL   //Parallel CI mode, TS_A pass CI card A; TS_B pass CI card B.
 *                 MODE_CHAIN        //Serial CI mode, TS_A pass CI card A and CI card B.
 ***********************************************************************************************/
#define tsi_check_dmx_src(dmxid, p_ts_id, p_tsi_id, p_ci_mode) \
        tsi_s3602_check_dmx_src(dmxid, p_ts_id, p_tsi_id, p_ci_mode)

#elif(SYS_CHIP_MODULE==ALI_M3329E)
#define tsi_parallel_mode_set(para)          tsi_m3329e_parallel_mode_set(para) // 0: serial mode; 1: parallel mode
#define tsi_mode_set(tsiid, attrib)            tsi_m3329e_mode_set(tsiid, attrib)

/***********************************************************************************************
 * tsi_select(ts_id, tsiid)
 *
 *     This function select source for TS_A or TS_B. TS_A is default output to DMX 0,  TS_B is default output to DMX 1.
 * TS A is the source for Cam card A, TS B is the sorce for Cam card B.
 *
 * Parameters:
 * ts_id :  TSI_TS_A     // TS A
 *          TSI_TS_B     // TS B
 *
 * tsiid:   TSI_SPI_QPSK     // SPI input from QPSK
 *             TSI_SPI_EX1     // 1st SPI input from DVB-C/T NIM
 *             TSI_SSI_0         // 1st SSI input from DVB-C/T NIM
 *             TSI_SSI_1         // 2nd SSI input from DVB-C/T NIM
 *             TSI_SPI_TSG     // SPI input from TSG
 ***********************************************************************************************/
#define tsi_select(ts_id, tsiid)                tsi_m3329e_select(ts_id, tsiid)

/***********************************************************************************************
 * tsi_dmx_src_select(dmxid, ts_id)
 *
 * This function select input TS for DMX_0 or DMX_1, the input TS id can be TSI_TS_A or TSI_TS_B
 *
 * Parameters:
 * dmxid :  TSI_DMX_0       // The ID of DMX 0
 *          TSI_DMX_1       // The ID of DMX 1
 *
 * ts_id :  TSI_TS_A         // TS A
 *          TSI_TS_B         // TS B
 ***********************************************************************************************/
#define tsi_dmx_src_select(dmxid, ts_id)     tsi_m3329e_dmx_src_select(dmxid, ts_id)

/***********************************************************************************************
 * tsi_check_dmx_src(dmxid, p_ts_id, p_tsi_id, p_ci_mode)
 *
 * This function is used to check dmx's input source, input TS stream and CI mode.
 * The input TS id can be TSI_TS_A or TSI_TS_B, the input TS stream can be any TSI input stream.
 *
 * Input Parameters:
 * dmxid :  TSI_DMX_0       // The ID of DMX 0
 *          TSI_DMX_1       // The ID of DMX 1
 *
 * Output Parameters: ( the type of the following 3 parameters should be  UINT8*  )
 * p_ts_id :    TSI_TS_A     // TS A
 *                TSI_TS_B     // TS B
 *
 * p_tsi_id: TSI_SPI_QPSK   // SPI input from QPSK
 *              TSI_SPI_EX1    // 1st SPI input from external DVB-C/T NIM
 *              TSI_SSI_0         // 1st SSI input from external DVB-C/T NIM
 *              TSI_SSI_1         // 2nd SSI input from external DVB-C/T NIM
 *              TSI_SPI_TSG    // SPI input from TSG
 *
 * p_ci_mode: MODE_PARALLEL // Parallel CI mode, TS_A pass CI card A; TS_B pass CI card B.
 *               MODE_CHAIN    // Serial CI mode, TS_A pass CI card A and CI card B.
 ***********************************************************************************************/
#define tsi_check_dmx_src(dmxid, p_ts_id, p_tsi_id, p_ci_mode) \
        tsi_m3329e_check_dmx_src(dmxid, p_ts_id, p_tsi_id, p_ci_mode)
#else
#define tsi_parallel_mode_set(para)          do{}while(0)
#define tsi_card_src_select(tsiid)             do{}while(0)
#define tsi_mode_set(tsiid, attrib)            tsi_m3202_mode_set(tsiid, attrib)
#define tsi_select(dmxid, tsiid)            tsi_m3202_select(dmxid, tsiid)
#define tsi_dmx_src_select(dmxid, ts_id)    do{}while(0)

#endif

#ifdef __cplusplus
}
#endif

#endif    /* __LLD_TSI_H__ */

