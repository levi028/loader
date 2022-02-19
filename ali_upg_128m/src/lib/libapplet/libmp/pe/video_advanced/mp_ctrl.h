#ifndef _DEC_CTRL_H
#define _DEC_CTRL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "plugin.h"

BOOL get_ctrl_cmd(t2decoder_control *decoder_ctrl);
AF_PE_PLAY_STATE get_next_play_state();
DWORD pestream_start(UINT8 video_stream_type);

extern ID dec_cmd_sema_id;
extern paf_pe_function_table p_dec_func_table;


#ifdef __cplusplus
 }
#endif

#endif

