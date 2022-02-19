#ifndef __CH455_GPIO_H__
#define __CH455_GPIO_H__
//==============================================================================================//

#define HAL_GPIO_IER_REG (0xB8000044)
#define HAL_GPIO_REC_REG (0xB8000048)
#define HAL_GPIO_FEC_REG (0xB800004C)
#define HAL_GPIO_ISR_REG (0xB800005C)
#define HAL_GPIO_DIR_REG (0xB8000058)
#define HAL_GPIO_DI_REG (0xB8000050)
#define HAL_GPIO_DO_REG (0xB8000054)
#define HAL_GPIO_EN_REG (0xB8000430)

#define HAL_GPIO1_IER_REG (0xB80000C4)
#define HAL_GPIO1_REC_REG (0xB80000C8)
#define HAL_GPIO1_FEC_REG (0xB80000CC)
#define HAL_GPIO1_ISR_REG (0xB80000DC)
#define HAL_GPIO1_DIR_REG (0xB80000D8)
#define HAL_GPIO1_DI_REG (0xB80000D0)
#define HAL_GPIO1_DO_REG (0xB80000D4)
#define HAL_GPIO1_EN_REG (0xB8000434)

#define HAL_GPIO2_IER_REG (0xB80000E4)
#define HAL_GPIO2_REC_REG (0xB80000E8)
#define HAL_GPIO2_FEC_REG (0xB80000EC)
#define HAL_GPIO2_ISR_REG (0xB80000FC)
#define HAL_GPIO2_DIR_REG (0xB80000F8)
#define HAL_GPIO2_DI_REG (0xB80000F0)
#define HAL_GPIO2_DO_REG (0xB80000F4)
#define HAL_GPIO2_EN_REG (0xB8000438)

#define HAL_GPIO3_IER_REG (0xB8000344)
#define HAL_GPIO3_REC_REG (0xB8000348)
#define HAL_GPIO3_FEC_REG (0xB800034C)
#define HAL_GPIO3_ISR_REG (0xB800035C)
#define HAL_GPIO3_DIR_REG (0xB8000358)
#define HAL_GPIO3_DI_REG (0xB8000350)
#define HAL_GPIO3_DO_REG (0xB8000354)
#define HAL_GPIO3_EN_REG (0xB800043C)

#define HAL_GPIO4_IER_REG (0xB8000444)
#define HAL_GPIO4_REC_REG (0xB8000448)
#define HAL_GPIO4_FEC_REG (0xB800044C)
#define HAL_GPIO4_ISR_REG (0xB800045C)
#define HAL_GPIO4_DIR_REG (0xB8000458)
#define HAL_GPIO4_DI_REG (0xB8000450)
#define HAL_GPIO4_DO_REG (0xB8000454)
#define HAL_GPIO4_EN_REG (0xB8000440)

#define HAL_GPIO_O_DIR (1)
#define HAL_GPIO_I_DIR (0)
#define HAL_GPIO_SET_HI (1)
#define HAL_GPIO_SER_LOW (0)

#define SETING_ADDR (0x48)
#define DIG0_ADDR (0x68)
#define DIG1_ADDR (0x6A)
#define DIG2_ADDR (0x6C)
#define DIG3_ADDR (0x6E)
#define KEY_ADDR (0x4F)
#define CH455_MODE (0x1)
#define STANDBY_EXTERNAL_PULL_HIGH (1)
#define STANDBY_ERR_I2C_SCL_LOCK (1)
#define STANDBY_ERR_I2C_SDA_LOCK (1)
#define STANDBY_I2C_GPIO_TIMES_OUT (10)
#define STANDBY_ERR_TIME_OUT (-34)/*Waiting time out.*/
#define STANDBY_ERR_FAILURE (0xFF)/*Common error, operation not success.*/   
#define STANDBY_SUCCESS (0)/*Common error, operation not success.*/
//====================================================================//

extern void pan_ch455_set_val_gpio(unsigned long gpio_pin, unsigned char val);
extern void pan_ch455_en_gpio(unsigned long gpio_pin, unsigned char en);
extern void pan_ch455_dir_set_gpio(unsigned long gpio_pin, unsigned char en);
extern void pan_delay_us(unsigned long i);
extern void stby_pan_init(void);
extern unsigned char stby_scan_key(void);
extern void stby_show_off(void);
extern void stby_show_time(unsigned char hour, unsigned char min, unsigned char sec);
extern unsigned char stby_i2c_gpio_phase_start(void);
extern unsigned char stby_i2c_gpio_phase_stop(void);
extern void stby_i2c_gpio_phase_set_bit(long val);
extern unsigned char stby_i2c_gpio_phase_get_bit(void);
extern unsigned char stby_i2c_gpio_phase_set_byte(unsigned char dat);
extern unsigned char stby_i2c_gpio_phase_get_byte(long ack);
extern unsigned char stby_i2c_gpio_read_no_stop( unsigned char slv_addr, unsigned char *dat, long len);
extern long stby_i2c_gpio_write_no_stop( unsigned char slv_addr, unsigned char *dat, long len);
extern unsigned char stby_i2c_gpio_read( unsigned char slv_addr, unsigned char *dat, long len);
extern void stdby_i2c_gpio_write( unsigned char slv_addr, unsigned char *dat, long len);
//==============================================================================================//

/*ch455 led map.*/
//{'.', 0x80},
//{'0', 0x3f}, {'1', 0x06}, {'2', 0x5b}, {'3', 0x4f},
//{'4', 0x66}, {'5', 0x6d}, {'6', 0x7d}, {'7', 0x07},
//{'8', 0x7f}, {'9', 0x6f}, {'a', 0x77}, {'A', 0x77},
//{'b', 0x7c}, {'B', 0x7c}, {'c', 0x39}, {'C', 0x39},
//{'d', 0x5e}, {'D', 0x5e}, {'e', 0x79}, {'E', 0x79},
//{'f', 0x71}, {'F', 0x71}, {'g', 0x6f}, {'G', 0x3d},
//{'h', 0x76}, {'H', 0x76}, {'i', 0x04}, {'I', 0x30},
//{'j', 0x0e}, {'J', 0x0e}, {'l', 0x38}, {'L', 0x38},
//{'n', 0x54}, {'N', 0x37}, {'o', 0x5c}, {'O', 0x3f},
//{'p', 0x73}, {'P', 0x73}, {'q', 0x67}, {'Q', 0x67},
//{'r', 0x50}, {'R', 0x77}, {'s', 0x6d}, {'S', 0x6d},
//{'t', 0x78}, {'T', 0x31}, {'u', 0x3e}, {'U', 0x3e},
//{'y', 0x6e}, {'Y', 0x6e}, {'z', 0x5b}, {'Z', 0x5b},
//{':', 0x80}, {'-', 0x40}, {'_', 0x08}, {' ', 0x00},
#endif
