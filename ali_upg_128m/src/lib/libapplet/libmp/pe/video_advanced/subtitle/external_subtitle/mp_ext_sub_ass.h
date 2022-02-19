#ifndef __MEDIA_PLAYER_SUBTITLE_ASS_H_
#define __MEDIA_PLAYER_SUBTITLE_ASS_H_


#ifdef __cplusplus
extern "C" {
#endif


void mp_subtitle_parse_ass(void *sub, char *str, unsigned long *pos, unsigned long *time);


#ifdef __cplusplus
}
#endif


#endif

