/***************************************************************************************
                  Silicon Laboratories Broadcast Si2141 Layer 2 API

   EVALUATION AND USE OF THIS SOFTWARE IS SUBJECT TO THE TERMS AND CONDITIONS OF
     THE SOFTWARE LICENSE AGREEMENT IN THE DOCUMENTATION FILE CORRESPONDING
     TO THIS SOURCE FILE.
   IF YOU DO NOT AGREE TO THE LIMITED LICENSE AND CONDITIONS OF SUCH AGREEMENT,
     PLEASE RETURN ALL SOURCE FILES TO SILICON LABORATORIES.

   L2 API for commands and properties
   FILE: Si2141_44_24_L2_API.c
   Supported IC : Si2141-A10, Si2141-B10, Si2144-A20, Si2124-A20
   Compiled for ROM 61 firmware 1_1_build_10
   Revision: 0.1
   Tag:  ROM61_1_1_build_10_V0.1
   Date: July 24 2015
  (C) Copyright 2015, Silicon Laboratories, Inc. All rights reserved.
****************************************************************************************/
#include <string.h>
/* Si2141_44_24 API Defines */
/* define this if using the DTV video filter */
#undef USING_DLIF_FILTER
/************************************************************************************************************************/
/* Si2141_44_24 API Specific Includes */
#include "Si2141_44_24_L2_API.h"               /* Include file for this code */
#include "Si2144_firmware_2_1_build_2.h"       /* firmware compatible with Si2144-A20 marking */
#include "Si2141_firmware_1_1_build_10.h"       /* firmware compatible with Si2141-A10 marking */
#define Si2141_44_24_BYTES_PER_LINE 8
#ifdef USING_DLIF_FILTER
#include "write_DLIF_video_coeffs.h"   /* .h file from custom Video filter Tool output */
#endif

/* define the following token if using >1 tuner and control1ing vco settings via the demod wrappper */
#undef Si2141_44_24_DEMOD_WRAPPER_VCO

/************************************************************************************************************************
  NAME: Si2141_44_24_Configure
  DESCRIPTION: Setup Si2141_44_24 video filters, GPIOs/clocks, Common Properties startup, etc.
  Parameter:  Pointer to Si2141_44_24 Context
  Returns:    I2C transaction error code, NO_Si2141_44_24_ERROR if successful
************************************************************************************************************************/
int Si2141_44_24_Configure           (L1_Si2141_44_24_Context *api)
{
    int return_code;
    return_code = NO_Si2141_44_24_ERROR;

 /* load DTV video filter file */
  #ifdef USING_DLIF_FILTER
     if ((return_code = Si2141_44_24_LoadVideofilter(api,DLIF_VIDFILT_TABLE,DLIF_VIDFILT_LINES)) != NO_Si2141_44_24_ERROR)
       return return_code;
  #endif

    /* Edit the Si2141_44_24_storeUserProperties if you have any properties settings different from the standard defaults */
    Si2141_44_24_storeUserProperties     (api->prop,api);

    /* Download properties different from 'propShadow' if api->propertyWriteMode = Si2141_44_24_DOWNLOAD_ON_CHANGE */
    /* Download properties in any case                 if api->propertyWriteMode = Si2141_44_24_DOWNLOAD_ALWAYS    */
    Si2141_44_24_downloadAllProperties(api);

    return return_code;
}
/************************************************************************************************************************
  NAME: Si2141_44_24_PowerUpWithPatch
  DESCRIPTION: Send Si2141_44_24 API PowerUp Command with PowerUp to bootloader,
  Check the Chip rev and part, and ROMID are compared to expected values.
  Load the Firmware Patch then Start the Firmware.
  Programming Guide Reference:    Flowchart A.2 (POWER_UP with patch flowchart)

  Parameter:  pointer to Si2141_44_24 Context
  Returns:    Si2141_44_24/I2C transaction error code, NO_Si2141_44_24_ERROR if successful
************************************************************************************************************************/
int Si2141_44_24_PowerUpWithPatch    (L1_Si2141_44_24_Context *api)
{
    int return_code;
    return_code = NO_Si2141_44_24_ERROR;

    if (!(api->load_control & SKIP_POWERUP))
    {
    /* always wait for CTS prior to POWER_UP command */
        if ((return_code = Si2141_44_24_pollForCTS  (api)) != NO_Si2141_44_24_ERROR) {
            SiTRACE ("Si2141_44_24_pollForCTS error 0x%02x\n", return_code);
            return return_code;
        }

    do {
            if ((return_code = Si2141_44_24_L1_RESET_HW(api,
                                                  Si2141_44_24_RESET_HW_CMD_SUBCODE_CODE,
                                                  Si2141_44_24_RESET_HW_CMD_RESERVED1_RESERVED,
                                                  Si2141_44_24_RESET_HW_CMD_RESERVED2_RESERVED)) != NO_Si2141_44_24_ERROR)
            {
                SiTRACE ("Si2141_44_24_L1_RESET_HW error 0x%02x: %s\n", return_code, Si2141_44_24_L1_API_ERROR_TEXT(return_code) );
                return return_code;
            }

            if ((return_code = Si2141_44_24_L1_POWER_UP (api,
                                    Si2141_44_24_POWER_UP_CMD_SUBCODE_CODE,
                                    api->cmd->power_up.clock_mode,
                                    api->cmd->power_up.en_xout,
                                    Si2141_44_24_POWER_UP_CMD_CONFIG_1P8V_INTERNAL_REG,
                                    Si2141_44_24_POWER_UP_CMD_RESERVED3_RESERVED,
                                    Si2141_44_24_POWER_UP_CMD_RESERVED4_RESERVED,
                                    Si2141_44_24_POWER_UP_CMD_RESERVED5_RESERVED,
                                    Si2141_44_24_POWER_UP_CMD_RESERVED6_RESERVED,
                                    Si2141_44_24_POWER_UP_CMD_RESERVED7_RESERVED,
                                    Si2141_44_24_POWER_UP_CMD_RESERVED8_RESERVED
                                    )) != NO_Si2141_44_24_ERROR)
            {
                SiTRACE ("Si2141_44_24_L1_POWER_UP error 0x%02x: %s\n", return_code, Si2141_44_24_L1_API_ERROR_TEXT(return_code) );
                return return_code;
            }

    } while (Si2141_44_24_GetStatus(api) == 0xFE);


        if ((return_code = Si2141_44_24_L1_WAKE_UP (api,
                                Si2141_44_24_WAKE_UP_CMD_SUBCODE_CODE,
                                Si2141_44_24_WAKE_UP_CMD_RESET_RESET,
                                Si2141_44_24_WAKE_UP_CMD_CLOCK_FREQ_CLK_24MHZ,
                                Si2141_44_24_WAKE_UP_CMD_RESERVED9_RESERVED,
                                Si2141_44_24_WAKE_UP_CMD_FUNC_BOOTLOADER,
                                Si2141_44_24_WAKE_UP_CMD_WAKE_UP_WAKE_UP
                                )) != NO_Si2141_44_24_ERROR)
        {
            SiTRACE ("Si2141_44_24_L1_WAKE_UP error 0x%02x: %s\n", return_code, Si2141_44_24_L1_API_ERROR_TEXT(return_code) );
            return return_code;
        }

        /* Get the Part Info from the chip.   This command is only valid in Bootloader mode */
        if ((return_code = Si2141_44_24_L1_PART_INFO(api)) != NO_Si2141_44_24_ERROR) {
            SiTRACE ("Si2141_44_24_L1_PART_INFO error 0x%02x: %s\n", return_code, Si2141_44_24_L1_API_ERROR_TEXT(return_code) );
            return return_code;
        }
        SiTRACE("chiprev %d\n",        api->rsp->part_info.chiprev);
        SiTRACE("part    Si21%d\n",    api->rsp->part_info.part   );
        SiTRACE("romid   %d\n",        api->rsp->part_info.romid  );
        SiTRACE("pmajor  %d\n",        api->rsp->part_info.pmajor );
        SiTRACE("pminor  %d\n",        api->rsp->part_info.pminor );
        SiTRACE("pbuild  %d\n",        api->rsp->part_info.pbuild );

        if ((api->rsp->part_info.pmajor >= 0x30) & (api->rsp->part_info.pminor >= 0x30)) {
        SiTRACE("pmajor  '%c'\n",    api->rsp->part_info.pmajor );
        SiTRACE("pminor  '%c'\n",    api->rsp->part_info.pminor );
        SiTRACE("Full Info       'Si21%02d-%c%c%c ROM%x NVM%c_%cb%d'\n\n\n", api->rsp->part_info.part, api->rsp->part_info.chiprev + 0x40, api->rsp->part_info.pmajor, api->rsp->part_info.pminor, api->rsp->part_info.romid, api->rsp->part_info.pmajor, api->rsp->part_info.pminor, api->rsp->part_info.pbuild );
        }
     }
     if (!(api->load_control & SKIP_LOADFIRMWARE))
     {
        /* Check part info values and load the proper firmware */
          if (api->rsp->part_info.romid == 0x61)
          {
             if ((return_code = Si2141_44_24_LoadFirmware_16(api, Si2141_FW_1_1b10, FIRMWARE_LINES_1_1b10)) != NO_Si2141_44_24_ERROR) {
               SiTRACE ("Si2141_44_24_LoadFirmware error 0x%02x: %s\n", return_code, Si2141_44_24_L1_API_ERROR_TEXT(return_code) );
               return return_code;
             }
          }
          else if (api->rsp->part_info.romid == 0x62)
          {
             if ((return_code = Si2141_44_24_LoadFirmware_16(api, Si2144_FW_2_1b2, FIRMWARE_LINES_2_1b2)) != NO_Si2141_44_24_ERROR) {
                SiTRACE ("Si2141_LoadFirmware error 0x%02x: %s\n", return_code, Si2141_44_24_L1_API_ERROR_TEXT(return_code) );
                return return_code;
             }
          }
          else
          {
              SiTRACE ("INCOMPATIBLE PART error ROMID 0x%02x\n", api->rsp->part_info.romid);
              return ERROR_Si2141_44_24_INCOMPATIBLE_PART;
          }
     }
     if (!(api->load_control & SKIP_STARTFIRMWARE))
     {
        /*Start the Firmware */
        if ((return_code = Si2141_44_24_StartFirmware(api)) != NO_Si2141_44_24_ERROR) { /* Start firmware */
            SiTRACE ("Si2141_44_24_StartFirmware error 0x%02x: %s\n", return_code, Si2141_44_24_L1_API_ERROR_TEXT(return_code) );
            return return_code;
        }
        Si2141_44_24_L1_GET_REV (api);
        SiTRACE("Si21%2d Part running 'FW_%c_%cb%d'\n", api->rsp->part_info.part, api->rsp->get_rev.cmpmajor, api->rsp->get_rev.cmpminor, api->rsp->get_rev.cmpbuild );

     }

    return NO_Si2141_44_24_ERROR;
}
/************************************************************************************************************************
  NAME: Si2141_44_24_PowerUpUsingBroadcastI2C
  DESCRIPTION: This is similar to PowerUpWithPatch() for tuner_count tuners but it uses the I2C Broadcast
  command to allow the firmware download simultaneously to all tuners.

  Parameter:  tuners, a pointer to a table of L1 Si2141_44_24 Contexts
  Parameter:  tuner_count, the number of tuners in the table
  Returns:    Si2141_44_24/I2C transaction error code, NO_Si2141_44_24_ERROR if successful
************************************************************************************************************************/
int Si2141_44_24_PowerUpUsingBroadcastI2C    (L1_Si2141_44_24_Context *tuners[], int tuner_count )
{

  int return_code;
  int t;
  #define Si2141_44_24_TUNER_BROADCAST_INDEX 0
  unsigned char logged_i2c_address;
  return_code = NO_Si2141_44_24_ERROR;

  /* for each tuner execute the powerup and part_info command but not the firmware download. */
  for (t = 0; t < tuner_count; t++)
  {
      tuners[t]->load_control = SKIP_LOADFIRMWARE | SKIP_STARTFIRMWARE;
      /* Si2141_44_24_PowerUpWithPatch will return right after Si2141_44_24_L1_PART_INFO, because SKIP_LOADFIRMWARE and SKIP_STARTFIRMWARE are set */
      if ( (return_code = Si2141_44_24_PowerUpWithPatch(tuners[t])) != NO_Si2141_44_24_ERROR) {
        SiTRACE ("Tuner %d Si2141_44_24_PowerUpWithPatch error 0x%02x\n", return_code);
        return return_code;
      }
       if ( (return_code = Si2141_44_24_L1_CONFIG_I2C(tuners[t], Si2141_44_24_CONFIG_I2C_CMD_SUBCODE_CODE, Si2141_44_24_CONFIG_I2C_CMD_I2C_BROADCAST_ENABLED)) != NO_Si2141_44_24_ERROR )
       {
          SiTRACE("Tuner %d L1_CONFIG_I2C error 0x%02x\n", t, return_code);
          return return_code;
       }
   }

  /* At this stage, all tuners are connected/powered up and in 'broadcast i2c' mode */

  /* store the address of tuner0 to the broadcast address and restore the original address after we're done.  */
  logged_i2c_address = tuners[Si2141_44_24_TUNER_BROADCAST_INDEX]->i2c->address;
  tuners[Si2141_44_24_TUNER_BROADCAST_INDEX]->i2c->address = Si2141_44_24_BROADCAST_ADDRESS;
  /* set the load_control flag to SKIP_POWERUP so the firmware is downloaded and started on all tuners only */
  tuners[Si2141_44_24_TUNER_BROADCAST_INDEX]->load_control = SKIP_POWERUP;

  /* Si2141_44_24_PowerUpWithPatch will now broadcast the tuner fw and return
      when all is completed, because load_fw is now '1'                               */
  if ( (return_code = Si2141_44_24_PowerUpWithPatch(tuners[Si2141_44_24_TUNER_BROADCAST_INDEX])) != NO_Si2141_44_24_ERROR) {
    SiTRACE("Tuner %d Si2141_44_24_PowerUpWithPatch error 0x%02x\n", return_code);
    return return_code;
  }

  /* At this stage, all tuners have received the patch, and have been issued
     'Si2141_44_24_StartFirmware'                                                          */
     if ((return_code = Si2141_44_24_Configure    (tuners[Si2141_44_24_TUNER_BROADCAST_INDEX])) != NO_Si2141_44_24_ERROR)
     {
         return return_code;
     }

  /* Return the broadcast tuner address to its 'normal' value                         */
  tuners[Si2141_44_24_TUNER_BROADCAST_INDEX]->i2c->address = logged_i2c_address;

  for (t = 0; t < tuner_count; t++)
  {
    if ( (return_code = Si2141_44_24_L1_CONFIG_I2C(tuners[t], Si2141_44_24_CONFIG_I2C_CMD_SUBCODE_CODE, Si2141_44_24_CONFIG_I2C_CMD_I2C_BROADCAST_DISABLED)) != NO_Si2141_44_24_ERROR )
     {
      SiTRACE("Tuner %d L1_CONFIG_I2C error 0x%02x\n", t, return_code);
         return return_code;
     }
  }
  for (t = 0; t < tuner_count; t++)
  {
    /* Set Properties startup configuration         */
    Si2141_44_24_storePropertiesDefaults (tuners[t]->propShadow,tuners[t]);
    /* Edit the Si2141_44_24_storeUserProperties if you have any properties settings different from the standard defaults */
    Si2141_44_24_storeUserProperties     (tuners[t]->prop,tuners[t]);

   /* Reset the load_control flag to execute all phases of PowerUpWithPatch */
      tuners[t]->load_control =  SKIP_NONE;

 /* Check CTS for all tuners */
    if ( (return_code = Si2141_44_24_pollForCTS(tuners[t])) != NO_Si2141_44_24_ERROR ) {
      SiTRACE("Tuner %d pollForCTS error 0x%02x\n", t, return_code);
      return return_code;
    }

  }
  return NO_Si2141_44_24_ERROR;
}


/************************************************************************************************************************
  NAME: Si2141_44_24_LoadFirmware_16
  DESCRIPTION: Load firmware from firmware_struct array in Si2141_44_24_Firmware_x_y_build_z.h file into Si2141_44_24
              Requires Si2141_44_24 to be in bootloader mode after PowerUp
  Programming Guide Reference:    Flowchart A.3 (Download FW PATCH flowchart)

  Parameter:  Si2141_44_24 Context (I2C address)
  Parameter:  pointer to firmware_struct array
  Parameter:  number of lines in firmware table array (size in bytes / firmware_struct)
  Returns:    Si2141_44_24/I2C transaction error code, NO_Si2141_44_24_ERROR if successful
************************************************************************************************************************/
int Si2141_44_24_LoadFirmware_16        (L1_Si2141_44_24_Context *api, firmware_struct fw_table[], int nbLines)
{
    int return_code;
    int line;
    return_code = NO_Si2141_44_24_ERROR;

    SiTRACE ("Si2141_44_24_LoadFirmware_16 starting...\n");
    SiTRACE ("Si2141_44_24_LoadFirmware_16 nbLines %d\n", nbLines);

    /* for each line in fw_table */
    for (line = 0; line < nbLines; line++)
    {
      if (fw_table[line].firmware_len > 0)  /* don't download if length is 0 , e.g. dummy firmware */
      {
        /* send firmware_len bytes (up to 16) to Si2141_44_24 */
        if ((return_code = Si2141_44_24_L1_API_Patch(api, fw_table[line].firmware_len, fw_table[line].firmware_table)) != NO_Si2141_44_24_ERROR)
        {
          SiTRACE("Si2141_44_24_LoadFirmware_16 error 0x%02x patching line %d: %s\n", return_code, line, Si2141_44_24_L1_API_ERROR_TEXT(return_code) );
          if (line == 0) {
          SiTRACE("The firmware is incompatible with the part!\n");
          }
          SiTraceConfiguration((char*)"traces resume");
          return ERROR_Si2141_44_24_LOADING_FIRMWARE;
        }
        if (line==3) 
		{
			SiTraceConfiguration("traces suspend");
        }
      }
    }
    SiTraceConfiguration("traces resume");
    /* Storing Properties startup configuration in propShadow                              */
    /* !! Do NOT change the content of Si2141_44_24_storePropertiesDefaults                   !! */
    /* !! It should reflect the part internal property settings after firmware download !! */
    Si2141_44_24_storePropertiesDefaults (api->propShadow,api);

    SiTRACE ("Si2141_44_24_LoadFirmware_16 complete...\n");
    return NO_Si2141_44_24_ERROR;
}
/************************************************************************************************************************
  NAME: Si2141_44_24_LoadFirmware
  DESCRIPTON: Load firmware from FIRMWARE_TABLE array in Si2141_44_24_Firmware_x_y_build_z.h file into Si2141_44_24
              Requires Si2141_44_24 to be in bootloader mode after PowerUp
  Programming Guide Reference:    Flowchart A.3 (Download FW PATCH flowchart)

  Parameter:  Si2141_44_24 Context (I2C address)
  Parameter:  pointer to firmware table array
  Parameter:  number of lines in firmware table array (size in bytes / BYTES_PER_LINE)
  Returns:    Si2141_44_24/I2C transaction error code, NO_Si2141_44_24_ERROR if successful
************************************************************************************************************************/
int Si2141_44_24_LoadFirmware        (L1_Si2141_44_24_Context *api, unsigned char fw_table[], int nbLines)
{
    int return_code;
    int line;
    return_code = NO_Si2141_44_24_ERROR;

    SiTRACE ("Si2141_44_24_LoadFirmware starting...\n");
    SiTRACE ("Si2141_44_24_LoadFirmware nbLines %d\n", nbLines);

    /* for each line in fw_table */
    for (line = 0; line < nbLines; line++)
    {
        /* send Si2141_44_24_BYTES_PER_LINE fw bytes to Si2141_44_24 */
        if ((return_code = Si2141_44_24_L1_API_Patch(api, Si2141_44_24_BYTES_PER_LINE, fw_table + Si2141_44_24_BYTES_PER_LINE*line)) != NO_Si2141_44_24_ERROR)
        {
          SiTRACE("Si2141_44_24_LoadFirmware error 0x%02x patching line %d: %s\n", return_code, line, Si2141_44_24_L1_API_ERROR_TEXT(return_code) );
          if (line == 0) {
          SiTRACE("The firmware is incompatible with the part!\n");
          }
          SiTraceConfiguration((char*)"traces resume");
          return ERROR_Si2141_44_24_LOADING_FIRMWARE;
        }
        if (line==3) {SiTraceConfiguration("traces suspend");}
    }
    SiTraceConfiguration("traces resume");

    /* Storing Properties startup configuration in propShadow                              */
    /* !! Do NOT change the content of Si2141_44_24_storePropertiesDefaults                   !! */
    /* !! It should reflect the part internal property settings after firmware download !! */
    Si2141_44_24_storePropertiesDefaults (api->propShadow,api);

    SiTRACE ("Si2141_44_24_LoadFirmware complete...\n");

    return NO_Si2141_44_24_ERROR;
}
/************************************************************************************************************************
  NAME: Si2141_44_24_StartFirmware
  DESCRIPTION: Start Si2141_44_24 firmware (put the Si2141_44_24 into run mode)
  Parameter:   Si2141_44_24 Context (I2C address)
  Parameter (passed by Reference):   ExitBootloadeer Response Status byte : tunint, atvint, dtvint, err, cts
  Returns:     I2C transaction error code, NO_Si2141_44_24_ERROR if successful
************************************************************************************************************************/
int Si2141_44_24_StartFirmware       (L1_Si2141_44_24_Context *api)
{

    if (Si2141_44_24_L1_EXIT_BOOTLOADER(api, Si2141_44_24_EXIT_BOOTLOADER_CMD_FUNC_TUNER, Si2141_44_24_EXIT_BOOTLOADER_CMD_CTSIEN_OFF) != NO_Si2141_44_24_ERROR)
    {
        return ERROR_Si2141_44_24_STARTING_FIRMWARE;
    }

    return NO_Si2141_44_24_ERROR;
}
/************************************************************************************************************************
  NAME: Si2141_44_24_Init
  DESCRIPTION:Reset and Initialize Si2141_44_24
  Parameter:  Si2141_44_24 Context (I2C address)
  Returns:    I2C transaction error code, NO_Si2141_44_24_ERROR if successful
************************************************************************************************************************/
int Si2141_44_24_Init                (L1_Si2141_44_24_Context *api)
{
    int return_code;
    SiTRACE("Si2141_44_24_Init starting...\n");

    if ((return_code = Si2141_44_24_PowerUpWithPatch(api)) != NO_Si2141_44_24_ERROR) {   /* PowerUp into bootloader */
        SiTRACE ("Si2141_44_24_PowerUpWithPatch error 0x%02x: %s\n", return_code, Si2141_44_24_L1_API_ERROR_TEXT(return_code) );
        return return_code;
    }
    /* At this point, FW is loaded and started.  */
    Si2141_44_24_Configure(api);
    SiTRACE("Si2141_44_24_Init complete...\n");
    return NO_Si2141_44_24_ERROR;
}

 /************************************************************************************************************************
  NAME: Si2141_44_24_GetRF
  DESCRIPTIION: Retrieve Si2141_44_24 tune freq

  Parameter:  Pointer to Si2141_44_24 Context (I2C address)
  Returns  :  frequency (Hz) as an int
************************************************************************************************************************/
int  Si2141_44_24_GetRF             (L1_Si2141_44_24_Context *api)
{
    Si2141_44_24_L1_TUNER_STATUS (api);
        return api->rsp->tuner_status.freq;
}
 /************************************************************************************************************************
  NAME: Si2141_44_24_Tune
  DESCRIPTIION: Tune Si2141_44_24 in specified mode (DTV) at center frequency, wait for TUNINT and xTVINT with timeout

  Parameter:  Pointer to Si2141_44_24 Context (I2C address)
  Parameter:  Mode (DTV) use Si2141_44_24_TUNER_TUNE_FREQ_CMD_MODE_DTV constants
  Parameter:  frequency (Hz) as a unsigned long integer
  Returns:    0 if channel found.  A nonzero value means either an error occurred or channel not locked.
  Programming Guide Reference:    Flowchart A.7 (Tune flowchart)
************************************************************************************************************************/
 int  Si2141_44_24_Tune              (L1_Si2141_44_24_Context *api, unsigned char mode, unsigned long freq)
{
    int start_time  = 0;
    int return_code = 0;
    int timeout     = 36;

#ifdef Si2141_44_24_DEMOD_WRAPPER_VCO
    if (Si2141_44_24_SetVcoProperties(api) != NO_Si2141_44_24_ERROR)
    {
        return ERROR_Si2141_44_24_SENDING_COMMAND;
    }
#endif // Si2141_44_24_DEMOD_WRAPPER_VCO

    if (Si2141_44_24_L1_TUNER_TUNE_FREQ (api,
                                   mode,
                                   freq) != NO_Si2141_44_24_ERROR)
    {
        return ERROR_Si2141_44_24_SENDING_COMMAND;
    }

    start_time = system_time();

    /* wait for TUNINT, timeout is 36 ms */
    while ( (system_time() - start_time) < timeout )
    {
        if ((return_code = Si2141_44_24_L1_CheckStatus(api)) != NO_Si2141_44_24_ERROR)
            return return_code;
        if (api->status->tunint)
            break;
    }
    if (!api->status->tunint) {
      SiTRACE("Timeout waiting for TUNINT\n");
      return ERROR_Si2141_44_24_TUNINT_TIMEOUT;
    }

    /* wait for xTVINT, timeout is 110ms for ATVINT and 10 ms for DTVINT */
    start_time = system_time();
    timeout    =  10;
    while ( (system_time() - start_time) < timeout )
    {
        if ((return_code = Si2141_44_24_L1_CheckStatus(api)) != NO_Si2141_44_24_ERROR)
            return return_code;

         if (api->status->dtvint)
           break;
    }

    if (api->status->dtvint)
    {
      SiTRACE("DTV Tune Successful\n");
      return_code = NO_Si2141_44_24_ERROR;
    }
    else
    {
      SiTRACE("Timeout waiting for DTVINT\n");
      return_code = ERROR_Si2141_44_24_xTVINT_TIMEOUT;
    }

    return return_code;
}
 /************************************************************************************************************************
  NAME: Si2141_44_24_DTVTune
  DESCRIPTION: Update DTV_MODE and tune DTV mode at center frequency
  Parameter:  Pointer to Si2141_44_24 Context (I2C address)
  Parameter:  frequency (Hz)
  Parameter:  bandwidth , 6,7 or 8 MHz
  Parameter:  modulation,  e.g. use constant Si2141_44_24_DTV_MODE_PROP_MODULATION_DVBT for DVBT mode
  Parameter:  invert_spectrum, 0= normal, 1= inverted
  Returns:    I2C transaction error code, 0 if successful
  Programming Guide Reference:    Flowchart A.7 (Tune flowchart)
************************************************************************************************************************/
int  Si2141_44_24_DTVTune           (L1_Si2141_44_24_Context *api, unsigned long freq, unsigned char bw, unsigned char modulation, unsigned char invert_spectrum)
{
    int return_code;
    return_code = NO_Si2141_44_24_ERROR;

    /* update DTV_MODE_PROP property */
    api->prop->dtv_mode.bw = bw;
    api->prop->dtv_mode.invert_spectrum = invert_spectrum;
    api->prop->dtv_mode.modulation = modulation;
    if (Si2141_44_24_L1_SetProperty2(api, Si2141_44_24_DTV_MODE_PROP) != NO_Si2141_44_24_ERROR)
    {
      return ERROR_Si2141_44_24_SENDING_COMMAND;
    }
    if (api->rsp->part_info.part == 41)
    {

       /* Set the Tuner return loss optimize and TF1 boundary if in DTMB mode, otherwise reset it to default values */
        if (modulation==Si2141_44_24_DTV_MODE_PROP_MODULATION_DTMB)
        {
            api->prop->tuner_return_loss_optimize.config               =    91;
            api->prop->tuner_return_loss_optimize_2.thld               =    15; /* (default    31) */
            api->prop->tuner_return_loss_optimize_2.window             =     5; /* (default     0) */
            api->prop->tuner_return_loss_optimize_2.engagement_delay   =    3; /* (default    15) */
            api->prop->tuner_tf1_boundary_offset.tf1_boundary_offset   =     22;
        }
        else
        {
            api->prop->tuner_return_loss_optimize.config               = Si2141_TUNER_RETURN_LOSS_OPTIMIZE_PROP_CONFIG_DISABLE;
            api->prop->tuner_return_loss_optimize_2.thld               =    31; /* (default    31) */
            api->prop->tuner_return_loss_optimize_2.window             =     0; /* (default     0) */
            api->prop->tuner_return_loss_optimize_2.engagement_delay   =    15; /* (default    15) */
            api->prop->tuner_tf1_boundary_offset.tf1_boundary_offset   =     0;
        }

        /* set the remaining optimize values to their defaults */
        api->prop->tuner_return_loss_optimize.thld                 =     0;
        api->prop->tuner_return_loss_optimize.engagement_delay     =     7;
        api->prop->tuner_return_loss_optimize.disengagement_delay  =    10;

        if (Si2141_44_24_L1_SetProperty2(api, Si2141_TUNER_RETURN_LOSS_OPTIMIZE_PROP) != NO_Si2141_44_24_ERROR)
        {
          return ERROR_Si2141_44_24_SENDING_COMMAND;
        }
        if (Si2141_44_24_L1_SetProperty2(api, Si2141_TUNER_RETURN_LOSS_OPTIMIZE_2_PROP) != NO_Si2141_44_24_ERROR)
        {
          return ERROR_Si2141_44_24_SENDING_COMMAND;
        }
        if (Si2141_44_24_L1_SetProperty2(api, Si2141_TUNER_TF1_BOUNDARY_OFFSET_PROP) != NO_Si2141_44_24_ERROR)
        {
          return ERROR_Si2141_44_24_SENDING_COMMAND;
        }
    }
    return_code = Si2141_44_24_Tune (api, Si2141_44_24_TUNER_TUNE_FREQ_CMD_MODE_DTV, freq);

    return return_code;
}
 /************************************************************************************************************************
  NAME: Si2141_44_24_XoutOn
  Parameter:  Pointer to Si2141_44_24 Context (I2C address)
  Returns:    I2C transaction error code, NO_Si2141_44_24_ERROR if successful
************************************************************************************************************************/
int Si2141_44_24_XoutOn             (L1_Si2141_44_24_Context *api)
{
    int return_code;
    SiTRACE("Si2141_44_24_XoutOn:  Turning Xout ON\n");

    if ((return_code = Si2141_44_24_L1_CONFIG_CLOCKS(api,
                                              Si2141_44_24_CONFIG_CLOCKS_CMD_SUBCODE_CODE,
                                              api->cmd->config_clocks.clock_mode,
                                              Si2141_44_24_CONFIG_CLOCKS_CMD_EN_XOUT_EN_XOUT)) != NO_Si2141_44_24_ERROR)
    return return_code;

    return NO_Si2141_44_24_ERROR;
}
 /************************************************************************************************************************
  NAME: Si2141_44_24_XoutOff
  Parameter:  Pointer to Si2141_44_24 Context (I2C address)
  Returns:    I2C transaction error code, NO_Si2141_44_24_ERROR if successful
************************************************************************************************************************/
int Si2141_44_24_XoutOff            (L1_Si2141_44_24_Context *api)
{
    int return_code;
    SiTRACE("Si2141_44_24_XoutOff:  Turning Xout OFF\n");

    if ((return_code = Si2141_44_24_L1_CONFIG_CLOCKS(api,
                                              Si2141_44_24_CONFIG_CLOCKS_CMD_SUBCODE_CODE,
                                              api->cmd->config_clocks.clock_mode,
                                              Si2141_44_24_CONFIG_CLOCKS_CMD_EN_XOUT_DIS_XOUT)) != NO_Si2141_44_24_ERROR)
    return return_code;

    return NO_Si2141_44_24_ERROR;
}
 /************************************************************************************************************************
  NAME: Si2141_44_24_Standby
  Parameter:  Pointer to Si2141_44_24 Context (I2C address)
  Returns:    I2C transaction error code, NO_Si2141_44_24_ERROR if successful
************************************************************************************************************************/
int Si2141_44_24_Standby         (L1_Si2141_44_24_Context *api)
{
    int return_code;
    SiTRACE("Si2141_44_24_Standby: Going to Standby. The part will wake on the next command\n");

    if ((return_code = Si2141_44_24_L1_STANDBY(api, Si2141_44_24_STANDBY_CMD_TYPE_LNA_ON)) != NO_Si2141_44_24_ERROR)
    return return_code;

    return NO_Si2141_44_24_ERROR;
}
 /************************************************************************************************************************
  NAME: Si2141_44_24_Powerdown
  Parameter:  Pointer to Si2141_44_24 Context (I2C address)
  Returns:    I2C transaction error code, NO_Si2141_44_24_ERROR if successful
************************************************************************************************************************/
int Si2141_44_24_Powerdown         (L1_Si2141_44_24_Context *api)
{
    int return_code;
    SiTRACE("Si2141_44_24_Powerdown: Powering down the part. Select InitAndConfig to reload.\n");

    if ((return_code = Si2141_44_24_L1_POWER_DOWN_HW(api,Si2141_44_24_POWER_DOWN_HW_CMD_SUBCODE_CODE,Si2141_44_24_POWER_DOWN_HW_CMD_PDB_XO_OSC_XO_OSC_POWER_DOWN,
                                               Si2141_44_24_POWER_DOWN_HW_CMD_RESERVED1_RESERVED,Si2141_44_24_POWER_DOWN_HW_CMD_EN_XOUT_DIS_XOUT,
                                               Si2141_44_24_POWER_DOWN_HW_CMD_RESERVED2_RESERVED,Si2141_44_24_POWER_DOWN_HW_CMD_RESERVED3_RESERVED,
                                               Si2141_44_24_POWER_DOWN_HW_CMD_RESERVED4_RESERVED,Si2141_44_24_POWER_DOWN_HW_CMD_RESERVED5_RESERVED,
                                               Si2141_44_24_POWER_DOWN_HW_CMD_RESERVED6_RESERVED,Si2141_44_24_POWER_DOWN_HW_CMD_RESERVED7_RESERVED,
                                               Si2141_44_24_POWER_DOWN_HW_CMD_RESERVED8_RESERVED,Si2141_44_24_POWER_DOWN_HW_CMD_RESERVED9_RESERVED)) != NO_Si2141_44_24_ERROR)
    return return_code;

    return NO_Si2141_44_24_ERROR;
}
 /************************************************************************************************************************
  NAME: Si2141_44_24_LoadVideofilter
  DESCRIPTION:        Load video filters from vidfiltTable in Si2141_44_24_write_xTV_video_coeffs.h file into Si2141_44_24
  Programming Guide Reference:    Flowchart A.4 (Download Video Filters flowchart)

  Parameter:  Si2141_44_24 Context (I2C address)
  Parameter:  pointer to video filter table array
  Parameter:  number of lines in video filter table array(size in bytes / atv_vid_filt_struct)
  Returns:    Si2141_44_24/I2C transaction error code, NO_Si2141_44_24_ERROR if successful
************************************************************************************************************************/
int Si2141_44_24_LoadVideofilter     (L1_Si2141_44_24_Context *api, vid_filt_struct vidFiltTable[], int lines)
{
    int line;
    /* for each line in VIDFILT_TABLE  (max 16 bytes) */
    for (line = 0; line < lines; line++)
    {
        /* send up to 16 byte I2C command to Si2141_44_24 */
        if (Si2141_44_24_L1_API_Patch(api, vidFiltTable[line].vid_filt_len, vidFiltTable[line].vid_filt_table) != NO_Si2141_44_24_ERROR)
        {
            return ERROR_Si2141_44_24_SENDING_COMMAND;
        }
    }
    return NO_Si2141_44_24_ERROR;
}
/************************************************************************************************************************
  NAME: Si2141_44_24_AGC_Override
  Parameter:  Pointer to Si2141_44_24 Context (I2C address)
  Parameter:  Mode 0=Normal, 1= Max, 2=TOP
  Returns:    I2C transaction error code, 0 if successful
************************************************************************************************************************/
int Si2141_44_24_AGC_Override    (L1_Si2141_44_24_Context *api, unsigned char mode )
{
    int return_code;
    SiTRACE("Si2141_44_24_AGC_Override: mode = %d\n",mode);
    switch (mode)
    {
        case  Si2141_44_24_FORCE_NORMAL_AGC:
            if ((return_code = Si2141_44_24_L1_AGC_OVERRIDE (api, Si2141_44_24_AGC_OVERRIDE_CMD_FORCE_MAX_GAIN_DISABLE, Si2141_44_24_AGC_OVERRIDE_CMD_FORCE_TOP_GAIN_DISABLE)) != NO_Si2141_44_24_ERROR)
                return return_code;
            break;
        case  Si2141_44_24_FORCE_MAX_AGC:
            if ((return_code = Si2141_44_24_L1_AGC_OVERRIDE (api, Si2141_44_24_AGC_OVERRIDE_CMD_FORCE_MAX_GAIN_ENABLE, Si2141_44_24_AGC_OVERRIDE_CMD_FORCE_TOP_GAIN_DISABLE)) != NO_Si2141_44_24_ERROR)
                return return_code;
            break;
        case  Si2141_44_24_FORCE_TOP_AGC:
            if ((return_code = Si2141_44_24_L1_AGC_OVERRIDE (api, Si2141_44_24_AGC_OVERRIDE_CMD_FORCE_MAX_GAIN_DISABLE, Si2141_44_24_AGC_OVERRIDE_CMD_FORCE_TOP_GAIN_ENABLE)) != NO_Si2141_44_24_ERROR)
                return return_code;
            break;
        default:
            return ERROR_Si2141_44_24_PARAMETER_OUT_OF_RANGE;
    }
    return NO_Si2141_44_24_ERROR;
  }
/************************************************************************************************************************
  NAME: Si2141_44_24_SetVcoProperties
  DESCRIPTION: This will set the properties for VCO blocking.
  This function assumes:
  - The vco codes from TUNER_STATUS are assigned to the vco_code property values (e.g.  api->prop->tuner_blocked_vco.vco_code)
  - The api->propertyWriteMode = DOWNLOAD_ON_CHANGE  -- setting propertyWriteMode = DOWNLOAD_ALWAYS will work also but is not optimal.

  If any VCO properties have changed, they will be downloaded.  Normally they will be at their default (0x8000)

  Parameter:  Pointer to Si2141_44_24 Context (I2C address)
  Returns  :  error code
************************************************************************************************************************/
int Si2141_44_24_SetVcoProperties (L1_Si2141_44_24_Context *api)
{
    if (Si2141_44_24_L1_SetProperty2(api, Si2141_44_24_TUNER_BLOCKED_VCO_PROP) != NO_Si2141_44_24_ERROR)
    {
      return ERROR_Si2141_44_24_SENDING_COMMAND;
    }
    if (Si2141_44_24_L1_SetProperty2(api, Si2141_44_24_TUNER_BLOCKED_VCO2_PROP) != NO_Si2141_44_24_ERROR)
    {
      return ERROR_Si2141_44_24_SENDING_COMMAND;
    }
    if (Si2141_44_24_L1_SetProperty2(api, Si2141_44_24_TUNER_BLOCKED_VCO3_PROP) != NO_Si2141_44_24_ERROR)
    {
      return ERROR_Si2141_44_24_SENDING_COMMAND;
    }
    return NO_Si2141_44_24_ERROR;
}

/************************************************************************************************************************
  NAME: Si2141_44_24_Tuner_Block_VCO
  Parameter:  Pointer to Si2141_44_24 Context (I2C address) of the OTHER tuner
  Parameter:  vco_code from TUNER_STATUS
  Returns:    I2C transaction error code, 0 if successful
************************************************************************************************************************/
int  Si2141_44_24_Tuner_Block_VCO      (L1_Si2141_44_24_Context *api, int vco_code)
{
        /* configure the TUNER_BLOCKED_VCO for the other tuner. */
    api->prop->tuner_blocked_vco.vco_code = vco_code;

    if (Si2141_44_24_L1_SetProperty2(api, Si2141_44_24_TUNER_BLOCKED_VCO_PROP) != NO_Si2141_44_24_ERROR)
    {
       return ERROR_Si2141_44_24_SENDING_COMMAND;
    }
    return NO_Si2141_44_24_ERROR;
}
/************************************************************************************************************************
  NAME: Si2141_44_24_Tuner_Block_VCO
  Parameter:  Pointer to Si2141_44_24 Context (I2C address) of the OTHER tuner
  Parameter:  vco_code from TUNER_STATUS
  Returns:    I2C transaction error code, 0 if successful
************************************************************************************************************************/
int  Si2141_44_24_Tuner_Block_VCO2      (L1_Si2141_44_24_Context *api, int vco_code)
{
        /* configure the TUNER_BLOCKED_VCO for the other tuner. */
    api->prop->tuner_blocked_vco2.vco_code = vco_code;
SiTRACE("TUNER_BLOCK_VCO2 addr=0x%02x, code = 0x%04x, Prop=0x%04x \n",api->i2c->address,vco_code,Si2141_44_24_TUNER_BLOCKED_VCO2_PROP );


    if (Si2141_44_24_L1_SetProperty2(api, Si2141_44_24_TUNER_BLOCKED_VCO2_PROP) != NO_Si2141_44_24_ERROR)
    {
       return ERROR_Si2141_44_24_SENDING_COMMAND;
    }
    return NO_Si2141_44_24_ERROR;
}
/************************************************************************************************************************
  NAME: Si2141_44_24_Tuner_Block_VCO
  Parameter:  Pointer to Si2141_44_24 Context (I2C address) of the OTHER tuner
  Parameter:  vco_code from TUNER_STATUS
  Returns:    I2C transaction error code, 0 if successful
************************************************************************************************************************/
int  Si2141_44_24_Tuner_Block_VCO3      (L1_Si2141_44_24_Context *api, int vco_code)
{
        /* configure the TUNER_BLOCKED_VCO for the other tuner. */
    api->prop->tuner_blocked_vco3.vco_code = vco_code;
SiTRACE("TUNER_BLOCK_VCO3 addr=0x%02x, code = 0x%04x, Prop=0x%04x \n",api->i2c->address,vco_code,Si2141_44_24_TUNER_BLOCKED_VCO3_PROP );

    if (Si2141_44_24_L1_SetProperty2(api, Si2141_44_24_TUNER_BLOCKED_VCO3_PROP) != NO_Si2141_44_24_ERROR)
    {
       return ERROR_Si2141_44_24_SENDING_COMMAND;
    }
    return NO_Si2141_44_24_ERROR;
}
/************************************************************************************************************************
  NAME: Si2141_44_24_L2_VCO_Blocking_PreTune
  DESCRIPTION: Si2141_44_24 Function for blocking using the pretune model
              Call this function before tuning.  A TUNER_STATUS command will be sent to all the other tuners and the
              TUNER_BLOCK_VCO properties will be updated before the tune command is called.
  Behavior:   The preferred method is the posttune function, since it more efficient.
               There are cases where the pretune method is needed (for example if tuners are powered up and down or put in standby).
               This method will retrieve the vco codes from all the other tuners just before tuning.
************************************************************************************************************************/
int Si2141_44_24_L2_VCO_Blocking_PreTune(L1_Si2141_44_24_Context *tuners[], int tuner_num, int tuner_count)
{
int errcode;
    if (tuner_count == 1)
    {
        return NO_Si2141_44_24_ERROR;
    }
    else if (tuner_count == 2)
    {
        /* get the tuner status of the other tuner !tuner_num */
        if ((errcode=Si2141_44_24_L1_TUNER_STATUS (tuners[!tuner_num])) != NO_Si2141_44_24_ERROR)
        {
            SiTRACE ("Error returned from TunerStatus error = %s\n",Si2141_44_24_L1_API_ERROR_TEXT(errcode));
            return errcode;
        }
        /* check the frequency returned from tuner_status.  If the freq=0 (no tune done) then don't update the vco code */
        if (tuners[!tuner_num]->rsp->tuner_status.freq > 0)
        {
            if ((errcode=Si2141_44_24_Tuner_Block_VCO(tuners[tuner_num],tuners[!tuner_num]->rsp->tuner_status.vco_code)) != NO_Si2141_44_24_ERROR)
            {
                SiTRACE ("Error returned from Si2141_44_24_Tuner_Block_VCO error = %s\n",Si2141_44_24_L1_API_ERROR_TEXT(errcode));
                return errcode;
            }
        }
    }
    else if (tuner_count==3)
    {
        switch (tuner_num)
        {
            case 0:

            if ((errcode=Si2141_44_24_L1_TUNER_STATUS (tuners[1])) != NO_Si2141_44_24_ERROR)
            {
                SiTRACE ("Error returned from TunerStatus error = %s\n",Si2141_44_24_L1_API_ERROR_TEXT(errcode));
                return errcode;
            }
            /* check the frequency returned from tuner_status.  If the freq=0 (no tune done) then don't update the vco code */
            if (tuners[1]->rsp->tuner_status.freq > 0)
            {

                if ((errcode=Si2141_44_24_Tuner_Block_VCO(tuners[0],tuners[1]->rsp->tuner_status.vco_code)) != NO_Si2141_44_24_ERROR)
                {
                    SiTRACE ("Error returned from Si2141_44_24_Tuner_Block_VCO error = %s\n",Si2141_44_24_L1_API_ERROR_TEXT(errcode));
                    return errcode;
                }
            }
            if ((errcode=Si2141_44_24_L1_TUNER_STATUS (tuners[2])) != NO_Si2141_44_24_ERROR)
            {
                SiTRACE ("Error returned from TunerStatus error = %s\n",Si2141_44_24_L1_API_ERROR_TEXT(errcode));
                return errcode;
            }
            /* check the frequency returned from tuner_status.  If the freq=0 (no tune done) then don't update the vco code */
            if (tuners[2]->rsp->tuner_status.freq > 0)
            {
                if ((errcode=Si2141_44_24_Tuner_Block_VCO2(tuners[0],tuners[2]->rsp->tuner_status.vco_code)) != NO_Si2141_44_24_ERROR)
                {
                    SiTRACE ("Error returned from Si2141_44_24_Tuner_Block_VCO error = %s\n",Si2141_44_24_L1_API_ERROR_TEXT(errcode));
                    return errcode;
                }
            }
            break;
            case 1:
            if ((errcode=Si2141_44_24_L1_TUNER_STATUS (tuners[0])) != NO_Si2141_44_24_ERROR)
            {
                SiTRACE ("Error returned from TunerStatus error = %s\n",Si2141_44_24_L1_API_ERROR_TEXT(errcode));
                return errcode;
            }
            /* check the frequency returned from tuner_status.  If the freq=0 (no tune done) then don't update the vco code */
            if (tuners[0]->rsp->tuner_status.freq > 0)
            {
                if ((errcode=Si2141_44_24_Tuner_Block_VCO(tuners[1],tuners[0]->rsp->tuner_status.vco_code)) != NO_Si2141_44_24_ERROR)
                {
                    SiTRACE ("Error returned from Si2141_44_24_Tuner_Block_VCO error = %s\n",Si2141_44_24_L1_API_ERROR_TEXT(errcode));
                    return errcode;
                }
            }
            if ((errcode=Si2141_44_24_L1_TUNER_STATUS (tuners[2])) != NO_Si2141_44_24_ERROR)
            {
                SiTRACE ("Error returned from TunerStatus error = %s\n",Si2141_44_24_L1_API_ERROR_TEXT(errcode));
                return errcode;
            }
            /* check the frequency returned from tuner_status.  If the freq=0 (no tune done) then don't update the vco code */
            if (tuners[2]->rsp->tuner_status.freq > 0)
            {
                if ((errcode=Si2141_44_24_Tuner_Block_VCO2(tuners[1],tuners[2]->rsp->tuner_status.vco_code)) != NO_Si2141_44_24_ERROR)
                {
                    SiTRACE ("Error returned from Si2141_44_24_Tuner_Block_VCO error = %s\n",Si2141_44_24_L1_API_ERROR_TEXT(errcode));
                    return errcode;
                }
            }
            break;
            case 2:
            if ((errcode=Si2141_44_24_L1_TUNER_STATUS (tuners[0])) != NO_Si2141_44_24_ERROR)
            {
                SiTRACE ("Error returned from TunerStatus error = %s\n",Si2141_44_24_L1_API_ERROR_TEXT(errcode));
                return errcode;
            }
            /* check the frequency returned from tuner_status.  If the freq=0 (no tune done) then don't update the vco code */
            if (tuners[0]->rsp->tuner_status.freq > 0)
            {
                if ((errcode=Si2141_44_24_Tuner_Block_VCO(tuners[2],tuners[0]->rsp->tuner_status.vco_code)) != NO_Si2141_44_24_ERROR)
                {
                    SiTRACE ("Error returned from Si2141_44_24_Tuner_Block_VCO error = %s\n",Si2141_44_24_L1_API_ERROR_TEXT(errcode));
                    return errcode;
                }
            }
            if ((errcode=Si2141_44_24_L1_TUNER_STATUS (tuners[1])) != NO_Si2141_44_24_ERROR)
            {
                SiTRACE ("Error returned from TunerStatus error = %s\n",Si2141_44_24_L1_API_ERROR_TEXT(errcode));
                return errcode;
            }
            /* check the frequency returned from tuner_status.  If the freq=0 (no tune done) then don't update the vco code */
            if (tuners[1]->rsp->tuner_status.freq > 0)
            {
                if ((errcode=Si2141_44_24_Tuner_Block_VCO2(tuners[2],tuners[1]->rsp->tuner_status.vco_code)) != NO_Si2141_44_24_ERROR)
                {
                    SiTRACE ("Error returned from Si2141_44_24_Tuner_Block_VCO error = %s\n",Si2141_44_24_L1_API_ERROR_TEXT(errcode));
                    return errcode;
                }
            }
            break;
        }
    }
    else if (tuner_count==4)
    {
        switch (tuner_num)
        {
            case 0:
                if ((errcode=Si2141_44_24_L1_TUNER_STATUS (tuners[1])) != NO_Si2141_44_24_ERROR)
                {
                    SiTRACE ("Error returned from TunerStatus error = %s\n",Si2141_44_24_L1_API_ERROR_TEXT(errcode));
                    return errcode;
                }
             /* check the frequency returned from tuner_status.  If the freq=0 (no tune done) then don't update the vco code */
                if (tuners[1]->rsp->tuner_status.freq > 0)
                {
                  if ((errcode=Si2141_44_24_Tuner_Block_VCO(tuners[0],tuners[1]->rsp->tuner_status.vco_code)) != NO_Si2141_44_24_ERROR)
                    {
                        SiTRACE ("Error returned from Si2141_44_24_Tuner_Block_VCO error = %s\n",Si2141_44_24_L1_API_ERROR_TEXT(errcode));
                        return errcode;
                    }
                }
                if ((errcode=Si2141_44_24_L1_TUNER_STATUS (tuners[2])) != NO_Si2141_44_24_ERROR)
                {
                    SiTRACE ("Error returned from TunerStatus error = %s\n",Si2141_44_24_L1_API_ERROR_TEXT(errcode));
                    return errcode;
                }
                /* check the frequency returned from tuner_status.  If the freq=0 (no tune done) then don't update the vco code */
                if (tuners[2]->rsp->tuner_status.freq > 0)
                {
                   if ((errcode=Si2141_44_24_Tuner_Block_VCO2(tuners[0],tuners[2]->rsp->tuner_status.vco_code)) != NO_Si2141_44_24_ERROR)
                    {
                        SiTRACE ("Error returned from Si2141_44_24_Tuner_Block_VCO error = %s\n",Si2141_44_24_L1_API_ERROR_TEXT(errcode));
                        return errcode;
                    }
                }
                if ((errcode=Si2141_44_24_L1_TUNER_STATUS (tuners[3])) != NO_Si2141_44_24_ERROR)
                {
                    SiTRACE ("Error returned from TunerStatus error = %s\n",Si2141_44_24_L1_API_ERROR_TEXT(errcode));
                    return errcode;
                }
                /* check the frequency returned from tuner_status.  If the freq=0 (no tune done) then don't update the vco code */
                if (tuners[3]->rsp->tuner_status.freq > 0)
                {
                   if ((errcode=Si2141_44_24_Tuner_Block_VCO3(tuners[0],tuners[3]->rsp->tuner_status.vco_code)) != NO_Si2141_44_24_ERROR)
                    {
                        SiTRACE ("Error returned from Si2141_44_24_Tuner_Block_VCO error = %s\n",Si2141_44_24_L1_API_ERROR_TEXT(errcode));
                        return errcode;
                    }
                }
            break;
            case 1:
                if ((errcode=Si2141_44_24_L1_TUNER_STATUS (tuners[0])) != NO_Si2141_44_24_ERROR)
                {
                    SiTRACE ("Error returned from TunerStatus error = %s\n",Si2141_44_24_L1_API_ERROR_TEXT(errcode));
                    return errcode;
                }
                /* check the frequency returned from tuner_status.  If the freq=0 (no tune done) then don't update the vco code */
                if (tuners[0]->rsp->tuner_status.freq > 0)
                {
                   if ((errcode=Si2141_44_24_Tuner_Block_VCO(tuners[1],tuners[0]->rsp->tuner_status.vco_code)) != NO_Si2141_44_24_ERROR)
                    {
                        SiTRACE ("Error returned from Si2141_44_24_Tuner_Block_VCO error = %s\n",Si2141_44_24_L1_API_ERROR_TEXT(errcode));
                        return errcode;
                    }
                }
                if ((errcode=Si2141_44_24_L1_TUNER_STATUS (tuners[2])) != NO_Si2141_44_24_ERROR)
                {
                    SiTRACE ("Error returned from TunerStatus error = %s\n",Si2141_44_24_L1_API_ERROR_TEXT(errcode));
                    return errcode;
                }
                /* check the frequency returned from tuner_status.  If the freq=0 (no tune done) then don't update the vco code */
                if (tuners[2]->rsp->tuner_status.freq > 0)
                {
                   if ((errcode=Si2141_44_24_Tuner_Block_VCO2(tuners[1],tuners[2]->rsp->tuner_status.vco_code)) != NO_Si2141_44_24_ERROR)
                    {
                        SiTRACE ("Error returned from Si2141_44_24_Tuner_Block_VCO error = %s\n",Si2141_44_24_L1_API_ERROR_TEXT(errcode));
                        return errcode;
                    }
                }
                if ((errcode=Si2141_44_24_L1_TUNER_STATUS (tuners[3])) != NO_Si2141_44_24_ERROR)
                {
                    SiTRACE ("Error returned from TunerStatus error = %s\n",Si2141_44_24_L1_API_ERROR_TEXT(errcode));
                    return errcode;
                }
                /* check the frequency returned from tuner_status.  If the freq=0 (no tune done) then don't update the vco code */
                if (tuners[3]->rsp->tuner_status.freq > 0)
                {

                   if ((errcode=Si2141_44_24_Tuner_Block_VCO3(tuners[1],tuners[3]->rsp->tuner_status.vco_code)) != NO_Si2141_44_24_ERROR)
                    {
                        SiTRACE ("Error returned from Si2141_44_24_Tuner_Block_VCO error = %s\n",Si2141_44_24_L1_API_ERROR_TEXT(errcode));
                        return errcode;
                    }
                }

            break;
            case 2:
                if ((errcode=Si2141_44_24_L1_TUNER_STATUS (tuners[0])) != NO_Si2141_44_24_ERROR)
                {
                    SiTRACE ("Error returned from TunerStatus error = %s\n",Si2141_44_24_L1_API_ERROR_TEXT(errcode));
                    return errcode;
                }
                /* check the frequency returned from tuner_status.  If the freq=0 (no tune done) then don't update the vco code */
                if (tuners[0]->rsp->tuner_status.freq > 0)
                {
                   if ((errcode=Si2141_44_24_Tuner_Block_VCO(tuners[2],tuners[0]->rsp->tuner_status.vco_code)) != NO_Si2141_44_24_ERROR)
                    {
                        SiTRACE ("Error returned from Si2141_44_24_Tuner_Block_VCO error = %s\n",Si2141_44_24_L1_API_ERROR_TEXT(errcode));
                        return errcode;
                    }
                }
                if ((errcode=Si2141_44_24_L1_TUNER_STATUS (tuners[1])) != NO_Si2141_44_24_ERROR)
                {
                    SiTRACE ("Error returned from TunerStatus error = %s\n",Si2141_44_24_L1_API_ERROR_TEXT(errcode));
                    return errcode;
                }
                /* check the frequency returned from tuner_status.  If the freq=0 (no tune done) then don't update the vco code */
                if (tuners[1]->rsp->tuner_status.freq > 0)
                {
                    if ((errcode=Si2141_44_24_Tuner_Block_VCO2(tuners[2],tuners[1]->rsp->tuner_status.vco_code)) != NO_Si2141_44_24_ERROR)
                    {
                        SiTRACE ("Error returned from Si2141_44_24_Tuner_Block_VCO error = %s\n",Si2141_44_24_L1_API_ERROR_TEXT(errcode));
                        return errcode;
                    }
                }
                if ((errcode=Si2141_44_24_L1_TUNER_STATUS (tuners[3])) != NO_Si2141_44_24_ERROR)
                {
                    SiTRACE ("Error returned from TunerStatus error = %s\n",Si2141_44_24_L1_API_ERROR_TEXT(errcode));
                    return errcode;
                }
                /* check the frequency returned from tuner_status.  If the freq=0 (no tune done) then don't update the vco code */
                if (tuners[3]->rsp->tuner_status.freq > 0)
                {
                   if ((errcode=Si2141_44_24_Tuner_Block_VCO3(tuners[2],tuners[3]->rsp->tuner_status.vco_code)) != NO_Si2141_44_24_ERROR)
                    {
                        SiTRACE ("Error returned from Si2141_44_24_Tuner_Block_VCO error = %s\n",Si2141_44_24_L1_API_ERROR_TEXT(errcode));
                        return errcode;
                    }
                }
            break;
                case 3:
                if ((errcode=Si2141_44_24_L1_TUNER_STATUS (tuners[0])) != NO_Si2141_44_24_ERROR)
                {
                    SiTRACE ("Error returned from TunerStatus error = %s\n",Si2141_44_24_L1_API_ERROR_TEXT(errcode));
                    return errcode;
                }
                /* check the frequency returned from tuner_status.  If the freq=0 (no tune done) then don't update the vco code */
                if (tuners[0]->rsp->tuner_status.freq > 0)
                {
                   if ((errcode=Si2141_44_24_Tuner_Block_VCO(tuners[3],tuners[0]->rsp->tuner_status.vco_code)) != NO_Si2141_44_24_ERROR)
                    {
                        SiTRACE ("Error returned from Si2141_44_24_Tuner_Block_VCO error = %s\n",Si2141_44_24_L1_API_ERROR_TEXT(errcode));
                        return errcode;
                    }
                }
                if ((errcode=Si2141_44_24_L1_TUNER_STATUS (tuners[1])) != NO_Si2141_44_24_ERROR)
                {
                    SiTRACE ("Error returned from TunerStatus error = %s\n",Si2141_44_24_L1_API_ERROR_TEXT(errcode));
                    return errcode;
                }
                /* check the frequency returned from tuner_status.  If the freq=0 (no tune done) then don't update the vco code */
                if (tuners[1]->rsp->tuner_status.freq > 0)
                {
                   if ((errcode=Si2141_44_24_Tuner_Block_VCO2(tuners[3],tuners[1]->rsp->tuner_status.vco_code)) != NO_Si2141_44_24_ERROR)
                    {
                        SiTRACE ("Error returned from Si2141_44_24_Tuner_Block_VCO error = %s\n",Si2141_44_24_L1_API_ERROR_TEXT(errcode));
                        return errcode;
                    }
                }
                if ((errcode=Si2141_44_24_L1_TUNER_STATUS (tuners[2])) != NO_Si2141_44_24_ERROR)
                {
                    SiTRACE ("Error returned from TunerStatus error = %s\n",Si2141_44_24_L1_API_ERROR_TEXT(errcode));
                    return errcode;
                }
                /* check the frequency returned from tuner_status.  If the freq=0 (no tune done) then don't update the vco code */
                if (tuners[2]->rsp->tuner_status.freq > 0)
                {
                   if ((errcode=Si2141_44_24_Tuner_Block_VCO3(tuners[3],tuners[2]->rsp->tuner_status.vco_code)) != NO_Si2141_44_24_ERROR)
                    {
                        SiTRACE ("Error returned from Si2141_44_24_Tuner_Block_VCO error = %s\n",Si2141_44_24_L1_API_ERROR_TEXT(errcode));
                        return errcode;
                    }
                }
            break;
       }

    }
 return NO_Si2141_44_24_ERROR;

}
/************************************************************************************************************************
  NAME: Si2141_44_24_L2_VCO_Blocking_PostTune
  DESCRIPTION: Si2141_44_24 user menu function
              Call this function after tuning.  The VCO_CODE from the TUNER_STATUS command will be sent to all the other
              tuners and the TUNER_BLOCK_VCO properties will be updated after the tune command is called.
  Behavior:   This is a more efficient method than setting up the VCO blocking using the pretune function but there are
               cases where the pretune method is needed (for example if tuners are powered up and down or put in standby).

************************************************************************************************************************/
int Si2141_44_24_L2_VCO_Blocking_PostTune(L1_Si2141_44_24_Context *tuners[], int tuner_num, int tuner_count)
{

  int  ((*Tuner_Block_VCO_ptr[3])( )) = {Si2141_44_24_Tuner_Block_VCO, Si2141_44_24_Tuner_Block_VCO2, Si2141_44_24_Tuner_Block_VCO3};
  int errcode;
  int vco_dest[]={0,0,0,0};
  int vco_fn[]={0,0,0,0};
  int i;
  int c;
  int vco_code;

/* if only using 1 tuner then no need for this function */
  if (tuner_count == 1)
  {
    return NO_Si2141_44_24_ERROR;
  }

  SiTRACE("Si2141_44_24_L2_VCO_Blocking_PostTune (tuners, tuner_num %d, tuner_count %d)\n", tuner_num, tuner_count);

  if ((errcode=Si2141_44_24_L1_TUNER_STATUS (tuners[tuner_num])) != NO_Si2141_44_24_ERROR)
  {
    SiTRACE ("Error returned from TunerStatus error = %s\n",Si2141_44_24_L1_API_ERROR_TEXT(errcode));
    return errcode;
  }

  vco_code = tuners[tuner_num]->rsp->tuner_status.vco_code;
  SiTRACE("Si2141_44_24_L2_VCO_Blocking_PostTune vco_code 0x%04x\n", vco_code);
/* set up a table of tuner destinations to write the properties to. */
/* tuner 0 = 1 2 3  */
/* tuner 1  = 0 2 3  */
/* tuner 2  = 0 1 3  */
/* tuner 3  = 0 1 2  */

  c = 0;
  for ( i = 0 ; i < tuner_count; i++ ) {
    if (i != tuner_num ) { vco_dest[c++] = i; }
  }
/* set up table of functions to avoid property collisions */
/* tuner 0 = 0 1 2  */
/* tuner 1  = 0 0 0  */
/* tuner 2  = 1 1 1  */
/* tuner 3  = 2 2 2  */
  for (i=0; i< tuner_count; i++)
  {
    if (tuner_num==0)
    {
      vco_fn[i]=i;
    }
    else
    {
      vco_fn[i]=tuner_num-1;
    }
  }

  SiTRACE("Si2141_44_24_L2_VCO_Blocking_PostTune tuner_num %d ==> vco_dest[0] =    %d,  vco_dest[1] =    %d,  vco_dest[2] =    %d \n", tuner_num, vco_dest[0], vco_dest[1], vco_dest[2]);
//  SiTRACE("Si2141_44_24_L2_VCO_Blocking_PostTune tuner_num %d ==> vco_dest[0] @ 0x%02x,  vco_dest[1] @ 0x%02x,  vco_dest[2] @ 0x%02x\n", tuner_num, tuners[vco_dest[0]]->i2c->address, tuners[vco_dest[1]]->i2c->address, tuners[vco_dest[2]]->i2c->address);
  SiTRACE("Si2141_44_24_L2_VCO_Blocking_PostTune tuner_num %d ==> vco_fn  [0] =    %d,  vco_fn  [1] =    %d,  vco_fn  [2]    = %d \n", tuner_num, vco_fn[0], vco_fn[1], vco_fn[2]);

  if (tuner_count >  1) {
    if ((errcode=Tuner_Block_VCO_ptr[vco_fn[0]]  (tuners[vco_dest[0]], vco_code)) != NO_Si2141_44_24_ERROR)
    {
      SiTRACE ("Error returned from SiLabs_TER_Tuner_Block_VCO_Code error = %d\n", errcode);
      return errcode;
    }
  }
  if (tuner_count >  2) {
    if ((errcode=Tuner_Block_VCO_ptr[vco_fn[1]]  (tuners[vco_dest[1]], vco_code)) != NO_Si2141_44_24_ERROR)
    {
      SiTRACE ("Error returned from SiLabs_TER_Tuner_Block_VCO2_Code error = %d\n", errcode);
      return errcode;
    }
  }
  if (tuner_count >  3) {
    if ((errcode=Tuner_Block_VCO_ptr[vco_fn[2]]  (tuners[vco_dest[2]], vco_code)) != NO_Si2141_44_24_ERROR)
    {
      SiTRACE ("Error returned from SiLabs_TER_Tuner_Block_VCO3_Code error = %d\n", errcode);
      return errcode;
    }
  }

  return NO_Si2141_44_24_ERROR;
}










