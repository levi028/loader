/*****************************************************************************
 *    Copyright (c) 2013 ALi Corp. All Rights Reserved
 *    This source is confidential and is ALi's proprietary information.
 *    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
 *    File: win_com_frontend.h
 *
 *    Description: To realize the common function of FE
 *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _WIN_COM_FRONTEND_H_
#define _WIN_COM_FRONTEND_H_
#include <basic_types.h>

#ifdef __cplusplus
extern "C"
{
#endif

extern UINT16 lnb_type_ids[4];
extern UINT16 diseqc_port_ids[];
#ifdef DVBS_SUPPORT
extern void sat2antenna(S_NODE *sat, struct ft_antenna *antenna);
#endif

#ifdef __cplusplus
}
#endif

#endif//_WIN_COM_FRONTEND_H_


