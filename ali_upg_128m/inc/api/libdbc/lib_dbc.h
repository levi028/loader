/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: lib_dbc.h
*
* Description:
*     the file is for parsing OTA DII, DSI, ect data.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __LIB_DBC_H__
#define __LIB_DBC_H__
#ifdef __cplusplus
extern "C"
{
#endif


#include <basic_types.h>
#include <mediatypes.h>

#if 0
struct dsi_filter_param
{
    UINT16    new_sw_version;
    UINT32     data_code;
    UINT32     time_code;
    struct dcgroup_info* dc_group;
};
struct dsi_readback_param
{
    UINT32     oui;
    UINT16    hw_version;
    UINT16    sw_version;
};
struct dii_filter_param
{
    UINT32     group_id;
    struct dcmodule_info* dc_module;
    UINT8 dc_module_num;

};
struct dii_readback_param
{
    UINT16    dc_blocksize;
};
#endif

//32*32 blocks in a module = 4Mbytes in a module
#define BIT_MAP_NUM 32

struct dcmodule_info
{
    UINT16 module_id;
    UINT8 module_linked;
    UINT8 module_first;
    UINT8 module_last;
    UINT8 *module_buf_pointer;
    UINT32 module_buf_size;
    UINT16 block_num;
    UINT16 next_module_id;
    UINT16 next_block_num;
    UINT32 block_received[BIT_MAP_NUM];
    UINT8 module_download_finish;
};

struct dcgroup_info
{
    UINT32 group_id;
    UINT32 group_size;
#ifdef STAR_OTA_GROUP_INFO_BYTE
    UINT32 OUI;                // Organization Unique Identifier
    UINT32 stbid_start;        // Start of ID of STB need OTA upgrade
    UINT32 stbid_end;        // End of ID of STB need OTA upgrade
    UINT32 global_sw_ver;    // Global software version
    UINT32 global_hw_ver;    // Global hardware version
    UINT32 upg_ctrl;
#endif
/*
    UINT32 hw_oui;
    UINT16 hw_model;
    UINT16 hw_version;
    UINT32 sw_oui;
    UINT16 sw_model;
    UINT16 sw_version;
*/
};
#ifdef AUTO_OTA_SAME_VERSION_CHECK

typedef enum
{
    DO_NOT_CARE_SOFTWARE_VERSION,
    DIFFERENT_SOFTWARE_VERSION,
    HIGHER_SOFTWARE_VERSION,
} OTA_SOFTWARE_VERSION_ARBITRATION;

void set_ota_software_version_arbitration(OTA_SOFTWARE_VERSION_ARBITRATION software_version_arbitration);
#endif

/*****************************************************************************
 * Function: dsi_message
 * Description:
 *    parse DSI section data, get information from the data.
 * Input:
 *      Para 1: UINT8* data, the raw section data from stream.
 *      Para 2: UINT16 len, the lenghth of section data.
 *      Para 3: UINT32 OUI, OUI number
 *      Para 4: UINT16 hw_model, hardware model for compare
 *      Para 5: UINT16 hw_version, hardware version for compare
 *      Para 6: UINT16 sw_model, software version for compare
 *      Para 7: UINT16 sw_version, software version for compare
 * Output:
 *      Para 8: UINT16 *new_sw_version, store the new version
 *      Para 9: struct DCGroupInfo* group, save new information to the group
 *
 * Returns: BOOL
 *
*****************************************************************************/
BOOL dsi_message(UINT8* data,UINT16 len,UINT32 OUI,UINT16 hw_model,UINT16 hw_version,
    UINT16 sw_model,UINT16 sw_version,UINT16* new_sw_version,struct dcgroup_info* group);

/*****************************************************************************
 * Function: dii_message
 * Description:
 *    parse DII section data, get information from the data.
 * Input:
 *      Para 1: UINT8* data, the raw section data from stream.
 *      Para 2: UINT16 len, the lenghth of section data.
 *      Para 3: UINT32 group_id, group_id for comare
 *      Para 4: UINT8* data_addr, the buffer strore download data
 *      Para 5: struct DCModuleInfo* module, module information
 * Output:
 *      Para 6: UINT16 *new_sw_version, store the new version
 *      Para 7: UINT16* blocksize, tempory use form block size
 *
 * Returns: BOOL
 *
*****************************************************************************/
BOOL dii_message(UINT8* data,UINT16 len,UINT32 group_id,UINT8* data_addr,
    struct dcmodule_info* module,UINT8* module_num,UINT16* blocksize);

/*****************************************************************************
 * Function: dii_message
 * Description:
 *    parse DDB section data, get information from the data.
 * Input:
 *      Para 1: UINT8* data, the raw section data from stream.
 *      Para 2: UINT16 len, the lenghth of section data.
 *      Para 3: struct DCModuleInfo* module, module information
 *      Para 4: UINT16 blocksize, block size
 *      Para 5: UINT8 blk_id, blk_id for comare
 * Output:
 *      NONE
 *
 * Returns: BOOL
 *
*****************************************************************************/
BOOL ddb_data(UINT8* data,UINT16 len,struct dcmodule_info* module,UINT16 blocksize,UINT8 blk_id);


#ifdef __cplusplus
}
#endif
#endif /*__LIB_DBC_H__*/

