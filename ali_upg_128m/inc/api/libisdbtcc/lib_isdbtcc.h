#ifndef __LIB_ISDBTCC_H__
#define __LIB_ISDBTCC_H__

#include <basic_types.h>
//#define    LIB_ISDBTCC_DEBUG

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef LIB_ISDBTCC_DEBUG
    #define ICC_PRINTF libc_printf
    #define USB_PRINTF soc_printf
#else
    #define ICC_PRINTF(...)
    #define USB_PRINTF(...)
#endif

#define CCDEC_ASSERT(x)         ICC_PRINTF("Assert!\n")

#define ISDBTCC_DMX_SERV_ID    4
struct t_isdbtcc_lang
{
    UINT16 pid;    //0 mean invalid
    UINT8 es_com;
    UINT8 lang[3];    //ex "por"
    UINT8 b_dmf;
    UINT8 b_timing;
};

enum isdbtcc_event
{
    ISDBTCC_PID_UPDATE = 1,
};

typedef void (*ISDBTCC_EVENT_CALLBACK)(enum isdbtcc_event event, UINT32 param);
#define ISDBTCC_LANG_NUM 8

INT32 isdbtcc_eit_data_content_desc(UINT8* desc_buf, UINT16 buf_len);
INT32 isdbtcc_stream_iden_desc(UINT8 tag, INT32 desc_length, UINT8 *desc, void *param);
INT32 isdbtcc_data_comp_desc(UINT8 tag, INT32 desc_length, UINT8 *desc, void *param);
void isdbtcc_get_language(struct t_isdbtcc_lang** list ,UINT8* num);
UINT8 isdbtcc_get_cur_language(void);
INT32 isdbtcc_set_language(UINT8 lang);
INT32 isdbtcc_enable(BOOL enable);
INT32 isdbtcc_show_onoff(BOOL b_on);
INT32 isdbtcc_register(UINT32 monitor_id);
INT32 isdbtcc_unregister(UINT32 monitor_id);
#if 1
void isdbtcc_pvr_set_language(struct t_isdbtcc_lang* list ,UINT8 num);
INT32 isdbtcc_pvr_enable(BOOL enable,UINT16 dmx_id);
INT32 isdbtcc_register(UINT32 monitor_id);
INT32 isdbtcc_unregister(UINT32 monitor_id);
void isdbtcc_reg_callback(ISDBTCC_EVENT_CALLBACK callback);
void isdbtcc_unreg_callback();
#endif
BOOL isdbtcc_is_available();

void lib_isdbtcc_init();
void osd_isdbtcc_enter();
void osd_isdbtcc_leave();
#ifdef __cplusplus
}
#endif

#endif /*__LIB_ISDBTCC_H__ */
