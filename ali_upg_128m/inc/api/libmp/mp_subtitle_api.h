/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: mp_subtitle_api.h
*
* Description:
*     media player subtitle process functions
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef __MEDIA_PLAYER_SUBTITLE_API_H_
#define __MEDIA_PLAYER_SUBTITLE_API_H_


#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    UINT16          height;     // font height
    UINT16          start;      // first char
    UINT16          end;        // last char
    UINT16          count;      // char number, the size of code_table
    UINT16          data_size;  // max data size of one char[( (h X max_w) + 7 ) / 8]
    const UINT8     *width;     // variable width table or single width
    const UINT8     *data;      // font data
    const UINT16    *code_table;    // ordered font code look up table(NULL if char is continuous)
    const UINT8     *zero_data_num; // 0 data number on head for compressed data
    const UINT16    *data_offset;   // none zero data offset table
    UINT8           offset_unit;    // 0: data_offset in (UINT16 *)
                                    // 1: data_offset in (UINT32 *)
    UINT8           data_type;      // 0: UINT8
                                    // 1: UINT16
                                    // 2: UINT32
}font_subt_t; // summer.xia 2010.8.25

#define    GET_SUBTITLE_FILE_SIZE                 0x1
#define    GET_SUBTITLE_FILE_TIME                0x2
#define SET_SUBTITLE_STRING_TO_UNICODE      0x3

typedef void (*mp_subtitle_string_to_unicode)(UINT8 *string, INT32 length,  \
                                            UINT16 *unicode, INT32 maxlen, UINT32 refcode);

typedef long (*mp_subtitle_get_time_function)();
typedef long (*mp_subtitle_osd_create_function)(void **psubtitle_osd);
typedef long (*mp_subtitle_osd_destroy_function)(void *subtitle_osd);
typedef long (*mp_subtitle_osd_control_function)(void *subtitle_osd, long control_code, long parameters);

typedef struct
{
    mp_subtitle_get_time_function        get_time;
    mp_subtitle_osd_create_function        osd_create;
    mp_subtitle_osd_destroy_function        osd_destroy;
    mp_subtitle_osd_control_function        osd_control;
    long                                reserved;
    long                                reserved1;
    long                                reserved2;
    long                                reserved3;
    long                                reserved4;
    unsigned char                       *file_buffer;
    unsigned long                       file_buffer_size;
    long                                reserved5;
    long                                reserved6;
    long                                reserved7;
    long                                reserved8;
    mp_subtitle_get_time_function        get_time_ms;
}mp_subtitle_init_parameter, *pmp_subtitle_init_parameter;



typedef struct
{
    void                    *subtitle_osd;
    unsigned short        x;
    unsigned short        y;
    unsigned char            fg_color;
    unsigned short        *string;
}mp_subtitle_draw_string_parameters, *pmp_subtitle_draw_string_parameters;


typedef struct
{
    void                    *subtitle_osd;
    unsigned short        x;
    unsigned short        y;
    unsigned short        width;
    unsigned short        height;
    unsigned char            *image;
    unsigned short      nb_color;
    unsigned char       *paleltte;
}mp_subtitle_draw_image_parameters, *pmp_subtitle_draw_image_parameters;


typedef enum
{
    MP_SUBTITLE_CONTROL_CLEAN_SCREEN,
    MP_SUBTITLE_CHANGE_PALLETE,
    MP_SUBTITLE_DRAW_STRING,
    MP_SUBTITLE_DRAW_IMAGE,
    MP_SUBTITLE_IO_CTRL_EXT, //Application can set some parameter to internal subtitile for single CPU solution
}mp_subtitle_control_code;

typedef struct
{
    void *(*mps_create)(mp_subtitle_init_parameter *config);
    void (*mps_delete)(void *handle);
	BOOL (*mps_find_ext_subtitle)(char search_path[]);
    long (*mps_auto_load)(void *handle, char *video_name);
    void (*mps_unload)(void *handle);
    void (*mps_hide_sub)(void *handle, long hide);
    long (*mps_query_language)(void *handle, long *number, long *current_language_index,    \
                                char *description, long *description_len);
    long (*mps_change_language)(void *handle, long language_index);
    long (*mps_load)(void *handle, char *subtile_name);
    long (*mps_io_ctrl)(void *handle, long io_cmd, long param1, long param2);
    #if 1//joey.che
    int (*mps_get_sub_file_num)(void *handle);
    int (*mps_get_cur_sub_id)(void *handle);
    long (*mps_change_ext_sub_id)(void *handle,char *video_name,int ext_sub_id);
    #endif
} EXT_SUB_PLUGIN;


void *mp_subtitle_create(mp_subtitle_init_parameter *config);
void mp_subtitle_delete(void *handle);
BOOL mp_find_ext_subtitle(char search_path[]);
long mp_subtitle_auto_load(void *handle, char *video_name);
void mp_subtitle_unload(void *handle);
void mp_subtitle_hide_sub(void *handle, long hide);
long mp_subtitle_load(void *handle, char *subtitle_name);
long mp_subtitle_query_language(void *handle, long *number, long *current_language_index,   \
                                char* description, long *description_len);
long mp_subtitle_change_language(void *handle, long language_index);
#if 1//joey.che
int mp_subtile_get_sub_file_num(void *handle);
int mp_subtile_get_cur_sub_id(void *handle);
long mp_subtitle_change_ext_sub_id(void *handle,char *video_name,int ext_sub_id);
#endif
void mp_subtitle_parse_txt(void *sub, char *str, unsigned long *pos, unsigned long *time);


#ifdef __cplusplus
}
#endif


#endif

