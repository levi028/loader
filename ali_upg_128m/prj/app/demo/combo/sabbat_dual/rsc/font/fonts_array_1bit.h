#include "font_asc_1bit.h"
#include "Font_asc_big_1bit.h"
#include "font_asc_small_1bit.h"

#if(MULTI_MENU_LANG)
#include "font_lat1_1bit.h"
#include "font_late_1bit.h"
#include "font_grk_1bit.h"
#include "font_cry_1bit.h"
#include "font_arb_1bit.h"
#include "font_her_1bit.h"
#include "Font_lat1_big_1bit.h"
#include "Font_late_big_1bit.h"
#include "font_grk_big_1bit.h"
#include "font_cry_big_1bit.h"
#include "font_arb_big_1bit.h"
#include "font_lat1_small_1bit.h"
#include "font_late_small_1bit.h"
#include "font_grk_small_1bit.h"
#include "font_cry_small_1bit.h"
#include "font_arb_small_1bit.h"
#endif

#include "../../osd_rsc.h"
#include <api/libmp/mp_subtitle_api.h>

font_t font_asc_1bit[] =
{
    {36, 95,    fonts_asc_width_1bit,    fonts_asc_data_1bit, (const UINT16*)fonts_asc_name_1bit, 1},
    {31, 96,    fonts_asc_small_width_1bit,fonts_asc_small_data_1bit, (const UINT16*)fonts_asc_small_name_1bit, 1},
    {42, 96,    fonts_asc_big_width_1bit,fonts_asc_big_data_1bit, (const UINT16*)fonts_asc_big_name_1bit, 1},
    {24, 96,   fonts_asc_24_width,    fonts_asc_24_data, (const UINT16*)fonts_asc_24_name, 1},
};

#if(MULTI_MENU_LANG)

font_t font_lat1_1bit[] =
{
    {36, 95,    fonts_lat1_width_1bit,    fonts_lat1_data_1bit, (const UINT16*)fonts_lat1_name_1bit, 1},
    {31, 96,    fonts_lat1_small_width_1bit,    fonts_lat1_small_data_1bit, (const UINT16*)fonts_lat1_small_name_1bit, 1},
    {42, 95,    fonts_lat1_big_width_1bit,    fonts_lat1_big_data_1bit, (const UINT16*)fonts_lat1_big_name_1bit, 1},
    {36, 95,    fonts_lat1_width_1bit,    fonts_lat1_data_1bit, (const UINT16*)fonts_lat1_name_1bit, 1},
};

font_t font_late_1bit[] =
{
    {36, 128,    fonts_late_width_1bit,    fonts_late_data_1bit, (const UINT16*)fonts_late_name_1bit, 1},
    {31, 128,    fonts_late_small_width_1bit,    fonts_late_small_data_1bit, (const UINT16*)fonts_late_small_name_1bit, 1},
    {42, 128,    fonts_late_big_width_1bit,    fonts_late_big_data_1bit, (const UINT16*)fonts_late_big_name_1bit, 1},
    {36, 128,    fonts_late_width_1bit,    fonts_late_data_1bit, (const UINT16*)fonts_late_name_1bit, 1},
};

font_t font_grk_1bit[] =
{
    {36, 105,    fonts_grk_width_1bit,    fonts_grk_data_1bit, (const UINT16*)fonts_grk_name_1bit, 1},
    {31, 103,    fonts_grk_small_width_1bit,    fonts_grk_small_data_1bit, (const UINT16*)fonts_grk_small_name_1bit, 1},
    {42, 103,    fonts_grk_big_width_1bit,    fonts_grk_big_data_1bit, (const UINT16*)fonts_grk_big_name_1bit, 1},
    {36, 105,    fonts_grk_width_1bit,    fonts_grk_data_1bit, (const UINT16*)fonts_grk_name_1bit, 1},
};

font_t font_cry_1bit[] =
{
    {36, 144,    fonts_cry_width_1bit,    fonts_cry_data_1bit, (const UINT16*)fonts_cry_name_1bit, 1},
    {31, 144,    fonts_cry_small_width_1bit,    fonts_cry_small_data_1bit, (const UINT16*)fonts_cry_small_name_1bit, 1},
    {42, 144,    fonts_cry_big_width_1bit,    fonts_cry_big_data_1bit, (const UINT16*)fonts_cry_big_name_1bit, 1},
    {36, 144,    fonts_cry_width_1bit,    fonts_cry_data_1bit, (const UINT16*)fonts_cry_name_1bit, 1},
};


font_t font_arb_1bit[] =
{
#if 1 //lwj for test
    {36, 458,    fonts_arb_width_1bit,    fonts_arb_data_1bit, (const UINT16*)fonts_arb_name_1bit, 1},
    {31, 458,    fonts_arb_small_width_1bit,    fonts_arb_small_data_1bit, (const UINT16*)fonts_arb_small_name_1bit, 1},
    {42, 458,    fonts_arb_big_width_1bit,    fonts_arb_big_data_1bit, (const UINT16*)fonts_arb_big_name_1bit, 1},
    {31, 458,    fonts_arb_width_1bit,    fonts_arb_data_1bit, (const UINT16*)fonts_arb_name_1bit, 1},
#else
    {33, 455,    fonts_arb_width_1bit,    fonts_arb_data_1bit, fonts_arb_name_1bit, 1},
    {33, 455,    fonts_arb_width_1bit,    fonts_arb_data_1bit, fonts_arb_name_1bit, 1},
    {33, 455,    fonts_arb_width_1bit,    fonts_arb_data_1bit, fonts_arb_name_1bit, 1},
    {33, 455,    fonts_arb_width_1bit,    fonts_arb_data_1bit, fonts_arb_name_1bit, 1},
#endif
};

#endif

font_t *font_array_1bit[]=
{
    font_asc_1bit,
#if(MULTI_MENU_LANG)
    font_lat1_1bit,
    font_late_1bit,
    font_grk_1bit,
    font_cry_1bit,
    font_arb_1bit
#endif
};


font_subt_t font_asc_1bit_subt[] =
{
   /*height start end   count size       width                      data                      code_table      zero_data_num data_offset offset_unit  data_type*/
    {36, 0x0020, 0x007e, 95,   0,  (const UINT8 *)fonts_asc_width_1bit,       (const UINT8 *)fonts_asc_data_1bit,      (const UINT16 *)fonts_asc_name_1bit,       NULL,       NULL,       0,            2},
    {31, 0x0020, 0x007e, 96,   0,  (const UINT8 *)fonts_asc_small_width_1bit,       (const UINT8 *)fonts_asc_small_data_1bit,     (const UINT16 *) fonts_asc_small_name_1bit,       NULL,       NULL,       0,            2},
    {42, 0x0020, 0x007f, 96,   0,  (const UINT8 *)fonts_asc_big_width_1bit,   (const UINT8 *)fonts_asc_big_data_1bit,  (const UINT16 *)fonts_asc_big_name_1bit,   NULL,       NULL,       0,            2},
    {36, 0x0020, 0x007e, 95,   0,  (const UINT8 *)fonts_asc_width_1bit,       (const UINT8 *)fonts_asc_data_1bit,      (const UINT16 *)fonts_asc_name_1bit,       NULL,       NULL,       0,            2},
};

#if(MULTI_MENU_LANG)

font_subt_t font_lat1_1bit_subt[] =
{
   /*height start end   count size       width                   data                        code_table      zero_data_num data_offset offset_unit  data_type*/
    {36, 0x00a1, 0x00ff, 95,   0,  (const UINT8 *)fonts_lat1_width_1bit,      (const UINT8 *)fonts_lat1_data_1bit,     (const UINT16 *)fonts_lat1_name_1bit,      NULL,       NULL,       0,            2},
    {31, 0x00a1, 0x00ff, 95,   0,  (const UINT8 *)fonts_lat1_small_width_1bit,      (const UINT8 *)fonts_lat1_small_data_1bit,     (const UINT16 *)fonts_lat1_small_name_1bit,      NULL,       NULL,       0,            2},
    {42, 0x00a1, 0x00ff, 95,   0,  (const UINT8 *)fonts_lat1_big_width_1bit,  (const UINT8 *)fonts_lat1_big_data_1bit, (const UINT16 *)fonts_lat1_big_name_1bit,  NULL,       NULL,       0,            2},
    {36, 0x00a1, 0x00ff, 95,   0,  (const UINT8 *)fonts_lat1_width_1bit,      (const UINT8 *)fonts_lat1_data_1bit,     (const UINT16 *)fonts_lat1_name_1bit,      NULL,       NULL,       0,            2},
};

font_subt_t font_late_1bit_subt[] =
{
   /*height start end   count size       width                   data                        code_table      zero_data_num data_offset offset_unit  data_type*/
    {36, 0x0100, 0x017f, 128,  0,  (const UINT8 *)fonts_late_width_1bit,      (const UINT8 *)fonts_late_data_1bit,     (const UINT16 *)fonts_late_name_1bit,      NULL,       NULL,       0,            2},
    {31, 0x0100, 0x017f, 128,  0,  (const UINT8 *)fonts_late_small_width_1bit,     (const UINT8 *)fonts_late_small_data_1bit,     (const UINT16 *)fonts_late_small_name_1bit,      NULL,       NULL,       0,            2},
    {42, 0x0100, 0x017f, 128,  0,  (const UINT8 *)fonts_late_big_width_1bit,  (const UINT8 *)fonts_late_big_data_1bit, (const UINT16 *)fonts_late_big_name_1bit,  NULL,       NULL,       0,            2},
    {36, 0x0100, 0x017f, 128,  0,  (const UINT8 *)fonts_late_width_1bit,      (const UINT8 *)fonts_late_data_1bit,     (const UINT16 *)fonts_late_name_1bit,      NULL,       NULL,       0,            2},
};

font_subt_t font_grk_1bit_subt[] =
{
   /*height start end   count size       width                   data                        code_table      zero_data_num data_offset offset_unit  data_type*/
    {36, 0x0374, 0x03ea, 105,  0,  (const UINT8 *)fonts_grk_width_1bit,       (const UINT8 *)fonts_grk_data_1bit,      (const UINT16 *)fonts_grk_name_1bit,       NULL,       NULL,       0,            2},
    {31, 0x0374, 0x03ea, 103,  0,  (const UINT8 *)fonts_grk_small_width_1bit,       (const UINT8 *)fonts_grk_small_data_1bit,      (const UINT16 *)fonts_grk_small_name_1bit,       NULL,       NULL,       0,            2},
    {42, 0x0374, 0x03ea, 103,  0,  (const UINT8 *)fonts_grk_big_width_1bit,       (const UINT8 *)fonts_grk_big_data_1bit,      (const UINT16 *)fonts_grk_big_name_1bit,       NULL,       NULL,       0,            2},
    {36, 0x0374, 0x03ea, 105,  0,  (const UINT8 *)fonts_grk_width_1bit,       (const UINT8 *)fonts_grk_data_1bit,      (const UINT16 *)fonts_grk_name_1bit,       NULL,       NULL,       0,            2},
};

font_subt_t font_cry_1bit_subt[] =
{
   /*height start end   count size       width                   data                        code_table      zero_data_num data_offset offset_unit  data_type*/
    {36, 0x0401, 0x0491, 144,  0,  (const UINT8 *)fonts_cry_width_1bit,       (const UINT8 *)fonts_cry_data_1bit,      (const UINT16 *)fonts_cry_name_1bit,       NULL,       NULL,       0,            2},
    {31, 0x0401, 0x0491, 144,  0,  (const UINT8 *)fonts_cry_small_width_1bit,       (const UINT8 *)fonts_cry_small_data_1bit,      (const UINT16 *)fonts_cry_small_name_1bit,       NULL,       NULL,       0,            2},
    {42, 0x0401, 0x0491, 144,  0,  (const UINT8 *)fonts_cry_big_width_1bit,       (const UINT8 *)fonts_cry_big_data_1bit,      (const UINT16 *)fonts_cry_big_name_1bit,       NULL,       NULL,       0,            2},
    {36, 0x0401, 0x0491, 144,  0,  (const UINT8 *)fonts_cry_width_1bit,       (const UINT8 *)fonts_cry_data_1bit,      (const UINT16 *)fonts_cry_name_1bit,       NULL,       NULL,       0,            2},
};


font_subt_t font_arb_1bit_subt[] =
{
   /*height start end   count size       width                   data                        code_table      zero_data_num data_offset offset_unit  data_type*/
    {36, 0x060c, 0xfefc, 458,  0,  (const UINT8 *)fonts_arb_width_1bit,       (const UINT8 *)fonts_arb_data_1bit,      (const UINT16 *)fonts_arb_name_1bit,       NULL,       NULL,       0,            2},
    {31, 0x060c, 0xfefc, 458,  0,  (const UINT8 *)fonts_arb_small_width_1bit,       (const UINT8 *)fonts_arb_small_data_1bit,      (const UINT16 *)fonts_arb_small_name_1bit,       NULL,       NULL,       0,            2},
    {42, 0x060c, 0xfefc, 458,  0,  (const UINT8 *)fonts_arb_big_width_1bit,       (const UINT8 *)fonts_arb_big_data_1bit,      (const UINT16 *)fonts_arb_big_name_1bit,       NULL,       NULL,       0,            2},
    {31, 0x060c, 0xfefc, 458,  0,  (const UINT8 *)fonts_arb_width_1bit,       (const UINT8 *)fonts_arb_data_1bit,      (const UINT16 *)fonts_arb_name_1bit,       NULL,       NULL,       0,            2},
};

font_subt_t font_heb_1bit_subt[] =
{
   /*height start end   count size       width                   data                        code_table      zero_data_num data_offset offset_unit  data_type*/
    {24, 0x05d0, 0x05ea, 27,  0,  (const UINT8 *)fonts_heb_width_1bit,       (const UINT8 *)fonts_heb_data_1bit,      (const UINT16 *)fonts_heb_name_1bit,       NULL,       NULL,       0,            2},
    {24, 0x05d0, 0x05ea, 27,  0,  (const UINT8 *)fonts_heb_width_1bit,       (const UINT8 *)fonts_heb_data_1bit,      (const UINT16 *)fonts_heb_name_1bit,       NULL,       NULL,       0,            2},
    {24, 0x05d0, 0x05ea, 27,  0,  (const UINT8 *)fonts_heb_width_1bit,       (const UINT8 *)fonts_heb_data_1bit,      (const UINT16 *)fonts_heb_name_1bit,       NULL,       NULL,       0,            2},
    {24, 0x05d0, 0x05ea, 27,  0,  (const UINT8 *)fonts_heb_width_1bit,       (const UINT8 *)fonts_heb_data_1bit,      (const UINT16 *)fonts_heb_name_1bit,       NULL,       NULL,       0,            2},
};
#endif

font_subt_t *font_array_1bit_subt[]=
{
    font_asc_1bit_subt,
#if(MULTI_MENU_LANG)
    font_lat1_1bit_subt,
    font_late_1bit_subt,
    font_grk_1bit_subt,
    font_cry_1bit_subt,
    font_arb_1bit_subt,
    font_heb_1bit_subt  //cl 06/29
#endif
};

#define FONT_MAX_SIZE 4

