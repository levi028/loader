
#ifndef _PPTP_MSG_H_
#define _PPTP_MSG_H_

#define hton8(x)  (x)
#define ntoh8(x)  (x)
#define hton16(x) htons(x)
#define ntoh16(x) ntohs(x)
#define hton32(x) htonl(x)
#define ntoh32(x) ntohl(x)


#define PPTP_MAGIC 0x1A2B3C4D
#define PPTP_PORT  1723
#define PPTP_PROTO 47


#define PPTP_MESSAGE_CONTROL        1
#define PPTP_MESSAGE_MANAGE     2

#define PPTP_START_CTRL_CONN_RQST   1
#define PPTP_START_CTRL_CONN_RPLY   2
#define PPTP_STOP_CTRL_CONN_RQST    3
#define PPTP_STOP_CTRL_CONN_RPLY    4
#define PPTP_ECHO_RQST          5
#define PPTP_ECHO_RPLY          6

#define PPTP_OUT_CALL_RQST      7
#define PPTP_OUT_CALL_RPLY      8
#define PPTP_IN_CALL_RQST       9
#define PPTP_IN_CALL_RPLY       10
#define PPTP_IN_CALL_CONNECT        11
#define PPTP_CALL_CLEAR_RQST        12
#define PPTP_CALL_CLEAR_NTFY        13

#define PPTP_WAN_ERR_NTFY       14

#define PPTP_SET_LINK_INFO      15

#define PPTP_VERSION_STRING "1.00"
#define PPTP_VERSION        0x100
#define PPTP_FIRMWARE_STRING    "0.01"
#define PPTP_FIRMWARE_VERSION   0x001
#define PPTP_MAX_CHANNELS 5
#define PPTP_HOSTNAME {'l','o','c','a','l',0}
#define PPTP_VENDOR   {'N','T',0}
#define PPTP_FRAME_CAP  2
#define PPTP_BEARER_CAP 1
#define PPTP_BPS_MIN 2400
#define PPTP_BPS_MAX 10000000
#define PPTP_WINDOW 3


#define PPTP_FRAME_ASYNC    1
#define PPTP_FRAME_SYNC     2
#define PPTP_FRAME_ANY          3

#define PPTP_BEARER_ANALOG  1
#define PPTP_BEARER_DIGITAL     2
#define PPTP_BEARER_ANY     3

#define PPTP_RESULT_GENERAL_ERROR 2

#define PPTP_STOP_NONE        1
#define PPTP_STOP_PROTOCOL    2
#define PPTP_STOP_LOCAL_SHUTDOWN  3


struct pptp_header
{
    UINT16 length;
    UINT16 pptp_type;
    UINT32 magic;
    UINT16 ctrl_type;
    UINT16 reserved0;
};

struct pptp_start_ctrl_conn
{
    struct pptp_header header;

    UINT16 version;
    UINT8  result_code;
    UINT8  error_code;
    UINT32 framing_cap;
    UINT32 bearer_cap;
    UINT16 max_channels;
    UINT16 firmware_rev;
    UINT8  hostname[64];
    UINT8  vendor[64];
};

struct pptp_stop_ctrl_conn
{
    struct pptp_header header;

    UINT8 reason_result;
    UINT8 error_code;
    UINT16 reserved1;
};

struct pptp_echo_rqst
{
    struct pptp_header header;
    UINT32 identifier;
};

struct pptp_echo_rply
{
    struct pptp_header header;
    UINT32 identifier;
    UINT8 result_code;
    UINT8 error_code;
    UINT16 reserved1;
};

struct pptp_out_call_rqst
{
    struct pptp_header header;
    UINT16 call_id;
    UINT16 call_sernum;
    UINT32 bps_min;
    UINT32 bps_max;
    UINT32 bearer;
    UINT32 framing;
    UINT16 recv_size;
    UINT16 delay;
    UINT16 phone_len;
    UINT16 reserved1;
    UINT8  phone_num[64];
    UINT8 subaddress[64];
};

struct pptp_out_call_rply
{
    struct pptp_header header;
    UINT16 call_id;
    UINT16 call_id_peer;
    UINT8  result_code;
    UINT8  error_code;
    UINT16 cause_code;
    UINT32 speed;
    UINT16 recv_size;
    UINT16 delay;
    UINT32 channel;
};

struct pptp_in_call_rqst
{
    struct pptp_header header;
    UINT16 call_id;
    UINT16 call_sernum;
    UINT32 bearer;
    UINT32 channel;
    UINT16 dialed_len;
    UINT16 dialing_len;
    UINT8 dialed_num[64];
    UINT8 dialing_num[64];
    UINT8 subaddress[64];
};

struct pptp_in_call_rply
{
    struct pptp_header header;
    UINT16 call_id;
    UINT16 call_id_peer;
    UINT8  result_code;
    UINT8  error_code;
    UINT16 recv_size;
    UINT16 delay;
    UINT16 reserved1;
};

struct pptp_in_call_connect
{
    struct pptp_header header;
    UINT16 call_id_peer;
    UINT16 reserved1;
    UINT32 speed;
    UINT16 recv_size;
    UINT16 delay;
    UINT32 framing;
};

struct pptp_call_clear_rqst
{
    struct pptp_header header;
    UINT16 call_id;
    UINT16 reserved1;
};

struct pptp_call_clear_ntfy
{
    struct pptp_header header;
    UINT16 call_id;
    UINT8  result_code;
    UINT8  error_code;
    UINT16 cause_code;
    UINT16 reserved1;
    UINT8 call_stats[128];
};

struct pptp_wan_err_ntfy
{
    struct pptp_header header;
    UINT16 call_id_peer;
    UINT16 reserved1;
    UINT32 crc_errors;
    UINT32 frame_errors;
    UINT32 hard_errors;
    UINT32 buff_errors;
    UINT32 time_errors;
    UINT32 align_errors;
};

struct pptp_set_link_info
{
    struct pptp_header header;
    UINT16 call_id_peer;
    UINT16 reserved1;
    UINT32 send_accm;
    UINT32 recv_accm;
};

#define pptp_isvalid_ctrl(header, type, length) \
    (!( ( ntoh16(((struct pptp_header *)header)->length)    < (length)  ) ||   \
        ( ntoh16(((struct pptp_header *)header)->pptp_type) !=(type)    ) ||   \
        ( ntoh32(((struct pptp_header *)header)->magic)     !=PPTP_MAGIC) ||   \
        ( ntoh16(((struct pptp_header *)header)->ctrl_type) > PPTP_SET_LINK_INFO) || \
        ( ntoh16(((struct pptp_header *)header)->reserved0) !=0         ) ))

#define PPTP_HEADER_CTRL(type)  \
    { hton16(PPTP_CTRL_SIZE(type)), \
        hton16(PPTP_MESSAGE_CONTROL), \
        hton32(PPTP_MAGIC),           \
        hton16(type), 0 }

#define PPTP_CTRL_SIZE(type) ( \
                               (type==PPTP_START_CTRL_CONN_RQST)?sizeof(struct pptp_start_ctrl_conn):  \
                               (type==PPTP_START_CTRL_CONN_RPLY)?sizeof(struct pptp_start_ctrl_conn):  \
                               (type==PPTP_STOP_CTRL_CONN_RQST )?sizeof(struct pptp_stop_ctrl_conn):   \
                               (type==PPTP_STOP_CTRL_CONN_RPLY )?sizeof(struct pptp_stop_ctrl_conn):   \
                               (type==PPTP_ECHO_RQST           )?sizeof(struct pptp_echo_rqst):    \
                               (type==PPTP_ECHO_RPLY           )?sizeof(struct pptp_echo_rply):    \
                               (type==PPTP_OUT_CALL_RQST       )?sizeof(struct pptp_out_call_rqst):    \
                               (type==PPTP_OUT_CALL_RPLY       )?sizeof(struct pptp_out_call_rply):    \
                               (type==PPTP_IN_CALL_RQST        )?sizeof(struct pptp_in_call_rqst): \
                               (type==PPTP_IN_CALL_RPLY        )?sizeof(struct pptp_in_call_rply): \
                               (type==PPTP_IN_CALL_CONNECT     )?sizeof(struct pptp_in_call_connect):  \
                               (type==PPTP_CALL_CLEAR_RQST     )?sizeof(struct pptp_call_clear_rqst):  \
                               (type==PPTP_CALL_CLEAR_NTFY     )?sizeof(struct pptp_call_clear_ntfy):  \
                               (type==PPTP_WAN_ERR_NTFY        )?sizeof(struct pptp_wan_err_ntfy): \
                               (type==PPTP_SET_LINK_INFO       )?sizeof(struct pptp_set_link_info):    \
                               0)
#define max(a,b) (((a)>(b))?(a):(b))
#define PPTP_CTRL_SIZE_MAX (max(sizeof(struct pptp_start_ctrl_conn),    \
                                max(sizeof(struct pptp_echo_rqst),      \
                                    max(sizeof(struct pptp_echo_rply),      \
                                        max(sizeof(struct pptp_out_call_rqst),      \
                                            max(sizeof(struct pptp_out_call_rply),      \
                                                max(sizeof(struct pptp_in_call_rqst),       \
                                                    max(sizeof(struct pptp_in_call_rply),       \
                                                        max(sizeof(struct pptp_in_call_connect),    \
                                                            max(sizeof(struct pptp_call_clear_rqst),    \
                                                                max(sizeof(struct pptp_call_clear_ntfy),    \
                                                                    max(sizeof(struct pptp_wan_err_ntfy),       \
                                                                        max(sizeof(struct pptp_set_link_info), 0)))))))))))))



#define PPTP_GRE_PROTO  0x880B
#define PPTP_GRE_VER    0x1

#define PPTP_GRE_FLAG_C 0x80
#define PPTP_GRE_FLAG_R 0x40
#define PPTP_GRE_FLAG_K 0x20
#define PPTP_GRE_FLAG_S 0x10
#define PPTP_GRE_FLAG_A 0x80

#define PPTP_GRE_IS_C(f) ((f)&PPTP_GRE_FLAG_C)
#define PPTP_GRE_IS_R(f) ((f)&PPTP_GRE_FLAG_R)
#define PPTP_GRE_IS_K(f) ((f)&PPTP_GRE_FLAG_K)
#define PPTP_GRE_IS_S(f) ((f)&PPTP_GRE_FLAG_S)
#define PPTP_GRE_IS_A(f) ((f)&PPTP_GRE_FLAG_A)

struct pptp_gre_header
{
    UINT8 flags;
    UINT8 ver;
    UINT16 protocol;
    UINT16 payload_len;
    UINT16 call_id;
    UINT32 seq;
    UINT32 ack;
};

#endif
