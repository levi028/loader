#ifndef __RTC_H__
#define __RTC_H__
//======================================================================================================================//

#define MCU_SYS_IPR                                                         (0xC021)
#define MCU_SYS_IER                                                         (0xC022)
#define MCU_SYS_INT_STS                                                  (0xC024)
#define MCU_SYS_IPR1                                                       (0xC025)
#define MCU_SYS_IER1                                                       (0xC026)

#define PMU_RTC_IOBASE                                                   (0xE600)
#define PMU_RTC0_CNT_LOW0                                            (PMU_RTC_IOBASE+0x00)
#define PMU_RTC0_CNT_LOW1                                            (PMU_RTC_IOBASE+0x01)
#define PMU_RTC0_CNT_LOW2                                            (PMU_RTC_IOBASE+0x02)
#define PMU_RTC0_CNT_LOW3                                            (PMU_RTC_IOBASE+0x03)
#define PMU_RTC0_CNT_HIGH0                                           (PMU_RTC_IOBASE+0x04)
#define PMU_RTC0_CNT_HIGH1                                           (PMU_RTC_IOBASE+0x05)
#define PMU_RTC0_CTRL                                                     (PMU_RTC_IOBASE+0x08)
#define MCU_CLK0_DIV                                                       (PMU_RTC_IOBASE+0x20)
#define MCU_CLK0_EN                                                        (1)

#define PMU_RTC1_CNT_LOW0                                            (PMU_RTC_IOBASE+0x10)
#define PMU_RTC1_CNT_LOW1                                            (PMU_RTC_IOBASE+0x11)
#define PMU_RTC1_CNT_LOW2                                            (PMU_RTC_IOBASE+0x12)
#define PMU_RTC1_CNT_LOW3                                            (PMU_RTC_IOBASE+0x13)
#define PMU_RTC1_CTRL                                                     (PMU_RTC_IOBASE+0x18)
#define PMU_RTC1_AUTO_LOAD                                           (PMU_RTC_IOBASE+0x1C)
#define MCU_CLK1_DIV                                                       (PMU_RTC_IOBASE+0x30)
#define MCU_CLK1_EN                                                        (1)

#define PMU_TM0_AIM_SEC0                                               (PMU_RTC_IOBASE+0x00)
#define PMU_TM0_AIM_SEC1                                               (PMU_RTC_IOBASE+0x01)
#define PMU_TM0_AIM_SEC2                                               (PMU_RTC_IOBASE+0x02)
#define PMU_TM0_AIM_SEC3                                               (PMU_RTC_IOBASE+0x03)
#define PMU_TM0_AIM_MS0                                                (PMU_RTC_IOBASE+0x04)
#define PMU_TM0_AIM_MS1                                                (PMU_RTC_IOBASE+0x05)
#define PMU_TM0_TICK_LOW0                                              (PMU_RTC_IOBASE+0x06)
#define PMU_TM0_TICK_LOW1                                              (PMU_RTC_IOBASE+0x07)
#define PMU_TM0_CUR_MS0                                                (PMU_RTC_IOBASE+0x0A)
#define PMU_TM0_CUR_MS1                                                (PMU_RTC_IOBASE+0x0B)
#define PMU_TM0_CUR_SEC0                                               (PMU_RTC_IOBASE+0x0C)
#define PMU_TM0_CUR_SEC1                                               (PMU_RTC_IOBASE+0x0D)
#define PMU_TM0_CUR_SEC2                                               (PMU_RTC_IOBASE+0x0E)
#define PMU_TM0_CUR_SEC3                                               (PMU_RTC_IOBASE+0x0F)
#define PMU_TM2_AIM_SEC0                                               (PMU_RTC_IOBASE+0x40)
#define PMU_TM2_AIM_SEC1                                               (PMU_RTC_IOBASE+0x41)
#define PMU_TM2_AIM_SEC2                                               (PMU_RTC_IOBASE+0x42)
#define PMU_TM2_AIM_SEC3                                               (PMU_RTC_IOBASE+0x43)
#define PMU_TM2_AIM_MS0                                                (PMU_RTC_IOBASE+0x44)
#define PMU_TM2_AIM_MS1                                                (PMU_RTC_IOBASE+0x45)
#define PMU_TM2_CTRL                                                       (PMU_RTC_IOBASE+0x48)
#define PMU_TM2_TICK_LOW0                                             (PMU_RTC_IOBASE+0x46)
#define PMU_TM2_TICK_LOW1                                             (PMU_RTC_IOBASE+0x47)
#define PMU_TM2_CUR_MS0                                                (PMU_RTC_IOBASE+0x4A)
#define PMU_TM2_CUR_MS1                                                (PMU_RTC_IOBASE+0x4B)
#define PMU_TM2_CUR_SEC0                                               (PMU_RTC_IOBASE+0x4C)
#define PMU_TM2_CUR_SEC1                                               (PMU_RTC_IOBASE+0x4D)
#define PMU_TM2_CUR_SEC2                                               (PMU_RTC_IOBASE+0x4E)
#define PMU_TM2_CUR_SEC3                                               (PMU_RTC_IOBASE+0x4F)

#define PMU_BASE                                                              (0xE000)
#define PMU_CFG                                                               (0x00+PMU_BASE)
#define KEY_PRESS_TIME                                                    (50000)/*50000us = 50ms.*/
#define RC_CLK_FREQ                                                         (1.5)/*3MHz.*/
#define PRS_KEY_CFG                                                         (0x10+PMU_BASE)

#define KEY_DISABLE_VAL_LOW0                                       (0x18+PMU_BASE)
#define KEY_DISABLE_VAL_LOW1                                       (0x19+PMU_BASE)
#define KEY_DISABLE_VAL_LOW2                                       (0x1A+PMU_BASE)
#define KEY_DISABLE_EN                                                   (0x1B+PMU_BASE)
#define AUTO_LOAD_ENABLE                                              (1)
#define TIMER1_OVERFLOW_CLEAR                                     (1<<3)
#define TIMER1_INT_STATUS                                               (1)
#define TIMER1_CLK_DIV                                                    (1)
#define TIMER1_INT_ENABLE                                               (1<<4)
#define TIMER1_INT_DISABLE                                             (~(1<<4))
#define AUTO_LOAD_EN                                                      (1<<7)
//======================================================================================================================//

typedef struct
{
    unsigned char year_h;
    unsigned char year_l;
    unsigned long year;
    unsigned char month;
    unsigned char day;
    unsigned char hour;
    unsigned char min;
    unsigned char sec;
} RTC_TIMER, *pRTC_TIMER;

void rtc_init(void);
void rtc_process();
void main_cpu_get_time();

#endif
