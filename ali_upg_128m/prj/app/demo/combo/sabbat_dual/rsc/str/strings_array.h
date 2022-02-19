
#include "str_English.h"
#if(MULTI_MENU_LANG)
#include "str_French.h"
#include "str_Italian.h"
#include "str_Portuguese.h"
#include "str_Spanish.h"
#include "str_German.h"
#include "str_Turkish.h"
#include "str_Polish.h"
#include "str_Russia.h"
#include "str_Arabic.h"
#include "str_Thai.h"
#include "str_Persian.h"
#endif

const unsigned char *strings_infor[]=
{
    English_strs_array,
#if(MULTI_MENU_LANG)
    French_strs_array,
    Italian_strs_array,
    Portuguese_strs_array,
    Spanish_strs_array,
    German_strs_array,
    Turkish_strs_array,
    Polish_strs_array,
    Russia_strs_array,
    Arabic_strs_array,
    Persian_strs_array,
//    Thai_strs_array,
#endif
};

const LANG_TABLE g_lang_rsc_map_table[] __attribute__((section(".flash")))=
{
    {ENGLISH_ENV,     LIB_STRING_ENGLISH, LIB_CHAR_ASCII,        LIB_FONT_DEFAULT},
    {FRANCE_ENV,     LIB_STRING_FRENCH,     LIB_CHAR_ASCII,        LIB_FONT_DEFAULT},
    {GERMANY_ENV,     LIB_STRING_GERMAN,     LIB_CHAR_ASCII,        LIB_FONT_DEFAULT},
    {ITALY_ENV,     LIB_STRING_ITALY,     LIB_CHAR_ASCII,        LIB_FONT_DEFAULT},
    {PORTUGUESE_ENV,LIB_STRING_PORTUGUESE, LIB_CHAR_ASCII,        LIB_FONT_DEFAULT},
    {RUSSIAN_ENV,    LIB_STRING_RUSSIAN, LIB_CHAR_ASCII,        LIB_FONT_DEFAULT},
    {SPANISH_ENV,   LIB_STRING_SPANISH, LIB_CHAR_ASCII,     LIB_FONT_DEFAULT},
    {TURKISH_ENV,   LIB_STRING_TURKISH, LIB_CHAR_ASCII,     LIB_FONT_DEFAULT},
    {POLISH_ENV,    LIB_STRING_POLISH,  LIB_CHAR_ASCII,     LIB_FONT_DEFAULT},
    {ARABIC_ENV,    LIB_STRING_ARABIC,  LIB_CHAR_ASCII,     LIB_FONT_DEFAULT},
    {PERSIAN_ENV,    LIB_STRING_PERSIAN,  LIB_CHAR_ASCII,     LIB_FONT_DEFAULT},
};

#define    SIZE_LANG_TABLE    sizeof(g_lang_rsc_map_table)/sizeof(LANG_TABLE)

