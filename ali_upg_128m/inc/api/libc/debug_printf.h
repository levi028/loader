#ifndef _LIB_C_DEBUG_PRINTF_H
#define _LIB_C_DEBUG_PRINTF_H


#ifdef __cplusplus
extern "C"
{
#endif

#define FIXED_PRINTF            libc_printf
#define ERRMSG                    DEBUGPRINTF

#ifdef _DEBUG_VERSION_
#ifdef _DEBUG_PRINTF_
#define PRINTF                    libc_printf
#define DEBUGPRINTF                libc_printf
#else
#define PRINTF(...)                do{}while(0)
#define DEBUGPRINTF(...)        do{}while(0)
#endif  /* _DEBUG_PRINTF_ */

#else    /* _DEBUG_VERSION_ */
//extern PRINTF(const char *fmt, ...);    /* This is for Able project only */
#ifdef _DEBUG_PRINTF_
#define PRINTF                    libc_printf
#define DEBUGPRINTF                libc_printf
#else
#define PRINTF(...)                do{}while(0)
#define DEBUGPRINTF(...)        do{}while(0)
#endif  /* _DEBUG_PRINTF_ */

#define ENTER_FUNCTION    DEBUGPRINTF("Function %s: Begin.\n",__FUNCTION__)
#define LEAVE_FUNCTION    DEBUGPRINTF("Function %s: End From %d line. \n", __FUNCTION__, __LINE__)

#endif    /* _DEBUG_VERSION_ */


//================================================================================================
//ASSERT defines
#ifdef ASSERT
#undef ASSERT
#endif

#ifdef _DEBUG_PRINTF_
#define    ASSERT(expression)                \
    {                                    \
        if (!(expression))                \
        {                                \
            DEBUGPRINTF("assertion(%s) failed: file \"%s\", line %d\n",    \
                #expression, __FILE__, __LINE__);    \
            SDBBP();                    \
        }                                \
    }

#define ASSERTMSG(expression, message)    \
    {                                    \
        if (!(expression))                \
        {                                \
            DEBUGPRINTF("assertion(%s) failed: file \"%s\", line %d, message %s\n", \
                #expression, __FILE__, __LINE__, #message);    \
            SDBBP();                    \
        }                                \
    }

#define    ASSERT_ADDR(address)            \
    {                                    \
        if((UINT32)(address)<0x010||((UINT32)(address)>=0x0180&&(UINT32)(address)<0x01a0)) \
        {                                \
            DEBUGPRINTF("assertion address  failed: [%s]=[0x%08x] in file \"%s\", line %d\n", \
                    #address,(address),__FILE__, __LINE__); \
            SDBBP();                    \
        }                                \
    }
#else

#ifdef _ALI_SYSDBG_ENABLE_
#define    ASSERT(expression)                \
    {                                    \
        if (!(expression))                \
        {                                \
            sysdbg_out_cb("assertion(%s) failed: file \"%s\", line %d\n",    \
                #expression, __FILE__, __LINE__);    \
            SDBBP();                    \
        }                                \
    }

#define ASSERTMSG(expression, message)    \
    {                                    \
        if (!(expression))                \
        {                                \
            sysdbg_out_cb("assertion(%s) failed: file \"%s\", line %d, message %s\n", \
                #expression, __FILE__, __LINE__, #message);    \
            SDBBP();                    \
        }                                \
    }

#define    ASSERT_ADDR(address)            \
    {                                    \
        if((UINT32)(address)<0x010||((UINT32)(address)>=0x0180&&(UINT32)(address)<0x01a0)) \
        {                                \
            sysdbg_out_cb("assertion address  failed: [%s]=[0x%08x] in file \"%s\", line %d\n", \
                    #address,(address),__FILE__, __LINE__); \
            SDBBP();                    \
        }                                \
    }
#else

#define    ASSERT(expression)                \
    {                                    \
        if (!(expression))                \
        {                                \
            sys_watchdog_reboot();        \
        }                                \
    }

#define ASSERTMSG(expression, message)    \
    {                                    \
        if (!(expression))                \
        {                                \
            sys_watchdog_reboot();        \
        }                                \
    }

#define    ASSERT_ADDR(address)            \
    {                                    \
        if((UINT32)(address)<0x010||((UINT32)(address)>=0x0180&&(UINT32)(address)<0x01a0)) \
        {                                \
            sys_watchdog_reboot();        \
        }                                \
    }
#endif /*_ALI_SYSDBG_ENABLE_*/

#endif /*_DEBUG_PRINTF_*/


//================================================================================================
//printf group functions

#if (defined BL_DISABLE_PRINTF || defined DISABLE_PRINTF)
//#define libc_printf do{}while(0);
#define libc_printf(...)
#define dbg_printf(...)
#else
void libc_printf(const char *fmt, ...);
#endif

void soc_printf(const char *fmt, ...);

#define GOS_DEBUG
#ifdef GOS_DEBUG
#define LDR_PRINT(fmt, ...)  do{ libc_printf("[LOADER]->[%s %d] "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);}while(0)
#define LDR_ERR(fmt, ...)    do{ libc_printf("[LOADER]->[%s %d][error] "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);}while(0)
#else
#define LDR_PRINT(fmt, ...)  do{}while(0)
#define LDR_ERR(fmt, ...)    do{}while(0)
#endif

#ifdef __cplusplus
 }
#endif


#endif /* _LIB_C_DEBUG_PRINTF_H */
