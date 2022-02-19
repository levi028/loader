  /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: utils.h
*
*    Description: This file describes utils api.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef __MEDIA_PLAYER_UTILS_H_
#define __MEDIA_PLAYER_UTILS_H_
#ifdef __cplusplus
extern "C"
{
#endif


struct file_type_map
{
    FILE_TYPE   filetype;
    char        *ext;
};

typedef enum
{
    DEV_USB,
    DEV_SD,
    DEV_HD,
    DEV_DLNA,
    DEV_SATA,
    DEV_UNKNOWN
}e_device_type;

struct device_type_map
{
    e_device_type devtype;
    char *dev;
};

FILE_TYPE file_map_type(file_list_type list_type, char *ext_name);
char *create_path_by_combination(const char *dirpath, const char *name, const char *name2);
char *path_combination(char *full_path, const char *dirpath, const char *name, const char *name2);
void free_combination_path(char *str);
e_device_type get_device_type(const char *name, char *index);
FILE_TYPE get_file_type(char *ext, const struct file_type_map *support_list);
int strcmp_c(const char *s1, const char *s2);
BOOL  dirs_skipped(const char * dir);
#ifndef _USE_32M_MEM_
BOOL try_get_key_val(char * string, int string_len, const char *keyword, unsigned char * keyval);
BOOL try_get_key_str(char * string, int string_len, const char *keyword, int * offset);
int write_val_record(char *pbuf, const char *keyword, unsigned long keyval);
int write_str_record(char *pbuf, const char *keyword, const char *str, int len);
void write_txt_line(char *dest_buffer,  int len);
#endif
void print_playlist(const P_PLAY_LIST playlist);
int check_file_type(FILE_TYPE type, play_list_type play_list_type);

#ifndef _USE_32M_MEM_
extern const struct file_type_map g_no32msupport_file_types[];
#endif

#ifdef __cplusplus
}
#endif
#endif
