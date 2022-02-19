#ifndef __PMU_CEC_H__
#define __PMU_CEC_H__
#include "sys.h"
//======================================================================================================================//

#define PMU_CEC_BASE_REG                              (0xE200)
#define CEC_WKUP_CMD1_REG                           (PMU_CEC_BASE_REG + 0x20)
#define CEC_WKUP_CMD2_REG                           (PMU_CEC_BASE_REG + 0x30)
#define CEC_WKUP_CMD3_REG                           (PMU_CEC_BASE_REG + 0x40)
#define CEC_CMD_RX_HAND_REG                        (PMU_CEC_BASE_REG + 0x50)
#define CEC_CMD_TX_HAND_REG                        (PMU_CEC_BASE_REG + 0x60)
#define CEC_EOM_1_REG                                    (PMU_CEC_BASE_REG + 0x70)
#define CEC_EOM_2_REG                                    (PMU_CEC_BASE_REG + 0x71)
#define LOGIC_ADDR_REG                                   (PMU_CEC_BASE_REG + 0x72)
#define DIVIDER_REG                                         (PMU_CEC_BASE_REG + 0x73)
#define FRQ_RES_REG                                        (PMU_CEC_BASE_REG + 0x74)

/*
 one_until_time must be 50us   
 50=one_until_time = 1/pmu_clk*cec_div*(cec_freq+1)
 so:
.cec_div   =  0x31,
.cec_freq  =  0x1a,
.pmu_clk   =  0x1b,
 */
#define CEC_DIV                                                (0x02)
#define CEC_FREQ                                              (0x18)
#define CEC_SYS_INT_EN                                    (1<<1)
#define LOGIC_ADDR                                           (0x03)/*for STB PORT.*/
#define CEC_WKUP_GRP_ZIZE                              (0x03)
#define CEC_CMD_SZIE_BYTE                              (0x10)
#define CEC_RX_SIZE                                          (0x10)
#define CEC_TX_SIZE                                          (0x10)
#define CEC_REST_BIT                                        (1<<1)
#define CEC_IPR_BIT                                          (1<<1)

#define CEC_SYS_INT_ENABLE()                          PMU_WRITE_BYTE(SYS_REG_IER, (PMU_READ_BYTE(SYS_REG_IER) | CEC_SYS_INT_EN))
#define CEC_SET_DIV(val)                                   PMU_WRITE_BYTE(DIVIDER_REG, val)
#define CEC_SET_FRQ(val)                                  PMU_WRITE_BYTE(FRQ_RES_REG, val)
#define CEC_SET_LOGADD(val)                            PMU_WRITE_BYTE(LOGIC_ADDR_REG, val)
#define CEC_SYS_INT_IPR()                                PMU_WRITE_BYTE(SYS_REG_IPR, (PMU_READ_BYTE(SYS_REG_IPR) | CEC_IPR_BIT))
#define CEC_GET_LOGADDR()                              PMU_READ_BYTE(LOGIC_ADDR_REG)
//======================================================================================================================//

extern void pmu_cec_init(void);
extern char get_cec_cmd(void);
extern void set_get_cec_cmd(char status);
#endif