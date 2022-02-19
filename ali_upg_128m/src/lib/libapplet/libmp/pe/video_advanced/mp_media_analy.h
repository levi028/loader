#ifndef __MP_MEDIA_ANALY__
#define __MP_MEDIA_ANALY__

#define TS_FEC_PACKET_SIZE 204
#define TS_DVHS_PACKET_SIZE 192
#define TS_PACKET_SIZE 188

#ifndef AV_RB32
#   define AV_RB32(x)                           \
    ((((const UINT8*)(x))[0] << 24) |         \
     (((const UINT8*)(x))[1] << 16) |         \
     (((const UINT8*)(x))[2] <<  8) |         \
      ((const UINT8*)(x))[3])
#endif

int ts_get_packet_size(const UINT8 *buf, int size);
int flv_get_header(const UINT8 *buf);
int avi_get_header(const UINT8 *buf);
int mkv_get_header(const UINT8 *buf);
int mp4_get_header(const UINT8 *buf);
int bye3_get_header(const UINT8 *buf);
int ps_get_header(const UINT8 *buf);

#endif


