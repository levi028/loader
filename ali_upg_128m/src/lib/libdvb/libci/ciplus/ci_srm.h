/*****************************************************************************
 *    Copyrights(C) 2010 ALI Corp. All Rights Reserved.
 *
 *    FILE NAME:        ci_srm.h
 *
 *    DESCRIPTION:    provide api for apply SRM data, and extern api in HDCP module
 *
 *    HISTORY:
 *        Date         Author           Version       Notes
 *        =========    =========      =========      =========
 *        2010/6/28   Elliott          0.1
 *
 *****************************************************************************/

#include <openssl/dsa.h>
#include <openssl/objects.h>
#include <openssl/sha.h>
#include <hld/hld_dev.h>
#include <bus/hdmi/m36/hdmi_dev.h>
#include "ci_stack.h"
#include <api/libc/string.h>


void api_ciplus_parse_apply_srm(UINT8 *srm_file, UINT32 srm_file_len);

extern INT32 hdmi_proc_ioctl(struct hdmi_device * dev, enum HDMI_IO_CMD_TYPE cmd, UINT32 param1, UINT32 param2);

