#ifndef __MEDIA_PLAYER_SUBTITLE_VOBSUB_H_
#define __MEDIA_PLAYER_SUBTITLE_VOBSUB_H_


#ifdef __cplusplus
extern "C" {
#endif


typedef struct
{
    unsigned long                pts;
    long                        aid;
    unsigned char                *packet;
    unsigned long                packet_reserve;
    unsigned long                packet_size;
} mp_vob_sub_mpeg, *pmp_vob_sub_mpeg;


typedef struct
{
    unsigned char                packet[52*1024];

    unsigned long                packet_size;
    unsigned long                packet_offset;

    unsigned long                pts;

    unsigned long                palette[4];
    unsigned long                alpha[4];
    unsigned long                control_start;
    unsigned long                current_nibble[2];
    long                        deinterlace_oddness;
    unsigned long                start_col, end_col;
    unsigned long                start_row, end_row;
    unsigned long                width, height, stride;
    unsigned long                start_pts, end_pts;

    unsigned char                *image;
    unsigned long                image_size;
} mp_vob_sub_spu, *pmp_vob_sub_spu;


#define    MAX_ID_COUNT    8

typedef struct
{
    unsigned long            idx_file_pos;

    unsigned long            palette[17];
    unsigned long            cuspal[4];

    long                    have_palette;
    long                    custom;
    long                    delay;
    long                    origin_x;
    long                    origin_y;
    long                    orig_frame_width;
    long                    orig_frame_height;
    long                    forced_subs;

    long                    current_id_index;
    long                    total_id_count;
    long                    id[MAX_ID_COUNT];
    long                    id_pos[MAX_ID_COUNT];
    char                    id_description[MAX_ID_COUNT][3];
}mp_vob_sub, *pmp_vob_sub;




void mp_subtitle_parse_vobsub(void *sub);
long mp_subtitle_vobsub_parse_ps_packet(unsigned char *buffer, long buffer_size, mp_vob_sub_mpeg *mpeg);
long mp_subtitle_vobsub_spu_parse_pxd(mp_vob_sub *vob, mp_vob_sub_spu *spu);
long mp_subtitle_vobsub_spu_reassemble(mp_vob_sub_spu *spu, mp_vob_sub_mpeg *mpeg);
long mp_subtitle_vobsub_query_language(void *sub, mp_vob_sub *vob, long *number, long *current_language_index, \
                                                  char* description, long *description_len);
long mp_subtitle_vobsub_change_language(void *sub, mp_vob_sub *vob, long language_index);



/**********************Global  Variables***********************************/
//delarate the Global  Variables
extern long vobsub_id;


#ifdef __cplusplus
}
#endif


#endif

