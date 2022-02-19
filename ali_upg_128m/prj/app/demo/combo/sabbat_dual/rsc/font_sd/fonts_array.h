
#ifdef HDOSD
#include "Font_asc_hd.h"
#else
#include "font_asc.h"
#if (defined(SUPPORT_BC_STD) || defined(SUPPORT_BC) )&& defined(BC_PATCH_UI)
#include "Font_asc_16.h"
#endif
#include "Font_late_big.h"
#include "Font_lat1_big.h"
#include "Font_asc_big.h"
#endif

//#include "Font_asc_hd.h"

#if(MULTI_MENU_LANG)

//#include "Font_09big.h"

#ifdef HDOSD
#include "Font_lat1_hd.h"
#include "Font_late_hd.h"
#include "Font_cry_hd.h"
#include "Font_grk_hd.h"
#include "Font_arb_hd.h"
#include "Font_heb_hd.h"
#ifdef ETHIOPIC_FONT_SUPPORT
#include "Font_eth_hd.h"
#endif
#else
#include "font_lat1.h"
#include "font_late.h"
#include "font_grk.h"
#include "font_cry.h"
#include "font_arb.h"
#include "font_heb.h"
#ifdef ETHIOPIC_FONT_SUPPORT
#include "font_eth.h"
#endif
#endif

#ifdef DYNAMIC_FONT
#include "font_asc_df.h"
#endif

#if (defined(_DTGSI_ENABLE_))
#include "font_iso6937_24.h"
#include "font_iso6937_31.h"
#endif

#ifdef CAS9_V6
#include "Font_letterlike.h" //char_selected
#include "Font_latinextb.h"
#include "Font_latinexta.h"
#endif

#endif

#ifdef CHINESE_SUPPORT
    #ifdef CHI_13053
        #ifdef HDOSD
            #include "font_cht_hd_13053.h"
        #else
            #include "font_chn_13053.h"
        #endif
    #else
        #include "font_chn_5401.h"
    #endif

    #ifdef HDOSD
        #include "font_cjk_hd.h"
    #else
        #include "font_cjk.h"
    #endif
#endif

#define FONT_MAX_SIZE 4

#ifdef DYNAMIC_FONT
typedef struct
{
    UINT8            height;
    UINT16            u_font_cnt;
    const UINT16*    size;
    const INT32*    data;
    const UINT16*    u_idx_tab;

}font_df_t;

font_df_t font_asc_df[] =
{
    {28, 96,    fonts_asc_size_df,    fonts_asc_data_df, fonts_asc_name_df},
    {32, 96,    fonts_asc_size_df,    fonts_asc_data_df, fonts_asc_name_df},
    {36, 96,    fonts_asc_size_df,    fonts_asc_data_df, fonts_asc_name_df},
    {36, 96,    fonts_asc_size_df,    fonts_asc_data_df, fonts_asc_name_df},
};
#endif

font_t font_asc[] =
{
#ifndef HDOSD
    {24, 96,    fonts_asc_width,    fonts_asc_data, fonts_asc_name , 0},
    //{30, 11,    fonts_09big_width,   fonts_09big_data, fonts_09big_name},
    {31, 96,    fonts_asc_big_width,fonts_asc_big_data, fonts_asc_big_name, 0},
    {24, 96,    fonts_asc_width,    fonts_asc_data, fonts_asc_name, 0},
    {24, 96,    fonts_asc_width,    fonts_asc_data, fonts_asc_name, 0},
    #if (defined(SUPPORT_BC_STD)||defined(SUPPORT_BC))&& defined(BC_PATCH_UI)
    //small font size:16
    {16, 96,    fonts_asc_16_width,    fonts_asc_16_data, fonts_asc_16_name, 0},
#endif
#else
    {36, 95,    fonts_asc_width_hd,    fonts_asc_data_hd, fonts_asc_name_hd, 0},
    //{30, 11,    fonts_09big_width,   fonts_09big_data, fonts_09big_name},
    {36, 95,    fonts_asc_width_hd,    fonts_asc_data_hd, fonts_asc_name_hd, 0},
    {36, 95,    fonts_asc_width_hd,    fonts_asc_data_hd, fonts_asc_name_hd, 0},
    {36, 95,    fonts_asc_width_hd,    fonts_asc_data_hd, fonts_asc_name_hd, 0},
#endif
};
#ifdef CHINESE_SUPPORT
font_t font_chi[] =
{
#ifndef HDOSD
    {20, sizeof(fonts_name)/sizeof(short),    chn_width,    chn_data, fonts_name},
    {20, sizeof(fonts_name)/sizeof(short),    chn_width,    chn_data, fonts_name},
    {20, sizeof(fonts_name)/sizeof(short),    chn_width,    chn_data, fonts_name},
    {20, sizeof(fonts_name)/sizeof(short),    chn_width,    chn_data, fonts_name},
#else
    {32, sizeof(fonts_cht_hd_name)/sizeof(short),    fonts_cht_hd_width,    fonts_cht_hd_data, fonts_cht_hd_name},
    {32, sizeof(fonts_cht_hd_name)/sizeof(short),    fonts_cht_hd_width,    fonts_cht_hd_data, fonts_cht_hd_name},
    {32, sizeof(fonts_cht_hd_name)/sizeof(short),    fonts_cht_hd_width,    fonts_cht_hd_data, fonts_cht_hd_name},
    {32, sizeof(fonts_cht_hd_name)/sizeof(short),    fonts_cht_hd_width,    fonts_cht_hd_data, fonts_cht_hd_name},
#endif
};
font_t font_cjk[] =
{
#ifndef HDOSD
    {20, 32,    fonts_cjk_width,    fonts_cjk_data, fonts_cjk_name},
    {20, 32,    fonts_cjk_width,    fonts_cjk_data, fonts_cjk_name},
    {20, 32,    fonts_cjk_width,    fonts_cjk_data, fonts_cjk_name},
    {20, 32,    fonts_cjk_width,    fonts_cjk_data, fonts_cjk_name},
#else
    {32, 32,    fonts_cjk_width_hd,    fonts_cjk_data_hd, fonts_cjk_name_hd},
    {32, 32,    fonts_cjk_width_hd,    fonts_cjk_data_hd, fonts_cjk_name_hd},
    {32, 32,    fonts_cjk_width_hd,    fonts_cjk_data_hd, fonts_cjk_name_hd},
    {32, 32,    fonts_cjk_width_hd,    fonts_cjk_data_hd, fonts_cjk_name_hd},
#endif
};
#endif

#if(MULTI_MENU_LANG)

font_t font_lat1[] =
{
#ifndef HDOSD
    {24, 95,    fonts_lat1_width,    fonts_lat1_data, fonts_lat1_name, 0},
    //{22, 95,    fonts_lat1_width,    fonts_lat1_data, fonts_lat1_name},
    {31, 95,    fonts_lat1_big_width,    fonts_lat1_big_data, fonts_lat1_big_name, 0},
    {24, 95,    fonts_lat1_width,    fonts_lat1_data, fonts_lat1_name, 0},
    {24, 95,    fonts_lat1_width,    fonts_lat1_data, fonts_lat1_name, 0},
#else
    {36, 95,    fonts_lat1_width_hd,    fonts_lat1_data_hd, fonts_lat1_name_hd, 0},
    //{22, 95,    fonts_lat1_width,    fonts_lat1_data, fonts_lat1_name},
    {36, 95,    fonts_lat1_width_hd,    fonts_lat1_data_hd, fonts_lat1_name_hd, 0},
    {36, 95,    fonts_lat1_width_hd,    fonts_lat1_data_hd, fonts_lat1_name_hd, 0},
    {36, 95,    fonts_lat1_width_hd,    fonts_lat1_data_hd, fonts_lat1_name_hd, 0},
#endif
};

font_t font_late[] =
{
#ifndef HDOSD
    {24, 128,    fonts_late_width,    fonts_late_data, fonts_late_name, 0},
    //{22, 128,    fonts_late_width,    fonts_late_data, fonts_lat1_name},
    {31, 128,    fonts_late_big_width,    fonts_late_big_data, fonts_late_big_name, 0},
    {24, 128,    fonts_late_width,    fonts_late_data, fonts_late_name, 0},
    {24, 128,    fonts_late_width,    fonts_late_data, fonts_late_name, 0},
#else
    {36, 128,    fonts_late_width_hd,    fonts_late_data_hd, fonts_late_name_hd, 0},
    //{22, 128,    fonts_late_width,    fonts_late_data, fonts_lat1_name},
    {36, 128,    fonts_late_width_hd,    fonts_late_data_hd, fonts_late_name_hd, 0},
    {36, 128,    fonts_late_width_hd,    fonts_late_data_hd, fonts_late_name_hd, 0},
    {36, 128,    fonts_late_width_hd,    fonts_late_data_hd, fonts_late_name_hd, 0},
#endif
};

font_t font_grk[] =
{
#ifndef HDOSD
    {24, 105,    fonts_grk_width,    fonts_grk_data, fonts_grk_name, 0},
    {24, 105,    fonts_grk_width,    fonts_grk_data, fonts_grk_name, 0},
    {24, 105,    fonts_grk_width,    fonts_grk_data, fonts_grk_name, 0},
    {24, 105,    fonts_grk_width,    fonts_grk_data, fonts_grk_name, 0},
#else
    {36, 105,    fonts_grk_width_hd,    fonts_grk_data_hd, fonts_grk_name_hd, 0},
    {36, 105,    fonts_grk_width_hd,    fonts_grk_data_hd, fonts_grk_name_hd, 0},
    {36, 105,    fonts_grk_width_hd,    fonts_grk_data_hd, fonts_grk_name_hd, 0},
    {36, 105,    fonts_grk_width_hd,    fonts_grk_data_hd, fonts_grk_name_hd, 0},
#endif
};

font_t font_cry[] =
{
#ifndef HDOSD
    {24, 144,    fonts_cry_width,    fonts_cry_data, fonts_cry_name, 0},
    {24, 144,    fonts_cry_width,    fonts_cry_data, fonts_cry_name, 0},
    {24, 144,    fonts_cry_width,    fonts_cry_data, fonts_cry_name, 0},
    {24, 144,    fonts_cry_width,    fonts_cry_data, fonts_cry_name, 0},
#else
    {36, 144,    fonts_cry_width_hd,    fonts_cry_data_hd, fonts_cry_name_hd, 0},
    {36, 144,    fonts_cry_width_hd,    fonts_cry_data_hd, fonts_cry_name_hd, 0},
    {36, 144,    fonts_cry_width_hd,    fonts_cry_data_hd, fonts_cry_name_hd, 0},
    {36, 144,    fonts_cry_width_hd,    fonts_cry_data_hd, fonts_cry_name_hd, 0},
#endif
};


font_t font_arb[] =
{
#ifndef HDOSD
    {24, 458,    fonts_arb_width,    fonts_arb_data, fonts_arb_name, 0},
    {24, 458,    fonts_arb_width,    fonts_arb_data, fonts_arb_name, 0},
    {24, 458,    fonts_arb_width,    fonts_arb_data, fonts_arb_name, 0},
    {24, 458,    fonts_arb_width,    fonts_arb_data, fonts_arb_name, 0},
#else
    {36, 458,    fonts_arb_width_hd,    fonts_arb_data_hd, fonts_arb_name_hd, 0},
    {36, 458,    fonts_arb_width_hd,    fonts_arb_data_hd, fonts_arb_name_hd, 0},
    {36, 458,    fonts_arb_width_hd,    fonts_arb_data_hd, fonts_arb_name_hd, 0},
    {36, 458,    fonts_arb_width_hd,    fonts_arb_data_hd, fonts_arb_name_hd, 0},
#endif
};

font_t font_heb[] =
{
#ifndef HDOSD
    {24, 27,    fonts_heb_width,    fonts_heb_data, fonts_heb_name, 0},
    {24, 27,    fonts_heb_width,    fonts_heb_data, fonts_heb_name, 0},
    {24, 27,    fonts_heb_width,    fonts_heb_data, fonts_heb_name, 0},
    {24, 27,    fonts_heb_width,    fonts_heb_data, fonts_heb_name, 0},
#else
    {36, 27,    fonts_heb_width_hd,    fonts_heb_data_hd, fonts_heb_name_hd, 0},
    {36, 27,    fonts_heb_width_hd,    fonts_heb_data_hd, fonts_heb_name_hd, 0},
    {36, 27,    fonts_heb_width_hd,    fonts_heb_data_hd, fonts_heb_name_hd, 0},
    {36, 27,    fonts_heb_width_hd,    fonts_heb_data_hd, fonts_heb_name_hd, 0},
#endif
};

#ifdef ETHIOPIC_FONT_SUPPORT
font_t font_eth[] =
{
#ifndef HDOSD
    {24, 355,    fonts_eth_width,    fonts_eth_data, fonts_eth_name},
    {24, 355,    fonts_eth_width,    fonts_eth_data, fonts_eth_name},
    {24, 355,    fonts_eth_width,    fonts_eth_data, fonts_eth_name},
    {24, 355,    fonts_eth_width,    fonts_eth_data, fonts_eth_name},
#else
    {36, 355,    fonts_eth_width_hd,    fonts_eth_data_hd, fonts_eth_name_hd},
    {36, 355,    fonts_eth_width_hd,    fonts_eth_data_hd, fonts_eth_name_hd},
    {36, 355,    fonts_eth_width_hd,    fonts_eth_data_hd, fonts_eth_name_hd},
    {36, 355,    fonts_eth_width_hd,    fonts_eth_data_hd, fonts_eth_name_hd},
#endif
};
#endif

#if (defined(_DTGSI_ENABLE_))
font_t font_iso6937[]=
{
    {24, 23,    fonts_iso6937_24_width,    fonts_iso6937_24_data, fonts_iso6937_24_name},
    {31, 23,    fonts_iso6937_31_width,    fonts_iso6937_31_data, fonts_iso6937_31_name},
    {24, 23,    fonts_iso6937_24_width,    fonts_iso6937_24_data, fonts_iso6937_24_name},
    {24, 23,    fonts_iso6937_24_width,    fonts_iso6937_24_data, fonts_iso6937_24_name},
};
#endif

#ifdef CAS9_V6 //char_selected
font_t font_letterlike[]=
{
    {24, 80,    fonts_letterlike_width,    fonts_letterlike_data, fonts_letterlike_name},
    {24, 80,    fonts_letterlike_width,    fonts_letterlike_data, fonts_letterlike_name},
    {24, 80,    fonts_letterlike_width,    fonts_letterlike_data, fonts_letterlike_name},
    {24, 80,    fonts_letterlike_width,    fonts_letterlike_data, fonts_letterlike_name},
};

//char_selected
font_t font_latinextb[]=
{
    {24, 208,    fonts_latinextb_width,    fonts_latinextb_data, fonts_latinextb_name},
    {24, 208,    fonts_latinextb_width,    fonts_latinextb_data, fonts_latinextb_name},
    {24, 208,    fonts_latinextb_width,    fonts_latinextb_data, fonts_latinextb_name},
    {24, 208,    fonts_latinextb_width,    fonts_latinextb_data, fonts_latinextb_name},
};

font_t font_latinexta[]=
{
    {24, 208,    fonts_latinexta_width,    fonts_latinexta_data, fonts_latinexta_name},
    {24, 208,    fonts_latinexta_width,    fonts_latinexta_data, fonts_latinexta_name},
    {24, 208,    fonts_latinexta_width,    fonts_latinexta_data, fonts_latinexta_name},
    {24, 208,    fonts_latinexta_width,    fonts_latinexta_data, fonts_latinexta_name},
};

#endif

#endif

font_t *font_array[]=
{
    font_asc,
#ifdef CHINESE_SUPPORT
    font_chi,
    font_cjk,
#endif
#if(MULTI_MENU_LANG)
    font_lat1,
    font_late,
    font_grk,
    font_cry,
    font_arb,
    font_heb,
#ifdef ETHIOPIC_FONT_SUPPORT
    font_eth,
#endif
#if (defined(_DTGSI_ENABLE_))
    font_iso6937,
#endif

#ifdef CAS9_V6 //char_selected
    font_letterlike,
    font_latinextb,
    font_latinexta,
#endif

#endif
};

