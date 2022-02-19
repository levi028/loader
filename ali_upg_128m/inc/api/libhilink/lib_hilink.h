#ifndef _LIB_HILINK_H_
#define _LIB_HILINK_H_

#ifdef HILINK_SUPPORT

#define HILINK_SETAUTOMODE_BUFLEN 1024
#define HILINK_SETPROFILE_BUFLEN  2 * 1024
#define HILINK_REQUEST_BUFLEN     1024
#define HILINK_RESPONSE_BUFLEN    3 * 1024
#define HILINK_SERVER_HTTPPORT    80
#define HILINK_PROFILE_LEN        50
#define HILINK_SLEEP_TIME         4000
#define SOCKERRNO         (lwip_socket_errno) 

//#define LIBHILINK_DEBUG
#ifdef LIBHILINK_DEBUG
    #define HILINK_PRINT libc_printf
#else
    #define HILINK_PRINT(...) 
#endif

enum
{
    HILINK_EVENT_PLUGOUT = 0,
    HILINK_EVENT_PLUGIN,
    
    HILINK_EVENT_MAX,
};

enum
{
    HILINK_USEFLAG_NOUSE = 0,
    HILINK_USEFLAG_USE,

    HILINK_USEFLAG_MAX,
};

enum
{
    HILINK_OK = 0,
    HILINK_EOS_ERR,
    HILINK_PARA_ERR,
    HILINK_MALLOC_ERR,
    HILINK_CALL_ERR,
    HILINK_TIMEOUT_ERR,
    HILINK_SPACESMALL_ERR,
    HILINK_HTTPCODE_ERR,
    HILINK_NOFIND_ERR,
    HILINK_NOFINDVALUE_ERR,
    HILINK_DHCP_ERR,
    HILINK_UPDATE_ERR,
    
    HILINK_ERR_MAX,
};

enum
{
    HILINK_AUTOMODE_ENABLE = 0,
    HILINK_AUTOMODE_DISABLE,

    HILINK_ATUOMODE_MAX,
};

enum
{
    HILINK_CONNECTSTATE_SETDISCONNECT = 0,
    HILINK_CONNECTSTATE_SETCONNECT,

    HILINK_CONNECTSTATE_SETMAX,
};

enum
{
    HILINK_SIMCARD_OFFLINE = 0,
    HILINK_SIMCARD_ONLINE,
    HILINK_SIMCARD_NOFIND,//?a???¨¦??¨º?simcard?¨²¡ê?¦Ì?¡Á??a¨º?¦Ì?¨º¡Ào¨°¨®D?¨¦?¨¹?¨¬2a2?¦Ì?
    
    HILINK_SIMCARD_MAX,
};

enum
{
    HILINK_BUTTONCONNECT_NOOPERATE = 0,
    HILINK_BUTTONCONNECT_CONNECT,
    HILINK_BUTTONCONNECT_DISCONNECT,

    HILINK_BUTTONCONNECT_MAX,
};

enum
{
    HILINK_GETSTATE_CONNECTING = 900,
    HILINK_GETSTATE_CONNECT,
    HILINK_GETSTATE_DISCONNECT,
    HILINK_GETSTATE_DISCONNECTING,

    HILINK_GETSTATE_MAX,
};

typedef enum
{
    HILINK_STATECHANGE_NOCARD = 0,
    HILINK_STATECHANGE_CONNECTING,
    HILINK_STATECHANGE_CONNECT,
    HILINK_STATECHANGE_DISCONNECT,
    HILINK_STATECHANGE_DISCONNECTING,
    HILINK_STATECHANGE_INITOK,
    
    HILINK_STATECHANGE_MAX,  
}HILINK_STATECHANGE_ENUM;

typedef enum
{
	HILINK_STATUS_PLUGIN = 100,
	HILINK_STATUS_PLUGOUT,
	HILINK_STATUS_CHANGE,
}hilink_state_t;

enum
{
    HILINK_BUFSTATUS_NOINIT = 0,
    HILINK_BUFSTATUS_IDLE,
    HILINK_BUFSTATUS_BUSY,
    HILINK_BUFSTATUS_FREE,

    HILINK_BUFSTATUS_MAX,
};

typedef struct HILINK_SETCONNECT_STATE
{
    unsigned int ulsetconnectstate;
}HILINK_SETCONNECT_STATE_STRU;

typedef struct HILINK_PROFILE_INFO
{
    unsigned int ulapnno;
    char acname[HILINK_PROFILE_LEN];
    char acapn[HILINK_PROFILE_LEN];
    char acuser[HILINK_PROFILE_LEN];
    char acpasswd[HILINK_PROFILE_LEN];
}HILINK_PROFILE_INFO_STRU;

typedef struct HILINK_AUTOMODE_INFO
{
    unsigned int ulroamautoconable;
    unsigned int ulautoreconnect;
    unsigned int ulroamautoreconable;
    unsigned int ulreconnectinterval;
    unsigned int ulmaxidletime;
    unsigned int ulautomode;
    unsigned int ulmtu;
}HILINK_AUTOMODE_INFO_STRU;

typedef struct HILINK_CONNECTSTATE_INFO
{
    unsigned int ulupdateflag;
    unsigned int ulconnectstate;
    unsigned int ulsimcardstate;
}HILINK_CONNECTSTATE_INFO_STRU;

typedef struct HILINK_INFO
{
    ID mutex_connectstateinfo;
    HILINK_PROFILE_INFO_STRU strprofileinfo;
    HILINK_AUTOMODE_INFO_STRU strautomodeinfo;
    HILINK_CONNECTSTATE_INFO_STRU strconnectstateinfo;
    unsigned int ulsetautomodebuflen;
    char *pcsetautomodebuf;
    unsigned int ulsetprofilebuflen;
    char *pcsetprofilebuf;
}HILINK_INFO_STRU;

int hilink_init();
int hilink_get_initflag(unsigned int *pulinitflag);
int hilink_set_useflag(unsigned int uluseflag);
int hilink_getprofileinfo_fromserver(HILINK_PROFILE_INFO_STRU *pstrprofileinfo);
int hilink_getautomodeinfo_fromserver(HILINK_AUTOMODE_INFO_STRU *pstrautomodeinfo);
int hilink_getconnectstate_fromserver(HILINK_CONNECTSTATE_INFO_STRU *pstrconnectstateinfo);
int hilink_getconnectstate_fromlocall(HILINK_CONNECTSTATE_INFO_STRU *pstrconnectstateinfo);
int hilink_setprofileinfo_toserver(HILINK_PROFILE_INFO_STRU strprofileinfo);
int hilink_setautomodeinfo_toserver(HILINK_AUTOMODE_INFO_STRU strautomodeinfo);
int hilink_setconnectstate_toserver(HILINK_SETCONNECT_STATE_STRU strsetconnectstate);

int hilink_plug_setflag(unsigned int ulflag);
int hilink_creat_task(unsigned int ulpara1,unsigned int ulpara2);

#endif
#endif

