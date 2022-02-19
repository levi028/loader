#include "ch455_gpio.h"
#include "standby_gpio.h"
#include <sys_config.h>
//==============================================================================================//

extern void standby_delay_time(unsigned long flag);
//==============================================================================================//

void pan_delay_us(unsigned long time_tick)
{
	standby_delay_time(time_tick*10);
}

void STANDBY_HAL_GPIO_BIT_ENABLE(unsigned char pos, unsigned char en)
{
	unsigned long temp_data = 0;

	if(32 > pos)
	{
		temp_data = *(volatile unsigned long *)(HAL_GPIO_EN_REG);
		temp_data &= ~(1<<pos);
		temp_data |= (en<<pos);
		*(volatile unsigned long *)(HAL_GPIO_EN_REG) = temp_data;
		pan_delay_us(5);
	}
	else if(64 > pos)
	{
		temp_data = *(volatile unsigned long *)(HAL_GPIO1_EN_REG);
		pos -= 32;
		temp_data &= ~(1<<pos);
		temp_data |= (en<<pos);
		*(volatile unsigned long *)(HAL_GPIO1_EN_REG) = temp_data;
		pan_delay_us(5);
	}
	else if(96 > pos)
	{
		temp_data = *(volatile unsigned long *)(HAL_GPIO2_EN_REG);
		pos -= 64;
		temp_data &= ~(1<<pos);
		temp_data |= (en<<pos);
		*(volatile unsigned long *)(HAL_GPIO2_EN_REG) = temp_data;
		pan_delay_us(5);
	}
	else if(128 > pos)
	{
		temp_data = *(volatile unsigned long *)(HAL_GPIO3_EN_REG);
		pos -= 96;
		temp_data &= ~(1<<pos);
		temp_data |= (en<<pos);
		*(volatile unsigned long *)(HAL_GPIO3_EN_REG) = temp_data;
		pan_delay_us(5);
	}
	else if(160 > pos)
	{
		temp_data = *(volatile unsigned long *)(HAL_GPIO4_EN_REG);
		pos -= 128;
		temp_data &= ~(1<<pos);
		temp_data |= (en<<pos);
		*(volatile unsigned long *)(HAL_GPIO4_EN_REG) = temp_data;
		pan_delay_us(5);
	}
}

unsigned char STANDBY_HAL_GPIO_BIT_GET(unsigned char pos)
{
	unsigned long temp_data = 0;

	if(32 > pos)
	{
		temp_data = *(volatile unsigned long *)(HAL_GPIO_DI_REG);
		temp_data >>= (pos);
		temp_data &= 0x1;
		pan_delay_us(5);
	}
	else if(64 > pos)
	{
		temp_data = *(volatile unsigned long *)(HAL_GPIO1_DI_REG);
		temp_data >>= (pos - 32);
		temp_data &= 0x1;
		pan_delay_us(5);
	}
	else if(96 > pos)
	{
		temp_data = *(volatile unsigned long *)(HAL_GPIO2_DI_REG);
		temp_data >>= (pos - 64);
		temp_data &= 0x1;
		pan_delay_us(5);
	}
	else if(128 > pos)
	{
		temp_data = *(volatile unsigned long *)(HAL_GPIO3_DI_REG);
		temp_data >>= (pos - 96);
		temp_data &= 0x1;
		pan_delay_us(5);
	}
	else if(160 > pos)
	{
		temp_data = *(volatile unsigned long *)(HAL_GPIO4_DI_REG);
		temp_data >>= (pos - 128);
		temp_data &= 0x1;
		pan_delay_us(5);
	}

	return (unsigned char)(temp_data);
}

void STANDBY_HAL_GPIO_BIT_DIR_SET(unsigned char pos, unsigned char value)
{
	unsigned long temp_data = 0;

	if(32 > pos)
	{
		temp_data = *(volatile unsigned long *)(HAL_GPIO_DIR_REG);
		temp_data &= ~(1<<pos);
		temp_data |= (value<<pos);
		*(volatile unsigned long *)(HAL_GPIO_DIR_REG) = temp_data;
		pan_delay_us(5);
	}
	else if(64 > pos)
	{
		temp_data = *(volatile unsigned long *)(HAL_GPIO1_DIR_REG);
		pos -= 32;
		temp_data &= ~(1<<pos);
		temp_data |= (value<<pos);
		*(volatile unsigned long *)(HAL_GPIO1_DIR_REG) = temp_data;
		pan_delay_us(5);
	}
	else if(96 > pos)
	{
		temp_data = *(volatile unsigned long *)(HAL_GPIO2_DIR_REG);
		pos -= 64;
		temp_data &= ~(1<<pos);
		temp_data |= (value<<pos);
		*(volatile unsigned long *)(HAL_GPIO2_DIR_REG) = temp_data;
		pan_delay_us(5);
	}
	else if(128 > pos)
	{
		temp_data = *(volatile unsigned long *)(HAL_GPIO3_DIR_REG);
		pos -= 96;
		temp_data &= ~(1<<pos);
		temp_data |= (value<<pos);
		*(volatile unsigned long *)(HAL_GPIO3_DIR_REG) = temp_data;
		pan_delay_us(5);
	}
	else if(160 > pos)
	{
		temp_data = *(volatile unsigned long *)(HAL_GPIO4_DIR_REG);
		pos -= 128;
		temp_data &= ~(1<<pos);
		temp_data |= (value<<pos);
		*(volatile unsigned long *)(HAL_GPIO4_DIR_REG) = temp_data;
		pan_delay_us(5);
	}
}

void STANDBY_HAL_GPIO_BIT_SET(unsigned char pos, unsigned char value)
{
	unsigned long temp_data = 0;

	if(32 > pos)
	{
		temp_data = *(volatile unsigned long *)(HAL_GPIO_DO_REG);
		temp_data &= ~(1<<pos);
		temp_data |= (value<<pos);
		*(volatile unsigned long *)(HAL_GPIO_DO_REG) = temp_data;
		pan_delay_us(5);
	}
	else if(64 > pos)
	{
		temp_data = *(volatile unsigned long *)(HAL_GPIO1_DO_REG);
		pos -= 32;
		temp_data &= ~(1<<pos);
		temp_data |= (value<<pos);
		*(volatile unsigned long *)(HAL_GPIO1_DO_REG) = temp_data;
		pan_delay_us(5);
	}
	else if(96 > pos)
	{
		temp_data = *(volatile unsigned long *)(HAL_GPIO2_DO_REG);
		pos -= 64;
		temp_data &= ~(1<<pos);
		temp_data |= (value<<pos);
		*(volatile unsigned long *)(HAL_GPIO2_DO_REG) = temp_data;
		pan_delay_us(5);
	}
	else if(128 > pos)
	{
		temp_data = *(volatile unsigned long *)(HAL_GPIO3_DO_REG);
		pos -= 96;
		temp_data &= ~(1<<pos);
		temp_data |= (value<<pos);
		*(volatile unsigned long *)(HAL_GPIO3_DO_REG) = temp_data;
		pan_delay_us(5);
	}
	else if(160 > pos)
	{
		temp_data = *(volatile unsigned long *)(HAL_GPIO4_DO_REG);
		pos -= 128;
		temp_data &= ~(1<<pos);
		temp_data |= (value<<pos);
		*(volatile unsigned long *)(HAL_GPIO4_DO_REG) = temp_data;
		pan_delay_us(5);
	}
}

void pan_ch455_set_val_gpio(unsigned long gpio_pin, unsigned char val)
{
	STANDBY_HAL_GPIO_BIT_SET(gpio_pin, val);
}

void pan_ch455_en_gpio(unsigned long gpio_pin, unsigned char en)
{
	STANDBY_HAL_GPIO_BIT_ENABLE(gpio_pin, en);
}

void pan_ch455_dir_set_gpio(unsigned long gpio_pin, unsigned char en)
{
	STANDBY_HAL_GPIO_BIT_DIR_SET(gpio_pin, en);
}

void stby_pan_init(void)
{
	/*enable gpio.*/
	pan_ch455_en_gpio(STANDBY_PAN_SDA_PIN, 1);
	pan_ch455_en_gpio(STANDBY_PAN_SCL_PIN, 1);
	pan_delay_us(5);

	/*set gpio dir.*/ 
	pan_ch455_dir_set_gpio(STANDBY_PAN_SDA_PIN, 1);
	pan_ch455_dir_set_gpio(STANDBY_PAN_SCL_PIN, 1);
	pan_delay_us(5);
}

unsigned char stby_scan_key(void)
{
	unsigned char pan_key = 0;

	stby_i2c_gpio_read(KEY_ADDR, &pan_key, 1);

	return pan_key; 
}
	
void stby_show_off(void)
{
	unsigned char led_map[3] = {0x00, 0x3F, 0x71};

	stdby_i2c_gpio_write(DIG0_ADDR, &led_map[0], 1);/*show "".*/
	stdby_i2c_gpio_write(DIG1_ADDR, &led_map[1], 1);/*show 'O'.*/
	stdby_i2c_gpio_write(DIG2_ADDR, &led_map[2], 1);/*show 'F'.*/
	stdby_i2c_gpio_write(DIG3_ADDR, &led_map[2], 1);/*show 'F'.*/
}
	
/*
ali demo only show 4 bit led, such as, hour_shi, hour_ge, min_shi,min_ge.
you can show more leds if you need for your board.
*/
void stby_show_time(unsigned char hour, unsigned char min, unsigned char sec)
{	
	unsigned char min_ge = 0, min_shi = 0, hour_ge = 0, hour_shi = 0, show_colon = 0;
	unsigned char led_map[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};

	min_ge = min%10;
	min_shi = min/10;
	hour_ge = hour%10;
	hour_shi= hour/10;
	show_colon = (led_map[hour_ge] | 0x80);/*show: code.*/

	stdby_i2c_gpio_write(DIG0_ADDR, &led_map[hour_shi], 1);
	stdby_i2c_gpio_write(DIG1_ADDR, &led_map[hour_ge], 1);
	if(sec%2 == 0)
	{
		stdby_i2c_gpio_write(DIG1_ADDR, &show_colon, 1);/*show ':'.*/
	}

	stdby_i2c_gpio_write(DIG2_ADDR, &led_map[min_shi], 1);
	stdby_i2c_gpio_write(DIG3_ADDR, &led_map[min_ge], 1);
}

void set_sda_out(void)
{
	STANDBY_HAL_GPIO_BIT_DIR_SET(STANDBY_PAN_SDA_PIN, HAL_GPIO_O_DIR);
}

void set_sda_in(void)
{
	STANDBY_HAL_GPIO_BIT_DIR_SET(STANDBY_PAN_SDA_PIN, HAL_GPIO_I_DIR);
}

#ifdef STANDBY_EXTERNAL_PULL_HIGH
void set_sda_hi(void)
{
	set_sda_in();
}

void set_sda_lo(void)
{
	set_sda_out();
	STANDBY_HAL_GPIO_BIT_SET(STANDBY_PAN_SDA_PIN, HAL_GPIO_SER_LOW);
}
#else
void set_sda_hi(void)
{
	STANDBY_HAL_GPIO_BIT_SET(STANDBY_PAN_SDA_PIN, HAL_GPIO_SET_HI);
}

void set_sda_lo(void)
{
	STANDBY_HAL_GPIO_BIT_SET(STANDBY_PAN_SDA_PIN, HAL_GPIO_SER_LOW);
}
#endif

void set_scl_out(void)
{
	STANDBY_HAL_GPIO_BIT_DIR_SET(STANDBY_PAN_SCL_PIN, HAL_GPIO_O_DIR);
}

void set_scl_in(void)
{
	STANDBY_HAL_GPIO_BIT_DIR_SET(STANDBY_PAN_SCL_PIN, HAL_GPIO_I_DIR);
}

#ifdef STANDBY_EXTERNAL_PULL_HIGH
void set_scl_hi(void)
{
	set_scl_in();
}

void set_scl_lo(void)
{
	set_scl_out();
	STANDBY_HAL_GPIO_BIT_SET(STANDBY_PAN_SCL_PIN, HAL_GPIO_SER_LOW);
}
#else
void set_scl_hi(void)
{
	STANDBY_HAL_GPIO_BIT_SET(STANDBY_PAN_SCL_PIN, HAL_GPIO_SET_HI);
}

void set_scl_lo(void)
{
	STANDBY_HAL_GPIO_BIT_SET(STANDBY_PAN_SCL_PIN, HAL_GPIO_SER_LOW);
}
#endif

unsigned char get_scl(void)
{
	unsigned char ret = 0;

	ret = STANDBY_HAL_GPIO_BIT_GET(STANDBY_PAN_SCL_PIN);

	return ret;
}

unsigned char get_sda(void)
{
	unsigned char ret = 0;

	ret = STANDBY_HAL_GPIO_BIT_GET(STANDBY_PAN_SDA_PIN);
	return ret;
}

/*---------------------------------------------------
unsigned char i2c_gpio_phase_start(void);
    Generate i2c_gpio_phase_start Condition:
    Stream Format:
        SCL   _____/--------\___
        SDA   =---------\_____
        width (4.7u)4.7u|4.7u|
    Arguments:
        NONE
    Return value:
        long STANDBY_SUCCESS 0
        long STANDBY_ERR_I2C_SCL_LOCK 1
        long STANDBY_ERR_I2C_SDA_LOCK 2
----------------------------------------------------*/
unsigned char stby_i2c_gpio_phase_start(void)
{
#ifdef STANDBY_EXTERNAL_PULL_HIGH
	set_sda_out();
	set_scl_out();
#endif

	set_sda_hi();
	if(!get_scl())
	{
		pan_delay_us(1);
	}

	set_scl_hi();
	pan_delay_us(1);
	if(!get_scl())
	{
		return STANDBY_ERR_I2C_SCL_LOCK;
	}

	if(!get_sda())
	{
		return STANDBY_ERR_I2C_SDA_LOCK;
	}

	set_sda_lo();
	pan_delay_us(1);
	set_scl_lo();

	return STANDBY_SUCCESS;
}

/*---------------------------------------------------
unsigned char i2c_gpio_phase_stop(id);
    Generate i2c_gpio_phase_stop Condition:
    Stream Format:
        SCL   _____/-------------------------------
        SDA   __________/--------------------------
        width  4.7u|4.7u|4.7u from next i2c_gpio_phase_start bit
    Arguments:
        NONE
    Return value:
        long STANDBY_SUCCESS 0
        long ERR_STANDBY_I2C_SCL_LOCK 1
        long STANDBY_ERR_I2C_SDA_LOCK 2
----------------------------------------------------*/
unsigned char stby_i2c_gpio_phase_stop(void)
{
#ifdef STANDBY_EXTERNAL_PULL_HIGH
	set_sda_out();
	set_scl_out();
#endif

	set_sda_lo();
	pan_delay_us(1);
	set_scl_hi();
	pan_delay_us(1);
	if(!get_scl())
	{
		return STANDBY_ERR_I2C_SCL_LOCK;
	}

	set_sda_hi();
	pan_delay_us(1);

	if(!get_sda())
	{
		return STANDBY_ERR_I2C_SDA_LOCK;
	}

	return STANDBY_SUCCESS;
}

/*---------------------------------------------------
void i2c_gpio_phase_set_bit(id, long val);
    Set a BIT (Hi or Low)
    Stream Format:
        SCL   _____/---\
        SDA   ??AAAAAAAA
        width  4.7u| 4u|
    Arguments:
        long i    : Set(1) or Clear(0) this bit on iic bus
    Return value:
        NONE
----------------------------------------------------*/
void stby_i2c_gpio_phase_set_bit(long val)
{
#ifdef STANDBY_EXTERNAL_PULL_HIGH
	set_sda_out();
	set_scl_out();
#endif

	set_scl_lo();
	if(val)
	{
		set_sda_hi();
	}
	else
	{
		set_sda_lo();
	}

	pan_delay_us(1);
	set_scl_hi();
	pan_delay_us(1);
	set_scl_lo();
}

/*---------------------------------------------------
long i2c_gpio_phase_get_bit(id);
    Set a BIT (Hi or Low)
    Stream Format:
        SCL   _____/---\
        SDA   ??AAAAAAAA
        width  4.7u| 4u|
    Arguments:
        NONE
    Return value:
        long i    : Set(1) or Clear(0) this bit on iic bus
----------------------------------------------------*/
unsigned char stby_i2c_gpio_phase_get_bit(void)
{
	unsigned char ret = 0;

	set_sda_in();

	/*Hi Ind.*/
	set_sda_hi();
	pan_delay_us(1);

	set_scl_hi();
	pan_delay_us(1);

	ret = get_sda();
	set_scl_lo();

	return ret;
}

/*---------------------------------------------------
long i2c_gpio_phase_set_byte(unsigned long id, unsigned char data);
    Perform a byte write process
    Stream Format:
        SCL   ___/-\___/-\___/-\___/-\___/-\___/-\___/-\___/-\__/-\
        SDA   --< B7>-< B6>-< B5>-< B4>-< B3>-< B2>-< B1>-< B0>-Check
        Clock Low: 4.7u, High: 4.0u.                            Ack
        Data exchanged at CLK Low, ready at SCL High
    Arguments:
        char data    - Data to send on iic bus
    return value:
        The /ack signal returned from slave
----------------------------------------------------*/
unsigned char stby_i2c_gpio_phase_set_byte(unsigned char dat)
{
	unsigned char i = 0;

	for(i=0; i<8; i++)
	{
		if(dat & 0x80)
		{
			stby_i2c_gpio_phase_set_bit(1);
		}
		else
		{
			stby_i2c_gpio_phase_set_bit(0);
		}

		dat <<= 1;
	}

	return(stby_i2c_gpio_phase_get_bit());
}

/*---------------------------------------------------
char i2c_gpio_phase_get_byte(unsigned long id, long ack);
    Perform a byte read process
            by Charlemagne Yue
    SCL   ___/-\___/-\___/-\___/-\___/-\___/-\___/-\___/-\___/-\
    SDA   --< B7>-< B6>-< B5>-< B4>-< B3>-< B2>-< B1>-< B0>-(Ack)
    Clock Low: 4.7u, High: 4.0u.
    Data exchanged at CLK Low, ready at SCL High
----------------------------------------------------*/
unsigned char stby_i2c_gpio_phase_get_byte(long ack)
{
	unsigned char ret = 0;
	unsigned char i = 0;

	for(i=0; i<8; i++)
	{
		ret <<= 1;
		ret |= stby_i2c_gpio_phase_get_bit();
	}

	stby_i2c_gpio_phase_set_bit(ack);

	return ret;
}

/*---------------------------------------------------
long32 i2c_gpio_read_no_stop(unsigned long id, unsigned char slv_addr, unsigned char *data, unsigned long len);
    Perform bytes read process but no stop
    Stream Format:
        S<SLV_R><Read>
        S        : Start
        <SLV_R>    : Set Slave addr & Read Mode
        <Read>    : Read Data
    Arguments:
        BYTE slv_addr - Slave Address
        BYTE reg_addr - Data address
    Return value:
        Data returned
----------------------------------------------------*/
unsigned char stby_i2c_gpio_read_no_stop(unsigned char slv_addr, unsigned char *dat, long len)
{
	unsigned char i = STANDBY_I2C_GPIO_TIMES_OUT;

	slv_addr |= 1;/*Read.*/
	while(--i)/*Ack polling!!.*/
	{
		stby_i2c_gpio_phase_start();/*Start I2C bus.*/

		/*has /ACK => i2c_gpio_phase_start transfer.*/
		if(!stby_i2c_gpio_phase_set_byte(slv_addr))
		{
			break;
		}

		/*device is busy, issue i2c_gpio_phase_stop and chack again later.*/
		stby_i2c_gpio_phase_stop();
		pan_delay_us(1);/*wait for 1mS.*/
	}

	if(i == 0)
	{
		return STANDBY_ERR_TIME_OUT;
	}

	for(i=0; i<(len-1); i++)
	{
		/*with no /ack to stop process.*/
		dat[i] = stby_i2c_gpio_phase_get_byte(0);
	}

	dat[len-1] = stby_i2c_gpio_phase_get_byte(1);

	return STANDBY_SUCCESS;
}

/*---------------------------------------------------
long32 i2c_gpio_write_no_stop(unsigned long id, unsigned char slv_addr, unsigned char *data, unsigned long len);
    Perform bytes write process but no stop
    Stream Format:
        S<SLV_W><Write>
        S        : Start
        <SLV_W>    : Set Slave addr & Write Mode
        <Write>    : Send Data
    Arguments:
        BYTE slv_addr - Slave Address
        BYTE value    - data to write
    Return value:
        NONE
----------------------------------------------------*/
long stby_i2c_gpio_write_no_stop(unsigned char slv_addr, unsigned char *dat, long len)
{
	unsigned char i = STANDBY_I2C_GPIO_TIMES_OUT;

	slv_addr &= 0xFE;/*Write.*/
	while(--i)/*Ack polling!!.*/
	{
		stby_i2c_gpio_phase_start();

		/*has /ACK => i2c_gpio_phase_start transfer.*/
		if(!stby_i2c_gpio_phase_set_byte(slv_addr))
		{
			break;
		}

		/*device is busy, issue i2c_gpio_phase_stop and chack again later.*/
		stby_i2c_gpio_phase_stop();
		pan_delay_us(1);/* wait for 1mS */
	}

	if(i == 0)
	{
		return STANDBY_ERR_TIME_OUT;
	}

	for(i=0; i<len; i++)
	{
		stby_i2c_gpio_phase_set_byte(dat[i]);
	}

	return STANDBY_SUCCESS;
}

/*---------------------------------------------------
long32 i2c_gpio_read(unsigned long id, unsigned char slv_addr, unsigned char *data, unsigned long len);
    Perform a byte read process
    Stream Format:
        S<SLV_R><Read>P
        S        : Start
        P        : Stop
        <SLV_R>    : Set Slave addr & Read Mode
        <Read>    : Read Data
    Arguments:
        BYTE slv_addr - Slave Address
        BYTE reg_addr - Data address
    Return value:
        Data returned
----------------------------------------------------*/
unsigned char stby_i2c_gpio_read(unsigned char slv_addr, unsigned char *dat, long len)
{
	unsigned char ret = STANDBY_ERR_FAILURE;

	if(STANDBY_SUCCESS != stby_i2c_gpio_read_no_stop( slv_addr, dat, len))
	{
		return ret;
	}
	stby_i2c_gpio_phase_stop();

	return STANDBY_SUCCESS;
}

/*---------------------------------------------------
long32 i2c_gpio_write(unsigned char slv_addr, unsigned char *data, unsigned long len);
    Perform bytes write process
    Stream Format:
        S<SLV_W><Write>P
        S        : Start
        P        : Stop
        <SLV_W>    : Set Slave addr & Write Mode
        <Write>    : Send Data
    Arguments:
        BYTE slv_addr - Slave Address
        BYTE value    - data to write
    Return value:
        NONE
----------------------------------------------------*/
void stdby_i2c_gpio_write(unsigned char slv_addr, unsigned char *dat, long len)
{
	if(STANDBY_SUCCESS != stby_i2c_gpio_write_no_stop(slv_addr, dat, len))
	{
		return;
	}

	stby_i2c_gpio_phase_stop();
}
