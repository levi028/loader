#include <types.h>
#include <retcode.h>
#include <sys_config.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/sci/sci.h>
#include <hld/hld_dev.h>
#include <hal/hal_mem.h>
#include <bus/flash/flash.h>
#include <api/libchunk/chunk.h>
#include <sys_parameters.h>
#include <api/libupg/p2p_upgrade.h>
#include <hld/sec_boot/sec_boot.h>
#include <bus/dog/dog.h>
#include <hld/dsc/dsc.h>
#include <api/librsa/flash_cipher.h>
#include <api/libfs2/lib_stdio.h>

#if 1
// debug program with front panel 
#define SUPPORT_PANEL_DISPLAY
#define SH_PRINTF   libc_printf
#define PRINTF      libc_printf
#else
#define SH_PRINTF(...)
#define PRINTF(...)
#endif

#ifdef SUPPORT_PANEL_DISPLAY
#include <hld/pan/pan.h>
#include <hal/hal_gpio.h>
#include <bus/i2c/i2c.h>
#include "./platform/chip/chip.h"
#endif

#ifdef _BUILD_USB_LOADER_RET2GAC
#include <api/libfs2/fs_main.h>
#include <api/libfs2/stat.h>
#include <api/libmp/mp_common.h>
#include <stdio.h>
#define ENABLE_USB_INTF
#define LOAD_SEE_FORM_USB
#endif

//#define SUPPORT_KEY_PROTECT
//#define SUPPORT_GEN_PROJ      //default is retailer,open it is GENCA 
#define CRC32_POLYNOMIAL 0xEDB88320

#define IMAGE_START     0x80200000
#define RAM_CODE_START  IMAGE_START

UINT8 g_crc_bad;
struct sto_device *flash_dev;
static unsigned long crc_32_tab[256];
static unsigned long g_dwChipVersion = 0;
static UINT32 g_bps = 0;
static unsigned char sec_flag[512];

//0:not burn; 1:burn flash(2 ladder enc sw uk); 2:burn flash(3 ladder enc sw uk)
static UINT8 flash_burn_flag=0;

//0:not burn; 1:burn otp(2 ladder enc sw uk); 2:burn otp(3 ladder enc sw uk)
static UINT8 bloader_enc_flag=0;

//load from mpclient.conf
static UINT32 otp_burn_encrypt_bl = 0;
static UINT32 otp_burn_cstm_id = 0;
static UINT32 otp_burn_boot_rom_sig = 0;
static UINT32 burn_ret2gca_otp = 0;

#ifdef SUPPORT_KEY_PROTECT
static char key_of_key[16] = {0xa1,0xb2,0xc3,0xd4,0xe5,0xf6,0xa7,0xb8,0xc1,0xd2,0xe3,0xf4,0x15,0x36,0x57,0x78};
#endif

static void system_hardware_init(void);

typedef enum{
    E_MP_IDLE,
    E_MP_MAIN_RUN,
    E_MP_LOADER_SEE,
    E_MP_SEE_RUN,
    E_MP_PREPARE_LOADER_FLASH_ABS,
    E_MP_BURN_FLASH,
    E_MP_FINISH,        
}MP_SYS_STAT;

static MP_SYS_STAT mp_state = E_MP_IDLE;

/////////////////////////////////////////////
static int usb_burn_flash(int data_addr, int data_len);
int burn_basic_otp_for_3527(void);
void panel_show(char* str,int len);
void parse_parameter(FILE* fconf);
void load_mpclient_config();


#ifdef SUPPORT_PANEL_DISPLAY
// enable front panel --->
chip_init_param chip_param;
extern void  global_chip_set(chip *chip);

static struct pan_hw_info pan_hw_info =
{
	0,				/* type_kb : 2; Key board (array) type */
	1,				/* type_scan : 1; 0: Slot scan, 1: Shadow scan */
	1,				/* type_key: 1; Key exit or not */
	1,				/* type_irp: 3; 0: not IRP, 1: NEC, 2: LAB */
	0,				/* type_mcu: 1; MCU exit or not */
	4,				/* num_com: 4; Number of com PIN, 0 to 8 */
	1,				/* Position of colon flag, 0 to 7 */
	1,				/* num_scan: 2; Number of scan PIN, 0 to 2 */
	0,				/* rsvd_bits:6; Reserved bits */
	0,              /* rsvd byte for align pan_info */
	{0, HAL_GPIO_O_DIR, 127},		    /* LATCH PIN */
	{0, HAL_GPIO_O_DIR, 	127},		/* CLOCK PIN */
	{1, HAL_GPIO_O_DIR, 	127},		/* DATA PIN */
	{{0, HAL_GPIO_I_DIR, 	127},		/* SCAN1 PIN */
	{0, HAL_GPIO_I_DIR, 127}},		    /* SCAN2 PIN */
	{{0, HAL_GPIO_O_DIR, 	127},		/* COM1 PIN */
	{0, HAL_GPIO_O_DIR, 	127},		/* COM2 PIN */
	{0, HAL_GPIO_O_DIR, 	127},		/* COM3 PIN */
	{0, HAL_GPIO_O_DIR, 	127},		/* COM4 PIN */
	{0, HAL_GPIO_O_DIR, 127},		    /* COM5 PIN */
	{0, HAL_GPIO_O_DIR, 127},		    /* COM6 PIN */
	{0, HAL_GPIO_O_DIR, 127},		    /* COM7 PIN */
	{0, HAL_GPIO_O_DIR, 127}},		    /* COM8 PIN */
	{{0, HAL_GPIO_O_DIR, 	127},		/* POWER PIN */
	{1, HAL_GPIO_O_DIR, 	127},		/* LOCK PIN */
	{0, HAL_GPIO_O_DIR, 127},		    /* Extend function LBD */
	{0, HAL_GPIO_O_DIR, 127}},		    /* Extend function LBD */
    {0, HAL_GPIO_O_DIR, 127},           /* rsvd extend function LBD */        
	300,							    /* Intv repeat first */
	250,							    /* Intv repeat */
	350,							    /* Intv release, 0: disable release key */
	NULL,	 	 	 	 	 	 	    /* hook_scan() callback */
	NULL,	 	 	 	 	 	 	    /* hook_show() callback */
	0 	 	 	 	 	 	 	        /* p_seg */
};
#define bitmap_list				NULL
#define bitmap_list_num		0
struct pan_configuration pan_config = {&pan_hw_info, bitmap_list_num, bitmap_list};
struct pan_device *g_erom_pan_dev = NULL;

#ifdef _M3821_
static gpio_info_t gpio_array[] = 
{    
    {1, HAL_GPIO_O_DIR, 129},       //XPMU_SDA
    {1, HAL_GPIO_O_DIR, 128},       //XPMU_SCL
    {0, HAL_GPIO_O_DIR, 118},       //MUTE
    {1, HAL_GPIO_O_DIR, 119},       //ANT 5V
    {1, HAL_GPIO_O_DIR, 6},         //NIM RESET GPIO
    {0, HAL_GPIO_I_DIR, 92},        //pok pin input mode
    {1, HAL_GPIO_O_DIR, 79},        //GPIO[79]/SW_5V/3V3
    {1, HAL_GPIO_O_DIR, 80}         //GPIO[80]/SW_3V3/1V8
};

enum{
    GPIO_I2C0_SDA_INDEX = 0,
    GPIO_I2C0_SCL_INDEX,
    SYS_MUTE_INDEX, 
    GPIO_ANT_5V,
    SYS_NIM_RESET0,
    GPIO_POK_PIN,
    SYS_SMC_SWITCH,
    SYS_SMC_SWITCH2
};
#elif defined(_M3505_)
//3527
static gpio_info_t gpio_array[] =
{
    {1, HAL_GPIO_O_DIR, 108},        //PANEL__SDA
    {1, HAL_GPIO_O_DIR, 107},        //PANEL_SCL
    {1, HAL_GPIO_O_DIR, 3},		     //hdmi_SDA
	{1, HAL_GPIO_O_DIR, 4},		     //hdmi_SCL
};

enum{
    GPIO_I2C0_SDA_INDEX = 0,
    GPIO_I2C0_SCL_INDEX,
    GPIO_I2C1_SDA_INDEX,
    GPIO_I2C1_SCL_INDEX,  
};

#endif

typedef struct
{
    UINT32 i2c_id;
    UINT32 bps;
    UINT32 en;
    gpio_info_t *sda;
    gpio_info_t *scl;
}i2c_cfg;

i2c_cfg i2c_config[6] =
{
    {I2C_TYPE_SCB0, 100000, 1, NULL, NULL},  	//hdmi
    {I2C_TYPE_SCB1, 100000, 1, NULL, NULL},     //inner nim:QAM(DVBC)
    {I2C_TYPE_SCB2, 100000, 1, NULL, NULL},     //inner nim:T2
    {I2C_TYPE_SCB3, 100000, 1, NULL, NULL},     //kent,inner nim:Full Nim 
    {I2C_TYPE_GPIO0, 40000, 1, &gpio_array[GPIO_I2C0_SDA_INDEX], &gpio_array[GPIO_I2C0_SCL_INDEX]}, //for panel CH455 panel 
};

// < --- enable front panel end
#endif

void bugen_crc32_table(void)
{
    unsigned int i, j;
    unsigned long crc32;

    for (i = 0; i < 256; i++)
    {
        crc32 = i;
        for (j = 8; j > 0; j--)
        {
            if (crc32 & 1)
    	        crc32 = (crc32 >> 1) ^ CRC32_POLYNOMIAL;
            else
    	        crc32 >>= 1;
        }
        crc_32_tab[i] = crc32;
    }
}

unsigned long gen_CRC32(unsigned long crc_32, unsigned char *p, \
    unsigned int len)
{
    unsigned char *pe = p + len;
    while (p != pe)
        crc_32 = crc_32_tab[(crc_32 & 0xFF) ^ *p++] ^ \
              (crc_32 >> 8);
    return crc_32;
}

void (* reboot_jump)(void);

void sys_reboot()
{
    __asm__ __volatile__("li $2, 0\n\t"
                         "move $30, $2\n\t"
                         "move $23, $2\n\t"
                         "move $21, $2\n\t"
                         );
    sys_ic_reboot();
}

unsigned long get_upgrade_param(unsigned long offset)
{
    return *((unsigned long *)(UPGRADE_PARAM_ADDR + offset));
}

unsigned long get_upgrade_param_ext(unsigned long offset)
{
    return *((unsigned long *)(UPGRADE_PARAM_ADDR_EXT + offset));
}

static void set_memory_byte_available()
{
    unsigned char bSetMem = 0;
    *((volatile unsigned char*)0xa0000001) = 0x33;
    if (*((volatile unsigned char*)0xa0000001) != 0x33)
    {
        bSetMem = 1;
    }
    else
    {
        *((volatile unsigned char*)0xa0000001) = 0xbb;
        if (*((volatile unsigned char*)0xa0000001) != 0xbb)
        {
            bSetMem = 1;
	}
    }

    if (1 == bSetMem)
    {
        *((volatile unsigned long*)0xb8000028) |= 0x400;
    }

}

static void config_uart_speed(UINT8 mode)
{
    if (mode != 1 && mode != 2)
        return;

	*((volatile UINT32 *)(0xB8001308)) = 0;	
	osal_task_sleep(100);

    if (1 == mode)
    	*((volatile UINT32 *)(0xB8000074)) = 0x40000000;
    else
    	*((volatile UINT32 *)(0xB8000074)) = 0x40008000;
	
	osal_task_sleep(100); // must delay here

	*((volatile UINT32 *)(0xB8001308)) = 0x08;	
	osal_task_sleep(100);
}

BOOL is_eromupg_burn_flash()
{
	if(flash_burn_flag)
		return TRUE;
	else
		return FALSE;
}

BOOL is_eromupg_bootloader_encryption()
{
	if(bloader_enc_flag)
		return TRUE;
	else
		return FALSE;
}

BOOL IsSwUk3LadderEnc()
{

	if((2==bloader_enc_flag) || (2==flash_burn_flag))
		return TRUE;
	else
		return FALSE;
}
int burn_basic_otp_for_3527(void)
{
    UINT32 addr_02 = 0;
    UINT32 addr_03 = 0;   
    UINT32 addr_dd = 0; 
	UINT32 key_num[4]={0x371E4132,0x40729D83,0x61D49966,0xAA0FF817};
	UINT32 key_temp[4]={0};
    UINT32 cstm_id = 0;
    UINT32 cstm_mask = 0;
	UINT32 cstm_id_offset = 0x140;
    UINT32 cstm_mask_offset = 0x144;
    UINT32 boot_sig_en = 1<<1;
    UINT32 enc_bl_en = 1<<13;
    int ret = 0;
    UINT32 i = 0;
    otp_init(NULL);	
    cstm_id = sto_fetch_long(cstm_id_offset);
    cstm_id = fetch_long((UINT8 *)&cstm_id);
    cstm_mask = sto_fetch_long(cstm_mask_offset);
    cstm_mask = fetch_long((UINT8 *)&cstm_mask);
    SH_PRINTF("cstm_id = 0x%08x,cstm_id = 0x%08x\n",cstm_id,cstm_mask);
    if(cstm_id)
    {
        SH_PRINTF("Burn CSTMID OTP\n");
        ret = otp_read(0x02*4, (UINT8 *)&addr_02, 4); 
        SH_PRINTF("Orig 0x2 data = 0x%08x,ret = %d\n",addr_02,ret);

        cstm_id = cstm_id&cstm_mask;
        addr_02 |= cstm_id;
        ret = otp_write((UINT8 *)&addr_02, 0x02*4, 4);       
        if(ret <=0)
        {
            SH_PRINTF("Write CSTM OTP error1\n"); 
        }

        ret = otp_read(0x02*4, (UINT8 *)&addr_02, 4); 
        SH_PRINTF("after 0x2 data = 0x%08x,ret = %d\n",addr_02,ret);  
        if((addr_02&cstm_mask) != cstm_id)
        {
            SH_PRINTF("Write CSTM OTP error2\n");
        }
    }


    SH_PRINTF("Burn BOOTROM SIG OTP\n");
    //bootrom sig check
    ret = otp_read(0xdd*4, (UINT8 *)&addr_dd, 4); 
    SH_PRINTF("Orig 0xdd data = 0x%08x,ret = %d\n",addr_dd,ret);
    addr_dd |= boot_sig_en;
    ret = otp_write((UINT8 *)&addr_dd, 0xdd*4, 4);       
    if(ret <=0)
    {
        SH_PRINTF("Write Boot Sig EN OTP error1\n"); 
    }

    ret = otp_read(0xdd*4, (UINT8 *)&addr_dd, 4); 
    SH_PRINTF("After 0xdd data = 0x%08x,ret = %d\n",addr_dd,ret);
    if((addr_dd & boot_sig_en) !=boot_sig_en)
    {
        SH_PRINTF("Write Boot Sig EN OTP error2\n"); 
    }

    SH_PRINTF("Burn Encrypted BL OTP\n");
    //enc bl
    ret = otp_read(0xdd*4, (UINT8 *)&addr_dd, 4); 
    SH_PRINTF("Orig 0xdd data = 0x%08x,ret = %d\n",addr_dd,ret);
    addr_dd |= enc_bl_en;
    ret = otp_write((UINT8 *)&addr_dd, 0xdd*4, 4);       
    if(ret <=0)
    {
        SH_PRINTF("Write ENCBL OTP error1\n"); 
    }

    ret = otp_read(0xdd*4, (UINT8 *)&addr_dd, 4); 
    SH_PRINTF("After 0xdd data = 0x%08x,ret = %d\n",addr_dd,ret);
    if((addr_dd & enc_bl_en) !=enc_bl_en)
    {
        SH_PRINTF("Write ENCBL OTP error2\n"); 
    } 
	
}

int burn_basic_otp_for_3716(void)
{
    UINT32 addr_02 = 0;
    UINT32 addr_03 = 0;   
    UINT32 addr_dd = 0;  
    UINT32 cstm_id = 0;
    UINT32 cstm_mask = 0;
	UINT32 cstm_id_offset = 0x140;
    UINT32 cstm_mask_offset = 0x144;
    UINT32 boot_sig_en = 1<<1;
    UINT32 enc_bl_en = 1<<13;
    int ret = 0;

    otp_init(NULL);
  sto_local_sflash_attach(NULL);
  sto_chunk_init(0, 0x800000);
	cstm_id = sto_fetch_long(cstm_id_offset);
	cstm_id = fetch_long((UINT8 *)&cstm_id);
	cstm_mask = sto_fetch_long(cstm_mask_offset);
	cstm_mask = fetch_long((UINT8 *)&cstm_mask);
    if(otp_burn_cstm_id)
    {
	    SH_PRINTF("cstm_id = 0x%08x,cstm_id = 0x%08x\n",cstm_id,cstm_mask);
	   
	    if(cstm_id)
	    {
	        SH_PRINTF("Burn CSTMID OTP\n");
	        ret = otp_read(0x02*4, (UINT8 *)&addr_02, 4); 
	        SH_PRINTF("Orig 0x2 data = 0x%08x,ret = %d\n",addr_02,ret);

	        cstm_id = cstm_id&cstm_mask;
	        addr_02 |= cstm_id;
	        ret = otp_write((UINT8 *)&addr_02, 0x02*4, 4);       
	        if(ret <=0)
	        {
	            SH_PRINTF("Write CSTM OTP error1\n"); 
	        }

	        ret = otp_read(0x02*4, (UINT8 *)&addr_02, 4); 
	        SH_PRINTF("after 0x2 data = 0x%08x,ret = %d\n",addr_02,ret);  
	        if((addr_02&cstm_mask) != cstm_id)
	        {
	            SH_PRINTF("Write CSTM OTP error2\n");
	        }
	    }
    }
	if(otp_burn_boot_rom_sig)
	{
	    SH_PRINTF("Burn BOOTROM SIG OTP\n");
	    //bootrom sig check
	    ret = otp_read(0xdd*4, (UINT8 *)&addr_dd, 4); 
	    SH_PRINTF("Orig 0xdd data = 0x%08x,ret = %d\n",addr_dd,ret);
	    addr_dd |= boot_sig_en;
	    ret = otp_write((UINT8 *)&addr_dd, 0xdd*4, 4);       
	    if(ret <=0)
	    {
	        SH_PRINTF("Write Boot Sig EN OTP error1\n"); 
	    }

	    ret = otp_read(0xdd*4, (UINT8 *)&addr_dd, 4); 
	    SH_PRINTF("After 0xdd data = 0x%08x,ret = %d\n",addr_dd,ret);
	    if((addr_dd & boot_sig_en) !=boot_sig_en)
	    {
	        SH_PRINTF("Write Boot Sig EN OTP error2\n"); 
	    }

	    SH_PRINTF("Burn Encrypted BL OTP\n");
	}
    //enc bl
    if(otp_burn_encrypt_bl)
    {
	    ret = otp_read(0xdd*4, (UINT8 *)&addr_dd, 4); 
	    SH_PRINTF("Orig 0xdd data = 0x%08x,ret = %d\n",addr_dd,ret);
	    addr_dd |= enc_bl_en;
	    ret = otp_write((UINT8 *)&addr_dd, 0xdd*4, 4);       
	    if(ret <=0)
	    {
	        SH_PRINTF("Write ENCBL OTP error1\n"); 
	    }

	    ret = otp_read(0xdd*4, (UINT8 *)&addr_dd, 4); 
	    SH_PRINTF("After 0xdd data = 0x%08x,ret = %d\n",addr_dd,ret);
	    if((addr_dd & enc_bl_en) !=enc_bl_en)
	    {
	        SH_PRINTF("Write ENCBL OTP error2\n"); 
	    }  
    }
}


void com_monitor(void)   
{
/***********************************
| data_len | data | crc |
***********************************/

    UINT32 mem_addr = RAM_CODE_START;
    UINT32 mem_addr_bak = RAM_CODE_START;
    UINT32 data_len = 0;
    UINT32 data_len_bak = 0;
    UINT32 crc = 0;
    UINT32 crc_new = 0;
    UINT32 receive_len = 0;
    UINT32 receive_data[2048];

    UINT32 temp_data = 0;

    int i=0;

    SH_PRINTF("com_monitor start\n");
	
    osal_interrupt_disable();

    //get data_len
    for(i=0;i<4;i++)
    {
        temp_data = (UINT32)(sci_16550uart_hight_speed_mode_read(SCI_FOR_RS232));
        data_len |= (temp_data<<(i*8));
    }
    data_len_bak = data_len;
	
    //get data
    for(i=0;i<data_len;i++)
    {
        *(unsigned char*)(mem_addr) = sci_16550uart_hight_speed_mode_read(SCI_FOR_RS232);
        mem_addr += 1;
    }

    //get crc
    for(i=0;i<4;i++)
    {
        temp_data = (UINT32)(sci_16550uart_hight_speed_mode_read(SCI_FOR_RS232));
        crc |= (temp_data<<(i*8));
    }
	
    //compare crc
    crc_new = mg_table_driven_crc(0xFFFFFFFF, mem_addr_bak, data_len_bak);
	
    SH_PRINTF("crc:%d, crc_new:%d\n", crc, crc_new);

    *(unsigned long*)0xa00001f8 = crc;
    *(unsigned long*)0xa00001fc = crc_new;

    if(crc_new != crc)
    {
        panel_show("ER01",4);
        SH_PRINTF("crc not patch\n");
        SDBBP();
    }	
	
    osal_interrupt_enable();
	
    return;
}

#if 0
int upgrade_all_3823(unsigned char mode)
{
    unsigned long temp_start;
    unsigned int sector_no;
    unsigned long len;
    unsigned char *p;
    unsigned int ret, unzip_len = 0;
    unsigned long binary_len;
    unsigned long crc;
    unsigned long crc1;
    unsigned int i;
    unsigned int erase_chip;
    int ret1;

    //receive data from uart
    com_monitor();
	
#ifdef SUPPORT_PANEL_DISPLAY
	pan_display(g_erom_pan_dev, "up8", 4);
#endif
	
	binary_len = get_upgrade_param(EUP_OFFSET(binary_len));
	if (binary_len == 0 || binary_len > flash_size_val)
		binary_len = flash_size_val;

    p = (unsigned char *)RAM_CODE_START;

#ifdef _M3821_
    recombine_image_ext(p, binary_len);
#endif

    crc = gen_CRC32(0xFFFFFFFF, p, binary_len) ^ 0xFFFFFFFF;
    SH_PRINTF("flash_size=0x%x, binary_len=0x%x, CRC=0x%08x, mode=0x%x\n", 
               flash_size_val, binary_len, crc, mode);
	
    erase_chip = (binary_len == flash_size_val);
    sector_no = 0;
    temp_start = 0;

    while (temp_start < binary_len)
    {
        len = flash_sector_size(sector_no);
        sec_flag[sector_no] = flash_verify(temp_start, p, len);
        if (sec_flag[sector_no] < 2)
        {
            erase_chip = 0;
        }
        sector_no++;
        p += len;
        temp_start += len;
    }
   
    p = (unsigned char *)RAM_CODE_START;
    i = 0;
    temp_start = 0;
	
    while (temp_start < binary_len)
    {
        len = flash_sector_size(i);
        switch (sec_flag[i])
        {
        case 2:
            flash_erase_sector(temp_start);
        case 1:
            ret = flash_copy(temp_start, p, len);
            if (ret == 0)
                ret = flash_verify(temp_start, p, len);
            break;
        default:
            ret = 0;
        }
        if (ret == 0)
        {
            SH_PRINTF("%08X OK\n", temp_start);
        }
        else
        {
            SH_PRINTF("%08X Fail\n", temp_start);
        }
		
        i++;
        p += len;
        temp_start += len;
    }
   
    if (binary_len <= 0x400000)
    {
        crc1 = gen_CRC32(0xFFFFFFFF, (unsigned char *)SYS_FLASH_BASE_ADDR, binary_len) ^ 0xFFFFFFFF;
        SH_PRINTF("Burn %08X %s, crc=0x%08x, crc1=0x%08x\n", temp_start,
		  crc == crc1 ? "OK" : "Fail", crc, crc1);
    }
    else if (binary_len <= 0x800000)
    {
        crc1 = gen_CRC32(0xFFFFFFFF, (unsigned char *)SYS_FLASH_BASE_ADDR, 0x400000);
        crc1 = gen_CRC32(crc1, (unsigned char *)(SYS_FLASH_BASE_ADDR - 0x400000), \
		         binary_len - 0x400000) ^ 0xFFFFFFFF;
    }
       
    if(crc != crc1)
    {   
        SDBBP();
    }
//-->watson add 
#if defined (_CAS9_CA_ENABLE_)   //cnx_se_chunk_hdr
	 //cnx_se_chunk_hdr
	#define NEW_HEAD_ID             0xaead0000 
	//#define BL_CODE_SIZE 0x40000
	//#define BL_CODE_SIZE 0x80000
	UINT32 bl_code_size = 0;
	
	get_bootloader_size_from_flash(&bl_code_size);

	struct sto_device *sto_dev = NULL;
	sto_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
	sto_open(sto_dev);
	sto_chunk_init(bl_code_size, sto_dev->totol_size - bl_code_size);
	//soc_printf("==>sto_dev->totol_size:    %d\n",sto_dev->totol_size);
	RET_CODE ret_code = RET_FAILURE;


	ret_code = update_allhdr_hmac(FALSE,0, NULL, 0);
	if(RET_SUCCESS != ret)
		return ret;
	ret_code = update_chunk_hmac(NEW_HEAD_ID);
	if(RET_SUCCESS != ret)
		return ret;
#endif
//<--add end

    //burn otp
    if (is_eromupg_bootloader_encryption())
    {
        eromupg_burn_otp_3823();
    }

#ifdef SUPPORT_PANEL_DISPLAY
    pan_display(g_erom_pan_dev, "up2", 4);
#endif
	
    //reboot
    *(volatile UINT32 *)(0xb8018500) = 0xffffff00;
    *(volatile UINT32 *)(0xb8018504) |= 0x64;
    osal_delay(1000000);
    sys_watchdog_reboot();
	
    return 0;
}
#else
int upgrade_all_3505(unsigned char mode)
{
    unsigned long temp_start;
    unsigned int sector_no;
    unsigned long len;
    unsigned char *p;
    unsigned int ret, unzip_len = 0;
    unsigned long binary_len;
    unsigned long crc;
    unsigned long crc1 = 0;
    unsigned char sector_flag = 0;
    unsigned int i;
    unsigned int erase_chip;
    unsigned char retry_cnt = 0;
    int ret1;

    UINT32 bk_encry_id = 0x07F80100;
    UINT32 uk_encry_id = DECRYPT_KEY_ID;


    unsigned char strbuf[16];
    unsigned long progress;

    sprintf(strbuf,"%s%d","u2p",bloader_enc_flag);
    panel_show(strbuf,4);

    //receive data from uart
    com_monitor();
	
	binary_len = get_upgrade_param(EUP_OFFSET(binary_len));
	if (binary_len == 0 || binary_len > flash_size_val)
	{
		binary_len = flash_size_val;
	}

    p = (unsigned char *)RAM_CODE_START;

    //recombine_image_ext(p, binary_len);

    crc = gen_CRC32(0xFFFFFFFF, p, binary_len) ^ 0xFFFFFFFF;
    SH_PRINTF("flash_size=0x%x, binary_len=0x%x, CRC=0x%08x, mode=0x%x\n", 
               flash_size_val, binary_len, crc, mode);
	
    erase_chip = (binary_len == flash_size_val);
    sector_no = 0;
    temp_start = 0;

    p = (unsigned char *)RAM_CODE_START;
    i = 0;
    temp_start = 0;
    panel_show("bun ",4);

    
    while (temp_start < binary_len)
    {
        if(retry_cnt == 0)
        {
            len = flash_sector_size(i);
        }
        else if(retry_cnt == 1)
        {
            len= 0x10000;//64K            
        }
        
        sector_flag = flash_verify(temp_start, p, len);
        switch (sector_flag)
        {
        case 2:
            flash_erase_sector(temp_start);
        case 1:
            ret = flash_copy(temp_start, p, len);
            if (ret == 0)
                ret = flash_verify(temp_start, p, len);
            break;
        default:
            ret = 0;
        }
        if (ret == 0)
        {
            SH_PRINTF("%08X OK\n", temp_start);
     
            progress = temp_start*100/binary_len;
            if (progress < 100)
                sprintf(strbuf,"b%.2d",progress);
            else
                sprintf(strbuf,"100 ");
            //pan_display(g_erom_pan_dev, strbuf, 4);
            panel_show(strbuf,4);               
        }
        else
        {
            SH_PRINTF("%08X Fail\n", temp_start);
            progress = temp_start*100/binary_len;
            if (progress < 100)
                sprintf(strbuf,"E%.2d",progress);
            else
                sprintf(strbuf,"100 ");
            //pan_display(g_erom_pan_dev,  strbuf, 4);
            panel_show(strbuf,4);      
            //
            if(0 == retry_cnt)
            {
                retry_cnt++;
                continue;
            }
            else
            {
                while(1);
            }            
        }
        i++;
        p += len;
        temp_start += len;
    }

      
    sprintf(strbuf,"b100");
    panel_show(strbuf,4);  

    sprintf(strbuf,"CRC0");
    panel_show(strbuf,4); 

	if (binary_len <= 0x400000)
	{
		crc1 = gen_CRC32(0xFFFFFFFF, (unsigned char *)SYS_FLASH_BASE_ADDR, binary_len) ^ 0xFFFFFFFF;
		SH_PRINTF("Burn %08X %s, crc=0x%08x, crc1=0x%08x\n", temp_start,
		          crc == crc1 ? "OK" : "Fail", crc, crc1);
	}
	else if (binary_len <= 0x800000)
	{
		crc1 = gen_CRC32(0xFFFFFFFF, (unsigned char *)SYS_FLASH_BASE_ADDR, 0x400000);
		crc1 = gen_CRC32(crc1, (unsigned char *)(SYS_FLASH_BASE_ADDR - 0x400000), \
		                 binary_len - 0x400000) ^ 0xFFFFFFFF;
	}
    
    if(crc != crc1)
    { 
        panel_show("ER04",4);
	    SDBBP();
    }

    sprintf(strbuf,"CRC1");
    panel_show(strbuf,4); 
    
#if 1
    panel_show("UH00",4);
    #define NEW_HEAD_ID             0xaead0000 
	//#define BL_CODE_SIZE 0x40000
	//#define BL_CODE_SIZE 0x80000
	UINT32 bl_code_size = 0;
    RET_CODE ret_code = RET_FAILURE;
    struct sto_device *sto_dev = NULL;
	
//	get_bootloader_size_from_flash(&bl_code_size);	
	sto_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
	sto_open(sto_dev);
	sto_chunk_init(bl_code_size, sto_dev->totol_size - bl_code_size);

    #if 0
    ret_code =update_allhdr_hmac();
    if(RET_SUCCESS != ret)
    {
        SH_PRINTF("update_allhdr_hmac failed \n");
    	return ret_code;
    }
    #endif
	#if 0
    ret_code= bl_update_chunk_hmac(NEW_HEAD_ID);
    if(RET_SUCCESS != ret)
    {
        SH_PRINTF("bl_update_chunk_hmac failed \n");
    	return ret_code;
    }
	#endif
    panel_show("UH01",4);
#endif

	//burn otp
	if(is_eromupg_bootloader_encryption())
	{
        panel_show("botp", 4);
		burn_basic_otp_for_3527();
	}
	
	return 0;
}
#endif

#ifdef _M3821_
#define MAX_SEE_LEN  1000000       //10M
static RET_CODE set_see_parameter(UINT32 run_addr, UINT32 code_addr,UINT32 len)
{
	*(volatile UINT32 *)(SEEROM_SEE_REAL_RUN_ADDR) = run_addr|0xa0000000;
	osal_cache_flush(code_addr,len);
	//*(volatile UINT32 *)(SEEROM_SEE_CODE_LEN_ADDR) = *(volatile UINT32 *)(0xa70001f0);
	*(volatile UINT32 *)(SEEROM_SEE_CODE_LEN_ADDR) = len;
	*(volatile UINT32 *)(SEEROM_SEE_CODE_LOCATION_ADDR) = code_addr|0xa0000000;      
	*(volatile UINT32 *)(SEEROM_SEE_SIGN_LOCATION_ADDR) = 0xa7000200;
	return RET_SUCCESS;
}
//3821
static void see_boot(UINT32 addr)
{
	/*run from AS bl*/
	if (is_hw_ack_flag_true(SEE_ROM_RUN_BIT))
	{  
  		set_main_trig_sw_run();
		osal_delay(100);
		/*wait see software run flag*/
        while(1)
        {
        	if((*(volatile UINT32 *)(SEEROM_MAIN_RUN_STATUS_ADDR)) == SEE_SW_RUN_FLAG )
            {
                break;
			}
		}
  	    return ;    	
  	}
	
	if (*(volatile UINT32 *)(0xb8000200) != 0xb8000280)
	{
        return;
	}
	
	if ( sys_ic_seerom_is_enabled())
	{  
		*(volatile UINT32 *)(0xb8000220) &= ~(0x2);
		set_see_parameter(addr, addr,MAX_SEE_LEN);
		*(volatile UINT32 *)(0xb8000220) |= 0x2; 
		osal_task_sleep(400);
		*(volatile UINT32 *)(0xb8000200) = addr; 
	}
	else
	{   
		*(volatile UINT32 *)(0xb8000200) = 0xb8000280;  //init see	
		*(volatile UINT32 *)(0xb8000220) |= 0x2; 
		addr = (addr & 0x0FFFFFFF) | 0x80000000;		
		*(volatile UINT32 *)(0xb8000200) = addr; 
		*(volatile UINT32 *)(0xb8000220) |= 0x2; 		
	}	
	
	osal_task_sleep (1000);
}
#elif defined(_M3505_)

#define MAX_SEE_LEN  500000       //5M
static RET_CODE set_see_parameter(UINT32 run_addr, UINT32 code_addr,UINT32 len)
{
	*(volatile UINT32 *)(SEEROM_SEE_REAL_RUN_ADDR) = run_addr|0xa0000000 ;
	osal_cache_flush((void *)code_addr,len);
    #if 1//ndef _USE_64M_MEM_
	*(volatile UINT32 *)(SEEROM_SEE_CODE_LEN_ADDR) = len ;//*(volatile UINT32 *)(0xa70001f0) ;
    *(volatile UINT32 *)(SEEROM_SEE_CODE_LOCATION_ADDR) = code_addr|0xa0000000 ;      
	*(volatile UINT32 *)(SEEROM_SEE_SIGN_LOCATION_ADDR) = 0xa7000200 ;
    #else
    *(volatile UINT32 *)(SEEROM_SEE_CODE_LEN_ADDR) = *(volatile UINT32 *)(0xa3d001f0) ;
    *(volatile UINT32 *)(SEEROM_SEE_CODE_LOCATION_ADDR) = code_addr|0xa0000000 ;      
	*(volatile UINT32 *)(SEEROM_SEE_SIGN_LOCATION_ADDR) = 0xa3d00200 ;
    #endif
	
	return RET_SUCCESS;
}


#define IS_DOWNLOAD_DEBUG (((*(volatile UINT32 *)(0xb8000000+SYS_FIRST_BOOT_REG)&SYS_FIRST_BOOT_BIT)) == 0ul)
//3505						   
static void see_boot(UINT32 addr)
{
    //libc_printf("enter %s\n",__FUNCTION__);
    /*run from AS bl*/
    if(is_hw_ack_flag_true(SEE_ROM_RUN_BIT))
    {
        check_see_clock();
        return ;
    }

    if(*(volatile UINT32 *)(0xb8000200) != 0xb8000280)
    {
        //libc_printf("see_boot : return\n");
        return;
    }


    if( sys_ic_seerom_is_enabled())
    {
        *(volatile UINT32 *)(0xb8000220) &= ~(0x2);
		if(IS_DOWNLOAD_DEBUG)
		{
        	set_see_parameter(addr, addr,MAX_SEE_LEN);
		}
		*(volatile UINT32 *)(0xb8000220) |= 0x2;
        osal_task_sleep(400);
        *(volatile UINT32 *)(0xb8000200) = addr;
    }
    else
    {

        UINT32 i = 0;		
		*(volatile UINT32 *)(0xb8000220) &= ~(0x2); 
		addr = (addr & 0x0FFFFFFF) | 0xA0000000;		
		*(volatile UINT32 *)(0xb8000200) = addr; 
	
		while ( *(volatile UINT32 *)(0xb8000200) != addr )
		for(i=0;i<1000;i++);
        *(volatile UINT32 *)(0xb8000220) |= 0x2;
    }

    osal_task_sleep (1000);    
}

#else //3281
static void see_boot(UINT32 addr)
{
    if (*(volatile UINT32 *)(0xb8000200) != 0xb8000280)
    {
        return;
    }
	
    *(volatile UINT32 *)(0xb8000200) = addr; 
    *(volatile UINT32 *)(0xb8000220) |= 0x2; 
}

#endif

void panel_hw_init(void)
{
#ifdef SUPPORT_PANEL_DISPLAY
    // parameters for chip and gpio
    chip_init_param chip_param;
    chip_param.mem_gpio_enabled = FALSE;
	chip_param.ci_power_ctl_enabled = FALSE;

	UINT8 all_gpio_count = (sizeof(gpio_array) / sizeof(gpio_array[0]));
	gpio_info_t *all_gpio_array = gpio_array;

    // parameters for I2C
    UINT8 i = 0;
    UINT32 i2c_id = 0;
    gpio_info_t *sda = NULL;
    gpio_info_t *scl = NULL;
	
	i2c_cfg *erom_i2c_config[6];
    UINT8 i2c_gpio_num;
    UINT8 i2c_scb_num;

    // parameters for panel
    UINT32 pan_i2c_id = I2C_TYPE_GPIO0;
    INT32 ret_code = -1;

    // chip and gpio initialization --->
	global_chip_set(chip_create());
	chip_init(&chip_param);

	chip_gpio_pin_init_ext(all_gpio_array, all_gpio_count);
	// < --- chip and gpio initialization end
	

    // I2C initialization --->
	for (i = 0; i < 6; i++)
	{
		erom_i2c_config[i] = &i2c_config[i];
	}

	i2c_gpio_num = 2;
	i2c_scb_num = 4;

	i2c_scb_attach(i2c_scb_num);
	i2c_gpio_attach(i2c_gpio_num);

	for (i = 0; i < 6; i++)
	{
	    if (NULL != erom_i2c_config[i])
	    {
	        i2c_id = erom_i2c_config[i]->i2c_id;
			sda = erom_i2c_config[i]->sda;
			scl = erom_i2c_config[i]->scl;
			if((I2C_TYPE_GPIO == (i2c_id & I2C_TYPE_MASK)) && (NULL != sda) && (NULL != scl))
			{
			    i2c_gpio_set(i2c_id & I2C_ID_MASK, sda->position, scl->position);
			}

			if (0 != erom_i2c_config[i]->bps)
			{
			    i2c_mode_set(i2c_id, erom_i2c_config[i]->bps, 1);
			}
	    }
	}
	// <--- I2C initialization end
	

    // enable panel (CH455H) --->
    pan_ch455_attach(&pan_config);
	g_erom_pan_dev = (struct pan_device *)dev_get_by_id(HLD_DEV_TYPE_PAN, 0);
	if (NULL != g_erom_pan_dev)
	{
	    pan_ch455_id_set(g_erom_pan_dev, pan_i2c_id);
		ret_code = pan_open(g_erom_pan_dev);		
	}
    // < --- enable panel (CH455H) end

    if (SUCCESS == ret_code)
	{
	    pan_display(g_erom_pan_dev, "on", 4);
	}    
#endif
}

void panel_show(char* str,int len)
{
#ifdef SUPPORT_PANEL_DISPLAY
    int slen = len <=4 ? len : 4;
    pan_display(g_erom_pan_dev, str, slen);
#endif
}

static void uart_init(void)
{
    // initialize uart --->
    sci_16550uart_attach(1);	/* Use just 1 UART for RS232 */
    sci_mode_set(SCI_FOR_RS232, 115200, SCI_PARITY_EVEN);
    osal_task_sleep(100);

    // < --- initialize uart end
}

static void system_hardware_init(void)
{ 
    panel_hw_init();
    return;
}

static void eromupg_burn_otp_3505()
{
    
}

#ifdef ENABLE_USB_INTF
#include <api/libsi/si_tdt.h>
void get_local_time(date_time *dt)
{
    if(dt)
    {
        dt->year = 2016;
        dt->month = 3;
        dt->day = 4;
        dt->hour = 12;
        dt->min = 0;
        dt->sec = 0;
    }
}

#define SEE_FILE_NAME "/mnt/uda1/sw_init_see.abs"
#define MP_FILE_NAME "/mnt/uda1/product_sabbat_dual.abs"    

static UINT32 usb_mounted = FALSE;
static UINT32 see_fw_state = 0;

int get_file_len(char* file_name)
{
    int flen = 0;
    FILE * file = NULL;
    file = fopen(file_name,"r");
    if(NULL == file)
    {
        SH_PRINTF("fopen file (%s) \n",file_name);
        return 0;
    }
    fseek(file,0,SEEK_END);
    flen = ftell(file);
    SH_PRINTF("[tell]file size = %d \n",flen);    
    
    fclose(file);

    return flen;
}

int read_file(char* file_name,char *buf, int offset,int buf_len)
{
#if 0    
    struct stat file_state;  
    int fret = 0;
    int flen = 0;
    int fd = fs_open(file_name, O_RDONLY, 0777);
    if(fd < 0)
    {
        SH_PRINTF("open file (%s) \n",file_name);
    }

    memset(&file_state,0,sizeof(file_state));

    fret = fs_fstat(fd,&file_state);
    if(fret < 0)
    {
        SH_PRINTF("read file stats fail (%d) \n",fret);
    }
    SH_PRINTF("file size = %d \n",file_state.st_size);

    memset(&file_state,0,sizeof(file_state));
    fret = fs_stat(file_name, &file_state);
    if(fret < 0)
    {
        SH_PRINTF("read file stats fail (%d) \n",fret);
    }
    SH_PRINTF("file size = %d \n",file_state.st_size);

    //fs_seek();
    fs_close(fd); 
#else
    int rlen = 0;
    FILE * file = NULL;
    file = fopen(file_name,"r");
    if(NULL == file)
    {
        SH_PRINTF("fopen file (%s) \n",file_name);
        return 0;
    }
    fseek(file,offset,SEEK_SET);
    rlen = fread(buf,buf_len,1,file);
    SH_PRINTF("Read len = %d \n",rlen);    
    
    fclose(file);

    return rlen;
#endif
}

static int dsc_is_ready(void)
{
    p_dsc_dev    pdscdev = NULL;
    p_ce_device  pcedev = NULL;
    
    pdscdev =(p_dsc_dev)dev_get_by_id(HLD_DEV_TYPE_DSC, 0);
    if(NULL == pdscdev)
    {
        return 0;
    }

    pcedev =(p_dsc_dev)dev_get_by_id(HLD_DEV_TYPE_CE, 0);
    if(NULL == pcedev)
    {
        return 0;
    }

    return 1;
}

static RET_CODE encrypt_sw_universal_key_by_keyfp(UINT8* clear_key,UINT8* cipher_key)
{
    RET_CODE ret=RET_FAILURE;   
    
	ret = aes_128_pure_ecb_crypt_keyotp(clear_key,cipher_key, 16, 
			DSC_ENCRYPT, OTP_KEY_FROM_FP);
	if(RET_SUCCESS != ret )
	{
		SH_PRINTF("%s aes_128_pure_ecb_crypt_keyotp en error!\n", __FUNCTION__);
		return ret;
	}

    return RET_SUCCESS;
}


static RET_CODE decrypt_universal_key_by_aes(UINT8* cipher_key,UINT8* clear_key)
{
    RET_CODE ret=RET_FAILURE;  
	
#ifdef SUPPORT_KEY_PROTECT    
	ret = aes_pure_ecb_crypt(key_of_key,cipher_key,clear_key,16,DSC_DECRYPT);
#endif

	if(RET_SUCCESS != ret )
	{
		SH_PRINTF("%s aes_pure_ecb_crypt de error!\n", __FUNCTION__);
		return ret;
	}

    return RET_SUCCESS;
}

//static unsigned char key_of_key[16] = {0xa1,0xb2,0xc3,0xd4,0xe5,0xf6,0xa7,0xb8,0xc1,0xd2,0xe3,0xf4,0x15,0x36,0x57,0x78};
static void key_dec(unsigned char* cipher_key,unsigned char *clear_key)
{
    int i = 0;    
    for(i = 0 ; i < 16 ; i++)
    {
       #ifdef SUPPORT_KEY_PROTECT   
        clear_key[i] = cipher_key[i] ^key_of_key[i];
	   #endif
    }
}

static int is_full_zero(UINT8* buf,UINT32 len)
{
    int i = 0;
    for(i = 0 ; i < len ; i ++)
    {
        UINT8 data = buf[i];
        if(data)
        {
            return 0;
        }
    }

    return 1;
}

static int is_key_enc(UINT8* buf,UINT32 len)
{
    int i = 0;
    for(i = 0 ; i < len ; i ++)
    {
        UINT8 data = buf[i];
        if(data != 0xAE)
        {
            return 0;
        }
    }

    return 1;
}

static int sw_universal_key_enc(char *buf, int len)
{
    int sw_uk_offset = 0x1880;  
    UINT8 orig_key_area[64];
    UINT8 key_area[64];
    int ret = 0;
    UINT8 *sw_key_ptr = buf+sw_uk_offset;

    MEMSET(orig_key_area,0,sizeof(orig_key_area));
    MEMSET(key_area,0,sizeof(key_area));

    MEMCPY(orig_key_area,sw_key_ptr,64);

    if(is_key_enc(&orig_key_area[32],16))
    {
        //ret = decrypt_universal_key_by_aes(&orig_key_area[48],&orig_key_area[0]);
        key_dec(&orig_key_area[48],&orig_key_area[0]);
    }
    
    if(!is_full_zero(&orig_key_area[0],16))
    {
        ret = encrypt_sw_universal_key_by_keyfp(&orig_key_area[0],&key_area[16]);
        if(ret == 0)
        {
            MEMCPY(sw_key_ptr,key_area,64);
        }
    }

    return ret;
}

static int bl_universal_key_enc(char *buf, int len)
{
    int bl_uk_offset = 0x6e0;  
    UINT8 orig_key_area[32];
    UINT8 key_area[32];
    UINT8 random_num[16];
    int ret = 0;
    UINT8 *bl_key_ptr = buf+bl_uk_offset;

    MEMSET(orig_key_area,0,sizeof(orig_key_area));
    MEMSET(key_area,0,sizeof(key_area));

    MEMCPY(orig_key_area,bl_key_ptr,32);

    if(is_key_enc(&orig_key_area[0],16))
    {
        //ret = decrypt_universal_key_by_aes(&orig_key_area[16],&orig_key_area[0]);
        key_dec(&orig_key_area[16],&orig_key_area[0]);
    }
    
    if(!is_full_zero(&orig_key_area[0],16))
    {
        ret = ali_dsc_encrypt_bl_uk(&orig_key_area[0], &random_num[0], &key_area[16], (OTP_KEY_0_7-OTP_KEY_0_6_R));
        //ret = encrypt_universal_key_ext(&orig_key_area[0],&key_area[0]);
        ret = encrypt_sw_universal_key_by_keyfp(&orig_key_area[0],&key_area[16]);
        if(ret == 0)
        {
            MEMCPY(bl_key_ptr,key_area,32);
        }
    }

    return ret;
}

static int decrypt_data_by_aes(UINT8 *buf, UINT32 len,UINT8 *outb)
{
    char key[16] = {0xB7,0xBB,0xDC,0xA7,0xA3,0x51,0x4B,0x65,
                0xA5,0x4E,0x10,0xCF,0x13,0xA3,0x38,0x90};
    char iv[16];
    char* out_buf = NULL;
    int ret = -1;

    MEMSET(iv,0,sizeof(iv));

    out_buf = MALLOC(len);
    MEMSET(out_buf,0,len);
    //osal_cache_flush(buf,len);
    //osal_cache_invalidate(out_buf,len);
    
    ret = aes_pure_cbc_crypt(key,iv,buf,out_buf,len,DSC_DECRYPT);
    if(ret != 0)
    {
        SH_PRINTF("AES decrypt failed \n");
        goto EXIT;
    }

    if(outb)
    {
        MEMCPY(outb,out_buf,len);
    }
EXIT:    
    free(out_buf);
    return ret;
}

static int decrypt_data_by_swaes(UINT8 *buf, UINT32 len,UINT8 *outb)
{
    char key[16] = {0xB7,0xBB,0xDC,0xA7,0xA3,0x51,0x4B,0x65,
                0xA5,0x4E,0x10,0xCF,0x13,0xA3,0x38,0x90};
    char iv[16];
    char* out_buf = NULL;
    int ret = 0;
 //   AES_KEY aes_key;

    MEMSET(iv,0,sizeof(iv));
  //  MEMSET(&aes_key,0,sizeof(aes_key));
    
    out_buf = MALLOC(len);
    MEMSET(out_buf,0,len);
    //osal_cache_flush(buf,len);
    //osal_cache_invalidate(out_buf,len);

 //   AES_set_decrypt_key(key, 128, &aes_key);
  //  AES_cbc_encrypt(buf, out_buf, len, &aes_key, iv, AES_DECRYPT);

    if(outb)
    {
        MEMCPY(outb,out_buf,len);
    }
    
    free(out_buf);
    return 0;
}

int aes_test(char *buf, int len)
{    
    UINT8 *out_buf1 = NULL;
    UINT8 *out_buf2 = NULL;
    int ret = 0;

    UINT32 decrypt_offset = 0x80080;
    UINT32 decrypt_len = 0x1B8020;
    
    out_buf1 = MALLOC(decrypt_len);
    out_buf2 = MALLOC(decrypt_len);

    ret = decrypt_data_by_aes(buf+decrypt_offset,decrypt_len,out_buf1);
    if(ret != 0)
    {
        goto EXIT;
    }
    
    ret = decrypt_data_by_swaes(buf+decrypt_offset,decrypt_len,out_buf2);
    if(ret != 0)
    {
        goto EXIT;
    }
    ret = MEMCMP(out_buf1,out_buf2,decrypt_len);
    if( ret!= 0)
    {
        SH_PRINTF("hw sw aes output diff !!!\n");
    }  

EXIT:    
    FREE(out_buf1);
    FREE(out_buf2);

    return ret;
}

typedef enum
{
    USBC_MSG_FS_MOUNT,
    USBC_MSG_FS_UNMOUNT,
}usbc_msg_type_t;

typedef struct
{
    usbc_msg_type_t    msg_type;
    UINT32                msg_code;
}usbc_msg_t, *p_usbc_msg_t;

static int usb_on(void)
{
    return usb_mounted;
}

static int need_load_see(void)
{
    int ret = see_fw_state ? 0 : 1;
    return ret;
}

static void load_see_finish()
{
    see_fw_state = 1;
}

static int see_load_finished()
{
    return see_fw_state;
}

static void usb_mon_task(void)
{
    SH_PRINTF("usb task start to run\n");
    int flen = 0;
    int srlen = 0;
    int fret = 0;
    UINT8* see_buf = 0xA6900200;
    UINT8* buf = NULL;
    int ret = 0;
    
    while(1)
    {
        if(usb_on())
        {
            #ifdef LOAD_SEE_FORM_USB
            if(need_load_see())
            {
                flen = get_file_len(SEE_FILE_NAME);
                if(flen)
                {
                    SH_PRINTF("Prepare to load see (%s)\n",SEE_FILE_NAME);
                    while(srlen < flen && usb_on())
                    {
                        buf = see_buf+srlen;
                        fret = read_file(SEE_FILE_NAME,buf,srlen,flen - srlen);
                        SH_PRINTF("\t %02x %02x %02x %02x\n",buf[0],buf[1],buf[2],buf[3]);

                        srlen += fret;
                        if(srlen == flen )
                        {
                            SH_PRINTF("Load SEE finished \n");
                            load_see_finish();                         
                        }
                        else
                        {
                            SH_PRINTF("Total len = %d, Haved load len = %d\n",flen,srlen);
                        }
                    }
                }                
            }
            #endif

            #ifdef SUPPORT_KEY_PROTECT
            if(!dsc_is_ready())
            {
                //SH_PRINTF("DSC is not ready!!! \n");
                osal_task_sleep(100);
                continue;
            }
            #endif

            load_mpclient_config();
            if(see_load_finished())
            {
                int flen = get_file_len(MP_FILE_NAME);
                if(flen > 0)
                {
                    SH_PRINTF("Prepare to load product (%s)\n",MP_FILE_NAME);
                    
                    //UINT8* buf = malloc(flen);
                    UINT8* buf =__MM_PVR_VOB_BUFFER_ADDR;
                    srlen = 0;
                  

                    if(NULL == buf)
                    {
                        SH_PRINTF("malloc failed, len = %d\n",flen);
                        continue;
                    }                    
                    
                    while(srlen < flen && usb_on())
                    {
                        fret = read_file(MP_FILE_NAME,buf+srlen,srlen,flen - srlen);
                        SH_PRINTF("\t %02x %02x %02x %02x\n",buf[0],buf[1],buf[2],buf[3]);

                        srlen += fret;
                    }

                    #if 0
                    ret = aes_test(buf,flen);
                    if(ret != 0)
                    {
                        SH_PRINTF("AES test failed \n");
                    }
                    else
                    {
                        SH_PRINTF("AES test OK \n");
                    }
                    #endif
                    
                    #ifdef SUPPORT_KEY_PROTECT
                    sw_universal_key_enc(buf,flen);
                    bl_universal_key_enc(buf,flen);
                    #endif

                    if(srlen == flen)
                    {       
                        SH_PRINTF("Load product finished\n");
                        mp_state = E_MP_BURN_FLASH;
                        ret = usb_burn_flash(buf,flen);
                        if(ret == 0)
                        {
							if(burn_ret2gca_otp)
							{
                               SH_PRINTF("Prepare to burn otp\n");	   
							   burn_basic_otp_for_genca();
							   burn_basic_otp_for_3716();
							}
                            SH_PRINTF("All finished !!! \n");
                            mp_state = E_MP_FINISH;   
                            return;
                        }
                    }
                    else
                    {
                        SH_PRINTF("Load product failed \n");
                    }
                    //free(buf);
                }
                else
                {
                    SH_PRINTF("Cannot find product (%s)\n",MP_FILE_NAME);
                }
            }
        }
        else
        {
            srlen = 0;
		//	SH_PRINTF("Please Insert USB DISK\n");
        }
        osal_task_sleep(10);
    }
    SH_PRINTF("usb task start finish\n");
}

static BOOL create_usb_monitor_task(void)
{
    ID usb_task_id = OSAL_INVALID_ID;
    ID usb_mbf_id = OSAL_INVALID_ID;
    OSAL_T_CMBF t_usbmbf;
    OSAL_T_CTSK t_usbtsk;

	MEMSET(&t_usbmbf, 0, sizeof(OSAL_T_CTSK));
    MEMSET(&t_usbtsk, 0, sizeof(OSAL_T_CTSK));

    usb_mbf_id = OSAL_INVALID_ID;   
    t_usbmbf.bufsz = 30 * sizeof(usbc_msg_t);
    t_usbmbf.maxmsz = sizeof(usbc_msg_t);
    t_usbmbf.name[0] = 'U';
    t_usbmbf.name[1] = 'M';
    t_usbmbf.name[2] = 'M';   
     
    usb_mbf_id = osal_msgqueue_create(&t_usbmbf);
    if (OSAL_INVALID_ID == usb_mbf_id)
    {
        SH_PRINTF("create usb mbf failed\n");
        return FALSE;
    }

    usb_task_id = OSAL_INVALID_ID;
    t_usbtsk.stksz = 0x4000 ;
    t_usbtsk.quantum = 10;
    t_usbtsk.itskpri = OSAL_PRI_NORMAL;
    t_usbtsk.name[0] = 'U';
    t_usbtsk.name[1] = 'M';
    t_usbtsk.name[2] = 'T';
    t_usbtsk.task = (FP) usb_mon_task;
    usb_task_id = osal_task_create(&t_usbtsk);
    if (OSAL_INVALID_ID == usb_task_id)
    {
        SH_PRINTF("create usb task failed\n");
        return FALSE;
    }
    
    return TRUE;
}

#define FLASH_START_ADDR	0xAFC00000
#define BOOT_TOTAL_AREA_LEN_OFFSET      0x264
#define BOOT_TOTAL_AREA_LEN_OFFSET_N3 	0x284
#define GET_DWORD(i)    (*(volatile UINT32 *)(i))
static RET_CODE get_bl_size_from_flash(UINT32* len)
{
    if (NULL == len)
    {
        return RET_FAILURE;
    }

    if(sys_ic_get_chip_id() == ALI_C3505)
    {
        *len = GET_DWORD((UINT32)FLASH_START_ADDR + BOOT_TOTAL_AREA_LEN_OFFSET_N3);
    }
    else
    {
        *len = GET_DWORD(FLASH_START_ADDR + BOOT_TOTAL_AREA_LEN_OFFSET);
    }
    
    return RET_SUCCESS;
}

static int usb_burn_flash(int data_addr, int data_len)
{
    unsigned long temp_start;
    unsigned int sector_no;
    unsigned long len;
    unsigned char *p;
    unsigned int ret, unzip_len = 0;
    unsigned long binary_len;
    unsigned long crc;
    unsigned long crc1;
    unsigned int i;
    unsigned int sect_flag = 0;
    int ret1;
	//unsigned char sec_flag[512];
    char strbuf[64];
    int progress = 0;
    unsigned int sector_size = 0x10000;
    unsigned char retry_cnt = 0;

    unsigned int start_tick = osal_get_tick(); 

#ifndef ENABLE_SERIAL_FLASH
    flash_info_pl_init();
#else
    flash_info_sl_init();
#endif

    memset(strbuf,0,sizeof(strbuf));
    memset(sec_flag,0,sizeof(sec_flag));
    
	if (flash_identify() == 0)
	{
		SH_PRINTF("Unknow Flash type, Halt.\n");
		SDBBP();
   	}

    binary_len = data_len;

	if (binary_len == 0 || binary_len > flash_size_val)
	{
		binary_len = flash_size_val;
	}

    p = (unsigned char *)data_addr;

    SH_PRINTF("flash_size=0x%x, binary_len=0x%x, p=0x%x\n", flash_size_val, binary_len, p);
	
    sector_no = 0;
    temp_start = 0;
   
    p = (unsigned char *)data_addr;
	SH_PRINTF("p=0x%x\n", p);

	i = 0;
    temp_start = 0;
    
    panel_show("bun ", 4);
    
    sector_no = flash_sector_align(temp_start);
    while (temp_start < binary_len)
    {
    	SH_PRINTF("flash %08X, data[0~3]:0x%x 0x%x 0x%x 0x%x\n", temp_start, p[0], p[1], p[2], p[3]);
        //len = sector_size;//flash_sector_size(i);
#if 1      
        if(0 == retry_cnt)
        {
            len = flash_sector_size(sector_no++);
        }
        else
        {
            len = 0x10000;
        }
#else
        len = 0x10000;
#endif
        sect_flag = flash_verify(temp_start, p, len);
        switch (sect_flag)
        {
        case 2:
			SH_PRINTF("flash erase %08X\n", temp_start);
            flash_erase_sector(temp_start);
        case 1:
			SH_PRINTF("flash copy %08X,len=0x08x\n", temp_start,len);
            ret = flash_copy(temp_start, p, len);
            if (ret == 0)
                ret = flash_verify(temp_start, p, len);
            break;
        default:
			SH_PRINTF("flash same %08X\n", temp_start);
            ret = 0;
        }
        if (ret == 0)
        {
            SH_PRINTF("%08X OK\n", temp_start);
         
            progress = temp_start*100/binary_len;
            if (progress < 100)
                sprintf(strbuf,"b%.2d",progress);
            else
                sprintf(strbuf,"100 ");
            panel_show( strbuf, 4);
              
        }
        else
        {
            SH_PRINTF("%08X Fail\n", temp_start);

            progress = temp_start*100/binary_len;

            if (progress < 100)
                sprintf(strbuf,"r%.2d",progress);
            else
                sprintf(strbuf,"100 ");
			
            panel_show(strbuf, 4);

            if(0 == retry_cnt)
            {
                retry_cnt++;
                continue;
            }
            else
            {
                return -1;
            } 
        }
        i++;
        p += len;
        temp_start += len;
    }
    sprintf(strbuf,"100 ");
    panel_show(strbuf, 4); 
    SH_PRINTF("%s(): consume time = %d \n",__FUNCTION__,osal_get_tick() - start_tick);  
	return 0;
}


static void fs_apcallback(UINT32 event_type,UINT32 param)
{
    UINT32 type = MNT_GET_TYPE(param);
    UINT32 sub_type = MNT_GET_SUB_TYPE(param);
    //storage_device_type dev_type = sto_type_mnt2dev(type);

    SH_PRINTF("%s : event_type = %d, type=%d,sub_type=%d\n",__FUNCTION__, event_type, type, sub_type);

    switch(event_type)
    {
    case MP_FS_MOUNT:
        if(sub_type == MNT_MOUNT_OK)
        {            
            SH_PRINTF("FS mount OK\n");
            usb_mounted = TRUE;         
        }
        break;
    case MP_FS_UNMOUNT:
        if((sub_type == UNMNT_UNMOUNT) || (sub_type == UNMNT_UNMOUNT_OK))
        {
            usb_mounted = FALSE;
            see_fw_state = 0;
            //SH_PRINTF("send ap unmount msg ...\n");
        }
        break;
    }
}

static void init_usb_fs(void)
{
    create_usb_monitor_task();
    fs_init(fs_apcallback); // fs_init should be after dm_init, or dm will be clear.
    usb_init();
}
#endif

int flash_hw_init(void)
{
    struct sto_flash_info flash_param;
    int ret = 0;
    // initialize flash --->
#ifdef ENABLE_SERIAL_FLASH    
    unsigned long sflash_reg_addr = 0xb8000000;
    if(ALI_M3329E==sys_ic_get_chip_id()&&sys_ic_get_rev_id()>=IC_REV_5)
        sflash_reg_addr |= 0x8000;
	
    *((volatile UINT32 *)(sflash_reg_addr+0x90)) = 0x231FA0FD;
	
    flash_param.flash_deviceid = NULL;
    flash_param.flash_id = NULL;
    flash_param.flash_deviceid_num = 0;
    
    flash_info_sl_init();
    if (SUCCESS != sto_local_sflash_attach(NULL))
    {
        return -1;
    }
#else
    flash_info_pl_init();
    if( SUCCESS != sto_local_flash_attach(NULL) )
    {
        return -1;
    }
#endif

    ret = flash_identify();
    if (ret == 0)
    {
        SH_PRINTF("Unknow Flash type, Halt.\n");
        //sys_reboot();
        return -1;
    }
    
#ifdef ENABLE_SERIAL_FLASH    
    *((volatile UINT32 *)(sflash_reg_addr+0x98)) &= ~0xc0000000;
    if (flash_info.flash_size <= 0x200000)
    {
        *((volatile UINT32 *)(sflash_reg_addr+0x98)) |= 0;
    }
    else if (flash_info.flash_size > 0x200000 && flash_info.flash_size <= 0x400000)
    {
        *((volatile UINT32 *)(sflash_reg_addr+0x98)) |= 0x40000000;
    }
    else if (flash_info.flash_size > 0x400000 && flash_info.flash_size <= 0x800000)
    {
        *((volatile UINT32 *)(sflash_reg_addr+0x98)) |= 0x80000000;
    }
    else
    {
        *((volatile UINT32 *)(sflash_reg_addr+0x98)) |= 0xc0000000;
    }
#endif

    return 0;
}

void tds_app_init()
{	
    unsigned int ret, flash_size;
    UINT32 param;
    UINT8 mode;

    mp_state = E_MP_MAIN_RUN; 

    uart_init();    

    //asm volatile(".word 0x1000FFFF;nop;");
#ifdef ENABLE_USB_INTF
    libc_printf("APP Main OK \n");
    SH_PRINTF("APP Main OK \n");

    init_usb_fs();
	libc_printf("USB init OK \n");
    SH_PRINTF("USB init OK \n");
	//otp_init();
#ifdef LOAD_SEE_FORM_USB
    while(!see_load_finished())
    {       
        osal_task_sleep(10);
    }    
#else
    load_see_finish();  
#endif    
#endif

    // slave cpu bootup
    see_boot(osal_dual_get_see_run_address());
    osal_task_sleep(100);
    hld_dev_see_init(__MM_SEE_DBG_MEM_ADDR); 
    
#ifdef ENABLE_USB_INTF
    SH_PRINTF("APP See OK \n");
#endif

    // 1. open UART; 2. Enable panel in debug mode
    system_hardware_init();
    ret = flash_hw_init();
    if(ret < 0)
    {
        panel_show("FE01",4);
        goto REBOOT;      
    }

    //otp_init();
    ce_api_attach();
    dsc_api_attach();

    panel_show("APOK",4);

    mp_state = E_MP_SEE_RUN;  
    otp_init(); 

#ifdef _BUILD_USB_LOADER_
    goto FINISH;
#endif
    
    
    //set_memory_byte_available();

    // get upgrade parameters --->
    param = get_upgrade_param(EUP_OFFSET(upgrade_mode));
    //param = get_upgrade_param(((UINT32)&(((PEROM_UPGRADE_PARAM)0)->upgrade_mode)));
    mode = (UINT8)param;
    g_bps = (UINT8)(param>>8);
    flash_burn_flag = (UINT8)(param>>16);
    bloader_enc_flag = (UINT8)(param>>24);
    // < ---- get upgrade parameters end 

    if(UPGRADE_MODE_ONE2ONE != mode && UPGRADE_MODE_MUL_NORMAL != mode 
        && UPGRADE_MODE_MUL_COMPRESS != mode )
    {
        panel_show("FE02",4);
        goto REBOOT;  
    }

    g_dwChipVersion = *(volatile unsigned long*)0xb8000000;
    SH_PRINTF("Chipset version: 0x%x\n", g_dwChipVersion);

 //   gen_crc32_table();
  //  mg_setup_crc_table();  

    flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
    if (flash_dev == NULL)
    {
        PRINTF("Can't find FLASH device!\n");
        return;
    }
    
    if ( SUCCESS != sto_open(flash_dev) )
    {
        return;
    }
	
    sto_chunk_init(0, flash_size_val);

    mp_state = E_MP_PREPARE_LOADER_FLASH_ABS;
    // < --- initialize flash end
	//burn_nor_boot_otp();

    panel_show("FSH",4);
    //return ;

    sci_16550uart_set_high_speed_mode(SCI_FOR_RS232, g_bps);
    
    if (is_eromupg_burn_flash())
    {
        if (UPGRADE_MODE_ONE2ONE == mode)
        {
            panel_show("P2P",4);
            // P2P mode
            //lib_ash_shell();
        }
        else
        {
            panel_show("P2M",4);
            // P2M mode
            upgrade_all_3505(mode);
        }

        mp_state = E_MP_FINISH;
    }
    

FINISH:
    while(mp_state != E_MP_FINISH)
    {
        osal_task_sleep(100);
    }

    panel_show("FSH",4);
    
REBOOT:    
    dog_m3327e_attach(1);
    /**
    *  before enter watchdog_reboot,you MUST NOT read/write flash
    *  as soon as possible.make sure the bus is clear.
    */
    osal_interrupt_disable();
    *((volatile UINT32 *)0xb802e098)=0xc2000d03;
    int tmp = *((volatile UINT32 *)0xb802e098);

    //dog_mode_set(0, DOG_MODE_WATCHDOG, 2000, NULL);
    hw_watchdog_reboot();
    while(1);
   
    return;	
}

//by keven
#define CFG_LINE_MAX_LENGTH 1024
#define MAX_FILE_NAME     1024

char cur_line[CFG_LINE_MAX_LENGTH];
char mpconf_path[MAX_FILE_NAME];

static char* mnt_name = "/mnt/uda1/";

typedef struct MP_CONFIG{
    int mp_bl_enc;
}MP_CONFIG_st;

MP_CONFIG_st mp_conf;

void parse_parameter(FILE* fconf)
{
    while(fgets(cur_line, CFG_LINE_MAX_LENGTH, fconf) != NULL)
    { 
        if((strncmp(cur_line,"#", 1) == 0)||(strncmp(cur_line,"//", 2) == 0))
	        continue;
        if(strncmp(cur_line,"[", 1) == 0)
            break;	
        if(strncmp(cur_line,"otp_burn_encrypt_bl=", 20) == 0)
        {
            SH_PRINTF("[PARAMETER] %s\n",cur_line);
            sscanf(cur_line, "otp_burn_encrypt_bl=%d\n", &otp_burn_encrypt_bl);
        }

        if(strncmp(cur_line,"otp_burn_cstm_id=", 17) == 0)
        {
            SH_PRINTF("[PARAMETER] %s\n",cur_line);
            sscanf(cur_line, "otp_burn_cstm_id=%d\n", &otp_burn_cstm_id);
        }

        if(strncmp(cur_line,"otp_burn_boot_rom_sig=", 22) == 0)
        {
            SH_PRINTF("[PARAMETER] %s\n",cur_line);
            sscanf(cur_line, "otp_burn_boot_rom_sig=%d\n", &otp_burn_boot_rom_sig);
        }
        //add ret2gac
		if(strncmp(cur_line,"burn_ret2gca_otp=", 17) == 0)
        {
            SH_PRINTF("[PARAMETER] %s\n",cur_line);
            sscanf(cur_line, "burn_ret2gca_otp=%d\n", &burn_ret2gca_otp);
        }
    }
}


void load_mpclient_config()
{
    char *conf_name = "mpclient.conf";

    mpconf_path[0] = 0;
    strcpy(mpconf_path,mnt_name);
    strcat(mpconf_path,conf_name);
    
    FILE* f_conf = fopen(mpconf_path,"r");
    if(NULL == f_conf)
    {
        SH_PRINTF("cannot find %s\n",mpconf_path);
        SH_PRINTF("Use default config for mp\n");
        return -1;        
    }

    MEMSET(&mp_conf,0,sizeof(MP_CONFIG_st));
    
    while(1)
    {
        if(fgets(cur_line, CFG_LINE_MAX_LENGTH, f_conf) == NULL)
            break;
        //printf("%s\n",cur_line);

        if((strncmp(cur_line,"#", 1) == 0)||(strncmp(cur_line,"//", 2) == 0))
        {
            continue;
        }

        if(strncmp(cur_line,"[PARAMETER]", 11) == 0)
        {
        	if(NULL == f_conf)
			{
				break;
			}
            SH_PRINTF("Parse parameter ... \n");
            parse_parameter(f_conf);
        }
    }
    
    fclose(f_conf);

    SH_PRINTF("Load Conf Finish \n");
    SH_PRINTF("otp_burn_encrypt_bl = %d\n",otp_burn_encrypt_bl);
    SH_PRINTF("otp_burn_boot_rom_sig = %d\n",otp_burn_boot_rom_sig);
    SH_PRINTF("otp_burn_cstm_id = %d\n",otp_burn_cstm_id);
	SH_PRINTF("burn_ret2gca_otp = %d\n",burn_ret2gca_otp); //
}

INT32 sdec_request_write(void *pdev, UINT32 u_size_requested,void **ppu_data,UINT32 *pu_size_got,
    struct control_block *ps_data_cb)
{
    return 0;
}
void sdec_update_write(void *pdev,UINT32 u_data_size)
{
;
}
RET_CODE dmx_get_see_data(struct dmx_device *dev, UINT32 uparam)
{
	return 0;
}
void lld_vsc_callee(UINT8 *msg)
{
	 ;
}




