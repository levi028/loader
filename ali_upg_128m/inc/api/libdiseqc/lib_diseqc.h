/*****************************************************************************
*    Copyright (C)2004 Ali Corporation. All Rights Reserved.
*
*    File:    lib_diseqc.h
*
*    Description:    This file contains head file for DiSEqC library.
*    History:
*           Date            Athor        Version          Reason
*        ============    =============    =========    =================
*    1.    Jan.8.2004          Leo Guo      Ver 1.0       Create file.
*    2.
*****************************************************************************/


#ifndef    __LIB_DISEQC_H__
#define __LIB_DISEQC_H__

#include <types.h>
#include <retcode.h>
#include <hld/nim/nim_dev.h>



/*force set port switch or not*/
#define FORCE_NONE                        0
#define FORCE_ALL                        1

/*Slave devices address definitions*/
#define ADDR_ALL_DEVICES                        0x00
#define ADDR_LNB_SWITCHER_SMATV                0x10
#define ADDR_LNB                                0x11
#define ADDR_LNB_WITH_LOOPTHROUGH_SWITCH    0x12
#define ADDR_SWITCH                            0x14
#define ADDR_SWITCH_WITH_LOOPTHROUGH        0x15
#define ADDR_SMATV                                0x18

#define    ADDR_ANY_POLARISER                    0x20
#define ADDR_LINEAR_POLARISATION_CONTROLLER    0x21

#define ADDR_ANY_POSITIONER                    0x30
#define ADDR_POLAR_POSITIONER                    0x31
#define ADDR_ELEVATION_POSITIONER                0x32

#define ADDR_ANY_INSTALLER_AID                    0x40
#define ADDR_SIGNAL_VALUE                        0x41

#define ADDR_ANY_INTELLIGENT_SLAVE            0x70
#define ADDR_SUBSCRIBER_HEADENDS                0x71

/*Commands definitions*/
#define CMD_DISEQC_RESET                0x00
#define CMD_CLEAR_RESET_FLAG            0x01
#define CMD_DISEQC_STANDBY            0x02
#define CMD_DISEQC_POWERON            0x03
#define CMD_SET_CONTENTION_FLAG        0x04
#define CMD_GET_CONTEND                0x05
#define CMD_CLEAR_CONTENTION_FLAG    0x06

#define CMD_READ_ADDR        0x07
#define CMD_READ_CONFIG    0x11
#define CMD_READ_STATUS    0x10
#define CMD_READ_SWITCH_COMMI_PORT        0x14
#define CMD_READ_SWITCH_UNCOMMI_PORT    0x15


/*CMDS 0x06-0x19*/
#define CMD_SELECT_LO                    0x20
#define CMD_SELECT_VR                    0x21
#define CMD_SELECT_POSITION_A            0x22
#define CMD_SELECT_SOA                    0x23

#define CMD_SELECT_HI                    0x24
#define CMD_SELECT_HL                    0x25
#define CMD_SELECT_POSITION_B            0x26
#define CMD_SELECT_SOB                    0x27

#define CMD_SELECT_S1A                    0x28
#define CMD_SELECT_S2A                    0x29
#define CMD_SELECT_S3A                    0x2A
#define CMD_SELECT_S4A                    0x2B

#define CMD_SELECT_S1B                    0x2C
#define CMD_SELECT_S2B                    0x2D
#define CMD_SELECT_S3B                    0x2E
#define CMD_SELECT_S4B                    0x2F

#define CMD_SLEEP                        0x30
#define CMD_AWAKE                        0x31

#define CMD_WRITE_PORT0                0x38
#define CMD_WRITE_PORT1                0x39

#define CMD_WRITE_CHANNEL_FREQUENCY    0x58
#define CMD_WRITE_SELECT_CH            0x59

#define CMD_UNICABLE_NORMAL_OPERATION    0x5A
#define CMD_UNICABLE_SPECIAL_OPERATION    0x5B

#define CMD_HALT_MOTOR                0x60
#define CMD_DISABLE_LIMITS                0x63

#define CMD_SET_EAST_LIMIT                0x66
#define CMD_SET_WEST_LIMIT                0x67

#define CMD_DRIVE_MOTOR_EAST            0x68
#define CMD_DRIVE_MOTOR_WEST            0x69

#define CMD_STORE_SATELLITE_POSITION    0x6A
#define CMD_DRIVEMOTOR_GOTO_POSITION    0x6B

#define CMD_DRIVEMOTOR_GOTO_ANGULAR    0x6E
#define CMD_SET_POSITIONS                0x6F

/*diseqc2.x return value*/
#define diseqc2x_err_no_reply            -1
#define diseqc2x_err_cmd_nsupport        -2
#define diseqc2x_err_cmd_parity        -3
#define diseqc2x_err_cmd_nrecognize    -4
/*error in revieved reply bytes*/
#define diseqc2x_err_reply_parity        -5
#define diseqc2x_err_reply_unknown    -6
#define diseqc2x_err_reply_buf_ful        -7
/*reply from slave*/
#define SLAVE_REPLY_OK            0xE4
#define SLAVE_CMD_NSUPPORT    0xE5
#define SLAVE_CMD_PARITY        0xE6
#define SLAVE_CMD_NRECOGNIZE    0xE7



//Lnb Polarity defines
#define    LNB_POL_AUTO      0x00     /* LNB POL AUTO */
#define    LNB_POL_H         0x01     /* LNB POL H       */
#define    LNB_POL_V         0x02     /* LNB POL V       */
#define    LNB_POL_POFF     0x03     /* LNB POWER OFF  */

//Lnb Power on/off defines
#ifdef    LNB_POL_REVERSE
#define    LNB_PW_ON          0x01     /* LNB power on*/
#define    LNB_PW_OFF     0x00      /* LNB power off */
#else
#define    LNB_PW_ON          0x00     /* LNB power on*/
#define    LNB_PW_OFF     0x01      /* LNB power off */
#endif

/*DISEqC1.1 cascaded */
enum
{
    DISEQC11_TYPE_DISABLE       = 0, /* disable(not set)*/
    DISEQC11_TYPE_1CASCADE      = 1, /* 1 Cascade */
    DISEQC11_TYPE_2CASCADE      = 2, /* 2 Cascade */
    DISEQC11_TYPE_RESERVED      = 3  /* reserved */
};

enum
{
    DISEQC11_1CASCADE_MODE1     = 0x00, /* mode 1(for 1 Cascade)*/
    DISEQC11_1CASCADE_MODE2     = 0x04, /* mode 1(for 1 Cascade)*/
    DISEQC11_1CASCADE_MODE3     = 0x08, /* mode 1(for 1 Cascade)*/
    DISEQC11_1CASCADE_RESERVED  = 0x0C, /* reserved */
};

#ifdef DISEQC_SUPPORT

#ifdef __cplusplus
extern "C"
{
#endif

INT32 api_diseqc_set_polar(struct nim_device *lib_diseqc_nim_dev, UINT8 polar);
INT32 api_diseqc_set_tone_burst(struct nim_device *lib_diseqc_nim_dev, UINT8 mode);
INT32 api_diseqc_set_22k_envelop(struct nim_device *lib_diseqc_nim_dev, UINT8 mode);
INT32 api_diseqc_set_22k(struct nim_device *lib_diseqc_nim_dev, UINT8 flag);
INT32 api_diseqc_set_12v(struct nim_device *lib_diseqc_nim_dev, UINT8 flag);
INT32  api_diseqc_16port_switch(struct nim_device *lib_diseqc_nim_dev, UINT8 flag);

INT32 api_diseqc_select_lo(struct nim_device *lib_diseqc_nim_dev);
INT32 api_diseqc_select_hi(struct nim_device *lib_diseqc_nim_dev);
INT32 api_diseqc_select_vr(struct nim_device *lib_diseqc_nim_dev);
INT32 api_diseqc_select_hl(struct nim_device *lib_diseqc_nim_dev);
INT32 api_diseqc_select_switch_option_a(struct nim_device *lib_diseqc_nim_dev);
INT32 api_diseqc_select_switch_option_b(struct nim_device *lib_diseqc_nim_dev);
INT32 api_diseqc_select_satellite_position_a(struct nim_device *lib_diseqc_nim_dev);
INT32 api_diseqc_select_satellite_position_b(struct nim_device *lib_diseqc_nim_dev);
INT32 api_diseqc_select_s1a(struct nim_device *lib_diseqc_nim_dev);
INT32 api_diseqc_select_s2a(struct nim_device *lib_diseqc_nim_dev);
INT32 api_diseqc_select_s3a(struct nim_device *lib_diseqc_nim_dev);
INT32 api_diseqc_select_s4a(struct nim_device *lib_diseqc_nim_dev);
INT32 api_diseqc_select_s1b(struct nim_device *lib_diseqc_nim_dev);
INT32 api_diseqc_select_s2b(struct nim_device *lib_diseqc_nim_dev);
INT32 api_diseqc_select_s3b(struct nim_device *lib_diseqc_nim_dev);
INT32 api_diseqc_select_s4b(struct nim_device *lib_diseqc_nim_dev);


#ifndef NEW_DISEQC_LIB
INT32 api_diseqc_goto_satellite_position(struct nim_device *lib_diseqc_nim_dev, UINT8 nn);
INT32 api_diseqc_store_satellite_position(struct nim_device *lib_diseqc_nim_dev, UINT8 nn);
INT32 api_diseqc_halt_motor(struct nim_device *lib_diseqc_nim_dev);
INT32 api_diseqc_drive_motor_west(struct nim_device *lib_diseqc_nim_dev);
INT32 api_diseqc_drive_motor_west_with_timeout(struct nim_device *lib_diseqc_nim_dev, INT8 timeout);
INT32 api_diseqc_drive_motor_west_by_steps(struct nim_device *lib_diseqc_nim_dev, INT8 steps);
INT32 api_diseqc_drive_motor_east(struct nim_device *lib_diseqc_nim_dev);
INT32 api_diseqc_drive_motor_east_with_timeout(struct nim_device *lib_diseqc_nim_dev, INT8 timeout);
INT32 api_diseqc_drive_motor_east_by_steps(struct nim_device *lib_diseqc_nim_dev, INT8 steps);
INT32 api_diseqc_set_west_limit(struct nim_device *lib_diseqc_nim_dev);
INT32 api_diseqc_set_east_limit(struct nim_device *lib_diseqc_nim_dev);
INT32 api_diseqc_disable_limits(struct nim_device *lib_diseqc_nim_dev);
INT32  api_diseqc_write_channel_frequency(struct nim_device *lib_diseqc_nim_dev, double frequency);
INT32 api_diseqc_goto_angle(struct nim_device *lib_diseqc_nim_dev, UINT16 wxyz);
INT32 api_diseqc_goto_reference(struct nim_device *lib_diseqc_nim_dev);
INT8 api_diseqc_calculate_satellite_position(struct nim_device *lib_diseqc_nim_dev, UINT8 satellite_number, UINT8 x, UINT8 y);
#else
INT32 api_diseqc_goto_satellite_position(struct nim_device *lib_diseqc_nim_dev, UINT8 nn, UINT8 k22_mode);
INT32 api_diseqc_store_satellite_position(struct nim_device *lib_diseqc_nim_dev, UINT8 nn, UINT8 k22_mode);
INT32 api_diseqc_halt_motor(struct nim_device *lib_diseqc_nim_dev, UINT8 k22_mode);
INT32 api_diseqc_drive_motor_west(struct nim_device *lib_diseqc_nim_dev, UINT8 k22_mode);
INT32 api_diseqc_drive_motor_west_with_timeout(struct nim_device *lib_diseqc_nim_dev, INT8 timeout, UINT8 k22_mode);
INT32 api_diseqc_drive_motor_west_by_steps(struct nim_device *lib_diseqc_nim_dev, INT8 steps, UINT8 k22_mode);
INT32 api_diseqc_drive_motor_east(struct nim_device *lib_diseqc_nim_dev, UINT8 k22_mode);
INT32 api_diseqc_drive_motor_east_with_timeout(struct nim_device *lib_diseqc_nim_dev, INT8 timeout, UINT8 k22_mode);
INT32 api_diseqc_drive_motor_east_by_steps(struct nim_device *lib_diseqc_nim_dev, INT8 steps, UINT8 k22_mode);
INT32 api_diseqc_set_west_limit(struct nim_device *lib_diseqc_nim_dev, UINT8 k22_mode);
INT32 api_diseqc_set_east_limit(struct nim_device *lib_diseqc_nim_dev, UINT8 k22_mode);
INT32 api_diseqc_disable_limits(struct nim_device *lib_diseqc_nim_dev, UINT8 k22_mode);
INT32  api_diseqc_write_channel_frequency(struct nim_device *lib_diseqc_nim_dev, double frequency,UINT8 k22_mode);
INT32 api_diseqc_goto_angle(struct nim_device *lib_diseqc_nim_dev, UINT16 wxyz,UINT8 k22_mode);
INT32 api_diseqc_goto_reference(struct nim_device *lib_diseqc_nim_dev, UINT8 k22_mode);
INT8 api_diseqc_calculate_satellite_position(struct nim_device *lib_diseqc_nim_dev, UINT8 satellite_number, UINT8 x, UINT8 y, UINT8 k22_mode);
#endif


double api_usals_get_sat_orbit(UINT32 orbit);
double api_usals_target_formula_calcuate(double sat_orbit, double local_longitude, double local_latitude);
UINT16 api_usals_xx_to_wxyz(double degree);

#if(SYS_LNB_POWER_OFF == SYS_FUNC_ON)
void api_diseqc_lnb_power(struct nim_device *lib_diseqc_nim_dev, UINT8 sw);
#endif
INT32 api_diseqc_operation(struct nim_device *dev,struct t_diseqc_info *d_info, UINT32 f_set);

#ifdef DISEQC2X_TEST
INT32 diseqc2x_read_slave_config(struct nim_device *dev, UINT8 slave_addr,UINT8 *reply, UINT8 *reply_cnt);
INT32 diseqc2x_read_slave_status(struct nim_device *dev, UINT8 slave_addr,UINT8 *reply, UINT8 *reply_cnt);
INT32 diseqc2x_read_switch(struct nim_device *dev, UINT8 slave_addr,UINT8 committed_switch,
    UINT8 *reply, UINT8 *reply_cnt);
INT32 diseqc2x_read_addr(struct nim_device *dev,UINT8 *reply, UINT8 *reply_cnt);
INT32 diseqc2x_switch(struct nim_device *dev, UINT8 addr,UINT8 committed,UINT8 *cmd, UINT8 port,
    UINT8 *reply, UINT8 *reply_cnt);
#endif
INT32 api_diseqc_unicable_channel_change(struct nim_device *lib_diseqc_nim_dev,
                    UINT8 UB, UINT16 fub,
                    UINT8 position, UINT8 polarization, UINT8 band,
                    UINT16 fif);

#ifdef __cplusplus
}
#endif


#else /*DISEQC_SUPPORT*/
#define api_diseqc_set_22k(...)         do{}while(0)
#define api_diseqc_operation(...)       do{}while(0)
#define api_diseqc_unicable_channel_change(...)       do{}while(0)

#define api_usals_get_sat_orbit(...)    (0)
#define api_usals_target_formula_calcuate(...)  (0)
#define api_usals_xx_to_wxyz(...)       (0)



#endif/*DISEQC_SUPPORT*/

#endif  /* __LIB_DISEQC_H__ */
