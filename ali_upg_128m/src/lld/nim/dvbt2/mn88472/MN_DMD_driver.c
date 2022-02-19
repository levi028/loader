/******************************************************
MN_DMD_driver.c
----------------------------------------------------
Demodulator IC control functions

<Revision History>
'11/10/24 : OKAMOTO    Update to "MN88472_Device_Driver_111012"
'11/10/19 : OKAMOTO    Correct warning: enumeration value ÅeDMD_E_ISDBTÅf not handled in switch
'11/08/29 : OKAMOTO    Select TS output.
'11/08/29 : OKAMOTO    Error Flag output.
'11/08/27 : OKAMOTO Update to "MN88472_Device_Driver_110826".
'11/08/26 : OKAMOTO Update to "MN88472_Device_Driver_110823".
'11/08/05 : OKAMOTO Update to "MN88472_Device_Driver_110802"
'11/07/29 : OKAMOTO    Update to "MN88472_Device_Driver_110715"
----------------------------------------------------
Copyright(C) 2011 SHARP CORPORATION
******************************************************/
#ifdef _WINDOWS
#pragma warning(disable: 4200)
#endif

#include <osal/osal.h>

#include "MN_DMD_driver.h"
#include "MN_DMD_device.h"
#include "MN_DMD_common.h"
#include "MN_I2C.h"
#ifdef MN88472_RF_SETTING_ENABLE
#include "MN_Tuner.h"
#endif

/* **************************************************** */
/* Local Functions */
/* **************************************************** */

/* **************************************************** */
/* Definition of Constant Strings */
/* **************************************************** */
const char g_dmd_err_str[DMD_ERROR_MAX][DMD_ERR_STR_SIZE]={
    "MN88472 OK",
    "MN88472 ERROR",
    "MN88472 Unlock"
};

/* **************************************************** */
/* Public Functions  */
/* **************************************************** */
#ifdef MN88472_DEVICE_ID_ENABLE
/*! Demodulator set device ID */
DMD_ERROR_t DMD_API DMD_set_devid( DMD_PARAMETER_t* param , DMD_u32_t id ){
    param->devid = id;

    return DMD_E_OK;
}
#endif

/*!    Demodulator API Initialize & Open */
/*!
    *initialize parameter variable (DMD_PARAMETER_t* param)
    *open & connect to USB Device for I2C communication
     (for WINDOWS API only)
*/
DMD_ERROR_t DMD_API DMD_open( DMD_PARAMETER_t* param)
{
    int i;

 #ifdef MN88472_DEVICE_ID_ENABLE
    param->devid = 0;
 #endif
    param->system= DMD_E_NOT_DEFINED;
    /* Initialize Parameter Variable */
    for(i=0;i<DMD_INFORMATION_MAX;i++){
        param->info[i] = 0;        //NOT_DEFINED
    }
    param->bw        = DMD_E_BW_NOT_SPECIFIED;
  #ifdef MN88472_RF_SETTING_ENABLE
    param->freq        = 666000;
    param->funit    = DMD_E_KHZ;
  #endif    /* #ifdef MN88472_RF_SETTING_ENABLE */

#if 0    /* Not used on SH EVA board. */
    if( DMD_device_open( param ) == DMD_E_ERROR )
    {
        DMD_DBG_TRACE( "ERROR: DMD open" );
        return DMD_E_ERROR;

    }

    if( DMD_I2C_open() == DMD_E_ERROR )
    {
        DMD_DBG_TRACE( "ERROR: I2C open" );
        return DMD_E_ERROR;
    }
#endif
    return DMD_E_OK;
}

/*! Demodulator LSI Initialize */
/*!
    *release power down
    *Tuner Initialize
    *transfer common register settings via I2C
    *transfer auto countrol sequence via I2C
*/
DMD_ERROR_t    DMD_API    DMD_init( DMD_PARAMETER_t* param )
{
#ifdef MN_TUNER_H
    /* Tuner_init */
    if( DMD_Tuner_init(param) == DMD_E_ERROR )
    {
        DMD_DBG_TRACE( "ERROR: Tuner Initialize" );
        return DMD_E_ERROR;
    }
#endif

    /* Demodulator LSI Initialize */
    if( DMD_device_init( param ) == DMD_E_ERROR )
    {
        DMD_DBG_TRACE( "ERROR: DMD Initialize" );
        return DMD_E_ERROR;
    }
    /* Auto Control Sequence Transfer */
    if( DMD_device_load_pseq( param ) == DMD_E_ERROR )
    {
        DMD_DBG_TRACE( "ERROR: DMD Load Autocontrol" );
        return DMD_E_ERROR;
    }
    return DMD_E_OK;
}

/*! Select Broadcast Sytem */
/*!
    *Transfer register settings for each system
*/
DMD_ERROR_t DMD_API DMD_set_system( DMD_PARAMETER_t* param ){
#ifdef MN_TUNER_H
    if( DMD_Tuner_set_system( param ) == DMD_E_ERROR ){
        DMD_DBG_TRACE("ERROR:Tuner set system");
        return DMD_E_ERROR;
    }
#endif

    if( DMD_device_set_system( param ) == DMD_E_ERROR ){
        DMD_DBG_TRACE("ERROR:device set system");
        return DMD_E_ERROR;
    }

    /* '11/08/29 : OKAMOTO    Error Flag output. */
    if( DMD_set_error_flag_output(param, 1 ) == DMD_E_ERROR ){
        DMD_DBG_TRACE("ERROR:set error flag output");
        return DMD_E_ERROR;
    }

    /* '11/08/29 : OKAMOTO    Select TS output. */
    if( DMD_set_ts_output(param, param->ts_out ) == DMD_E_ERROR ){
        DMD_DBG_TRACE("ERROR:set ts output");
        return DMD_E_ERROR;
    }


    return DMD_E_OK;
}

/*! Tune */
DMD_ERROR_t DMD_API DMD_tune( DMD_PARAMETER_t* param )
{
    DMD_ERROR_t ret;
    /* Call pre tune process */
    ret = DMD_device_pre_tune( param );
 #ifdef MN_TUNER_H
    /* Tuner */
    ret |= DMD_Tuner_tune( param );
 #endif
    /* Call post tune process */
    ret |= DMD_device_post_tune( param );
    return ret;
}

/*! scan */
DMD_ERROR_t DMD_API DMD_scan( DMD_PARAMETER_t* param )
{
    DMD_ERROR_t    ret;
    /* Call pre tune process */
    DMD_device_pre_tune( param );

 #ifdef MN_TUNER_H
    /* Tuner */
    DMD_Tuner_tune( param );
 #endif
    /* Call post tune process */
    DMD_device_post_tune( param );

    /* Call Recieve Status Judgement */
    ret = DMD_device_scan( param );
    return ret;
}

/*! term */
DMD_ERROR_t DMD_API DMD_term( DMD_PARAMETER_t* param )
{
    DMD_ERROR_t    ret;
    ret = DMD_E_OK;

    ret = DMD_device_term(param);
#ifdef MN_TUNER_H
    ret |= DMD_Tuner_term();
#endif

    return ret;
}

/*! close */
DMD_ERROR_t DMD_API DMD_close( DMD_PARAMETER_t* param )
{
    DMD_ERROR_t    ret;
    ret = DMD_E_OK;

    ret = DMD_device_close(param);

    return ret;
}

/* **************************************************** */
/* Set Information */
/* **************************************************** */
DMD_ERROR_t DMD_API DMD_set_info( DMD_PARAMETER_t* param , DMD_u32_t id  , DMD_u32_t val){

    return DMD_device_set_info( param , id , val );
}

/* **************************************************** */
/* Get Information */
/* **************************************************** */
DMD_ERROR_t DMD_API DMD_get_info( DMD_PARAMETER_t* param , DMD_u32_t id ){

    return DMD_device_get_info( param , id );
}

DMD_text_t DMD_API DMD_info_title( DMD_SYSTEM_t sys ,DMD_u32_t id )
{

    if( id < DMD_E_INFO_COMMON_END_OF_INFORMATION )
    {
        return DMD_INFO_TITLE_COMMON[id];

    }
    else
    switch( sys )
    {
    case DMD_E_DVBT:
        if( id < DMD_E_INFO_DVBT_END_OF_INFORMATION ){
            id -= DMD_E_INFO_COMMON_END_OF_INFORMATION;
            return DMD_INFO_TITLE_DVBT[id];
        }
        else
        {
            return "Not Defined";
        }
        break;
    case DMD_E_DVBT2:
        if( id < DMD_E_INFO_DVBT2_END_OF_INFORMATION ){
            id -= DMD_E_INFO_COMMON_END_OF_INFORMATION;
            return DMD_INFO_TITLE_DVBT2[id];
        }
        else
        {
            return "Not Defined";
        }
        break;
    case DMD_E_DVBC:
        /*
        if( id < DMD_E_INFO_DVBC_END_OF_INFORMATION ){
            id -= DMD_E_INFO_COMMON_END_OF_INFORMATION;
            return DMD_INFO_TITLE_DVBC[id];
        }
        else
        {
            return "Not Defined";
        }
        */
        return "Not Defined";
        break;
  #ifndef DMD_DISABLE_ISDB
    case DMD_E_ISDBT:
    case DMD_E_ISDBT_BRAZIL:
        if( id < DMD_E_INFO_ISDBT_END_OF_INFORMATION ){
            id -= DMD_E_INFO_COMMON_END_OF_INFORMATION;
            return DMD_INFO_TITLE_ISDBT[id];
        }
        else
        {
            return "Not Defined";
        }
        break;
    case DMD_E_ISDBS:
        if( id < DMD_E_INFO_ISDBS_END_OF_INFORMATION ){
            id -= DMD_E_INFO_COMMON_END_OF_INFORMATION;
            return DMD_INFO_TITLE_ISDBS[id];
        }
        else
        {
        return "Not Defined";
        }
        break;
#endif
    /* '11/10/19 : OKAMOTO    Correct warning: enumeration value ÅeDMD_E_ISDBTÅf not handled in switch */
    default:
        return "Not Defined";
        break;
    }
#if 0    /* '11/12/27 : OKAMOTO    Correct warning C4702: êßå‰Ç™ìnÇÁÇ»Ç¢ÉRÅ[ÉhÇ≈Ç∑ÅB */
    return "Not Defined";
#endif
}

DMD_text_t DMD_API DMD_info_value( DMD_SYSTEM_t sys ,DMD_u32_t id , DMD_u32_t val )
{


    if( id < DMD_E_INFO_COMMON_END_OF_INFORMATION && id != DMD_E_INFO_STATUS )
    {
        return DMD_info_value_common( id ,val );

    }
    else
    {
        switch( sys )
        {
        case DMD_E_DVBT2:
            return DMD_info_value_dvbt2( id , val );
            break;
        case DMD_E_DVBT:
            return DMD_info_value_dvbt( id , val );
            break;
    #ifndef DMD_DISABLE_ISDB
        case DMD_E_ISDBT:
        case DMD_E_ISDBT_BRAZIL:
            return DMD_info_value_isdbt( id , val );
            break;
        case DMD_E_ISDBS:
            return DMD_info_value_isdbs( id , val );
    #endif
        case DMD_E_ATSC:
        case DMD_E_QAMB_256QAM:
        case DMD_E_QAMB_64QAM:
        case DMD_E_QAMC_256QAM:
        case DMD_E_QAMC_64QAM:
            return DMD_info_value_vq( id , val );
        case DMD_E_DVBC:
            return DMD_info_value_dvbc( id , val );
            break;
        /* '11/10/19 : OKAMOTO    Correct warning: enumeration value ÅeDMD_E_ISDBTÅf not handled in switch */
        default:
            return "Not Defined";
            break;
        }
    }
#if 0    /* '11/12/27 : OKAMOTO    Correct warning C4702: êßå‰Ç™ìnÇÁÇ»Ç¢ÉRÅ[ÉhÇ≈Ç∑ÅB */
    return "Not Defined";
#endif
}
DMD_text_t DMD_API DMD_value_text( DMD_PARAMETER_t* param , DMD_u32_t id )
{
    return DMD_info_value( param->system , id , param->info[id] );
}


/* **************************************************** */
/* Floating Functions */
/* **************************************************** */
// Get BER
DMD_float_t DMD_API DMD_get_ber( DMD_PARAMETER_t* param )
{
    DMD_float_t ret;
 #ifdef DMD_FLOATING_FUNCTION
    DMD_get_info( param , DMD_E_INFO_BERRNUM );

    if( param->info[DMD_E_INFO_BITNUM] == 0 )
    {
        ret = 1.0;
    }
    else
    {

        ret = (DMD_float_t) param->info[DMD_E_INFO_BERRNUM]
            / (DMD_float_t) param->info[DMD_E_INFO_BITNUM];
    }
 #else
    ret = 1;
 #endif
    return ret;

}
DMD_float_t DMD_API DMD_get_per( DMD_PARAMETER_t* param )
{
    DMD_float_t ret;
 #ifdef DMD_FLOATING_FUNCTION
    DMD_get_info( param , DMD_E_INFO_PACKETNUM );

    if( param->info[DMD_E_INFO_PACKETNUM] == 0 )
    {
        ret = 1.0;
    }
    else
    {

        ret = (DMD_float_t) param->info[DMD_E_INFO_PERRNUM]
            / (DMD_float_t) param->info[DMD_E_INFO_PACKETNUM];
    }
 #else
    ret = 1;
 #endif
    return ret;

}
//Get CNR
DMD_float_t DMD_API DMD_get_cnr( DMD_PARAMETER_t* param )
{
    DMD_float_t    ret;
    DMD_get_info( param , DMD_E_INFO_CNR_INT );
 #ifdef DMD_FLOATING_FUNCTION

    ret = (DMD_float_t) param->info[DMD_E_INFO_CNR_INT]
         +  (DMD_float_t) param->info[DMD_E_INFO_CNR_DEC] / ( (DMD_float_t) 100.0 );
 #else
    ret = (DMD_float_t) param->info[DMD_E_INFO_CNR_INT];
 #endif


    return ret;
}

int Get_RF_T_REF_PWR(DMD_PARAMETER_t * param)
{
    int   T_REF_PWR;
    DMD_DVBT_CONST_t   constellation;
    DMD_DVBT_HIERARCHY_t   hierarchy_exist;
    DMD_DVBT_HIER_SEL_t hierarchy_selection;
    DMD_DVBT_CR_t   coderate;

    T_REF_PWR = -100;

    if ( param->system != DMD_E_DVBT)
      {
//                   SHARP6158_LOG(param, " ERROR : Get_T_REF_PWR, system is not DVB-T @!!\n");
             return  T_REF_PWR;
      }

    // get LP constelation mode ; HP constelation only one : QPSK
    DMD_get_info(param, DMD_E_INFO_DVBT_CONSTELLATION);
    constellation = param->info[DMD_E_INFO_DVBT_CONSTELLATION];

    // Check HP stream exist or not
    DMD_get_info(param, DMD_E_INFO_DVBT_HIERARCHY);
    hierarchy_exist= param->info[DMD_E_INFO_DVBT_HIERARCHY];

    DMD_get_info(param, DMD_E_INFO_DVBT_HP_CODERATE);
    DMD_get_info(param, DMD_E_INFO_DVBT_LP_CODERATE);
    // 0 : DMD_E_DVBT_HIER_SEL_LP(default),    1: DMD_E_DVBT_HIER_SEL_HP
    // [ Attention !]Selection should be executed at topper layer:
    // DMD_device_set_info(param , DMD_E_INFO_DVBT_HIERARCHY_SELECT , DMD_u32_t val ) // val -> 1 :  HP ; 0 : LP
    hierarchy_selection = param->info[DMD_E_INFO_DVBT_HIERARCHY_SELECT];

    if  (hierarchy_exist != DMD_E_DVBT_HIERARCHY_NO)
    {
        if (hierarchy_selection == DMD_E_DVBT_HIER_SEL_HP)//Hierarchy, Alpha 1,2,4 & Customer chooses to decode HP data stream
        {
            coderate = param->info[DMD_E_INFO_DVBT_HP_CODERATE];
            constellation = DMD_E_DVBT_CONST_QPSK; //HP only supports QPSK
        }
        else //Non-hierarchy OR Customer choose LP when HP&LP both transmitted.
        {
            coderate = param->info[DMD_E_INFO_DVBT_LP_CODERATE];
        }
    }
    else //no hierarchy
    {
        //troy, 20130131, tested, found that if not open HIERARCHY, DVB-T code rate gets from HP area.
         coderate = param->info[DMD_E_INFO_DVBT_HP_CODERATE];
    }

//    SHARP6158_LOG(param, "Get_RF_T_REF_PWR(): constellation=%d coderate=%d , hierarchy_selection = %d( 1 : HP; 0 : LP ) \n",constellation,coderate,hierarchy_selection);

/*
typedef enum{    DMD_E_DVBT_CONST_QPSK=0,    DMD_E_DVBT_CONST_16QAM,
    DMD_E_DVBT_CONST_64QAM    }    DMD_DVBT_CONST_t;

typedef enum{    DMD_E_DVBT_CR_1_2=0,    DMD_E_DVBT_CR_2_3,
    DMD_E_DVBT_CR_3_4,    DMD_E_DVBT_CR_5_6,    DMD_E_DVBT_CR_7_8    }    DMD_DVBT_CR_t;
*/

/*--- Based on Nordig 2.3, chapter 3.4.4.6 Requirements for the signal strength indicator (SSI) ---*/
if      ( constellation == DMD_E_DVBT_CONST_QPSK ) {                   // QPSK
          if      ( coderate == 0 ) T_REF_PWR = -93;        //  1/2
          else if ( coderate == 1 ) T_REF_PWR = -91;        //  2/3
          else if ( coderate == 2 ) T_REF_PWR = -90;        //  3/4
          else if ( coderate == 3 ) T_REF_PWR = -89;        //  5/6
          else if ( coderate == 4 ) T_REF_PWR = -88;        //  7/8
          else T_REF_PWR = -100;                           //  unknown
          }
      else if ( constellation == DMD_E_DVBT_CONST_16QAM) {                   // 16QAM
          if      ( coderate == 0 ) T_REF_PWR = -87;       //  1/2
          else if ( coderate == 1 ) T_REF_PWR = -85;       //  2/3
          else if ( coderate == 2 ) T_REF_PWR = -84;       //  3/4
          else if ( coderate == 3 ) T_REF_PWR = -83;       //  5/6
          else if ( coderate == 4 ) T_REF_PWR = -82;       //  7/8
          else T_REF_PWR = -100;                           //  unknown
          }
      else if ( constellation == DMD_E_DVBT_CONST_64QAM) {                   // 64QAM
          if      ( coderate == 0 ) T_REF_PWR = -82;       //  1/2
          else if ( coderate == 1 ) T_REF_PWR = -80;       //  2/3
          else if ( coderate == 2 ) T_REF_PWR = -78;       //  3/4
          else if ( coderate == 3 ) T_REF_PWR = -77;        //  5/6
          else if ( coderate == 4 ) T_REF_PWR = -76;       //  7/8
          else T_REF_PWR = -100;                           //  unknown
          }
      else T_REF_PWR = -100;                           //  unknown

  return T_REF_PWR;
}


int Get_RF_T2_REF_PWR(DMD_PARAMETER_t * param)
{
       int   T2_REF_PWR;
    DMD_DVBT2_PLP_MOD_t   constellation;
    DMD_DVBT2_CR_t   coderate;

    T2_REF_PWR = -100;

    if ( param->system != DMD_E_DVBT2)
      {
//             SHARP6158_LOG(param, " ERROR : Get_T2_REF_PWR, system is not DVB-T2 @!!\n");
             return  T2_REF_PWR;
      }

       DMD_get_info(param, DMD_E_INFO_DVBT2_DAT_PLP_MOD);
    constellation = param->info[DMD_E_INFO_DVBT2_DAT_PLP_MOD];

    DMD_get_info(param,DMD_E_INFO_DVBT2_DAT_PLP_COD);
    coderate = param->info[DMD_E_INFO_DVBT2_DAT_PLP_COD];

//  SHARP6158_LOG(param, "%s %d  constellation=%d   coderate=%d   \n",__FUNCTION__,__LINE__,constellation,coderate);

/*
typedef enum{    DMD_E_DVBT2_PLP_MOD_QPSK=0,    DMD_E_DVBT2_PLP_MOD_16QAM,
DMD_E_DVBT2_PLP_MOD_64QAM,    DMD_E_DVBT2_PLP_MOD_256QAM    }DMD_DVBT2_PLP_MOD_t;

typedef enum{    DMD_E_DVBT2_CR_1_2=0,    DMD_E_DVBT2_CR_3_5,    DMD_E_DVBT2_CR_2_3,
DMD_E_DVBT2_CR_3_4,    DMD_E_DVBT2_CR_4_5,    DMD_E_DVBT2_CR_5_6    }DMD_DVBT2_CR_t;
*/

/*--- Based on Nordig 2.3, chapter 3.4.4.6 Requirements for the signal strength indicator (SSI) ---*/
  if      ( constellation == DMD_E_DVBT2_PLP_MOD_QPSK ) {                       // QPSK
      if      ( coderate == 0 ) T2_REF_PWR = -96;            //  1/2
      else if ( coderate == 1 ) T2_REF_PWR = -95;            //  3/5
      else if ( coderate == 2 ) T2_REF_PWR = -94;            //  2/3
      else if ( coderate == 3 ) T2_REF_PWR = -93;            //  3/4
      else if ( coderate == 4 ) T2_REF_PWR = -92;            //  4/5
      else if ( coderate == 5 ) T2_REF_PWR = -92;            //  5/6
      else T2_REF_PWR = -100;                                //  unknown
      }
  else if ( constellation ==     DMD_E_DVBT2_PLP_MOD_16QAM) {                       // 16QAM
      if      ( coderate == 0 ) T2_REF_PWR = -91;            //  1/2
      else if ( coderate == 1 ) T2_REF_PWR = -89;            //  3/5
      else if ( coderate == 2 ) T2_REF_PWR = -88;           //  2/3
      else if ( coderate == 3 ) T2_REF_PWR = -87;           //  3/4
      else if ( coderate == 4 ) T2_REF_PWR = -86;           //  4/5
      else if ( coderate == 5 ) T2_REF_PWR = -86;           //  5/6
      else T2_REF_PWR = -100;                                //  unknown
      }
  else if ( constellation ==     DMD_E_DVBT2_PLP_MOD_64QAM) {                       // 64QAM
      if      ( coderate == 0 ) T2_REF_PWR = -86;           //  1/2
      else if ( coderate == 1 ) T2_REF_PWR = -85;           //  3/5
       else if ( coderate == 2 ) T2_REF_PWR = -83;            //  2/3
      else if ( coderate == 3 ) T2_REF_PWR = -82;           //  3/4
      else if ( coderate == 4 ) T2_REF_PWR = -81;           //  4/5
      else if ( coderate == 5 ) T2_REF_PWR = -80;           //  5/6
      else T2_REF_PWR = -100;                                //  unknown
      }
  else if ( constellation ==     DMD_E_DVBT2_PLP_MOD_256QAM) {                       // 256QAM
       if      ( coderate == 0 ) T2_REF_PWR = -83;           //  1/2
      else if ( coderate == 1 ) T2_REF_PWR = -80;           //  3/5
      else if ( coderate == 2 ) T2_REF_PWR = -79;          //  2/3
      else if ( coderate == 3 ) T2_REF_PWR = -77;            //  3/4
      else if ( coderate == 4 ) T2_REF_PWR = -75;           //  4/5
      else if ( coderate == 5 ) T2_REF_PWR = -75;           //  5/6
     else T2_REF_PWR = -100;                                //  unknown
      }
  else T2_REF_PWR = -100;                                //  unknown


  return T2_REF_PWR;
}


