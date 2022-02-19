/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: crypto.h
*
*    Description: this file is used to define some macros and structures
*                 for secure key ladder
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef  _CRYPTO_H_
#define  _CRYPTO_H_

#ifdef __cplusplus
extern "C" {
#endif

/*! @~English @addtogroup DeviceDriver
 *  @{
    @~
 */

/*! @~English @addtogroup KeyLadder(KL)
 *  @{
    @~
 */

#define  IO_OTP_ROOT_KEY_GET           0
//!< Load the specified OTP root key to KEY_0_X, along with 'struct OTP_PARAM'.
#define  IO_SECOND_KEY_GENERATE        1
//!< Generate the second(next) key from first(previous) key
#define  IO_CRYPT_DATA_INPUT           2
//!< Write the Input Data to the KL.
#define  IO_CRYPT_PARAM_SET            3
//!< Set the parameters to the KL.
#define  IO_CRYPT_SECOND_KEY_CONFIG    4
//!< Configurate the second key.

#define  IO_CRYPT_DEBUG_GET_KEY        5
//!< Reserved for caller to get the KL debug key.
#define  IO_CRYPT_POS_IS_OCCUPY        6
//!< Check if the key idex is occupied or not, along with 'struct CE_POS_STS_PARAM'.
#define  IO_CRYPT_POS_SET_USED         7
//!< Set the key index status to busy directly, usually used on the application initialization stage.
#define  IO_CRYPT_POS_SET_IDLE         8
//!< Set the key index status to idle.
#define  IO_CRYPT_FOUND_FREE_POS       9
//!< Get an idle key index and set this index status to busy, along with 'struct CE_FOUND_FREE_POS_PARAM'.
#define  IO_DECRYPT_PVR_USER_KEY       10
//!< Used to decrypt the PVR cipher key to the specified PVR key index, refer to 'struct CE_PVR_KEY_PARAM', only used in DMX driver.
#define  IO_CRYPT_SET_PVR_CST_KEY       11

#define  IO_CRYPT_GEN_NLEVEL_KEY       12
/*!< Generate n level keys, refer to 'struct CE_NLEVEL_PARAM'. HW has only one intermediate key_pos for one KL, 
thus driver shall generate all the required levels in one call to ensure the thread-safety. */

#define  IO_CRYPT_CW_DERIVE_CW       13
/*!< Generate CW from CW, refer to 'struct CE_CW_DERIVATION'.*/

#define  IO_CRYPT_ETSI_CHALLENGE       14
/*!< Perform ETSI challenge*/

#define  IO_CRYPT_GEN_NLEVEL_KEY_ADVANCED       15
/*!< Generate n level keys, refer to 'struct CE_NLEVEL_ADVANCED_PARAM'. it's extension of  IO_CRYPT_GEN_NLEVEL_KEY. 
*	you can control all the middle level paramter with this interface.
*/


#define CE_PARITY_EVEN (1<<0)
//!< Only used for #CE_SELECT_DES, generate to even position.
#define CE_PARITY_ODD (1<<1)
//!< Only used for #CE_SELECT_DES, generate to odd position.
#define CE_PARITY_EVEN_ODD (CE_PARITY_EVEN | CE_PARITY_ODD)
//!< Only used for #CE_SELECT_DES, generate both the even and odd key.

#define IO_CE_VSC_OFFSET 0x50
/*! @VSC dediacated key ladder.
 *	 @bried the ioctl cmd for vsc dedicated key ladder.
 */
#define IO_CRYPT_VSC_DECW_KEY					(IO_CE_VSC_OFFSET + 0)
//!< Send the encrypted CW to the dedicated vsc module, along with 'CE_DECW_KEY_PARAM'.
#define IO_CRYPT_VSC_REC_EN_KEY					(IO_CE_VSC_OFFSET + 1)
//!< Send the encrypted key for the pvr record, along with 'CE_REC_KEY_PARAM'.
#define IO_CRYPT_VSC_UK_EN_KEY					(IO_CE_VSC_OFFSET + 2)
//!< Send the encrypted key for the UK, along with 'CE_UK_KEY_PARAM'.

#define OTP_ADDESS_1 0x4d  //!< OTP physical address of secure key 0.
#define OTP_ADDESS_2 0x51  //!< OTP physical address of secure key 1.
#define OTP_ADDESS_3 0x55  //!< OTP physical address of secure key 2.
#define OTP_ADDESS_4 0x59  //!< OTP physical address of secure key 3.
#define OTP_ADDESS_5 0x60  //!< OTP physical address of secure key 4.
#define OTP_ADDESS_6 0x64  //!< OTP physical address of secure key 5.
#define OTP_ADDESS_7 0x68  //!< OTP physical address of secure key 6.
#define OTP_ADDESS_8 0x6c  //!< OTP physical address of secure key 7.

#define AES_CE_KEY_LEN  16
 //!< Indicate the AES cipher block length in KL is 16 bytes.
#define XOR_CE_KEY_LEN  16
 //!< Indicate the XOR cipher block length in KL is 16 bytes.
#define TDES_CE_KEY_LEN  8
 //!< Indicate the TDES cipher block length in KL is 16 bytes.

#define INVALID_ALI_CE_KEY_POS 0xff
  //!< Define the invalid KL key index
#define ALI_INVALID_CRYPTO_KEY_POS   INVALID_ALI_CE_KEY_POS
 //!< Define the invalid KL key index

/*! @enum CRYPTO_STATUS
 *   @brief Return values of the KL functions.
 */
typedef enum ce_crypto_status
{
    ALI_CRYPTO_SUCCESS = RET_SUCCESS,  //!< The intended operation was executed successfully.
    ALI_CRYPTO_ERROR, //!< The function terminated abnormally. The intended operation failed.
    ALI_CRYPTO_WARNING_DRIVER_ALREADY_INITIALIZED, //!< The SEE KL is already initialized.

    ALI_CRYPTO_ERROR_INVALID_PARAMETERS, //!< The passed parameters are invalid.
    ALI_CRYPTO_ERROR_OPERATION_NOT_ALLOWED, //!< The requested operation is not allowed.
    ALI_CRYPTO_ERROR_OPERATION_NOT_SUPPORTED, //!< The requested operation is not supported.
    ALI_CRYPTO_ERROR_INITIALIZATION_FAILED, //!< The SEE KL initialization failed.
    ALI_CRYPTO_ERROR_DRIVER_NOT_INITIALIZED, //!< The SEE KL has not been initialized.
    ALI_CRYPTO_ERROR_INVALID_ADDR, //!< The passed address is invalid.
    ALI_CRYPTO_ERROR_INVALID_DEV, //!< The passed device pointer is invalid.
    ALI_CRYPTO_ERROR_PVR_CST_KEY_NOT_SET, //!< The CST Key is not set.
}CRYPTO_STATUS;

#define ALI_KL_PREFIX (0x00)
#define ETSI_KL_PREFIX (0x10)
/*! @enum CE_OTP_KEY_SEL
 *   @brief Define the KL root key index.
 */
enum CE_OTP_KEY_SEL
{
    OTP_KEY_0_0 = 0, //!< Load root key from OTP 0x4d.
    OTP_KEY_0_1 = 1, //!< Load root key from OTP 0x51.
    OTP_KEY_0_2 = 2, //!< Load root key from OTP 0x55 or OTP 0x60.
    OTP_KEY_0_3 = 3, //!< Load root key from OTP 0x59 or OTP 0x64.
    OTP_KEY_0_4 = 4,
    OTP_KEY_0_5 = 5,

    ALI_KL_0 = 0, //! < choose to use ALi Kl 0, load root key from OTP 0x4d
    ALI_KL_1 = 1, //! < choose to use ALi Kl 1, load root key from OTP 0x51
    ALI_KL_2 = 2, //! < choose to use ALi Kl 2, load root key from OTP 0x55 or 0x60
    ALI_KL_3 = 3, //! < choose to use ALi Kl 3, load root key from OTP 0x59 or 0x64
    ALI_KL_4 = 4, //! < choose to use ALi Kl 4, load root key from OTP 0x60
    ALI_KL_5 = 5, //! < choose to use AKL
	ALI_KL_HMAC = 6, //hmac
    
    ETSI_KL_0 = (ETSI_KL_PREFIX | 0), //!< choose to use ETSI Kl 0, introduced from CAP210 on
    ETSI_KL_1 = (ETSI_KL_PREFIX | 1), //!< choose to use ETSI Kl 1
    ETSI_KL_2 = (ETSI_KL_PREFIX | 2), //!< choose to use ETSI Kl 2
    ETSI_KL_3 = (ETSI_KL_PREFIX | 3), //!< choose to use ETSI Kl 3
    ETSI_KL_4 = (ETSI_KL_PREFIX | 4), //!< choose to use ETSI Kl 4
};

/*! @enum HDCP_DECRYPT_MODE
 *   @brief Define KL SRAM operation mode, which is usually NOT_FOR_HDCP.
 */
enum  HDCP_DECRYPT_MODE
{
    NOT_FOR_HDCP = 0, //!< For KL key operation.
    TARGET_IS_HDCP_KEY_SRAM = (1 << 14) //!< For HDCP key operation
};

/*! @enum CE_CRYPT_TARGET
 *   @brief Define the 1st, 2nd, and 3rd level keys. CRYPTO_KEY_X_X is the equivalent to KEY_X_X. 
 *	The first X indicates the key's level, the second X indicates the key's index in current level.
 *    CRYPT_KEY_1_X is genereted by OTP_KEY_0_X; CRYPT_KEY_2_X is genereted by CRYPT_KEY_1_X;
 *    CRYPT_KEY_3_X is genereted by CRYPT_KEY_2_X;
 */
enum CE_CRYPT_TARGET
{
    CRYPT_KEY_1_0 = 0x4,
    CRYPT_KEY_1_1 = 0x5,
    CRYPT_KEY_1_2 = 0x6,
    CRYPT_KEY_1_3 = 0x7,
    CRYPT_KEY_1_4 = 0x41, /*the 5th KL's 1st addr, introduced from nv3 chip*/

    CRYPT_KEY_2_0 = 0x8,
    CRYPT_KEY_2_1 = 0x9,
    CRYPT_KEY_2_2 = 0xa,
    CRYPT_KEY_2_3 = 0xb,
    CRYPT_KEY_2_4 = 0xc,
    CRYPT_KEY_2_5 = 0xd,
    CRYPT_KEY_2_6 = 0xe,
    CRYPT_KEY_2_7 = 0xf,
    CRYPT_KEY_2_8 = 0x42, /*the 5th KL's 2nd addr, introduced from nv3 chip*/

    /* KL 0 */
    CRYPT_KEY_3_0 = 0x10,
    CRYPT_KEY_3_1 = 0x11,
    CRYPT_KEY_3_2 = 0x12,
    CRYPT_KEY_3_3 = 0x13,
    CRYPT_KEY_3_4 = 0x14,
    CRYPT_KEY_3_5 = 0x15,
    CRYPT_KEY_3_6 = 0x16,
    CRYPT_KEY_3_7 = 0x17,
    CRYPT_KEY_3_8 = 0x18,
    CRYPT_KEY_3_9 = 0x19,
    CRYPT_KEY_3_10 = 0x1a,
    CRYPT_KEY_3_11 = 0x1b,
    CRYPT_KEY_3_12 = 0x1c,
    CRYPT_KEY_3_13 = 0x1d,
    CRYPT_KEY_3_14 = 0x1e,
    CRYPT_KEY_3_15 = 0x1f,

    /* KL 1 */
    CRYPT_KEY_3_16=0x20,
    CRYPT_KEY_3_17=0x21,
    CRYPT_KEY_3_18=0x22,
    CRYPT_KEY_3_19=0x23,
    CRYPT_KEY_3_20=0x24,
    CRYPT_KEY_3_21=0x25,
    CRYPT_KEY_3_22=0x26,
    CRYPT_KEY_3_23=0x27,
    CRYPT_KEY_3_24=0x28,
    CRYPT_KEY_3_25=0x29,
    CRYPT_KEY_3_26=0x2a,
    CRYPT_KEY_3_27=0x2b,
    CRYPT_KEY_3_28=0x2c,
    CRYPT_KEY_3_29=0x2d,
    CRYPT_KEY_3_30=0x2e,
    CRYPT_KEY_3_31=0x2f,

    /* KL 2-1 */
    CRYPT_KEY_3_32=0x30,
    CRYPT_KEY_3_33=0x31,
    CRYPT_KEY_3_34=0x32,
    CRYPT_KEY_3_35=0x33,
    CRYPT_KEY_3_36=0x34,
    CRYPT_KEY_3_37=0x35,
    CRYPT_KEY_3_38=0x36,
    CRYPT_KEY_3_39=0x37,

    /* KL 3-1 */
    CRYPT_KEY_3_40=0x38,
    CRYPT_KEY_3_41=0x39,
    CRYPT_KEY_3_42=0x3a,
    CRYPT_KEY_3_43=0x3b,
    CRYPT_KEY_3_44=0x3c,
    CRYPT_KEY_3_45=0x3d,
    CRYPT_KEY_3_46=0x3e,
    CRYPT_KEY_3_47=0x3f,

    /* KL 2-2 */ /*enlarge KL2 CW number to 16, from nv3 chip on*/
    CRYPT_KEY_3_48=0x52,
    CRYPT_KEY_3_49=0x53,
    CRYPT_KEY_3_50=0x54,
    CRYPT_KEY_3_51=0x55,
    CRYPT_KEY_3_52=0x56,
    CRYPT_KEY_3_53=0x57,
    CRYPT_KEY_3_54=0x58,
    CRYPT_KEY_3_55=0x59,

    /* KL 3-2 */ /*enlarge KL3 CW number to 16, from nv3 chip on*/
    CRYPT_KEY_3_56=0x5a,
    CRYPT_KEY_3_57=0x5b,
    CRYPT_KEY_3_58=0x5c,
    CRYPT_KEY_3_59=0x5d,
    CRYPT_KEY_3_60=0x5e,
    CRYPT_KEY_3_61=0x5f,
    CRYPT_KEY_3_62=0x60,
    CRYPT_KEY_3_63=0x61,

    /* KL 4 */ /* KL4 has 16 CWs*/
    CRYPT_KEY_3_64=0x62,
    CRYPT_KEY_3_65=0x63,
    CRYPT_KEY_3_66=0x64,
    CRYPT_KEY_3_67=0x65,
    CRYPT_KEY_3_68=0x66,
    CRYPT_KEY_3_69=0x67,
    CRYPT_KEY_3_70=0x68,
    CRYPT_KEY_3_71=0x69,
    CRYPT_KEY_3_72=0x6a,
    CRYPT_KEY_3_73=0x6b,
    CRYPT_KEY_3_74=0x6c,
    CRYPT_KEY_3_75=0x6d,
    CRYPT_KEY_3_76=0x6e,
    CRYPT_KEY_3_77=0x6f,
    CRYPT_KEY_3_78=0x70,
    CRYPT_KEY_3_79=0x71,

    /* KL 5 */ /* KL5 (AKL) has 16 CWs*/
    CRYPT_KEY_3_80=0x72,
    CRYPT_KEY_3_81=0x73,
    CRYPT_KEY_3_82=0x74,
    CRYPT_KEY_3_83=0x75,
    CRYPT_KEY_3_84=0x76,
    CRYPT_KEY_3_85=0x77,
    CRYPT_KEY_3_86=0x78,
    CRYPT_KEY_3_87=0x79,
    CRYPT_KEY_3_88=0x7a,
    CRYPT_KEY_3_89=0x7b,
    CRYPT_KEY_3_90=0x7c,
    CRYPT_KEY_3_91=0x7d,
    CRYPT_KEY_3_92=0x7e,
    CRYPT_KEY_3_93=0x7f,
    CRYPT_KEY_3_94=0x80,
    CRYPT_KEY_3_95=0x81,
};

/*! @enum CE_KEY
 *   @brief Define the 1st, 2nd, and 3rd level keys, KEY_X_X is the equivalent to CRYPTO_KEY_X_X. 
 *   The first X indicates the key's level, the second X indicates the key's index in current level.
 */
enum CE_KEY
{
	KEY_0_0=0,
	KEY_0_1=1,
	KEY_0_2=2,
	KEY_0_3=3,
	KEY_0_4=0x40, /*the 5th KL's root key addr, introduced from nv3 chip*/
	
	KEY_1_0=4,
	KEY_1_1=5,
	KEY_1_2=6,
	KEY_1_3=7,
	KEY_1_4=0x41, /*the 5th KL's 1st level addr, introduced from nv3 chip*/
	
	KEY_2_0=8,
	KEY_2_1=9,
	KEY_2_2=0xa,
	KEY_2_3=0xb,
	KEY_2_4=0xc,
	KEY_2_5=0xd,
	KEY_2_6=0xe,
	KEY_2_7=0xf,
	KEY_2_8=0x42, /*the 5th KL's 2nd level addr, introduced from nv3 chip*/
	
	/* KL 0 */
	KEY_3_0=0x10,
	KEY_3_1=0x11,
	KEY_3_2=0x12,
	KEY_3_3=0x13,
	KEY_3_4=0x14,
	KEY_3_5=0x15,
	KEY_3_6=0x16,
	KEY_3_7=0x17,
	KEY_3_8=0x18,
	KEY_3_9=0x19,
	KEY_3_10=0x1a,
	KEY_3_11=0x1b,
	KEY_3_12=0x1c,
	KEY_3_13=0x1d,
	KEY_3_14=0x1e,
	KEY_3_15=0x1f,
		
	/* KL 1 */
	KEY_3_16=0x20,
	KEY_3_17=0x21,
	KEY_3_18=0x22,
	KEY_3_19=0x23,
	KEY_3_20=0x24,
	KEY_3_21=0x25,
	KEY_3_22=0x26,
	KEY_3_23=0x27,
	KEY_3_24=0x28,
	KEY_3_25=0x29,
	KEY_3_26=0x2a,
	KEY_3_27=0x2b,
	KEY_3_28=0x2c,
	KEY_3_29=0x2d,
	KEY_3_30=0x2e,
	KEY_3_31=0x2f,
	
	/* KL 2-1 */
	KEY_3_32=0x30,
	KEY_3_33=0x31,
	KEY_3_34=0x32,
	KEY_3_35=0x33,
	KEY_3_36=0x34,
	KEY_3_37=0x35,
	KEY_3_38=0x36,
	KEY_3_39=0x37,
	
	/* KL 3-1 */
	KEY_3_40=0x38,
	KEY_3_41=0x39,
	KEY_3_42=0x3a,
	KEY_3_43=0x3b,
	KEY_3_44=0x3c,
	KEY_3_45=0x3d,
	KEY_3_46=0x3e,
	KEY_3_47=0x3f,	
	
	/* KL 2-2 */ /*enlarge KL2 CW number to 16, from nv3 chip on*/
	KEY_3_48=0x52,
	KEY_3_49=0x53,
	KEY_3_50=0x54,
	KEY_3_51=0x55,
	KEY_3_52=0x56,
	KEY_3_53=0x57,
	KEY_3_54=0x58,
	KEY_3_55=0x59,
	
	/* KL 3-2 */ /*enlarge KL3 CW number to 16, from nv3 chip on*/
	KEY_3_56=0x5a,
	KEY_3_57=0x5b,
	KEY_3_58=0x5c,
	KEY_3_59=0x5d,
	KEY_3_60=0x5e,
	KEY_3_61=0x5f,
	KEY_3_62=0x60,
	KEY_3_63=0x61,
	
	/* KL 4 */ /* KL4 has 16 CWs*/
	KEY_3_64=0x62,
	KEY_3_65=0x63,
	KEY_3_66=0x64,
	KEY_3_67=0x65,
	KEY_3_68=0x66,
	KEY_3_69=0x67,
	KEY_3_70=0x68,
	KEY_3_71=0x69,
	KEY_3_72=0x6a,
	KEY_3_73=0x6b,
	KEY_3_74=0x6c,
	KEY_3_75=0x6d,
	KEY_3_76=0x6e,
	KEY_3_77=0x6f,
	KEY_3_78=0x70,
	KEY_3_79=0x71,
	
	/* KL 5 */ /* KL5(AKL) has 16 CWs*/
	KEY_3_80=0x72,
	KEY_3_81=0x73,
	KEY_3_82=0x74,
	KEY_3_83=0x75,
	KEY_3_84=0x76,
	KEY_3_85=0x77,
	KEY_3_86=0x78,
	KEY_3_87=0x79,
	KEY_3_88=0x7a,
	KEY_3_89=0x7b,
	KEY_3_90=0x7c,
	KEY_3_91=0x7d,
	KEY_3_92=0x7e,
	KEY_3_93=0x7f,
	KEY_3_94=0x80,
	KEY_3_95=0x81,
};

/*! @enum CE_CRYPT_SELECT
 *   @brief Define KL encryption and decryption mode.
 */
enum CE_CRYPT_SELECT
{
    CE_IS_DECRYPT = 1, //!<Decryption
    CE_IS_ENCRYPT = 0  //!<Encryption
};

/*! @enum CE_MODULE_SELECT
 *   @brief Define KL algorithm selection.
 */
enum CE_MODULE_SELECT
{
    CE_SELECT_DES = 0,  //!<KL algorithm is DES.
    CE_SELECT_AES = 1,  //!<KL algorithm is AES.
    CE_SELECT_XOR = 2,  //!<KL algorithm is XOR.
    CE_SELECT_AES64BIT = 3, //!<KL algorithm is AES-64Bit.
    CE_SELECT_XOR64BIT = 4, //!<KL algorithm is XOR-64Bit.
};

enum DATA_MODULE_MODE
{
    BIT128_DATA_MODULE = 0,
    BIT64_DATA_MODULE =  1
};

enum DATA_HILO_MODE
{
    LOW_64BITS_DATA = 0,
    HIGH_64BITS_DATA= 1
};

enum KEY_MODULE_FROM
{
    CE_KEY_FROM_SRAM = 0,
    CE_KEY_FROM_CPU  = 1,
};

enum DATA_MODULE_FROM
{
    CE_DATA_IN_FROM_CPU = 0,
    CE_DATA_IN_FROM_SRAM = 1,
};

/*! @enum HDCP_KEY_SELECT
 *   @brief Specify key type which is used for reading the KL debug key.
 */
enum HDCP_KEY_SELECT
{
    CE_KEY_READ = 0,  //!<Read KL key
    HDCP_KEY_READ = 1 //!<Read HDCP key
};

/*! @enum ce_key_level
 *   @brief Define the key level that API will get from KL.
 */
enum ce_key_level
{
    SKIP_LEVEL = 0, //!<Internal reserved.
    ONE_LEVEL, //!<Get an idle key index from 1st, 2nd and 3rd level keys.
    TWO_LEVEL, //!<Get an idle key index from 2nd and 3rd level keys.
    THREE_LEVEL, //!<Get an idle key index from 3rd level keys.

    FIVE_LEVEL = 5, //! <Get an idle key pos from 5th level, support from CAP210 on
};

/*! @struct CE_PVR_KEY_PARAM
  *   @brief Struct used to change the PVR key. Only used in DMX driver when doing playback.
 */
typedef struct ce_pvr_key_param
{
    UINT8 *input_addr; //!<Buffer address of the 16 bytes cipher CW
    UINT32 second_pos; //!<Specify the target key index.
    UINT32 first_pos; //!<Specify the source key index.
} CE_PVR_KEY_PARAM, *p_ce_pvr_key_param;

/*! @struct CE_PVR_CST_KEY
 *   @brief Struct used to set intermediate const key.
 */
typedef struct ce_pvr_cst_key
{
	UINT8 pvr_otp_key_pos; //!< KL OTP key index. OTP_KEY_0_0 ~ OTP_KEY_0_5
	UINT8 cst_key_length; //!< Intermidate const key length in byte, usually it's 16 or 32 bytes
	UINT8 cst_key[32]; //!< const key buffer, lower level key is in the lower memory address.
} CE_PVR_CST_KEY, *ce_pvr_cst_key;

/*! @struct OTP_PARAM
 *   @brief Struct used to load physical OTP root key to KL KEY_0_X.
 */
typedef struct otp_param
{
    UINT8 otp_addr; //!<OTP root key physical address.
    enum CE_OTP_KEY_SEL  otp_key_pos; //!<KL root key index, KEY_0_X.
} OTP_PARAM, *p_otp_param;

/*! @struct DATA_PARAM
 *  @brief Struct used to specify the data block information.
 */
typedef struct data_param
{
    UINT32 crypt_data[4] ;  //!<Input data buffer.
    UINT32 data_len ; //!<Input data length.
} DATA_PARAM, *p_data_param;

/*! @struct DES_PARAM
 *  @brief Struct used to specify the cryption mode, algorithm and result location(TDES only).
 */
typedef struct  des_param
{
    enum CE_CRYPT_SELECT  crypt_mode; //!<Encryption or decryption selection.
    enum CE_MODULE_SELECT aes_or_des; //!<AES or TDES algorithm selection.
    UINT8 des_low_or_high; //!<Select TDES result location in higher 8bytes or lower 8bytes.
} DES_PARAM, *p_des_param;

/*! define CE_KEY_PARAM.kl_index_sel
 *  @brief KL select
 */
#define CE_SEL_KL_0 (0)
#define CE_SEL_KL_1 (1)
#define CE_SEL_KL_2 (2)
#define CE_SEL_KL_3 (3)
#define CE_SEL_KL_4 (4)
#define CE_SEL_ETSI_0 (ETSI_KL_PREFIX + 0)
#define CE_SEL_ETSI_1 (ETSI_KL_PREFIX + 1)
#define CE_SEL_ETSI_2 (ETSI_KL_PREFIX + 2)
#define CE_SEL_ETSI_3 (ETSI_KL_PREFIX + 3)
#define CE_SEL_ETSI_4 (ETSI_KL_PREFIX + 4)

/*! define CE_KEY_PARAM.level
 *  @brief KL select
 */
#define CE_GEN_1ST_LEVEL_KEY (1)
#define CE_GEN_2ND_LEVEL_KEY (2)
#define CE_GEN_3RD_LEVEL_KEY (3)
#define CE_GEN_4TH_LEVEL_KEY (4)
#define CE_GEN_5TH_LEVEL_KEY (5)

/*! @struct CE_KEY_PARAM
 *  @brief Define source key index, target key index and KL SRAM operation mode.
 */
typedef struct ce_key_param
{
    enum CE_KEY first_key_pos; //!<Source key index.
    enum CE_CRYPT_TARGET second_key_pos; //!<Target key index.
    enum HDCP_DECRYPT_MODE hdcp_mode ; //!<KL SRAM operation mode.

    /*introduced for CAP210 chipset*/
    int use_new_mng_style; //! setting 1 indicates to use below info for the new KL key management and generation.
    int kl_index_sel; //! < CE_SEL_KL_0~4, CE_SEL_ETSI_0~4
    int gen_level; //! <generating which level key
    int cw_pos; //! <the cw position. this pos should be get free from driver. when generating level 5 need this pos parameter.
} CE_KEY_PARAM, *p_ce_key_param;

/*! @struct CE_DEBUG_KEY_INFO
 *   @brief Reserved.
 */
typedef struct ce_debug_key_info
{
    enum HDCP_KEY_SELECT sel; //!<Read HDCP or not HDCP.
    UINT32 buffer[4]; //!<Buffer to store the debug key.
    UINT32 len;  //!<Length in byte, equal to algorithm block length, AES-16, TDES-8.

	/*introduced for CAP210 chipset*/
    int kl_index_sel; //! < CE_SEL_KL_0~4, CE_SEL_ETSI_0~4
    int pos;
} CE_DEBUG_KEY_INFO, *p_ce_debug_key_info;

/*! @struct CE_DATA_INFO
 *   @brief Struct used for KL to generate the single level key.
 */
typedef struct ce_data_info
{
    OTP_PARAM otp_info; //!<Load physical OTP root key to KL KEY_0_X.
    DATA_PARAM data_info; //!<Specify the data block information.
    DES_PARAM des_aes_info; //!<Specify the cryption mode, algorithm and result location(TDES only).
    CE_KEY_PARAM key_info;  //!<Specify the source key index, target key index and KL SRAM operation mode.
} CE_DATA_INFO, *p_ce_data_info;

/*! @struct CE_POS_STS_PARAM
 *   @brief Struct used to get the status of the specified key index.
 */
typedef struct ce_pos_status_param
{
    UINT32 pos;  //!<Specify the key index.
    UINT32 status;  //!<Returned status(busy or idle).
} CE_POS_STS_PARAM, *p_ce_pos_sts_param;

/*! @struct CE_FOUND_FREE_POS_PARAM
 *   @brief Struct used to get the idle key index from KL key table.
 */
typedef struct ce_found_free_pos_param
{
    UINT32 pos;  //!<Idle index returned from key table.
    enum ce_key_level ce_key_level;  //!<Specify the initial key level of KL.
    UINT8 number;  //!<Specify the key number that caller wants to get, which is usually 1 or 2, with default of 1.
    enum CE_OTP_KEY_SEL root; //!<Specify root key index, and driver will return relevant idle key index.
} CE_FOUND_FREE_POS_PARAM, *p_ce_found_free_pos_param;

/*! @struct CE_DECW_KEY_PARAM
 *   @brief Struct used to send the decw key for VSC dedicated key ladder.
 */
typedef struct ce_decw_key_param
{
	char en_key[16]; //!<Specify the encrypted key.
	CE_DATA_INFO ce_data_info; //!<Specify the key ladder generate key information.
}CE_DECW_KEY_PARAM, *pCE_DECW_KEY_PARAM;

/*! @struct CE_REC_KEY_PARAM
 *   @brief Struct used to send the encrypted key for pvr record on VSC dedicated key ladder.
 */
typedef struct ce_rec_key_param
{
	char en_key[16]; //!<specify the encrypted key for pvr.
	unsigned int pos; //!<Specify the key index.
}CE_REC_KEY_PARAM, *pCE_REC_KEY_PARAM;

/*! @struct CE_UK_KEY_PARAM
 *   @brief Struct used to send the encrypted key for UK on VSC dedicated key ladder.
 */
typedef struct ce_uk_key_param
{
	char en_key[48]; //!<specify the encrypted key for UK.
	unsigned int pos; //!<Specify the key index.
}CE_UK_KEY_PARAM, *pCE_UK_KEY_PARAM;

/*! @struct CE_NLEVEL_PARAM.
 *   @brief Define the parameters to generate all the levels in one call to ensure the thread-safety.
 *
 * 	Max. 5 levels.
 */
typedef struct ce_generate_nlevel
{
	UINT32 kl_index;
	/*!<Specify the KL index.  #ALI_KL_0  ~ #ALI_KL_3.
	or the ETSI KL index, #ETSI_KL_0  ~ #ETSI_KL_3.
	*/
	UINT32 otp_addr;
	/*!<Specify the root otp key address which will be used for this KeyLadder. 
		#OTP_ADDESS_1  ~ #OTP_ADDESS_6, or 0.
		If otp_addr is 0, driver will not load the otp root key.
	*/
	int algo;
	/*!<Specify the KeyLadder algorithm.
		#CE_SELECT_AES or #CE_SELECT_DES
	*/
	int crypto_mode;
	/*!<#CE_IS_DECRYPT or #CE_IS_ENCRYPT
	*/
	UINT32 pos;
	/*!<Specify the target key position.
	*/
	UINT8 protecting_key[64];
	/*!<Buffer for the protecting keys.
	*/
	UINT32 protecting_key_num; 
	/*!<Number of the protecting keys, 0, 1, 2 or 4. (Shall equal to the KL level subtract 1).
	*/
	UINT8 content_key[16];
	/*!<If the algo is #CE_SELECT_DES, user needs to copy the 64-bit even key to
		 &content_key[8] and copy the 64-bit odd key to &content_key[0].
		If the algo is #CE_SELECT_AES, copy the 128-bit key to &content_key[0].
	*/
	int parity;
	/*!<Because one key position is 128-bit width and the #CE_SELECT_DES 
		block length is 64-bit, so when using #CE_SELECT_DES, user needs to
		specify the target position within this key. Valid values:
		#CE_PARITY_EVEN, #CE_PARITY_ODD or #CE_PARITY_EVEN_ODD.

		When using #CE_SELECT_DES to generate key for DSC AES, TDES or
		CSA3 (algo's key length is 128-bit), the 'parity' must be set to 
		#CE_PARITY_EVEN_ODD and user shall set 128-bit content key to the
		 'content_key' buffer.

		When using #CE_SELECT_AES, this parameter can be ignored.
	*/
} CE_NLEVEL_PARAM;


/*! @struct CE_MIDDLE_LEVEL_PARAM.
 *   @brief Define the parameters to generate the middle level key, used by CE_NLEVEL_ADVANCED_PARAM.
 */
typedef struct ce_middle_level
{
	int algo;
	/*!<Specify the KeyLadder algorithm.
		#CE_SELECT_AES or #CE_SELECT_DES
	*/
	int crypto_mode;
	/*!<#CE_IS_DECRYPT or #CE_IS_ENCRYPT
	*/
	UINT8 protecting_key[16];
	/*!<Buffer for the protecting key of the current level.
	*/
}CE_MIDDLE_LEVEL_PARAM;

/*! @struct CE_CW_LEVEL_PARAM.
 *   @brief Define the parameters to generate the last cw, used by CE_NLEVEL_ADVANCED_PARAM.
 */
typedef struct ce_cw_level
{
	int algo;
	/*!<Specify the KeyLadder algorithm.
		#CE_SELECT_AES or #CE_SELECT_DES
	*/
	int crypto_mode;
	/*!<#CE_IS_DECRYPT or #CE_IS_ENCRYPT
	*/
	UINT32 pos;
	/*!<Specify the target key position(cw pos). if you gen 128bit odd key, 
		you need set this pos to odd_pos(odd_pos = even_pos + 1)
	*/
	UINT8 content_key[16];
	/*!<If the algo is #CE_SELECT_DES, user needs to copy the 64-bit even key to
		 &content_key[8] and copy the 64-bit odd key to &content_key[0].
		If the algo is #CE_SELECT_AES, copy the 128-bit key to &content_key[0].
	*/
	int parity;
	/*!<only for #CE_SELECT_DES.
	
		Because one key position is 128-bit width and the #CE_SELECT_DES 
		block length is 64-bit, so when using #CE_SELECT_DES, user needs to
		specify the target position within this key. Valid values:
		#CE_PARITY_EVEN, #CE_PARITY_ODD or #CE_PARITY_EVEN_ODD.

		When using #CE_SELECT_DES to generate key for DSC AES, TDES or
		CSA3 (algo's key length is 128-bit), the 'parity' must be set to 
		#CE_PARITY_EVEN_ODD and user shall set 128-bit content key to the
		 'content_key' buffer.
	*/
}CE_CW_LEVEL_PARAM;

/*! @struct CE_NLEVEL_PARAM.
 *   @brief Define the parameters to generate all the levels in one call to ensure the thread-safety.
 *
 * 	Max. 5 levels.
 */
typedef struct ce_generate_nlevel_advanced
{
	UINT32 kl_index;
	/*!<Specify the KL index.  #ALI_KL_0  ~ #ALI_KL_3.
	or the ETSI KL index, #ETSI_KL_0  ~ #ETSI_KL_3.
	*/
	UINT32 otp_addr;
	/*!<Specify the root otp key address which will be used for this KeyLadder. 
		#OTP_ADDESS_1  ~ #OTP_ADDESS_6, or 0.
		If otp_addr is 0, driver will not load the otp root key.
	*/	
	UINT32 kl_level;
	/*!<Number of key derivation to process (1,2, 3 or 5).
	*/
	CE_MIDDLE_LEVEL_PARAM middle_level[4];
	/*!<Specify the middle level paramter, when generate 1 level key(level=1), it will not be used.
	*/
	CE_CW_LEVEL_PARAM cw_level;
	/*!<Specify the last cw level paramter.
	*/
} CE_NLEVEL_ADVANCED_PARAM;



/*! @struct CE_CW_DERIVATION.
 *   @brief Define the parameters to derive next stage CW using CW.
 *   CW -> CW (#IO_CRYPT_CW_DERIVE_CW)
 */
typedef struct ce_cw_derivation
{
	/**
	 * Derivation algorithm, #CE_SELECT_AES
	 * or #CE_SELECT_XOR.
	 */
	int algo;
	/**
	 * #CE_IS_DECRYPT or #CE_IS_ENCRYPT
	 */
	int crypto_mode;
	/**
	 * Specify the key from where will be used for derivation. 
	 * KL SRAM (#CE_KEY_FROM_SRAM) or buffer 
	 * in DRAM (#CE_KEY_FROM_CPU).
	 */
	int key_src;
	/**
	 * Key is from KL SRAM or data buffer.
	 * key.pos is associated with the KL key SRAM,
	 * key.buf is the buffer contains the key.
	 */
	union {
		int pos;
		unsigned char buf[16];
	} key;
	/**
	 * Specify the data from where will be decrypted. 
	 * KL SRAM (#CE_DATA_IN_FROM_SRAM) or buffer 
	 * in DRAM (#CE_DATA_IN_FROM_CPU).
	 */
	int data_src;
	/**
	 * Data is from KL SRAM or data buffer.
	 * data.pos is associated with the KL SRAM
	 *
	 * data.buf is the buffer for ECW.
	 */
	union {
		int pos;
		unsigned char buf[16];
	} data;
	/**
	 * Target KL SRAM pos.
	 */
	int target_pos;
} CE_CW_DERIVATION;


/*! @struct CE_ETSI_CHALLENGE.
 *   @brief Define the parameters to perform the ETSI challenge.
 *   (#IO_CRYPT_ETSI_CHALLENGE)
 */
typedef struct ce_etsi_challenge
{
	int kl_index;
	/*!<Specify the ETSI KL index. 
	 * #ETSI_KL_0  ~ #ETSI_KL_4.
	*/
	int otp_addr;
	/*!<Specify the root otp key address which will be used for this KeyLadder. 
		#OTP_ADDESS_1  ~ #OTP_ADDESS_6, or 0.
		If otp_addr is 0, driver will not load the otp root key.
	*/
	int algo;
	/*!<
	 * Challenge algorithm, #CE_SELECT_AES
	 * or #CE_SELECT_TDES.
	 */
	unsigned char ek2[16];
	/*!<
	 * ek3(k2)
	 */
	unsigned char nonce[16];
	/*!<
	 * nonce
	 */
	unsigned char da_nonce[16];
	/*!<
	 * Da(Nonce), output parameter.
	 */
} CE_ETSI_CHALLENGE;

/*! @struct CE_DEVICE
 *   @brief KL device struct reserved on HLD layer.
 */
typedef struct ce_device
{
    struct ce_device  *next;  /*next device */
    INT32 type;
    INT8 name[HLD_MAX_NAME_SIZE];
    void *p_ce_priv;    
    UINT32 base_addr;
    UINT32   interrupt_id;

    ID semaphore_id;
    ID semaphore_id2;

    RET_CODE      (*attach)(void);
    RET_CODE      (*detach)(void);
    RET_CODE    (*open)( struct ce_device *);
    RET_CODE       (*close)( struct ce_device *);
    RET_CODE  (*ioctl)(struct ce_device *, UINT32 , UINT32 );
    RET_CODE (*key_generate)(struct ce_device *, p_ce_data_info );
    RET_CODE (*key_load)(struct ce_device *, p_otp_param);
} CE_DEVICE, *p_ce_device;

typedef struct aes_key_ladder_param
{
    UINT32 key_ladder;      // 1,2,3
    UINT32 root_key_pos;
    UINT8 r[256];           //16 groups key
} AES_KEY_LADDER_BUF_PARAM, *p_aes_key_ladder_buf_param;

/*****************************************************************************
 * Function: hdmi_set_aksv
 * Description:
 *    This function is used to patch the HDMI swith crypto key issue
 * Input:
 *        None
 * Output:
 *        None
 * Returns:
 *        None
 *
*****************************************************************************/
void hdmi_set_aksv(void);

/*****************************************************************************
 * Function: patch_write_bksv2hdmi
 * Description:
 *    This function is used to patch the HDMI swith crypto key issue
 * Input:
 *        None
 * Output:
 *        None
 * Returns:
 *        None
 *
*****************************************************************************/
void patch_write_bksv2hdmi(UINT8 *bksv_data);

/*****************************************************************************
 * Function: ce_api_attach
 * Description:
 *    This function initializes the key ladder hardware and software structures.
 *      This function has to be called once before any other function call of key ladder.
 * Input:
 *      None
 * Output:
 *      None
 *
 * Returns:
 *         0: RET_SUCCESS
 *        1: RET_FAILURE
 *
*****************************************************************************/
RET_CODE ce_api_attach(void);

/*****************************************************************************
 * Function: ce_api_detach
 * Description:
 *    This function is used to terminal key ladder low level driver and release the key ladder occupied resources.
 *    If ce_api_detach is performed, key ladder functions should not be called.
 * Input:
 *      None
 * Output:
 *      None
 *
 * Returns:
 *         0: RET_SUCCESS
 *        1: RET_FAILURE
 *
*****************************************************************************/
RET_CODE ce_api_detach(void);

/*****************************************************************************
 * Function: ce_ioctl
 * Description:
 *    This function is used to implement Key ladder IO control.
 * Input:
 *      p_ce_dev    key ladder device pointer.
 *        cmd            IO control commands defined in above.
 *      param        Parameters pointer.
 * Output:
 *      param
 * Returns:
 *         0: RET_SUCCESS
 *        1: RET_FAILURE
*****************************************************************************/
RET_CODE ce_ioctl(CE_DEVICE *p_ce_dev, UINT32 cmd, UINT32 param);
RET_CODE ce_ioctl_rpc(CE_DEVICE *pcedev, UINT32 cmd, UINT32 param);

/*****************************************************************************
 * Function: ce_key_generate
 * Description:
 *    This function is used to generate the first level key from OTP level.
 *      It will load the OTP key from OTP into Key ladder firstly,
 *      then it will use the OTP key to generate next level key.
 * Input:
 *      p_ce_dev:              key ladder device pointer.
 *        p_ce_data:    key data information pointer
 * Output:
 *      None
 * Returns:
 *         0: RET_SUCCESS
 *        1: RET_FAILURE
*****************************************************************************/
RET_CODE ce_key_generate(CE_DEVICE *p_ce_dev, CE_DATA_INFO *p_ce_data);
RET_CODE ce_key_generate_rpc(CE_DEVICE *pcedev, CE_DATA_INFO *pce_data_info);

/*****************************************************************************
 * Function: ce_key_load
 * Description:
 *    This function is used to load OTP key from OTP into Key Ladder SRAM.
 * Input:
 *      p_ce_dev:              key ladder device pointer.
 *        p_ce_opt_info:        key data information pointer
 * Output:
 *      None
 * Returns:
 *         0: RET_SUCCESS
 *        1: RET_FAILURE
*****************************************************************************/
RET_CODE ce_key_load(CE_DEVICE *p_ce_dev, OTP_PARAM *p_ce_otp_info);
RET_CODE ce_key_load_rpc(CE_DEVICE *pcedev, OTP_PARAM *pce_otp_info);

/*****************************************************************************
 * Function: ce_generate_cw_key
 * Description:
 *    This function is used to generate key from the in_cw_data to second pos using the key in first key pos.
 * Input:
 *      UINT8 *in_cw_data:              key ladder device pointer.
 *        UINT8 mode:                        AES mode or TDES mode
 *        UINT8 first_pos:                the first key position
 *        UINT8 second_pos:                the second key position
 * Output:
 *      None
 * Returns:
 *         0: RET_SUCCESS
 *        1: RET_FAILURE
*****************************************************************************/
RET_CODE ce_generate_cw_key(const UINT8 *in_cw_data, UINT8 mode,
                             UINT8 first_pos, UINT8 second_pos);
RET_CODE ce_generate_cw_key_rpc( const UINT8 *in_cw_data,UINT8 mode,
	                      UINT8 first_pos,UINT8 second_pos);


/*****************************************************************************
 * Function: ce_generate_single_level_key
 * Description:
 *    This function is used to generate single level key to any key level.
 * Input:
 *      p_ce_dev:              key ladder device pointer.
 *        p_ce_data_info:    key data information pointer
 * Output:
 *      None
 * Returns:
 *         0: RET_SUCCESS
 *        1: RET_FAILURE
*****************************************************************************/
RET_CODE ce_generate_single_level_key(CE_DEVICE *p_ce_dev, CE_DATA_INFO *p_ce_data_info);
RET_CODE ce_generate_single_level_key_rpc( CE_DEVICE *p_ce_dev, CE_DATA_INFO *p_ce_data_info );

/*****************************************************************************
 * Function: ce_generate_single_level_key_vsc
 * Description:
 *    This function is dedicated for vsc lib used to generate single level key to any key level.
 * Input:
 *      p_ce_dev:              key ladder device pointer.
 *        p_ce_data_info:    key data information pointer
 * Output:
 *      None
 * Returns:
 *         0: RET_SUCCESS
 *        1: RET_FAILURE
*****************************************************************************/
RET_CODE ce_generate_single_level_key_vsc(CE_DEVICE *p_ce_dev, CE_DATA_INFO *p_ce_data_info);

/*****************************************************************************
 * Function: aes_generate_key_with_multi_keyladder
 * Description:
 *    This function is used to generate multi level key using AES mode.
 * Input:
 *      AES_KEY_LADDER_BUF_PARAM *p_ce_aesparam:   key ladder information pointer.
 * Output:
 *      UINT32 *key_pos:  output key position
 * Returns:
 *         0: RET_SUCCESS
 *        1: RET_FAILURE
*****************************************************************************/
RET_CODE aes_generate_key_with_multi_keyladder( \
        AES_KEY_LADDER_BUF_PARAM *p_ce_aesparam, \
        UINT32 *key_pos);

/*****************************************************************************
 * Function: ce_load_otp_key
 * Description:
 *    This function is used to load OTP key from OTP into Key Ladder SRAM by indicated key pos.
 * Input:
 *      UINT32 key_pos:   first level OTP key position.
 * Output:
 *      The output key position will be fixed in key_pos + 4.
 * Returns:
 *         0: RET_SUCCESS
 *        1: RET_FAILURE
*****************************************************************************/
int ce_load_otp_key(UINT32 key_pos);

/*****************************************************************************
 * Function: ce_key_load
 * Description: 
 *    This function is used to load OTP key from OTP into Key Ladder SRAM by indicated key pos.
 * Input: 
 *      CE_DEVICE *p_ce_dev: the CE deivce pointer
 *      OTP_PARAM *otp_info: OTP key information
 * Output: 
 *      The output key position will be fixed in key_pos + 4.
 * Returns: 
 * 		0: RET_SUCCESS
 *		1: RET_FAILURE
*****************************************************************************/
RET_CODE ce_key_load(CE_DEVICE *p_ce_dev, OTP_PARAM *otp_info);

/*****************************************************************************
 * Function: ce_generate_cw_key
 * Description:
 *    This function is used to generate key from the key to second pos using the key in first key pos with AES.
 * Input:
 *      UINT8 *key:                      key ladder device pointer.
 *        UINT32 ce_crypt_select:            Encrypt mode or Decrypt mode
 *        UINT8 first_pos:                the first key position
 *        UINT8 second_pos:                the second key position
 * Output:
 *      output key in second key position
 * Returns:
 *         0: RET_SUCCESS
 *        1: RET_FAILURE
*****************************************************************************/
int ce_generate_key_by_aes(const UINT8 *key, UINT32 first_key_pos,
                           UINT32 second_key_pos, UINT32 ce_crypt_select);


/*****************************************************************************
 * Function: ce_generate_key_by_tdes
 * Description: 
 *    This function is used to generate single level key using TDES mode.
 *	  If key_pos is zero level key, the target_pos must equal to level_one +4. 
 *	  This function will generate 128 bits key, so the input eck should be 128 bits data
 * Input: 
 *      UINT8 *eck:   				 input data pointer which need to be crypt, the data length should be 128 bits.
        UINT32 key_pos:				 first key position	
        UINT32 target_pos:  second key position
 * Output: 
 *      output key will be stored in Key Ladder SRAM by key position target_pos
 * Returns: 
 * 		0: RET_SUCCESS
 *		1: RET_FAILURE
*****************************************************************************/
RET_CODE ce_generate_key_by_tdes(const UINT8 *eck, UINT32 key_pos,
        UINT32 target_pos);



/*****************************************************************************
 * Function: tdes_decrypt_key_to_ce_64bit
 * Description:
 *    This function is used to generate 64bits key by using TDES mode.
 *      If first_key is zero level key, the target must equal to first_key +4.
 *      This function will generate 64 bits key, so the input eck should be 64 bits data
 * Input:
 *      UINT8 *eck:                        input data pointer which need to be crypt, the data length should be 128 bits.
         UINT8 first_key:                 first key position
         enum CE_CRYPT_TARGET target:  second key position
         UINT8 hilo_addr:                decrypt eck to the target position high 64bits or low 64 bit
 * Output:
 *      output key will be stored in Key Ladder SRAM by key position "target"
 * Returns:
 *         0: RET_SUCCESS
 *        1: RET_FAILURE
*****************************************************************************/
RET_CODE tdes_decrypt_key_to_ce_64bit(UINT8 *eck, UINT8 first_key, enum CE_CRYPT_TARGET target, UINT8 hilo_addr);


/*****************************************************************************
 * Function: tdes_decrypt_key_to_ce_one_level
 * Description:
 *    This function is used to generate single level key using TDES mode.
 *      If level_one is zero level key, the level_gen must equal to level_one +4.
 *      This function will generate 128 bits key, so the input eck should be 128 bits data
 * Input:
 *      UINT8 *eck:                        input data pointer which need to be crypt, the data length should be 128 bits.
         UINT8 level_one:                 first key position
         enum CE_CRYPT_TARGET level_gen:  second key position
 * Output:
 *      output key will be stored in Key Ladder SRAM by key position level_gen
 * Returns:
 *         0: RET_SUCCESS
 *        1: RET_FAILURE
*****************************************************************************/
RET_CODE tdes_decrypt_key_to_ce_one_level(UINT8 *eck, UINT8 level_one,
        enum CE_CRYPT_TARGET level_gen);


/*

input -> data to be decrypt
level_root -> root pos in CE(OTP_KEY_0_0 OTP_KEY_0_1 OTP_KEY_0_2 OTP_KEY_0_3)
key_pos -> level_two key_pos in CE, will return to caller for other use
after used the key_pos, need to set idle: ce_ioctl(pCeDev, IO_CRYPT_POS_SET_IDLE, key_pos_param.pos);
*/

/*****************************************************************************
 * Function: tdes_decrypt_to_ce_two_level
 * Description:
 *    This function is used to generate two level key using TDES mode.
 *      TDES 128bit decrypt 16 + 16 byte data to CE
 *      If level_root must be zero level key or first level key position.
 *      This function will generate 128 bits key using two level key, so the input eck should be 128*2 bits data
 * Input:
 *      UINT8 *input:                    input data pointer which need crypt, the data length should be 128*2 bits.
         UINT8 level_root:                 first key position
 * Output:
 *        UINT8 *key_pos:        The output key position in Key Ladder SRAM
 *      output key will be stored in Key Ladder SRAM by key position key_pos.
 * Returns:
 *         0: RET_SUCCESS
 *        1: RET_FAILURE
*****************************************************************************/
RET_CODE tdes_decrypt_to_ce_two_level(UINT8 *input, UINT8 level_root, UINT32 *key_pos);


/*****************************************************************************
 * Function: tdes_decrypt_setup_kl_three_level
 * Description:
 * TDES 128bit decrypt 3 level (3x16Bytes) data to key ladder's secure SRAM
 *
 * Input:
 *      UINT8 *input:  input data pointer which need crypt, the data length should be 128*3 bits.
 *      UINT8 root_pos:     root key position
 * Output:
 *    UINT32 *key_pos:     Output an idle key position in Key Ladder SRAM
 *      output key will be stored in Key Ladder SRAM by key position key_pos.
 *    If this key_pos won't be used anymore, need to set it idle:
 *       ce_ioctl(pCeDev, IO_CRYPT_POS_SET_IDLE, key_pos_param.pos);
 * Returns:
 *         0: RET_SUCCESS
 *        1: RET_FAILURE
*****************************************************************************/
RET_CODE tdes_decrypt_setup_kl_three_level(UINT8 *input,UINT8 root_pos,UINT32 *key_pos);


/*****************************************************************************
 * Function: aes_decrypt_setup_kl_three_level
 * Description:
 * AES 128bit decrypt 3 level (3x16Bytes) data to key ladder's secure SRAM
 *
 * Input:
 *      UINT8 *input:                    input data pointer which need crypt, the data length should be 128*3 bits.
 *      UINT8 root_pos:                 root key position
 * Output:
 *        UINT32 *key_pos:        Output an idle key position in Key Ladder SRAM
 *       output key will be stored in Key Ladder SRAM by key position key_pos. If this key_pos won't be used anymore,
 *    need to set it idle:
 *                       ce_ioctl(pCeDev, IO_CRYPT_POS_SET_IDLE, key_pos_param.pos);
 * Returns:
 *         0: RET_SUCCESS
 *        1: RET_FAILURE
*****************************************************************************/
RET_CODE aes_decrypt_setup_kl_three_level(UINT8 *input,UINT8 root_pos,UINT32 *key_pos);


/*****************************************************************************
 * Function: ce_generate_hdcp_key
 * Description:
 *    This function is used to generate HDCP Key.
 *    Use AES decrypt (for decrypt hdcp key), CE lib
 *
 * Input:
 *      p_ce_dev:              key ladder device pointer.
 *      en_hdcp_key:
 *      len:
 * Output:
 *      None
 * Returns:
 *         0: RET_SUCCESS
 *        1: RET_FAILURE
*****************************************************************************/
RET_CODE ce_generate_hdcp_key(p_ce_device p_ce_dev, UINT8 *en_hdcp_key, UINT16 len);

/*****************************************************************************
 * Just define below functions for compiling, user no need to use these symbols.
 *
*****************************************************************************/

UINT32 ce_set_clear_middle_key_enable(CE_DEVICE *p_ce_dev, UINT8 enable);
UINT32 ce_get_clear_middle_key_enable(CE_DEVICE *p_ce_dev);


void hld_crypto_callee(UINT8 *msg);
void lld_crypto_m36f_callee( UINT8 *msg );

#ifdef __cplusplus
}
#endif

/*!
@}
@}
*/

#endif  /*_CRYPTO_H_*/
