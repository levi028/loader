#ifndef __MEDIA_PLAYER_SUBTITLE_PRIV_H_
#define __MEDIA_PLAYER_SUBTITLE_PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "mp_plugin_ext_sub.h"

void mp_subtitle_get_time(unsigned long *time, char **xtime1, unsigned long multiple, char const_c);
void mp_subtitle_unicode_get_time(unsigned long *time, UINT16 **xtime1, unsigned long multiple, UINT16 const_c);
void mp_subtitle_get_timenum(unsigned long *time, char **xtime1, unsigned long multiple, unsigned long utf8);
void mp_subtitle_unicode_get_timenum(unsigned long *time, UINT16 **xtime1, unsigned long multiple, unsigned long utf8);
void mp_subtitle_vobsub_spu_next_line(mp_vob_sub_spu *spu);
long mp_subtitle_vobsub_parse_forced_subs(mp_vob_sub *vob, const char *line);
void mp_subtitle_fix(mp_subtitle *subtitle, char *str,unsigned long *pos);
long  mp_subtitle_get_previous_sub(mp_subtitle *subtitle);
long mp_subtitle_get_next_sub(mp_subtitle *subtitle);
long mp_subtitle_get_sub(mp_subtitle *subtitle, unsigned long curtime);
long mp_subtitle_get_type_by_name(char *name);
long mp_subtitle_parse_file(void *handle);
void mp_subtitle_stop(mp_subtitle *subtitle);
void mp_subtitle_pause(mp_subtitle *subtitle, long paused);


#ifdef __cplusplus
}
#endif


#endif

