/******************************************************
MN_DMD_common.h
----------------------------------------------------
Demodulator IC control functions

<Revision History>
'11/12/27 : OKAMOTO    Update to "MN88472_Device_Driver_111220".
'11/10/24 : OKAMOTO    Update to "MN88472_Device_Driver_111012"
'11/10/19 : OKAMOTO    Correnct warning: " / * " within comment
'11/08/27 : OKAMOTO Update to "MN88472_Device_Driver_110826".
'11/08/26 : OKAMOTO Update to "MN88472_Device_Driver_110823".
'11/08/08 : OKAMOTO [MN88472] Implement "Debug trace".
'11/08/05 : OKAMOTO Update to "MN88472_Device_Driver_110802"
'11/08/01 : OKAMOTO    Correct Error.
'11/07/29 : OKAMOTO    Update to "MN88472_Device_Driver_110715"
----------------------------------------------------
Copyright(C) 2011 SHARP CORPORATION
******************************************************/



#ifndef MN_DMD_COMMON_H
#define MN_DMD_COMMON_H

#ifdef _WINDOWS
#pragma warning(disable: 4200)
#endif

#ifdef MN_DMD_COLLECT_LOG_ENABLE
#include "windows.h"
#include "CollectLogIf.h"
#endif

#include "MN_DMD_driver.h"


/* **************************************************** */
/* Compile switch */
/* **************************************************** */
/* '11/12/27 : OKAMOTO    Move option from project to source code. */
#define DMD_DISABLE_ISDB

#ifdef __cplusplus
extern "C" {
#endif


/* **************************************************** */
/* DMD Defines */
/* **************************************************** */
#define DMD_NOT_SUPPORT        0
#define DMD_SYSTEM_MAX        15
#define DMD_TCB_DATA_MAX    256
#define DMD_REGISTER_MAX    2048
#define DMD_INFORMATION_MAX    512

/* **************************************************** */
/* for Debug */
/* **************************************************** */

/* '11/12/27 : OKAMOTO    Move option from project to source code. */
//#define DMD_DEBUG

#ifdef DMD_DEBUG
#include <stdio.h>
    #ifdef MN_DMD_COLLECT_LOG_ENABLE
        #define DMD_DBG_TRACE(str)    {CollectLog_printf( str ); CollectLog_printf( "\n" ) ; }
        #define DMD_CHECK_ERROR( var, msg )    \
            if( var == DMD_E_ERROR ){ \
            CollectLog_printf( "ERROR:%s" , msg ) ;}
    #else
#define DMD_DBG_TRACE(str)    {fprintf( stderr , str ); fprintf( stderr , "\n" ) ; }
#define DMD_CHECK_ERROR( var, msg )    \
    if( var == DMD_E_ERROR ){ \
    fprintf( stderr , "ERROR:%s" , msg ) ;}
    #endif
#else
#define DMD_DBG_TRACE(str)
#define DMD_CHECK_ERROR( var ,msg );
#endif


#if 1
#define DMD_DEBUG_LOG(param, fmt, args...) \
    do{\
    }while(0)
//#define DMD_DEBUG_LOG(param, fmt, args...) \
//    do{\
//            libc_printf(fmt, ##args);\
//    }while(0)
#else
#define DMD_DEBUG_LOG(param, fmt, args...) \
    do{\
        if ( param->output_buffer && param->fn_output_string )\
        {\
            sprintf(param->output_buffer, fmt, ##args);\
            param->fn_output_string(param->output_buffer);\
        };\
    }while(0)
#endif

/* **************************************************** */
/* Structure & types */
/* **************************************************** */

/*! DMD Text List */
typedef struct
{
    DMD_u32_t        size;
#if 0
    const DMD_text_t        list[];
#else
    /* '11/10/24 : OKAMOTO    Correct warning C4510 */
    DMD_text_t        list[];
#endif
} DMD_textlist_t;

/*! ARRAY FLAG */
typedef enum {
    DMD_E_END_OF_ARRAY = 0,
    DMD_E_ARRAY
} DMD_ARRAY_FLAG_t;

/*! I2C Register Structure */
typedef struct {
    DMD_u8_t    slvadr;
    DMD_u8_t    adr;
    DMD_u8_t    data;
    DMD_ARRAY_FLAG_t    flag;
} DMD_I2C_Register_t;

/* **************************************************** */
/* I2C useful function */
/* **************************************************** */

DMD_ERROR_t    DMD_I2C_MaskWrite(DMD_PARAMETER_t* param, DMD_u8_t    slvadr , DMD_u8_t adr , DMD_u8_t mask , DMD_u8_t  data );

/* **************************************************** */
/*  Functions for Local (not exported) */
/* **************************************************** */
extern DMD_ERROR_t DMD_trans_reg( DMD_PARAMETER_t* param, DMD_I2C_Register_t* t);
extern DMD_u32_t log10_easy( DMD_u32_t cnr );

extern DMD_text_t    DMD_value_decimal( DMD_u32_t val );
extern DMD_text_t    DMD_value_hex( DMD_u32_t val );
extern DMD_text_t    DMD_value_textlist( DMD_textlist_t* list ,DMD_u32_t val );
#ifdef MN88472_RF_SETTING_ENABLE
extern DMD_u32_t    DMD_get_freq_kHz( DMD_PARAMETER_t* param );
#endif

extern DMD_u32_t    DMD_calc_SQI( DMD_PARAMETER_t* param);

extern DMD_ERROR_t DMD_send_registers( DMD_PARAMETER_t* param, DMD_u8_t    *slaveset , DMD_u8_t* regset );
/* **************************************************** */
/*  Text List */
/* **************************************************** */
extern DMD_text_t DMD_info_value_common( DMD_u32_t id , DMD_u32_t val );
extern DMD_text_t DMD_info_value_dvbt( DMD_u32_t id , DMD_u32_t val );
extern DMD_text_t DMD_info_value_dvbt2( DMD_u32_t id , DMD_u32_t val );
extern DMD_text_t DMD_info_value_isdbt( DMD_u32_t id , DMD_u32_t val );
extern DMD_text_t DMD_info_value_isdbs( DMD_u32_t id , DMD_u32_t val );
extern DMD_text_t DMD_info_value_vq( DMD_u32_t id , DMD_u32_t val );
extern DMD_text_t DMD_info_value_dvbc( DMD_u32_t id , DMD_u32_t val );


extern DMD_text_t DMD_INFO_TITLE_COMMON[];
extern DMD_text_t DMD_INFO_TITLE_DVBT[];
extern DMD_text_t DMD_INFO_TITLE_DVBT2[];
extern DMD_text_t DMD_INFO_TITLE_DVBC[];
extern DMD_text_t DMD_INFO_TITLE_ISDBT[];
extern DMD_text_t DMD_INFO_TITLE_ISDBS[];
extern DMD_text_t DMD_INFO_TITLE_ATSC[];
extern DMD_text_t DMD_INFO_TITLE_QAM[];

extern    DMD_textlist_t    DMD_TEXTLIST_SYSTEM    ;
extern    DMD_textlist_t    DMD_TEXTLIST_BW    ;
extern    DMD_textlist_t    DMD_TEXTLIST_YESNO    ;

extern    DMD_textlist_t    DMD_TEXTLIST_LOCK    ;
extern    DMD_textlist_t    DMD_TEXTLIST_DVBT_STATUS    ;
extern    DMD_textlist_t    DMD_TEXTLIST_ERRORFREE    ;
extern    DMD_textlist_t    DMD_TEXTLIST_HIERARCHY_SEL    ;
extern    DMD_textlist_t    DMD_TEXTLIST_TPS_ALL    ;
extern    DMD_textlist_t    DMD_TEXTLIST_DVBT_MODE    ;
extern    DMD_textlist_t    DMD_TEXTLIST_DVBT_GI    ;
extern    DMD_textlist_t    DMD_TEXTLIST_DVBT_CONST    ;
extern    DMD_textlist_t    DMD_TEXTLIST_DVBT_HIERARCHY    ;
extern    DMD_textlist_t    DMD_TEXTLIST_DVBT_CR    ;

extern    DMD_textlist_t    DMD_TEXTLIST_DVBC_STATUS;

extern    DMD_textlist_t    DMD_TEXTLIST_DVBT2_STATUS;
extern    DMD_textlist_t    DMD_TEXTLIST_DVBT2_MODE    ;
extern    DMD_textlist_t    DMD_TEXTLIST_DVBT2_GI    ;
extern    DMD_textlist_t    DMD_TEXTLIST_DVBT2_TYPE    ;
extern    DMD_textlist_t    DMD_TEXTLIST_DVBT2_PAPR    ;
extern    DMD_textlist_t    DMD_TEXTLIST_DVBT2_L1_MOD    ;
extern    DMD_textlist_t    DMD_TEXTLIST_DVBT2_COD    ;
extern    DMD_textlist_t    DMD_TEXTLIST_DVBT2_FEC_TYPE    ;
extern    DMD_textlist_t    DMD_TEXTLIST_DVBT2_PILOT_PATTERN    ;
extern    DMD_textlist_t    DMD_TEXTLIST_DVBT2_PLP_MODE    ;
extern    DMD_textlist_t    DMD_TEXTLIST_DVBT2_PLP_TYPE    ;
extern    DMD_textlist_t    DMD_TEXTLIST_DVBT2_PAYLOAD_TYPE    ;
extern    DMD_textlist_t    DMD_TEXTLIST_DVBT2_PLP_MOD    ;

extern    DMD_textlist_t    DMD_TEXTLIST_ISDBT_MODE    ;
extern    DMD_textlist_t    DMD_TEXTLIST_ISDBT_GI    ;
extern    DMD_textlist_t    DMD_TEXTLIST_ISDBT_SYSTEM    ;
extern    DMD_textlist_t    DMD_TEXTLIST_ISDBT_MAP    ;
extern    DMD_textlist_t    DMD_TEXTLIST_ISDBT_CR    ;
extern    DMD_textlist_t    DMD_TEXTLIST_ISDBT_INT    ;

extern    DMD_textlist_t    DMD_TEXTLIST_ISDBS_MOD    ;
#ifdef __cplusplus
}
#endif

#endif
