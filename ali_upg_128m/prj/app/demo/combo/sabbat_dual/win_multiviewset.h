/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               win_multiviewset.h
 *  @brief
 *
 *  @version            1.0
 *  @date               11/22/2013 09:59:12 AM
 *  @revision           none
 *
 *  @author             Summer Xia <summer.xia@alitech.com>
 */

#ifndef _WIN_MULTIVIEWSET_H_
#define _WIN_MULTIVIEWSET_H_

#ifdef __cplusplus
extern "C"
{
#endif

PRESULT comlist_menu_multiview_callback(POBJECT_HEAD p_obj,
                VEVENT event,
                UINT32 param1,
                UINT32 param2);

UINT8 win_multiview_get_num();

#ifdef __cplusplus
}
#endif

#endif
