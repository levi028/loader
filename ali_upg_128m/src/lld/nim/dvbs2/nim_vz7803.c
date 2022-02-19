/*****************************************************************************
*    Copyright (C)2008 Ali Corporation. All Rights Reserved.
*
*    File:    nim_vz7803.c
*
*    Description:    This file contains SHARP BS2S7VZ7803 tuner basic function.
*    History:
*           Date                    Athor                   Version             Reason
*       ============    =============       =========   =================
*   1.  2008-12-2           Douglass yan            Ver 0.1     Create file.
*
*****************************************************************************/

#include <sys_config.h>
#include <retcode.h>
#include <types.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/i2c/i2c.h>
#include <osal/osal.h>
#include "nim_vz7803.h"

#ifndef I2C_FOR_VZ7803
#define I2C_FOR_VZ7803  I2C_TYPE_SCB0
#endif
#define VZ7803_PRINTF(...)



#define I2cWriteRegs(Addr, rAddr, lpbData, bLen) i2c_write(I2C_FOR_VZ7803, Addr, lpbData, bLen)
#define I2cReadReg(Addr, rAddr, lpbData) i2c_read(I2C_FOR_VZ7803, Addr, lpbData, 1)


//static unsigned char init_data1[] = { 0x44, 0x7e, 0xe1, 0x42 };
//static unsigned char init_data2[] = { 0xe5 };
//static unsigned char init_data3[] = { 0xfd, 0x0d };

struct QPSK_TUNER_CONFIG_EXT * vz7803_dev_id[MAX_TUNER_SUPPORT_NUM] = {NULL};
static UINT32 vz7803_tuner_cnt = 0;

/*
UINT8 tuner_reg[] =
{
    0x48,
    0x1C,
    0xA0,
    0x10,
    0xBC,
    0xC5,
    0x20,
    0x33,
    0x06,
    0x00,

    0x00,   //10
    0x00,
    0x03,
    0x00,
    0x00,   //
    0x00,
    0x00,
    0xFF,
    0xF3,
    0x00,

    0x2A, //
    0x64,
    0xA6,
    0x86,
    0x8C,
    0xCF,
    0xB8,
    0xF1,
    0xA8, //
    0xF2,

    0x89,
    0x00,
};

#define TUNER_REG_CNT (sizeof(tuner_reg)/sizeof(tuner_reg[0]))
*/

//------------------------------------------------------------------------------
//      Tuner read/write function
static int nim_tuner_write(UINT32 tuner_id, unsigned char reg_start, UINT8* buff, unsigned char length)
{
	UINT8 data[16]={0};  
	//UINT32 rd = 0;
	int i = 0;
	int i2c_result =-1;
    struct QPSK_TUNER_CONFIG_EXT * vz7803_ptr = NULL;
    
    if((buff == NULL) || (tuner_id>=MAX_TUNER_SUPPORT_NUM))
        return RET_FAILURE;

    vz7803_ptr = vz7803_dev_id[tuner_id];
    if(length>15)
        return !SUCCESS;

    data[0] = reg_start;
    for(i=0;i<length;i++)
    {
        data[i+1] = buff[i];
    }

    i2c_result = i2c_write(vz7803_ptr->i2c_type_id, vz7803_ptr->c_tuner_base_addr, data, length+1);

    return i2c_result;
}

static int nim_tuner_read(UINT32 tuner_id, unsigned char reg_start, UINT8* buff, unsigned char length)
{
	UINT8 data[16]={0};
	//UINT32 rd = 0;
	int i =0;
	int i2c_result = -1;

	i = i2c_result; //clean warning
	i = 0; //clean warning
    struct QPSK_TUNER_CONFIG_EXT * vz7803_ptr = NULL;

    if((buff == NULL) || (tuner_id>=MAX_TUNER_SUPPORT_NUM))
        return RET_FAILURE;
    vz7803_ptr = vz7803_dev_id[tuner_id];

    if(length>15)
        return !SUCCESS;
    data[0] = reg_start;
    i2c_result = i2c_write_read(vz7803_ptr->i2c_type_id, vz7803_ptr->c_tuner_base_addr, data, 1, length);
    for(i=0;i<length;i++)
    {
        buff[i] = data[i];
    }

    return SUCCESS;
}

//------------------------------------------------------------------------------
//      Tuner initial function

INT32 nim_vz7803_init(UINT32* tuner_id, struct QPSK_TUNER_CONFIG_EXT * ptrTuner_Config)
{
    UINT8 data[16] = {0x48,0x1c,0xA0,0x10,0xBC,0xC5,0x20,0x33,
                     0x06,0x00,0x00,0x00,0x03};
	//int i = 0;
    struct QPSK_TUNER_CONFIG_EXT * vz7803_ptr = NULL;
    INT32 result =0;

    //soc_printf("Enter Sharp 7803 initial ....................\n");

    /* check Tuner Configuration structure is available or not */
	if ((ptrTuner_Config == NULL)||(vz7803_tuner_cnt>=MAX_TUNER_SUPPORT_NUM) || (tuner_id == NULL))
        return ERR_FAILUE;
    vz7803_ptr = (struct QPSK_TUNER_CONFIG_EXT *)MALLOC(sizeof(struct QPSK_TUNER_CONFIG_EXT));
    if(!vz7803_ptr)
          return ERR_FAILUE;

    MEMCPY(vz7803_ptr, ptrTuner_Config, sizeof(struct QPSK_TUNER_CONFIG_EXT));
    vz7803_dev_id[vz7803_tuner_cnt] = vz7803_ptr;
    *tuner_id = vz7803_tuner_cnt;
    vz7803_tuner_cnt++;

    // Soft reset
    data[0] = 0x0c;
    result = nim_tuner_write (*tuner_id,  0x01, data, 1);
    if(result!=SUCCESS)
        ali_trace(&result);
    result = nim_tuner_write (*tuner_id,  0x01, data, 1);
    if(result!=SUCCESS)
        ali_trace(&result);
	result = nim_tuner_write (*tuner_id,  0x01, data, 1);
    if(result!=SUCCESS)
        ali_trace(&result);
    osal_delay(1000);

    //soft reset off
	result = nim_tuner_read(*tuner_id, 0x01, data, 1);
    if(result!=SUCCESS)
        ali_trace(&result);
        data[0] |= 0x10;
    result = nim_tuner_write (*tuner_id,  0x01, data, 1);
    if(result!=SUCCESS)
        ali_trace(&result);

    // Write fixed value
	result = nim_tuner_write(*tuner_id, 0x01, data+1, 12);   // 01~0C
	if(result!=SUCCESS)
        ali_trace(&result);
        data[0] = 0xFF;
    data[1] = 0xF3;
    data[2] = 0x00;
        data[3] = 0x2a;  // set bb gain    BB_Gain = 0x01; // 00:? 01:-1db, 11:-4db
        data[4] = 0x64;
        data[5] = 0xa6;
        data[6] = 0x86;
        data[7] = 0xcc;
        data[8] = 0xcf;
        data[9] = 0x95;
        data[10] = 0xf1;
        data[11] = 0xa8;
        data[12] = 0xf2;
        data[13] = 0x09;
        data[14] = 0x00;
    result = nim_tuner_write (*tuner_id,  0x11, data, 15); // 11~1F
    if(result!=SUCCESS)
        ali_trace(&result);

//    data[0] = 0x01; // fast search mode
//    nim_tuner_write (*tuner_id,  0x03, data, 1);


    //soc_printf("Leave Sharp 7803 initial ....................\n");
    return SUCCESS;
}

//------------------------------------------------------------------------------
//      Tuner control function

INT32 nim_vz7803_control(UINT32 tuner_id,UINT32 freq, UINT32 sym)
{
    UINT8 data[16]={0}; // data[15] is i2c read_only
    UINT32 Rs = 0,BW = 0;
    UINT8 LPF = 13;
    UINT8 BA = 1;
    UINT8 DIV = 1;
    UINT8 i =0;//rdata =0;
    UINT8 search_mode = 0;
    //UINT8 F_ref = 0, pll_ref_div =0, alpha =0, N =0, A =0, CSEL_Offset =0, BB_Gain =0;
    UINT8 pll_ref_div =0, alpha =0, N =0, A =0, CSEL_Offset =0;
    UINT32 M =0, sd =0;
    INT32 result =0;

    //struct QPSK_TUNER_CONFIG_EXT * vz7803_ptr = NULL; //clean warning
    if((tuner_id>=vz7803_tuner_cnt)||(tuner_id>=MAX_TUNER_SUPPORT_NUM))
        return ERR_FAILUE;
    //vz7803_ptr = vz7803_dev_id[tuner_id]; //clean warning

    //soc_printf("Enter Sharp 7803 control, freq is %d, Sym is %d\n", freq, sym);
    //BB_Gain = 0x01; // 00:? 01:-1db, 11:-4db

    for(i=0;i<16;i++)
        data[i] = 0;

    /* LPF cut_off */
    Rs = sym;
    if (freq>2200)
        freq = 2200;
    if (Rs==0)
        Rs = 45000;
    BW = Rs*135/200;                // rolloff factor is 35%
    if (Rs<6500)  BW = BW + 4000;   // add 3M when Rs<5M, since we need shift 3M to avoid DC
    BW = BW + 2000;                 // add 2M for LNB frequency shifting
//ZCY: the cutoff freq of IX2410 is not 3dB point, it more like 0.1dB, so need not 30%
//  BW = BW*130/100;                // 0.1dB to 3dB need about 30% transition band for 7th order LPF
    BW = BW*108/100;                // add 8% margin since fc is not very accurate

    if (BW< 4000)   BW =  4000;     // Sharp2410 LPF can be tuned form 10M to 30M, step is 2.0M
    if (BW>34000)   BW = 34000;     // the range can be 6M~~34M actually, 4M is not good

    if (BW<=4000)  LPF = 0;
    else if (BW<=6000 )  LPF = 1;
    else if (BW<=8000 )  LPF = 2;
    else if (BW<=10000)  LPF = 3;
    else if (BW<=12000)  LPF = 4;
    else if (BW<=14000)  LPF = 5;
    else if (BW<=16000)  LPF = 6;
    else if (BW<=18000)  LPF = 7;
    else if (BW<=20000)  LPF = 8;
    else if (BW<=22000)  LPF = 9;
    else if (BW<=24000)  LPF = 10;
    else if (BW<=26000)  LPF = 11;
    else if (BW<=28000)  LPF = 12;
    else if (BW<=30000)  LPF = 13;
    else if (BW<=32000)  LPF = 14;
    else                 LPF = 15;

    if(LPF<3)   CSEL_Offset = 3;
    else if(LPF <6) CSEL_Offset = 2;
    else if(LPF <13) CSEL_Offset = 1;
    else CSEL_Offset = 0;


    /* local oscillator select */
    DIV = 1;
    if(freq <= 975)
    {
        BA = 6;
        DIV = 0;
    }
    else if(freq <= 1200)
    {
        BA = 7;
        DIV = 0;
    }
    else if(freq <= 1250)
        BA = 2;
    else if(freq <= 1450)
        BA = 3;
    else if(freq <= 1600)
        BA = 4;
    else if(freq <= 1800)
        BA = 5;
    else if(freq <= 1950)
        BA = 6;
    else //if(freq <= 2150)
        BA = 7;

    //F_ref = 16;
    pll_ref_div = 0;

    M = (freq<<16);
    alpha = (((M>>19)+1)>>1);
    if(M>((UINT32)(alpha<<20)))
        sd = M - (alpha<<20);
    else
        sd = 0x800000 - (alpha<<20) + M;

    N = (alpha-12)/4;
    A = alpha - 4*(N + 1)-5;

    //soc_printf("BA: %d, M: %d, alpha: %d, N: %d, A: %d, sd: %d, LPF: %d\n",BA,M,alpha, N,A,sd,LPF);

    // Set VCO_TM and LFP_TM
    //nim_tuner_read  (tuner_id,  0x0c, &data, 1);
    //tuner_reg[12] = data & 0x3f;
    //nim_tuner_write (tuner_id,  0x0c, &tuner_reg[12], 1);
    result = nim_tuner_read  (tuner_id,  0x0c, data, 1);
    if(result!=SUCCESS)
        ali_trace(&result);
    data[0] &= 0x3F;
    result = nim_tuner_write (tuner_id,  0x0c, data, 1);
    if(result!=SUCCESS)
        ali_trace(&result);
    osal_delay(1200);

    //nim_tuner_read  (tuner_id,  0x02, &data, 1);
    //tuner_reg[2] = data | ((DIV<<7) | (BA<<4));
    //nim_tuner_write (tuner_id,  0x0c, &tuner_reg[2], 1);

    //data[0] = 0x01; //register address
    //data[0] = 0x1C;
    data[2] = (DIV<<7) | (BA<<4) ;
    data[3] = 0x10;

    if(1 == search_mode)
    {
        data[3] |= 0x01;       //fast search
    }
    else
    {
        data[3] &= 0xfe;       //normal
    }
	result = nim_tuner_write (tuner_id,  0x02, data+2, 2);
    if(result!=SUCCESS)
        ali_trace(&result);

    //data[4] = 0xBC;
    data[5] = 0xC5;
    data[6] = ((pll_ref_div<<7)|(N));
    data[7] = (A&0x0F) | 0x30;
    //data[8] = (LPF&0x0f);
    data[8] = 0x02;
    data[9] = (sd >>16)&0x3f;
    data[10] = (sd >>8)&0xff;
    data[11] = (sd )&0xff;
//    data[12] = 0xc0;
    result = nim_tuner_write (tuner_id,  0x05, data+5, 7);
    if(result!=SUCCESS)
        ali_trace(&result);

    /*VCO_TM , LPF_TM*/
    //nim_tuner_read  (tuner_id,  0x0c, &data, 1);
    //data |= 0xc0;
    //nim_tuner_write (tuner_id,  0x0c, &data, 1);
    result =  nim_tuner_read  (tuner_id,  0x0c, data, 1);
    if(result!=SUCCESS)
        ali_trace(&result);
    data[0] |= 0xC0;
    result = nim_tuner_write (tuner_id,  0x0c, data, 1);
    if(result!=SUCCESS)
        ali_trace(&result);

    osal_delay(20000);

    data[0] = (LPF&0x0f);
    result = nim_tuner_write (tuner_id,  0x08, data, 1);
    if(result!=SUCCESS)
        ali_trace(&result);

    data[0] = 0xf3;    
    result = nim_tuner_write (tuner_id,  0x12, data, 1);
    if(result!=SUCCESS)
        ali_trace(&result);

    data[0] = ((CSEL_Offset&0x03)<<5);
    result = nim_tuner_write (tuner_id,  0x13, data, 1);
    if(result!=SUCCESS)
        ali_trace(&result);

    //soc_printf("Leave Sharp 7803 control,\n");
    return SUCCESS;
}

//------------------------------------------------------------------------------
//      Tuner read status function

INT32 nim_vz7803_status(UINT32 tuner_id,UINT8 *lock)
{
    INT32 result =-1;
    UINT8 data =0;
    if(lock == NULL)
        return RET_FAILURE;

//    struct QPSK_TUNER_CONFIG_EXT * vz7803_ptr = NULL;
    if((tuner_id>=vz7803_tuner_cnt)||(tuner_id>=MAX_TUNER_SUPPORT_NUM))
    {
        *lock = 0;
        return ERR_FAILUE;
    }
   // vz7803_ptr = vz7803_dev_id[tuner_id];

    result = nim_tuner_read  (tuner_id,  0x0d, &data, 1);
    if(result!=SUCCESS)
        ali_trace(&result);
    *lock = ((data & 0x40) >> 6);
    //soc_printf ("lock = %d,result = %d\n",*lock,result);
    return result;

}

