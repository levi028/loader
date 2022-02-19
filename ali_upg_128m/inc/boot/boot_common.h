#ifndef  _BOOT_COMMON_H_
#define  _BOOT_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#define ALI_SOC_BASE            0xb8000000
#define NEW_HEAD_ID             0xaead0000
#define NEW_HEAD_MASK           0xffff0000
#define SYS_UK_OFFSET_WITH_NEW_HEAD 0x80
#define UK_OFFSET_WITH_NEW_HEAD 0x1000
#define BL_UK_TYPE_MASK         0xFF00
#define BL_CODE_OFFSET          0x1800
#define BL_CODE_OFFSET_2        0x3800     

//#define  UK_OFFSET_WITH_NEW_HEAD_N3 0x3800

#ifdef _M3702_
#define  UK_OFFSET_WITH_NEW_HEAD_N3 0x3800 /**<Unchecked area offset base on the flash base address.VER_N_3.0 */
#elif defined(_M3711C_)
#define  UK_OFFSET_WITH_NEW_HEAD_N3 0x8000 /**<Unchecked area offset base on the flash base address.VER_N_3.0 */
#else
#define  UK_OFFSET_WITH_NEW_HEAD_N3 0x1800 /**<Unchecked area offset base on the flash base address.VER_N_3.0 */
#endif


#define  GET_UK_OFFSET(offset)	\
do{ \
	if(sys_ic_get_chip_id() < ALI_CAP210){	\
		offset = UK_OFFSET_WITH_NEW_HEAD ; \
	}else{	\
		offset = UK_OFFSET_WITH_NEW_HEAD_N3 ; \
	}	\
}while(0)



#define RSA_PUBLIC_KEY1_ADDR    0xB8042200
#define RSA_PUBLIC_KEY2_ADDR    0xB8042324

/*shadow register to indicate the otp status */
#define OTP_IOBASE_M32 0xb8000100
#define OTP_03_SHADOW_M32 0x28
#define OTP_IOBASE_M37 0xb8042000
#define OTP_03_SHADOW_M37 0x44
#define OTP_DC_SHADOW_M37 0x78
#define C3503B_REV 2
#define KL_0_INDEX 0
#define KL_2_INDEX 2
#define KL_3_INDEX 3
#define KL_CTRL_VALUE_0 0
#define KL_CTRL_VALUE_1 1
#define KL_CTRL_VALUE_2 2
#define KL_CTRL_VALUE_3 3

#define GET_DWORD(i)            (*(volatile UINT32 *)(i))
#define SET_DWORD(i, d)         (*(volatile UINT32 *)(i)) = (d)
#define ARRAY_SIZE(_array)      (sizeof(_array)/sizeof(_array[0]))

enum BOOT_TYPE
{
	BOOT_TYPE_NOR = 0,  /**<Boot from Nor flash. */
    BOOT_TYPE_NAND = 1  /**<Boot from Nand flash. */
};


void sys_uart_init(void);
void sys_gpio_init(void);
void sys_panel_init(void);

#ifdef _MPLOADER_ENABLE_
RET_CODE mp_app(void);
#endif

#ifdef _BOOT_LOGO_
RET_CODE bl_show_logo(UINT8 *logo_buf, UINT32 logo_len);
#endif

/******************************************************************************/
/*                                                                            */
/*                             MACROS DEFINITION                              */
/*                                                                            */
/******************************************************************************/
#define ALI_SOC_BASE            0xb8000000

#define STAGE_TRANSFER_ADDR0    0xA0000018
#define STAGE_TRANSFER_ADDR1    0xA000001C
#define STAGE_TRANSFER_ADDR2    0xA0000010
#define STAGE_TRANSFER_ADDR3    0xA0000014
#define ALI_TRANSFER_MAGIC_NUM  0xABCD5AA5
#define TRANS_SYSINFO_MAX_SIZE  0x1000

#define GET_DWORD(i)            (*(volatile UINT32 *)(i))
#define SET_DWORD(i, d)         (*(volatile UINT32 *)(i)) = (d)

/******************************************************************************/
/*                                                                            */
/*                             TYPES DEFINITIONS                              */
/*                                                                            */
/******************************************************************************/
enum TRANS_SYSINFO_TYPE
{
    TRANS_VDEC_TYPE = 0,
    TRANS_VPO_TYPE,
};

struct TRANS_VDEC_INFO
{
	UINT16 pic_width;
	UINT16 pic_height;
	UINT32 aspect_ratio;
    UINT32 resv[4];
};

/* Modify by chen 2014-12-07, Add the get vpo info support  */
struct TRANS_VPO_INFO
{
    UINT8 tv_mode;
    UINT8 scart_out;
    UINT8 tv_ratio;
    UINT8 display_mode;
    UINT32 resv[1];
};

struct TRANS_SYSINFO
{
    struct TRANS_VDEC_INFO vdec_info;
    struct TRANS_VPO_INFO vpo_info;  
};

#ifdef __cplusplus
 }
#endif

#endif  /*_BOOT_COMMON_H_*/

