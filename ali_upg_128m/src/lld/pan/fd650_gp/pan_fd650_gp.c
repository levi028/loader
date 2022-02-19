/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2005 Copyright (C)
*
*    File:    pan_pt6964.c
*
*    Description:    This file contains all functions definition
*		             of Front Panel driver.
*
*    Note:
*    This driver is used to support pt6964 front panel controller. 
*
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	2007.5.25      Goliath Peng        Ver 0.1     Create file.
*****************************************************************************/

#include <sys_config.h>

#include <retcode.h>
#include <types.h>

#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <hal/hal_gpio.h>
#include <hld/hld_dev.h>
#include <hld/pan/pan.h>
#include <hld/pan/pan_dev.h>

typedef unsigned char   u_int8;
typedef unsigned short  u_int16;	
typedef unsigned long 		u_int32;

/* *************************************硬件相关*********************************************** */
//#define		fd650_delay()		osal_delay(1)
/* i.e.
void Delay(void)
{
	int iCount = 150;
	while( iCount )
	{
		iCount--;
		if(iCount == 0)
			return;
	}
}
*/

/* 接口的PIO连接,与实际电路有关 */
#define	FD650_SCL        (24)       //定义IO端口
#define	FD650_SDA        (23)       //定义IO端口

//#define HIGH 1
//#define LOW  0

/* 接口的PIO操作，与平台IO操作有关 */
/* **************************************硬件相关*********************************************** */

/* ********************************************************************************************* */
// 设置系统参数命令

#define FD650_BIT_ENABLE	0x01		// 开启/关闭位
#define FD650_BIT_SLEEP		0x04		// 睡眠控制位
//#define FD650_BIT_7SEG		0x08		// 7段控制位
#define FD650_BIT_INTENS1	0x10		// 1级亮度
#define FD650_BIT_INTENS2	0x20		// 2级亮度
#define FD650_BIT_INTENS3	0x30		// 3级亮度
#define FD650_BIT_INTENS4	0x40		// 4级亮度
#define FD650_BIT_INTENS5	0x50		// 5级亮度
#define FD650_BIT_INTENS6	0x60		// 6级亮度
#define FD650_BIT_INTENS7	0x70		// 7级亮度
#define FD650_BIT_INTENS8	0x00		// 8级亮度

#define FD650_SYSOFF		0x0400			// 关闭显示、关闭键盘
#define FD650_SYSON			( FD650_SYSOFF | FD650_BIT_ENABLE )	// 开启显示、键盘
#define FD650_SLEEPOFF		FD650_SYSOFF	// 关闭睡眠
#define FD650_SLEEPON		( FD650_SYSOFF | FD650_BIT_SLEEP )	// 开启睡眠
//#define FD650_7SEG_ON		( FD650_SYSON | FD650_BIT_7SEG )	// 开启七段模式
#define FD650_8SEG_ON		( FD650_SYSON | 0x00 )	// 开启八段模式
#define FD650_SYSON_1		( FD650_SYSON | FD650_BIT_INTENS1 )	// 开启显示、键盘、1级亮度
//以此类推
#define FD650_SYSON_4		( FD650_SYSON | FD650_BIT_INTENS4 )	// 开启显示、键盘、4级亮度
//以此类推
#define FD650_SYSON_8		( FD650_SYSON | FD650_BIT_INTENS8 )	// 开启显示、键盘、8级亮度


// 加载字数据命令
#define FD650_DIG0			0x1400			// 数码管位0显示,需另加8位数据
#define FD650_DIG1			0x1500			// 数码管位1显示,需另加8位数据
#define FD650_DIG2			0x1600			// 数码管位2显示,需另加8位数据
#define FD650_DIG3			0x1700			// 数码管位3显示,需另加8位数据

#define FD650_DOT			0x0080			// 数码管小数点显示

#define FD650_DOT_ON			0x0080			// 数码管小数点显示
#define FD650_DOT_OFF			0x0000			// 数码管小数点显示
// 读取按键代码命令
#define FD650_GET_KEY	0x0700					// 获取按键,返回按键代码


static struct led_bitmap BCD_decode_tab[] =
{	
	{'.', 0x80}, /* Let's put the dot bitmap into the table */
	{'0', 0x3f}, {'1', 0x06}, {'2', 0x5b}, {'3', 0x4f}, 
	{'4', 0x66}, {'5', 0x6d}, {'6', 0x7d}, {'7', 0x07}, 
	{'8', 0x7f}, {'9', 0x6f}, {'a', 0x5f}, {'A', 0x77}, 
	{'b', 0x7c}, {'B', 0x7c}, {'c', 0x39}, {'C', 0x39}, 
	{'d', 0x5e}, {'D', 0x5e}, {'e', 0x79}, {'E', 0x79}, 
	{'f', 0x71}, {'F', 0x71}, {'g', 0x6f}, {'G', 0x3d}, 
	{'h', 0x76}, {'H', 0x76}, {'i', 0x04}, {'I', 0x30}, 
	{'j', 0x0e}, {'J', 0x0e}, {'l', 0x38}, {'L', 0x38}, 
	{'n', 0x54}, {'N', 0x37}, {'o', 0x5c}, {'O', 0x3f}, 
	{'p', 0x73}, {'P', 0x73}, {'q', 0x67}, {'Q', 0x67}, 
	{'r', 0x50}, {'R', 0x77}, {'s', 0x6d}, {'S', 0x6d}, 
	{'t', 0x78}, {'T', 0x31}, {'u', 0x3e}, {'U', 0x3e}, 
	{'y', 0x6e}, {'Y', 0x6e}, {'z', 0x5b}, {'Z', 0x5b}, 
	{':', 0x80}, {'-', 0x40}, {'_', 0x08}, {' ', 0x00},
};//BCD码字映射

struct key_bitmap
{
	UINT8 bitmap;
	UINT32 key_value;
};
#define PAN_GP_HKEY_POWER		0xffff0080
#define PAN_GP_HKEY_MENU		0xffff0002
#define PAN_GP_HKEY_ENTER		0xffff0008
#define PAN_GP_HKEY_EXIT		0xffff0020
#define PAN_GP_HKEY_UP			0xffff0001
#define PAN_GP_HKEY_DOWN		0xffff0004
#define PAN_GP_HKEY_LEFT		0xffff0010
#define PAN_GP_HKEY_RIGHT		0xffff0040


struct key_bitmap BCD_key_tab[] = 
{
	{0x77,PAN_GP_HKEY_POWER}, 
	{0x57,PAN_GP_HKEY_ENTER}, 
	{0x5f,PAN_GP_HKEY_MENU}, 
	//{0x67,PAN_GP_HKEY_UP}, 
	//{0x6f,PAN_GP_HKEY_DOWN}, 
	//{0x47,PAN_GP_HKEY_LEFT}, 
	//{0x4f,PAN_GP_HKEY_RIGHT}, 
	{0x4f,PAN_GP_HKEY_UP}, 
	{0x47,PAN_GP_HKEY_DOWN}, 
	{0x67,PAN_GP_HKEY_LEFT}, 
	{0x6f,PAN_GP_HKEY_RIGHT}, 
	{0x66,PAN_GP_HKEY_EXIT}, 
};

#define FD650_LED_MAP_NUM sizeof(BCD_decode_tab)/sizeof(struct led_bitmap)
#define FD650_KEY_MAP_NUM sizeof(BCD_key_tab)/sizeof(struct key_bitmap)

static struct pan_fd650_gp_info
{
	struct pan_hw_info *hw_info;
	int					bitmap_len;
	struct led_bitmap  *bitmap_list;
    	OSAL_ID                task_id;
	UINT8				dis_buff[8];
	UINT32				dis_id;
	UINT8				led_buff;
	UINT8				dotmap;
	UINT8				colonmap;
	UINT8				blankmap;
	UINT32				cnt_repeat_first;
	UINT32				cnt_repeat;
	UINT32				key_cnt;			/* Continue press key times */
	UINT32				keypress_cnt;		/* Continue press key counter */
	UINT32 				keypress_intv;		/* Continue press key interval */
	UINT32 				keypress_num;		/* Pressed key COM index saver */
	UINT32				keypress_bak;		/* Pressed key saver */
	UINT8 				cur_dis_mem[7][2];
	UINT8 				new_dis_mem[7][2];
} pan_fd650_gp_data;
static char  pan_fd650_gp_name[HLD_MAX_NAME_SIZE] = "PAN_FD650_GP_0";
static unsigned char  	fd_650_scl   = FD650_SCL;
static unsigned char 	fd_650_sda  = FD650_SDA;
void pan_fd650_gp_interrupt(UINT32 param);
void pan_fd650_gp_tsak(UINT32 param);

static void fd650_delay(void)
{
	osal_delay(7);
}

static void fd650_scl_set(void)
{
	HAL_GPIO_BIT_SET(fd_650_scl, 1);
}

static void fd650_scl_clr(void)
{
	HAL_GPIO_BIT_SET(fd_650_scl, 0);
}

static void fd650_scl_d_out(void)
{
	HAL_GPIO_BIT_DIR_SET(fd_650_scl, HAL_GPIO_O_DIR);
}
static void fd650_sda_set(void)
{
	HAL_GPIO_BIT_SET(fd_650_sda, 1);
}
static void fd650_sda_clr(void)
{
	HAL_GPIO_BIT_SET(fd_650_sda, 0);
}
static  unsigned char fd650_sda_in(void)
{
	HAL_GPIO_BIT_DIR_SET(fd_650_sda, HAL_GPIO_I_DIR);
	return HAL_GPIO_BIT_GET(fd_650_sda);
}

static void fd650_sda_d_out(void)
{
	HAL_GPIO_BIT_DIR_SET(fd_650_sda, HAL_GPIO_O_DIR);
}
static void fd650_sda_d_in(void)
{
	HAL_GPIO_BIT_DIR_SET(fd_650_sda, HAL_GPIO_I_DIR);
}

/****************************************************************
 *
 *	Function Name:fd650_start
 *
 *	Description:
 *
 *  Parameter:
 *
 *  return：
****************************************************************/
void fd650_start( void )
{
	fd650_sda_set();  
	fd650_sda_d_out();
	fd650_scl_set();
	fd650_scl_d_out();
	fd650_delay();
	fd650_sda_clr();
	fd650_delay();      
	fd650_scl_clr();
}
/****************************************************************
 *
 *	Function Name:fd650_stop
 *
 *	Description:
 *
 *  Parameter:
 *
 *  return：
****************************************************************/
void fd650_stop( void )
{
	fd650_sda_clr();
	fd650_sda_d_out();
	fd650_delay();
	fd650_scl_set();
	fd650_delay();
	fd650_sda_set();
	fd650_delay();
	fd650_sda_d_in();
}

/****************************************************************
 *
 *	Function Name:fd650_write_byte
 *
 *	Description: Write One Byte data
 *
 *  Parameter: data
 *
 *  return：
****************************************************************/
void fd650_write_byte( u_int8 dat )
{
	u_int8 i;
	fd650_sda_d_out();
	for( i = 0; i != 8; i++ )
	{
		if( dat & 0x80 ) 
		{
		    fd650_sda_set();
		}
		else 
		{
		    fd650_sda_clr();
		}
		fd650_delay();
		fd650_scl_set();
		dat <<= 1;
		fd650_delay();  // 可选延时
		fd650_scl_clr();
	}
	fd650_sda_set();
	fd650_sda_d_in();
	fd650_delay();
	fd650_scl_set();
	fd650_delay();
	fd650_scl_clr();
}
/****************************************************************
 *
 *	Function Name:fd650_write_byte
 *
 *	Description: Read One Byte data
 *
 *  Parameter: 
 *
 *  return：data
****************************************************************/
u_int8  fd650_read_byte( void )
{
	u_int8 dat,i;
	fd650_sda_set();
	fd650_sda_d_in();
	dat = 0;
	for( i = 0; i != 8; i++ )
	{
		fd650_delay();  // 可选延时
		fd650_scl_set();
		fd650_delay();  // 可选延时
		dat <<= 1;
		if( fd650_sda_in() ) dat++;
		fd650_scl_clr();
	}
	fd650_sda_set();
	fd650_delay();
	fd650_scl_set();
	fd650_delay();
	fd650_scl_clr();
	return dat;
}

/****************************************FD650操作函数*********************************************/

/****************************************************************
 *
 *	Function Name:fd650_wirte
 *
 *	Description:通过发送命令参数
 *
 *  Parameter: cmd见FD650.H
 *
 *  return：无
****************************************************************/
void fd650_wirte( u_int16 cmd )	//写命令
{
	fd650_start(); 
	fd650_write_byte(((u_int8)(cmd>>7)&0x3E)|0x40);
	fd650_write_byte((u_int8)cmd);
	fd650_stop();

	return;
}

/****************************************************************
 *
 *	Function Name:fd650_read
 *
 *	Description:读取按键按下状态的键值，如读到无效按键值返回0
 *
 *  Parameter: 无
 *
 *  return：按键按下状态的键值
****************************************************************/
u_int8 fd650_read( void )		//读取按键
{
	u_int8 keycode = 0;
	
	fd650_start(); 
	fd650_write_byte((u_int8)(FD650_GET_KEY>>7)&0x3E|0x01|0x40);
	keycode=fd650_read_byte();
	fd650_stop();
	if(keycode&0x00000040 ==0)
		keycode = 0;

	return keycode;
}


/****************************************************************
 *
 *	Function Name:fd650_led_get_code
 *
 *	Description:转换字符为数码管的显示码，如遇到无法转换的字符返回0
 *
 *  Parameter: cTemp  待转换为显示码的字符
 *
 *  return：显示码值
****************************************************************/
static u_int8 fd650_led_get_code(char cTemp)
{
	u_int8 i, bitmap=0x00;

	for(i=0; i<FD650_LED_MAP_NUM; i++)
	{
		if(BCD_decode_tab[i].character == cTemp)
		{
			bitmap = BCD_decode_tab[i].bitmap;
			break;
		}
	}

	return bitmap;
}

static u_int32 fd650_key_get_value(u_int8 cTemp)
{
	u_int32 i;
	u_int32 key_value=PAN_KEY_INVALID;

	for(i=0; i<FD650_KEY_MAP_NUM; i++)
	{
		if(BCD_key_tab[i].bitmap == cTemp)
		{
			key_value = BCD_key_tab[i].key_value;
			break;
		}
	}
	return key_value;
}

/****************************************************************
 *
 *	Function Name:Led_Show_650
 *
 *	Description:简单的数码管显示应用示例
 *
 *  Parameter: 	acFPStr  		显示的字符串；
 *							sec_flag	  开启小数点标志位；
 *							Lock				开启小数点标志位；
 *
 *  return：无
****************************************************************/
void fd650_led_show( char *acFPStr, unsigned char sec_flag, unsigned char Lock)
{
	int i, iLenth;
	int	data[4]={0x00, 0x00, 0x00, 0x00};
	if( STRCMP(acFPStr, "") == 0 )
	{
		return;
	}
	iLenth = STRLEN(acFPStr);
	if(iLenth>4)
		iLenth = 4;
	
	/*for(i=0; i<iLenth; i++)
	{
		data[i] = fd650_led_get_code(acFPStr[i]);
	}*/
	fd650_wirte(FD650_SYSON_8);// 开启显示和键盘，8段显示方式
	//发显示数据
	fd650_wirte( FD650_DIG0 | (u_int8)data[0] );	//点亮第一个数码管
	if(sec_flag)
		fd650_wirte( FD650_DIG1 | (u_int8)data[1] | FD650_DOT ); //点亮第二个数码管
	else
		fd650_wirte( FD650_DIG1 | (u_int8)data[1] ); 
	if(Lock)
		fd650_wirte( FD650_DIG2 | (u_int8)data[2] | FD650_DOT ); //点亮第三个数码管
	else
		fd650_wirte( FD650_DIG2 | (u_int8)data[2] );
	fd650_wirte( FD650_DIG3 | (u_int8)data[3] ); //点亮第四个数码管
}

/******************************************************************************
Name        	:	fd650_key_scan
Description 	:	650前面板 按键扫描底层硬件驱动
Version  		:  
Returns     	:
Notes			:			     
*******************************************************************************/
unsigned char fd650_key_scan( void )
{
#if 0
	unsigned char temp,keycode = 0;
	fd650_start(); 
	fd650_write_byte((unsigned char)(FD650_GET_KEY>>7)&0x3E|0x01|0x40);
	temp = fd650_read_byte();
	fd650_stop();
	fd650_start(); //
	fd650_write_byte((unsigned char)(FD650_GET_KEY>>7)&0x3E|0x01|0x40);
	keycode = fd650_read_byte();
	fd650_stop();
	//FD650_Write( FD650_SYSON_8 );// 开启显示和键盘，8段显示方式
	//if(keycode!=0&&keycode!=0xff)

	if(temp == keycode)
	{
		//PT_DEBUG_PRINT(("keycode = %x \n",(HR_WORD)keycode));
		return fd650_led_get_code(keycode);
	}
	return 0;
#else
    	unsigned char temp,keycode = 0;

	temp 	= fd650_read();
	keycode 	= fd650_read();
	if(temp == keycode)
		return keycode;
	else
		return 0;
#endif
}
void fd650_tuner_on_lock(UINT8 led2_value)
{	
    fd650_wirte( FD650_DIG3 | led2_value|FD650_DOT_ON );
}

void fd650_tuner_off_lock(UINT8 led2_value)
{	
    fd650_wirte( FD650_DIG3 | led2_value|FD650_DOT_OFF );
}

static void pan_fd650_gp_set_mode(UINT16 mode)
{
	osal_interrupt_disable();
	fd650_wirte(mode);
	osal_interrupt_enable();			
}

static void pan_fd650_gp_led_onoff(UINT8 on_off) // 1: on, 0: off
{
	UINT16 cmd;
	if(on_off)
		cmd = FD650_SYSON;
	else
		cmd = FD650_SYSOFF;
	osal_interrupt_disable();
	fd650_wirte(cmd);
	osal_interrupt_enable();
}

static void pan_fd650_gp_update_led(UINT8 * byte, UINT8 len,UINT8 dot)
{
 	UINT8 i;
	UINT16 cmd;
	
	if(len > 4)
		len = 4;
	cmd = FD650_DIG0;	

	osal_interrupt_disable();
	fd650_wirte(FD650_SYSON_8);// 开启显示和键盘，8段显示方式
	//发显示数据

	for(i=0;i<len;i++)
	{		
		fd650_wirte( cmd | (u_int8)byte[i] );
		cmd += 0x100;
	}
	if(dot)
		fd650_wirte( FD650_DIG1 | (u_int8)byte[1] | FD650_DOT ); 
	osal_interrupt_enable();

}

static void pan_fd650_gp_led_init(struct pan_device *dev)
 {
 	struct pan_fd650_gp_info *tp = (struct pan_fd650_gp_info *)dev->priv;
 	UINT8 i; 
	pan_fd650_gp_led_onoff(1);
	for(i = 0; i<8; i++)
	{
		tp->dis_buff[i] = tp->blankmap;
	}
	pan_fd650_gp_update_led(tp->dis_buff, 4,0);
 }

static void pan_fd650_gp_read_key(UINT8 * byte, UINT8 len)
 {
	osal_interrupt_disable();
	* byte =fd650_key_scan();
	osal_interrupt_enable();
 }

static INT32 pan_fd650_gp_ioctl(struct pan_device *dev, INT32 cmd,
								UINT32 param)
{
	switch (cmd)
	{
		case PAN_DRIVER_ATTACH:
			break;
		case PAN_DRIVER_SUSPEND:
			break;
		case PAN_DRIVER_RESUME:
			break;
		case PAN_DRIVER_DETACH:
			break;
		case PAN_DRIVER_READ_LNB_POWER:
			break;
		default:
			break;
	}

	return SUCCESS;
}

/*
* Name          :   pan_fd650_gp_open()
* Description   :   Open front panel
* Parameter     :   struct pan_device *dev
* Return        :   void
*/
static INT32  pan_fd650_gp_open(struct pan_device *dev)
{
	struct pan_fd650_gp_info *tp = (struct pan_fd650_gp_info *)dev->priv;
	struct pan_hw_info *hp = tp->hw_info;
	UINT8 led_data = 0;
	int i;

	tp->keypress_intv 		= tp->cnt_repeat_first/10;
	tp->keypress_bak 		= PAN_KEY_INVALID;
	tp->keypress_num 	= tp->hw_info->num_com;
	tp->keypress_cnt 		= 0;
	tp->key_cnt 			= 0;

	/* Init LBD to invalid state */
	if (hp->type_kb & 0x02)
	{
		for (i = 0; i < 4; i++)
		{
			if (hp->lbd[i].polar == 0)	/* Set to invalidate status */
				led_data |= (1 << hp->lbd[i].position);
		}
		led_data &= 0xff;
	}
	pan_fd650_gp_data.led_buff = led_data;
	pan_fd650_gp_led_init(dev);
#ifdef _DEBUG_VERSION_
	if (tp->hw_info->type_irp != 0)
		irc_m6303irc_init(tp->hw_info);
#endif
#if 0
	/* Register an 1mS cycle interrupt ISR */
	osal_interrupt_register_lsr(7, pan_fd650_gp_interrupt, (UINT32)dev);
#else
	OSAL_T_CTSK task_param;

	//creat ch455 task for keyboard scan.
	task_param.task   =  (FP)pan_fd650_gp_tsak;
	task_param.stksz  =  0x1000;
	task_param.quantum = 10;
	task_param.itskpri = OSAL_PRI_NORMAL;
	task_param.para1= (UINT32)dev;
	task_param.para2 = 0;

	tp->task_id = osal_task_create(&task_param);
	if(OSAL_INVALID_ID == tp->task_id)
	{
		return ERR_FAILUE;
	}
#endif	
	return SUCCESS;
}


/*
* Name          :   pan_fd650_gp_close()
* Description   :   close front panel
* Parameter     :   struct pan_device *dev
* Return        :   void
*/
static INT32  pan_fd650_gp_close(struct pan_device *dev)
{
	struct pan_fd650_gp_info *tp = (struct pan_fd650_gp_info *)dev->priv;
	int i;

#ifdef _DEBUG_VERSION_
	if (tp->hw_info->type_irp != 0)
		irc_m6303irc_close();
#endif

	/* Un-register an 1mS cycle interrupt ISR */
#if 0
	osal_interrupt_unregister_lsr(7, pan_fd650_gp_interrupt);
#else
	if(OSAL_INVALID_ID == tp->task_id)
	{
		osal_task_delete(tp->task_id);
		tp->task_id = OSAL_INVALID_ID;
	}
#endif	
	/* Clear last displayed char map. */
	for(i = 0; i<8; i++)
	{
		tp->dis_buff[i] = tp->blankmap;
	}
	pan_fd650_gp_update_led(tp->dis_buff, 4,0);
	pan_fd650_gp_led_onoff(0);
	return SUCCESS;
}

/*
* Name          :   pan_fd650_gp_esc_command()
* Description   :   Do ESC command
* Parameter     :   UINT8* data			: Command data
*                   UINT32 limit_len	: Length limit
* Return        :   UINT32				: command length
*/
	static BOOL lock_led = FALSE;
	static BOOL standby_led = FALSE;

static UINT32 pan_fd650_gp_esc_command(struct pan_device *dev, UINT8 *data, UINT32 limit_len)
{
	struct pan_fd650_gp_info *tp = (struct pan_fd650_gp_info *)dev->priv;
	struct pan_gpio_info *gp;
	UINT32 dp;
	/* Search ESC command untill no-ESC or reached data limit */
	for (dp = 0; dp < limit_len && data[dp] == 27; dp += 4)
	{	/* LBD operate command */
		if (PAN_ESC_CMD_LBD == data[dp + 1] || 'l' == data[dp + 1])
		{
			int i;
			switch(data[dp+2])
			{
				case PAN_ESC_CMD_LBD_POWER :
					
					break;
					
				case PAN_ESC_CMD_LBD_LOCK :
					if (data[dp+3] == PAN_ESC_CMD_LBD_ON)
					{
						lock_led = TRUE;
					}
					else if (data[dp+3] == PAN_ESC_CMD_LBD_OFF)
					{
						lock_led = FALSE;
					}	
					osal_interrupt_disable();
					fd650_wirte(FD650_SYSON_8);// 开启显示和键盘，8段显示方式
					if (lock_led)
						 fd650_wirte( FD650_DIG3 |tp->dis_buff[3]|FD650_DOT_ON );
					else
						 fd650_wirte( FD650_DIG3 |tp->dis_buff[3]|FD650_DOT_OFF );
					osal_interrupt_enable();					
					break;
			
   				case PAN_ESC_CMD_LBD_FUNCA ://standby
					if (data[dp+3] == PAN_ESC_CMD_LBD_ON)
					{
						standby_led = TRUE;
					}
					else if (data[dp+3] == PAN_ESC_CMD_LBD_OFF)
					{
						standby_led = FALSE;
					}			
					break;
					
   				case PAN_ESC_CMD_LBD_FUNCB :
					break;					
					
				case PAN_ESC_CMD_LBD_LEVEL :
					break;
					
				default :
					break;
			}
				 
		}
		else if (PAN_ESC_CMD_LED == data[dp + 1] || 'e' == data[dp + 1])
		{
			tp->dis_buff[data[dp + 2]] = data[dp + 3];
		}
	}
	return dp;
}


/*
* Name          :   pan_fd650_gp_char_map()
* Description   :   Do string display
* Parameter     :   UINT8* data			: Command data
*                   UINT32 limit_len	: Length limit
* Return        :   UINT32				: string length
*/
static UINT32 pan_fd650_gp_char_map(struct pan_device *dev,
											 UINT8 *data, UINT32 len)
{
	struct pan_fd650_gp_info *tp = (struct pan_fd650_gp_info *)dev->priv;
	UINT32 pdata, pbuff;
	int i, j;
	UINT8 dot_flag = 0;
	UINT8 col_flag = 0;

	pdata = 0;
	pbuff = 0;
	tp->dis_buff[tp->hw_info->pos_colon] = tp->blankmap;/* Clear colon flag */
	MEMSET(tp->dis_buff,0,8);
	while (pdata < len && data[pdata] != 27 && pbuff < dev->led_num)
	{
		if (data[pdata] == ':')		/* Process colon charactor */
		{
			//col_flag 		= 1;
			dot_flag 	= 1;
			pdata++;
			continue;
		}
		if (data[pdata] == '.')		/* Process dot charactor */
		{
			tp->dis_buff[pbuff] = tp->dotmap;
			pdata++;
			pbuff++;
			continue;
		}
		/* Generate the bitmap */
		//tp->dis_buff[pbuff] = fd650_led_get_code(data[pdata])|tp->blankmap;	/* Pre-set buffer to NULL */
		//tp->dis_buff[pbuff] = tp->blankmap;	/* Pre-set buffer to NULL */
		for (j = 0; j < tp->bitmap_len; j++)
		{
			if(data[pdata] == tp->bitmap_list[j].character)
			{
				tp->dis_buff[pbuff] |= tp->bitmap_list[j].bitmap;
				break;
			}
		}
		
		/* Move to next position */
		pbuff++;
		if (data[pdata + 1] == '.' && (pdata + 1) < len)
		{
			//dot_flag = 1;
			pdata += 2;
		} else
		{
			pdata += 1;
		}
		if(pbuff == 7)
			break;
	}
	
	pan_fd650_gp_update_led(tp->dis_buff, 4,dot_flag);

	if (lock_led)
		 fd650_wirte( FD650_DIG3 |tp->dis_buff[3]|FD650_DOT_ON );
	else
		 fd650_wirte( FD650_DIG3 |tp->dis_buff[3]|FD650_DOT_OFF );

	/* Display buffer full, stop display process */
	if (data[pdata] != 27 && pbuff == dev->led_num)
	{
		return len;
	}
	return pdata;
}

/*
* Name          :   pan_fd650_gp_display()
* Description   :   Set display data
* Parameter     :   char* disp
* Return        :   void
*/
static void pan_fd650_gp_display(struct pan_device *dev, char *data, UINT32 len)
{
	struct pan_fd650_gp_info *tp = (struct pan_fd650_gp_info *)dev->priv;
	UINT32 pdata = 0;

	if (tp->hw_info->hook_show)
		len = tp->hw_info->hook_show(dev, data, len);

	while (pdata < len)
	{
		if (data[pdata] == 27)			/* ESC command */
		{
			pdata += pan_fd650_gp_esc_command(dev, &(data[pdata]), len - pdata);
		} else							/* String display */
		{
			pdata += pan_fd650_gp_char_map(dev, &(data[pdata]), len - pdata);
		}
	}
}

/*
 * 	Name		:   pan_fd650_gp_attach()
 *	Description	:   Panel init funciton should be called in
 *                  system boot up.
 *	Parameter	:	None
 *	Return		:	INT32				: return value
 *
 */
__ATTRIBUTE_REUSE_
INT32 pan_fd650_gp_attach(struct pan_configuration *config)
{
	struct pan_device *dev;
	struct pan_fd650_gp_info *tp;
	int i, j;

	if (config == NULL || config->hw_info == NULL)
	{
		return ERR_FAILUE;
	}
	dev = dev_alloc(pan_fd650_gp_name, HLD_DEV_TYPE_PAN, sizeof(struct pan_device));
	if (dev == NULL)
	{
		PRINTF("Error: Alloc front panel device error!\n");
		return ERR_NO_MEM;
	}
	/* Alloc structure space of private */
	MEMSET(&pan_fd650_gp_data, 0, sizeof(struct pan_fd650_gp_info));

	if (config->bitmap_list == NULL)
	{
		pan_fd650_gp_data.bitmap_len = FD650_LED_MAP_NUM;
		pan_fd650_gp_data.bitmap_list = &(BCD_decode_tab[0]);
	}
	else
	{
		pan_fd650_gp_data.bitmap_len = config->bitmap_len;
		pan_fd650_gp_data.bitmap_list = config->bitmap_list;
	}
	pan_fd650_gp_data.hw_info = config->hw_info;

	dev->priv 		= &pan_fd650_gp_data;

	dev->led_num 	= pan_fd650_gp_data.hw_info->num_com;
	dev->key_num 	= 0;

	/* Function point init */
	dev->init 		= pan_fd650_gp_attach;
	dev->open 		= pan_fd650_gp_open;
	dev->stop 		=  NULL;//pan_fd650_gp_close;
	dev->do_ioctl 	= pan_fd650_gp_ioctl;
	dev->display 		= pan_fd650_gp_display;
	dev->send_data 	= NULL;
	dev->receive_data = NULL;

	/* Add this device to queue */
	if (dev_register(dev) != SUCCESS)
	{
		PRINTF("Error: Register panel device error!\n");
		dev_free(dev);
		return ERR_NO_DEV;
	}
	//pan_common_gpio_init(config->hw_info, 1);
	tp = (struct pan_fd650_gp_info *)dev->priv;
	for(i=0; i<4; i++)
	{
		HAL_GPIO_BIT_DIR_SET(tp->hw_info->lbd[i].position, tp->hw_info->lbd[i].io);
		HAL_GPIO_BIT_SET(tp->hw_info->lbd[i].position, !tp->hw_info->lbd[i].polar);
	}
	/* Set default bitmap in buffer. At same time get the dot bitmap. */
	for (i = 0; i < tp->bitmap_len; i++)
	{
		if (' ' == tp->bitmap_list[i].character)
		{
			tp->blankmap = tp->bitmap_list[i].bitmap;
			for (j = 0; j < tp->hw_info->num_com; j++)
				tp->dis_buff[j] = tp->blankmap;
		}
		else if ('.' == tp->bitmap_list[i].character)
		{
			tp->dotmap = tp->bitmap_list[i].bitmap;
		}
		else if (':' == tp->bitmap_list[i].character)
		{
			tp->colonmap = tp->bitmap_list[i].bitmap;
		}
	}
	/* If is shadow scan, all key scan per 1mS interrupt, else... */
	if (tp->hw_info->type_scan == 1)
	{
		tp->cnt_repeat_first = tp->hw_info->intv_repeat_first;
		tp->cnt_repeat = tp->hw_info->intv_repeat;
	} else
	{
		tp->cnt_repeat_first = tp->hw_info->intv_repeat_first / tp->hw_info->num_com;
		tp->cnt_repeat = tp->hw_info->intv_repeat / tp->hw_info->num_com;
	}

	fd_650_scl 	= tp->hw_info->fclock.position;
	fd_650_sda 	= tp->hw_info->fdata.position;
	return SUCCESS;
}

/*
* Name          :   pan_pt6964_gp_interrupt()
* Description   :   front panel interrupt function
* Parameter     :   None
* Return        :   void
*/
void pan_fd650_gp_interrupt(UINT32 param)
{
	struct pan_device *dev = (struct pan_device *)param;
	struct pan_fd650_gp_info *tp = (struct pan_fd650_gp_info *)dev->priv;
	UINT32 keypress = PAN_KEY_INVALID;
	UINT8 key_650[5] = {0,0,0,0,0};
	UINT16 key_matrix = 0;
	struct pan_key key;
	static UINT8 cnt = 0;
	static UINT8 tmpcnt = 0;
	UINT32 i;

	if (tmpcnt++<10)
		return;
	tmpcnt = 0;

//	pan_fd650_gp_set_mode(FD650_SYSON_8);
	//pan_fd650_gp_update_led(tp->dis_buff, dev->led_num,0);
//	pan_fd650_gp_led_onoff(1);
	pan_fd650_gp_read_key((UINT8 *)(&key_650), 5);

	keypress = fd650_key_get_value(key_650[0]);

/*		
	
	key_matrix = ((key_650[1] & 0x18) << 3) | ((key_650[1] & 0x03) << 4) | \
		((key_650[0] & 0x18) >> 1) | (key_650[0] & 0x03);

	for(i=0; i<16; i++)
		if((key_matrix>>i)&0x1)
			break;
	if(i==16)
		keypress = PAN_KEY_INVALID;
	else
		keypress = 0xffff0000|(1<<i);

	if (tp->hw_info->hook_scan)
		keypress = tp->hw_info->hook_scan(dev, keypress);
*/
	/* Some key input */
	if (keypress != PAN_KEY_INVALID)
	{
		/* If is the same key, count it */
		if (tp->keypress_bak == keypress)
		{
			tp->keypress_cnt++;
			if (tp->keypress_cnt == 2)
			{
	    			key.type 		= PAN_KEY_TYPE_PANEL;
	    			key.state 	= PAN_KEY_PRESSED;
				key.count 	= tp->key_cnt;
				key.code 	= tp->keypress_bak;
				pan_buff_queue_tail(&key);
				tp->key_cnt++;
			}
			else if (tp->keypress_cnt == tp->keypress_intv)
			{
				tp->keypress_intv = tp->cnt_repeat;
				tp->keypress_cnt = 0;	/* Support continue press key */
			}
		}
		/* Else is a new key, backup it */
		else
		{
			if (tp->hw_info->intv_release & 1)	/* If intv_release is odd, pan key repeat enable */
			{
				if (tp->keypress_bak != PAN_KEY_INVALID)
				{
			   		key.type 		= PAN_KEY_TYPE_PANEL;
			   		key.state 	= PAN_KEY_RELEASE;
					key.count 	= 0;
					key.code 	= tp->keypress_bak;
					pan_buff_queue_tail(&key);
				}
			}
			tp->keypress_intv 		= tp->cnt_repeat_first;
			tp->keypress_bak 		= keypress;
			tp->keypress_num 	= tp->dis_id;
			tp->keypress_cnt 		= 1;
			tp->key_cnt = 0;
		}
	}
	/* No key input, if the same key switch to invalid, reset it */
	else if (/*tp->keypress_num == tp->dis_id &&*/ tp->keypress_bak != PAN_KEY_INVALID)
	{
		if (cnt++ > 5)
		{
			if (tp->hw_info->intv_release & 1)	/* If intv_release is odd, pan key repeat enable */
			{
		   		key.type = PAN_KEY_TYPE_PANEL;
		   		key.state = PAN_KEY_RELEASE;
				key.count = 0;
				key.code = tp->keypress_bak;
				pan_buff_queue_tail(&key);
			}
			tp->keypress_intv = tp->cnt_repeat_first/10;
			tp->keypress_bak = PAN_KEY_INVALID;
			tp->keypress_num = tp->hw_info->num_com;
			tp->keypress_cnt = 0;
			tp->key_cnt = 0;
			cnt = 0;
		}
	}
}

/*
* Name          :   pan_pt6964_gp_interrupt()
* Description   :   front panel interrupt function
* Parameter     :   None
* Return        :   void
*/
void pan_fd650_gp_tsak(UINT32 param)
{
	struct pan_device *dev = (struct pan_device *)param;
	struct pan_fd650_gp_info *tp = (struct pan_fd650_gp_info *)dev->priv;
	UINT32 keypress = PAN_KEY_INVALID;
	UINT8 key_650[5] = {0,0,0,0,0};
	UINT16 key_matrix = 0;
	struct pan_key key;
	static UINT8 cnt = 0;
	UINT32 i;

	while(1)
	{
		pan_fd650_gp_read_key((UINT8 *)(&key_650), 5);
		keypress = fd650_key_get_value(key_650[0]);
		if (keypress != PAN_KEY_INVALID)
		{
			libc_printf("%s :key_value =%d \n",__FUNCTION__,keypress);
			/* If is the same key, count it */
			if (tp->keypress_bak == keypress)
			{
				tp->keypress_cnt++;
				if (tp->keypress_cnt == 2)
				{
		    			key.type 		= PAN_KEY_TYPE_PANEL;
		    			key.state 	= PAN_KEY_PRESSED;
					key.count 	= tp->key_cnt;
					key.code 	= tp->keypress_bak;
					pan_buff_queue_tail(&key);
					tp->key_cnt++;
				}
				else if (tp->keypress_cnt == tp->keypress_intv)
				{
					tp->keypress_intv = tp->cnt_repeat;
					tp->keypress_cnt = 0;	/* Support continue press key */
				}
			}
			/* Else is a new key, backup it */
			else
			{
				if (tp->hw_info->intv_release & 1)	/* If intv_release is odd, pan key repeat enable */
				{
					if (tp->keypress_bak != PAN_KEY_INVALID)
					{
				   		key.type 		= PAN_KEY_TYPE_PANEL;
				   		key.state 	= PAN_KEY_RELEASE;
						key.count 	= 0;
						key.code 	= tp->keypress_bak;
						pan_buff_queue_tail(&key);
					}
				}
				tp->keypress_intv 		= tp->cnt_repeat_first;
				tp->keypress_bak 		= keypress;
				tp->keypress_num 	= tp->dis_id;
				tp->keypress_cnt 		= 1;
				tp->key_cnt = 0;
			}
		}
		/* No key input, if the same key switch to invalid, reset it */
		else if (/*tp->keypress_num == tp->dis_id &&*/ tp->keypress_bak != PAN_KEY_INVALID)
		{
			if (cnt++ > 5)
			{
				if (tp->hw_info->intv_release & 1)	/* If intv_release is odd, pan key repeat enable */
				{
			   		key.type = PAN_KEY_TYPE_PANEL;
			   		key.state = PAN_KEY_RELEASE;
					key.count = 0;
					key.code = tp->keypress_bak;
					pan_buff_queue_tail(&key);
				}
				tp->keypress_intv = tp->cnt_repeat_first/10;
				tp->keypress_bak = PAN_KEY_INVALID;
				tp->keypress_num = tp->hw_info->num_com;
				tp->keypress_cnt = 0;
				tp->key_cnt = 0;
				cnt = 0;
			}
		}
fd650_sleep:
        	osal_task_sleep(10);
    	}
}

/*
* Name          :   pan_scan_callback()
* Description   :   This function is used for callback by some disabled IRQ app
* Parameter     :   struct pan_device *dev
* Return        :   void
*/
#if 0//def DBG_LED_BMP
void pan_fd650_gp_scan_callback(struct pan_device *dev)
{
	static UINT32 last_tsc = 0;
	UINT32 cur_tsc;

	cur_tsc = read_tsc();
	if ((cur_tsc - last_tsc) / (SYS_CPU_CLOCK / 400) == 0)
	{
		return;
	}
	last_tsc = cur_tsc;

	pan_fd650_gp_interrupt((UINT32)dev);
}

//#define DBG_LED_BMP
/*
This function is based bitmap_table above to generate a new led bmp table
//
 *      ###b7##
 *    b2#     #b6
 *      ###b1##
 *    b3#     #b5
 *      ###b4##  #b0#
*/
static UINT8 led_bmp_mask[] = {0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80};

#define SET_BIT(v, bitn, bitv) v |= (bitv<<bitn);
void pan_gen_led_bmp(struct led_bitmap *dest, UINT8 *led_bmp, UINT8 led_light)
{
    UINT16 i, j;
    for(i = 0; i < PAN_COMMON_CHAR_LIST_NUM; i++)
    {
        dest[i].bitmap = 0;
        for(j = 0; j < 8; j++)
        {
            if(!(bitmap_table[i].bitmap&led_bmp_mask[j]))
            {
                //This bit needs to be light
                SET_BIT(dest[i].bitmap, led_bmp[j], led_light);
            }
            else
            {
                //This bit needs to be dark
                SET_BIT(dest[i].bitmap, led_bmp[j], (!led_light));
            }
        }
    }

    for(i = 0; i < PAN_COMMON_CHAR_LIST_NUM; i++)
    {
        osal_delay(2000);
        LIBC_PRINTF("{'%c', 0x%02x}, ", dest[i].character, dest[i].bitmap);
        osal_delay(2000);
        if((i && (i%4 == 0)) || (i == 0))
            LIBC_PRINTF("\n");
    }
    LIBC_PRINTF("\n");
}
#endif

