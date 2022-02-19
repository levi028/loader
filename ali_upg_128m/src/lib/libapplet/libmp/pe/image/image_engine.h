#ifndef __PICTURE_ENGINE_H__
#define __PICTURE_ENGINE_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define IMAGE_DEC_E_OK              0
#define IMAGE_DEC_E_FAIL            -1
#define IMAGE_DEC_E_OPEN_FAIL       -2

struct image_config
{
    char *file_name;

    unsigned char    decode_mode;
    unsigned char    show_mode;
    unsigned char    vpo_mode;
    unsigned char    rotate;

    //rect for source
    unsigned short    src_left;
    unsigned short    src_top;
    unsigned short    src_width;
    unsigned short    src_height;

    //rect for display
    unsigned short    dest_left;
    unsigned short    dest_top;
    unsigned short    dest_width;
    unsigned short    dest_height;

    //slide show mode
    struct image_slideshow_effect *effect;

    //callback function
    mp_callback_func mp_cb;


};

struct image_3d_para
{
    int output_format;
    int user_depth;
    int user_shift;
    int user_vip;
};

int image_engine_decode(char *filename, struct image_config *cfg);
int image_engine_show();
int image_engine_abort();
int image_engine_init(struct pe_image_cfg *pe_image_cfg);
int image_engine_cleanup();
int image_engine_get_info(char *filename, struct image_info *info);
int image_engine_zoom(struct rect *dst_rect, struct rect *src_rect);
int image_engine_rotate(unsigned char rotate_angle);
int image_engine_2d_to_3d_swap(int type);
int image_engine_set_disp_param(int display_type, struct image_3d_para *para);

#ifdef __cplusplus
}
#endif

#endif //__PICTURE_ENGINE_H__




