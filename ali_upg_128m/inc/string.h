#ifndef __STRING_H__
#define __STRING_H__

#include <sys_config.h>
#include <types.h>
#include <retcode.h>
#include <api/libc/string.h>

#ifdef __cplusplus
extern "C" {
#endif

int strcmp(const char *dest, const char *src);
char *ali_strerror(int errnum);
int ali_gethostname(char *name, size_t len);
int ali_sethostname(const char *name, size_t len);
int xdigit2hex(char c);
int xdigit2bin(char *str, int str_len, unsigned char *bin);

#ifdef __cplusplus
}
#endif



#endif
