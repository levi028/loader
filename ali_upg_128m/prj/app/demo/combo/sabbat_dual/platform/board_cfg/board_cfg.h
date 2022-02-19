#include <hld/nim/nim_tuner.h>
#include "../board.h"
#ifndef _BOARD_CFG_H_
#define _BOARD_CFG_H_
#ifdef __cplusplus
extern "C"
{
#endif

board_cfg* get_board_config(void);
void front_end_c_get_qam_config(UINT8 dvbc_mode,UINT32 nim_clk,struct DEMOD_CONFIG_ADVANCED *p_demod_config);
#ifdef __cplusplus
}
#endif

#endif

