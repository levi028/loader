/*****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2004 Copyright (C)
 *
 *  File: modules.c
 *
 *  Contents:     This file define callback of remote modules.
 *  History:
 *        Date        Author              Version     Comment
 *        ==========    ==================    ==========     =======
 *  1.  05/07/2009  Wen Liu             0.1.000     Initial
 *
 *****************************************************************************/
#include <basic_types.h>
#include <modules.h>
extern void hld_base_callee(UINT8 *);
extern void hld_deca_callee(UINT8 *);
extern void hld_snd_callee(UINT8 *);
extern void hld_vp_callee(UINT8 *);
extern void lld_deca_m36f_callee(UINT8 *);
extern void lld_snd_m36f_callee(UINT8 *);
extern void lld_vp_m36f_callee(UINT8 *);
extern void lld_vbi_m33_callee(UINT8 *);
extern void lld_dmx_m36f_callee(UINT8 *);
extern void lld_dsc_m36f_callee(UINT8 *);
extern void lld_crypto_m36f_callee(UINT8 *);
extern void hld_dsc_callee(UINT8 *);
extern void hld_crypto_callee(UINT8 *);
extern void hld_decv_callee(UINT8 *);
extern void lld_decv_m36_callee(UINT8 *);
extern void lld_decv_avc_callee(UINT8 *);
extern void lib_pe_adv_video_engine_callee(UINT8 *);
extern void lib_pe_music_engine_callee(UINT8 *);
extern void lib_pe_image_engine_callee(UINT8 *);
extern void lld_trng_m36f_callee(UINT8 *);
extern void lib_close_caption_callee(UINT8 *);
extern void lld_sbm_callee(UINT8 *);
extern void hld_pvr_callee(UINT8 *);
extern void hld_avsync_callee(UINT8 *);
extern void lld_avsync_callee(UINT8 *);
extern void lld_vsc_callee(UINT8 *);


UINT32 g_remote_callee[] = {
(UINT32)hld_base_callee,
(UINT32)0,
(UINT32)0,
(UINT32)0,
(UINT32)0,
(UINT32)0,
(UINT32)0,
(UINT32)0,
(UINT32)0,
(UINT32)0,
(UINT32)0,
(UINT32)0,
(UINT32)0,
(UINT32)0,
(UINT32)0,
(UINT32)0,
(UINT32)0,
(UINT32)0,
(UINT32)0,
(UINT32)0,
(UINT32)0,
(UINT32)0,
(UINT32)0,
(UINT32)0,
(UINT32)0,
(UINT32)lld_vsc_callee,
};


