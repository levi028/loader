#ifndef _LIVE555_PORTING_API_H_
#define _LIVE555_PORTING_API_H_
#include <basic_types.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define live555_print(...)  do{} while (0)
//libc_printf

#define RTSP_LIVE555_SET_PRIV_DATA                  0x01
#define RTSP_LIVE555_SET_DATA_CALLBACK              0x02
/* The following just for debug */
#define RTSP_LIVE555_SET_DEBUG_CALLBACK             0x03
#define RTSP_LIVE555_SET_DB_V_DATA_CALLBACK         0x04
#define RTSP_LIVE555_SET_DB_A_DATA_CALLBACK         0x05
#define RTSP_LIVE555_SET_DB_ALIP_V_DATA_CALLBACK    0x06
#define RTSP_LIVE555_SET_DB_ALIP_A_DATA_CALLBACK    0x07
#define RTSP_LIVE555_SET_WAIT_DATA_TIMEOUT          0x08

#define RTSP_LIVE555_DEFAULT_WAIT_DATA_TIMEOUT      10*1000000 // 10 SECONDS
/* rtsp_live555_open
 * Send option, describe
 * When success, it will create one RTSP Client object and return it.
 */
void* rtsp_live555_open(char *rtsp_url);

/* rtsp_live555_play
 * This API will cause send SETUP and PLAY command to server, it will choice appropriate UDP/TCP connection to server.
 * BE NOTE that, it will dive into data loop if play success, and won't return until stop play.
 */
int rtsp_live555_play(void* client);

/* rtsp_live555_seek
 * Seek to the time that specified if this session is seekable. It will case PAUSE first, then PLAY to the time again.
 */
int rtsp_live555_seek(void *client, double seek_time);

/* rtsp_live555_pause
 * Just send PAUSE command to server.
 */
int rtsp_live555_pause(void *client);

/* rtsp_live555_resume
 * Resume play from PAUSE, it just send PLAY to the time.
 */
int rtsp_live555_resume(void *client, double play_time);

/* rtsp_live555_stop
 * It will send TEARDOWN to server, and signal exit data loop.
 */
int rtsp_live555_stop(void *client);

/* rtsp_live555_close
 * It will free all client resource.
 */
int rtsp_live555_close(void *client);

/* rtsp_live555_set_opt
 * This API use to set some important parameter to porting layer.
 * RTSP_LIVE555_SET_PRIV_DATA: use to identify user, default use URLFILE handle, it will send back with data;
 * RTSP_LIVE555_SET_DATA_CALLBACK: the callback use to recieve data.
 * return 0, success
 *        -1, client is NULL
 *        -2, no this param
 */
int rtsp_live555_set_opt(void *client, int type, void* param);

/* rtsp_live555_get_duration
 * Get stream's duration after rtsp_live555_open
 */
BOOL rtsp_live555_get_duration(void *client, double *val);

#ifdef __cplusplus
}
#endif

#endif //_LIVE555_PORTING_API_H_


