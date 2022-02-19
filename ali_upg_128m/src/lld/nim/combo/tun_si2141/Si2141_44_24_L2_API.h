/***************************************************************************************
                  Silicon Laboratories Broadcast Si2141_44_24 Layer 2 API

   EVALUATION AND USE OF THIS SOFTWARE IS SUBJECT TO THE TERMS AND CONDITIONS OF
     THE SOFTWARE LICENSE AGREEMENT IN THE DOCUMENTATION FILE CORRESPONDING
     TO THIS SOURCE FILE.
   IF YOU DO NOT AGREE TO THE LIMITED LICENSE AND CONDITIONS OF SUCH AGREEMENT,
     PLEASE RETURN ALL SOURCE FILES TO SILICON LABORATORIES.

   L2 API header for commands and properties
   FILE: Si2141_44_24_L2_API.h
   Supported IC : Si2141-A10, Si2141-B10, Si2144-A20, Si2124-A20
   Compiled for ROM 61 firmware 1_1_build_10
   Revision: 0.1
   Date: July 24 2015
  (C) Copyright 2015, Silicon Laboratories, Inc. All rights reserved.
****************************************************************************************/

#ifndef   Si2141_44_24_L2_API_H
#define   Si2141_44_24_L2_API_H

#include "Si2141_44_24_L1_API.h"
#include "Si2141_44_24_Properties_Strings.h"

#ifdef    TUNER_ONLY
/* define some externals, required for VCO_BLOCKING */
/* only used for TUNER_ONLY mode */
extern int fe;
extern L1_Si2141_44_24_Context  FrontEnd_Table[];
#endif // TUNER_ONLY

int  Si2141_44_24_Init                      (L1_Si2141_44_24_Context *api);
int  Si2141_44_24_Configure                 (L1_Si2141_44_24_Context *api);
int  Si2141_44_24_PowerUpWithPatch          (L1_Si2141_44_24_Context *api);
int Si2141_44_24_PowerUpUsingBroadcastI2C    (L1_Si2141_44_24_Context *tuners[], int tuner_count );
int  Si2141_44_24_LoadFirmware              (L1_Si2141_44_24_Context *api, unsigned char *fw_table, int lines);
int  Si2141_44_24_StartFirmware             (L1_Si2141_44_24_Context *api);

int Si2141_44_24_LoadFirmware_16    (L1_Si2141_44_24_Context *api, firmware_struct fw_table[], int nbLines);
int Si2141_44_24_UpdateChannelScanFrequency(int freq,int channelsFound);
int Si2141_44_24_GetRF                 (L1_Si2141_44_24_Context *api);
int Si2141_44_24_Tune                  (L1_Si2141_44_24_Context *api, unsigned char mode, unsigned long freq);
int Si2141_44_24_DTVTune               (L1_Si2141_44_24_Context *api, unsigned long freq, unsigned char bw, unsigned char modulation, unsigned char invert_spectrum);
int Si2141_44_24_XoutOn                (L1_Si2141_44_24_Context *api);
int Si2141_44_24_XoutOff               (L1_Si2141_44_24_Context *api);
int Si2141_44_24_Standby               (L1_Si2141_44_24_Context *api);
int Si2141_44_24_Powerdown             (L1_Si2141_44_24_Context *api);
int Si2141_44_24_LoadVideofilter       (L1_Si2141_44_24_Context *api, vid_filt_struct vidFiltTable[], int lines);
int Si2141_44_24_AGC_Override    (L1_Si2141_44_24_Context *api, unsigned char mode );
int Si2141_44_24_Tuner_Block_VCO       (L1_Si2141_44_24_Context *api, int vco_code);
int Si2141_44_24_Tuner_Block_VCO2       (L1_Si2141_44_24_Context *api, int vco_code);
int Si2141_44_24_Tuner_Block_VCO3       (L1_Si2141_44_24_Context *api, int vco_code);
int Si2141_44_24_L2_VCO_Blocking_PostTune(L1_Si2141_44_24_Context *tuners[], int tuner_num, int tuner_count);
int Si2141_44_24_L2_VCO_Blocking_PreTune(L1_Si2141_44_24_Context *tuners[], int tuner_num, int tuner_count);
int Si2141_44_24_SetVcoProperties (L1_Si2141_44_24_Context *api);

#define Si2141_44_24_FORCE_NORMAL_AGC       0
#define Si2141_44_24_FORCE_MAX_AGC          1
#define Si2141_44_24_FORCE_TOP_AGC          2

#endif /* Si2141_44_24_L2_API_H */







