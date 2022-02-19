#include <sys_config.h>
#include "slot.h"
//=====================================================================================//

unsigned long standby_wakeup_time = 0xFF;
unsigned char standby_second_value = 0xFF;
unsigned char standby_min_value = 0xFF;
unsigned char standby_hour_value = 0xFF;
unsigned char standby_date_value = 0xFF;
unsigned char standby_month_value = 0xFF;
unsigned char standby_year_value = 0xFF;

#if (!defined _M3503C_) && (!defined _NO_PANEL_APPLIED_)
#if defined _PANEL_SCAN_ENABLE
extern void stby_gpio_show_off(void);
extern void stby_gpio_show_time(unsigned char hour, unsigned char min, unsigned char sec);
#else
extern void stby_show_off(void);
extern void stby_show_time(unsigned char hour, unsigned char min, unsigned char sec);
#endif
#endif

#ifdef _PANEL_SCAN_ENABLE
extern void standby_pan_hw_init(void);
extern void standby_hw_scan_show(unsigned char hour, unsigned char min, unsigned char sec);
#endif
extern void exit_standby(void);
//=====================================================================================//

void standby_delay_time(unsigned long flag)
{
	unsigned long i = 0, j = 0;

	for(i=0; i<flag; i++)
	{
		j++;
	}
}

void Exit_Standby_Process(void)
{
	unsigned long current_time = 0;
	current_time |= (standby_second_value | (standby_min_value << 6) | (standby_hour_value << 12) \
					| (standby_date_value << 17) | (standby_month_value << 22) | (standby_year_value << 26));
	stdby_save_time(current_time);
	exit_standby();
}

void sys_prestandby(unsigned long wakeup_time, unsigned long current_time)
{
	/*Get second.*/
	standby_second_value = (unsigned char)(current_time&0x3f);

	/*get minute.*/
	standby_min_value = (unsigned char)((current_time>>6)&0x3f);

	/*Get hour.*/
	standby_hour_value = (unsigned char)((current_time>>12)&0x1f);

	/*Get date.*/
	standby_date_value = (unsigned char)((current_time>>17)&0x1f);

	/*Get month.*/
	standby_month_value = (unsigned char)((current_time>>22)&0xf);

	/*Get year.*/
	standby_year_value = (unsigned char)((current_time>>26)&0x3f);

	/*Backup wakeup time.*/
	standby_wakeup_time = wakeup_time;

#if (!defined _M3503C_) && (!defined _NO_PANEL_APPLIED_)
	#ifndef _PANEL_SCAN_ENABLE
	stby_pan_init();
	#else
	standby_pan_hw_init();
	#endif
#endif

	stdby_init_ir();
}

unsigned char standby_is_leap_year(unsigned long year)
{
	if((year%4 == 0) && (year%100 != 0))
	{
		return 1;
	}
	else if((year%400) == 0)
	{
		return 1;
	}
	else 
	{
		return 0;
	}
}

void stby_update_time(void)
{
	unsigned long current_time = 0;

	standby_second_value++;
	if(standby_second_value > 59)
	{
		standby_second_value = 0;
		standby_min_value++;
		if(standby_min_value > 59)
		{
			standby_min_value = 0;
			standby_hour_value++;
			if(standby_hour_value > 23)
			{
				standby_hour_value = 0;
				standby_date_value++;

				if((standby_month_value == 1) || (standby_month_value == 3) \
					|| (standby_month_value == 5) || (standby_month_value == 7) \
					|| (standby_month_value == 8) || (standby_month_value == 10) \
					|| (standby_month_value == 12))
				{
					if(standby_date_value > 31)
					{
						standby_date_value = 1;
						standby_month_value++;
						if(standby_month_value > 12)
						{
							standby_month_value = 1;
							standby_year_value++;
						}
					}
				}

				if(standby_month_value == 2)
				{
					if((standby_date_value > 29) && (standby_is_leap_year(standby_year_value)))
					{
						standby_date_value = 1;
						standby_month_value++;
					}
					else if((standby_date_value > 28) && (standby_is_leap_year(standby_year_value) == 0))
					{
						standby_date_value = 1;
						standby_month_value++;
					}
				}

				if((standby_month_value == 4) || (standby_month_value == 6) \
					|| (standby_month_value == 9) || (standby_month_value == 11))
				{
					if(standby_date_value > 30)
					{
						standby_date_value = 1;
						standby_month_value++;
					}
				}
			}
		}
	}

	current_time = 0;
	current_time |= (standby_second_value | (standby_min_value << 6) | (standby_hour_value << 12) \
					| (standby_date_value << 17) | (standby_month_value << 22) | (standby_year_value << 26));
	if((standby_wakeup_time <= current_time) && (0 != standby_wakeup_time))
	{
		Exit_Standby_Process();
	}
}

#if (!defined _M3503C_)  && (!defined _PANEL_SCAN_ENABLE)
void get_fp_process(void)
{
	unsigned char fp_key = 0;

	fp_key = stby_scan_key();
#if 0/*Reserved for debug.*/
	unsigned char temp = 0;
	temp = ((fp_key>>4) & 0xF);
	stdby_uart_debug(temp);
	temp = (fp_key & 0xF);
	stdby_uart_debug(temp);
#endif
	if(STANDBY_PAN_KEY == fp_key)
	{
		Exit_Standby_Process();
	}
}
#endif

void slot_process(void)
{
	/*Disable all external interrupts.*/
#ifndef _M3702_
	*(volatile unsigned long *)(0xB8000038) = 0;
	*(volatile unsigned long *)(0xB800003C) = 0;
#else
	*(volatile unsigned long *)(0xB8081040) = 0;
	*(volatile unsigned long *)(0xB8081044) = 0;
	*(volatile unsigned long *)(0xB8081048) = 0;
	*(volatile unsigned long *)(0xB808104C) = 0;
	*(volatile unsigned long *)(0xB8081050) = 0;
	*(volatile unsigned long *)(0xB8081054) = 0;
#endif

#if (!defined _M3503C_) && (!defined _NO_PANEL_APPLIED_)
#ifndef _PANEL_SCAN_ENABLE
	#ifdef CPU_STANDBY_SHOW_TIME_ENABLE
	stby_show_time(standby_hour_value, standby_min_value, standby_second_value);
	#else
	stby_show_off();
	#endif
	get_fp_process();
#else
	standby_hw_scan_show(standby_hour_value, standby_min_value, standby_second_value);
#endif
#endif
}
