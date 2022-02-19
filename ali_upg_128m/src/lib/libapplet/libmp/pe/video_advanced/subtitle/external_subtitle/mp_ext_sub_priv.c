/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2007 Copyright (C)
 *
 *  File: mp_subtitle.c
 *
 *  Description: This file describes subtitle file operations.
 *
 *  History:
 *      Date            Author          Version          Comment
 *      ====            ======      =======      =======
 *  1.  2009.5.29  Martin_Xia      0.0.1
 *
 ****************************************************************************/

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

static void mp_subtitle_fix_utf8(char *str, unsigned long *pos);
static void mp_subtitle_fix_unicode(UINT16 *str,unsigned long *pos);
static void mp_subtitle_sort(mp_subtitle *subtitle,char *str,unsigned long *pos,unsigned long *time);


void mp_subtitle_get_time(unsigned long *time, char **xtime1, unsigned long multiple, char const_c)
{
    long tt = 0;

    if(!time || !xtime1 || !(*xtime1))
    {
        return;
    }

    tt=0;
    while (*(*xtime1)!=const_c)
    {
        tt*=10;
        tt+=(*(*xtime1)-0x30);
        (*xtime1)++;
    }
    (*xtime1)++;
	*time+=tt;
    *time*=multiple;
}

void mp_subtitle_unicode_get_time(unsigned long *time, UINT16 **xtime1, unsigned long multiple, UINT16 const_c)
{
    long tt = 0;

    if(!time || !xtime1 || !(*xtime1))
    {
        return;
    }

    tt=0;
    while (*(*xtime1)!=const_c)
    {
        tt*=10;
        tt+=(*(*xtime1)-0x30);
        (*xtime1)++;
    }
    (*xtime1)++;
	*time+=tt;
    *time*=multiple;
}

void mp_subtitle_get_timenum(unsigned long *time, char **xtime1, unsigned long multiple, unsigned long utf8)
{
    long tt = 0;

    if(!time || !xtime1 || !(*xtime1))
    {
        return;
    }

    tt=0;
    if(utf8)
    {
        while ((*(*xtime1)>='0') && (*(*xtime1)<='9'))
        {
            tt*=10;
            tt+=(*(*xtime1)-0x30);
            (*xtime1)++;
        }
    }
    else
    {
        while ((*(*xtime1)>=L'0') && (*(*xtime1)<=L'9'))
        {
            tt*=10;
            tt+=(*(*xtime1)-0x30);
            (*xtime1)++;
        }
    }
    (*xtime1)++;
	*time+=tt;
    *time*=multiple;
}

void mp_subtitle_unicode_get_timenum(unsigned long *time, UINT16 **xtime1, unsigned long multiple, unsigned long utf8)
{
    long tt = 0;

    if(!time || !xtime1 || !(*xtime1))
    {
        return;
    }

    tt=0;
    if(utf8)
    {
        while ((*(*xtime1)>='0') && (*(*xtime1)<='9'))
        {
            tt*=10;
            tt+=(*(*xtime1)-0x30);
            (*xtime1)++;
        }
    }
    else
    {
        while ((*(*xtime1)>=L'0') && (*(*xtime1)<=L'9'))
        {
            tt*=10;
            tt+=(*(*xtime1)-0x30);
            (*xtime1)++;
        }
    }
    (*xtime1)++;
	*time+=tt;
    *time*=multiple;
}

/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_vobsub_spu_next_line
//
// Description:
//    Get next line of pixel data in a spu packet
//
// Arguments:
//    spu - Pointer to a struct contains Spu infomation
//
// Return Value:
//    None
//
/////////////////////////////////////////////////////////////////////////////
void mp_subtitle_vobsub_spu_next_line(mp_vob_sub_spu *spu)
{
    long mod = 2;

    MP_SUB_DEBUG("====>>mp_subtitle_vobsub_spu_next_line()\n");

    if ((!spu))
    {
        MP_SUB_DEBUG("mp_subtitle_vobsub_spu_next_line: Invalid Parameters!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_vobsub_spu_next_line()\n");
        return;
    }

    if (spu->current_nibble[spu->deinterlace_oddness] % mod)
    {
        spu->current_nibble[spu->deinterlace_oddness]++;
    }
    spu->deinterlace_oddness = (spu->deinterlace_oddness + 1) % 2;

    MP_SUB_DEBUG("<<====mp_subtitle_vobsub_spu_next_line()\n");
}

/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_vobsub_parse_forced_subs
//
// Description:
//    Parse forced sub infomation of a idx file
//
// Arguments:
//    vob - Vob handle
//    line - The line which contains forced sub infomation
//
// Return Value:
//    0 - Success
//    -1 - Failed
//
/////////////////////////////////////////////////////////////////////////////
long mp_subtitle_vobsub_parse_forced_subs(mp_vob_sub *vob, const char *line)
{
    const char *p = NULL;

    p  = line;
    while (isspace(*p))
    {
        ++p;
    }

    if (0 == strncasecmp("on",p,2))
    {
        vob->forced_subs=~0;
        return 0;

    }
    else if (0 == strncasecmp("off",p,3))
    {
        vob->forced_subs=0;
        return 0;
    }

    return -1;
}

/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_fix_unicode
//
// Description:
//    Remove unnicode file double space
//
// Arguments:
//    str - File buffer
//    pos- offset
//
// Return Value:
//    None
//
/////////////////////////////////////////////////////////////////////////////
static void mp_subtitle_fix_unicode(UINT16 *str,unsigned long *pos)
{
    UINT16                        *bot = NULL;
    UINT16                        *eot = NULL;

    MP_SUB_DEBUG("====>>mp_subtitle_fix_unicode()\n");

    if ((!str)||(!pos))
    {
        MP_SUB_DEBUG("mp_subtitle_fix_unicode: Invalid Parameters!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_fix_unicode()\n");
        return;
    }

    bot=0;
    eot=0;
    // Step 3 - remove double spaces
    while (*pos!=0xFFFFFFFF)
    {
        bot=str+*pos;
        eot=bot;
        while (1)
        {
            while (L' ' == *eot && (L' ' == *(eot+1) || L'\0' == *(eot+1)))
            {
                eot++;
            }

            *bot=*eot;

            if (L'\0' == *bot)
            {
                break;
            }

            if (bot!=eot)
            {
                *eot=L' ';
            }

            bot++;
            eot++;
        }
        pos++;
    }

    MP_SUB_DEBUG("<<====mp_subtitle_fix_unicode()\n");
}
/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_fix_utf8
//
// Description:
//    Remove utf8 file double space
//
// Arguments:
//    str - File buffer
//    pos- offset
//
// Return Value:
//    None
//
/////////////////////////////////////////////////////////////////////////////
static void mp_subtitle_fix_utf8(char *str, unsigned long *pos)
{
    char                        *bot = NULL;
    char                        *eot = NULL;

    MP_SUB_DEBUG("====>>mp_subtitle_fix_utf8()\n");

    if ((!str)||(!pos))
    {
        MP_SUB_DEBUG("mp_subtitle_fix_utf8: Invalid Parameters!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_fix_utf8()\n");
        return;
    }

    bot=0;
    eot=0;
    // Step 3 - remove double spaces
    while (*pos!=0xFFFFFFFF)
    {
        bot=str+*pos;
        eot=bot;
        while (1)
        {
            while (' ' == *eot && (' ' == *(eot+1) || '\0' == *(eot+1)))
            {
                eot++;
            }

            *bot=*eot;

            if ('\0' == *bot)
            {
                break;
            }

            if (bot!=eot)
            {
                *eot=' ';
            }

            bot++;
            eot++;
        }
        pos++;
    }

    MP_SUB_DEBUG("<<====mp_subtitle_fix_utf8()\n");
}
/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_fix
//
// Description:
//    Remove subtitle file double space
//
// Arguments:
//    subtitle - Handle
//    str - File buffer
//    pos - Offset
//
// Return Value:
//    None
//
/////////////////////////////////////////////////////////////////////////////
void mp_subtitle_fix(mp_subtitle *subtitle, char *str,unsigned long *pos)
{
    if ((!subtitle)||(!str)||(!pos))
    {
        MP_SUB_DEBUG("mp_subtitle_fix: Invalid Parameters!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_fix()\n");
        return;
    }

    if (MP_SUBTITLE_TXT_TYPE_L_UNICODE == subtitle->txt_file.txt_type)
    {
        mp_subtitle_fix_unicode((UINT16*)str, pos);
    }
    else if ((MP_SUBTITLE_TXT_TYPE_UTF8 == subtitle->txt_file.txt_type) \
             || (MP_SUBTITLE_TXT_TYPE_ANSI == subtitle->txt_file.txt_type))
    {
        mp_subtitle_fix_utf8(str, pos);
    }

    MP_SUB_DEBUG("<<====mp_subtitle_fix()\n");
}

/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_get_previous_sub
//
// Description:
//    Get previous subtitle
//
// Arguments:
//    subtitle - handle
//
// Return Value:
//     time - Previous subtitle time
//    -1 - Failed
//
/////////////////////////////////////////////////////////////////////////////
long  mp_subtitle_get_previous_sub(mp_subtitle *subtitle)
{
    long                            index = 0;
    long long                        real_time = 0;
    char                            str = 0;
    unsigned short                uni_str = 0;

    MP_SUB_DEBUG("====>>mp_subtitle_get_previous_sub()\n");

    if (!subtitle)
    {
        MP_SUB_DEBUG("mp_subtitle_get_previous_sub: Invalid Parameters!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_get_previous_sub()\n");
        return -1;
    }

    index=subtitle->current_pos;
    index--;
    while ((index>=0) && (index<subtitle->total_sub_count-1))
    {
        str = *(subtitle->file_buffer+subtitle->position_offset[index]);
        uni_str = *((unsigned short*)(subtitle->file_buffer+subtitle->position_offset[index]));
        if ((MP_SUBTITLE_TXT_TYPE_B_UNICODE == subtitle->txt_file.txt_type) \
            || (MP_SUBTITLE_TXT_TYPE_L_UNICODE == subtitle->txt_file.txt_type))
        {
            if (L'0' == uni_str)
            {
                index--;
                continue;
            }
            else
            {
                break;
            }

        }
        else
        {
            if ('\0' == str)
            {
                index--;
                continue;
            }
            else
            {
                break;
            }
        }
    }

    if ((index<0) || (index>subtitle->total_sub_count-1))
    {
        MP_SUB_DEBUG("mp_subtitle_get_previous_sub: Can't find previous subtitle!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_get_previous_sub()\n");
        return -1;
    }

    //real_time=subtitle->speed;
    real_time = 1;
    real_time*=TICKSPERSEC;
    real_time/=-10;
    real_time+=subtitle->position_time[index];

    MP_SUB_DEBUG("<<====mp_subtitle_get_previous_sub()\n");

    return (long)real_time;
}
/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_get_next_sub
//
// Description:
//    Get next subtitle
//
// Arguments:
//    subtitle - handle
//
// Return Value:
//     time - Next subtitle time
//    -1 - Failed
//
/////////////////////////////////////////////////////////////////////////////

long mp_subtitle_get_next_sub(mp_subtitle *subtitle)
{
    long                            index = 0;
    long long                        real_time = 0;
    char                            str = 0;
    unsigned short                uni_str = 0;

    MP_SUB_DEBUG("====>>mp_subtitle_get_next_sub()\n");

    if (!subtitle)
    {
        MP_SUB_DEBUG("mp_subtitle_get_next_sub: Invalid Parameters!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_get_next_sub()\n");
        return -1;
    }

    index=subtitle->current_pos;
    index++;
    while ((index>=0) && (index<subtitle->total_sub_count-1))
    {
        str = *(subtitle->file_buffer+subtitle->position_offset[index]);
        uni_str = *((unsigned short*)(subtitle->file_buffer+subtitle->position_offset[index]));
        if ((MP_SUBTITLE_TXT_TYPE_B_UNICODE == subtitle->txt_file.txt_type) \
            || (MP_SUBTITLE_TXT_TYPE_L_UNICODE == subtitle->txt_file.txt_type))
        {
            if (L'0' == uni_str)
            {
                index--;
                continue;
            }
            else
            {
                break;
            }
        }
        else
        {
            if ('\0' == str)
            {
                index--;
                continue;
            }
            else
            {
                break;
            }
        }
    }

    if ((index<0) || (index>subtitle->total_sub_count-1))
    {
        MP_SUB_DEBUG("mp_subtitle_get_next_sub: Invalid Parameters!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_get_next_sub()\n");
        return -1;
    }

    real_time = 1;
    real_time*=TICKSPERSEC;
    real_time/=-10;
    real_time+=subtitle->position_time[index];

    MP_SUB_DEBUG("<<====mp_subtitle_get_next_sub()\n");

    return (long)real_time;
}

/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_get_sub
//
// Description:
//    Get a subtitle related to input time
//
// Arguments:
//    subtitle - Handle
//    curtime - Time
//
// Return Value:
//     time - Subtitle time
//    -1 - Failed
//
/////////////////////////////////////////////////////////////////////////////
long mp_subtitle_get_sub(mp_subtitle *subtitle, unsigned long curtime)
{
    unsigned long time = 0;
    unsigned long prevtime = 0;
    long long real_time = 0;
    long pos = 0;
    long k = 0;

    MP_SUB_DEBUG("====>>mp_subtitle_get_sub_pos()\n");

    if (!subtitle)
    {
        MP_SUB_DEBUG("mp_subtitle_get_sub_pos: Invalid Parameters!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_get_sub_pos()\n");
        return -1;
    }

    if (0xffffffff == curtime)
    {
        MP_SUB_DEBUG("mp_subtitle_get_sub_pos: Invalid Parameters!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_get_sub_pos()\n");
        return -1;
    }

    pos = subtitle->current_pos;

    //realtime=subtitle->speed;
    real_time = 1;
    real_time*=TICKSPERSEC;
    real_time/=10;
    real_time+=curtime;
    curtime=(unsigned long)real_time;
    if ((NULL == subtitle->position_time) \
        || (0==subtitle->total_sub_count) \
        || (curtime>subtitle->position_time[subtitle->total_sub_count-2]))
    {
        subtitle->current_pos=-1;
        MP_SUB_DEBUG("mp_subtitle_get_sub_pos: Invalid Parameters!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_get_sub_pos()\n");
        return -1;
    }

    if ((pos<0) || (pos>subtitle->total_sub_count-1))
    {
        pos = 0;
    }

    for (k=0;k<subtitle->total_sub_count;k++)
    {
        if ((pos<0) || (pos>subtitle->total_sub_count-1))
        {
            pos = 0;
        }

        time    = subtitle->position_time[pos+1];
        prevtime= subtitle->position_time[pos];
        if (subtitle->time_in_ms)
        {
            time /= 100;
            time *= 100;
            prevtime /= 100;
            prevtime *= 100;
        }
        else
        {
            time /= 1000;
            time *= 1000;
            prevtime /= 1000;
            prevtime *= 1000;
        }
        if (pos>subtitle->total_sub_count-1)
        {
            break;
        }
        if (curtime > time)
        {
            pos++;
        }
        else if (curtime < prevtime)
        {
            pos--;
        }
        else
        {
            pos++;
            break;
        }
    }

    if ((pos<0) || (pos>subtitle->total_sub_count-1))
    {
        pos = 0;
    }
    else
    {
        subtitle->current_pos=pos;
    }

    MP_SUB_DEBUG("<<====mp_subtitle_get_sub_pos()\n");

    return subtitle->position_time[pos];
}

/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_get_type_by_name
//
// Description:
//    Get subtitle type
//
// Arguments:
//    name
//
// Return Value:
//    >0 - Type
//    -1 - Failed
//
/////////////////////////////////////////////////////////////////////////////
long mp_subtitle_get_type_by_name(char *name)
{
    char *ext = NULL;
    long type = 0;
    long ext_length = 4;

    MP_SUB_DEBUG("====>>mp_subtitle_get_type_by_name()\n");

    if (!name)
    {
        MP_SUB_DEBUG("mp_subtitle_get_type_by_name: Invalid Parameters!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_get_type_by_name()\n");
        return -1;
    }

    type = MP_SUBTITLE_TYPE_UNKNOWN;
    ext = strrchr(name, (long)'.');
    if (ext)
    {
        if (!strncasecmp(ext, ".SRT", ext_length))
        {
            type = MP_SUBTITLE_TYPE_SRT;
        }
        else if (!strncasecmp(ext, ".IDX", ext_length))
        {
            type = MP_SUBTITLE_TYPE_SUB;
        }
        else if (!strncasecmp(ext, ".TXT", ext_length))
        {
            type = MP_SUBTITLE_TYPE_TXT;//MP_SUBTITLE_TYPE_SUB;//cmx0815
        }
        else if (!strncasecmp(ext, ".SMI", ext_length))
        {
            type = MP_SUBTITLE_TYPE_SMI;
        }
        else if (!strncasecmp(ext, ".ASS", ext_length))
        {
            type = MP_SUBTITLE_TYPE_ASS;
        }
        else if (!strncasecmp(ext, ".SSA", ext_length))
        {
            type = MP_SUBTITLE_TYPE_SSA;
        }
    }

    MP_SUB_DEBUG("<<====mp_subtitle_get_type_by_name()\n");

    return type;
}

/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_sort
//
// Description:
//    Sort a processed subtitle file to time order
//
// Arguments:
//    subtitle - Handle
//    str - File buffer
//    pos - Offset
//    time - Time code
//
// Return Value:
//    None
//
/////////////////////////////////////////////////////////////////////////////
static void mp_subtitle_sort(mp_subtitle *subtitle,char *str,unsigned long *pos,unsigned long *time)
{
    unsigned long                        *pos1 = NULL;
    unsigned long                        *pos2 = NULL;
    unsigned long                        pos3 = 0;
    unsigned long                        *tim1 = NULL;
    unsigned long                        *tim2 = NULL;
    unsigned long                        tim3 = 0;
    unsigned long						 counter;	 //Robin 140320

    if ((!subtitle)||(!str)||(!pos)||(!time))
    {
        MP_SUB_DEBUG("mp_subtitle_sort: Invalid Parameters!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_sort()\n");
        return;
    }

    //Robin 140320	-->
	pos1=pos; tim1=time;
	counter = 0;
	while(*pos1!=0xFFFFFFFF)
	{
	    MP_SUB_DEBUG("counter:%d,pos1:%d,tim1:%d\n",counter,*pos1,*tim1);
		pos2=pos1+1; tim2=tim1+1;
		if(*pos2==0xFFFFFFFF)
			break;

		if((*tim1>*tim2)&&(counter&1))  //just Compare the end of the current and the beginning of the next(1/3/5/7/9 .etc)
		{
			*tim2 = *tim1;
		}
		pos1++;
		tim1++;
		counter++;
	}
	//Robin 140320	<--

    pos1=pos;
    tim1=time;

    while (*pos1!=0xFFFFFFFF)
    {
        pos2=pos1+1;
        tim2=tim1+1;
        while (*pos2!=0xFFFFFFFF)
        {
            if (*tim1>*tim2)
            {
                pos3=*pos2;
                tim3=*tim2;
                *pos2=*pos1;
                *tim2=*tim1;
                *pos1=pos3;
                *tim1=tim3;
            }
            pos2++;
            tim2++;
        }
        pos1++;
        tim1++;
    }

    MP_SUB_DEBUG("<<====mp_subtitle_sort()\n");
}

/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_parse_file
//
// Description:
//    Parse a subtitle file
//
// Arguments:
//    handle - Handle
//
// Return Value:
//     1 - Success
//    -1 - Failed
//
/////////////////////////////////////////////////////////////////////////////
long mp_subtitle_parse_file(void *handle)
{
    mp_subtitle                        *subtitle = NULL;
    FILE                                *file = NULL;
    char                                *filename = NULL;
    unsigned long                        file_length = 0;
    long                                ret = 0;
    unsigned char                        txt_header[10];
    unsigned long                       txt_header_size = 10;

    MP_SUB_DEBUG("====>>mp_subtitle_parse()\n");

    if (!handle)
    {
        MP_SUB_DEBUG("mp_subtitle_parse: Invalid Parameters!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_parse()\n");
        return -1;
    }

    subtitle = (mp_subtitle*)handle;

    if ((subtitle->current_sub_file_id < 0) || (subtitle->avail_file_count > MP_SUBTITLE_TYPE_NUMBER))
    {
        MP_SUB_DEBUG("mp_subtitle_parse: Invalid Parameters!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_parse()\n");
        return -1;
    }

    filename = subtitle->avail_sub_files[subtitle->current_sub_file_id].avail_sub_file;
    file = fopen(filename, "rb");
    if (!file)
    {
        MP_SUB_DEBUG("mp_subtitle_parse: Open file failed!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_parse()\n");
        return -1;
    }

    fseek(file, 0, SEEK_END);
    file_length = ftell(file);
    fseek(file, 0, SEEK_SET);

    ret = fread(txt_header, txt_header_size, 1, file);
    if (ret != (long)txt_header_size)
    {
        MP_SUB_DEBUG("mp_subtitle_parse: fread txt header failed!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_parse()\n");
        fclose(file);
        return -1;
    }

    if ((0xFF == txt_header[0]) && (0xFE == txt_header[1]) )
    {
        subtitle->txt_file.txt_type = MP_SUBTITLE_TXT_TYPE_L_UNICODE;
        subtitle->txt_file.bom = 2;
    }
    else if ((0xFE == txt_header[0]) && (0xFF == txt_header[1]))
    {
        subtitle->txt_file.txt_type = MP_SUBTITLE_TXT_TYPE_B_UNICODE;
        subtitle->txt_file.bom = 2;
    }
    else if ((0xEF == txt_header[0]) && (0xBB == txt_header[1]) && (0xBF == txt_header[2]))
    {
        subtitle->txt_file.txt_type = MP_SUBTITLE_TXT_TYPE_UTF8;
        subtitle->txt_file.bom = 3;
    }
    else if ((0x2B==txt_header[0]) && (0x2F==txt_header[1]) && (0x76==txt_header[2]) \
             && ((0x38==txt_header[3]) || (0x39==txt_header[3]) || (0x2B==txt_header[3]) || (0x2F==txt_header[3])))
    {
        subtitle->txt_file.txt_type = MP_SUBTITLE_TXT_TYPE_UTF7;
        subtitle->txt_file.bom = 4;
    }
    else
    {
        subtitle->txt_file.txt_type = MP_SUBTITLE_TXT_TYPE_ANSI;
        subtitle->txt_file.bom = 0;
    }

    if (file_length+32> subtitle->file_buffer_size_limit)
    {
        MP_SUB_DEBUG("mp_subtitle_parse: Don't have enough memory!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_parse()\n");
        fclose(file);
        return -1;
    }

    fseek(file, subtitle->txt_file.bom, SEEK_SET);
    file_length = file_length-subtitle->txt_file.bom;
    ret = fread(subtitle->file_buffer, file_length, 1, file);
    fclose(file);
    if ((unsigned long)ret != file_length)
    {
        MP_SUB_DEBUG("mp_subtitle_parse: fread file failed!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_parse()\n");
        return -1;
    }

    subtitle->file_buffer[file_length] = 0;
    subtitle->file_buffer[file_length+1] = 0;

    ret = subtitle->avail_sub_files[subtitle->current_sub_file_id].avail_file_type;
    if (MP_SUBTITLE_TYPE_SRT == ret)
    {
        mp_subtitle_parse_srt(subtitle,  subtitle->file_buffer, subtitle->position_offset, subtitle->position_time);
        mp_subtitle_fix(subtitle, subtitle->file_buffer, subtitle->position_offset);
        mp_subtitle_sort(subtitle, subtitle->file_buffer, subtitle->position_offset, subtitle->position_time);
        //mp_subtitle_output_sub(subtitle);

    }
    else if(MP_SUBTITLE_TYPE_TXT == ret)//add for txt special    //cmx0815
    {
        mp_subtitle_parse_txt(subtitle,  subtitle->file_buffer, subtitle->position_offset, subtitle->position_time);
        mp_subtitle_fix(subtitle, subtitle->file_buffer, subtitle->position_offset);
        mp_subtitle_sort(subtitle, subtitle->file_buffer, subtitle->position_offset, subtitle->position_time);
        //mp_subtitle_output_sub(subtitle);
    }
    else if (MP_SUBTITLE_TYPE_SUB == ret)
    {
        mp_subtitle_parse_vobsub(subtitle);
        //mp_subtitle_output_image(subtitle);
    }
    else if (MP_SUBTITLE_TYPE_SMI == ret)
    {
        mp_subtitle_parse_smi(subtitle,  subtitle->file_buffer, subtitle->position_offset, subtitle->position_time);
        mp_subtitle_fix(subtitle, subtitle->file_buffer, subtitle->position_offset);
        mp_subtitle_sort(subtitle, subtitle->file_buffer, subtitle->position_offset, subtitle->position_time);
        //mp_subtitle_output_sub(subtitle);
    }
    else if ((MP_SUBTITLE_TYPE_ASS == ret) || (MP_SUBTITLE_TYPE_SSA == ret))
    {
        mp_subtitle_parse_ass(subtitle,  subtitle->file_buffer, subtitle->position_offset, subtitle->position_time);
        mp_subtitle_fix(subtitle, subtitle->file_buffer, subtitle->position_offset);
        mp_subtitle_sort(subtitle, subtitle->file_buffer, subtitle->position_offset, subtitle->position_time);
        //mp_subtitle_output_sub(subtitle);
    }
    else
    {
        MP_SUB_DEBUG("mp_subtitle_parse: We don't support this type now!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_parse()\n");
        return -1;
    }

    MP_SUB_DEBUG("<<====mp_subtitle_parse()\n");

    return 1;
}

/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_stop
//
// Description:
//    To stop subtitle task
//
// Arguments:
//    subtitle - Subtitle Handle
//
// Return Value:
//    None
//
/////////////////////////////////////////////////////////////////////////////
void mp_subtitle_stop(mp_subtitle *subtitle)
{
    unsigned long                                    flag = 0;

    MP_SUB_DEBUG("====>>mp_subtitle_stop()\n");

    if ((!subtitle))
    {
        MP_SUB_DEBUG("mp_subtitle_stop: Invalid Parameters!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_stop()\n");
        return;
    }

    if (MP_SUBTITLE_TASK_STOPPED == subtitle->sub_task_control.status)
    {
        MP_SUB_DEBUG("mp_subtitle_stop: Task already stopped!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_stop()\n");
        return;
    }

    subtitle->sub_task_control.command = MP_SUBTITLE_TASK_COMMAND_STOP;
    osal_flag_wait(&flag, subtitle->sub_task_control.parameter.flag_id, MP_SUBTITLE_FLAG_MASK, \
                   OSAL_TWF_ORW | OSAL_TWF_CLR, OSAL_WAIT_FOREVER_TIME);

    subtitle->sub_task_control.parameter.task_id = OSAL_INVALID_ID;
    osal_flag_delete(subtitle->sub_task_control.parameter.flag_id);
    subtitle->sub_task_control.parameter.flag_id = OSAL_INVALID_ID;

    subtitle->sub_task_control.status = MP_SUBTITLE_TASK_STOPPED;

    mp_subtitle_callback = NULL;

    MP_SUB_DEBUG("<<====mp_subtitle_stop()\n");

}
/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_pause
//
// Description:
//    To pause subtitle task
//
// Arguments:
//    subtitle - Subtitle Handle
//    pasued - Pause or Resume
//
// Return Value:
//    None
//
/////////////////////////////////////////////////////////////////////////////
void mp_subtitle_pause(mp_subtitle *subtitle, long paused)
{
    MP_SUB_DEBUG("====>>mp_subtitle_pause()\n");

    if ((!subtitle))
    {
        MP_SUB_DEBUG("mp_subtitle_pause: Invalid Parameters!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_pause()\n");
        return;
    }

    if (paused)
    {
        subtitle->sub_task_control.command = MP_SUBTITLE_TASK_COMMAND_PAUSE;
    }
    else
    {
        subtitle->sub_task_control.command = MP_SUBTITLE_TASK_COMMAND_NONE;
    }

    MP_SUB_DEBUG("<<====mp_subtitle_pause()\n");
}


