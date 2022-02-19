/*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: lib_char.h
 *
 *    Description: This header file declares all charactor translation APIs.
 *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
 *****************************************************************************/

#include <basic_types.h>

#ifndef __LIB_CHAR_H__
#define __LIB_CHAR_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define LIBCHAR_PR(...) do{}while(0);


/* Reference code value */
#define RC_ISO_8859_1   0x01
#define RC_ISO_8859_2   0x02
#define RC_ISO_8859_3   0x03
#define RC_ISO_8859_4   0x04
#define RC_ISO_8859_5   0x05
#define RC_ISO_8859_6   0x06
#define RC_ISO_8859_7   0x07
#define RC_ISO_8859_8   0x08
#define RC_ISO_8859_9   0x09
#define RC_ISO_8859_10  0x0a
#define RC_ISO_8859_11  0x0b
#define RC_ISO_8859_12  0x0c
#define RC_ISO_8859_13  0x0d
#define RC_ISO_8859_14  0x0e
#define RC_ISO_8859_15  0x0f
#define RC_ISO_8859_16  0x10
#define RC_ISO_6937     6937
#define RC_CHN_GB2312   2312

#define PRINTABLE_CHAR  0x20
#define CHAR_LEFT_RND_BRACKET       0x28    // '('
#define CHAR_RIGHT_RND_BRACKET      0x29    // ')'
#define CHAR_LEFT_ANG_BRACKET       0x003C  // '<'
#define CHAR_RIGHT_ANG_BRACKET      0x003E  // '>'
#define CHAR_LEFT_SQU_BRACKET       0x005B  // '['
#define CHAR_RIGHT_SQU_BRACKET      0x005D  // ']'
#define CHAR_RIGHT_OPEN_BRACKET     0x007B  // '{'
#define CHAR_RIGHT_CLOSE_BRACKET    0x007D  // '}'
#define CHAR_CARRIAGE_RETURN        0x0D    // CR
#define CHAR_LINE_FEED              0x0A    // LF
#define ASCII_EXT_BEGIN_CHAR        0x80    //extention ascii : 0x80 ~ 0xFF


/* EN300 468: Annex A
 * CCT: Character coding table */
#define CCT_IDX_8859_5      0x01
#define CCT_IDX_8859_6      0x02
#define CCT_IDX_8859_7      0x03
#define CCT_IDX_8859_8      0x04
#define CCT_IDX_8859_9      0x05
#define CCT_IDX_8859_10     0x06
#define CCT_IDX_8859_11     0x07
#define CCT_IDX_RESV_8      0x08
#define CCT_IDX_8859_13     0x09
#define CCT_IDX_8859_14     0x0A
#define CCT_IDX_8859_15     0x0B
#define CCT_IDX_RESV_C      0x0C
#define CCT_IDX_RESV_D      0x0D
#define CCT_IDX_RESV_E      0x0E
#define CCT_IDX_RESV_F      0x0F
#define CCT_IDX_8859_MORE   0x10
#define CCT_IDX_10646       0x11
#define CCT_IDX_KSX1001     0x12
#define CCT_IDX_GB2312      0x13
#define CCT_IDX_10646BIG5   0x14
#define CCT_IDX_10646UTF8   0x15
#define CCT_IDX_TYPE_ID     0x1F

#define CCT_BYTE3_ARABIC    0x06
#define CCT_BYTE3_HEBREW    0x08

#define DVB_CHAR_CRLF       0x8A

// forward char: NON, CONS, LV, FV1, FV2, FV3
// dead char: CTRL, BV1, BV2, BD, TONE, AD1, AD2, AD3, AV1, AV2, AV3
struct thai_cell
{
    UINT16 char_num;   // 0~3
    UINT16 unicode[3]; // [0]: forward char; [1]: dead char; [2]: dead char
};


struct devanagari_cell
{
	UINT16	consonant_str[6];
	UINT16	unicode_name;
	UINT16	group_idx;
	UINT16	count;
};

enum devanagari_group
{
	DEVAN_SINGLE = 1,
	DEVAN_DOUBLE,
	DEVAN_TRIPLE,
	DEVAN_QUADRUPLE,
	DEVAN_QUINTUPLE,
	DEVAN_SEXTUPLE
};

struct telugu_cell
{
	UINT16	combination[8];
	UINT16	unicode_name;
	UINT16	group_idx;
	UINT16	count;
};

struct bengali_cell
{
	UINT16	combination[8];
	UINT16	unicode_name;
	UINT16	group_idx;
	UINT16	count;
};

enum telugu_group
{
	TEL_SINGLE = 1,
	TEL_DOUBLE,
	TEL_TRIPLE,
	TEL_QUADRUPLE,
	TEL_QUINTUPLE,
	TEL_SEXTUPLE,
	TEL_SEPTUPLE
};


/**
 * name        : get_iso639_refcode
 * description    : return reference code of ISO8859/ISO6937 according to
 *                iso639 code.
 * parameter    : 1
 *    IN    UINT8 *iso639:    the ISO-639 string, could be 2 or 3 letters.
 * return value    : UINT32
 *    0    : no reference code.
 *    1~15    : the ISO-8859 code page value.
 *    6937    : the ISO-6937 reference code.
 *    other    : reserved.
 */
UINT32 get_iso639_refcode(UINT8 *iso639);

/**
 * name        : dvb_to_unicode
 * description : Convert DVB SI code to unicode, ignore what it doesn't support.
 * parameter   : 4
 *    IN    UINT8 *dvb:    the DVB si code string.
 *    IN    INT32 length:    the DVB si code length.
 *    OUT   UINT16 *unicode: generated unicode string.
 *    IN    INT32 maxlen:    the unicode maximum length.
 * return value : INT32
 *    the length of the converted string!
 */

INT32 dvb_to_unicode(UINT8 *dvb, INT32 length, UINT16 *unicode, INT32 maxlen, UINT32 refcode);

/*
 * compare 2 iso639 language codes
 * example:
 *  "ENG" == "eng",    not care small letter and capital letter;
 *  "deu" == "ger",    some language may have multi-abbreviatives.
 */
INT32 compare_iso639_lang(const UINT8 *iso639_1, const UINT8 *iso639_2);

INT32 iso8859_to_unicode(const UINT8 *dvb, INT32 length, UINT16 *unicode, INT32 maxlen, UINT32 code);

INT32 convert_utf8_to_gb2312(const UINT8 *utf8, INT32 utf_len, UINT8 *gb2312, INT32 g_maxlen, INT32 uni_maxlen);
INT32 convert_gb2312_to_utf8(const UINT8 *gb2312, INT32 g_len, UINT8 *utf8, INT32 utf_maxlen);

/* thai_get_cell
 * Get one thailand cell from text string buffer.
 * Don't use this API any more, it is not safe because not figure out text buffer's len.
 * Use thai_get_cell_ext replace it.
 */
UINT32 thai_get_cell(UINT8  *p_text, struct thai_cell *cell);

/* thai_get_cell_ext
 * Get one thailand cell from text string buffer.
 * This is safe version of thai_get_cell. Use it replace thai_get_cell.
 */
UINT32 thai_get_cell_ext(UINT8 *ptext, UINT32 text_siz, struct thai_cell *cell);
BOOL is_thai_unicode(UINT16 code);
UINT32 thai_get_cell_size(UINT8  *p_text, UINT32 text_siz);

void persian_process(unsigned short *str, unsigned int size);

void arabic_process(unsigned short *str, unsigned int size);


#ifdef __cplusplus
 }
#endif

#endif /* __LIB_CHAR_H__ */
