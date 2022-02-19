/***************************************************************************
 *                      _   _ ____  _
 *  Project         ___| | | |  _ \| |
 *                 / __| | | | |_) | |
 *                | (__| |_| |  _ <| |___
 *                 \___|\___/|_| \_\_____|
 *
 * Copyright (C) 2010, Howard Chu, <hyc@highlandsun.com>
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at http://curl.haxx.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ***************************************************************************/

#include "setup.h"

#ifdef USE_LIBRTMP

#include "urldata.h"
#include "nonblock.h" /* for curlx_nonblock */
#include "progress.h" /* for Curl_pgrsSetUploadSize */
#include "transfer.h"
#include "warnless.h"
#include <api/libcurl/curl.h>
#include "librtmp/rtmp.h"
#include "librtmp/log.h"

#define _MPRINTF_REPLACE /* use our functions only */
#include <api/libcurl/mprintf.h>

#include "curl_memory.h"
/* The last #include file should be: */
#include "memdebug.h"
#include "urldata.h"

#ifdef _WIN32
#define setsockopt(a,b,c,d,e) (setsockopt)(a,b,c,(const char *)d,(int)e)
#define SET_RCVTIMEO(tv,s)   int tv = s*1000
#else
#define SET_RCVTIMEO(tv,s)   int tv = s*1000  	// NOTE: need modify it when upgrade libcurl version
												//struct timeval tv = {s,0}
#endif

#define DEF_BUFTIME    (2*60*60*1000)    /* 2 hours */

static CURLcode rtmp_setup(struct connectdata *conn);
static CURLcode rtmp_do(struct connectdata *conn, bool *done);
static CURLcode rtmp_done(struct connectdata *conn, CURLcode, bool premature);
static CURLcode rtmp_connect(struct connectdata *conn, bool *done);
static CURLcode rtmp_disconnect(struct connectdata *conn, bool dead);

static Curl_recv rtmp_recv;
static Curl_send rtmp_send;

/*
 * RTMP protocol handler.h, based on http://rtmpdump.mplayerhq.hu
 */

const struct Curl_handler Curl_handler_rtmp = {
  "RTMP",                               /* scheme */
  rtmp_setup,                           /* setup_connection */
  rtmp_do,                              /* do_it */
  rtmp_done,                            /* done */
  ZERO_NULL,                            /* do_more */
  rtmp_connect,                         /* connect_it */
  ZERO_NULL,                            /* connecting */
  ZERO_NULL,                            /* doing */
  ZERO_NULL,                            /* proto_getsock */
  ZERO_NULL,                            /* doing_getsock */
  ZERO_NULL,                            /* domore_getsock */
  ZERO_NULL,                            /* perform_getsock */
  rtmp_disconnect,                      /* disconnect */
  ZERO_NULL,                            /* readwrite */
  PORT_RTMP,                            /* defport */
  CURLPROTO_RTMP,                       /* protocol */
  PROTOPT_NONE                          /* flags*/
};

const struct Curl_handler Curl_handler_rtmpt = {
  "RTMPT",                              /* scheme */
  rtmp_setup,                           /* setup_connection */
  rtmp_do,                              /* do_it */
  rtmp_done,                            /* done */
  ZERO_NULL,                            /* do_more */
  rtmp_connect,                         /* connect_it */
  ZERO_NULL,                            /* connecting */
  ZERO_NULL,                            /* doing */
  ZERO_NULL,                            /* proto_getsock */
  ZERO_NULL,                            /* doing_getsock */
  ZERO_NULL,                            /* domore_getsock */
  ZERO_NULL,                            /* perform_getsock */
  rtmp_disconnect,                      /* disconnect */
  ZERO_NULL,                            /* readwrite */
  PORT_RTMPT,                           /* defport */
  CURLPROTO_RTMPT,                      /* protocol */
  PROTOPT_NONE                          /* flags*/
};

const struct Curl_handler Curl_handler_rtmpe = {
  "RTMPE",                              /* scheme */
  rtmp_setup,                           /* setup_connection */
  rtmp_do,                              /* do_it */
  rtmp_done,                            /* done */
  ZERO_NULL,                            /* do_more */
  rtmp_connect,                         /* connect_it */
  ZERO_NULL,                            /* connecting */
  ZERO_NULL,                            /* doing */
  ZERO_NULL,                            /* proto_getsock */
  ZERO_NULL,                            /* doing_getsock */
  ZERO_NULL,                            /* domore_getsock */
  ZERO_NULL,                            /* perform_getsock */
  rtmp_disconnect,                      /* disconnect */
  ZERO_NULL,                            /* readwrite */
  PORT_RTMP,                            /* defport */
  CURLPROTO_RTMPE,                      /* protocol */
  PROTOPT_NONE                          /* flags*/
};

const struct Curl_handler Curl_handler_rtmpte = {
  "RTMPTE",                             /* scheme */
  rtmp_setup,                           /* setup_connection */
  rtmp_do,                              /* do_it */
  rtmp_done,                            /* done */
  ZERO_NULL,                            /* do_more */
  rtmp_connect,                         /* connect_it */
  ZERO_NULL,                            /* connecting */
  ZERO_NULL,                            /* doing */
  ZERO_NULL,                            /* proto_getsock */
  ZERO_NULL,                            /* doing_getsock */
  ZERO_NULL,                            /* domore_getsock */
  ZERO_NULL,                            /* perform_getsock */
  rtmp_disconnect,                      /* disconnect */
  ZERO_NULL,                            /* readwrite */
  PORT_RTMPT,                           /* defport */
  CURLPROTO_RTMPTE,                     /* protocol */
  PROTOPT_NONE                          /* flags*/
};

const struct Curl_handler Curl_handler_rtmps = {
  "RTMPS",                              /* scheme */
  rtmp_setup,                           /* setup_connection */
  rtmp_do,                              /* do_it */
  rtmp_done,                            /* done */
  ZERO_NULL,                            /* do_more */
  rtmp_connect,                         /* connect_it */
  ZERO_NULL,                            /* connecting */
  ZERO_NULL,                            /* doing */
  ZERO_NULL,                            /* proto_getsock */
  ZERO_NULL,                            /* doing_getsock */
  ZERO_NULL,                            /* domore_getsock */
  ZERO_NULL,                            /* perform_getsock */
  rtmp_disconnect,                      /* disconnect */
  ZERO_NULL,                            /* readwrite */
  PORT_RTMPS,                           /* defport */
  CURLPROTO_RTMPS,                      /* protocol */
  PROTOPT_NONE                          /* flags*/
};

const struct Curl_handler Curl_handler_rtmpts = {
  "RTMPTS",                             /* scheme */
  rtmp_setup,                           /* setup_connection */
  rtmp_do,                              /* do_it */
  rtmp_done,                            /* done */
  ZERO_NULL,                            /* do_more */
  rtmp_connect,                         /* connect_it */
  ZERO_NULL,                            /* connecting */
  ZERO_NULL,                            /* doing */
  ZERO_NULL,                            /* proto_getsock */
  ZERO_NULL,                            /* doing_getsock */
  ZERO_NULL,                            /* domore_getsock */
  ZERO_NULL,                            /* perform_getsock */
  rtmp_disconnect,                      /* disconnect */
  ZERO_NULL,                            /* readwrite */
  PORT_RTMPS,                           /* defport */
  CURLPROTO_RTMPTS,                     /* protocol */
  PROTOPT_NONE                          /* flags*/
};

#ifdef RTMP_OPTIONS_SUPPORT
#define DEF_TIMEOUT 30  /* seconds */
#define DEF_SKIPFRM 0
#define STR2AVAL(av,str)    av.av_val = str; av.av_len = strlen(av.av_val)
#define HEX2BIN(a)      (((a)&0x40)?((a)&0xf)+9:((a)&0xf))

struct rtmp_option
{
    /* name of long option */
    const char *name;
    /*
     * one of no_argument, required_argument, and optional_argument:
     * whether option takes an argument
     */
    int has_arg;
    /* if not NULL, set *flag to val when option found */
    int *flag;
    /* if flag not NULL, value to set *flag to; else return value */
    int val;
};

static const AVal av_onMetaData = AVC("onMetaData");
static const AVal av_duration = AVC("duration");
static const AVal av_conn = AVC("conn");
static const AVal av_token = AVC("token");
static const AVal av_playlist = AVC("playlist");
static const AVal av_true = AVC("true");

static struct rtmp_option rtmp_options[] =
{
    {"help", 0, NULL, 'h'},
    {"host", 1, NULL, 'n'},
    {"port", 1, NULL, 'c'},
    {"socks", 1, NULL, 'S'},
    {"protocol", 1, NULL, 'l'},
    {"playpath", 1, NULL, 'y'},
    {"playlist", 0, NULL, 'Y'},
    {"rtmp", 1, NULL, 'r'},
    {"swfUrl", 1, NULL, 's'},
    {"tcUrl", 1, NULL, 't'},
    {"pageUrl", 1, NULL, 'p'},
    {"app", 1, NULL, 'a'},
    {"auth", 1, NULL, 'u'},
    {"conn", 1, NULL, 'C'},
#ifdef CRYPTO
    {"swfhash", 1, NULL, 'w'},
    {"swfsize", 1, NULL, 'x'},
    {"swfVfy", 1, NULL, 'W'},
    {"swfAge", 1, NULL, 'X'},
#endif
    {"flashVer", 1, NULL, 'f'},
    {"live", 0, NULL, 'v'},
    {"flv", 1, NULL, 'o'},
    {"resume", 0, NULL, 'e'},
    {"timeout", 1, NULL, 'm'},
    {"buffer", 1, NULL, 'b'},
    {"skip", 1, NULL, 'k'},
    {"subscribe", 1, NULL, 'd'},
    {"start", 1, NULL, 'A'},
    {"stop", 1, NULL, 'B'},
    {"token", 1, NULL, 'T'},
    {"hashes", 0, NULL, '#'},
    {"debug", 0, NULL, 'z'},
    {"quiet", 0, NULL, 'q'},
    {"verbose", 0, NULL, 'V'},
    {0, 0, 0, 0}
};

int rtmp_hex2bin(char *str, char **hex)
{
    char *ptr;
    int i, l = strlen(str);

    if (l & 1)
    {
        return 0;
    }

    *hex = malloc(l / 2);
    ptr = *hex;
    if (!ptr)
    {
        return 0;
    }

    for (i = 0; i < l; i += 2)
    {
        *ptr++ = (HEX2BIN(str[i]) << 4) | HEX2BIN(str[i + 1]);
    }
    return l / 2;
}

int rtmp_optcmp(struct rtmp_option *opt, char *arg, char **val)
{
    int ret = 0;
    int optlen = strlen(opt->name);

    if (ret == 0)
    {
        if (0 == strncasecmp(opt->name, arg, optlen) && (arg[optlen] == '='))
        {
            *val = arg + optlen + 1;
            ret = 1;
        }
    }
    if (ret == 0)
    {
        if (arg[0] == '-' && arg[1] == '-' && 0 == strncasecmp(opt->name, arg + 2, optlen))
        {
            ret = 1;
        }
    }
    if (ret == 0)
    {
        if (arg[0] == '-' && opt->val == arg[1])
        {
            ret = 1;
        }
    }

    return ret;
}

static int rtmp_check_option(char *arg, char **val)
{
    int ret = 0;
    struct rtmp_option *opt = rtmp_options;

    for (; (opt) && (opt->name); opt++)
    {
        if (rtmp_optcmp(opt, arg, val))
        {
            ret = opt->val;
            break;
        }
    }
    return ret;
}

static int rtmp_build_argv(char **argv, int cnt, char *param)
{
    int   argc = 0;
    char  dquote = 0;
    char *ptr = param;

    if (param == NULL || argv == NULL) { return 0; }

    argv[0] = "-r";
    argc++;

    while (argc < cnt)
    {
        if (0x22 == *ptr) { dquote = !dquote; }

        if (*ptr == 0)
        {
            argv[argc++] = param;
            break;
        }

        if ((0x20 == *ptr) && (0 == dquote))
        {
            if(param != ptr)
            {
                argv[argc++] = param;
            }
            *ptr++ = 0;
            param = ptr;
        }
        else
        {
            ptr++;
        }
    }

    return argc;
}

static CURLcode rtmp_setup_options(struct connectdata *conn)
{
    RTMP *rtmp = NULL;
    double percent = 0;
    double duration = 0.0;

    int nSkipKeyFrames = DEF_SKIPFRM; // skip this number of keyframes when resuming

    int bOverrideBufferTime = FALSE;  // if the user specifies a buffer time override this is true
    int bStdoutMode = TRUE;   // if true print the stream directly to stdout, messages go to stderr
    int bResume = FALSE;      // true in resume mode
    uint32_t dSeek = 0;       // seek position in resume mode, 0 otherwise
    uint32_t bufferTime = DEF_BUFTIME;

    // meta header and initial frame for the resume mode (they are read from the file and compared with
    // the stream we are trying to continue
    char *metaHeader = 0;
    uint32_t nMetaHeaderSize = 0;

    // video keyframe for matching
    char *initialFrame = 0;
    uint32_t nInitialFrameSize = 0;
    int initialFrameType = 0; // tye: audio or video

    AVal hostname = { 0, 0 };
    AVal playpath = { 0, 0 };
    AVal subscribepath = { 0, 0 };
    int port = -1;
    int protocol = RTMP_PROTOCOL_UNDEFINED;
    int retries = 0;
    int bLiveStream = FALSE;  // is it a live stream? then we can't seek/resume
    int bHashes = FALSE;      // display byte counters not hashes by default

    long int timeout = DEF_TIMEOUT;   // timeout connection after 120 seconds
    uint32_t dStartOffset = 0;    // seek position in non-live mode
    uint32_t dStopOffset = 0;

    AVal swfUrl = { 0, 0 };
    AVal tcUrl = { 0, 0 };
    AVal pageUrl = { 0, 0 };
    AVal app = { 0, 0 };
    AVal auth = { 0, 0 };
    AVal swfHash = { 0, 0 };
    uint32_t swfSize = 0;
    AVal flashVer = { 0, 0 };
    AVal sockshost = { 0, 0 };

    char *argv[64] = {0};
    char *optarg = NULL;
    int   argc = 0;
    int   opt = 0;
    int   i = 0;
    int   rtmp_url = 0;

#ifdef CRYPTO
    int swfAge = 30;  /* 30 days for SWF cache by default */
    int swfVfy = 0;
    unsigned char hash[RTMP_SWF_HASHLEN];
#endif
    if (strlen(conn->data->change.url) >= RTMP_OPTIONS_LEN)
    {
        RTMP_Log(RTMP_LOGWARNING, "RTMP: URL is too long!!!\n");
    }

    rtmp = RTMP_Alloc();
    if (!rtmp)
    {
        return CURLE_OUT_OF_MEMORY;
    }
    
    RTMP_debuglevel = RTMP_LOGERROR;
    
    RTMP_Init(rtmp);
    strncpy(rtmp->rtmp_options, conn->data->change.url, RTMP_OPTIONS_LEN - 1);
    argc = rtmp_build_argv(argv, 64, rtmp->rtmp_options);

    if (conn->data->set.rtmp_livestream)
    {
        bLiveStream = TRUE;
        RTMP_Log(RTMP_LOGWARNING, "play live stream!\n");
    }
    if (conn->data->set.rtmp_starttime > 0)
    {
        dSeek = conn->data->set.rtmp_starttime;
    }
    if (conn->data->set.rtmp_stoptime > 0)
    {
        dStopOffset = conn->data->set.rtmp_stoptime;
    }
    if (conn->data->set.rtmp_timeout > 0)
    {
        timeout = conn->data->set.rtmp_timeout;
    }

    while (i < argc)
    {
        optarg = NULL;
        opt = rtmp_check_option(argv[i], &optarg);
        if (opt && optarg == NULL)
        {
            i++;
            optarg = argv[i];
        }
        i++;

        switch (opt)
        {
#if 0 /* not support option --Doy.Dong, 2014-8-26*/
            case 'h':
                usage(argv[0]);
                return RD_SUCCESS;
#endif
#ifdef CRYPTO
            case 'w':
            {
                int res = rtmp_hex2bin(optarg, &swfHash.av_val);
                if (res != RTMP_SWF_HASHLEN)
                {
                    swfHash.av_val = NULL;
                    RTMP_Log(RTMP_LOGWARNING,
                             "Couldn't parse swf hash hex string, not hexstring or not %d bytes, ignoring!",
                             RTMP_SWF_HASHLEN);
                }
                swfHash.av_len = RTMP_SWF_HASHLEN;
                break;
            }
            case 'x':
            {
                int size = atoi(optarg);
                if (size <= 0)
                {
                    RTMP_Log(RTMP_LOGERROR, "SWF Size must be at least 1, ignoring\n");
                }
                else
                {
                    swfSize = size;
                }
                break;
            }
            case 'W':
                STR2AVAL(swfUrl, optarg);
                swfVfy = 1;
                break;
            case 'X':
            {
                int num = atoi(optarg);
                if (num < 0)
                {
                    RTMP_Log(RTMP_LOGERROR, "SWF Age must be non-negative, ignoring\n");
                }
                else
                {
                    swfAge = num;
                }
            }
            break;
#endif
            case 'k':
                nSkipKeyFrames = atoi(optarg);
                if (nSkipKeyFrames < 0)
                {
                    RTMP_Log(RTMP_LOGERROR,
                             "Number of keyframes skipped must be greater or equal zero, using zero!");
                    nSkipKeyFrames = 0;
                }
                else
                {
                    RTMP_Log(RTMP_LOGDEBUG, "Number of skipped key frames for resume: %d",
                             nSkipKeyFrames);
                }
                break;
            case 'b':
            {
                int32_t bt = atol(optarg);
                if (bt < 0)
                {
                    RTMP_Log(RTMP_LOGERROR,
                             "Buffer time must be greater than zero, ignoring the specified value %d!",
                             bt);
                }
                else
                {
                    bufferTime = bt;
                    bOverrideBufferTime = TRUE;
                }
                break;
            }
            case 'v':
                bLiveStream = TRUE;   // no seeking or resuming possible!
                break;
            case 'd':
                STR2AVAL(subscribepath, optarg);
                break;
            case 'n':
                STR2AVAL(hostname, optarg);
                break;
            case 'c':
                port = atoi(optarg);
                break;
            case 'l':
                protocol = atoi(optarg);
                if (protocol < RTMP_PROTOCOL_RTMP || protocol > RTMP_PROTOCOL_RTMPTS)
                {
                    RTMP_Log(RTMP_LOGERROR, "Unknown protocol specified: %d", protocol);
                    return CURLE_FAILED_INIT;
                }
                break;
            case 'y':
                STR2AVAL(playpath, optarg);
                break;
            case 'Y':
                RTMP_SetOpt(&rtmp, &av_playlist, (AVal *)&av_true);
                break;
            case 'r':
            {
                AVal parsedHost, parsedApp;
                unsigned int parsedPort = 0;
                int parsedProtocol = RTMP_PROTOCOL_UNDEFINED;

                if (!RTMP_ParseURL
                        (optarg, &parsedProtocol, &parsedHost, &parsedPort,
                         &rtmp->Link.playpath0, &parsedApp))
                {
                    RTMP_Log(RTMP_LOGWARNING, "Couldn't parse the specified url (%s)!",
                             optarg);
                }
                else
                {
                    if (!hostname.av_len)
                    {
                        hostname = parsedHost;
                    }
                    if (port == -1)
                    {
                        port = parsedPort;
                    }
                    if (playpath.av_len == 0 && rtmp->Link.playpath0.av_len)
                    {
                        playpath = rtmp->Link.playpath0;
                    }
                    if (protocol == RTMP_PROTOCOL_UNDEFINED)
                    {
                        protocol = parsedProtocol;
                    }
                    if (app.av_len == 0 && parsedApp.av_len)
                    {
                        app = parsedApp;
                    }
                }
                break;
            }
            case 's':
                STR2AVAL(swfUrl, optarg);
                break;
            case 't':
                STR2AVAL(tcUrl, optarg);
                break;
            case 'p':
                STR2AVAL(pageUrl, optarg);
                break;
            case 'a':
                STR2AVAL(app, optarg);
                break;
            case 'f':
                STR2AVAL(flashVer, optarg);
                break;
#if 0 /* not support option --Doy.Dong, 2014-8-26*/
            case 'o':
                flvFile = optarg;
                if (strcmp(flvFile, "-"))
                {
                    bStdoutMode = FALSE;
                }

                break;
#endif
            case 'e':
                bResume = TRUE;
                break;
            case 'u':
                STR2AVAL(auth, optarg);
                break;
            case 'C':
            {
                AVal av;
                STR2AVAL(av, optarg);
                if (!RTMP_SetOpt(&rtmp, &av_conn, &av))
                {
                    RTMP_Log(RTMP_LOGERROR, "Invalid AMF parameter: %s", optarg);
                    return CURLE_FAILED_INIT;
                }
            }
            break;
            case 'm':
                timeout = atoi(optarg);
                break;
            case 'A':
                dStartOffset = (int)(atof(optarg) * 1000.0);
                break;
            case 'B':
                dStopOffset = (int)(atof(optarg) * 1000.0);
                break;
            case 'T':
            {
                AVal token;
                STR2AVAL(token, optarg);
                RTMP_SetOpt(&rtmp, &av_token, &token);
            }
            break;
            case '#':
                bHashes = TRUE;
                break;
            case 'q':
                RTMP_debuglevel = RTMP_LOGCRIT;
                break;
            case 'V':
                RTMP_debuglevel = RTMP_LOGDEBUG;
                break;
            case 'z':
                RTMP_debuglevel = RTMP_LOGALL;
                break;
            case 'S':
                STR2AVAL(sockshost, optarg);
                break;
            default:
                RTMP_LogPrintf("unknown option: %c\n", opt);
#if 0 /* not support --Doy.Dong, 2014-8-26*/
                usage(argv[0]);
                return CURLE_FAILED_INIT;
#endif
                break;
        }

    }

    if (!hostname.av_len)
    {
        RTMP_Log(RTMP_LOGERROR,
                 "You must specify a hostname (--host) or url (-r \"rtmp://host[:port]/playpath\") containing a hostname");
        return CURLE_FAILED_INIT;
    }
    if (playpath.av_len == 0)
    {
        RTMP_Log(RTMP_LOGERROR,
                 "You must specify a playpath (--playpath) or url (-r \"rtmp://host[:port]/playpath\") containing a playpath");
        return CURLE_FAILED_INIT;
    }

    if (protocol == RTMP_PROTOCOL_UNDEFINED)
    {
        RTMP_Log(RTMP_LOGWARNING,
                 "You haven't specified a protocol (--protocol) or rtmp url (-r), using default protocol RTMP");
        protocol = RTMP_PROTOCOL_RTMP;
    }
    if (port == -1)
    {
        RTMP_Log(RTMP_LOGWARNING,
                 "You haven't specified a port (--port) or rtmp url (-r), using default port 1935");
        port = 0;
    }
    if (port == 0)
    {
        if (protocol & RTMP_FEATURE_SSL)
        {
            port = 443;
        }
        else if (protocol & RTMP_FEATURE_HTTP)
        {
            port = 80;
        }
        else
        {
            port = 1935;
        }
    }

#if 0 /* not support  --Doy.Dong, 2014-8-26*/
    if (flvFile == 0)
    {
        RTMP_Log(RTMP_LOGWARNING,
                 "You haven't specified an output file (-o filename), using stdout");
        bStdoutMode = TRUE;
    }
#endif

    if (bStdoutMode && bResume)
    {
        RTMP_Log(RTMP_LOGWARNING,
                 "Can't resume in stdout mode, ignoring --resume option");
        bResume = FALSE;
    }

    if (bLiveStream && bResume)
    {
        RTMP_Log(RTMP_LOGWARNING, "Can't resume live stream, ignoring --resume option");
        bResume = FALSE;
    }

#ifdef CRYPTO
    if (swfVfy)
    {
        if (RTMP_HashSWF(swfUrl.av_val, &swfSize, hash, swfAge) == 0)
        {
            swfHash.av_val = (char *)hash;
            swfHash.av_len = RTMP_SWF_HASHLEN;
        }
    }

    if (swfHash.av_len == 0 && swfSize > 0)
    {
        RTMP_Log(RTMP_LOGWARNING,
                 "Ignoring SWF size, supply also the hash with --swfhash");
        swfSize = 0;
    }

    if (swfHash.av_len != 0 && swfSize == 0)
    {
        RTMP_Log(RTMP_LOGWARNING,
                 "Ignoring SWF hash, supply also the swf size  with --swfsize");
        swfHash.av_len = 0;
        swfHash.av_val = NULL;
    }
#endif

    if (tcUrl.av_len == 0)
    {
        char str[512] = { 0 };

        tcUrl.av_len = snprintf(str, 511, "%s://%.*s:%d/%.*s",
                                RTMPProtocolStringsLower[protocol], hostname.av_len,
                                hostname.av_val, port, app.av_len, app.av_val);
        tcUrl.av_val = (char *) malloc(tcUrl.av_len + 1);
        rtmp->Link.lFlags |= RTMP_LF_FTCU;
        strcpy(tcUrl.av_val, str);
    }

    // User defined seek offset
    if (dStartOffset > 0)
    {
        // Live stream
        if (bLiveStream)
        {
            RTMP_Log(RTMP_LOGWARNING,
                     "Can't seek in a live stream, ignoring --start option");
            dStartOffset = 0;
        }
    }

    RTMP_SetupStream(rtmp, protocol, &hostname, port, &sockshost, &playpath,
                     &tcUrl, &swfUrl, &pageUrl, &app, &auth, &swfHash, swfSize,
                     &flashVer, &subscribepath, dSeek, dStopOffset, bLiveStream, timeout);

    /* Try to keep the stream moving if it pauses on us */
    if (!bLiveStream && !(protocol & RTMP_FEATURE_HTTP))
    {
        rtmp->Link.lFlags |= RTMP_LF_BUFX;
    }

    RTMP_Log(RTMP_LOGDEBUG, "Setting buffer time to: %dms", bufferTime);
    RTMP_SetBufferMS(rtmp, bufferTime);
    conn->proto.generic = rtmp;
#ifdef RTMP_EXTEN
    rtmp->user_data = conn;
#endif
    return CURLE_OK;
}
#endif


static CURLcode rtmp_setup(struct connectdata *conn)
{
#ifdef RTMP_OPTIONS_SUPPORT
    return rtmp_setup_options(conn);
#else
  RTMP *r = RTMP_Alloc();

  if(!r)
    return CURLE_OUT_OF_MEMORY;

  RTMP_Init(r);
  RTMP_SetBufferMS(r, DEF_BUFTIME);
  if(!RTMP_SetupURL(r, conn->data->change.url)) {
    RTMP_Free(r);
    return CURLE_URL_MALFORMAT;
  }
  conn->proto.generic = r;

  r->Link.seekTime = conn->data->set.rtmp_starttime;
  r->Link.stopTime = conn->data->set.rtmp_stoptime;
  r->Link.timeout = conn->data->set.rtmp_timeout > 0 ? conn->data->set.rtmp_timeout : 4;
#ifdef RTMP_EXTEN
  r->user_data = conn;
#endif
  libc_printf("rtmp recv/send timeout:%ds\n", r->Link.timeout);
  if (conn->data->set.rtmp_livestream)
  {
    r->Link.lFlags |= RTMP_LF_LIVE;
    libc_printf("play live stream!\n");
  }
  
  return CURLE_OK;
#endif
}

static CURLcode rtmp_connect(struct connectdata *conn, bool *done)
{
  int on = 1;
  RTMP *r = conn->proto.generic;
  SET_RCVTIMEO(tv, r->Link.timeout);

  r->m_sb.sb_socket = conn->sock[FIRSTSOCKET];

  /* We have to know if it's a write before we send the
   * connect request packet
   */
  if(conn->data->set.upload)
    r->Link.protocol |= RTMP_FEATURE_WRITE;

  /* For plain streams, use the buffer toggle trick to keep data flowing */
  if(!(r->Link.lFlags & RTMP_LF_LIVE) && !(r->Link.protocol & RTMP_FEATURE_HTTP))
    r->Link.lFlags |= RTMP_LF_BUFX;

  curlx_nonblock(r->m_sb.sb_socket, FALSE);
  setsockopt(r->m_sb.sb_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(tv));
  setsockopt(r->m_sb.sb_socket, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv, sizeof(tv));
  setsockopt(r->m_sb.sb_socket, IPPROTO_TCP, TCP_NODELAY, (char *) &on, sizeof(on));

  if(!RTMP_Connect1(r, NULL))
  {
    libc_printf("%s: failed\n", __FUNCTION__);
    return CURLE_FAILED_INIT;
  }

  libc_printf("%s: OK\n", __FUNCTION__);
  /* Clients must send a periodic BytesReceived report to the server */
  r->m_bSendCounter = true;

  *done = TRUE;
  return CURLE_OK;
}

static CURLcode rtmp_do(struct connectdata *conn, bool *done)
{
  RTMP *r = conn->proto.generic;

  if(!RTMP_ConnectStream(r, r->Link.seekTime))
    return CURLE_FAILED_INIT;

  if(conn->data->set.upload) {
    Curl_pgrsSetUploadSize(conn->data, conn->data->set.infilesize);
    Curl_setup_transfer(conn, -1, -1, FALSE, NULL, FIRSTSOCKET, NULL);
  }
  else
    Curl_setup_transfer(conn, FIRSTSOCKET, -1, FALSE, NULL, -1, NULL);
  *done = TRUE;
  
  conn->recv[FIRSTSOCKET] = rtmp_recv;
  conn->send[FIRSTSOCKET] = rtmp_send;
  return CURLE_OK;
}

static CURLcode rtmp_done(struct connectdata *conn, CURLcode status,
                          bool premature)
{
  (void)conn; /* unused */
  (void)status; /* unused */
  (void)premature; /* unused */

  return CURLE_OK;
}

static CURLcode rtmp_disconnect(struct connectdata *conn,
                                bool dead_connection)
{
  RTMP *r = conn->proto.generic;
  (void)dead_connection;
  if(r) {
    conn->proto.generic = NULL;
    RTMP_Close(r);
    RTMP_Free(r);
  }
  return CURLE_OK;
}

static ssize_t rtmp_recv(struct connectdata *conn, int sockindex, char *buf,
                         size_t len, CURLcode *err)
{
  RTMP *r = conn->proto.generic;
  ssize_t nread;

  (void)sockindex; /* unused */

  nread = RTMP_Read(r, buf, curlx_uztosi(len));
  if(nread < 0) {
    if(r->m_read.status == RTMP_READ_COMPLETE ||
        r->m_read.status == RTMP_READ_EOF) {
      conn->data->req.size = conn->data->req.bytecount;
      nread = 0;
    }
    else
      *err = CURLE_RECV_ERROR;
  }
  return nread;
}

static ssize_t rtmp_send(struct connectdata *conn, int sockindex,
                         const void *buf, size_t len, CURLcode *err)
{
  RTMP *r = conn->proto.generic;
  ssize_t num;

  (void)sockindex; /* unused */

  num = RTMP_Write(r, (char *)buf, curlx_uztosi(len));
  if(num < 0)
    *err = CURLE_SEND_ERROR;

  return num;
}

int rtmp_check_close(void *data)
{
    int ret = 0;
    struct connectdata *conn = (struct connectdata *)data;

    if(conn && conn->data && conn->data->set.fprogress)
    {
      ret= conn->data->set.fprogress(conn->data->set.progress_client, (double)conn->data->progress.size_dl,
                               (double)conn->data->progress.downloaded, (double)conn->data->progress.size_ul,
                               (double)conn->data->progress.uploaded);
    }
    return ret;
}

#endif  /* USE_LIBRTMP */
