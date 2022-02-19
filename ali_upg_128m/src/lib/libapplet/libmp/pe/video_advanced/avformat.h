/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2002 Copyright (C)
 *
 *  File: avformat.h
 *
 *  Description:
 *  History:
 *      Date             Author         Version      Comment
 *      ======           ======          =====       =======
 *  1.  2010.01.23       Summer xia      0.1.000     First version Created
 ****************************************************************************/

#ifndef __AVFORMAT_H
#define __AVFORMAT_H


#ifdef __cplusplus
extern "C" {
#endif

#include "t2mpgdec.h"
#include "plugin.h"
#include "plugin_subt.h"

#ifdef WIN32
#include "../../inc/t2typedef.h"
#else
#include <basic_types.h>
#include <mediatypes.h>
#endif
#include <hld/decv/decv_media_player.h>
#include <api/libmp/es_player_api.h>
#ifdef WIN32
#define INLINE __inline
#else
#define INLINE inline
#endif

//#define AV_NOPTS_VALUE          -1//INT64_C(0x8000000000000000)
#define INT_MIN (-2147483647-1)
#define AV_TIME_BASE            1000000

#define AVERROR_UNKNOWN     (-22)   /**< unknown error */
#define AVERROR_IO          (-5)    /**< I/O error */
#define AVERROR_NUMEXPECTED (-33)   /**< Number syntax expected in filename. */
#define AVERROR_INVALIDDATA (-22)   /**< invalid data found */
#define AVERROR_NOMEM       (-12)   /**< not enough memory */
#define AVERROR_NOFMT       (-42)   /**< unknown format */
#define AVERROR_NOTSUPP     (-40)   /**< Operation not supported. */
#define AVERROR_NOENT       (-2)    /**< No such file or directory. */
#define AVERROR_EOF         (-32)   /**< End of file. */
#define AVERROR_SPECIAL_CASE        (-100)

#define PKT_FLAG_KEY 0x0001

#define AV_DISPOSITION_DEFAULT   0x0001
#define AV_DISPOSITION_DUB       0x0002
#define AV_DISPOSITION_ORIGINAL  0x0004
#define AV_DISPOSITION_COMMENT   0x0008
#define AV_DISPOSITION_LYRICS    0x0010
#define AV_DISPOSITION_KARAOKE   0x0020

#define AV_METADATA_MATCH_CASE      1
#define AV_METADATA_IGNORE_SUFFIX   2
#define AV_METADATA_DONT_STRDUP_KEY 4
#define AV_METADATA_DONT_STRDUP_VAL 8


#define MKTAG(a,b,c,d) (a | (b << 8) | (c << 16) | (d << 24))
#define MKBETAG(a,b,c,d) (d | (c << 8) | (b << 16) | (a << 24))

#define AV_RB8(x)     (((const UINT8*)(x))[0])
#define AV_WB8(p, d)  do { ((UINT8*)(p))[0] = (d); } while(0)

#define AV_RL8(x)     AV_RB8(x)
#define AV_WL8(p, d)  AV_WB8(p, d)

#ifndef AV_RB16
#   define AV_RB16(x)                           \
    ((((const UINT8*)(x))[0] << 8) |          \
      ((const UINT8*)(x))[1])
#endif
#ifndef AV_WB16
#   define AV_WB16(p, d) do {                   \
        ((UINT8*)(p))[1] = (d);               \
        ((UINT8*)(p))[0] = (d)>>8;            \
    } while(0)
#endif

#ifndef AV_RL16
#   define AV_RL16(x)                           \
    ((((const UINT8*)(x))[1] << 8) |          \
      ((const UINT8*)(x))[0])
#endif
#ifndef AV_WL16
#   define AV_WL16(p, d) do {                   \
        ((UINT8*)(p))[0] = (d);               \
        ((UINT8*)(p))[1] = (d)>>8;            \
    } while(0)
#endif

#ifndef AV_RB32
#   define AV_RB32(x)                           \
    ((((const UINT8*)(x))[0] << 24) |         \
     (((const UINT8*)(x))[1] << 16) |         \
     (((const UINT8*)(x))[2] <<  8) |         \
      ((const UINT8*)(x))[3])
#endif
#ifndef AV_WB32
#   define AV_WB32(p, d) do {                   \
        ((UINT8*)(p))[3] = (d);               \
        ((UINT8*)(p))[2] = (d)>>8;            \
        ((UINT8*)(p))[1] = (d)>>16;           \
        ((UINT8*)(p))[0] = (d)>>24;           \
    } while(0)
#endif

#ifndef AV_RL32
#   define AV_RL32(x)                           \
    ((((const UINT8*)(x))[3] << 24) |         \
     (((const UINT8*)(x))[2] << 16) |         \
     (((const UINT8*)(x))[1] <<  8) |         \
      ((const UINT8*)(x))[0])
#endif
#ifndef AV_WL32
#   define AV_WL32(p, d) do {                   \
        ((UINT8*)(p))[0] = (d);               \
        ((UINT8*)(p))[1] = (d)>>8;            \
        ((UINT8*)(p))[2] = (d)>>16;           \
        ((UINT8*)(p))[3] = (d)>>24;           \
    } while(0)
#endif

#define FF_ARRAY_ELEMS(a) (sizeof(a) / sizeof((a)[0]))

enum avstream_parse_type
{
    AVSTREAM_PARSE_NONE,
    AVSTREAM_PARSE_FULL,       /**< full parsing and repack */
    AVSTREAM_PARSE_HEADERS,    /**< Only parse headers, do not repack. */
    AVSTREAM_PARSE_TIMESTAMPS,
    /**< full parsing and interpolation of
    //timestamps for frames not starting on a packet boundary */
    AVSTREAM_PARSE_FULL_ONCE,  /**< full parsing and repack of the first frame only, only implemented for H.264 currently */
    AVSTREAM_PARSE_FULL_RAW=MKTAG(0,'R','A','W'),       /**< full parsing and repack with timestamp and position generation by parser for raw
                                                             this assumes that each packet in the file contains no demuxer level headers and
                                                             just codec level data, otherwise position generation would fail */
};

/**
 * Identifies the syntax and semantics of the bitstream.
 * The principle is roughly:
 * Two decoders with the same ID can decode the same streams.
 * Two encoders with the same ID can encode compatible streams.
 * There may be slight deviations from the principle due to implementation
 * details.
 *
 * If you add a codec ID to this list, add it so that
 * 1. no value of a existing codec ID changes (that would break ABI),
 * 2. it is as close as possible to similar codecs.
 */
enum codec_id
{
    CODEC_ID_NONE,

    /* video codecs */
    CODEC_ID_MPEG1VIDEO,
    CODEC_ID_MPEG2VIDEO, ///< preferred ID for MPEG-1/2 video decoding
    CODEC_ID_MPEG2VIDEO_XVMC,
    CODEC_ID_H261,
    CODEC_ID_H263,
    CODEC_ID_RV10,
    CODEC_ID_RV20,
    CODEC_ID_MJPEG,
    CODEC_ID_MJPEGB,
    CODEC_ID_LJPEG,
    CODEC_ID_SP5X,
    CODEC_ID_JPEGLS,
    CODEC_ID_MPEG4,
    CODEC_ID_RAWVIDEO,
    CODEC_ID_MSMPEG4V1,
    CODEC_ID_MSMPEG4V2,
    CODEC_ID_MSMPEG4V3,
    CODEC_ID_BYE21,
    CODEC_ID_BYE22,
    CODEC_ID_H263P,
    CODEC_ID_H263I,
    CODEC_ID_FLV1,
    CODEC_ID_SVQ1,
    CODEC_ID_SVQ3,
    CODEC_ID_DVVIDEO,
    CODEC_ID_HUFFYUV,
    CODEC_ID_CYUV,
    CODEC_ID_H264,
    CODEC_ID_INDEO3,
    CODEC_ID_VP3,
    CODEC_ID_THEORA,
    CODEC_ID_ASV1,
    CODEC_ID_ASV2,
    CODEC_ID_FFV1,
    CODEC_ID_4XM,
    CODEC_ID_VCR1,
    CODEC_ID_CLJR,
    CODEC_ID_MDEC,
    CODEC_ID_ROQ,
    CODEC_ID_INTERPLAY_VIDEO,
    CODEC_ID_XAN_WC3,
    CODEC_ID_XAN_WC4,
    CODEC_ID_RPZA,
    CODEC_ID_CINEPAK,
    CODEC_ID_WS_VQA,
    CODEC_ID_MSRLE,
    CODEC_ID_MSVIDEO1,
    CODEC_ID_IDCIN,
    CODEC_ID_8BPS,
    CODEC_ID_SMC,
    CODEC_ID_FLIC,
    CODEC_ID_TRUEMOTION1,
    CODEC_ID_VMDVIDEO,
    CODEC_ID_MSZH,
    CODEC_ID_ZLIB,
    CODEC_ID_QTRLE,
    CODEC_ID_SNOW,
    CODEC_ID_TSCC,
    CODEC_ID_ULTI,
    CODEC_ID_QDRAW,
    CODEC_ID_VIXL,
    CODEC_ID_QPEG,
    CODEC_ID_XVID,
    CODEC_ID_PNG,
    CODEC_ID_PPM,
    CODEC_ID_PBM,
    CODEC_ID_PGM,
    CODEC_ID_PGMYUV,
    CODEC_ID_PAM,
    CODEC_ID_FFVHUFF,
    CODEC_ID_RV30,
    CODEC_ID_RV40,
    CODEC_ID_VC1,
    CODEC_ID_BYE23,
    CODEC_ID_LOCO,
    CODEC_ID_WNV1,
    CODEC_ID_AASC,
    CODEC_ID_INDEO2,
    CODEC_ID_FRAPS,
    CODEC_ID_TRUEMOTION2,
    CODEC_ID_BMP,
    CODEC_ID_CSCD,
    CODEC_ID_MMVIDEO,
    CODEC_ID_ZMBV,
    CODEC_ID_AVS,
    CODEC_ID_SMACKVIDEO,
    CODEC_ID_NUV,
    CODEC_ID_KMVC,
    CODEC_ID_FLASHSV,
    CODEC_ID_CAVS,
    CODEC_ID_JPEG2000,
    CODEC_ID_VMNC,
    CODEC_ID_VP5,
    CODEC_ID_VP6,
    CODEC_ID_VP6F,
    CODEC_ID_TARGA,
    CODEC_ID_DSICINVIDEO,
    CODEC_ID_TIERTEXSEQVIDEO,
    CODEC_ID_TIFF,
    CODEC_ID_GIF,
    CODEC_ID_FFH264,
    CODEC_ID_DXA,
    CODEC_ID_DNXHD,
    CODEC_ID_THP,
    CODEC_ID_SGI,
    CODEC_ID_C93,
    CODEC_ID_BETHSOFTVID,
    CODEC_ID_PTX,
    CODEC_ID_TXD,
    CODEC_ID_VP6A,
    CODEC_ID_AMV,
    CODEC_ID_VB,
    CODEC_ID_PCX,
    CODEC_ID_SUNRAST,
    CODEC_ID_INDEO4,
    CODEC_ID_INDEO5,
    CODEC_ID_MIMIC,
    CODEC_ID_RL2,
    CODEC_ID_8SVX_EXP,
    CODEC_ID_8SVX_FIB,
    CODEC_ID_ESCAPE124,
    CODEC_ID_DIRAC,
    CODEC_ID_BFI,
    CODEC_ID_CMV,
    CODEC_ID_MOTIONPIXELS,
    CODEC_ID_TGV,
    CODEC_ID_TGQ,
    CODEC_ID_TQI,
    CODEC_ID_AURA,
    CODEC_ID_AURA2,
    CODEC_ID_V210X,
    CODEC_ID_TMV,
    CODEC_ID_V210,
    CODEC_ID_DPX,
    CODEC_ID_MAD,
    CODEC_ID_FRWU,
    CODEC_ID_FLASHSV2,
    CODEC_ID_CDGRAPHICS,
    CODEC_ID_R210,

    CODEC_ID_ANM,
    CODEC_ID_BINKVIDEO,
    CODEC_ID_IFF_ILBM,
    CODEC_ID_IFF_BYTERUN1,
    CODEC_ID_KGV1,
    CODEC_ID_YOP,
    CODEC_ID_VP8,
    CODEC_ID_HEVC = MKBETAG('H','2','6','5'),

    /* various PCM "codecs" */
    CODEC_ID_PCM_S16LE= 0x10000,
    CODEC_ID_PCM_S16BE,
    CODEC_ID_PCM_U16LE,
    CODEC_ID_PCM_U16BE,
    CODEC_ID_PCM_S8,
    CODEC_ID_PCM_U8,
    CODEC_ID_PCM_MULAW,
    CODEC_ID_PCM_ALAW,
    CODEC_ID_PCM_S32LE,
    CODEC_ID_PCM_S32BE,
    CODEC_ID_PCM_U32LE,
    CODEC_ID_PCM_U32BE,
    CODEC_ID_PCM_S24LE,
    CODEC_ID_PCM_S24BE,
    CODEC_ID_PCM_U24LE,
    CODEC_ID_PCM_U24BE,
    CODEC_ID_PCM_S24DAUD,
    CODEC_ID_PCM_ZORK,
    CODEC_ID_PCM_S16LE_PLANAR,
    CODEC_ID_PCM_DVD,
    CODEC_ID_PCM_F32BE,
    CODEC_ID_PCM_F32LE,
    CODEC_ID_PCM_F64BE,
    CODEC_ID_PCM_F64LE,
    CODEC_ID_PCM_BLURAY,

    /* various ADPCM codecs */
    CODEC_ID_ADPCM_IMA_QT= 0x11000,
    CODEC_ID_ADPCM_IMA_WAV,
    CODEC_ID_ADPCM_IMA_DK3,
    CODEC_ID_ADPCM_IMA_DK4,
    CODEC_ID_ADPCM_IMA_WS,
    CODEC_ID_ADPCM_IMA_SMJPEG,
    CODEC_ID_ADPCM_MS,
    CODEC_ID_ADPCM_4XM,
    CODEC_ID_ADPCM_XA,
    CODEC_ID_ADPCM_ADX,
    CODEC_ID_ADPCM_EA,
    CODEC_ID_ADPCM_G726,
    CODEC_ID_ADPCM_CT,
    CODEC_ID_ADPCM_SWF,
    CODEC_ID_ADPCM_YAMAHA,
    CODEC_ID_ADPCM_SBPRO_4,
    CODEC_ID_ADPCM_SBPRO_3,
    CODEC_ID_ADPCM_SBPRO_2,
    CODEC_ID_ADPCM_THP,
    CODEC_ID_ADPCM_IMA_AMV,
    CODEC_ID_ADPCM_EA_R1,
    CODEC_ID_ADPCM_EA_R3,
    CODEC_ID_ADPCM_EA_R2,
    CODEC_ID_ADPCM_IMA_EA_SEAD,
    CODEC_ID_ADPCM_IMA_EA_EACS,
    CODEC_ID_ADPCM_EA_XAS,
    CODEC_ID_ADPCM_EA_MAXIS_XA,
    CODEC_ID_ADPCM_IMA_ISS,

    /* AMR */
    CODEC_ID_AMR_NB= 0x12000,
    CODEC_ID_AMR_WB,

    /* RealAudio codecs*/
    CODEC_ID_RA_144= 0x13000,
    CODEC_ID_RA_288,

    /* various DPCM codecs */
    CODEC_ID_ROQ_DPCM= 0x14000,
    CODEC_ID_INTERPLAY_DPCM,
    CODEC_ID_XAN_DPCM,
    CODEC_ID_SOL_DPCM,

    /* audio codecs */
    CODEC_ID_MP2= 0x15000,
    CODEC_ID_MP3, ///< preferred ID for decoding MPEG audio layer 1, 2 or 3
    CODEC_ID_AAC,
    CODEC_ID_AC3,
    CODEC_ID_DTS,
    CODEC_ID_AAC_ADTS,
    CODEC_ID_PCM,
    CODEC_ID_VORBIS,
    CODEC_ID_DVAUDIO,
    CODEC_ID_BYE1V1,
    CODEC_ID_BYE1V2,
    CODEC_ID_MACE3,
    CODEC_ID_MACE6,
    CODEC_ID_VMDAUDIO,
    CODEC_ID_SONIC,
    CODEC_ID_SONIC_LS,
    CODEC_ID_FLAC,
    CODEC_ID_MP3ADU,
    CODEC_ID_MP3ON4,
    CODEC_ID_SHORTEN,
    CODEC_ID_ALAC,
    CODEC_ID_WESTWOOD_SND1,
    CODEC_ID_GSM, ///< as in Berlin toast format
    CODEC_ID_QDM2,
    CODEC_ID_COOK,
    CODEC_ID_TRUESPEECH,
    CODEC_ID_TTA,
    CODEC_ID_SMACKAUDIO,
    CODEC_ID_QCELP,
    CODEC_ID_WAVPACK,
    CODEC_ID_DSICINAUDIO,
    CODEC_ID_IMC,
    CODEC_ID_MUSEPACK7,
    CODEC_ID_MLP,
    CODEC_ID_GSM_MS, /* as found in WAV */
    CODEC_ID_ATRAC3,
    CODEC_ID_VOXWARE,
    CODEC_ID_APE,
    CODEC_ID_NELLYMOSER,
    CODEC_ID_MUSEPACK8,
    CODEC_ID_SPEEX,
    CODEC_ID_BYE1VOICE,
    CODEC_ID_BYE1PRO,
    CODEC_ID_BYE1LOSSLESS,
    CODEC_ID_ATRAC3P,
    CODEC_ID_EAC3,
    CODEC_ID_SIPR,
    CODEC_ID_MP1,
    CODEC_ID_TWINVQ,
    CODEC_ID_TRUEHD,
    CODEC_ID_MP4ALS,
    CODEC_ID_ATRAC1,
    CODEC_ID_AAC_LATM,
    CODEC_ID_DTSHD,
    CODEC_ID_OGG = MKBETAG( 0 ,'O','G','G'),

    /* subtitle codecs */
    CODEC_ID_DVD_SUBTITLE= 0x17000,
    CODEC_ID_DVB_SUBTITLE,
    CODEC_ID_TEXT,  ///< raw UTF-8 text
    CODEC_ID_XSUB,
    CODEC_ID_SSA,
    CODEC_ID_MOV_TEXT,
    CODEC_ID_HDMV_PGS_SUBTITLE,
    CODEC_ID_DVB_TELETEXT,

    /* other specific kind of codecs (generally used for attachments) */
    CODEC_ID_TTF= 0x18000,

    CODEC_ID_PROBE= 0x19000, ///< codec_id is not known (like CODEC_ID_NONE) but lavf should attempt to identify it

    CODEC_ID_MPEG2TS= 0x20000, /**< _FAKE_ codec to indicate a raw MPEG-2 TS stream (only used by libavformat) */
};

typedef struct id_to_tags
{
    UINT32 tag;
    enum codec_id id;
}id_to_tags;

enum avdiscard
{
    /* We leave some space between them for extensions (drop some
     * keyframes for intra-only or drop just some bidir frames). */
    AVDISCARD_NONE   =-16, ///< discard nothing
    AVDISCARD_DEFAULT=  0, ///< discard useless packets like 0 size packets in avi
    AVDISCARD_NONREF =  8, ///< discard all non reference
    AVDISCARD_BIDIR  = 16, ///< discard all bidirectional frames
    AVDISCARD_NONKEY = 32, ///< discard all frames except keyframes
    AVDISCARD_ALL    = 48, ///< discard all
};

#define AVSEEK_FLAG_FORWARD  0 ///< seek forward
#define AVSEEK_FLAG_BACKWARD 1 ///< seek backward
#define AVSEEK_FLAG_BYTE     2 ///< seeking based on position in bytes
#define AVSEEK_FLAG_ANY      4 ///< seek to any frame, even non-keyframes
#define AVSEEK_FLAG_FRAME    8 ///< seeking based on frame number
#define AVSEEK_FLAG_MMS      16

#define AV_INDEX_KEYFRAME 0x0001

#define bit_clear(var, bit)         ((var) &= ~(bit))
#define bit_set(var, bit)           ((var) |= (bit))
#define bit_test_any(var, bit)      ((var) & (bit))
#define bit_test_all(var, bit)      (((var) & (bit)) == (bit))
#define bit_test_eq(var, bit)       ((var) == (bit))

#define AVPLAY_MODE_NONE            (0)
#define AVPLAY_MODE_ALL             (0xFFFFFFFF)
#define AVPLAY_MODE_VIDEO           (1<<0)
#define AVPLAY_MODE_AUDIO           (1<<1)
#define AVPLAY_MODE_RESUME_STOP     (1<<2)
#define AVPLAY_MODE_STOP            (1<<3)
#define AVPLAY_MODE_PAUSE           (1<<4)
#define AVPLAY_MODE_STEP            (1<<5)
#define AVPLAY_MODE_FF              (1<<6)
#define AVPLAY_MODE_FB              (1<<7)
#define AVPLAY_MODE_SUBTT           (1<<8)
#define AVPLAY_MODE_CHANGE_TITTLE   (1<<9)
#define AVPLAY_MODE_SLOW_FORWARD    (1<<10)
#define AVPLAY_MODE_BUILD_INDEX     (1<<11)

enum avinitial_status
{
    AVINIT_STATUS_NONE    = 0,  // initialize not finished
    AVINIT_STATUS_SUCCESS = 1,  // video support or audio support
    AVINIT_STATUS_CODEC_UNSUPPORT = 2,// video and audio codec not support
    AVINIT_STATUS_ERROR = 3, // initialize error
    AVINIT_STATUS_SOLUTION_UNSUPPORT = 4, // solution not support, for example, M3101 not support HD
    AVINIT_STATUS_NOFMT = 5  // not support this file format
};

typedef struct _little_avindex_entry
{
    INT64 pos:36;
    INT64 timestamp:28;
    //UINT32 pos;
    //INT32 timestamp;
 //Yeah, trying to keep the size of this small to
 //reduce memory requirements (it is 24 vs. 32 bytes
 //due to possible 8-byte alignment).
    INT32 size:30;
    INT32 flags:2;
    //*< Minimum distance between this and the
    //previous keyframe, used to avoid unneeded searching. */
//    INT32 min_distance;
} __attribute__ ((packed))little_avindex_entry;

typedef struct _avindex_entry
{
    INT64 pos;
    INT64 timestamp;
    //Yeah, trying to keep the size of this small to
    //reduce memory requirements (it is 24 vs. 32 bytes
    //due to possible 8-byte alignment).
    INT32 size:30;
    INT32 flags:2;
    //*< Minimum distance between this and the
    //previous keyframe, used to avoid unneeded searching. */
//    INT32 min_distance;
} __attribute__ ((packed))avindex_entry;

enum codec_type
{
    CODEC_TYPE_UNKNOWN = -1,
    CODEC_TYPE_VIDEO,
    CODEC_TYPE_AUDIO,
    CODEC_TYPE_SUBTITLE,
    CODEC_TYPE_DATA,
    CODEC_TYPE_ATTACHMENT,
    CODEC_TYPE_NB
};

typedef struct
{
    const UINT8 *buf;
    const UINT8 *buf_end;
    const UINT8 *buf_ptr;
    INT32   size_in_bits;
    UINT32  bitptr;
    UINT32  data;
} BSTRM;

typedef struct _avcodec_tag
{
    enum codec_id id;
    UINT32 tag;
} avcodec_tag;

typedef struct
{
    CHAR *key;
    CHAR *value;
}metadata_tag;

typedef struct
{
    INT32 count;
    metadata_tag *elems;
}metadata;

typedef struct
{
    const char *native;
    const char *generic;
}metadata_conv;

typedef struct
{
    UINT32 type;
    INT64 offset;
    INT64 size; /* total size (excluding the size and type fields) */
} atom_t;

/**
 * rational number numerator/denominator
 */
typedef struct _rational
{
    int num; ///< numerator
    int den; ///< denominator
} rational;

/**
 * New fields can be added to the end with minor version bumps.
 * Removal, reordering and changes to existing fields require a major
 * version bump.
 * sizeof(AVProgram) must not be used outside libav*.
 */
typedef struct _avprogram {
    int            id;
    int            flags;
    enum avdiscard discard;        ///< selects which program to discard and which to feed to the caller
    unsigned int   *stream_index;
    unsigned int   nb_stream_indexes;
    metadata *metadata;

    int program_num;
    int pmt_pid;
    int pcr_pid;

    /*****************************************************************
     * All fields below this line are not part of the public API. They
     * may not be used outside of libavformat and can be changed and
     * removed at will.
     * New public fields should be added right above.
     *****************************************************************
     */
    int64_t start_time;
    int64_t end_time;
    int64_t duration;
} avprogram;

typedef struct _avchapter
{
    uint64_t id;                 ///< unique ID to identify the chapter
    rational time_base;   ///< time base in which the start/end timestamps are specified
    int64_t  start, end;     ///< chapter start/end time in time_base units
    BOOL     hidden;
    metadata *metadata;
} avchapter;

typedef struct _avplay_chapter
{
    UINT32   begin;         ///< chapter begin time in ms
    UINT32   end;           ///< chapter end time in ms
    UINT32   duaration;     ///< chapter duation in ms
    UINT32   play_begin;    ///< actually play begin time in ms
    UINT32   play_end;      ///< actually play end time in ms
} avplay_chapter;

typedef struct _avedition
{
    uint64_t id;
    unsigned int nb_chapters;
    avchapter **chapters;

    unsigned int i_cur_play_chapter;
    avplay_chapter *p_cur_play_chapter;
    unsigned int nb_play_chapters;
    avplay_chapter **play_chapters;
    UINT32 total_time;
} avedition;

enum avpacket_side_data_type {
    AV_PKT_DATA_PALETTE,
    AV_PKT_DATA_NEW_EXTRADATA,

    /**
     * An AV_PKT_DATA_PARAM_CHANGE side data packet is laid out as follows:
     * @code
     * u32le param_flags
     * if (param_flags & AV_SIDE_DATA_PARAM_CHANGE_CHANNEL_COUNT)
     *     s32le channel_count
     * if (param_flags & AV_SIDE_DATA_PARAM_CHANGE_CHANNEL_LAYOUT)
     *     u64le channel_layout
     * if (param_flags & AV_SIDE_DATA_PARAM_CHANGE_SAMPLE_RATE)
     *     s32le sample_rate
     * if (param_flags & AV_SIDE_DATA_PARAM_CHANGE_DIMENSIONS)
     *     s32le width
     *     s32le height
     * @endcode
     */
    AV_PKT_DATA_PARAM_CHANGE,

    /**
     * An AV_PKT_DATA_H263_MB_INFO side data packet contains a number of
     * structures with info about macroblocks relevant to splitting the
     * packet into smaller packets on macroblock edges (e.g. as for RFC 2190).
     * That is, it does not necessarily contain info about all macroblocks,
     * as long as the distance between macroblocks in the info is smaller
     * than the target payload size.
     * Each MB info structure is 12 bytes, and is laid out as follows:
     * @code
     * u32le bit offset from the start of the packet
     * u8    current quantizer at the start of the macroblock
     * u8    GOB number
     * u16le macroblock address within the GOB
     * u8    horizontal MV predictor
     * u8    vertical MV predictor
     * u8    horizontal MV predictor for block number 3
     * u8    vertical MV predictor for block number 3
     * @endcode
     */
    AV_PKT_DATA_H263_MB_INFO,

    /**
     * This side data should be associated with an audio stream and contains
     * ReplayGain information in form of the AVReplayGain struct.
     */
    AV_PKT_DATA_REPLAYGAIN,

    /**
     * This side data contains a 3x3 transformation matrix describing an affine
     * transformation that needs to be applied to the decoded video frames for
     * correct presentation.
     *
     * See libavutil/display.h for a detailed description of the data.
     */
    AV_PKT_DATA_DISPLAYMATRIX,

    /**
     * This side data should be associated with a video stream and contains
     * Stereoscopic 3D information in form of the AVStereo3D struct.
     */
    AV_PKT_DATA_STEREO3D,

    /**
     * Recommmends skipping the specified number of samples
     * @code
     * u32le number of samples to skip from start of this packet
     * u32le number of samples to skip from end of this packet
     * u8    reason for start skip
     * u8    reason for end   skip (0=padding silence, 1=convergence)
     * @endcode
     */
    AV_PKT_DATA_SKIP_SAMPLES=70,

    /**
     * An AV_PKT_DATA_JP_DUALMONO side data packet indicates that
     * the packet may contain "dual mono" audio specific to Japanese DTV
     * and if it is true, recommends only the selected channel to be used.
     * @code
     * u8    selected channels (0=mail/left, 1=sub/right, 2=both)
     * @endcode
     */
    AV_PKT_DATA_JP_DUALMONO,

    /**
     * A list of zero terminated key/value strings. There is no end marker for
     * the list, so it is required to rely on the side data size to stop.
     */
    AV_PKT_DATA_STRINGS_METADATA,

    /**
     * Subtitle event position
     * @code
     * u32le x1
     * u32le y1
     * u32le x2
     * u32le y2
     * @endcode
     */
    AV_PKT_DATA_SUBTITLE_POSITION,

    /**
     * Data found in BlockAdditional element of matroska container. There is
     * no end marker for the data, so it is required to rely on the side data
     * size to recognize the end. 8 byte id (as found in BlockAddId) followed
     * by data.
     */
    AV_PKT_DATA_MATROSKA_BLOCKADDITIONAL,

    /**
     * The optional first identifier line of a WebVTT cue.
     */
    AV_PKT_DATA_WEBVTT_IDENTIFIER,

    /**
     * The optional settings (rendering instructions) that immediately
     * follow the timestamp specifier of a WebVTT cue.
     */
    AV_PKT_DATA_WEBVTT_SETTINGS,

    /**
     * A list of zero terminated key/value strings. There is no end marker for
     * the list, so it is required to rely on the side data size to stop. This
     * side data includes updated metadata which appeared in the stream.
     */
    AV_PKT_DATA_METADATA_UPDATE,
};

typedef struct avio_context
{
    unsigned char *buffer;
    int buffer_size;
    unsigned char *buf_ptr, *buf_end;
    void *opaque;
//    int (*read_packet)(void *opaque, uint8_t *buf, int buf_size);
//    int (*write_packet)(void *opaque, uint8_t *buf, int buf_size);
//    int64_t (*seek)(void *opaque, int64_t offset, int whence);

    DWORD   (*read_packet)(BYTE *buf, DWORD size);
    DWORD   (*write_packet)(BYTE *buf, DWORD size);
    BOOL    (*seek)(INT64 offset, DWORD origin);
    INT64   (*tellpos)(void);

    int64_t fsize;
    int64_t pos; /**< position in the file of the current buffer */
    int must_flush; /**< true if the next seek should flush */
    int eof_reached; /**< true if eof reached */
    int write_flag;  /**< true if open for writing */
    int is_streamed;
    int max_packet_size;
    unsigned long checksum;
    unsigned char *checksum_ptr;
    unsigned long (*update_checksum)(unsigned long checksum, const uint8_t *buf, unsigned int size);
    int error;         ///< contains the error code or 0 if no error happened
    int (*read_pause)(void *opaque, int pause);
    int64_t (*read_seek)(void *opaque, int stream_index,
                         int64_t timestamp, int flags);
	INT64  (*get_file_size)(void);
	BOOL is_network_play;
} avio_context;

typedef struct _avinfo
{
	INT32 frame_rate;
    BOOL  flv_file;
    BOOL  mp4_file;
    BOOL is_network_seektype;
    BOOL bye2_file;
    BOOL build_little_index_table;
    BOOL first_dts_invalid;
	BOOL avi_file;
	BOOL mpg_file;
	BOOL ts_file;
	BOOL is_network_play;
	BOOL alip_file;
}avinfo;

#define MAX_PROBE_PACKETS 2500
#define MAX_REORDER_DELAY 16

/**
 * Required number of additionally allocated bytes at the end of the input bitstream for decoding.
 * This is mainly needed because some optimized bitstream readers read
 * 32 or 64 bit at once and could read over the end.<br>
 * Note: If the first 23 bits of the additional bytes are not 0, then damaged
 * MPEG bitstreams could cause overread and segfault.
 */
#define AV_INPUT_BUFFER_PADDING_SIZE 8

typedef struct _avcodec_context
{
    INT32   frm_rate;           /* number of frames per second */
    unsigned long   total_frms; /*total video frame number */
    unsigned long   total_time; /*total playback time in ms*/

    UINT32  fb_max_stride;
    UINT32  fb_max_height;
    UINT32  dv_max_stride;
    UINT32  dv_max_height;

    CHAR    codec_name[32];
    enum    codec_type codec_type;
    enum    codec_id codec_id;
    UINT32  codec_tag;

    /* audio only */
    UINT64 sample_rate; ///< samples per second
    INT32 channels;    ///< number of audio channels

    /* The following data should not be initialized. */
    /**
     * Samples per packet, initialized when calling 'init'.
     */
    INT32 frame_size;
    INT32 frame_number;   ///< audio or video frame number

    /**
     * bits per sample/pixel from the demuxer (needed for huffyuv).
     * - encoding: Set by libavcodec.
     * - decoding: Set by user.
     */
    INT32 bits_per_coded_sample;

    /**
     * color table ID
     * - encoding: unused
     * - decoding: Which clrtable should be used for 8bit RGB images.
     *             Tables have to be stored somewhere. FIXME
     */
    INT32 color_table_id;

    /**
     * number of bytes per packet if constant and known or 0
     * Used by some WAV based audio codecs.
     */
    INT32 block_align;

    UINT8 *extradata;
    INT32 extradata_size;
    INT64 extradata_pos;

    rational time_base;
    INT64 channel_layout;
    INT32 samples_per_block;
//    UINT32 encode_options;

    /* video only */
    /**
     * picture width / height.
     * - encoding: MUST be set by user.
     * - decoding: Set by libavcodec.
     * Note: For compatibility it is possible to set this instead of
     * coded_width/height before decoding.
     */
    INT32 width, height;

    INT32 bit_rate;

    /**
     * Size of the frame reordering buffer in the decoder.
     * For MPEG-2 it is 1 IPB or 0 low delay IP.
     * - encoding: Set by libavcodec.
     * - decoding: Set by libavcodec.
     */
    INT32 has_b_frames;

//    /**
//     * palette control structure
//     * - encoding: ??? (no palette-enabled encoder yet)
//     * - decoding: Set by user.
//     */
//    struct MP4PaletteControl *palctrl;

    /* some codec may need disable
     in a special container */
    INT32 disable_codec;
    INT32 decoder_reset;
} avcodec_context;

typedef struct _mpeg4audio_config
{
    INT32 object_type;
    INT32 sampling_index;
    INT32 sample_rate;
    INT32 chan_config;
    INT32 sbr; //< -1 implicit, 1 presence
    INT32 ext_object_type;
    INT32 ext_sampling_index;
    INT32 ext_sample_rate;
    INT32 ext_chan_config;
    INT32 channels;
} mpeg4audio_config;

typedef struct AVIndexEntry {
    int64_t pos;
    int64_t timestamp;        /**<
                               * Timestamp in AVStream.time_base units, preferably the time from which on correctly decoded frames are available
                               * when seeking to this entry. That means preferable PTS on keyframe based formats.
                               * But demuxers can choose to store a different timestamp, if it is more convenient for the implementation or nothing better
                               * is known
                               */
#define AVINDEX_KEYFRAME 0x0001
    int flags:2;
    int size:30; //Yeah, trying to keep the size of this small to reduce memory requirements (it is 24 vs. 32 bytes due to possible 8-byte alignment).
    int min_distance;         /**< Minimum distance between this and the previous keyframe, used to avoid unneeded searching. */
} AVIndexEntry;

typedef struct _avstream
{
    INT32 index;    /**< stream index in AVFormatContext */
    INT32 id;       /**< format-specific stream ID */
    avcodec_context *codec; /**< codec context */
    /**
     * Real base framerate of the stream.
     * This is the lowest framerate with which all timestamps can be
     * represented accurately (it is the least common multiple of all
     * framerates in the stream). Note, this value is just a guess!
     * For example, if the time base is 1/90000 and all frames have either
     * approximately 3600 or 1800 timer ticks, then r_frame_rate will be 50/1.
     */
//    AVRational r_frame_rate;
    void *priv_data;

    /* internal data used in av_find_stream_info() */
//    INT64 first_dts;
    /** encoding: pts generation when outputting stream */
//    struct AVFrac pts;

    /**
     * This is the fundamental unit of time (in seconds) in terms
     * of which frame timestamps are represented. For fixed-fps content,
     * time base should be 1/framerate and timestamp increments should be 1.
     */
    rational time_base;
    INT32 pts_wrap_bits; /**< number of bits in pts (used for wrapping control) */
    /* ffmpeg.c private use */
//    int stream_copy; /**< If set, just copy stream. */
    enum avdiscard discard; ///< Selects which packets can be discarded at will and do not need to be demuxed.
    //FIXME move stuff to a flags field?
    /** Quality, as it has been removed from AVCodecContext and put in AVVideoFrame.
     * MN: dunno if that is the right place for it */
//    float quality;
    /**
     * Decoding: pts of the first frame of the stream, in stream time base.
     * Only set this if you are absolutely 100% sure that the value you set
     * it to really is the pts of the first frame.
     * This may be undefined (AV_NOPTS_VALUE).
     * @note The BYE3 header does NOT contain a correct start_time the BYE3
     * demuxer must NOT set this.
     */
    INT64 start_time;
    /**
     * Decoding: duration of the stream, in stream time base.
     * If a source file does not specify a duration, but does specify
     * a bitrate, this value will be estimated from bitrate and file size.
     */
    INT64 duration;

//#if LIBAVFORMAT_VERSION_INT < (53<<16)
//    char language[4]; /** ISO 639-2/B 3-letter language code (empty string if undefined) */
//#endif

    /* av_read_frame() support */
    enum avstream_parse_type need_parsing;
    struct AVCodecParserContext *parser;

    INT64 cur_dts;
//    int last_IP_duration;
    int64_t last_IP_pts;
    /* av_seek_frame() support */
    avindex_entry *index_entries; /**< Only used if the format does not support seeking natively. */
    little_avindex_entry *little_index_entries;
    INT32 nb_index_entries;
    UINT32 index_entries_allocated_size;

    INT64 nb_frames;                 ///< number of frames in this stream if known or 0
    BOOL build_little_index_table;

//#if LIBAVFORMAT_VERSION_INT < (53<<16)
//    int64_t unused[4+1];
//
//    CHAR *filename; /**< source filename of the stream */
//#endif

    INT32 disposition; /**< AV_DISPOSITION_* bit field */

//    AVProbeData probe_data;
//#define MAX_REORDER_DELAY 16
//    INT64 pts_buffer[MAX_REORDER_DELAY+1];

    /**
     * sample aspect ratio (0 if unknown)
     * - encoding: Set by user.
     * - decoding: Set by libavformat.
     */
    rational sample_aspect_ratio;

    metadata *metadata;

    /* av_read_frame() support */
//    const UINT8 *cur_ptr;
//    INT32 cur_len;
//    AVPacket cur_pkt;

    // Timestamp generation support:
    /**
     * Timestamp corresponding to the last dts sync point.
     *
     * Initialized when AVCodecParserContext.dts_sync_point >= 0 and
     * a DTS is received from the underlying container. Otherwise set to
     * AV_NOPTS_VALUE by default.
     */
//    UINT64 reference_dts;

    /**
     * Number of packets to buffer for codec probing
     * NOT PART OF PUBLIC API
     */
//#define MAX_PROBE_PACKETS 2500
//    INT32 probe_packets;

    /**
     * last packet in packet_buffer for this stream when muxing.
     * used internally, NOT PART OF PUBLIC API, dont read or write from outside of libav*
     */
//    struct AVPacketList *last_in_packet_buffer;

    /**
     * Average framerate
     */
//    AVRational avg_frame_rate;
    uint64_t attachement_link;
    uint64_t attachement_uid;
    int request_probe;
	BOOL build_index_table_err;

    uint16 com_page;
    uint16 anci_page;
} avstream;

typedef struct _avpacket
{
    /**
     * Presentation timestamp in AVStream->time_base units; the time at which
     * the decompressed packet will be presented to the user.
     * Can be AV_NOPTS_VALUE if it is not stored in the file.
     * pts MUST be larger or equal to dts as presentation cannot happen before
     * decompression, unless one wants to view hex dumps. Some formats misuse
     * the terms dts and pts/cts to mean something different. Such timestamps
     * must be converted to true pts/dts before they are stored in AVPacket.
     */
    INT64   pts;
    /**
     * Decompression timestamp in AVStream->time_base units; the time at which
     * the packet is decompressed.
     * Can be AV_NOPTS_VALUE if it is not stored in the file.
     */
    INT64   dts;
    UINT8   *data;
    INT32   size;
    INT32   stream_index;
    INT32   flags;
    /**
     * Duration of this packet in AVStream->time_base units, 0 if unknown.
     * Equals next_pts - this_pts in presentation order.
     */
    INT32   duration;
    void    (*destruct)(struct _avpacket *);
    void    *priv;
    INT64   pos;                            ///< byte position in stream, -1 if unknown

    /**
     * Time difference in AVStream->time_base units from the pts of this
     * packet to the point at which the output from the decoder has converged
     * independent from the availability of previous frames. That is, the
     * frames are virtually identical no matter if decoding started from
     * the very first frame or from this keyframe.
     * Is AV_NOPTS_VALUE if unknown.
     * This field is not the display duration of the current packet.
     *
     * The purpose of this field is to allow seeking in streams that have no
     * keyframes in the conventional sense. It corresponds to the
     * recovery point SEI in H.264 and match_time_delta in NUT. It is also
     * essential for some types of subtitle streams to ensure that all
     * subtitles are correctly displayed after seeking.
     */
    INT64   convergence_duration;
    BOOL    read_extradata;
    UINT8   drm_info_size;
    UINT8   *drm_info;
    struct
    {
        UINT8 *data;
        int      size;
        enum avpacket_side_data_type type;
    } *side_data;
    int side_data_elems;
} avpacket;

typedef struct avpacket_list
{
    avpacket pkt;
    struct avpacket_list *next;
} avpacket_list;

typedef struct avpacket_buf
{
    struct avpacket_list *pktl_base;     /* Packet list base */
    struct avpacket_list *pktl_cur;      /* The first unread packet in list */
    struct avpacket_list *pktl_end;      /* Packet list end */
    struct avpacket_list *pktl_reserve;      /* */
    struct avpacket_list *pktl_discard;
    UINT32 nb_of_pkts;
    UINT32 max_pkts;
    INT64  used_size;
} avpacket_buf;

typedef struct avffmode
{
	UINT32 prev_fast_time;
	UINT32 play_forward;
	UINT32 ff_fb_time_count;
	UINT32 last_show_pts;
	INT32 next_fast_time;
}avffmode;

#define MAX_TITTLES 20
#define MAX_STREAMS 60
#define MAX_DISPLAY_QUEUE 128
/*eric.cai debug*/
#define MAX_AUDIO_STREAMS 30
#define MAX_VIDEO_STREAMS 30
#define MAX_SUBTT_STREAMS 30

#define AVPLAYER_MAX_VIDEO_PACKETS  5
#define AVPLAYER_MAX_AUDIO_PACKETS  5
#define AVPLAYER_MAX_SUBTT_PACKETS  15

typedef struct avinput_format
{
    DWORD   (*read)(BYTE *buf, DWORD size);
    BOOL    (*seek)(INT64 offset, DWORD origin);
    INT64   (*tellpos)(void);
	INT32   (*get_seek_type)(void);
	BOOL    (*rollback)(INT64 offset, DWORD origin);
	INT64   (*get_file_size)(void);
	UINT32 (*decoder_get_cache_size)(void);
	UINT32 (*decoder_check_end)(void);

    INT32   (*read_header)(struct avinput_format *input, avinfo *info);
    INT32   (*read_packet)(struct avinput_format *input, avpacket *pkt);
    INT32   (*read_seek)(struct avinput_format *s, INT32 stream_index, INT64 *sample_time, INT32 flags);
    INT32   (*read_close)(struct avinput_format *s);
    INT64  (*read_timestamp)(struct avinput_format *s, INT32 stream_index, INT64 *pos, INT64 pos_limit);
    INT32   (*build_index)(struct avinput_format *s);

    avinfo info;

    INT32 audio_stream_array[MAX_AUDIO_STREAMS];
    INT32 audio_stream_num;
    INT32 audio_stream_idx;
    INT32 audio_stream;
    INT32 video_stream_array[MAX_VIDEO_STREAMS];
	INT64 video_stream_pts_ref;
    INT32 video_stream_num;
    INT32 video_stream_idx;
    INT32 video_stream;
    INT32 subtt_stream_array[MAX_SUBTT_STREAMS];
    INT32 subtt_stream_num;
    INT32 subtt_stream_idx;
    INT32 subtt_stream;
    INT32 ignore_stream_array[MAX_STREAMS];
    INT32 ignore_stream_num;
    UINT32 video_frame_period[MAX_VIDEO_STREAMS];
    UINT32 video_total_time[MAX_VIDEO_STREAMS];
    INT32 cur_prog_id;
    avio_context   *pb;

    void    *priv_data;

    UINT32  nb_streams;
    avstream *streams[MAX_STREAMS];

    metadata *metadata;

    INT64 duration;
    UINT32 seek_time_backup;
    unsigned int cur_edition;
    unsigned int nb_editions;
    avedition **editions;

    unsigned int nb_auto_chapters;
    avchapter **auto_chapters;

    /**
     * Forced video codec_id.
     * Demuxing: Set by user.
     */
    enum codec_id video_codec_id;
    /**
     * Forced audio codec_id.
     * Demuxing: Set by user.
     */
    enum codec_id audio_codec_id;
    /**
     * Forced subtitle codec_id.
     * Demuxing: Set by user.
     */
    enum codec_id subtitle_codec_id;

    UINT8   *file_buf;
    UINT32  file_buf_size;

    avpacket_buf *v_pkt_buf;
    avpacket_buf *a_pkt_buf;
    avpacket_buf *s_pkt_buf[MAX_SUBTT_STREAMS];

    INT32   file_structure_type;    /* 0: A/V intercross placed 1: A/V separated placed */
    INT32   sample_rate;
    INT32   channel_number;
    BOOL    is_hd;
    BOOL    eof_reached;
	BOOL    no_memory;
    INT64   file_start;
    INT64   file_end;
    UINT8   *vdec_buffer;
    int     vdec_size;
    uint64_t uid;
    char   *v_codec_name;
    char    *segment_uid;
    uint64_t  enhanced_uid;
    char    *enhanced_seg_uid;
    uint64_t  enhanced_flag;
    char    *tittle;
    BOOL     is_tpt;
    INT32    master_tpt_idx;
    uint64_t drm_version;
    uint64_t drm_header_size;
    UINT8    *drm_header;
    UINT32   movie_start_time;
    int      no_cues;
    int      flv_entries_added;
    const UINT8 *key;
    int keylen;
    INT64 data_offset;
    unsigned int packet_size;
    ali_video_config    *decv_config;
    ali_audio_config    *deca_config;
    UINT32  nb_programs;
    avprogram **programs;
  	INT64 start_time;
    BOOL get_audio_packet;
	avffmode ff_play;
	BOOL got_play_ctrl_cmd;
	DEC_INIT_PAR dec_init_param;
	UINT32 pe_seek_type;
	UINT32 variable_cache_size;
	UINT32 flowdata_increase_size;
	INT64 flow_pts_time;
	BOOL  b_first_ref_val;
	BOOL  get_cache_size_first_en;
	BOOL  is_net_Buffering_en;
} avinput_format;

struct sub_pkt_param
{
    struct control_block ctrl_blk;
	void * device;
	RET_CODE (* request_write)(void *, UINT32, void **, UINT32 *, struct control_block *);
	void (* update_write)(void *, UINT32 );
    RET_CODE (* showonoff)(void *, BOOL);
};


struct decoder_param{
	struct control_block * ctrl_blk;
	void * device;
	RET_CODE (* request_write)(void *, UINT32, void **, UINT32 *, struct control_block *);
	void (* update_write)(void *, UINT32 );
    RET_CODE (* showonoff)(void *, BOOL);
};



typedef struct _avoutput_format
{
    UINT32  cur_cpu_time;   /* current cpu time in ms */
    UINT32  cur_video_time; /* current video play time in ms */
    UINT32  video_total_time; /* total time in ms*/
    UINT32  video_frame_period; /* a video frame period in us */
    BOOL    video_support_mirror;
    BOOL    audio_support_mirror;
    BOOL    subtt_support_mirror;
    BOOL    preview;
    // if it is interleaced stream, it need to
    //decode two fields to construct one frame in FF/FB */
    BOOL    output_next_subtt;
} avoutput_format;

typedef struct _video_decode_context
{
    INT32 unsupported;
    /* the following parameters must be initialized before encoding */
    INT32 width, height; /* picture size. must be a multiple of 16 */
    INT32 frame_rate; /* number of frames per second */
    unsigned long frame_period;
    INT32 bit_rate;        /* wanted bit rate */
    unsigned long total_frames; /*total video frame number */
    unsigned long total_time; /*total playback time in ms*/

    INT32 total_buffer_number;

    BOOL need_extradata;
    void *p_mem_top_addr;
    void *p_new_mem_top_addr;
} video_decode_context;

typedef struct _avplayer
{
    avinput_format  *input;
    avoutput_format *output;
    avinput_format  *input_array[MAX_TITTLES];
    avoutput_format *output_array[MAX_TITTLES];
    void * subtitle;
    INT32 num_of_tittles;
    INT32 current_tittles;
    video_decode_context *vdec_ctx;
    SUBT_INPUT_PLUGIN *subt_plugin;
    struct sub_pkt_param subt_dvb;
    BOOL (*getcmd)(t2decoder_control *decoder_ctrl);
    t2decoder_control *decoder_ctrl;
    mp_callback_func cb;
    UINT32 initial_status;
    INT32  avplay_mode;
    UINT32 video_get_cnt;
    UINT32 audio_get_cnt;
    UINT32 set_start_play_time_ms;
} avplayer;

typedef struct AVProbeData {
    const char *filename;
    unsigned char *buf; /**< Buffer must have AVPROBE_PADDING_SIZE of extra allocated bytes filled with zero. */
    int buf_size;       /**< Size of buf except extra allocated bytes */
} AVProbeData;

typedef struct _avdemuxer
{
    const char *name;
    INT32   (*init)(void *media_cfg);
    INT32   (*read_probe)(char *filename);
    INT32   (*read_header)(struct avinput_format *input, avinfo *info);
    INT32   (*read_packet)(struct avinput_format *input, avpacket *pkt);
    INT64   (*read_timestamp)(struct avinput_format *s, INT32 stream_index, INT64 *pos, INT64 pos_limit);
    INT32   (*read_seek)(struct avinput_format *s, INT32 stream_index, INT64 *sample_time, INT32 flags);
    INT32   (*get_stream_info)(char *filename, void *media_info);
    INT32   (*get_decoder_info)(char *filename, void *media_info);
    INT32   (*read_close)(struct avinput_format *s);
    INT32   (*build_index)(struct avinput_format *s);
    INT32   (*get_song_info)(char *filename, void *media_info);
    INT32   (*cleanup)(void);
} AVDMX_PLUGIN;

typedef INT32	(*ptf_init)(void *media_cfg);
typedef INT32	(*ptf_read_probe)(char *filename);
typedef INT32	(*ptf_read_header)(struct avinput_format *input, avinfo *info);
typedef INT32	(*ptf_read_packet)(struct avinput_format *input, avpacket *pkt);
typedef INT64	(*ptf_read_timestamp)(struct avinput_format *s, INT32 stream_index, INT64 *pos, INT64 pos_limit);
typedef INT32	(*ptf_read_seek)(struct avinput_format *s, INT32 stream_index, INT64 *sample_time, INT32 flags);
typedef INT32	(*ptf_get_stream_info)(char *filename, void *media_info);
typedef INT32	(*ptf_get_decoder_info)(char *filename, void *media_info);
typedef INT32	(*ptf_read_close)(struct avinput_format *s);
typedef INT32	(*ptf_build_index)(struct avinput_format *s);
typedef INT32	(*ptf_get_song_info)(char *filename, void *media_info);
typedef INT32	(*ptf_cleanup)(void);

typedef struct _avlast_status
{
    UINT32 format_height;
    UINT32 format_width;
}avlast_status;

typedef unsigned char ff_bye3_guid[16];
typedef struct
{
    enum codec_id id;
    ff_bye3_guid guid;
} avcodec_guid;


typedef struct AVCodecParserContext {
    void *priv_data;
    struct AVCodecParser *parser;
    int64_t frame_offset; /* offset of the current frame */
    int64_t cur_offset; /* current offset
                           (incremented by each av_parser_parse()) */
    int64_t next_frame_offset; /* offset of the next frame */
    /* video info */
    int pict_type; /* XXX: Put it back in AVCodecContext. */
    /**
     * This field is used for proper frame duration computation in lavf.
     * It signals, how much longer the frame duration of the current frame
     * is compared to normal frame duration.
     *
     * frame_duration = (1 + repeat_pict) * time_base
     *
     * It is used by codecs like H.264 to display telecined material.
     */
    int repeat_pict; /* XXX: Put it back in AVCodecContext. */
    int64_t pts;     /* pts of the current frame */
    int64_t dts;     /* dts of the current frame */

    /* private data */
    int64_t last_pts;
    int64_t last_dts;
    int fetch_timestamp;

#define AV_PARSER_PTS_NB 4
    int cur_frame_start_index;
    int64_t cur_frame_offset[AV_PARSER_PTS_NB];
    int64_t cur_frame_pts[AV_PARSER_PTS_NB];
    int64_t cur_frame_dts[AV_PARSER_PTS_NB];

    int flags;
#define PARSER_FLAG_COMPLETE_FRAMES           0x0001
#define PARSER_FLAG_ONCE                      0x0002
/// Set if the parser has a valid file offset
#define PARSER_FLAG_FETCHED_OFFSET            0x0004
#define PARSER_FLAG_USE_CODEC_TS              0x1000

    int64_t offset;      ///< byte offset from starting packet start
    int64_t cur_frame_end[AV_PARSER_PTS_NB];

    /**
     * Set by parser to 1 for key frames and 0 for non-key frames.
     * It is initialized to -1, so if the parser doesn't set this flag,
     * old-style fallback using AV_PICTURE_TYPE_I picture type as key frames
     * will be used.
     */
    int key_frame;

    /**
     * Time difference in stream time base units from the pts of this
     * packet to the point at which the output from the decoder has converged
     * independent from the availability of previous frames. That is, the
     * frames are virtually identical no matter if decoding started from
     * the very first frame or from this keyframe.
     * Is AV_NOPTS_VALUE if unknown.
     * This field is not the display duration of the current frame.
     * This field has no meaning if the packet does not have AV_PKT_FLAG_KEY
     * set.
     *
     * The purpose of this field is to allow seeking in streams that have no
     * keyframes in the conventional sense. It corresponds to the
     * recovery point SEI in H.264 and match_time_delta in NUT. It is also
     * essential for some types of subtitle streams to ensure that all
     * subtitles are correctly displayed after seeking.
     */
    int64_t convergence_duration;

    // Timestamp generation support:
    /**
     * Synchronization point for start of timestamp generation.
     *
     * Set to >0 for sync point, 0 for no sync point and <0 for undefined
     * (default).
     *
     * For example, this corresponds to presence of H.264 buffering period
     * SEI message.
     */
    int dts_sync_point;

    /**
     * Offset of the current timestamp against last timestamp sync point in
     * units of AVCodecContext.time_base.
     *
     * Set to INT_MIN when dts_sync_point unused. Otherwise, it must
     * contain a valid timestamp offset.
     *
     * Note that the timestamp of sync point has usually a nonzero
     * dts_ref_dts_delta, which refers to the previous sync point. Offset of
     * the next frame after timestamp sync point will be usually 1.
     *
     * For example, this corresponds to H.264 cpb_removal_delay.
     */
    int dts_ref_dts_delta;

    /**
     * Presentation delay of current frame in units of AVCodecContext.time_base.
     *
     * Set to INT_MIN when dts_sync_point unused. Otherwise, it must
     * contain valid non-negative timestamp delta (presentation time of a frame
     * must not lie in the past).
     *
     * This delay represents the difference between decoding and presentation
     * time of the frame.
     *
     * For example, this corresponds to H.264 dpb_output_delay.
     */
    int pts_dts_delta;

    /**
     * Position of the packet in file.
     *
     * Analogous to cur_frame_pts/dts
     */
    int64_t cur_frame_pos[AV_PARSER_PTS_NB];

    /**
     * Byte position of currently parsed frame in stream.
     */
    int64_t pos;

    /**
     * Previous frame byte position.
     */
    int64_t last_pos;

    /**
     * Duration of the current frame.
     * For audio, this is in units of 1 / AVCodecContext.sample_rate.
     * For all other types, this is in units of AVCodecContext.time_base.
     */
    int duration;

//    enum AVFieldOrder field_order;

    /**
     * Indicate whether a picture is coded as a frame, top field or bottom field.
     *
     * For example, H.264 field_pic_flag equal to 0 corresponds to
     * AV_PICTURE_STRUCTURE_FRAME. An H.264 picture with field_pic_flag
     * equal to 1 and bottom_field_flag equal to 0 corresponds to
     * AV_PICTURE_STRUCTURE_TOP_FIELD.
     */
//    enum AVPictureStructure picture_structure;

    /**
     * Picture number incremented in presentation or output order.
     * This field may be reinitialized at the first picture of a new sequence.
     *
     * For example, this corresponds to H.264 PicOrderCnt.
     */
    int output_picture_number;
} AVCodecParserContext;

typedef struct AVCodecParser {
    int codec_ids[5]; /* several codec IDs are permitted */
    int priv_data_size;
    int (*parser_init)(AVCodecParserContext *s);
    int (*parser_parse)(AVCodecParserContext *s,
                        avcodec_context *avctx,
                        const uint8_t **poutbuf, int *poutbuf_size,
                        const uint8_t *buf, int buf_size);
    void (*parser_close)(AVCodecParserContext *s);
    int (*split)(avcodec_context *avctx, const uint8_t *buf, int buf_size);
    struct AVCodecParser *next;
} AVCodecParser;

#define FFMIN(a, b) ((a) > (b) ? (b) : (a))
#define FFMAX(a,b) ((a) > (b) ? (a) : (b))

#define FF_MEDIASUBTYPE_BASE_GUID \
    0x00,0x00,0x10,0x00,0x80,0x00,0x00,0xAA,0x00,0x38,0x9B,0x71

 int  avplayer_decoder_init(PDEC_INIT_PAR dec_init_par);
 BOOL    avplayer_decoder_reset(UINT8 iPreview);
 BOOL    avplayer_decoder_decode(mp_callback_func video_cb);
 DWORD   avplayer_decoder_get_last_err_msg(void);
 int     avplayer_get_init_param(const struct pe_video_cfg *pe_video_cfg, \
                           int media_type, \
                           DEC_INIT_PAR *p_init_param, \
                           void *p_other_param);
 int     avplayer_rls_init_param(const struct pe_video_cfg *pe_video_cfg, \
                           DEC_INIT_PAR *p_init_param, \
                           void *p_other_param);
 UINT32 avplayer_pts_to_ms(INT64 pts, rational r);
 DWORD   avplayer_get_play_time(void);
 DWORD   avplayer_get_duration(void);
 INT32   avplayer_get_stream_info(char *file, void * info);
 int     avplayer_decoder_ioctl(unsigned long io_cmd, \
                          unsigned long param1, \
                          unsigned long param2);

 INT32 avplayer_check_tpt(void);
 void avplayer_build_auto_chapter(void);
 INT32 avplayer_get_chapter_info(PDEC_CHAPTER_INFO p_chapter_info);
 UINT32 avplayer_ui2edition_time(avinput_format *ip, UINT32 time);
 UINT32 avplayer_edition2ui_time(avinput_format *ip, UINT32 time);
 void avplayer_update_play_chapter_by_time(avinput_format *ip, UINT32 time);
 void avplayer_update_play_chapter_by_delt(avinput_format *ip, BOOL inc_dec);
 BOOL avplayer_is_in_cur_play_chapter(avinput_format *ip, UINT32 edition_time);
 BOOL avplayer_is_in_pre_chapter(avinput_format *ip, UINT32 edition_time);

#ifdef __cplusplus
}
#endif

#endif

