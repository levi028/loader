/*
pack the living stream into ali special media stream.
it helps to play the living stream by the ali's media player.
Descritption to the ali media stream:
1, magic sequence
    6 : 0 --> "alitech" (magic number)
    7      -->  0 ~ 255 (version)

2, blocks
    3 : 0    --> type
    7 : 4--> length (size of block data, type and length itself)
    end : 8 --> block data

3, blocks introduce

    type        data                        comment

    "NULL"     *                        empty

    "DATA"    struct ls_data_header;         meida data
            unsigned char data[];

    "PROG"    struct ls_prog_header;        media program info
            unsigned char data[];

    "RAND"    struct ls_rand;                random access pointer
*/

#ifndef    __LIB_STREAM_H__
#define    __LIB_STREAM_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define LS_END_OFFSET                (0x0FFFFFFFFFFFFFFF)

#define BLOCK_TYPE(A, B, C, D) (\
    ((unsigned long) A) | \
    (((unsigned long) B)<<8) | \
    (((unsigned long) C)<<16) | \
    (((unsigned long) D)<<24) )

#define LS_NULL \
    BLOCK_TYPE('N', 'U', 'L', 'L')

#define LS_DATA \
    BLOCK_TYPE('D', 'A', 'T', 'A')

#define LS_PROG \
    BLOCK_TYPE('P', 'R', 'O', 'G')

#define LS_RAND \
    BLOCK_TYPE('R', 'A', 'N', 'D')

typedef unsigned long LS_HANDLE;

enum LS_DATA_TYPE
{
    LS_DATA_AUDI = 0,
    LS_DATA_VIDE,
    LS_DATA_SUBT
};

enum LS_SEEK_FLAG
{
    LS_SEEK_SET = 0,
    LS_SEEK_CUR,
    LS_SEEK_END
};

struct ls_block_header
{
    unsigned long type;
    unsigned long length;
};

#define ls_none_ts                                        (0x00000000)
#define ls_presentation_ts                                (0x00000001)
#define ls_decoding_ts                                    (0x00000002)

struct ls_timestamp
{
    unsigned long flag;
    unsigned long value;
};

struct ls_data_header
{
    int prog_index;
    int sample_index;

    struct ls_timestamp timestamp;
};

struct ls_rand_entry
{
    int sample_index;
    unsigned long long sample_offset;
};

struct ls_rand
{
    int prog_index;
    int count;

    void *data;
};

struct ls_prog_header
{
    int prog_index;
    enum LS_DATA_TYPE type;
    unsigned long codec_tag; // fourcc code
    unsigned long version;
    unsigned long profile;
    unsigned long level;
    unsigned long reserved;

    unsigned long timescale;
    unsigned long duration;
    int sample_num;
    int sample_duration;

    // resolution
    unsigned long resolution;

    void *private;
};

struct ls_prog_audi_header
{
    // sample rate
    unsigned long sample_rate;

    // channel count
    unsigned long channel_count;

    // bytes per sample
    unsigned long sample_size;

    // stream bit rate
    unsigned long bit_rate;
};

struct ls_prog_vide_header
{
    // track size and display area
    long width;
    long height;
    long d_left;
    long d_top;
    long d_width;
    long d_height;

    // frame rate. (frames/second * 1000)
    unsigned long frame_rate;

    // horizontal and vertical resolution
    unsigned long h_resolution;
    unsigned long v_resolution;
};

struct ls_prog_subt_header
{
    int language;
};

struct ls_config
{
    int cache_size;
    int reserved_size;

    char *file_path; // if it is NULL, the media data will just be reserved in the cache buffer
};

/* after setting all the program info, start to push media data */
#define LS_CONTROL_START                        0x00000001

/* stop the push operation */
#define LS_CONTROL_STOP                        0x00000002

/* check the stream info if it don't have program info */
#define LS_CONTROL_CHECK_STREAM                0x00000003

/* get the stream end flag */
#define LS_CONTROL_CHECK_END                    0x00000004

#define LS_CONTROL_SET_END                        0x00000005
#define LS_CONTROL_SET_THRESHOLD                0x00000006
#define LS_CONTROL_GET_FREE_SIZE                0x00000007
#define LS_CONTROL_GET_DATA_HEADER_SIZE         0x00000008

LS_HANDLE libstream_open(const struct ls_config *cfg);
void libstream_close(LS_HANDLE ls);
int libstream_set_prog_info(LS_HANDLE ls, const void *info);
int libstream_control(LS_HANDLE ls, unsigned long cmd, unsigned long par);
int libstream_push_data(LS_HANDLE ls, const void *buf_start, int buf_size, const void *info);
int libstream_fread(LS_HANDLE ls, void *buf_start, int buf_size);
long long libstream_fseek(LS_HANDLE ls, enum LS_SEEK_FLAG flag, long long offset);
long long libstream_ftell(LS_HANDLE ls);

#ifdef __cplusplus
}
#endif

#endif

