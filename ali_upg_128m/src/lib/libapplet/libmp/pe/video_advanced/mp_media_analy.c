#include <types.h>
#include <sys_config.h>
#include <retcode.h>
#include <api/libc/string.h>
#include <api/libc/printf.h>
#include <osal/osal.h>

#include <api/libmp/pe.h>
#include <hld/decv/decv_media_player.h>
#include "mp_media_analy.h"

#include <api/libmp/pe.h>

#define EBML_ID_HEADER             0x1A45DFA3

const unsigned char bye3_header[16] = {
    0x30, 0x26, 0xB2, 0x75, 0x8E, 0x66, 0xCF, 0x11, 0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C
};

static int mp3_analyze(unsigned int header){
    /* header */
    if ((header & 0xffe00000) != 0xffe00000)
        return -1;
    /* layer check */
    if ((header & (3<<17)) == 0)
        return -1;
    /* bit rate */
    if ((header & (0xf<<12)) == 0xf<<12)
        return -1;
    /* frequency */
    if ((header & (3<<10)) == 3<<10)
        return -1;
    return 0;
}

static int ts_analyze(const uint8_t *buf, int size, int packet_size, int *index)
{
    int *stat;
    int i;
    int x=0;
    int best_score=0;

	stat = malloc(packet_size*sizeof(int));
    MEMSET(stat, 0, packet_size*sizeof(int));
	if(stat==NULL)
		return 0;

    for(x=i=0; i<size-3; i++)
    {
        if(buf[i] == 0x47 && !(buf[i+1] & 0x80))
        {
            stat[x]++;
            if(stat[x] > best_score)
            {
                best_score= stat[x];
                if(index) *index= x;
            }
        }

        x++;
        if(x == packet_size) x= 0;
    }

    free(stat);
    return best_score;
}

/* autodetect fec presence. Must have at least 1024 bytes  */
int ts_get_packet_size(const UINT8 *buf, int size)
{
    int score, fec_score, dvhs_score;

    if (size < (TS_FEC_PACKET_SIZE * 5 + 1))
        return -1;

    score    = ts_analyze(buf, size, TS_PACKET_SIZE, NULL);
    dvhs_score    = ts_analyze(buf, size, TS_DVHS_PACKET_SIZE, NULL);
    fec_score= ts_analyze(buf, size, TS_FEC_PACKET_SIZE, NULL);
//    av_log(NULL, AV_LOG_DEBUG, "score: %d, dvhs_score: %d, fec_score: %d \n", score, dvhs_score, fec_score);
    if     (score > fec_score && score > dvhs_score) return TS_PACKET_SIZE;
    else if(dvhs_score > score && dvhs_score > fec_score) return TS_DVHS_PACKET_SIZE;
    else if(score < fec_score && dvhs_score < fec_score) return TS_FEC_PACKET_SIZE;
    else                       return -1;
}

int ps_get_header(const UINT8 *buf)
{
    if(buf[0] == 0x00 && buf[1] == 0x00 && buf[2] == 0x01 && buf[3] == 0xBA)
        return 1;
    return 0;
}


int flv_get_header(const UINT8 *buf)
{
    const UINT8 probe_data[] = "FLV";

    if(memcmp(buf, probe_data, sizeof(probe_data)-1))
        return 0;
    return 1;
}

int alip_get_header(const UINT8 *buf)
{
    const UINT8 probe_data[] = "ALIP";

    if(memcmp(buf, probe_data, sizeof(probe_data)-1))
        return 0;
    return 1;
}

int avi_get_header(const UINT8 *buf)
{
    const UINT8 probe_data[] = "RIFF";
    const UINT8 ext_data[] = "AVI";
    const UINT8 ext_data1[] = "AVIX";

    if(memcmp(buf, probe_data, sizeof(probe_data)-1))
        return 0;
    if((memcmp(buf+8, ext_data, sizeof(ext_data)-1) && memcmp(buf+8, ext_data1, sizeof(ext_data1)-1)))
        return 0;
    return 1;
}

int mkv_get_header(const UINT8 *buf)
{
    if(AV_RB32(buf) != EBML_ID_HEADER)
        return 0;
    return 1;
}

int mp4_get_header(const UINT8 *buf)
{
    //UINT32 size = 0;
    const UINT8 probe_data[] = "ftyp";

    //size = AV_RB32(buf);
    if(memcmp(buf+4, probe_data, sizeof(probe_data)-1))
        return 0;
    return 1;
}

int bye3_get_header(const UINT8 *buf)
{
    if(memcmp(buf, bye3_header, 16))
        return 0;
    return 1;
}

int aac_get_header(const UINT8 *buf)
{
    return 0;
}

int ac3_get_header(const UINT8 *buf)
{
    return 0;
}

int bye1_get_header(const UINT8 *buf)
{
    return 0;
}


int flac_get_header(const UINT8 *buf)
{
    return 0;
}

int mp3_get_packet_size(const UINT8 *buf,int size)
{
    int i;
    unsigned int stat;

    if(size < 4)
        return 0;

    for(i = 0;i < size - 3; i ++)
    {
        stat = (buf[i]<<24) + (buf[i+1]<<16) + (buf[i+2] << 8) + buf[3];
        if(mp3_analyze(stat) == 0)
        {
            return 1;
        }
    }
    return 0;
}

int ogg_get_header(const UINT8 *buf)
{
    return 0;
}

int wav_get_header(const UINT8 *buf)
{
    return 0;
}

