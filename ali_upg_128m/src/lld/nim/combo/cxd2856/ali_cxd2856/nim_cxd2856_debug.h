#ifndef _NIM_CXD2856_DEBUG_H_
#define _NIM_CXD2856_DEBUG_H_

/**
	MONITOR_DEBUG : control switch of the debug task
	if you enable it,will create task to call the monitor function and print it(per,snr,agc,rf,cn.....) 
**/
#define MONITOR_DEBUG 0

#if 0
#define CXD2856_LOG libc_printf
#else
#define CXD2856_LOG(...)
#endif
#if 0
#define CXD2856_LOG(param, fmt, args...) \
    do{\
        if ( param->output_buffer && param->fn_output_string )\
        {\
            sprintf(param->output_buffer, fmt, ##args);\
            param->fn_output_string(param->output_buffer);\
        };\
    }while(0)
#endif

INT32 nim_cxd2856_monitor_task_init(struct nim_device *dev);
#endif
