/*************************************************************************************
                  Silicon Laboratories Broadcast Si2141_44_24 Layer 1 API

   EVALUATION AND USE OF THIS SOFTWARE IS SUBJECT TO THE TERMS AND CONDITIONS OF
     THE SOFTWARE LICENSE AGREEMENT IN THE DOCUMENTATION FILE CORRESPONDING
     TO THIS SOURCE FILE.
   IF YOU DO NOT AGREE TO THE LIMITED LICENSE AND CONDITIONS OF SUCH AGREEMENT,
     PLEASE RETURN ALL SOURCE FILES TO SILICON LABORATORIES.

   API properties functions definitions
   FILE: Si2141_44_24_Properties_Functions.h
   Supported IC : Si2141-A10, Si2141-B10, Si2144-A20, Si2124-A20
   Compiled for ROM 61 firmware 1_1_build_10
   Revision: 0.1
   Date: July 24 2015
  (C) Copyright 2015, Silicon Laboratories, Inc. All rights reserved.
**************************************************************************************/
#ifndef   _Si2141_44_24_PROPERTIES_FUNCTIONS_H_
#define   _Si2141_44_24_PROPERTIES_FUNCTIONS_H_

void          Si2141_44_24_storeUserProperties           (Si2141_44_24_PropObj   *prop, L1_Si2141_44_24_Context *api);
unsigned char Si2141_44_24_PackProperty                  (L1_Si2141_44_24_Context *api,Si2141_44_24_PropObj   *prop, unsigned int prop_code, int *data);
unsigned char Si2141_44_24_UnpackProperty                (L1_Si2141_44_24_Context *api,Si2141_44_24_PropObj   *prop, unsigned int prop_code, int  data);
void          Si2141_44_24_storePropertiesDefaults (Si2141_44_24_PropObj   *prop, L1_Si2141_44_24_Context *api);

int  Si2141_44_24_downloadCOMMONProperties(L1_Si2141_44_24_Context *api);
int  Si2141_44_24_downloadDTVProperties   (L1_Si2141_44_24_Context *api);
int  Si2141_44_24_downloadTUNERProperties (L1_Si2141_44_24_Context *api);
int  Si2141_44_24_downloadAllProperties   (L1_Si2141_44_24_Context *api);

#endif /* _Si2141_44_24_PROPERTIES_FUNCTIONS_H_ */







