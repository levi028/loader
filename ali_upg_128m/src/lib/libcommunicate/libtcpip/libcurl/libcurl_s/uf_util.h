#include <sys_config.h>
#include <basic_types.h>
#include <api/libc/alloc.h>
#include <string.h>
#include <stdio.h>

#ifndef _UF_UTIL_H__
#define _UF_UTIL_H__

char *uf_strstr(char *s1, int len, char *s2);
void uf_strstrip(char* str);
char* uf_playlist_get_line(char *buf, char **line);

#endif // _UF_UTIL_H__

