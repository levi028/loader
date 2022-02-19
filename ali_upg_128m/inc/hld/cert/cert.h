/*****************************************************************************
*	 Copyright (c) 2013 ALi Corp. All Rights Reserved
*	 This source is confidential and is ALi's proprietary information.
*	 This source is subject to ALi License Agreement, and shall not be 
	 disclosed to unauthorized individual.	  
*	 File: cert.h
*	
*	 Description: this file is used to define some macros and structures 
*				  for CERT ASA/AKL, Youri - 2013/12/26 
*	 THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
	  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
	  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
	  PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _CERT_H
#define _CERT_H

#ifdef __cplusplus
extern "C"
{
#endif

#define CERT_AKL_DATA_SIZE  (32)
/*!< CERT AKL cmd size in bytes (32 bytes).*/

#define CERT_IO_CMD(cmd)  (cmd & 0xFF) 
/*!< Mask the cmd to 8 bits, only 8 bits valid for CERT see ioctl cmd.*/

#define ASA (0x10)
/*!< CERT ASA algorithm ID.*/

/*!
 *	 @brief Define the return-value for CERT 
*/
#define CERT_SUCCESS (0)
#define CERT_ERROR_DEFAULT (0x1000)
#define CERT_ERROR_NODEV (0x1001)
#define CERT_ERROR_INVALID_PARAM (0x1002)
#define CERT_ERROR_SAVE_KEY (0x1004)
#define CERT_ERROR_EXTIMEOUT (0x1008)
#define CERT_ERROR_ALWAYS_BUSY (0x1010)
#define CERT_ERROR_PIDFULL (0x1020)
#define CERT_ERROR_CHANFULL (0x1040)
#define CERT_ERROR_INVALID_CHAN (0x1080)
#define CERT_ERROR_SYNC_BYTE (0x1100)
#define CERT_ERROR_NOT_SUPPORTED (0x1200)
#define CERT_ERROR_NOT_ALLOWED (0x1400)
#define CERT_ERROR_EXCHANGE_STATUS (0x1800)

/*! @struct cert_asa_node
 *	 @brief Define elements for one ASA node.
*/
struct cert_asa_node {
	unsigned char *input; /* pointer to the input data */
	unsigned char *output; /* pointer to the output data, can be same as input */
	unsigned int size; /* TS data size in bytes */
};

/*!
     @brief Define packet format
*/
#define CERT_ASA_TS188		(1)
	/*!< DVB-MPEG2 TS packet, 188 Bytes */
#define CERT_ASA_TS188_LTSID (2)
	/*!< DVB-MPEG2 TS packet using LTSID replace Sync Byte, 188 Bytes */
#define CERT_ASA_TS200		(3)
	/*!< CCIF2.0 TS packet, 200 Bytes */

/*!
 * @brief Define the AKL key parity for saving key to the corresponding key position.
*/
#define CERT_AKL_ODD_PARITY (1)
/*!< copy the key to even position.*/
#define CERT_AKL_EVEN_PARITY (2)
/*!< copy the key to odd position.*/

/*!
 * @brief Define the AKL key usage.
*/
#define CERT_AKL_FOR_AES (0)
/*!< Specify that the AKL key is for AES.*/
#define CERT_AKL_FOR_TDES (1)
/*!< Specify that the AKL key is for TDES.*/
#define CERT_AKL_FOR_ASA	(8)
/*!< Specify that the AKL key is for ASA.*/
#define CERT_AKL_FOR_CSA3 (9)
/*!< Specify that the AKL key is for CSA3.*/
#define CERT_AKL_FOR_CSA2 (10)
/*!< Specify that the AKL key is for CSA2/CSA1.1.*/

/*! @struct cert_akl_savekey
 *	 @brief Define the algorithm and parity information for storing the AKL key.
*/
struct cert_akl_savekey
{
	int pos;
	/*!< Specify the destination position which will be
	used for saving this key.*/
	int algo;
	/*!< Specify which crypto algorithm will use this AKL key,
	algorithms are #CERT_AKL_FOR_AES, #CERT_AKL_FOR_TDES,
	#CERT_AKL_FOR_ASA, #CERT_AKL_FOR_CSA3 and #CERT_AKL_FOR_CSA2.
	*/
	int parity;
	/*!< Specify the key parity #CERT_AKL_EVEN_PARITY or
	#CERT_AKL_ODD_PARITY to be saved to.*/
};
 

/*! @struct cert_akl_cmd
 *	 @brief Define the parameters for exchanging data with AKL. @~
 */
struct cert_akl_cmd
{
	unsigned char data_in[CERT_AKL_DATA_SIZE];
	/*!< Input command to exchange with AKL.*/ 
	unsigned char data_out[CERT_AKL_DATA_SIZE];
	/*!< Output data of AKL.*/ 
	unsigned char status[4];
	/*!< Output status of AKL.*/ 
	unsigned char opcodes[4];
	/*!< Opcodes of AKL.*/ 
	unsigned int timeout;
	/*!< Timeout type, default is 0.*/ 
};

/*! @struct cert_asa_pid 
 *	 @brief Define the parameters for adding the ASA PID and key information. @~
 */
struct cert_asa_pid
{
	int pos;
	/*!< Specify which key pos will be used for this PID(or PIDs). */ 
	unsigned char ltsid;
	/*!< Specify the LTSID to be added/removed, only valid when 
	TS200 or TS188 with LTSID mode, normal TS188 doesn't need this parameter. */ 
	unsigned char tsc;
	/*!< Specify the Transport Scramble Control value after descrambling, 0~3*/ 
	unsigned short pid; 
	/*!< Specify the PID value.
	*/ 
};

struct cert_akl_dev
{
	struct cert_akl_dev *next;
	int type;
	char name[16];
	void *priv;
	int (*open)(void);
	int (*close)(int sess);
	int (*exchange)(int sess, struct cert_akl_cmd *param);
	int (*savekey)(int sess, struct cert_akl_savekey *param);
	int (*ack)(int sess);
};

struct cert_asa_dev
{
	struct cert_asa_dev *next;
	int type;
	char name[16];
	void *priv;
	int (*open)(void);
	int (*close)(int sess);
	int (*setfmt)(int sess, int fmt);
	int (*addpid)(int sess, struct cert_asa_pid *param);
	int (*delpid)(int sess, struct cert_asa_pid *param);
	int (*decrypt)(int sess, UINT8 *input, UINT8 *output, UINT32 length);
	int (*decrypt_nodes)(int sess, void *buf, UINT32 count);
};

int cert_asa_attach(void);
int cert_asa_detach(void);

int cert_asa_open(void);
int cert_asa_close(int sess);
int cert_asa_set_format(int sess, int format);
int cert_asa_add_pid(int sess, struct cert_asa_pid *param);
int cert_asa_del_pid(int sess, struct cert_asa_pid *param);

/*****************************************************************************
 * Function: cert_asa_decrypt
 * Description:
 *    This function is used to decrypt the TS data using CERT ASA
 * Input:
 *      int sess:        The ASA session handler
 *      UINT8 *input:   The input buffer pointer
 *      UINT32 length:  The data length in byte
 * Output:
 *        UINT8 *output:  the output buffer pointer
 * Returns:
 *        0: Success
 *        Others: Fail
*****************************************************************************/
int cert_asa_decrypt(int sess, UINT8 *input, UINT8 *output, UINT32 length);

int cert_akl_attach(void);
int cert_akl_detach(void);

int cert_akl_open(void);
int cert_akl_close(int sess);
int cert_akl_exchange(int sess, struct cert_akl_cmd *param);
int cert_akl_save_key(int sess, struct cert_akl_savekey *param);
int cert_akl_ack(int sess);

#ifdef __cplusplus
}
#endif

#endif

