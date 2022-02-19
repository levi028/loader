/*****************************************************************************
*    Copyright (C)2004 Ali Corporation. All Rights Reserved.
*
*    File:    lib_diseqc.c
*
*    Description:    This file contains all DiSEqC library define.
*    History:
*           Date            Athor        Version          Reason
*        ============    =============    =========    =================
*    1.    Jan.8.2004         Leo Guo       Ver 0.1       Create file.
*    2.
*****************************************************************************/
#include <sys_config.h>
#include <types.h>
#include <retcode.h>
#include <osal/osal.h>
#include <hal/hal_gpio.h>
#include <hld/hld_dev.h>
#include <hld/nim/nim.h>
#include <hld/nim/nim_dev.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libdiseqc/lib_diseqc.h>


#define DISEQC_PRINTF    PRINTF
#define DISEQC2_PRINTF    libc_printf
#define DISEQC2X_TEST

#if (SYS_SDRAM_SIZE != 2)
#define LIB_DISEQC_HIGH_FUNC_SUPPORT 1

#endif

//=>> these macro use for our power supply problem when sending diseqc command.
#define V13_HELP_ON            1
#define V13_HELP_OFF        0
#define DISEQC_WITH_13V_HELP    V13_HELP_OFF //V13_HELP_ON//

#define CASCADE_DELAY_TIME            150
#define POLAR_CHANGE_DELAY_TIME    100

INT32  api_diseqc_set_polar(struct nim_device *lib_diseqc_nim_dev, UINT8 polar)
{
    if (lib_diseqc_nim_dev == NULL)
        return RET_SUCCESS;

    lib_diseqc_nim_dev->diseqc_info.diseqc_polar = polar+1;

#ifdef LNB_POLARITY_INVERT
/*for cost down polar control circuit, if H wanted, should set nim polar V,
then after the circuit, outpput H   */
    //return nim_set_polar(lib_diseqc_nim_dev, 1-polar);
    return nim_io_control(lib_diseqc_nim_dev, NIM_DRIVER_SET_POLAR, 1-polar);
#else
    //return nim_set_polar(lib_diseqc_nim_dev, polar);
    return nim_io_control(lib_diseqc_nim_dev, NIM_DRIVER_SET_POLAR, polar);
#endif
}


static INT32 di_seq_c_operate(struct nim_device *dev, UINT32 mode, UINT8* cmd, UINT8 cnt)
{
    nim_diseqc_operate_para_t diseqc_para;
    if(NULL != dev && NULL != dev->do_ioctl_ext)
    {
        diseqc_para.mode = mode;
        diseqc_para.cmd = cmd;
        diseqc_para.cnt = cnt;
        return dev->do_ioctl_ext(dev, NIM_DRIVER_DISEQC_OPERATION, (void *)&diseqc_para);
    }
    return ERR_FAILED;
}

static INT32 di_seq_c2x_operate(struct nim_device *dev, UINT32 mode, UINT8* cmd, UINT8 cnt, \
                                    UINT8 *rt_value, UINT8 *rt_cnt)
{
    nim_diseqc_operate_para_t diseqc_para;
    if(NULL != dev && NULL != dev->do_ioctl_ext)
    {
        diseqc_para.mode = mode;
        diseqc_para.cmd = cmd;
        diseqc_para.cnt = cnt;
        diseqc_para.rt_value = rt_value;
        diseqc_para.rt_cnt = rt_cnt;
        return dev->do_ioctl_ext(dev, NIM_DRIVER_DISEQC2X_OPERATION, (void *)&diseqc_para);
    }
    return ERR_FAILED;
}

/*****************************************************************************
* INT32  api_nim_set_22k_envelop(UINT8 flag)
*
* Set 22KHz enable or disable
*
* Arguments:
*  Parameter1: UINT8 mode  0:envelop off  1:envelop on
*
* Return Value: INT32
*****************************************************************************/
INT32  api_diseqc_set_22k_envelop(struct nim_device *lib_diseqc_nim_dev, UINT8 mode)
{
    if( 0 == mode )
        return di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_ENVELOP_OFF , NULL, 0);
    else if( 1 == mode )
        return di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_ENVELOP_ON , NULL, 0);
    else
        return ERR_FAILED;
}

/*****************************************************************************
* INT32  api_nim_set_22k(UINT8 flag)
*
* Set 22KHz enable or disable
*
* Arguments:
*  Parameter1: UINT8 flag
*
* Return Value: INT32
*****************************************************************************/
INT32  api_diseqc_set_22k(struct nim_device *lib_diseqc_nim_dev, UINT8 flag)
{
    if (lib_diseqc_nim_dev == NULL)
        return RET_SUCCESS;
    lib_diseqc_nim_dev->diseqc_info.diseqc_k22 = flag;
    return di_seq_c_operate(lib_diseqc_nim_dev, flag ? NIM_DISEQC_MODE_22KON : NIM_DISEQC_MODE_22KOFF, NULL, 0);
}


/*****************************************************************************
* INT32  api_nim_set_tone_bust(UINT8 mode)
*
* Set mode 0 or 1
*
* Arguments:
*  Parameter1: UINT8 mode
*
* Return Value: INT32
*****************************************************************************/
INT32  api_diseqc_set_tone_burst(struct nim_device *lib_diseqc_nim_dev, UINT8 mode)
{
    if( 0 == mode )
        return di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BURST0, NULL, 0);
    else
        return di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BURST1, NULL, 0);

}

/*****************************************************************************
* INT32  api_nim_set_12v(UINT8 flag)
*
* Set 12V enable or disable
*
* Arguments:
*  Parameter1: UINT8 flag
*
* Return Value: INT32
*****************************************************************************/
INT32  api_diseqc_set_12v(struct nim_device *lib_diseqc_nim_dev, UINT8 flag)
{
//It should be put to driver lay,but in order to min change on driver,so put it here.
//    return nim_set_12v(lib_diseqc_nim_dev, flag);
#if(SYS_12V_SWITCH == SYS_FUNC_ON)
{
    HAL_GPIO_BIT_DIR_SET(SYS_12V_GPIO, HAL_GPIO_O_DIR);
    if (flag == 1)
    {
        HAL_GPIO_BIT_SET(SYS_12V_GPIO, 1);
    }
    else
    {
        HAL_GPIO_BIT_SET(SYS_12V_GPIO, 0);
    }
    return SUCCESS;
}
#else
    return SUCCESS;
#endif
}

#if(SYS_LNB_POWER_OFF == SYS_FUNC_ON)
static UINT8 lnb_power_cut_pos = GPIO_NULL;
void api_diseqc_set_lnb_power_cut_pos(UINT8 pos)
{
    lnb_power_cut_pos = pos;
}

//->these tow following fuction are from power.c, related to diff project.
void api_diseqc_lnb_power(struct nim_device *lib_diseqc_nim_dev, UINT8 sw)
{
    UINT8 pos;
    pos = lnb_power_cut_pos;
#ifdef SYS_LNB_POWER_CUT
    pos = SYS_LNB_POWER_CUT;
#endif
    if(GPIO_NULL != pos)
    {
        HAL_GPIO_BIT_DIR_SET(pos, HAL_GPIO_O_DIR);
        if (sw == 1)
        {
            HAL_GPIO_BIT_SET(pos, 1);
        }
        else
        {
            HAL_GPIO_BIT_SET(pos, 0);
        }
    }
}
#endif


INT32  api_diseqc_16port_switch(struct nim_device *lib_diseqc_nim_dev, UINT8 flag)
{
    UINT8 cmd[4];

    if (flag > 15)
    {
        return ERR_FAILUE;
    }

    cmd[0] = 0xe0;
    cmd[1] = 0x10;
    cmd[2] = 0x38;
    switch (flag)
    {
        case 0:
            cmd[3] = 0xf0;
            break;
        case 1:
            cmd[3] = 0xf1;
            break;
        case 2:
            cmd[3] = 0xf2;
            break;
        case 3:
            cmd[3] = 0xf3;
            break;
        case 4:
            cmd[3] = 0xf4;
            break;
        case 5:
            cmd[3] = 0xf5;
            break;
        case 6:
            cmd[3] = 0xf6;
            break;
        case 7:
            cmd[3] = 0xf7;
            break;
        case 8:
            cmd[3] = 0xf8;
            break;
        case 9:
            cmd[3] = 0xf9;
            break;
        case 10:
            cmd[3] = 0xfa;
            break;
        case 11:
            cmd[3] = 0xfb;
            break;
        case 12:
            cmd[3] = 0xfc;
            break;
        case 13:
            cmd[3] = 0xfd;
            break;
        case 14:
            cmd[3] = 0xfe;
            break;
        case 15:
            cmd[3] = 0xff;
            break;
    default:
            break;
    }

    return di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 4);

}



INT32 diseqc_switch(struct nim_device *dev, UINT8 addr,UINT8 committed,UINT8 *cmd, UINT8 port, UINT8 *reply, UINT8 *reply_cnt)
{
    if(port > 15)
    {
        return ERR_FAILUE;
    }

    cmd[1] = addr;
    if(committed == 1)
        cmd[2] = CMD_WRITE_PORT0;
    else
        cmd[2] = CMD_WRITE_PORT1;

    switch (port)
    {
        case 0:
            cmd[3] = 0xf0;
            break;
        case 1:
            cmd[3] = 0xf1;
            break;
        case 2:
            cmd[3] = 0xf2;
            break;
        case 3:
            cmd[3] = 0xf3;
            break;
        case 4:
            cmd[3] = 0xf4;
            break;
        case 5:
            cmd[3] = 0xf5;
            break;
        case 6:
            cmd[3] = 0xf6;
            break;
        case 7:
            cmd[3] = 0xf7;
            break;
        case 8:
            cmd[3] = 0xf8;
            break;
        case 9:
            cmd[3] = 0xf9;
            break;
        case 10:
            cmd[3] = 0xfa;
            break;
        case 11:
            cmd[3] = 0xfb;
            break;
        case 12:
            cmd[3] = 0xfc;
            break;
        case 13:
            cmd[3] = 0xfd;
            break;
        case 14:
            cmd[3] = 0xfe;
            break;
        case 15:
            cmd[3] = 0xff;
            break;

    }
    if(cmd[0] <= 0xE1)
        return di_seq_c_operate(dev, NIM_DISEQC_MODE_BYTES, cmd, 4);
       else
        return di_seq_c2x_operate(dev, NIM_DISEQC_MODE_BYTES, cmd, 4, reply, reply_cnt);

}


#if (LIB_DISEQC_HIGH_FUNC_SUPPORT)
/*
static INT32 diseqc_cmd_set_reply(UINT8 *cmd)
{
    if(cmd == NULL)
        return ERR_FAILUE;

    cmd[0] = (cmd[0]|0x02);

}

static INT32 diseqc_cmd_set_slave_addr(UINT8 *cmd ,UINT8 addr)
{
    if(cmd == NULL)
        return ERR_FAILUE;

    cmd[1] = addr;

}
*/

static INT32 diseqc_cmd_set_repeate(UINT8 *cmd)
{
    if(cmd == NULL)
        return ERR_FAILUE;

    cmd[0] = (cmd[0]|0x01);
    return SUCCESS;
}


INT32 diseqc2x_switch(struct nim_device *dev, UINT8 addr,UINT8 committed,UINT8 *cmd, UINT8 port, UINT8 *reply, UINT8 *reply_cnt)
{
    UINT8 i=0;
    INT32 ret = SUCCESS;

    ret = diseqc_switch(dev, addr,committed,cmd, port, reply, reply_cnt);
    DISEQC2_PRINTF("diseqc2x switch first cmd,addr=%x,port =%d, ret=%d,reply_cnt=%d,reply[0]=%x\n",addr,port,ret,*reply_cnt,reply[0]);
    if((ret == SUCCESS) && (*reply_cnt >= 1) && (reply[0]==SLAVE_CMD_NSUPPORT))
        return diseqc2x_err_cmd_nsupport;
    else if(ret != SUCCESS || ((ret == SUCCESS)&&(reply[0]!=SLAVE_REPLY_OK)))
    {
        //DISEQC2_PRINTF("diseqc2x switch, first send not ok, return byte=%d\n",reply[0]);
        diseqc_cmd_set_repeate(cmd);
        for(i = 0; i < 1; i++)
        {
            MEMSET(reply, 0, 6);
            *reply_cnt = 0;
            ret = diseqc_switch(dev, addr,committed,cmd, port, reply, reply_cnt);
            DISEQC2_PRINTF("%d resend, ret=%d,reply_cnt=%d,reply[0]=%x\n",i+1, ret,*reply_cnt,reply[0]);
            if((SUCCESS==ret) && (*reply_cnt >= 1) && (reply[0]==SLAVE_REPLY_OK))
                break;
        }
    }
    //after re-send 2 times
    if(ret != SUCCESS)
    {
        DISEQC2_PRINTF("diseqc2x switch, after resend, unsuccess in driver recieve, error code=%d\n\n",i,reply[0]);
        if(reply[0]==DISEQC2X_ERR_NO_REPLY)
            ret = diseqc2x_err_no_reply;
        else if(reply[0]==DISEQC2X_ERR_REPLY_PARITY)
            ret = diseqc2x_err_reply_parity;
        else if(reply[0]==DISEQC2X_ERR_REPLY_UNKNOWN)
            ret = diseqc2x_err_reply_unknown;
        else if(reply[0]==DISEQC2X_ERR_REPLY_BUF_FUL)
            ret = diseqc2x_err_reply_buf_ful;
    }
    else
    {
        if((*reply_cnt >= 1) && (reply[0]!=SLAVE_REPLY_OK))
        {
            DISEQC2_PRINTF("diseqc2x switch, after 2 resend, slave reply error byte,reply[0]=%d\n\n",reply[0]);
            if(reply[0]==SLAVE_CMD_NSUPPORT)
                ret = diseqc2x_err_cmd_nsupport;
            else if(reply[0]==SLAVE_CMD_PARITY)
                ret = diseqc2x_err_cmd_parity;
            else if(reply[0]==SLAVE_CMD_NRECOGNIZE)
                ret = diseqc2x_err_cmd_nrecognize;
        }
    }
    return ret;

}

/*****************************************************************************
* INT32  api_diseqc_reset(void)
*
* Reset the DiSEqC microcontroller
*
* Arguments:
*  None
*
* Return Value: INT32
*****************************************************************************/
INT32 api_diseqc_reset(struct nim_device *lib_diseqc_nim_dev)
{
    UINT8 cmd[3];

    cmd[0] = 0xe0;

    cmd[1] = ADDR_ALL_DEVICES;
    cmd[2] = CMD_DISEQC_RESET;

    return di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 3);
}

/*****************************************************************************
* INT32  api_diseqc_clr_reset(void)
*
* Clear the DiSEqC microcontroller reset flag
*
* Arguments:
*  None
*
* Return Value: INT32
*****************************************************************************/
INT32 api_diseqc_clr_reset(struct nim_device *lib_diseqc_nim_dev)
{
    UINT8 cmd[3];

    cmd[0] = 0xe0;

    cmd[1] = ADDR_ALL_DEVICES;
    cmd[2] = CMD_CLEAR_RESET_FLAG;

    return di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 3);
}

/*****************************************************************************
* INT32  api_diseqc_standby(void)
*
* Switch the peripheral power supply off
*
* Arguments:
*  None
*
* Return Value: INT32
*****************************************************************************/
INT32 api_diseqc_standby(struct nim_device *lib_diseqc_nim_dev)
{
    UINT8 cmd[3];

    cmd[0] = 0xe0;

    cmd[1] = ADDR_ALL_DEVICES;
    cmd[2] = CMD_DISEQC_STANDBY;

    return di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 3);
}

/*****************************************************************************
* INT32  api_diseqc_power_on(void)
*
* Switch the peripheral power supply on
*
* Arguments:
*  None
*
* Return Value: INT32
*****************************************************************************/
INT32 api_diseqc_power_on(struct nim_device *lib_diseqc_nim_dev)
{
    UINT8 cmd[3];

    cmd[0] = 0xe0;

    cmd[1] = ADDR_ALL_DEVICES;
    cmd[2] = CMD_DISEQC_POWERON;

    return di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 3);
}

/*****************************************************************************
* INT32  api_diseqc_sleep(void)
*
* Set the slave devices in sleep mode, ignore all coammands except "awake"
*
* Arguments:
*  None
*
* Return Value: INT32
*****************************************************************************/
INT32 api_diseqc_sleep(struct nim_device *lib_diseqc_nim_dev)
{
    UINT8 cmd[3];

    cmd[0] = 0xe0;

    cmd[1] = ADDR_ALL_DEVICES;
    cmd[2] = CMD_SLEEP;

    return di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 3);
}

/*****************************************************************************
* INT32  api_diseqc_awake(void)
*
* Awake the slave devices
*
* Arguments:
*  None
*
* Return Value: INT32
*****************************************************************************/
INT32 api_diseqc_awake(struct nim_device *lib_diseqc_nim_dev)
{
    UINT8 cmd[3];

    cmd[0] = 0xe0;

    cmd[1] = ADDR_ALL_DEVICES;
    cmd[2] = CMD_AWAKE;

    return di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 3);
}


/*****************************************************************************
* INT32  api_diseqc_select_lo(void)
*
* Select the low local oscillator frequency
*
* Arguments:
*     none
*
* Return Value: INT32
*****************************************************************************/
INT32 api_diseqc_select_lo(struct nim_device *lib_diseqc_nim_dev)
{
    UINT8 cmd[3];

    cmd[0] = 0xe0;

    cmd[1] = ADDR_LNB;
    cmd[2] = CMD_SELECT_LO;

    return di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 3);
}

/*****************************************************************************
* INT32  api_diseqc_select_hi(void)
*
* Select the high local oscillator frequency
*
* Arguments:
*     none
*
* Return Value: INT32
*****************************************************************************/
INT32 api_diseqc_select_hi(struct nim_device *lib_diseqc_nim_dev)
{
    UINT8 cmd[3];
    
    cmd[0] = 0xe0;

    cmd[1] = ADDR_LNB;
    cmd[2] = CMD_SELECT_HI;

    return di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 3);
}

/*****************************************************************************
* INT32  api_diseqc_select_vr(void)
*
* Select the LNB vertical polarisation
*
* Arguments:
*     none
*
* Return Value: INT32
*****************************************************************************/
INT32 api_diseqc_select_vr(struct nim_device *lib_diseqc_nim_dev)
{
    UINT8 cmd[3];

    cmd[0] = 0xe0;

    cmd[1] = ADDR_LNB;
    cmd[2] = CMD_SELECT_VR;

    return di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 3);
}

/*****************************************************************************
* INT32  api_diseqc_select_hl(void)
*
* Select the LNB horizontal polarisation
*
* Arguments:
*     none
*
* Return Value: INT32
*****************************************************************************/
INT32 api_diseqc_select_hl(struct nim_device *lib_diseqc_nim_dev)
{
    UINT8 cmd[3];

    cmd[0] = 0xe0;

    cmd[1] = ADDR_LNB;
    cmd[2] = CMD_SELECT_HL;

    return di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 3);
}

/*****************************************************************************
* INT32  api_diseqc_select_switch_option_a(void)
*
* Select DiSEqC switch option A
*
* Arguments:
*     none
*
* Return Value: INT32
*****************************************************************************/
INT32 api_diseqc_select_switch_option_a(struct nim_device *lib_diseqc_nim_dev)
{
    UINT8 cmd[3];

    cmd[0] = 0xe0;

    cmd[1] = ADDR_SWITCH;
    cmd[2] = CMD_SELECT_SOA;
    return di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 3);
}

/*****************************************************************************
* INT32  api_diseqc_select_switch_option_b(void)
*
* Select DiSEqC switch option B
*
* Arguments:
*     none
*
* Return Value: INT32
*****************************************************************************/
INT32 api_diseqc_select_switch_option_b(struct nim_device *lib_diseqc_nim_dev)
{
    UINT8 cmd[3];

    cmd[0] = 0xe0;

    cmd[1] = ADDR_SWITCH;
    cmd[2] = CMD_SELECT_SOB;
    return di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 3);
}

/*****************************************************************************
* INT32  api_diseqc_select_satellite_position_a(void)
*
* Select the satellite position a(or position c)
*
* Arguments:
*     none
*
* Return Value: INT32
*****************************************************************************/
INT32 api_diseqc_select_satellite_position_a(struct nim_device *lib_diseqc_nim_dev)
{
    UINT8 cmd[3];

    cmd[0] = 0xe0;

    cmd[1] = ADDR_SWITCH;
    cmd[2] = CMD_SELECT_POSITION_A;
    return di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 3);
}

/*****************************************************************************
* INT32  api_diseqc_select_satellite_position_b(void)
*
* Select the satellite position b(or position d)
*
* Arguments:
*     none
*
* Return Value: INT32
*****************************************************************************/
INT32 api_diseqc_select_satellite_position_b(struct nim_device *lib_diseqc_nim_dev)
{
    UINT8 cmd[3];

    cmd[0] = 0xe0;

    cmd[1] = ADDR_SWITCH;
    cmd[2] = CMD_SELECT_POSITION_B;
    return di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 3);
}

/*****************************************************************************
* INT32  api_diseqc_select_S1A(void)
*
* Select switch S1 input A(deselect B)
*
* Arguments:
*     none
*
* Return Value: INT32
*****************************************************************************/
INT32 api_diseqc_select_s1a(struct nim_device *lib_diseqc_nim_dev)
{
    UINT8 cmd[3];

    cmd[0] = 0xe0;

    cmd[1] = ADDR_SWITCH;
    cmd[2] = CMD_SELECT_S1A;
    return di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 3);
}

/*****************************************************************************
* INT32  api_diseqc_select_S2A(void)
*
* Select switch S2 input A(deselect B)
*
* Arguments:
*     none
*
* Return Value: INT32
*****************************************************************************/
INT32 api_diseqc_select_s2a(struct nim_device *lib_diseqc_nim_dev)
{
    UINT8 cmd[3];

    cmd[0] = 0xe0;

    cmd[1] = ADDR_SWITCH;
    cmd[2] = CMD_SELECT_S2A;
    return di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 3);
}

/*****************************************************************************
* INT32  api_diseqc_select_S3A(void)
*
* Select switch S3 input A(deselect B)
*
* Arguments:
*     none
*
* Return Value: INT32
*****************************************************************************/
INT32 api_diseqc_select_s3a(struct nim_device *lib_diseqc_nim_dev)
{
    UINT8 cmd[3];

    cmd[0] = 0xe0;

    cmd[1] = ADDR_SWITCH;
    cmd[2] = CMD_SELECT_S3A;
    return di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 3);
}

/*****************************************************************************
* INT32  api_diseqc_select_S4A(void)
*
* Select switch S4 input A(deselect B)
*
* Arguments:
*     none
*
* Return Value: INT32
*****************************************************************************/
INT32 api_diseqc_select_s4a(struct nim_device *lib_diseqc_nim_dev)
{
    UINT8 cmd[3];

    cmd[0] = 0xe0;

    cmd[1] = ADDR_SWITCH;
    cmd[2] = CMD_SELECT_S4A;
    return di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 3);
}

/*****************************************************************************
* INT32  api_diseqc_select_S1B(void)
*
* Select switch S1 input B(deselect A)
*
* Arguments:
*     none
*
* Return Value: INT32
*****************************************************************************/
INT32 api_diseqc_select_s1b(struct nim_device *lib_diseqc_nim_dev)
{
    UINT8 cmd[3];

    cmd[0] = 0xe0;

    cmd[1] = ADDR_SWITCH;
    cmd[2] = CMD_SELECT_S1B;
    return di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 3);
}

/*****************************************************************************
* INT32  api_diseqc_select_S2B(void)
*
* Select switch S2 input B(deselect A)
*
* Arguments:
*     none
*
* Return Value: INT32
*****************************************************************************/
INT32 api_diseqc_select_s2b(struct nim_device *lib_diseqc_nim_dev)
{
    UINT8 cmd[3];

    cmd[0] = 0xe0;

    cmd[1] = ADDR_SWITCH;
    cmd[2] = CMD_SELECT_S2B;
    return di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 3);
}

/*****************************************************************************
* INT32  api_diseqc_select_S3B(void)
*
* Select switch S3 input B(deselect A)
*
* Arguments:
*     none
*
* Return Value: INT32
*****************************************************************************/
INT32 api_diseqc_select_s3b(struct nim_device *lib_diseqc_nim_dev)
{
    UINT8 cmd[3];

    cmd[0] = 0xe0;

    cmd[1] = ADDR_SWITCH;
    cmd[2] = CMD_SELECT_S3B;
    return di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 3);
}

/*****************************************************************************
* INT32  api_diseqc_select_S4B(void)
*
* Select switch S4 input B(deselect A)
*
* Arguments:
*     none
*
* Return Value: INT32
*****************************************************************************/
INT32 api_diseqc_select_s4b(struct nim_device *lib_diseqc_nim_dev)
{
    UINT8 cmd[3];

    cmd[0] = 0xe0;

    cmd[1] = ADDR_SWITCH;
    cmd[2] = CMD_SELECT_S4B;
    return di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 3);
}




/*****************************************************************************
* INT32  api_diseqc_goto_satellite_position(UINT8 nn)
*
* Drive the motor to the satellite[nn] position
*
* Arguments:
*  Parameter1: UINT8 nn
*
* Return Value: INT32
*****************************************************************************/
#ifndef NEW_DISEQC_LIB
INT32 api_diseqc_goto_satellite_position(struct nim_device *lib_diseqc_nim_dev, UINT8 nn)
#else
INT32 api_diseqc_goto_satellite_position(struct nim_device *lib_diseqc_nim_dev, UINT8 nn, UINT8 k22_mode)
#endif
{
    UINT8 cmd[4];

    cmd[0] = 0xe0;

    cmd[1] = ADDR_POLAR_POSITIONER;
    cmd[2] = CMD_DRIVEMOTOR_GOTO_POSITION;
    cmd[3] = nn;

    return di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 4);
}

/*****************************************************************************
* INT32  api_diseqc_store_satellite_position(UINT8 nn)
*
* Save the current antenna position as position[nn]
*
* Arguments:
*  Parameter1: UINT8 nn
*
* Return Value: INT32
*****************************************************************************/
#ifndef NEW_DISEQC_LIB
INT32 api_diseqc_store_satellite_position(struct nim_device *lib_diseqc_nim_dev, UINT8 nn)
#else
INT32 api_diseqc_store_satellite_position(struct nim_device *lib_diseqc_nim_dev, UINT8 nn, UINT8 k22_mode)
#endif
{
    UINT8 cmd[4];

    cmd[0] = 0xe0;

    cmd[1] = ADDR_POLAR_POSITIONER;
    cmd[2] = CMD_STORE_SATELLITE_POSITION;
    cmd[3] = nn;

    return di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 4);
}


/*****************************************************************************
* INT32  api_diseqc_halt_motor(void)
*
* Stop the motor
*
* Arguments:
*  none
*
* Return Value: INT32
*****************************************************************************/
#ifndef NEW_DISEQC_LIB
INT32 api_diseqc_halt_motor(struct nim_device *lib_diseqc_nim_dev)
#else
INT32 api_diseqc_halt_motor(struct nim_device *lib_diseqc_nim_dev, UINT8 k22_mode)
#endif
{
    UINT8 cmd[3];

    cmd[0] = 0xe0;

    cmd[1] = ADDR_POLAR_POSITIONER;
    cmd[2] = CMD_HALT_MOTOR;

    return di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 3);
}


/*****************************************************************************
* INT32  api_diseqc_drive_motor_west(void)
*
* Drive the motor west continuously
*
* Arguments:
*  none
*
* Return Value: INT32
*****************************************************************************/
#ifndef NEW_DISEQC_LIB
INT32 api_diseqc_drive_motor_west(struct nim_device *lib_diseqc_nim_dev)
#else
INT32 api_diseqc_drive_motor_west(struct nim_device *lib_diseqc_nim_dev, UINT8 k22_mode)
#endif
{
    UINT8 cmd[4];

    cmd[0] = 0xe0;

    cmd[1] = ADDR_POLAR_POSITIONER;
    cmd[2] = CMD_DRIVE_MOTOR_WEST;
    cmd[3] = 0;
    return di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 4);
}

/*****************************************************************************
* INT32  api_diseqc_drive_motor_west_with_timeout(INT8 timeout)
*
* Drive the motor west with timeout
*
* Arguments:
*  Parameter1: INT8 timeout
*
* Return Value: INT32
*****************************************************************************/
#ifndef NEW_DISEQC_LIB
INT32 api_diseqc_drive_motor_west_with_timeout(struct nim_device *lib_diseqc_nim_dev, INT8 timeout)
#else
INT32 api_diseqc_drive_motor_west_with_timeout(struct nim_device *lib_diseqc_nim_dev, INT8 timeout, UINT8 k22_mode)
#endif
{
    UINT8 cmd[4];

    cmd[0] = 0xe0;

    cmd[1] = ADDR_POLAR_POSITIONER;
    cmd[2] = CMD_DRIVE_MOTOR_WEST;
    cmd[3] = timeout;

    return di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 4);
}


/*****************************************************************************
* INT32  api_diseqc_drive_motor_west_by_steps(INT8 steps)
*
* Drive the motor west by steps
*
* Arguments:
*  Parameter1: INT8 steps
*
* Return Value: INT32
*****************************************************************************/
#ifndef NEW_DISEQC_LIB
INT32 api_diseqc_drive_motor_west_by_steps(struct nim_device *lib_diseqc_nim_dev, INT8 steps)
#else
INT32 api_diseqc_drive_motor_west_by_steps(struct nim_device *lib_diseqc_nim_dev, INT8 steps, UINT8 k22_mode)
#endif
{
    UINT8 cmd[4];

    cmd[0] = 0xe0;

    cmd[1] = ADDR_POLAR_POSITIONER;
    cmd[2] = CMD_DRIVE_MOTOR_WEST;
    cmd[3] = 0xff-steps;

    return di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 4);
}

/*****************************************************************************
* INT32  api_diseqc_drive_motor_east(void)
*
* Drive the motor east continuously
*
* Arguments:
*  none
*
* Return Value: INT32
*****************************************************************************/
#ifndef NEW_DISEQC_LIB
INT32 api_diseqc_drive_motor_east(struct nim_device *lib_diseqc_nim_dev)
#else
INT32 api_diseqc_drive_motor_east(struct nim_device *lib_diseqc_nim_dev, UINT8 k22_mode)
#endif
{
    UINT8 cmd[4];

    cmd[0] = 0xe0;

    cmd[1] = ADDR_POLAR_POSITIONER;
    cmd[2] = CMD_DRIVE_MOTOR_EAST;
    cmd[3] = 0;
    return di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 4);
}

/*****************************************************************************
* INT32  api_diseqc_drive_motor_east_with_timeout(INT8 timeout)
*
* Drive the motor east with timeout
*
* Arguments:
*  Parameter1: INT8 timeout
*
* Return Value: INT32
*****************************************************************************/
#ifndef NEW_DISEQC_LIB
INT32 api_diseqc_drive_motor_east_with_timeout(struct nim_device *lib_diseqc_nim_dev, INT8 timeout)
#else
INT32 api_diseqc_drive_motor_east_with_timeout(struct nim_device *lib_diseqc_nim_dev, INT8 timeout, UINT8 k22_mode)
#endif
{
    UINT8 cmd[4];

    cmd[0] = 0xe0;

    cmd[1] = ADDR_POLAR_POSITIONER;
    cmd[2] = CMD_DRIVE_MOTOR_EAST;
    cmd[3] = timeout;

    return di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 4);
}


/*****************************************************************************
* INT32  api_diseqc_drive_motor_east_by_steps(INT8 steps)
*
* Drive the motor east by steps
*
* Arguments:
*  Parameter1: INT8 steps
*
* Return Value: INT32
*****************************************************************************/
#ifndef NEW_DISEQC_LIB
INT32 api_diseqc_drive_motor_east_by_steps(struct nim_device *lib_diseqc_nim_dev, INT8 steps)
#else
INT32 api_diseqc_drive_motor_east_by_steps(struct nim_device *lib_diseqc_nim_dev, INT8 steps, UINT8 k22_mode)
#endif
{
    UINT8 cmd[4];

    cmd[0] = 0xe0;

    cmd[1] = ADDR_POLAR_POSITIONER;
    cmd[2] = CMD_DRIVE_MOTOR_EAST;
    cmd[3] = 0xff-steps;

    return di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 4);
}

/*****************************************************************************
* INT32  api_diseqc_set_west_limit(void)
*
* Set the current antenna position as the west postion limit and enable it
*
* Arguments:
*  none
*
* Return Value: INT32
*****************************************************************************/
#ifndef NEW_DISEQC_LIB
INT32 api_diseqc_set_west_limit(struct nim_device *lib_diseqc_nim_dev)
#else
INT32 api_diseqc_set_west_limit(struct nim_device *lib_diseqc_nim_dev, UINT8 k22_mode)
#endif
{
    UINT8 cmd[3];

    cmd[0] = 0xe0;

    cmd[1] = ADDR_POLAR_POSITIONER;
    cmd[2] = CMD_SET_WEST_LIMIT;

    return di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 3);
}


/*****************************************************************************
* INT32  api_diseqc_set_east_limit(void)
*
* Set the current antenna position as the east postion limit and enable it
*
* Arguments:
*  none
*
* Return Value: INT32
*****************************************************************************/
#ifndef NEW_DISEQC_LIB
INT32 api_diseqc_set_east_limit(struct nim_device *lib_diseqc_nim_dev)
#else
INT32 api_diseqc_set_east_limit(struct nim_device *lib_diseqc_nim_dev, UINT8 k22_mode)
#endif
{
    UINT8 cmd[3];

    cmd[0] = 0xe0;

    cmd[1] = ADDR_POLAR_POSITIONER;
    cmd[2] = CMD_SET_EAST_LIMIT;

    return di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 3);
}


/*****************************************************************************
* INT32  api_diseqc_disable_limits(void)
*
* Disable the west and east limits
*
* Arguments:
*  none
*
* Return Value: INT32
*****************************************************************************/
#ifndef NEW_DISEQC_LIB
INT32 api_diseqc_disable_limits(struct nim_device *lib_diseqc_nim_dev)
#else
INT32 api_diseqc_disable_limits(struct nim_device *lib_diseqc_nim_dev, UINT8 k22_mode)
#endif
{
    UINT8 cmd[3];

    cmd[0] = 0xe0;

    cmd[1] = ADDR_POLAR_POSITIONER;
    cmd[2] = CMD_DISABLE_LIMITS;

    return di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 3);
}

/*****************************************************************************
* INT32  api_diseqc_enable_limits(void)
*
* enable the west and east limits
*
* Arguments:
*  none
*
* Return Value: INT32
*****************************************************************************/
#ifndef NEW_DISEQC_LIB
INT32 api_diseqc_enable_limits(struct nim_device *lib_diseqc_nim_dev)
#else
INT32 api_diseqc_enable_limits(struct nim_device *lib_diseqc_nim_dev, UINT8 k22_mode)
#endif
{
    UINT8 cmd[4];

    cmd[0] = 0xe0;
    cmd[1] = ADDR_POLAR_POSITIONER;
    cmd[2] = CMD_STORE_SATELLITE_POSITION;
    cmd[3]=00;

    return di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 4);
}

/*****************************************************************************
* INT32  api_diseqc_goto_reference(void)
*
* goto  the reference position
*
* Arguments:
*  none
*
* Return Value: INT32
*****************************************************************************/
#ifndef NEW_DISEQC_LIB
INT32 api_diseqc_goto_reference(struct nim_device *lib_diseqc_nim_dev)
#else
INT32 api_diseqc_goto_reference(struct nim_device *lib_diseqc_nim_dev, UINT8 k22_mode)
#endif
{
    UINT8 cmd[4];

    cmd[0] = 0xe0;
    cmd[1] = ADDR_POLAR_POSITIONER;
    cmd[2] = CMD_DRIVEMOTOR_GOTO_POSITION;
    cmd[3]=00;

    return di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 4);
}


/***************************************
* INT32  api_diseqc_goto_angle(UINT16 wxyz)
*
**************************************/
#ifndef NEW_DISEQC_LIB
INT32 api_diseqc_goto_angle(struct nim_device *lib_diseqc_nim_dev, UINT16 wxyz)
#else
INT32 api_diseqc_goto_angle(struct nim_device *lib_diseqc_nim_dev, UINT16 wxyz, UINT8 k22_mode)
#endif
{
    UINT8 cmd[5];

    cmd[0] = 0xe0;
    cmd[1] = ADDR_POLAR_POSITIONER;
    cmd[2] = CMD_DRIVEMOTOR_GOTO_ANGULAR;

    if (wxyz == 0)
        {
            cmd[3] = 0xe0;
            cmd[4] = 0x00;
        }
    else
        {
            cmd[3] = (wxyz>>8) & 0xff;
            cmd[4] = wxyz & 0xff;
        }

    return di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 5);
}

#ifndef NEW_DISEQC_LIB
INT32  api_diseqc_write_channel_frequency(struct nim_device *lib_diseqc_nim_dev, double frequency)
#else
INT32  api_diseqc_write_channel_frequency(struct nim_device *lib_diseqc_nim_dev, double frequency, UINT8 k22_mode)
#endif
{
    float temp;
    UINT32  freq;
    UINT8 cmd[6];

    cmd[0] = 0xe0;

    cmd[1] = ADDR_SUBSCRIBER_HEADENDS;
    cmd[2] = CMD_WRITE_CHANNEL_FREQUENCY;

    temp = (float)(frequency*10000);
    freq = (UINT32)temp;

    cmd[3] = (freq/100000)<<4;
    freq -= 100000*(cmd[3]>>4);

    cmd[3] |= (freq/10000);
    freq -= 10000*(cmd[3]&0x0f);

    cmd[4] = (freq/1000)<<4;
    freq -= 1000*(cmd[4]>>4);

    cmd[4] |= (freq/100);
    freq -= 100*(cmd[4]&0x0f);

    cmd[5] = (freq/10)<<4;
    freq -= 10*(cmd[5]>>4);

    cmd[5] |= freq;

    return di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 6);
}

#ifndef NEW_DISEQC_LIB
INT8 api_diseqc_calculate_satellite_position(struct nim_device *lib_diseqc_nim_dev, UINT8 satellite_number, UINT8 x, UINT8 y)
#else
INT8 api_diseqc_calculate_satellite_position(struct nim_device *lib_diseqc_nim_dev, UINT8 satellite_number, UINT8 x, UINT8 y, UINT8 k22_mode)
#endif
{
	UINT8 cmd[7];

    cmd[0] = 0xe0;

    cmd[1] = ADDR_POLAR_POSITIONER;
    cmd[2] = CMD_SET_POSITIONS;
    cmd[3] = satellite_number;
    cmd[4] = x;
    cmd[6] = y;

    //for diseqc1.2 recalculation function,Humax box use 4 bytes cmd,cmd[3] is s_node.postion number,
    //this is not same with Spec., but indeed work.So we follow this way.
    //if (satellite_number==0)
        return di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 4);
    //else
    //    return nim_DiSEqC_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 6);

}



#if 0

/*****************************************************************************
* INT32  api_diseqc_write_channel_frequency(double frequency)
*
* Write the channel frequency
*
* Arguments:
*     Parameter1: double frequency
*
* Return Value: INT32
*****************************************************************************/
INT32  api_diseqc_write_channel_frequency(struct nim_device *lib_diseqc_nim_dev, double frequency, UINT8 k22_mode)
{
    float temp;
    UINT32  freq;
    UINT8 cmd[6];
    INT32 ret;
/*
    if(k22_mode==1)
    {
        api_diseqc_set_22k(lib_diseqc_nim_dev,0);
        osal_task_sleep(20);
    }
*/
    cmd[0] = 0xe0;

    cmd[1] = ADDR_SUBSCRIBER_HEADENDS;
    cmd[2] = CMD_WRITE_CHANNEL_FREQUENCY;

    temp = (float)(frequency*10000);
    freq = (UINT32)temp;

    cmd[3] = (freq/100000)<<4;
    freq -= 100000*(cmd[3]>>4);

    cmd[3] |= (freq/10000);
    freq -= 10000*(cmd[3]&0x0f);

    cmd[4] = (freq/1000)<<4;
    freq -= 1000*(cmd[4]>>4);

    cmd[4] |= (freq/100);
    freq -= 100*(cmd[4]&0x0f);

    cmd[5] = (freq/10)<<4;
    freq -= 10*(cmd[5]>>4);

    cmd[5] |= freq;

    ret = nim_di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 6);
/*
    if(k22_mode==1)
    {
        osal_task_sleep(50);
        api_diseqc_set_22k(lib_diseqc_nim_dev,1);

    }
*/
    return ret;
}

/*****************************************************************************
* INT8  api_diseqc_calculate_satellite_position()
*
* Recalculate the satellite position
*
* Arguments:
*     uint8  satellite number
*    uint8  x ( site longitude)
*    uint8  y ( site latitude )
*
* Return Value: INT8
*****************************************************************************/




INT32 api_diseqc_goto_angle(struct nim_device *lib_diseqc_nim_dev, UINT16 wxyz, UINT8 k22_mode)
{
    UINT8 cmd[5];
    INT32 ret;
/*
    if(k22_mode==1)
    {
        api_diseqc_set_22k(lib_diseqc_nim_dev,0);
        osal_task_sleep(20);
    }
*/
    cmd[0] = 0xe0;
    cmd[1] = ADDR_POLAR_POSITIONER;
    cmd[2] = CMD_DRIVEMOTOR_GOTO_ANGULAR;

    if (wxyz == 0)
        {
            cmd[3] = 0xe0;
            cmd[4] = 0x00;
        }
    else
        {
            cmd[3] = (wxyz>>8) & 0xff;
            cmd[4] = wxyz & 0xff;
        }

    ret = nim_di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 5);
/*
    if(k22_mode==1)
    {
        osal_task_sleep(50);
        api_diseqc_set_22k(lib_diseqc_nim_dev,1);

    }
*/
    return ret;
}


INT32 api_diseqc_goto_reference(struct nim_device *lib_diseqc_nim_dev, UINT8 k22_mode)
{
    UINT8 cmd[4];
    INT32 ret;
/*
    if(k22_mode==1)
    {
        api_diseqc_set_22k(lib_diseqc_nim_dev,0);
        osal_task_sleep(20);
    }
*/
    cmd[0] = 0xe0;
    cmd[1] = ADDR_POLAR_POSITIONER;
    cmd[2] = CMD_DRIVEMOTOR_GOTO_POSITION;
    cmd[3]=00;

    ret = nim_di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 4);
/*
    if(k22_mode==1)
    {
        osal_task_sleep(50);
        api_diseqc_set_22k(lib_diseqc_nim_dev,1);

    }
*/
    return ret;
}

INT32 api_diseqc_enable_limits(struct nim_device *lib_diseqc_nim_dev, UINT8 k22_mode)
{
    UINT8 cmd[4];
    INT32 ret;
/*
    if(k22_mode==1)
    {
        api_diseqc_set_22k(lib_diseqc_nim_dev,0);
        osal_task_sleep(20);
    }
*/
    cmd[0] = 0xe0;
    cmd[1] = ADDR_POLAR_POSITIONER;
    cmd[2] = CMD_STORE_SATELLITE_POSITION;
    cmd[3]=00;

    ret = nim_di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 4);
/*
    if(k22_mode==1)
    {
        osal_task_sleep(50);
        api_diseqc_set_22k(lib_diseqc_nim_dev,1);

    }
*/
    return ret;
}


INT32 api_diseqc_disable_limits(struct nim_device *lib_diseqc_nim_dev, UINT8 k22_mode)
{
    UINT8 cmd[3];
    INT32 ret;
/*
    if(k22_mode==1)
    {
        api_diseqc_set_22k(lib_diseqc_nim_dev,0);
        osal_task_sleep(20);
    }
*/
    cmd[0] = 0xe0;

    cmd[1] = ADDR_POLAR_POSITIONER;
    cmd[2] = CMD_DISABLE_LIMITS;

    ret = nim_di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 3);
/*
    if(k22_mode==1)
    {
        osal_task_sleep(50);
        api_diseqc_set_22k(lib_diseqc_nim_dev,1);

    }
*/
    return ret;
}


INT32 api_diseqc_set_east_limit(struct nim_device *lib_diseqc_nim_dev, UINT8 k22_mode)
{
    UINT8 cmd[3];
    INT32 ret;

/*
    if(k22_mode==1)
    {
        api_diseqc_set_22k(lib_diseqc_nim_dev,0);
        osal_task_sleep(20);
    }
*/
    cmd[0] = 0xe0;

    cmd[1] = ADDR_POLAR_POSITIONER;
    cmd[2] = CMD_SET_EAST_LIMIT;

    ret = nim_di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 3);
/*
    if(k22_mode==1)
    {
        osal_task_sleep(50);
        api_diseqc_set_22k(lib_diseqc_nim_dev,1);

    }
*/
    return ret;
}


INT32 api_diseqc_set_west_limit(struct nim_device *lib_diseqc_nim_dev, UINT8 k22_mode)
{
    UINT8 cmd[3];
    INT32 ret;
/*
    if(k22_mode==1)
    {
        api_diseqc_set_22k(lib_diseqc_nim_dev,0);
        osal_task_sleep(20);
    }
*/
    cmd[0] = 0xe0;

    cmd[1] = ADDR_POLAR_POSITIONER;
    cmd[2] = CMD_SET_WEST_LIMIT;

    ret = nim_di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 3);
/*
    if(k22_mode==1)
    {
        osal_task_sleep(50);
        api_diseqc_set_22k(lib_diseqc_nim_dev,1);

    }
*/
    return ret;
}

INT32 api_diseqc_drive_motor_east_by_steps(struct nim_device *lib_diseqc_nim_dev, INT8 steps, UINT8 k22_mode)
{
    UINT8 cmd[4];
    INT32 ret;
/*
    if(k22_mode==1)
    {
        api_diseqc_set_22k(lib_diseqc_nim_dev,0);
        osal_task_sleep(20);
    }
*/
    cmd[0] = 0xe0;

    cmd[1] = ADDR_POLAR_POSITIONER;
    cmd[2] = CMD_DRIVE_MOTOR_EAST;
    cmd[3] = 0xff-steps;

    ret = nim_di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 4);
/*
    if(k22_mode==1)
    {
        osal_task_sleep(50);
        api_diseqc_set_22k(lib_diseqc_nim_dev,1);

    }
*/
    return ret;
}



INT32 api_diseqc_drive_motor_east_with_timeout(struct nim_device *lib_diseqc_nim_dev, INT8 timeout, UINT8 k22_mode)
{
    UINT8 cmd[4];
    INT32 ret;
/*
    if(k22_mode==1)
    {
        api_diseqc_set_22k(lib_diseqc_nim_dev,0);
        osal_task_sleep(20);
    }
*/
    cmd[0] = 0xe0;

    cmd[1] = ADDR_POLAR_POSITIONER;
    cmd[2] = CMD_DRIVE_MOTOR_EAST;
    cmd[3] = timeout;

    ret = nim_di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 4);
/*
    if(k22_mode==1)
    {
        osal_task_sleep(50);
        api_diseqc_set_22k(lib_diseqc_nim_dev,1);

    }
*/
    return ret;
}


INT32 api_diseqc_drive_motor_east(struct nim_device *lib_diseqc_nim_dev, UINT8 k22_mode)
{
    UINT8 cmd[4];
    INT32 ret;
/*
    if(k22_mode==1)
    {
        api_diseqc_set_22k(lib_diseqc_nim_dev,0);
        osal_task_sleep(20);
    }
*/
    cmd[0] = 0xe0;

    cmd[1] = ADDR_POLAR_POSITIONER;
    cmd[2] = CMD_DRIVE_MOTOR_EAST;
    cmd[3] = 0;
    ret = nim_di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 4);
/*
    if(k22_mode==1)
    {
        osal_task_sleep(50);
        api_diseqc_set_22k(lib_diseqc_nim_dev,1);

    }
*/
    return ret;
}


INT32 api_diseqc_drive_motor_west_by_steps(struct nim_device *lib_diseqc_nim_dev, INT8 steps, UINT8 k22_mode)
{
    UINT8 cmd[4];
    INT32 ret;
/*
    if(k22_mode==1)
    {
        api_diseqc_set_22k(lib_diseqc_nim_dev,0);
        osal_task_sleep(20);
    }
*/    
    cmd[0] = 0xe0;

    cmd[1] = ADDR_POLAR_POSITIONER;
    cmd[2] = CMD_DRIVE_MOTOR_WEST;
    cmd[3] = 0xff-steps;

    ret = nim_di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 4);
/*
    if(k22_mode==1)
    {
        osal_task_sleep(50);
        api_diseqc_set_22k(lib_diseqc_nim_dev,1);

    }
*/
    return ret;
}

INT32 api_diseqc_drive_motor_west_with_timeout(struct nim_device *lib_diseqc_nim_dev, INT8 timeout, UINT8 k22_mode)
{
    UINT8 cmd[4];
    INT32 ret;
/*
    if(k22_mode==1)
    {
        api_diseqc_set_22k(lib_diseqc_nim_dev,0);
        osal_task_sleep(20);
    }
*/
    cmd[0] = 0xe0;

    cmd[1] = ADDR_POLAR_POSITIONER;
    cmd[2] = CMD_DRIVE_MOTOR_WEST;
    cmd[3] = timeout;

    ret = nim_di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 4);
/*
    if(k22_mode==1)
    {
        osal_task_sleep(50);
        api_diseqc_set_22k(lib_diseqc_nim_dev,1);

    }
*/
    return ret;

}

INT32 api_diseqc_drive_motor_west(struct nim_device *lib_diseqc_nim_dev, UINT8 k22_mode)
{
    UINT8 cmd[4];
    INT32 ret;
/*
    if(k22_mode==1)
    {
        api_diseqc_set_22k(lib_diseqc_nim_dev,0);
        osal_task_sleep(20);
    }
*/
    cmd[0] = 0xe0;

    cmd[1] = ADDR_POLAR_POSITIONER;
    cmd[2] = CMD_DRIVE_MOTOR_WEST;
    cmd[3] = 0;
    ret = nim_di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 4);
/*
    if(k22_mode==1)
    {
        osal_task_sleep(50);
        api_diseqc_set_22k(lib_diseqc_nim_dev,1);

    }
*/
    return ret;
}



INT32 api_diseqc_halt_motor(struct nim_device *lib_diseqc_nim_dev, UINT8 k22_mode)
{
    UINT8 cmd[3];
    INT32 ret;
/*
    if(k22_mode==1)
    {
        api_diseqc_set_22k(lib_diseqc_nim_dev,0);
        osal_task_sleep(20);
    }
*/

    cmd[0] = 0xe0;

    cmd[1] = ADDR_POLAR_POSITIONER;
    cmd[2] = CMD_HALT_MOTOR;

    ret = nim_di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 3);
/*
    if(k22_mode==1)
    {
        osal_task_sleep(50);
        api_diseqc_set_22k(lib_diseqc_nim_dev,1);

    }
*/
    return ret;
}



INT32 api_diseqc_store_satellite_position(struct nim_device *lib_diseqc_nim_dev, UINT8 nn, UINT8 k22_mode)
{
    UINT8 cmd[4];
    INT32 ret;
/*
    if(k22_mode==1)
    {
        api_diseqc_set_22k(lib_diseqc_nim_dev,0);
        osal_task_sleep(20);
    }
*/
    cmd[0] = 0xe0;

    cmd[1] = ADDR_POLAR_POSITIONER;
    cmd[2] = CMD_STORE_SATELLITE_POSITION;
    cmd[3] = nn;

    ret= nim_di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 4);
/*
    if(k22_mode==1)
    {
        osal_task_sleep(50);
        api_diseqc_set_22k(lib_diseqc_nim_dev,1);

    }
*/
    return ret;
}


INT32 api_diseqc_goto_satellite_position(struct nim_device *lib_diseqc_nim_dev, UINT8 nn, UINT8 k22_mode)
{
    UINT8 cmd[4];
    INT32 ret;
/*
    if(k22_mode==1)
    {
        api_diseqc_set_22k(lib_diseqc_nim_dev,0);
        osal_task_sleep(20);
    }
*/
    cmd[0] = 0xe0;

    cmd[1] = ADDR_POLAR_POSITIONER;
    cmd[2] = CMD_DRIVEMOTOR_GOTO_POSITION;
    cmd[3] = nn;

    ret = nim_di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 4);
/*
    if(k22_mode==1)
    {
        osal_task_sleep(50);
        api_diseqc_set_22k(lib_diseqc_nim_dev,1);

    }
*/
    return ret;

}


INT8 api_diseqc_calculate_satellite_position(struct nim_device *lib_diseqc_nim_dev, UINT8 satellite_number, UINT8 x, UINT8 y, UINT8 k22_mode)
{
    UINT8 cmd[6];
    INT32 ret;
/*
    if(k22_mode==1)
    {
        api_diseqc_set_22k(lib_diseqc_nim_dev,0);
        osal_task_sleep(20);
    }
*/
    cmd[0] = 0xe0;

    cmd[1] = ADDR_POLAR_POSITIONER;
    cmd[2] = CMD_SET_POSITIONS;
    cmd[3] = satellite_number;
    cmd[4] = x;
    cmd[6] = y;

    if (satellite_number==0)
        ret = nim_di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 4);
    else
        ret = nim_di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 6);
/*
    if(k22_mode==1)
    {
        osal_task_sleep(50);
        api_diseqc_set_22k(lib_diseqc_nim_dev,1);

    }
*/
    return ret;

}
#endif


#endif
/*****************************************************************************
* INT32  api_diseqc_operation(struct nim_device *lib_diseqc_nim_dev, struct t_diseqc_info *d_info, UINT32 f_set))
*
* Operate the Diseqc devices
*
* Arguments:
*
* Return Value: INT32
*****************************************************************************/

#ifndef NEW_DISEQC_LIB
INT32  api_diseqc_operation(struct nim_device *lib_diseqc_nim_dev,\
                                struct t_diseqc_info *d_info, UINT32 f_set)
{
    UINT8 flag;
    INT32 ret=SUCCESS;
    UINT8 polar_flag = 0;

    if(f_set == FORCE_ALL) //change all para to force re-command.
        {
            lib_diseqc_nim_dev->diseqc_info.diseqc_type = 0;
            lib_diseqc_nim_dev->diseqc_info.diseqc_toneburst = 0;
            lib_diseqc_nim_dev->diseqc_info.positioner_type = 0;
            lib_diseqc_nim_dev->diseqc_info.position = 0;
            lib_diseqc_nim_dev->diseqc_info.wxyz = 0;
        }
    PRINTF("\ncome into api_diseqc_operation(),f_set = %d!!!!\n",f_set);

#if(SYS_LNB_POWER_OFF == SYS_FUNC_ON)
    //->if lnb power off, just call power-off function, other function are ignored!
    if(LNB_POL_POFF == d_info->diseqc_polar)
        {
            api_diseqc_lnb_power(lib_diseqc_nim_dev,LNB_PW_OFF);
            lib_diseqc_nim_dev->diseqc_info.diseqc_polar = LNB_POL_POFF;
            return ret;
        }
    else if(LNB_POL_POFF == lib_diseqc_nim_dev->diseqc_info.diseqc_polar)
        {
            api_diseqc_lnb_power(lib_diseqc_nim_dev,LNB_PW_ON);
            lib_diseqc_nim_dev->diseqc_info.diseqc_type = 0;
            lib_diseqc_nim_dev->diseqc_info.diseqc_toneburst = 0;
            lib_diseqc_nim_dev->diseqc_info.positioner_type = 0;
            lib_diseqc_nim_dev->diseqc_info.position = 0;
            lib_diseqc_nim_dev->diseqc_info.wxyz = 0;
        }
    //<-lnb power judgement!
#endif
    if((lib_diseqc_nim_dev->diseqc_info.diseqc_type != d_info->diseqc_type) \
        || (lib_diseqc_nim_dev->diseqc_info.diseqc_port != d_info->diseqc_port) \
        || (d_info->sat_or_tp == 0))
        {
        // 1st step stop 22k tone.
        //    api_diseqc_set_22k(lib_diseqc_nim_dev,0);

        //then change voltage.
#if(DISEQC_WITH_13V_HELP != V13_HELP_ON)
            if((d_info->diseqc_polar != 0) && \
                (lib_diseqc_nim_dev->diseqc_info.diseqc_polar != d_info->diseqc_polar))
                {
                    api_diseqc_set_polar(lib_diseqc_nim_dev, d_info->diseqc_polar-1);
                    osal_task_sleep(100);
                }
#else
    //=>> removed because of its temperal use.
        //force voltage to 13v,if new value is auto, hold the original value for later use.
            api_diseqc_set_polar(lib_diseqc_nim_dev, 1);
            osal_task_sleep(100);
    //<<= end of removed.
#endif

        //full diseqc message, here for switch work.
        if((d_info->diseqc_polar != 0) && \
            ((lib_diseqc_nim_dev->diseqc_info.diseqc_type != d_info->diseqc_type) \
            || (lib_diseqc_nim_dev->diseqc_info.diseqc_port != d_info->diseqc_port)))
            {
                if(d_info->diseqc_type != 0)
                    if(lib_diseqc_nim_dev->diseqc_info.diseqc_k22==1)
                    {
                        api_diseqc_set_22k(lib_diseqc_nim_dev,0);
                        DISEQC_PRINTF("stop 22k!\n");
                        osal_task_sleep(20);
                    }

                switch(d_info->diseqc_type)
                {
                    case 0:
                        break;
                    case 1:    //the para for 2 port is same with 4 port.
                    case 2:
                        //to support SMATV,polar H control bit '1';polar V control bit '0'
                        //polar H
                        if(d_info->diseqc_polar==1)
                            polar_flag = 1;
                        //polar V
                        else if(d_info->diseqc_polar==2)
                            polar_flag = 0;
                        flag = d_info->diseqc_port * 4 + polar_flag* 2 + d_info->diseqc_k22;
                        ret = api_diseqc_16port_switch(lib_diseqc_nim_dev,flag);
                        //again, 100ms delay between diseqc message and 22k tone.
                        osal_task_sleep(100);
                PRINTF("\ndeal with api_diseqc_4port_switch()!!!!\n");

                        break;
                    case 3:    //the para for 8 port is same with 16 port.
                    case 4:
                        ret = api_diseqc_16port_switch(lib_diseqc_nim_dev,d_info->diseqc_port);
                    //again, 100ms delay between diseqc message and 22k tone.
                        osal_task_sleep(100);
                PRINTF("\ndeal with api_diseqc_16port_switch()!!!!\n");

                        break;
                    default:
                        break;
                }
        //save diseqc state parametres.
            lib_diseqc_nim_dev->diseqc_info.diseqc_type = d_info->diseqc_type;
            lib_diseqc_nim_dev->diseqc_info.diseqc_port = d_info->diseqc_port;
            }
        //full diseqc command, here for positioner
        #if(LIB_DISEQC_HIGH_FUNC_SUPPORT)
            if(d_info->sat_or_tp == 0)
            {
                if(d_info->positioner_type != 0)
                    if(lib_diseqc_nim_dev->diseqc_info.diseqc_k22==1)
                    {
                        api_diseqc_set_22k(lib_diseqc_nim_dev,0);
                        DISEQC_PRINTF("stop 22k!\n");
                        osal_task_sleep(20);
                    }

                switch(d_info->positioner_type)
                {
                    case 0:
                        break;
                    case 1:
                        if((d_info->position != 0))
                        //    (lib_diseqc_nim_dev->diseqc_info.position != d_info->position))
                            {
                                api_diseqc_goto_satellite_position(lib_diseqc_nim_dev, d_info->position);
                                osal_task_sleep(100);
                                lib_diseqc_nim_dev->diseqc_info.position = d_info->position;
                            }
                        break;
                    case 2:
                        if(0xFFFF != d_info->wxyz)
                            {
                                api_diseqc_goto_angle(lib_diseqc_nim_dev, d_info->wxyz);
                                osal_task_sleep(100);
                                lib_diseqc_nim_dev->diseqc_info.wxyz = d_info->wxyz;

                            }
                        break;
                    default:
                        break;
                }
            }
        #endif
        //set tone burst.
            if(d_info->diseqc_toneburst != 0)
                {
                    if(lib_diseqc_nim_dev->diseqc_info.diseqc_k22==1)
                    {
                        api_diseqc_set_22k(lib_diseqc_nim_dev,0);
                        DISEQC_PRINTF("stop 22k!\n");
                        osal_task_sleep(20);
                    }

                    api_diseqc_set_tone_burst(lib_diseqc_nim_dev, d_info->diseqc_toneburst-1);
                    lib_diseqc_nim_dev->diseqc_info.diseqc_toneburst = d_info->diseqc_toneburst;
                    osal_task_sleep(20);
                }
        //set 22k continuous tone.
            //if(d_info->diseqc_k22==1)
                api_diseqc_set_22k(lib_diseqc_nim_dev, d_info->diseqc_k22);


#if(DISEQC_WITH_13V_HELP == V13_HELP_ON)
    //=>> removed because of its temperal use.
        //set true voltage level.
            if(d_info->diseqc_polar == 0)
                api_diseqc_set_polar(lib_diseqc_nim_dev, lib_diseqc_nim_dev->diseqc_info.diseqc_polar-1);
            else
                {
                    api_diseqc_set_polar(lib_diseqc_nim_dev, d_info->diseqc_polar-1);
                    lib_diseqc_nim_dev->diseqc_info.diseqc_polar = d_info->diseqc_polar;
                }
    //<<= end of removed.
#endif
        }
    else if((d_info->diseqc_toneburst != 0) && \
            (lib_diseqc_nim_dev->diseqc_info.diseqc_toneburst != d_info->diseqc_toneburst))
            {
        // 1st step stop 22k tone.
            if(lib_diseqc_nim_dev->diseqc_info.diseqc_k22==1)
                api_diseqc_set_22k(lib_diseqc_nim_dev,0);

        //then change voltage.
#if(DISEQC_WITH_13V_HELP != V13_HELP_ON)
            if((d_info->diseqc_polar != 0) && \
                (lib_diseqc_nim_dev->diseqc_info.diseqc_polar != d_info->diseqc_polar))
                {
                    api_diseqc_set_polar(lib_diseqc_nim_dev, d_info->diseqc_polar-1);
                    lib_diseqc_nim_dev->diseqc_info.diseqc_polar = d_info->diseqc_polar;
                }
#else
    //=>> removed because of its temperal use.
        //force voltage to 13v,if new value is auto, hold the original value for later use.
            api_diseqc_set_polar(lib_diseqc_nim_dev, 1);
    //<<= end of removed.
#endif

        //each time do a 20ms delay.
            osal_task_sleep(20);

        //set tone burst.
            api_diseqc_set_tone_burst(lib_diseqc_nim_dev, d_info->diseqc_toneburst-1);
            lib_diseqc_nim_dev->diseqc_info.diseqc_toneburst = d_info->diseqc_toneburst;
            osal_task_sleep(20);

        //set 22k continuous tone.
            //if(d_info->diseqc_k22==1)
                api_diseqc_set_22k(lib_diseqc_nim_dev, d_info->diseqc_k22);


#if(DISEQC_WITH_13V_HELP == V13_HELP_ON)
    //=>> removed because of its temperal use.
        //set true voltage level.
            if(d_info->diseqc_polar == 0)
                api_diseqc_set_polar(lib_diseqc_nim_dev, lib_diseqc_nim_dev->diseqc_info.diseqc_polar-1);
            else
                {
                    api_diseqc_set_polar(lib_diseqc_nim_dev, d_info->diseqc_polar-1);

                }
    //<<= end of removed.
#endif
        }
    else
        {
        //change voltage.
            if((d_info->diseqc_polar != 0) && \
                (lib_diseqc_nim_dev->diseqc_info.diseqc_polar != d_info->diseqc_polar))
                {
                    api_diseqc_set_polar(lib_diseqc_nim_dev, d_info->diseqc_polar-1);

                }
        //set 22k continuous tone
            //if(lib_diseqc_nim_dev->diseqc_info.diseqc_k22 != d_info->diseqc_k22)
            {
                api_diseqc_set_22k(lib_diseqc_nim_dev, d_info->diseqc_k22);

            }
        }
    PRINTF("\nleave api_diseqc_operation()!!!!\n");

    return ret;

}

#else
INT32  api_diseqc_operation(struct nim_device *dev,struct t_diseqc_info *d_info, UINT32 f_set)
{
    UINT8 flag;
    INT32 ret=SUCCESS;
    UINT8 polar_flag = 0;
    UINT8 reply_cnt = 0;
    UINT8 diseqc_cmd[6];
    UINT8 diseqc_reply[6];
	//UINT8 i;

    if (dev == NULL)
        return RET_SUCCESS;

    if(f_set == FORCE_ALL) //change all para to force re-command.
    {
        dev->diseqc_info.diseqc_type = 0;
        dev->diseqc_info.diseqc11_type = 0;
        dev->diseqc_info.diseqc_toneburst = 0;
        dev->diseqc_info.positioner_type = 0;
        dev->diseqc_info.position = 0;
        dev->diseqc_info.wxyz = 0;
        dev->diseqc_info.diseqc_polar = 0;
    }
    DISEQC_PRINTF("\ncome into api_diseqc_operation(),f_set = %d!!!!\n",f_set);

#if (SYS_CHIP_MODULE!=ALI_S3602)
 #if(SYS_LNB_POWER_OFF == SYS_FUNC_ON)
      //->if lnb power off, just call power-off function, other function are ignored!
      if(LNB_POL_POFF == d_info->diseqc_polar)
    {
        api_diseqc_lnb_power(dev,LNB_PW_OFF);
        dev->diseqc_info.diseqc_polar = LNB_POL_POFF;
        return ret;
    }
     else if(LNB_POL_POFF == dev->diseqc_info.diseqc_polar)
    {
        api_diseqc_lnb_power(dev,LNB_PW_ON);
        dev->diseqc_info.diseqc_type = 0;
        dev->diseqc_info.diseqc11_type = 0;
        dev->diseqc_info.diseqc_toneburst = 0;
        dev->diseqc_info.positioner_type = 0;
        dev->diseqc_info.position = 0;
        dev->diseqc_info.wxyz = 0;

    }
      //<-lnb power judgement!
  #endif
#endif
        //change voltage.    /* need a parameter to identify Unicable ,do not need change voltage */
        if((d_info->diseqc_polar != 0)&&(dev->diseqc_info.diseqc_polar!=d_info->diseqc_polar))
        {
            DISEQC_PRINTF("change polar from %d to %d!\n",dev->diseqc_info.diseqc_polar,d_info->diseqc_polar);
            api_diseqc_set_polar(dev, d_info->diseqc_polar-1);
            osal_task_sleep(POLAR_CHANGE_DELAY_TIME);
        }

        MEMSET(diseqc_cmd, 0, 6);
        diseqc_cmd[0] = 0xE0;
        MEMSET(diseqc_reply, 0, 6);
        //set diseqcx.0 port
        if(/*(d_info->diseqc_polar != 0) &&*/ (d_info->diseqc_type != 0)&&\
            ((dev->diseqc_info.diseqc_type != d_info->diseqc_type) \
            || (dev->diseqc_info.diseqc_port != d_info->diseqc_port)))
         {
            if(dev->diseqc_info.diseqc_k22==1)
            {
                api_diseqc_set_22k(dev,0);
                DISEQC_PRINTF("stop 22k!\n");
                osal_task_sleep(20);
            }
            //polar H
            if(d_info->diseqc_polar==1)
                polar_flag = 1;
            //polar V
            else if(d_info->diseqc_polar==2)
                polar_flag = 0;
            //auto, no polar
            else if(d_info->diseqc_polar==0)
                polar_flag = 1;
            switch(d_info->diseqc_type)
            {

                case 1:    //the 1.0 para for 2 and 4 port. oxf for auto diseqc function
                case 2:
                case 0xf:
                    flag = d_info->diseqc_port * 4 + polar_flag* 2 + d_info->diseqc_k22;
                    ret = diseqc_switch(dev, 0x10, 1, diseqc_cmd, flag, diseqc_reply, &reply_cnt);
                    DISEQC_PRINTF("diseqc1.0 type=%d, port =%d,pol=%d, 22k=%d\n",d_info->diseqc_type, d_info->diseqc_port, polar_flag,d_info->diseqc_k22);

                    break;
                case 3:    //the 1.0 para for 8 and 16 port.
                case 4:
                    ret = diseqc_switch(dev, 0x10, 1, diseqc_cmd, d_info->diseqc_port, diseqc_reply, &reply_cnt);
                    DISEQC_PRINTF("diseqc1.0 type=%d, port=%d\n",d_info->diseqc_type,d_info->diseqc_port);

                    break;
                case 5://diseqc2.0
                #if (LIB_DISEQC_HIGH_FUNC_SUPPORT)
                    flag = d_info->diseqc_port * 4 + polar_flag* 2 + d_info->diseqc_k22;
                    diseqc_cmd[0] = 0xE2;
                    ret = diseqc2x_switch(dev, ADDR_SWITCH,1,diseqc_cmd, flag, diseqc_reply, &reply_cnt);
                    if(ret != SUCCESS)
                    {
                        diseqc_cmd[0] = 0xE2;
                        ret = diseqc2x_switch(dev, ADDR_SWITCH_WITH_LOOPTHROUGH,1,diseqc_cmd, flag, diseqc_reply, &reply_cnt);
                    }
                    if(ret != SUCCESS)
                        return ret;
                #endif
                    break;
            }
            //for cascaded, 200ms delay for the cascaded device power up
            if(d_info->diseqc11_type != 0 || d_info->positioner_type != 0)
                osal_task_sleep(CASCADE_DELAY_TIME);
            //else
            //    osal_task_sleep(50);
            dev->diseqc_info.diseqc_type = d_info->diseqc_type;
              dev->diseqc_info.diseqc_port = d_info->diseqc_port;
        }

        #if(LIB_DISEQC_HIGH_FUNC_SUPPORT)
        //set diseqc1.1 port
        if(/*(d_info->diseqc_polar != 0) &&*/ (d_info->diseqc11_type != 0) && \
            ((dev->diseqc_info.diseqc11_type != d_info->diseqc11_type) \
            || (dev->diseqc_info.diseqc11_port != d_info->diseqc11_port)))
         {
            if(dev->diseqc_info.diseqc_k22==1)
            {
                api_diseqc_set_22k(dev,0);
                DISEQC_PRINTF("stop 22k!\n");
                osal_task_sleep(20);
            }
            switch(d_info->diseqc11_type)
            {
                case 1:    //for 1 Cascade
                case 2:    //for 2 Cascade
                    if (d_info->diseqc11_type == 1)
                    {
                        /* get 1 Cascade Mode */
                        flag = (d_info->diseqc11_port & 0x0C) >> 2;
                        if (flag == 3)
                            flag = 0;
                        /* convert to DiSEqC 1.1 command */
                        flag = (d_info->diseqc11_port & 3) << flag;
                    }
                    else
                    {
                        flag = d_info->diseqc11_port & 0x0F;
                        ret = diseqc_switch(dev, 0x10, 0, diseqc_cmd, flag, diseqc_reply, &reply_cnt);
                        osal_task_sleep(150);
                    }
                    ret = diseqc_switch(dev, 0x10, 0,diseqc_cmd, flag, diseqc_reply, &reply_cnt);;
                    DISEQC_PRINTF("diseqc1.1 type=%d, port=%d\n",d_info->diseqc11_type,d_info->diseqc11_port);

            }
            //for cascaded, 200ms delay for the cascaded device power up
            if(d_info->diseqc_type != 0 || d_info->positioner_type != 0)
                osal_task_sleep(CASCADE_DELAY_TIME);
            //else
                //osal_task_sleep(50);
            dev->diseqc_info.diseqc11_type = d_info->diseqc11_type;
            dev->diseqc_info.diseqc11_port = d_info->diseqc11_port;
        }
        //diseqc1.0 and 1.1 cascaded
        if(d_info->diseqc_type!=0 && d_info->diseqc11_type !=0)
        {
            /*if(dev->diseqc_info.diseqc_k22==1)
            {
                api_diseqc_set_22k(dev,0);
                DISEQC_PRINTF("stop 22k!\n");
                osal_task_sleep(20);
            }*/
            DISEQC_PRINTF("diseqc1.0 and diseqc1.1 cascade,repeat diseqc1.0 command:\n");
            //polar H
            if(d_info->diseqc_polar==1)
                polar_flag = 1;
            //polar V
            else if(d_info->diseqc_polar==2)
                polar_flag = 0;
            //auto, no polar
            else if(d_info->diseqc_polar==0)
                polar_flag = 1;
            if(d_info->diseqc_type==1 || d_info->diseqc_type==2)
            {

                flag = d_info->diseqc_port * 4 + polar_flag* 2 + d_info->diseqc_k22;
                ret = diseqc_switch(dev, 0x10, 1, diseqc_cmd, flag, diseqc_reply, &reply_cnt);
                DISEQC_PRINTF("repeat diseqc1.0 type=%d, port flag=%d\n",d_info->diseqc_type,flag);

            }
            else if(d_info->diseqc_type==3 || d_info->diseqc_type==4)
            {
                ret = diseqc_switch(dev, 0x10, 1, diseqc_cmd, d_info->diseqc_port, diseqc_reply, &reply_cnt);
                DISEQC_PRINTF("repeat diseqc1.0 type=%d, port=%d\n",d_info->diseqc_type,d_info->diseqc_port);

            }
            else if(d_info->diseqc_type==5)//diseqc2.0 switch
            {
                flag = d_info->diseqc_port * 4 + polar_flag* 2 + d_info->diseqc_k22;
                diseqc_cmd[0] = 0xE2;
                ret = diseqc2x_switch(dev, ADDR_SWITCH,1,diseqc_cmd, flag, diseqc_reply, &reply_cnt);
                if(ret != SUCCESS)
                {
                    diseqc_cmd[0] = 0xE2;
                    ret = diseqc2x_switch(dev, ADDR_SWITCH_WITH_LOOPTHROUGH,1,diseqc_cmd, flag, diseqc_reply, &reply_cnt);
                }
                if(ret != SUCCESS)
                    return ret;
            }
            if(d_info->positioner_type != 0)
                osal_task_sleep(CASCADE_DELAY_TIME);
            //else
                //osal_task_sleep(50);
        }
        #endif


        //full diseqc command, here for positioner
        #if(LIB_DISEQC_HIGH_FUNC_SUPPORT)
            if((d_info->sat_or_tp==0) &&(d_info->positioner_type != 0))
            {

                switch(d_info->positioner_type)
                {
                    case 1://diseqc1.2
                        if((d_info->position != 0))
                        //    (lib_diseqc_nim_dev->diseqc_info.position != d_info->position))
                        {
                            api_diseqc_goto_satellite_position(dev, d_info->position,0);
                            DISEQC_PRINTF("set diseqc1.2 goto sat position=%d\n",d_info->position);
                            //osal_task_sleep(20);
                            dev->diseqc_info.position = d_info->position;
                        }
                        break;
                    case 2://usals
                        if(0xFFFF != d_info->wxyz)
                        {
                            api_diseqc_goto_angle(dev, d_info->wxyz,0);
                            DISEQC_PRINTF("set usal goto sat angle=%d\n",d_info->wxyz);
                            //osal_task_sleep(20);
                            dev->diseqc_info.wxyz = d_info->wxyz;

                        }
                        break;
                }
            }
        #endif
            //set tone burst.
            if(d_info->diseqc_toneburst != 0 && d_info->diseqc_toneburst != dev->diseqc_info.diseqc_toneburst)
            {
                if(dev->diseqc_info.diseqc_k22==1)
                {
                    api_diseqc_set_22k(dev,0);
                    DISEQC_PRINTF("stop 22k!\n");
                    osal_task_sleep(20);
                }

                api_diseqc_set_tone_burst(dev, d_info->diseqc_toneburst-1);
                dev->diseqc_info.diseqc_toneburst = d_info->diseqc_toneburst;
                osal_task_sleep(20);
                DISEQC_PRINTF("set toneburst=%d\n",d_info->diseqc_toneburst-1);
            }
            //set 22k continuous tone.
            //if(d_info->diseqc_k22==1)
            {
                api_diseqc_set_22k(dev, d_info->diseqc_k22);
                // for multi-switcher delay
                osal_task_sleep(50);
                DISEQC_PRINTF("set 22k = %d\n",d_info->diseqc_k22);
                }

    DISEQC_PRINTF("exit api_diseqc_operation()!!!!\n");
    return ret;

}



#endif

/****************************************************************************
 *
 *    The following will be supported in future DiSEqC Version above2.0
 *
 ****************************************************************************/

#ifdef DISEQC2X_TEST

INT32 diseqc2x_read_slave_config(struct nim_device *dev, UINT8 slave_addr,UINT8 *reply, UINT8 *reply_cnt)
{
    UINT8 cmd[3];
    INT32 ret = SUCCESS;

    cmd[0] = 0xE2;
    cmd[1] = slave_addr;
    cmd[2] = CMD_READ_CONFIG;// CMD_SELECT_LO;//CMD_DISABLE_LIMITS

    ret = di_seq_c2x_operate(dev,NIM_DISEQC_MODE_BYTES,cmd,3,reply,reply_cnt);
    DISEQC2_PRINTF("read slave 0x%x config, reply_cnt=%d, reply[0]=%x, reply[1]=%x\n",slave_addr, *reply_cnt,reply[0],reply[1]);
    return ret;
}

INT32 diseqc2x_read_slave_status(struct nim_device *dev, UINT8 slave_addr,UINT8 *reply, UINT8 *reply_cnt)
{
    UINT8 cmd[3];
    INT32 ret = SUCCESS;

    cmd[0] = 0xE2;
    cmd[1] = slave_addr;
    cmd[2] = CMD_READ_STATUS;//CMD_SLEEP;//

    ret = di_seq_c2x_operate(dev,NIM_DISEQC_MODE_BYTES,cmd,3,reply,reply_cnt);
    DISEQC2_PRINTF("read slave 0x%x status, reply_cnt=%d, reply[0]=%x, reply[1]=%x\n",slave_addr,*reply_cnt,reply[0],reply[1]);
    return ret;
}

INT32 diseqc2x_read_switch(struct nim_device *dev, UINT8 slave_addr,UINT8 committed_switch,UINT8 *reply, UINT8 *reply_cnt)
{
    UINT8 cmd[3];
    INT32 ret = SUCCESS;

    cmd[0] = 0xE2;
    cmd[1] = slave_addr;//ADDR_SWITCH;
    if(committed_switch == 1)
        cmd[2] = CMD_READ_SWITCH_COMMI_PORT;
    else
        cmd[2] = CMD_READ_SWITCH_UNCOMMI_PORT;

    ret = di_seq_c2x_operate(dev,NIM_DISEQC_MODE_BYTES,cmd,3,reply,reply_cnt);
    DISEQC2_PRINTF("read switch register, reply_cnt=%d, reply[0]=%x,reply[1]=%x\n",*reply_cnt,reply[0],reply[1]);
    return ret;
}


INT32 diseqc2x_read_addr(struct nim_device *dev,UINT8 *reply, UINT8 *reply_cnt)
{
    UINT8 cmd[3];
    INT32 ret = SUCCESS;

    cmd[0] = 0xE2;
    cmd[1] = 0x10;
    cmd[2] = CMD_READ_ADDR;

    ret = di_seq_c2x_operate(dev, NIM_DISEQC_MODE_BYTES, cmd, 3, reply, reply_cnt);
    DISEQC2_PRINTF("read slave addr, reply_cnt=%d, reply[0]=%x,reply[1]=%x\n",*reply_cnt,reply[0],reply[1]);
    return ret;

}


#endif


/*****************************************************************************
* INT32  api_diseqc_unicable_channel_change()//for Unicable
*
* Unicable ODU_Channel_change
*
* Arguments:
*  lib_diseqc_nim_dev: IN, diseqc/nim device handle
*  UB: IN, 0-7
*  Fub: IN, freq of UB
*  Position: A - 0
*            B - 1
*  Polarization: Vertical   - 0
*                Horizontal - 1
*  Band: Low  - 0
*        High - 1
*  Fif: IN, freq of IF
*
* Return Value: INT32
*****************************************************************************/
INT32 api_diseqc_unicable_channel_change(struct nim_device *lib_diseqc_nim_dev,
                    UINT8 UB, UINT16 fub,
                    UINT8 position, UINT8 polarization, UINT8 band,
                    UINT16 fif)
{
    INT32 ret;
    UINT8 cmd[5];

//    UINT8 UB;    //bit 2:0
//    UINT16 Fub;    //UB freq

    UINT8 bank;    //bit 2:0
//    UINT8 Position;
//    UINT8 Polarization;
//    UINT8 Band;

    UINT16 T;    //bit 9:0
    UINT8 step_size = 4;
//    UINT16 Fif;    //IF freq = | Ftransponder - Flo |
//    UINT16 Flo;    //LNB Oscillator frequency

    PRINTF("api_diseqc_odu_channel_change: UB %d, Freq(UB) %d\n", UB, fub);
    PRINTF("api_diseqc_odu_channel_change: Position %d, Polarization %d, Band %d\n", position, polarization, band);
    PRINTF("api_diseqc_odu_channel_change: Freq(IF) %d\n", fif);

    //Position: A - 0
    //            B - 1
    //Polarization: Vertical   - 0
    //                Horizontal - 1
    //Band: Low  - 0
    //        High - 1
    bank = position * 4 + polarization * 2 + band;
    PRINTF("api_diseqc_odu_channel_change: Bank[2:0] - %X\n", bank);
    T = (fif + fub) / step_size - 350;
    PRINTF("api_diseqc_odu_channel_change: T[9:0] - %X\n", T);

    cmd[0] = 0xe0;
    cmd[1] = ADDR_LNB_SWITCHER_SMATV;
    cmd[2] = CMD_UNICABLE_NORMAL_OPERATION;
    cmd[3] = (UB<<5) | (bank<<2) | (T>>8);
    cmd[4] = T & 0xFF;
    if(ALI_S3281 != sys_ic_get_chip_id())
    {
    //first set to 13V for a while
    api_diseqc_set_polar(lib_diseqc_nim_dev, 1);
//    osal_task_sleep(50);
    di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES_EXT_STEP1, cmd, 5);
    }
    //up to 18v
    api_diseqc_set_polar(lib_diseqc_nim_dev, 0);

    // send Diseqc command
    if(ALI_S3281 == sys_ic_get_chip_id())
        ret = nim_di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 5);
     else
    ret = di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES_EXT_STEP2, cmd, 5);

    //down to 13v
    api_diseqc_set_polar(lib_diseqc_nim_dev, 1);

    return ret;
}

INT32 api_diseqc_unicable_odu_signal_on(struct nim_device *lib_diseqc_nim_dev)
{
    INT32 ret;
    UINT8 cmd[5];

    cmd[0] = 0xe0;
    cmd[1] = ADDR_LNB_SWITCHER_SMATV;
    cmd[2] = CMD_UNICABLE_SPECIAL_OPERATION;
    cmd[3] = 0x00;
    cmd[4] = 0x00;

    //up to 18v
//    api_diseqc_lnb_power(NULL,LNB_PW_OFF);
    api_diseqc_set_polar(lib_diseqc_nim_dev, 0);
//     api_diseqc_lnb_power(NULL,LNB_PW_ON);
//  osal_task_sleep(250);

    // send Diseqc command
    ret = di_seq_c_operate(lib_diseqc_nim_dev, NIM_DISEQC_MODE_BYTES, cmd, 5);

    //down to 13v
    api_diseqc_set_polar(lib_diseqc_nim_dev, 1);

    return ret;
}

#if 0
#define AGC_SIGNAL_LEVEL 150
#define MAX_SIGNAL_NUM  32
#define MAX_SLOT_NUM   8
static INT32 api_diseqc_unicale_autodetect(struct nim_device *lib_diseqc_nim_dev,UINT8 step,UINT8* slot_num,UINT32* channel_freq)
{
    struct NIM_CHANNEL_CHANGE channel_param;
    UINT32 freq;
    UINT8 agc[1201];//950~2150
    UINT32 i,last_pos;
    UINT8 agc_max;
    UINT8 agc_trh;
    UINT32 unicable_freq[MAX_SLOT_NUM];
    UINT8 find_signal,signal_counter;
    UINT32 start_pos[MAX_SIGNAL_NUM],end_pos[MAX_SIGNAL_NUM];
    struct nim_m3327_private * priv = (struct nim_m3327_private*)lib_diseqc_nim_dev->priv ;
    INT32 result;

    api_diseqc_unicable_odu_signal_on(lib_diseqc_nim_dev);
//    priv = (struct nim_m3327_private*)nim_dev->priv;
    agc_max = 0;
    if(0 == step)  step = 1;
    for(i=0;(freq = 950+i*step)<=2150;i++)
    {
        channel_param.freq = freq;
        channel_param.sym = 45000;
        channel_param.fec = 0;
        if(0==i)
             nim_ioctl_ext(lib_diseqc_nim_dev, NIM_DRIVER_CHANNEL_CHANGE, (void *)( &channel_param));
        lib_diseqc_nim_dev->priv->nim_tuner_control(priv->tuner_id, channel_param.freq, channel_param.sym);
        osal_delay(10000);
        nim_get_agc(lib_diseqc_nim_dev,&agc[i]);
        if(agc[i] > agc_max)
            agc_max = agc[i];
        DISEQC_PRINTF("freq is %d,AGC is %d\n",channel_param.freq,agc[i]);
    }
    libc_printf("agc_max is %d\n",agc_max);
    if(agc_max < AGC_SIGNAL_LEVEL)
        return FALSE;
    libc_printf("analyse......\n");

    agc_trh = agc_max*4/5;
//    if(agc_trh < AGC_SIGNAL_LEVEL)
//        agc_trh = AGC_SIGNAL_LEVEL;
    libc_printf("agc_trh is %d\n",agc_trh);

    last_pos = i;

    find_signal = 0;
    signal_counter = 0;

    for(i=0; i<last_pos; i++)
    {
        if(agc[i] >= agc_trh)
        {
            if(find_signal ==0)
            {
                find_signal = 1;
                if(signal_counter>=MAX_SIGNAL_NUM)
                    return FALSE;
                start_pos[signal_counter] = i;
            }
        }
        if((agc[i] < agc_trh ) && (find_signal == 1))
        {
            if(signal_counter>=MAX_SIGNAL_NUM)
                return FALSE;
            end_pos[signal_counter]=i-1;
            signal_counter++;
            find_signal = 0;
        }
    }
    if(signal_counter > MAX_SLOT_NUM)
        signal_counter = MAX_SLOT_NUM;
    *slot_num = signal_counter;
    for(i=0;i<signal_counter;i++)
    {
        channel_freq[i]=950+(start_pos[i]+end_pos[i])*step/2;
        libc_printf("unicable_freq[%d]= %d\n",i,unicable_freq[i]);
    }

    return TRUE;
}
#endif
