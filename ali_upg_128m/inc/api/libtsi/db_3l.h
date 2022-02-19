/****************************************************************************
*
*  ALi (Shanghai) Corporation, All Rights Reserved. 2003 Copyright (C)
*
*  File: db_3l.h
*
*  Description: The head file of program database
*
*  History:
*      Date        Author         Version   Comment
*      ====        ======         =======   =======
*  1.  2004.07.04  David Wang     0.1.000   Initial
*  2.  2005.03.24  Zhengdao Li    0.1.001   Add marco for SERVICE_PROVIDER name option field.
*  3.  2005.04.21  Zhengdao Li    0.1.002   Add support for program sort by dl-frequency by default.
****************************************************************************/
#ifndef __LIB_DB_3L_H__
#define __LIB_DB_3L_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <sys_config.h>

#ifndef COMBOUI
#if( SYS_PROJECT_FE == PROJECT_FE_DVBT )
#if DB_VERSION <40
    #include "db_3l_T.h"
#else
    #include <api/libdb/db_return_value.h>
    #include <api/libdb/db_config.h>
    #include <api/libdb/db_node_t.h>
    #include <api/libdb/db_interface.h>
#endif
#elif(SYS_PROJECT_FE == PROJECT_FE_ISDBT)
#if DB_VERSION <40
    #include "db_3l_ISDBT.h"
#else
    #include <api/libdb/db_return_value.h>
    #include <api/libdb/db_config.h>
    #include <api/libdb/db_node_isdbt.h>
    #include <api/libdb/db_interface.h>
#endif
#elif(SYS_PROJECT_FE == PROJECT_FE_ATSC)
#if DB_VERSION <40
    #include "db_3l_ATSC.h"
#else
    #include <api/libdb/db_return_value.h>
    #include <api/libdb/db_config.h>
    #include <api/libdb/db_node_atsc.h>
    #include <api/libdb/db_interface.h>
#endif
#elif(SYS_PROJECT_FE == PROJECT_FE_DVBS||SYS_PROJECT_FE == PROJECT_FE_DVBS2)

#if DB_VERSION < 40
    #include "db_3l_S.h"
#else
    #include <api/libdb/db_return_value.h>
    #include <api/libdb/db_config.h>
    #include <api/libdb/db_node_s.h>
    #include <api/libdb/db_interface.h>
#endif


#elif( SYS_PROJECT_FE == PROJECT_FE_DVBC )
#if DB_VERSION < 40
    #include "db_3l_C.h"
#else
    #include <api/libdb/db_return_value.h>
    #include <api/libdb/db_config.h>
    #include <api/libdb/db_node_c.h>
    #include <api/libdb/db_interface.h>
#endif
#endif

#else

#include <api/libdb/db_return_value.h>
#include <api/libdb/db_config.h>
#include <api/libdb/db_node_combo.h>
#include <api/libdb/db_interface.h>
#endif

#if DB_VERSION < 40

/*
 * Description
 *  This function will checkup the database for specific node is full or not.
 * Parameter
 *  n_type      The node type, shoud be TYPE_xx_NODE.
 * Return Value
 *  TRUE        This type of node is full.
 *  FALSE       This type of node is not full.
 */

BOOL db_is_node_full(UINT8 n_type);
/*
 * Description
 *  This function will checkup the database for certain node
 * Parameter
 *  n_type      The node type, should be TYPE_xx_NODE.
 *  node        The node memory address, should not be null.
 *  add_flag    The method of check up.
 * Return Value
 *  SUCCESS     The node exists in current database.
 *  !SUCCESS    The node doesn't exist in database yet.
 */
INT32 lookup_node(UINT8 n_type,void *node,UINT8 add_flag);

/*
 * Description
 *  Convert ISO639 char into 2byte format.
 * Parameter
 *  src     The iso639 (3)letters
 *  des     The target memory space for the 2byte.
 * Return Value
 *  void
 */
void get_audio_lang2b(UINT8 *src,UINT8 *des);

#if (SYS_SDRAM_SIZE>2)
INT32 undo_prog_modify(UINT16 tp_id,UINT16 prog_number);
#endif

void _reset_tmp_buff_count(UINT8 n_type);

INT32 _del_prog_by_other_id(UINT8 id_flag,UINT16 id);
INT32 _modify_node(UINT8 n_type,UINT16 pos,UINT8 *node,UINT16 node_len);
INT32 _recreate_prog_view(INT8 create_flag,UINT16 id);
INT8 _get_av_mode();


#endif //DB_VERSION < 40

#ifdef __cplusplus
}
#endif

#endif /* __LIB_DB_3L_H__ */

