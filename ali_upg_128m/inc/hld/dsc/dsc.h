/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: dsc.h
*
*    Description: this file is used to define some macros and structures
*                 for descrambler and scrambler
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef  _DSC_H_
#define  _DSC_H_

#include <basic_types.h>
#include <hld/hld_dev.h>

#ifdef __cplusplus
extern "C" {
#endif

//! io cmd
#define IO_INIT_CMD  0
//!< Initialize the AES/TDES/SHA/CSA device. refer to 'struct AES_INIT_PARAM DES_INIT_PARAM CSA_INIT_PARAM SHA_INIT_PARAM'.
#define IO_CREAT_CRYPT_STREAM_CMD  1
//!< Ioctl for AES/TDES/CSA to create the key HANDLE in DSC, refer to 'struct KEY_PARAM'.
#define IO_DELETE_CRYPT_STREAM_CMD  2
//!< Ioctl for AES/TDES/CSA to delete the key HANDLE in DSC.
#define IO_KEY_INFO_UPDATE_CMD  3
//!< Update the clear key value for AES/TDES/CSA, refer to 'struct KEY_PARAM'.
/*!< Only the handle/pid/key_length/p_xxx_key_info are needed when updating clear key.
*/
#define IO_ADD_DEL_PID 4
//!< Add or Delete PID from one key handle. refer to the 'struct PID_PARAM'
#define IO_PARAM_INFO_UPDATE 5
//!< Update IV and other parameter Info. refer to the 'struct IV_OTHER_PARAM'
#define IO_CONTINUOUS_RENEW 6
//!< In Continuous mode, clear the last streams' residue and IV for starting a new stream with different segments.
#define IO_AES_CRYPT_WITH_FP 7
//!< Crypt aes pure data with the FlashProt-RootKey
#define IO_HMAC_SET_CLEAR_KEY 8


/* Used for DSC ioctl 
*/
#define IO_PARSE_DMX_ID_SET_CMD   0
//!< Set the stream ID for current live play or playback stream.
/*!< The stream ID should be same with the ID used for #IO_INIT_CMD and #IO_CREAT_CRYPT_STREAM_CMD.
	
	This IO cmd is usually used along with the DMX IO cmd as below:\n
	ALI_DMX_SEE_CRYPTO_TYPE_SET(Connect the decryption SEE device pointer and algorithm with DMX driver).\n
	ALI_DMX_SEE_CRYPTO_START (enable the decryption).
*/

#define IO_PARSE_DMX_ID_GET_CMD   1
//!< Get the stream ID of current live play or playback stream, and use this ID to decrypt the stream.
/*!<This ID is set by '#IO_PARSE_DMX_ID_SET_CMD', it contains the key information for decrytion, only used in DMX driver.
*/

#define IO_DSC_GET_DES_HANDLE     2
//!< Get the DES device 0 pointer.
#define IO_DSC_GET_AES_HANDLE     3
//!< Get the AES device 0 pointer.
#define IO_DSC_GET_CSA_HANDLE     4
//!< Get the CSA device 0 pointer.
#define IO_DSC_GET_SHA_HANDLE     5
//!< Get the SHA device 0 pointer.
#define IO_DSC_SET_PVR_KEY_PARAM  6
//!< Set the PVR key information to DSC.
/*!< The recording file size is limited by the user configuration, if the file size reach to the max value while recording, 
	PVR will create a new file for the continuous recording, at this time the DSC will update the recording key for 
	this new file (DSC counts the number/size of the re-encrypted packets, if this size reach to the configured file size,
	DSC will update the key). Refer to 'struct DSC_PVR_KEY_PARAM'.
*/
#define IO_DSC_ENCRYTP_BL_UK      7
//!< Reserved for BL to encrypt the BL universal key.
#define IO_DSC_SET_PVR_KEY_IDLE   8
//!< Delete one set of PVR key based on the record stream ID.
#define IO_DSC_VER_CHECK          9
//!< Reserved for BL to check the system firmware version.
#define IO_DSC_SET_ENCRYPT_PRIORITY 10
//!< Change the TS encryption parity (even or odd) when doing PVR, along with 'struct DSC_EN_PRIORITY'.
#define IO_DSC_GET_DRIVER_VERSION 11
//!< Get current driver version, max to 20 bytes characters.
#define IO_DSC_SET_CLR_CMDQ_EN 12
//!< Enable or disable the DSC command queue mode, CMDQ ensures the high performance when multi-channel.
#define IO_DSC_DELETE_HANDLE_CMD 13
//!< Ioctl for DSC to delete the key HANDLE created by AES/TDES/CSA directly, same functionality with #IO_DELETE_CRYPT_STREAM_CMD.

#define IO_DSC_FIXED_DECRYPTION 30
//!< Reserved for BL.
#define IO_DSC_SYS_UK_FW 31
//!< Reserved for BL to handle the system firmware or universal key.
#define IO_DSC_SET_STB_STORAGE_SUPPORT 32
//!< Ioctl for telling DSC whether support STB storage capacity.

#define ALI_INVALID_CRYPTO_STREAM_HANDLE  0xffffffff
//!< Define the invalid key HANDLE value.
#define ALI_INVALID_CRYPTO_STREAM_ID        0xff
//!< Define the invalid stream iD value.
#define ALI_INVALID_DSC_SUB_DEV_ID          0xff
//!< Define the invalid sub-device iD value.

#define TS_MAX_SIZE  0x10000
//!< Define max TS data packets number for encryption and decryption
#define PURE_DATA_MAX_SIZE  0x4000000
//!< Define max PURE data size for encryption, decryption and hash.
#define INVALID_DSC_SUB_DEV_ID            0xff
//!< Define the invalid sub-device iD value.
#define INVALID_DSC_STREAM_ID            0xff
//!< Define the invalid stream iD value.
#define INVALID_CE_KEY_POS 0xff
//!< Define the invalid key position value.

#define    TS_PID_MASK 0x1FFF
#define VIRTUAL_DEV_NUM 8
//!< Define the AES/TDES/SHA/CSA sub-device number.
/*!< The AES/TDES/SHA/CSA are sub-algotithms of DSC(Descramble Scramble Core) 
	and DSC takes the responsibility to manage the device/stream ID and key HANDLE for its sub-algorithms.
	Every algorithm in DSC has 8 (VIRTUAL_DEV_NUM) sub-devices. ('8' is for multi-channel).
*/

#define TS_PACKET_SIZE 188
//!< Define the packet size, unit in bytes.

/*just for version check caller*/
#define CHECK_HEAD 0x1000
#define CHECK_END  0x0001
#define CHUNKID_UBOOT        0x01FE0101
#define CHUNKID_KERNEL       0x06F90101
#define CHUNKID_FW           0x06F90101
#define CHUNKID_SEE          0x00FF0100
#define CHUNKID_BL           0x10000123

#define SHA1_HASH_SIZE         20 //!< SHA1 hash size is 20 bytes.
#define SHA224_HASH_SIZE     28 //!< SHA224 hash size is 28 bytes.
#define SHA256_HASH_SIZE     32 //!< SHA256 hash size is 32 bytes.
#define SHA384_HASH_SIZE     48 //!< SHA384 hash size is 48 bytes
#define SHA512_HASH_SIZE     64 //!< SHA512 hash size is 64 bytes

#define HMAC_KEY_SIZE     16

#ifndef offsetof
#define offsetof(type, f) ((unsigned long) \
    ((char *)&((type *)0)->f - (char *)(type *)0))
#endif

#define SHA_MODE_CHECK(mode)	(((mode) != SHA_SHA_1) && \
		((mode) != SHA_SHA_224) && ((mode) != SHA_SHA_256) && \
		((mode) != SHA_SHA_384 )&& ((mode) != SHA_SHA_512))
//!< Define the macro for SHA input mode checking.		


/*! @enum CSA_VERSION
 *   @brief Define DVB-CSA version.
*/
enum CSA_VERSION
{
	CSA1=1,//!< DVB-CSA1.1
	CSA_1=1,//!< DVB-CSA1.1
	
	CSA2=0,//!<DVB-CSA2.0
	CSA_2=0,//!<DVB-CSA2.0
	
	CSA3=2,//!< DVB-CSA3
	CSA_3=2,//!< DVB-CSA3
};

/*! @enum SHA_MODE
 *   @brief Define SHA hash mode.
*/
enum SHA_MODE
{
    SHA_SHA_1 = 0, //!< SHA1, HASH length is 20 bytes.
    SHA_SHA_224 = (1 << 29), //!< SHA224, HASH length is 28 bytes. 
    SHA_SHA_256 =  (2 << 29), //!< SHA256, HASH length is 32 bytes.
    SHA_SHA_384 = (3 << 29), //!< SHA384, HASH length is 48 bytes.
    SHA_SHA_512  = (4 << 29), //!< SHA512, HASH length is 64 bytes.
};

enum HMAC_MODE
{
	HMAC_SHA_256 = 2,//!< SHA256, HASH length is 32 bytes.
	HMAC_SHA_384 = 3,//!< SHA384, HASH length is 48 bytes.
	HMAC_SHA_512 = 4,//!< SHA512, HASH length is 64 bytes.
};

enum HMAC_KEY_OPTION
{
	SET_CLEAR_KEY = 1,
	SET_KL_KEY = 2,
};


/*! @enum SHA_DATA_SOURCE
 *   @brief Define the SHA data source.
*/
enum SHA_DATA_SOURCE
{
    SHA_DATA_SOURCE_FROM_DRAM = 0, //!<DRAM
    SHA_DATA_SOURCE_FROM_FLASH = 1, //!<NOR FLASH
};

/*! @enum PARITY_MODE
 *  @brief Define encryption and decryption parity mode.
*/
enum PARITY_MODE
{
    EVEN_PARITY_MODE  = 0, //!< Even parity, usually used for TS encryption.
    ODD_PARITY_MODE = 1, //!< Parity is odd.
    AUTO_PARITY_MODE0 = 2, //!< Used in TS decryption, auto detects the TS packet parity.
    AUTO_PARITY_MODE1 = 3, //!< Used in TS decryption, auto detects the TS packet parity.
};

/*! @enum KEY_OTP_MODE
 *  @brief Define OTP mode when #KEY_FROM_OTP.
*/
enum KEY_OTP_MODE
{
    OTP_KEY_FROM_68 = 4,//!< Key is directly from OTP key6 when #KEY_FROM_OTP.
    OTP_KEY_FROM_6C = 5,//!< Key is directly from OTP key7 when #KEY_FROM_OTP.
    OTP_KEY_FROM_FP = 6, //<! Key is directly from OTP Flash-Protect root key when #KEY_FROM_OTP.

    /* introduced from S3922, not available to OS lib due to security */
#if 1
    OTP_KEY_FROM_TEE_FPK = 0x10, //<! Key is directly from OTP TEEFirmwareProtectionKey when #KEY_FROM_OTP.
    OTP_KEY_FROM_TZ_HUK = 0x11, //<! Key is directly from OTP TrustZone_HUK when #KEY_FROM_OTP.
    OTP_KEY_FROM_TZ_SSK = 0x12, //<! Key is directly from OTP TrustZone_SSK when #KEY_FROM_OTP.
    OTP_KEY_FROM_UK_PK = 0x13, //<! Key is directly from OTP UK Protect RootKey when #KEY_FROM_OTP.
    OTP_KEY_FROM_CCCK = 0x14, //<! Key is directly from OTP CCCK when #KEY_FROM_OTP.
    OTP_KEY_FROM_SSUK = 0x15, //<! Key is directly from OTP SSUK when #KEY_FROM_OTP.
    OTP_KEY_FROM_DRM_UK = 0x16, //<! Key is directly from OTP DRM_UK when #KEY_FROM_OTP.
    OTP_KEY_FROM_DRM_GK = 0x17, //<! Key is directly from OTP DRM_GK when #KEY_FROM_OTP.
#endif
};

/*! @enum KEY_TYPE
 *  @brief Define key source.
*/
enum KEY_TYPE
{
    KEY_FROM_REG = 0, //!< Not supprt.
    KEY_FROM_SRAM = 1, //!< Key is from DSC internal SRAM, it's a clear key.
    KEY_FROM_CRYPTO = 2, //!< Key is from KL, it's an advanced secure key.
    KEY_FROM_OTP = 3, //!<  Key is directly from OTP secure key6/7, it's an advanced secure key.
    KEY_FROM_MIX = 4, //!< Key is from both KL and Host. NV Service 16.
};

/*! @enum KEY_MAP_MODE
 *  @brief Define the key mode according to the key length.
*/
enum KEY_MAP_MODE
{
   CSA_MODE=0, //!< Identify the key length is 64 bits. 
   DES_MODE=0, //!< Identify the key length is 64 bits. 
   CSA3_MODE=1, //!< Identify the key length is 128 bits.
   AES_128BITS_MODE=1, //!< Identify the key length is 128 bits.
   TDES_ABA_MODE=1, //!< Identify the key length is 128 bits. 
   CRYPTO_128BITS_MODE=1, //!< Identify the key length is 128 bits. 
   AES_192BITS_MODE=2, //!< Identify the key length is 192 bits. 
   TDES_ABC_MODE=2, //!<  Identify the key length is 192 bits. 
   AES_256BITS_MODE=3 //!< Identify the key length is 256 bits. 
};

/*! @enum KEY_BIT_NUM
 *  @brief Define the bit length of DSC key.
*/
enum KEY_BIT_NUM
{
    BIT_NUMOF_KEY_64 = 64,  //!< Identify the bit length is 64 bits. 
    BIT_NUMOF_KEY_128 = 128, //!< Identify the bit length is 128 bits. 
    BIT_NUMOF_KEY_192 = 192, //!< Identify the bit length is 192 bits. 
    BIT_NUMOF_KEY_256 = 256, //!< Identify the bit length is 256 bits.
};

/*! @enum DMA_MODE
 *  @brief Define the operation data type.
 */
enum DMA_MODE
{
    PURE_DATA_MODE = 0, //!< The operation data is raw/pure data.
    TS_MODE = (1 << 24), //!< The operation data is TS data. 
};

/*! @enum RESIDUE_BLOCK
 *   @brief Define residue block handling mode.
 */
enum RESIDUE_BLOCK
{
    RESIDUE_BLOCK_IS_NO_HANDLE = 0, //!< Does not process residue block. residue block data is same with input.
    RESIDUE_BLOCK_IS_AS_ATSC = (1 << 12), //!< Process residue block as ANSI SCT 52 standard. 
    RESIDUE_BLOCK_IS_HW_CTS = (2 << 12), //!< The residue block handling uses cipher stealing method.
	RESIDUE_BLOCK_IS_CTR = (3 << 12),
	RESIDUE_BLOCK_IS_CTS3 = (5 << 12),
    RESIDUE_BLOCK_IS_RESERVED = (7 << 12), //!< Reserved.
};

/*! @enum WORK_MODE
 *   @brief Define block chaing mode.
 */
enum WORK_MODE
{
    WORK_MODE_IS_CBC =  0, //!< Cipher block chaining.
    WORK_MODE_IS_ECB =   (1 << 4), //!< Electric codebook. 
    WORK_MODE_IS_OFB =  (2 << 4), //!< Output feedback.
    WORK_MODE_IS_CFB  = (3 << 4), //!< Cipher Feedback.
    WORK_MODE_IS_CTR  = (4 << 4), //!< Counter mode, only available in AES.
};

/*! @enum WORK_SUB_MODULE
 *   @brief Define all sub-algorithms DSC suppoted.
 */
enum WORK_SUB_MODULE
{
    DES = 0, //!< Indicates the sub-algorithm is DES. 
    TDES = 3, //!< Indicates the sub-algorithm is DES. 
    AES = 1, //!<  Indicates the sub-algorithm is AES. 
    SHA = 2, //!<  Indicates the sub-algorithm is SHA.
    CSA = 4, //!<  Indicates the sub-algorithm is CSA. 
	HMAC = 5, //!<  Indicates the sub-algorithm is HMAC. 
};

/*! @enum CRYPT_SELECT
 *   @brief Encrypt/decrypt selection
 */
enum CRYPT_SELECT
{
    DSC_DECRYPT = 1, //!< Indicates the mode is decryption.
    DSC_ENCRYPT = 0  //!< Indicates the mode is encryption.
};

/*! @enum DSC_STATUS
 *   @brief Return values of the DSC functions.
 */
typedef enum dsc_ret_status
{
    ALI_DSC_SUCCESS = 0, //!< The intended operation was executed successfully.
    ALI_DSC_ERROR,  //!< The function terminated abnormally. The intended operation failed.
    ALI_DSC_WARNING_DRIVER_ALREADY_INITIALIZED, //!< The SEE DSC is already initialized.

    ALI_DSC_ERROR_INVALID_PARAMETERS = 0x1F00, //!< The passed parameters are invalid.
    ALI_DSC_ERROR_OPERATION_NOT_ALLOWED,  //!< The requested operation is not allowed.
    ALI_DSC_ERROR_OPERATION_NOT_SUPPORTED, //!< The requested operation is not supported.
    ALI_DSC_ERROR_INITIALIZATION_FAILED, //!< The SEE DSC initialization failed.
    ALI_DSC_ERROR_DRIVER_NOT_INITIALIZED, //!< The SEE DSC isn't initialized.
    ALI_DSC_ERROR_INVALID_ADDR, //!< The passed address is invalid.
    ALI_DSC_ERROR_INVALID_DEV, //!< The passed device pointer is invalid.
    ALI_DSC_ERROR_INVALID_HANDLE, //!< The passed key HANDLE is invalid.
    ALI_DSC_ERROR_NO_IDLE_HANDLE, //!< Key HANDLE table is full.
    ALI_DSC_ERROR_NO_IDLE_PIDRAM, //!< PID table is full.
    ALI_DSC_ERROR_NO_IDLE_KEYRAM, //!< Clear key table is full.
    ALI_DSC_ERROR_PID_EXIST,
    ALI_DSC_ERROR_PID_FULL,
    ALI_DSC_ERROR_PID_NOT_EXIST,
    ALI_DSC_ERROR_NO_MEM,

    ALI_SHA_ERROR_SIZE_2BIG,
}DSC_STATUS;


/*! @struct des_init_param
 *   @brief Define DES device initialization parameters.
 */
typedef struct des_init_param
{
    enum PARITY_MODE  parity_mode; //!< Parity of key.
    enum KEY_TYPE key_from; //!< Key source.
    UINT32 scramble_control; //!< Reserved item.
    enum KEY_MAP_MODE key_mode; //!< Key mode based on key length.
    UINT32 stream_id; //!< The stream ID for current device.
    enum DMA_MODE dma_mode; //!< The data mode, TS or pure data.
    enum RESIDUE_BLOCK  residue_mode; //!< The residue block handling mode.
    enum WORK_MODE work_mode; //!< The cipher chaining mode.
    enum WORK_SUB_MODULE sub_module; //!< The DES sub mode, DES or TDES.
    UINT32 cbc_cts_enable ; //!< CBC CTS mode control.
    UINT8 continuous; //! from cap210 on, IC supports ts/pure data chaining. . 0: separate; 1: continuous
    UINT8 ts_format; // 0<FMT_TS_188>, 1<FMT_TS_200>, 2<FMT_TS_LTSID>
    UINT8 sc_mode; // scramble control mode (0: mode 1; 1: mode 2)
} DES_INIT_PARAM, *p_des_init_param;


/*! @struct aes_init_param
 *   @brief Define AES device initialization parameters.
 */
typedef struct aes_init_param
{
    enum PARITY_MODE  parity_mode; //!< Parity of key.
    enum KEY_TYPE key_from; //!< Key source.
    UINT32 scramble_control; //!< Reserved item.
    enum KEY_MAP_MODE key_mode; //!< Key mode based on key length. Refer to 'KEY_MAP_MODE'.
    UINT32 stream_id; //!< The stream ID for current device.
    enum DMA_MODE dma_mode; //!< The data mode, TS or pure data.
    enum RESIDUE_BLOCK  residue_mode; //!< The residue block handling mode.
    enum WORK_MODE work_mode; //!< The cipher chaining mode.
    UINT32 cbc_cts_enable ;  //!< CBC CTS mode control.
    UINT8 continuous; //! from cap210 on, IC supports ts/pure data chaining. 0: separate; 1: continuous
    UINT8 ts_format; // 0<FMT_TS_188>, 1<FMT_TS_200>, 2<FMT_TS_LTSID>
    UINT8 sc_mode; // scramble control mode (0: mode 1; 1: mode 2)
} AES_INIT_PARAM, *p_aes_init_param;


/*! @struct csa_init_param
 *   @brief Define CSA device initialization parameters.
 */
typedef struct csa_init_param
{
    enum CSA_VERSION version; //!< Version of DVB-CSA.
    enum DMA_MODE dma_mode; //!< The data mode, TS or pure data. 
    UINT32 dcw[4]; //!<Reserved item. 
    UINT32 pes_en; //!<Reserved item. 
    enum PARITY_MODE  parity_mode;  //!< Parity of key.
    enum KEY_TYPE key_from; //!< Key source.
    UINT32 scramble_control; //!< Reserved item.
    UINT32 stream_id; //!< The stream ID for current device.
    UINT8 ts_format; // 0<FMT_TS_188>, 1<FMT_TS_200>, 2<FMT_TS_LTSID>
} CSA_INIT_PARAM, *p_csa_init_param;

/*! @struct sha_init_param
 *   @brief Define SHA device initialization parameters.
 */
typedef struct sha_init_param
{
    enum SHA_MODE sha_work_mode;  //!<SHA hash mode.
    enum SHA_DATA_SOURCE sha_data_source; //!<SHA data source.
} SHA_INIT_PARAM, *p_sha_init_param;

/*! @struct CRYPTO_128BITS_KEY
 *   @brief Define 128 bits clear key buffer.
 */
typedef struct crypto_128bit_key
{
    UINT8 even_key[16]; //!< 16 bytes even key buffer.
    UINT8 odd_key[16]; //!< 16 bytes odd key buffer.
}CRYPTO_128BITS_KEY;

/*! @struct CRYPTO_128BITS_IV
 *   @brief Define 128 bits iv buffer.
 */
typedef struct crypto_128bit_iv
{
    UINT8 even_iv[16]; //!< 16 bytes even iv buffer.
    UINT8 odd_iv[16]; //!< 16 bytes odd iv buffer.
}CRYPTO_128BITS_IV;

/*! @struct AES_128Bit_KEY
 *   @brief Define 128 bits AES clear key buffer.
 */
struct aes_128bit_key
{
    UINT8 even_key[16];  //!< 16 bytes even key buffer.
    UINT8 odd_key[16];  //!< 16 bytes odd key buffer.
};

typedef struct aes_128bit_key AES_128BITS_KEY;

/*! @struct AES_192Bit_KEY
 *   @brief Define 192 bits AES clear key buffer.
 */
struct aes_192bit_key
{
    UINT8 even_key[24];  //!< 24 bytes even key buffer.
    UINT8 odd_key[24]; //!< 24 bytes odd key buffer.
};

typedef struct aes_192bit_key AES_192BITS_KEY;

/*! @struct AES_256Bit_KEY
 *   @brief Define 256 bits AES clear key buffer.
 */
struct aes_256bit_key
{
    UINT8 even_key[32]; //!< 32 bytes even key buffer.
    UINT8 odd_key[32]; //!< 32 bytes ddd key buffer.
};

typedef struct aes_256bit_key AES_256BITS_KEY;

/*! @union AES_KEY_PARAM
 *   @brief Define AES clear key buffer.
 */
typedef union aes_key_param
{
    struct aes_128bit_key aes_128bit_key ; //!< 128 bits key.
    struct aes_192bit_key aes_192bit_key ; //!< 192 bits key.
    struct aes_256bit_key aes_256bit_key ; //!< 256 bits key.
} AES_KEY_PARAM;

/*! @struct AES_IV_INFO
 *   @brief Define AES initialization vector buffer.
 */
typedef struct aes_iv_info
{
    UINT8 even_iv[16];  //!< 16 bytes even iv buffer.
    UINT8 odd_iv[16];   //!< 16 bytes odd iv buffer.
} AES_IV_INFO;

struct  AES_KEY
{
    UINT8 odd_key[16];
    UINT8 even_key[16];
};

/*! @struct CSA_KEY
 *   @brief Define CSA1.1/CSA2.0 64bits clear key buffer.
 */
struct  CSA_KEY
{
    UINT8 odd_key[8]; //!< 8 bytes odd key buffer.
    UINT8 even_key[8]; //!< 8 bytes even key buffer.
};

/*! @struct CSA_64BITS_KEY
 *   @brief Define CSA1.1/CSA2.0 64bits clear key buffer.
 */
typedef struct CSA_KEY CSA_64BITS_KEY;

/*! @struct CSA3_KEY
 *   @brief Define CSA3 128bits clear key buffer.
 */
struct  CSA3_KEY
{
    UINT8 odd_key[16];  //!< 16 bytes odd key buffer.
    UINT8 even_key[16]; //!< 16 bytes even key buffer.
};

/*! @struct CSA3_128BITS_KEY
 *   @brief Define CSA3 128bits clear key buffer.
 */
typedef struct CSA3_KEY CSA3_128BITS_KEY;

/*! @union CSA_KEY_PARAM
 *   @brief Define DVB-CSA clear key buffer. 
 */
typedef union csa_key_param
{
    struct  CSA_KEY csa_key ;  //!< CSA1.1/CSA2.0 64 bits key
    struct  CSA3_KEY csa3_key ; //!< CSA3 128 bits key
} CSA_KEY_PARAM;

/*! @struct DES_64BITS_KEY_INFO
 *   @brief Define DES 64bits clear key buffer.
 */
struct  DES_64BITS_KEY_INFO
{
    UINT8 odd_key[8]; //!< 8 bytes odd key buffer.
    UINT8 even_key[8]; //!< 8 bytes even key buffer.
};

/*! @struct DES_64BITS_KEY
 *   @brief Define DES 64bits clear key buffer.
 */
typedef struct DES_64BITS_KEY_INFO DES_64BITS_KEY;

/*! @struct DES_128BITS_KEY_INFO
 *   @brief Define TDES 128bits clear key buffer. 
 */
struct  DES_128BITS_KEY_INFO
{
    UINT8 odd_key[16]; //!< 16 bytes odd key buffer.
    UINT8 even_key[16]; //!< 16 bytes even key buffer.
};

/*! @struct TDES_128BITS_KEY
 *   @brief Define TDES 128bits clear key buffer. 
 */
typedef struct DES_128BITS_KEY_INFO TDES_128BITS_KEY;

/*! @struct DES_192BITS_KEY_INFO
 *   @brief Define TDES 192bits clear key buffer.
 */
struct  DES_192BITS_KEY_INFO
{
    UINT8 odd_key[24];  //!< 16 bytes odd key buffer.
    UINT8 even_key[24];  //!< 16 bytes even key buffer.
};

/*! @struct TDES_192BITS_KEY
 *   @brief Define TDES 192bits clear key buffer.
 */
typedef struct DES_192BITS_KEY_INFO TDES_192BITS_KEY;

/*! @union DES_KEY_PARAM
 *   @brief Define DES/TDES clear key buffer.
 */
typedef union des_key_param
{
    struct  DES_64BITS_KEY_INFO  des_64bits_key ; //!< DES 64 bits key
    struct  DES_128BITS_KEY_INFO des_128bits_key ; //!< TDES 128 bits key
    struct  DES_192BITS_KEY_INFO des_192bits_key ; //!< TDES 192 bits key
} DES_KEY_PARAM;

/*! @struct DES_IV_INFO
 *   @brief Define DES initialization vector buffer.
 */
typedef struct des_iv_info
{
    UINT8 even_iv[8];  //!< 8 bytes even iv buffer.
    UINT8 odd_iv[8];   //!< 8 bytes odd iv buffer.
} DES_IV_INFO;

/*! @struct KEY_PARAM
 *   @brief Define key parameters. The KEY_PARAM must be initialized to zero before configuration.
 */
typedef struct
{
    UINT32 handle ;  //!< Key HANDLE returned from driver.
    UINT16 *pid_list; //!< Specify the PIDs which will be set with this key.
    UINT16 pid_len;   //!<Specify the '*pid_list' elements number, recommand value is 1.
    AES_KEY_PARAM *p_aes_key_info; //!< AES clear key buffer for KEY_FROM_SRAM mode.
    /*!< Only need to copy the input clear key to the even key buffer if the AES device works in PURE_DATA_MODE.
	*/
    CSA_KEY_PARAM *p_csa_key_info; //!<CSA clear key buffer for KEY_FROM_SRAM mode.
    DES_KEY_PARAM *p_des_key_info; //!<DES/TDES clear key buffer for KEY_FROM_SRAM mode.
    /*!< Only need to copy the input clear key to the even key buffer if the DES device works in PURE_DATA_MODE.
	*/
    UINT32 key_length; //!<Key length (bit).
    AES_IV_INFO *p_aes_iv_info; //!< AES IV buffer.
    /*!< When using KEY_FROM_OTP mode, the IV pointer is *init_vector. When using KEY_FROM_SRAM or KEY_FROM_CRYPTO mode, 
	*	the IV pointer is *p_aes_iv_info. Only need to copy the input iv to the even iv buffer if the AES device works in PURE_DATA_MODE.
	*/	
    DES_IV_INFO *p_des_iv_info; //!< DES/TDES IV buffer.
	/*!< When using KEY_FROM_OTP mode, the IV pointer is *init_vector. When using KEY_FROM_SRAM or KEY_FROM_CRYPTO mode, 
	*	the IV pointer is *p_des_iv_info. Only need to copy the input iv to the even iv buffer if the DES device works in PURE_DATA_MODE.
	*/	
    UINT16 stream_id;
    //!< The corresponding stream ID. Should be same as the stream ID which is used for initializing the device.
    UINT8 *init_vector;  //!<The initialization vector pointer in KEY_FROM_OTP mode.
    UINT8 *ctr_counter; //!<The counter pointer for AES CTR mode.
    UINT8 force_mode; //!<Reserved item.
    UINT8 pos ; //!<When using KEY_FROM_CRYPTO mode, the corresponding key index in KL should be assigned to this pos.
    UINT8 no_even; //!< Even key configuration control.
    	/*!< If the 'no_even = 1' is set, the even clear key will not be updated/configured.
	*	If the 'no_even = 0' is set, the even clear key will be updated/configured.
	*/
    UINT8 no_odd; //!< Odd key configuration control.
	/*!< If the 'no_odd = 1' is set, the odd clear key will not be updated/configured.
		If the 'no_odd = 0' is set, the odd clear key will be updated/configured.
	*/	
    UINT8 not_refresh_iv; //!<IV chaining control.
    	/*!< If the 'not_refresh_iv = 1' is set, the new IV value will not be updated/configured. (Using the original chaing register value).
	*	If the 'not_refresh_iv = 0' is set, the new IV value will be updated/configured.
	*/
    UINT8 even_key_locate; //!< 0:host, 1:kl - only KEY_FROM_MIX cares this para
    UINT8 odd_key_locate; //!< 0:host, 1:kl - only KEY_FROM_MIX cares this para
    UINT8 kl_sel; //!< KL0[0], KL1[1], KL2[2], KL3[3], KL4[4] 
    UINT32 key_from;
} KEY_PARAM, *p_key_param;

typedef struct
{
	UINT32 handle;
		//!< Key HANDLE returned from driver.
	UINT8 operate;
		//!< 1: add; 2: del; other: ignore
	UINT8 ltsid;
		//!< Used when the stream format is TS-200
	UINT8 tsc_flag;
		//!< the user defined TSC bit configuration flags. 0<not change>, 1<odd>, 2<even>
	UINT16 pid;
		//!< The PID value
}PID_PARAM;

#define UP_PARAM_KEY_ODD	(1 << 0)
#define UP_PARAM_KEY_EVEN	(1 << 1)
#define UP_PARAM_IV_ODD		(1 << 2)
#define UP_PARAM_IV_EVEN	(1 << 3)
#define UP_PARAM_CRYPT_MODE	(1 << 4)
#define UP_PARAM_CHAINING	(1 << 5)
#define UP_PARAM_RESIDUE	(1 << 6)
#define UP_PARAM_PARITY		(1 << 7)
#define UP_PARAM_TSC_FLAG	(1 << 8)


typedef struct
{
	UINT32 handle;
		//!< Key HANDLE returned from driver.
	UINT32 chaining;
	UINT32 residue;
	UINT8 iv_odd[16];
	UINT8 iv_even[16];
	UINT32 parity;
	UINT32 key_from;
	UINT8 no_even;
		/*!< If the 'no_even = 1' is set, the even clear key will not be updated/configured.
		*	If the 'no_even = 0' is set, the even clear key will be updated/configured.
		*/
	UINT8 no_odd;
		/*!< If the 'no_odd = 1' is set, the odd clear key will not be updated/configured.
			If the 'no_odd = 0' is set, the odd clear key will be updated/configured.
		*/
	UINT8  even_key_locate;
		//!< 0:host, 1:kl - only KEY_FROM_MIX cares this para
	UINT8  odd_key_locate;
		//!< 0:host, 1:kl - only KEY_FROM_MIX cares this para
	UINT8 pos;
	UINT8 kl_sel;
		//!< KL0[0], KL1[1], KL2[2], KL3[3], KL4[4] 
	UINT32 valid_mask;
		/*!< Mask to indicate which of the fields of this structure
		     are valid. Only parameters with their respective bits
		     set to 1 in this field will be updated.

		     UP_PARAM_CHAINING, UP_PARAM_RESIDUE,
		     UP_PARAM_IV_ODD, UP_PARAM_IV_EVEN, UP_PARAM_PARITY
		  */
	UINT8 tsc_flag;
}IV_OTHER_PARAM;

/*! @struct INIT_PARAM_NOS
 *   @brief Reserved item.
 */
typedef struct crypt_init_nos
{
    enum PARITY_MODE parity_mode;
    enum KEY_TYPE key_from;
    UINT32 scramble_control;
    enum KEY_MAP_MODE key_mode;
    UINT32 stream_id; /**which stream id is working*/
    enum DMA_MODE dma_mode;
    enum RESIDUE_BLOCK residue_mode;
    enum WORK_MODE work_mode;
    enum WORK_SUB_MODULE sub_module;
    UINT32 cbc_cts_enable ;  /*for pure data*/

    KEY_PARAM *pkey_param;
}INIT_PARAM_NOS;


/*! @struct AES_CRYPT_WITH_FP
 *   @brief Aes crypt with the FlashPort-Key.
 */
typedef struct aes_crypt_with_fp
{
	UINT8 crypt_mode;
	UINT8 *input;
	UINT8 output[16];
}AES_CRYPT_WITH_FP;


/*! @struct SHA_DEV
 *   @brief Reserved for HLD SHA device structure.
 */
typedef struct sha_device
{
    struct sha_device  *next;  /*next device */
    unsigned int type;
    INT8 name[HLD_MAX_NAME_SIZE];
    SHA_INIT_PARAM *p_info_nos; /* Only For NOS AS */
    void *priv ;
    void (*open)(struct sha_device *);
    void (*close)(struct sha_device *);
    RET_CODE (*digest)(struct sha_device *p_sha_dev, UINT8 *input,
                       UINT8 *output, UINT32 data_length);
    RET_CODE  (*ioctl)( struct sha_device *p_sha_dev ,
                        UINT32 cmd , UINT32 param);
    UINT8 id_number;
	RET_CODE (*update)(struct sha_device *p_sha_dev, UINT8 *input,
		UINT32 data_length);
} SHA_DEV, *p_sha_dev;

typedef struct hmac_device
{
    struct hmac_device *next;  /* next device */
    int type;
    char name[HLD_MAX_NAME_SIZE];
    void *priv;
    RET_CODE (*digest)(struct hmac_device *p_hmac_dev, UINT8 *input,
                       UINT8 *output, UINT32 data_length);
    RET_CODE (*ioctl)(struct hmac_device *p_hmac_dev,
                        UINT32 cmd, UINT32 param);
    unsigned char id_number;
} HMAC_DEV, *p_hmac_dev;

/*! @struct AES_DEV
 *   @brief Reserved for HLD AES device structure.
 */
typedef struct aes_device
{
    struct aes_device  *next;  /*next device */
    INT32 type;
    INT8 name[HLD_MAX_NAME_SIZE];
    INIT_PARAM_NOS *p_info_nos; /* Only For NOS AS */
    void *priv ;
    void (*open)(struct aes_device *);
    void (*close)(struct aes_device *);
    RET_CODE (*encrypt)(struct aes_device *p_aes_dev, UINT16 stream_id,
                        UINT8 *input, UINT8 *output, UINT32 total_length);
    RET_CODE (*decrypt)(struct aes_device *p_aes_dev, UINT16 stream_id,
                        UINT8 *input, UINT8 *output, UINT32 total_length);
    RET_CODE (*ioctl)(struct aes_device *p_aes_dev, UINT32 cmd, UINT32 param);
    UINT8 id_number;
} AES_DEV, *p_aes_dev;


/*! @struct DES_DEV
 *   @brief Reserved for HLD DES device structure.
 */
typedef struct des_device
{
    struct des_device  *next;  /*next device */
    INT32 type;
    INT8 name[HLD_MAX_NAME_SIZE];
    INIT_PARAM_NOS *p_info_nos; /* Only For NOS AS */
    void *priv ;
    void (*open)(struct des_device *p_des_dev );
    void (*close)(struct des_device *p_des_dev );
    RET_CODE (*encrypt)(struct des_device *p_des_dev, UINT16 stream_id,
                        UINT8 *input, UINT8 *output, UINT32 total_length);
    RET_CODE (*decrypt)(struct des_device *p_des_dev, UINT16 stream_id,
                        UINT8 *input, UINT8 *output, UINT32 total_length);
    RET_CODE (*ioctl)(struct des_device *p_des_dev, UINT32 cmd, UINT32 param);
    UINT8 id_number;
} DES_DEV, *p_des_dev;

/*! @struct CSA_DEV
 *   @brief Reserved for HLD CSA device structure.
 */
typedef struct csa_device
{
    struct csa_device  *next;  /*next device */
    INT32 type;
    INT8 name[HLD_MAX_NAME_SIZE];
    void *p_root;
    void *priv ;
    void (*open)(struct csa_device *);
    void (*close)(struct csa_device *);
    RET_CODE (*decrypt)(struct csa_device *p_csa_dev, UINT16 stream_id,
                        UINT8 *input, UINT8 *output, UINT32 total_length);
    RET_CODE (*ioctl)(struct csa_device *p_csa_dev, UINT32 cmd, UINT32 param);
    UINT8 id_number;
} CSA_DEV, *p_csa_dev;


/*! @struct DSC_DEV
 *   @brief Reserved for HLD DSC device structure.
 */
typedef struct descrambler_device
{
    struct descrambler_device  *next;  /*next device */
    INT32 type;
    INT8 name[HLD_MAX_NAME_SIZE];
    void *priv;
    UINT32 base_addr;
    UINT32  interrupt_id;
    RET_CODE      (*attach)(void);
    RET_CODE      (*detach)(void);
    RET_CODE    (*open)( struct descrambler_device *p_dsc_dev);
    RET_CODE       (*close)( struct descrambler_device *p_dsc_dev);
    RET_CODE   (*ioctl)(struct descrambler_device *p_dsc_dev,
                        UINT32 cmd, UINT32 param);
} DSC_DEV, *p_dsc_dev;

/*! @struct DSC_PVR_KEY_PARAM
 *   @brief Define PVR key parameters.
 */
typedef struct DSC_PVR_KEY_PARAM
{
    UINT32 input_addr; //!<PVR cipher key address in memory.
    UINT32 valid_key_num; //!<Specify the total valid cipher key number, one key corresponding to one recorded file.
    UINT32 current_key_num; //!<Cipher key index used for current recording.
    UINT32 pvr_key_length; //!<PVR key bit number, should be set to 128 bits.
    UINT8 pvr_user_key_pos; //!<PVR key index in the KL. It's generated by using the PVR cipher key.
    UINT32 total_quantum_number; //!<Specify the max file size for each recording file, file size is equal to total_quantum_number*47K Bytes.
    UINT32 current_quantum_number; //!<Reserved item.
    UINT32 ts_packet_number; //!<Reserved item.
    UINT8 pvr_key_change_enable; //!<The value is always "true".
    	/*!< When the file size reached the specified max file size, DSC will call KL API to generate the next 
	 *	key for recording the next file.
	*/
    UINT16 stream_id;
        //!<Encryption stream ID for current PVR channel. It should be same as the stream ID when creating the key HANDLE for PVR.
    UINT8 pvr_first_key_pos;
        //!<User need to configure the 2nd level key index if the KL works in 3-level mode. 
    void *enc_dev;
        //!<User need to configure the device pointer for encryption.
    UINT8 encrypt_mode;
        //!<User need to configure the encryption mode(AES/DES)
} DSC_PVR_KEY_PARAM, *p_dsc_pvr_key_param;

/*! @struct DSC_BL_UK_PARAM
 *   @brief Reserved for BL UK.
 */
typedef struct DSC_BL_UK_PARAM
{
    UINT8 *input_key;
    UINT8 *r_key;
    UINT8 *output_key;
    UINT32 crypt_type;
} DSC_BL_UK_PARAM, *p_dsc_bl_uk_param;

/*! @struct HW_CHECK_USER_NODE
 *   @brief 
 */
typedef struct HW_CHECK_USER_NODE
{
    UINT32 startaddr;
    UINT32 length;
}HW_CHK_NODE;

/*! @struct HW_CHECK_USER_CONFIG
 *   @brief 
 */
typedef struct HW_CHECK_USER_CONFIG
{
    UINT32 nodecount;
    HW_CHK_NODE *pnodelist;
}HW_CHK_USER_CFG;

/*! @struct DSC_VER_CHK_PARAM
 *   @brief Reserved for version check.
 */
typedef struct DSC_VER_CHK_PARAM
{
    UINT32 input_mem;
    UINT32 len;
    UINT32 chk_mode;
    UINT32 chk_id ;
} DSC_VER_CHK_PARAM, *p_dsc_ver_chk_param;

/*!  @struct DEEN_CONFIG
 *   @brief Define PVR re-encryption configuration parameters.
 */
typedef struct de_encrypt_config
{
    UINT32 do_encrypt ; //!<Encryption enabled. 
	/*!< If do_encrypt is true, the decrypted stream will be encrypted by the specified sub-algorithm. 
	* If the stream to be recorded is in scrambled format, the do_encrypt will be forced to true by driver.
	*/
    void *dec_dev;  //!<Assign the sub-algorithm's device pointer for decryption.
    UINT8 decrypt_mode; //!<Specify the sub-algorithm for decryption, CSA, AES or TDES. 
    UINT16 dec_dmx_id; //!<Stream ID for decryption.
    UINT32 do_decrypt ; //!<Decryption enable.
    void *enc_dev;   //!<Assign the sub-algorithm's device pointer for encryption.
    UINT8 encrypt_mode; //!<Specify the sub-algorithm for encryption, AES or TDES.
    UINT16 enc_dmx_id; //!<Stream ID for encryption. This value must not be same as the dec_dmx_id.
} DEEN_CONFIG, *p_deen_config;

/*! @struct DSC_EN_PRIORITY
 *   @brief Struct for user to change the encryption parity while do PVR re-encryption.
 */
typedef struct dsc_en_priority
{
    enum WORK_SUB_MODULE sub_module; //!<The encryption module.
    enum PARITY_MODE priority; //!<New parity mode.
    UINT32 dev_ptr; //!<The encryption device pointer.
} DSC_EN_PRIORITY, *p_dsc_en_priority;

/*! @struct DSC_FIXED_CRYPTION
 *   @brief Reserved item.
 */
typedef struct dsc_fixed_cryption_param
{
    UINT8 *input;
    UINT32 length;
    UINT32 pos;
}DSC_FIXED_CRYPTION,*p_dsc_fixed_cryption;

/*! @struct DSC_SYS_UK_FW
 *   @brief Reserved item.
 */
typedef struct dsc_deal_sys_fw
{
	UINT8 *input;
	UINT8 *output;
	UINT32 length;
	UINT8 ck[16];
	UINT8 pos;
	enum CRYPT_SELECT mode;
	UINT8 root;
}DSC_SYS_UK_FW;

#define DSC_INVALID_SESSION_HANDLE (0xFFFFFFFF)

struct dsc_sbm_packet {
	unsigned char *input;
	unsigned char *output;
	unsigned int len;
	unsigned int type;
	unsigned char crypto;
	unsigned int sess_handle;
};

/*****************************************************************************
 * Function: dsc_api_attach
 * Description:
 *    This function initializes the DSC hardware and software structures.
 *      This function has to be called once before any other function call of DSC.
 * Input:
 *      None
 * Output:
 *      None
 * Returns:
 *         0: RET_SUCCESS
 *        1: RET_FAILURE
 *
*****************************************************************************/
RET_CODE dsc_api_attach(void);

/*****************************************************************************
 * Function: dsc_api_detach
 * Description:
 *    This function is used to terminal DSC low level driver and release the DSC occupied resources.
 *    If dsc_api_detach is performed, DSC functions should not be called.
 * Input:
 *      None
 * Output:
 *      None
 * Returns:
 *         0: RET_SUCCESS
 *        1: RET_FAILURE
 *
*****************************************************************************/
RET_CODE dsc_api_detach(void);

/*****************************************************************************
 * Function: des_decrypt
 * Description:
 *    This function is used to implement DES/TDES decryption.
 * Input:
 *      DES_DEV *p_des_dev        DES device pointer.
 *        UINT16 stream_id    Stream ID for current decryption.
 *        UINT8 *input    Input data address. *
 *        UINT32 total_length    Data length. For TS data, it should be the total packets number;
 *            For raw data, it should be the total bytes number.
 * Output:
 *      UINT8 *output    Output data address.
 * Returns:
 *         0: RET_SUCCESS
 *        1: RET_FAILURE
*****************************************************************************/
RET_CODE des_decrypt(DES_DEV *p_des_dev, UINT16 stream_id,  UINT8 *input,
                     UINT8 *output, UINT32 total_length);

/*****************************************************************************
 * Function: des_decrypt_rpc
 * Description:
 *    This function is just used to specify the DES/TDES decryption poniter for RPC function list.
 * Input:
 *      DES_DEV     *p_des_dev     DES device pointer.
 *        UINT16 stream_id    Stream ID for current decryption.
 *        UINT8 *input    Input data address. *
 *        UINT32 total_length    Data length. For TS data, it should be the total packets number;
 *            For raw data, it should be the total bytes number.
 * Output:
 *      UINT8 *output    Output data address.
 * Returns:
 *         0: RET_SUCCESS
 *        1: RET_FAILURE
*****************************************************************************/
RET_CODE des_decrypt_rpc(DES_DEV *p_des_dev,UINT16 stream_id,  UINT8 *input,
            UINT8 *output, UINT32 total_length);

/*****************************************************************************
 * Function: des_encrypt
 * Description:
 *    This function is used to implement DES/TDES encryption.
 * Input:
 *      DES_DEV *p_des_dev     DES device pointer.
 *        UINT16 stream_id    Stream ID for current encryption.
 *        UINT8 *input        Input data address. *
 *        UINT32 total_length    Data length. For TS data, it should be the total packets number;
 *        For raw data, it should be the total bytes number.
 * Output:
 *      UINT8 *output    Output data address.
 * Returns:
 *         0: RET_SUCCESS
 *        1: RET_FAILURE
*****************************************************************************/
RET_CODE des_encrypt(DES_DEV *p_des_dev, UINT16 stream_id,  UINT8 *input,
                     UINT8 *output, UINT32 total_length);

/*****************************************************************************
 * Function: des_encrypt_rpc
 * Description:
 *    This function is just used to specify the DES/TDES encryption poniter for RPC function list.
 * Input:
 *      DES_DEV *p_des_dev     DES device pointer.
 *        UINT16 stream_id    Stream ID for current encryption.
 *        UINT8 *input        Input data address. *
 *        UINT32 total_length    Data length. For TS data, it should be the total packets number;
 *        For raw data, it should be the total bytes number.
 * Output:
 *      UINT8 *output    Output data address.
 * Returns:
 *         0: RET_SUCCESS
 *        1: RET_FAILURE
*****************************************************************************/
RET_CODE des_encrypt_rpc ( DES_DEV *p_des_dev, UINT16 stream_id, UINT8 *input,
            UINT8 *output, UINT32 total_length );

/*****************************************************************************
 * Function: des_ioctl
 * Description:
 *    This function is used to implement some DES IO controls.
 * Input:
 *      DES_DEV *p_des_dev     DES device pointer.
 *        UINT32 cmd            IO control commands defined in above.
 *      UINT32 param        Parameters defined in des_init_param etc.
 * Output:
 *      None
 *
 * Returns:
 *         0: RET_SUCCESS
 *        1: RET_FAILURE
*****************************************************************************/
RET_CODE des_ioctl( DES_DEV *p_des_dev , UINT32 cmd , UINT32 param);

/*****************************************************************************
 * Function: aes_decrypt
 * Description:
 *    This function is used to implement AES decryption.
 * Input:
 *      AES_DEV *p_aes_dev     AES device pointer.
 *        UINT16 stream_id    Stream ID for current decryption.
 *        UINT8 *input        Input data address. *
 *        UINT32 total_length    Data length. For TS data, it should be the total packets number;
 *        For raw data, it should be the total bytes number.
 * Output:
 *      UINT8 *output    Output data address.
 * Returns:
 *         0: RET_SUCCESS
 *        1: RET_FAILURE
*****************************************************************************/
RET_CODE aes_decrypt(AES_DEV *p_aes_dev, UINT16 stream_id,  UINT8 *input,
                     UINT8 *output, UINT32 total_length);

/*****************************************************************************
 * Function: aes_decrypt_rpc
 * Description:
 *    This function is just used to specify the AES decryption poniter for RPC function list.
 * Input:
 *      AES_DEV *p_aes_dev     AES device pointer.
 *        UINT16 stream_id    Stream ID for current decryption.
 *        UINT8 *input        Input data address. *
 *        UINT32 total_length    Data length. For TS data, it should be the total packets number;
 *        For raw data, it should be the total bytes number.
 * Output:
 *      UINT8 *output    Output data address.
 * Returns:
 *         0: RET_SUCCESS
 *        1: RET_FAILURE
*****************************************************************************/
RET_CODE aes_decrypt_rpc(AES_DEV *p_aes_dev,UINT16 stream_id,  UINT8 *input,
            UINT8 *output, UINT32 total_length);

/*****************************************************************************
 * Function: aes_encrypt
 * AEScription:
 *    This function is used to implement AES/TAES encryption.
 * Input:
 *      AES_DEV *p_aes_dev     AES device pointer.
 *        UINT16 stream_id    Stream ID for current decryption.
 *        UINT8 *input        Input data address. *
 *        UINT32 total_length    Data length. For TS data, it should be the total packets number;
 *        For raw data, it should be the total bytes number.
 * Output:
 *      UINT8 *output    Output data address.
 * Returns:
 *         0: RET_SUCCESS
 *        1: RET_FAILURE
*****************************************************************************/
RET_CODE aes_encrypt(AES_DEV *p_aes_dev, UINT16 stream_id,  UINT8 *input,
                     UINT8 *output, UINT32 total_length);

/*****************************************************************************
 * Function: aes_encrypt_rpc
 * Description:
 *    This function is just used to specify the AES encryption poniter for RPC function list.
 * Input:
 *      p_aes_dev p_aes_dev     AES device pointer.
 *        UINT16 stream_id    Stream ID for current encryption.
 *        UINT8 *input        Input data address. *
 *        UINT32 total_length    Data length. For TS data, it should be the total packets number;
 *        For raw data, it should be the total bytes number.
 * Output:
 *      UINT8 *output    Output data address.
 * Returns:
 *         0: RET_SUCCESS
 *        1: RET_FAILURE
*****************************************************************************/
RET_CODE aes_encrypt_rpc(AES_DEV *p_aes_dev,UINT16 stream_id,  UINT8 *input,
            UINT8 *output, UINT32 total_length);


/*****************************************************************************
 * Function: aes_ioctl
 * Description:
 *    This function is used to implement some AES IO controls.
 * Input:
 *      AES_DEV *p_aes_dev     AES device pointer.
 *        UINT32 cmd            IO control commands defined in above.
 *      UINT32 param        Parameters defined in aes_init_param etc.
 * Output:
 *      None
 * 
 * Returns:
 *         0: RET_SUCCESS
 *        1: RET_FAILURE
*****************************************************************************/
RET_CODE aes_ioctl( AES_DEV *p_aes_dev , UINT32 cmd , UINT32 param);

/*****************************************************************************
 * Function: csa_decrypt
 * Description:
 *    This function is used to implement CSA decryption.
 * Input:
 *      CSA_DEV *p_csa_dev     CSA device pointer.
 *        UINT16 stream_id    Stream ID for current decryption.
 *        UINT8 *input        Input data address. *
 *        UINT32 total_length    Data length. For TS data, it should be the total packets number;
 *        For raw data, it should be the total bytes number.
 * Output:
 *      UINT8 *output    Output data address.
 * Returns:
 *         0: RET_SUCCESS
 *        1: RET_FAILURE
*****************************************************************************/
RET_CODE csa_decrypt(CSA_DEV *p_csa_dev, UINT16 stream_id,  UINT8 *input,
                     UINT8 *output, UINT32 total_length);

/*****************************************************************************
 * Function: csa_decrypt_rpc
 * Description:
 *    This function is just used to specify the CSA decryption poniter for RPC function list.
 * Input:
 *      CSA_DEV *p_csa_dev     CSA device pointer.
 *        UINT16 stream_id    Stream ID for current decryption.
 *        UINT8 *input        Input data address. *
 *        UINT32 total_length    Data length. For TS data, it should be the total packets number;
 *        For raw data, it should be the total bytes number.
 * Output:
 *      UINT8 *output    Output data address.
 * Returns:
 *         0: RET_SUCCESS
 *        1: RET_FAILURE
*****************************************************************************/

RET_CODE csa_decrypt_rpc ( CSA_DEV *p_csa_dev, UINT16 stream_id, UINT8 *input,
            UINT8 *output, UINT32 total_length);

/*****************************************************************************
 * Function: CSA_ioctl
 * Description:
 *    This function is used to implement some CSA IO controls.
 * Input:
 *      CSA_DEV *p_csa_dev     CSA device pointer.
 *        UINT32 cmd            IO control commands defined in above.
 *      UINT32 param        Parameters defined in CSA_init_param etc.
 * Output:
 *      None
 * Returns:
 *         0: RET_SUCCESS
 *        1: RET_FAILURE
*****************************************************************************/
RET_CODE csa_ioctl( CSA_DEV *p_csa_dev , UINT32 cmd , UINT32 param);


/*****************************************************************************
 * Function: SHA_ioctl
 * Description:
 *    This function is used to implement some SHA IO controls.
 * Input:
 *      SHA_DEV *p_sha_dev     SHA device pointer.
 *        UINT32 cmd            IO control commands defined in above.
 *      UINT32 param        Parameters defined in SHA_init_param etc.
 * Output:
 *      None
 * Returns:
 *         0: RET_SUCCESS
 *        1: RET_FAILURE
*****************************************************************************/
RET_CODE sha_ioctl( SHA_DEV *p_sha_dev , UINT32 cmd , UINT32 param);

/*****************************************************************************
 * Function: sha_digest
 * Description:
 *      This function is used to generate a digest for the input data by using SHA module.
 * Input:
 *      SHA_DEV *p_sha_dev     SHA device pointer.
 *        UINT8 *input        Input data address.
 *        UINT32 data_length    Specify the data size in byte.
 * Output:
 *      UINT8 *output    Output data address.
 * Returns:
 *         0: RET_SUCCESS
 *        1: RET_FAILURE
*****************************************************************************/
RET_CODE sha_digest(SHA_DEV *p_sha_dev, UINT8 *input,
                    UINT8 *output, UINT32 data_length);
RET_CODE sha_digest_rpc(SHA_DEV *p_sha_dev, UINT8 *input,
                    UINT8 *output, UINT32 data_length);

/*****************************************************************************
 * Function: dsc_ioctl
 * Description:
 *    This function is used to implement DSC IO control.
 * Input:
 *      DSC_DEV *p_dsc_dev     DSC device pointer.
 *        UINT32 cmd            IO control commands defined in above.
 *      UINT32 param        Parameters pointer.
 * Output:
 *      None
 * Returns:
 *         0: RET_SUCCESS
 *        1: RET_FAILURE
*****************************************************************************/
RET_CODE dsc_ioctl( DSC_DEV *p_dsc_dev, UINT32 cmd , UINT32 param);
RET_CODE dsc_ioctl_rpc(DSC_DEV *p_dsc_dev, UINT32 cmd, UINT32 param);


/*****************************************************************************
 * Function: trig_ram_mon
 * Description:
 *    This function used to enable the memory monitor feature
 * Input:
 *      UINT32 start_addr: the monitor start address.
 *      UINT32 end_addr: the monitor end address.
 *      UINT32 interval: the interval time of each monitor checking.(enforced by driver, 10~15s)
 *      enum SHA_MODE sha_mode: Memory monitor SHA mode (enforced by driver, SHA256)
 *        BOOL DisableOrEnable: enable or disable (enforced by driver, enable)
 * Output:
 *      None
 * Returns:
 *        0: RET_SUCCESS
 *        1: RET_FAILURE
 *
*****************************************************************************/
RET_CODE trig_ram_mon(UINT32 start_addr, UINT32 end_addr, UINT32 interval,
                      enum SHA_MODE sha_mode, BOOL disable_or_enable);

/*****************************************************************************
 * Function: DeEncrypt
 * Description:
 *    This function is used to re-encrypt the transport stream packets.
 * Input:
 *      DEEN_CONFIG *p_de_en    Re-encryption configuration.
 *        UINT8 *input    Input TS data address.
 *        UINT32 total_length    Data length in TS packets.
 * Output:
 *        UINT8 *output    Output TS data address.
 * Returns:
 *        0: RET_SUCCESS
 *        1: RET_FAILURE
*****************************************************************************/
RET_CODE de_encrypt(DEEN_CONFIG *p_de_en, UINT8 *input,
                   UINT8 *output, UINT32 total_length);

/*****************************************************************************
 * Function: dsc_get_free_stream_id
 * Description:
 *    This function is used to get an idle stream ID with specified data mode.
 *      If the function is performed without error, it will return 4~7 for raw data mode;
 *      it will return 0~3 or 8~15 (not available on the M3603/M3281) for TS data mode.
 * Input:
 *      enum DMA_MODE dma_mode    Specify the data mode, for raw data or TS data.
 * Output:
 *      UINT16 return value
 *
 * Returns:
 *        Success: return a free stream id number which can be used
 *         Fail:     return ALI_INVALID_CRYPTO_STREAM_ID
*****************************************************************************/
UINT16 dsc_get_free_stream_id(enum DMA_MODE dma_mode);


/*****************************************************************************
 * Function: dsc_get_free_sub_device_id
 * Description:
 *    This function is used to get an idle device ID of the specified sub-module.
 * Input:
 *    enum WORK_SUB_MODULE sub_mode    Specify the sub-module.
 * Output:
 *      Equal to the return value UINT32
 *
 * Returns:
 *        Success: return a free sub device id which can be used
 *      Fail:    return ALI_INVALID_DSC_SUB_DEV_ID
*****************************************************************************/
UINT32 dsc_get_free_sub_device_id(enum WORK_SUB_MODULE sub_mode);

/*****************************************************************************
This function is used to get the number of the idle pid sram slot. 
@param[in] enum DMA_MODE dma_mode  Specify the DMA mode(TS_MODE, PURE_DATA_MODE).
@return UINT32
@retval  Success: return the number of the pid sram slot that can be used.
@retval  Fail:    return 0.
*****************************************************************************/
UINT32 dsc_get_idle_pid_sram_num(enum DMA_MODE dma_mode);

/***************************************************************************** 
This function is used to get the number of the idle key sram slot. 
@param[in] KEY_BIT_NUM key_len   Specify the key size(64,128,192,256).
@return UINT32
@retval  Success: return the number of the key sram slot that can be used.
@retval  Fail:    return 0.
*****************************************************************************/
UINT32 dsc_get_idle_key_sram_num( UINT32 key_len );

/*****************************************************************************
This function is used to get the number of the free sub device slot. 
@param[in] enum WORK_SUB_MODULE sub_mode  Specify the sub-module(DES,TDES,AES,SHA,CSA,HMAC).
@return UINT32
@retval  Success: return the number of the free sub device slot that can be used.
@retval  Fail:    return 0.
*****************************************************************************/
UINT32 dsc_get_free_sub_device_num( enum WORK_SUB_MODULE sub_mode);

/*****************************************************************************
This function is used to get the number of the free stream id slot. 
@param[in] enum DMA_MODE dma_mode  Specify the DMA mode(TS_MODE, PURE_DATA_MODE).
@return UINT32
@retval  Success: return the number of the free stream id slot that can be used.
@retval  Fail:    return 0.
*****************************************************************************/
UINT16 dsc_get_free_stream_id_num( enum DMA_MODE dma_mode);

/*****************************************************************************
 * Function: dsc_set_sub_device_id_idle
 * Description:
 *    This function is used to free the specified device ID of the specified sub-module.
 * Input:
 *      enum WORK_SUB_MODULE sub_mode    Specify the sub-module.
 *        UINT32 device_id    Specify the device ID to be freed.
 * Output:
 *      None
 *
 * Returns:
 *        0: RET_SUCCESS
 *        1: RET_FAILURE
 *
*****************************************************************************/
RET_CODE dsc_set_sub_device_id_idle(enum WORK_SUB_MODULE sub_mode,
                                    UINT32 device_id);

/*****************************************************************************
 * Function: dsc_set_sub_device_id_used
 * Description:
 *    This function is used to occupy the specified device id of the specified sub-module.
 * Input:
 *      enum WORK_SUB_MODULE sub_mode    Specify the sub-module.
 *        UINT32 device_id    Specify the device ID to be used.
 * Output:
 *      None
 * Returns:
 *        0: RET_SUCCESS
 *        1: RET_FAILURE
 ****************************************************************************/
RET_CODE dsc_set_sub_device_id_used(enum WORK_SUB_MODULE sub_mode,
                                    UINT32 device_id);

/*****************************************************************************
 * Function: dsc_set_stream_id_idle
 * Description:
 *    This function is used to free the specified stream ID.
 * Input:
 *      UINT32 pos    Specify the value.
 * Output:
 *      None
 * Returns:
 *        0: RET_SUCCESS
 *        1: RET_FAILURE
*****************************************************************************/
RET_CODE dsc_set_stream_id_idle(UINT32 pos);

/*****************************************************************************
 * Function: dsc_set_stream_id_used
 * Description:
 *    This function is used to occupy the specified stream ID.
 * Input:
 *      UINT32 pos    Specify the value.
 * Output:
 *      None
 * Returns:
 *        0: RET_SUCCESS
 *        1: RET_FAILURE
 *
*****************************************************************************/
void dsc_set_stream_id_used(UINT32 pos);

/*****************************************************************************
 * Function: dsc_deal_quantum_for_mixed_ts
 * Description:
 *    This function is used to change key for PVR by quantum
 * Input:
 *      DEEN_CONFIG *p_de_en: Re-encryption configuration.
 *      UINT32 temp_length: the ts packet number of each quantum.
 * Output:
 *      None
 * Returns:
 *        0: RET_SUCCESS
 *        1: RET_FAILURE
 *
*****************************************************************************/
RET_CODE dsc_deal_quantum_for_mixed_ts(DEEN_CONFIG *p_de_en,
                                       UINT32 temp_length);

/*****************************************************************************
 * Function: aes_crypt_puredata_with_ce_key
 * Description:
 *    This function is used to decrypt or encrypt the puredata use key from crypto engine
 * Input:
 *            u8 *input,                the input data
 *            u32 length,               the data length
 *            u32 key_pos,              the key pos in crypto engine
 *            enum CRYPT_SELECT sel     decrypt or encrypt select
 * Output:
 *            u8 *output,               the output data
 * Returns:
 *        0: RET_SUCCESS
 *        1: RET_FAILURE
 *
*****************************************************************************/
RET_CODE aes_crypt_puredata_with_ce_key(UINT8 *input, UINT8 *output,
                                        UINT32 length, UINT32 key_pos,
                                        enum CRYPT_SELECT sel);


/*****************************************************************************
 * Function: aes_pure_ecb_crypt
 * Description:
 *    This function is used to decrypt or encrypt the puredata with ECB mode using key from DSC sram (host key)
 * Input:
 *        UINT8 *key:        the crypt key pointer
 *      UINT8 *input:   the input buffer pointer
 *      UINT32 length:  the crypt data length
 *        UINT8 crypt_mode: the crypt mode
 * Output:
 *        UINT8 *output:  the output buffer pointer
 * Returns:
 *        0: RET_SUCCESS
 *        1: RET_FAILURE
*****************************************************************************/
RET_CODE aes_pure_ecb_crypt( UINT8 *key, UINT8 *input,
                             UINT8 *output, UINT32 length, UINT8 crypt_mode);

/*****************************************************************************
 * Function: aes_pure_cbc_crypt
 * Description:
 *    This function is used to decrypt or encrypt the puredata with CBC mode using key from DSC sram (host key)
 * Input:
 *        UINT8 *key:        the crypt key pointer
 *      UINT8 *input:   the input buffer pointer
 *      UINT32 length:  the crypt data length
 *        UINT8 crypt_mode: the crypt mode
 * Output:
 *        UINT8 *output:  the output buffer pointer
 * Returns:
 *        0: RET_SUCCESS
 *        1: RET_FAILURE
*****************************************************************************/
RET_CODE aes_pure_cbc_crypt( UINT8 *key, UINT8 *iv, UINT8 *input,
                             UINT8 *output, UINT32 length, UINT8 crypt_mode);

/*****************************************************************************
 * Function: aes_pure_ctr_crypt
 * Description:
 *    This function is used to decrypt or encrypt the puredata with CTR mode using key from DSC sram (host key)
 * Input:
 *        UINT8 *key:        the crypt key pointer
 *        UINT8 *ctr:        the ctr buffer pointer
 *      UINT8 *input:   the input buffer pointer
 *      UINT32 length:  the crypt data length
 *        UINT8 crypt_mode: the crypt mode
 * Output:
 *        UINT8 *output:  the output buffer pointer
 * Returns:
 *        0: RET_SUCCESS
 *        1: RET_FAILURE
*****************************************************************************/
RET_CODE aes_pure_ctr_crypt( UINT8 *key, UINT8 *ctr, UINT8 *input,
                             UINT8 *output, UINT32 length, UINT8 crypt_mode);


/*****************************************************************************
 * Function: tdes_pure_ecb_crypt
 * Description:
 *    This function is used to decrypt or encrypt the puredata with ECB mode
 *      using key from DSC sram (host key)
 * Input:
 *        UINT8 *key:        the crypt key pointer
 *      UINT8 *input:   the input buffer pointer
 *      UINT32 length:  the crypt data length
 *        UINT8 crypt_mode: the crypt mode
 * Output:
 *        UINT8 *output:  the output buffer pointer
 * Returns:
 *        0: RET_SUCCESS
 *        1: RET_FAILURE
 *
*****************************************************************************/
RET_CODE tdes_pure_ecb_crypt( UINT8 *key, UINT8 *input, UINT8 *output,
                              UINT32 length, UINT8 crypt_mode);

/*****************************************************************************
 * Function: tdes_pure_ecb_crypt_with_ce_key
 * Description:
 *    This function is used to decrypt or encrypt the puredata with ECB mode
 *      using key from crypto engine sram
 * Input:
 *        UINT32 key_pos:    the crypt key position in ecrypto engine
 *      UINT8 *input:   the input buffer pointer
 *      UINT32 length:  the crypt data length
 *        UINT8 crypt_mode: the crypt mode
 * Output:
 *        UINT8 *output:  the output buffer pointer
 * Returns:
 *        0: RET_SUCCESS
 *        1: RET_FAILURE
 *
*****************************************************************************/
RET_CODE tdes_pure_ecb_crypt_with_ce_key( UINT32 key_pos, UINT8 *input,
        UINT8 *output, UINT32 length,
        UINT8 crypt_mode);

/*****************************************************************************
 * Function: aes_pure_ctr_crypt_with_ce_key
 * Description:
 *    This function is used to decrypt or encrypt the puredata with CTR mode
 *      using key from crypto engine sram
 * Input:
 *        UINT32 key_pos:    the crypt key position in ecrypto engine
 *      UINT8 *input:   the input buffer pointer
 *      UINT32 length:  the crypt data length
 *        UINT8 crypt_mode: the crypt mode
 * Output:
 *        UINT8 *output:  the output buffer pointer
 * Returns:
 *        0: RET_SUCCESS
 *        1: RET_FAILURE
 *
*****************************************************************************/
RET_CODE aes_pure_ctr_crypt_with_ce_key( UINT32 key_pos, UINT8 *ctr,
        UINT8 *input, UINT8 *output,
        UINT32 length, UINT8 crypt_mode);

/*****************************************************************************
 * Function: ali_dsc_encrypt_bl_uk
 * Description:
 *    This function is used to encrypt the Bootloader universal key
 * Input:
 *      UINT8 *input:   the input buffer pointer
 *      UINT8 *r_key:   the random number buffer pointer
 *        UINT32 encrypt_type: the crypt mode using which key and which mode
 * Output:
 *        UINT8 *output:  the output buffer pointer
 * Returns:
 *        0: RET_SUCCESS
 *        1: RET_FAILURE
 *
*****************************************************************************/
RET_CODE ali_dsc_encrypt_bl_uk(UINT8 *input, UINT8 *r_key,
        UINT8 *output, UINT32 encrypt_type);

RET_CODE aes_nbits_pure_cbc_crypt_keysram( UINT8 *key, UINT8 *iv, UINT8 *input, 
	UINT8 *output, UINT32 length, UINT8 crypt_mode, UINT32 key_length, UINT32 residue);

RET_CODE aes_128_pure_ecb_crypt_keyotp(UINT8 *input, UINT8 *output, 
UINT32 length, UINT8 crypt_mode, UINT32 otp_key_pos);

/*****************************************************************************
 * Function: see_version_check
 * Description:
 *    This function is used to do the version checking of software
 *    This function only is available in SEE software
 * Input:
 *            UINT32 block_id,      the check block id number
 *            UINT32 block_addr,    the check block address in dram
 *            UINT32 block_len,     the check block length
 * Output:
 *            None
 * Returns:
 *        0: RET_SUCCESS
 *        1: RET_FAILURE
 *
*****************************************************************************/
RET_CODE see_version_check(UINT32 block_id, UINT32 block_addr, UINT32 block_len);

/*****************************************************************************
 * Function: ali_sha_digest
 * Description:
 *    This function is used to caculate the digest of input data with specified SHA mode
 * Input:
 *            UINT8 *input,               the input buffer pointer
 *            UINT32 input_len,           the crypt data length
 *            enum SHA_MODE sha_mode,     SHA mode
 * Output:
 *            UINT8 *output:  the output buffer pointer
 * Returns:
 *        0: RET_SUCCESS
 *        1: RET_FAILURE
 *
*****************************************************************************/
RET_CODE ali_sha_digest(UINT8 *input, UINT32 input_len, enum SHA_MODE sha_mode, UINT8 *output);

/*****************************************************************************
 * Function: see_ali_sha_digest
 * Description:
 *    This function is used to caculate the digest of input data with specified SHA mode
 * Input:
 *            UINT8 *input,               the input buffer pointer
 *            UINT32 input_len,           the crypt data length
 *            enum SHA_MODE sha_mode,     SHA mode
 * Output:
 *            UINT8 *output:  the output buffer pointer
 * Returns:
 *        0: RET_SUCCESS
 *        1: RET_FAILURE
 *
*****************************************************************************/
RET_CODE see_ali_sha_digest(UINT8 *input, UINT32 input_len, enum SHA_MODE sha_mode, UINT8 *output);

/*****************************************************************************
 * Function: dsc_set_fb_region
 * Description:
 *    This function is to set the DSC special region.
 * Input:
 *            UINT32 addr         the region addr
 *            UINT32 size          the region size
 * Returns:
 *        0: RET_SUCCESS
 *        1: RET_FAILURE
 *
*****************************************************************************/
RET_CODE dsc_set_fb_region(UINT32 addr, UINT32 size);

/*****************************************************************************
 * Function: dsc_fixed_cryption
 * Description:
 *    This function is to used to one DSC fixed cryption.
 * Input:
 *            UINT32 addr         the region addr
 *            UINT32 size          the region size
 * Returns:
 *        0: RET_SUCCESS
 *        1: RET_FAILURE
 *
*****************************************************************************/
RET_CODE dsc_fixed_cryption(UINT8 *input, UINT32 length, UINT32 pos);

/*****************************************************************************
 * Function: dsc_deal_sys_uk_fw
 * Description:
 *    This function is to used encrypt/decrypt the system UK or FW.
 * Input:
 *            UINT8 *input,                          input data addr
 *            UINT8 *output                         output data addr
 *            UINT8 *key                           cipher key
 *            UINT32 length                          data size (in byte)
 *            UINT32 pos                               key index in keyladder
 *            enum CRYPT_SELECT mode        DSC_ENCRYPT or DSC_DECRYPT
 *
 * Returns:
 *        0: RET_SUCCESS
 *        1: RET_FAILURE
 *
*****************************************************************************/
RET_CODE dsc_deal_sys_uk_fw(UINT8 *input, UINT8 *output, UINT8 *key, UINT32 length,
								UINT32 pos, enum CRYPT_SELECT mode, UINT8 root);

/*****************************************************************************
 * Function: dsc_enable_disable_cmdq
 * Description:
 *    This function is to used enable or disable cmdq mode in DSC(available from M3823).
 * Input:
 *            UINT32 en_or_dis                        en is 1, dis is 0.
 *
 * Returns:
 *        0: RET_SUCCESS
 *        1: RET_FAILURE
 *
*****************************************************************************/
RET_CODE dsc_enable_disable_cmdq(UINT32 en_or_dis);


/*****************************************************************************
 * Just define below functions for compiling, user no need to use these symbols.
 *
*****************************************************************************/
void hld_dsc_callee(UINT8 *msg);
void lld_dsc_m36f_callee( UINT8 *msg );

/*
	Transfer data from SEE address to Main address, at most 47KB.

	*input: private address
	*output: main address
	length: buffer length, should less than 47*l024
*/
RET_CODE dsc_dma_transfer(UINT8 *input, UINT8 *output, UINT32 length);

RET_CODE dsc_crypt_list_rpc(struct dsc_sbm_packet *buf, UINT32 count);

#ifdef __cplusplus
}
#endif

/*!
@}
@}
*/


#endif  /*_DSC_H_*/
