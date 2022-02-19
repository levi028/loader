/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: lib_ota_parse.h
*
* Description:
*     the file is for parse ota section data
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef  _LIB_OTA_INTERNAL_H_
#define _LIB_OTA_INTERNAL_H_
#ifdef __cplusplus
extern "C"
{
#endif

#define DC_MODULE_MAX           10

typedef void (*t_burn_progress_disp)(UINT32);
typedef INT32 (*ota_nit_callback)(UINT8 , void*);


extern  UINT32       g_oui;
extern  UINT16       g_model;
extern  UINT16       g_hw_version;
extern  UINT16       g_sw_model;
extern  UINT16       g_sw_version;
extern struct dl_info   g_dl_info;
extern  UINT8        g_dc_module_num;
//extern  UINT16       g_dc_blocksize0;
extern  struct dcgroup_info  g_dc_group;
extern  struct dcmodule_info g_dc_module[DC_MODULE_MAX];
extern  struct service_info  g_service_info;
extern  t_progress_disp      download_progress_disp;
extern  UINT8        *g_ota_ram1_addr;
extern  UINT16       g_dc_blocksize;

#ifdef LIB_TSI3_FULL
/*****************************************************************************
 * Function: parse_nit_tp
 * Description:
 *    use for parse NIT table.
 * Input:
 *      Para 1: UINT8 *data, the raw data of section buffer.
 *      Para 2: INT32 data_len, the length of section buffer.
 *      Para 3: void *param, for usage of the section parse.
 * Output:
 *      NONE
 * Returns: 0, success; !0, failure.
 *
*****************************************************************************/
INT32 parse_nit_tp(UINT8 *data, INT32 data_len, ota_nit_callback nit_progress);

/*****************************************************************************
 * Function: parse_nit
 * Description:
 *    use for parse NIT table.
 * Input:
 *      Para 1: UINT8 *data, the raw data of section buffer.
 *      Para 2: INT32 data_len, the length of section buffer.
 *      Para 3: UINT16 filter_stream_id, stream id for comparation.
 *      Para 4: void *param, for usage of the section parse.
 * Output:
 *      NONE
 * Returns: 0, success; !0, failure.
 *
*****************************************************************************/
INT32 parse_nit(UINT8 *data, INT32 data_len, UINT16 filter_stream_id,
    ota_nit_callback nit_progress);

/*****************************************************************************
 * Function: parse_nit_bat_2
 * Description:
 *    use for parse NIT table.
 * Input:
 *      Para 1: UINT8 *buffer, the raw data of section buffer.
 *      Para 2: INT32 buffer_length, the length of section buffer.
 *      Para 3: void *param, for usage of the section parse.
 * Output:
 *      NONE
 * Returns: 0, success; !0, failure.
 *
*****************************************************************************/
INT32 parse_nit_bat_2(UINT8 *buffer, INT32 buffer_length, void *param);
#else
INT32 parse_nit_bat_2(UINT8 *buffer, INT32 buffer_length, void *param);
#endif

/*****************************************************************************
 * Function: parse_nit_bat
 * Description:
 *    use for parse NIT table.
 * Input:
 *      Para 1: UINT8 *buffer, the raw data of section buffer.
 *      Para 2: INT32 buffer_length, the length of section buffer.
 *      Para 3: void *param, for usage of the section parse.
 * Output:
 *      NONE
 * Returns: 0, success; !0, failure.
 *
*****************************************************************************/
INT32 parse_nit_bat(UINT8 *buffer, INT32 buffer_length, void *param);


/*****************************************************************************
 * Function: parse_pat
 * Description:
 *    use for parse PAT table.
 * Input:
 *      Para 1: UINT8 *buffer, the raw data of section buffer.
 *      Para 2: INT32 buffer_length, the length of section buffer.
 *      Para 3: void *param, for usage of the section parse.
 * Output:
 *      NONE
 * Returns: 0, success; !0, failure.
 *
*****************************************************************************/
INT32 parse_pat(UINT8 *buffer, INT32 buffer_length, void *param);

/*****************************************************************************
 * Function: parse_pmt
 * Description:
 *    use for parse PMT table.
 * Input:
 *      Para 1: UINT8 *buffer, the raw data of section buffer.
 *      Para 2: INT32 buffer_length, the length of section buffer.
 *      Para 3: void *param, for usage of the section parse.
 * Output:
 *      NONE
 * Returns: 0, success; !0, failure.
 *
*****************************************************************************/
INT32 parse_pmt(UINT8 *buffer, INT32 buffer_length, void *param);

#ifdef __cplusplus
}
#endif
#endif

