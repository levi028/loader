#ifndef _CF_H
#define _CF_H

#include <basic_types.h>
#include <hld/hld_dev.h>

/*!
 *	 @brief Define the return-value for CF 
*/
#define CF_NO_ERROR (0)
#define CF_ERROR (0x1000)
#define CF_ERROR_NODEV (0x1001)
#define CF_ERROR_INVALID_PARAM (0x1002)
#define CF_ERROR_TIMEOUT (0x1004)
#define CF_ERROR_BUSY (0x1008)
#define CF_ERROR_IO (0x1010)
#define CF_ERROR_STATE (0x1020)
#define CF_ERROR_INVALID_CHAN (0x1040)
#define CF_ERROR_NOT_SUPPORTED (0x1080)

/**
 * \ingroup cf_defs
 * \brief Default CF operational command values
 */
typedef enum cfDefaultOpValues
{
    CF_DEFAULT_OP_PERSO_CMD               = 0x000029FF,    /**< Default Perso command. */
    CF_DEFAULT_OP_MFR_TEST_CMD            = 0x000089FF,    /**< Default Manufacturer Test command. */
    CF_DEFAULT_OP_DIFF_CMD                = 0x000149FF,    /**< Default Differentiation command. */
    CF_DEFAULT_OP_FEATURE_CMD             = 0x000169FF,    /**< Default Feature command. */
    CF_DEFAULT_OP_CWC_CMD                 = 0x000000FF,	
} CF_DEFAULT_OP_VALUES;

/**
 * \ingroup cf_defs
 * \brief Valid transaction status field values
 */
#define CF_TRANS_STATUS_BUSY_INIT (0x0) /**< Reset from power on condition. */
#define CF_TRANS_STATUS_BUSY_SETUP (0x1)  /**< Reset after a command has completed. */
#define CF_TRANS_STATUS_READY (0x2)  /**< CF ready to receive a new transaction. */
#define CF_TRANS_STATUS_INPUT (0x3)  /**< CF is receiving a new transaction. */
#define CF_TRANS_STATUS_BUSY_OP (0x4) /**< CF is processing transaction. */
#define CF_TRANS_STATUS_DONE_OK (0x8) /**< Transaction was completed successfully. */
#define CF_TRANS_STATUS_DONE_ERROR (0x9)  /**< Transaction has completed with an error. */
#define CF_TRANS_STATUS_DONE_MFR_TEST (0xE)  /**< Special case of successful test unlock. */

/**
 * \ingroup cf_defs
 * \brief Valid NVM status field values
 */
#define CF_NVM_STATUS_READY (0x0) /**< NVM personalized and ready. */
#define CF_NVM_STATUS_UNBISTED (0x1) /**< NVM is unbisted.  Currently unsupported state and should be treated as an error. */
#define CF_NVM_STATUS_UNPERSO (0x2) /**< NVM is pre-personalized. */
#define CF_NVM_STATUS_ERROR (0x3) /**< NVM personalized and ready. */

/**
 * \ingroup cf_defs
 * \brief Valid differentiation status field values
 */
#define CF_DIFF_STATUS_UNDIFFERENTIATED (0x0)    /**< CF is undifferentiated. */
#define CF_DIFF_STATUS_DIFFERENTIATED (0x1)     /**< CF is differentiated. */

/**
 * \ingroup cf_defs
 * \brief Valid operation type request field values
 */
#define CF_OPERATION_TYPE_OP_CWC (0x0) /**< Request an CWC generation operation. */
#define CF_OPERATION_TYPE_OP_PERSO (0x1) /**< Request a Personalization operation. */
#define CF_OPERATION_TYPE_OP_DIFF (0x2) /**< Request a Differentiation operation. */
#define CF_OPERATION_TYPE_OP_FEATURE (0x3) /**< Request a Feature operation. */
#define CF_OPERATION_TYPE_OP_MFR_TEST (0x4) /**< Request a Manufacturer Test operation. */
#define CF_OPERATION_TYPE_OP_UNDEFINED1 (0x5) /**< Reserved operation. */
#define CF_OPERATION_TYPE_OP_UNDEFINED2 (0x6) /**< Reserved operation. */
#define CF_OPERATION_TYPE_OP_UNDEFINED3 (0x7) /**< Reserved operation. */

/**
 * \ingroup cf_defs
 * \brief Valid output usage request field values
 */
#define CF_OUTPUT_USAGE_SHV (0x0) /**< Output a secure hash value. */
#define CF_OUTPUT_USAGE_CWC_DIRECT (0x1) /**< Direct transfer of CWC to key table. */
#define CF_OUTPUT_USAGE_CWC_XOR (0x2) /**< XOR of key table entry with CWC. */
#define CF_OUTPUT_USAGE_CWC_AES_KEY (0x3) /**< 128-bit AES decrypt of key table entry with CWC. */

/**
 * \ingroup cf_defs
 * \brief Valid DECM source request field values
 */
#define CF_DECM_SOURCE_NONE (0x0) /**< Invalid DECM source. Should not be used in normal operation. */
#define CF_DECM_SOURCE_SW (0x1) /**< DECM source from software. */
#define CF_DECM_SOURCE_KEY (0x2) /**< DECM source from key table. */
#define CF_DECM_SOURCE_MIX (0x3) /**< Combine DECM input from software and key table. */

typedef struct ali_cf_version {
    /**
     *  Version Epoch
     */
    unsigned char versionEpoch;
    /**
     *  Manufacturer Identifier
     */
    unsigned char manufacturerId;
    /**
     *  Netlist Version
     */
    unsigned char netlistVersion;
    /**
     *  Build Identifier
     */
    unsigned char versionBuildId; 
} ALI_CF_VERSION;

typedef struct ali_cf_feature {
    /**
     *  Current feature vector
     */
    unsigned int featureVector;
} ALI_CF_FEATURE;

typedef struct ali_cf_cf_status {
    /**
     *  Current status of Non-Volatile Memory
     */
    unsigned char nvmStatus;
    /**
     *  Flag indicating Differentiation status of the Transport CF
     */
    unsigned char differentiationStatus;
    /**
     *  Flag indicating Transport CF was recently reset (ie no transactions have
     *  been issued since reset)
     */
    unsigned char recentReset;
    /**
     *  Flag indicating Transport CF alert has been tripped
     */
    unsigned char cfAlert;
    /**
     *  Flag indicating Transport CF is in development mode
     */
    unsigned char developmentMode;
    /**
     *  Flag indicating state of Transport CF activation fuse
     */
    unsigned char fuseActivate;
    /**
     *  Flag indicating state of Transport CF block fuse
     */
    unsigned char fuseBlock;
}ALI_CF_CF_STATUS;

typedef struct ali_cf_trans_status {   
    /**
     *  Current status of transaction
     */
    unsigned char transactionStatus;
    /**
     *  Enforced value of the useNvmKey flag for the current transaction
     */
    unsigned char useNvmKey;
    /**
     *  Enforced value of the operationType for the current transaction
     */
    unsigned char operationType;
    /**
     *  Enforced value of the decmSource for the current transaction
     */
    unsigned char decmSource;
    /**
     *  Enforced value of the outputUsage for the current transaction
     */
    unsigned char outputUsage;
    /**
     *  Enforced value of the productRange flag for the current transaction
     */
    unsigned char productRange;
    /**
     *  Enforced value of the productOffset for the current transaction
     */
    unsigned char productOffset;
}ALI_CF_TRANS_STATUS;

typedef struct ali_cf_decm_status {
    /**
     *  Flag indicating Transport CF hardware DECM valid input signal is asserted
     */
    unsigned char hwDecmValid;
    /**
     *  Flag indicating Transport CF hardware DECM valid input signal was not
     *  asserted during read of hardware DECM for the current transaction
     */
    unsigned char hwDecmError;
} ALI_CF_DECM_STATUS;

typedef struct ali_cf_operation {
   /**
     * Operation input payload
     */
    unsigned int operation[21];
} ALI_CF_OPERATION;

typedef struct ali_cf_result {
    /**
     *  Enforced value of the operationType for the current transaction
     */
    unsigned char operationType;
    /**
     *  Current status of transaction
     */
    unsigned char transactionStatus;
    /**
     *  Flag indicating Transport CF CWC valid output signal was asserted
     */
    unsigned char cwcValid;
    /**
     *  Resulting secure hash value if operation type was \link #CF_OPERATION_TYPE_OP_CWC
     *  \endlink with output usage set to \link #CF_OUTPUT_USAGE_SHV \endlink, or all zeros
     *  for all other operations. Values read from the \link #CF_REG_ADDR_OUTPUT \endlink
     *  register.
     */
    unsigned int shv[4];
}ALI_CF_RESULT;

/*
 * Ioctl definitions
 */
#define ALI_CF_IOC_VERSION_INFO       0
#define ALI_CF_IOC_FEATURE_VECTOR      1
#define ALI_CF_IOC_CF_STATUS           2
#define ALI_CF_IOC_TRANS_STATUS        3
#define ALI_CF_IOC_DECM_STATUS        4
#define ALI_CF_IOC_ISSUE_OP_FEATURE     5
#define ALI_CF_IOC_ISSUE_OP_DIFF      6
#define ALI_CF_IOC_ISSUE_OP_CWCORSHV    7
#define ALI_CF_IOC_READ_OP_RESULT     9
#define ALI_CF_IOC_WAIT_OP_DONE     10

struct cf_dev
{
	struct cf_dev *next;
	int type;
	char name[HLD_MAX_NAME_SIZE];
	void *priv;
	int  (*open)(void);
	int  (*close)(int chan);
	int  (*ioctl)(int chan, int cmd, void *param);
	int  (*write)(int chan, UINT8 *, int );
	int  (*read)(int chan, UINT8 *, int );
	int  (*set_target)(int );
};

int cf_api_attach(void);
int cf_api_detach(void);

int cf_open(void);
int cf_close(int chan);
int cf_ioctl(int chan, int cmd, void *param);
int cf_write(int chan, UINT8 *buf, int count);
int cf_read(int chan, UINT8 *buf, int count);
int cf_set_target(int pos);
void hld_cf_callee(UINT8 *msg);

#endif /* _CF_H_ */

