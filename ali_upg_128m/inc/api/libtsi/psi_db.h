/*
 * PSI_DB.H
 *
 * Interfacing function for PSI and DATABASE conversions.
 *
 * History
 * 1. 20060516  Zhengdao Li 0.0.1       Initialize.
 */
#ifndef __PSI_DB_H__
#define __PSI_DB_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <types.h>
#include <sys_config.h>
#include <api/libtsi/db_3l.h>
#include <api/libtsi/si_types.h>
#include <api/libtsi/p_search.h>

#if(defined( _MHEG5_SUPPORT_) ||defined(_LCN_ENABLE_) ||defined(_MHEG5_V20_ENABLE_))
#define INVALID_LCN_NUM     0xFFFF
#endif

typedef INT32 (*prog_node_return)(P_NODE *node );

typedef void (*backup_new_prog)(UINT32 prog_id, UINT8 av_flag);//auto update tp information

/*
 * name     : pg2db
 * description  : convert PSI program information into database node.
 * parameter    : 2
 * @pg      : the database node.
 * @info    : the PSI program information.
 * return value : INT32
 * @SI_SUCCESS
 */
INT32 pd_pg2db(P_NODE *pg, PROG_INFO *info);

/*
 * name     : update_pids
 * description  : copy all PID information from source database node to destination.
 * parameter    : 2
 * @dest    : the target database node.
 * @src     : the source database node.
 * return value : INT32
 * @SI_SUCCESS
 */
INT32 pd_update_pids(P_NODE *dest, P_NODE *src);

/*
 * name     : update_name
 * description  : copy all name information from source database node to destination.
 * parameter    : 2
 * @dest    : the target database node.
 * @src     : the source database node.
 * return value : INT32
 * @SI_SUCCESS.
 */
INT32 pd_update_name(P_NODE *dest, P_NODE *src);
INT32 pid_callback_register(backup_new_prog callback);
INT32 pid_callback_unregister(void);
INT32 prog_callback_register(prog_node_return callback);
INT32 prog_callback_unregister(void);
UINT32 get_prog_flg();
INT32 psi_install_prog(UINT8 *string_buf,PROG_INFO *p_info,P_NODE *p_node,UINT32 search_scheme,UINT32 storage_scheme);
void psi_fake_name(UINT8 *name, UINT16 program_number);
INT32 psi_monitor_pg2db(P_NODE *pg, PROG_INFO *info);
INT32 psi_pg2db(P_NODE *pg, PROG_INFO *info);

extern int snprintf(char *str, unsigned int size, const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif /* __PSI_DB_H__ */

