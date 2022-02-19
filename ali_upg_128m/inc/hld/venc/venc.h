#ifndef    __HLD_VENC_H__
#define __HLD_VENC_H__

#include <basic_types.h>
#include <hld/decv/decv.h>

enum venc_img_type
{
    VENC_IMG_BMP,
    VENC_IMG_JPG
};

struct venc_img
{
    enum venc_img_type type;

    UINT8 *img_buf;
    UINT32 img_buf_size;
    UINT32 img_buf_valid_size;
};

#include "venc_dev.h"

#ifdef __cplusplus
extern "C"
{
#endif

INT32 venc_open(struct venc_device* dev);
INT32 venc_close(struct venc_device *dev);
INT32 venc_i_frame(struct venc_device *dev,UINT8* y_addr,UINT8* c_addr,UINT16 width,\
                    UINT16 height,UINT8* out_stream_buf,UINT32* out_stream_size);
//INT32  venc_start(struct venc_device *dev);
//INT32  venc_stop(struct venc_device *dev);

INT32 venc_mp2enc(struct venc_device *dev,UINT8* y_addr,UINT8* c_addr,UINT16 width,UINT16 height,\
                 UINT8* out_stream_buf,UINT32* out_stream_size,UINT8* f_y_addr,UINT8* f_c_addr);
INT32 venc_img_enc(struct venc_device *dev, struct vdec_picture *ppic, struct venc_img *pimg);
#ifdef __cplusplus
}
#endif

#endif /*__HLD_VENC_H__*/

