#ifndef __LIBNET_CTRL_H__
#define __LIBNET_CTRL_H__
#include <basic_types.h>
#include <api/libsi/lib_epg.h>
#include <api/libdb/db_config.h>
#include <api/libdb/db_node_combo.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
	BOOL				(*get_curprog_info)(P_NODE*);
	eit_event_info_t*	(*get_cur_service_event)(INT32, UINT32, date_time*, date_time*, INT32*, BOOL);
	UINT16				(*get_cur_group_cur_mode_channel)(void);
	BOOL				(*send_key)(UINT32);
	INT32				(*send_msg)(UINT8*);
	INT32				(*gen_epg)(UINT8 *, UINT32 *);
	UINT32					reserved[3];
}NET_CTL_CBREG;
typedef INT32 (*LIBNETCTL_CALLBACK)(UINT8 *str);

INT32 libnetctl_callback_register(NET_CTL_CBREG* cbarray);
INT32 libnetctl_init(LIBNETCTL_CALLBACK cb);

#ifdef __cplusplus
}
#endif

#endif


