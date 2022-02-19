#ifndef _VPN_PPTP_H_
#define _VPN_PPTP_H_

typedef struct
{
    INT32    sock;
    UINT32   saddr;
    UINT32   sport;
    UINT32   timer;
    INT16    cstate;
    INT16    state;
    int      kptmo;
    int      kpcnt;

    UINT16 call_id;
    UINT16 peer_call_id;
    UINT16 sernum;
    UINT32 speed;
    UINT16 call_serial_number;
    UINT16 version;      /* PPTP protocol version.  = PPTP_VERSION     */
    UINT8  result_code;  /* these two fields should be zero on rqst msg*/
    UINT8  error_code;   /* 0 unless result_code==2 (General Error)    */
    UINT32 framing_cap;  /* Framing capabilities                       */
    UINT32 bearer_cap;   /* Bearer Capabilities                        */
    UINT16 max_channels; /* Maximum Channels (=0 for PNS, PAC ignores) */
    UINT16 firmware_rev; /* Firmware or Software Revision              */
    UINT8  hostname[64]; /* Host Name (64 octets, zero terminated)     */
    UINT8  vendor[64];   /* Vendor string (64 octets, zero term.)      */
    UINT32 ka_id;
    UINT8  phonenr[64];
} vpn_pptp;

int vpn_pptp_getcallid(vpn_pptp *pptp, UINT16 *call_id, UINT16 *peer_call_id);

int vpn_pptp_error(vpn_pptp *pptp);

int vpn_pptp_start(vpn_pptp *pptp);

int vpn_pptp_close(vpn_pptp *pptp);

#endif
