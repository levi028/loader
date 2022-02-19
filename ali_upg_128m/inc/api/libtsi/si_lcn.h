/*
 * Copyright (C) ALi Shanghai Corp. 2006
 *
 * A logical channel number descriptor definition from Australian TDDB.
 */
#ifndef __SI_LCN_H__
#define __SI_LCN_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <types.h>
#include <sys_config.h>
#include <api/libtsi/si_types.h>

struct lcn_info
{
    UINT8 service_id[2];
    UINT8 reserved                  : 6;
    UINT8 logical_channel_number_hi : 2;
    UINT8 logical_channel_number_lo;
}__attribute__((packed));

/*
 * For australian DVB-T.
 */
struct logical_channel_descriptor
{
    UINT8 descriptor_tag;
    UINT8 descriptor_length;
    struct lcn_info info[0];
}__attribute__((packed));

#ifdef __cplusplus
}
#endif

#endif /* __SI_LCN_H__ */

