#ifndef _IW_HANDLER_H_
#define _IW_HANDLER_H_

#include "wireless.h"

#ifdef __cplusplus
extern "C"
{
#endif

struct iw_request_info
{
    UINT16        cmd;        /* Wireless Extension command */
    UINT16        flags;        /* More to come ;-) */
};

struct net_device;

/*
 * This is how a function handling a Wireless Extension should look
 * like (both get and set, standard and private).
 */
typedef int (*iw_handler)(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);

/*
 * This define all the handler that the driver export.
 * As you need only one per driver type, please use a static const
 * shared by all driver instances... Same for the members...
 * This will be linked from net_device in <linux/netdevice.h>
 */
struct iw_handler_def
{
    /* Number of handlers defined (more precisely, index of the
     * last defined handler + 1) */
    UINT16            num_standard;
    UINT16            num_private;
    /* Number of private arg description */
    UINT16            num_private_args;

    /* Array of handlers for standard ioctls
     * We will call dev->wireless_handlers->standard[ioctl - SIOCSIWCOMMIT]
     */
    const iw_handler *    standard;

    /* Array of handlers for private ioctls
     * Will call dev->wireless_handlers->private[ioctl - SIOCIWFIRSTPRIV]
     */
    const iw_handler *    private;

    /* Arguments of private handler. This one is just a list, so you
     * can put it in any order you want and should not leave holes...
     * We will automatically export that to user space... */
    const struct iw_priv_args *    private_args;

    /* New location of get_wireless_stats, to de-bloat struct net_device.
     * The old pointer in struct net_device will be gradually phased
     * out, and drivers are encouraged to use this one... */
    struct iw_statistics*    (*get_wireless_stats)(struct net_device *dev);
};

 /*------------------------------------------------------------------*/
/*
 * Wrapper to add an Wireless Event to a stream of events.
 */
static inline char *iwe_stream_add_event(struct iw_request_info *info, char *stream, char *ends,
    struct iw_event *iwe, int event_len)
{
    int lcp_len = IW_EV_LCP_LEN;

    (void)info;
    //event_len = iwe_stream_event_len_adjust(info, event_len);

    /* Check if it's possible */
    if((stream + event_len) < ends) {
        iwe->len = event_len;
        /* Beware of alignement issues on 64 bits */
        MEMCPY(stream, (char *) iwe, IW_EV_LCP_PK_LEN);
        MEMCPY(stream + lcp_len, &iwe->u, event_len - lcp_len);
        stream += event_len;
    }
    return stream;
}

/*------------------------------------------------------------------*/
/*
 * Wrapper to add an short Wireless Event containing a pointer to a
 * stream of events.
 */
static inline char *iwe_stream_add_point(struct iw_request_info *info, char *stream, char *ends,
    struct iw_event *iwe, char *extra)
{
    int event_len = IW_EV_POINT_LEN + iwe->u.data.length;
    int point_len = IW_EV_POINT_LEN;
    int lcp_len   = IW_EV_LCP_LEN;

    (void)info;
    /* Check if it's possible */
    if((stream + event_len) < ends) {
        iwe->len = event_len;
        MEMCPY(stream, (char *) iwe, IW_EV_LCP_PK_LEN);
        MEMCPY(stream + lcp_len, ((char *) &iwe->u) + IW_EV_POINT_OFF, IW_EV_POINT_PK_LEN - IW_EV_LCP_PK_LEN);
        MEMCPY(stream + point_len, extra, iwe->u.data.length);
        stream += event_len;
    }
    return stream;
}

/*------------------------------------------------------------------*/
/*
 * Wrapper to add a value to a Wireless Event in a stream of events.
 * Be careful, this one is tricky to use properly :
 * At the first run, you need to have (value = event + IW_EV_LCP_LEN).
 */
static inline char *iwe_stream_add_value(struct iw_request_info *info, char *event, char *value,
    char *ends, struct iw_event *iwe, int event_len)
{
    int lcp_len = IW_EV_LCP_LEN;

    (void)info;
    /* Don't duplicate LCP */
    event_len -= IW_EV_LCP_LEN;

    /* Check if it's possible */
    if((value + event_len) < ends) {
        /* Add new value */
        MEMCPY(value, &iwe->u, event_len);
        value += event_len;
        /* Patch LCP */
        iwe->len = value - event;
        MEMCPY(event, (char *) iwe, lcp_len);
    }
    return value;
}

#ifdef __cplusplus
}
#endif

#endif
