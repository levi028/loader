
#include <sys_config.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>
#include <hld/cert/cert.h>

/*!
 *	 @brief Define the RPC func list for CERT 
*/
#define CERT_RPC_AKL_ATTACH (0)
#define CERT_RPC_AKL_DETACH (1)
#define CERT_RPC_AKL_OPEN (2)
#define CERT_RPC_AKL_CLOSE (3)
#define CERT_RPC_AKL_EXCHANGE (4)
#define CERT_RPC_AKL_SAVEKEY (5)
#define CERT_RPC_AKL_ACK (6)

#define CERT_RPC_ASA_ATTACH (10)
#define CERT_RPC_ASA_DETACH (11)
#define CERT_RPC_ASA_SETFMT (12)
#define CERT_RPC_ASA_OPEN (13)
#define CERT_RPC_ASA_CLOSE (14)
#define CERT_RPC_ASA_ADDPID (15)
#define CERT_RPC_ASA_DELPID (16)
#define CERT_RPC_ASA_DECRYPT (17)

#ifdef DUAL_ENABLE

#include <modules.h>
#define CERT_NPARA(x) ((HLD_CERT_MODULE<<24)|(x<<16))

#ifndef _HLD_CERT_REMOTE

static UINT32 hld_cert_entry[] =
{
	(UINT32)cert_akl_attach,
	(UINT32)cert_akl_detach,
	(UINT32)cert_akl_open,
	(UINT32)cert_akl_close,
	(UINT32)cert_akl_exchange,
	(UINT32)cert_akl_save_key,
	(UINT32)cert_akl_ack,
	/* reserve 3 func entries for akl future */
	(UINT32)NULL, 
	(UINT32)NULL,
	(UINT32)NULL,

	(UINT32)cert_asa_attach,
	(UINT32)cert_asa_detach,
	(UINT32)cert_asa_set_format,
	(UINT32)cert_asa_open,
	(UINT32)cert_asa_close,
	(UINT32)cert_asa_add_pid,
	(UINT32)cert_asa_del_pid,
	(UINT32)cert_asa_decrypt,
};

void hld_cert_callee(UINT8 *msg)
{
	if(NULL == msg)
	{
		return;
	}
	os_hld_callee((UINT32)hld_cert_entry, msg);
}

#endif /*_HLD_CERT_REMOTE*/

#ifdef _HLD_CERT_REMOTE

/*static UINT32 cert_io_control[] =
{
    //desc of pointer para
    1, DESC_STATIC_STRU(0, 0),
    1, DESC_P_PARA(0, 2, 0),
    //desc of pointer ret
    0,
    0,
};*/

int cert_akl_attach(void)
{
     jump_to_func(NULL, os_hld_caller, NULL, CERT_NPARA(0) | CERT_RPC_AKL_ATTACH, NULL);
}

int cert_akl_detach(void)
{
     jump_to_func(NULL, os_hld_caller, NULL, CERT_NPARA(0) | CERT_RPC_AKL_DETACH, NULL);
}

int cert_akl_open(void)
{
     jump_to_func(NULL, os_hld_caller, NULL, CERT_NPARA(0) | CERT_RPC_AKL_OPEN, NULL);
}

int cert_akl_close(int sess)
{
     jump_to_func(NULL, os_hld_caller, sess, CERT_NPARA(1) | CERT_RPC_AKL_CLOSE, NULL);
}

int cert_akl_exchange(int sess, struct cert_akl_cmd *param)
{
	UINT32 desc[] =
	{
		//desc of pointer para
		1, DESC_OUTPUT_STRU(0, sizeof(struct cert_akl_cmd)),
		1, DESC_P_PARA(0, 1, 0),
		//desc of pointer ret
		0,
		0,
	};
	jump_to_func(NULL, os_hld_caller, sess, CERT_NPARA(2) | CERT_RPC_AKL_EXCHANGE, desc);
}

int cert_akl_save_key(int sess, struct cert_akl_savekey *param)
{
	UINT32 desc[] =
	{
		//desc of pointer para
		1, DESC_STATIC_STRU(0, sizeof(struct cert_akl_savekey)),
		1, DESC_P_PARA(0, 1, 0),
		//desc of pointer ret
		0,
		0,
	};
	jump_to_func(NULL, os_hld_caller, sess, CERT_NPARA(2) | CERT_RPC_AKL_SAVEKEY, desc);
}

int cert_akl_ack(int sess)
{
	jump_to_func(NULL, os_hld_caller, sess, CERT_NPARA(1) | CERT_RPC_AKL_ACK, NULL);
}

int cert_asa_attach(void)
{
	jump_to_func(NULL, os_hld_caller, NULL, CERT_NPARA(0) | CERT_RPC_ASA_ATTACH, NULL);
}

int cert_asa_detach(void)
{
	jump_to_func(NULL, os_hld_caller, NULL, CERT_NPARA(0) | CERT_RPC_ASA_DETACH, NULL);
}

int cert_asa_set_format(int sess, int format)
{
	jump_to_func(NULL, os_hld_caller, sess, CERT_NPARA(2) | CERT_RPC_ASA_SETFMT, NULL);
}

int cert_asa_open(void)
{
     jump_to_func(NULL, os_hld_caller, NULL, CERT_NPARA(0) | CERT_RPC_ASA_OPEN, NULL);
}

int cert_asa_close(int sess)
{
     jump_to_func(NULL, os_hld_caller, sess, CERT_NPARA(1) | CERT_RPC_ASA_CLOSE, NULL);
}

int cert_asa_add_pid(int sess, struct cert_asa_pid *param)
{
	UINT32 desc[] =
	{
		//desc of pointer para
		1, DESC_STATIC_STRU(0, sizeof(struct cert_asa_pid)),
		1, DESC_P_PARA(0, 1, 0),
		//desc of pointer ret
		0,
		0,
	};
	jump_to_func(NULL, os_hld_caller, sess, CERT_NPARA(2) | CERT_RPC_ASA_ADDPID, desc);
}

int cert_asa_del_pid(int sess, struct cert_asa_pid *param)
{
	UINT32 desc[] =
	{
		//desc of pointer para
		1, DESC_STATIC_STRU(0, sizeof(struct cert_asa_pid)),
		1, DESC_P_PARA(0, 1, 0),
		//desc of pointer ret
		0,
		0,
	};
	jump_to_func(NULL, os_hld_caller, sess, CERT_NPARA(2) | CERT_RPC_ASA_DELPID, desc);
}

static int _cert_asa_decrypt(int sess, UINT8 *input, UINT8 *output, UINT32 length)
{
	jump_to_func(NULL, os_hld_caller, sess, CERT_NPARA(4) | CERT_RPC_ASA_DECRYPT, NULL);
}

int cert_asa_decrypt(int sess, UINT8 *input, UINT8 *output, UINT32 length)
{
	int ret = -1;

	osal_cache_flush(input, length);
	ret = _cert_asa_decrypt(sess, input, output, length);
	osal_cache_invalidate(output, length);
	return ret;
}

#endif  /*_HLD_CE_REMOTE*/

#endif /*DUAL_ENABLE*/

