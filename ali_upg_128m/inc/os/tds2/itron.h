/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: itron.h
*
*    Description:
*   This header file contains the definitions and decalrations of TDS.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _ITRON_H_
#define _ITRON_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <sys_config.h>
#include "alitypes.h"
#include "platform.h"
#include <osal/osal_mm.h>
#include <osal/osal_int.h>
#include <api/libc/debug_printf.h>

#if (SYS_CHIP_MODULE == ALI_S3601)
#define IRQ_NUM                        8
#define EIRQ_NUM                    32
#define TIMER_IRQ                    7
#define IRQ_INTC_NUMBER                2
#define    IRQ_NUMBER_IR                15

#elif (SYS_CHIP_MODULE == ALI_S3602)
	#ifdef ALI_SYS_IO_V2 //C3922, C3702
		#define EIRQ_NUM				192
	#else
	    #if((SYS_CPU_MODULE == CPU_M6303) && \
	        (SYS_PROJECT_FE==PROJECT_FE_DVBT || defined(M3105_CHIP)))//for 3105 or M3105D+3501
	        #define EIRQ_NUM                32
	    #else
	        #define EIRQ_NUM                64
	    #endif
	#endif
#define IRQ_NUM                        8
#define TIMER_IRQ                    7
#define IRQ_INTC_NUMBER                3
#define    IRQ_NUMBER_IR                15
#else
#define IRQ_NUM                        8
#define EIRQ_NUM                    32
#define TIMER_IRQ                    7
#define IRQ_INTC_NUMBER                3
#define    IRQ_NUMBER_IR                15

#endif



#ifdef _DEB

    #define _TDS_TASK_PERFORMANCE    //for task statics
    //#define ZEBU_TEST

    //#define _TDS_INFO_MONITOR    //for tds monitor
    #define _TDS_PERFORMANCE            //
//        #define _TDS_HSR_PERFORMANCE        //for HSR performance measure
//        #define _TDS_STACK_PERFORMANCE        //for stack performance measure
//        #define _TDS_COUNT_PERFORMANCE        //for dispatch and interrupt performance measure
#else // #ifdef _DEB
//    #define ENTER_FUNCTION
//    #define LEAVE_FUNCTION
//    #define ASSERT(exp)
//    #define ASSERT_ADDR(address)
//    #define DEBUGPRINTF    VOIDPRINT
#endif // #ifdef _DEB

    #define    DEBUG_TO_MONITOR    0
    #define    DEBUG_TO_MEMORY    1
    #define    DEBUG_MEMORY_LENGTH    0x8000
    #define    DEBUG_MEMORY_OFFSET    0X20
    //added by jeff wu ,for control debug

#ifdef _TDS_INFO_MONITOR
    #define MONITOR_SYS 0x00000001
    #define MONITOR_TCB 0x00000002
    #define MONITOR_READY 0x00000004
    #define MONITOR_WAIT 0x00000008
    #define MONITOR_MEMORY 0x00000010
    #define MONITOR_FLAG 0x00000020
    #define MONITOR_SIGNAL 0x00000040
    #define MONITOR_SEMAPHORE 0x00000080
    #define MONITOR_MESSAGE 0x00000100
    #define MONITOR_CPU    0x00000200
    #define monitor_task_cpu_measure    0x00000400
    #define OS_ASSERT ASSERT
#else
    #define OS_ASSERT(...)      do{}while(0)
#endif






#define    INVALID_ID      0xFFFF


#define NADR               ((VP)-1)

#define TA_ASM             0    /* Assembler language program */
#define TA_HLNG            1    /* High-level language program */

#define RSN_TIMEOUT        0
#define RSN_BLOCK          1



/* System call Error Code */
#define E_OK               0
#define E_FAILURE        (-1)
#define E_TIMEOUT        (-2)


///////////////////////////////////////////////////////
//         Task interface                 //
///////////////////////////////////////////////////////
/* task priority config */

#define LOW_PRI             31    // Min number, but the highest level
#define HIGH_PRI            0     // Max number,but the lowest level
#define DEF_PRI             20    // default level for normal thread
#define HSR_PRI             10    // HSR level, higher than normal
#define DYN_PRI             18    // Dynamic level, only  once.

#define T_CTHD    T_CTSK

#define TA_TPRI            0x00000001
#define TTW_MPL            0x00001000
#define ref_mpl            i_ref_mpl

///////////////////////////////////////////////////////
//         Event flags interface                 //
///////////////////////////////////////////////////////

#define TA_TFIFO           0          /* FIFO wait queue */
#define TA_WSGL            0x00000000
#define TA_WMUL            0x00000008

#define TWF_ANDW           0x00000001
#define TWF_ORW            0x00000002
#define TWF_CLR            0x00000004

// Mask to clear all flag
#define TDS_FLAG_ALL       0xFFFFFFFF

///////////////////////////////////////////////////////
//        Timer interface                     //
///////////////////////////////////////////////////////

// Timer Type
#define TIMER_ALARM         0x01
#define TIMER_CYCLIC        0x02

#define TMO_POL             0
#define TMO_FEVR            (DWORD)(-1)

#define TCY_OFF         0x00000000          /* Not use */
#define TCY_ON          0x00000001
#define TCY_INI         0x00000002          /* Not use */

#define TTM_ABS         0x00000000
#define TTM_REL         0x00000001


#if 0
#define NESTEDIRQ
#else
#define NO_NESTEDIRQ
#endif

// Special Interface
#define os_thread_sleep    dly_tsk
#define os_task_sleep os_thread_sleep
#define    os_wait_flag(A,B,C,D)        os_wait_flag_time_out(A,B,C,D,TMO_FEVR)
#define    os_test_flag(A,B,C,D)        os_wait_flag_time_out(A,B,C,D,TMO_POL)
#define os_acquire_semaphore(A)        os_acquire_semaphore_time_out(A,TMO_FEVR)


#ifdef _DEBUG_RTOS
    #define KER_PRINTF DEBUGPRINTF
#else
    #define KER_PRINTF(...)
#endif

typedef struct debug_control_block
{
    UINT16    direct;
    UINT32    head;
    UINT32    rear;
    UINT32    length;
    UINT32    index;
} DBGCB;

#ifdef _TDS_TASK_PERFORMANCE
typedef struct st_tcb_mon_info
{
    UINT32 c0_cnt_mon_start;
    UINT32 c0_cnt_sum;
    UINT32 c0_cnt_cur;
    UINT32 mcp_cnt;
    UINT32 mcp_len;
    UINT32 mset_cnt;
    UINT32 mset_len;
    UINT32 mcmp_cnt;
    UINT32 mcmp_len;
    UINT32 dma_cnt;
    UINT32 dma_len;
    UINT32 dma_cnt_vcap;
    UINT32 dma_len_vcap;
    UINT32 dma_wait_cnt;
    UINT32 cache_flush_cnt;
    UINT32 cache_invalde_cnt;
}tcb_mon_info;
#endif
    //added by jeff wu ,for control debug




//////////////////////////////////////////////////////////////////
// Special type definition for ITRON RTOS
typedef short              BOOL_ID;
typedef short              H;
typedef char               B;
typedef char               VB;
typedef short              HNO;
typedef WORD               ID;
typedef ID                 ID_TASK;
typedef ID                 ID_THREAD;
typedef ID                 ID_MODULE;    // Software module ID
typedef ID                 ID_INT;       /* interrupt ID */
typedef ID                 ID_HSR;       /* high service routine ID */
typedef ID                 ID_SIGNAL;
typedef ID                 ID_STRUCTYPE; /* struct type */
typedef short              PRI;

// Common functions
typedef void               (*T_VOID_FUNC_PTR)(void);
typedef void               (*T_SIG_PROC_FUNC_PTR)(ID_SIGNAL, DWORD, DWORD);
typedef void               (*T_HSR_PROC_FUNC_PTR)(DWORD);

typedef unsigned long      ATR;
typedef unsigned long      TMO;
typedef unsigned long      UW;

typedef long               ER;
typedef long               VW;  /* variable data type (32 bit) */

//typedef void               (*FP)(void);
typedef void (*FP)(DWORD, DWORD);
typedef void (*TP)(UINT,UINT);
typedef void (*VOIDFP)(void);


////////////////// Create Fail  //////////////////
enum create_err
{
    _CREATE_TSK_FAIL = 0x10000001,    // Create Task Fail
    _CREATE_FLG_FAIL,                // Create Flag Fail
    _CREATE_SEM_FAIL,                // Create Semaphor Fail
    _CREATE_MSG_FAIL,                // Create Message Fail
    _CREATE_MBX_FAIL,                // Create Mail box Fail
    _CREATE_ALM_FAIL                // Create Timer Fail
};
////////////////// Release Fail  //////////////////
enum delete_err
{
    _DELETE_TSK_FAIL = 0x11000001,     // Delete Task Fail
    _DELETE_FLG_FAIL,                  // Delete Flag Fail
    _DELETE_SEM_FAIL,                  // Delete Semaphor Fail
    _DELETE_MSG_FAIL,                  // Delete Message Fail
    _DELETE_MBX_FAIL,                  // Delete Mail box Fail
    _DELETE_ALM_FAIL,                  // Delete Timer Fail
};

////////////////// Set Fail  //////////////////
enum seting_err
{
    _SET_SEM_FAIL = 0x12000001,     // Set Semaphor Fail
    _SEND_MSG_FAIL,                   // Send Message Fail
    _STA_TSK_FAIL,                    // Start Task Fail
};
////////////////// Wait Fail  //////////////////
enum wait_err
{
    _WAIT_SEM_FAIL = 0x13000001,    // Wait Semaphor Fail
    _WAIT_MSG_FAIL,                   // Receive Message Fail
    _DELAY_TSK_FAIL,                  // Delay Task Fail
};


struct os_config
{

    UINT16 task_num;
    UINT16 sema_num;
    UINT16 flg_num;
    UINT16 mbf_num;
    UINT16 mutex_num;
    UINT16 parity_value;
};


typedef struct t_ctsk
{
    FP      task;
    PRI     itskpri;
    INT32   stksz;
    UINT32  quantum;
    UINT32  para1;
    UINT32  para2;
    char    name[3];
} T_CTSK;


typedef TMO                DLYTIME;        /* delay time */




///////////////////////////////////////////////////////
//        Part II        memory pool interface                  //
///////////////////////////////////////////////////////

typedef    struct t_cmpl
{
    VP    exinf;                // extended information
    ATR    mplatr;
    INT    mplsz;                // size of request
} T_CMPL;

typedef struct t_rmpl
{
    BOOL_ID    wtsk;
    INT    frsz;
    INT    maxsz;
} T_RMPL;





///////////////////////////////////////////////////////
//        Part V        Message buffer interface             //
///////////////////////////////////////////////////////
typedef enum
{
    MSG_TYPE_INPUT,
    MSG_TYPE_SIGNAL,
    MSG_TYPE_DEVSTA,
    MSG_TYPE_NVINFO,
    MSG_TYPE_PAUSE,
    MSG_TYPE_RESUME,
    MSG_TYPE_UPDATE,
    MSG_TYPE_CLOSEWIN,//add by sunny 0818
    MSG_TYPE_REDRAWWIN,//add by sunny 0818
    MSG_TYPE_HSR,            //add by jeff 0820
    MSG_TYPE_EXIT,//add by sunny 1029
    MSG_TYPE_SYSTEM,    // Singer for System Message
    MSG_TYPE_ATAPI_INFO,    // tom for atapi tray-closed message
}MSG_TYPE;


typedef struct t_cmbf
{
    INT     bufsz;
    INT     maxmsz;
    char    name[3];    //added by jeff wu 2003-05-27
} T_CMBF;

typedef struct tag_msg_head
{
    MSG_TYPE    b_msg_type;        // Message type
    ID_MODULE    b_module_id;        // Module ID of the sender
}MSG_HEAD, *PMSG_HEAD;


typedef struct
{
    ID_MODULE   mod_id;            //sender module ID
    MSG_TYPE    msg_type;        //sender message Type
    DWORD       d_para;            // info 1
    DWORD       w_para;            // info 2,should be dPara2
    WORD        w_id;            //ext ID info
    TMO         sys_time;        //current time,ms based
}TMSG,*LPTMSG;



///////////////////////////////////////////////////////
//        Part VI        Timer interface                     //
///////////////////////////////////////////////////////


typedef void                (*TMR_PROC)(UINT);

typedef struct t_timer
{
    TMR_PROC              callback;
    UINT16               type;
    TMO                time;
    UINT                param;
    char                name[3];    //added by jeff wu 2003-05-27
} T_TIMER;



typedef void    (*ISR_PROC)(UINT);

#ifdef _DEB
    //extern UINT g_code_begin;
    //extern UINT g_code_end;
#endif

__ATTRIBUTE_RAM_ ID os_get_current_thread_id(void);
ID_THREAD os_create_thread(T_CTHD * pk_cthd);
ID_THREAD os_create_thread_ex(T_CTHD * pk_cthd,void* stack_addr);
ER dly_tsk(DLYTIME dlytim);
void os_exit(ER exit_code);
ER os_delete_thread(ID_THREAD thd_id);
ER os_disable_dispatch(void);
ER os_enable_dispatch(void);
void create_server_task(void);




void tds_mem_mon_begin(void);       // start the monitor;
void tds_mem_mon_stop(void);        // stop and release the monitor, but doesn't
                                   // de-allocate the leak blocks
/* Function Description: [TDS_MemMon_LeakOccurs]
     Check whether memory leak occurs in current check point;
   Return Value:
     If return 0, none leak occurs;
     Otherwise, the number of no-freed blocks, always positive;
 */
int  tds_mem_mon_leak_occurs(void);  // if return 0, no leak; otherwise,
                                   // number of leak blocks; always positive
void tds_mem_mon_dump_leak(void);    // Dump the no-freed block;
void tds_mem_mon_dump_cur_stat(void); // Dump current no-freed block;



// Common Interface
VP os_mem_alloc(INT blksz);
ER os_mem_free(VP blk);
//Special Interface
//for un-cached memory allocation & free
VP os_mem_alloc_syn(INT blksz);
ER os_mem_free_syn(VP blk);



// Common Interface
ID  os_create_flag(UINT32 flgptn);
ER os_del_flag(const ID flgid);
__ATTRIBUTE_RAM_ ER os_wait_flag_time_out(UINT* p_flgptn, ID flgid,\
                                  UINT waiptn, UINT wfmode, TMO tmout) ;
__ATTRIBUTE_RAM_ ER os_set_flag(ID flgid, UINT setptn);
__ATTRIBUTE_RAM_ ER os_clear_flag(ID flgid, UINT clrptn);




// Common Interfaces
ID os_create_semaphore(INT32 semcnt);
ER os_del_semaphore(ID semid);
__ATTRIBUTE_RAM_ ER os_acquire_semaphore_time_out(ID semid,TMO tmout);
__ATTRIBUTE_RAM_ ER os_free_semaphore(ID semid);
__ATTRIBUTE_RAM_ ER os_free_semaphore2(ID semid);

void os_enter_critical_section(void);
void os_leave_critical_section(void);

// Common Interface
ID os_create_msg_buffer(T_CMBF *pk_cmbf);
ER os_del_message_buffer(ID mbfid);
__ATTRIBUTE_RAM_ ER os_send_msg_time_out(ID mbfid, VP msg, INT msgsz, TMO tmout);
__ATTRIBUTE_RAM_ ER os_get_msg_time_out(VP msg, INT *p_msgsz, ID mbfid, TMO tmout);
//Special Interface
ER os_get_message(VP, INT *, ID);
ER os_peek_message(VP, INT *, ID);
ER os_send_message(ID, VP, INT);
ER os_post_message(ID, VP, INT);



// Common Interface
ID os_create_timer (T_TIMER* pk_dalm);
ER os_delete_timer(ID tmr_id);
ER os_set_timer(ID tmr_id,UINT32 new_val);
ER os_activate_timer(ID tmr_id,BOOL b_val);
DWORD os_get_tick_count(void);
void os_delay(WORD us);
DWORD os_get_time(void);



ER os_register_isr(UINT16 irq, ISR_PROC irq_rsp, UINT32 param);
ER os_delete_isr(UINT16 irq, ISR_PROC isr_func);
__ATTRIBUTE_RAM_ ER os_register_hsr(T_HSR_PROC_FUNC_PTR p_hsr_func ,DWORD param);
__ATTRIBUTE_RAM_ void os_disable_interrupt(void);
__ATTRIBUTE_RAM_ void os_enable_interrupt(void);
void irq_initialize(void);

unsigned long ref_ie(void);

UINT32 osal_tds2_cache_flush(void *src, UINT32 len);
UINT32 osal_tds2_cache_flush_all(void);
UINT32 osal_tds2_cache_invalidate(void *src,UINT32 len);

UINT32  read_CP0_config2(void);


ID os_create_mutex(void);
ER os_delete_mutex(ID meid);
ER os_lock_mutex(ID meid,TMO tmout);
ER os_unlock_mutex(ID meid);
ID osal_dual_mutex_create(void);
ER osal_dual_mutex_delete(ID meid);
ER osal_dual_mutex_lock(ID meid,TMO tmout);
ER osal_dual_mutex_unlock(ID meid);
void osal_dual_spinlock(ID meid);
void osal_dual_spinunlock(ID meid);

__ATTRIBUTE_REUSE_ void _INIT(void);
void tds_app_init(void);
BOOL osal_dual_is_private_address(void *dest);
void ali_debug_enable(void);
BOOL module_attach(UINT32 param);
void os_remote_cb_send_msg(T_HSR_PROC_FUNC_PTR cb_func, UINT32 param);
void dram_access_protect_enable(void);
BOOL module_func(UINT32 func, UINT32 param);
void run_time_integrity_check(void);

#ifdef SEE_CPU
void config_split(void);
void sys_ic_enable_io_private(void);
void ver_check(void);
void hw_monitor_init(UINT16 lower_pll, UINT16 higher_pll);
#endif

#if 0
    #define ali_trace(a)  _ali_trace(a, __FUNCTION__, __LINE__)
#else
    void ali_trace(void *pmsg);
#endif

//#include <osal/osal_mm.h>
//#include <osal/osal_int.h>


#ifdef __cplusplus
}
#endif










#endif /* _ITRON_H_ */



