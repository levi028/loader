#ifndef __SI_SDTT_H__
#define __SI_SDTT_H__
#include <types.h>
#include <sys_config.h>

#include "si_module.h"

struct sdtt_section_info
{
    INT16 max_map_nr;
    INT16 map_counter;
    UINT16 ts_id;
    UINT16 nit_pid;
    struct extension_info ext[1];
    struct program_map *map;
};


#define SDTT_DOWNLV_DISCRETIONARY   0x00
#define SDTT_DOWNLV_COMPULSORY      0x01

#define SDTT_VER_INDICAT_ALL        0x00
#define SDTT_VER_INDICAT_LATER      0x01
#define SDTT_VER_INDICAT_EARLIER    0x02
#define SDTT_VER_INDICAT_ONLY       0x03

#define SDTT_COMPAB_DESC_TYPE_HW    0x01
#define SDTT_COMPAB_DESC_TYPE_SW    0x02

#ifdef __cplusplus
extern "C" {
#endif

void st_sdtt_set_channel_index(UINT16 chanindex);
INT32 si_sdtt_parser(UINT8 *pat, INT32 len, struct section_parameter *param);

#ifdef __cplusplus
}
#endif

#endif /* __SI_SDTT_H__ */
