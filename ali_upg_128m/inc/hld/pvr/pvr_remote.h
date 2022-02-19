/*****************************************************************************
*    Copyright (c) 2015 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
*    disclosed to unauthorized individual.
*    File: pvr_remote.h
*
*    Description: this file is used to define some macros and structures
*                 for PVR remote functionality.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef  _PVR_REMOTE_H
#define  _PVR_REMOTE_H

#include <basic_types.h>
#include <alidefinition/adf_pvr.h>
#include <api/libc/list.h>


#ifdef __cplusplus
extern "C" {
#endif


//#define PVR_RPC_DEBUG
#ifdef PVR_RPC_DEBUG
#define PVR_RPC_PRINTF libc_printf
#define PVR_RPC_DUMP(data,len) \
	do{ \
		int i, l=(len); \
		for(i=0; i<l; i++){ \
			PVR_RPC_PRINTF("0x%x,",*(UINT8 *)((UINT32)data+i)); \
			if(0==((i+1)%16)) \
				PVR_RPC_PRINTF("\n");\
		}\
	}while(0)
#else
#define PVR_RPC_PRINTF(...)  do{}while(0)
#define PVR_RPC_DUMP(...)  do{}while(0)
#endif

#define PVR_RPC_NPARA(x) ((HLD_PVR_MODULE<<24)|(x<<16))

enum block_data_mode
{    
    BLOCK_DATA_MODE_PURE_DATA = 0,
    BLOCK_DATA_MODE_TS = 1,
};

enum decrypt_res_iv_param
{    
    KEY_IV_MODE_EVEN = 0,
    KEY_IV_MODE_ODD  = 1,
};

struct ca_session_attr {
	void *sub_dev_see_hdl;
		/*!< Pointer of the sub device handler at see for the current session. */
	unsigned short stream_id;
		/*!< Stream id of the current session. */
	int crypt_mode;
		/*!< Crypt mode of the current session. */        
    unsigned short sub_dev_id;
        /*sub dev id */
};


struct ca_key_attr {
	unsigned int key_handle;
		/*!< Key handle at see for the current key. */
	int kl_sel;
		/*!< KeyLadder key sel for the current key. */
	int key_pos;
		/*!< Key pos for the current key. */
};

struct decrypt_buffer_item
{
    unsigned int  buffer;
    unsigned int  length;
    unsigned int  state;    /*0,free;1,using*/
};

struct decrypt_buffer_mgr
{
    unsigned int                max_block_item_support;
    struct decrypt_buffer_item  *buf_item;
};


struct ali_pvr_capt_decrypt_res_param
{
    unsigned int      decrypt_hdl;                  //[output] the get decrypt handle
    unsigned int      decrypt_dsc_num;              //[input] decrypt dsc num; 
    unsigned int      block_data_size;              //[input] block data size,188byte integral multiple;
    unsigned char *buf;								//[output] buffer point
};

struct pvr_key_item
{
    unsigned int    switch_block_count;/*switch key block count*/
    PVR_RPC_RAW_DECRYPT_EVO decrypt_evo_info;/*decrypt dsc infomation*/
};

struct pvr_decrypt_res_param
{
    struct list_head list_head;
    unsigned int    decrypt_hdl;           /* decrypt_hdl*/
    unsigned char   magic[4];               /*magic number */

    unsigned int    block_data_size;        /* block data size */
    unsigned char   *block_data_buf;        /* block data buffer */
    unsigned int    dsc_handle_num;         /*decrypt dsc_num*/
    struct pvr_key_item *key_item_info;     /*decrypt dsc infomation*/
};

struct ali_pvr_set_decrypt_res_param
{
    unsigned int                decrypt_hdl;                  //[input] the get decrypt handle
    unsigned int                decrypt_index;                //[input] descrypt_index , form  0  to  decrypt_dsc_num - 1
    enum block_data_mode        dsc_mode;                     //[input] pure data mode and ts mode 
    unsigned int                decrypt_switch_block;         //[input] switch block 1 ~ 0xFFFFFFFF
    unsigned int                block_data_size;              //[input] every block size  
    enum decrypt_res_iv_param   iv_parity;                    //[input] iv parity
    unsigned int                iv_lenth;                     //[input] iv length
    unsigned char               *iv_data;                     //[input] iv data  
    unsigned int                key_handle;                   //[input] main dsc key handle 
    struct ca_session_attr  dsc_ca_attr;					  //[input] dsc resource
    struct ca_key_attr      dsc_key_attr;   				  //[input] kl resource
};

struct ali_pvr_data_decrypt_param
{
    unsigned int        decrypt_hdl;                //[input] the get decrypt handle
	UINT8               *input;                     //[input] input data block 
	UINT32              length;                     //[input] input data block lenth  
    UINT32              block_index;                  //[input] index of data block 0 ~ 0xFFFFFFFF-1 and 188byte integral multiple
    UINT32              block_vob_indicator;          //[input] block vob attribute:0:block normal,1: block start
                                                      // 2: block end
	enum pvr_ott_data_type	des_flag; /*flag: PVR_OTT_DATA_DMX -> DMX, PVR_OTT_DATA_VBV -> VBV, PVR_OTT_DATA_AUDIO -> AUDIO*/
	UINT32 iv_length;/*Initial vector length,if = 0,needn't update iv,or need update iv to dsc driver*/ 
	UINT32 *iv_data;  /*Initial vector*/
	UINT32 decv_id;	/*it is set to 0 under default,if pip is used,it is set in practice*/
};



/*! @struct pvr_key_param_rescourse
 *   @brief Define PVR key parameters.
 */
typedef struct pvr_key_param_resourse
{
    UINT32 is_used; //0:unused ,1:used
    UINT32 total_qn_num;
    UINT32 cur_qu_num;
    UINT32 ts_packet_num;
    UINT32 cur_key_num;
    PVR_KEY_PARAM pvr_key_param;
} PVR_KEY_PARAM_RESOURSE;

struct PVR_BLOCK_RESCOURSE
{
    UINT8  is_used ;
    UINT8  is_key_changed ;
    UINT8  is_chang_all;
    ID dsc_sema_block_pvr  ;        // it need to be get while en/decryption
    ID dsc_sema_block_pvr_update  ; // it need to be get while keys/status update 
    struct PVR_BLOCK_ENC_PARAM cur_item ;        // the current keys/mode status will be saved here.
    struct PVR_BLOCK_ENC_PARAM next_item ;       // it will be all zero while no status is updating.
    UINT8 *dsc_block_buf ;                              // temp buffer decryption, only be seen in SEE
    UINT8 *origin_buf_ptr ;                              //the initial allocation of the first buff address
    UINT8 key_store[160];        //first 80 for even, last 80 for odd
    UINT8 iv_store[32];         //first 16 for even, last 16 for odd
    enum pvr_kl_level kl_level;	//kl generated level
    struct pvr_key_param_resourse   *pvr_key_param_res;
};

/*!< Raw data decryption (block mode). Also refer to PVR_RPC_RAW_DECRYPT.
*/


/*****************************************************************************
 * Function: pvr_rpc_ioctl
 * Description:
 *    This function initializes the DSC hardware and software structures.
 * Input:
 *      int cmd: cmd index.
 *      UINT32 param: related param.
 * Output:
 *      None
 * Returns:
 *        0: RET_SUCCESS
 *        others: RET_FAILURE
 *
*****************************************************************************/
RET_CODE pvr_rpc_ioctl(int cmd, void *param);

RET_CODE pvr_update_enc_param( struct PVR_BLOCK_ENC_PARAM *enc_param);
RET_CODE pvr_block_de_encrypt (DEEN_CONFIG *p_de_en, UINT8 *input, UINT8 *output,
									UINT32 total_length, PVR_REC_VIDEO_PARAM *p_video_param);
RET_CODE pvr_block_aes_decrypt(p_aes_dev paesdev, UINT16 stream_id, UINT8 *input, UINT32 total_length);
RET_CODE pvr_block_des_decrypt(p_des_dev pdesdev, UINT16 stream_id, UINT8 *input, UINT32 total_length);
/*****************************************************************************
 * Function: pvr_deal_quantum_for_mixed_ts
 * Description:
 *    This function update fta quantum's number on see.
 * Input:
 *      DEEN_CONFIG* p_deen: de/encryption config.
 *      UINT32 temp_length: len in TS packet.
 * Output:
 *      None
 * Returns:
 *        0: RET_SUCCESS
 *        others: RET_FAILURE
 *
*****************************************************************************/
RET_CODE pvr_deal_quantum_for_mixed_ts(DEEN_CONFIG* p_deen, UINT32 temp_length);


#ifdef __cplusplus
}
#endif

/*!
@}
@}
*/


#endif


