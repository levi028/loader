#include <sys_config.h>
#include <types.h>
#include <retcode.h>
#include <osal/osal.h>
#include <hal/hal_gpio.h>
#include <bus/i2c/i2c.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/nim/nim.h>
#include <hld/nim/nim_tuner.h>

#define MAX_LNB_A8293    2
#define LNB_MUTEX_ENTER(id)        osal_mutex_lock(lnb_a8293_array[id].mutex_id, OSAL_WAIT_FOREVER_TIME)
#define LNB_MUTEX_LEAVE(id)        osal_mutex_unlock(lnb_a8293_array[id].mutex_id)
#define LNB_READ(id, p, l)            i2c_read(lnb_a8293_array[id].i2c_type_id, lnb_a8293_array[id].i2c_base_addr, (p), l)
#define LNB_WRITE(id, p, l)        i2c_write(lnb_a8293_array[id].i2c_type_id, lnb_a8293_array[id].i2c_base_addr, (p), l)
#define LNB_PRINT(...)            do{}while(0)
//#define LNB_PRINT                soc_printf
#define LNB_INT_CHECK_TIME        20
#define LNB_INT_MONITOR_TIME    2000
/********************************************/
/*VSEL3        VSEL2    VSEL1    VSEL0    LNB (V)    */
/*0             0         0         0         12.709    */
/*0             0         0         1         13.042    */
/*0             0         1         0         13.375    */
/*0             0         1         1         13.709    */
/*0             1         0         0         14.042    */
/*0             1         0         1         14.375    */
/*0             1         1         0         14.709    */
/*0             1         1         1         15.042    */
/*1             0         0         0         18.042    */
/*1             0         0         1         18.375    */
/*1             0         1         0         18.709    */
/*1             0         1         1         19.042    */
/*1             1         0         0         19.375    */
/*1             1         0         1         19.709    */
/*1             1         1         0         20.042    */
/*1             1         1         1         20.375    */
/*********************************************/
#define LNB_H_VALUE                0xa
#define LNB_V_VALUE                0x2

#define LNB_ODT_EN                (1<<4)
#define LNB_ODT_DIS                0

#define LNB_OUT_EN                (1<<5)
#define LNB_OUT_DIS                0

/*********************************************/
#define LNB_INTERNAL_TONE        1
#define LNB_EXTERNAL_TONE        0

#define LNB_TGATE_ON            (1<<1)
#define LNB_TGATE_OFF            0

#define LNB_CADT_ON            (1<<2)
#define LNB_CADT_OFF            0
/*********************************************/
#define LNB_REGC_0                0
#define LNB_REGC_1                (1<<7)
/*********************************************/
#define LNB_DIS_MSK                (1<<0)
#define LNB_OCP_MSK                (1<<2)
#define LNB_PNG_MSK            (1<<4)
#define LNB_TSD_MSK                (1<<6)
#define LNB_VUV_MSK                (1<<7)
/*********************************************/
#define LNB_CAD_MSK            (1<<0)

UINT32 lnb_a8293_cnt = 0;

struct LNB_A8293_CONTROL
{
    UINT32 i2c_base_addr;
    UINT32 i2c_type_id;
    OSAL_ID mutex_id;
    OSAL_ID timmer_id;
    UINT8   int_gpio_en;
    UINT8   int_gpio_polar;
    UINT8   int_gpio_num;
    UINT8   current_polar;
    UINT8   power_en;
};

struct LNB_A8293_CONTROL lnb_a8293_array[MAX_LNB_A8293] =
{
{0, 0, OSAL_INVALID_ID, OSAL_INVALID_ID,0, 0, 0, 0, 0},
{0, 0, OSAL_INVALID_ID, OSAL_INVALID_ID,0, 0, 0, 0, 0}
};
INT32 lnb_a8293_init(UINT32 id);

void lnb_a8293_monitor(UINT32 id)
{
    UINT32 data = 0;
    UINT16 mask = (LNB_OCP_MSK|LNB_TSD_MSK)|(LNB_CAD_MSK<<8);
	long unsigned int len = 2;
    LNB_READ(id, &data, 2);
    LNB_PRINT("status: %04x \n", data);
    if(data&mask)
    {
        if(lnb_a8293_array[id].int_gpio_en)
        {
            OSAL_T_CTIM lnb_timer_param;
            lnb_timer_param.callback = lnb_a8293_monitor;
            lnb_timer_param.param = id;
            lnb_timer_param.type = OSAL_TIMER_ALARM;
            lnb_timer_param.time  = LNB_INT_MONITOR_TIME;

            lnb_a8293_array[id].timmer_id = osal_timer_create(&lnb_timer_param);
            if(lnb_a8293_array[id].timmer_id == OSAL_INVALID_ID)
            {
                LNB_PRINT("create timer error\n");
                ASSERT(0);
            }
        }
    }
    else
    {
        if(data&LNB_DIS_MSK)
        {
            lnb_a8293_init(id);
            if(lnb_a8293_array[id].int_gpio_en)
            {
                lnb_a8293_array[id].timmer_id = OSAL_INVALID_ID;
                HAL_GPIO_INT_SET(lnb_a8293_array[id].int_gpio_num, 1);
            }
        }
    }
}

void lnb_a8293_int(UINT32 id)
{
    if(HAL_GPIO_INT_STA_GET(lnb_a8293_array[id].int_gpio_num)==0)
        return;
    if(HAL_GPIO_BIT_GET(lnb_a8293_array[id].int_gpio_num)!=lnb_a8293_array[id].int_gpio_polar)
        return;
    HAL_GPIO_INT_SET(lnb_a8293_array[id].int_gpio_num, 0);
    HAL_GPIO_INT_CLEAR(lnb_a8293_array[id].int_gpio_num);
    if(lnb_a8293_array[id].timmer_id == OSAL_INVALID_ID)
    {
        OSAL_T_CTIM lnb_timer_param;
        lnb_timer_param.callback = lnb_a8293_monitor;
        lnb_timer_param.param = id;
        lnb_timer_param.type = OSAL_TIMER_ALARM;
        lnb_timer_param.time  = LNB_INT_CHECK_TIME;

        lnb_a8293_array[id].timmer_id = osal_timer_create(&lnb_timer_param);
        if(lnb_a8293_array[id].timmer_id == OSAL_INVALID_ID)
        {
            LNB_PRINT("create timer error\n");
            ASSERT(0);
        }
    }
    else
        osal_timer_set(lnb_a8293_array[id].timmer_id, LNB_INT_CHECK_TIME);
}

INT32 lnb_a8293_init(UINT32 id)
{
    UINT32 data;
    INT32 rlt;
	long unsigned int len = 2;
	long unsigned int len1 = 1; 

    rlt = LNB_READ(id, &data, &len);
    if(rlt!=SUCCESS)
    {
        LNB_PRINT("i2c read error at %08x: %08x\n", \
            lnb_a8293_array[id].i2c_type_id, lnb_a8293_array[id].i2c_base_addr);
        return rlt;
    }
    if(lnb_a8293_array[id].mutex_id == OSAL_INVALID_ID)
    {
        //UINT32 int_number;
        //First time init
        lnb_a8293_array[id].mutex_id = osal_mutex_create();

        //Register int handler
        if(lnb_a8293_array[id].int_gpio_en)
        {
            HAL_GPIO_INT_SET(lnb_a8293_array[id].int_gpio_num , 1);
            HAL_GPIO_INT_REDG_SET(lnb_a8293_array[id].int_gpio_num, 1);
            //HAL_GPIO_INT_FEDG_SET(lnb_a8293_array[id].int_gpio_num, 1);
            HAL_GPIO_INT_CLEAR(lnb_a8293_array[id].int_gpio_num);
            if(ALI_S3602==sys_ic_get_chip_id())
            {
                //int_number = 8;
            }
            else
            {
                LNB_PRINT("Fix Me: %s %d: I don't know the gpio interrupt number setting!\n", __FILE__, __LINE__);
                while(1);
                //if(ALI_M3327C==sys_ic_get_chip_id())
                //if(ALI_M3329E==sys_ic_get_chip_id())
                //if(sys_ic_is_M3202())
            }
            osal_interrupt_register_lsr(lnb_a8293_array[id].int_gpio_num, lnb_a8293_int, id);
        }
        else
        {
            OSAL_T_CTIM lnb_timer_param;
            lnb_timer_param.callback = lnb_a8293_monitor;
            lnb_timer_param.param = id;
            lnb_timer_param.type = OSAL_TIMER_CYCLE;
            lnb_timer_param.time  = LNB_INT_MONITOR_TIME;

            lnb_a8293_array[id].timmer_id = osal_timer_create(&lnb_timer_param);
            if(lnb_a8293_array[id].timmer_id == OSAL_INVALID_ID)
            {
                LNB_PRINT("create timer error\n");
                ASSERT(0);
            }
            osal_timer_activate(lnb_a8293_array[id].timmer_id, 1);
        }
    }

    LNB_MUTEX_ENTER(id);

    if(NIM_PORLAR_HORIZONTAL==lnb_a8293_array[id].current_polar)
        data = LNB_H_VALUE;
    else
        data = LNB_V_VALUE;
    if(lnb_a8293_array[id].power_en)
        data |= (LNB_ODT_EN|LNB_OUT_EN|LNB_REGC_0);
    else
        data |= (LNB_ODT_EN|LNB_OUT_DIS|LNB_REGC_0);
    LNB_WRITE(id, &data, &len1);

    data = (LNB_EXTERNAL_TONE|LNB_TGATE_ON|LNB_CADT_OFF|LNB_REGC_1);
    LNB_WRITE(id, &data, &len1);
    LNB_MUTEX_LEAVE(id);
    return SUCCESS;
}

void lnb_a8293_power_en(UINT32 id, UINT32 en)
{
    lnb_a8293_array[id].power_en = (UINT8)en;
    if(lnb_a8293_array[id].mutex_id != OSAL_INVALID_ID)
        lnb_a8293_init(id);
}
INT32 lnb_a8293_set_pol(UINT32 id, UINT8 param)
{
    UINT32 data;
	long unsigned int len = 1;
    LNB_MUTEX_ENTER(id);
    lnb_a8293_array[id].current_polar = param;
    if(NIM_PORLAR_HORIZONTAL==lnb_a8293_array[id].current_polar)
        data = LNB_H_VALUE;
    else
        data = LNB_V_VALUE;
    if(lnb_a8293_array[id].power_en)
        data |= (LNB_ODT_EN|LNB_OUT_EN|LNB_REGC_0);
    else
        data |= (LNB_ODT_EN|LNB_OUT_DIS|LNB_REGC_0);
    LNB_WRITE(id, &data, &len);
    LNB_MUTEX_LEAVE(id);
    return SUCCESS;
}

INT32 lnb_a8293_control(UINT32 id, UINT32 cmd, UINT32 param)
{
    INT32 rlt = SUCCESS;
    if(id>MAX_LNB_A8293)
        return !SUCCESS;
    switch(cmd)
    {
        case LNB_CMD_ALLOC_ID:
            lnb_a8293_array[lnb_a8293_cnt].mutex_id = OSAL_INVALID_ID;
            *((UINT32 *)param) = lnb_a8293_cnt;
            lnb_a8293_cnt++;
            if(lnb_a8293_cnt>MAX_LNB_A8293)
                rlt = !SUCCESS;
            break;
        case LNB_CMD_INIT_CHIP:
            {
                struct EXT_LNB_CTRL_CONFIG * lnb_config = (struct EXT_LNB_CTRL_CONFIG *)param;
                lnb_a8293_array[id].i2c_base_addr = lnb_config->i2c_base_addr;
                lnb_a8293_array[id].i2c_type_id = lnb_config->i2c_type_id;
                lnb_a8293_array[id].int_gpio_en = lnb_config->int_gpio_en;
                lnb_a8293_array[id].int_gpio_polar = lnb_config->int_gpio_polar;
                lnb_a8293_array[id].int_gpio_num = lnb_config->int_gpio_num;
                lnb_a8293_array[id].current_polar = NIM_PORLAR_HORIZONTAL;
                lnb_a8293_array[id].mutex_id = OSAL_INVALID_ID;
                lnb_a8293_array[id].timmer_id = OSAL_INVALID_ID;
                rlt = lnb_a8293_init(id);
            }
            break;
        case LNB_CMD_SET_POLAR:
            rlt = lnb_a8293_set_pol(id, (UINT8)param);
            break;
        case LNB_CMD_POWER_EN:
            lnb_a8293_power_en(id, param);
            break;
        default:
            rlt = !SUCCESS;
            break;
    }
    return rlt;
}

