/****************************************************************************
 *
 *  ALi (zhuhai) Corporation, All Rights Reserved. 2002 Copyright (C)
 *
 *  File: lib_ashcmd.c
 *
 *  Description: This file contains all functions definition SFU test
 *		         command functions.
 *  History:
 *      Date        	Author      	Version  		Comment
 *      =======  	========	========	=========
 *  1   2016.02.15 	robin.gan  		0.1 			Creat
 *  2   2016.03.09 	paladin.ye  		0.2 			Add test function
 ****************************************************************************/
#include <types.h>
#include <retcode.h>
#include <sys_config.h>
#include <basic_types.h>
#include <api/libc/string.h>
#include <api/libc/printf.h>
#include <api/libnim/lib_nim.h>
#ifndef NEW_DEMO_FRAME
#include <api/libpub29/lib_pub27.h>
#else
#include <api/libpub/lib_pub.h>
#endif
#include <hld/dmx/dmx.h>
#include <hld/hld_dev.h>
#include <hld/nim/nim.h> 
#include <bus/tsi/tsi.h>
#include <bus/sci/sci.h>
#include "control.h"
#include <hld/sto/sto_dev.h>
#include "nim_test.h"

#ifdef NIM_TEST_SUPPORT

//#define _NIM_TEST_
#ifdef _NIM_TEST_
#define NIM_TEST_PRINT   libc_printf
#else
#define NIM_TEST_PRINT(...)
#endif

#ifdef DVBS_SUPPORT
struct str_nim_test_cmd *nim_test_cmd = NULL;

struct str_nim_test_cmd nim_test_command[] =
{
    {"help",\
     "<null>",\
     "Add 'help' to get more information,  e.g. 'get_reg help'",\
     nim_test_help},  
	
    {"get_reg",\
    "<reg_addr> <len>",\
    "Read register value",\
    nim_test_reg_read},
    
    {"set_reg",\
    "<reg_addr> <len> <data...>",\
    "Write register value, 'len' must be match the 'data' number",\
    nim_test_reg_write},

    {"get_chip_type",\
    "<nim_num>",\
    "Get current chip type",\
    nim_test_get_chip_type},
    
    {"get_info",\
    "<nim_num>",\
    "Get channel information",\
    nim_test_get_channel_info},

    {"test_diseqc",\
    "<nim_num> <operate_type> <param...>",\
    "Test diseqc main function, add 'help' to get more information",\
    nim_test_diseqc},

    {"stop",\
    "<null>",\
    "Stop nim test task",\
    nim_test_stop},
 
    {NULL, NULL, NULL, NULL}
};

#define NIM_TEST_COMMAND_LST			8
#define NIM_TEST_COMMAND_LEN			128

static unsigned char nim_test_cmd_buffer[NIM_TEST_COMMAND_LEN];
static unsigned char *argv[NIM_TEST_COMMAND_LST];
static unsigned int argc;

static UINT8   value_22k = 0x00;
static UINT8   chip_type = 0x00; //3501D and M3501B:0xd0, C3503 is 0xe0 C3505 is 0xf0;   
static UINT32  chip_base_addr = 0x00; // Chip base address 

#define NIM_TEST_TASK_STACKSIZE	0x8000//0x1000
#define NIM_TEST_TASK_QUANTUM		10
static ID nim_test_task_id = OSAL_INVALID_ID;
static BOOL m_exit_task = FALSE;

static void nim_test_task();
static void nim_test_cm_register(struct str_nim_test_cmd *cm);
static void nim_test_get_command(void);

static BOOL nim_test_task_init()
{
	OSAL_T_CTSK		t_ctsk;

    if(OSAL_INVALID_ID != nim_test_task_id)
    {
        NIM_TEST_PRINT("Nim test task existing!\n");
        return FALSE;
    }

	t_ctsk.stksz	= NIM_TEST_TASK_STACKSIZE;
	t_ctsk.quantum	= NIM_TEST_TASK_QUANTUM;
	t_ctsk.itskpri	= OSAL_PRI_NORMAL;
	t_ctsk.name[0]	= 'N';//Nim
	t_ctsk.name[1]	= 'T';//Test
	t_ctsk.name[2]	= 'T';//Task
	t_ctsk.task = (FP)nim_test_task;
	nim_test_task_id = osal_task_create(&t_ctsk);
	if(OSAL_INVALID_ID == nim_test_task_id)
	{
		PRINTF("cre_tsk nim_test_task_id failed\n");
		return FALSE;
	}

	return TRUE;
}

static int nim_test_pow(int x, int y)
{
	return 1 << (y << 2);
}

static int nim_test_atoi(char *value)
{
	int len, i, ret;
	if (NULL == value )
	{
		return 0;
	}

	len = strlen(value);
	ret = 0;
	if (len > 2 
	&& value[0] == '0'
	&& (value[1] == 'x' || value[1] == 'X'))
	{
		for (i = len-1; i > 1; i --)
		{
			if (value[i] >= '0' && value[i] <= '9')
			{
				ret += ((value[i] - '0') * nim_test_pow(16, (len-i-1)));
			}
			else if (value[i] >= 'A' && value[i] <= 'F')
			{
				ret += ((value[i] - 'A' + 10) * nim_test_pow(16, (len-i-1)));
			}
			else if (value[i] >= 'a' && value[i] <= 'f')
			{
				ret += ((value[i] - 'a' + 10) * nim_test_pow(16, (len-i-1)));
			}
			else
			{	
				//error
				return 0;
			}
		}
	}
	else
	{
		return atoi(value);
	}
	return ret;
}

int nim_test_printf_error(void)
{
    NIM_TEST_PRINT("Error parameter! Please enter 'help' to get more information\n");
    return 0;
}

static void nim_test_cm_register(struct str_nim_test_cmd *cm)
{
	nim_test_cmd = cm;
}

static void nim_test_get_command(void)
{
    unsigned char c;
    unsigned char *p, *q;

    p = nim_test_cmd_buffer;
    q = nim_test_cmd_buffer + NIM_TEST_COMMAND_LEN - 1;
    memset(nim_test_cmd_buffer, 0x0, sizeof(nim_test_cmd_buffer));
    while (p < q)
    {
        /* Get a charactor and echo it */
        //NIM_TEST_SEND_CHAR(c = NIM_TEST_RECV_CHAR());
        c = NIM_TEST_RECV_CHAR();

        if (c == 0x0D || c == 0x0A)
        {
            q = p;
            break;
        }
        /* Backspace */
        else if (c == 0x08)
        {
            if (p != nim_test_cmd_buffer)
                p--;
        }
        else
            *p++ = c;
    }

    argc = 0;
    p = nim_test_cmd_buffer;
    while (p < q)
    {
        /* skip blank */
        while ((*p == ' ' || *p == '\t') && p < q)
            p++;
        /* new word */
        argv[argc++] = p;
        /* find end of word */
        while (*p != ' ' && p < q)
            p++;
        *p++ = 0;
    }
}

void nim_test_task_delete()
{
	if(OSAL_INVALID_ID != nim_test_task_id)
	{
        //osal_task_delete(nim_test_task_id);
        m_exit_task = TRUE;
        nim_test_task_id = OSAL_INVALID_ID;
	}
}

void nim_test_task()
{
    unsigned int i = 0;

    /* Command line process */
#ifdef   __MM_OSD_VSRC_MEM_ADDR  
    //OSD_TaskBuffer_Init(osal_task_get_current_id(),__MM_OSD_VSRC_MEM_ADDR);
#endif
    m_exit_task = FALSE;
    while (1)
    {
			nim_test_get_command();
			
			if (argc == 0)
				continue;

            // Printf input information
            NIM_TEST_PRINT("\nCommand: ");
            for(i = 0; i < argc; i++)
            {
                NIM_TEST_PRINT("%s ",  argv[i]);
            }
			NIM_TEST_PRINT("\n");
            
			i = 0;
			
			if(NULL != nim_test_cmd)
			{
				do
				{  
					if(nim_test_cmd[i].command)
					{
						if (0 == STRCMP(nim_test_cmd[i].command, argv[0]))
						{
						    argv[0] = &i;
                            nim_test_cmd[i].handler(argc, argv);
                            break;
						}
						else
							i++;
					}
					else
                    {        
                        NIM_TEST_PRINT("Error command!  enter 'help' to get more information \n\n");
						break;	        	
                    }
                }while(1);
			}
			
			if(m_exit_task == TRUE)
				break;
    }
}


/*nim test init*/
void nim_test_init()
{
    nim_test_cm_register(nim_test_command);
    
	key_set_upgrade_check_flag(0);
	if(FALSE != nim_test_task_init())
    {  
        NIM_TEST_PRINT("\n*********************************************************************\n");
        NIM_TEST_PRINT("\        Enter nim test, enter 'help' to get more information!\n");
        NIM_TEST_PRINT("\*********************************************************************\n");
    }
    else
    {
        NIM_TEST_PRINT("Create nim test task failed!\n");
    }
}

int nim_test_help(unsigned int argc, unsigned char *argv[])
{	
	UINT8 i;
    
    NIM_TEST_PRINT("Command        Parameter                               Description\n");

	i = 0;
	while (1)
	{				
		if(NULL == nim_test_cmd[i].command)
		{
            NIM_TEST_PRINT("\n");
			return 0;			
		}
		else
		{  
			NIM_TEST_PRINT(" %-15s", nim_test_cmd[i].command);
			NIM_TEST_PRINT("%-40s", nim_test_cmd[i].param);
			NIM_TEST_PRINT("%s \n", nim_test_cmd[i].help_info);
			i++;
		}
	}
	return 0;
}

int nim_test_stop(unsigned int argc, unsigned char *argv[])
{
	if (argc != 1)
    {
        nim_test_printf_error();
        return -1;
    }
	nim_test_task_delete();
	NIM_TEST_PRINT("Exit nim test!\n");
	return 0;
}

int get_chip_type(struct nim_device *dev)
{
    UINT32 data = 0; 

    if(NULL == dev->do_ioctl_ext)
    {
        NIM_TEST_PRINT("[%s %d]NULL == nim_dev->do_ioctl\n", __FUNCTION__, __LINE__);
        return -1;
    }

    dev->do_ioctl_ext(dev, NIM_DRIVER_GET_ID, &data);
    chip_type = (UINT8)data;
    
    switch(chip_type)
    {
        case 0xd0: 
            chip_base_addr = C3501_BASE_ADDR;
            break;
        case 0xe0:
            chip_base_addr = C3503_BASE_ADDR;
            break;
        case 0xf0:
            chip_base_addr = C3505_BASE_ADDR;
            break;
        default:
            chip_base_addr = 0xb804c000;
            break;            
    }
    
    //NIM_TEST_PRINT("[%s ]chip_base_addr == %x\n", __FUNCTION__, chip_base_addr);

    return 0;
}


int nim_test_get_chip_type(unsigned int argc, unsigned char *argv[])
{
  	struct nim_device *nim_dev = NULL;
    UINT16 nim_id;
    char *chip_info;

    if ((0 == STRCMP(nim_test_cmd[0].command, argv[1])) || (1 == argc)) 
    {
        NIM_TEST_PRINT("Command        Parameter                               Description\n");
        NIM_TEST_PRINT(" %-15s", nim_test_cmd[*argv[0]].command);
        NIM_TEST_PRINT("%-40s", nim_test_cmd[*argv[0]].param);
        NIM_TEST_PRINT("%s \n", nim_test_cmd[*argv[0]].help_info);
        return 0;
    }
    else if(2 != argc)
    {
        nim_test_printf_error();
        return -1;
    }

	nim_id = nim_test_atoi(argv[1]);
    nim_dev = dev_get_by_id(HLD_DEV_TYPE_NIM, nim_id);

    if(NULL == nim_dev->do_ioctl_ext)
    {
    	NIM_TEST_PRINT("[%s %d]NULL == nim_dev->do_ioctl\n", __FUNCTION__, __LINE__);
        return -1;
    }

    get_chip_type(nim_dev);
    
    //3501D and M3501B:0xd0, C3503 is 0xe0 C3505 is 0xf0;
    switch(chip_type)
    {
        case 0xd0: 
            chip_info = "chip tpye: 3501b/3501d, base addr: 0xb8003000";
            break;
        case 0xe0:
            chip_info = "chip tpye: 3503, base addr: 0xb8028000";
            break;
        case 0xf0:
            chip_info = "chip tpye: 3503c/3505, base addr: 0xb804c000";
            break;
        default:
            chip_info = "chip tpye: unknown, base addr: unknown";
            break;            
    }
        
    NIM_TEST_PRINT("Nim num = %d, %s\n", nim_id, chip_info);
    
    return 0;
}


/*****************************************************************************
* INT32 nim_reg_read(struct nim_device *dev, UINT8 bMemAdr, UINT8 *pData, UINT8 bLen)
* Description: S3501 register read function
*                   Can read 8-byte one time and bLen must no more than 8
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 pol
*
* Return Value: void
*****************************************************************************/
int nim_test_reg_read(unsigned int argc, unsigned char *argv[])
{
    UINT32 addr = 0;
    UINT16 len = 0;
	UINT16 i = 0;
    UINT8 data = 0;
	struct nim_device *nim_dev = NULL;
	UINT16 nim_id;
    
    if ((0 == STRCMP(nim_test_cmd[0].command, argv[1])) || (1 == argc)) 
    {
        NIM_TEST_PRINT("Command        Parameter                               Description\n");
        NIM_TEST_PRINT(" %-15s", nim_test_cmd[*argv[0]].command);
        NIM_TEST_PRINT("%-40s", nim_test_cmd[*argv[0]].param);
        NIM_TEST_PRINT("%s \n", nim_test_cmd[*argv[0]].help_info);
        return 0;
    }
    else if(3 != argc)
    {
        nim_test_printf_error();
        return -1;
    }

    addr = nim_test_atoi(argv[1]);
    len = nim_test_atoi(argv[2]);

	if ((len > 1000) || (0 == len))
	{
	    NIM_TEST_PRINT("Exit with illegal parameter! len = %d\n", len);
		return -1;
	}

	for (i = 0; i < len; i++)
	{
        data = NIM_GET_BYTE(addr + i);          
		
        if(0 == i)
        {
            NIM_TEST_PRINT("Read data from addr: 0x%02x \n", addr);
            NIM_TEST_PRINT("%02x ", data);
        }
        else if(0 == ((i+1)%16))
        {   
            NIM_TEST_PRINT("%02x ", data);
            NIM_TEST_PRINT("\n");
        }
        else
        {
            NIM_TEST_PRINT("%02x ", data);
        }
	}
    
    NIM_TEST_PRINT("\n");
	return 0;
}

/*****************************************************************************
* INT32 nim_reg_write(struct nim_device *dev, UINT8 bMemAdr, UINT8 *pData, UINT8 bLen)
* Description: S3501 register write function
*                   Can write 8-byte one time and bLen must no more than 8
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 pol
*
* Return Value: void
*****************************************************************************/
int nim_test_reg_write(unsigned int argc, unsigned char *argv[])
{
    UINT32 addr = 0;
    UINT16 len = 0;
	UINT16 i = 0;
    UINT8 data = 0;

	struct nim_device *nim_dev = NULL;
	UINT16 nim_id;

   if ((0 == STRCMP(nim_test_cmd[0].command, argv[1]))|| (1 == argc)) 
   {
       NIM_TEST_PRINT("Command        Parameter                               Description\n");
       NIM_TEST_PRINT(" %-15s", nim_test_cmd[*argv[0]].command);
       NIM_TEST_PRINT("%-40s", nim_test_cmd[*argv[0]].param);
       NIM_TEST_PRINT("%s \n", nim_test_cmd[*argv[0]].help_info);
       return 0;
   }
   else if(4 > argc)
   {
       nim_test_printf_error();
       return -1;
   }

    addr = nim_test_atoi(argv[1]);
    len = nim_test_atoi(argv[2]);

	if (argc != (len + 3))
	{
	    NIM_TEST_PRINT("Exit with data number can't matching <len>! data_number = %d, len = %d \n", argc - 3, len);
		return -1;
	}
    
	for (i = 0; i < len; i++)
	{
        data = nim_test_atoi(argv[i + 3]);
        NIM_SET_BYTE(addr + i, data);
        if(0 == i)
        {
            NIM_TEST_PRINT("Write data to addr: 0x%02x \n", addr);
            NIM_TEST_PRINT("%02x ", data);
        }
        else if(0 == ((i+1)%16))
        {   
            NIM_TEST_PRINT("%02x ", data);
            NIM_TEST_PRINT("\n");
        }
        else
        {
            NIM_TEST_PRINT("%02x ", data);
        }
	}

    NIM_TEST_PRINT("\n");
	return 0;
}


int nim_test_get_channel_info(unsigned int argc, unsigned char *argv[])
{
	INT32 ret = -1;  
    UINT32 agc = 0;
    UINT32 ber = 0;
    UINT32 lcok_status = 0;
	struct nim_device *nim_dev = NULL;
	UINT16 nim_id = 0;

    // Check param
   if ((0 == STRCMP(nim_test_cmd[0].command, argv[1])) || (1 == argc)) 
   {
       NIM_TEST_PRINT("Command        Parameter                               Description\n");
       NIM_TEST_PRINT(" %-15s", nim_test_cmd[*argv[0]].command);
       NIM_TEST_PRINT("%-40s", nim_test_cmd[*argv[0]].param);
       NIM_TEST_PRINT("%s \n", nim_test_cmd[*argv[0]].help_info);
       return 0;
   }
   else if(argc != 2)
   {
       nim_test_printf_error();
       return -1;
   }

    // Get device
	nim_id = nim_test_atoi(argv[1]);
	//NIM_TEST_PRINT("[%s %d]nim_id = %d\n", __FUNCTION__, __LINE__, nim_id);
    nim_dev = dev_get_by_id(HLD_DEV_TYPE_NIM, nim_id);

    if(NULL == nim_dev->do_ioctl)
    {
    	NIM_TEST_PRINT("[%s %d]NULL == nim_dev->do_ioctl\n", __FUNCTION__, __LINE__);
        return -1;
    }

    // Get lock status
    ret = nim_dev->do_ioctl(nim_dev, NIM_DRIVER_READ_QPSK_STATUS, (UINT32)&lcok_status); 
    if (ret != SUCCESS)
    {
        NIM_TEST_PRINT("%s: operation error! %d\n", __FUNCTION__, ret);
        return -1;
    }    		
	NIM_TEST_PRINT("Lock status = %s\n", lcok_status ? "Lock":"Unlock"); 

    // Get ber
    ret = nim_dev->do_ioctl(nim_dev, NIM_DRIVER_GET_BER, (UINT32)&ber);
    if (ret != SUCCESS)
    {
        NIM_TEST_PRINT("%s: operation error! %d\n", __FUNCTION__, ret);
        return -1;
    }    		
	NIM_TEST_PRINT("ber = %d\n", ber);  
    
    // Get agc
    ret = nim_dev->do_ioctl(nim_dev, NIM_DRIVER_GET_AGC, (UINT32)&agc);
    if (ret != SUCCESS)
    {
        NIM_TEST_PRINT("%s: operation error! %d\n", __FUNCTION__, ret);
        return -1;
    }    		
	NIM_TEST_PRINT("agc = %ddBm\n", agc);  
 
	return 0;
}

int nim_test_diseqc(unsigned int argc, unsigned char *argv[])
{
	INT32 ret;  
	struct nim_device *nim_dev = NULL;
	UINT16 nim_id;
    UINT16 index = 0;

    if ((0 == STRCMP(nim_test_cmd[0].command, argv[1])) || (1 == argc))
    {
        NIM_TEST_PRINT("Index  Subcommand    Parameter                         Description\n");
        NIM_TEST_PRINT(" 0      open          <null>                            Backups config of diseqc\n");
        NIM_TEST_PRINT(" 1      close         <null>                            Recover config of diseqc\n");
        NIM_TEST_PRINT(" 2      polar         <h|v>                             Set LNB Polar\n");
        NIM_TEST_PRINT(" 3      22k           <on|off>                          Open/close 22k wave\n");
        NIM_TEST_PRINT(" 4      toneburst     <0|1>                             Trigger burst 0/1\n");
        NIM_TEST_PRINT(" 5      envelope      <on|off>                          Select source of 22k wave\n");
        NIM_TEST_PRINT(" 6      write data    <len> <data...>                   Write data to diseqc,'len' must be match the 'data' number\n");
        NIM_TEST_PRINT(" 7      combined      <write data> <toneburst> <22k>    Combined transmission of diseqc,\n");

        NIM_TEST_PRINT("\n");
        NIM_TEST_PRINT("User must be enter <command> <index> to select subcommand\n");
        NIM_TEST_PRINT("e.g. 22k: 'test_diseqc 0 3 on', means open 22k wave of nim_0\n");
        NIM_TEST_PRINT("e.g. combined transmission: 'test_diseqc 1 7 3 0x33 0x44 0x55 0 off', means 'sent data 0x33 0x44 0x55' 'toneburst 0' and '22k off' of nim_1\n");
        
        return 0;
    }

    index = nim_test_atoi(argv[2]);

    switch(index)
    {
        case 0:
            ret = nim_diseqc_test_on(argc, argv);
            break;
        case 1:
            ret = nim_diseqc_test_off(argc, argv);
            break;             
        case 2:
            ret = nim_test_polar(argc, argv);
            break;
        case 3:
            ret = nim_test_22k(argc, argv);
            break;
        case 4:
            ret = nim_test_toneburst(argc, argv);
            break;
        case 5:
            ret = nim_test_envelope(argc, argv);
            break;
        case 6:
            ret = nim_test_diseqc_write_bytes(argc, argv);
            break;
        case 7:
            ret = nim_test_combined_transmission(argc, argv);
            break;

        default:
            nim_test_printf_error();
            return -1;
            break;
    }

	return 0;
}


int nim_diseqc_test_on(unsigned int argc, unsigned char *argv[])
{
	INT32 ret;  
	struct nim_device *nim_dev = NULL;
	UINT16 nim_id;
	
    if (argc != 3)
    {
        nim_test_printf_error();
        return -1;
    }
   
	nim_id = nim_test_atoi(argv[1]);
	//NIM_TEST_PRINT("[%s %d]nim_id = %d\n", __FUNCTION__, __LINE__, nim_id);
    nim_dev = dev_get_by_id(HLD_DEV_TYPE_NIM, nim_id);

    // store DISEQC_22k origianl value
    value_22k = NIM_GET_BYTE(chip_base_addr + 0x7c);
    value_22k &= 0x41;
    NIM_TEST_PRINT("Diseqc test open success!\n");

	return 0;
}


int nim_diseqc_test_off(unsigned int argc, unsigned char *argv[])
{
 	 INT32 ret;  
 	 struct nim_device *nim_dev = NULL;
     UINT8 data = 0;
 	 UINT16 nim_id;
	
     if (argc != 3)
     {
         nim_test_printf_error();
         return -1;
     }
    
 	nim_id = nim_test_atoi(argv[1]);
 	//NIM_TEST_PRINT("[%s %d]nim_id = %d\n", __FUNCTION__, __LINE__, nim_id);
    nim_dev = dev_get_by_id(HLD_DEV_TYPE_NIM, nim_id);
 
    //resume DISEQC_22k origianl value
    data = NIM_GET_BYTE(chip_base_addr + 0x7c);          
    data = ((data & 0xB8) | (value_22k));
    NIM_SET_BYTE(chip_base_addr + 0x7c, data);
    NIM_TEST_PRINT("Diseqc test close success!\n");

    return 0;
}



int nim_test_polar(unsigned int argc, unsigned char *argv[])
{
    INT32 ret;  
    struct nim_device *nim_dev = NULL;
    UINT16 nim_id;
    
    if (argc != 4)
    {
        nim_test_printf_error();
        return -1;
    }

    nim_id = nim_test_atoi(argv[1]);
    //NIM_TEST_PRINT("[%s %d]nim_id = %d\n", __FUNCTION__, __LINE__, nim_id);
    nim_dev = dev_get_by_id(HLD_DEV_TYPE_NIM, nim_id);
    
    if(NULL == nim_dev->do_ioctl)
    {
        NIM_TEST_PRINT("[%s %d]NULL == nim_dev->do_ioctl\n", __FUNCTION__, __LINE__);
        return -1;
    }

    if (0 == STRCMP("h", argv[3])) 
    {
        nim_set_polar(nim_dev, NIM_PORLAR_HORIZONTAL); 
        NIM_TEST_PRINT("Set polar H success!\n");
    }
    else if (0 == STRCMP("v", argv[3])) 
    {
        nim_set_polar(nim_dev, NIM_PORLAR_VERTICAL); 
        NIM_TEST_PRINT("Set polar V success!\n");
    }
    else
    {
        nim_test_printf_error();
        return -1;
    }

    return 0;
}


int nim_test_22k(unsigned int argc, unsigned char *argv[])
{
	INT32 ret;  
	struct nim_device *nim_dev = NULL;
	UINT16 nim_id;
	
    if (argc != 4)
    {
        nim_test_printf_error();
        return -1;
    }

	nim_id = nim_test_atoi(argv[1]);
	//NIM_TEST_PRINT("[%s %d]nim_id = %d\n", __FUNCTION__, __LINE__, nim_id);
    nim_dev = dev_get_by_id(HLD_DEV_TYPE_NIM, nim_id);
    
    if(NULL == nim_dev->do_ioctl)
    {
        NIM_TEST_PRINT("[%s %d]NULL == nim_dev->do_ioctl\n", __FUNCTION__, __LINE__);
        return -1;
    }

    if (0 == STRCMP("on", argv[3])) 
    {
        nim_di_seq_c_operate(nim_dev, NIM_DISEQC_MODE_22KON, NULL, 0); 
        NIM_TEST_PRINT("Set 22k on success!\n");
    }
    else if (0 == STRCMP("off", argv[3])) 
    {
        nim_di_seq_c_operate(nim_dev, NIM_DISEQC_MODE_22KOFF, NULL, 0); 
        NIM_TEST_PRINT("Set 22k off success!\n");
    }
    else
    {
        nim_test_printf_error();
        return -1;
    }

	return 0;
}


int nim_test_diseqc_write_bytes(unsigned int argc, unsigned char *argv[])
{
	INT32 ret;  
	struct nim_device *nim_dev = NULL;
	UINT16 nim_id;
    UINT8 len;
	UINT8 cmd_buf[8];
	UINT8 i;
    
    // 12 = 1(cmd) + 1(nim_id) + 1(subcommand) + 1(byte_cnt) + 8(byte)
    if((5 > argc) || (12 < argc))
    {
        nim_test_printf_error();
        return -1;
    }
    
     nim_id = nim_test_atoi(argv[1]);
     nim_dev = dev_get_by_id(HLD_DEV_TYPE_NIM, nim_id);
    
     if(NULL == nim_dev->do_ioctl)
     {
         NIM_TEST_PRINT("[%s %d]NULL == nim_dev->do_ioctl\n", __FUNCTION__, __LINE__);
         return -1;
     }
     
     len = nim_test_atoi(argv[3]);
    
     if (argc != (len + 4))
     {
         NIM_TEST_PRINT("Exit with data number can't matching <len>! data_number = %d, len = %d \n", argc - 4, len);
         return -1;
     }

	for (i = 0; i < len; i++)
	{
		cmd_buf[i] = nim_test_atoi(argv[4 + i]);
	}

    nim_di_seq_c_operate(nim_dev, NIM_DISEQC_MODE_BYTES, cmd_buf, len); 
    NIM_TEST_PRINT("Sent diseqc data success!\n");
    
	return 0;
}


int nim_test_toneburst(unsigned int argc, unsigned char *argv[])
{
	INT32 ret;  
	struct nim_device *nim_dev = NULL;
	UINT16 nim_id;
    UINT16 data = 0;
	
    if (argc != 4)
    {
        nim_test_printf_error();
        return -1;
    }

	nim_id = nim_test_atoi(argv[1]);
	//NIM_TEST_PRINT("[%s %d]nim_id = %d\n", __FUNCTION__, __LINE__, nim_id);
    nim_dev = dev_get_by_id(HLD_DEV_TYPE_NIM, nim_id);
    
    if(NULL == nim_dev->do_ioctl)
    {
        NIM_TEST_PRINT("[%s %d]NULL == nim_dev->do_ioctl\n", __FUNCTION__, __LINE__);
        return -1;
    }

    data = nim_test_atoi(argv[3]);
    
    if (0 == data) 
    {
        nim_di_seq_c_operate(nim_dev, NIM_DISEQC_MODE_BURST0, NULL, 0); 
        NIM_TEST_PRINT("Set tone burst 0 success!\n");
    }
    else if (1 == data) 
    {
        nim_di_seq_c_operate(nim_dev, NIM_DISEQC_MODE_BURST1, NULL, 0); 
        NIM_TEST_PRINT("Set tone burst 1 success!\n");
    }
    else
    {
        nim_test_printf_error();
        return -1;
    }
    
	return 0;
}

int nim_test_envelope(unsigned int argc, unsigned char *argv[])
{
	INT32 ret;  
	struct nim_device *nim_dev = NULL;
	UINT16 nim_id;
    UINT16 data = 0;
	
    if (argc != 4)
    {
        nim_test_printf_error();
        return -1;
    }

	nim_id = nim_test_atoi(argv[1]);
	//NIM_TEST_PRINT("[%s %d]nim_id = %d\n", __FUNCTION__, __LINE__, nim_id);
    nim_dev = dev_get_by_id(HLD_DEV_TYPE_NIM, nim_id);
    
    if(NULL == nim_dev->do_ioctl)
    {
        NIM_TEST_PRINT("[%s %d]NULL == nim_dev->do_ioctl\n", __FUNCTION__, __LINE__);
        return -1;
    }

    data = nim_test_atoi(argv[3]);
    
    if (0 == STRCMP("on", argv[3])) 
    {
        nim_di_seq_c_operate(nim_dev, NIM_DISEQC_MODE_ENVELOP_ON, NULL, 0); 
        NIM_TEST_PRINT("Set envelope on success!\n");
    }
    else if (0 == STRCMP("off", argv[3])) 
    {
        nim_di_seq_c_operate(nim_dev, NIM_DISEQC_MODE_ENVELOP_OFF, NULL, 0); 
        NIM_TEST_PRINT("Set envelope off success!\n");
    }
    else
    {
        nim_test_printf_error();
        return -1;
    }
    
	return 0;
}

int nim_test_combined_transmission(unsigned int argc, unsigned char *argv[])
{
	INT32 ret;  
	struct nim_device *nim_dev = NULL;
	UINT16 nim_id;
	UINT8 len;
	UINT8 cmd_buf[8];
	UINT8 subcmd = 0;
	UINT8 i;
    
    // 14 = 1(cmd) + 1(nim_id) + 1(subcommand)  + 1(byte_num) + 8(byte)+ 1(toneburst 0/1) + 1(22k on/off)
    if((5 > argc) || (14 < argc))
    {
        nim_test_printf_error();
        return -1;
    }
    
     nim_id = nim_test_atoi(argv[1]);
     nim_dev = dev_get_by_id(HLD_DEV_TYPE_NIM, nim_id);
    
     if(NULL == nim_dev->do_ioctl)
     {
         NIM_TEST_PRINT("[%s %d]NULL == nim_dev->do_ioctl\n", __FUNCTION__, __LINE__);
         return -1;
     }
     
     len = nim_test_atoi(argv[3]);
    
     if ((len + 6) != argc)
     {
         NIM_TEST_PRINT("Exit with data number can't matching <len>! data_number = %d, len = %d \n",\
            argc - 4, len);
         return -1;
     }
    
	for (i = 0; i < len; i++)
	{
		cmd_buf[i] = nim_test_atoi(argv[4 + i]);
	}
  
    nim_di_seq_c_operate(nim_dev, NIM_DISEQC_MODE_BYTES, cmd_buf, len); 
    NIM_TEST_PRINT("Combined transmission: sent diseqc data success!\n");
	if((len + 4) < argc)
	{
		subcmd = nim_test_atoi(argv[4 + len]);
		NIM_TEST_PRINT("cmd_count = %x,subcmd Value = %x\n", len, subcmd);
		if(0 == subcmd)
		{
			nim_di_seq_c_operate(nim_dev, NIM_DISEQC_MODE_BURST0, NULL, 0);
            NIM_TEST_PRINT("Combined transmission: set tone burst 0 success!\n");
		}
		else if(1 == subcmd)
		{
			nim_di_seq_c_operate(nim_dev, NIM_DISEQC_MODE_BURST1, NULL, 0);
            NIM_TEST_PRINT("Combined transmission: set tone burst 1 success!\n");
		}
		else
		{
			NIM_TEST_PRINT("Combined transmission fail due to wrong sub command");
			return -1;
		}

		if((len + 6) == argc)
		{
			if(0 == STRCMP("off", argv[5 + len])) 
			{
				nim_di_seq_c_operate(nim_dev, NIM_DISEQC_MODE_22KOFF, NULL, 0);
                NIM_TEST_PRINT("Combined transmission: set 22k off success!\n");
			}
			else if(0 == STRCMP("on", argv[5 + len])) 
			{
				nim_di_seq_c_operate(nim_dev, NIM_DISEQC_MODE_22KON, NULL, 0);
                NIM_TEST_PRINT("Combined transmission: set 22k on success!\n"); 
			}
			else
			{
				NIM_TEST_PRINT("Combined transmission fail due to wrong sub command");
				return -1;
			}
		}
	    else
		{
			NIM_TEST_PRINT("Combined transmission fail due to wrong sub command");
			return -1;
		}
	}
	return 0;
}
#endif

#endif

