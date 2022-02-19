#ifndef LIBNET_INTERNAL_H_
#define LIBNET_INTERNAL_H_

#define LIBNET_SPEED_CACULATE_TIME 1000  


typedef struct _header_t
{
    UINT8 seek_info; // 0 no seek, 1 byte seek, default should have seek
    UINT8 transfer_chunked; // 0 no, 1 chunked
    UINT16 response_code;
    UINT8 *newLocation; // when response code= 301 302, server response me new URL, we should request this URL.
}HeaderInfo;

typedef struct _speed_t
{
    unsigned int    speed;
    unsigned int    last_data_cnt;          // keep last data count;
    unsigned int    last_speed_tick;        // keep last caculate tick;
}SpeedInfo;

typedef enum _DownLoadStatus
{
	LIBNET_DOWNLOAD_NA,
	LIBNET_DOWNLOAD_INIT,
	LIBNET_DOWNLOAD_START,
	LIBNET_DOWNLOAD_DOWNLOADING,
	LIBNET_DOWNLOAD_WAITDATA,
	LIBNET_DOWNLOAD_SETSEEK,
	LIBNET_DOWNLOAD_FINISH,
	LIBNET_DOWNLOAD_ERROR,
	LIBNET_DOWNLOAD_ABORT,
}DownLoadStatus;

typedef struct _Rsrc_t
{
	char * 			full_url;
	unsigned char *	buffer;  // pe cache buffer
	unsigned int 	buf_len; // pe cache buffer len
	off_t 	        offset;  // User read offset of this file
	off_t 	        offset_newval;  // User read offset of this file
	off_t 	        fileSize;  // total size of this file
	CURL * 			pCurl;      // keep current src's using CURL
	off_t           yDataCnt;   // recieved data count since new GET request
	swapbuf_t       *swapbuf;       // swapbuf, no lock
	unsigned int 	yTransTime;     // timeout
	unsigned int 	idx:				4;
	unsigned int 	reserved:			28;
    libnet_callback notify;
    SpeedInfo       speedInfo;
    HeaderInfo      headerInfo;
    UINT32          debug_open_time;
    unsigned int ultimeseekflag;
    double    dtimelength;
    double   dtimeoffset;
}Rsrc_t;

typedef struct _RsrcStatus
{
	DownLoadStatus	yDownLoadStatus;
	unsigned int 	bUsed:				1;  // flag this object is used by anyone
	unsigned int 	NetDownLoadStatus:	1;  // set is download began
	unsigned int	bUsePeCache:		1;  // flag this net open use pecache or not
	//unsigned int	bPeCacheRequest:	1;  // no use anymore
	unsigned int 	reserved:			29;		
    ID              mutex; 
    struct _Rsrc_t  tpRsrc;
}RsrcStatus_t;

size_t write_data(void *ptr, size_t size, size_t nmemb, void *userdata);
INT32 libnet_rsc_lock(INT32 idx);
INT32 libnet_rsc_unlock(INT32 idx);

#endif

