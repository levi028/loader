
#include <sys_config.h>

#ifdef VPN_ENABLE

#ifndef _VPN_INCLUDE_H_
#define _VPN_INCLUDE_H_
typedef struct
{
    char usr[64];
    char pwd[64];
    unsigned int saddr;
    int  encryt;
} vpn_cfg;

typedef enum
{
    VPN_MSG_START_CONNECTTING = 0x1, //start connect vpn server
    VPN_MSG_CONNECT_SERVER_FAILED,   //connect vpn server failed
    VPN_MSG_AUTHENTICATE_FAILED,     //authenticate failed
    VPN_MSG_VPN_SUCESS,              //vpn build success
    VPN_MSG_CONNECTION_LOST,         //vpn connection lost
    VPN_MSG_VPN_CLOSED,              //vpn has been closed
} VPN_MSG;

typedef void (*vpn_notify)(int msg);

#endif
#endif
