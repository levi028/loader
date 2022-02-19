//The sample code use method 1 of gain fine-tune function.
/*****************************************************************************
*    Tuner sample code
*
*    History:
*          Date           Athor          Version    Reason
*      ============    =============    =========    =================
*    1.Jan.13.2010    monsen           Ver 0.1      Method1
*    2.Apr.7.2010    Roman             Ver 0.1.1    cleanup the interface
*****************************************************************************/
#include <sys_config.h>
#include <retcode.h>
#include <types.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/i2c/i2c.h>
#include <osal/osal.h>
#include "nim_av2011.h"

// Tuner crystal CLK Freqency
static UINT32 av2011_tuner_cnt = 0;
static struct QPSK_TUNER_CONFIG_EXT * av2011_dev_id[MAX_TUNER_SUPPORT_NUM] = {NULL};
static unsigned char tuner_initial[MAX_TUNER_SUPPORT_NUM] = {0,0};
static unsigned short tuner_crystal_fixed = 27; // unit is MHz

// I2C write function (register start address, register array pointer, register length)
static int Tuner_I2C_write(UINT32 tuner_id, unsigned char reg_start, unsigned char* buff, unsigned char length)
{
	UINT8 data[16]={0};
    UINT32 rd = 0;
	int i2c_result = -1;
    struct QPSK_TUNER_CONFIG_EXT * av2011_ptr = NULL;

    if((buff == NULL) || (tuner_id>=MAX_TUNER_SUPPORT_NUM))
        return RET_FAILURE;
    av2011_ptr = av2011_dev_id[tuner_id];
    data[0] = reg_start;

    while((rd+15)<length)
    {
        MEMCPY(&data[1], &buff[rd], 15);
        i2c_result = i2c_write(av2011_ptr->i2c_type_id, av2011_ptr->c_tuner_base_addr, data, 16);
        rd+=15;
        data[0] += 15;
        if(SUCCESS != i2c_result)
            return i2c_result;
    }
    MEMCPY(&data[1], &buff[rd], length-rd);
    i2c_result = i2c_write(av2011_ptr->i2c_type_id, av2011_ptr->c_tuner_base_addr, data, length-rd+1);

    return i2c_result;
}

static int Tuner_I2C_read(UINT32 tuner_id, unsigned char reg_start, unsigned char* buff, unsigned char length)
{
	UINT8 data[16]={0};
    UINT32 rd = 0;
	int i2c_result = -1;
    struct QPSK_TUNER_CONFIG_EXT * av2011_ptr = NULL;

    if((buff == NULL) || (tuner_id>=MAX_TUNER_SUPPORT_NUM))
        return RET_FAILURE;

    av2011_ptr = av2011_dev_id[tuner_id];
    data[0] = reg_start;

    while((rd+15)<length)
    {
        i2c_result = i2c_write_read(av2011_ptr->i2c_type_id, av2011_ptr->c_tuner_base_addr, data, 1, 15);
        MEMCPY(&buff[rd], &data[0], 15);
        rd+=15;
        data[0] += 15;
        if(SUCCESS != i2c_result)
            return i2c_result;
    }
    i2c_result = i2c_write_read(av2011_ptr->i2c_type_id, av2011_ptr->c_tuner_base_addr, data, 1, length-rd);
    MEMCPY(&buff[rd], &data[0], 15);
    return i2c_result;
}
// time delay function ( minisecond )
static void Time_DELAY_MS (unsigned int ms)
{
    if(ms)
    {
    osal_task_sleep(ms);
    }
}

INT32 nim_av2011_init(UINT32* tuner_id, struct QPSK_TUNER_CONFIG_EXT * ptrTuner_Config)
{
	//INT32 result;
    struct QPSK_TUNER_CONFIG_EXT * av2011_ptr = NULL;

	if ((ptrTuner_Config == NULL)||(av2011_tuner_cnt>=MAX_TUNER_SUPPORT_NUM) || (tuner_id == NULL))
        return ERR_FAILUE;
    av2011_ptr = (struct QPSK_TUNER_CONFIG_EXT *)MALLOC(sizeof(struct QPSK_TUNER_CONFIG_EXT));
    if(!av2011_ptr)
          return ERR_FAILUE;
    MEMCPY(av2011_ptr, ptrTuner_Config, sizeof(struct QPSK_TUNER_CONFIG_EXT));
    av2011_dev_id[av2011_tuner_cnt] = av2011_ptr;
    *tuner_id = av2011_tuner_cnt;
    av2011_tuner_cnt++;

    return SUCCESS;
}

/***********************************************************************
* unsigned int nim_av2011_control (unsigned int tuner_id, unsigned int channel_freq, unsigned int bb_sym)
*  Arguments:
*  Parameter1: unsigned int channel_freq        : Channel frequency (MHz)
*  Parameter2: unsigned int bb_sym            : Baseband Symbol Rate (KHz)
*  Return Value: unsigned int            : Result

2012.02.29 Update by bill
                    add extend standby function :
                        if  channel_freq equal defined command in Nim_tuner.h (exp:NIM_TUNER_SET_STANDBY_CMD)
                    nim_av2011_control(UINT32 tuner_id, UINT32 channel_freq, UINT32 bb_sym) useed for extend function,

                    while channel_freq equal NIM_TUNER_SET_STANDBY_CMD
                        bb_sym: 0:standby mode
                                  1:normal mode


***********************************************************************/
INT32 nim_av2011_control(UINT32 tuner_id, UINT32 channel_freq, UINT32 bb_sym)
{
	unsigned char reg[50]={0};
	unsigned int fracN = 0;
	unsigned int BW = 0;
	unsigned int BF = 0;
	unsigned short tuner_crystal = 0;	
    int result = 0; 
    // Auto-scan mode flag. Default is not at auto-scan mode
    unsigned char auto_scan = 0;

	if((tuner_id >= av2011_tuner_cnt) || (tuner_id>=MAX_TUNER_SUPPORT_NUM))
        return ERR_FAILUE;


    // Crystal is fixed. and for dual-tuner, the have same fixed value.
    tuner_crystal = tuner_crystal_fixed;
        
    //add by bill for standby
    if( channel_freq == NIM_TUNER_SET_STANDBY_CMD )
    {
        if( bb_sym==0 )
        {
            reg[12] = 0xf6;
        }
        else if( bb_sym==1 )
        {
            reg[12] = 0xd6;
        }
        else
        {
            return ERR_FAILUE;
        }
		result = Tuner_I2C_write(tuner_id, 12, reg+12, 1);
         //Time delay 4ms
        Time_DELAY_MS(4);
		return result;//SUCCESS;
    }

    // At Power ON, tuner_initial = 0, will run sequence 1~3 at first call of "nim_av2011_control().
    if (tuner_initial[tuner_id] == 0)
    {
           //Initail registers R0~R41
         reg[0]=(char) (0x38);
        reg[1]=(char) (0x00);
        reg[2]=(char) (0x00);
        reg[3]=(char) (0x50);
        reg[4]=(char) (0x1f);
        reg[5]=(char) (0xa3);
        reg[6]=(char) (0xfd);
        reg[7]=(char) (0x58);
        reg[8]=(char) (0x0e);
        reg[9]=(char) (0x82);
        reg[10]=(char) (0x88);
        reg[11]=(char) (0xb4);

        reg[12]=(char) (0xd6); //RFLP=ON at Power on initial
        //reg[12]=(char) (0x96); //RFLP=OFF at Power on initial

        reg[13]=(char) (0x40);

        // reg[14]=(char) (0x5b); //AV2010
        reg[14]=(char) (0x94); //AV2011

        // reg[15]=(char) (0x6a); //AV2010
        reg[15]=(char) (0x4a); //AV2011

        reg[16]=(char) (0x66);
        reg[17]=(char) (0x40);
        reg[18]=(char) (0x80);
        reg[19]=(char) (0x2b);
        reg[20]=(char) (0x6a);
        reg[21]=(char) (0x50);
        reg[22]=(char) (0x91);
        reg[23]=(char) (0x27);
        reg[24]=(char) (0x8f);
        reg[25]=(char) (0xcc);
        reg[26]=(char) (0x21);
        reg[27]=(char) (0x10);
        reg[28]=(char) (0x80);
		reg[29]=(char) (0xE2);
        reg[30]=(char) (0xf5);
        reg[31]=(char) (0x7f);
        reg[32]=(char) (0x4a);
        reg[33]=(char) (0x9b);
        reg[34]=(char) (0xe0);
        reg[35]=(char) (0xe0);
        reg[36]=(char) (0x36);

        reg[37]=(char) (0x00);    // Disble FT function at Power on initial

        reg[38]=(char) (0xab);
        reg[39]=(char) (0x97);
        reg[40]=(char) (0xc5);
        reg[41]=(char) (0xa8);

        // Sequence 1
        // Send Reg0 ->Reg11
        result = Tuner_I2C_write(tuner_id, 0,reg,12);
        if(result!=SUCCESS)
            ali_trace(&result);

        // Sequence 2
        // Send Reg13 ->Reg24
        result = Tuner_I2C_write(tuner_id, 13,reg+13,12);
        if(result!=SUCCESS)
            ali_trace(&result);
          // Send Reg25 ->Reg35
        result = Tuner_I2C_write(tuner_id, 25,reg+25,11);
        if(result!=SUCCESS)
            ali_trace(&result);
        // Send Reg36 ->Reg41
        result = Tuner_I2C_write(tuner_id, 36,reg+36,6);
        if(result!=SUCCESS)
            ali_trace(&result);

        // Sequence 3
        // Send reg12
        result = Tuner_I2C_write(tuner_id, 12,reg+12,1);
		if(result!=SUCCESS)
            ali_trace(&result);

        Time_DELAY_MS(100);
        //Reinitial again
        {
            // Sequence 1
            // Send Reg0 ->Reg11
            result = Tuner_I2C_write(tuner_id, 0,reg,12);
            if(result!=SUCCESS)
                ali_trace(&result);

            // Sequence 2
            // Send Reg13 ->Reg24
            result = Tuner_I2C_write(tuner_id, 13,reg+13,12);
            if(result!=SUCCESS)
                ali_trace(&result);
            // Send Reg25 ->Reg35
            result = Tuner_I2C_write(tuner_id, 25,reg+25,11);
            if(result!=SUCCESS)
                ali_trace(&result);
            // Send Reg36 ->Reg41
            result = Tuner_I2C_write(tuner_id, 36,reg+36,6);
            if(result!=SUCCESS)
                ali_trace(&result);

             // Sequence 3
             // Send reg12
            result = Tuner_I2C_write(tuner_id, 12,reg+12,1);
            if(result!=SUCCESS)
                ali_trace(&result);

         }


        // After power on initial
        tuner_initial[tuner_id] = 1;

         // Time delay 4ms
        Time_DELAY_MS(4);

    }

    // Channel Frequency Calculation.
    fracN = (channel_freq + tuner_crystal/2)/tuner_crystal;
    if(fracN > 0xff)
         fracN = 0xff;
    reg[0]=(char) (fracN & 0xff);
      fracN = (channel_freq<<17)/tuner_crystal;
      fracN = fracN & 0x1ffff;
      reg[1]=(char) ((fracN>>9)&0xff);
      reg[2]=(char) ((fracN>>1)&0xff);

      /******************************************************
         reg[3]_D7 is frac<0>, D6~D0 is 0x50, For differential RXIQ out
         reg[3]_D7 is frac<0>, D6~D0 is 0x54, For single RXIQ out
        ******************************************************/

    reg[3]=(char) (((fracN<<7)&0x80) | 0x50);
 // reg[3]=(char) (((fracN<<7)&0x80) | 0x54);

      // Channel Filter Bandwidth Calculation.
      // rolloff is 35%
      BW = bb_sym*135/200;
      // add 6M when Rs<6.5M for low IF
      if(bb_sym<6500)
        BW = BW + 6000;
    // add 2M for LNB frequency shifting
    BW = BW + 2000;
    // add 8% margin since the calculated fc of BB Auto-scanning is not very accurate
    BW = BW*108/100;
    // Bandwidth can be tuned from 4M to 40M
    if( BW< 4000)
      BW = 4000;
    if( BW> 40000)
      BW = 40000;
    //BW(MHz) * 1.27 / 211KHz
    BF = (BW*127 + 21100/2) / (21100);
    reg[5] = (unsigned char)BF;

    // Auto-scan mode setting is depended on the algorithm of Base-Band.
    // Here shows a example.
    // When bb_sym is 0 or 45000, means auto-scan channel.
    // Base-band set a fixed BW=27MHz at auto-scan mode.
    if ((bb_sym == 0) || (bb_sym == 45000))
    {
        auto_scan = 1;
        reg[5] = 0xA3; //BW=27MHz
    }

    if(auto_scan)
    {
          // Sequence 4
          // Send Reg0 ->Reg4
        result = Tuner_I2C_write(tuner_id, 0,reg,4);
        if(result!=SUCCESS)
            ali_trace(&result);
          // Time delay 4ms
          Time_DELAY_MS(10);

          // Sequence 5
          // Send Reg5
        result = Tuner_I2C_write(tuner_id, 5, reg+5, 1);
        if(result!=SUCCESS)
            ali_trace(&result);

          // Fine-tune Function Control
          //Auto-scan mode. FT_block=1, FT_EN=0, FT_hold=1
        reg[37] = 0x00;
        result = Tuner_I2C_write(tuner_id, 37, reg+37, 1);
        if(result!=SUCCESS)
            ali_trace(&result);
        // Time delay 4ms
        Time_DELAY_MS(10);
    }
    else
    {
        // Sequence 4
          // Send Reg0 ->Reg4
        result = Tuner_I2C_write(tuner_id, 0,reg,4);
        if(result!=SUCCESS)
            ali_trace(&result);

        // Time delay 4ms
        Time_DELAY_MS(4);

        // Sequence 5
        // Send Reg5
        result = Tuner_I2C_write(tuner_id, 5, reg+5, 1);
        if(result!=SUCCESS)
            ali_trace(&result);

          // Fine-tune Function Control
          // Non-auto-scan mode. FT_block=1, FT_EN=1, FT_hold=0
        reg[37] = 0x06;
        result = Tuner_I2C_write(tuner_id, 37, reg+37, 1);
        if(result!=SUCCESS)
            ali_trace(&result);
        // Fine-tune function is starting tracking after sending reg[37].
        // Make sure the RFAGC do not have a sharp jump.

        reg[12] = 0xd6; //Enable RFLP at Lock Channel sequence after reg[37]
        // reg[12] = 0xd6; //Enable RFLP at Lock Channel sequence after reg[37]

        result = Tuner_I2C_write(tuner_id, 12, reg+12, 1);
        if(result!=SUCCESS)
            ali_trace(&result);

        // Time delay 4ms
        Time_DELAY_MS(4);
    }

    return SUCCESS;
}

INT32 nim_av2011_status(UINT32 tuner_id, UINT8 *lock)
{
    if(lock == NULL)
        return RET_FAILURE;
	if ((tuner_id>=av2011_tuner_cnt)||(tuner_id>=MAX_TUNER_SUPPORT_NUM))
    {
        *lock = 0;
        return ERR_FAILUE;
    }
    /* Because AV2011 doesn't has this flag,return 1 directly */
    *lock = 1;
    return SUCCESS;
}



