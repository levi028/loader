#ifndef __PLUGIN_PICTURE_H__
#define __PLUGIN_PICTURE_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define IMAGE_IS_OUR_FILE       1
#define IMAGE_NOT_OUR_FILE      0
#define IMAGE_OPEN_FILE_FAIL    -1

typedef int image_raw_id;

typedef struct
{
    int (*raw_alloc)(image_raw_id id);
    int (*raw_free)(image_raw_id id);
    int (*raw_initconf)(image_raw_id id, void* config);
    int (*raw_init)(image_raw_id id);
    int (*raw_decconf)(image_raw_id id, void* config);
	int (*raw_decode)(image_raw_id id);
    int (*raw_dispconf)(image_raw_id id, void* config);
    int (*raw_display)(image_raw_id id);
    int (*raw_abort)(image_raw_id id);
    int (*raw_info)(image_raw_id id, void* info);
} plugin_image_raw;


typedef struct
{
    int (*init_opentv) (struct pe_image_cfg *cfg);
	int (*decode_opentv) (char *filename, UINT32 addr, UINT32 len, int scale_coef, void *args);
    int (*display_buffer) (st_image_buffer *image_buffer, int layer, struct rect *src_rect, struct rect *dst_rect);
    int (*get_info_opentv) (char *filename, struct image_info *info);
    int (*get_decoded_buffer )(void);
    int (*abort_opentv )(void);
} image_plugin_opentv;

typedef struct
{
    void *handle;                            /* Filled in by app */
    char *filename;                            /* Filled in by app */
    char *description;                        /* The description that is shown in the preferences box */
    int (*init) (struct pe_image_cfg *cfg);                    /* Called when the plugin is loaded */
    void (*about) (void);                    /* Show the about box */
    void (*configure) (void);
    int (*is_our_file) (char *filename, void *args);   /* Return 1 if the plugin can handle the file */
    int (*decode) (char *filename, void *args);        /* deocode the image */
    int (*show) (void);                            /* deocode the image */
    int (*abort) (void);                    /* Tricky one */
    int (*cleanup) (void);                    /* Called when exit */
    int (*get_info) (char *filename, struct image_info *info);    /* Function to grab the title string */

    int (*rotate)(unsigned char rotate_angle);    /*Function to rotate current JPEG*/
    int (*zoom)(struct rect *dst_rect, struct rect *src_rect);
    int (*swap) (int type);
    int (*option) (int output_format, int user_depth, int user_shift, int user_vip);
    void (*set_param) (int display_type, int output_format, int user_depth, int user_shift, int user_vip);

    image_plugin_opentv  opentv;

    plugin_image_raw* raw;
}image_plugin;




#ifdef __cplusplus
}
#endif

#endif// __PLUGIN_PICTURE_H__


