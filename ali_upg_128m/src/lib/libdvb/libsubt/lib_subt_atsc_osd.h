/*****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.
   *    File: lib_subt_atsc_osd.h
   *
   *    Description: The file is mainly to declare the function of showing ATSC
        SUBTITLE.
   *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
         KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
         IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
         PARTICULAR PURPOSE.
   *****************************************************************************/



#ifdef __cplusplus
extern "C"
{
#endif

#include <sys_config.h>
#include <sys_parameters.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>

#define    OSD_INVALID_REGION        0xFF
#define osd_set_rect(r, _l, _t, _w, _h) \
    do{ \
        ((POSD_RECT)(r))->u_left = (UINT16)(_l); \
        ((POSD_RECT)(r))->u_top = (UINT16)(_t); \
        ((POSD_RECT)(r))->u_width = (UINT16)(_w); \
        ((POSD_RECT)(r))->u_height = (UINT16)(_h); \
    }while(0)

#define osd_copy_rect(dest, src) \
    do{ \
        ((UINT32 *)(dest))[0] = ((UINT32 *)(src))[0]; \
        ((UINT32 *)(dest))[1] = ((UINT32 *)(src))[1]; \
    }while(0)
#define osd_set_rect2    osd_copy_rect

typedef struct osdrect  OSD_RECT, *POSD_RECT;

struct osd_device* lib_subt_atsc_get_osd_handle(void);
RET_CODE lib_subt_atsc_clear_osd(struct osd_device *subt_atsc_osd_dev);
void lib_subt_atsc_display(struct osdrect *rect,atsc_subt_bmp_header *bmp_header,UINT16 color);
BOOL lib_subt_atsc_get_osd_rect(DISPLAY_STANDARD_TYPE type, struct osdrect *rect);
RET_CODE lib_subt_atsc_osd_set_pallette
    (struct osd_device *subt_atsc_osd_dev,atsc_subt_bmp_header *bmp_header);
void lib_subt_atsc_osd_set_present(BOOL present);
void lib_subt_atsc_show_screen();
RET_CODE lib_subt_atsc_get_region_pos(struct osd_device *subt_atsc_osd_dev);


#ifdef __cplusplus
 }
#endif




