/****************************************************************************
 *
 *  File: tun_at810.c
 *
 *  Description: This file contains the AT810 tuner driver for Miko platform
 *
 *  History:
 *      Date                Author             Version       Comment
 *      ====                ======             =======       =======
 *      1.  2010.8.13      V. Pilloux       Ver 0.1        Create file.
 *
 ****************************************************************************/
#if ((SYS_TUN_MODULE == AT810) || (SYS_TUN_MODULE == ANY_TUNER))
#include <sys_config.h>
#include <retcode.h>
#include <types.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/i2c/i2c.h>
#include <osal/osal.h>
#include <hld/nim/nim_dev.h>
#include "tun_AT810.h"
#include "at_registry.h"
#include "at_i2c_driver.h"
#include "at_fw.h"
#include "at_fw.c"
#include "at_i2c_driver.c"
/* AT810 custom configuration */
#define CFG_XO_CAP                0x2222
#define CFG_LOOP_THROUGH_MODE     FW_LOOP_THROUGH_MODE_TUNERA_LOOPTHROUGH
#define CFG_CLOCK_OUT             FW_CLOCK_OUT_RESET_VALUE /* disabled by default */
#define CFG_BB_AGC_EXT_GAIN_VSS   FW_BB_AGC_EXT_GAIN_VSS_RESET_VALUE
#define CFG_BB_AGC_EXT_GAIN_VDD   FW_BB_AGC_EXT_GAIN_VDD_RESET_VALUE
#define CFG_LNA_CROWBAR           FW_LNA_CROWBAR_RESET_VALUE /* OFF by default */
#define RF_GAIN_SIG_INTENSITY_MIN  84 /* RF gain in dB for a -90 dBm signal */
#define RF_GAIN_SIG_INTENSITY_MAX  44 /* RF gain in dB for a -50 dBm signal */
/* end of AT810 custom configuration */

#define MAX_TRANSFER_SIZE 16
#define AT810_DEBUG

/* local table of pointer on tuners configuration structure */
static struct COFDM_TUNER_CONFIG_EXT *tuner_dev_cfg[MAX_TUNER_SUPPORT_NUM];
static UINT32 tuner_cnt = 0;
/* local table of pointer on tuner handles of the AT810 create_channel function */
static handle_t *p_tuner_handle[MAX_TUNER_SUPPORT_NUM];

/* brief function to upload firmware code in binary format into AT810 chip */
static int at_fw_upload_binary(handle_t *phandle);

/* AT810 read/write wrapper functions */
int32_t at810_i2c_read_function(void * p_user_data,
                                uint8_t i2c_address,
                                uint16_t nb_bytes,
                                uint8_t * p_buffer)
{
   UINT32 tuner_nb=*(UINT32*)p_user_data;

   return(tuner_dev_cfg[tuner_nb]->tuner_read(tuner_dev_cfg[tuner_nb]->i2c_type_id, i2c_address,
                                              p_buffer, (int)nb_bytes)==SUCCESS)?HOST_RC_OK:HOST_RC_ERROR;
}

int32_t at810_i2c_write_function(void * p_user_data,
                                 uint8_t i2c_address,
                                 uint16_t nb_bytes,
                                 uint8_t * p_buffer)
{
   UINT32 tuner_nb=*(UINT32*)p_user_data;

   return (tuner_dev_cfg[tuner_nb]->tuner_write(tuner_dev_cfg[tuner_nb]->i2c_type_id, i2c_address,
                                                p_buffer, (int)nb_bytes)==SUCCESS)?HOST_RC_OK:HOST_RC_ERROR;
}

/*****************************************************************************
 * INT32 tun_AT810_init(struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config)
 *
 * Tuner AT810 Initialization. The configuration structure is copied localy in a table.
 * Only the index of the table given to the other functions.
 *
 * Arguments:
 *  Parameter1: struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config        : pointer for Tuner configuration structure
 *              filled in nim.c
 *
 * Return Value: INT32            : Result
 *****************************************************************************/
INT32 tun_AT810_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config)
{
   UINT32 *tuner_nb;
   struct COFDM_TUNER_CONFIG_EXT *cfg;

   NIM_PRINTF("Starting AT810 initialization!\n");

   if ((ptrTuner_Config == NULL) || tuner_cnt >= MAX_TUNER_SUPPORT_NUM)
      return ERR_FAILUE;

   cfg = (struct COFDM_TUNER_CONFIG_EXT *)MALLOC(sizeof(struct COFDM_TUNER_CONFIG_EXT));
   if(cfg == NULL)
      return ERR_FAILUE;

   tuner_dev_cfg[tuner_cnt] = cfg;
   MEMCPY(cfg, ptrTuner_Config, sizeof(struct COFDM_TUNER_CONFIG_EXT));
   *tuner_id = tuner_cnt;
   /* malloc to keep the memory zone for at_create_channel */
   if ((tuner_nb=(UINT32 *)MALLOC(sizeof(UINT32)))==NULL)
      return ERR_FAILUE;

   *tuner_nb=tuner_cnt;
   /* Create the I2C handle. Low level read/write callbacks have to be addressed here. */
   if ((p_tuner_handle[tuner_cnt] = at_create_channel(ptrTuner_Config->c_tuner_base_addr,
                                                MAX_TRANSFER_SIZE,
                                                (void *)tuner_nb,
                                                at810_i2c_read_function,
                                                at810_i2c_write_function)) == NULL)
   {
      return ERR_FAILUE;
   }

   if (at_reset(p_tuner_handle[tuner_cnt])!=HOST_RC_OK)
   {
      NIM_PRINTF("Reset error!\n");
      return ERR_FAILUE;
   }

   NIM_PRINTF("Firmware upload\n");

   if (at_fw_upload_binary(p_tuner_handle[tuner_cnt]))
   {
      NIM_PRINTF("FW upload error!\n");
      return ERR_FAILUE;
   }

#ifdef AT810_DEBUG
   { /* print the FW version */
      uint32_t at_reg;
      if (at_read(p_tuner_handle[tuner_cnt], AT_FW_PATCH_VERSION, &at_reg)!=HOST_RC_OK)
      {
         NIM_PRINTF("Read AT_FW_PATCH_VERSION error!\n");
         return ERR_FAILUE;
      }
      NIM_PRINTF("(TEST2)AT810 FW version: %d.%d.%d\n", at_reg>>16, (at_reg >> 8)&0xFF, at_reg&0xFF);
   }
#endif

   /* Setup the crystal freq.: this is mandatory before using the tuner */
 /*  NIM_PRINTF("Setup the crystal frequency to:%d MHz\n", ptrTuner_Config->cTuner_Crystal/1000);
   if (at_write(p_tuner_handle[tuner_cnt], FW_CRYSTAL_FREQ, ptrTuner_Config->cTuner_Crystal/1000)!=HOST_RC_OK){
      NIM_PRINTF("Write error!\n");
      return ERR_FAILUE;
   }
*/
   NIM_PRINTF("Setup the crystal frequency to:%d MHz\n", 24);
   if (at_write(p_tuner_handle[tuner_cnt], FW_CRYSTAL_FREQ, 24)!=HOST_RC_OK){
      NIM_PRINTF("Write error!\n");
      return ERR_FAILUE;
   }



#if CFG_XO_CAP != RF_XO_CAP_RESET_VALUE
   /* Setup RF_X0_CAP register */
   NIM_PRINTF("Setup RF_X0_CAP register to %d\n",CFG_XO_CAP);
   if (at_write(p_tuner_handle[tuner_cnt], RF_XO_CAP, CFG_XO_CAP)!=HOST_RC_OK){
      NIM_PRINTF("Write error!\n");
      return ERR_FAILUE;
   }
#endif

#if CFG_LOOP_THROUGH_MODE != FW_LOOP_THROUGH_RESET_VALUE
   /* Setup Loop through mode */
   NIM_PRINTF("Setup Loop through mode to: %d\n", CFG_LOOP_THROUGH_MODE);
   if (at_write(p_tuner_handle[tuner_cnt], FW_LOOP_THROUGH, CFG_LOOP_THROUGH_MODE)!=HOST_RC_OK){
      NIM_PRINTF("Write error!\n");
      return ERR_FAILUE;
   }
#endif

#if CFG_CLOCK_OUT != FW_CLOCK_OUT_RESET_VALUE
   /* Setup Loop through mode */
   NIM_PRINTF("Setup FW_CLOCK_OUT to: %d\n", CFG_CLOCK_OUT);
   if (at_write(p_tuner_handle[tuner_cnt], FW_CLOCK_OUT, CFG_CLOCK_OUT)!=HOST_RC_OK){ 
      NIM_PRINTF("Write error!\n");
      return ERR_FAILUE;
   }
#endif

#if CFG_BB_AGC_EXT_GAIN_VSS != FW_BB_AGC_EXT_GAIN_VSS_RESET_VALUE
   /* Setup Loop through mode */
   NIM_PRINTF("Setup FW_BB_AGC_EXT_GAIN_VSS to: %d\n", CFG_BB_AGC_EXT_GAIN_VSS);
   if (at_write(p_tuner_handle[tuner_cnt], FW_BB_AGC_EXT_GAIN_VSS, CFG_BB_AGC_EXT_GAIN_VSS)!=HOST_RC_OK){
      NIM_PRINTF("Write error!\n");
      return ERR_FAILUE;
   }
#endif

#if CFG_BB_AGC_EXT_GAIN_VDD != FW_BB_AGC_EXT_GAIN_VDD_RESET_VALUE
   /* Setup Loop through mode */
   NIM_PRINTF("Setup FW_BB_AGC_EXT_GAIN_VDD to: %d\n", CFG_BB_AGC_EXT_GAIN_VDD);
   if (at_write(p_tuner_handle[tuner_cnt], FW_BB_AGC_EXT_GAIN_VDD, CFG_BB_AGC_EXT_GAIN_VDD)!=HOST_RC_OK){
      NIM_PRINTF("Write error!\n");
      return ERR_FAILUE;
   }
#endif

#if CFG_LNA_CROWBAR != FW_LNA_CROWBAR_RESET_VALUE
   /* Setup Loop through mode */
   NIM_PRINTF("Setup FW_LNA_CROWBAR to: %d\n", CFG_LNA_CROWBAR);
   if (at_write(p_tuner_handle[tuner_cnt], FW_LNA_CROWBAR, CFG_LNA_CROWBAR)!=HOST_RC_OK){
      NIM_PRINTF("Write error!\n");
      return ERR_FAILUE;
   }
#endif

// ML            WBD threshold set       25/11/2010    start
/*
   NIM_PRINTF("Setup RF_WBD1 Threshold to: %d\n", 0x12);
   if (at_write(p_tuner_handle[tuner_cnt], RF_WBD1, 0x12)!=HOST_RC_OK){
      NIM_PRINTF("Write error!\n");
      return ERR_FAILUE;
   }
*/
   // ML            WBD threshold set       25/11/2010    end


   tuner_cnt++;
   return SUCCESS;
}

/*****************************************************************************
 * INT32 tun_AT810_status(UINT8 *lock)
 *
 * Tuner read operation
 *
 * Arguments:
 *  Parameter1: UINT8 *lock        : Phase lock status to return
 *
 * Return Value: INT32            : Result
 *****************************************************************************/
INT32 tun_AT810_status(UINT32 tuner_id, UINT8 *lock)
{
   uint32_t tuner_ctrl, stck1, i=0;
   struct COFDM_TUNER_CONFIG_EXT *ptrTuner_Config;

   *lock = 0;
   if (tuner_id >= tuner_cnt || tuner_id >= MAX_TUNER_SUPPORT_NUM)
   {
      return ERR_FAILUE;
   }
   ptrTuner_Config = tuner_dev_cfg[tuner_id];

   do{
      if(i>0)
         osal_task_sleep(10); /* Wait 10 ms between two tuner ctrl reg read */
      if (at_read(p_tuner_handle[tuner_id], RF_TUNER_CONTROL, &tuner_ctrl)!=HOST_RC_OK)
      {
         NIM_PRINTF("Read RF_TUNER_CONTROL error!\n");
         return ERR_FAILUE;
      }
   }while(((tuner_ctrl & RF_TUNER_CONTROL_REQUEST_MASK)!=RF_TUNER_CONTROL_REQUEST_DONE)&&(i++<10));

   if (at_read(p_tuner_handle[tuner_id], RF_STCK1, &stck1)!=HOST_RC_OK)
   {
      NIM_PRINTF("Read RF_STCK1 error!\n");
      return ERR_FAILUE;
   }

   *lock = ((tuner_ctrl & (RF_TUNER_CONTROL_CMD_MASK | RF_TUNER_CONTROL_STATUS_MASK | RF_TUNER_CONTROL_REQUEST_MASK))==
            (RF_TUNER_CONTROL_REQUEST_DONE | RF_TUNER_CONTROL_STATUS_PASS | RF_TUNER_CONTROL_CMD_SET_TUNE) &&
            (stck1 & RF_STCK1_LO_OOL_MASK) == RF_STCK1_LO_OOL_NO);

   NIM_PRINTF("Tuner status, lock: %d \n", *lock);
   return SUCCESS;
}

/*****************************************************************************
 * INT32 tun_AT810_control(UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd)
 *
 * Tuner write operation
 *
 * Arguments:
 *  Parameter1: UINT32 freq        : Synthesiser programmable divider [kHz]
 *  Parameter2: UINT8 bandwidth        : channel bandwidth (6, 7, 8 [MHz])
 *  Parameter3: UINT8 AGC_Time_Const    : not used
 *  Parameter4: UINT8 *data        : not used
 *  Parameter5: UINT8 _i2c_cmd   : not used
 *
 * Return Value: INT32            : Result
 *****************************************************************************/
INT32 tun_AT810_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd)
{
   struct COFDM_TUNER_CONFIG_EXT *ptrTuner_Config;

   if (tuner_id >= tuner_cnt || tuner_id >= MAX_TUNER_SUPPORT_NUM)
   {
      return ERR_FAILUE;
   }
   ptrTuner_Config = tuner_dev_cfg[tuner_id];

   NIM_PRINTF("Tuner control(), freq=%d [kHz], bw=%d [MHz]\n", freq, bandwidth);

   if (at_write(p_tuner_handle[tuner_id], FW_LO_FREQUENCY, freq)!=HOST_RC_OK)
   {
      NIM_PRINTF("Write FW_LO_FREQUENCY error!\n");
      return ERR_FAILUE;
   }
   if (at_write(p_tuner_handle[tuner_id], FW_BANDWIDTH, bandwidth)!=HOST_RC_OK)
   {
      NIM_PRINTF("Write FW_BANDWIDTH error!\n");
      return ERR_FAILUE;
   }
   if (at_write(p_tuner_handle[tuner_id], RF_TUNER_CONTROL, RF_TUNER_CONTROL_CMD_SET_TUNE | RF_TUNER_CONTROL_REQUEST_REQUEST)!=HOST_RC_OK)
   {
      NIM_PRINTF("Write RF_TUNER_CONTROL error!\n");
      return ERR_FAILUE;
   }

   return SUCCESS;
}

/*****************************************************************************
 * tun_AT810_command
 *
 * Tuner mode operation: put the tuner either in standby or run mode.
 *
 * Arguments:
 *  Parameter1: cmd        : tuner command. handled command:
 *                         - NIM_TUNER_POWER_CONTROL
 *                         - NIM_TUNER_GET_AGC
 *  Parameter2: param    : command param as follows:
 *                         - NIM_TUNER_POWER_CONTROL: a boolean true=run mode, false=standby mode
 *                         - NIM_TUNER_GET_AGC: agc value returned in *param
 *
 * Return Value: INT32    : Result
 *****************************************************************************/
INT32 tun_AT810_command(UINT32 tuner_id, INT32 cmd, UINT32 param)
{
   uint32_t data;
   int32_t tuner_gain, agc;

   if (tuner_id >= tuner_cnt || tuner_id >= MAX_TUNER_SUPPORT_NUM)
   {
      return ERR_FAILUE;
   }
   if (cmd==NIM_TUNER_POWER_CONTROL)
   {
      if ((bool_t)param)
      {
         data=RF_TUNER_CONTROL_CMD_SET_TUNE | RF_TUNER_CONTROL_REQUEST_REQUEST;
      }
      else
      {
         data=RF_TUNER_CONTROL_CMD_STANDBY | RF_TUNER_CONTROL_REQUEST_REQUEST;
      }
      if (at_write(p_tuner_handle[tuner_id], RF_TUNER_CONTROL, data)!=HOST_RC_OK)
      {
         NIM_PRINTF("Write RF_TUNER_CONTROL error!\n");
         return ERR_FAILUE;
      }
   }
   else if(cmd == NIM_TUNER_GET_AGC){
      if (at_read(p_tuner_handle[tuner_id], FW_GET_TUNER_GAIN, &tuner_gain)!=HOST_RC_OK)
      {
         NIM_PRINTF("Read FW_GET_TUNER_GAIN error!");
         return ERR_FAILUE;
      }
      agc = tuner_gain/256; /* tuner gain unit is [dB/256] */
      /* RF gain relevant for signal intensity indicator is limited in the range: */
      /* RF_GAIN_SIG_INTENSITY_MAX -> RF_GAIN_SIG_INTENSITY_MIN */
      if(agc <= RF_GAIN_SIG_INTENSITY_MAX)
         agc = RF_GAIN_SIG_INTENSITY_MAX;
      else if(agc > RF_GAIN_SIG_INTENSITY_MIN)
         agc = RF_GAIN_SIG_INTENSITY_MIN;

      agc = (agc-RF_GAIN_SIG_INTENSITY_MAX) * 100 / (RF_GAIN_SIG_INTENSITY_MIN-RF_GAIN_SIG_INTENSITY_MAX);

      NIM_PRINTF("AGC RF = %d   -   ", agc);
      *(INT32*)param = agc;
   }

   return SUCCESS;
}

/**
   \brief function to upload firmware code in binary
          format into AT810 chip
   \param void
   \return HOST_RC_OK if succeed
           HOST_RC_ERROR
           HOST_RC_FIRMWARE_UPLOAD_FAILED
   \callgraph
 */
static int at_fw_upload_binary(handle_t *phandle)
{
   uint16_t firmware_line, line_nb;
   int32_t  res;
   fw_data_format *pfw_data;
   uint16_t  max_lg    = at_get_max_size_rdwr(phandle);

   /* Upload ARC FW */
   line_nb = FW_TABLE_SIZE/sizeof(fw_data_format);
   pfw_data = (fw_data_format *) fw_table;

   /* blocks the CPU in ROM */
   res = at_write(phandle, AT_FW_PATCH_VERSION, 1);
   res += at_fw_reboot(phandle);
   if (res!=HOST_RC_OK)
   {
      return HOST_RC_ERROR;
   }

   for (firmware_line = 0; firmware_line < line_nb; firmware_line++, pfw_data++)
   {
      uint32_t  addr = (pfw_data->address[0]<<24)|(pfw_data->address[1]<<16)|(pfw_data->address[2]<<8)|pfw_data->address[3];
      uint16_t  remain_lg = pfw_data->length;
      uint16_t  sent_lg   = 0;
      uint16_t  xfer_lg;

      /* split line in multiple xfer when necessary */
      do{
         xfer_lg = (remain_lg > max_lg)? max_lg : remain_lg;
         if (at_write_block(phandle, addr+sent_lg, xfer_lg,
                            (uint8_t*)(&pfw_data->data[sent_lg]))!=HOST_RC_OK)
         {
            return HOST_RC_FIRMWARE_UPLOAD_FAILED;
         }
         sent_lg += xfer_lg;
         remain_lg -= xfer_lg;
      }while(sent_lg < pfw_data->length);
   }
   return HOST_RC_OK;
}
#endif



