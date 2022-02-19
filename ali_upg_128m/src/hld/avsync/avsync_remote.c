#include <osal/osal.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <sys_config.h>
#include <mediatypes.h>
#include <hld/avsync/avsync.h>

#ifdef DUAL_ENABLE
#include <modules.h>

enum HLD_AVSYNC_FUNC
{
    FUNC_AVSYNC_OPEN = 0,
    FUNC_AVSYNC_CLOSE,
    FUNC_AVSYNC_START,
    FUNC_AVSYNC_STOP,
    FUNC_AVSYNC_RESET,
    FUNC_AVSYNC_IOCTL,
    FUNC_AVSYNC_SET_SYNCMODE,
    FUNC_AVSYNC_GET_SYNCMODE,
    FUNC_AVSYNC_CFG_PARAMS,
    FUNC_AVSYNC_GET_PARAMS,
    FUNC_AVSYNC_CFG_ADV_PARAMS,
    FUNC_AVSYNC_GET_ADV_PARAMS,
    FUNC_AVSYNC_SET_SRCTYPE,
    FUNC_AVSYNC_GET_SRCTYPE,
    FUNC_AVSYNC_GET_STATUS,
    FUNC_AVSYNC_GET_STATISTICS,
    FUNC_AVSYNC_VIDEO_SMOOTHLY_PLAY_ONOFF,
    FUNC_AVSYNC_DBG_SET_PRINT_OPTION,
    FUNC_AVSYNC_DBG_SET_POLL_ONOFF,
    FUNC_AVSYNC_DBG_SET_POLL_OPTION,
    FUNC_AVSYNC_DBG_SET_POLL_INTERVAL,
};

#ifndef _HLD_AVSYNC_REMOTE
UINT32 hld_avsync_entry[] =
{
    (UINT32)avsync_open,
    (UINT32)avsync_close,
    (UINT32)avsync_start,
    (UINT32)avsync_stop,
    (UINT32)avsync_reset,
    (UINT32)avsync_ioctl,
    (UINT32)avsync_set_syncmode,
    (UINT32)avsync_get_syncmode,
    (UINT32)avsync_config_params,
    (UINT32)avsync_get_params,
    (UINT32)avsync_config_advance_params,
    (UINT32)avsync_get_advance_params,
    (UINT32)avsync_set_sourcetype,
    (UINT32)avsync_get_sourcetype,
    (UINT32)avsync_get_status,
    (UINT32)avsync_get_statistics,
    (UINT32)avsync_video_smoothly_play_onoff,
/*
	(UINT32)avsync_dbg_set_print_option,
    (UINT32)avsync_dbg_polling_onoff,
    (UINT32)avsync_dbg_set_polling_option,
    (UINT32)avsync_dbg_set_polling_interval,
*/
};

void hld_avsync_callee(UINT8 *msg)
{
    os_hld_callee((UINT32)hld_avsync_entry, msg);
}

#else /* _HLD_DECV_REMOTE */

static UINT32 desc_avsync_p_uint32[] =
{
    //desc of pointer para
    1, DESC_OUTPUT_STRU(0, 4),
    1, DESC_P_PARA(0, 1, 0),
    //desc of pointer ret
    0,
    0,
};

static UINT32 desc_avsync_io_uint32[] =
{
    //desc of pointer para
    1, DESC_OUTPUT_STRU(0, 4),
    1, DESC_P_PARA(0, 2, 0),
    //desc of pointer ret
    0,
    0,
};

static UINT32 desc_avsync_cfg_param[] =
{
    //desc of pointer para
    1, DESC_STATIC_STRU(0, sizeof(avsync_cfg_param_t)),
    1, DESC_P_PARA(0, 1, 0),
    //desc of pointer ret
    0,
    0,
};

static UINT32 desc_avsync_get_param[] =
{
    //desc of pointer para
    1, DESC_OUTPUT_STRU(0, sizeof(avsync_cfg_param_t)),
    1, DESC_P_PARA(0, 1, 0),
    //desc of pointer ret
    0,
    0,
};

static UINT32 desc_avsync_cfg_adv_param[] =
{
    //desc of pointer para
    1, DESC_STATIC_STRU(0, sizeof(avsync_adv_param_t)),
    1, DESC_P_PARA(0, 1, 0),
    //desc of pointer ret
    0,
    0,
};

static UINT32 desc_avsync_get_adv_param[] =
{
    //desc of pointer para
    1, DESC_OUTPUT_STRU(0, sizeof(avsync_adv_param_t)),
    1, DESC_P_PARA(0, 1, 0),
    //desc of pointer ret
    0,
    0,
};

static UINT32 desc_avsync_get_status[] =
{
    //desc of pointer para
    1, DESC_OUTPUT_STRU(0, sizeof(avsync_status_t)),
    1, DESC_P_PARA(0, 1, 0),
    //desc of pointer ret
    0,
    0,
};

static UINT32 desc_avsync_get_statistics[] =
{
    //desc of pointer para
    1, DESC_OUTPUT_STRU(0, sizeof(avsync_statistics_t)),
    1, DESC_P_PARA(0, 1, 0),
    //desc of pointer ret
    0,
    0,
};

RET_CODE avsync_open(struct avsync_device *dev)
{
    if(NULL == dev)
    {
        return RET_FAILURE;
    }

    jump_to_func(NULL, os_hld_caller, dev, (HLD_AVSYNC_MODULE<<24)|(1<<16)|FUNC_AVSYNC_OPEN, NULL);
}

RET_CODE avsync_close(struct avsync_device *dev)
{
    if(NULL == dev)
    {
        return RET_FAILURE;
    }

    jump_to_func(NULL, os_hld_caller, dev, (HLD_AVSYNC_MODULE<<24)|(1<<16)|FUNC_AVSYNC_CLOSE, NULL);
}

RET_CODE avsync_start(struct avsync_device *dev)
{
    if(NULL == dev)
    {
        return RET_FAILURE;
    }

    jump_to_func(NULL, os_hld_caller, dev, (HLD_AVSYNC_MODULE<<24)|(1<<16)|FUNC_AVSYNC_START, NULL);
}

RET_CODE avsync_stop(struct avsync_device *dev)
{
    if(NULL == dev)
    {
        return RET_FAILURE;
    }

    jump_to_func(NULL, os_hld_caller, dev, (HLD_AVSYNC_MODULE<<24)|(1<<16)|FUNC_AVSYNC_STOP, NULL);
}

RET_CODE avsync_reset(struct avsync_device *dev)
{
    if(NULL == dev)
    {
        return RET_FAILURE;
    }

    jump_to_func(NULL, os_hld_caller, dev, (HLD_AVSYNC_MODULE<<24)|(1<<16)|FUNC_AVSYNC_RESET, NULL);
}

RET_CODE avsync_ioctl(struct avsync_device *dev, UINT32 io_code, UINT32 param)
{
	UINT32 *desc = NULL;

    if(NULL == dev)
    {
        return RET_FAILURE;
    }

	switch(io_code)
	{
		case AVSYNC_IO_GET_CURRENT_PLAY_PTS:
			desc = desc_avsync_io_uint32;
			break;
		default:
			break;
	}

	jump_to_func(NULL, os_hld_caller, dev, (HLD_AVSYNC_MODULE<<24)|(3<<16)|FUNC_AVSYNC_IOCTL, desc);
}

RET_CODE avsync_set_syncmode(struct avsync_device *dev, AVSYNC_MODE_E mode)
{
	if(NULL == dev)
    {
        return RET_FAILURE;
    }

	jump_to_func(NULL, os_hld_caller, dev, (HLD_AVSYNC_MODULE<<24)|(2<<16)|FUNC_AVSYNC_SET_SYNCMODE, NULL);
}

RET_CODE avsync_get_syncmode(struct avsync_device *dev, AVSYNC_MODE_E *pmode)
{
	if((NULL == dev) || (NULL == pmode))
    {
        return RET_FAILURE;
    }

	jump_to_func(NULL, os_hld_caller, dev, (HLD_AVSYNC_MODULE<<24)|(2<<16)|FUNC_AVSYNC_GET_SYNCMODE,
		desc_avsync_p_uint32);
}

RET_CODE avsync_config_params(struct avsync_device *dev,  avsync_cfg_param_t *pcfg_params)
{
	if((NULL == dev) || (NULL == pcfg_params))
    {
        return RET_FAILURE;
    }

	jump_to_func(NULL, os_hld_caller, dev, (HLD_AVSYNC_MODULE<<24)|(2<<16)|FUNC_AVSYNC_CFG_PARAMS,
		desc_avsync_cfg_param);
}

RET_CODE avsync_get_params(struct avsync_device *dev,  avsync_cfg_param_t *pcfg_params)
{
	if((NULL == dev) || (NULL == pcfg_params))
    {
        return RET_FAILURE;
    }

	jump_to_func(NULL, os_hld_caller, dev, (HLD_AVSYNC_MODULE<<24)|(2<<16)|FUNC_AVSYNC_GET_PARAMS,
		desc_avsync_get_param);
}

RET_CODE avsync_config_advance_params(struct avsync_device *dev,  avsync_adv_param_t *pcfg_params)
{
	if((NULL == dev) || (NULL == pcfg_params))
    {
        return RET_FAILURE;
    }

	jump_to_func(NULL, os_hld_caller, dev, (HLD_AVSYNC_MODULE<<24)|(2<<16)|FUNC_AVSYNC_CFG_ADV_PARAMS,
		desc_avsync_cfg_adv_param);
}

RET_CODE avsync_get_advance_params(struct avsync_device *dev,  avsync_adv_param_t *pcfg_params)
{
	if((NULL == dev) || (NULL == pcfg_params))
    {
        return RET_FAILURE;
    }

	jump_to_func(NULL, os_hld_caller, dev, (HLD_AVSYNC_MODULE<<24)|(2<<16)|FUNC_AVSYNC_GET_ADV_PARAMS,
		desc_avsync_get_adv_param);
}

RET_CODE avsync_set_sourcetype(struct avsync_device *dev, AVSYNC_SRCTYPE_E type)
{
	if(NULL == dev)
    {
        return RET_FAILURE;
    }

	jump_to_func(NULL, os_hld_caller, dev, (HLD_AVSYNC_MODULE<<24)|(2<<16)|FUNC_AVSYNC_SET_SRCTYPE, NULL);
}

RET_CODE avsync_get_sourcetype(struct avsync_device *dev, AVSYNC_SRCTYPE_E *ptype)
{
	if(NULL == dev)
    {
        return RET_FAILURE;
    }

	jump_to_func(NULL, os_hld_caller, dev, (HLD_AVSYNC_MODULE<<24)|(2<<16)|FUNC_AVSYNC_GET_SRCTYPE,
		desc_avsync_p_uint32);
}

RET_CODE avsync_get_status(struct avsync_device *dev, avsync_status_t *pstatus)
{
	if((NULL == dev) || (NULL == pstatus))
    {
        return RET_FAILURE;
    }

	jump_to_func(NULL, os_hld_caller, dev, (HLD_AVSYNC_MODULE<<24)|(2<<16)|FUNC_AVSYNC_GET_STATUS,
		desc_avsync_get_status);
}

RET_CODE avsync_get_statistics(struct avsync_device *dev, avsync_statistics_t *pstatistics)
{
	if((NULL == dev) || (NULL == pstatistics))
    {
        return RET_FAILURE;
    }

	jump_to_func(NULL, os_hld_caller, dev, (HLD_AVSYNC_MODULE<<24)|(2<<16)|FUNC_AVSYNC_GET_STATISTICS,
		desc_avsync_get_statistics);
}

RET_CODE avsync_video_smoothly_play_onoff(struct avsync_device *dev, UINT8 onoff,
	AVSYNC_VIDEO_SMOOTH_LEVEL level, UINT8 interval)
{
	if(NULL == dev)
    {
        return RET_FAILURE;
    }

	jump_to_func(NULL, os_hld_caller, dev, (HLD_AVSYNC_MODULE<<24)|(4<<16)|FUNC_AVSYNC_VIDEO_SMOOTHLY_PLAY_ONOFF, NULL);
}

#endif /* _HLD_DECV_REMOTE */

#endif

