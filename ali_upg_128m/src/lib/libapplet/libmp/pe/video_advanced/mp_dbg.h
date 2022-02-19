#ifndef _MP_DBG_H
#define _MP_DBG_H

//#define MP_DEBUG

#ifdef MP_DEBUG
#define MP_DBG_VPTS 1
#define MP_DBG_APTS 2
#define MP_DBG_SBM   4
#define MP_DBG_DUMP_AUDIO   8
#define MP_DBG_DUMP_VIDEO   0x10
#define MP_DBG_VIDEO_ABR   0x20
#define MP_DBG_AUDIO_ABR   0x40

extern UINT32 g_dbg_mp_option;
extern UINT32 g_dbg_apkt_cnt, g_dbg_vpkt_cnt; 
#endif

#endif

