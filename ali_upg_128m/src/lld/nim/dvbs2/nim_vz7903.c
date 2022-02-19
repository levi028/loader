/*****************************************************************************
*    Copyright (C)2008 Ali Corporation. All Rights Reserved.
*
*    File:    nim_vz7903.c
*
*    Description:    This file contains SHARP BS2S7VZ7903 tuner basic function.
*    History:
*           Date                    Athor                    Version              Reason
*        ============    =============        =========    =================
*    1.  2011-10-16               Russell Luo             Ver 0.1        Create file.
*
*****************************************************************************/

#include <sys_config.h>
#include <retcode.h>
#include <types.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/i2c/i2c.h>
#include <osal/osal.h>
#include "nim_vz7903.h"

#ifndef I2C_FOR_VZ7903
#define I2C_FOR_VZ7903     I2C_TYPE_SCB0
#endif

//#define VZ7903_PRINTF(...)
#define VZ7903_PRINTF  libc_printf

//static UINT8 QM1D1C0045_d_reg[QM1D1C0045_INI_REG_MAX];
#define    DEF_XTAL_FREQ                16000
#define INIT_DUMMY_RESET    0x0C
#define LPF_CLK_16000kHz    16000

#define I2cWriteRegs(Addr, rAddr, lpbData, bLen) i2c_write(I2C_FOR_VZ7903, Addr, lpbData, bLen)
#define I2cReadReg(Addr, rAddr, lpbData) i2c_read(I2C_FOR_VZ7903, Addr, lpbData, 1)

struct QPSK_TUNER_CONFIG_EXT * vz7903_dev_id[MAX_TUNER_SUPPORT_NUM] = {NULL};
static UINT32 vz7903_tuner_cnt = 0;

//static unsigned char init_data1[] = { 0x44, 0x7e, 0xe1, 0x42 };
//static unsigned char init_data2[] = { 0xe5 };
//static unsigned char init_data3[] = { 0xfd, 0x0d };

BOOL QM1D1C0045_Initialize(UINT32 tuner_id,PQM1D1C0045_CONFIG_STRUCT	apConfig) ;
BOOL QM1D1C0045_LocalLpfTuning(UINT32 tuner_id, PQM1D1C0045_CONFIG_STRUCT	apConfig) ;
BOOL QM1D1C0045_Set_Operation_Param(UINT32 tuner_id, PQM1D1C0045_CONFIG_STRUCT	apConfig);
BOOL QM1D1C0045_set_searchmode(UINT32 tuner_id, PQM1D1C0045_CONFIG_STRUCT	apConfig) ;
/*====================================================*
    QM1D1C0045_register_real_write
   --------------------------------------------------
    Description     register write
    Argument        RegAddr
                    RegData
    Return Value    BOOL (TRUE:success, FALSE:error)
 *====================================================*/
BOOL QM1D1C0045_register_real_write(UINT32 tuner_id, UINT8 RegAddr, UINT8 RegData){
	UINT8 slvAddr =0;
	UINT8 i2c_tmp_buf[2]={0};
    UINT16 i2c_access_size = 2;
    i2c_tmp_buf[0] = RegAddr;
    i2c_tmp_buf[1] = RegData;
    slvAddr = 0xc0;
    i2c_write(tuner_id, slvAddr, i2c_tmp_buf, i2c_access_size);
    return TRUE;
}

/*====================================================*
    QM1D1C0045_register_real_read
   --------------------------------------------------
    Description     register read
    Argument        RegAddr (Register Address)
                    apData (Read data)
    Return Value    BOOL (TRUE:success, FALSE:error)
 *====================================================*/
BOOL QM1D1C0045_register_real_read(UINT32 tuner_id, UINT8 RegAddr, UINT8 *apData)
{
	BOOL bRetVal = FALSE;
	UINT8 slvAddr = 0;

    if(apData == NULL)
        return FALSE;
    slvAddr = 0xc0; //QM1D1C0045_i2c_slave_addr_set(QM1D1C0045_ILLEAGAL_SLAVE_ADDR);

    {
		UINT8 i2c_tmp_buf[1]={0};
        UINT16 i2c_access_size = 1;
        i2c_tmp_buf[0] = RegAddr;
        bRetVal = i2c_write(tuner_id, slvAddr, i2c_tmp_buf, i2c_access_size);
    }
    if(bRetVal != TRUE){
        //VZ7903_PRINTF("  write error --\n");
        //return bRetVal;

    }

    {
        UINT16 i2c_access_size = 1;
        bRetVal = i2c_read(tuner_id, slvAddr, apData, i2c_access_size);

    }
    if(bRetVal != TRUE){

        //VZ7903_PRINTF("  write error --\n");
        //return bRetVal;

    }

    return TRUE;
}


INT32 nim_vz7903_init(UINT32* tuner_id, struct QPSK_TUNER_CONFIG_EXT * ptrTuner_Config)
{
	//INT32 result = -1;
	//UINT8 i =0;
    struct QPSK_TUNER_CONFIG_EXT * vz7903_ptr = NULL;
     struct _QM1D1C0045_CONFIG_STRUCT *vz7903_cfg = NULL;
	BOOL rval = FALSE;
	//UINT8 rbuf =0;
	if ((ptrTuner_Config == NULL)||(vz7903_tuner_cnt>=MAX_TUNER_SUPPORT_NUM) || (tuner_id == NULL))
        return ERR_FAILUE;
    vz7903_ptr = (struct QPSK_TUNER_CONFIG_EXT *)MALLOC(sizeof(struct QPSK_TUNER_CONFIG_EXT));
    if(!vz7903_ptr)
          return ERR_FAILUE;
    MEMCPY(vz7903_ptr, ptrTuner_Config, sizeof(struct QPSK_TUNER_CONFIG_EXT));
    vz7903_dev_id[vz7903_tuner_cnt] = vz7903_ptr;
    *tuner_id = vz7903_tuner_cnt;
    vz7903_tuner_cnt++;
/*VZ7903_PRINTF("  VZ7903 initial --------------- ");
 for(i=0;i<16;i++)
     {
         rbuf = 0xff;
            QM1D1C0045_register_real_write(vz7903_ptr->i2c_type_id, i, rbuf);
         VZ7903_PRINTF(" read reg %d is: ",i);
         QM1D1C0045_register_real_read(vz7903_ptr->i2c_type_id, i, &rbuf);
         // result = i2c_read(vz7903_ptr->i2c_type_id, 0xc0, &rbuf, 1);
         VZ7903_PRINTF(" : %d \n",rbuf);
     }    */

 vz7903_cfg = ( struct _QM1D1C0045_CONFIG_STRUCT*) MALLOC ( sizeof (struct _QM1D1C0045_CONFIG_STRUCT)) ;
 vz7903_cfg->ui_QM1D1C0045_RFChannelkHz = 1550000 ;                    /*XtalFreqKHz*/
 vz7903_cfg->ui_QM1D1C0045_XtalFreqKHz = 16000 ;                        /*XtalFreqKHz*/
 vz7903_cfg->b_QM1D1C0045_fast_search_mode = FALSE ;                        /*b_fast_search_mode*/
 vz7903_cfg->b_QM1D1C0045_loop_through = FALSE ;                        /*b_loop_through*/
 vz7903_cfg->b_QM1D1C0045_tuner_standby = FALSE ;                        /*b_tuner_standby*/
 vz7903_cfg->b_QM1D1C0045_head_amp = TRUE ;                        /*b_head_amp*/
 vz7903_cfg->QM1D1C0045_lpf = QM1D1C0045_LPF_FC_10MHz ;    /*lpf*/
 vz7903_cfg->ui_QM1D1C0045_LpfWaitTime = 20 ;                        /*QM1D1C0045_LpfWaitTime*/
 vz7903_cfg->ui_QM1D1C0045_FastSearchWaitTime = 4 ;                            /*QM1D1C0045_FastSearchWaitTime*/
 vz7903_cfg->ui_QM1D1C0045_NormalSearchWaitTime = 15 ;                        /*QM1D1C0045_NormalSearchWaitTime*/

    //BOOL                b_QM1D1C0045_iq_output;


    rval = QM1D1C0045_Initialize(vz7903_ptr->i2c_type_id,vz7903_cfg ) ;
    if(rval!=TRUE)
        ali_trace(&rval);

/*
    if ((result = i2c_write(vz7903_ptr->i2c_type_id, vz7903_ptr->c_tuner_base_addr, init_data1, sizeof(init_data1))) != SUCCESS)

    {
        VZ7903_PRINTF("nim_vz7903_init: I2C write error\n");
        return result;
    }

    if ((result = i2c_write(vz7903_ptr->i2c_type_id, vz7903_ptr->c_tuner_base_addr, init_data2, sizeof(init_data2))) != SUCCESS)
        return result;


    osal_delay(10000);

    if ((result = i2c_write(vz7903_ptr->i2c_type_id, vz7903_ptr->c_tuner_base_addr, init_data3, sizeof(init_data3))) != SUCCESS)
        return result;

*/
    return SUCCESS;
}

/*****************************************************************************
* INT32 nim_vz7903_control(UINT32 freq, UINT8 sym, UINT8 cp)
*
* Tuner write operation
*
* Arguments:
*  Parameter1: UINT32 freq        : Synthesiser programmable divider
*  Parameter2: UINT8 sym        : symbol rate
*
* Return Value: INT32            : Result
*****************************************************************************/
INT32 nim_vz7903_control(UINT32 tuner_id,UINT32 freq, UINT32 sym)
{
	//UINT8 data[4]={0};
	//UINT16 Npro =0,tmp=0;
	UINT32 Rs=0, BW=0;
	//UINT8 Nswa=0;
    UINT8 LPF = 15;
	//UINT8 BA = 1;
	//UINT8 DIV = 0;
	//INT32 result=-1;
    BOOL rval = FALSE;

    struct QPSK_TUNER_CONFIG_EXT * vz7903_ptr = NULL;
     struct _QM1D1C0045_CONFIG_STRUCT *vz7903_cfg = NULL;

	if((tuner_id>=vz7903_tuner_cnt)||(tuner_id>=MAX_TUNER_SUPPORT_NUM))
        return ERR_FAILUE;
    vz7903_ptr = vz7903_dev_id[tuner_id];

    vz7903_cfg = ( struct _QM1D1C0045_CONFIG_STRUCT*) MALLOC ( sizeof (struct _QM1D1C0045_CONFIG_STRUCT)) ;
    if(vz7903_cfg == NULL)
        return ERR_FAILUE;

    MEMSET(vz7903_cfg, 0, sizeof(struct _QM1D1C0045_CONFIG_STRUCT));
 vz7903_cfg->ui_QM1D1C0045_RFChannelkHz = 1550000 ;                    /*XtalFreqKHz*/
 vz7903_cfg->ui_QM1D1C0045_XtalFreqKHz = 16000 ;                        /*XtalFreqKHz*/
 vz7903_cfg->b_QM1D1C0045_fast_search_mode = FALSE ;                        /*b_fast_search_mode*/
 vz7903_cfg->b_QM1D1C0045_loop_through = FALSE ;                        /*b_loop_through*/
 vz7903_cfg->b_QM1D1C0045_tuner_standby = FALSE ;                        /*b_tuner_standby*/
 vz7903_cfg->b_QM1D1C0045_head_amp = TRUE ;                        /*b_head_amp*/
 vz7903_cfg->QM1D1C0045_lpf = QM1D1C0045_LPF_FC_10MHz ;    /*lpf*/
 vz7903_cfg->ui_QM1D1C0045_LpfWaitTime = 20 ;                        /*QM1D1C0045_LpfWaitTime*/
 vz7903_cfg->ui_QM1D1C0045_FastSearchWaitTime = 4 ;                            /*QM1D1C0045_FastSearchWaitTime*/
 vz7903_cfg->ui_QM1D1C0045_NormalSearchWaitTime = 15 ;                        /*QM1D1C0045_NormalSearchWaitTime*/

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


  vz7903_cfg->QM1D1C0045_lpf = (QM1D1C0045_LPF_FC) LPF;
  vz7903_cfg->ui_QM1D1C0045_RFChannelkHz = freq * 1000;

     rval = QM1D1C0045_LocalLpfTuning(vz7903_ptr->i2c_type_id, vz7903_cfg) ;
    if(rval!=TRUE)
        ali_trace(&rval);

    return SUCCESS;
}

/*****************************************************************************
* INT32 nim_vz7903_status(UINT8 *lock)
*
* Tuner read operation
*
* Arguments:
*  Parameter1: UINT8 *lock        : Phase lock status
*
* Return Value: INT32            : Result
*****************************************************************************/
INT32 nim_vz7903_status(UINT32 tuner_id,UINT8 *lock)
{
	//INT32 result =1;
	//UINT8 data =0;
    if(lock == NULL)
        return RET_FAILURE;

    //struct QPSK_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;
	if((tuner_id>=vz7903_tuner_cnt)||(tuner_id>=MAX_TUNER_SUPPORT_NUM))
    {
        *lock = 0;
        return ERR_FAILUE;
    }
    //tuner_dev_ptr = vz7903_dev_id[tuner_id];
/*
    if ((result = i2c_read(tuner_dev_ptr->i2c_type_id, tuner_dev_ptr->c_tuner_base_addr, &data, 1)) == SUCCESS)
    {
        VZ7903_PRINTF("nim_vz7903_status: data = 0x%x\n", data);
        *lock = ((data & 0x40) >> 6);
    }
    */
    return SUCCESS;
}


void QM1D1C0045_set_lpf(QM1D1C0045_LPF_FC lpf)
{
    QM1D1C0045_d_reg[QM1D1C0045_REG_08] &= 0xF0;
    QM1D1C0045_d_reg[QM1D1C0045_REG_08] |= (lpf&0x0F);
}

BOOL QM1D1C0045_pll_setdata_once(UINT32 tuner_id, QM1D1C0045_INIT_REG_DATA RegAddr, UINT8 RegData){
	BOOL bRetValue = FALSE;
    bRetValue = QM1D1C0045_register_real_write( tuner_id,RegAddr, RegData);
    return bRetValue;
}

UINT8 QM1D1C0045_pll_getdata_once(UINT32 tuner_id,QM1D1C0045_INIT_REG_DATA RegAddr){
	UINT8 data =0;
    BOOL ret = FALSE;
	ret = QM1D1C0045_register_real_read(tuner_id,RegAddr, &data);
    if(ret!= TRUE)
        ali_trace(&ret);
    return data;
}



void QM1D1C0045_get_lock_status(UINT32 tuner_id,BOOL* pbLock)
{
    if(pbLock == NULL)
        return;
    QM1D1C0045_d_reg[QM1D1C0045_REG_0D] = QM1D1C0045_pll_getdata_once(tuner_id,QM1D1C0045_REG_0D);
    if(QM1D1C0045_d_reg[QM1D1C0045_REG_0D]&0x40){
        *pbLock = TRUE;
    }else{
        *pbLock = FALSE;
    }
}


BOOL QM1D1C0045_Initialize(UINT32 tuner_id,PQM1D1C0045_CONFIG_STRUCT    apConfig)
{
	UINT8 i_data=0 , i=0;
	BOOL bRetValue = FALSE;

    if(apConfig == NULL){
        return FALSE;
    }

    /*Soft Reaet ON*/
	bRetValue = QM1D1C0045_pll_setdata_once(tuner_id,QM1D1C0045_REG_01 , INIT_DUMMY_RESET);
    if(bRetValue!= TRUE)
        ali_trace(&bRetValue);
    osal_delay(1000);
    /*Soft Reaet OFF*/
    i_data = QM1D1C0045_pll_getdata_once(tuner_id,QM1D1C0045_REG_01);
    i_data |= 0x10;
	bRetValue = QM1D1C0045_pll_setdata_once(tuner_id,QM1D1C0045_REG_01 , i_data);
    if(bRetValue!= TRUE)
        ali_trace(&bRetValue);
    /*ID Check*/
    i_data = QM1D1C0045_pll_getdata_once(tuner_id,QM1D1C0045_REG_00);

    if( QM1D1C0045_d[QM1D1C0045_CHIP_ID] != i_data){
        return FALSE;    //"I2C Comm Error", NULL, MB_ICONWARNING);
    }

    /*LPF Tuning On*/
    osal_delay(1000);
    QM1D1C0045_d_reg[QM1D1C0045_REG_0C] |= 0x40;
	bRetValue = QM1D1C0045_pll_setdata_once(tuner_id, QM1D1C0045_REG_0C , QM1D1C0045_d_reg[QM1D1C0045_REG_0C]);
    if(bRetValue!= TRUE)
        ali_trace(&bRetValue);
    osal_delay(apConfig->ui_QM1D1C0045_LpfWaitTime*1000);

    /*LPF_CLK Setting Addr:0x08 b6*/
    if(apConfig->ui_QM1D1C0045_XtalFreqKHz == LPF_CLK_16000kHz){
        QM1D1C0045_d_reg[QM1D1C0045_REG_08] &= ~(0x40);
    }else{
        QM1D1C0045_d_reg[QM1D1C0045_REG_08] |= 0x40;
    }

    /*Timeset Setting Addr:0x12 b[2-0]*/
    if(apConfig->ui_QM1D1C0045_XtalFreqKHz == LPF_CLK_16000kHz){
        QM1D1C0045_d_reg[QM1D1C0045_REG_12] &= 0xF8;
        QM1D1C0045_d_reg[QM1D1C0045_REG_12] |= 0x03;
    }else{
        QM1D1C0045_d_reg[QM1D1C0045_REG_12] &= 0xF8;
        QM1D1C0045_d_reg[QM1D1C0045_REG_12] |= 0x04;
    }

    /*IQ Output Setting Addr:0x17 b5 , 0x1B b3*/
    if(apConfig->b_QM1D1C0045_iq_output){//Differential
        QM1D1C0045_d_reg[QM1D1C0045_REG_17] &= 0xDF;
    }else{//SinglEnd
        QM1D1C0045_d_reg[QM1D1C0045_REG_17] |= 0x20;
    }

    for(i=0 ; i<QM1D1C0045_INI_REG_MAX ; i++){
		if(QM1D1C0045_d_flg[i] == TRUE)
        {
			bRetValue = QM1D1C0045_pll_setdata_once(tuner_id, (QM1D1C0045_INIT_REG_DATA)i , QM1D1C0045_d_reg[i]);
            if(bRetValue!= TRUE)
                ali_trace(&bRetValue);
        }
    }
//    apConfig->b_QM1D1C0045_fast_search_mode = FALSE;//Normal Search
//    apConfig->b_QM1D1C0045_loop_through = TRUE;//LoopThrough Enable
//    apConfig->b_QM1D1C0045_tuner_standby = FALSE;//Normal Mode

    bRetValue = QM1D1C0045_Set_Operation_Param( tuner_id,apConfig);
    if(!bRetValue){
        return FALSE;
    }
    bRetValue = QM1D1C0045_set_searchmode(tuner_id, apConfig);
    if(!bRetValue){
        return FALSE;
    }
    return TRUE;
}


BOOL QM1D1C0045_LocalLpfTuning(UINT32 tuner_id, PQM1D1C0045_CONFIG_STRUCT    apConfig)
{
	UINT8 i_data=0 , i_data1=0 , i=0;
	unsigned int COMP_CTRL=0;
    BOOL bRetValue = FALSE;

    if(apConfig == NULL){
        return FALSE;
    }

    /*LPF*/
    QM1D1C0045_set_lpf(apConfig->QM1D1C0045_lpf);

    /*div2/vco_band*/
    for(i=0;i<15;i++){
        if(QM1D1C0045_local_f[i]==0){
            continue;
        }
        if((QM1D1C0045_local_f[i+1]<=apConfig->ui_QM1D1C0045_RFChannelkHz) && (QM1D1C0045_local_f[i]>apConfig->ui_QM1D1C0045_RFChannelkHz)){
            i_data = QM1D1C0045_pll_getdata_once(tuner_id,QM1D1C0045_REG_02);
            i_data &= 0x0F;
            i_data |= ((QM1D1C0045_div2[i]<<7)&0x80);
            i_data |= ((QM1D1C0045_vco_band[i]<<4)&0x70);
			bRetValue = QM1D1C0045_pll_setdata_once(tuner_id, QM1D1C0045_REG_02 , i_data);
            if(bRetValue!= TRUE)
                ali_trace(&bRetValue);

            i_data = QM1D1C0045_pll_getdata_once(tuner_id, QM1D1C0045_REG_03);
            i_data &= 0xBF;
            i_data |= ((QM1D1C0045_div45_lband[i]<<6)&0x40);
			bRetValue = QM1D1C0045_pll_setdata_once(tuner_id, QM1D1C0045_REG_03 , i_data);
            if(bRetValue!= TRUE)
                ali_trace(&bRetValue);
        }
    }

    /*PLL Counter*/
    {
		int F_ref=0 , pll_ref_div=0 , alpha=0 , N=0 , A=0 , sd=0;
		double M=0 , beta=0;
        if(apConfig->ui_QM1D1C0045_XtalFreqKHz==DEF_XTAL_FREQ){
            F_ref = apConfig->ui_QM1D1C0045_XtalFreqKHz;
            pll_ref_div = 0;
        }else{
            F_ref = (apConfig->ui_QM1D1C0045_XtalFreqKHz>>1);
            pll_ref_div = 1;
        }
        M = (double)( (apConfig->ui_QM1D1C0045_RFChannelkHz)/((double)F_ref) );
        alpha = (int)(M+0.5);
        beta = (double)(M - alpha);
        N = (int)((double)(alpha-12.0)/4.0);
        A = alpha - 4*(N + 1)-5;
        if(beta>=0){
            sd = (int)(1048576*beta);//sd = (int)(pow(2.,20.)*beta);
        }else{
            sd = (int)(0x400000 + 1048576*beta);//sd = (int)(0x400000 + pow(2.,20.)*beta)
        }
        QM1D1C0045_d_reg[QM1D1C0045_REG_06] &= 0x40;
        QM1D1C0045_d_reg[QM1D1C0045_REG_06] |= ((pll_ref_div<<7)|(N));
		bRetValue = QM1D1C0045_pll_setdata_once(tuner_id, QM1D1C0045_REG_06 , QM1D1C0045_d_reg[QM1D1C0045_REG_06]);
        if(bRetValue!= TRUE)
            ali_trace(&bRetValue);

        QM1D1C0045_d_reg[QM1D1C0045_REG_07] &= 0xF0;
        QM1D1C0045_d_reg[QM1D1C0045_REG_07] |= (A&0x0F);
		bRetValue = QM1D1C0045_pll_setdata_once(tuner_id, QM1D1C0045_REG_07 , QM1D1C0045_d_reg[QM1D1C0045_REG_07]);
        if(bRetValue!= TRUE)
            ali_trace(&bRetValue);

        /*LPF_CLK , LPF_FC*/
        i_data = QM1D1C0045_d_reg[QM1D1C0045_REG_08]&0xF0;
		if((apConfig->ui_QM1D1C0045_XtalFreqKHz>=6000) && (apConfig->ui_QM1D1C0045_XtalFreqKHz<66000)){
            i_data1 = ((unsigned char)((double)(apConfig->ui_QM1D1C0045_XtalFreqKHz)/4000.+0.5)*2 - 4)>>1;
        }else if(apConfig->ui_QM1D1C0045_XtalFreqKHz<6000){
            i_data1 = 0x00;
        }else{
            i_data1 = 0x0F;
        }
        i_data |= i_data1;
		bRetValue =QM1D1C0045_pll_setdata_once(tuner_id, QM1D1C0045_REG_08 , i_data);
        if(bRetValue!= TRUE)
            ali_trace(&bRetValue);

        QM1D1C0045_d_reg[QM1D1C0045_REG_09] &= 0xC0;
        QM1D1C0045_d_reg[QM1D1C0045_REG_09] |= ((sd>>16)&0x3F);
        QM1D1C0045_d_reg[QM1D1C0045_REG_0A] = (UINT8)((sd>>8)&0xFF);
        QM1D1C0045_d_reg[QM1D1C0045_REG_0B] = (UINT8)(sd&0xFF);
		bRetValue = QM1D1C0045_pll_setdata_once(tuner_id, QM1D1C0045_REG_09 , QM1D1C0045_d_reg[QM1D1C0045_REG_09]);
        if(bRetValue!= TRUE)
            ali_trace(&bRetValue);
		bRetValue = QM1D1C0045_pll_setdata_once(tuner_id, QM1D1C0045_REG_0A , QM1D1C0045_d_reg[QM1D1C0045_REG_0A]);
        if(bRetValue!= TRUE)
            ali_trace(&bRetValue);
		bRetValue = QM1D1C0045_pll_setdata_once(tuner_id, QM1D1C0045_REG_0B , QM1D1C0045_d_reg[QM1D1C0045_REG_0B]);
        if(bRetValue!= TRUE)
            ali_trace(&bRetValue);
    }

    /*COMP_CTRL*/

    if(apConfig->ui_QM1D1C0045_XtalFreqKHz == 17000){
        COMP_CTRL = 0x03;
    }else if(apConfig->ui_QM1D1C0045_XtalFreqKHz == 18000){
        COMP_CTRL = 0x00;
    }else if(apConfig->ui_QM1D1C0045_XtalFreqKHz == 19000){
        COMP_CTRL = 0x01;
    }else if(apConfig->ui_QM1D1C0045_XtalFreqKHz == 20000){
        COMP_CTRL = 0x03;
    }else if(apConfig->ui_QM1D1C0045_XtalFreqKHz == 21000){
        COMP_CTRL = 0x04;
    }else if(apConfig->ui_QM1D1C0045_XtalFreqKHz == 22000){
        COMP_CTRL = 0x01;
    }else if(apConfig->ui_QM1D1C0045_XtalFreqKHz == 23000){
        COMP_CTRL = 0x02;
    }else if(apConfig->ui_QM1D1C0045_XtalFreqKHz == 24000){
        COMP_CTRL = 0x03;
    }else if(apConfig->ui_QM1D1C0045_XtalFreqKHz == 25000){
        COMP_CTRL = 0x04;
    }else if(apConfig->ui_QM1D1C0045_XtalFreqKHz == 26000){
        COMP_CTRL = 0x02;
    }else if(apConfig->ui_QM1D1C0045_XtalFreqKHz == 27000){
        COMP_CTRL = 0x03;
    }else if(apConfig->ui_QM1D1C0045_XtalFreqKHz == 28000){
        COMP_CTRL = 0x04;
    }else if(apConfig->ui_QM1D1C0045_XtalFreqKHz == 29000){
        COMP_CTRL = 0x04;
    }else if(apConfig->ui_QM1D1C0045_XtalFreqKHz == 30000){
        COMP_CTRL = 0x03;
    }else if(apConfig->ui_QM1D1C0045_XtalFreqKHz == 31000){
        COMP_CTRL = 0x04;
    }else if(apConfig->ui_QM1D1C0045_XtalFreqKHz == 32000){
        COMP_CTRL = 0x04;
    }else{
        COMP_CTRL = 0x02;
    }

    QM1D1C0045_d_reg[QM1D1C0045_REG_1D] &= 0xF8;
    QM1D1C0045_d_reg[QM1D1C0045_REG_1D] |= COMP_CTRL;
	bRetValue = QM1D1C0045_pll_setdata_once(tuner_id, QM1D1C0045_REG_1D , QM1D1C0045_d_reg[QM1D1C0045_REG_1D]);
    if(bRetValue!= TRUE)
        ali_trace(&bRetValue);

    /*VCO_TM , LPF_TM*/
    i_data = QM1D1C0045_pll_getdata_once(tuner_id, QM1D1C0045_REG_0C);
    i_data &= 0x3F;
	bRetValue = QM1D1C0045_pll_setdata_once(tuner_id, QM1D1C0045_REG_0C , i_data);
    if(bRetValue!= TRUE)
        ali_trace(&bRetValue);
        osal_delay(1000);//(1000);;//1024usec

    /*VCO_TM , LPF_TM*/
    i_data = QM1D1C0045_pll_getdata_once(tuner_id, QM1D1C0045_REG_0C);
    i_data |= 0xC0;
	bRetValue = QM1D1C0045_pll_setdata_once(tuner_id, QM1D1C0045_REG_0C , i_data);
    if(bRetValue!= TRUE)
        ali_trace(&bRetValue);

    osal_delay(apConfig->ui_QM1D1C0045_LpfWaitTime*1000);

    /*LPF_FC*/
	bRetValue = QM1D1C0045_pll_setdata_once(tuner_id, QM1D1C0045_REG_08 , QM1D1C0045_d_reg[QM1D1C0045_REG_08]);
    if(bRetValue!= TRUE)
        ali_trace(&bRetValue);

    /*CSEL_Offset*/
    QM1D1C0045_d_reg[QM1D1C0045_REG_13] &= 0x9F;
	bRetValue = QM1D1C0045_pll_setdata_once(tuner_id, QM1D1C0045_REG_13 , QM1D1C0045_d_reg[QM1D1C0045_REG_13]);
    if(bRetValue!= TRUE)
        ali_trace(&bRetValue);

    /*PLL Lock*/
    {
		BOOL bLock = FALSE;
        QM1D1C0045_get_lock_status(tuner_id, &bLock);
        if(bLock!=TRUE){
            return FALSE;
        }
    }
    return TRUE;
}


BOOL QM1D1C0045_Set_Operation_Param(UINT32 tuner_id, PQM1D1C0045_CONFIG_STRUCT    apConfig)
{
	UINT8 u8TmpData =0;
	BOOL bRetValue = FALSE;

    QM1D1C0045_d_reg[QM1D1C0045_REG_01] = QM1D1C0045_pll_getdata_once(tuner_id, QM1D1C0045_REG_01);//Volatile
    QM1D1C0045_d_reg[QM1D1C0045_REG_05] = QM1D1C0045_pll_getdata_once(tuner_id, QM1D1C0045_REG_05);//Volatile

	if((apConfig->b_QM1D1C0045_loop_through) && (apConfig->b_QM1D1C0045_tuner_standby)){
        //LoopThrough = Enable , TunerMode = Standby
        QM1D1C0045_d_reg[QM1D1C0045_REG_01] |= (0x01<<3);//BB_REG_enable
        QM1D1C0045_d_reg[QM1D1C0045_REG_01] |= (0x01<<2);//HA_LT_enable
        QM1D1C0045_d_reg[QM1D1C0045_REG_01] |= (0x01<<1);//LT_enable
        QM1D1C0045_d_reg[QM1D1C0045_REG_01] |= 0x01;//STDBY
        QM1D1C0045_d_reg[QM1D1C0045_REG_05] |= (0x01<<3);//pfd_rst STANDBY

        bRetValue = QM1D1C0045_pll_setdata_once(tuner_id, QM1D1C0045_REG_05 , QM1D1C0045_d_reg[QM1D1C0045_REG_05]);
        if(!bRetValue){
            return bRetValue;
        }
        bRetValue = QM1D1C0045_pll_setdata_once(tuner_id, QM1D1C0045_REG_01 , QM1D1C0045_d_reg[QM1D1C0045_REG_01]);
        if(!bRetValue){
            return bRetValue;
        }
	}else if((apConfig->b_QM1D1C0045_loop_through) && !(apConfig->b_QM1D1C0045_tuner_standby)){
        //LoopThrough = Enable , TunerMode = Normal
        QM1D1C0045_d_reg[QM1D1C0045_REG_01] |= (0x01<<3);//BB_REG_enable
        QM1D1C0045_d_reg[QM1D1C0045_REG_01] |= (0x01<<2);//HA_LT_enable
        QM1D1C0045_d_reg[QM1D1C0045_REG_01] |= (0x01<<1);//LT_enable
        QM1D1C0045_d_reg[QM1D1C0045_REG_01] &= (~(0x01))&0xFF;//NORMAL
        QM1D1C0045_d_reg[QM1D1C0045_REG_05] &= (~(0x01<<3))&0xFF;//pfd_rst NORMAL

        bRetValue = QM1D1C0045_pll_setdata_once(tuner_id, QM1D1C0045_REG_01 , QM1D1C0045_d_reg[QM1D1C0045_REG_01]);
        if(!bRetValue){
            return bRetValue;
        }
        bRetValue = QM1D1C0045_pll_setdata_once(tuner_id, QM1D1C0045_REG_05 , QM1D1C0045_d_reg[QM1D1C0045_REG_05]);
        if(!bRetValue){
            return bRetValue;
        }
	}else if(!(apConfig->b_QM1D1C0045_loop_through) && (apConfig->b_QM1D1C0045_tuner_standby)){
        //LoopThrough = Disable , TunerMode = Standby
        QM1D1C0045_d_reg[QM1D1C0045_REG_01] &= (~(0x01<<3))&0xFF;//BB_REG_disable
        QM1D1C0045_d_reg[QM1D1C0045_REG_01] &= (~(0x01<<2))&0xFF;//HA_LT_disable
        QM1D1C0045_d_reg[QM1D1C0045_REG_01] &= (~(0x01<<1))&0xFF;//LT_disable
        QM1D1C0045_d_reg[QM1D1C0045_REG_01] |= 0x01;//STDBY
        QM1D1C0045_d_reg[QM1D1C0045_REG_05] |= (0x01<<3);//pfd_rst STANDBY

        bRetValue = QM1D1C0045_pll_setdata_once(tuner_id, QM1D1C0045_REG_05 , QM1D1C0045_d_reg[QM1D1C0045_REG_05]);
        if(!bRetValue){
            return bRetValue;
        }
        bRetValue = QM1D1C0045_pll_setdata_once(tuner_id, QM1D1C0045_REG_01 , QM1D1C0045_d_reg[QM1D1C0045_REG_01]);
        if(!bRetValue){
            return bRetValue;
        }
    }else{//!(iLoopThrough) && !(iTunerMode)
        //LoopThrough = Disable , TunerMode = Normal
        QM1D1C0045_d_reg[QM1D1C0045_REG_01] |= (0x01<<3);//BB_REG_enable
        QM1D1C0045_d_reg[QM1D1C0045_REG_01] |= (0x01<<2);//HA_LT_enable
        QM1D1C0045_d_reg[QM1D1C0045_REG_01] &= (~(0x01<<1))&0xFF;//LT_disable
        QM1D1C0045_d_reg[QM1D1C0045_REG_01] &= (~(0x01))&0xFF;//NORMAL
        QM1D1C0045_d_reg[QM1D1C0045_REG_05] &= (~(0x01<<3))&0xFF;//pfd_rst NORMAL

        bRetValue = QM1D1C0045_pll_setdata_once(tuner_id, QM1D1C0045_REG_01 , QM1D1C0045_d_reg[QM1D1C0045_REG_01]);
        if(!bRetValue){
            return bRetValue;
        }
        bRetValue = QM1D1C0045_pll_setdata_once(tuner_id, QM1D1C0045_REG_05 , QM1D1C0045_d_reg[QM1D1C0045_REG_05]);
        if(!bRetValue){
            return bRetValue;
        }
    }

    /*Head Amp*/
    u8TmpData = QM1D1C0045_pll_getdata_once(tuner_id, QM1D1C0045_REG_01);
    if(u8TmpData&0x04){
        apConfig->b_QM1D1C0045_head_amp = TRUE;
    }else{
        apConfig->b_QM1D1C0045_head_amp = FALSE;
    }
    return TRUE;
}

BOOL QM1D1C0045_set_searchmode(UINT32 tuner_id, PQM1D1C0045_CONFIG_STRUCT    apConfig)
{
	BOOL bRetValue = FALSE;

    if(apConfig->b_QM1D1C0045_fast_search_mode){
        QM1D1C0045_d_reg[QM1D1C0045_REG_03] = QM1D1C0045_pll_getdata_once(tuner_id, QM1D1C0045_REG_03);
        QM1D1C0045_d_reg[QM1D1C0045_REG_03] |= 0x01;
        bRetValue = QM1D1C0045_pll_setdata_once(tuner_id, QM1D1C0045_REG_03 , QM1D1C0045_d_reg[QM1D1C0045_REG_03]);
        if(!bRetValue){
            return bRetValue;
        }
    }else{
        QM1D1C0045_d_reg[QM1D1C0045_REG_03] = QM1D1C0045_pll_getdata_once(tuner_id, QM1D1C0045_REG_03);
        QM1D1C0045_d_reg[QM1D1C0045_REG_03] &= 0xFE;
        bRetValue = QM1D1C0045_pll_setdata_once(tuner_id, QM1D1C0045_REG_03 , QM1D1C0045_d_reg[QM1D1C0045_REG_03]);
        if(!bRetValue){
            return bRetValue;
        }
    }
    return TRUE;
}

