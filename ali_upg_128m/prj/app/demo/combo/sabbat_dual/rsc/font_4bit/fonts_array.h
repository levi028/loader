
#include "Font_asc.h"
#include "Font_asc_big.h"

#if(MULTI_MENU_LANG)
#include "Font_lat1.h"
#include "Font_late.h"
#include "Font_grk.h"
#include "Font_cry.h"
#include "Font_arb.h"
//#include "Font_09big.h"
#include "Font_late_big.h"
#include "Font_lat1_big.h"
#include "Font_thai.h"
#ifdef HINDI_LANGUAGE_SUPPORT
#include "Font_devanagari.h"
#endif
#ifdef TELUGU_LANGUAGE_SUPPORT
#include "Font_telugu.h"
#endif
#ifdef BENGALI_LANGUAGE_SUPPORT
#include "Font_bengali.h"
#endif
#ifdef CAS9_V6
#include "Font_letterlike.h" //char_selected
#include "Font_latinextb.h"
#include "Font_latinexta.h"
#endif
#endif

#include "../../osd_rsc.h"

font_t font_asc[] =
{
    {30, 95,    fonts_asc_width,    fonts_asc_data, (const UINT16*)fonts_asc_name, 4},
    {30, 95,    fonts_asc_width,    fonts_asc_data, (const UINT16*)fonts_asc_name, 4},
    {36, 96,    fonts_asc_big_width,fonts_asc_big_data, (const UINT16*)fonts_asc_big_name, 4},
    {30, 95,    fonts_asc_width,    fonts_asc_data, (const UINT16*)fonts_asc_name, 4},
};

#if(MULTI_MENU_LANG)

font_t font_lat1[] =
{
    {30, 95,    fonts_lat1_width,    fonts_lat1_data, (const UINT16*)fonts_lat1_name, 4},
    {30, 95,    fonts_lat1_width,    fonts_lat1_data, (const UINT16*)fonts_lat1_name, 4},
//    {22, 95,    Fonts_lat1_width,    Fonts_lat1_data, Fonts_lat1_name},
    {36, 95,    fonts_lat1_big_width,    fonts_lat1_big_data, (const UINT16*)fonts_lat1_big_name, 4},
    {30, 95,    fonts_lat1_width,    fonts_lat1_data, (const UINT16*)fonts_lat1_name, 4},
};

font_t font_late[] =
{
    {30, 128,    fonts_late_width,    fonts_late_data, (const UINT16*)fonts_late_name, 4},
    {30, 128,    fonts_late_width,    fonts_late_data,(const UINT16*) fonts_late_name, 4},
//    {22, 128,    Fonts_late_width,    Fonts_late_data, Fonts_late_name},
    {36, 128,    fonts_late_big_width,    fonts_late_big_data, (const UINT16*)fonts_late_big_name, 4},
    {30, 128,    fonts_late_width,    fonts_late_data, (const UINT16*)fonts_late_name, 4},
};

font_t font_grk[] =
{
    {30, 105,    fonts_grk_width,    fonts_grk_data, (const UINT16*)fonts_grk_name, 4},
    {30, 105,    fonts_grk_width,    fonts_grk_data, (const UINT16*)fonts_grk_name, 4},
    {30, 105,    fonts_grk_width,    fonts_grk_data, (const UINT16*)fonts_grk_name, 4},
    {30, 105,    fonts_grk_width,    fonts_grk_data, (const UINT16*)fonts_grk_name, 4},
};

font_t font_cry[] =
{
    {30, 144,    fonts_cry_width,    fonts_cry_data, (const UINT16*)fonts_cry_name, 4},
    {30, 144,    fonts_cry_width,    fonts_cry_data, (const UINT16*)fonts_cry_name, 4},
    {30, 144,    fonts_cry_width,    fonts_cry_data, (const UINT16*)fonts_cry_name, 4},
    {30, 144,    fonts_cry_width,    fonts_cry_data, (const UINT16*)fonts_cry_name, 4},
};


font_t font_arb[] =
{
#if 1
    {32, 458,    fonts_arb_width,    fonts_arb_data, (const UINT16*)fonts_arb_name, 4},
    {32, 458,    fonts_arb_width,    fonts_arb_data, (const UINT16*)fonts_arb_name, 4},
    {32, 458,    fonts_arb_width,    fonts_arb_data, (const UINT16*)fonts_arb_name, 4},
    {32, 458,    fonts_arb_width,    fonts_arb_data, (const UINT16*)fonts_arb_name, 4},
#else
    {30, 160,    fonts_arb_width,    fonts_arb_data, fonts_arb_name, 4},
    {30, 160,    fonts_arb_width,    fonts_arb_data, fonts_arb_name, 4},
    {30, 160,    fonts_arb_width,    fonts_arb_data, fonts_arb_name, 4},
    {30, 160,    fonts_arb_width,    fonts_arb_data, fonts_arb_name, 4},
#endif
};

font_t font_thai[] =
{
    {24, 87,    fonts_thai_width,    fonts_thai_data, (const UINT16*)fonts_thai_name, 4},
    {24, 87,    fonts_thai_width,    fonts_thai_data,(const UINT16*) fonts_thai_name, 4},
    {24, 87,    fonts_thai_width,    fonts_thai_data, (const UINT16*)fonts_thai_name, 4},
    {24, 87,    fonts_thai_width,    fonts_thai_data, (const UINT16*)fonts_thai_name, 4},


};

#ifdef HINDI_LANGUAGE_SUPPORT
font_t font_devanagari[] = 
{
	{32, 514, fonts_devanagari_width,	fonts_devanagari_data,	(const UINT16*)fonts_devanagari_name, 1},
	{32, 514, fonts_devanagari_width,	fonts_devanagari_data,	(const UINT16*)fonts_devanagari_name, 1},
	{32, 514, fonts_devanagari_width,	fonts_devanagari_data,	(const UINT16*)fonts_devanagari_name, 1},
	{32, 514, fonts_devanagari_width,	fonts_devanagari_data,	(const UINT16*)fonts_devanagari_name, 1},
};
#endif

#ifdef TELUGU_LANGUAGE_SUPPORT
font_t font_telugu[] = 
{
	{39, 669, fonts_telugu_width,	fonts_telugu_data,	(const UINT16*)fonts_telugu_name, 1},
	{39, 669, fonts_telugu_width,	fonts_telugu_data,	(const UINT16*)fonts_telugu_name, 1},
	{39, 669, fonts_telugu_width,	fonts_telugu_data,	(const UINT16*)fonts_telugu_name, 1},
	{39, 669, fonts_telugu_width,	fonts_telugu_data,	(const UINT16*)fonts_telugu_name, 1},
};
#endif

#ifdef BENGALI_LANGUAGE_SUPPORT
font_t font_bengali[] = 
{
	{32, 505, fonts_bengali_width,	fonts_bengali_data,	(const UINT16*)fonts_bengali_name, 1},
	{32, 505, fonts_bengali_width,	fonts_bengali_data,	(const UINT16*)fonts_bengali_name, 1},
	{32, 505, fonts_bengali_width,	fonts_bengali_data,	(const UINT16*)fonts_bengali_name, 1},
	{32, 505, fonts_bengali_width,	fonts_bengali_data,	(const UINT16*)fonts_bengali_name, 1},
};
#endif

#ifdef CAS9_V6 //char_selected
font_t font_letterlike[]=
{
    {32, 80,    fonts_letterlike_width,    fonts_letterlike_data, (const UINT16*)fonts_letterlike_name,4},
    {32, 80,    fonts_letterlike_width,    fonts_letterlike_data, (const UINT16*)fonts_letterlike_name,4},
    {32, 80,    fonts_letterlike_width,    fonts_letterlike_data, (const UINT16*)fonts_letterlike_name,4},
    {32, 80,    fonts_letterlike_width,    fonts_letterlike_data, (const UINT16*)fonts_letterlike_name,4},
};

//char_selected
font_t font_latinextb[]=
{
    {32, 208,    fonts_latinextb_width,    fonts_latinextb_data, (const UINT16*)fonts_latinextb_name,4},
    {32, 208,    fonts_latinextb_width,    fonts_latinextb_data, (const UINT16*)fonts_latinextb_name,4},
    {32, 208,    fonts_latinextb_width,    fonts_latinextb_data, (const UINT16*)fonts_latinextb_name,4},
    {32, 208,    fonts_latinextb_width,    fonts_latinextb_data, (const UINT16*)fonts_latinextb_name,4},
};

font_t font_latinexta[]=
{
    {32, 128,    fonts_latinexta_width,    fonts_latinexta_data, (const UINT16*)fonts_latinexta_name,4},
    {32, 128,    fonts_latinexta_width,    fonts_latinexta_data, (const UINT16*)fonts_latinexta_name,4},
    {32, 128,    fonts_latinexta_width,    fonts_latinexta_data, (const UINT16*)fonts_latinexta_name,4},
    {32, 128,    fonts_latinexta_width,    fonts_latinexta_data, (const UINT16*)fonts_latinexta_name,4},
};

#endif

#endif

font_t *font_array[]=
{
    font_asc,
#if(MULTI_MENU_LANG)
    font_lat1,
    font_late,
    font_grk,
    font_cry,
    font_arb,
    font_thai,
#ifdef HINDI_LANGUAGE_SUPPORT
    font_devanagari,
#endif
#ifdef TELUGU_LANGUAGE_SUPPORT	
    font_telugu,
#endif
#ifdef BENGALI_LANGUAGE_SUPPORT
    font_bengali,
#endif	
#ifdef CAS9_V6 //char_selected
    font_letterlike,
    font_latinextb,
    font_latinexta,
#endif
#endif
};

#define FONT_MAX_SIZE 4

