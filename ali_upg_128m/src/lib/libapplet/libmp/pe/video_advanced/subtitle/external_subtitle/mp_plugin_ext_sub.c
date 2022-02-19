/****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.

   *    File: mp_subtitle.c

   *    Description: This file describes media player subtitle file operations.
   *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
         KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
         IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
         PARTICULAR PURPOSE.
   *****************************************************************************/

#include <types.h>
#include <retcode.h>
#include <api/libc/printf.h>

#include <api/libc/string.h>
#include <osal/osal.h>
#include <api/libfs2/stat.h>
#include <api/libfs2/dirent.h>
#include <api/libfs2/statvfs.h>
#include <api/libfs2/unistd.h>
#include <api/libfs2/stdio.h>

#include <api/libmp/media_player_api.h>
#include <api/libmp/mp_subtitle_api.h>
#include "mp_ext_sub_vobsub.h"
#include "mp_plugin_ext_sub.h"
#include "mp_ext_sub_srt.h"
#include "mp_ext_sub_smi.h"
#include "mp_ext_sub_ass.h"
#include "mp_ext_sub_priv.h"

static UINT32 mp_lang_type = 0;    //add by mark

mp_subtitle_string_to_unicode mp_subtitle_callback = NULL;

/**************************Marco  Define***********************************/
//define Marco
#define utf8_to_u_hostendian(str, uni_str, err_flag) \
{\
    err_flag = 0;\
    if (0 == (str[0]&0x80)) {\
        *uni_str++ = *str++;\
    } else if ((str[1] & 0xC0) != 0x80) {\
        *uni_str++ = 0xfffd;\
        str+=1;\
    } else if (0 == (str[0]&0x20)) {\
        *uni_str++ = ((str[0]&31)<<6) | (str[1]&63);\
        str+=2;\
    } else if ((str[2] & 0xC0) != 0x80) {\
        *uni_str++ = 0xfffd;\
        str+=2;\
    } else if (0 == (str[0]&0x10)) {\
        *uni_str++ = ((str[0]&15)<<12) | ((str[1]&63)<<6) | (str[2]&63);\
        str+=3;\
    } else if ((str[3] & 0xC0) != 0x80) {\
        *uni_str++ = 0xfffd;\
        str+=3;\
    } else {\
        err_flag = 1;\
    }\
}

#define utf8_char_len(c) ((((int)0xE5000000 >> ((c >> 3) & 0x1E)) & 3) + 1)

/**************************Global  Variables***********************************/
//define the Global  Variables
static char mp_subtitle_ext[MP_SUBTITLE_TYPE_NUMBER][4] = {
    {'.' , 'S', 'R', 'T'},
    {'.' , 'I', 'D', 'X'},
    {'.' , 'T', 'X', 'T'},
    {'.' , 'S', 'M', 'I'},
    {'.' , 'A', 'S', 'S'},
    {'.' , 'S', 'S', 'A'}
};

/**************************Static  Function Declare***********************************/
#if(SYS_CPU_ENDIAN==ENDIAN_BIG)
static unsigned long mp_subtitle_unicode_string_length(const unsigned short* string);
#endif
static unsigned long mp_subtitle_unicode_string_to_mb(unsigned short* pw_str);
static long mp_subtitle_utf8_to_unicode(const char* src, long* src_len, unsigned short* dst);
static long mp_subtitle_utf8_string_to_unicode(unsigned char* utf8,unsigned short* uni_str);
static long mp_subtitle_output_sub1(mp_subtitle *subtitle);
static long mp_subtitle_output_image1(mp_subtitle *subtitle, FILE *file, struct stat *file_status, \
                        unsigned char *buffer, mp_vob_sub_mpeg *mpeg, mp_vob_sub_spu *spu, unsigned long *delay);
static void mp_subtitle_txt_task(unsigned long param1,unsigned long param2);
static void mp_subtitle_image_task(unsigned long param1,unsigned long param2);
static long mp_subtitle_start(mp_subtitle *subtitle);

/****************************API  Function ***********************************/
/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_create
// Description:
//    Create function
// Arguments:
//    None
// Return Value:
//    Handle
/////////////////////////////////////////////////////////////////////////////
void *mp_subtitle_create(mp_subtitle_init_parameter *config)
{
    mp_subtitle            *subtitle = NULL;

    MP_SUB_DEBUG("====>>mp_subtitle_create()\n");

    if ((!config) \
        || ((!config->get_time) && (!config->get_time_ms)) \
        || (!config->osd_create) \
        || (!config->osd_destroy) \
        || (!config->osd_control))
    {
        MP_SUB_DEBUG("mp_subtitle_create: Invalid Parameters!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_create()\n");
        return NULL;
    }
    subtitle = (mp_subtitle*)malloc(sizeof(mp_subtitle));
    if (!subtitle)
    {
        MP_SUB_DEBUG("mp_subtitle_create: malloc Failed!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_create()\n");
        return NULL;
    }
    MEMSET(subtitle, 0, sizeof (mp_subtitle));
    subtitle->current_sub_file_id = -1;
    if ((config->file_buffer) && (config->file_buffer_size >= 1024*370))
    {
        subtitle->file_buffer = (char*)(config->file_buffer);
        subtitle->file_buffer_size_limit = config->file_buffer_size;
        subtitle->file_buffer_assigned = 1;
    }
    else
    {
        subtitle->file_buffer = (char*)malloc(1024*370);

        /* For Check if MALLOC return value : memory pointer is NULL, Modified by CHEN 20141013  */
        if (NULL == subtitle->file_buffer)
        {        
            MP_SUB_DEBUG("mp_subtitle_create: subtitle->file_buffer is NULL!\n");
            free(subtitle);
            return NULL;
        }
        subtitle->file_buffer_size_limit = 1024*370;
    }
    subtitle->position_offset = (unsigned long*)malloc(MP_SUBTITLE_MAX_SUB_COUNT*sizeof(long));

    /* For Check if MALLOC return value : memory pointer is NULL, Modified by CHEN 20141013  */
    if (NULL == subtitle->position_offset)
    {        
        MP_SUB_DEBUG("mp_subtitle_create: subtitle->position_offset is NULL!\n");
        free(subtitle);
        free(subtitle->file_buffer);
        return NULL;
    }
    subtitle->position_time = (unsigned long*)malloc(MP_SUBTITLE_MAX_SUB_COUNT*sizeof(long));

    /* For Check if MALLOC return value : memory pointer is NULL, Modified by CHEN 20141013  */
    if (NULL == subtitle->position_time)
    {        
        MP_SUB_DEBUG("mp_subtitle_create: subtitle->position_time is NULL!\n");
        free(subtitle);
        free(subtitle->file_buffer);
        free(subtitle->position_offset );
        return NULL;
    }

    subtitle->subtitle_osd_functions.osd_create = config->osd_create;
    subtitle->subtitle_osd_functions.osd_destroy= config->osd_destroy;
    subtitle->subtitle_osd_functions.osd_control= config->osd_control;
    if (config->get_time_ms)
    {
        subtitle->get_time = config->get_time_ms;
        subtitle->time_in_ms = 1;
    }
    else
    {
        subtitle->get_time = config->get_time;
    }

    MP_SUB_DEBUG("<<====mp_subtitle_create()\n");

    return subtitle;
}
/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_delete
//
// Description:
//    Delete function
//
// Arguments:
//    Handle
//
// Return Value:
//    None
//
/////////////////////////////////////////////////////////////////////////////
void mp_subtitle_delete(void *handle)
{
    mp_subtitle            *subtitle = NULL;

    MP_SUB_DEBUG("====>>mp_subtitle_delete()\n");

    if (!handle)
    {
        MP_SUB_DEBUG("mp_subtitle_delete: Invalid Parameters!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_delete()\n");
        return;
    }

    subtitle = (mp_subtitle*)handle;

    mp_subtitle_stop(subtitle);

    if ((!subtitle->file_buffer_assigned) && (subtitle->file_buffer))
    {
        FREE(subtitle->file_buffer);
    }

    if (subtitle->position_offset)
    {
        FREE(subtitle->position_offset);
    }

    if (subtitle->position_time)
    {
        FREE(subtitle->position_time);
    }

    FREE(subtitle);

    MP_SUB_DEBUG("<<====mp_subtitle_delete()\n");

}

/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_auto_load
//
// Description:
//    Auto load a subtitle file
//
// Arguments:
//    Handle
//    Subtitle file name
//
// Return Value:
//      1 - Success
//    -1 - Failed
//
/////////////////////////////////////////////////////////////////////////////
//vic100507#1 begin
static char *strlwr(char *s)
{
    unsigned char al = 0;
    register char *dx = NULL;
    register char *si = NULL;

    dx=s;
    si=s;
    while((al=*si++)!='\0')
    {
        al-='A';
        if(al>'Z'-'A')
        {
           continue;
        }
        si[-1]+='a'-'A';
    }
    return dx;
}
//vic100507#1 end
int mp_subtile_get_sub_file_num(void *handle)
{
    mp_subtitle  *subtitle = NULL;

    subtitle = (mp_subtitle*)handle;
    if(subtitle)
    {
        return subtitle->avail_file_count;
    }
    else
    {
        return 0;
    }
}
int mp_subtile_get_cur_sub_id(void *handle)
{
    mp_subtitle  *subtitle = NULL;

    subtitle = (mp_subtitle*)handle;
    if(subtitle)
    {
        return subtitle->current_sub_file_id;
    }
    else
    {
        return -1;
    }
}
long mp_subtitle_change_ext_sub_id(void *handle,char *video_name,int ext_subld)
{
    mp_subtitle                        *subtitle = NULL;
    char                             *ext = NULL;
    char                                *file_path = NULL; //[FULL_PATH_SIZE];
    long                                index = 0;
    long                                ret = 0;
    struct stat                        file_status;

    MP_SUB_DEBUG("====>>mp_subtitle_auto_load()\n");

    if ((!handle)||(!video_name))
    {
        MP_SUB_DEBUG("mp_subtitle_auto_load: Invalid Parameters!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_auto_load()\n");
        return -1;
    }

    subtitle = (mp_subtitle*)handle;

    file_path = (char *)malloc(FULL_PATH_SIZE);
    if(!file_path)
    {
        return -1;
    }
    //STRCPY(file_path, video_name);
    MEMSET(file_path, 0, FULL_PATH_SIZE);
    strncpy(file_path, video_name, FULL_PATH_SIZE-1);

    ext = strrchr(file_path, (long)'.');
    if (!ext)
    {
        MP_SUB_DEBUG("mp_subtitle_auto_load: Wrong Video File Name!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_auto_load()\n");
        free(file_path);
        file_path = NULL;
        return -1;
    }

    for (index=0; index<MP_SUBTITLE_TYPE_NUMBER; ++index)
    {
        if (NULL == strncpy(ext, (char*)(mp_subtitle_ext+index), 4))
        {
            if (NULL != file_path)
            {
                free(file_path);
                file_path = NULL;
            }
            return -1;
        }
        ret = fs_stat(file_path, &file_status);
        if (ret < 0)
        {
            if (NULL == strlwr(ext))
            {
                if (NULL != file_path)
                {
                    FREE(file_path);
                    file_path = NULL;
                }
                return -1;
            }
            ret = fs_stat(file_path, &file_status);
        }

        if (0 == ret)
        {
            MP_SUB_DEBUG("mp_subtitle_auto_load:file exist! File name: %s\n", file_path);
            ret = mp_subtitle_get_type_by_name(file_path);
            if (ret != MP_SUBTITLE_TYPE_UNKNOWN)
            {
                subtitle->avail_sub_files[subtitle->avail_file_count].avail_file_type = ret;
                //STRCPY(subtitle->avail_sub_files[subtitle->avail_file_count].avail_sub_file, file_path);
                strncpy(subtitle->avail_sub_files[subtitle->avail_file_count].avail_sub_file, file_path,FULL_PATH_SIZE-1);
                subtitle->avail_sub_files[subtitle->avail_file_count].avail_sub_file[FULL_PATH_SIZE-1] = 0;

                subtitle->avail_file_count++;
            }
        }
    }

    subtitle->current_sub_file_id = ext_subld;

    ret = mp_subtitle_parse_file(handle);
    if (ret <0)
    {
        MP_SUB_DEBUG("mp_subtitle_auto_load: Parse Subtitle Failed!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_auto_load()\n");
        if (NULL != file_path)
        {
            FREE(file_path);
            file_path = NULL;
        }
        return -1;
    }

    ret = mp_subtitle_start(subtitle);
    if (ret <0)
    {
        MP_SUB_DEBUG("mp_subtitle_auto_load: Start Subtitle Task Failed!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_auto_load()\n");
        if (NULL != file_path)
        {
            FREE(file_path);
            file_path = NULL;
        }
        return -1;
    }

    MP_SUB_DEBUG("<<====mp_subtitle_auto_load()\n");
    if (NULL != file_path)
    {
        free(file_path);
        file_path = NULL;
    }
    return 1;
}

BOOL mp_find_ext_subtitle(char *search_path)
{
 	BOOL find = FALSE;
	UINT8 index =0;
	char *ext = NULL;
	struct stat	file_status;
	long ret = -1;
	
	MP_SUB_DEBUG("===>find_ext_subtitle\n");
	
	if(search_path!=NULL)
	{
		ext = strrchr(search_path, (long)'.');
		for (index=0; index<MP_SUBTITLE_TYPE_NUMBER; ++index)
		{
			MEMSET(ext, 0, strlen(ext));
			strncpy(ext, (char*)(mp_subtitle_ext+index), 4);
			ret = fs_stat(search_path, &file_status);
			if (ret < 0)
			{
				strlwr(ext);
				ret = fs_stat(search_path, &file_status);
			}
			
			if (ret == 0)
			{
			 	find = TRUE;
				break;
			}
		}
	}
	MP_SUB_DEBUG("<====find_ext_subtitle\n");

	return find;
}

long mp_subtitle_auto_load(void *handle, char *video_name)
{
    mp_subtitle                        *subtitle = NULL;
    char                                *ext = NULL;
    char                                *file_path = NULL; //[FULL_PATH_SIZE];
    long                                index = 0;
    long                                ret = 0;
    struct stat                        file_status;
    int length = 0;

    MP_SUB_DEBUG("====>>mp_subtitle_auto_load()\n");
    if ((!handle)||(!video_name))
    {
        MP_SUB_DEBUG("mp_subtitle_auto_load: Invalid Parameters!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_auto_load()\n");
        return -1;
    }
    subtitle = (mp_subtitle*)handle;
    //MEMSET(&file_path[len], 0, FULL_PATH_SIZE - len);

    file_path = (char *)malloc(FULL_PATH_SIZE);
    if(!file_path)
    {
        return -1;
    }
    //STRCPY(file_path, video_name);
    MEMSET(file_path, 0, FULL_PATH_SIZE);
    strncpy(file_path, video_name, FULL_PATH_SIZE-1);

    ext = strrchr(file_path, (long)'.');
    if (!ext)
    {
        MP_SUB_DEBUG("mp_subtitle_auto_load: Wrong Video File Name!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_auto_load()\n");
        FREE(file_path);
        return -1;
    }
    for (index=0; index<MP_SUBTITLE_TYPE_NUMBER; ++index)
    {
        length = strlen(ext);
        MEMSET(ext, 0, length);
        strncpy(ext, (char*)(mp_subtitle_ext+index), 4);
        ret = fs_stat(file_path, &file_status);
        if (ret < 0)
        {
            if (NULL == strlwr(ext))
            {
                FREE(file_path);
                return -1;
            }
            ret = fs_stat(file_path, &file_status);
        }

        if (0 == ret)
        {
            MP_SUB_DEBUG("mp_subtitle_auto_load:file exist! File name: %s\n", file_path);
            ret = mp_subtitle_get_type_by_name(file_path);
            if (ret != MP_SUBTITLE_TYPE_UNKNOWN)
            {
                subtitle->avail_sub_files[subtitle->avail_file_count].avail_file_type = ret;
                //STRCPY(subtitle->avail_sub_files[subtitle->avail_file_count].avail_sub_file, file_path);
                strncpy(subtitle->avail_sub_files[subtitle->avail_file_count].avail_sub_file, file_path,FULL_PATH_SIZE-1);
                subtitle->avail_sub_files[subtitle->avail_file_count].avail_sub_file[FULL_PATH_SIZE-1] = 0;
                subtitle->avail_file_count++;
            }
        }
    }

    subtitle->current_sub_file_id = 0;

    ret = mp_subtitle_parse_file(handle);
    if (ret <0)
    {
        MP_SUB_DEBUG("mp_subtitle_auto_load: Parse Subtitle Failed!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_auto_load()\n");
        FREE(file_path);
        return -1;
    }

    ret = mp_subtitle_start(subtitle);
    if (ret <0)
    {
        MP_SUB_DEBUG("mp_subtitle_auto_load: Start Subtitle Task Failed!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_auto_load()\n");
        FREE(file_path);
        return -1;
    }

    MP_SUB_DEBUG("<<====mp_subtitle_auto_load()\n");
    if (file_path != NULL)
    {
        free(file_path);
    }

    return 1;
}


long mp_subtitle_load(void *handle, char *subtitle_name)
{
    mp_subtitle                        *subtitle = NULL;
    long                                ret = 0;

    MP_SUB_DEBUG("====>>mp_subtitle_auto_load()\n");

    if ((!handle)||(!subtitle_name))
    {
        MP_SUB_DEBUG("mp_subtitle_auto_load: Invalid Parameters!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_auto_load()\n");
        return -1;
    }

    subtitle = (mp_subtitle*)handle;

     ret = mp_subtitle_get_type_by_name(subtitle_name);
    if (ret != MP_SUBTITLE_TYPE_UNKNOWN)
    {
        subtitle->avail_sub_files[subtitle->avail_file_count].avail_file_type = ret;
        //STRCPY(subtitle->avail_sub_files[subtitle->avail_file_count].avail_sub_file, subtitle_name);
        strncpy(subtitle->avail_sub_files[subtitle->avail_file_count].avail_sub_file, subtitle_name, FULL_PATH_SIZE-1);
        subtitle->avail_sub_files[subtitle->avail_file_count].avail_sub_file[FULL_PATH_SIZE-1] = 0;

        subtitle->avail_file_count++;
    }

    subtitle->current_sub_file_id = 0;

    ret = mp_subtitle_parse_file(handle);
    if (ret <0)
    {
        MP_SUB_DEBUG("mp_subtitle_auto_load: Parse Subtitle Failed!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_auto_load()\n");
        return -1;
    }

    ret = mp_subtitle_start(subtitle);
    if (ret <0)
    {
        MP_SUB_DEBUG("mp_subtitle_auto_load: Start Subtitle Task Failed!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_auto_load()\n");
        return -1;
    }

    MP_SUB_DEBUG("<<====mp_subtitle_auto_load()\n");

    return 1;
}

/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_unload
//
// Description:
//    Unload a subtitle file
//
// Arguments:
//    Handle
//
// Return Value:
//    None
//
/////////////////////////////////////////////////////////////////////////////
void mp_subtitle_unload(void *handle)
{
    mp_subtitle                        *subtitle = NULL;

    MP_SUB_DEBUG("====>>mp_subtitle_unload()\n");

    if ((!handle))
    {
        MP_SUB_DEBUG("mp_subtitle_unload: Invalid Parameters!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_unload()\n");
        return;
    }

    subtitle = (mp_subtitle*)handle;

    mp_subtitle_stop(subtitle);

    MP_SUB_DEBUG("<<====mp_subtitle_unload()\n");

}
/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_hide_sub
//
// Description:
//    Hide subtitle display
//
// Arguments:
//    Handle
//    Hide or Not
//
// Return Value:
//    None
//
/////////////////////////////////////////////////////////////////////////////
void mp_subtitle_hide_sub(void *handle, long hide)
{
    mp_subtitle                        *subtitle = NULL;

    MP_SUB_DEBUG("====>>mp_subtitle_hide_sub()\n");

    if ((!handle))
    {
        MP_SUB_DEBUG("mp_subtitle_hide_sub: Invalid Parameters!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_hide_sub()\n");
        return;
    }

    subtitle = (mp_subtitle*)handle;

    mp_subtitle_pause(subtitle, hide);

    MP_SUB_DEBUG("<<====mp_subtitle_hide_sub()\n");

}
/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_query_language
//
// Description:
//    Query language infomation in subtitle file
//
// Arguments:
//    handle - Handle
//    number - Language Number
//    description - Language description
//    description_len - Length of language description. Should >= 4*number
//
// Return Value:
//      1 - Success
//    -1 - Failed
//
/////////////////////////////////////////////////////////////////////////////
long mp_subtitle_query_language(void *handle, long *number, long *current_language_index, \
                                        char *description, long *description_len)
{
    mp_subtitle                        *subtitle = NULL;
    long                                ret = 0;

    MP_SUB_DEBUG("====>>mp_subtitle_change_language()\n");

    if ((!handle))
    {
        MP_SUB_DEBUG("mp_subtitle_change_language: Invalid Parameters!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_change_language()\n");
        return -1;
    }

    subtitle = (mp_subtitle*)handle;
    ret = -1;

    if (MP_SUBTITLE_TYPE_SUB == subtitle->avail_sub_files[subtitle->current_sub_file_id].avail_file_type)
    {
        ret = mp_subtitle_vobsub_query_language(subtitle, &subtitle->vob_sub, number, \
                                                current_language_index, description, \
                                                description_len);
    }

    MP_SUB_DEBUG("<<====mp_subtitle_change_language()\n");

    return ret;

}
/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_change_language
//
// Description:
//    Change subtitle language
//
// Arguments:
//    handle - Handle
//    language_index - Language index. Start from 0, not 1.
//
// Return Value:
//      1 - Success
//    -1 - Failed
//
/////////////////////////////////////////////////////////////////////////////
long mp_subtitle_change_language(void *handle, long language_index)
{
    mp_subtitle                        *subtitle = NULL;
    long                                ret = 0;

    MP_SUB_DEBUG("====>>mp_subtitle_change_language()\n");

    if ((!handle))
    {
        MP_SUB_DEBUG("mp_subtitle_change_language: Invalid Parameters!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_change_language()\n");
        return -1;
    }

    subtitle = (mp_subtitle*)handle;
    ret = -1;

    if (MP_SUBTITLE_TYPE_SUB == subtitle->avail_sub_files[subtitle->current_sub_file_id].avail_file_type)
    {
        mp_subtitle_pause(subtitle, TRUE);
        osal_task_sleep(200);
        ret = mp_subtitle_vobsub_change_language(subtitle, &subtitle->vob_sub, language_index);
        mp_subtitle_pause(subtitle, FALSE);
    }

    MP_SUB_DEBUG("<<====mp_subtitle_change_language()\n");

    return ret;
}


/**************************Internal Static  Function ***********************************/
/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_unicode_string_length
//
// Description:
//    Borrowed from osd library.
//
// Arguments:
//    ?
//
// Return Value:
//    0 - Failed
//    other - string length
//
/////////////////////////////////////////////////////////////////////////////
#if(SYS_CPU_ENDIAN==ENDIAN_BIG)
static unsigned long mp_subtitle_unicode_string_length(const unsigned short *string)
{
    unsigned long i=0;

    if(NULL == string)
    {
        return 0;
    }

    while (string[i])
    {
        i++;
    }
    return i;
}
#endif
/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_unicode_string_to_mb
//
// Description:
//    Borrowed from osd library.
//
// Arguments:
//    ?
//
// Return Value:
//    0 - Failed
//    other - Success
//
/////////////////////////////////////////////////////////////////////////////
static unsigned long mp_subtitle_unicode_string_to_mb(unsigned short *pw_str)
 {
     if(NULL == pw_str)
     {
        return 0;
     }

 #if(SYS_CPU_ENDIAN==ENDIAN_BIG)
    return mp_subtitle_unicode_string_length(pw_str);
#else
    unsigned long i=0;

    while(pw_str[i])
    {
        pw_str[i]=(unsigned short)(((pw_str[i]&0x00ff)<<8) | ((pw_str[i]&0xff00)>>8));
        i++;
    }
    return i;
#endif
 }

/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_utf8_to_unicode
//
// Description:
//    Borrowed from osd library.
//
// Arguments:
//    ?
//
// Return Value:
//    0 - Success
//    -1 - Failed
//
/////////////////////////////////////////////////////////////////////////////
static long mp_subtitle_utf8_to_unicode(const char *src, long *src_len, unsigned short *dst)
{
    long src_limit = *src_len;
    long src_count = 0;
    long utf8_len = 0;
    unsigned short    unicode = 0;
    unsigned short    *p_unicode = &unicode;
    unsigned char    *p_utf8 = (unsigned char *)src + src_count;
    long     err_flag = 0;

    while (src_count < src_limit)
    {
        utf8_len = utf8_char_len(src[src_count]);
        if ((src_count + utf8_len) > src_limit)
        {
            break;
        }
        utf8_to_u_hostendian(p_utf8, p_unicode, err_flag);
        if (1 == err_flag)
        {
            return -1;
        }
#if(SYS_CPU_ENDIAN==ENDIAN_LITTLE)
        *dst++ = ((unicode&0x00ff)<<8) | ((unicode&0xff00)>>8);
#else
        *dst++ = unicode;
#endif
        src_count += p_utf8 - ((unsigned char *)(src + src_count));
    }

    *src_len = src_count;

    return 0;
}

/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_utf8_string_to_unicode
//
// Description:
//    Borrowed from osd library.
//
// Arguments:
//    ?
//
// Return Value:
//    0 - Success
//    -1 - Failed
//
/////////////////////////////////////////////////////////////////////////////
static long mp_subtitle_utf8_string_to_unicode(unsigned char *utf8,unsigned short *uni_str)
{
    long result = 0;
    unsigned long utf8len = 0;

    utf8len = strlen((const char *)utf8) + 1;

    result = mp_subtitle_utf8_to_unicode((const char *)utf8, (long *)&utf8len,uni_str);

    return result;
}

/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_output_sub1
//
// Description:
//    Output a txt subtitle
//
// Arguments:
//    subtitle - Handle
//
// Return Value:
//     1 - Sucess
//    -1 - Failed
//
/////////////////////////////////////////////////////////////////////////////
static long mp_subtitle_output_sub1(mp_subtitle *subtitle)
{
    long                                    index = 0;
    unsigned long                            offset = 0;
    char                                    *string = NULL;
    unsigned short                        uni_string[1024] = {0};
    mp_subtitle_draw_string_parameters            para;
    unsigned long utf8len = 0;

    MEMSET(uni_string, 0, 1024);
    MP_SUB_DEBUG("====>>mp_subtitle_output_sub1()\n");

    if (!subtitle)
    {
        MP_SUB_DEBUG("mp_subtitle_output_sub: Invalid Parameters!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_output_sub1()\n");
        return -1;
    }

    subtitle->subtitle_osd_functions.osd_control(subtitle->subtitle_osd, MP_SUBTITLE_CONTROL_CLEAN_SCREEN, 0);

    index = subtitle->current_pos;
    if ((index<0) || (index>subtitle->total_sub_count-1))
    {
        index=0;
    }

    subtitle->current_pos = index;

    offset = subtitle->position_offset[index];
    if (offset)
    {
        if (MP_SUBTITLE_TXT_TYPE_L_UNICODE == subtitle->txt_file.txt_type)
        {
            string= subtitle->file_buffer+offset*2;
            para.subtitle_osd = subtitle->subtitle_osd;
            para.x = 14;
            para.y = 20;
            para.fg_color = 7;
            para.string = (unsigned short *)string;
            subtitle->subtitle_osd_functions.osd_control(subtitle->subtitle_osd, \
                                                         MP_SUBTITLE_DRAW_STRING, (long)(&para));
        }
        else if ((MP_SUBTITLE_TXT_TYPE_UTF8 == subtitle->txt_file.txt_type) \
                  || (MP_SUBTITLE_TXT_TYPE_ANSI == subtitle->txt_file.txt_type))
        {
            string= subtitle->file_buffer+offset;
            if(MP_SUBTITLE_TXT_TYPE_UTF8 == subtitle->txt_file.txt_type)//cmx0801
            {
                mp_subtitle_utf8_string_to_unicode((unsigned char *)string, uni_string);
            }
            else
            {
                    utf8len = strlen(string) + 1;
                    mp_subtitle_callback((UINT8 *)string, utf8len, uni_string, 100, mp_lang_type);
             }
            mp_subtitle_unicode_string_to_mb(uni_string);
            para.subtitle_osd = subtitle->subtitle_osd;
            para.x = 14;
            para.y = 20;
            para.fg_color = 7;
            para.string = (unsigned short *)(&uni_string[0]);
            subtitle->subtitle_osd_functions.osd_control(subtitle->subtitle_osd, MP_SUBTITLE_DRAW_STRING, (long)(&para));
        }
    }

    MP_SUB_DEBUG("<<====mp_subtitle_output_sub1()\n");
    return 1;
}



/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_output_image1
//
// Description:
//    Output a image subtitle
//
// Arguments:
//    subtitle - Handle
//    file - File handle
//    file_status - File status
//    buffer - Cache buffer
//    mpeg - Pointer to ps infomation struct
//    spu -  Pointer to spu infomation struct
//    delay - How many ms this subtitle last
//
// Return Value:
//     1 - Sucess
//    -1 - Failed
//
/////////////////////////////////////////////////////////////////////////////
static long mp_subtitle_output_image1(mp_subtitle *subtitle, FILE *file, struct stat *file_status, \
                                              unsigned char *buffer, mp_vob_sub_mpeg *mpeg, mp_vob_sub_spu *spu, \
                                              unsigned long *delay)
{
    long                                    ret = 0;
    long                                    index = 0;
    unsigned long                            offset = 0;
    unsigned long                            offset2 = 0;
    unsigned long                            time_delay = 0;
    unsigned long                            pts_delay = 0;
    long                                    seek_address = 0;
    long                                    aid = 0;
    mp_subtitle_draw_image_parameters            para;
    long                                    max_delay_thd = 5000;
    long                                    suc_proc_packet = 2;

    MP_SUB_DEBUG("====>>mp_subtitle_output_image1()\n");

    if ((!subtitle) || (!file) || (!file_status) || (!buffer) || (!mpeg) || (!spu))
    {
        MP_SUB_DEBUG("mp_subtitle_output_image1: Invalid Parameters!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_output_image1()\n");
        return -1;
    }

    subtitle->subtitle_osd_functions.osd_control(subtitle->subtitle_osd, MP_SUBTITLE_CONTROL_CLEAN_SCREEN, 0);

    index = subtitle->current_pos;
    offset = subtitle->position_offset[index];
    if (index == subtitle->total_sub_count-2)
    {
        offset2 = file_status->st_size;
        time_delay = 0xffffffff;
    }
    else
    {
        offset2 = subtitle->position_offset[index+1];
        if (subtitle->position_time[index+1] > subtitle->position_time[index])
        {
            time_delay = subtitle->position_time[index+1] - subtitle->position_time[index];
        }
        else
        {
            time_delay = subtitle->position_time[index] - subtitle->position_time[index+1];
        }
    }

    ret = fseek(file, offset, SEEK_SET);
    if (ret < 0)
    {
        MP_SUB_DEBUG("mp_subtitle_output_image1: Seek file failed!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_output_image1()\n");
        return -1;
    }

    ret = fread(buffer, 2148, 1, file);
    if (0 == ret)
    {
        MP_SUB_DEBUG("mp_subtitle_output_image1: Seek file failed!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_output_image1()\n");
        return -1;
    }

    ret = mp_subtitle_vobsub_parse_ps_packet(buffer, ret, mpeg);
    seek_address = ret;
    if (ret < 0)
    {
        MP_SUB_DEBUG("<<====mp_subtitle_output_image1()\n");
        return -1;
    }
    aid = mpeg->aid;

    for (;(offset+seek_address)<offset2;)
    {
        ret = mp_subtitle_vobsub_spu_reassemble(spu, mpeg);
        if (ret < 0)
        {
            MP_SUB_DEBUG("<<====mp_subtitle_output_image1()\n");
            return -1;
        }

        if (suc_proc_packet == ret)
        {
            break;
        }

        ret = fseek(file, offset+seek_address, SEEK_SET);
        if (ret < 0)
        {
            MP_SUB_DEBUG("mp_subtitle_output_image1: Seek file failed!\n");
            MP_SUB_DEBUG("<<====mp_subtitle_output_image1()\n");
            return -1;
        }

        ret = fread(buffer, 2148, 1, file);
        if (0 == ret)
        {
            MP_SUB_DEBUG("mp_subtitle_output_image1: Seek file failed!\n");
            MP_SUB_DEBUG("<<====mp_subtitle_output_image1()\n");
            return -1;
        }

        ret = mp_subtitle_vobsub_parse_ps_packet(buffer, ret, mpeg);
        seek_address += ret;
        if (ret < 0)
        {
            MP_SUB_DEBUG("<<====mp_subtitle_output_image1()\n");
            return -1;
        }

        if (aid != mpeg->aid)
        {
            mpeg->packet_size = 0;
        }
    }

    ret = mp_subtitle_vobsub_spu_parse_pxd(&subtitle->vob_sub, spu);
    if (ret < 0)
    {
        MP_SUB_DEBUG("<<====mp_subtitle_output_image1()\n");
        return -1;
    }

    para.subtitle_osd = subtitle->subtitle_osd;
    para.x = spu->start_col;
    para.y = spu->start_row;
    para.width = spu->stride;
    para.height = spu->height;
    para.image = spu->image;
    subtitle->subtitle_osd_functions.osd_control(subtitle->subtitle_osd, MP_SUBTITLE_DRAW_IMAGE, (long)(&para));

    if (spu->end_pts >= spu->start_pts)
    {
        pts_delay = spu->end_pts - spu->start_pts;
    }
    else
    {
        pts_delay = spu->start_pts - spu->end_pts;
    }

    pts_delay /= 9;
    if (pts_delay>(UINT32)max_delay_thd)
    {
        pts_delay = max_delay_thd;
    }

    if (pts_delay > time_delay)
    {
        pts_delay = time_delay;
    }

    *delay = pts_delay;

    MP_SUB_DEBUG("<<====mp_subtitle_output_image1()\n");
    return 1;
}


/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_txt_task
//
// Description:
//    The task for output txt subtitle. It will auto sync with current playing video
//
// Arguments:
//    param1 - Subtitle Handle
//    param2 - Not in use
//
// Return Value:
//    None
//
/////////////////////////////////////////////////////////////////////////////
static void mp_subtitle_txt_task(unsigned long param1,unsigned long param2)
{
    long                                time = 0;
    long                                sub_time = 0;
    mp_subtitle                        *subtitle = NULL;
    long                                ret = 0;
    int                              last_pos = -1;

    MP_SUB_DEBUG("====>>mp_subtitle_task()\n");

    subtitle = (mp_subtitle*)param1;
    sub_time = 0;

    ret = subtitle->subtitle_osd_functions.osd_create(&subtitle->subtitle_osd);
    if (ret < 0)
    {
        osal_flag_set(subtitle->sub_task_control.parameter.flag_id, MP_SUBTITLE_FLAG_UNSUCCESSFUL);
        MP_SUB_DEBUG("<<====mp_subtitle_task()\n");
        return;
    }
    else
    {
        osal_flag_set(subtitle->sub_task_control.parameter.flag_id, MP_SUBTITLE_FLAG_SUCCESS);
    }

    while(subtitle->sub_task_control.command != MP_SUBTITLE_TASK_COMMAND_STOP)
    {

        if (MP_SUBTITLE_TASK_COMMAND_NONE == subtitle->sub_task_control.command)
        {
            time = subtitle->get_time();
            if (subtitle->time_in_ms)
            {
                time /= 100;
            }

            if (0 == sub_time)
            {
                if (subtitle->time_in_ms)
                {
                    sub_time = mp_subtitle_get_sub(subtitle, time*100);
                    sub_time /= 100;
                }
                else
                {
                    sub_time = mp_subtitle_get_sub(subtitle, time*1000);
                    sub_time /= 1000;
                }
            }

            if (sub_time < 0)
            {
                osal_task_sleep(10);
                continue;
            }


            if (time == sub_time)
            {
                if(subtitle->current_pos == last_pos)
                {
                    sub_time = 0;
                    continue;
                }

                last_pos = subtitle->current_pos;
               // MP_SUB_DEBUG("sub_time=%d last_sub_time=%d\n",sub_time,last_sub_time);
//                if(sub_time == last_sub_time)
//                {
//                    sub_time = 0;
//                    continue;
//                }
                sub_time = 0;
                mp_subtitle_output_sub1(subtitle);
            }
            else if (time>sub_time)
            {
                sub_time =0;
            }
            else if (((sub_time -time)-3) > 0)
            {
                sub_time =0;
            }

            ret = 1;
        }
        else if (MP_SUBTITLE_TASK_COMMAND_PAUSE == subtitle->sub_task_control.command)
        {
            if (ret)
            {
                subtitle->subtitle_osd_functions.osd_control(subtitle->subtitle_osd, MP_SUBTITLE_CONTROL_CLEAN_SCREEN, 0);
                ret = 0;
            }
        }
        osal_task_sleep(10);
    };

    subtitle->subtitle_osd_functions.osd_destroy(subtitle->subtitle_osd);

    osal_flag_set(subtitle->sub_task_control.parameter.flag_id, MP_SUBTITLE_FLAG_TASK_EXIT);

    MP_SUB_DEBUG("<<====mp_subtitle_task()\n");
}

static void mp_subtitle_image_proc(mp_subtitle *subtitle, FILE *file, struct stat *file_status, \
                                          char *file_path, mp_vob_sub_mpeg *mpeg, mp_vob_sub_spu *spu)
{
     long                                time = 0;
    long                                sub_time = 0;
    unsigned long                        delay_time = 0;
    unsigned long                        start_time = 0;
    unsigned long                        elapse_time = 0;
    long                                ret = 0;

    sub_time = 0;
    delay_time = 0;
    start_time = 0;
    elapse_time = 0;
    while(subtitle->sub_task_control.command != MP_SUBTITLE_TASK_COMMAND_STOP)
    {
        if (MP_SUBTITLE_TASK_COMMAND_NONE == subtitle->sub_task_control.command)
        {
            time = subtitle->get_time();
            if (subtitle->time_in_ms)
            {
                time /= 100;
            }

            if (time <= 0)
            {
                osal_task_sleep(10);
                continue;
            }

            if (0 == sub_time)
            {
                if (subtitle->time_in_ms)
                {
                    sub_time = mp_subtitle_get_sub(subtitle, time*100);
                    sub_time /= 100;
                }
                else
                {
                    sub_time = mp_subtitle_get_sub(subtitle, time*1000);
                    sub_time /= 1000;
                }
            }

            if (sub_time < 0)
            {
                osal_task_sleep(10);
                continue;
            }

            if (time == sub_time)
            {
                sub_time = 0;
                delay_time = 0;
                mp_subtitle_output_image1(subtitle, file, file_status, (unsigned char *)file_path, mpeg, spu, &delay_time);
                start_time = osal_get_tick();

            }
            else if (time>sub_time)
            {
                sub_time =0;
            }
            else if (((sub_time -time) -3) > 0)
            {
                sub_time =0;
            }

            if (delay_time)
            {
                elapse_time = osal_get_tick() - start_time;
                if (elapse_time >= delay_time)
                {
                    subtitle->subtitle_osd_functions.osd_control(subtitle->subtitle_osd, MP_SUBTITLE_CONTROL_CLEAN_SCREEN, 0);
                    delay_time = 0;
                }
            }

            ret = 1;
        }
        else if (MP_SUBTITLE_TASK_COMMAND_PAUSE == subtitle->sub_task_control.command)
        {
            if (ret)
            {
                subtitle->subtitle_osd_functions.osd_control(subtitle->subtitle_osd, MP_SUBTITLE_CONTROL_CLEAN_SCREEN, 0);
                ret = 0;
            }

            delay_time = 0;
        }

        osal_task_sleep(10);

    }
}

/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_image_task
//
// Description:
//    The task for output image subtitle. It will auto sync with current playing video
//
// Arguments:
//    param1 - Subtitle Handle
//    param2 - Not in use
//
// Return Value:
//    None
//
/////////////////////////////////////////////////////////////////////////////
static void mp_subtitle_image_task(unsigned long param1,unsigned long param2)
{
    mp_subtitle                        *subtitle = NULL;
    long                                ret = 0;
    long                                    index = 0;
    char                                    *filename = NULL;
    FILE                                    *file = NULL;
    char                                    *file_path  = NULL; //[FULL_PATH_SIZE+100];
    struct stat                            file_status;
    char                                 *ext = NULL;
    mp_vob_sub_mpeg                        mpeg;
    char                                    *spu_fragment = NULL; //[2048];
    mp_vob_sub_spu                            *spu = NULL;

    MP_SUB_DEBUG("====>>mp_subtitle_image_task()\n");

    subtitle = (mp_subtitle*)param1;

    index = subtitle->current_pos;
    if ((index<0) || (index>subtitle->total_sub_count-1))
    {
        index=0;
    }

    subtitle->current_pos = index;

    filename = subtitle->avail_sub_files[subtitle->current_sub_file_id].avail_sub_file;
    file_path = (char*)malloc(FULL_PATH_SIZE+100);
    if(!file_path)
    {
        return;
    }
    spu_fragment = (char *)malloc(2048);
    if (!spu_fragment)
    {
        return;
    }
    MEMSET(file_path, 0, FULL_PATH_SIZE+100);
    filename = subtitle->avail_sub_files[subtitle->current_sub_file_id].avail_sub_file;
    //STRCPY(file_path, filename);
    strncpy(file_path, filename, FULL_PATH_SIZE+100-1);
    ext = strrchr(file_path, (long)'.');
    if (!ext)
    {
        free(file_path);
        file_path = NULL;
        MP_SUB_DEBUG("mp_subtitle_image_task: Wrong Video File Name!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_image_task()\n");
        osal_flag_set(subtitle->sub_task_control.parameter.flag_id, MP_SUBTITLE_FLAG_UNSUCCESSFUL);
        return;
    }

    char str_suffix[] = ".SUB";

    strncpy(ext, str_suffix, strlen(str_suffix)+1);

    ret = fs_stat(file_path, &file_status);
    if (ret < 0)
    {
        strlwr(ext);
        ret = fs_stat(file_path, &file_status);
    }

    if (ret < 0)
    {
        if (NULL != file_path)
        {
            free(file_path);
            file_path = NULL;
        }
        MP_SUB_DEBUG("mp_subtitle_image_task: Sub File is not existed!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_image_task()\n");
        osal_flag_set(subtitle->sub_task_control.parameter.flag_id, MP_SUBTITLE_FLAG_UNSUCCESSFUL);
        return;
    }

    file = fopen(file_path, "rb");
    if (!file)
    {
        if (NULL != file_path)
        {
            free(file_path);
            file_path = NULL;
        }
        MP_SUB_DEBUG("mp_subtitle_image_task: Open file failed!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_image_task()\n");
        osal_flag_set(subtitle->sub_task_control.parameter.flag_id, MP_SUBTITLE_FLAG_UNSUCCESSFUL);
        return;
    }

    spu = (mp_vob_sub_spu*)malloc(sizeof(mp_vob_sub_spu));
    if (!spu)
    {
        fclose(file);
        if (NULL != file_path)
        {
            free(file_path);
            file_path = NULL;
        }
        MP_SUB_DEBUG("mp_subtitle_image_task: Malloc spu failed!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_image_task()\n");
        osal_flag_set(subtitle->sub_task_control.parameter.flag_id, MP_SUBTITLE_FLAG_UNSUCCESSFUL);
        return;
    }

    MEMSET(spu, 0, sizeof(mp_vob_sub_spu));

    FREE(subtitle->file_buffer);
    subtitle->file_buffer = NULL;
    subtitle->file_buffer_size_limit = 0;

    mpeg.packet_reserve = 2048;
    mpeg.packet = (unsigned char *)spu_fragment;

    ret = subtitle->subtitle_osd_functions.osd_create(&subtitle->subtitle_osd);
    if (ret < 0)
    {
        fclose(file);
        free(spu);
        if (NULL != file_path)
        {
            free(file_path);
            file_path = NULL;
        }
        MP_SUB_DEBUG("mp_subtitle_image_task: Create osd failed!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_image_task()\n");
        osal_flag_set(subtitle->sub_task_control.parameter.flag_id, MP_SUBTITLE_FLAG_UNSUCCESSFUL);
        return;
    }

    subtitle->subtitle_osd_functions.osd_control(subtitle->subtitle_osd, MP_SUBTITLE_CHANGE_PALLETE, \
                                                 (long)subtitle->vob_sub.palette);

    osal_flag_set(subtitle->sub_task_control.parameter.flag_id, MP_SUBTITLE_FLAG_SUCCESS);

    mp_subtitle_image_proc(subtitle, file, &file_status, file_path, &mpeg, spu);

    if (spu->image)
    {
        FREE(spu->image);
        spu->image = NULL;
    }

    FREE(spu);
    fclose(file);
    if (NULL != file_path)
    {
        free(file_path);
        file_path = NULL;
    }

    subtitle->subtitle_osd_functions.osd_destroy(subtitle->subtitle_osd);

    osal_flag_set(subtitle->sub_task_control.parameter.flag_id, MP_SUBTITLE_FLAG_TASK_EXIT);

    MP_SUB_DEBUG("<<====mp_subtitle_task()\n");
}
/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_start
//
// Description:
//    Start a subtitle task
//
// Arguments:
//    subtitle - Subtitle Handle
//
// Return Value:
//    1 - Success
//    -1 - Failed
//
/////////////////////////////////////////////////////////////////////////////
static long mp_subtitle_start(mp_subtitle *subtitle)
{
    OSAL_T_CTSK                                        task_attribute;
    unsigned long                                            flag = 0;

    MP_SUB_DEBUG("====>>mp_subtitle_start()\n");
    MEMSET(&task_attribute, 0, sizeof(OSAL_T_CTSK));
    if ((!subtitle))
    {
        MP_SUB_DEBUG("mp_subtitle_start: Invalid Parameters!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_start()\n");
        return -1;
    }

    if (MP_SUBTITLE_TASK_RUNNING == subtitle->sub_task_control.status)
    {
        MP_SUB_DEBUG("mp_subtitle_start: Task already running!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_start()\n");
        return -1;
    }

    subtitle->sub_task_control.command = MP_SUBTITLE_TASK_COMMAND_NONE;

    subtitle->sub_task_control.parameter.flag_id = OSAL_INVALID_ID;
    subtitle->sub_task_control.parameter.task_id = OSAL_INVALID_ID;

    //code to create flag
    flag = MP_SUBTITLE_FLAG_INITIALIZE;
    subtitle->sub_task_control.parameter.flag_id = osal_flag_create(flag);
    if ( OSAL_INVALID_ID == subtitle->sub_task_control.parameter.flag_id)
    {
        MP_SUB_DEBUG("mp_subtitle_start osal_flag_create failure! \n");
        MP_SUB_DEBUG("<<====mp_subtitle_start()\n");
        return -1;
    }

    //start thread
    task_attribute.stksz    = MP_SUBTITLE_PLAY_TASK_STACKSIZE;
    task_attribute.quantum    = MP_SUBTITLE_PLAY_TASK_QUANTUM;
    task_attribute.itskpri    = OSAL_PRI_NORMAL;
    task_attribute.para1    = (unsigned long)subtitle;
    task_attribute.name[0]    = 'S';
    task_attribute.name[1]    = 'u';
    task_attribute.name[2]    = 'b';
    if (MP_SUBTITLE_TYPE_SUB == subtitle->avail_sub_files[subtitle->current_sub_file_id].avail_file_type)
    {
        task_attribute.task = (FP)mp_subtitle_image_task;
    }
    else
    {
        task_attribute.task = (FP)mp_subtitle_txt_task;
    }
    subtitle->sub_task_control.parameter.task_id = osal_task_create(&task_attribute);
    if(OSAL_INVALID_ID == subtitle->sub_task_control.parameter.task_id)
    {
        osal_flag_delete(subtitle->sub_task_control.parameter.flag_id);
        subtitle->sub_task_control.parameter.flag_id = OSAL_INVALID_ID;
        MP_SUB_DEBUG("mp_subtitle_start osal_task_create failed! \n");
        MP_SUB_DEBUG("<<====mp_subtitle_start()\n");
        return -1;
    }

    //code to sync
    osal_flag_wait(&flag, subtitle->sub_task_control.parameter.flag_id, \
                   MP_SUBTITLE_FLAG_MASK, OSAL_TWF_ORW | OSAL_TWF_CLR, OSAL_WAIT_FOREVER_TIME);
    if (flag != MP_SUBTITLE_FLAG_SUCCESS)
    {
        osal_flag_delete(subtitle->sub_task_control.parameter.flag_id);
        subtitle->sub_task_control.parameter.flag_id = OSAL_INVALID_ID;
        MP_SUB_DEBUG("mp_subtitle_start osal_task_create failed! \n");
        MP_SUB_DEBUG("<<====mp_subtitle_start\n");
        return -1;
    }

    subtitle->sub_task_control.status = MP_SUBTITLE_TASK_RUNNING;

    MP_SUB_DEBUG("<<====mp_subtitle_start\n");

    return 1;

}

static long mp_subtitle_io_ctrl(void *handle, long io_cmd, long param1, long param2)
{
    mp_subtitle                            *subtitle = NULL;
    FILE                                *file = NULL;
    char                                *filename = NULL;
    char                                *ext = NULL;
    long                                type = 0;
    long                                ret = 0;
    char                                str_suffix[] = ".sub";

    MP_SUB_DEBUG("====>>mp_subtitle_io_ctrl()\n");

    if (!handle)
    {
        MP_SUB_DEBUG("mp_subtitle_parse: Invalid Parameters!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_io_ctrl()\n");
        return -1;
    }

    subtitle = (mp_subtitle*)handle;

    if ((subtitle->current_sub_file_id < 0) || (subtitle->avail_file_count > MP_SUBTITLE_TYPE_NUMBER))
    {
        MP_SUB_DEBUG("mp_subtitle_io_ctrl: Invalid Parameters!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_io_ctrl()\n");
        return -1;
    }

    type = subtitle->avail_sub_files[subtitle->current_sub_file_id].avail_file_type;

     switch(io_cmd)
        {
            case GET_SUBTITLE_FILE_SIZE:
                 filename = subtitle->avail_sub_files[subtitle->current_sub_file_id].avail_sub_file;
                if( MP_SUBTITLE_TYPE_SUB == type)
                {
                    ext = strrchr(filename, (long)'.');
                    if (!ext)
                    {
                        return -1;
                    }

                    strncpy(ext, str_suffix, 5);
                    file = fopen(filename, "rb");
                }
                else
                {
                    file = fopen(filename, "rb");
                }

                MP_SUB_DEBUG("the subtitle file name is: %s\n", filename);
                if (!file)
                {
                    MP_SUB_DEBUG("mp_subtitle_io_ctrl: Open file failed!\n");
                    MP_SUB_DEBUG("<<====mp_subtitle_io_ctrl()\n");
                    return -1;
                }

                fseek(file, 0, SEEK_END);
                ret = ftell(file);
                fseek(file, 0, SEEK_SET);
                fclose(file);
                break;
        case GET_SUBTITLE_FILE_TIME:
            if( MP_SUBTITLE_TYPE_SUB == type)
            {
                ret =subtitle->position_time[subtitle->total_sub_count-1] ;
            }
            else
            {
                ret =subtitle->position_time[subtitle->total_sub_count-2] ;
            }
            MP_SUB_DEBUG("the subtitle count is: %d\n", subtitle->total_sub_count);
            break;
        case SET_SUBTITLE_STRING_TO_UNICODE:
            mp_lang_type = param1;
            mp_subtitle_callback = (mp_subtitle_string_to_unicode) param2;
            break;
        default:
            break;
     }

    return ret;
}

#if (SYS_CHIP_MODULE == ALI_S3602 || SYS_CHIP_MODULE == ALI_M3329E)
const EXT_SUB_PLUGIN mp_subtitle_plugin __attribute__ ((section(".extsubt.plugin"))) =
{
    mp_subtitle_create,
    mp_subtitle_delete,
    mp_find_ext_subtitle,
    mp_subtitle_auto_load,
    mp_subtitle_unload,
    mp_subtitle_hide_sub,
    mp_subtitle_query_language,
    mp_subtitle_change_language,
    mp_subtitle_load,
    mp_subtitle_io_ctrl,
    mp_subtile_get_sub_file_num,
    mp_subtile_get_cur_sub_id,
    mp_subtitle_change_ext_sub_id,
};
#endif
