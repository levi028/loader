/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: osd_rsc.h
*
*    Description: This file contains some functions to get osd resource by analyzing raw data.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _RSC_LIB_H
#define _RSC_LIB_H
#ifdef __cplusplus
extern "C"
{
#endif
#include <types.h>
#include <api/libosd/osd_lib.h>
#include <api/libosd/osd_common_draw.h>
#include <api/libmp/mp_subtitle_api.h>


///////////////////////////////////////////////////////////////
// Language definition

/*
typedef enum
{
    NULL_ENV,
    ENGLISH_ENV,
    FRANCE_ENV,
    GERMANY_ENV,
     ITALY_ENV,
     RUSSIAN_ENV,
     SPANISH_ENV,
     TURKISH_ENV,
     PORTUGUESE_ENV,
     ARABIC_ENV,
    POLISH_ENV,
     CHINESE_ENV,
    INDONESIAN_ENV,
    IRAN_EVN,
    GREECE_EVN,
    SWEDISH_EVN,
    SLAVIC_EVN,
    DUTCH_EVN,
    SLOVAK_EVN,
    ROMANIAN_EVN,
    HUNGARIAN_EVN,
    CZECH_EVN,
    BULGARIAN_EVN,
    DANISH_EVN,
    FINNISH_EVN,
    ESTONIAN_EVN,
    CATANIAN_EVN,
    PERSIAN_EVN,
    LATIN_EVN,
}LANG_GROUP_t;
*/

typedef enum
{
    ENGLISH_ENV = 0,
    FRANCE_ENV,
#ifdef HINDI_LANGUAGE_SUPPORT
    HINDI_ENV,
#endif
#ifdef TELUGU_LANGUAGE_SUPPORT	
    TELUGU_ENV,
#endif
#ifdef BENGALI_LANGUAGE_SUPPORT	
    BENGALI_ENV,
#endif	
    GERMANY_ENV ,
    ITALY_ENV,
    SPANISH_ENV,
    PORTUGUESE_ENV,
    RUSSIAN_ENV,
    TURKISH_ENV,
    POLISH_ENV,
    ARABIC_ENV,
    PERSIAN_ENV,
    GREEK_ENV,
    ENDLANG_ENV
}lang_group_t;

#define DEFAULT_LANG_ENV    GERMANY_ENV//GERMANY_ENV


///////////////////////////////////////////////////////////////
// Font definition
#define LIB_FONT_NULL        0x1000
#define LIB_FONT_DEFAULT    0x1100
#define LIB_FONT_LATIN1     0x1200
#define LIB_FONT_LATINE     0x1300
#define LIB_FONT_GREEK          0x1400
#define LIB_FONT_CYRILLIC        0x1500
#define LIB_FONT_ARABIC        0x1600
#define LIB_FONT_MASSCHAR1        0x1D00
#define LIB_FONT_MASSCHAR        0x1F00
#define LIB_FONT_THAI       0x1700
#ifdef CAS9_V6
#define LIB_FONT_LETTERLIKE 0x1800  //char_selected
#define LIB_FONT_LATINEXTB 0x1900  //char_selected
#define LIB_FONT_LATINEXTA 0x1A00
#endif
#ifdef HINDI_LANGUAGE_SUPPORT
#define LIB_FONT_DEVANAGARI 0x1B00
#endif
#ifdef TELUGU_LANGUAGE_SUPPORT
#define LIB_FONT_TELUGU	0x1C00
#endif
#ifdef BENGALI_LANGUAGE_SUPPORT
#define LIB_FONT_BENGALI	0x1D00
#endif
///////////////////////////////////////////////////////////////
// String definition

#define LIB_STRING_ENGLISH    0x2100
#define LIB_STRING_FRENCH        0x2200
#define LIB_STRING_GERMAN        0x2300
#define LIB_STRING_ITALY        0x2400
#define LIB_STRING_RUSSIAN    0x2500
#define LIB_STRING_SPANISH    0x2600
#define LIB_STRING_TURKISH    0x2700
#define LIB_STRING_PORTUGUESE    0x2800
#define LIB_STRING_POLISH        0x2900
#define LIB_STRING_ARABIC        0x2a00
#define LIB_STRING_THAI     0x2b00
#define LIB_STRING_PERSIAN    0x2c00
#ifdef HINDI_LANGUAGE_SUPPORT
#define LIB_STRING_HINDI	0x2d00
#endif
#ifdef TELUGU_LANGUAGE_SUPPORT
#define LIB_STRING_TELUGU	0x2e00
#endif
#ifdef BENGALI_LANGUAGE_SUPPORT
#define LIB_STRING_BENGALI	0x2f00
#endif

///////////////////////////////////////////////////////////////
// Pallete definition

#define LIB_PALLETE_8BPP    0x4080
#define LIB_PALLETE_4BPP    0x4040
#define LIB_PALLETE_2BPP    0x4020



#define LIB_CHAR_ASCII 0
/***************************************************************************************/
typedef struct tag_lang_table
{
    UINT16    lang_id;
    ID_RSC    str_id;
    ID_RSC    char_type;
    ID_RSC    font_id;
} LANG_TABLE, *lp_lang_table;

typedef struct
{

    UINT8            height;
    UINT16            u_font_cnt;
    const UINT8*    width;
    const UINT32*    data;
    const UINT16*    u_idx_tab;
    UINT8             bit_cnt;
}font_t;
/*******************************************
***-the functions the client can visit***-*/

UINT16    osd_get_lang_environment(void);
BOOL    osd_set_lang_environment(UINT16 w_lang_class);

BOOL    osd_get_obj_info(UINT16 w_lib_class,UINT16 u_index,lp_objectinfo p_object_info);
UINT8*     osd_get_rsc_obj_data(UINT16 w_lib_class,UINT16 u_index,lp_objectinfo object_info);
UINT8*    osd_get_rsc_obj_data_ext(UINT16 w_lib_class, UINT16 u_index, \
    OBJECTINFO *p_object_info, UINT8 *font_bitmap_ext, UINT32 font_bitmap_size);

UINT8*     osd_get_thai_font_data(UINT16 w_lib_class,struct thai_cell *cell,OBJECTINFO* p_object_info);
#ifdef HINDI_LANGUAGE_SUPPORT
BOOL osd_get_devanagari_info(UINT16 w_lib_class, UINT16 group_index, UINT16 *pWt, UINT16 *pHt);

UINT8 *osd_get_devanagari_data(UINT16 w_lib_class, struct devanagari_cell *pCell, OBJECTINFO *pObjInfo);
#endif
#ifdef TELUGU_LANGUAGE_SUPPORT
BOOL osd_get_telugu_info(UINT16 w_lib_class, UINT16 group_index, UINT16 *pWt, UINT16 *pHt);

UINT8 *osd_get_telugu_data(UINT16 w_lib_class, struct telugu_cell *pCell, OBJECTINFO *pObjInfo);
#endif
#ifdef BENGALI_LANGUAGE_SUPPORT
BOOL osd_get_bengali_info(UINT16 w_lib_class, UINT16 group_index, UINT16 *pWt, UINT16 *pHt);

UINT8 *osd_get_bengali_data(UINT16 w_lib_class, struct bengali_cell *pCell, OBJECTINFO *pObjInfo);
#endif
BOOL       osd_release_obj_data(UINT8* lp_data_buf,lp_objectinfo p_object_info);

ID_RSC  osd_get_default_font_lib(UINT16 u_char);


PWINSTYLE osdext_get_win_style(UINT8 b_style_idx);
ID_RSC    osdext_get_msg_lib_id(UINT16 env_id);

#ifdef BIDIRECTIONAL_OSD_STYLE
BOOL    osd_get_mirror_flag(void);
#endif
font_t *osd_get_font_array_1bit(void);
font_subt_t *osd_get_font_array_1bit_subt(void);

/*******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif

