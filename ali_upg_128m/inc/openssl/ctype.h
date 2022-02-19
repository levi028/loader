
#ifndef Ctype_H__
#define Ctype_H__

#define in_range(c, lo, up)  ((unsigned char)c >= lo && (unsigned char)c <= up)
#define isprint(c)           in_range(c, 0x20, 0x7f)
#define isdigit(c)           in_range(c, '0', '9')
#define isxdigit(c)          (isdigit(c) || in_range(c, 'a', 'f') || in_range(c, 'A', 'F'))
#define islower(c)           in_range(c, 'a', 'z')
#define isspace(c)           (c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v')

#define tolower(c)             ((unsigned char)(c + 'a' - 'A'))
//int tolower(int ch);

#endif
