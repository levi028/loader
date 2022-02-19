#ifdef __SDCC__
#include "dp8051xp_sdcc.h"
#else
#include <intrins.h>
#include <dp8051xp.h>
#endif
#include <stdio.h>
#include "panel.h"
#include "rtc.h"
//======================================================================================================================//

//#define EXTERNAL_PULL_HIGH                                       (TRUE)
#define SETING_ADDR                                                    (0x48)
#define DIG0_ADDR                                                        (0x68)
#define DIG1_ADDR                                                       (0x6A)
#define DIG2_ADDR                                                       (0x6C)
#define DIG3_ADDR                                                       (0x6E)
#define KEY_ADDR                                                         (0x4F)
#define ERR_I2C_SCL_LOCK                                           (1)
#define ERR_I2C_SDA_LOCK                                           (1)
#define I2C_GPIO_TIMES_OUT                                        (10)
#define ERR_TIME_OUT                                                  (-34)
#define ERR_FAILURE                                                    (-9)

#if ((defined PMU_MCU_M3821) || (defined PMU_MCU_3821B))
#define SYS_I2C_SDA         XPMU_GPIO_1
#define SYS_I2C_SCL         XPMU_GPIO_0
#elif (defined PMU_MCU_M3503C)
#define SYS_I2C_SDA         XPMU1_CEC
#define SYS_I2C_SCL         XPMU_GPIO_0
#elif (defined PMU_MCU_M3503D)
#define SYS_I2C_SDA         XPMU_GPIO_0
#define SYS_I2C_SCL         XPMU_GPIO_1
#elif (defined PMU_MCU_M3711C)
#define SYS_I2C_SDA         XPMU_GPIO_0
#define SYS_I2C_SCL         XPMU_GPIO_1
#elif (defined PMU_MCU_M3505)
#ifndef M3526_IC_SUPPORT
#define SYS_I2C_SDA     XPMU_GPIO_1
#define SYS_I2C_SCL     XPMU_GPIO_0
#else
#define SYS_I2C_SDA     XPMU1_CEC
#define SYS_I2C_SCL     XPMU_GPIO_0
#endif
#elif ((defined PMU_MCU_M3702) || (defined PMU_MCU_M3922))
#define SYS_I2C_SDA         XPMU_GPIO_1
#define SYS_I2C_SCL         XPMU_GPIO_0
#elif (defined PMU_MCU_M3921)
#define SYS_I2C_SDA         XPMU_GPIO_0
#define SYS_I2C_SCL         XPMU_GPIO_8
#else
#define SYS_I2C_SDA         XPMU_GPIO_0
#define SYS_I2C_SCL         XPMU_GPIO_1
#endif

#ifndef PMU_MCU_DEBUG

//======================================================================================================================//

unsigned char first_scan_panel = FIRST_SCAN_GOING;
UINT8 Detected_Panel_Key1 = 0;
UINT8 Detected_Panel_Key2 = 0;
UINT8 Received_Panel_Key1 = 0;
UINT8 Received_Panel_Key2 = 0;
//======================================================================================================================//

static void set_sda_out(void)
{
    hal_gpio_bit_dir_set(SYS_I2C_SDA, HAL_GPIO_O_DIR);
}

static void set_sda_in(void)
{
    hal_gpio_bit_dir_set(SYS_I2C_SDA, HAL_GPIO_I_DIR);
}

#ifdef EXTERNAL_PULL_HIGH
static void set_sda_hi(void)
{
    set_sda_in();
}

static void set_sda_lo(void)
{
    set_sda_out();
    hal_gpio_bit_set(SYS_I2C_SDA, HAL_GPIO_SER_LOW);
}
#else
static void set_sda_hi(void)
{
    set_sda_out();
    hal_gpio_bit_set(SYS_I2C_SDA, HAL_GPIO_SET_HI);
}

static void set_sda_lo(void)
{
    set_sda_out();
    hal_gpio_bit_set(SYS_I2C_SDA, HAL_GPIO_SER_LOW);
}
#endif

static void set_scl_out(void)
{
    hal_gpio_bit_dir_set(SYS_I2C_SCL, HAL_GPIO_O_DIR);
}

#ifdef EXTERNAL_PULL_HIGH
static void set_scl_in(void)
{
    hal_gpio_bit_dir_set(SYS_I2C_SCL, HAL_GPIO_I_DIR);
}

static void set_scl_hi(void)
{
    set_scl_in();
}

static void set_scl_lo(void)
{
    set_scl_out();
    hal_gpio_bit_set(SYS_I2C_SCL, HAL_GPIO_SER_LOW);
}
#else
static void set_scl_hi(void)
{
    set_scl_out();
    hal_gpio_bit_set(SYS_I2C_SCL, HAL_GPIO_SET_HI);
}

static void set_scl_lo(void)
{
    set_scl_out();
    hal_gpio_bit_set(SYS_I2C_SCL, HAL_GPIO_SER_LOW);
}
#endif

static UINT8 get_scl(void)
{
    UINT8 ret = 0;

    ret = hal_gpio_bit_get(SYS_I2C_SCL);
    return ret;
}

static UINT8 get_sda(void)
{
    UINT8 ret = 0;

    ret = hal_gpio_bit_get(SYS_I2C_SDA);
    return ret;
}

void OneNop(void)
{
    unsigned char i = 0;

    for (i = 0; i < 60; i++)
    {
        NOP;
    }
}

/*---------------------------------------------------
 INT8 i2c_gpio_phase_start(void);
 Generate i2c_gpio_phase_start Condition:
 Stream Format:
 SCL   _____/--------\___
 SDA   =---------\_____
 width (4.7u)4.7u|4.7u|
 Arguments:
 NONE
 Return value:
 int SUCCESS				0
 int ERR_I2C_SCL_LOCK	1
 int ERR_I2C_SDA_LOCK	2
 ----------------------------------------------------*/
static INT8 i2c_gpio_phase_start(void)
{
#ifndef EXTERNAL_PULL_HIGH
    set_sda_out();
    set_scl_out();
#endif

    set_sda_hi();
    if (!get_scl())
    {
        OneNop();
    }
    set_scl_hi();
    OneNop();
    if (!get_scl())
    {
        return ERR_I2C_SCL_LOCK;
    }

    if (!get_sda())
    {
        return ERR_I2C_SDA_LOCK;
    }
    set_sda_lo();
    OneNop();
    set_scl_lo();

    return SUCCESS;
}

/*---------------------------------------------------
 INT8 i2c_gpio_phase_stop(void);
 Generate i2c_gpio_phase_stop Condition:
 Stream Format:
 SCL   _____/-------------------------------
 SDA   __________/--------------------------
 width  4.7u|4.7u|4.7u from next i2c_gpio_phase_start bit
 Arguments:
 NONE
 Return value:
 int SUCCESS				0
 int ERR_I2C_SCL_LOCK	1
 int ERR_I2C_SDA_LOCK	2
 ----------------------------------------------------*/
static INT8 i2c_gpio_phase_stop(void)
{
#ifndef EXTERNAL_PULL_HIGH
    set_sda_out();
    set_scl_out();
#endif

    set_sda_lo();
    OneNop();
    set_scl_hi();
    OneNop();
    if (!get_scl())
    {
        return ERR_I2C_SCL_LOCK;
    }
    /*Send I2C bus transfer end signal.*/
    set_sda_hi();
    OneNop();
    if (!get_sda())
    {
        return ERR_I2C_SDA_LOCK;
    }
    return SUCCESS;
}

/*---------------------------------------------------
 void i2c_gpio_phase_set_bit(int val);
 Set a BIT (Hi or Low)
 Stream Format:
 SCL   _____/---\
		SDA   ??AAAAAAAA
 width  4.7u| 4u|
 Arguments:
 int i	: Set(1) or Clear(0) this bit on iic bus
 Return value:
 NONE
 ----------------------------------------------------*/
static void i2c_gpio_phase_set_bit(int val)
{
#ifndef EXTERNAL_PULL_HIGH
    set_sda_out();
    set_scl_out();
#endif

    set_scl_lo();
    if (val)
    {
        set_sda_hi();
    }
    else
    {
        set_sda_lo();
    }
    OneNop();
    set_scl_hi();
    OneNop();
    OneNop();
    OneNop();
    OneNop();
    set_scl_lo();
}

/*---------------------------------------------------
 UINT8 i2c_gpio_phase_get_bit(id);
 Set a BIT (Hi or Low)
 Stream Format:
 SCL   _____/---\
		SDA   ??AAAAAAAA
 width  4.7u| 4u|
 Arguments:
 NONE
 Return value:
 int i	: Set(1) or Clear(0) this bit on iic bus
 ----------------------------------------------------*/
static UINT8 i2c_gpio_phase_get_bit(void)
{
    UINT8 ret = 0;

    //set_sda_out();
    /*Hi Ind.*/
    //set_sda_lo();

    set_sda_in();
    OneNop();
    set_scl_hi();
    OneNop();
    OneNop();
    OneNop();
    OneNop();

    ret = get_sda();
    set_scl_lo();
    OneNop();
    OneNop();
    OneNop();
    OneNop();

    return ret;
}

/*---------------------------------------------------
 UINT8 i2c_gpio_phase_set_byte(UINT8 data);
 Perform a byte write process
 Stream Format:
 SCL   ___/-\___/-\___/-\___/-\___/-\___/-\___/-\___/-\__/-\
		SDA   --< B7>-< B6>-< B5>-< B4>-< B3>-< B2>-< B1>-< B0>-Check
 Clock Low: 4.7u, High: 4.0u.                            Ack
 Data exchanged at CLK Low, ready at SCL High
 Arguments:
 char data	- Data to send on iic bus
 return value:
 The /ack signal returned from slave
 ----------------------------------------------------*/
static UINT8 i2c_gpio_phase_set_byte(UINT8 dat)
{
    UINT8 i = 0;

    for (i = 0; i < 8; i++)
    {
        if (dat & 0x80)
        {
            i2c_gpio_phase_set_bit(1);
        }
        else
        {
            i2c_gpio_phase_set_bit(0);
        }

        dat <<= 1;
    }

    return (i2c_gpio_phase_get_bit());
}

/*---------------------------------------------------
 UINT8 i2c_gpio_phase_get_byte(int ack);
 Perform a byte read process
 by Charlemagne Yue
 SCL   ___/-\___/-\___/-\___/-\___/-\___/-\___/-\___/-\___/-\
	SDA   --< B7>-< B6>-< B5>-< B4>-< B3>-< B2>-< B1>-< B0>-(Ack)
 Clock Low: 4.7u, High: 4.0u.
 Data exchanged at CLK Low, ready at SCL High
 ----------------------------------------------------*/
static UINT8 i2c_gpio_phase_get_byte(int ack)
{
    UINT8 ret = 0;
    UINT8 i = 0;

    for (i = 0; i < 8; i++)
    {
        ret <<= 1;
        ret |= i2c_gpio_phase_get_bit();
    }

    i2c_gpio_phase_set_bit(ack);

    return ret;
}

/*---------------------------------------------------
 INT8 i2c_gpio_read_no_stop(UINT8 slv_addr, UINT8 *data, UINT32 len);
 Perform bytes read process but no stop
 Stream Format:
 S<SLV_R><Read>
 S		: Start
 <SLV_R>	: Set Slave addr & Read Mode
 <Read>	: Read Data
 Arguments:
 BYTE slv_addr - Slave Address
 BYTE reg_addr - Data address
 Return value:
 Data returned
 ----------------------------------------------------*/
static INT8 i2c_gpio_read_no_stop(UINT8 slv_addr, UINT8 *dat, UINT32 len)
{
    UINT8 i = I2C_GPIO_TIMES_OUT;

    if (dat == NULL)
    {
        return ERROR;
    }

    slv_addr |= 1;/*Read.*/
    while (--i)/*Ack polling !!.*/
    {
        if (i2c_gpio_phase_start() != SUCCESS)
        {
            return ERROR;
        }
        /*has /ACK => i2c_gpio_phase_start transfer.*/
        if (!i2c_gpio_phase_set_byte(slv_addr))
        {
            break;
        }

        /*device is busy, issue i2c_gpio_phase_stop and chack again later.*/
        if (i2c_gpio_phase_stop() != SUCCESS)
        {
            return ERROR;
        }
        OneNop();
    }

    if (i == 0)
    {
        return ERR_TIME_OUT;
    }

    for (i = 0; i < (len - 1); i++)
    {
        /*with no /ack to stop process.*/
        dat[i] = i2c_gpio_phase_get_byte(0);
    }
    dat[len - 1] = i2c_gpio_phase_get_byte(1);

    return SUCCESS;
}

/*---------------------------------------------------
 INT8 i2c_gpio_write_no_stop(UINT8 slv_addr, UINT8 *data, UINT32 len);
 Perform bytes write process but no stop
 Stream Format:
 S<SLV_W><Write>
 S		: Start
 <SLV_W>	: Set Slave addr & Write Mode
 <Write>	: Send Data
 Arguments:
 BYTE slv_addr - Slave Address
 BYTE value    - data to write
 Return value:
 NONE
 ----------------------------------------------------*/
static INT8 i2c_gpio_write_no_stop(UINT8 slv_addr, UINT8 *dat, UINT32 len)
{
    UINT8 i = I2C_GPIO_TIMES_OUT;

    if (dat == NULL)
    {
        return ERROR;
    }
    slv_addr &= 0xFE;/*Write.*/
    while (--i)/*Ack polling !!.*/
    {
        if (i2c_gpio_phase_start() != SUCCESS)
        {
            return ERROR;
        }
        /*has /ACK => i2c_gpio_phase_start transfer.*/
        if (!i2c_gpio_phase_set_byte(slv_addr))
        {
            //PMU_PRINTF("i2c_gpio_write_no_stop Ack polling OK!\n");
            break;
        }

        /*device is busy, issue i2c_gpio_phase_stop and chack again later.*/
        if (i2c_gpio_phase_stop() != SUCCESS)
        {
            return ERROR;
        }
        OneNop();
        //PMU_PRINTF("i2c_gpio_write_no_stop Ack polling !\n");
    }

    if (i == 0)
    {
        //PMU_PRINTF("i2c_gpio_write_no_stop err ERR_TIME_OUT!\n");
        return ERR_TIME_OUT;
    }

    for (i = 0; i < len; i++)
    {
        i2c_gpio_phase_set_byte(dat[i]);
    }

    return SUCCESS;
}

/*---------------------------------------------------
 UINT8 i2c_gpio_read(UINT8 slv_addr, UINT8 *data, UINT32 len);
 Perform a byte read process
 Stream Format:
 S<SLV_R><Read>P
 S		: Start
 P		: Stop
 <SLV_R>	: Set Slave addr & Read Mode
 <Read>	: Read Data
 Arguments:
 BYTE slv_addr - Slave Address
 BYTE reg_addr - Data address
 Return value:
 Data returned
 ----------------------------------------------------*/
static UINT8 i2c_gpio_read(UINT8 slv_addr, UINT8 *dat, UINT32 len)
{
    INT8 ret = 0;

    if (dat == NULL)
    {
        return ERROR;
    }
    if (SUCCESS != i2c_gpio_read_no_stop(slv_addr, dat, len))
    {
        return ret;
    }

    if (i2c_gpio_phase_stop() != SUCCESS)
    {
        return ERROR;
    }
    return SUCCESS;
}

/*---------------------------------------------------
 INT8 i2c_gpio_write(UINT8 slv_addr, UINT8 *data, UINT32 len);
 Perform bytes write process
 Stream Format:
 S<SLV_W><Write>P
 S		: Start
 P		: Stop
 <SLV_W>	: Set Slave addr & Write Mode
 <Write>	: Send Data
 Arguments:
 BYTE slv_addr - Slave Address
 BYTE value    - data to write
 Return value:
 NONE
 ----------------------------------------------------*/
static INT8 i2c_gpio_write(UINT8 slv_addr, UINT8 *dat, UINT32 len)
{
    INT8 ret = 0;

    if (dat == NULL)
    {
        return ERROR;
    }
    if ((ret = i2c_gpio_write_no_stop(slv_addr, dat, len)) != SUCCESS)
    {
        return ret;
    }
    if (i2c_gpio_phase_stop() != ERROR)
    {
        return ERROR;
    }

    return SUCCESS;
}

static void show_off(void)
{
    INT8 ret = SUCCESS;
#ifndef FD650_PAN_SUPPORT
    UINT8 led_map[4] =
    { 0x00, 0x3F, 0x71, 0x1 };
#else
    UINT8 led_map[4] =
    {   0x00, 0x5F, 0x65, 0x1};
#endif

    ret |= i2c_gpio_write(SETING_ADDR, &led_map[3], 1);/*Enable CH455 Panel.*/
    ret |= i2c_gpio_write(DIG3_ADDR, &led_map[2], 1);/*show 'F'.*/
#ifndef FD650_PAN_SUPPORT
    ret |= i2c_gpio_write(DIG2_ADDR, &led_map[2], 1);/*show 'F'.*/
    ret |= i2c_gpio_write(DIG1_ADDR, &led_map[1], 1);/*show 'O'.*/
#else
    ret |= i2c_gpio_write(DIG1_ADDR, &led_map[2], 1);/*show 'F'.*/
    ret |= i2c_gpio_write(DIG2_ADDR, &led_map[1], 1);/*show 'O'.*/
#endif
    ret |= i2c_gpio_write(DIG0_ADDR, &led_map[0], 1);/*show blank.*/

    return;
}

static void show_bank(void)
{
    INT8 ret = SUCCESS;
    const UINT8 led_map[4] =
    { 0x0, 0x0, 0x0, 0x1 };

    ret |= i2c_gpio_write(SETING_ADDR, &led_map[3], 1);/*Enable CH455 Panel.*/
    ret |= i2c_gpio_write(DIG0_ADDR, &led_map[2], 1);
    ret |= i2c_gpio_write(DIG1_ADDR, &led_map[2], 1);
    ret |= i2c_gpio_write(DIG2_ADDR, &led_map[2], 1);
    ret |= i2c_gpio_write(DIG3_ADDR, &led_map[2], 1);

    return;
}

extern RTC_TIMER g_rtc;
static void show_time()
{
    INT8 ret = SUCCESS;
    UINT8 min_ge = 0;
    UINT8 min_shi = 0;
    UINT8 hour_ge = 0;
    UINT8 hour_shi = 0;
    UINT8 show_colon = 0;
#ifndef FD650_PAN_SUPPORT
    const UINT8 led_map[11] =
    { 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x1 };/*ASCII of data 0~9.*/
#else
    const UINT8 led_map[11] =
    {   0x5F, 0x12, 0x2F, 0x3B, 0x72, 0x79, 0x7D, 0x53, 0x7F, 0x7B, 0x1};/*ASCII of data 0~9.*/
#endif
    pRTC_TIMER rtc = &g_rtc;

    min_ge = rtc->min % 10;
    min_shi = rtc->min / 10;
    hour_ge = rtc->hour % 10;
    hour_shi = rtc->hour / 10;
    show_colon = (led_map[hour_ge] | 0x80);

    ret |= i2c_gpio_write(SETING_ADDR, &led_map[10], 1);/*Enable CH455 Panel.*/
    ret |= i2c_gpio_write(DIG0_ADDR, &led_map[hour_shi], 1);
#ifndef FD650_PAN_SUPPORT
    ret |= i2c_gpio_write(DIG1_ADDR, &led_map[hour_ge], 1);

    if (rtc->sec % 2 == 0)/*show ':'.*/
    {
        ret |= i2c_gpio_write(DIG1_ADDR, &show_colon, 1);
    }

    ret |= i2c_gpio_write(DIG2_ADDR, &led_map[min_shi], 1);
#else
    ret |= i2c_gpio_write(DIG2_ADDR, &led_map[hour_ge], 1);
    ret |= i2c_gpio_write(DIG1_ADDR, &led_map[min_shi], 1);
#endif
    ret |= i2c_gpio_write(DIG3_ADDR, &led_map[min_ge], 1);

    return;
}

void panel_init(void)
{
    hal_mcu_gpio_en (SYS_I2C_SDA);
    hal_mcu_gpio_en (SYS_I2C_SCL);
#ifdef EXTERNAL_PULL_HIGH
    hal_gpio_bit_dir_set(SYS_I2C_SDA, HAL_GPIO_O_DIR);
#endif
    hal_gpio_bit_dir_set(SYS_I2C_SCL, HAL_GPIO_O_DIR);

    Received_Panel_Key1 = PMU_READ_BYTE(PANEL_POWER_KEY1_ADDR);
    Received_Panel_Key2 = PMU_READ_BYTE(PANEL_POWER_KEY2_ADDR);
}

void show_panel(enum SHOW_TYPE show_type)
{

    if (show_type == SHOW_BANK)
    {
        show_bank();
    }
    else if (show_type == SHOW_TIME)
    {
        show_time();
    }
    else if (show_type == SHOW_OFF)
    {
        show_off();
    }
    else if (show_type == SHOW_DEFAULT)
    {
        show_off();
    }
    else if (show_type == SHOW_NO_CHANGE)
    {
        return;/*let the ch455(includding panle and lockleds...) alone.*/
    }
    else
    {
        show_off();
    }
}

static UINT8 panel_scan(void)
{
    UINT8 ret = ERROR;

#if ((defined PMU_MCU_3821B) && (defined HW_SCAN_PANEL_ENABLE)) \
	|| (defined GPIO_POWER_KEY_SUPPORT)
    unsigned char power_key_press_count = 0;

    /*Enable xpmu_gpio[0] and xpmu_gpio[1].*/
    hal_mcu_gpio_en(XPMU_GPIO_0);
    hal_mcu_gpio_en(XPMU_GPIO_1);
    hal_gpio_bit_dir_set(XPMU_GPIO_0, HAL_GPIO_O_DIR);
    hal_gpio_bit_dir_set(XPMU_GPIO_1, HAL_GPIO_I_DIR);

    /*xpmu_gpio[0] always output '1'.*/
    hal_gpio_bit_set(XPMU_GPIO_0, 1);
    while(S3821B_PANEL_KEY_PRESSED == hal_gpio_bit_get(XPMU_GPIO_1))
    {
        power_key_press_count++;
        if(20 <= power_key_press_count)
        {
            ret = SUCCESS;
        }
    }
#else

    if (FIRST_SCAN_GOING == first_scan_panel)
    {
        if (i2c_gpio_read(KEY_ADDR, &Detected_Panel_Key1, 1) != SUCCESS)
            return ERROR;
        if ((Detected_Panel_Key1 != Received_Panel_Key1)
        && (Detected_Panel_Key1 != Received_Panel_Key2))
        {
            first_scan_panel = FIRST_SCAN_DONE;
        }
    }
    else
    {
        if (i2c_gpio_read(KEY_ADDR, &Detected_Panel_Key2, 1) != SUCCESS)
            return ERROR;

        if ((Detected_Panel_Key1 != Detected_Panel_Key2)
        && (((Detected_Panel_Key2 == Received_Panel_Key1)
        && (INVALID_PANEL_KEY1 != Received_Panel_Key1)
        && (INVALID_PANEL_KEY2 != Received_Panel_Key1))

        || ((Detected_Panel_Key2 == Received_Panel_Key2)
        && (INVALID_PANEL_KEY1 != Received_Panel_Key2)
        && (INVALID_PANEL_KEY2 != Received_Panel_Key2))))
        {
            return SUCCESS;
        }
    }
#endif

    return ret;
}

void panel_process(void)
{
    pRTC_TIMER rtc = &g_rtc;
    unsigned char type = PMU_READ_BYTE(SHOW_TYPE_PAR);
#if (defined HW_SCAN_PANEL_ENABLE) || (defined SHOW_PANEL_DISABLE)
    type = SHOW_NO_CHANGE;
#endif

#if (!defined HW_SCAN_PANEL_ENABLE) && (!defined SHOW_PANEL_DIS)
    show_panel(type);
#endif
    if (SUCCESS == panel_scan())
    {
        show_panel (SHOW_BANK);
        mcu_disable_int();/*Disable all mcu interrupts.*/
        PMU_WRITE_BYTE(EXIT_STANDBY_TYPE, EXIT_STANDBY_TYPE_PANEL);
        exit_standby_status();
        main_cpu_get_time();
        mcu_gpio_init();/*disable all gpio.*/
        stb_power_on();

        while (1)
            ;/*exit standby, then hold mcu.*/
    }
}
#endif
