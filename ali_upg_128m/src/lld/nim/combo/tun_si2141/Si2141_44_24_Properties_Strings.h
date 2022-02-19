/*************************************************************************************
                  Silicon Laboratories Broadcast Si2141_44_24 Layer 1 API

   EVALUATION AND USE OF THIS SOFTWARE IS SUBJECT TO THE TERMS AND CONDITIONS OF
     THE SOFTWARE LICENSE AGREEMENT IN THE DOCUMENTATION FILE CORRESPONDING
     TO THIS SOURCE FILE.
   IF YOU DO NOT AGREE TO THE LIMITED LICENSE AND CONDITIONS OF SUCH AGREEMENT,
     PLEASE RETURN ALL SOURCE FILES TO SILICON LABORATORIES.

   API properties strings definitions
   FILE: Si2141_44_24_Properties_Strings.h
   Supported IC : Si2141-A10, Si2141-B10, Si2144-A20, Si2124-A20
   Compiled for ROM 61 firmware 1_1_build_10
   Revision: 0.1
   Date: July 24 2015
   Tag: ROM61_1_1_build_10_V0.1
  (C) Copyright 2015, Silicon Laboratories, Inc. All rights reserved.
**************************************************************************************/
#ifndef   _Si2141_44_24_PROPERTIES_STRINGS_H_
#define   _Si2141_44_24_PROPERTIES_STRINGS_H_

#ifdef Si2141_44_24_COMMAND_LINE_APPLICATION

extern Si2141_44_24_PropObj Si2141_44_24_prop;

typedef enum   {
  Si2141_44_24_UNSIGNED_CHAR,
  Si2141_44_24_SIGNED_CHAR,
  Si2141_44_24_UNSIGNED_INT,
  Si2141_44_24_SIGNED_INT
} Si2141_44_24_datatypeEnum;

typedef struct {
  const char *name;
  int         value;
} Si2141_44_24_optionStruct;

typedef enum {
  Si2141_44_24_COMMON,
  Si2141_44_24_DTV,
  Si2141_44_24_TUNER,
  Si2141_TUNER,
  Si2144_TUNER
} Si2141_44_24_featureEnum;

typedef struct {
  const char          *name;
  Si2141_44_24_datatypeEnum  datatype;
  void                *pField;
  int                  nbOptions;
  Si2141_44_24_optionStruct *option;
} Si2141_44_24_fieldDicoStruct;

typedef struct {
  Si2141_44_24_featureEnum      feature;
  const char             *name;
  int                     nbFields;
  Si2141_44_24_fieldDicoStruct *field;
  unsigned int            propertyCode;
} Si2141_44_24_propertyInfoStruct;


extern Si2141_44_24_optionStruct    Si2144_ACTIVE_LOOP_THROUGH_PROP_ENABLE[];
extern Si2141_44_24_fieldDicoStruct Si2144_ACTIVE_LOOP_THROUGH_PROP_FIELDS[];

extern Si2141_44_24_optionStruct    Si2141_44_24_CRYSTAL_TRIM_PROP_XO_CAP[];
extern Si2141_44_24_fieldDicoStruct Si2141_44_24_CRYSTAL_TRIM_PROP_FIELDS[];

extern Si2141_44_24_optionStruct    Si2141_44_24_DTV_AGC_AUTO_FREEZE_PROP_THLD[];
extern Si2141_44_24_optionStruct    Si2141_44_24_DTV_AGC_AUTO_FREEZE_PROP_TIMEOUT[];
extern Si2141_44_24_fieldDicoStruct Si2141_44_24_DTV_AGC_AUTO_FREEZE_PROP_FIELDS[];

extern Si2141_44_24_optionStruct    Si2141_44_24_DTV_AGC_SPEED_PROP_IF_AGC_SPEED[];
extern Si2141_44_24_optionStruct    Si2141_44_24_DTV_AGC_SPEED_PROP_AGC_DECIM[];
extern Si2141_44_24_fieldDicoStruct Si2141_44_24_DTV_AGC_SPEED_PROP_FIELDS[];

extern Si2141_44_24_optionStruct    Si2141_44_24_DTV_CONFIG_IF_PORT_PROP_DTV_OUT_TYPE[];
extern Si2141_44_24_fieldDicoStruct Si2141_44_24_DTV_CONFIG_IF_PORT_PROP_FIELDS[];

extern Si2141_44_24_optionStruct    Si2141_44_24_DTV_EXT_AGC_PROP_MIN_10MV[];
extern Si2141_44_24_optionStruct    Si2141_44_24_DTV_EXT_AGC_PROP_MAX_10MV[];
extern Si2141_44_24_fieldDicoStruct Si2141_44_24_DTV_EXT_AGC_PROP_FIELDS[];

extern Si2141_44_24_optionStruct    Si2141_44_24_DTV_IF_AGC_SPEED_PROP_ATTACK[];
extern Si2141_44_24_optionStruct    Si2141_44_24_DTV_IF_AGC_SPEED_PROP_DECAY[];
extern Si2141_44_24_fieldDicoStruct Si2141_44_24_DTV_IF_AGC_SPEED_PROP_FIELDS[];

extern Si2141_44_24_optionStruct    Si2141_44_24_DTV_INITIAL_AGC_SPEED_PROP_IF_AGC_SPEED[];
extern Si2141_44_24_optionStruct    Si2141_44_24_DTV_INITIAL_AGC_SPEED_PROP_AGC_DECIM[];
extern Si2141_44_24_fieldDicoStruct Si2141_44_24_DTV_INITIAL_AGC_SPEED_PROP_FIELDS[];

extern Si2141_44_24_optionStruct    Si2141_44_24_DTV_INITIAL_AGC_SPEED_PERIOD_PROP_PERIOD[];
extern Si2141_44_24_fieldDicoStruct Si2141_44_24_DTV_INITIAL_AGC_SPEED_PERIOD_PROP_FIELDS[];

extern Si2141_44_24_optionStruct    Si2141_44_24_DTV_INTERNAL_ZIF_PROP_ATSC[];
extern Si2141_44_24_optionStruct    Si2141_44_24_DTV_INTERNAL_ZIF_PROP_QAM_US[];
extern Si2141_44_24_optionStruct    Si2141_44_24_DTV_INTERNAL_ZIF_PROP_DVBT[];
extern Si2141_44_24_optionStruct    Si2141_44_24_DTV_INTERNAL_ZIF_PROP_DVBC[];
extern Si2141_44_24_optionStruct    Si2141_44_24_DTV_INTERNAL_ZIF_PROP_ISDBT[];
extern Si2141_44_24_optionStruct    Si2141_44_24_DTV_INTERNAL_ZIF_PROP_ISDBC[];
extern Si2141_44_24_optionStruct    Si2141_44_24_DTV_INTERNAL_ZIF_PROP_DTMB[];
extern Si2141_44_24_fieldDicoStruct Si2141_44_24_DTV_INTERNAL_ZIF_PROP_FIELDS[];

extern Si2141_44_24_optionStruct    Si2141_44_24_DTV_LIF_FREQ_PROP_OFFSET[];
extern Si2141_44_24_fieldDicoStruct Si2141_44_24_DTV_LIF_FREQ_PROP_FIELDS[];

extern Si2141_44_24_optionStruct    Si2141_44_24_DTV_LIF_OUT_PROP_OFFSET[];
extern Si2141_44_24_optionStruct    Si2141_44_24_DTV_LIF_OUT_PROP_AMP[];
extern Si2141_44_24_fieldDicoStruct Si2141_44_24_DTV_LIF_OUT_PROP_FIELDS[];

extern Si2141_44_24_optionStruct    Si2141_44_24_DTV_MODE_PROP_BW[];
extern Si2141_44_24_optionStruct    Si2141_44_24_DTV_MODE_PROP_MODULATION[];
extern Si2141_44_24_optionStruct    Si2141_44_24_DTV_MODE_PROP_INVERT_SPECTRUM[];
extern Si2141_44_24_fieldDicoStruct Si2141_44_24_DTV_MODE_PROP_FIELDS[];

extern Si2141_44_24_optionStruct    Si2141_44_24_DTV_PGA_LIMITS_PROP_MIN[];
extern Si2141_44_24_optionStruct    Si2141_44_24_DTV_PGA_LIMITS_PROP_MAX[];
extern Si2141_44_24_fieldDicoStruct Si2141_44_24_DTV_PGA_LIMITS_PROP_FIELDS[];

extern Si2141_44_24_optionStruct    Si2141_44_24_DTV_PGA_TARGET_PROP_PGA_TARGET[];
extern Si2141_44_24_optionStruct    Si2141_44_24_DTV_PGA_TARGET_PROP_OVERRIDE_ENABLE[];
extern Si2141_44_24_fieldDicoStruct Si2141_44_24_DTV_PGA_TARGET_PROP_FIELDS[];

extern Si2141_44_24_optionStruct    Si2141_44_24_DTV_RF_AGC_SPEED_PROP_ATTACK[];
extern Si2141_44_24_optionStruct    Si2141_44_24_DTV_RF_AGC_SPEED_PROP_DECAY[];
extern Si2141_44_24_fieldDicoStruct Si2141_44_24_DTV_RF_AGC_SPEED_PROP_FIELDS[];

extern Si2141_44_24_optionStruct    Si2141_44_24_DTV_RF_TOP_PROP_DTV_RF_TOP[];
extern Si2141_44_24_fieldDicoStruct Si2141_44_24_DTV_RF_TOP_PROP_FIELDS[];

extern Si2141_44_24_optionStruct    Si2141_44_24_DTV_RSQ_RSSI_THRESHOLD_PROP_LO[];
extern Si2141_44_24_fieldDicoStruct Si2141_44_24_DTV_RSQ_RSSI_THRESHOLD_PROP_FIELDS[];

extern Si2141_44_24_optionStruct    Si2141_44_24_DTV_WB_AGC_SPEED_PROP_ATTACK[];
extern Si2141_44_24_optionStruct    Si2141_44_24_DTV_WB_AGC_SPEED_PROP_DECAY[];
extern Si2141_44_24_fieldDicoStruct Si2141_44_24_DTV_WB_AGC_SPEED_PROP_FIELDS[];

extern Si2141_44_24_optionStruct    Si2141_44_24_TUNER_BLOCKED_VCO_PROP_VCO_CODE[];
extern Si2141_44_24_fieldDicoStruct Si2141_44_24_TUNER_BLOCKED_VCO_PROP_FIELDS[];

extern Si2141_44_24_optionStruct    Si2141_44_24_TUNER_BLOCKED_VCO2_PROP_VCO_CODE[];
extern Si2141_44_24_fieldDicoStruct Si2141_44_24_TUNER_BLOCKED_VCO2_PROP_FIELDS[];

extern Si2141_44_24_optionStruct    Si2141_44_24_TUNER_BLOCKED_VCO3_PROP_VCO_CODE[];
extern Si2141_44_24_fieldDicoStruct Si2141_44_24_TUNER_BLOCKED_VCO3_PROP_FIELDS[];

extern Si2141_44_24_optionStruct    Si2141_44_24_TUNER_LO_INJECTION_PROP_BAND_1[];
extern Si2141_44_24_optionStruct    Si2141_44_24_TUNER_LO_INJECTION_PROP_BAND_2[];
extern Si2141_44_24_optionStruct    Si2141_44_24_TUNER_LO_INJECTION_PROP_BAND_3[];
extern Si2141_44_24_fieldDicoStruct Si2141_44_24_TUNER_LO_INJECTION_PROP_FIELDS[];

extern Si2141_44_24_optionStruct    Si2141_TUNER_RETURN_LOSS_PROP_CONFIG[];
extern Si2141_44_24_optionStruct    Si2141_TUNER_RETURN_LOSS_PROP_MODE[];
extern Si2141_44_24_optionStruct    Si2141_TUNER_RETURN_LOSS_PROP_RESERVED[];
extern Si2141_44_24_fieldDicoStruct Si2141_TUNER_RETURN_LOSS_PROP_FIELDS[];

extern Si2141_44_24_optionStruct    Si2144_TUNER_RETURN_LOSS_PROP_RESERVED[];
extern Si2141_44_24_optionStruct Si2144_TUNER_RETURN_LOSS_PROP_RETURNLOSS[];
extern Si2141_44_24_fieldDicoStruct Si2144_TUNER_RETURN_LOSS_PROP_FIELDS[];

extern Si2141_44_24_optionStruct    Si2141_TUNER_RETURN_LOSS_OPTIMIZE_PROP_THLD[];
extern Si2141_44_24_optionStruct    Si2141_TUNER_RETURN_LOSS_OPTIMIZE_PROP_CONFIG[];
extern Si2141_44_24_optionStruct    Si2141_TUNER_RETURN_LOSS_OPTIMIZE_PROP_ENGAGEMENT_DELAY[];
extern Si2141_44_24_optionStruct    Si2141_TUNER_RETURN_LOSS_OPTIMIZE_PROP_DISENGAGEMENT_DELAY[];
extern Si2141_44_24_fieldDicoStruct Si2141_TUNER_RETURN_LOSS_OPTIMIZE_PROP_FIELDS[];

extern Si2141_44_24_optionStruct    Si2141_TUNER_RETURN_LOSS_OPTIMIZE_2_PROP_THLD[];
extern Si2141_44_24_optionStruct    Si2141_TUNER_RETURN_LOSS_OPTIMIZE_2_PROP_WINDOW[];
extern Si2141_44_24_optionStruct    Si2141_TUNER_RETURN_LOSS_OPTIMIZE_2_PROP_ENGAGEMENT_DELAY[];
extern Si2141_44_24_fieldDicoStruct Si2141_TUNER_RETURN_LOSS_OPTIMIZE_2_PROP_FIELDS[];

extern Si2141_44_24_optionStruct    Si2141_TUNER_TF1_BOUNDARY_OFFSET_PROP_TF1_BOUNDARY_OFFSET[];
extern Si2141_44_24_fieldDicoStruct Si2141_TUNER_TF1_BOUNDARY_OFFSET_PROP_FIELDS[];

extern Si2141_44_24_optionStruct    Si2141_44_24_WIDE_BAND_ATT_THRS_PROP_WB_ATT_THRS[];
extern Si2141_44_24_fieldDicoStruct Si2141_44_24_WIDE_BAND_ATT_THRS_PROP_FIELDS[];

extern Si2141_44_24_optionStruct    Si2141_44_24_XOUT_PROP_AMP[];
extern Si2141_44_24_fieldDicoStruct Si2141_44_24_XOUT_PROP_FIELDS[];

extern Si2141_44_24_propertyInfoStruct Si2141_44_24_propertyDictionary[];

int   Si2141_44_24_PropertyNames       (L1_Si2141_44_24_Context *api, char *msg);
int   Si2141_44_24_PropertyIndex       (L1_Si2141_44_24_Context *api, const char *property);
int   Si2141_44_24_PropertyFields      (L1_Si2141_44_24_Context *api, int propIndex, char *msg);
int   Si2141_44_24_PropertyFieldIndex  (L1_Si2141_44_24_Context *api, int propIndex, char *field);
int   Si2141_44_24_PropertyFieldEnums  (L1_Si2141_44_24_Context *api, int propIndex, int fieldIndex, char *msg);
int   Si2141_44_24_GetValueFromEntry   (L1_Si2141_44_24_Context *api, Si2141_44_24_fieldDicoStruct field, char *entry, int *value);
char *Si2141_44_24_GetEnumFromValue    (L1_Si2141_44_24_Context *api, Si2141_44_24_fieldDicoStruct field, int iValue);
char *Si2141_44_24_GetEnumFromField    (L1_Si2141_44_24_Context *api, Si2141_44_24_fieldDicoStruct field);
int   Si2141_44_24_SetFieldFromEntry   (L1_Si2141_44_24_Context *api, Si2141_44_24_fieldDicoStruct field, char *entry);
void  Si2141_44_24_showProperty        (L1_Si2141_44_24_Context *api, int propertyIndex);
void  Si2141_44_24_showPropertyNamed   (L1_Si2141_44_24_Context *api, char *property);
int   Si2141_44_24_showProperties      (L1_Si2141_44_24_Context *api, Si2141_44_24_featureEnum feature);
void  Si2141_44_24_setupProperty       (L1_Si2141_44_24_Context *api, int propIndex);
int   Si2141_44_24_setupProperties     (L1_Si2141_44_24_Context *api, Si2141_44_24_featureEnum feature);

#endif /* Si2141_44_24_COMMAND_LINE_APPLICATION */


int  Si2141_44_24_setupCOMMONProperties(L1_Si2141_44_24_Context *api);
int  Si2141_44_24_setupDTVProperties   (L1_Si2141_44_24_Context *api);
int  Si2141_44_24_setupTUNERProperties (L1_Si2141_44_24_Context *api);
int  Si2141_44_24_setupAllProperties   (L1_Si2141_44_24_Context *api);

#endif /* _Si2141_44_24_PROPERTIES_STRINGS_H_ */







