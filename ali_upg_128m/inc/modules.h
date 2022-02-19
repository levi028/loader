/*****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2004 Copyright (C)
 *
 *  File: modules.h
 *
 *  Contents:     This file define modules remote called.
 *  History:
 *        Date        Author              Version     Comment
 *        ==========    ==================    ==========     =======
 *  1.  05/07/2009  Wen Liu             0.1.000     Initial
 *
 *****************************************************************************/
#ifndef __MODULES_H__
#define __MODULES_H__

#ifdef __cplusplus
extern "C"
{
#endif

typedef void (*NORMAL_CALLEE)(UINT8 *);

#define RPC_MSG_NORMAL    0
#define RPC_MSG_RETURN    1
#define RPC_MSG_WAIT      2
#define RPC_MSG_FAILURE   3

struct remote_call_msg
{
    UINT32 size:30;
    volatile UINT32 flag:2;
    UINT8  func;
    UINT8  pri;
    UINT8  resv;
    UINT8  module;
    UINT32 caller;
    UINT32 retbuf;
    UINT32 parasize;
    UINT32 para0;
    UINT32 para1;
    UINT32 para2;
    UINT32 para3;
};

struct remote_response_msg
{
    UINT32 size:30;
    UINT32 flag:2;
    UINT32 ret;
    UINT32 caller;
    UINT32 retbuf;
};

struct remote_caller_private
{
	UINT32 mailbox_out;
	UINT32 mailbox_in;
};

struct remote_callee_private
{
	UINT32 mailbox_out;
	UINT32 mailbox_in;
};

struct remote_call_private
{
	UINT32 int_tx;
	UINT32 shm_out;
	UINT32 shm_in;
	struct remote_caller_private remote_caller_private_data;
	struct remote_callee_private remote_callee_private_data;	
};

struct remote_call_type_private
{
	UINT32 cpu_id:16;
	UINT32 rpc_type:16;//call or ret
};

struct remote_ret_private
{
	UINT32 rpc_id:16;
	UINT32 call_flag:8;
	UINT32 ret_flag:8;
	UINT32 int_tx;
	UINT32 mailbox_out;
	UINT32 mailbox_in;
	UINT32 sharemem_send;
	UINT32 sharemem_receive;
};

//Serialize structure
//被串行化结构的描述
struct desc_storage_struct
{
    UINT16 size;  //size of this structure 结构大小
    UINT8  flag;  //flag of this structure 结构标志
    UINT8  ind;   //index in out para list
    UINT32 off;   //offset of message buffer for storage 结构的存储位置
    UINT32 wp;    //write back address for out parameter
};
//指向被串行化结构的指针的描述
//不支持不对齐的指针
struct desc_pointer_struct
{
    UINT16 pflag:2; //0: pointer of this structure is a parameter
                    //1: pointer of this structure is member of another structure
    UINT16 poff:14; //pointer offset if pflag == 1, max struct 16K
                    //index of parameter list if  pflag == 0
    UINT8  pind;    //index of desc_struct list for pointer
    UINT8  sind;    //index of desc_struct list for pointed
};

//Attribute of pointer
#define DESC_P_IS_PARA(x)    (((x)&0x1)  == 0)
#define DESC_P_IS_MEMBER(x)  (((x)&0x1)  == 1)
#define DESC_P_IS_RET(x)     (((x)&0x1)  == 0)

#define DESC_STRU_IS_OUTPUT(x) (((x)&0x1) == 1)
#define DESC_STRU_IS_CONSTR(x) (((x)&0x2) == 2)
#define DESC_STRU_IS_LIST(x)   (((x)&0x4) == 4)
#define DESC_STRU_IS_DYNAMIC(x)   ((x)&0xe)

#define TYPE_OUTPUT            1
#define TYPE_STRING            2
#define TYPE_LIST              4
#define TYPE_SIZE_UB           0x10
#define TYPE_SIZE_UH           0x20
#define TYPE_SIZE_UW           0x00

#define DESC_P_PARA(ind, pind, sind)       ((pind<<16)|(sind<<24))
#define DESC_P_STRU(ind, pind, sind, poff) ((pind<<16)|(sind<<24)|(poff<<2)|1)
#define DESC_STATIC_STRU(ind, size)        (size), 0, 0
#define DESC_OUTPUT_STRU(ind, size)        (size|0x10000), 0, 0
#define DESC_DYNAMIC_STRU(ind, flag, off)    ((flag)<<16|size), off, 0
#define DESC_STATIC_STRU_SET_SIZE(desc, ind, size) ((UINT32 *)desc)[1 + 3*ind]=size
#define DESC_OUTPUT_STRU_SET_SIZE(desc, ind, size) ((UINT32 *)desc)[1 + 3*ind]=0x10000|size
//For example para0->xxx->yyy->zzz
/*UINT32 desc_pointer_func1[] =
{
  3, DESC_STRU(sizeof(struct), DESC_STRU(sizeof(struct)), DESC_STRU(sizeof(struct)) \
  3, DESC_P_PARA(0, 0),        DESC_P_STRU(0, 1, off0),   DESC_P_STRU(1, 2, off1)  \
  0,
};
*/

//DESC_DYNAMIC_STRU(0, TYPE_OUTPUT|TYPE_LIST|TYPE_SIZE_UB, offset)
//  jump to void module_caller(msg, fp, funcdesc, desc)
//  return;
//  call_para_serialize(msg,desc,fp, funcdesc)
//  void OS_RemoteCall(msg, size)
//  return;

#define normal_para_serialize(x)   asm volatile ("sw $5, 4(%0); sw $6, 8(%0); sw $7, 12(%0);"::"r"(x))
#define normal_ret_unserialize(x)  asm volatile ("lw $2, 4(%0);"::"r"(x))


//Save all parameters into frame stack and then call function to serialize all into call msg
/*#define jump_to_func(msg, func, para0,funcdesc,desc)  \
                                   do{     \
                                        volatile unsigned long null; \
                                        register unsigned long *fp asm ("$8") = (unsigned long *)&para0; \
                                     asm volatile ("sw $5, 4(%0); sw $6, 8(%0); sw $7, 12(%0);"::"r"(fp));\
                                     func(msg, fp,funcdesc,desc);       \
                                     null = 0; \
                                  }while(0)*/
/*
#define jump_to_func(msg, func, para0,funcdesc,desc)  \
                                   do{     \
                                        volatile unsigned long null[4]; \
                                            register unsigned long a0 asm ("$4"); \
                                          register unsigned long a1 asm ("$5"); \
                                          register unsigned long a2 asm ("$6"); \
                                          register unsigned long a3 asm ("$7"); \
                                          unsigned long * fp = (unsigned long *)&para0; \
                                          fp[0] =  a0; \
                                          fp[1] =  a1; \
                                          fp[2] =  a2; \
                                          fp[3] =  a3; \
                                          func(msg, fp,funcdesc,desc);       \
                                          null[0] = 0; \
                                  }while(0)
 */
 #define BUILTIN_APPLY_ARGS
 #define jump_to_func(msg, func, para0,funcdesc,desc)  \
                                   do{     \
                                        unsigned long *fp = __builtin_apply_args();\
                                        unsigned long ret = 0; \
                                        ret = func(msg, fp,funcdesc,desc);       \
                                         __builtin_return(&ret);            \
                                  }while(0)
//__builtin_apply(ssss, __builtin_apply_args(), 7);

void para_serialize(UINT8 *msg, UINT8 *desc, UINT32 *fp, UINT8 parasize);
void ret_serialize(UINT8 *msg, UINT8 *desc, UINT32 ret);
UINT8 *malloc_in_stack(void);
UINT32 *malloc_sm(UINT32 len);
void free_sm(UINT32 *fptr, UINT32 len);
UINT32 os_hld_caller(UINT8 *msg, UINT32 *fp, UINT32 func_desc, UINT32 *api_desc);
void os_hld_callee(UINT32 entry_func, UINT8 *msg);
BOOL is_rpc_ready(void);

enum REMOTE_MODULES{
HLD_BASE_MODULE,
HLD_DECA_MODULE,
HLD_SND_MODULE,
HLD_VP_MODULE,
LLD_DECA_M36F_MODULE,
LLD_SND_M36F_MODULE,
LLD_VP_M36F_MODULE,
LLD_VBI_M33_MODULE,
LLD_DMX_M36F_MODULE,
LLD_DSC_M36F_MODULE,
LLD_CRYPTO_M36F_MODULE,
HLD_DSC_MODULE,
HLD_CRYPTO_MODULE,
HLD_DECV_MODULE,
LLD_DECV_M36_MODULE,
LLD_DECV_AVC_MODULE,
LIB_PE_ADV_VIDEO_ENGINE_MODULE,
LIB_PE_MUSIC_ENGINE_MODULE,
LIB_PE_IMAGE_ENGINE_MODULE,
LLD_TRNG_M36F_MODULE,
LIB_CLOSE_CAPTION_MODULE,
LLD_SBM_MODULE,
HLD_PVR_MODULE,
HLD_AVSYNC_MODULE,
LLD_AVSYNC_MODULE,
LLD_VSC_MODULE,
};

//dual remote caller
extern UINT32 g_remote_callee[];

#ifdef __cplusplus
}
#endif

#endif    //__MODULES_H__

