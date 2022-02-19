/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: snd.h
*
*    Description: This file define the struct used by decoder libray,
*    such as mp3 aac ogg.etc
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/


#ifndef _SOFT_COMMON_H_
#define _SOFT_COMMON_H_
#ifdef __cplusplus
extern "C"
{
#endif
typedef int bool;

typedef enum
{
    c_audio_no_err,                 /* -> always first entry */
                                /* remaining entry order is not guaranteed */
    c_audio_failed,
    c_audio_bad_argument,
    c_audio_bad_bye3_header,
    c_audio_bad_packet_header,
    c_audio_broken_frame,
    c_audio_no_more_frames,
    c_audio_bad_sampling_rate,
    c_audio_bad_number_of_channels,
    c_audio_bad_version_number,
    c_audio_bad_weighting_mode,
    c_audio_bad_packetization,

    c_audio_bad_drmtype,
    c_audio_drmfailed,
    c_audio_drmunsupported,

    c_audio_demo_expired,

    c_audio_bad_state,
    c_audio_internal,               /* really bad */
    c_audio_no_more_data_this_time,
    c_audio_no_more_buffer
} t_audio_dec_status;

typedef void* decoder_instance;
typedef struct
{
    const void *pv_encoded_data; // pointer to the encoded MP3 data stream.
    unsigned long ui_total_encoded_bytes; // total number of encoded MP3 data in byte.
    unsigned long ui_packet_index; //Packet index, start from 0, the MAX value is (total_packet_num - 1)

} soft_decoder_in;

typedef struct
{
    void *pv_decoded_data; /*pointer to decoded PCM data stream.*/
    /*PCM data buffer length in byte. It is used to put the decoded data.
    Out put format should be :
    1. Each sample presented as a signed-16-bits-integer;
    2. Byte order is little endian;
    3. PCM Sample order is LRLR...*/
    unsigned long ui_max_decoded_bytes;
    unsigned long ui_bytes_written; /*real length copied decoded PCM data to PCM data buffer, in bytes.*/
    /*used number of encoded MP3 data stream. User should move the MP3
    encoded data pointer according this value. This value tell the user how
    many encoded data the decoder used. If this value is zero, means decoder
    did not use any encoded data, that's because the decoder will decode the
    encoded data frame by frame, if the encoded data is less than one frame,
    the decoder will not decode it, it will wait the user send more data.  */
    unsigned long ui_used_encoded_data;
    int b_more_output_available; //flag of PCM data transfer end.
} soft_decoder_out;

typedef enum tag_bits_per_sample
{
    c_audio_bps_8 = 8,
    c_audio_bps_16 = 16

} t_bits_per_sample;

/* sample data Signedness */
typedef enum tag_sample_signedness
{
    c_audio_unsigned,
    c_audio_signed

} t_sample_signess;

/* sample data endianness */
typedef enum tag_sample_endianness
{
    c_audio_big_endian,
    c_audio_little_endian

} t_sample_endian;

/* sample data order */
typedef enum tag_sample_data_order
{
    c_audio_lrlr,
    c_audio_rlrl

} t_sample_data_order;

typedef struct tag_track_name
{
    bool b_present;
    unsigned char* pui_track_name; // Only valid if bPresent is true
} t_track_name;

typedef struct tag_track_duration
{
    bool b_present;
    UINT32 ui_track_duration; // of the file in milliseconds, Only valid if bPresent is true
} t_track_duration;

typedef struct tag_artist
{
    bool b_present;
    UINT32 ui_artist; // Only valid if bPresent is true
} t_artist;

typedef struct tag_album
{
    bool b_present;
    UINT32 ui_artist; // Only valid if bPresent is true
} t_album;

typedef struct tag_original_track_number
{
    bool b_present;
    UINT32 ui_original_track_number; // Only valid if bPresent is true
} t_original_track_number;

typedef struct tag_date
{
    bool b_present;
    UINT32 ui_date; // Only valid if bPresent is true
} t_date;

typedef struct tag_total_file_size
{
    bool b_present;
    UINT32 ui_totalfilesize; // Only valid if bPresent is true
} t_total_file_size;

/* audio info */
#define MAX_TITLE_NUM 256
#define MAX_AUTHOR_NUM 256

typedef struct tag_file_hdr_info
{
    unsigned char p_title[MAX_TITLE_NUM];      /* Title of the file */
    unsigned char p_author[MAX_AUTHOR_NUM];     /* Author of the file */

    t_track_name track_name;
    t_track_duration track_duration;
    t_artist artist;
    t_album album;
    t_original_track_number original_track_num;
    t_date date;
    t_total_file_size total_file_size;
    UINT32 first_packet_offset; // the offset of the first packet after header.
    UINT32 packet_size; //packet size for each packet.
    UINT32 total_packet_num; //total packet number in the file.

    bool is_vbr;
    unsigned long bitrate;     /* bit-rate of the bitstream */
    UINT32 sample_rate;  /* sampling rate in HZ*/
    UINT32 num_channels;   /* number of audio channels */
    t_bits_per_sample bitspersample;  /* bits per sample */
    t_sample_signess sample_signed; /* are samples signed or unsigned quantities? */
    t_sample_endian sample_endina; /* are samples little- or big-endian? */

    /*how is multi-channel data is emitted, e.g. is stereo output LRLRLR or RLRLRL?*/
    t_sample_data_order sample_data_order;
} t_file_hdr_info;

struct soft_decoder_callback
{
    t_audio_dec_status (*initialise)(decoder_instance *);
    t_audio_dec_status (*decode)(decoder_instance, soft_decoder_in *, soft_decoder_out *);
    void (*seektime)(decoder_instance *, long , unsigned char);
    t_audio_dec_status (*finalise)(decoder_instance *);
};

struct soft_decoder_callback2
{
    t_audio_dec_status (*initialise)(decoder_instance *);
    t_audio_dec_status (*decode)(decoder_instance, soft_decoder_in *, soft_decoder_out *);
    t_audio_dec_status (*seektime)(decoder_instance, unsigned long);
    t_audio_dec_status (*finalise)(decoder_instance *);
    bool (*candecode)(const void *,UINT32 , UINT32 *);
    t_audio_dec_status (*discontinuity)(decoder_instance);
    t_audio_dec_status (*sync)(char *, int *);
    bool (*header)(decoder_instance , const void *, UINT32 ,\
                                  t_file_hdr_info *);
    bool (*setdeca_buf)(char *,unsigned long);
};
#ifdef __cplusplus
 }
#endif
#endif
