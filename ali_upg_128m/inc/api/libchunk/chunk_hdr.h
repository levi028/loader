/*****************************************************************************
*    Copyright (c) 2015 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_otaupg.h
*
*    Description:   The realize of chunk header verify
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _CHUNK_HDR_H_
#define _CHUNK_HDR_H_
#ifdef __cplusplus
extern "C"
{
#endif

enum Chunk_hmac_type
{
    HMAC_STATIC_HDRCHUNK=0,
    HMAC_STATIC_ALLHDR,
    HMAC_STATIC_TOTAL, //HDRCHUNK + ALLHDR
    HMAC_DYNAMIC_HDRCHUNK,
    HMAC_DYNAMIC_ALLHDR,
    HMAC_DYNAMIC_TOTAL,//HDRCHUNK + ALLHDR
};

/*****************************************************************************
 * Function: verify_hdrchunk_hmac
 * Description: verfify static/dynamic data chunk's HMAC
 * Input: 
 *      Para 1: UINT8 type, should be HMAC_STATIC_HDRCHUNK or HMAC_DYNAMIC_HDRCHUNK
 * Output: None
 * Returns: RET_SUCCESS:verify OK, others: verfiy fail
*****************************************************************************/
RET_CODE verify_hdrchunk_hmac(UINT8 type);
/*****************************************************************************
 * Function: verify_allhdr_hmac
 * Description: verfify all static/dynamic hdr's HMAC
 * Input: 
 *      Para 1: UINT8 type, should be HMAC_STATIC_ALLHDR or HMAC_DYNAMIC_ALLHDR
 * Output: None
 * Returns: RET_SUCCESS:verify OK, others: verfiy fail
*****************************************************************************/
RET_CODE verify_allhdr_hmac(UINT8 type);
/*****************************************************************************
 * Function: update_allhdr_hmac
 * Description: calculate all static/dynamic hdr and static/dynamic chunk's HMAC, then write to flash
 * Input: 
 *      Para 1: UINT8 type, should be HMAC_STATIC_TOTAL or HMAC_DYNAMIC_TOTAL
 * Output: None
 * Returns: RET_SUCCESS:update OK, others: update fail
*****************************************************************************/
RET_CODE update_allhdr_hmac(UINT8 type);
/*****************************************************************************
 * Function: verify_otapara_hmac
 * Description: verfify ota parameter chunk's HMAC
 * Input: 
 *      None
 * Output: None
 * Returns: RET_SUCCESS:verify OK, others: verfiy fail
*****************************************************************************/
RET_CODE verify_otapara_hmac();
/*****************************************************************************
 * Function: update_otapara_hmac
 * Description: calculate ota parameter's HMAC, then write to flash
 * Input: 
 *      Para 1: UINT8 *p_ota_para, point to new ota parmeter(exclude ota parameter chunk header)
 *      Para 2: const UINT8 src_len, new ota parmeter length
 * Output: None
 * Returns: RET_SUCCESS:update OK, others: update fail
*****************************************************************************/
RET_CODE update_otapara_hmac(UINT8 *p_ota_para , const UINT16 src_len); 
//RET_CODE update_otaloader_hmac(UINT8 *p_src, UINT32 src_len);
//UINT8 is_hdr_missing(UINT32 total_size);
#ifdef __cplusplus
}  /* End of the 'extern "C"' block */
#endif

#endif

