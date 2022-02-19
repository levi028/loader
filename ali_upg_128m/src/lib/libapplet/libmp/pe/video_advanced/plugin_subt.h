#ifndef __PLUGIN_SUBT_H__
#define __PLUGIN_SUBT_H__

typedef int  (*pfn_subtitle_create)(void **psubtitle_osd,
                                    unsigned long left, unsigned long top,
                                    unsigned long pic_width, unsigned long pic_height,
                                    void **font_array, UINT32 nb_font_array);
typedef long (*pfn_subtitle_control)(void *subtitle_osd, long control_code, long parameters);
typedef int  (*pfn_subtitle_destroy)(void *subtitle_osd);

typedef struct
{
    const char              *name;
    pfn_subtitle_create     subtitle_create;
    pfn_subtitle_control    subtitle_control;
    pfn_subtitle_destroy    subtitle_destroy;
} SUBT_INPUT_PLUGIN;

#ifndef WIN32
    #define _subt_input_plugin(x) const SUBT_INPUT_PLUGIN x __attribute__ ((section(".subt.plugin")))
#else
    #define _subt_input_plugin(x) SUBT_INPUT_PLUGIN x
#endif

#endif// __PLUGIN_SUBT_H__


