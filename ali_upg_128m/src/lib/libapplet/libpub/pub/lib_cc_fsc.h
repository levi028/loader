#ifndef __LIB_CC_FSC_H
#define __LIB_CC_FSC_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <sys_config.h>

#ifdef FSC_SUPPORT
#include <api/libpub/lib_frontend.h>

////////////////////////////////////////////////////////
#if 0//def FSC_CC_PRT
    #define FSC_CC_PRINTF          libc_printf
#else
    #define FSC_CC_PRINTF(...)     do{}while(0)
#endif

#undef offsetof
#undef container_of
#define offsetof(st, m) \
     ((UINT32) ( (char *)&((st *)(0))->m - (char *)0 ))

#define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type,member) );})

////////////////////////////////////////////////////////
INT8 fsc_stop_mode_get(void);
void fsc_stop_mode_set(INT8 stop_mode);
INT32 fsc_channel_chanege_act(struct ft_frontend *ft, struct cc_param *param, enum fsc_state_machine state,UINT32 cmd_bits);
INT32 fsc_channel_stop(UINT32 cmd, struct cc_es_info *es, struct cc_device_list *dev_list);
void set_fsc_vdec_first_show(struct vdec_device *vdec);

#endif //FSC_SUPPORT

#ifdef __cplusplus
}
#endif // extern "C"
#endif //__LIB_CC_FSC_H
