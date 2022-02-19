/* THIS FILE WAS AUTOMAGICALLY GENERATED!
   Please modify and use keysets.pl to regenerate it. */

#define CONF_NUMBER        1
#define CONF_UPPER        2
#define CONF_LOWER        4
#define CONF_UNDER        256
#define CONF_PUNCTUATION    512
#define CONF_WS            16
#define CONF_ESC        32
#define CONF_QUOTE        64
#define CONF_DQUOTE        1024
#define CONF_COMMENT        128
#define CONF_FCOMMENT        2048
#define CONF_EOF        8
#define CONF_HIGHBIT        4096
#define CONF_ALPHA        (CONF_UPPER|CONF_LOWER)
#define CONF_ALPHA_NUMERIC    (CONF_ALPHA|CONF_NUMBER|CONF_UNDER)
#define CONF_ALPHA_NUMERIC_PUNCT (CONF_ALPHA|CONF_NUMBER|CONF_UNDER| \
                    CONF_PUNCTUATION)

#define KEYTYPES(c)        ((unsigned short *)((c)->meth_data))
#ifndef CHARSET_EBCDIC
#define IS_COMMENT(c,a)        (KEYTYPES(c)[(a)&0xff]&CONF_COMMENT)
#define IS_FCOMMENT(c,a)    (KEYTYPES(c)[(a)&0xff]&CONF_FCOMMENT)
#define IS_EOF(c,a)        (KEYTYPES(c)[(a)&0xff]&CONF_EOF)
#define IS_ESC(c,a)        (KEYTYPES(c)[(a)&0xff]&CONF_ESC)
#define IS_NUMBER(c,a)        (KEYTYPES(c)[(a)&0xff]&CONF_NUMBER)
#define IS_WS(c,a)        (KEYTYPES(c)[(a)&0xff]&CONF_WS)
#define IS_ALPHA_NUMERIC(c,a)    (KEYTYPES(c)[(a)&0xff]&CONF_ALPHA_NUMERIC)
#define IS_ALPHA_NUMERIC_PUNCT(c,a) \
                (KEYTYPES(c)[(a)&0xff]&CONF_ALPHA_NUMERIC_PUNCT)
#define IS_QUOTE(c,a)        (KEYTYPES(c)[(a)&0xff]&CONF_QUOTE)
#define IS_DQUOTE(c,a)        (KEYTYPES(c)[(a)&0xff]&CONF_DQUOTE)
#define IS_HIGHBIT(c,a)        (KEYTYPES(c)[(a)&0xff]&CONF_HIGHBIT)

#else /*CHARSET_EBCDIC*/

#define IS_COMMENT(c,a)        (KEYTYPES(c)[os_toascii[a]&0xff]&CONF_COMMENT)
#define IS_FCOMMENT(c,a)    (KEYTYPES(c)[os_toascii[a]&0xff]&CONF_FCOMMENT)
#define IS_EOF(c,a)        (KEYTYPES(c)[os_toascii[a]&0xff]&CONF_EOF)
#define IS_ESC(c,a)        (KEYTYPES(c)[os_toascii[a]&0xff]&CONF_ESC)
#define IS_NUMBER(c,a)        (KEYTYPES(c)[os_toascii[a]&0xff]&CONF_NUMBER)
#define IS_WS(c,a)        (KEYTYPES(c)[os_toascii[a]&0xff]&CONF_WS)
#define IS_ALPHA_NUMERIC(c,a)    (KEYTYPES(c)[os_toascii[a]&0xff]&CONF_ALPHA_NUMERIC)
#define IS_ALPHA_NUMERIC_PUNCT(c,a) \
                (KEYTYPES(c)[os_toascii[a]&0xff]&CONF_ALPHA_NUMERIC_PUNCT)
#define IS_QUOTE(c,a)        (KEYTYPES(c)[os_toascii[a]&0xff]&CONF_QUOTE)
#define IS_DQUOTE(c,a)        (KEYTYPES(c)[os_toascii[a]&0xff]&CONF_DQUOTE)
#define IS_HIGHBIT(c,a)        (KEYTYPES(c)[os_toascii[a]&0xff]&CONF_HIGHBIT)
#endif /*CHARSET_EBCDIC*/

static unsigned short CONF_type_default[256]={
    0x0008,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x0010,0x0010,0x0000,0x0000,0x0010,0x0000,0x0000,
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0010,0x0200,0x0040,0x0080,0x0000,0x0200,0x0200,0x0040,
    0x0000,0x0000,0x0200,0x0200,0x0200,0x0200,0x0200,0x0200,
    0x0001,0x0001,0x0001,0x0001,0x0001,0x0001,0x0001,0x0001,
    0x0001,0x0001,0x0000,0x0200,0x0000,0x0000,0x0000,0x0200,
    0x0200,0x0002,0x0002,0x0002,0x0002,0x0002,0x0002,0x0002,
    0x0002,0x0002,0x0002,0x0002,0x0002,0x0002,0x0002,0x0002,
    0x0002,0x0002,0x0002,0x0002,0x0002,0x0002,0x0002,0x0002,
    0x0002,0x0002,0x0002,0x0000,0x0020,0x0000,0x0200,0x0100,
    0x0040,0x0004,0x0004,0x0004,0x0004,0x0004,0x0004,0x0004,
    0x0004,0x0004,0x0004,0x0004,0x0004,0x0004,0x0004,0x0004,
    0x0004,0x0004,0x0004,0x0004,0x0004,0x0004,0x0004,0x0004,
    0x0004,0x0004,0x0004,0x0000,0x0200,0x0000,0x0200,0x0000,
    0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,
    0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,
    0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,
    0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,
    0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,
    0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,
    0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,
    0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,
    0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,
    0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,
    0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,
    0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,
    0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,
    0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,
    0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,
    0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,
    };

static unsigned short CONF_type_win32[256]={
    0x0008,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x0010,0x0010,0x0000,0x0000,0x0010,0x0000,0x0000,
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0010,0x0200,0x0400,0x0000,0x0000,0x0200,0x0200,0x0000,
    0x0000,0x0000,0x0200,0x0200,0x0200,0x0200,0x0200,0x0200,
    0x0001,0x0001,0x0001,0x0001,0x0001,0x0001,0x0001,0x0001,
    0x0001,0x0001,0x0000,0x0A00,0x0000,0x0000,0x0000,0x0200,
    0x0200,0x0002,0x0002,0x0002,0x0002,0x0002,0x0002,0x0002,
    0x0002,0x0002,0x0002,0x0002,0x0002,0x0002,0x0002,0x0002,
    0x0002,0x0002,0x0002,0x0002,0x0002,0x0002,0x0002,0x0002,
    0x0002,0x0002,0x0002,0x0000,0x0000,0x0000,0x0200,0x0100,
    0x0000,0x0004,0x0004,0x0004,0x0004,0x0004,0x0004,0x0004,
    0x0004,0x0004,0x0004,0x0004,0x0004,0x0004,0x0004,0x0004,
    0x0004,0x0004,0x0004,0x0004,0x0004,0x0004,0x0004,0x0004,
    0x0004,0x0004,0x0004,0x0000,0x0200,0x0000,0x0200,0x0000,
    0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,
    0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,
    0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,
    0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,
    0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,
    0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,
    0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,
    0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,
    0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,
    0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,
    0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,
    0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,
    0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,
    0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,
    0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,
    0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,0x1000,
    };

