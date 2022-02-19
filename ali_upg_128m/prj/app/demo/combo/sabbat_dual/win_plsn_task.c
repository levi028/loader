#include <types.h>
#include <osal/osal.h>
#include <sys_config.h>
#include <api/libc/string.h>
#include <api/libc/printf.h>
#include <hld/nim/nim.h>
#include <hld/nim/nim_dev.h>

#ifdef PLSN_SUPPORT
#if 0     //remove to sys_memory
#define __MM_TABLE_LEN		0x319A00
#define __MM_CAPTURE_LEN	0x400000
#if (defined(M36F_SINGLE_CPU) && defined(SUPPORT_64M_MEM))
#define __MM_CAPTURE_ADDR 	(__MM_PVR_VOB_BUFFER_ADDR & 0x8FFFFFFF)
#define __MM_TABLE_ADDR		((__MM_VBV_START_ADDR&0x8FFFFFFF))
#else
#define __MM_CAPTURE_ADDR 	((__MM_PVR_VOB_BUFFER_ADDR + __MM_AUTOSCAN_DB_BUFFER_LEN) & 0x8FFFFFFF)
#define __MM_TABLE_ADDR		((__MM_CAPTURE_ADDR + __MM_CAPTURE_LEN) & 0x8FFFFFFF)
#endif
#endif

extern struct nim_device  *g_nim_dev;
static OSAL_ID plsn_task_id = OSAL_INVALID_ID;
//static BOOL plsn_task_running = FALSE;


void plsn_table_release()
{
    nim_io_control(g_nim_dev, NIM_DRIVER_RELEASE_TABLE, 0);
}


static void plsn_table_generate(void)
{
	struct ali_plsn_address plsn_addr;

	MEMSET(&plsn_addr, 0, sizeof(struct ali_plsn_address));
	plsn_addr.capture_address = __MM_CAPTURE_ADDR;
	plsn_addr.table_address = __MM_TABLE_ADDR;
    nim_io_control(g_nim_dev, NIM_DRIVER_GENERATE_TABLE, &plsn_addr);
}


static BOOL plsn_task_init(void)
{
	T_CTSK t_ctsk;

	t_ctsk.task = (FP)plsn_table_generate;
	t_ctsk.stksz = 0x4000;
	t_ctsk.quantum = 10;
	t_ctsk.itskpri = OSAL_PRI_NORMAL;
	t_ctsk.name[0] = 'P';
	t_ctsk.name[1] = 'L';
	t_ctsk.name[2] = 'S';
	plsn_task_id = osal_task_create(&t_ctsk);
	if (plsn_task_id == OSAL_INVALID_ID)
	{
    	PRINTF("cre_tsk plsn_task_id failed in %s\n", __FUNCTION__);
    	return FALSE;
	}

	return TRUE;	
}


__ATTRIBUTE_REUSE_
RET_CODE plsn_table_init()
{	
	INT32 ret = RET_SUCCESS;
	
	if (plsn_task_init() != TRUE)
	{
		ret = RET_FAILURE;
	}

	return ret;
}
#endif
