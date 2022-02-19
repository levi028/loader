/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_countryband.c
*
*    Description: The realize of country setting
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#include <types.h>
#include <api/libtsi/db_3l.h>
#include <api/libc/string.h>
#include "copper_common/system_data.h"
#include "win_countryband.h"


#ifdef DVBC_COUNTRY_BAND_SUPPORT
//add on 2011-11-04
#define MAX_BAND_COUNT                  10
#define MAX_COUNTRY                 20
#define MAX_BAND_PARAMETERS         7
#define GERM_TYPE  3

/*
typedef struct Band_param
{
    UINT32 start_freq;
    UINT32 end_freq;
    UINT32 bandwidth;
    UINT8  start_ch_no;
    UINT8  end_ch_no;
    UINT8  show_ch_no;
    char   show_ch_prefix;
    //UINT32 band_type;
}Band_param;

typedef enum
{
    GERMANY = 0,
    AUSRIA,
    SWITZERLAND,
    ITALY,
    GREECE,
    PORTUGAL
}COUTRY_TYPE;

*/

static const band_param countryband[MAX_COUNTRY][MAX_BAND_COUNT]\
             __attribute__((section(".flash")))=
{
    //GERMANY
    /*
    {
     {5050,6450,700,1,3,2,' '},
     {7300,7300,0,4,4,73,'D'},
     {8100,8100,0,5,5,81,'D'},
     {10750,10750,0,6,6,1,'S'},
     {11300,11300,0,7,7,2,'S'},
     {12150,17050,700,8,15,3,'S'},
     {17750,22650,700,16,23,5,' '},
     {23350,29650,700,24,33,11,'S'},
     {30600,46600,800,34,54,21,'S'},
     {47400,85800,800,55,103,21,' '}
    },
    */
    //change on 2011-11-07 for DVBC
    //beacuse the struct Band_param have change for combo db_combo_node.h
#ifdef DVBC_SUPPORT
    {
        {5050,6450,0,0,700,1,3,2,' '},
        {11350,16950,0,0,700,4,12,2,'S'},
        {17750,22650,0,0,700,13,20,5,' '},
        {23350,29650,0,0,700,21,30,11,'S'},
        {30600,46600,0,0,800,31,51,21,'S'},
        {47400,85800,0,0,800,52,100,21,' '}
    },

    {
        {5050,6450,0,0,700,1,3,2,' '},
        //{11450,17050,700,4,12,2,'S'},
        //{11300,12850,700,4,7,2,'S'},
        //{13700,14500,800,8,9,5,'S'},
        //{14850,17050,700,10,12,7,'S'},
        {11300, 16900, 0,0,800, 4, 11, 2, 'S'},
        {17750,22650,0,0,700,12,20,5,' '},
        {23350,29650,0,0,700,20,30,11,'S'},
        {30600,46600,0,0,800,30,51,21,'S'},
        {47400,85800,0,0,800,51,100,21,' '}
    },
    ////////////////new//////////////////////
    //Argentina : PAL M/N
    {
        {5700,6900,0,0,600,1,3,2,' '},
        {7900,8500,0,0,600,4,5,5,' '},
        {17700,21300,0,0,600,6,12,7,' '},
        {12300,17100,0,0,600,13,21,14,' '},
        {21900,64500,0,0,600,22,93,23,' '},
        {9300,11700,0,0,600,94,98,95,' '},
        {65100,80100,0,0,600,99,124,100,' '}
    },
    //PAL B/G: Italy Germany Greecc Austria
    //Portual Switzerland Denmark Poland Singapore Spain Sweden
    {
        {5050,6450,0,0,700,1,3,2,' '},
        {17750,22650,0,0,700,4,11,5,' '},
        {12150,17050,0,0,700,12,19,13,' '},
        {23350,80050,0,0,700,20,101,21,' '},
    },
    //PAL B/H: Algeria Belgium
    {
        {5050,6450,0,0,700,1,3,2,' '},
        {17750,22650,0,0,700,4,11,5,' '},
        {12150,17050,0,0,700,12,19,13,' '},
        {23350,80050,0,0,700,20,101,21,' '},
    },
    //PAL B/H(AUSTRALIA): Australia //don't have dvbc
    //SCEAM L: France
    {
        {5300,5300,0,0,800,1,1,2,' '},
        {5775,5775,0,0,800,2,2,3,' '},
        {6100,6100,0,0,800,3,3,4,' '},
        {17875,21875,0,0,800,4,9,5,' '},
        {12275,17075,0,0,800,10,16,11,' '},
        {22675,80275,0,0,800,17,89,18,' '},
    },
    //PAL I: Hong Kong
    //SCEAM D/K : Russia
    {
        {5250,5250,0,0,800,1,1,1,' '},
        {6200,6200,0,0,800,2,2,2,' '},
        {8000,9600,0,0,800,3,5,3,' '},
        {17800,22600,0,0,800,4,12,4,' '},
        {11500,16300,0,0,800,13,19,13,' '},
        {22700,80300,0,0,800,20,92,20,' '},
    },

#else
    {
        {5050,6450,700,1,3,2,' '},
        {11350,16950,700,4,12,2,'S'},
        {17750,22650,700,13,20,5,' '},
        {23350,29650,700,21,30,11,'S'},
        {30600,46600,800,31,51,21,'S'},
        {47400,85800,800,52,100,21,' '}
    },

    {
        {5050,6450,700,1,3,2,' '},
        //{11450,17050,700,4,12,2,'S'},
        //{11300,12850,700,4,7,2,'S'},
        //{13700,14500,800,8,9,5,'S'},
        //{14850,17050,700,10,12,7,'S'},
        {11300, 16900, 800, 4, 11, 2, 'S'},
        {17750,22650,700,12,20,5,' '},
        {23350,29650,700,20,30,11,'S'},
        {30600,46600,800,30,51,21,'S'},
        {47400,85800,800,51,100,21,' '}
    },


#endif
};


static const band_param countryband_qamb[MAX_COUNTRY][MAX_BAND_COUNT] \
             __attribute__((section(".flash")))=
{
#ifdef DVBC_SUPPORT
        //Argentina
        {
            {5700,7500,0,0,600,1,3,2,' '},
            {7900,9100,0,0,600,4,5,5,' '},
            {17700,21900,0,0,600,6,12,7,' '},
            {12300,17700,0,0,600,13,21,14,' '},
            {21900,65100,0,0,600,22,93,23,' '},
            {9300,12300,0,0,600,94,98,95,' '},
            {65100,80700,0,0,600,99,124,100,' '},
        },
#endif
};

void get_default_bandparam(UINT8 index, band_param *buf)
{
    UINT32 addr = 0;
    index = 0;
    SYSTEM_DATA *sys_data = sys_data_get();

    if((GERMANY==sys_data->country ) && (GERM_TYPE==sys_data->germ_servs_type ))
    {
        addr = (UINT32)countryband + 1 * (sizeof(band_param)*MAX_BAND_COUNT);
        MEMCPY(buf, (void*)addr, (sizeof(band_param)*MAX_BAND_COUNT));
        return;
    }

    if(ARGNITINA == sys_data->country)
    {
        index = 2;
    }
    else if((DENMARK == sys_data->country) || (SPAIN ==sys_data->country)
        || (POLAND == sys_data->country) || (PORTUGAL == sys_data->country)
        || (GREECE == sys_data->country) || (ITALY == sys_data->country)
        ||(SWITZERLAND == sys_data->country))
    {
        index = 3;
    }
    else if(FRANCE == sys_data->country)
    {
        index = 5;
    }
    else if(RUSSIA == sys_data->country)
    {
        index = 6;
    }
    else
    {
        index = 0;
    }

    addr = (UINT32)countryband + index * (sizeof(band_param)*MAX_BAND_COUNT);
    MEMCPY(buf, (void*)addr, (sizeof(band_param)*MAX_BAND_COUNT));

    return;

}

void get_default_bandparam_qamb(UINT8 index, band_param *buf)
{

    //for test
    index = 0;

        //SYSTEM_DATA* sys_data = sys_data_get();

    UINT32 addr = 0;

    addr = (UINT32)countryband_qamb + \
            index * (sizeof(band_param)*MAX_BAND_COUNT);

    MEMCPY(buf, (void*)addr, (sizeof(band_param)*MAX_BAND_COUNT));

}


#endif

