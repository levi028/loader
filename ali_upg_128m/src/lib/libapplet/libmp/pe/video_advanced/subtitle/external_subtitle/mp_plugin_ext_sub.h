

#ifndef __MEDIA_PLAYER_SUBTITLE_H_
#define __MEDIA_PLAYER_SUBTITLE_H_


#ifdef __cplusplus
extern "C" {
#endif


//#define MP_SUB_DEBUG  libc_printf
#define MP_SUB_DEBUG(...)

#define MP_SUBTITLE_PLAY_TASK_STACKSIZE 0x4000
#define MP_SUBTITLE_PLAY_TASK_QUANTUM 10
#define MP_SUBTITLE_MBF_SIZE 1024


#define MP_SUBTITLE_MAX_SUB_FILES            6
#define MP_SUBTITLE_MAX_SUB_COUNT        5000

#define TICKSPERSEC                            1

#define isspace(c)           (' ' == c || '\f' == c || '\n' == c || '\r' == c || '\t' == c || '\v' == c)

//#define wchar_t unsigned short


typedef struct
{
    OSAL_ID    message_buffer_id;
    OSAL_ID flag_id;
    OSAL_ID task_id;
}mp_subtitle_parameter;


typedef enum
{
    MP_SUBTITLE_FLAG_INITIALIZE                = 0x00000000,
    MP_SUBTITLE_FLAG_SUCCESS                    = 0x00000001,
    MP_SUBTITLE_FLAG_UNSUCCESSFUL            = 0x00000002,
    MP_SUBTITLE_FLAG_TASK_EXIT                = 0x00000004,
    MP_SUBTITLE_FLAG_MASK                    = 0x00000007,
}mp_subtitle_flag;


typedef enum
{
    MP_SUBTITLE_TASK_STOPPED = 0,
    MP_SUBTITLE_TASK_RUNNING,
}mp_subtitle_task_status;


typedef enum
{
    MP_SUBTITLE_TASK_COMMAND_NONE = 0,
    MP_SUBTITLE_TASK_COMMAND_STOP,
    MP_SUBTITLE_TASK_COMMAND_PAUSE,
}mp_subtitle_task_command;


typedef struct
{
    mp_subtitle_parameter        parameter;
    int                        status;
    int                        command;
}mp_subtitle_task_control;


typedef enum
{
    MP_SUBTITLE_TXT_TYPE_ANSI = 0,
    MP_SUBTITLE_TXT_TYPE_L_UNICODE,
    MP_SUBTITLE_TXT_TYPE_B_UNICODE,
    MP_SUBTITLE_TXT_TYPE_UTF8,
    MP_SUBTITLE_TXT_TYPE_UTF7,

}mp_txt_type;


typedef struct
{
    int            txt_type;
    int            bom;
}mp_txt_file, *pmp_txt_file;


typedef enum
{
    MP_SUBTITLE_TYPE_UNKNOWN = 0,
    MP_SUBTITLE_TYPE_SMI,
    MP_SUBTITLE_TYPE_ASS,
    MP_SUBTITLE_TYPE_SSA,
    MP_SUBTITLE_TYPE_SRT,
    MP_SUBTITLE_TYPE_SUB,
    MP_SUBTITLE_TYPE_TXT,


    MP_SUBTITLE_TYPE_MAX,
}mp_subtitle_type;

#define MP_SUBTITLE_TYPE_NUMBER (MP_SUBTITLE_TYPE_MAX-1)

typedef struct
{
    mp_subtitle_osd_create_function            osd_create;
    mp_subtitle_osd_destroy_function            osd_destroy;
    mp_subtitle_osd_control_function            osd_control;
}mp_subtitle_osd_functions, *pmp_subtitle_osd_functions;


typedef struct
{
    int                avail_file_type;
    char                avail_sub_file[FULL_PATH_SIZE];
}mp_sub_files, *pmp_sub_files;

typedef struct
{
    int                    current_sub_file_id;

    mp_txt_file                txt_file;

    int                    avail_file_count;
    mp_sub_files            avail_sub_files[MP_SUBTITLE_MAX_SUB_FILES];


    int                    current_pos;

    unsigned long            *position_offset;
    unsigned long            *position_time;

    int                    total_sub_count;

    char                    *file_buffer;
    unsigned long            file_buffer_size_limit;
    char                    file_buffer_assigned;

    mp_vob_sub            vob_sub;

    void                    *subtitle_osd;
    mp_subtitle_osd_functions    subtitle_osd_functions;

    mp_subtitle_get_time_function    get_time;
    int                            time_in_ms;


    mp_subtitle_task_control    sub_task_control;
}mp_subtitle, *pmp_subtitle;


extern mp_subtitle_string_to_unicode mp_subtitle_callback;
extern const EXT_SUB_PLUGIN mp_subtitle_plugin __attribute__ ((section(".extsubt.plugin")));



#ifdef __cplusplus
}
#endif


#endif

