/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2007 Copyright (C)
 *
 *  File: mp_subtitle_vobsub.c
 *
 *  Description: This file describes .idx .sub file operations.
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
#include "mp_ext_sub_priv.h"
#include "mp_plugin_ext_sub.h"


#define in_range(c, lo, up)  ((unsigned char)c >= lo && (unsigned char)c <= up)
#define isprint(c)           in_range(c, 0x20, 0x7f)
#define isdigit(c)           in_range(c, '0', '9')
#define isxdigit(c)          (isdigit(c) || in_range(c, 'a', 'f') || in_range(c, 'A', 'F'))
#define islower(c)           in_range(c, 'a', 'z')
#define VERSION_TYPE_2 2
#define VERSION_TYPE_4 4
#define PACKET_SIZE 2048

long vobsub_id = 0;

/**************************Static  Function Declare***********************************/
static inline unsigned char mp_subtitle_vobsub_clip_uint8(long a);
static long mp_subtitle_vobsub_getline(char **lineptr, unsigned long *n, mp_subtitle *subtitle);
static long mp_subtitle_vobsub_add_id(mp_subtitle *subtitle, char *id, unsigned long idlen, unsigned long index);
static long mp_subtitle_vobsub_add_timestamp(mp_subtitle *subtitle, unsigned long filepos, long ms);
static long mp_subtitle_vobsub_parse_id(mp_subtitle *subtitle, char *line);
static long mp_subtitle_vobsub_parse_timestamp(mp_subtitle *subtitle, const char *line);
static long mp_subtitle_vobsub_parse_size(mp_vob_sub *vob, const char *line);
static long mp_subtitle_vobsub_parse_origin(mp_vob_sub *vob, const char *line);
static long mp_subtitle_vobsub_parse_palette(mp_vob_sub *vob, const char *line);
static long mp_subtitle_vobsub_parse_custom(mp_vob_sub *vob, const char *line);
static long mp_subtitle_vobsub_parse_cuspal(mp_vob_sub *vob, const char *line);
static long mp_subtitle_vobsub_parse_tridx(const char *line);
static long mp_subtitle_vobsub_parse_delay(mp_vob_sub *vob, const char *line);
static long mp_subtitle_vobsub_set_lang(const char *line);
static long mp_subtitle_vobsub_parse_one_line(mp_subtitle *subtitle, mp_vob_sub *vob);
static inline unsigned long mp_subtitle_vobsub_spu_get_be16(const unsigned char *p);
static inline unsigned long mp_subtitle_vobsub_spu_get_be24(const unsigned char *p);
static inline unsigned char mp_subtitle_vobsub_spu_get_nibble(mp_vob_sub_spu *spu);
static long mp_subtitle_vobsub_spu_parse_dcqt(mp_vob_sub_spu *spu);
static long mp_subtitle_vobsub_spu_parse_dccmd(mp_vob_sub_spu *spu);
static long mp_subtitle_vobsub_parse_packet(unsigned char *current_buffer, unsigned char *buffer_end, \
                                                      unsigned char *buf, mp_vob_sub_mpeg *mpeg, unsigned long *idx, \
                                                      unsigned long *len, long *c);

/****************************API  Function ***********************************/
/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_parse_vobsub
//
// Description:
//    Parse a idx file
//
// Arguments:
//    sub - Subtitle handle
//
// Return Value:
//    None
//
/////////////////////////////////////////////////////////////////////////////
void mp_subtitle_parse_vobsub(void *sub)
{
    long                    ret = 0;
    mp_subtitle            *subtitle = NULL;

    MP_SUB_DEBUG("====>>mp_subtitle_parse_vobsub()\n");

    if ((!sub))
    {
        MP_SUB_DEBUG("mp_subtitle_parse_vobsub: Invalid Parameters!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_parse_vobsub()\n");
        return ;
    }

    subtitle = (mp_subtitle*)sub;

    do
    {
        ret = mp_subtitle_vobsub_parse_one_line(subtitle, &subtitle->vob_sub);
    }while (ret >= 0);

    MP_SUB_DEBUG("<<====mp_subtitle_parse_vobsub()\n");
    return;
}
/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_vobsub_parse_ps_packet
//
// Description:
//    Parse ps packet of a sub file
//
// Arguments:
//    buffer - Buffer contains ps packet
//    buffer_size - Buffer size
//    mpeg - mpeg infomation handle
//
// Return Value:
//    1 - Success
//    -1 - Failed
//
/////////////////////////////////////////////////////////////////////////////
long mp_subtitle_vobsub_parse_ps_packet(unsigned char *buffer, long buffer_size, mp_vob_sub_mpeg *mpeg)
{
    unsigned long len = 0;
    unsigned long idx = 0;
    unsigned long version = 0;
    long          c = 0;
    unsigned char buf[5];
    unsigned char *current_buffer = NULL;
    unsigned char *buffer_end = NULL;

    MP_SUB_DEBUG("====>>mp_subtitle_vobsub_parse_ps_packet()\n");

    if ((!buffer)||(buffer_size <= 0))
    {
        MP_SUB_DEBUG("mp_subtitle_vobsub_parse_ps_packet: Invalid Parameters!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_vobsub_parse_ps_packet()\n");
        return -1;
    }

    mpeg->aid = -1;
    current_buffer = buffer;
    buffer_end = buffer + buffer_size;

    do
    {
        if ((current_buffer + 2) >= buffer_end)
        {
            MP_SUB_DEBUG("mp_subtitle_vobsub_parse_ps_packet: Packet left not enough !\n");
            MP_SUB_DEBUG("<<====mp_subtitle_vobsub_parse_ps_packet()\n");
            return -1;
        }

        if ((current_buffer[0] != 0) || (current_buffer[1] != 0) || (current_buffer[2] != 1))
        {
            ++current_buffer;
            continue;
        }

        current_buffer += 3;
        if (current_buffer >= buffer_end)
        {
            MP_SUB_DEBUG("mp_subtitle_vobsub_parse_ps_packet: Packet left not enough !\n");
            MP_SUB_DEBUG("<<====mp_subtitle_vobsub_parse_ps_packet()\n");
            return -1;
        }

        switch (current_buffer[0])
        {
            case 0xb9:
            {
                //System End Code
                MP_SUB_DEBUG("<<====mp_subtitle_vobsub_parse_ps_packet()\n");
            }
            break;
            case 0xba:
            {
                //Packet start code
                ++current_buffer;
                if (current_buffer >= buffer_end)
                {
                    MP_SUB_DEBUG("mp_subtitle_vobsub_parse_ps_packet: Packet left not enough !\n");
                    MP_SUB_DEBUG("<<====mp_subtitle_vobsub_parse_ps_packet()\n");
                    return -1;
                }

                c = current_buffer[0];
                ++current_buffer;
                if (current_buffer >= buffer_end)
                {
                    MP_SUB_DEBUG("mp_subtitle_vobsub_parse_ps_packet: Packet left not enough !\n");
                    MP_SUB_DEBUG("<<====mp_subtitle_vobsub_parse_ps_packet()\n");
                    return -1;
                }

                if (0x40 == (c & 0xc0))
                {
                    version = VERSION_TYPE_4;
                }
                else if (0x20 == (c & 0xf0))
                {
                    version = VERSION_TYPE_2;
                }
                else
                {
                    MP_SUB_DEBUG("mp_subtitle_vobsub_parse_ps_packet: Unsupported MPEG version: 0x%02x\n", c);
                    MP_SUB_DEBUG("<<====mp_subtitle_vobsub_parse_ps_packet()\n");
                    return -1;
                }

                if (VERSION_TYPE_4 == version)
                {
                    current_buffer += 9;
                    if (current_buffer >= buffer_end)
                    {
                        MP_SUB_DEBUG("mp_subtitle_vobsub_parse_ps_packet: Packet left not enough !\n");
                        MP_SUB_DEBUG("<<====mp_subtitle_vobsub_parse_ps_packet()\n");
                        return -1;
                    }
                }
                else
                {
                    current_buffer += 7;
                    if (current_buffer >= buffer_end)
                    {
                        MP_SUB_DEBUG("mp_subtitle_vobsub_parse_ps_packet: Packet left not enough !\n");
                        MP_SUB_DEBUG("<<====mp_subtitle_vobsub_parse_ps_packet()\n");
                        return -1;
                    }
                }
            }
            break;
            case 0xbd:
            {
                //packet
                if(mp_subtitle_vobsub_parse_packet(current_buffer, buffer_end, buf, mpeg, &idx, &len, &c) > 0)
                {
                    return (current_buffer -buffer + mpeg->packet_size);
                }
            }
            break;
            case 0xbe:
            {
                // Padding
                ++current_buffer;
                if (current_buffer >= buffer_end)
                {
                    MP_SUB_DEBUG("mp_subtitle_vobsub_parse_ps_packet: Packet left not enough !\n");
                    MP_SUB_DEBUG("<<====mp_subtitle_vobsub_parse_ps_packet()\n");
                    return -1;
                }

                current_buffer += 2;
                if (current_buffer >= buffer_end)
                {
                    MP_SUB_DEBUG("mp_subtitle_vobsub_parse_ps_packet: Packet left not enough !\n");
                    MP_SUB_DEBUG("<<====mp_subtitle_vobsub_parse_ps_packet()\n");
                    return -1;
                }

                buf[0] = *(current_buffer -2);
                buf[1] = *(current_buffer -1);

                len = buf[0] << 8 | buf[1];

                current_buffer += len;
                return (current_buffer-buffer);
            }
            default:
            {
                if ((0xc0 <= current_buffer[0]) && (current_buffer[0] < 0xf0))
                {
                    // MPEG audio or video
                    current_buffer += 2;
                    if (current_buffer >= buffer_end)
                    {
                        MP_SUB_DEBUG("mp_subtitle_vobsub_parse_ps_packet: Packet left not enough !\n");
                        MP_SUB_DEBUG("<<====mp_subtitle_vobsub_parse_ps_packet()\n");
                        return -1;
                    }

                    buf[0] = *(current_buffer -2);
                    buf[1] = *(current_buffer -1);
    
                    len = buf[0] << 8 | buf[1];

                    current_buffer += len;
                    if (current_buffer >= buffer_end)
                    {
                        MP_SUB_DEBUG("mp_subtitle_vobsub_parse_ps_packet: Packet left not enough !\n");
                        MP_SUB_DEBUG("<<====mp_subtitle_vobsub_parse_ps_packet()\n");
                        return -1;
                    }
                    continue;
                }
                else
                {
                    MP_SUB_DEBUG("mp_subtitle_vobsub_parse_ps_packet: Unknown header 0x%02X%02X%02X%02X\n",
                    buf[0], buf[1], buf[2], buf[3]);
                    MP_SUB_DEBUG("<<====mp_subtitle_vobsub_parse_ps_packet()\n");
                    return -1;
                }
            }
            break;
        }
    }while(current_buffer < buffer_end);

    MP_SUB_DEBUG("mp_subtitle_vobsub_parse_ps_packet: We don't find a valid packet !\n");
    MP_SUB_DEBUG("<<====mp_subtitle_vobsub_parse_ps_packet()\n");
    return -1;
}

static long mp_subtitle_vobsub_parse_packet(unsigned char *current_buffer, unsigned char *buffer_end, \
                                                      unsigned char *buf, mp_vob_sub_mpeg *mpeg, unsigned long *idx, \
                                                      unsigned long *len, long *c)
{
    // System-2 (.VOB) stream
    unsigned long pts_flags = 0;
    unsigned long hdrlen = 0;
    unsigned long dataidx = 0;

    if(!current_buffer || !buffer_end || !buf)
    {
        return -1;
    }
    if(!idx || !len || !c)
    {
        return -1;
    }

    ++current_buffer;
    if (current_buffer >= buffer_end)
    {
        MP_SUB_DEBUG("mp_subtitle_vobsub_parse_packet: Packet left not enough !\n");
        MP_SUB_DEBUG("<<====mp_subtitle_vobsub_parse_packet()\n");
        return -1;
    }

    current_buffer += 2;
    if (current_buffer >= buffer_end)
    {
        MP_SUB_DEBUG("mp_subtitle_vobsub_parse_packet: Packet left not enough !\n");
        MP_SUB_DEBUG("<<====mp_subtitle_vobsub_parse_packet()\n");
        return -1;
    }

    buf[0] = *(current_buffer -2);
    buf[1] = *(current_buffer -1);
    *len = buf[0] << 8 | buf[1];

    *idx = (unsigned long)current_buffer;

    *c = current_buffer[0];
    ++current_buffer;
    if (current_buffer >= buffer_end)
    {
        MP_SUB_DEBUG("mp_subtitle_vobsub_parse_packet: Packet left not enough !\n");
        MP_SUB_DEBUG("<<====mp_subtitle_vobsub_parse_packet()\n");
        return -1;
    }

    if (0x40 == (*c & 0xC0))
    {
        // skip STD scale & size
        *c = current_buffer[0];
        current_buffer += 2;
        if (current_buffer >= buffer_end)
        {
            MP_SUB_DEBUG("mp_subtitle_vobsub_parse_packet: Packet left not enough !\n");
            MP_SUB_DEBUG("<<====mp_subtitle_vobsub_parse_packet()\n");
            return -1;
        }    
    }

    if (0x20 == (*c & 0xf0))
    {
        // System-1 stream timestamp
        // Do we need this?
        MP_SUB_DEBUG("mp_subtitle_vobsub_parse_packet: Unexpected System-1 stream timestamp!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_vobsub_parse_packet()\n");
        return -1;
    }
    else if (0x30 == (*c & 0xf0))
    {
        // Do we need this?
        MP_SUB_DEBUG("mp_subtitle_vobsub_parse_packet: Unexpected value!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_vobsub_parse_packet()\n");
        return -1;
    }
    else if (0x80 == (*c & 0xc0))
    {
        *c = current_buffer[0];
        current_buffer += 1;
        if (current_buffer >= buffer_end)
        {
            MP_SUB_DEBUG("mp_subtitle_vobsub_parse_packet: Packet left not enough !\n");
            MP_SUB_DEBUG("<<====mp_subtitle_vobsub_parse_packet()\n");
            return -1;
        }
        pts_flags = *c;

        *c = current_buffer[0];
        current_buffer += 1;
        if (current_buffer >= buffer_end)
        {
            MP_SUB_DEBUG("mp_subtitle_vobsub_parse_packet: Packet left not enough !\n");
            MP_SUB_DEBUG("<<====mp_subtitle_vobsub_parse_packet()\n");
            return -1;
        }
        hdrlen = *c;

        dataidx = (unsigned long)current_buffer + hdrlen;
        if (dataidx > *idx + *len)
        {
            MP_SUB_DEBUG("mp_subtitle_vobsub_parse_packet: Invalid header length: %d (total length: %d, idx: %d, dataidx: %d)\n",
            hdrlen, *len, *idx, dataidx);
            MP_SUB_DEBUG("<<====mp_subtitle_vobsub_parse_packet()\n");
            return -1;
        }
        if (0x80 == (pts_flags & 0xc0))
        {
            current_buffer += 5;
            if (current_buffer >= buffer_end)
            {
                MP_SUB_DEBUG("mp_subtitle_vobsub_parse_packet: Packet left not enough !\n");
                MP_SUB_DEBUG("<<====mp_subtitle_vobsub_parse_packet()\n");
                return -1;
            }

            buf[0] = *(current_buffer -5);
            buf[1] = *(current_buffer -4);
            buf[2] = *(current_buffer -3);
            buf[3] = *(current_buffer -2);
            buf[4] = *(current_buffer -1);

            if (!((0x20 == (buf[0] & 0xf0)) && (buf[0] & 1) && (buf[2] & 1) &&  (buf[4] & 1)))
            {
                MP_SUB_DEBUG("mp_subtitle_vobsub_parse_packet: vobsub PTS error: 0x%02x %02x%02x %02x%02x \n",
                buf[0], buf[1], buf[2], buf[3], buf[4]);
                mpeg->pts = 0;
            }
            else
            {
                mpeg->pts = ((buf[0] & 0x0e) << 29 | buf[1] << 22 | (buf[2] & 0xfe) << 14
                | buf[3] << 7 | (buf[4] >> 1));
            }
        }

        current_buffer = (unsigned char*)dataidx;

        *c = current_buffer[0];
        current_buffer += 1;
        if (current_buffer >= buffer_end)
        {
            MP_SUB_DEBUG("mp_subtitle_vobsub_parse_packet: Packet left not enough !\n");
            MP_SUB_DEBUG("<<====mp_subtitle_vobsub_parse_packet()\n");
            return -1;
        }

        mpeg->aid = *c;
        mpeg->packet_size = *len - ((unsigned long) current_buffer - *idx);
        if (mpeg->packet_size > PACKET_SIZE)
        {
            ASSERT(0);
            MP_SUB_DEBUG("mp_subtitle_vobsub_parse_packet: Packet Size too large!\n");
            MP_SUB_DEBUG("<<====mp_subtitle_vobsub_parse_packet()\n");
            return -1;
        }

        if ((current_buffer + mpeg->packet_size - 1) >= buffer_end)
        {
            MP_SUB_DEBUG("mp_subtitle_vobsub_parse_packet: Packet left not enough !\n");
            MP_SUB_DEBUG("<<====mp_subtitle_vobsub_parse_packet()\n");
            return -1;
        }

        MEMCPY(mpeg->packet, current_buffer, mpeg->packet_size);

        *idx = *len;

        MP_SUB_DEBUG("<<====mp_subtitle_vobsub_parse_packet()\n");
        return 1; //(current_buffer -buffer + mpeg->packet_size);
    }
    return 0;
}
/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_vobsub_spu_parse_pxd
//
// Description:
//    Parse pixel data of spu packet
//
// Arguments:
//    vob - Pointer to a struct contains Vob infomation
//    spu - Pointer to a struct contains Spu infomation
//
// Return Value:
//    1 - Success
//    -1 - Failed
//
/////////////////////////////////////////////////////////////////////////////
long mp_subtitle_vobsub_spu_parse_pxd(mp_vob_sub *vob, mp_vob_sub_spu *spu)
{
    unsigned long i = 0;
    unsigned long x = 0;
    unsigned long y = 0;
    unsigned long len = 0;
    unsigned long color = 0;
    unsigned long rle = 0;
    unsigned long rle_len4 = 0x04;
    unsigned long rle_len8 = 0x10;
    unsigned long rle_len64 = 0x40;

    MP_SUB_DEBUG("====>>mp_subtitle_vobsub_spu_parse_pxd()\n");

    if ((!vob) || (!spu))
    {
        MP_SUB_DEBUG("mp_subtitle_vobsub_spu_parse_pxd: Invalid Parameters!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_vobsub_spu_parse_pxd()\n");
        return -1;
    }

    if (spu->image_size < spu->stride * spu->height)
    {
        if (spu->image != NULL)
        {
            FREE(spu->image);
            spu->image_size = 0;
        }

        spu->image = MALLOC(spu->stride * spu->height);
        if (spu->image)
        {
            spu->image_size = spu->stride * spu->height;
        }
    }

    if (NULL == spu->image)
    {
        MP_SUB_DEBUG("mp_subtitle_vobsub_spu_parse_pxd: Malloc failed!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_vobsub_spu_parse_pxd()\n");
        return -1;
    }

    if (spu->width < spu->stride)
    {
        for (y = 0; y < spu->height; ++y)
        {
            MEMSET(spu->image + y * spu->stride + spu->width, 16, spu->stride - spu->width);
        }
    }

    i = spu->current_nibble[1];
    x = 0;
    y = 0;
    while (spu->current_nibble[0] < i
    && spu->current_nibble[1] / 2 < spu->control_start
    && y < spu->height)
    {
        rle = mp_subtitle_vobsub_spu_get_nibble(spu);
        if (rle < rle_len4)
        {
            rle = (rle << 4) | mp_subtitle_vobsub_spu_get_nibble(spu);
            if (rle < rle_len8)
            {
                rle = (rle << 4) | mp_subtitle_vobsub_spu_get_nibble(spu);
                if (rle < rle_len64)
                {
                    rle = (rle << 4) | mp_subtitle_vobsub_spu_get_nibble(spu);
                    if (rle < rle_len4)
                    {
                        rle |= ((spu->width - x) << 2);
                    }
                }
            }
        }
        color = 3 - (rle & 0x3);
        len = rle >> 2;
        if ((len > spu->width - x) || (0 == len))
        {
            len = spu->width - x;
        }
        //have to use palette and alpha map
        if ((0 == color) || (0 == spu->alpha[color]))
        {
            color = 16;
        }
        else
        {
            color = spu->palette[color];
        }
        MEMSET(spu->image + y * spu->stride + x, color, len);
        x += len;
        if (x >= spu->width)
        {
            mp_subtitle_vobsub_spu_next_line(spu);
            x = 0;
            ++y;
        }
    }

    //spudec_cut_image(spu);
    MP_SUB_DEBUG("<<====mp_subtitle_vobsub_spu_parse_pxd()\n");
    return 1;
}

/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_vobsub_spu_reassemble
//
// Description:
//    Reassemble a spu packet. Decode spu packet when reassemble is done.
//
// Arguments:
//    spu - Pointer to a struct contains Spu infomation
//    mpeg - Pointer to a struct contains Mpeg infomation
//
// Return Value:
//    1 - Success
//    -1 - Failed
//
/////////////////////////////////////////////////////////////////////////////
long mp_subtitle_vobsub_spu_reassemble(mp_vob_sub_spu *spu, mp_vob_sub_mpeg *mpeg)
{
    long                                ret = 0;
    unsigned long                        packet_length = 0;

    MP_SUB_DEBUG("====>>mp_subtitle_vobsub_spu_reassemble()\n");

    if ((!mpeg) || (!spu))
    {
        MP_SUB_DEBUG("mp_subtitle_vobsub_spu_reassemble: Invalid Parameters!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_vobsub_spu_reassemble()\n");
        return -1;
    }

    if (0 == spu->packet_offset)
    {
        packet_length = mp_subtitle_vobsub_spu_get_be16(mpeg->packet);
        spu->packet_size = packet_length;
        if (mpeg->packet_size > packet_length)
        {
            MP_SUB_DEBUG("mp_subtitle_vobsub_spu_reassemble: Invalid fragment length!\n");
            MP_SUB_DEBUG("<<====mp_subtitle_vobsub_spu_reassemble()\n");
            return -1;
        }
        MEMCPY(spu->packet, mpeg->packet, mpeg->packet_size);
        spu->packet_offset = mpeg->packet_size;
        spu->pts = mpeg->pts;
    }
    else
    {
        //Continue current fragment
        if (spu->packet_size < spu->packet_offset + mpeg->packet_size)
        {
            spu->packet_size = 0;
            spu->packet_offset = 0;
            MP_SUB_DEBUG("mp_subtitle_vobsub_spu_reassemble: Invalid fragment length!\n");
            MP_SUB_DEBUG("<<====mp_subtitle_vobsub_spu_reassemble()\n");
            return -1;
        }

        MEMCPY(spu->packet + spu->packet_offset, mpeg->packet, mpeg->packet_size);
        spu->packet_offset += mpeg->packet_size;
    }


    if ((spu->packet_offset == spu->packet_size) || ((spu->packet_offset + 1) == spu->packet_size))
    {
        //We got a whole SPU packet. Decode it!
        ret = mp_subtitle_vobsub_spu_parse_dcqt(spu);
        if (ret < 0)
        {
            MP_SUB_DEBUG("mp_subtitle_vobsub_spu_reassemble: Parse DCQT failed!\n");
            MP_SUB_DEBUG("<<====mp_subtitle_vobsub_spu_reassemble()\n");
            return -1;
        }

        MP_SUB_DEBUG("<<====mp_subtitle_vobsub_spu_reassemble()\n");
        return 2;
    }

    MP_SUB_DEBUG("<<====mp_subtitle_vobsub_spu_reassemble()\n");
    return 1;
}
/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_vobsub_query_language
//
// Description:
//    Query language infomation in vobsub subtitle file
//
// Arguments:
//    subtitle - Subtitle handle
//    vob - Pointer to a struct contains Vob infomation
//    number - Language Number
//    description - Language description
//    description_len - Length of language description
//
// Return Value:
//    1 - Success
//    -1 - Failed
//
/////////////////////////////////////////////////////////////////////////////
long mp_subtitle_vobsub_query_language(void *sub, mp_vob_sub *vob, long *number, long *current_language_index, \
                                                  char *description, long *description_len)
{
    long                            temp = 0;
    long                            char_copyed = 0;

    MP_SUB_DEBUG("====>>mp_subtitle_vobsub_query_language()\n");

    if ((!sub) || (!vob))
    {
        MP_SUB_DEBUG("mp_subtitle_vobsub_query_language: Invalid Parameters!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_vobsub_query_language()\n");
        return -1;
    }

    char_copyed= 0;

    if (number)
    {
        *number = vob->total_id_count;
    }

    if (current_language_index)
    {
        *current_language_index = vob->current_id_index;
    }

    if (description)
    {
        for (temp = 0; temp < vob->total_id_count; ++temp)
        {
            if ((description_len) && (*description_len > 0))
            {
                if ((*description_len - 3) >= 0)
                {
                    strncpy(description, vob->id_description[temp], 3);
                    char_copyed += 3;
                    description += 3;
                    *description_len -= 3;
                    if (*description_len >= 1)
                    {
                        *description = 0;
                        char_copyed += 1;
                        description += 1;
                        *description_len -= 1;
                    }
                    else
                    {
                        break;
                    }
                }
                else
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }
    }

    if (description_len)
    {
        *description_len = char_copyed;
    }

    MP_SUB_DEBUG("<<====mp_subtitle_vobsub_query_language()\n");
    return 1;
}

/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_vobsub_change_language
//
// Description:
//    Change vobsub language.
//
// Arguments:
//    subtitle - Subtitle handle
//    vob - Pointer to a struct contains Vob infomation
//    language_index - Language index. Started from 0
//
// Return Value:
//    1 - Success
//    -1 - Failed
//
/////////////////////////////////////////////////////////////////////////////
long mp_subtitle_vobsub_change_language(void *sub, mp_vob_sub *vob, long language_index)
{
    long                            temp = 0;
    mp_subtitle                    *subtitle = NULL;

    MP_SUB_DEBUG("====>>mp_subtitle_vobsub_change_language()\n");

    if ((!sub) || (!vob) || (language_index < 0) || (language_index >= vob->total_id_count))
    {
        MP_SUB_DEBUG("mp_subtitle_vobsub_change_language: Invalid Parameters!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_vobsub_change_language()\n");
        return -1;
    }

    subtitle = (mp_subtitle*)sub;

    temp = vob->id_pos[language_index];

    if (temp < subtitle->total_sub_count -1)
    {
        subtitle->current_pos = temp;
        vob->current_id_index = language_index;
    }

    MP_SUB_DEBUG("<<====mp_subtitle_vobsub_change_language()\n");
    return 1;
}


/**************************Internal Static  Function ***********************************/
/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_vobsub_clip_uint8
//
// Description:
//    Convert a long value to byte
//
// Arguments:
//    Input value
//
// Return Value:
//    Converted value
//
/////////////////////////////////////////////////////////////////////////////
static inline unsigned char mp_subtitle_vobsub_clip_uint8(long a)
{
    long b = a&(~255);

    if (b)
    {
        return (-a)>>31;
    }
    else
    {
        return a;
    }
}

/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_vobsub_getline
//
// Description:
//    Get a line from .idx file
//
// Arguments:
//    lineptr - Pointer to line's 1st byte
//    n - Line's length
//    subtitle - Subtitle Handle
//
// Return Value:
//    0 - Success
//    -1 - Failed
//
/////////////////////////////////////////////////////////////////////////////
static long mp_subtitle_vobsub_getline(char **lineptr, unsigned long *n, mp_subtitle *subtitle)
{
    mp_vob_sub                        *vob = NULL;
    unsigned long                    pos = 0;

    MP_SUB_DEBUG("====>>mp_subtitle_vobsub_getline()\n");

    if ((!lineptr)||(!n)||(!subtitle))
    {
        MP_SUB_DEBUG("mp_subtitle_vobsub_getline: Invalid Parameters!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_vobsub_getline()\n");
        return -1;
    }

    vob= &subtitle->vob_sub;
    pos = vob->idx_file_pos;

    while (1)
    {
        if (('\0' == subtitle->file_buffer[pos]))
        {
            pos = 0xffffffff;
            break;
        }
        if ('\n' == subtitle->file_buffer[pos])
        {
            subtitle->file_buffer[pos] = '\0';
            break;
        }

        ++pos;
    }

    if (0xffffffff == pos)
    {
        MP_SUB_DEBUG("mp_subtitle_vobsub_getline: We do not find a line!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_vobsub_getline()\n");
        return -1;
    }

    *lineptr = subtitle->file_buffer + vob->idx_file_pos;
    *n = pos - vob->idx_file_pos;
    vob->idx_file_pos = pos + 1;

    MP_SUB_DEBUG("<<====mp_subtitle_vobsub_getline()\n");
    return 1;
}
/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_vobsub_add_id
//
// Description:
//    Add id of idx file to struct MpVobSub
//
// Arguments:
//    subtitle - Subtitle handle
//    id - We do not record it now
//    idlen - The length of string id
//    index - The index of id. We record it instead of id
//
// Return Value:
//    0 - Success
//    -1 - Failed
//
/////////////////////////////////////////////////////////////////////////////
static long mp_subtitle_vobsub_add_id(mp_subtitle *subtitle, char *id, unsigned long idlen, unsigned long index)
{
    mp_vob_sub                        *vob = NULL;
    long                            length = 0;
    unsigned long                   idlen_max = 3;

    MP_SUB_DEBUG("====>>mp_subtitle_vobsub_add_id()\n");

    if ((!subtitle)||(!id))
    {
        MP_SUB_DEBUG("mp_subtitle_vobsub_add_id: Invalid Parameters!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_vobsub_add_id()\n");
        return -1;
    }

    vob= &subtitle->vob_sub;
    if (vob->total_id_count >= MAX_ID_COUNT)
    {
        MP_SUB_DEBUG("mp_subtitle_vobsub_add_id: Max id reached!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_vobsub_add_id()\n");
        return -1;
    }

    vob->id[vob->total_id_count] = index;
    vob->id_pos[vob->total_id_count] = subtitle->total_sub_count;
    if (idlen>idlen_max)
    {
        length = 3;
    }
    else
    {
        length = idlen;
    }
    strncpy(&(vob->id_description[vob->total_id_count][0]), id, length);
    ++vob->total_id_count;


    MP_SUB_DEBUG("<<====mp_subtitle_vobsub_add_id()\n");
    return 1;
}
/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_vobsub_add_timestamp
//
// Description:
//    Record timestamp of the idx file
//
// Arguments:
//    subtitle - Subtitle handle
//    filepos - The corresponding file pos of timestamp
//    ms - Timestamp
//
// Return Value:
//    0 - Success
//    -1 - Failed
//
/////////////////////////////////////////////////////////////////////////////
static long mp_subtitle_vobsub_add_timestamp(mp_subtitle *subtitle, unsigned long filepos, long ms)
{
    MP_SUB_DEBUG("====>>mp_subtitle_vobsub_add_timestamp()\n");

    if ((!subtitle))
    {
        MP_SUB_DEBUG("mp_subtitle_vobsub_add_timestamp: Invalid Parameters!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_vobsub_add_timestamp()\n");
        return -1;
    }

    if (subtitle->total_sub_count > MP_SUBTITLE_MAX_SUB_COUNT-3)
    {
        MP_SUB_DEBUG("mp_subtitle_vobsub_add_timestamp: Max sub count reached!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_vobsub_add_timestamp()\n");
        return -1;
    }

    subtitle->position_time[subtitle->total_sub_count] = ms;
    subtitle->position_offset[subtitle->total_sub_count] = filepos;
    ++subtitle->total_sub_count;

    MP_SUB_DEBUG("<<====mp_subtitle_vobsub_add_timestamp()\n");

    return 1;
}

/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_vobsub_parse_id
//
// Description:
//    Parse id infomation of a idx file
//
// Arguments:
//    subtitle - Subtitle handle
//    line - The line which contains id infomation
//
// Return Value:
//    0 - Success
//    -1 - Failed
//
/////////////////////////////////////////////////////////////////////////////
static long mp_subtitle_vobsub_parse_id(mp_subtitle *subtitle, char *line)
{
    // id: xx, index: n
    long idlen = 0;
    char *p = NULL;
    char *q = NULL;
    UINT32 len = 0;

    p  = line;
    while (isspace(*p))
    {
        ++p;
    }
    q = p;

    while (isalpha(*q))
    {
        ++q;
    }
    idlen = q - p;

    if (0 == idlen)
    {
        return -1;
    }
    ++q;

    while (isspace(*q))
    {
        ++q;
    }
    len = strlen("index:");
    if (strncmp("index:", q, len))
    {
        return -1;
    }
    q += 6;

    while (isspace(*q))
    {
        ++q;
    }
    if (!isdigit(*q))
    {
        return -1;
    }
    return mp_subtitle_vobsub_add_id(subtitle, p, idlen, atoi(q));
}
/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_vobsub_parse_timestamp
//
// Description:
//    Parse timestamp infomation of a idx file
//
// Arguments:
//    subtitle - Subtitle handle
//    line - The line which contains timestamp infomation
//
// Return Value:
//    0 - Success
//    -1 - Failed
//
/////////////////////////////////////////////////////////////////////////////
static long mp_subtitle_vobsub_parse_timestamp(mp_subtitle *subtitle, const char *line)
{
    // timestamp: HH:MM:SS.mmm, filepos: 0nnnnnnnnn
    mp_vob_sub                        *vob = NULL;
    const char *p = NULL;
    long h = 0;
    long m = 0;
    long s = 0;
    long ms = 0;
    unsigned long filepos = 0;
    unsigned long len_sec = 2;
    unsigned long len_ms = 3;

    vob= &subtitle->vob_sub;

    while (isspace(*line))
    {
        ++line;
    }
    p = line;

    while (isdigit(*p))
    {
        ++p;
    }

    if (p - line != (long)len_sec)
    {
        return -1;
    }
    h = atoi(line);

    if (*p != ':')
    {
        return -1;
    }
    line = ++p;

    while (isdigit(*p))
    {
        ++p;
    }

    if (p - line != (long)len_sec)
    {
        return -1;
    }

    m = atoi(line);
    if (*p != ':')
    {
        return -1;
    }
    line = ++p;

    while (isdigit(*p))
    {
        ++p;
    }

    if (p - line != (long)len_sec)
    {
        return -1;
    }
    s = atoi(line);

    if (*p != ':')
    {
        return -1;
    }
    line = ++p;

    while (isdigit(*p))
    {
        ++p;
    }

    if (p - line != (long)len_ms)
    {
        return -1;
    }
    ms = atoi(line);

    if (*p != ',')
    {
        return -1;
    }
    line = p + 1;

    while (isspace(*line))
    {
        ++line;
    }

    if (strncmp("filepos:", line, strlen("filepos:")))
    {
        return -1;
    }
    line += 8;

    while (isspace(*line))
    {
        ++line;
    }

    if (!isxdigit(*line))
    {
        return -1;
    }
    filepos = strtol(line, NULL, 16);

    return mp_subtitle_vobsub_add_timestamp(subtitle, filepos, vob->delay + ms + 1000 * (s + 60 * (m + 60 * h)));
}
/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_vobsub_parse_size
//
// Description:
//    Parse frame size infomation of a idx file
//
// Arguments:
//    vob - Vob handle
//    line - The line which contains frame size infomation
//
// Return Value:
//    0 - Success
//    -1 - Failed
//
/////////////////////////////////////////////////////////////////////////////
static long mp_subtitle_vobsub_parse_size(mp_vob_sub *vob, const char *line)
{
    // size: WWWxHHH
    char *p = NULL;

    while (isspace(*line))
    {
        ++line;
    }

    if (!isdigit(*line))
    {
        return -1;
    }

    vob->orig_frame_width = strtoul(line, &p, 10);

    if (*p != 'x')
    {
        return -1;
    }
    ++p;

    vob->orig_frame_height = strtoul(p, NULL, 10);

    return 0;
}
/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_vobsub_parse_origin
//
// Description:
//    Parse origin x,y infomation of a idx file
//
// Arguments:
//    vob - Vob handle
//    line - The line which contains origin x,y infomation
//
// Return Value:
//    0 - Success
//    -1 - Failed
//
/////////////////////////////////////////////////////////////////////////////
static long mp_subtitle_vobsub_parse_origin(mp_vob_sub *vob, const char *line)
{
    // org: X,Y
    char *p = NULL;

    while (isspace(*line))
    {
        ++line;
    }

    if (!isdigit(*line))
    {
        return -1;
    }

    vob->origin_x = strtoul(line, &p, 10);

    if (*p != ',')
    {
        return -1;
    }

    ++p;

    vob->origin_y = strtoul(p, NULL, 10);

    return 0;
}
/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_vobsub_parse_palette
//
// Description:
//    Parse palette infomation of a idx file
//
// Arguments:
//    vob - Vob handle
//    line - The line which contains palette infomation
//
// Return Value:
//    0 - Success
//    -1 - Failed
//
/////////////////////////////////////////////////////////////////////////////
static long mp_subtitle_vobsub_parse_palette(mp_vob_sub *vob, const char *line)
{
    unsigned long n = 0;
    const char *p = NULL;
    long r = 0;
    long g = 0;
    long b = 0;
    long y = 0;
    long u = 0;
    long v = 0;
    long tmp = 0;
    unsigned long loop_times = 16;
    unsigned long num_xdigit = 6;

    n = 0;
    while (1)
    {
        while (isspace(*line))
        {
            ++line;
        }
        p = line;

        while (isxdigit(*p))
        {
            ++p;
        }

        if (p - line != (long)num_xdigit)
        {
            return -1;
        }

        tmp = strtoul(line, NULL, 16);
        r = tmp >> 16 & 0xff;
        g = tmp >> 8 & 0xff;
        b = tmp & 0xff;

        y = mp_subtitle_vobsub_clip_uint8( 0.1494  * r + 0.6061 * g + 0.2445 * b);
        u = mp_subtitle_vobsub_clip_uint8( 0.6066  * r - 0.4322 * g - 0.1744 * b + 128);
        v = mp_subtitle_vobsub_clip_uint8(-0.08435 * r - 0.3422 * g + 0.4266 * b + 128);
        vob->palette[n++] = (0xf <<24 ) | v << 16 | u << 8 | y;

        if (loop_times == n)
        {
            break;
        }

        if (',' == *p)
        {
            ++p;
        }
        line = p;
    }
    vob->have_palette = 1;
    return 0;
}
/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_vobsub_parse_custom
//
// Description:
//    Parse custom color infomation of a idx file
//
// Arguments:
//    vob - Vob handle
//    line - The line which contains custom color infomation
//
// Return Value:
//    0 - Success
//    -1 - Failed
//
/////////////////////////////////////////////////////////////////////////////
static long mp_subtitle_vobsub_parse_custom(mp_vob_sub *vob, const char *line)
{
    //custom colors: OFF/ON(0/1)
    if ((0 == strncmp("ON", line + 15, 2)) || 0 == strncmp("1", line + 15, 1))
    {
        vob->custom=1;
    }
    else if ((0 == strncmp("OFF", line + 15, 3)) || 0 == strncmp("0", line + 15, 1))
    {
        vob->custom=0;
    }
    else
    {
        return -1;
    }

    return 0;
}
/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_vobsub_parse_cuspal
//
// Description:
//    Parse custom palette infomation of a idx file
//
// Arguments:
//    vob - Vob handle
//    line - The line which contains custom palette infomation
//
// Return Value:
//    0 - Success
//    -1 - Failed
//
/////////////////////////////////////////////////////////////////////////////
static long mp_subtitle_vobsub_parse_cuspal(mp_vob_sub *vob, const char *line)
{
    //colors: XXXXXX, XXXXXX, XXXXXX, XXXXXX
    unsigned long n = 0;
    const char *p = NULL;
    unsigned long loop_times = 4;
    unsigned long num_digit = 6;

    n = 0;
    line += 40;
    while(1)
    {
        while (isspace(*line))
        {
            ++line;
        }
        p=line;

        while (isxdigit(*p))
        {
            ++p;
        }

        if (p - line != (long)num_digit)
        {
            return -1;
        }

        vob->cuspal[n++] = strtoul(line, NULL,16);

        if (n == loop_times)
        {
            break;
        }

        if(',' == *p)
        {
            ++p;
        }
        line = p;
    }
    return 0;
}
/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_vobsub_parse_tridx
//
// Description:
//    Parse tridx infomation of a idx file
//
// Arguments:
//    line - The line which contains tridx infomation
//
// Return Value:
//    tridx
//
/////////////////////////////////////////////////////////////////////////////
static long mp_subtitle_vobsub_parse_tridx(const char *line)
{
    //tridx: XXXX
    long tridx = 0;

    tridx = strtoul((line + 26), NULL, 16);
    tridx = ((tridx&0x1000)>>12) | ((tridx&0x100)>>7) | ((tridx&0x10)>>2) | ((tridx&1)<<3);
    return tridx;
}
/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_vobsub_parse_delay
//
// Description:
//    Parse delay infomation of a idx file
//
// Arguments:
//    vob - Vob handle
//    line - The line which contains delay infomation
//
// Return Value:
//    0 - Success
//
/////////////////////////////////////////////////////////////////////////////
static long mp_subtitle_vobsub_parse_delay(mp_vob_sub *vob, const char *line)
{
    long h = 0;
    long m = 0;
    long s = 0;
    long ms = 0;
    long forward = 1;

    if ('+' == *(line + 7))
    {
        forward = 1;
        line++;
    }
    else if ('-' == *(line + 7))
    {
        forward = -1;
        line++;
    }

    MP_SUB_DEBUG("forward=%d", forward);
    h = atoi(line + 7);
    MP_SUB_DEBUG("h=%d," ,h);
    m = atoi(line + 10);
    MP_SUB_DEBUG("m=%d,", m);
    s = atoi(line + 13);
    MP_SUB_DEBUG("s=%d,", s);
    ms = atoi(line + 16);
    MP_SUB_DEBUG("ms=%d", ms);
    vob->delay = (ms + 1000 * (s + 60 * (m + 60 * h))) * forward;
    return 0;
}
/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_vobsub_set_lang
//
// Description:
//    Parse language infomation of a idx file
//
// Arguments:
//    line - The line which contains language infomation
//
// Return Value:
//    0 - Success
//
/////////////////////////////////////////////////////////////////////////////
static long mp_subtitle_vobsub_set_lang(const char *line)
{
    if (-1 == vobsub_id)
    {
        vobsub_id = atoi(line + 8);
    }

    return 0;
}

/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_vobsub_parse_one_line
//
// Description:
//    Parse a line of a idx file
//
// Arguments:
//    subtitle - Subtitle handle
//    vob - Vob handle
//
// Return Value:
//    0 - Success
//    -1 - Failed
//
/////////////////////////////////////////////////////////////////////////////
static long mp_subtitle_vobsub_parse_one_line(mp_subtitle *subtitle, mp_vob_sub *vob)
{
    long line_size = 0;
    long res = -1;
    unsigned long line_reserve = 0;
    char *line = NULL;

    do
    {
        line_size = mp_subtitle_vobsub_getline(&line, &line_reserve, subtitle);
        if (line_size < 0)
        {
            break;
        }

        if ((0 == *line) || ('\r' == *line) || ('\n' == *line) || ('#' == *line))
        {
            continue;
        }
        else if (0 == strncmp("langidx:", line, 8))
        {
            res = mp_subtitle_vobsub_set_lang(line);
        }
        else if (0 == strncmp("delay:", line, 6))
        {
            res = mp_subtitle_vobsub_parse_delay(vob, line);
        }
        else if (0 == strncmp("id:", line, 3))
        {
            res = mp_subtitle_vobsub_parse_id(subtitle, line + 3);
        }
        else if (0 == strncmp("palette:", line, 8))
        {
            res = mp_subtitle_vobsub_parse_palette(vob, line + 8);
        }
        else if (0 == strncmp("size:", line, 5))
        {
            res = mp_subtitle_vobsub_parse_size(vob, line + 5);
        }
        else if (0 == strncmp("org:", line, 4))
        {
            res = mp_subtitle_vobsub_parse_origin(vob, line + 4);
        }
        else if (0 == strncmp("timestamp:", line, 10))
        {
            res = mp_subtitle_vobsub_parse_timestamp(subtitle, line + 10);
        }
        else if (0 == strncmp("custom colors:", line, 14))
        {
            //custom colors: ON/OFF, tridx: XXXX, colors: XXXXXX, XXXXXX, XXXXXX,XXXXXX
            res = mp_subtitle_vobsub_parse_cuspal(vob, line) \
                  + mp_subtitle_vobsub_parse_tridx(line) \
                  + mp_subtitle_vobsub_parse_custom(vob, line);
        }
        else if (0 == strncmp("forced subs:", line, 12))
        {
            res = mp_subtitle_vobsub_parse_forced_subs(vob, line + 12);
        }
        else
        {
            MP_SUB_DEBUG("vobsub: ignoring %s", line);
            continue;
        }

        if (res < 0)
        {
            MP_SUB_DEBUG("ERROR in %s", line);
        }

        break;
    } while (1);

    return res;
}
/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_vobsub_spu_get_be16
//
// Description:
//    Convert 2 bytes to a 16bit value
//
// Arguments:
//    p - buffer contains 2 byte
//
// Return Value:
//    Converted 16bit value
//
/////////////////////////////////////////////////////////////////////////////
static inline unsigned long mp_subtitle_vobsub_spu_get_be16(const unsigned char *p)
{
    return (p[0] << 8) + p[1];
}
/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_vobsub_spu_get_be24
//
// Description:
//    Convert 2 bytes to a 24bit value
//
// Arguments:
//    p - buffer contains 3 byte
//
// Return Value:
//    Converted 24bit value
//
/////////////////////////////////////////////////////////////////////////////
static inline unsigned long mp_subtitle_vobsub_spu_get_be24(const unsigned char *p)
{
    return (mp_subtitle_vobsub_spu_get_be16(p) << 8) + p[2];
}
/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_vobsub_spu_get_nibble
//
// Description:
//    Get nibble of pixel data in a spu packet
//
// Arguments:
//    spu - Pointer to a struct contains Spu infomation
//
// Return Value:
//    Nibble
//
/////////////////////////////////////////////////////////////////////////////
static inline unsigned char mp_subtitle_vobsub_spu_get_nibble(mp_vob_sub_spu *spu)
{
    unsigned char                    nib = 0;
    unsigned long                    *nibblep = NULL;


    nibblep = spu->current_nibble + spu->deinterlace_oddness;
    if (*nibblep / 2 >= spu->control_start)
    {
        MP_SUB_DEBUG("mp_subtitle_vobsub_spu_get_nibble: ERROR: get_nibble past end of packet\n");
        return 0;
    }

    nib = spu->packet[*nibblep / 2];
    if (*nibblep % 2)
    {
        nib &= 0xf;
    }
    else
    {
        nib >>= 4;
    }

    ++*nibblep;
    return nib;
}

/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_vobsub_spu_parse_dcqt
//
// Description:
//    Parse DCQT in a spu packet
//
// Arguments:
//    spu - Pointer to a struct contains Spu infomation
//
// Return Value:
//    1 - Success
//    -1 - Failed
//
/////////////////////////////////////////////////////////////////////////////
static long mp_subtitle_vobsub_spu_parse_dcqt(mp_vob_sub_spu *spu)
{
    unsigned long                        x = 0;
    unsigned long                        y = 0;
    unsigned long                       off = 4;

    MP_SUB_DEBUG("====>>mp_subtitle_vobsub_spu_parse_dcqt()\n");

    if ((!spu))
    {
        MP_SUB_DEBUG("mp_subtitle_vobsub_spu_parse_dcqt: Invalid Parameters!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_vobsub_spu_parse_dcqt()\n");
        return -1;
    }

    x = 0;
    while(x+4<=spu->packet_offset)
    {
         //Get next control pointer
        y=mp_subtitle_vobsub_spu_get_be16(spu->packet+x+2);
        MP_SUB_DEBUG("mp_subtitle_vobsub_spu_parse_dcqt: x=%d y=%d off=%d size=%d\n",x,y,spu->packet_offset,spu->packet_size);
        if((x>=off) && (x==y))
        {
            // we got it!
            MP_SUB_DEBUG("mp_subtitle_vobsub_spu_parse_dcqt: off=%d  size=%d \n",spu->packet_offset,spu->packet_size);
            mp_subtitle_vobsub_spu_parse_dccmd(spu);
            spu->packet_offset = 0;
            break;
        }

        if((y<=x) || (y>=spu->packet_size))
        {
            //invalid packet
            MP_SUB_DEBUG("mp_subtitle_vobsub_spu_parse_dcqt: broken packet!!!!! y=%d < x=%d\n",y,x);
            spu->packet_size = 0;
            spu->packet_offset = 0;
            break;
        }
        x=y;
    }
    //packet is done; start new packet
    spu->packet_offset = 0;
    spu->deinterlace_oddness = 0;

    MP_SUB_DEBUG("<<====mp_subtitle_vobsub_spu_parse_dcqt()\n");
    return 1;
}
/////////////////////////////////////////////////////////////////////////////
// mp_subtitle_vobsub_spu_parse_dccmd
//
// Description:
//    Parse DC_Command in a spu packet
//
// Arguments:
//    spu - Pointer to a struct contains Spu infomation
//
// Return Value:
//    1 - Success
//    -1 - Failed
//
/////////////////////////////////////////////////////////////////////////////
static long mp_subtitle_vobsub_spu_parse_dccmd(mp_vob_sub_spu *spu)
{
    long                    a = 0;
    long                    b = 0;
    unsigned long            date = 0;
    unsigned long           type = 0;
    unsigned long            off = 0;
    unsigned long            start_off = 0;
    unsigned long            next_off = 0;
    unsigned long            start_pts = 0;
    unsigned long            end_pts = 0;
    unsigned long            current_nibble[2] = {0};
    unsigned long            control_start = 0;
    unsigned long            display = 0;
    unsigned long            start_col = 0;
    unsigned long            end_col = 0;
    unsigned long            start_row = 0;
    unsigned long            end_row = 0;
    unsigned long            width = 0;
    unsigned long            height = 0;
    unsigned long            stride = 0;
    unsigned long            palette[4] = {0};
    unsigned long            alpha[4] = {0};
    unsigned char            *packet = NULL;
    unsigned long            pts = 0;
    unsigned long           flag = 0;
    long                    i = 0;

    MP_SUB_DEBUG("====>>mp_subtitle_vobsub_parse_spu_dccmd()\n");

    if ((!spu) || (!spu->packet) || (spu->packet_size <= 0))
    {
        MP_SUB_DEBUG("mp_subtitle_vobsub_parse_spu_dccmd: Invalid Parameters!\n");
        MP_SUB_DEBUG("<<====mp_subtitle_vobsub_parse_spu_dccmd()\n");
        return -1;
    }

    packet = spu->packet;
    pts = spu->pts;

    control_start = mp_subtitle_vobsub_spu_get_be16(packet + 2);
    next_off = control_start;
    while (start_off != next_off)
    {
        start_off = next_off;
        date = mp_subtitle_vobsub_spu_get_be16(packet + start_off) * 1024;
        next_off = mp_subtitle_vobsub_spu_get_be16(packet + start_off + 2);
        MP_SUB_DEBUG("mp_subtitle_vobsub_parse_spu_dccmd: date=%d\n", date);
        off = start_off + 4;
        for (type = packet[off++]; type != 0xff; type = packet[off++])
        {
            switch(type)
            {
                case 0x00:
                {
                    // Menu ID, 1 byte
                    MP_SUB_DEBUG("mp_subtitle_vobsub_parse_spu_dccmd: Menu ID\n");
                    start_pts = pts + date;
                    end_pts = 0xffffffff;
                    display = 1;
                    //this->is_forced_sub=~0; // current subtitle is forced
                    break;
                }
                case 0x01:
                {
                    // Start display
                    MP_SUB_DEBUG("mp_subtitle_vobsub_parse_spu_dccmd: Start display!\n");
                    start_pts = pts + date;
                    end_pts = 0xffffffff;
                    display = 1;
                    //this->is_forced_sub=0;
                    break;
                }
                case 0x02:
                {
                    // Stop display
                    MP_SUB_DEBUG("mp_subtitle_vobsub_parse_spu_dccmd: Stop display!\n");
                    end_pts = pts + date;
                    break;
                }
                case 0x03:
                {
                    // Palette
                    palette[0] = packet[off] >> 4;
                    palette[1] = packet[off] & 0xf;
                    palette[2] = packet[off + 1] >> 4;
                    palette[3] = packet[off + 1] & 0xf;
                    MP_SUB_DEBUG("mp_subtitle_vobsub_parse_spu_dccmd: Palette %d, %d, %d, %d\n",
                    palette[0], palette[1], palette[2], palette[3]);
                    off+=2;
                    break;
                }
                case 0x04:
                {
                    // Alpha
                    alpha[0] = packet[off] >> 4;
                    alpha[1] = packet[off] & 0xf;
                    alpha[2] = packet[off + 1] >> 4;
                    alpha[3] = packet[off + 1] & 0xf;
                    MP_SUB_DEBUG("mp_subtitle_vobsub_parse_spu_dccmd: Alpha %d, %d, %d, %d\n",
                    alpha[0], alpha[1], alpha[2], alpha[3]);
                    off+=2;
                    break;
                }
                case 0x05:
                {
                    // Co-ords
                    a = mp_subtitle_vobsub_spu_get_be24(packet + off);
                    b = mp_subtitle_vobsub_spu_get_be24(packet + off + 3);
                    start_col = a >> 12;
                    end_col = a & 0xfff;
                    width = (end_col < start_col) ? 0 : end_col - start_col + 1;
                    stride = (width + 7) & ~7;
                    start_row = b >> 12;
                    end_row = b & 0xfff;
                    height = (end_row < start_row) ? 0 : end_row - start_row; // + 1
                    MP_SUB_DEBUG("mp_subtitle_vobsub_parse_spu_dccmd: Coords  col: %d - %d  row: %d - %d  (%dx%d)\n",
                    start_col, end_col, start_row, end_row,
                    width, height);
                    off+=6;
                    break;
                }
                case 0x06:
                {
                    // Graphic lines
                    current_nibble[0] = 2 * mp_subtitle_vobsub_spu_get_be16(packet + off);
                    current_nibble[1] = 2 * mp_subtitle_vobsub_spu_get_be16(packet + off + 2);
                    MP_SUB_DEBUG("mp_subtitle_vobsub_parse_spu_dccmd: Graphic offset 1: %d  offset 2: %d\n",
                    current_nibble[0] / 2, current_nibble[1] / 2);
                    off+=4;
                    break;
                }
                default:
                {
                    MP_SUB_DEBUG("mp_subtitle_vobsub_parse_spu_dccmd: Error determining control type 0x%02x.  Skipping %d bytes.\n",
                    type, next_off - off);
                    flag = 1;
                    break;
                }
            }
            if(flag > 0)
            {
                break;
            }
        }


        if (display)
        {
            spu->start_pts = start_pts;
            if ((0xffffffff == end_pts) && (start_off != next_off))
            {
                end_pts = pts + mp_subtitle_vobsub_spu_get_be16(packet + next_off) * 1024;
                spu->end_pts = end_pts;
            }
            else
            {
                spu->end_pts = end_pts;
            }

            spu->current_nibble[0] = current_nibble[0];
            spu->current_nibble[1] = current_nibble[1];
            spu->start_row = start_row;
            spu->end_row = end_row;
            spu->start_col = start_col;
            spu->end_col = end_col;
            spu->width = width;
            spu->height = height;
            spu->stride = stride;
            spu->control_start = control_start;
            for (i=0; i<4; i++)
            {
                spu->alpha[i] = alpha[i];
                spu->palette[i] = palette[i];
            }
        }
    }

    return RET_SUCCESS;
}


