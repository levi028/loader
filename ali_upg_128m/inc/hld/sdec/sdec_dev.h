/****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.

   *    File: sdec_dev.h

   *    Description:define the MACRO, the variable and structure uisng by subtitle
        decoder device.
   *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
         KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
         IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
         PARTICULAR PURPOSE.
   *****************************************************************************/

#ifndef    __HLD_SDEC_DEV_H__
#define __HLD_SDEC_DEV_H__


#ifdef __cplusplus
extern "C"
{
#endif

#include <basic_types.h>
#include <mediatypes.h>

struct sdec_device
{
    struct sdec_device  *next;  /*next device */
    INT32  type;
    INT8  name[32];
    INT32  flags;
    INT32 busy;
    void *priv;        /* Used to be 'private' but that upsets C++ */
    INT32 (*init) ();
    INT32 (*open) (struct sdec_device *);
    INT32 (*close) (struct sdec_device *);
    INT32 (*start) (struct sdec_device *,UINT16,UINT16);
    INT32 (*stop) (struct sdec_device *);
    INT32 (*pause) (struct sdec_device *);
    INT32 (*showonoff) (struct sdec_device *,BOOL );
    INT32 (*ioctl)(struct sdec_device *, UINT32 , UINT32);
    INT32 (*request_write)(struct sdec_device *, UINT32 ,
                           struct control_block*, UINT8**, UINT32*);
    void (*update_write)(struct sdec_device *, UINT32);

};


#ifdef __cplusplus
}
#endif


#endif /*__HLD_SDEC_DEV_H__*/










