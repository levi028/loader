/*************************************************************************************
                  Silicon Laboratories Broadcast Si2141_44_24 Layer 1 API

   EVALUATION AND USE OF THIS SOFTWARE IS SUBJECT TO THE TERMS AND CONDITIONS OF
     THE SOFTWARE LICENSE AGREEMENT IN THE DOCUMENTATION FILE CORRESPONDING
     TO THIS SOURCE FILE.
   IF YOU DO NOT AGREE TO THE LIMITED LICENSE AND CONDITIONS OF SUCH AGREEMENT,
     PLEASE RETURN ALL SOURCE FILES TO SILICON LABORATORIES.

   API functions definitions used by commands and properties
   FILE: Si2141_44_24_L1_API.c
   Supported IC : Si2141-A10, Si2141-B10, Si2144-A20, Si2124-A20
   Compiled for ROM 61 firmware 1_1_build_10
   Revision: 0.1
   Tag:  ROM61_1_1_build_10_V0.1
   Date: July 24 2015
  (C) Copyright 2015, Silicon Laboratories, Inc. All rights reserved.
**************************************************************************************/
#define   Si2141_44_24_COMMAND_PROTOTYPES

#include "Si2141_44_24_L1_API.h"


/***********************************************************************************************************************
  Si2141_44_24_L1_API_Init function
  Use:        software initialisation function
              Used to initialize the software context
  Returns:    0 if no error
  Comments:   It should be called first and once only when starting the application
  Parameter:   **ppapi         a pointer to the api context to initialize
  Parameter:  add            the Si2141_44_24 I2C address
  Porting:    Allocation errors need to be properly managed.
  Porting:    I2C initialization needs to be adapted to use the available I2C functions
 ***********************************************************************************************************************/
unsigned char    Si2141_44_24_L1_API_Init      (L1_Si2141_44_24_Context *api, int add) {
    api->i2c = &(api->i2cObj);

    L0_Init(api->i2c);
    L0_SetAddress(api->i2c, add, 0);

    api->cmd               = &(api->cmdObj);
    api->rsp               = &(api->rspObj);
    api->prop              = &(api->propObj);
    api->status            = &(api->statusObj);
    api->propShadow        = &(api->propShadowObj);
    /* Set the propertyWriteMode to Si2141_44_24_DOWNLOAD_ON_CHANGE to only download property settings on change (recommended) */
    /*      if propertyWriteMode is set to Si2141_44_24_DOWNLOAD_ALWAYS the properties will be downloaded regardless of change */
    api->propertyWriteMode = DOWNLOAD_ON_CHANGE;
    /* The load_control flag is used to control the interaction between Si2141_44_24_PowerUpWithPatch() and Si2141_44_24_PowerUpUsingBroadcastI2C().  */
    /* the default is SKIP_NONE */
    api->load_control = SKIP_NONE;
    api->rsp->get_rev.pn = 0;
    #ifdef    Si2141_44_24_TUNER_BLOCKED_VCO_PROP
        api->prop->tuner_blocked_vco.vco_code           = 0x8000; /* (default 0x8000) */
    #endif /* Si2141_44_24_TUNER_BLOCKED_VCO_PROP */
    #ifdef    Si2141_44_24_TUNER_BLOCKED_VCO2_PROP
        api->prop->tuner_blocked_vco2.vco_code          = 0x8000; /* (default 0x8000) */
    #endif /* Si2141_44_24_TUNER_BLOCKED_VCO2_PROP */
    #ifdef    Si2141_44_24_TUNER_BLOCKED_VCO3_PROP
        api->prop->tuner_blocked_vco3.vco_code          = 0x8000; /* (default 0x8000) */
    #endif /* Si2141_44_24_TUNER_BLOCKED_VCO3_PROP */
    return NO_Si2141_44_24_ERROR;
}
/***********************************************************************************************************************
  Si2141_44_24_L1_API_Patch function
  Use:        Patch information function
              Used to send a number of bytes to the Si2141_44_24. Useful to download the firmware.
  Returns:    0 if no error
  Parameter:  error_code the error code.
  Porting:    Useful for application development for debug purposes.
  Porting:    May not be required for the final application, can be removed if not used.
 ***********************************************************************************************************************/
unsigned char    Si2141_44_24_L1_API_Patch     (L1_Si2141_44_24_Context *api, int iNbBytes, unsigned char *pucDataBuffer) {
    int res;
    unsigned char rspByteBuffer[1];

    SiTRACE("Si2141_44_24 Patch %d bytes\n",iNbBytes);

    res = L0_WriteCommandBytes(api->i2c, iNbBytes, pucDataBuffer);
    if (res!=iNbBytes) {
      SiTRACE("Si2141_44_24_L1_API_Patch error writing bytes: %s\n", Si2141_44_24_L1_API_ERROR_TEXT(ERROR_Si2141_44_24_LOADING_FIRMWARE) );
      return ERROR_Si2141_44_24_LOADING_FIRMWARE;
    }

    res = Si2141_44_24_pollForResponse(api, 1, rspByteBuffer);
    if (res != NO_Si2141_44_24_ERROR) {
      SiTRACE("Si2141_44_24_L1_API_Patch error 0x%02x polling response: %s\n", res, Si2141_44_24_L1_API_ERROR_TEXT(res) );
      return ERROR_Si2141_44_24_POLLING_RESPONSE;
    }

    return NO_Si2141_44_24_ERROR;
}
/***********************************************************************************************************************
  Si2141_44_24_L1_CheckStatus function
  Use:        Status information function
              Used to retrieve the status byte
  Returns:    0 if no error
  Parameter:  error_code the error code.
 ***********************************************************************************************************************/
unsigned char    Si2141_44_24_L1_CheckStatus   (L1_Si2141_44_24_Context *api) {
    unsigned char rspByteBuffer[1];
    return Si2141_44_24_pollForResponse(api, 1, rspByteBuffer);
}
/***********************************************************************************************************************
  Si2141_44_24_L1_API_ERROR_TEXT function
  Use:        Error information function
              Used to retrieve a text based on an error code
  Returns:    the error text
  Parameter:  error_code the error code.
  Porting:    Useful for application development for debug purposes.
  Porting:    May not be required for the final application, can be removed if not used.
 ***********************************************************************************************************************/
char*            Si2141_44_24_L1_API_ERROR_TEXT(int error_code) {
    switch (error_code) {
        case NO_Si2141_44_24_ERROR                     : return (char *)"No Si2141_44_24 error";
        case ERROR_Si2141_44_24_ALLOCATING_CONTEXT     : return (char *)"Error while allocating Si2141_44_24 context";
        case ERROR_Si2141_44_24_PARAMETER_OUT_OF_RANGE : return (char *)"Si2141_44_24 parameter(s) out of range";
        case ERROR_Si2141_44_24_SENDING_COMMAND        : return (char *)"Error while sending Si2141_44_24 command";
        case ERROR_Si2141_44_24_CTS_TIMEOUT            : return (char *)"Si2141_44_24 CTS timeout";
        case ERROR_Si2141_44_24_ERR                    : return (char *)"Si2141_44_24 Error (status 'err' bit 1)";
        case ERROR_Si2141_44_24_POLLING_CTS            : return (char *)"Si2141_44_24 Error while polling CTS";
        case ERROR_Si2141_44_24_POLLING_RESPONSE       : return (char *)"Si2141_44_24 Error while polling response";
        case ERROR_Si2141_44_24_LOADING_FIRMWARE       : return (char *)"Si2141_44_24 Error while loading firmware";
        case ERROR_Si2141_44_24_LOADING_BOOTBLOCK      : return (char *)"Si2141_44_24 Error while loading bootblock";
        case ERROR_Si2141_44_24_STARTING_FIRMWARE      : return (char *)"Si2141_44_24 Error while starting firmware";
        case ERROR_Si2141_44_24_SW_RESET               : return (char *)"Si2141_44_24 Error during software reset";
        case ERROR_Si2141_44_24_INCOMPATIBLE_PART      : return (char *)"Si2141_44_24 Error Incompatible part";
        case ERROR_Si2141_44_24_UNKNOWN_COMMAND        : return (char *)"Si2141_44_24 Error unknown command";
        case ERROR_Si2141_44_24_UNKNOWN_PROPERTY       : return (char *)"Si2141_44_24 Error unknown property";
        case ERROR_Si2141_44_24_TUNINT_TIMEOUT         : return (char *)"Si2141_44_24 Error TUNINT Timeout";
        case ERROR_Si2141_44_24_xTVINT_TIMEOUT         : return (char *)"Si2141_44_24 Error xTVINT Timeout";
        default                                  : return (char *)"Unknown Si2141_44_24 error code";
    }
}
/***********************************************************************************************************************
  Si2141_44_24_L1_API_TAG_TEXT function
  Use:        Error information function
              Used to retrieve a text containing the TAG information (related to the code version)
  Returns:    the TAG text
  Porting:    May not be required for the final application, can be removed if not used.
 ***********************************************************************************************************************/
char*            Si2141_44_24_L1_API_TAG_TEXT(void) { return (char *)"ROM61_1_1_build_10_V0.1";}








