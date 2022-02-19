/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
*    disclosed to unauthorized individual.
*    File:     printf.h
*
*    Description:This file contains all functions definition of LIBC printf.
*
*    History:
*           Date            Athor        Version          Reason
*        ============    =============    =========    =================
*    1.    2002.??.??      Justin Wu       Ver 0.1     Create file.
*    2.    2003.12.05      Justin Wu       Ver 0.2     Support wDVD Standard TDS
*                                                    -- Common Service:
*                                                    .ERRMSG & DEBUGPRINTF
*                                                    .ASSERTMSG & ASSERT_ADDR
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/


#ifndef _LIB_C_PRINTF_H
#define _LIB_C_PRINTF_H

#include <sys_config.h>
#include <osal/osal.h>
#include <hal/machine.h>
#include <asm/chip.h>
#include <stdarg.h>
#include "debug_printf.h"

#ifdef __cplusplus
extern "C" 
{
#endif

#define MODULE_CNT_MAX (50)
//================================================================================================
//macro define for the printf functons


struct arg_printf
{
    void *data;
    int (*put)(void*, unsigned int, void*);
};

struct arg_scanf
{
  void *data;
  int (*getch)(void*);
  int (*putch)(int,void*);
};



////////////////////////////////////////////
/*add for debug printf by ray ---start*/

typedef enum en_module_name
{
    MDL_OS=0,
    MDL_DECA,
    MDL_SND,
    MDL_DECV,
    MDL_PVR,
    MDL_DMX0,
    MDL_DMX1,
    MDL_DMX2,
}module_id;

typedef struct st_module4prt
{
    module_id mdl_id;
    UINT32 ui_prt_level;
}module4prt;


UINT32 drv_set_module_print_level(module_id mdlid,UINT32 uiprtlevel);
/*add for debug printf by ray ---end*/
///////////////////////////////////////////

//__v_printf
int __v_printf(struct arg_printf *fn, const char *format, va_list arg_ptr);
/*
* Name  : vsprintf
*
* Return: 
*   the string length, not include '\0': normal return.
*   negative value: abnormal return.
*/
int vsprintf(char *dest, const char *format, va_list arg_ptr);

/*
* Name  : vsnprintf
* Description: put the output string into str.
*  Return value 
       Upon successful return, these functions return the number of characters 
       printed  (not  including  the  trailing  ¡¯/0¡¯  used  to  end  output to 
       strings).  The functions snprintf() and vsnprintf() do not  write  more 
       than size bytes (including the trailing ¡¯/0¡¯). If the output was trun- 
       cated due to this limit then the return value is the number of  charac- 
       ters (not including the trailing ¡¯/0¡¯) which would have been written to 
       the final string if enough space had been  available.  Thus,  a  return 
       value  of  size  or more means that the output was truncated. (See also 
       below under NOTES.)  If an output  error  is  encountered,  a  negative 
       value is returned.
*/
int vsnprintf(char *str, unsigned int size, const char *format, va_list arg_ptr);

/*
* Name  :    sprintf
* Description: 
* Return: 
*   the string length, not include '\0': normal return.
*   negative value: abnormal return.
*/
int sprintf(char *dest, const char *format, ...);

/*
* Name  : vprintf
*
* Return: 
*   the string length, not include '\0': normal return.
*   negative value: abnormal return.
*/
int vprintf(const char *format, va_list ap);

/*
* Name  : snprintf
*
* Return: 
*   the string length, not include '\0': normal return.
*   negative value: abnormal return.
*/
int snprintf(char *str, unsigned int size, const char *format, ...);

/*
* Name  : aprintf
*
* Return value: 
*/
char* aprintf(const char *format, ...);

/*
* Name  : vaprintf
*
* Return: 
*/
char* vaprintf(const char *fmt, va_list arg_ptr);
//int asprintf(char **s, const char *format, ...);

/*
* Name  : __v_scanf
*
* Return: 
*/
int __v_scanf(struct arg_scanf *fn, const char *format, va_list arg_ptr);

/*
* Name  : vsscanf
*
* Return: 
*/
int vsscanf(const char *str, const char *format, va_list arg_ptr);

/*
* Name  : sscanf
*
* Return: 
*/
int sscanf(const char *str, const char *format, ...);

/*
* Name  : outbyte
*
* Return: 
*/
int outbyte(char ch);


/**
* id = SCI_FOR_RS232 or id = SCI_FOR_MDM
*/
void libc_printf_uart_id_set(UINT8 id);

/*
* Name  : set_dsh_task_id
*
* Return value: NONE
*/
void set_dsh_task_id(ID task_id);

/*
* Name  : set_switch_display_mode
*
* Return value:  NONE
*/
void set_switch_display_mode(UINT8 display_mode);

#ifdef _DBG_PRINT_DMA //((!defined BL_DISABLE_PRINTF)&&(!defined DISABLE_PRINTF))
#define DBG_DMA_PRINT_BUF_LEN   (1024*200)
extern UINT32 g_prt2mem_mod;
extern UINT32 g_prt2mem_level;
enum drv_dbg_prt_mod_en {

    DRV_DBG_MOD_DMX_COMM,
    DRV_DBG_MOD_DMX_AVSYNC,
    DRV_DBG_MOD_DMX_LIVE,
    DRV_DBG_MOD_DMX_PVR,
    DRV_DBG_MOD_SND,
    DRV_DBG_MOD_SND_AVSYNC,
    DRV_DBG_MOD_SED_TASK,
    DRV_DBG_MOD_VSC,

}drv_dbg_prt_mod;

struct st_ring_buf {
    unsigned char *buffer_addr;
    unsigned long buffer_size;
    unsigned long reserve_size;
    unsigned long data_len;
	volatile struct share_wt_rd_pointer *prw;
	OSAL_ID mutex;
};


#endif
            
#ifdef DUAL_ENABLE
    #ifndef MAIN_CPU
        #ifdef  _DBG_PRINT_DMA // ((!defined BL_DISABLE_PRINTF)&&(!defined DISABLE_PRINTF))

            #define prt_2mem(mod, level, fmt, args...) do { \
                                                              if((g_prt2mem_mod & (1<<mod))&&(g_prt2mem_level & (level))) { \
                                                                  _prt_2mem(fmt, ##args); \
                                                              } \
                                                          } while(0)
                                                      
        #else
            #define prt_2mem(...) do{}while(0)

        #endif
    #else
        #define prt_2mem(...) do{}while(0)
    #endif
#else 
    #define prt_2mem(...) do{}while(0)
#endif
#ifdef __cplusplus
 }
#endif


#endif /* _LIB_C_PRINTF_H */
