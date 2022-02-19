 /*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: codepage_1256.h
 *
 *    Description: CodePage_1256 charset mapping file. Refer to microsoft
      codepage_1256 mapping table. More detail information to learn about
      CP1256, please access http://en.wikipedia.org/wiki/Windows-1256
 *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
 *****************************************************************************/

#include <types.h>
#include <sys_config.h>
#include "char_map.h"
#ifdef __cplusplus
extern "C"
{
#endif
static UINT16   codepage_1256_map_80_ff[] =
{
    0x20AC, /*80        */
    0x067E, /*81        */
    0x201A, /*82        */
    0x0192, /*83        */
    0x201E, /*84        */
    0x2026, /*85        */
    0x2020, /*86        */
    0x2021, /*87        */
    0x02C6, /*88        */
    0x2030, /*89        */
    0x0679, /*8A        */
    0x2039, /*8B        */
    0x0152, /*8C        */
    0x0686, /*8D        */
    0x0698, /*8E        */
    0x0688, /*8F        */
    0x06AF, /*90        */
    0x2018, /*91        */
    0x2019, /*92        */
    0x201C, /*93        */
    0x201D, /*94        */
    0x2022, /*95        */
    0x2013, /*96        */
    0x2014, /*97        */
    0x06A9, /*98        */
    0x2122, /*99        */
    0x0691, /*9A        */
    0x203A, /*9B        */
    0x0153, /*9C        */
    0x200C, /*9D        */
    0x200D, /*9E        */
    0x06BA, /*9F        */
    0x00A0, /*A0    NO-BREAK SPACE */
    0x060C, /*A1    INVALID */
    0x00A2, /*A2    INVALID */
    0x00A3, /*A3    INVALID */
    0x00A4, /*A4    CURRENCY SIGN */
    0x00A5, /*A5    INVALID */
    0x00A6, /*A6    INVALID */
    0x00A7, /*A7    INVALID */
    0x00A8, /*A8    INVALID */
    0x00A9, /*A9    INVALID */
    0x06BE, /*AA    INVALID */
    0x00AB, /*AB    INVALID */
    0x00AC, /*AC    ARABIC COMMA */
    0x00AD, /*AD    SOFT HYPHEN */
    0x00AE, /*AE    INVALID */
    0x00AF, /*AF    INVALID */
    0x00B0, /*B0    INVALID */
    0x00B1, /*B1    INVALID */
    0x00B2, /*B2    INVALID */
    0x00B3, /*B3    INVALID */
    0x00B4, /*B4    INVALID */
    0x00B5, /*B5    INVALID */
    0x00B6, /*B6    INVALID */
    0x00B7, /*B7    INVALID */
    0x00B8, /*B8    INVALID */
    0x00B9, /*B9    INVALID */
    0x061B, /*BA    INVALID */
    0x00BB, /*BB    ARABIC SEMICOLON */
    0x00BC, /*BC    INVALID */
    0x00BD, /*BD    INVALID */
    0x00BE, /*BE    INVALID */
    0x061F, /*BF    ARABIC QUESTION MARK */
    0x06C1, /*C0    INVALID */
    0x0621, /*C1    ARABIC LETTER HAMZA */
    0x0622, /*C2    ARABIC LETTER ALEF WITH MADDA ABOVE */
    0x0623, /*C3    ARABIC LETTER ALEF WITH HAMZA ABOVE */
    0x0624, /*C4    ARABIC LETTER WAW WITH HAMZA ABOVE */
    0x0625, /*C5    ARABIC LETTER ALEF WITH HAMZA BELOW */
    0x0626, /*C6    ARABIC LETTER YEH WITH HAMZA ABOVE */
    0x0627, /*C7    ARABIC LETTER ALEF */
    0x0628, /*C8    ARABIC LETTER BEH */
    0x0629, /*C9    ARABIC LETTER TEH MARBUTA */
    0x062A, /*CA    ARABIC LETTER TEH */
    0x062B, /*CB    ARABIC LETTER THEH */
    0x062C, /*CC    ARABIC LETTER JEEM */
    0x062D, /*CD    ARABIC LETTER HAH */
    0x062E, /*CE    ARABIC LETTER KHAH */
    0x062F, /*CF    ARABIC LETTER DAL */
    0x0630, /*D0    ARABIC LETTER THAL */
    0x0631, /*D1    ARABIC LETTER REH */
    0x0632, /*D2    ARABIC LETTER ZAIN */
    0x0633, /*D3    ARABIC LETTER SEEN */
    0x0634, /*D4    ARABIC LETTER SHEEN */
    0x0635, /*D5    ARABIC LETTER SAD */
    0x0636, /*D6    ARABIC LETTER DAD */
    0x00D7, /*D7    ARABIC LETTER TAH */
    0x0637, /*D8    ARABIC LETTER ZAH */
    0x0638, /*D9    ARABIC LETTER AIN */
    0x0639, /*DA    ARABIC LETTER GHAIN */
    0x063A, /*DB    INVALID */
    0x0640, /*DC    INVALID */
    0x0641, /*DD    INVALID */
    0x0642, /*DE    INVALID */
    0x0643, /*DF    INVALID */
    0x00E0, /*E0    ARABIC TATWEEL */
    0x0644, /*E1    ARABIC LETTER FEH */
    0x00E2, /*E2    ARABIC LETTER QAF */
    0x0645, /*E3    ARABIC LETTER KAF */
    0x0646, /*E4    ARABIC LETTER LAM */
    0x0647, /*E5    ARABIC LETTER MEEM */
    0x0648, /*E6    ARABIC LETTER NOON */
    0x00E7, /*E7    ARABIC LETTER HEH */
    0x00E8, /*E8    ARABIC LETTER WAW */
    0x00E9, /*E9    ARABIC LETTER ALEF MAKSURA */
    0x00EA, /*EA    ARABIC LETTER YEH */
    0x00EB, /*EB    ARABIC FATHATAN */
    0x0649, /*EC    ARABIC DAMMATAN */
    0x064A, /*ED    ARABIC KASRATAN */
    0x00EE, /*EE    ARABIC FATHA */
    0x00EF, /*EF    ARABIC DAMMA */
    0x064B, /*F0    ARABIC KASRA */
    0x064C, /*F1    ARABIC SHADDA */
    0x064D, /*F2    ARABIC SUKUN */
    0x064E, /*F3    INVALID */
    0x00F4, /*F4    INVALID */
    0x064F, /*F5    INVALID */
    0x0650, /*F6    INVALID */
    0x00F7, /*F7    INVALID */
    0x0651, /*F8    INVALID */
    0x00F9, /*F9    INVALID */
    0x0652, /*FA    INVALID */
    0x00FB, /*FB    INVALID */
    0x00FB, /*FC    INVALID */
    0x200E, /*FD    INVALID */
    0x200F, /*FE    INVALID */
    0x06D2, /*FF    INVALID */
};
#ifdef __cplusplus
}
#endif
