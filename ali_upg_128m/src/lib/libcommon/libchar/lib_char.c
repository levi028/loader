/*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: lib_char.c
 *
 *    Description: DVB stream charset conversion to unicode utility.
 *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
 *****************************************************************************/

#include <types.h>
#include <sys_config.h>
#include <retcode.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libsi/si_config.h>
#include <api/libchar/lib_char.h>
#include "char_map.h"
#include "iso_5937.h"
#include "gb_2312.h"
#include "arabic_convert.h"
#if defined(GB2312_SUPPORT)
#include "gb_2312.h"
#endif
#include "iso_8859_2.h"
#include "iso_8859_3.h"
#include "iso_8859_4.h"
#include "iso_8859_6.h"
#include "iso_8859_7.h"
#include "iso_8859_8.h"
#include "iso_8859_9.h"
#include "iso_8859_10.h"
#include "iso_8859_11.h"
#include "iso_8859_13.h"
#include "iso_8859_14.h"
#include "iso_8859_15.h"
#include "iso_8859_16.h"
#include "CodePage_1256.h"

#define ISO_8859_EXT_UND_BEGIN  0x80
#define ISO_8859_EXT_UND_END    0x9F
// Count the number of bytes of a UTF-8 character
#define utf8_char_len(c)    ((((UINT32) 0xE5000000 >> ((c >> 3) & 0x1E)) & 3) + 1)

enum ISO_639_LANG_ID
{
    ISO_639_NONE        = 0,
    ISO_639_ARABIC      = 1,
    ISO_639_CHINESE     = 2,
    ISO_639_CZECH       = 3,
    ISO_639_DANISH,
    ISO_639_GERMAN,
    ISO_639_DUTCH,
    ISO_639_GREEK,
    ISO_639_ENGLISH,
    ISO_639_SPANISH,
    ISO_639_ESTONIAN,
    ISO_639_FINNISH,
    ISO_639_FRENCH,
    ISO_639_IRISH,
    ISO_639_HEBREW,
    ISO_639_HUNGARIAN,
    ISO_639_ICELANDIC,
    ISO_639_ITALIAN,
    ISO_639_KURDISH,
    ISO_639_NORWEGIAN,
    ISO_639_PERSIAN,
    ISO_639_POLISH,
    ISO_639_PORTUGUESE,
    ISO_639_ROMANIAN,
    ISO_639_RUSSIAN,
    ISO_639_SCOTS,
    ISO_639_SLOVAK,
    ISO_639_SLOVENIAN,
    ISO_639_SWEDISH,
    ISO_639_TURKISH,
    ISO_639_SORBIAN
};
struct ISO_639_LANG
{
    //iso 639 language code, no matter small letter and capital letter
    UINT8   lang[3];

    //unique id, see enum ISO_639_LANG_ID
    UINT16  id;
};
#ifdef EPG_LATIN_SUBSETS_SUPPORT
#define NAT_OPT_NATION_MAX  14
#define NAT_OPT_CHAR_MAX    13

/* ก่  */
#define NAT_OPT_SPECIAL_CHAR_24 0x24
/*  |  */
#define NAT_OPT_SPECIAL_CHAR_7C 0x7C
/* ก๖  */
#define NAT_OPT_SPECIAL_CHAR_7F 0x7F

/*
 *  ETS 300 706 Table 36: Latin National Option Sub-sets
 *
 *  Latin G0 character code to Unicode mapping per national subset,
 *  unmodified codes (NO_SUBSET) in row zero.
 *
 *  [13][0] Turkish currency symbol not in Unicode, use private code U+E800
 */
static const UINT16 national_subset[NAT_OPT_NATION_MAX][NAT_OPT_CHAR_MAX] =
{
    { 0x0023u, 0x0024u, 0x0040u, 0x005Bu, 0x005Cu, 0x005Du,
        0x005Eu, 0x005Fu, 0x0060u, 0x007Bu, 0x007Cu, 0x007Du, 0x007Eu },
    { 0x0023u, 0x016Fu, 0x010Du, 0x0165u, 0x017Eu, 0x00FDu,
        0x00EDu, 0x0159u, 0x00E9u, 0x00E1u, 0x011Bu, 0x00FAu, 0x0161u },
    { 0x00A3u, 0x0024u, 0x0040u, 0x2190u, 0x00BDu, 0x2192u,
        0x2191u, 0x0023u, 0x2014u, 0x00BCu, 0x2016u, 0x00BEu, 0x00F7u },
    { 0x0023u, 0x00F5u, 0x0160u, 0x00C4u, 0x00D6u, 0x017Du,
        0x00DCu, 0x00D5u, 0x0161u, 0x00E4u, 0x00F6u, 0x017Eu, 0x00FCu },
    { 0x00E9u, 0x00EFu, 0x00E0u, 0x00EBu, 0x00EAu, 0x00F9u,
        0x00EEu, 0x0023u, 0x00E8u, 0x00E2u, 0x00F4u, 0x00FBu, 0x00E7u },
    { 0x0023u, 0x0024u, 0x00A7u, 0x00C4u, 0x00D6u, 0x00DCu,
        0x005Eu, 0x005Fu, 0x00B0u, 0x00E4u, 0x00F6u, 0x00FCu, 0x00DFu },
    { 0x00A3u, 0x0024u, 0x00E9u, 0x00B0u, 0x00E7u, 0x2192u,
        0x2191u, 0x0023u, 0x00F9u, 0x00E0u, 0x00F2u, 0x00E8u, 0x00ECu },
    { 0x0023u, 0x0024u, 0x0160u, 0x0117u, 0x0229u, 0x017Du,
        0x010Du, 0x016Bu, 0x0161u, 0x0105u, 0x0173u, 0x017Eu, 0x012Fu },
    { 0x0023u, 0x0144u, 0x0105u, 0x01B5u, 0x015Au, 0x0141u,
        0x0107u, 0x00F3u, 0x0119u, 0x017Cu, 0x015Bu, 0x0142u, 0x017Au },
    { 0x00E7u, 0x0024u, 0x00A1u, 0x00E1u, 0x00E9u, 0x00EDu,
        0x00F3u, 0x00FAu, 0x00BFu, 0x00FCu, 0x00F1u, 0x00E8u, 0x00E0u },
    { 0x0023u, 0x00A4u, 0x0162u, 0x00C2u, 0x015Eu, 0x01CDu,
        0x00CDu, 0x0131u, 0x0163u, 0x00E2u, 0x015Fu, 0X01CEu, 0x00EEu },
    { 0x0023u, 0x00CBu, 0x010Cu, 0x0106u, 0x017Du, 0x00D0u,
        0x0160u, 0x00EBu, 0x010Du, 0x0107u, 0x017Eu, 0x00F0u, 0x0161u },
    { 0x0023u, 0x00A4u, 0x00C9u, 0x00C4u, 0x00D6u, 0x00C5u,
        0x00DCu, 0x005Fu, 0x00E9u, 0x00E4u, 0x00F6u, 0x00E5u, 0x00FCu },
    { 0xE800u, 0x011Fu, 0x0130u, 0x015Eu, 0x00D6u, 0x00C7u,
        0x00DCu, 0x011Eu, 0x0131u, 0x015Fu, 0x00F6u, 0x00E7u, 0x00FCu }
};
static UINT8        _g_subset = 0;  // subset that selected.

#endif

#if defined(ISO8859_SUPPORT)
static UINT16   *iso8859_convert_map[] =
{
    NULL,
    NULL,   /* same as ASCII code map */
    iso8859_2_map_a0_ff,
    iso8859_3_map_a0_ff,
    iso8859_4_map_a0_ff,
    NULL,   /* use code to lookup since it has some rules to follow */
    iso8859_6_map_a0_ff,
    iso8859_7_map_a0_ff,
    iso8859_8_map_a0_ff,
    iso8859_9_map_a0_ff,
    iso8859_10_map_a0_ff,
    iso8859_11_map_a0_ff,
    NULL,   /* indian charset, have no clue of this. */
    iso8859_13_map_a0_ff,
    iso8859_14_map_a0_ff,
    iso8859_15_map_a0_ff,
    iso8859_16_map_a0_ff,
};
#endif


//infact, only multi-abbreviatives of the language needs this table.
static const struct ISO_639_LANG    iso639_lang[] =
{
    //Arabic
    { "ara", ISO_639_ARABIC },

    //Chinese
    { "chi", ISO_639_CHINESE },

    //Czech
    { "cze", ISO_639_CZECH },

    //Danish
    { "dan", ISO_639_DANISH },

    //German
    { "deu", ISO_639_GERMAN },

    //Dutch
    { "dut", ISO_639_DUTCH },

    //Greek, Modern (1453-)
    { "ell", ISO_639_GREEK },

    //English
    { "eng", ISO_639_ENGLISH },

    //Spanish
    { "esl", ISO_639_SPANISH },

    //Estonian
    { "est", ISO_639_ESTONIAN },

    //Persian
    { "fas", ISO_639_PERSIAN },

    //Finnish
    { "fin", ISO_639_FINNISH },

    //French
    { "fra", ISO_639_FRENCH },

    //French
    { "fre", ISO_639_FRENCH },

    //Irish
    { "gai", ISO_639_IRISH },

    //German
    { "ger", ISO_639_GERMAN },

    //Greek, Modern (1453-)
    { "gre", ISO_639_GREEK },

    //Hebrew
    { "heb", ISO_639_HEBREW },

    //Hungarian
    { "hun", ISO_639_HUNGARIAN },

    //Icelandic
    { "ice", ISO_639_ICELANDIC },

    //Irish
    { "iri", ISO_639_IRISH },

    //Icelandic
    { "isl", ISO_639_ICELANDIC },

    //Italian
    { "ita", ISO_639_ITALIAN },

    //Kurdish
    { "kur", ISO_639_KURDISH },

    //Dutch
    { "nla", ISO_639_DUTCH },

    //Norwegian (Nynorsk)
    { "nno", ISO_639_NORWEGIAN },

    //Norwegian
    { "nor", ISO_639_NORWEGIAN },

    //Persian
    { "per", ISO_639_PERSIAN },

    //Polish
    { "pol", ISO_639_POLISH },

    //Portuguese
    { "por", ISO_639_PORTUGUESE },

    //Romanian
    { "ron", ISO_639_ROMANIAN },

    //Romanian
    { "rum", ISO_639_ROMANIAN },

    //Russian
    { "rus", ISO_639_RUSSIAN },

    //Scots
    { "sco", ISO_639_SCOTS },

    //Slovak
    { "slk", ISO_639_SLOVAK },

    //Slovak
    { "slo", ISO_639_SLOVAK },

    //Slovenian
    { "slv", ISO_639_SLOVENIAN },

    //Spanish
    { "spa", ISO_639_SPANISH },

    //Swedish
    { "sve", ISO_639_SWEDISH },

    //Swedish
    { "swe", ISO_639_SWEDISH },

    //Turkish
    { "tur", ISO_639_TURKISH },

    //Sorbian Languages
    { "wen", ISO_639_SORBIAN },

    //Chinese
    { "zho", ISO_639_CHINESE },

    //Iranian
    { "ira", ISO_639_PERSIAN },
};
static const INT32  iso639_lang_cnt = (INT32)(sizeof(iso639_lang) / sizeof(iso639_lang[0]));

#if defined(ISO8859_SUPPORT)
static INT32    _iso8859_to_unicode(const UINT8 *dvb, INT32 length, UINT16 *unicode, INT32 maxlen, UINT32 code);
#ifdef EPG_LATIN_SUBSETS_SUPPORT
static INT32    _vbi_iso8859_to_unicode(const UINT8 *dvb, INT32 length, UINT16 *unicode, INT32 maxlen, UINT32 code);
#endif
static INT32    _big5_to_unicode(const UINT8 *dvb, INT32 length, UINT16 *unicode, INT32 maxlen);
static INT32    _utf8_to_unicode(const UINT8 *src, INT32 srclen, UINT16 *dst, INT32 maxlen);
static INT32    _dvb_to_unicode(UINT8 *dvb, INT32 length, UINT16 *unicode, INT32 maxlen, UINT32 refcode, UINT8 vbi);
#endif

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
UINT32 get_iso639_refcode(UINT8 *iso639)
{
    /*1;, 8859-1 can be replaced by 8859-15*/
    UINT32  refcode = RC_ISO_8859_15;

    if (NULL == iso639)
    {
        LIBCHAR_PR("%s ARG NULL\n", __FUNCTION__);
        refcode = 0;
    }
    else if ((0 == MEMCMP(iso639, "slk", 3))
         || (0 == MEMCMP(iso639, "SLK", 3))
         || (0 == MEMCMP(iso639, "pol", 3))
         || (0 == MEMCMP(iso639, "POL", 3))
         || (0 == MEMCMP(iso639, "slo", 3))
         || (0 == MEMCMP(iso639, "SLO", 3)))
    {
        refcode = RC_ISO_8859_2;     // ISO/IEC 8859-2
    }
    else if ((0 == MEMCMP(iso639, "rus", 3))
         || (0 == MEMCMP(iso639, "RUS", 3))
         || (0 == MEMCMP(iso639, "wen", 3))
         || (0 == MEMCMP(iso639, "WEN", 3)))
    {
        refcode = RC_ISO_8859_5;     // ISO/IEC 8859-5
    }
    else if ((0 == MEMCMP(iso639, "ara", 3))
         || (0 == MEMCMP(iso639, "ARA", 3))
         || (0 == MEMCMP(iso639, "per", 3))
         || (0 == MEMCMP(iso639, "PER", 3))
         || (0 == MEMCMP(iso639, "fas", 3))
         || (0 == MEMCMP(iso639, "FAS", 3))
         || (0 == MEMCMP(iso639, "ira", 3))
         || (0 == MEMCMP(iso639, "IRA", 3)))
    {
        refcode = RC_ISO_8859_6;     // ISO/IEC 8859-6 Latin/Arabic alphabet
    }
    else if ((0 == MEMCMP(iso639, "gre", 3)) || (0 == MEMCMP(iso639, "GRE", 3)))
    {
        refcode = RC_ISO_8859_7;     // ISO/IEC 8859-7 Latin/Greek alphabet
    }
    else if ((0 == MEMCMP(iso639, "heb", 3)) || (0 == MEMCMP(iso639, "HEB", 3)))
    {
        refcode = RC_ISO_8859_8;     // ISO/IEC 8859-8 Latin/Hebrew alphabet
    }
    else if ((0 == MEMCMP(iso639, "tur", 3))
         || (0 == MEMCMP(iso639, "TUR", 3))
         || (0 == MEMCMP(iso639, "kur", 3))
         || (0 == MEMCMP(iso639, "KUR", 3)))
    {
        refcode = RC_ISO_8859_9;     // ISO/IEC 8859-9
    }
    else if ((0 == MEMCMP(iso639, "dan", 3))
         || (0 == MEMCMP(iso639, "DAN", 3))
         || (0 == MEMCMP(iso639, "ice", 3))
         || (0 == MEMCMP(iso639, "ICE", 3))
         || (0 == MEMCMP(iso639, "isl", 3))
         || (0 == MEMCMP(iso639, "ISL", 3))
         || (0 == MEMCMP(iso639, "nor", 3))
         || (0 == MEMCMP(iso639, "NOR", 3))
         || (0 == MEMCMP(iso639, "nno", 3))
         || (0 == MEMCMP(iso639, "NNO", 3))
         || (0 == MEMCMP(iso639, "sve", 3))
         || (0 == MEMCMP(iso639, "SVE", 3))
         || (0 == MEMCMP(iso639, "swe", 3))
         || (0 == MEMCMP(iso639, "SWE", 3)))
    {
        refcode = RC_ISO_8859_10;     // ISO/IEC 8859-10
    }
    else if ((0 == MEMCMP(iso639, "fin", 3))
         || (0 == MEMCMP(iso639, "FIN", 3))
         || (0 == MEMCMP(iso639, "est", 3))
         || (0 == MEMCMP(iso639, "EST", 3)))
    {
        refcode = RC_ISO_8859_13;     // ISO/IEC 8859-13
    }
    else if((0 == MEMCMP(iso639, "tha", 3))
         || (0 == MEMCMP(iso639, "THA", 3)))
    {
        refcode = RC_ISO_8859_11; // ISO/IEC 8859-11 Latin/Thai
    }
    else if ((0 == MEMCMP(iso639, "por", 3))
         || (0 == MEMCMP(iso639, "POR", 3))
         || (0 == MEMCMP(iso639, "spa", 3))
         || (0 == MEMCMP(iso639, "SPA", 3))
         || (0 == MEMCMP(iso639, "esl", 3))
         || (0 == MEMCMP(iso639, "ESL", 3))
         || (0 == MEMCMP(iso639, "fra", 3))
         || (0 == MEMCMP(iso639, "fre", 3))
         || (0 == MEMCMP(iso639, "ger", 3))
         || (0 == MEMCMP(iso639, "GER", 3))
         || (0 == MEMCMP(iso639, "sco", 3))
         || (0 == MEMCMP(iso639, "SCO", 3))
         || (0 == MEMCMP(iso639, "iri", 3))
         || (0 == MEMCMP(iso639, "IRI", 3))
         || (0 == MEMCMP(iso639, "dan", 3))
         || (0 == MEMCMP(iso639, "DAN", 3))
         || (0 == MEMCMP(iso639, "dut", 3))
         || (0 == MEMCMP(iso639, "DUT", 3))
         || (0 == MEMCMP(iso639, "nla", 3))
         || (0 == MEMCMP(iso639, "NLA", 3))
         || (0 == MEMCMP(iso639, "deu", 3))
         || (0 == MEMCMP(iso639, "DEU", 3)))
    {
        refcode = RC_ISO_8859_15;     // ISO/IEC 8859-15
    }
    else if ((0 == MEMCMP(iso639, "hun", 3))
         || (0 == MEMCMP(iso639, "HUN", 3))
         || (0 == MEMCMP(iso639, "ron", 3))
         || (0 == MEMCMP(iso639, "RON", 3))
         || (0 == MEMCMP(iso639, "rum", 3))
         || (0 == MEMCMP(iso639, "RUM", 3))
         || (0 == MEMCMP(iso639, "ita", 3))
         || (0 == MEMCMP(iso639, "ITA", 3))
         || (0 == MEMCMP(iso639, "slv", 3))
         || (0 == MEMCMP(iso639, "SLV", 3)))
    {
        refcode = RC_ISO_8859_16;     // ISO/IEC 8859-16
#if defined(GB2312_SUPPORT) || defined(CHINESE_SUPPORT)
    }
    else if (0 == (MEMCMP(iso639, "chi", 3)) || (0 == MEMCMP(iso639, "zho", 3)))
    {
#ifdef CHINESE_SUPPORT
        refcode = CCT_IDX_10646UTF8;     // for utf-8 chinese
#else
        refcode = CCT_IDX_GB2312;     // GB2312
#endif
#endif
    }
    else if ((0 == MEMCMP(iso639, "cze", 3))
         || (0 == MEMCMP(iso639, "CZE", 3))
         || (0 == MEMCMP(iso639, "eng", 3))
         || (0 == MEMCMP(iso639, "wel", 3))
         || (0 == MEMCMP(iso639, "ces", 3))
         || (0 == MEMCMP(iso639, "ces", 3)))
    {
        refcode = RC_ISO_6937;
    }

    return refcode;
}

/**
 * name        : dvb_to_unicode
 * description    : Convert DVB SI code to unicode, ignore what
 *                it doesn't support.
 * parameter    : 4
 *    IN    UINT8 *dvb:    the DVB si code string.
 *    IN    INT32 length:    the DVB si code length.
 *    OUT   UINT16 *unicode: generated unicode string.
 *    IN    INT32 maxlen:    the unicode maximum length.
 *    IN    UINT32 refcode:  the reference code
 * return value : INT32
 *    RET_SUCCESS    : the DVB string is partially or wholy converted.
 *    other    : the DVB string can not be converted at all!
 */
INT32 dvb_to_unicode(UINT8 *dvb, INT32 length, UINT16 *unicode, INT32 maxlen, UINT32 refcode)
{
    INT32 ret = RET_FAILURE;

    if (NULL == dvb)
    {
        LIBCHAR_PR("%s ARG NULL\n", __FUNCTION__);
        return ret;
    }
    return _dvb_to_unicode(dvb, length, unicode, maxlen, refcode, 0);
}

static UINT32 unicode_str_len(const UINT16 *string)
{
    UINT32  i   = 0;

    if (NULL == string)
    {
        return 0;
    }

    while (string[i])
    {
        i++;
    }
    return i;
}

static void convert_unicode_str_pos(UINT16 *str)
{
    UINT16  temp = 0;

    if (NULL == str)
    {
        return;
    }

    while (*str != 0)
    {
        temp = *str;
        *str = (temp << 8) | (temp >> 8);   //change between big_end and little_end
        str++;
    }
}

static INT32 _dvb_to_unicode(UINT8 *dvb, INT32 len, UINT16 *unicode, INT32 maxlen, UINT32 refcode, UINT8 vbi)
{
    INT32   si = 0;
    INT32   di = 0;
    INT32   nlen = 0;
    UINT8   code = 0;
    UINT16  c8859 = 0;
    UINT16  tmp = 0;
    
    if((!dvb) || (!unicode))
    {
        return -1;
    }
    if (len < 1)
    {
        *unicode = 0x0000;
        return 0;
    }
    code = dvb[0]; // first byte, select character code table. EN300 468: Annex A
    if (code >= PRINTABLE_CHAR)
    {
        /* use refcode or ISO-6937, because the first byte does not indicate character table */
        if ((refcode > 0) && (refcode <= RC_ISO_8859_16))
        {
            si--;
            if (RC_ISO_8859_1 == refcode)
            {   // use 8859-15 replace 8859-1
                refcode = RC_ISO_8859_15;
            }
            /* compatible with dvb spec */
            code = refcode >= RC_ISO_8859_5 ? (refcode - 0x04) : (refcode + 0xb);
        }
#ifdef ISO_5937_SUPPORT
        else if ((0 == refcode) || (RC_ISO_6937 == refcode))
        {
            /* refcode==0, when search programme */
            // set ISO-6937 as default character coding table
            return iso6937_convert_ch(dvb, len, unicode, maxlen);
        }
#endif
        else
        {       // code > 0x10, so : 0x15(unicode), 0x13 or 2312
            code = refcode;
#ifdef CHINESE_SUPPORT
            /* Because some tradition chinese stream use UTF-8, but
             * first byte not indicate about this, so we withdrawal one byte */
            if (CCT_IDX_10646UTF8 == refcode)
            {
                si--;
            }
#endif
#ifdef GB2312_SUPPORT
            if ((CCT_IDX_GB2312 == refcode) || (RC_CHN_GB2312 == refcode))
            {   // gb2312
                return convert_gb2312_to_unicode(dvb, len, unicode, maxlen);
            }
#endif
        }
    }
    else if (CCT_IDX_RESV_E == code)  // utf-8
    {
        code = 0x15;
    }
#ifdef ISO_5937_SUPPORT
    else if ((CCT_IDX_8859_5 == code) && (RC_ISO_8859_2 == refcode))
    {
        /* some Polish stream give misleading code = 1 that incicate selecting Russian character table
         * but infact that stream use ISO 6937 character set so here will patch for Polish. */
        nlen = len - 1;
        // set ISO-6937 as default character coding table
        return iso6937_convert_ch(dvb + 1, nlen, unicode, maxlen);
    }
#endif
    switch (code)
    {
    case CCT_IDX_RESV_D:  /*8859-2*/
    case CCT_IDX_RESV_E:  /*8859-3*/
    case CCT_IDX_RESV_F:  /*8859-4*/
        code -= 0x0b;
        si += 1;
#ifdef EPG_LATIN_SUBSETS_SUPPORT
        if (1 == vbi)
        {
            nlen = len - si;
            di = _vbi_iso8859_to_unicode(dvb + si, nlen, unicode, maxlen, code);
        }
#endif
        if (0 == vbi)
        {
            nlen = len - si;
            di = _iso8859_to_unicode(dvb + si, nlen, unicode, maxlen, code);
        }
        break;
    case CCT_IDX_8859_5:  /*8859-5*/
    case CCT_IDX_8859_6:  /*8859-6*/
    case CCT_IDX_8859_7:  /*8859-7*/
    case CCT_IDX_8859_8:  /*8859-8*/
    case CCT_IDX_8859_9:  /*8859-9*/
    case CCT_IDX_8859_10:  /*8859-10*/
    case CCT_IDX_8859_11:  /*8859-11*/
    case CCT_IDX_8859_13:  /*8859-13*/
    case CCT_IDX_8859_14:  /*8859-14*/
    case CCT_IDX_8859_15:  /*8859-15*/
    case CCT_IDX_RESV_C:  /*8859-16*/
        code += 4;
        si += 1;
#ifdef EPG_LATIN_SUBSETS_SUPPORT
        if (1 == vbi)
        {
            nlen = len - si;
            di = _vbi_iso8859_to_unicode(dvb + si, nlen, unicode, maxlen, code);
        }
#endif
        if (0 == vbi)
        {
            nlen = len - si;
            di = _iso8859_to_unicode(dvb + si, nlen, unicode, maxlen, code);
        }
        break;
    case CCT_IDX_8859_MORE:  /* ISO/IEC 8859 */
        c8859 = (dvb[1] << 8) | dvb[2];
        si += 3;
#ifdef EPG_LATIN_SUBSETS_SUPPORT
        if (1 == vbi)
        {
            nlen = len - si;
            di = _vbi_iso8859_to_unicode(dvb + si, nlen, unicode, maxlen, c8859);
        }
#endif
        if (0 == vbi)
        {
            nlen = len - si;
            di = _iso8859_to_unicode(dvb + si, nlen, unicode, maxlen, c8859);
        }
        break;
    case CCT_IDX_10646:  /* ISO-10646-1 Basic Multilingual Plane*/
        for (di = 0, si = 1; (si < len - 1) && (di < maxlen); si += 2)
        {
            tmp = (UINT16) (dvb[si] << 8 | dvb[si + 1]);    // maybe have bug here
            if (((tmp != 0) && (tmp < 0xE080)) || (tmp > 0xE09F))
            {
                unicode[di] = U16_TO_UNICODE(tmp);
                di++;
            }
            else if (0xE08A == tmp)
            {
                unicode[di] = U16_TO_UNICODE((UINT16) '\n');
                di++;
            }
        }

        if (di >= maxlen)
        {
            di = maxlen - 1;
        }
        unicode[di] = 0x0000; // make loop: di < (maxlen-1), to avoid unicode[di] overflow
        break;
#ifdef GB2312_SUPPORT

    case CCT_IDX_GB2312:  /*GB-2312-1980*/
        di = convert_gb2312_to_unicode(&dvb[1], len - 1, unicode, maxlen);
        break;
#endif
    case CCT_IDX_10646BIG5:  /*ISO/IEC 10646-1, Big5*/
        si += 1;
        di = _big5_to_unicode(dvb + si, len - si, unicode, maxlen);
        break;
    case CCT_IDX_10646UTF8:  //UTF-8 encode in single byte when 0x15
        si += 1;
        di = _utf8_to_unicode(dvb + si, len - si, unicode, maxlen);
        break;
    default:
        *unicode = 0x0000;
        di = 0;
        break;
    }
    /*code == 0x15 && refcode == 0x06  Arabic(Persian) with UTF8 encoding */
    /*8859-6 for Arabic & 8859-8 for Hebrew*/
    if ((CCT_IDX_8859_10 == code)
    || (CCT_IDX_RESV_8 == code)
    || ((CCT_IDX_10646UTF8 == code) && (RC_ISO_8859_6 == refcode))
    || ((CCT_IDX_8859_14 == code) && ((CCT_BYTE3_ARABIC == c8859) || (CCT_BYTE3_HEBREW == c8859))))
    {
#if (SYS_CPU_ENDIAN == ENDIAN_LITTLE)
        convert_unicode_str_pos(unicode);
#endif
        if ((CCT_IDX_10646UTF8 == code) || (CCT_IDX_RESV_8 == code) || (CCT_BYTE3_HEBREW == c8859))
        {
            // modify punctuation marks for Hebrew -> <> & () & [] & {}
            hebrew_punctuation_process(unicode, di);
        }
        if ((CCT_IDX_10646UTF8 == code) && (RC_ISO_8859_6 == refcode))
        {
            persian_process(unicode, di);
        }
        else
        {
            arabic_process(unicode, di);
        }
#if (SYS_CPU_ENDIAN == ENDIAN_LITTLE)
        convert_unicode_str_pos(unicode);
#endif
        di = unicode_str_len(unicode);
    }
    return di;
}

#ifdef EPG_LATIN_SUBSETS_SUPPORT
UINT8 get_latin_national_subset(UINT8 *lang)
{
    UINT32  i;
    struct
    {
        UINT8   lang[3];
        UINT8   n;
    }

    lang_set[] =
    {
        {"ces", 1,},    /*Czech*/
        {"cze", 1,},    /*Czech*/
        {"slk", 1,},    /*Slovak*/
        {"slo", 1,},    /*Slovak*/
        {"eng", 2,},    /*English*/
        {"est", 3,},    /*Estonian*/
        {"fra", 4,},    /*French*/
        {"fre", 4,},    /*French*/
        {"deu", 5,},    /*German*/
        {"ger", 5,},    /*German*/
        {"ita", 6,},    /*Italian*/
        //{"???", 7,},    /*Lettish*/
        {"lit", 7,},    /*Lithuanian*/
        {"pol", 8,},    /*Polish*/
        {"por", 9,},    /*Portuguese*/
        {"esl", 9,},    /*Spanish*/
        {"spa", 9,},    /*Spanish*/
        {"ron", 10,},    /*Rumanian*/
        {"rum", 10,},    /*Rumanian*/
        //{"???", 11,},    /*Serbian*/
        {"scr", 11,},    /*Croatian*/
        {"slv", 11,},    /*Slovenian*/
        {"sve", 12,},    /*Swedish*/
        {"swe", 12,},    /*Swedish*/
        {"fin", 12,},    /*Finnish*/
        {"tur", 13,},    /*Turkish*/
    };
    if (lang == NULL)
    {
        return 0;
    }

    for (i = 0; i < 24; i++)
    {
        if (MEMCMP(lang_set[i].lang, lang, 3) == 0)
        {
            return lang_set[i].n;
        }
    }

    return 0;
}

/*
 * Translate national subset Teletext character code to Unicode.
 * n: national subset
 * c: character
 */
static UINT16 vbi_national_subset_teletext_unicode(UINT8 n, UINT8 c)
{
    INT32   i;

    /* shortcut */
    if (c >= PRINTABLE_CHAR && (0xF8000019UL & (1 << (c & 31))))
    {
        if ((n > 0) && (n < NAT_OPT_NATION_MAX))
        {
            for (i = 0; i < NAT_OPT_CHAR_MAX; i++)
            {
                if (c == national_subset[0][i])
                {
                    return national_subset[n][i];
                }
            }
        }

        if (NAT_OPT_SPECIAL_CHAR_24 == c)
        {
            return 0x00A4u;// ก่
        }
        else if (NAT_OPT_SPECIAL_CHAR_7C == c)
        {
            return 0x00A6u; // |
        }
        else if (NAT_OPT_SPECIAL_CHAR_7F == c)
        {
            return 0x25A0u;// ก๖
        }
    }

    return U16_TO_UNICODE((UINT16) c);
}

INT32 dvb_vbi_to_unicode(UINT8 *dvb, INT32 length, UINT16 *unicode, INT32 maxlen, UINT32 refcode, UINT8 subset)
{
    INT32 ret = RET_FAILURE;

    if (NULL == dvb)
    {
        return ret;
    }
    _g_subset = subset;

    return _dvb_to_unicode(dvb, length, unicode, maxlen, refcode, 1);
}

static INT32 _vbi_iso8859_to_unicode(const UINT8 *dvb, INT32 length, UINT16 *unicode, INT32 maxlen, UINT32 code)
{
    INT32   di = 0;
    INT32   si = 0;
    UINT16  value = 0;

    if((!dvb) || (!unicode))
    {
        return -1;
    }
    if (RC_ISO_8859_1 == code)       // ISO-8859-1 is ASCII
    {
        for (di = 0; (si < length) && (di < maxlen); si++)
        {
            if (DVB_CHAR_CRLF == dvb[si])
            {               //CR/LF
                unicode[di] = U16_TO_UNICODE((UINT16) '\n');
                di++;
            }
            else
            {
                value = vbi_national_subset_teletext_unicode(_g_subset, dvb[si]);
                unicode[di] = value;
                di++;
            }
        }
    }
    else if (RC_ISO_8859_5 == code)  // ISO-8859-5
    {
        for (di = 0; (si < length) && (di < maxlen); si++)
        {
            if ((dvb[si] < ASCII_EXT_BEGIN_CHAR) && (dvb[si] >= PRINTABLE_CHAR) && (_g_subset > 0))
            {
                value = vbi_national_subset_teletext_unicode(_g_subset, dvb[si]);
                unicode[di] = value;
                di++;
            }
            else if ((0xA0 == dvb[si]) || (0xAD == dvb[si]))
            {
                unicode[di] = U16_TO_UNICODE((UINT16) dvb[si]);
                di++;
            }
            else if (0xF0 == dvb[si])
            {
                unicode[di] = U16_TO_UNICODE(0x2116);
                di++;
            }
            else if (0xFD == dvb[si])
            {
                unicode[di] = U16_TO_UNICODE(0x00A7);
                di++;
            }
            else if (dvb[si] > ISO_8859_EXT_UND_END)
            {
                unicode[di] = U16_TO_UNICODE(dvb[si] + 0x360);
                di++;
            }
            else if (DVB_CHAR_CRLF == dvb[si])
            {   //CR/LF
                unicode[di] = U16_TO_UNICODE((UINT16) '\n');
                di++;
            }
        }

        if (di >= maxlen)
        {
            di = maxlen - 1;
        }
        unicode[di] = 0x0000;
    }
    else if ((code >= RC_ISO_8859_2) && (code <= RC_ISO_8859_16))  // ISO-8859-[2,3,4,6,..,11,13,14,15,16]
    {
        for (di = 0; (si < length) && (di < maxlen); si++)
        {
            if ((dvb[si] < ASCII_EXT_BEGIN_CHAR) && (dvb[si] >= PRINTABLE_CHAR))
            {
                value = vbi_national_subset_teletext_unicode(_g_subset, dvb[si]);
                unicode[di] = value;
                di++;
            }

#if defined(ISO8859_SUPPORT)
            else if (dvb[si] > ISO_8859_EXT_UND_END)
            {
                value = U16_TO_UNICODE(iso8859_convert_map[code][dvb[si] - 0xA0]);
                unicode[di] = value;
                di++;
            }
#endif
            else if (DVB_CHAR_CRLF == dvb[si])
            {
                unicode[di] = U16_TO_UNICODE((UINT16) '\n');
                di++;
            }
        }

        if (di >= maxlen)
        {
            di = maxlen - 1;
        }
        unicode[di] = 0x0000;
    }

    return di;
}
#endif //EPG_LATIN_SUBSETS_SUPPORT
static INT32 _iso8859_to_unicode(const UINT8 *dvb, INT32 length, UINT16 *unicode, INT32 maxlen, UINT32 code)
{
    INT32   di = 0;
    INT32   si = 0;
    UINT16  value = 0;

    if((!dvb) || (!unicode))
    {
        return -1;
    }
    if (RC_ISO_8859_1 == code)       // ISO-8859-1 is ASCII
    {
        for (di = 0; (si < length) && (di < maxlen); si++)
        {
            if (DVB_CHAR_CRLF == dvb[si])
            {               //CR/LF
                unicode[di] = U16_TO_UNICODE((UINT16) '\n');
                di++;
            }
            else
            {
                unicode[di] = U16_TO_UNICODE((UINT16) dvb[si]);
                di++;
            }
        }
    }
    else if (RC_ISO_8859_5 == code)  // ISO-8859-5
    {
        for (di = 0; (si < length) && (di < maxlen); si++)
        {
            if ((dvb[si] < ASCII_EXT_BEGIN_CHAR) && (dvb[si] >= PRINTABLE_CHAR))
            {
                unicode[di] = U16_TO_UNICODE((UINT16) dvb[si]);
                di++;
            }
            else if ((0xA0 == dvb[si]) || (0xAD == dvb[si]))
            {
                unicode[di] = U16_TO_UNICODE((UINT16) dvb[si]);
                di++;
            }
            else if (0xF0 == dvb[si])
            {
                unicode[di] = U16_TO_UNICODE((UINT16) 0x2116);
                di++;
            }
            else if (0xFD == dvb[si])
            {
                unicode[di] = U16_TO_UNICODE((UINT16) 0x00A7);
                di++;
            }
            else if (dvb[si] > ISO_8859_EXT_UND_END)
            {
                unicode[di] = U16_TO_UNICODE((UINT16) (dvb[si] + 0x360));
                di++;
            }
            else if (DVB_CHAR_CRLF == dvb[si])
            {           //CR/LF
                unicode[di] = U16_TO_UNICODE((UINT16) '\n');
                di++;
            }
        }

        if (di >= maxlen)
        {
            di = maxlen - 1;
        }
        unicode[di] = 0x0000;
    }
    else if ((RC_ISO_8859_2 >= 0x02) && (RC_ISO_8859_16 <= 0x10))
    {                   // ISO-8859-[2,3,4,6,..,11,13,14,15,16]
        for (di = 0; (si < length) && (di < maxlen); si++)
        {
            if ((dvb[si] < ASCII_EXT_BEGIN_CHAR) && (dvb[si] >= PRINTABLE_CHAR))
            {
                unicode[di] = U16_TO_UNICODE((UINT16) dvb[si]);
                di++;
            }
            else if (dvb[si] > ISO_8859_EXT_UND_END)
            {
                value = iso8859_convert_map[code][dvb[si] - 0xA0];
                unicode[di] = U16_TO_UNICODE(value);
                di++;
            }
		//Ben 151231#1,for jiuzhou M3281P beijing Conax pt	
            //else if (DVB_CHAR_CRLF == dvb[si])
	 	else if ((DVB_CHAR_CRLF == dvb[si])||(0x0A == dvb[si]))
            {
                unicode[di] = U16_TO_UNICODE((UINT16) '\n');
                di++;
            }
		//Ben 151231#1		
        }

        if (di >= maxlen)
        {
            di = maxlen - 1;
        }
        unicode[di] = 0x0000;
    }

    return di;
}

INT32 iso8859_to_unicode(const UINT8 *dvb, INT32 length, UINT16 *unicode, INT32 maxlen, UINT32 code)
{
    INT32   di = 0;
    INT32   si = 0;
    UINT16  value = 0;

    if ((NULL == dvb) || (NULL == unicode))
    {
        LIBCHAR_PR("%s ARG NULL\n", __FUNCTION__);
        return 0;
    }

    if (RC_ISO_8859_1 == code)   // ISO-8859-1 is ASCII
    {
        for (di = 0; (si < length) && (di < maxlen); si++)
        {
            if (DVB_CHAR_CRLF == dvb[si])
            {           //CR/LF
                unicode[di] = U16_TO_UNICODE((UINT16) '\n');
                di++;
            }
            else
            {
                unicode[di] = U16_TO_UNICODE((UINT16) dvb[si]);
                di++;
            }
        }
    }
    else if (RC_ISO_8859_5 == code)  // ISO-8859-5
    {
        for (di = 0; (si < length) && (di < maxlen); si++)
        {
            if ((dvb[si] < ASCII_EXT_BEGIN_CHAR) && (dvb[si] >= PRINTABLE_CHAR))
            {
                unicode[di] = U16_TO_UNICODE((UINT16) dvb[si]);
                di++;
            }
            else if ((0xA0 == dvb[si]) || (0xAD == dvb[si]))
            {
                unicode[di] = U16_TO_UNICODE((UINT16) dvb[si]);
                di++;
            }
            else if (0xF0 == dvb[si])
            {
                unicode[di] = U16_TO_UNICODE((UINT16) 0x2116);
                di++;
            }
            else if (0xFD == dvb[si])
            {
                unicode[di] = U16_TO_UNICODE((UINT16) 0x00A7);
                di++;
            }
            else if (dvb[si] > ISO_8859_EXT_UND_END)
            {
                unicode[di] = U16_TO_UNICODE((UINT16) (dvb[si] + 0x360));
                di++;
            }
            else if (DVB_CHAR_CRLF == dvb[si])
            {   //CR/LF
                unicode[di] = U16_TO_UNICODE((UINT16) '\n');
                di++;
            }
        }

        if (di >= maxlen)
        {
            di = maxlen - 1;
        }
        unicode[di] = 0x0000;
    }
    else if ((code >= RC_ISO_8859_2) && (code <= RC_ISO_8859_16))
    {           // ISO-8859-[2,3,4,6,..,11,13,14,15,16]
        for (di = 0; (si < length) && (di < maxlen); si++)
        {
            if (code != RC_ISO_8859_6)  //cmx0801
            {
                if ((dvb[si] < ASCII_EXT_BEGIN_CHAR) && (dvb[si] >= PRINTABLE_CHAR))
                {
                    unicode[di] = U16_TO_UNICODE((UINT16) dvb[si]);
                    di++;
                }

#if defined(ISO8859_SUPPORT)
                else if (dvb[si] > ISO_8859_EXT_UND_END)
                {
                    value = iso8859_convert_map[code][dvb[si] - 0xA0];
                    unicode[di] = U16_TO_UNICODE(value);
                    di++;
                }
#endif
                else if (DVB_CHAR_CRLF == dvb[si])
                {
                    unicode[di] = U16_TO_UNICODE((UINT16) '\n');
                    di++;
                }
                else if ((CHAR_CARRIAGE_RETURN == dvb[si]) || (CHAR_LINE_FEED == dvb[si]))
                {           //cmx0801
                    unicode[di] = U16_TO_UNICODE((UINT16) dvb[si]);
                    di++;
                }
            }
            else
            {
                //if arabic,persian using codepage1256,
                //please refer to the code as below.
                if ((dvb[si] < ASCII_EXT_BEGIN_CHAR) && (dvb[si] >= PRINTABLE_CHAR))
                {
                    unicode[di] = U16_TO_UNICODE((UINT16) dvb[si]);
                    di++;
                }
                else if (dvb[si] >= ASCII_EXT_BEGIN_CHAR)
                {
                    value = codepage_1256_map_80_ff[dvb[si] - ASCII_EXT_BEGIN_CHAR];
                    unicode[di] = U16_TO_UNICODE(value);
                    di++;
                }
                else if ((CHAR_CARRIAGE_RETURN == dvb[si]) || (CHAR_LINE_FEED == dvb[si]))
                {           //cmx0801
                    unicode[di] = U16_TO_UNICODE((UINT16) dvb[si]);
                    di++;
                }
            }
        }

        if (di >= maxlen)
        {
            di = maxlen - 1;
        }
        unicode[di] = 0x0000;
    }

    return di;
}

static INT32 _big5_to_unicode(const UINT8 *dvb, INT32 length, UINT16 *unicode, INT32 maxlen)
{
    INT32   di = 0;
    INT32   si = 0;

    if ((NULL == dvb) || (NULL == unicode))
    {
        LIBCHAR_PR("%s ARG NULL\n", __FUNCTION__);
        return 0;
    }
    while ((si < length - 1) && (di < maxlen))  // avoid dvb[si+1] overflow
    {
        if ((0xE0 == dvb[si]) && (dvb[si + 1] >= ASCII_EXT_BEGIN_CHAR) && (dvb[si + 1] <= ISO_8859_EXT_UND_END))
        {
            si++;           /* control code, ignore them now. */
        }
        else if ((0xff == dvb[si]) && (dvb[si + 1] <= 0x5f))
        {
            //temp use because we have no full-width font
            unicode[di] = (dvb[si + 1] + PRINTABLE_CHAR) << 8;
            si++;
            di++;
        }
        else
        {
            unicode[di] = ((((UINT16) dvb[si + 1]) << 8) | (UINT16) dvb[si]);
            si++;
            di++;
        }

        //si++;
    }

    if (di >= maxlen)
    {
        di = maxlen - 1;
    }
    unicode[di] = 0x0000;
    return di;
}

static INT32 _utf8_to_unicode(const UINT8 *src, INT32 srclen, UINT16 *dst, INT32 maxlen)
{
    INT32   src_idx = 0;
    INT32   dst_idx = 0;
    UINT16  unicode = 0;
    UINT8   i = 0;

    if ((NULL == src) || (NULL == dst))
    {
        LIBCHAR_PR("%s ARG NULL\n", __FUNCTION__);
        return 0;
    }
    while ((src_idx < srclen) && (dst_idx < (maxlen - 1)))
    {
        if ((src_idx + utf8_char_len(src[src_idx])) > (UINT32)srclen)
        {
            break;
        }

        for (i = 0; i < 8; i++)
        {
            if (0 == (src[src_idx] & ((UINT8) ASCII_EXT_BEGIN_CHAR >> i)))
            {
                break;
            }
        }

        switch (i)
        {
        case 0:     // ascii
            unicode = U16_TO_UNICODE((UINT16) src[src_idx]);
            src_idx++;
            break;

        case 1:     // unknow character
            unicode = 0xFFFD;
            src_idx += 1;
            break;

        case 2:     // 2 byte
            if ((src[src_idx + 1] & 0xC0) != ASCII_EXT_BEGIN_CHAR)
            {
                unicode = 0xFFFD;
                src_idx += 1;
                break;
            }

            unicode = ((UINT16) (src[src_idx] & 0x1F)) << 6;
            unicode = unicode | (UINT16) (src[src_idx + 1] & 0x3F);
            unicode = U16_TO_UNICODE(unicode);
            src_idx += 2;
            break;

        case 3:     // 3 byte
            if (((src[src_idx + 1] & 0xC0) != 0x80) || ((src[src_idx + 2] & 0xC0) != ASCII_EXT_BEGIN_CHAR))
            {
                unicode = 0xFFFD;
                src_idx += 1;
                break;
            }

            unicode = ((UINT16) (src[src_idx] & 0x0F)) << 12;
            unicode = unicode | ((UINT16) (src[src_idx + 1] & 0x3F)) << 6;
            unicode = unicode | (UINT16) (src[src_idx + 2] & 0x3F);
            unicode = U16_TO_UNICODE(unicode);
            src_idx += 3;
            break;

        default:    // no process for bytes>3 utf8 character
            unicode = 0xFFFD;
            src_idx += i;
            break;
        }

        dst[dst_idx] = unicode;
        dst_idx++;
    }

    dst[dst_idx] = 0x0000;
    return dst_idx;
}

static UINT16 *hostendian_to_utf8(UINT8 *str, UINT16 *uni_str)
{
    UINT32  val = 0;

    if ((NULL == str) || (NULL == uni_str))
    {
        return NULL;
    }

    if (0 == (uni_str[0] & 0xff80))
    {
        *str = (UINT8) * uni_str;
        str++;
        uni_str++;
    }
    else if (0 == (uni_str[0] & 0xf800))
    {
        str[0] = 0xc0 | (uni_str[0] >> 6);
        str[1] = 0x80 | (*uni_str &0x3f);
        uni_str++;
        str += 2;
    }
    else if ((uni_str[0] & 0xfc00) != 0xd800)
    {
        str[0] = 0xe0 | (uni_str[0] >> 12);
        str[1] = 0x80 | ((uni_str[0] >> 6) & 0x3f);
        str[2] = 0x80 | (*uni_str &0x3f);
        uni_str++;
        str += 3;
    }
    else
    {
        val = ((uni_str[0] - 0xd7c0) << 10) | (uni_str[1] & 0x3ff);
        str[0] = 0xf0 | (val >> 18);
        str[1] = 0x80 | ((val >> 12) & 0x3f);
        str[2] = 0x80 | ((val >> 6) & 0x3f);
        str[3] = 0x80 | (val & 0x3f);
        uni_str += 2;
        str += 4;
    }

    return uni_str;
}

static void _unicode_to_utf8(const UINT16 *src, INT32 *srclen, char *dst, INT32 *dst_len)
{
    int             j = 0;
    int             __MAYBE_UNUSED__ result  = 0;
    int             origlen = 0;
    int             src_limit = 0;
    int             dst_limit = 0;
    int             src_count = 0;
    int             dst_count = 0;
    int             utf8_len = 0;
    unsigned char   utf8[4] = { 0 };
    unsigned char   *putf8 = utf8;
    unsigned short  *unicode = NULL;

    if ((!src) || (!dst) || (!srclen) || (!dst_len))
    {
        LIBCHAR_PR("%s ARG NULL\n", __FUNCTION__);
        return;
    }
    origlen = *srclen;
    src_limit = *srclen;
    dst_limit = *dst_len;
    for (src_count = 0; src_count < src_limit; src_count++)
    {
        MEMSET(utf8, 0, 4);
        unicode = (unsigned short *) &src[src_count];
        unicode = hostendian_to_utf8(putf8, unicode);
        utf8_len = putf8 - utf8;
        if ((dst_count + utf8_len) > dst_limit)
        {
            break;
        }

        for (j = 0; j < utf8_len; j++)
        {
            dst[dst_count + j] = utf8[j];
        }

        dst_count += utf8_len;
    }

    *srclen = src_count;
    *dst_len = dst_count;
    dst[dst_count] = '\0';
    result = ((dst_count > 0) ? 0 : -1);
    if (*srclen < origlen)
    {
        return;     //-1;
    }

    return;         // result;
}


static void smallletter(UINT8 *dst, const UINT8 *src, UINT32 len)
{
    UINT32  i = 0;

    if ((NULL == dst) || (NULL == src))
    {
        return;
    }

    for (i = 0; i < len; i++)
    {
        if ((src[i] >= 'A') && (src[i] <= 'Z'))
        {
            dst[i] = src[i] + 0x21;
        }
        else
        {
            dst[i] = src[i];
        }
    }
}

static UINT16 get_iso639_lang_id(UINT8 *iso639)
{
    int i = 0;

    if(!iso639)
    {
        return ISO_639_NONE;
    }
    for (i = 0; i < iso639_lang_cnt; i++)
    {
        if (0 == MEMCMP(iso639, iso639_lang[i].lang, 3))
        {
            return iso639_lang[i].id;
        }
    }

    return ISO_639_NONE;
}

/*
 * compare 2 iso639 language codes
 * example:
 *        "ENG" == "eng",    not care small letter and capital letter;
 *        "deu" == "ger",    some language may have multi-abbreviatives.
 */
INT32 compare_iso639_lang(const UINT8 *iso639_1, const UINT8 *iso639_2)
{
    UINT8   lang1[3] = { 0 };
    UINT8   lang2[3] = { 0 };
    INT32   lang1_id = 0;
    INT32   lang2_id = 0;
    INT32   ret = -1;

    if ((NULL == iso639_1) || (NULL == iso639_2))
    {
        LIBCHAR_PR("%s:ARG NULL\n", __FUNCTION__);
        return ret;
    }
    smallletter(lang1, iso639_1, 3);
    smallletter(lang2, iso639_2, 3);
    lang1_id = get_iso639_lang_id(lang1);
    lang2_id = get_iso639_lang_id(lang2);
    if ((lang1_id != ISO_639_NONE) && ((lang2_id) != ISO_639_NONE))
    {
        ret = (lang2_id - lang1_id);
    }
    else
    {
        ret = MEMCMP(iso639_1, iso639_2, 3);
    }

    return ret;
}

INT32 convert_gb2312_to_utf8(const UINT8 *gb2312, INT32 g_len, UINT8 *utf8, INT32 utf_maxlen)
{
    UINT16  *unicode = NULL;
    INT32   uni_len = 0;
    INT32   uni_real_len = 0;
    INT32   i = 0;
    INT32   ret = -1;


    if ((NULL == gb2312) || (NULL == utf8))
    {
        LIBCHAR_PR("%s:ARG NULL\n", __FUNCTION__);
        return ret;
    }

    if (g_len > 0)
    {
        uni_len = 512;
    }
    else
    {
        return ret;
    }

    unicode = (UINT16 *)malloc(uni_len);
    if (NULL == unicode)
    {
        LIBCHAR_PR("%s, unicode malloc error!!!", __FUNCTION__);
        return ret;
    }

    //reduce the space for ota
#ifdef GB2312_SUPPORT
    uni_real_len = convert_gb2312_to_unicode(gb2312, g_len, unicode, uni_len);
    if (0 == uni_real_len)
    {
        free(unicode);
        unicode = NULL;
        LIBCHAR_PR("%s, convert_gb2312_to_unicode error!!!", __FUNCTION__);
        return ret;
    }
#endif
    for (i = 0; i < uni_real_len; i++)
    {
        unicode[i] = cpu_to_be16u(unicode[i]);
    }

    _unicode_to_utf8(unicode, &uni_real_len, (char *)utf8, &utf_maxlen);
    free(unicode);
    unicode = NULL;

    ret = 0;
    return ret;
}

INT32 convert_utf8_to_gb2312(const UINT8 *utf8, INT32 utf_len, UINT8 *gb2312, INT32 g_maxlen, INT32 uni_maxlen)
{
    UINT16  *unicode = NULL;
    INT32   uni_real_len = 0;
    INT32   ret = -1;

    if ((NULL == utf8) || (NULL == gb2312))
    {
        LIBCHAR_PR("%s:ARG NULL\n", __FUNCTION__);
        return ret;
    }

    if (uni_maxlen > 0)
    {
        unicode = (UINT16 *)malloc(uni_maxlen);
    }

    if (NULL == unicode)
    {
        LIBCHAR_PR("%s, unicode malloc error!!!", __FUNCTION__);
        return ret;
    }

    uni_real_len = _utf8_to_unicode(utf8, utf_len, unicode, uni_maxlen);
    if (0 == uni_real_len)
    {
        free(unicode);
        unicode = NULL;
        LIBCHAR_PR("%s, _utf8_to_unicode error!!!", __FUNCTION__);
        return ret;
    }

    //reduce the space for ota
#ifdef GB2312_SUPPORT
    if (0 == convert_unicode_to_gb2312(unicode, uni_real_len, gb2312, g_maxlen))
    {
        free(unicode);
        unicode = NULL;
        LIBCHAR_PR("%s, convert_gb2312_to_unicode error!!!", __FUNCTION__);
        return ret;
    }
#endif
    free(unicode);
    unicode = NULL;

    ret = 0;

    return ret;
}
