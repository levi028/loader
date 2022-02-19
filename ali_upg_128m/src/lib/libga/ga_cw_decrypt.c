
#include <basic_types.h>
#include <api/libc/string.h>

#include <hld/hld_dev.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/dsc/dsc.h>
#include <hld/crypto/crypto.h>

#include <api/libga/lib_ga.h>

#define MAX_TS_NUM      4

typedef struct
{
    UINT8   used;
    UINT16  pid;
    UINT32  pid_list_len;
    UINT32  stream_id;
    UINT32  key_pos;
    UINT8   type;       // 0:video 1:audio
    UINT32  handle;
}GACAS_TS_INFO_T;

static UINT32 g_gacas_stream_id = INVALID_DSC_STREAM_ID;

static GACAS_TS_INFO_T g_ts_info[MAX_TS_NUM];
static OSAL_ID   g_gacas_key_sema_id = OSAL_INVALID_ID;
static UINT8    g_gacas_rootkey_idx = 0;
static UINT8   g_gacas_key1[16] = {0};
static UINT8   g_gacas_key1_type = 0;
static UINT8   g_gacas_key2[16] = {0};
static UINT8   g_gacas_key2_type = 0;
static UINT8    g_gacas_cw_type = 0;
static UINT8    g_gacas_enc_cw[16] = {0};

//Ben 180619#2
UINT32 get_ca_stream_id(void)
{
	return g_gacas_stream_id;
}
//
// CAS device initialization:
// Step 1: get a free stream from DSC, which will return a valid stream id
// Step 2: CSA Init
// Step 3: DMX IO control 
//              Set CSA handle to DMX
//              Set Dec status
// Step 4: DSC IO control
//              set dsc stream id
// Param:  KeyMode (Input): 1 - key is from crypto engine
//                          2 - key is from ram

int gacas_device_init(UINT32 KeyMode)
{
	CSA_INIT_PARAM csa_param ;
	UINT32 ret = RET_SUCCESS;
    
	if (INVALID_DSC_STREAM_ID == g_gacas_stream_id)
		g_gacas_stream_id = dsc_get_free_stream_id(TS_MODE);
	if (INVALID_DSC_STREAM_ID == g_gacas_stream_id)
	{
		ASSERT(0);
		return -1;
	}

	p_csa_dev pCsaDev = NULL;
	struct dmx_device *dmx_dev = NULL;
	struct dec_parse_param param;

	MEMSET(&csa_param, 0, sizeof(CSA_INIT_PARAM));

    	pCsaDev = (p_csa_dev)dev_get_by_id(HLD_DEV_TYPE_CSA, 0);

	csa_param.dcw[0] = 0 ;
	csa_param.dcw[1] = 1 ;
	csa_param.dcw[2] = 2 ;
	csa_param.dcw[3] = 3 ;
	csa_param.dma_mode = TS_MODE;
	if (1 == KeyMode)
		csa_param.key_from = KEY_FROM_CRYPTO;
	else
		csa_param.key_from = KEY_FROM_SRAM;
	csa_param.parity_mode = AUTO_PARITY_MODE0;
	csa_param.pes_en = 1;
	csa_param.scramble_control = 0 ; 			/*dont used default CW*/
	csa_param.stream_id = g_gacas_stream_id;
	csa_param.version = CSA2 ;
    	ret = csa_ioctl(pCsaDev, IO_INIT_CMD, (UINT32)&csa_param);
	if (RET_SUCCESS != ret)
	{
		ASSERT(0);
		return RET_FAILURE;
	}

	dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);
	p_dsc_dev pDscDev = (p_dsc_dev)dev_get_by_id(HLD_DEV_TYPE_DSC, 0);

	MEMSET(&param, 0, sizeof(struct dec_parse_param));
	param.dec_dev = pCsaDev;
	param.type = CSA;

   	ret = dmx_io_control(dmx_dev, IO_SET_DEC_HANDLE, (UINT32)&param);
	if (RET_SUCCESS != ret)
	{
		// CSA IO_SET_DEC_HANDLE failed!
		ASSERT(0);
		return RET_FAILURE;
	}

    	ret = dmx_io_control(dmx_dev, IO_SET_DEC_STATUS, 1);
	if (RET_SUCCESS != ret)
	{
		// CSA IO_SET_DEC_STATUS failed!
		ASSERT(0);
		return RET_FAILURE;
	}

	ret = dsc_ioctl(pDscDev, IO_PARSE_DMX_ID_SET_CMD, g_gacas_stream_id);
	if(ret != RET_SUCCESS)
	{
		// CSA IO_PARSE_DMX_ID_SET_CMD fail!
		ASSERT(0);

		return RET_FAILURE;
	} 
    	return RET_SUCCESS;
}


// Delete the stream in CSA, and set the key pos in CE to idle status
// the resource of CSA and CE were used for the specified pid
int gacas_delete_stream_from_ce(UINT16 pid)
{
	p_csa_dev pCsaDev = NULL;
	p_ce_device ce_dev = NULL;
	int i = 0;
	RET_CODE ret = RET_FAILURE;

	pCsaDev = (p_csa_dev)dev_get_by_id(HLD_DEV_TYPE_CSA, 0);
	ce_dev = (p_ce_device)dev_get_by_type(NULL, HLD_DEV_TYPE_CE);

	if (NULL == pCsaDev || NULL == ce_dev || 0x1fff == pid || 0 == pid)
	{
		// Delete stream from ce failed!
		return -1;
	}

	for (i = 0; i < MAX_TS_NUM; i++)
	{
		if ((pid == g_ts_info[i].pid) && (1 == g_ts_info[i].used))
			break;
	}
	if (i >= MAX_TS_NUM)
	{
		// there is no key in list, no need to delete!
		return -1;
	}

	ret = csa_ioctl(pCsaDev, IO_DELETE_CRYPT_STREAM_CMD, g_ts_info[i].handle);
	ce_ioctl(ce_dev, IO_CRYPT_POS_SET_IDLE, g_ts_info[i].key_pos);

	if (RET_SUCCESS != ret)
	{
		// CE IO_CRYPT_POS_SET_IDLE failed!
		MEMSET(&g_ts_info[i], 0, sizeof(GACAS_TS_INFO_T));
		return -1;
	}
	MEMSET(&g_ts_info[i], 0, sizeof(GACAS_TS_INFO_T));

	return 0;
}


// CAS create stream to Cypto Engine
// Step 1:  find a free key pos to 2 level keyladder from Cypto Engine
// Step 2:  involve PID, KeyPos, stream id together; Create such relation into CSA
//          (So the CSA will know: by using which stream , keyladder and key pos, to decrypt which PID)
// Param:   pid     (Input): the TS data pid which need to be decrypted
//          type    (Input): TS data type, VIDEO_DSC or AUDIO_DSC
int gacas_create_stream_to_ce(UINT16 pid, UINT8 type)
{
    p_ce_device ce_dev = NULL;
    p_csa_dev pCsaDev = NULL;
    int i;
    UINT8 root = 0;
    CE_FOUND_FREE_POS_PARAM ce_param;
    KEY_PARAM   key_param;
    RET_CODE ret = RET_FAILURE;

    ce_dev = (p_ce_device)dev_get_by_type(NULL, HLD_DEV_TYPE_CE);
    pCsaDev = (p_csa_dev)dev_get_by_id(HLD_DEV_TYPE_CSA, 0);

    if (NULL == pCsaDev || pid == 0 || (INVALID_PID == pid))
    {
        // pCsaDev is NULL or pid Invalid!
        return RET_FAILURE;
    }

    for (i = 0; i < MAX_TS_NUM; i++)
    {
        if ((1 == g_ts_info[i].used) && (pid == g_ts_info[i].pid))
        {
            // create_stream_to_ce failed, pid already exist!
            return RET_FAILURE;
        }
    }

    for (i = 0; i < MAX_TS_NUM; i++)
    {
        if (0 == g_ts_info[i].used)
            break;
    }
    if (i >= MAX_TS_NUM)
    {
        // no more CE stream resource!
        ASSERT(0);
        return RET_FAILURE;
    }

    // because we use DVB_CSA2_RootKey, so we set root to KEY_0_0
    root = KEY_0_0;

    // Find a key_pos
    MEMSET(&ce_param, 0, sizeof(CE_FOUND_FREE_POS_PARAM));
    ce_param.ce_key_level = THREE_LEVEL;
    ce_param.pos = INVALID_CE_KEY_POS;
    ce_param.number = 1;
    ce_param.root = root;
    ce_ioctl(ce_dev, IO_CRYPT_FOUND_FREE_POS, &ce_param);

    // Record in a global array g_ts_info[]
    g_ts_info[i].pid = pid;
	g_ts_info[i].pid_list_len = 1;	//Ben 180619#1
    g_ts_info[i].used = 1;
    g_ts_info[i].stream_id = g_gacas_stream_id;
    g_ts_info[i].key_pos = ce_param.pos;
    g_ts_info[i].type = type;

    // Create stream
    MEMSET(&key_param, 0, sizeof(KEY_PARAM));
    key_param.handle = 0xFF;
    key_param.ctr_counter = NULL;
    key_param.init_vector = NULL;
    key_param.key_length = 64;//128;
    key_param.pid_len = g_ts_info[i].pid_list_len;
    key_param.pid_list = &g_ts_info[i].pid;
    key_param.p_aes_iv_info = NULL;
    key_param.stream_id = g_ts_info[i].stream_id;
    key_param.key_from = KEY_FROM_CRYPTO;
    key_param.pos = g_ts_info[i].key_pos & 0xFF;
    key_param.kl_sel = (g_ts_info[i].key_pos >> 8) & 0x0F;
    ret = csa_ioctl(pCsaDev, IO_CREAT_CRYPT_STREAM_CMD, (UINT32)&key_param);
    if (RET_SUCCESS != ret)
    {
        // CSA create stream failed!
    	 ce_ioctl(ce_dev, IO_CRYPT_POS_SET_IDLE, g_ts_info[i].key_pos);
	 MEMSET(&g_ts_info[i], 0, sizeof(GACAS_TS_INFO_T));
        return RET_FAILURE;
    }

    g_ts_info[i].handle = key_param.handle;

    return RET_SUCCESS;
}

// Feed Encypted ControlWord to Crypto Engine, by using the specific key_pos of CE
// Notice: before calling this funciton, those global should be configured already!
//          g_cacas_key_sema_id
//          g_gacas_key2_type
//          g_gacas_key2
//          g_gacas_key1_type
//          g_gacas_key1
//          g_gacas_rootkey_idx
//          g_gacas_cw_type
// param:   enc_cw  (Input): the encrypted CW
//          key_pos (Input): key pos found from CE

static int gacas_feed_cw(UINT8 *enc_cw, UINT32 key_pos)
{
    UINT8 key2_algo_type = 0, key1_algo_type = 0, cw_algo_type = 0;
    int ret = -1;
    int kl_sel = ALI_KL_0;
    UINT32 otp_addr = 0;
    CE_NLEVEL_ADVANCED_PARAM nlevel_param;
    p_ce_device p_ce_dev = (p_ce_device)dev_get_by_id(HLD_DEV_TYPE_CE, 0);
    
    osal_semaphore_capture(g_gacas_key_sema_id, TMO_FEVR);

    // Configure key2's decrypt algo (maybe CWPK'')
    if (g_gacas_key2_type == AES_TYPE)
        key2_algo_type = CE_SELECT_AES;
    else if (g_gacas_key2_type == DES_TYPE)
        key2_algo_type = CE_SELECT_DES;
    else
    {
        // wrong key type
        goto end;
    }

    // Configure rootkey (which comes from OTP)
    if (g_gacas_rootkey_idx == 0)
    {
        kl_sel = ALI_KL_0;
        otp_addr = OTP_ADDESS_1;
    }
    else if (g_gacas_rootkey_idx == 1)
    {
        kl_sel = ALI_KL_1;
        otp_addr = OTP_ADDESS_2;
    }
    else
    {
        // rootkey idx wrong!
        goto end;
    }

    // Configure key1's decrypt algo
    if (g_gacas_key1_type == AES_TYPE)
    {
        key1_algo_type = CE_SELECT_AES;
    }
    else if (g_gacas_key1_type == DES_TYPE)
    {
        key1_algo_type = CE_SELECT_DES;
    }
    else
    {
        // key1 type wrong!
        goto end;
    }

    // Configure CW decrypt algo
    if (g_gacas_cw_type == AES_TYPE)
    {
        cw_algo_type = CE_SELECT_AES;
    }
    else if (g_gacas_cw_type == DES_TYPE)
    {
        cw_algo_type = CE_SELECT_DES;
    }
    else
    {
        // cw type wrong!
        goto end;
    }

    // Feed key2, key1 and encrypted cw to CE (3 level keyladder mode)
    MEMSET(&nlevel_param, 0, sizeof(CE_NLEVEL_ADVANCED_PARAM));
    nlevel_param.kl_index = kl_sel;
    nlevel_param.otp_addr = otp_addr;
    nlevel_param.kl_level = 3;

    // configure key2, for GSP it may be CWPK''
    nlevel_param.middle_level[0].algo = key2_algo_type;
    nlevel_param.middle_level[0].crypto_mode = CE_IS_DECRYPT;
    MEMCPY(nlevel_param.middle_level[0].protecting_key, g_gacas_key2, 16);

    // configure key1, for GSP it may be CWPK'
    nlevel_param.middle_level[1].algo = key1_algo_type;
    nlevel_param.middle_level[1].crypto_mode = CE_IS_DECRYPT;
    MEMCPY(nlevel_param.middle_level[1].protecting_key, g_gacas_key1, 16);

    // configure cw'
    nlevel_param.cw_level.algo = cw_algo_type;
    nlevel_param.cw_level.crypto_mode = CE_IS_DECRYPT;
    nlevel_param.cw_level.pos = key_pos;
    nlevel_param.cw_level.parity = CE_PARITY_EVEN_ODD;
    MEMCPY(&(nlevel_param.cw_level.content_key[8]), &enc_cw[8], 8);
    MEMCPY(nlevel_param.cw_level.content_key, &enc_cw[0], 8);

    ret = ce_ioctl(p_ce_dev, IO_CRYPT_GEN_NLEVEL_KEY_ADVANCED, (UINT32)&nlevel_param);
	//libc_printf("gacas_feed_cw_ret=%04d\n",ret);
    if (RET_SUCCESS != ret)
    {
        // Gen Nlevel key failed!
        goto end;
    }
    ret = 0;

end:
    osal_semaphore_release(g_gacas_key_sema_id);
    return ret;
}

int gacas_generaet_cw_to_ce(char *enc_cw, UINT8 data_type)
{
    p_ce_device ce_dev = NULL;
    RET_CODE ret = -1;
    UINT8 type = 0;
    int i = 0;

    ce_dev = (p_ce_device)dev_get_by_type(NULL, HLD_DEV_TYPE_CE);
    if (NULL == ce_dev || NULL == enc_cw)
    {
        // CE device or enc_cw is NULL!
        return ret;
    }

    if (data_type == DES_VIDEO)
        type = VIDEO_DSC;
    else if (data_type == DES_AUDIO)
        type = AUDIO_DSC;
    else
    {
        // wrong ts data type
    }

    for (i = 0; i < MAX_TS_NUM; i++)
    {
        if (g_ts_info[i].key_pos && (g_ts_info[i].type == type))
        {
            ret = gacas_feed_cw(enc_cw, g_ts_info[i].key_pos);
        }
    }

    return ret;
}


int gacas_set_key1(char *key1, unsigned char key1_type)
{
    if (NULL != key1)
        MEMCPY(g_gacas_key1, key1, 16);

    g_gacas_key1_type = key1_type;

    return 0;
}

int gacas_set_key2(char *key2, unsigned char key2_type)
{
    if (NULL != key2)
        MEMCPY(g_gacas_key2, key2, 16);

    g_gacas_key2_type = key2_type;

    return 0;
}

int gacas_set_rootkey_index(unsigned char rk_idx)
{
    g_gacas_rootkey_idx = rk_idx;

    return 0;
}

int gacas_set_cw_type(unsigned char cw_type)
{
    g_gacas_cw_type = cw_type;
}

// Set Odd/Even Key to CE, then start CE to decrypt
int gacas_set_odd_even_key(unsigned char d_type, unsigned char *odd_key, unsigned char *even_key)
{
    if (NULL == odd_key || NULL == even_key)
    {
        // Input key NULL!
        return -1;
    }

    MEMSET(g_gacas_enc_cw, 0, 16);
    MEMCPY(g_gacas_enc_cw, odd_key, 8);
    MEMCPY(&g_gacas_enc_cw[8], even_key, 8);

    if (0 != gacas_generaet_cw_to_ce(g_gacas_enc_cw, d_type))
    {
        // generate cw to ce failed!
    }

    return 0;
}

int gacas_create_key_samaphore()
{
    if (OSAL_INVALID_ID == g_gacas_key_sema_id)
    {
        g_gacas_key_sema_id = osal_semaphore_create(1);
        if (OSAL_INVALID_ID == g_gacas_key_sema_id)
        {
            // Create semaphore failed!
            ASSERT(0)
            return -1;
        }
    }

    return 0;
}

int gacas_delete_key_semaphore()
{
    if (OSAL_INVALID_ID != g_gacas_key_sema_id)
    {
        if (OSAL_E_OK!=osal_semaphore_delete(g_gacas_key_sema_id))
        {
            // delete key semaphore failed!
            return -1;
        }
        return 0;
    }
    else
    {
        // No invalid key semaphore
        return -1;
    }
}

#define ONE_PROGRAMME_MAX_PID_NUM 8 //you can change the max value by yourself.

typedef struct pid_key_pair_node {
	UINT16 pid;
	UINT32 key_handle;
}PID_KEY_PAIR_NODE;

static UINT32 csa_device_id = ALI_INVALID_DSC_SUB_DEV_ID;
static UINT32 xStreamID = ALI_INVALID_CRYPTO_STREAM_ID;
static PID_KEY_PAIR_NODE des_stream_handle_list[ONE_PROGRAMME_MAX_PID_NUM] = {0};

void des_stream_handle_list_reset(void)
{
	UINT32 i = 0;
	
	for(i=0; i<ONE_PROGRAMME_MAX_PID_NUM; i++)
	{
		des_stream_handle_list[i].pid = 0;
		des_stream_handle_list[i].key_handle = ALI_INVALID_CRYPTO_STREAM_HANDLE;
	}
}

void des_stream_handle_list_add(UINT32 handle, UINT16 pid)
{
	UINT32 i = 0;
	
	for(i=0; i<ONE_PROGRAMME_MAX_PID_NUM; i++)
	{
		if(handle == des_stream_handle_list[i].key_handle)
		{
			des_stream_handle_list[i].pid = pid;
			break;
		}

		//if stream handles is not occupied, we can save the new stream handle in this node.
		if(ALI_INVALID_CRYPTO_STREAM_HANDLE == des_stream_handle_list[i].key_handle)
		{
			des_stream_handle_list[i].key_handle = handle;
			des_stream_handle_list[i].pid = pid;
			break;
		}
	}
}

void des_stream_handle_list_release(void)
{
	UINT32 i = 0;
	p_csa_dev pCSADev = NULL;
	RET_CODE ret =0;

	pCSADev = (CSA_DEV *)dev_get_by_id(HLD_DEV_TYPE_CSA, csa_device_id);
	
	for(i=0; i<ONE_PROGRAMME_MAX_PID_NUM; i++)
	{
		if(ALI_INVALID_CRYPTO_STREAM_HANDLE != des_stream_handle_list[i].key_handle)
		{
			ret = csa_ioctl(pCSADev, IO_DELETE_CRYPT_STREAM_CMD, des_stream_handle_list[i].key_handle);
			des_stream_handle_list[i].key_handle = ALI_INVALID_CRYPTO_STREAM_HANDLE;
			des_stream_handle_list[i].pid = 0;
		}
	}
}


UINT32 find_des_stream_handle_by_pid(UINT16 pid)
{
	UINT32 i = 0;
	
	for(i=0; i<ONE_PROGRAMME_MAX_PID_NUM; i++)
	{
		if(pid == des_stream_handle_list[i].pid)
		{
			break;
		}
	}

	if(ONE_PROGRAMME_MAX_PID_NUM == i)
	{
		return ALI_INVALID_CRYPTO_STREAM_HANDLE;
	}
	else
	{
		return des_stream_handle_list[i].key_handle;
	}
}

BOOL link_dmx_and_dsc_chanel(int dmxid)
{
	p_csa_dev pCsaDev = NULL;
	struct dec_parse_param param;

	if (csa_device_id == ALI_INVALID_DSC_SUB_DEV_ID)
	{
		csa_device_id = dsc_get_free_sub_device_id(CSA);
		if (ALI_INVALID_DSC_SUB_DEV_ID == csa_device_id)
		{
			printf("\ndsc_get_free_sub_device_id error id=%d\n\n", csa_device_id);
			return FALSE;
		}
		pCsaDev = (CSA_DEV *)dev_get_by_id(HLD_DEV_TYPE_CSA, csa_device_id);
		xStreamID = dsc_get_free_stream_id(TS_MODE);
		if (xStreamID == ALI_INVALID_CRYPTO_STREAM_ID)
		{
			dsc_set_sub_device_id_idle(CSA, csa_device_id);
			csa_device_id = ALI_INVALID_DSC_SUB_DEV_ID;
			printf("get free ts stream id failed\n");
			return FALSE;
		}

		des_stream_handle_list_reset();

		param.dec_dev = pCsaDev;
		param.type = CSA;

        	struct dmx_device *dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, dmxid);
		dmx_io_control(dmx_dev, IO_SET_DEC_HANDLE, (UINT32)&param);
		dmx_io_control(dmx_dev, IO_SET_DEC_STATUS, (UINT32)1);
		dsc_ioctl((p_dsc_dev)dev_get_by_id(HLD_DEV_TYPE_DSC, 0), IO_PARSE_DMX_ID_SET_CMD, xStreamID);
	}
	return TRUE;
}

/* Using clear key -> KEY_FROM_SRAM to set DES_ECB key for TS data Scramble/DeScramble
pDesDev -> DES device
UINT32 xStreamID -> stream id for this pid
UINT8 *pxKey 	-> if xParity == EVEN_PARITY_MODE,  	pxKey[8] = {8 byte even key}
				-> if xParity == ODD_PARITY_MODE, 	pxKey[8] = {8 byte odd key}
				-> if xParity == AUTO_PARITY_MODE0, pxKey[16] = {8 byte even key, 8 byte odd key}
xParity -> EVEN_PARITY_MODE, ODD_PARITY_MODE, AUTO_PARITY_MODE0
xPid -> the pid to be set to key table

return value: (only available when KEY_FROM_SRAM and KEY_FROM_CRYPTO)
will return "handle" number from low level driver
*/
static UINT32 csa_keysram_pid_map(p_csa_dev pCsaDev, UINT32 keyHandle, UINT32 xStreamID, UINT8 *pxKey, enum PARITY_MODE xParity, UINT16 xPid)
{
	struct csa_init_param xCsaParam;
	KEY_PARAM xKeyParam;
	CSA_KEY_PARAM xKeyInfo[1];
	RET_CODE ret = ALI_INVALID_CRYPTO_STREAM_HANDLE;
	UINT16 pxPid[1] = {xPid & 0x1fff};

	if (!pCsaDev || !pxKey)
	{
		return ret;
	}

	memset(&xCsaParam, 0, sizeof(struct csa_init_param));
	xCsaParam.dma_mode = TS_MODE;
	xCsaParam.key_from = KEY_FROM_SRAM;
	//xCsaParam.key_mode = CSA_MODE;
	xCsaParam.version = CSA2;
	xCsaParam.parity_mode = xParity;
	xCsaParam.stream_id = xStreamID;
	ret = csa_ioctl(pCsaDev, IO_INIT_CMD, (UINT32)&xCsaParam);
	if (ret != RET_SUCCESS)
	{
		return ALI_INVALID_CRYPTO_STREAM_HANDLE;
	}

	memset(&xKeyParam, 0, sizeof(KEY_PARAM));
	if (xParity == EVEN_PARITY_MODE)
	{
		memcpy(xKeyInfo[0].csa_key.even_key, pxKey, 8);
		xKeyParam.no_odd = 1; //only update even key.
	}
	else if (xParity == ODD_PARITY_MODE)
	{
		memcpy(xKeyInfo[0].csa_key.odd_key, pxKey, 8);
		xKeyParam.no_even = 1; //only update odd key.
	}
	else
	{
		memcpy(xKeyInfo[0].csa_key.even_key, pxKey, 8);
		memcpy(xKeyInfo[0].csa_key.odd_key, pxKey+8, 8);
	}
	xKeyParam.handle = keyHandle;
	xKeyParam.ctr_counter = NULL;
	xKeyParam.init_vector = NULL;
	xKeyParam.key_length = 64;
	xKeyParam.pid_list = pxPid;
	xKeyParam.pid_len = 1;
	xKeyParam.p_csa_key_info = xKeyInfo;
	xKeyParam.stream_id = xStreamID;
	if(ALI_INVALID_CRYPTO_STREAM_HANDLE == keyHandle)
	{
		ret = csa_ioctl(pCsaDev, IO_CREAT_CRYPT_STREAM_CMD, (UINT32)&xKeyParam);
		if (ret != RET_SUCCESS)
		{
			return ALI_INVALID_CRYPTO_STREAM_HANDLE;
		}
	}
	else
	{
		ret = csa_ioctl(pCsaDev, IO_KEY_INFO_UPDATE_CMD, (UINT32)&xKeyParam);
		if (ret != RET_SUCCESS)
		{
			return ALI_INVALID_CRYPTO_STREAM_HANDLE;
		}
	}
	

	return xKeyParam.handle;
}

BOOL set_csa_cw(int dmxid, unsigned char *cw, enum PARITY_MODE xParity, unsigned short pid)
{
	UINT32 i = 0;
	p_csa_dev pCsaDev = NULL;
	UINT32 handle = ALI_INVALID_CRYPTO_STREAM_HANDLE;

	if (csa_device_id == ALI_INVALID_DSC_SUB_DEV_ID)
	{
		return FALSE;
	}

	pCsaDev = (CSA_DEV *)dev_get_by_id(HLD_DEV_TYPE_CSA, csa_device_id);

	handle = find_des_stream_handle_by_pid(pid);
	if(ALI_INVALID_CRYPTO_STREAM_HANDLE == handle)
	{
		//create
		handle = csa_keysram_pid_map(pCsaDev, handle, xStreamID, cw, xParity, pid);
		if (ALI_INVALID_CRYPTO_STREAM_HANDLE == handle)
		{
			printf("\ntdes_ecb_nbits_ts_keysram_map error\n\n");
			return FALSE;
		}
	
		des_stream_handle_list_add(handle, pid);
	}
	else
	{
		//update
		csa_keysram_pid_map(pCsaDev, handle, xStreamID, cw, xParity, pid);
	}
	
	return TRUE;
}

#if 1
//TPE 180725#1
//static UINT32 csa_device_id = ALI_INVALID_DSC_SUB_DEV_ID;
//static UINT32 xStreamID = ALI_INVALID_CRYPTO_STREAM_ID;
UINT32 csa_init = FALSE;
static UINT32 key_handle = ALI_INVALID_CRYPTO_STREAM_HANDLE;
static UINT32 csa_key_pos = INVALID_CE_KEY_POS;
static BOOL link_dmx_and_dsc_chanel2(int dmxid)
{
	p_csa_dev pCsaDev = NULL;
	struct dec_parse_param param;
	
	if (csa_device_id == ALI_INVALID_DSC_SUB_DEV_ID)
	{
		csa_device_id = dsc_get_free_sub_device_id(CSA);
		if (ALI_INVALID_DSC_SUB_DEV_ID == csa_device_id)
		{
			libc_printf("dsc_get_free_sub_device_id error id=%d\n", csa_device_id);
			return FALSE;
		}
		pCsaDev = (CSA_DEV *)dev_get_by_id(HLD_DEV_TYPE_CSA, csa_device_id);
		xStreamID = dsc_get_free_stream_id(TS_MODE);
		if (xStreamID == ALI_INVALID_CRYPTO_STREAM_ID)
		{
			dsc_set_sub_device_id_idle(CSA, csa_device_id);
			csa_device_id = ALI_INVALID_DSC_SUB_DEV_ID;
			libc_printf("dsc_get_free_stream_id failed\n");			
			return FALSE; 
		}

		//dmxid is not important for DSC module, you just can use dmx0 for any dmx channel.
		param.dec_dev = pCsaDev;
		param.type = CSA;
		struct dmx_device *dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, dmxid);
		dmx_io_control(dmx_dev, IO_SET_DEC_HANDLE, (UINT32)&param);
		dmx_io_control(dmx_dev, IO_SET_DEC_STATUS, (UINT32)1);

		dsc_ioctl((p_dsc_dev)dev_get_by_id(HLD_DEV_TYPE_DSC, 0), IO_PARSE_DMX_ID_SET_CMD, xStreamID);
	}
	return TRUE;
}

static UINT8 set_cw_via_csa(UINT16 *pid, UINT8 pidNum, UINT8 *evenCW, UINT8 *oddCW)
{
	p_csa_dev pCasDev;
	CSA_INIT_PARAM csa_param;
	KEY_PARAM key_param;
	CSA_KEY_PARAM key_info[1];
	RET_CODE ret = 0;

	pCasDev = (p_csa_dev)dev_get_by_id(HLD_DEV_TYPE_CSA, 0);

	MEMSET(&key_param, 0, sizeof(KEY_PARAM));
	MEMCPY(key_info[0].csa_key.odd_key, oddCW, 8);
	MEMCPY(key_info[0].csa_key.even_key, evenCW, 8);

	if(FALSE == csa_init)
	{
		MEMSET(&csa_param, 0, sizeof(struct csa_init_param));
		//csa_param.dcw[0] = 0;
		//csa_param.dcw[1] = 1;
		//csa_param.dcw[2] = 2;
		//csa_param.dcw[3] = 3;
		csa_param.dma_mode = TS_MODE;
		csa_param.key_from = KEY_FROM_SRAM;
		csa_param.parity_mode = AUTO_PARITY_MODE0;
		//csa_param.pes_en = 1;
		//csa_param.scramble_control = 0;
		csa_param.stream_id = xStreamID;
		csa_param.version = CSA2;
		ret = csa_ioctl(pCasDev, IO_INIT_CMD, (UINT32)&csa_param);
		if(ret != RET_SUCCESS)
		{
			libc_printf("CSA_IO_INIT_CMD fail, ret = %d\n", ret);
		}

		csa_init = TRUE;
    	}

	MEMSET(&key_param, 0, sizeof(KEY_PARAM));
	key_param.handle = 0xFF;
	//key_param.ctr_counter = NULL;
	//key_param.init_vector = NULL;
	key_param.key_length = 64;
	key_param.pid_len = pidNum;
	key_param.pid_list = pid;
	//key_param.p_aes_iv_info = NULL;
	key_param.stream_id = xStreamID;//(g_dmx_dev->type & HLD_DEV_ID_MASK);//0;
	//key_param.force_mode = 0;
	key_param.p_csa_key_info = key_info;
	ret = csa_ioctl(pCasDev, IO_CREAT_CRYPT_STREAM_CMD, (UINT32)&key_param);
	if(ret != RET_SUCCESS)
	{
		libc_printf("IO_CREAT_CRYPT_STREAM_CMD fail, ret = %d\n", ret);
	}
	key_handle = key_param.handle;

	return RET_SUCCESS;
}
static UINT8 update_cw_via_csa(UINT16 *pid, UINT8 pidNum, UINT8 *evenCW, UINT8 *oddCW)
{
	p_csa_dev pCasDev;
	KEY_PARAM key_param;
	CSA_KEY_PARAM key_info[1];
	RET_CODE ret = 0;

	//MEMCPY(pids, pid, pidNum);

	pCasDev = (p_csa_dev)dev_get_by_id(HLD_DEV_TYPE_CSA, 0);

	MEMSET(&key_param, 0, sizeof(KEY_PARAM));
	MEMCPY(key_info[0].csa_key.odd_key, oddCW, 8);	
	MEMCPY(key_info[0].csa_key.even_key, evenCW, 8);  

	MEMSET(&key_param, 0, sizeof(KEY_PARAM));
	key_param.handle = key_handle;
	key_param.key_length = 64;
	key_param.pid_len = pidNum;
	key_param.pid_list = pid;
	//key_param.stream_id = stream_id;//(g_dmx_dev->type & HLD_DEV_ID_MASK);//0;
	//key_param.force_mode = 0;
	key_param.p_csa_key_info = key_info;
	ret = csa_ioctl(pCasDev, IO_KEY_INFO_UPDATE_CMD, (UINT32)&key_param);
	if(ret != RET_SUCCESS)
	{
		libc_printf("IO_KEY_INFO_UPDATE_CMD fail, ret = %d\n", ret);
	}

	return RET_SUCCESS;
}

static void play_clear_cw_stream(void)
{
	//ALi550000001.ts - STV1 Norrbotton
	UINT8 abOdd1[8] = {0x6F,0x8E,0x95,0x92,0x7D,0x16,0xCE,0x61};
	UINT8 abEven1[8] = {0x32,0xF3,0xC7,0xEC,0x60,0xDA,0xCD,0x07};
	//UINT8 abOdd2[8] = {0xBF,0x94,0xA5,0xF8,0xB4,0x23,0xAC,0x83};
	//UINT8 abEven2[8] = {0x32,0xF3,0xC7,0xEC,0x60,0xDA,0xCD,0x07};
	UINT16 pids[3] = {1019,1018,1019};

	set_cw_via_csa(pids, 3, abEven1, abOdd1);
}

//#define cryptoguard 1
static UINT32 gen_nlevel_key_pos(UINT8 *stage1,UINT8 *stage2,UINT8 *DECW)
{
	p_ce_device pce_dev;
	RET_CODE ret = RET_FAILURE;
	UINT32 kl_level = 0;
	CE_FOUND_FREE_POS_PARAM key_pos_param={0,0,0,0};
	CE_NLEVEL_PARAM nlevel_param;

	pce_dev = (p_ce_device)dev_get_by_type(NULL, HLD_DEV_TYPE_CE);
	if(NULL == pce_dev)
	{
		return RET_FAILURE;
	}

    //ALi550000001.ts - STV1 Norrbotton
#ifdef cryptoguard
	kl_level = sys_ic_get_kl_key_mode (KEY_0_3);
	libc_printf("Key Ladder %d is %d level\n", KEY_0_3 + 1, kl_level);
#else
	kl_level = sys_ic_get_kl_key_mode (KEY_0_0);
	libc_printf("Key Ladder %d is %d level\n", KEY_0_0 + 1, kl_level);
#endif
    
	if(csa_key_pos == INVALID_CE_KEY_POS)
	{
		//Find key slot. 
		MEMSET(&key_pos_param, 0, sizeof(CE_FOUND_FREE_POS_PARAM));
		key_pos_param.pos = ALI_INVALID_CRYPTO_KEY_POS;
		//ALi550000001.ts - STV1 Norrbotton
#ifdef cryptoguard        
		key_pos_param.root = KEY_0_3;
#else
		key_pos_param.root = KEY_0_0;
#endif
		key_pos_param.number= 1;
		key_pos_param.ce_key_level = kl_level; //Must same to the otp	
		ret = ce_ioctl(pce_dev, IO_CRYPT_FOUND_FREE_POS, (UINT32)&key_pos_param);
		if (RET_SUCCESS != ret)
		{
			libc_printf("IO_CRYPT_FOUND_FREE_POS fail!\n");
			return RET_FAILURE;
		}

		csa_key_pos = key_pos_param.pos;

	}
	libc_printf("csa_key_pos: 0x%x\n", csa_key_pos);
	MEMSET(&nlevel_param, 0, sizeof(CE_NLEVEL_PARAM));
	//ALi550000001.ts - STV1 Norrbotton
#ifdef cryptoguard     
	nlevel_param.kl_index = ALI_KL_3;
	nlevel_param.otp_addr = OTP_ADDESS_6;
#else
	nlevel_param.kl_index = ALI_KL_0;
	nlevel_param.otp_addr = OTP_ADDESS_1;
#endif
	nlevel_param.algo 			= CE_SELECT_AES;
	nlevel_param.crypto_mode 	= CE_IS_DECRYPT;
	nlevel_param.pos 			= csa_key_pos;
	nlevel_param.protecting_key_num = (kl_level-1);
	//copy first stage pk to protecting_key[0-15]
	MEMCPY(nlevel_param.protecting_key, stage1, 16);
	//copy second stage pk to &protecting_key[16-32]
	MEMCPY(&(nlevel_param.protecting_key[16]), stage2, 16);
	//copy even ck to content_key[0-15]
	MEMCPY(nlevel_param.content_key, DECW, 16);
    //nlevel_param.parity = CE_PARITY_EVEN_ODD; //for CE_SELECT_TDES

	ret = ce_ioctl(pce_dev, IO_CRYPT_GEN_NLEVEL_KEY, (UINT32)&nlevel_param);
	if (RET_SUCCESS != ret)
	{
		libc_printf("IO_CRYPT_GEN_NLEVEL_KEY fail!, ret = %d\n", ret);
		ce_ioctl(pce_dev, IO_CRYPT_POS_SET_IDLE, (UINT32)csa_key_pos);
		csa_key_pos = INVALID_CE_KEY_POS;
		return INVALID_CE_KEY_POS;
	}
#if 0
	CE_DEBUG_KEY_INFO debug_key_info;
	MEMSET(&debug_key_info, 0, sizeof(debug_key_info));
	debug_key_info.kl_index_sel = ALI_KL_3;
	debug_key_info.len = 16;
	debug_key_info.pos = (key_pos&0xFF);
	debug_key_info.sel = CE_KEY_READ;
	ret = ce_ioctl(pce_dev, IO_CRYPT_DEBUG_GET_KEY, (UINT32)&debug_key_info);
	if (RET_SUCCESS != ret)
	{
		libc_printf("IO_CRYPT_DEBUG_GET_KEY fail!\n");
		ce_ioctl(pce_dev, IO_CRYPT_POS_SET_IDLE, (UINT32)key_pos);
		return INVALID_CE_KEY_POS;
	}
	libc_printf("");
#endif
	return csa_key_pos;
}

static UINT8 set_cw_key_pos_via_csa(UINT16 *pid, UINT8 pidNum, UINT32 KEY_POS)
{
	p_csa_dev pCasDev;
	CSA_INIT_PARAM csa_param;
	KEY_PARAM key_param;
	RET_CODE ret = 0;

	pCasDev = (p_csa_dev)dev_get_by_id(HLD_DEV_TYPE_CSA, csa_device_id);

	if(FALSE == csa_init)
	{
		MEMSET(&csa_param, 0, sizeof(struct csa_init_param));
		//csa_param.dcw[0] = 0;
		//csa_param.dcw[1] = 1;
		//csa_param.dcw[2] = 2;
		//csa_param.dcw[3] = 3;
		csa_param.dma_mode = TS_MODE;
		csa_param.key_from = KEY_FROM_CRYPTO;
		csa_param.parity_mode = AUTO_PARITY_MODE0;
		//csa_param.pes_en = 1;
		//csa_param.scramble_control = 0;
		csa_param.stream_id = xStreamID;
		csa_param.version = CSA2;
		ret = csa_ioctl(pCasDev, IO_INIT_CMD, (UINT32)&csa_param);
		if(ret != RET_SUCCESS)
		{
			libc_printf("CSA_IO_INIT_CMD fail, ret = %d\n", ret);
		}

		csa_init = TRUE;
	}

	MEMSET(&key_param, 0, sizeof(KEY_PARAM));
	key_param.handle = 0xFF;
	//key_param.ctr_counter = NULL;
	//key_param.init_vector = NULL;
	key_param.key_length = 64;
	key_param.pid_len = pidNum;
	key_param.pid_list = pid;
	key_param.kl_sel = (KEY_POS>>8)&0x0F;
	//key_param.p_aes_iv_info = NULL;
	key_param.stream_id = xStreamID;;
	//key_param.force_mode = 0;
	key_param.pos = (KEY_POS&0xFF);
	ret = csa_ioctl(pCasDev, IO_CREAT_CRYPT_STREAM_CMD, (UINT32)&key_param);
	if(ret != RET_SUCCESS)
	{
		libc_printf("IO_CREAT_CRYPT_STREAM_CMD fail, ret = %d\n", ret);
	}
	key_handle = key_param.handle;
	return RET_SUCCESS;
}

void play_encrypted_cw_stream(UINT16 pid_v,UINT16 pid_a,UINT16 pid_c,UINT8 *key1,UINT8 *key2,UINT8*cw_key)
{
	UINT32 key_pos = INVALID_CE_KEY_POS;

	//ALi550000001.ts - STV1 Norrbotton
	UINT16 pids[3] = {1019,1018,1019}; 

	//UINT8 decw_1[16] = {0x48,0x18,0x57,0xe4,0x83,0x38,0xfd,0x96,0x68,0x54,0x75,0xab,0x94,0xec,0x25,0x71};
	//UINT8 decw_2[16] = {0x43,0x15,0x9e,0xa5,0x94,0x03,0x00,0x6f,0xbb,0x96,0x0b,0x05,0x91,0xfe,0x77,0xef};
	//if want to update cw, just only need to do gen_nlevel_key_pos.
	key_pos = gen_nlevel_key_pos(key1,key2,cw_key);
	pids[0] = pid_v;
	pids[1] = pid_a;
	pids[2] = pid_c;
	set_cw_key_pos_via_csa(pids, 3, key_pos);
}
#endif

