/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_pvr_hint.h

*    Description: recording hint menu, indicate if recording or not.

*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_PVR_HINT_H_
#define _WIN_PVR_HINT_H_

#ifdef __cplusplus
extern "C"
{
#endif

/* function declare */
BOOL get_rec_hint_state(void);
BOOL set_rec_hint_state(BOOL flag);
void show_rec_hint_osdon_off(UINT8 flag);


#ifdef __cplusplus
}
#endif

#endif //_WIN_PVR_HINT_H_


