/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
*    File: db_reclaim.h
*
*    Description: declare the reclaim operation interfaces about database.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __DB_RECLAIM_H__
#define __DB_RECLAIM_H__

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Description:
 *       This function is used to write header sector while doing reclaim.
 * Name: db_reclaim_write_header
 * Parameter: 
 *		NONE.
 * Return Value:
 *	   DB_SUCCESS: success
 *     !DB_SUCCESS: fail
*/
INT32 db_reclaim_write_header(void);

/**
 * Description:
 *       This function is used to reclaim the sectors that db used.
 * Name: db_reclaim
 * Parameter: 
 *		mode: there are 2 mode to reclaim:DB_RECLAIM_OPERATION/DB_RECLAIM_DATA
 * Return Value:
 *	   DB_SUCCESS: success
 *     !DB_SUCCESS: fail
*/
INT32 db_reclaim(UINT8 mode);

#ifdef __cplusplus
 }
#endif


#endif

