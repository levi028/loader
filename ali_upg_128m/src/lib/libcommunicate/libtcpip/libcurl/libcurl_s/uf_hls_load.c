#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <api/libcurl/rbuffer.h>

#ifdef __linux
#include <stdint.h>
#include <arpa/inet.h>
#include <curl/curl.h>
#include "ali_hls.h"
#else
#include <api/libcurl/curl.h>
//#include <api/libalihls/ali_hls.h>
#endif
#include <api/libcurl/urlfile.h>

#include "uf_hls.h"

#define LOG(FMT, ARGS...) __log("HLS", FMT, ##ARGS)
#define in_range(c, lo, up)  ((unsigned char)c >= lo && (unsigned char)c <= up)
#define isdigit(c)           in_range(c, '0', '9')
#define isspace(c)    (' ' == c || '\f' == c || '\n' == c || '\r' == c || '\t' == c || '\v' == c)


#ifdef HLS_SUPPORT

#ifdef DYNAMIC_RESOLUTION_SUPPORT
#define VARIANTS_SWITCH
#endif
// for debug
//#define IS_RECORD_FILE

//static int user_choice = 0xFFFFFFFF;

#define is_hex_num(c) ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))


static uint64_t http_get_content_length(CURL *curl);

static void MutexInit(MUTEX *mutex)
{
#ifdef __tds__
	*mutex = osal_mutex_create();
#else
	pthread_mutex_init(mutex, NULL);
#endif
}

static void MutexLock(MUTEX *mutex)
{
#ifdef __tds__
	osal_mutex_lock(*mutex, TMO_FEVR);
#else
	pthread_mutex_lock(mutex);
#endif
}

static void MutexUnlock(MUTEX *mutex)
{
#ifdef __tds__
	osal_mutex_unlock(*mutex);
#else
	pthread_mutex_unlock(mutex);
#endif
}

static void MutexDestroy(MUTEX *mutex)
{
#ifdef __tds__
	osal_mutex_delete(*mutex);
#else
	pthread_mutex_destroy(mutex);
#endif
}


static inline int av_toupper(int c)
{
	if (c >= 'a' && c <= 'z')
		c ^= 0x20;
	return c;
}

static int ff_hex_to_data(uint8_t *data, const char *p)
{
	int c, len, v;

	len = 0;
	v = 1;
	for (;;) {
		p += strspn(p, SPACE_CHARS);
		if (*p == '\0')
			break;
		c = av_toupper((unsigned char) *p++);
		if (c >= '0' && c <= '9')
			c = c - '0';
		else if (c >= 'A' && c <= 'F')
			c = c - 'A' + 10;
		else
			break;        
		v = (v << 4) | c;
		if (v & 0x100) {
			if (data)
				data[len] = v;
			len++;
			v = 1;
		}
	}
	return len;
}


void url_decode(char *url)
{
    int i = 0;
	char *p = NULL;
	char *buf = NULL;
    int length = 0;

	length = (int)strlen(url);
    buf = (char*)malloc(length+10);
	
	if (NULL == buf)
	{
		return;
	}

    memset(buf, 0, length);
    p = buf;
    while(i < length)
    {
        if(i <= length -3 && url[i] == '\%' && is_hex_num(url[i+1]) && is_hex_num(url[i+2]))
        {
            //sscanf(url + i + 1, "%2x", p++);
            ff_hex_to_data(p++, url + i + 1);
            i += 3;
        }
        else
        {
            *(p++) = url[i++];
        }
    }
	memcpy(url, buf, length);
	free(buf);
}

    
static int av_strstart(const char *str, const char *pfx, const char **ptr)
{
	while (*pfx && *pfx == *str) {
		pfx++;
		str++;
	}
	if (!*pfx && ptr)
		*ptr = str;
	return !*pfx;
}

static size_t av_strlcpy(char *dst, const char *src, size_t size)
{
	size_t len = 0;
	while (++len < size && *src)
		*dst++ = *src++;
	if (len <= size)
		*dst = 0;
	return len + strlen(src) - 1;
}

static size_t av_strlcat(char *dst, const char *src, size_t size)
{
	size_t len = strlen(dst);
	if (size <= len + 1)
		return len + strlen(src);
	return len + av_strlcpy(dst + len, src, size - len);
}

static void ff_make_absolute_url(char *buf, int size, const char *base,
				 const char *rel)
{
	char *sep, *path_query;
	/* Absolute path, relative to the current server */
	if (base && strstr(base, "://") && rel[0] == '/') {
		if (base != buf)
			av_strlcpy(buf, base, size);
		sep = strstr(buf, "://");
		if (sep) {
			/* Take scheme from base url */
			if (rel[1] == '/') {
				sep[1] = '\0';
			} else {
				/* Take scheme and host from base url */
				sep += 3;
				sep = strchr(sep, '/');
				if (sep)
					*sep = '\0';
			}
		}
		av_strlcat(buf, rel, size);
		return;
	}
	/* If rel actually is an absolute url, just copy it */
	if (!base || strstr(rel, "://") || rel[0] == '/') {
		av_strlcpy(buf, rel, size);
		return;
	}
	if (base != buf)
		av_strlcpy(buf, base, size);

	/* Strip off any query string from base */
	path_query = strchr(buf, '?');
	if (path_query != NULL)
		*path_query = '\0';

	/* Is relative path just a new query part? */
	if (rel[0] == '?') {
		av_strlcat(buf, rel, size);
		return;
	}

	/* Remove the file name from the base url */
	sep = strrchr(buf, '/');
	if (sep)
		sep[1] = '\0';
	else
		buf[0] = '\0';
	while (av_strstart(rel, "../", NULL) && sep) {
		/* Remove the path delimiter at the end */
		sep[0] = '\0';
		sep = strrchr(buf, '/');
		/* If the next directory name to pop off is "..", break here */
		if (!strcmp(sep ? &sep[1] : buf, "..")) {
			/* Readd the slash we just removed */
			av_strlcat(buf, "/", size);
			break;
		}
		/* Cut off the directory name */
		if (sep)
			sep[1] = '\0';
		else
			buf[0] = '\0';
		rel += 3;
	}
	av_strlcat(buf, rel, size);
}


static int curl_progress_cb(void *clientp,
                                      double dltotal,
                                      double dlnow,
                                      double ultotal,
                                      double ulnow)
{
    HLSContext *context = NULL;

    context = (HLSContext *)clientp;
    if (context)
    {
        return context->hls_m3u8_eof;
    }
    else
    {
        return 0;
    }
}

static size_t common_download_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	COMMON_DOWNLOAD_CONTEXT *download_context = (COMMON_DOWNLOAD_CONTEXT *)userdata;
    HLSContext *context = NULL; 
	int copy_size = 0;
	int left = 0;

    if (NULL == download_context)
    {
        return 0;
    }
    context = download_context->context;
    if((NULL == context) || (context->hls_m3u8_eof))
    {
        return 0;
    }
	copy_size = size * nmemb;
	left = download_context->size - download_context->offset;

	if (copy_size > left) {
		copy_size = left;
	}

	if (0 < copy_size) {
		memcpy(download_context->buf + download_context->offset, ptr, copy_size);
		download_context->offset += copy_size;
	}

	LOG("copy_size: %d\n", copy_size);
	return copy_size;
}

static size_t m3u_download_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	COMMON_DOWNLOAD_CONTEXT *download_context = (COMMON_DOWNLOAD_CONTEXT *)userdata;
    HLSContext *context = NULL; 
	int copy_size = 0;
	int left = 0;

    if (NULL == download_context)
    {
        return 0;
    }
    context = download_context->context;
    if((NULL == context) || (context->hls_m3u8_eof))
    {
        return 0;
    }
    if (0 == context->m3u_length)
    {
        context->m3u_length = http_get_content_length(context->curl);
        //libc_printf("%s(), content length:%d\n", __FUNCTION__, context->m3u_length);
        if ((0xffffffff == context->m3u_length) || (HLS_M3U8FILE_MAX_LENGTH < context->m3u_length))
        {
            LOG("invalid content length\n");
            context->m3u_length = HLS_M3U8FILE_MAX_LENGTH;
        }
        if (context->m3u8_stream)
        {
            free(context->m3u8_stream);
            context->m3u8_stream = NULL;    
        }
        context->m3u8_stream = (char *)malloc(context->m3u_length + 5);
        if (NULL == context->m3u8_stream)
        {
            LOG("OOM: alloc m3u8 buf fail\n");
            return 0;
        }
        memset(context->m3u8_stream, 0x0, context->m3u_length + 5);
        download_context->buf = context->m3u8_stream;
        download_context->size = context->m3u_length;
    }
    
	copy_size = size * nmemb;
	left = download_context->size - download_context->offset;

	if (copy_size > left) {
		copy_size = left;
	}

	if (0 < copy_size) {
		memcpy(download_context->buf + download_context->offset, ptr, copy_size);
		download_context->offset += copy_size;
	}


	LOG("copy_size: %d\n", copy_size);
	return copy_size;
}

static int http_download(HLSContext *context, const char *url, uint8_t *buf, uint32_t size)
{
    CURL *curl = NULL;
    COMMON_DOWNLOAD_CONTEXT download_context;
    int retval = 0;

    memset(&download_context, 0x0, sizeof (download_context));
    download_context.buf = buf;
    download_context.size = size;
    download_context.offset = 0;
    download_context.context = context;

    curl = curl_easy_init( );
    if (NULL == curl) {
        LOG("curl easy init fail\n");
        return -1;
    }

    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(curl, CURLOPT_URL, url);
//    curl_easy_setopt(curl, CURLOPT_TIMEOUT, HLS_NETWORK_TIMEOUT);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, HLS_NETWORK_CONN_TIMEOUT);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, common_download_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &download_context);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1; Trident/4.0; CIBA)");
    
    retval = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    return retval;
}

static uint64_t http_get_content_length(CURL *curl)
{
    uint64_t content_length = 0;

    curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, (double *)&content_length);
    return content_length;
}

#ifdef DSC_SUPPORT

//return: the length of descramble the data
int hls_des_data(HLSContext *context, char *ptr, size_t size)
{
    uint64_t content_length = 0;
    int total = (int)size;
    int cpy_len = HLS_DEC_BUF_SIZE - context->enc_len;
    int descrambled_len = 0;
    
    cpy_len = cpy_len < total ? cpy_len : total;
    memcpy(context->enc_buf + context->enc_len, ptr, cpy_len);
    context->enc_len += cpy_len;

    if (context->enc_len == HLS_DEC_BUF_SIZE)
    {
        aes_cbc_decrypt_hls(context->dec_handle, 
    		context->enc_buf, 
    		context->dec_buf, 
    		context->enc_len, 
    		context->ivec);


        descrambled_len = context->enc_len;
        context->dec_len = context->enc_len;
        context->enc_len = 0;
        
        if (cpy_len < total)
        {
            context->enc_len = total - cpy_len;
            memcpy(context->enc_buf, ptr + cpy_len, context->enc_len);			
        }
    }

    context->cur_size += total;
    content_length = http_get_content_length(context->curl_ts);

    if (context->cur_size == content_length && 0 < context->enc_len)
    {
        context->enc_len = context->enc_len / HLS_AES_BLOCK_SIZE * HLS_AES_BLOCK_SIZE;
        aes_cbc_decrypt_hls(context->dec_handle, 
    		context->enc_buf, 
    		context->dec_buf, 
    		context->enc_len, 
    		context->ivec);

        descrambled_len = context->enc_len;
        context->dec_len = context->enc_len;
        context->enc_len = 0;
    }

    return descrambled_len;
    
}

#endif


static int hls_get_current_url(HLSContext *context, char *url, int length, char *input_line)
{
    uf_hlspriv *hlspriv = NULL;
    char *path = NULL;
    int ret = 0;

    if ((NULL == context) || (NULL == context->file) || (NULL == context->file->private))
    {
        url = NULL;
        return -1;
    }
    hlspriv = (uf_hlspriv *)(context->file->private);
    curl_easy_getinfo(context->curl, CURLINFO_EFFECTIVE_URL, &path);
    
    if (path)
    {
        ff_make_absolute_url(url, length, path, input_line);
        ret = 0;
    }
    else
    {
        ret = -1;
    }
    return ret;
}

#ifdef DSC_SUPPORT
static int download_key(HLSContext *context, const char *segment)
{
    char new_path[HTTP_COMMON_LINE_LENGTH] = {0};

    hls_get_current_url(context, new_path, HTTP_COMMON_LINE_LENGTH, segment);

    return http_download(context, new_path, context->key, 16);
}
#endif
#if 0
static int ff_get_line(const char *stream, char *buf, int maxlen)
{
    int i = 0;
    char c = '\0';

    c = stream[0];
    while(('\n' != c) && c)
    {
        if(i>= maxlen)
        {
            LOG("###Error line length:%s\n", buf);
            return -1;
        }
        c = stream[i];
        buf[i++] = c;
    }
    return i;
}

#else
static int ff_get_line(const char *stream, char *buf, int maxlen)
{
    int i = 0;
    char c = '\0';

    do
    {
        c = stream[i];
        if (c && i < maxlen-1)
        {
            buf[i++] = c;
        }
        else if (i >= (maxlen-1))
        {
            return -1;
        }
    } while (c != '\n' && c);

    buf[i] = 0;
    return i;
}
#endif

static int read_chomp_line(const char *stream, char *buf, int maxlen)
{
    int len = 0;
    int valid_len = 0;

    len = ff_get_line(stream, buf, maxlen);
    if(len<0)
    {
        return -1;
    }

    valid_len = len;
    while (valid_len > 0 && isspace(buf[valid_len - 1]))
    {
        buf[--valid_len] = '\0';
    }
    //LOG("valid length:%d\n",valid_len);
    return len;
}

#ifdef DSC_SUPPORT
//#EXT-X-KEY:METHOD=AES-128,URI="test.key",IV=0x30313032303330343035303630373038
static int update_key_info(HLSContext *context, const char *key_line)
{
    char *str_start = NULL;
    char key_path[HTTP_COMMON_LINE_LENGTH] = {0};
    sscanf(key_line, "%*[^\"]\"%[^\"]", key_path);
    LOG("key path: %s\n", key_path);

    if (-1 == download_key(context, key_path))
    {
        LOG("download key file fail\n");
        return -1;
    }
    if ((str_start = strstr(key_line, "IV=0x")) != NULL ||
            (str_start = strstr(key_line, "IV=0X")) != NULL)
    {
        context->has_ivec = 1;
        ff_hex_to_data(context->ivec, str_start + 5);
        LOG("key: %.16s\n", context->key);
        LOG("ivec: %.16s\n", context->ivec);
    }
    if (NULL != context->dec_handle)
    {
        aes_cbc_destroy(context->dec_handle);
        context->dec_handle = NULL;
    }
    if ((context->dec_handle = aes_cbc_init(context->key, 128)) == NULL)
    {
        LOG("aes cbc init fail\n");
        return -1;
    }
    if (!context->has_ivec)
    {
        memset(context->ivec, 0x0, sizeof (*context->ivec));
    if( context->is_new_seqno==1)
        *((int *)(context->ivec + 12)) = htonl(context->segments[context->segment_idx].seq_no);
    else
        *((int *)(context->ivec + 12)) = htonl(context->download_seq_no);
        
    }
    
    context->is_encrypted = 1;// consider the case: METHOD=NONE

    return 0;
}
#endif


static char *download_m3u8_stream(HLSContext *context)
{
    COMMON_DOWNLOAD_CONTEXT download_context;
    UINT16 ret = 0;
    int retry = 0;

    memset(&download_context, 0x0, sizeof (download_context));
    download_context.buf = NULL;
    download_context.size = 0;
    download_context.offset = 0;
    download_context.context = context;

    context->m3u_length = 0;
    
    CURL *newcurl = curl_easy_init( );
    if (NULL == newcurl) 
    {
        libc_printf("%s:%d: curl_easy_init failed!\n", __FUNCTION__, __LINE__);
        return NULL;
    }
    
    if(context->curl){
        curl_easy_cleanup(context->curl);
    }
    context->curl = newcurl;
    
    curl_easy_reset(context->curl);
    curl_easy_setopt(context->curl, CURLOPT_URL, context->m3u8_url);
    curl_easy_setopt(context->curl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(context->curl, CURLOPT_TIMEOUT, HLS_NETWORK_TIMEOUT);
    curl_easy_setopt(context->curl, CURLOPT_CONNECTTIMEOUT, HLS_NETWORK_CONN_TIMEOUT);
    curl_easy_setopt(context->curl, CURLOPT_NOBODY, 0);
    curl_easy_setopt(context->curl, CURLOPT_WRITEFUNCTION, m3u_download_callback);
    curl_easy_setopt(context->curl, CURLOPT_PROGRESSFUNCTION, curl_progress_cb);
    curl_easy_setopt(context->curl, CURLOPT_PROGRESSDATA, (void *)context);
    curl_easy_setopt(context->curl, CURLOPT_NOPROGRESS, 0);
    curl_easy_setopt(context->curl, CURLOPT_WRITEDATA, &download_context);
	curl_easy_setopt(context->curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(context->curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(context->curl, CURLOPT_USERAGENT, "Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1; Trident/4.0; CIBA)");
    for (retry = 0; retry < 3; retry ++)
    {
        ret = curl_easy_perform(context->curl);
        if (0 == ret)
        {
            break;
        }
        osal_task_sleep(100);
    }
    if (retry >= 3)
    {
        LOG("perform return fail 2, err code=%d\n", ret);
        return NULL;
    }
    
    //buffer context->m3u8_stream can be malloced from m3u_download_callback()
    // when data is coming.
    return context->m3u8_stream;
}


static BOOL set_user_choice(HLSContext *context, int num)
{
    BOOL choice_change = FALSE;
    if(num != context->user_choice)
    {
        choice_change = TRUE;
    }
    context->user_choice = num;
    return choice_change; 
}

//according download speed of network, get corresponding bandwidth variant
static int hls_select_bandwidth_variants(HLSContext *context, UINT32 speed)
{
    int variants_num = -1;
	int i = 0;
	int max = 0;
	int max_idx = 0;
	int diff = 0;

    if ((NULL == context) || (0 == context->variant_nr))
    {
        return variants_num;
    }
    
    //find the max bandwidth of variant stream
	max = context->variants[0].bandwidth;
	for (i = 1; i < context->variant_nr; i ++)
	{
        LOG("bandwidth[%d]: %d!\n",i,context->variants[i].bandwidth);
		if (max < context->variants[i].bandwidth)
		{
			max = context->variants[i].bandwidth;
            max_idx = i;
		}
	}
    if (0 == speed)
    {
        variants_num = max_idx;
        return variants_num;
    }
    
	diff = max;
	//get min difference between download speed and variant bandwidth.
	// download speed faster than bandwidth
	for (i = 0; i < context->variant_nr; i ++)
	{
		if (speed >= context->variants[i].bandwidth)
		{
			if (diff > (speed - context->variants[i].bandwidth))
			{
				diff = speed - context->variants[i].bandwidth;
				variants_num = i;
			}
		}
	}

	//if all bandwidth larger than download speed, get the variant bandwidth of min difference
	if (-1 == variants_num)
	{
		diff = max;
		for (i = 0; i < context->variant_nr; i ++)
		{
			if (speed < context->variants[i].bandwidth)
			{
				if (diff > (context->variants[i].bandwidth - speed))
				{
					diff = context->variants[i].bandwidth - speed;
					variants_num = i;
				}
			}
		}
	}

	//if all bandwidth small than download speed, get max bandwidth
	if (-1 == variants_num)
	{
        variants_num = max_idx;
    }
    
    libc_printf("%s(), current speed: %d, choose bandwidth[%d]: %d!\n", \
        __FUNCTION__, speed, variants_num, context->variants[variants_num].bandwidth);

    if (-1 == variants_num)
    {
        variants_num = 0;
    }
    return variants_num;
}

//get variants_url according to current download speed of network
static int hls_get_variants_url(HLSContext *context, char *variants_url, int url_length, char *input_line)
{
    int ret = -1;
    int variants_num = 0;
    char *variant_path = NULL;
    double speed = 0; //bit
    
    if ((NULL == context) || (NULL == variants_url) || (0 == context->variant_nr))
    {
        return ret;
    }

    /////////////////////////////////////////
    //step 1: get current download speed of network
    //curl_easy_getinfo(context->curl_ts, CURLINFO_SPEED_DOWNLOAD, &speed);
    //speed = win_test_get_speed();
    //speed = speed*8; // bps
    speed = context->dl_speed;
    
    //step 2: compare download speed with bandwidth of variants 
    variants_num = hls_select_bandwidth_variants(context, (UINT32)speed);
    
    //step 3: choose the variants that the bandwidth range within download speed
    variant_path = context->variants[variants_num].url_path;
    ff_make_absolute_url(variants_url, url_length, variant_path, input_line);
    ret = 1;
    return ret;
}

//Set the variant string info for dynamic TS stream switching.
static int hls_set_variants_url(HLSContext *context, char *m3u8_stream)
{
    int line_length = 0;
    int i = 0;
    int ret = -1;
    char cur_line[HTTP_COMMON_LINE_LENGTH] = {0};
    //char new_path[HTTP_COMMON_LINE_LENGTH] = {0};
    
    for (i = 0; i < context->variant_nr; i ++)
    {
        line_length = read_chomp_line(m3u8_stream + context->variants[i].url_pos, cur_line, HTTP_COMMON_LINE_LENGTH);
        if(0 > line_length)
        {
            LOG("###Error,illegal length!\n");
            break;
        }
        if(strstr(cur_line, "://"))
        {
            strncpy(context->variants[i].url_path, cur_line, HTTP_COMMON_LINE_LENGTH);
        }
        else
        {
            ff_make_absolute_url(context->variants[i].url_path, HTTP_COMMON_LINE_LENGTH, context->file->url, cur_line);
        }
        ret = 0;
    }
    return ret;
}


static int variants_choice(HLSContext *context, char *m3u8_stream)
{
    char cur_line[HTTP_COMMON_LINE_LENGTH] = {0};
    char new_path[HTTP_COMMON_LINE_LENGTH] = {0};
    int user_choice_num = 0;
    int line_length = 0;
    int i = 0;
	int max_bandwidth = 0;

    //find the max bandwidth of variant stream
    max_bandwidth = context->variants[0].bandwidth;
	for (i = 1; i < context->variant_nr; i ++)
	{
		if (context->variants[i].bandwidth > max_bandwidth)
		{
			max_bandwidth = context->variants[i].bandwidth;
            user_choice_num = i;
		}
	}

    if(set_user_choice(context, user_choice_num))
    {
        line_length = read_chomp_line(m3u8_stream + context->variants[user_choice_num].url_pos, cur_line, HTTP_COMMON_LINE_LENGTH);
        if(0 > line_length)
        {
            LOG("###Error,illegal length!\n");
            return -1;
        }
        if(strstr(cur_line, "://"))
        {
            //curl_easy_setopt(context->curl, CURLOPT_URL, cur_line);
            strcpy(context->m3u8_url, cur_line);
        }
        else
        {
            hls_get_current_url(context, new_path, HTTP_COMMON_LINE_LENGTH, cur_line);
            //curl_easy_setopt(context->curl, CURLOPT_URL, new_path);
            strcpy(context->m3u8_url, new_path);
        }
    }
    else
    {
        //LOG("user choice no changing!\n");
    }
    return 0;
}

static int is_m3u8_format(const char *m3u8_stream)
{
    /* Require #EXTM3U at the start, and either one of the ones below
    * somewhere for a proper match. */
    if (!strstr(m3u8_stream, "#EXTM3U"))
    {
        return 0;
    }
    if (strstr(m3u8_stream, "#EXT-X-STREAM-INF:") ||strstr(m3u8_stream, "#EXT-X-TARGETDURATION:")
        ||strstr(m3u8_stream, "#EXT-X-MEDIA-SEQUENCE:"))
    {
        return 1;
    }
    return 0;
}


static int playlist_overview(HLSContext *context, char *m3u8_stream,int * sq_num)
{
    char cur_line[HTTP_COMMON_LINE_LENGTH] = {0};
    int offset = 0;
    int line_length = 0;
    double duration = 0;
    int start_seq_no = 0;
//    char *var_bandwidth = NULL;

    LOG("m3u8:\n%s\n", m3u8_stream);
    if(!is_m3u8_format(m3u8_stream))
    {
        return -1;
    }

//!get first sequence num.
    for (offset=0; ; offset+=line_length)
    {
        line_length = read_chomp_line(m3u8_stream + offset, cur_line, HTTP_COMMON_LINE_LENGTH);

        if (0 > line_length)
        {
            LOG("###Error,illegal length!\n");
            return -1;
        }
        if (0 == line_length)
        {
            break;
        }

        if (NULL != strstr(cur_line, "#EXT-X-MEDIA-SEQUENCE:"))
        {
            sscanf(cur_line, "%*[^:]:%d", &start_seq_no);
            break;
        }
    }
    *sq_num = start_seq_no;
        
//!check basic infor
    for (offset=0; ; offset+=line_length)
    {
        line_length = read_chomp_line(m3u8_stream + offset, cur_line, HTTP_COMMON_LINE_LENGTH);

        if(0 > line_length)
        {
            LOG("###Error,illegal length!\n");
            return -1;
        }
        if (0 == line_length)
        {
            break;
        }

        if (NULL != strstr(cur_line, "#EXTINF:"))
        {
            sscanf(cur_line, "%*[^:]:%lf", &duration);
            context->duration += duration;
            ++context->segment_nr;
        }
        else if(NULL != strstr(cur_line, "#EXT-X-STREAM-INF:"))
        {
            ++context->variant_nr;
            context->variant_m3u = 1;
        }
        else if (NULL != strstr(cur_line, "#EXT-X-ENDLIST"))
        {
            context->is_continue = 0;
            LOG("###VOD m3u8:%d\n",context->is_continue);
            break;
        }
    }
    if ((context->is_continue) && (context->duration > 0))
    {
        context->duration = (double)(-1);
    }

//!alloc memory for variants,segments infor saving
    if(context->segment_nr)
    {
        context->segments = (segment *)malloc(sizeof (segment) * context->segment_nr);
        if (NULL == context->segments)
        {
            LOG("malloc segments fail\n");
            return -1;
        }
        memset(context->segments, 0x0, sizeof (segment) * context->segment_nr);
    }
    if(context->variant_m3u)
    {
        if (context->variants)
        {
            free(context->variants);
            context->variants = NULL;
        }
        
        context->variants = (variants *)malloc(sizeof (variants) * context->variant_nr);
        if (NULL == context->variants)
        {
            LOG("malloc segments fail\n");
            return -1;
        }
        memset(context->variants, 0x0, sizeof (variants) * context->variant_nr);
    }
    return 0;
}

static int parse_playlist(HLSContext *context, char *m3u8_stream)
{
    char cur_line[HTTP_COMMON_LINE_LENGTH] = {0};
    int offset = 0;
    int line_length = 0;
    int start_seq_no = 0;//jary test
    char *var_bandwidth = NULL;
    int key_pos = 0;
    int i = 0;
    int j = 0;
    char *key_offset = NULL;
    
//!free or reset old data
    if(context->segments)
    {
        free(context->segments);
    }
    context->segments = NULL;
    context->segment_nr = 0;
    context->duration = 0;
    context->variant_m3u = 0;
    context->is_continue = 1;

//!overview the new m3u8    
    if(-1 == playlist_overview(context,m3u8_stream,&start_seq_no))
    {
        return -1;
    }

//!get start sequence number and check if there any changes  
    if(start_seq_no != context->start_seq_no)
    {
        context->is_new_seqno = 1;//!it's a new m3u8 file
        
        //some HLS live stream will loop back the start_seq_no,
        //for example: the start_seq_no is 400, but next time it may
        // change to 1 to loop back. so we force context->download_seq_no
        // also re-count again.
        if (start_seq_no < context->start_seq_no)
        {
            context->download_seq_no = start_seq_no;
        }
        context->start_seq_no = start_seq_no;
    }
    else
    {
        context->is_new_seqno = 0;
    }
       
    LOG("start squence no:%d\n", context->start_seq_no);
    LOG("duration: %lf\n", context->duration);
    LOG("segment number: %d\n", context->segment_nr);

#if 0
    //Init the first seq_no, may update_key_info() should use seq_no when
    //it is encrypt stream.
    if (context->segments)
    {
        context->segments[context->segment_idx].seq_no = context->start_seq_no;
        libc_printf("%s:%d: modify:%p,%d,index:%d\n", __FUNCTION__, __LINE__, 
            &context->segments[context->segment_idx],
            context->start_seq_no,
            context->segment_idx);
    }
#endif

//!parse other's infor,and save variants/segments infor  
    for (offset=0; ; offset+=line_length)
    {
        line_length = read_chomp_line(m3u8_stream + offset, cur_line, HTTP_COMMON_LINE_LENGTH);

        if(0 > line_length)
        {
            LOG("###Error,illegal length!\n");
            return -1;
        }
        if (0 == line_length)
        {
            break;
        }

        if (NULL != strstr(cur_line, "#EXT-X-VERSION"))
        {
            //not process
        }
        else if (NULL != strstr(cur_line, "#EXT-X-PLAYLIST-TYPE"))
        {
            //not process
        }
        else if (NULL != strstr(cur_line, "#EXT-X-STREAM-INF:"))
        {
            LOG("multi-rate m3u8 file\n");
            var_bandwidth = strstr(cur_line, "BANDWIDTH=");
            if(NULL != var_bandwidth)
            {
                context->variants[j].bandwidth = ATOI(var_bandwidth+10);
            }
            else
            {
                LOG("error,no bandwidth infor!\n");
            }
            context->variants[j].url_pos = offset+line_length;
            j++;           
        }
        else if (NULL != strstr(cur_line, "#EXT-X-I-FRAME-STREAM-INF:"))
        {
            //not process...
        }
        else if(NULL != strstr(cur_line, "#EXT-X-DISCONTINUITY"))
        {
            //not process...
        }
        else if (NULL != strstr(cur_line, "#EXT-X-PROGRAM-DATA-TIME"))
        {
            //not process
        }
        else if (NULL != strstr(cur_line, "#EXT-X-KEY:"))
        {
            key_pos = offset;
        #ifdef DSC_SUPPORT
            context->cur_key_pos = offset;
        #endif
            if(NULL != strstr(cur_line, "METHOD=NONE"))
            {
                continue;
            }

            /*
        #ifdef DSC_SUPPORT
            if (-1 == update_key_info(context, cur_line))
            {
                LOG("update key info fail\n");
                return -1;
            }
        #endif
            */
            if (NULL == key_offset)
            {
                key_offset = m3u8_stream + offset;
            }
        }
        else if (NULL != strstr(cur_line, "#EXT-X-TARGETDURATION:"))
        {
            sscanf(cur_line, "%*[^:]:%d", &context->target_duration);
        }
        else if (NULL != strstr(cur_line, "#EXTINF:"))
        {
            sscanf(cur_line, "%*[^:]:%lf", &context->segments[i].duration);
            context->segments[i].pos = offset;
            context->segments[i].key_pos = key_pos;
            context->segments[i].seq_no = start_seq_no++;
            ++i;
        }
        else if (NULL != strstr(cur_line, "#"))
        {
            LOG("comment line, ignored:[%s]\n", cur_line);
        }
        else
        {
            //LOG("Not Proc:[%s]\n",cur_line);
        }
    }

//!first download postion of segments in live play, should be one of the last 3 segments in m3u8 file 
    if ((0 == context->download_seq_no) && (context->is_continue))
    {
        if(context->segment_nr >= 3)
        {
            context->download_seq_no = context->start_seq_no + context->segment_nr - 3;
            context->cur_pos = context->segments[context->segment_nr - 3].pos;

            context->segment_idx = context->segment_nr - 3;
        }
        else
        {
            context->download_seq_no = context->start_seq_no;
        }
        LOG("first download_seq_no:[%d]\n",context->download_seq_no);
    }

//!get the download position for the update m3u8
    for(i= 0; i< context->segment_nr; i++)
    {
        if(((context->segments[i].seq_no) >= (context->download_seq_no)) && (0 !=context->download_seq_no))
        {
            context->cur_pos = context->segments[i].pos;
            context->download_seq_no = context->segments[i].seq_no;
            LOG("next download_seq_no:[%d]\n",context->download_seq_no);
            context->segment_idx = i;
            break;
        }
    }

    if (key_offset)
    {
    #ifdef DSC_SUPPORT
        line_length = read_chomp_line(key_offset, cur_line, HTTP_COMMON_LINE_LENGTH);    
        if(line_length <= 0)
        {
            LOG("###Error,illegal length!\n");
            return -1;
        }
        if (-1 == update_key_info(context, cur_line))
        {
            LOG("update key info fail\n");
            return -1;
        }
    #endif
    }

    return 0;
}


static void hls_m3u8_process(void *param)
{
    HLSContext *context = NULL;
    int reload_interval = 0;
    int i = 0;
    char *m3u8_stream = NULL;
    UINT32 try_cnt = 0;
    urlfile *file = NULL;
    uf_hlspriv *hlspriv = NULL;

    context = (HLSContext *)param;
    if (NULL == context)
    {
        return;
    }
    file = context->file;
    hlspriv = file->private;
    while((!context->exit_flag) && (!context->hls_m3u8_eof))
    {

        LOG("m3u8 process!\n");
//!download m3u8 file    
        if (hlspriv->ref.buff)
        {
            //use the identify http data for m3u file data,
            // avoid a duplicated m3u file downloading.
            m3u8_stream = hlspriv->ref.buff;
            hlspriv->ref.buff = NULL;
            context->m3u8_stream = m3u8_stream;
        }
        else
        {
            download_m3u8_stream(context);
            m3u8_stream = context->m3u8_stream;
        }

        MutexLock(&context->lock);
        if (NULL == m3u8_stream) 
        {
            LOG("*****download_m3u8_stream error!, try :%d\n", try_cnt);
            MutexUnlock(&context->lock);
            if (try_cnt ++ < HLS_CONNECT_TYR_CNT)
            {
                osal_task_sleep(200);
                continue;
            }
            context->hls_m3u8_eof = 1;
            break;
        }
//!parse new m3u8 file        
        if (-1 == parse_playlist(context, m3u8_stream)) 
        {
            LOG("*****parse_playlist error!\n");
            MutexUnlock(&context->lock);
            if(!context->is_continue)
            {
                context->hls_m3u8_eof = 1;
                break;
            }
            continue;
        }

        if(context->variant_m3u)
        {
            if(0 != variants_choice(context,m3u8_stream))//check user choice for variants source
            {
                context->hls_m3u8_eof = 1;
                break;
            }
            hls_set_variants_url(context,m3u8_stream);
            MutexUnlock(&context->lock);
            continue;
        }

       
        MutexUnlock(&context->lock);
        
        if(!context->is_continue)
        {
            break;
        }
        
//!wait for next m3u8 download
        if(context->is_new_seqno)
        {
            reload_interval = context->segment_nr > 0 ? 
            context->segments[context->segment_nr - 1].duration : context->target_duration;
        }
        else
        {
            reload_interval = context->target_duration / 2;
        }
        
        for(i=0; i<reload_interval*100; i++)
        {
            
            if(context->exit_flag)
            {
                break;
            }
            osal_task_sleep(10);
        }
    }

    MutexLock(&context->lock);
    context->tid_m3u8_exit = 1;
    MutexUnlock(&context->lock);

}

static int hls_m3u8_parse_thread(HLSContext *context)
{
	ID parse_id = OSAL_INVALID_ID;
	OSAL_T_CTSK		t_ctsk;

    memset(&t_ctsk, 0, sizeof(t_ctsk));    
	t_ctsk.stksz	= 0x4000;
	t_ctsk.quantum	= 10;
	t_ctsk.itskpri	= OSAL_PRI_NORMAL;
	t_ctsk.name[0]	= 'H';
	t_ctsk.name[1]	= 'L';
	t_ctsk.name[2]	= 'S';
	t_ctsk.para1 = (UINT32)(context);

	t_ctsk.task = (FP)hls_m3u8_process;


	parse_id = osal_task_create(&t_ctsk);
	if(OSAL_INVALID_ID == parse_id)
	{
		//delete control_mbf_id
		LOG("create HLS parse failed\n");
		return -1;
	}

//    osal_task_sleep(1000);
    return 0;
    
}

static void cleanup(HLSContext *context)
{
    if (NULL != context)
    {
        MutexDestroy(&context->lock);

        if (context->m3u8_stream)
        {
            free(context->m3u8_stream);
            context->m3u8_stream = NULL;
        }
        if(context->segments)
        {
            free(context->segments);
            context->segments = NULL;
            context->segment_nr = 0;
        }
        if(context->variants)
        {
            free(context->variants);
            context->variants = NULL;
            context->variant_nr = 0;
        }

        if (NULL != context->curl)
        {
            curl_easy_cleanup(context->curl);
            context->curl = NULL;
        }
        if(NULL != context->m3u8_url)
        {
            FREE(context->m3u8_url);
            context->m3u8_url = NULL;
        }
    #ifdef DSC_SUPPORT
        if (NULL != context->dec_handle)
        {
            aes_cbc_destroy(context->dec_handle);
            context->dec_handle = NULL;
        }
    #endif        
        LOG("...cleanup\n");
    #ifdef IS_RECORD_FILE
        if (NULL != context->stream)
        {
            LOG("close record file\n");
            fclose(context->stream);
            //fsync("/mnt/uda1/ALIDVRS2");
            fsync("/mnt/uda1");
            _fsync_root("/mnt/uda1/ALIDVRS2");
            //fs_unmount("/mnt/uda1", 1);
        }
    #endif
        free(context);
    }
}

/*
int is_http_live_stream(const char *url)
{
    LOG("HLS Module Version: %s\n", "aliVersion1");

    char probe_buf[HLS_PROBE_BUF_SIZE] = {0};
    int ret = 0;

    if (-1 == http_download(url, probe_buf, HLS_PROBE_BUF_SIZE - 1)) {
        LOG("m3u8 download fail\n");
        return ret;
    }

    LOG("m3u8:\n%s\n", probe_buf);
    ret = is_m3u8_format(probe_buf);
    m_is_hls = ret;
    
    return ret;
}
*/

UINT32 hls_get_duration(HLSContext *context)
{
    UINT32 duration = 0;
    if(NULL == context)
    {
        return 0;
    }
    MutexLock(&context->lock);
    duration = (UINT32)context->duration;
    MutexUnlock(&context->lock);
    return duration;
}

int hls_get_segment_url(HLSContext *context, char *segment_url)
{
	char cur_line[HTTP_COMMON_LINE_LENGTH]={0};
	int line_length = 0;     
    int ret = URL_SEGMENT_NO;

    if (NULL == segment_url)
    {
        return URL_SEGMENT_EXIT;
    }

    if(context->exit_flag)
    {
        return URL_SEGMENT_EXIT;
    }
        
    MutexLock(&context->lock);

    // the content of variant m3u file(1 layer m3u file) is ignored.
    if((NULL == context->m3u8_stream) || (1 == context->variant_m3u))
    {
        MutexUnlock(&context->lock);   
        osal_task_sleep(5);
        return URL_SEGMENT_NO;
    }
    line_length = read_chomp_line(context->m3u8_stream + context->cur_pos, cur_line, HTTP_COMMON_LINE_LENGTH);

    if(0 > line_length)
    {
        context->hls_m3u8_eof = 1;
        MutexUnlock(&context->lock);            
        return URL_SEGMENT_EXIT;
    }

	if (0 == line_length)
    {
        //read the end of m3u8 data. if playing live programme. we should
        //continue update m3u8 files, so it may sleep to wait  m3u8 file updated.
        MutexUnlock(&context->lock);            
        osal_task_sleep(5);
        return URL_SEGMENT_NO;
	}

	LOG("line length: %d, cur line: %s\n", line_length, cur_line);
    context->cur_pos += line_length;

    if (NULL != strstr(cur_line, "#EXTINF"))
    {
        context->is_segment = 1;
        ret = URL_SEGMENT_NO;
    }
    else if (NULL != strstr(cur_line, "#EXT-X-ENDLIST"))
    {
        LOG("list end!\n");
        ret = URL_SEGMENT_EXIT;
    }
    else if (NULL != strstr(cur_line, "#"))
    {
        LOG("comment line, it's ignored, line length: %d\n", line_length);
        ret = URL_SEGMENT_NO;
    }
    else if (cur_line[0])
    {		
        if (context->is_segment)
        {
            context->is_segment = 0;
            //NOTE: change the segment URL 
          #ifdef  VARIANTS_SWITCH
            if (context->variant_nr)
            {
                hls_get_variants_url(context, segment_url, HTTP_COMMON_LINE_LENGTH, cur_line);
            }
            else
          #endif
            {
                hls_get_current_url(context, segment_url, HTTP_COMMON_LINE_LENGTH, cur_line);
            }
        #ifdef DSC_SUPPORT
            if (context->is_encrypted)
            {
                if (context->segments[context->segment_idx].key_pos != context->cur_key_pos)
                {
                    //char key_line[HTTP_COMMON_LINE_LENGTH] = {0};
                    int line_length = 0;

                    // update the key pos
                    LOG("update key info, cur key pos: %d, seg key pos: %d\n", context->cur_key_pos,
                                context->segments[context->segment_idx].key_pos);

                    context->cur_key_pos = context->segments[context->segment_idx].key_pos;

                    memset(cur_line, 0, sizeof(cur_line));
                    line_length = read_chomp_line(context->m3u8_stream + context->segments[context->segment_idx].key_pos, 
                        cur_line, HTTP_COMMON_LINE_LENGTH);
                    if(0 >= line_length)
                    {
                        context->hls_m3u8_eof = 1;
                        MutexUnlock(&context->lock);	
                        return URL_SEGMENT_EXIT;
                    }

                    if (NULL != strstr(cur_line, "#EXT-X-KEY:"))
                    {
                        if(NULL != strstr(cur_line, "METHOD=NONE"))
                        {
                            LOG("switch to clear stream!\n");
                            context->is_encrypted = 0;
                        }
                        else
                        {
                            if (-1 == update_key_info(context, cur_line))
                            {
                                LOG("update key info fail\n");
                                MutexUnlock(&context->lock); 
                                context->hls_m3u8_eof = 1;
                                return URL_SEGMENT_EXIT;
                            } 
                        }
                    }
                    else
                    {
                        MutexUnlock(&context->lock); 
                        context->hls_m3u8_eof = 1;
                        return URL_SEGMENT_EXIT;
                    }
                }
                
            }
            context->cur_size = 0;
        #endif
            LOG("Now download_seq_no:%d, segment_idx:%d\n",
            context->download_seq_no, context->segment_idx);
            //context->download_seq_no ++;

            ret = URL_SEGMENT_NORMAL;
            if((0 == context->is_continue) && 
                (context->download_seq_no == context->segments[context->segment_nr -1].seq_no))
            {
                context->hls_m3u8_eof = 1;
                //ret = URL_SEGMENT_FINISH;
            }
            context->download_seq_no ++;

            ++context->segment_idx;
        }			
    }
    else
    {
        LOG("not proc line, it's ignored\n");			
    }
    
    MutexUnlock(&context->lock);            
	return ret;
    
}


void hls_close(HLSContext *context)
{
    //user_choice = 0xFFFFFFFF;
    if (NULL == context)
        return;
    
    MutexLock(&context->lock);
    context->exit_flag = 1;
    LOG("***exit_flag\n");


    MutexUnlock(&context->lock);

    while(!context->tid_m3u8_exit)
    {
        osal_task_sleep(10);
    }

    cleanup(context);
}


HLSContext *hls_open(urlfile *file)
{
    HLSContext *context = NULL;
    char *path = NULL;
    uf_hlspriv *hlspriv = NULL;

    hlspriv = file->private;
    path = file->url;
     context = (HLSContext *)malloc(sizeof (HLSContext));
    if (NULL == context) {
        LOG("alloc hls context fail\n");
        return NULL;
    }
    memset(context, 0x0, sizeof (HLSContext));
    
    context->m3u8_url = MALLOC(UF_URL_LEN);
    if(context->m3u8_url == NULL){
        LOG("alloc url buffer failed\n");
        return NULL;
    }
    MEMSET(context->m3u8_url,0,UF_URL_LEN);
    strncpy(context->m3u8_url, path, UF_URL_LEN - 1);
    
    MutexInit(&context->lock);
    if (hlspriv && hlspriv->m3u_curl)
    {
        context->curl = hlspriv->m3u_curl;
        hlspriv->m3u_curl = NULL;
        //curl_easy_reset(context->curl);
    }
    else
    {
        context->curl = curl_easy_init( );
        //curl_easy_reset(context->curl);
        //NOTE: set same URL also may change getting the data of m3u file
        //curl_easy_setopt(context->curl, CURLOPT_URL, path);
    }
    if (NULL == context->curl) 
    {
        free(context);
        return NULL;
    }
    context->curl_ts = file->curl;
    context->file = file;
    context->user_choice=-1;
    curl_easy_setopt(context->curl, CURLOPT_FOLLOWLOCATION, 1);//jary
    curl_easy_setopt(context->curl, CURLOPT_TIMEOUT, HLS_NETWORK_TIMEOUT);
    curl_easy_setopt(context->curl, CURLOPT_CONNECTTIMEOUT, HLS_NETWORK_CONN_TIMEOUT);
    if (0 != hls_m3u8_parse_thread(context))
    {
        free(context);
        return NULL;
    }
 
    return context;
}


//so far, HLS only support time seek.
int hls_stream_seek(HLSContext *context, long time)
{
    int i = 0;
    double sum = 0.0;
    uf_hlspriv *priv = NULL;


    if (NULL == context) 
    {
    	return -1;
    }

    priv = context->file->private;

    MutexLock(&context->lock);

    for (i=0, sum=0.0; i<context->segment_nr; i++)
    {
        sum += context->segments[i].duration;
        if (sum > time)
        {
            priv->ts_duration = context->segments[i].duration;
            priv->ts_time = priv->ts_duration - (sum - time);
            
            //move to here, when comfirm seek, clear ring buffer at once.
            UF_LOCK_WRITE(context->file);
            uf_state(context->file, UF_ROPEN);      //reopen the url. Exit data transfer at once
            context->file->size = -1;
            rbuffer_clean(&context->file->buffer); //clean old data
            UF_UNLOCK_WRITE(context->file);

            context->is_do_seek = 1;
            break;
        }
    }
    if (i == context->segment_nr) 
    {
    	--i;
    }

    LOG("seek i: %d, offset:%2.2d:%2.2d:%2.2d\n", i, time/3600, (time%3600)/60, time%60);

    context->segment_idx = i; 
    context->cur_pos = context->segments[context->segment_idx].pos;
    context->download_seq_no = context->segments[context->segment_idx].seq_no;
#ifdef DSC_SUPPORT
    if (context->segments[context->segment_idx].key_pos != context->cur_key_pos)
    {
        char cur_line[HTTP_COMMON_LINE_LENGTH] = {0};
        int line_length = 0;

        // update the key pos
        LOG("update key info, cur key pos: %d, seg key pos: %d\n", context->cur_key_pos,
            context->segments[context->segment_idx].key_pos);

        context->cur_key_pos = context->segments[context->segment_idx].key_pos;

        line_length = read_chomp_line(context->m3u8_stream + context->cur_key_pos, cur_line, HTTP_COMMON_LINE_LENGTH);

        if (0 >= line_length) 
        {		
            MutexUnlock(&context->lock);	
            return -1;
        }

        if (NULL != strstr(cur_line, "#EXT-X-KEY:"))
        {
            if(NULL != strstr(cur_line, "METHOD=NONE"))
            {
                LOG("switch to clear stream!\n");
                context->is_encrypted = 0;
            }
            else
            {
                if (-1 == update_key_info(context, cur_line))
                {
                    LOG("update key info fail\n");
                    MutexUnlock(&context->lock); 
                    return -1;
                } 
            }
        }

    }
#endif

    MutexUnlock(&context->lock);

    LOG("did seek\n");

    return 0;
}


#endif

