#ifndef __SYS_H__
#define __SYS_H__
//======================================================================================================================//
//#define PMU_MCU_DEBUG
//#define PMU_MCU_M3503
//#define PMU_MCU_M3505
//#define PMU_MCU_M3821
//#define PMU_MCU_M3921
//#define PMU_MCU_M3922
//#define PMU_MCU_M3702
#define PMU_MCU_M3711C
//#define M3526_IC_SUPPORT
//#define PMU_MCU_M3503C
//#define PMU_MCU_M3711C
//#define PMU_MCU_M3503D
//#define PMU_MCU_3821B

//#define GPIO_POWER_KEY_SUPPORT
/*Add for support HW scan panel.*/
//#define HW_SCAN_PANEL_ENABLE
//#define IR_SHARE_WITH_PANEL_ENABLE/*Reserved for special case: ir share pin with panel power key.*/
//#define SHOW_PANEL_DIS
//#define FD650_PAN_SUPPORT/*This macro default is closed for ALi, because CH455 panel is default solution.*/
/*If you want to use cec pin as gpio, please close maro----HDMI_CEC_SUPPORT.*/
#if ((!defined PMU_MCU_M3503C) && (!defined M3526_IC_SUPPORT) && (!defined PMU_MCU_3821B))
//#define HDMI_CEC_SUPPORT
#endif

#define MAILBOX_GET_EXIT_STANDBY_STATUS2                           (0x3FE9)
#define MAILBOX_GET_EXIT_STANDBY_STATUS1                           (0x3FE8)
#define MAILBOX_GET_EXIT_STANDBY_STATUS0                           (0x3FE7)
#define PMUSRAM_GET_YEAR_H      (0x3FE6)
#define PMUSRAM_GET_YEAR_L      (0x3FE5)
#define PMUSRAM_GET_MONTH       (0x3FE4)
#define PMUSRAM_GET_DAY         (0x3FE3)
#define PMUSRAM_GET_HOUR        (0x3FE2)
#define PMUSRAM_GET_MIN         (0x3FE1)
#define PMUSRAM_GET_SEC         (0x3FE0)

#define MAILBOX_WAKE_MONTH      (0xC23F)
#define MAILBOX_WAKE_DAY        (0xC23E)
#define MAILBOX_WAKE_HOUR       (0xC23D)
#define MAILBOX_WAKE_MIN        (0xC23C)
#define MAILBOX_WAKE_SECOND     (0xC23B)

#define MAILBOX_SET_YEAR_H      (0xC23A)
#define MAILBOX_SET_YEAR_L      (0xC239)
#define MAILBOX_SET_MONTH       (0xC238)
#define MAILBOX_SET_DAY         (0xC237)
#define MAILBOX_SET_HOUR        (0xC236)
#define MAILBOX_SET_MIN         (0xC235)
#define MAILBOX_SET_SEC         (0xC234)
#define MAILBOX_SET_POWERLOW3   (0xC233)
#define MAILBOX_SET_POWERLOW2   (0xC232)
#define MAILBOX_SET_POWERLOW1   (0xC231)
#define MAILBOX_SET_POWERLOW0   (0xC230)

#define SYS_IOBASE              (0xC000)
#define SYS_REG_ISR             (SYS_IOBASE+0x20)
#define SYS_REG_IPR             (SYS_IOBASE+0x21)
#define SYS_REG_IER             (SYS_IOBASE+0x22)
#define SYS_REG_RST             (SYS_IOBASE+0x90)
#define CPU_TO_MCU_INT_CLEAR_REG  (0xC220)
#define CLEAR_MAILBOX3          (0x8)
#define CLEAR_MAILBOX2          (0x4)
#define CLEAR_MAILBOX1          (0x2)
#define CLEAR_MAILBOX0          (0x1)
#define ASCII_P                 (0x50)
#define ASCII_M                 (0x4D)
#define ASCII_U                 (0x55)

/*for gpio.*/
#define HAL_GPIO1_DO_REG             (0xC05D)/*output register.*/
#define HAL_GPIO1_DI_REG              (0xC05C)/*Input Status Register.*/
#define HAL_GPIO1_DIR_REG            (0xC05E)/*Output control register.*/
#define HAL_GPIO1_EN                     (0xC05F)/*Output control register.*/
#define HAL_GPIO_DIR_REG              (0xC056)/*Output control register.*/
#define HAL_GPIO_DI_REG                (0xC054)/*Input Status Register.*/
#define HAL_GPIO_DO_REG               (0xC055)/*output register.*/
#define HAL_GPIO_EN                       (0xC057)/*GPIO ENABLE.*/
#define HAL_GPIO_O_DIR                  (1)
#define HAL_GPIO_I_DIR                   (0)
#define HAL_GPIO_SET_HI                (1)
#define HAL_GPIO_SER_LOW             (0)
#define S3821B_PANEL_KEY_PRESS (0)

#ifdef  PMU_MCU_M3503
#define XPMU_GPIO_0     (0)
#define XPMU_GPIO_1     (1)
#define XPMU_CEC        (3)
#elif ((defined PMU_MCU_M3503C) || (defined PMU_MCU_M3503D))
#define XPMU_GPIO_0     (0)
#define XPMU_GPIO_1     (1)
#define XPMU1_CEC       (10)
#elif (defined PMU_MCU_M3711C)
#define XPMU_GPIO_0     (0)
#define XPMU_GPIO_1     (1)
#define XPMU1_CEC       (10)
#elif ((defined PMU_MCU_M3505) || (defined PMU_MCU_M3702) || (defined PMU_MCU_M3922))
#define XPMU_GPIO_0     (0)
#define XPMU_GPIO_1     (1)
#define XPMU_CEC        (3)
#define XPMU1_CEC       (10)
#elif (defined PMU_MCU_M3921)
#define XPMU_GPIO_0     (0)
#define XPMU_GPIO_1     (1)
#define XPMU_GPIO_7     (7)
#define XPMU_GPIO_8     (8)
#elif (defined PMU_MCU_M3821)
/*for gpio0.*/
#define XPMU_GPIO_0     (0)
#define XPMU_GPIO_1     (1)
#define XPMU_GPIO_2     (2)
#define XPMU_GPIO_3     (3)
#define XPMU_GPIO_4     (4)
/*for gpio1.*/
#define XPMU_GPIO1_0    (8)
#define XPMU1_CEC       (10)
#else
/*for gpio0.*/
#define XPMU_GPIO_0     (0)
#define XPMU_GPIO_1     (1)
#define XPMU_GPIO_2     (2)
#define XPMU_GPIO_3     (3)
#define XPMU_GPIO_4     (4)
/*for gpio1.*/
#define XPMU_GPIO1_0    (8)
#define XPMU1_CEC       (10)
#endif

#ifdef __SDCC__
#define NOP __asm nop __endasm
#define PMU_WRITE_BYTE(address, value)              (*(volatile __xdata unsigned char *)(address)=value)
#define PMU_READ_BYTE(address)                      (*(volatile __xdata unsigned char *)(address))
#else
#define NOP _nop_()
#define PMU_WRITE_BYTE(address, value)              (*(volatile unsigned char xdata *)(address)=value)
#define PMU_READ_BYTE(address)                      (*(volatile unsigned char xdata *)(address))
#endif

#define SUCCESS                                     (0)
#define ERROR                                       (-1)
#define SHOW_TYPE_PAR                               (0x3FFF)
#define panel_POWER_STATUS                          (0x3FFE)
#define EXIT_STANDBY_TYPE                           (0x3FFD)
#define PANEL_POWER_KEY1_ADDR                       (0x3FFC)
#define PANEL_POWER_KEY2_ADDR                       (0x3FFB)

/*define wakeup reason.*/
#define EXIT_STANDBY_TYPE_PANEL                     (0x1)
#define EXIT_STANDBY_TYPE_IR                        (0x2)
#define EXIT_STANDBY_TYPE_RTC                       (0x3)
#define EXIT_STANDBY_TYPE_CEC                       (0x4)
#define EXIT_STANDBY_TYPE_ALI_POWER_KEY             (0x5)

#define PRS_KEY_STANDBY_LED                         (0xC101)
#define NORMAL_MODE                                 (0)
#define STANDBY_MODE                                (1)
#define PMU_TIMER0_BIT                              (1<<7)
#define TIMER0_OVERFLOW_CLEAR                       (1<<3)
#define KEY_INT_BIT                                 (1<<3)
#define KEY_PRS_IN_STANDBY_CLEAR                    (1<<5)
#define CEC_INT_BIT                                 (1<<1)
#define MCU_WORK_IN_STANDBY                         (1<<7)
#define MCU_WORK_IN_NORMAL                          (~(1<<7))
#define PRS_KEY_WAKEUP_ENABLE                       (1<<7)
#define PRS_KEY_WAKEUP_DISABLE                      (~(1<<7))
#define PRS_KEY_INT_IN_STANDBY_ENABLE               (1<<3)
#define PRS_KEY_INT_IN_STANDBY_HIGH_POLARITY        (1<<3)
#define PRS_KEY_INT_IN_NORMAL_ENABLE                (1<<6)
#define PRS_KEY_INT_IN_NORMAL_HIGH_POLARITY         (1<<6)
#define IR_INTERRUPT_DISABLE                        (~(1<<0))
#define POWER_OFF_DONE                              (0x12345678)
//======================================================================================================================//

typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned long UINT32;
typedef char INT8;
typedef short INT16;
typedef long INT32;
typedef enum
{
    BIT_ZERO = 0, BIT_ONE = 1
} UINT1;
//======================================================================================================================//

extern UINT8 g_standby_flag;
void pmu_init(void);
void rtc_init(void);
void rtc_process(void);
void hal_mcu_gpio_en(UINT8 pos);
#ifdef IR_SHARE_WITH_PANEL_ENABLE
extern void ali_power_key_process();
#endif
void hal_gpio_bit_dir_set(UINT8 pos, UINT8 val);
void hal_gpio_bit_set(UINT8 pos, UINT8 val);
UINT8 hal_gpio_bit_get(UINT8 pos);
void hal_mcu_gpio_en(UINT8 pos);
void mcu_disable_int(void);
void exit_standby_status(void);
void mcu_gpio_init(void);
void stb_power_on(void);

#endif
