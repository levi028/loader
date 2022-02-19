#ifndef _UF_HLS_H__
#define _UF_HLS_H__


#define ATTR_ALIGN_32        __attribute__((aligned(64)))
#define HTTP_COMMON_LINE_LENGTH 1024
#define HLS_NETWORK_CONN_TIMEOUT	5
#define HLS_NETWORK_TIMEOUT		30
#define HLS_PROBE_BUF_SIZE	1024
#define HLS_M3U8FILE_MAX_LENGTH     (1024 * 512)
#define HLS_AES_BLOCK_SIZE		16
#define HLS_DEC_BUF_SIZE		(1024 * 64)//64k
#define SPACE_CHARS	" \t\r\n"
#define HLS_URL_LENGTH     1024

#define HLS_CONNECT_TYR_CNT 0xffffffff

#define  __tds__
#if 1
#ifdef __tds__
#define __log(MOD, FMT, ARGS...)						\
	do { libc_printf("<%4.4s>[%30.30s]\t", MOD, __FUNCTION__); libc_printf(FMT, ##ARGS); } while(0)
#else
#define __log(MOD, FMT, ARGS...)						\
	do { fprintf(stderr, "<%4.4s>[%30.30s]\t", MOD, __FUNCTION__); fprintf(stderr, FMT, ##ARGS); } while(0)
#endif
#else
#define __log(MOD, FMT, ARGS...)
#endif


typedef ID			MUTEX;

enum{
    URL_SEGMENT_NO = -1,
    URL_SEGMENT_NORMAL,
    URL_SEGMENT_FINISH,
    URL_SEGMENT_EXIT,
};

typedef struct {
	int pos;
	int key_pos;
	int seq_no;
	double duration;
} segment;
typedef struct{
    int url_pos;
    int bandwidth;
    char url_path[HTTP_COMMON_LINE_LENGTH];
}variants;


typedef struct {
	CURL *curl;
    CURL *curl_ts;//jary test
	uint8_t tid_m3u8_exit;
	int start_seq_no;
	int download_seq_no;
	int target_duration;
	int is_continue;

	int cur_pos;
	int is_segment;
	char *m3u8_stream;
    char *m3u8_url;

#ifdef DSC_SUPPORT
	int is_encrypted;
	int has_ivec;
	void *dec_handle;
	int cur_key_pos;
	ATTR_ALIGN_32
	uint8_t dec_buf[HLS_DEC_BUF_SIZE];
	ATTR_ALIGN_32
	uint8_t enc_buf[HLS_DEC_BUF_SIZE];
	uint32_t dec_len;
	uint32_t enc_len;
	uint32_t cur_size;// download size of the current segment
	uint8_t key[16];// current key
	uint8_t ivec[16];// current initialize vector
#endif

#ifdef IS_RECORD_FILE
	FILE *stream;
#endif

	segment *segments;
	int segment_nr;
	int segment_idx;
	int seek_idx;
	double duration;

    int variant_m3u; // flag the is m3u variant file
    variants *variants;
    int variant_nr;

    MUTEX lock;
    uint64_t m3u_length;
	int is_do_seek;
    int is_new_seqno;
    int exit_flag;
    urlfile *file;
    int hls_m3u8_eof;
    uint32_t dl_count;
    uint32_t dl_start_tick;
    uint32_t dl_speed; //bps
    int user_choice;
}HLSContext;

typedef struct {
	char *buf;
	uint32_t size;
	uint32_t offset;// actual size
	HLSContext *context;
} COMMON_DOWNLOAD_CONTEXT;

typedef struct
{
    char *buff;
    int   size;
    int   len;
    int   idx;
} uf_hls_ref;

typedef struct
{
    HLSContext* context;
    uf_hls_ref ref;
    CURL *m3u_curl;
    UINT32 timer;
    
    off_t   range_start; // for seek/reopen
    UINT32 ts_size;
    UINT32 ts_duration;
    UINT32 ts_time;

    int     retry_connect;
} uf_hlspriv;


HLSContext *hls_open(urlfile *file);
int hls_get_segment_url(HLSContext *context, char *segment_url);
#ifdef DSC_SUPPORT
  int hls_des_data(HLSContext *context, char *ptr, size_t size);
#endif

#endif //end of _UF_HLS_H__
