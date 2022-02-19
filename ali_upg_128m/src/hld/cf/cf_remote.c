
#include <sys_config.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>
#include <hld/cf/cf.h>

#ifdef DUAL_ENABLE
#include <modules.h>
enum CF_RFUNC
{
	FUNC_CF_ATTACH = 0,
	FUNC_CF_DETACH,
	FUNC_CF_OPEN,
	FUNC_CF_CLOSE,
	FUNC_CF_IOCTL,
	FUNC_CF_WRITE,
	FUNC_CF_READ,
	FUNC_CF_SET_TARGET,
/*
	FUNC_CF_SUSPEND,
	FUNC_CF_RESUME,
*/
};

#ifdef _HLD_CF_REMOTE
#define CF_NPARA(x) ((HLD_CF_MODULE<<24)|(x<<16))

int cf_api_attach(void)
{
	jump_to_func(NULL, os_hld_caller, NULL,
		CF_NPARA(0) | FUNC_CF_ATTACH, NULL);
}

int cf_api_detach(void)
{
	jump_to_func(NULL, os_hld_caller, NULL,
		CF_NPARA(0) | FUNC_CF_DETACH, NULL);
}

int cf_open(void)
{
	jump_to_func(NULL, os_hld_caller, NULL,
		CF_NPARA(0) | FUNC_CF_OPEN, NULL);
}

int cf_close(int chan)
{
	jump_to_func(NULL, os_hld_caller, chan,
		CF_NPARA(1) | FUNC_CF_CLOSE, NULL);
}

int cf_ioctl(int chan, int cmd, void *param)
{
	UINT32 desc[] = { //desc of pointer para
		1, DESC_STATIC_STRU(0, 0),
		1, DESC_P_PARA(0, 2, 0), 
		//desc of pointer ret
		0,                          
		0,
	};

	switch (cmd) {
	case ALI_CF_IOC_VERSION_INFO:
		DESC_OUTPUT_STRU_SET_SIZE(desc, 0,
				sizeof(struct ali_cf_version));
		break;
	case ALI_CF_IOC_FEATURE_VECTOR:
		DESC_OUTPUT_STRU_SET_SIZE(desc, 0,
				sizeof(struct ali_cf_feature));
		break;
	case ALI_CF_IOC_CF_STATUS:
		DESC_OUTPUT_STRU_SET_SIZE(desc, 0,
				sizeof(struct ali_cf_cf_status));
		break;
	case ALI_CF_IOC_TRANS_STATUS:
		DESC_OUTPUT_STRU_SET_SIZE(desc, 0,
				sizeof(struct ali_cf_trans_status));
		break;
	case ALI_CF_IOC_DECM_STATUS:
		DESC_OUTPUT_STRU_SET_SIZE(desc, 0,
				sizeof(struct ali_cf_decm_status));
		break;
	case ALI_CF_IOC_ISSUE_OP_FEATURE:
	case ALI_CF_IOC_ISSUE_OP_DIFF:
	case ALI_CF_IOC_ISSUE_OP_CWCORSHV:
		DESC_STATIC_STRU_SET_SIZE(desc, 0,
				sizeof(struct ali_cf_operation));
		break;
	case ALI_CF_IOC_READ_OP_RESULT:
		DESC_OUTPUT_STRU_SET_SIZE(desc, 0,
				sizeof(struct ali_cf_result));
		break;
	case ALI_CF_IOC_WAIT_OP_DONE:
		DESC_OUTPUT_STRU_SET_SIZE(desc, 0,
				sizeof(struct ali_cf_trans_status));
		break;
	default:
		return -CF_ERROR_INVALID_PARAM;
	}

	jump_to_func(NULL, os_hld_caller, chan,
		CF_NPARA(3) | FUNC_CF_IOCTL, desc);
}

int cf_write(int chan, UINT8 *buf, int count)
{
	UINT32 desc[] = { //desc of pointer para
		1, DESC_STATIC_STRU(0, sizeof(struct ali_cf_operation)),
		1, DESC_P_PARA(0, 1, 0), 
		//desc of pointer ret
		0,                          
		0,
	};

	jump_to_func(NULL, os_hld_caller, chan,
		CF_NPARA(3) | FUNC_CF_WRITE, desc);
}

int cf_read(int chan, UINT8 *buf, int count)
{
	UINT32 desc[] = { //desc of pointer para
		1, DESC_OUTPUT_STRU(0, sizeof(struct ali_cf_result)),
		1, DESC_P_PARA(0, 1, 0), 
		//desc of pointer ret
		0,                          
		0,
	};

	jump_to_func(NULL, os_hld_caller, chan,
		CF_NPARA(3) | FUNC_CF_READ, desc);
}

int cf_set_target(int pos)
{
	jump_to_func(NULL, os_hld_caller, pos,
		CF_NPARA(1) | FUNC_CF_SET_TARGET, NULL);
}

#else

static UINT32 hld_cf_entry[] =
{
	(UINT32)cf_api_attach,
	(UINT32)cf_api_detach,
	(UINT32)cf_open,
	(UINT32)cf_close,
	(UINT32)cf_ioctl,
	(UINT32)cf_write,
	(UINT32)cf_read,
	(UINT32)cf_set_target,
};

void hld_cf_callee(UINT8 *msg)
{
    if (NULL == msg)
    {
        return;
    }
    os_hld_callee((UINT32)hld_cf_entry, msg);
}

#endif /*_HLD_CF_REMOTE*/

#endif /*DUAL_ENABLE*/

