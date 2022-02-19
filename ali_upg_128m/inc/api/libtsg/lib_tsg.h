#ifndef __LIB_TSG_H__
#define __LIB_TSG_H__
#include <types.h>
#include <sys_config.h>

#ifdef __cplusplus
extern "C"
{
#endif

// interfaces for CC TSG task
void cc_tsg_task_init(void);
RET_CODE cc_tsg_task_start(UINT32 prog_id);
RET_CODE cc_tsg_task_stop(void);

BOOL cc_tsg_task_is_running(void);


#ifdef __cplusplus
}
#endif

#endif /* __LIB_TSG_H__ */


