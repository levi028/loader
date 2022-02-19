/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: plugin.h
*
*    Description: music plugin use common struct
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __MUSIC_ENGINE_PLUGIN_H_
#define __MUSIC_ENGINE_PLUGIN_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define MUSIC_IS_OUR_FILE       1
#define MUSIC_NOT_OUR_FILE      0
#define MUSIC_OPEN_FILE_FAIL    -1

typedef struct
{
    void *handle;       /* Filled in by app */
    char *filename;     /* Filled in by app */
    char *description;  /* The description that is shown in the preferences box */
    int (*init) (struct pe_music_cfg *pe_music_cfg);    /* Called when the plugin is loaded */
    void (*about) (void);   /* Show the about box */
    void (*configure) (void);
    int (*is_our_file) (char *filename);    /* Return 1 if the plugin can handle the file */
    int (*play_file) (char *filename, mp_callback_func cb); /* Guess what... */
    void (*stop) (void);    /* Tricky one */
    void (*pause) (BOOL paused);    /* Pause or unpause */
    void (*seek) (int time);    /* Seek to the specified time */
    void (*set_eq) (int on, float preamp, float *bands);/* Set the equalizer, most plugins won't use it */
    int (*get_time) (void); /* Get the time, usually returns the output plugins output time */
    void (*get_volume) (int *l, int *r);    /* Input-plugin specific volume functions, just provide a NULL if */
    void (*set_volume) (int l, int r);  /*  you want the output plugin to handle it */
    void (*cleanup) (void);         /*  */

    /* Send data to the visualization plugins Preferably 512 samples/block */
    void (*add_vis_pcm) (int time, music_type fmt, int nch, int length, void *ptr);

    /* Fill in the stuff that is shown in the player window set length to -1 if it's unknown. */
    void (*set_info) (char *title, int length, int rate, int freq, int nch);

    /* Show some text in the song title box in the main window,
     * call it with NULL as argument to reset it to the song title. */
    void (*set_info_text) (char *text);

    int (*get_song_info) (char *filename, music_info *music_info);   /* Function to grab the title string */
    int (*get_decoder_info) (char *filename, decoder_info *decoder_info);        /* get run-time decoder info */
}
input_plugin;

#define _audio_input_plugin(x) const input_plugin x __attribute__ ((section(".audio.init")))

#ifdef __cplusplus
}
#endif

#endif

