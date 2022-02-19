#ifndef  _ADF_PVR_H
#define  _ADF_PVR_H


#ifndef SEE_CPU
//#include <alidefinition/adf_dsc.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*!@addtogroup PVR_REMOTE
 *	@{
	@~
 */

/* IO macros, prefix is "PVR_RPC_IO_"
*/
#define PVR_RPC_IO_BASE 0xC000
/*!< PVR RPC ioctl offset.
*/
#define PVR_RPC_IO_RAW_DECRYPT          (PVR_RPC_IO_BASE + 1)
#define PVR_RPC_IO_SET_BLOCK_SIZE       (PVR_RPC_IO_BASE + 2)
#define PVR_RPC_IO_UPDATE_ENC_PARAMTOR  (PVR_RPC_IO_BASE + 3)
#define PVR_RPC_IO_FREE_BLOCK           (PVR_RPC_IO_BASE + 4)
#define PVR_RPC_IO_START_BLOCK          (PVR_RPC_IO_BASE + 5)
#define PVR_RPC_IO_RAW_DECRYPT_EVO      (PVR_RPC_IO_BASE + 6)
#define PVR_IO_UPDATE_ENC_PARAMTOR_EVO  (PVR_RPC_IO_BASE + 7)
#define PVR_IO_FREE_BLOCK_EVO           (PVR_RPC_IO_BASE + 8)
#define PVR_IO_START_BLOCK_EVO          (PVR_RPC_IO_BASE + 9)

#define PVR_IO_CAPTURE_DECRYPT_RES      (PVR_RPC_IO_BASE + 10)
#define PVR_IO_SET_DECRYPT_RES          (PVR_RPC_IO_BASE + 11)
#define PVR_IO_RELEASE_DECRYPT_RES      (PVR_RPC_IO_BASE + 12)
#define PVR_IO_DECRYPT_EVO              (PVR_RPC_IO_BASE + 13)

#define PVR_RPC_IO_CAPTURE_PVR_KEY          (PVR_RPC_IO_BASE + 14)
#define PVR_RPC_IO_RELEASE_PVR_KEY          (PVR_RPC_IO_BASE + 15)
#define PVR_RPC_IO_PVR_PLAYBCK_SET_KEY      (PVR_RPC_IO_BASE + 16)
#define PVR_RPC_IO_FREE_BLOCK_EX		(PVR_RPC_IO_BASE + 17)




/*!< Raw data decryption (block mode). Also refer to PVR_RPC_RAW_DECRYPT.
*/

/* Return macros, prefix is "PVR_RPC_"
*/
#define PVR_RPC_NO_ERROR (0)
#define PVR_RPC_ERROR (-0x1000)
#define PVR_RPC_ERROR_PARAM (-0x1001)
#define PVR_RPC_ERROR_NOT_ALLOWED (-0x1002)
#define PVR_RPC_ERROR_NOT_SUPPORTED (-0x1004)
#define PVR_RPC_ERROR_NOMEM (-0x1008)

enum pvr_ott_data_type{
	PVR_OTT_DATA_DMX,
	PVR_OTT_DATA_VBV,
	PVR_OTT_DATA_AUDIO,
};

enum pvr_kl_level{
	PVR_KL_ONE = 1,
	PVR_KL_TWO,
	PVR_KL_THREE,
	PVR_KL_FIVE = 5
};

/*! @struct PVR_RPC_RAW_DECRYPT
 *   @brief transfer the raw data address and length to SEE.
 */
typedef struct PVR_RPC_RAW_DECRYPT
{
	int algo;
	void *dev;
	int stream_id;
	UINT8 *input;
	UINT32 length;
	UINT32 indicator;
}PVR_RPC_RAW_DECRYPT;
/*! @struct PVR_RPC_RAW_DECRYPT_EVO
 *   @brief transfer the raw data address and length to SEE.
 */
typedef struct PVR_RPC_RAW_DECRYPT_EVO
{
	int             algo;
	void            *dev;
	int             stream_id;
	UINT8           *input;
	UINT32          length;    
    unsigned int    key_handle;//see dsc key handle
    int             dsc_dma_mode;
    int             iv_parity;
    int             iv_length;
    unsigned char   iv_data[32];
	UINT32 			indicator;
}PVR_RPC_RAW_DECRYPT_EVO;
typedef struct PVR_BLOCK_ENC_PARAM
{
    UINT32 dsc_sub_device; //[input] CSA , AES, TDES ...
    UINT32 work_mode;           //[input] CBC, ECB, CTR ...
    UINT32 source_mode;          //[input] TS, puredata mode
    UINT32 residue_mode;    //[input] process residue block
    UINT8 root_key_pos;                 //[input] User need to configure the root key pos. 1 0xFF is clear key mode, 2 OTP_KEY_0_X is keyladder key mode.
    UINT16 sub_device_id;               //[input/output] encryption sub device id
    UINT16 stream_id;                   //[input/output] Encryption stream ID for current PVR channel. It should be same as the stream ID when creating the key HANDLE for PVR.
    UINT32 key_handle;                  //[input/output] key handle
    UINT16 target_key_pos;              //[input/output] User need to configure the target key pos. 
    UINT8 kl_mode;                      //[input] key ladder generation mode (0:AES ECB, 1:TDES ECB)
    UINT8 input_key[80];                //[input] PVR cipher key address in memory. [0-15] is first stage key data, [16-31] is second stage key data, [32-47] is third stage key data or clear key.[48-79] for fourth and forth stage
    UINT8 input_iv[16];                 //[input] IV data of key.
    UINT32 block_count;                 //[output] blcok count for the change of encryption parameters.
    UINT32 key_mode;                    //evan key , odd key
    UINT16 pid_num;                   //[input] the re-encryption pid number
    UINT16 pid_list[32];                   //[iutput] the pid count
    UINT32 request_res;                  //[iutput],if you don't know this field what to do,pls assign it 0 valure.  
    enum pvr_kl_level kl_level;						//kl generated level
    UINT32  reencrypt_type;               //[input],0:vmx/genca/nagra reencrypt,1:conax reencrypt 
}PVR_BLOCK_ENC_PARAM;

typedef struct DSC_BLOCK_PVR_KEY_PARAM
{
    UINT16 stream_id; //!<Encryption stream ID for current PVR channel. It should be same as the stream ID when creating the key HANDLE for PVR.
    //UINT16 key_length; //!<PVR key bit number, 64bits or 128bits.
    UINT16 used;  //!<The value is always "true".

    UINT8 root_key_pos; //!<User need to configure the root key pos. 1 0xFF is clear key mode, 2 OTP_KEY_0_X is keyladder key mode.
    UINT8 target_key_pos; //!<User need to configure the target key pos. 
    UINT8 mode; //!<User need to configure the encryption mode(AES/DES)
    UINT8 is_key_changed; //!<if it's 1, we should change to the new key.

    UINT32 input_key; //!<PVR cipher key address in memory. [0-15] is first stage key data, [16-31] is second stage key data, [32-47]is clear key.
    UINT32 input_iv; //!<PVR cipher key length. only for clear key mode.

    UINT32 block_count; //!<Reserved item.
} DSC_BLOCK_PVR_KEY_PARAM, *pDSC_BLOCK_PVR_KEY_PARAM;

typedef struct PVR_decrypt_evo
{
    UINT8         *input;
    UINT32        length;
    INT32         algo;
    void          *dev;
    INT32         stream_id;
    INT32         key_handle;
    INT32         data_mode;
    INT32         iv_parity;
    INT32         iv_length;
    UINT8         iv_data[32];    
    UINT32        block_indicator;
	UINT32  des_flag;  /*flag: PVR_OTT_DATA_DMX -> DMX, PVR_OTT_DATA_VBV -> VBV, PVR_OTT_DATA_AUDIO -> AUDIO*/
	UINT32	decv_id;   /*it is set to 0 under default,if pip is used,it is set in practice.*/
}PVR_decrypt_evo;


/*! @struct struct pvr_key_param
 *   @brief Define PVR key parameters.
 */
typedef struct pvr_key_param
{
    UINT8  *input_key;         //
    UINT32 valid_key_num;       //
    UINT32 pvr_key_length;      //
    UINT32 qn_per_key;          //
    UINT32 quantum_size;        //
    UINT16 stream_id;           //
} PVR_KEY_PARAM;

/*! @struct struct pvr_rec_video_param
 *   @brief define video parameters in TS to be re-encrypted. For PVR3.2, trick mode improvement.
 */
typedef struct pvr_rec_video_param
{
	UINT8 type;
	UINT16 pid;
	UINT16 ifm_offset;
}PVR_REC_VIDEO_PARAM;

#ifdef __cplusplus
}
#endif

/*!
@}
@}
*/


#endif

