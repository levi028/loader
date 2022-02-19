
#ifndef _PPTP_GRE_H_
#define _PPTP_GRE_H_

#define PACKET_MAX 8196

typedef struct pack_track
{
    UINT32 seq;
    UINT64 time;
} pack_track_t;

typedef struct gre_stats
{
    UINT32 rx_accepted;
    UINT32 rx_lost;
    UINT32 rx_underwin;
    UINT32 rx_overwin;
    UINT32 rx_buffered;
    UINT32 rx_errors;
    UINT32 rx_truncated;
    UINT32 rx_invalid;
    UINT32 rx_acks;


    UINT32 tx_sent;
    UINT32 tx_failed;
    UINT32 tx_short;
    UINT32 tx_acks;
    UINT32 tx_oversize;

    pack_track_t pt;
    int rtt;
} gre_stats_t;

typedef enum
{
    PPTP_GRE_STATE_NONE = 0,
    PPTP_GRE_STATE_RUNNING,
    PPTP_GRE_STATE_STOP,
} gre_state;

typedef struct
{
    UINT16 callid;
    UINT16 peer_callid;
    INT32  pty_fd;
    INT32  gre_fd;
    INT32  state;
    ID     slock;
    ID     taskid;
    ///////////////////
    int syncppp;
    int log_level;
    int disable_buffer;
    int test_type;
    int test_rate;
    int rtmark;
    UINT32 ack_sent;
    UINT32 ack_recv;
    UINT32 seq_sent;
    UINT32 seq_recv;
    gre_stats_t stats;

    //encaps_hdlc buffer
    UINT8   encaps_hdlc_buffer[2 * PACKET_MAX + 2];

    //decaps_ppp buffer
    UINT8   decaps_hdlc_buffer[PACKET_MAX];
    UINT32  decaps_hdlc_len;
    UINT32  decaps_hdlc_escape;
    UINT8   decaps_hdlc_copy[PACKET_MAX];
    INT32   decaps_hdlc_checkedsync;

    //decaps_gre buffer
    UINT8   decaps_gre_buffer[PACKET_MAX + 64];
    INT32   decaps_gre_first;

    //encaps_gre
    UINT8   encaps_gre_buffer[PACKET_MAX + 20];
    UINT32  encaps_gre_seq;
} vpn_gre;
#endif
