#ifndef __SI_SERVICE_TYPE_H__
#define __SI_SERVICE_TYPE_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _INVW_JUICE
#include <basic_types.h>
#endif
enum
{
    SERVICE_TYPE_DTV            = 0x01,
    SERVICE_TYPE_DRADIO         = 0x02,
    SERVICE_TYPE_TELTEXT        = 0x03,
    SERVICE_TYPE_NVOD_REF       = 0x04,
    SERVICE_TYPE_NVOD_TS        = 0x05,
    SERVICE_TYPE_MOSAIC         = 0x06,
    SERVICE_TYPE_FMRADIO        = 0x0a,
    SERVICE_TYPE_DATABROAD      = 0x0C,
#ifdef ITALY_HD_BOOK_SUPPORT
    SERVICE_TYPE_DVBMHP         = 0x10,
#endif
    SERVICE_TYPE_HD_MPEG2       = 0x11,
    SERVICE_TYPE_SD_MPEG4       = 0x16,
    SERVICE_TYPE_HD_MPEG4       = 0x19,
#ifdef ITALY_HD_BOOK_SUPPORT
    SERVICE_TYPE_FRAME_COMPATIBLE_STEREOSCOPIC_HD_DTV = 0x1C,
#endif
    SERVICE_TYPE_HEVC = 0x1F, 
    SERVICE_TYPE_DATA_SERVICE   = 0xC0,
        //private defined advertise service
    SERVICE_TYPE_PRIVATE        = 0xFE,
};

RET_CODE psi_service_type_exist(UINT8 service_type);

#ifdef __cplusplus
}
#endif

#endif /* __SI_SERVICE_TYPE_H__ */

