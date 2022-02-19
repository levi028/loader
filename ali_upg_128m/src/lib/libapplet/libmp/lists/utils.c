  /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: utils.c
*
*    Description: This file describes utis api.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <types.h>
#include <retcode.h>
#include <sys_config.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/sci/sci.h>
#include <hld/hld_dev.h>
#include <osal/osal_timer.h>
#include <api/libmp/media_player_api.h>
#include "play_list.h"
#include "utils.h"

static int strcmp_byex(const char *s1, const char *s2);

static const struct file_type_map  _all_file_types[] = {
    ////////////////MUSIC type //////////////////////////////////////
    {F_MP3, "MP3"},
    {F_MP3, "MP2"},
    {F_MP3, "MP1"},
    {F_OGG, "OGG"},
#ifndef _S3281_        
    {F_MP3, "AAC"},
    {F_MP3, "AC3"},
#endif        
#ifdef DLNA_CTT_TEST
    {F_MP3, "PCM"},
#endif
#ifdef MEDIA_PLAYER_BYE1_SUPPORT
    {F_BYE1, "BYE1"},
#endif
#ifndef _S3281_
    {F_WAV, "WAV"},
    {F_FLC, "FLAC"},
#endif
    ////////////////IMAGE type //////////////////////////////////////
    {F_JPG, "JPG"},
    {F_JPG, "JPEG"},
    {F_BMP, "BMP"},
#ifdef PNG_GIF_TEST
    {F_BMP, "GIF"},
    {F_BMP, "PNG"},
#endif
    ////////////////VIDEO type //////////////////////////////////////
    {F_MPG, "MPG"},
    {F_MPEG, "MPEG"},
    {F_MPEG, "DAT"},
#ifdef SUPPORT_MPEG4_TEST
#ifndef _S3281_
    {F_MPEG, "VOB"},
    {F_MPG, "AVI"},
#endif
    {F_MPG, "TS"},
    {F_MPG, "TRP"},
    {F_MPG, "TP"},
    {F_MPG, "M2T"},
    {F_MPG, "M2TS"},
    {F_MPG, "MTS"},
#ifndef _S3281_
    {F_MPG, "MP4"},
    {F_MPG, "MKV"},
    {F_MPG, "MOV"},
    {F_MPG, "3GP"},
    {F_MPG, "BYE2"},
    {F_MPG, "BYE3"},
    {F_MPG, "FLV"},
    {F_MPG, "RMVB"},
    {F_MPG, "RM"},
    {F_MPG, "RAM"},
#if 1//((defined _S3811_) || (defined _M3503_) ||(defined(_M3821_)||defined(_M3505_)))
    {F_MPG, "WEBM"},
#endif
#endif
#ifdef SUPPORT_ES_PLAYER
    {F_MPG, "ES"},
#endif
#endif
    {F_UNKOWN, ""}
};

#ifndef _USE_32M_MEM_
const struct file_type_map g_no32msupport_file_types[] = {
    {F_MP3, "MP3"},
    {F_JPG, "JPG"},
    {F_BMP, "BMP"},
    {F_OGG, "OGG"},
    {F_MP3, "AAC"},
    {F_MP3, "AC3"},
    {F_WAV, "WAV"},
    {F_FLC, "FLAC"},
    {F_UNKOWN, ""}
};
#endif

static const struct device_type_map _dev_list[] =
{
    {DEV_USB, "ud"},
    {DEV_SD, "sd"},
    {DEV_HD, "hd"},
    {DEV_DLNA, "dl"},
    {DEV_SATA, "sh"},
    {DEV_UNKNOWN, NULL},
};

/////////////////////////////////////////////////////////////////////////////
// file_map_type
//
// Description:
//
// Arguments:
//
// Return Value:
//
/////////////////////////////////////////////////////////////////////////////
FILE_TYPE file_map_type(file_list_type list_type, char *ext_name)
{
    FILE_TYPE file_type = F_UNKOWN;

    UINT32  music_type = F_MP3*F_BYE1*F_OGG*F_WAV*F_FLC*F_PCM;
    UINT32  video_type = F_MPG*F_MPEG;
    UINT32  image_type = F_JPG*F_BMP;

    if(!ext_name)
    {
        libc_printf("ext_name is null\n");
        return F_UNKOWN;
    }
    file_type = get_file_type(ext_name, _all_file_types);

    switch(list_type)
    {
    case MUSIC_FILE_LIST:
    case MUSIC_FAVORITE_LIST:
        if (music_type % file_type != 0)
        {
            file_type = F_UNKOWN;
        }
        break;
    case IMAGE_FILE_LIST:
        if (image_type % file_type != 0)
        {
            file_type = F_UNKOWN;
        }
        break;
    case VIDEO_FILE_LIST:
        if (video_type % file_type != 0)
        {
            file_type = F_UNKOWN;
        }
        break;
    case DLNA_FILE_LIST:
        file_type = F_UNKOWN;
        break;
    default:
        file_type = F_UNKOWN;
        break;
    }

    return file_type;

}

/////////////////////////////////////////////////////////////////////////////
// create_path_by_combination
//
// Description:
//
// Arguments:
//
// Return Value:
//
/////////////////////////////////////////////////////////////////////////////
char *create_path_by_combination(const char *dirpath, const char *name, const char *name2)
{
    char *str = NULL;

    if((!dirpath) || (!name))
    {
        libc_printf("dirpath or name is null\n");
        return NULL;
    }
    str = (char *)MALLOC(FULL_PATH_SIZE);
    if(NULL == str)
    {
        libc_printf("failed to create path by combination\n");
        return NULL;
    }

    (void)path_combination(str, dirpath, name, name2);
    return str;
}

/////////////////////////////////////////////////////////////////////////////
// path_combination
//
// Description:
//
// Arguments:
//
// Return Value:
//
/////////////////////////////////////////////////////////////////////////////
char *path_combination(char *full_path, const char *dirpath, const char *name, const char *name2)
{
    UINT32 s_len = 0;

    if((!dirpath) || (!name))
    {
         libc_printf("dirpath or name is null\n");
        return NULL;
    }
    MEMSET(full_path, 0, FULL_PATH_SIZE);
    strncpy(full_path, dirpath, FULL_PATH_SIZE - 1);
    s_len = STRLEN(full_path);
    strncat(full_path, name, FULL_PATH_SIZE-s_len-1);
    if(name2)
    {
        s_len = STRLEN(full_path);
        strncat(full_path, name2, FULL_PATH_SIZE - 1 - s_len);
    }
    return full_path;
}

/////////////////////////////////////////////////////////////////////////////
// free_combination_path
//
// Description:
//
// Arguments:
//
// Return Value:
//
/////////////////////////////////////////////////////////////////////////////
void free_combination_path(char *str)
{
    if(str)
    {
        FREE(str);
        str = NULL;
    }
}

/////////////////////////////////////////////////////////////////////////////
// get_device_type
//
// Description:
//
// Arguments:
//
// Return Value:
//
/////////////////////////////////////////////////////////////////////////////
e_device_type get_device_type(const char *name, char *index)
{
    int k = 0;
    char i = 0;

    if((!name) || (!index))
    {
        libc_printf("name or index is null\n");
        return DEV_UNKNOWN;
    }
    while(_dev_list[k].dev)
    {
        if (0 == strncmp(name, _dev_list[k].dev, 2))
        {
            if(DEV_DLNA == _dev_list[k].devtype)
            {
                return _dev_list[k].devtype;
            }

            i = *(name + 2);
            if((i < 'a') || (i > 'z'))
            {
                return DEV_UNKNOWN;
            }

            *index = i;
            return _dev_list[k].devtype;
        }
        k++;
    }
    return DEV_UNKNOWN;
}

/////////////////////////////////////////////////////////////////////////////
// get_file_type
//
// Description:
//
// Arguments:
//
// Return Value:
//
/////////////////////////////////////////////////////////////////////////////
FILE_TYPE get_file_type(char *ext, const struct file_type_map *supportlist)
{
    int i = 0;

    if((!ext) || (!supportlist))
    {
        libc_printf("ext or supportlist is null\n");
        return F_UNKOWN;
    }
    while(supportlist[i].filetype != F_UNKOWN)
    {
        if(!strcmp_c(ext, supportlist[i].ext))
        {
            return supportlist[i].filetype;
        }
        if(strcmp_byex(ext, supportlist[i].ext))
        {
            return supportlist[i].filetype;
        }
        i++;
    }
    return F_UNKOWN;
}

/////////////////////////////////////////////////////////////////////////////
// strcmp_c
//
// Description:
//
// Arguments:
//
// Return Value:
//
/////////////////////////////////////////////////////////////////////////////
int strcmp_c(const char *s1, const char *s2)
{
    int i = 0;
    char c1 = 0;
    char c2 = 0;

    for (i = 0, c1 = *s1, c2 = *s2; (c1 != '\0') && (c2 != '\0'); i++)
    {
        c1 = *(s1 + i);
        c2 = *(s2 + i);
        if ((c1 >= 'A') && (c1 <='Z'))
        {
            c1 += 'a' - 'A';
        }
        if ((c2 >= 'A') && (c2 <='Z'))
        {
            c2 += 'a' - 'A';
        }
        if (c1 != c2)
        {
            break;
        }
    }

    return (int)c1 - (int)c2;
}

static int strcmp_byex(const char *s1, const char *s2)
{
    int i = 0;
    char kw1[4]={0x58,0x4e,0x42,'\0'};
    char kw2[4]={0x58,0x4e,0x57,'\0'};
    char kw3[4]={0x42,0x54,0x47,'\0'};
    char c1 = 0;//*s1;

    if((s1== NULL)||(s2 == NULL))
    {
        return 0;
    }
    c1 = *s1;
    for(i=0;i<3;i++)
    {
        kw1[i]--;
        kw2[i]--;
        kw3[i]--;
    }

    if ((c1 >= 'A') && (c1 <='Z'))
    {
        c1 += 'a' - 'A';
    }

    if(c1 == 'w')
    {
        if(!strcmp_c(s2, "BYE1"))
        {
            if(!strcmp_c(s1, kw1))
            {
                return 1;
            }
        }
        else if(!strcmp_c(s2, "BYE2"))
        {
            if(!strcmp_c(s1, kw2))
            {
                return 1;
            }
        }
        else
        {
            return 0;
        }
    }
    else if(c1== 'a')
    {
        if(!strcmp_c(s2, "BYE3"))
        {
            if(!strcmp_c(s1, kw3))
            {
                return 1;
            }
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }

    return 0;
}

/////////////////////////////////////////////////////////////////////////////
// dirs_skipped
//
// Description:
//
// Arguments:
//
// Return Value:
//
/////////////////////////////////////////////////////////////////////////////
BOOL  dirs_skipped(const char *dir)
{
    if(!dir)
    {
        libc_printf("dir is null\n");
        return FALSE;
    }
    if (!STRCMP(dir, "."))
    {
        return TRUE;
    }

    if (!STRCMP(dir, ".."))
    {
        return TRUE;
    }

    if (!strcmp_c(dir, "ALiDvr"))
    {
        return TRUE;
    }

    if (!strcmp_c(dir, "ALIDVRS2"))
    {
        return TRUE;
    }

    if (!strcmp_c(dir, "recycler"))
    {
        return TRUE;
    }

    if (!strcmp_c(dir, "RECYCLE.BIN"))
    {
        return TRUE;
    }

    if (!strcmp_c(dir, "$RECYCLE.BIN"))
    {
        return TRUE;
    }
    
    if (!strcmp_c(dir, "LOST.DIR"))
    {
        return TRUE;
    }

    if (!strcmp_c(dir, "system volume information"))
    {
        return TRUE;
    }
    return FALSE;
}

#ifndef _USE_32M_MEM_
/////////////////////////////////////////////////////////////////////////////
// try_get_key_val
//
// Description:
//
// Arguments:
//
// Return Value:
//
/////////////////////////////////////////////////////////////////////////////
BOOL try_get_key_val(char *string, int string_len, const char *keyword, unsigned char*keyval)
{
    int keylen = STRLEN(keyword);

    if((!string) || (!keyword) || (!keyval))
    {
        libc_printf("string or keywork or keyval is null\n");
        return FALSE;
    }
    if ((string_len>keylen)&&(!strncasecmp(keyword, string, keylen)))
    {
        string += keylen;
        string_len -= keylen;
        while((*string<'0')||(*string>'9'))
        {
            string++;
            string_len--;
            if (string_len<=0)
            {
                return FALSE;
            }
        }
        *keyval = (unsigned char)atoi(string);;
        return TRUE;
    }
    return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// try_get_key_str
//
// Description:
//
// Arguments:
//
// Return Value:
//
/////////////////////////////////////////////////////////////////////////////
BOOL try_get_key_str(char *string, int string_len, const char *keyword, int *offset)
{
    int keylen = STRLEN(keyword);
    int off = 0;

    if((!string)||(!keyword) ||(!offset))
    {
        libc_printf("string or keywork or offset is null\n");
        return FALSE;
    }
    if ((string_len>keylen)&&(!strncasecmp(keyword, string, keylen)))
    {
        string += keylen;
        string_len -= keylen;
        off = keylen;
        while(((*string<'A')||(*string>'Z'))&&((*string<'a')||(*string>'z')))
        {
            string++;
            string_len--;
            off ++;
            if (string_len<=0)
            {
                return FALSE;
            }
        }
        *offset = off;
        return TRUE;
    }
    return FALSE;
}

static void _write_txt_line(char *dest_buffer,  int len)
{
    if(!dest_buffer)
    {
        libc_printf("dest_buffer is null\n");
        return;
    }
    dest_buffer[len]='\r';
    dest_buffer[len+1]='\n';
}

/////////////////////////////////////////////////////////////////////////////
// write_val_record
//
// Description:
//
// Arguments:
//
// Return Value:
//
/////////////////////////////////////////////////////////////////////////////
int write_val_record(char *pbuf, const char *keyword, unsigned long keyval)
{
    int offset = 0;
    int keylen = STRLEN(keyword);

    if((!pbuf) || (!keyword))
    {
        libc_printf("pbuf or keyword is null\n");
        return -1;
    }
    MEMCPY(pbuf, keyword, keylen);
    offset = keylen;
    offset += 1 + ITOA(pbuf + offset, keyval);
    _write_txt_line(pbuf, offset);
    offset += 2;
    return offset;
}

/////////////////////////////////////////////////////////////////////////////
// write_str_record
//
// Description:
//
// Arguments:
//
// Return Value:
//
/////////////////////////////////////////////////////////////////////////////
int write_str_record(char *pbuf, const char *keyword, const char *str, int len)
{
    int offset = 0;
    int keylen = STRLEN(keyword);

    if((!pbuf) || (!keyword))
    {
        libc_printf("pbuf or keyword is null\n");
        return -1;
    }
    MEMCPY(pbuf, keyword, keylen);
    offset = keylen;
    if(str)
    {
        MEMCPY(pbuf + offset, str, len);
        offset += len;
    }
    _write_txt_line(pbuf, offset);
    offset += 2;
    return offset;
}

/////////////////////////////////////////////////////////////////////////////
// write_txt_line
//
// Description:
//
// Arguments:
//
// Return Value:
//
/////////////////////////////////////////////////////////////////////////////
void write_txt_line(char *dest_buffer,  int len)
{
    if(!dest_buffer)
    {
        libc_printf("dest_buffer is null\n");
        return ;
    }
    dest_buffer[len]='\r';
    dest_buffer[len+1]='\n';
}

#endif

/////////////////////////////////////////////////////////////////////////////
// print_playlist
//
// Description:
//
// Arguments:
//
// Return Value:
//
/////////////////////////////////////////////////////////////////////////////
/*
void print_playlist(const pPlayList playlist)
{
    struct list_head                            *ptr = NULL;
    pPlayListItem                                item = NULL;

    libc_printf("current playlist type: %d\n", playlist->type);
    libc_printf("current playlist state: %d\n", playlist->state);
    libc_printf("current playlist name: %s\n", playlist->info.name);
    libc_printf("current playlist playidx: %d\n", playlist->info.playidx);
    libc_printf("current playlist playednum: %d\n", playlist->info.playednum);
    libc_printf("current playlist looptype: %d\n", playlist->info.looptype);
    libc_printf("current playlist loopnum: %d\n", playlist->info.loopnum);
    libc_printf("there are %d files in list_playlist\n", playlist->info.filenum);
    libc_printf("current playlist num_limit_enable: %d\n", playlist->info.num_limit_enable);
    libc_printf("current playlist add_once: %d\n", playlist->info.add_once);
    libc_printf("current playlist limit_num: %d\n", playlist->info.limit_num);
    libc_printf("current playlist sorttype: %d\n", playlist->info.sorttype);
    libc_printf("current playlist sortorder: %d\n", playlist->info.sortorder);


    list_for_each(ptr, &playlist->head)
    {
        item = list_entry(ptr, PlayListItem, listpointer);
        if(item != NULL)
        {
            libc_printf("PL %d\t %s\n", i++, item->name);
        }
    }

    return;
}*/
/////////////////////////////////////////////////////////////////////////////
// check_file_type
//
// Description:
//
// Arguments:
//
// Return Value:
//
/////////////////////////////////////////////////////////////////////////////
int check_file_type(FILE_TYPE type, play_list_type play_list_type)
{
    int                result = 0;

    if (!type)
    {
        return 0;
    }

    switch (play_list_type)
    {
        case MUSIC_PLAY_LIST:
        {
            result = F_MP3 * F_OGG * F_BYE1*F_FLC*F_WAV;
            break;
        }

        case IMAGE_PLAY_LIST:
        {
            result = F_JPG * F_BMP;
            break;
        }

        case VIDEO_PLAY_LIST:
        {
            result = F_MPEG * F_MPG;
            break;
        }

        default:
        {
            result = 0;
            break;
        }
    }

    result = result % type;

    return (!result);

}

