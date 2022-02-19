/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_security_upg.c
*
*    Description: The realize of security upgrade
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>

#ifdef SECURITY_MP_UPG_SUPPORT

#define NOT_BURN_OPT_CONFIG_TEST    //for test, don't burn opt config

#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <hld/pan/pan_dev.h>
#include <hld/pan/pan.h>

#include <api/libosd/osd_lib.h>
#include <api/libchunk/chunk.h>
#include <api/libupg/p2p_upgrade.h>

#include "osdobjs_def.h"

#include "string.id"
#include "images.id"
#include "osd_config.h"
#include "osd_rsc.h"

#include "win_com.h"

#include "disk_manager.h"
#include <api/libmp/pe.h>
#include <api/libfs2/fs_main.h>
#include <api/libupg/usb_upgrade.h>

#include <api/libc/string.h>


#include <bus/otp/otp.h>

#include <api/libc/fast_crc.h>
#include <api/libfs2/stat.h>
#include <api/libfs2/dirent.h>
#include <api/libfs2/statvfs.h>
#include <api/libfs2/unistd.h>
#include <api/libfs2/stdio.h>


#define EJTAG_KEY_SIZE 8 // 4 byte
#define PVR_KEY_SIZE 16//byte
#define HDCP_PROTECT_KEY_SIZE 16//byte
#define ROOT_PUBLIC_KEY_SIZE 292//byte
#define HDCP_KEY_SIZE 288//286//byte
#define CIPLUS_KEY_SIZE 0x10600//19824//2345//byte
#define ENCRYPT_PACK_SIZE \
    (EJTAG_KEY_SIZE+PVR_KEY_SIZE+HDCP_PROTECT_KEY_SIZE+ROOT_PUBLIC_KEY_SIZE+HDCP_KEY_SIZE+CIPLUS_KEY_SIZE) //byte

#define ENCRYPT_PACK_ADDR 0xA1FB9400//the encrypted key package addr //__MM_AUTOSCAN_DB_BUFFER_ADDR=0xa1fb9400
#define DECRYPT_PACK_ADDR (ENCRYPT_PACK_ADDR+ENCRYPT_PACK_SIZE)//the package after decrypt

#define EJTAG_KEY_ADDR  DECRYPT_PACK_ADDR
#define PVR_KEY_ADDR    (EJTAG_KEY_ADDR+EJTAG_KEY_SIZE)
#define HDCP_PROTECT_KEY_ADDR   (PVR_KEY_ADDR+PVR_KEY_SIZE)
#define ROOT_PUBLIC_KEY_ADDR    (HDCP_PROTECT_KEY_ADDR+HDCP_PROTECT_KEY_SIZE)
#define HDCP_KEY_ADDR   (ROOT_PUBLIC_KEY_ADDR+ROOT_PUBLIC_KEY_SIZE)
#define CIPLUS_KEY_ADDR (HDCP_KEY_ADDR+HDCP_KEY_SIZE)

#define EJTAG_OTP_ADDR   0x5D
#define PVR_OTP_ADDR    0x51
#define HDCP_OTP_ADDR   0x59
#define ROOT_OTP_ADDR   0x04


#define SECTOR_SIZE (64*1024)
#define CHUNK_HEADER_SIZE  128
#define CHUNK_SUBHEADER_SIZE 0x70

#define BLOADER_CHUNK_ID    0x10000123//0x00
#define HDCPKEY_CHUNK_ID    0x09F60100
#define MAINCODE_CHUNK_ID   0x01FE0101
#define SEECODE_CHUNK_ID    0x06F90101
#define CIPLUSKEY_CHUNK_ID  0x09F60101

#define SECURITY_LOG_FILE_NAME  "Security_log.log"

#define UPG_MAX_FILENUM_SUPPORT 1000

#define FILENAME_COMPARE_LENGTH 14

#define FREE_P_SAFELY(p) do{\
                        if(NULL!=p)\
                            FREE(p);\
                     } while(0)

char pwd[] = {"0rstu0vwx88yz1df06789ab00cde2345fgh10ijklm00no60p47rstuv13wxyz"};

static char usb_upg_msg[256];

typedef struct
{
    char upg_file_name[128];
    USBUPG_BLOCK_TYPE upg_file_type;
}UPG_FILENODE,*PUPG_FILENODE;

static UPG_FILENODE *upg_filelist=NULL;
static UINT16 upg_node_num=0;

void decrypt_pack_data(char* input_data, char* output_data, int byte_size, char* password)
{
    register char ch;
    int j=0;
    int j0=0;
    int i=0;

    while(password[++j0]);
    ch=input_data[0];

    /*加密算法开始*/
    for(i=0;i<byte_size;i++)
    {
        ch=input_data[i];
        output_data[i]=ch^password[j>=j0?j=0:j++];
    }
}

static BOOL all_data_is_0xff(UINT8 *addr, UINT32 len)
{
    UINT32 i;
    for (i = 0; i < len; i++)
    {
        if (addr[i] != 0xff)
            return FALSE;
    }
    return TRUE;
}

static BOOL all_data_is_zero(UINT8 *addr, UINT32 len)
{
    UINT32 i;
    for (i = 0; i < len; i++)
    {
        if (addr[i] != 0x00)
            return FALSE;
    }
    return TRUE;
}

INT32 otp_write_mp(UINT8 *buf, UINT32 offset, INT32 len)
{
    if(OTP_WRITE_ERROR == otp_write(buf, offset, len))
    {
        api_otp_rw_check_callback();
    }
}
static INT32 data_change_for_sto(INT32 data)
{
    /*
     unsigned char buf[4];
    char *buf = &data;
    return (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
    */
    INT32 tmp=0;
    tmp |= (data&0x000000ff)<<24;
    tmp |= (data&0x0000ff00)<<8;
    tmp |= (data&0x00ff0000)>>8;
    tmp |= (data&0xff000000)>>24;

    return tmp;
}

BOOL keyupg_burnflash(UINT32 chunk_id, char *buffer_adr,UINT32 buffer_size)
{
    UINT32 chunk_len,chunk_addr;
    UINT32 param;
    struct sto_device *f_dev;
    UINT32 sector_count=0;


    //api_get_chuck_addlen(chunk_id, &chunk_addr, &chunk_len);//chunk_addr is the real data addr not include header
    chunk_addr = (UINT32)sto_chunk_goto(&chunk_id,0xFFFFFFFF,1) + CHUNK_HEADER_SIZE;

    if ((f_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0)) == NULL)
    {
        libc_printf("ERROR: Not found Flash device!\n");
        return FALSE;
    }

    if(chunk_id==HDCPKEY_CHUNK_ID)
    {
        //hdcp key,need rewrite bootloader
        UINT8   *bootloader_sector = NULL;
        CHUNK_HEADER bloader_chuck_hdr;
  
        sto_get_chunk_header(BLOADER_CHUNK_ID,&bloader_chuck_hdr);
        bootloader_sector = (UINT8 *)MALLOC(sizeof(UINT8)*(bloader_chuck_hdr.offset));
        if(NULL == bootloader_sector)
        {
            libc_printf("ERROR: MALLOC Failed\n");
            return FALSE; 
        }

        sto_lseek(f_dev, 0x00, STO_LSEEK_SET);
        sto_read(f_dev, bootloader_sector, bloader_chuck_hdr.offset);

        /*
        //param = 0x00;
        //param = param << 10;
        //param +=( (SECTOR_SIZE*(bloader_chuck_hdr.offset/SECTOR_SIZE+1)) >> 10);
        //sto_io_control(f_dev, STO_DRIVER_SECTOR_ERASE, param);
        UINT32 tmp_param[2];
        tmp_param[0] = 0x00;
        tmp_param[1] = (SECTOR_SIZE*(bloader_chuck_hdr.offset/SECTOR_SIZE+1))>>10 ;
        param= (UINT32)tmp_param;
        sto_io_control(f_dev, STO_DRIVER_SECTOR_ERASE_EXT, param);//Summic add for check the >=4M Flash
        */
        for(sector_count=0;sector_count< ((bloader_chuck_hdr.offset-1)/SECTOR_SIZE+1);sector_count++)
        {
            //param = 0x00+sector_count*SECTOR_SIZE;
            //param = param << 10;  /* Lower 10 bits of LSB is length in K bytes*/
            //param += SECTOR_SIZE >> 10;
            //sto_io_control(f_dev, STO_DRIVER_SECTOR_ERASE, param);
            UINT32 tmp_param[2];
            tmp_param[0] = 0x00+sector_count*SECTOR_SIZE;;
            tmp_param[1] = SECTOR_SIZE>>10 ;
            param= (UINT32)tmp_param;
            sto_io_control(f_dev, STO_DRIVER_SECTOR_ERASE_EXT, param);//Summic add for check the >=4M Flash
        }

        sto_lseek(f_dev, 0x00, STO_LSEEK_SET);
        sto_write(f_dev, bootloader_sector, bloader_chuck_hdr.offset);

        CHUNK_HEADER hdcp_chuck_hdr;
        //sto_get_chunk_header(chunk_id,&hdcp_chuck_hdr);
        hdcp_chuck_hdr.id = data_change_for_sto(chunk_id);
        hdcp_chuck_hdr.len = data_change_for_sto(buffer_size);
           hdcp_chuck_hdr.offset = data_change_for_sto(0x200);
            hdcp_chuck_hdr.crc = data_change_for_sto(0x4e435243);
        strncpy(hdcp_chuck_hdr.name,"HDCPKey", 15);
        strncpy(hdcp_chuck_hdr.version,"Encrypted", 15);
        strncpy(hdcp_chuck_hdr.time,"2010.09.01", 15);
        sto_write(f_dev,&hdcp_chuck_hdr,sizeof(CHUNK_HEADER));
        sto_write(f_dev,buffer_adr, buffer_size);

        FREE(bootloader_sector);
    }
    else if(chunk_id==CIPLUSKEY_CHUNK_ID)
    {
        //ciplus key,no need to rewrite other data
        CHUNK_HEADER ciplus_chuck_hdr;
        sto_get_chunk_header(CIPLUSKEY_CHUNK_ID,&ciplus_chuck_hdr);
        for(sector_count=0;sector_count< ((ciplus_chuck_hdr.offset-1)/SECTOR_SIZE+1);sector_count++)
        {
            //param = chunk_addr-CHUNK_HEADER_SIZE+sector_count*SECTOR_SIZE;
            //param = param << 10;  /* Lower 10 bits of LSB is length in K bytes*/
            //param += SECTOR_SIZE >> 10;
            //sto_io_control(f_dev, STO_DRIVER_SECTOR_ERASE, param);
            UINT32 tmp_param[2];
            tmp_param[0] =chunk_addr-CHUNK_HEADER_SIZE+sector_count*SECTOR_SIZE;
            tmp_param[1] =SECTOR_SIZE>>10  ;
            param= (UINT32)tmp_param;
            sto_io_control(f_dev, STO_DRIVER_SECTOR_ERASE_EXT, param);//Summic add for check the >=4M Flash
    }
        ciplus_chuck_hdr.len = data_change_for_sto(buffer_size);
        sto_lseek(f_dev, chunk_addr-CHUNK_HEADER_SIZE, STO_LSEEK_SET);
        sto_write(f_dev,&ciplus_chuck_hdr,sizeof(CHUNK_HEADER));
        sto_write(f_dev,buffer_adr, buffer_size);
    }
    else if(chunk_id==BLOADER_CHUNK_ID)
    {
        //bloader, need to rewrite hdcp key
        UINT8   *hdcp_sector = NULL;
        CHUNK_HEADER bloader_chuck_hdr;

        sto_get_chunk_header(BLOADER_CHUNK_ID,&bloader_chuck_hdr);
        hdcp_sector = (UINT8 *)MALLOC(sizeof(UINT8)*(HDCP_KEY_SIZE+CHUNK_HEADER_SIZE));
        if(NULL == hdcp_sector)
        {
            libc_printf("ERROR: MALLOC Failed\n");
            return FALSE; 
        }
        
        sto_lseek(f_dev, bloader_chuck_hdr.offset, STO_LSEEK_SET);
        sto_read(f_dev, hdcp_sector, HDCP_KEY_SIZE+CHUNK_HEADER_SIZE);

        /*
        //param = 0x00;
        //param = param << 10;
        //param += ((SECTOR_SIZE*(bloader_chuck_hdr.offset/SECTOR_SIZE+1)) >> 10);
        //sto_io_control(f_dev, STO_DRIVER_SECTOR_ERASE, param);
            UINT32 tmp_param[2];
        tmp_param[0] = 0x00;;
        tmp_param[1] = (SECTOR_SIZE*(bloader_chuck_hdr.offset/SECTOR_SIZE+1))>>10;
        param= (UINT32)tmp_param;
        sto_io_control(f_dev, STO_DRIVER_SECTOR_ERASE_EXT, param);//Summic add for check the >=4M Flash
        */
        for(sector_count=0;sector_count< ((bloader_chuck_hdr.offset-1)/SECTOR_SIZE+1);sector_count++)
        {
        //  param = 0x00+sector_count*SECTOR_SIZE;
        //  param = param << 10;    /* Lower 10 bits of LSB is length in K bytes*/
        //  param += SECTOR_SIZE >> 10;
            //sto_io_control(f_dev, STO_DRIVER_SECTOR_ERASE, param);
            UINT32 tmp_param[2];
            tmp_param[0] = 0x00+sector_count*SECTOR_SIZE;;
            tmp_param[1] = SECTOR_SIZE>>10 ;
            param= (UINT32)tmp_param;
            sto_io_control(f_dev, STO_DRIVER_SECTOR_ERASE_EXT, param);//Summic add for check the >=4M Flash
        }


        sto_lseek(f_dev, bloader_chuck_hdr.offset, STO_LSEEK_SET);
        sto_write(f_dev, hdcp_sector, HDCP_KEY_SIZE+CHUNK_HEADER_SIZE);

        sto_lseek(f_dev,chunk_addr-CHUNK_HEADER_SIZE,STO_LSEEK_SET) ;
        sto_write(f_dev,buffer_adr, buffer_size);

        FREE(hdcp_sector);
    }
    else if(chunk_id==MAINCODE_CHUNK_ID)
    {
        //maincode
        CHUNK_HEADER maincode_chuck_hdr;

        sto_get_chunk_header(MAINCODE_CHUNK_ID,&maincode_chuck_hdr);
/*
        //param = chunk_addr-CHUNK_HEADER_SIZE;;
        //param = param << 10;
        //param += ((SECTOR_SIZE*(maincode_chuck_hdr.offset/(SECTOR_SIZE+1)+1)) >> 10);
        //sto_io_control(f_dev, STO_DRIVER_SECTOR_ERASE, param);
        UINT32 tmp_param[2];
        tmp_param[0] = chunk_addr-CHUNK_HEADER_SIZE;;
        tmp_param[1] = (SECTOR_SIZE*(maincode_chuck_hdr.offset/(SECTOR_SIZE+1)+1))>>10;
        param= (UINT32)tmp_param;
        sto_io_control(f_dev, STO_DRIVER_SECTOR_ERASE_EXT, param);//Summic add for check the >=4M Flash
*/
        for(sector_count=0;sector_count< ((maincode_chuck_hdr.offset-1)/SECTOR_SIZE+1);sector_count++)
        {
            //param = chunk_addr-CHUNK_HEADER_SIZE+sector_count*SECTOR_SIZE;
            //param = param << 10;  /* Lower 10 bits of LSB is length in K bytes*/
            //param += SECTOR_SIZE >> 10;
            //sto_io_control(f_dev, STO_DRIVER_SECTOR_ERASE, param);
            UINT32 tmp_param[2];
            tmp_param[0] = chunk_addr-CHUNK_HEADER_SIZE+sector_count*SECTOR_SIZE;;
            tmp_param[1] = SECTOR_SIZE>>10 ;
            param= (UINT32)tmp_param;
            sto_io_control(f_dev, STO_DRIVER_SECTOR_ERASE_EXT, param);//Summic add for check the >=4M Flash
        }

        sto_lseek(f_dev,chunk_addr-CHUNK_HEADER_SIZE,STO_LSEEK_SET) ;
        sto_write(f_dev,buffer_adr, buffer_size);
    }
    else if(chunk_id==SEECODE_CHUNK_ID)
    {
        //seecode
        CHUNK_HEADER seecode_chuck_hdr;

        sto_get_chunk_header(SEECODE_CHUNK_ID,&seecode_chuck_hdr);
/*
        //param = chunk_addr-CHUNK_HEADER_SIZE;;
    //  param = param << 10;
        //param += ((SECTOR_SIZE*(maincode_chuck_hdr.offset/(SECTOR_SIZE+1)+1)) >> 10);
        //sto_io_control(f_dev, STO_DRIVER_SECTOR_ERASE, param);
        UINT32 tmp_param[2];
        tmp_param[0] = chunk_addr-CHUNK_HEADER_SIZE;;
        tmp_param[1] = (SECTOR_SIZE*(maincode_chuck_hdr.offset/(SECTOR_SIZE+1)+1))>>10;
        param= (UINT32)tmp_param;
            sto_io_control(f_dev, STO_DRIVER_SECTOR_ERASE_EXT, param);  //Summic add for check the >=4M Flash
*/
        for(sector_count=0;sector_count< ((seecode_chuck_hdr.offset-1)/SECTOR_SIZE+1);sector_count++)
        {
            //param = chunk_addr-CHUNK_HEADER_SIZE+sector_count*SECTOR_SIZE;
            //param = param << 10;  /* Lower 10 bits of LSB is length in K bytes*/
            //param += SECTOR_SIZE >> 10;
            //sto_io_control(f_dev, STO_DRIVER_SECTOR_ERASE, param);
            UINT32 tmp_param[2];
            tmp_param[0] =chunk_addr-CHUNK_HEADER_SIZE+sector_count*SECTOR_SIZE;
            tmp_param[1] = SECTOR_SIZE>>10 ;
            param= (UINT32)tmp_param;
            sto_io_control(f_dev, STO_DRIVER_SECTOR_ERASE_EXT, param);//Summic add for check the >=4M Flash
        }

        sto_lseek(f_dev,chunk_addr-CHUNK_HEADER_SIZE,STO_LSEEK_SET) ;
        sto_write(f_dev,buffer_adr, buffer_size);
    }

    return TRUE;

}

//change the order of data ,ey:0x01,0x02,0x03,0x04, 0x05,0x06,0x07,0x08 <-->0x08,0x07,0x06,0x05, 0x04,0x03,0x02,0x01
int data_inverse(UINT32 src_data_addr_offset, UINT32* dest_buff, UINT32 len)
{
    int ret = 0;

    UINT8 *malloc_src_buff;
    UINT8 *malloc_dest_buff;
    malloc_src_buff = (UINT8*)MALLOC(sizeof(UINT8)*len);
    malloc_dest_buff = (UINT8*)MALLOC(sizeof(UINT8)*len);
    if((NULL == malloc_src_buff) || (NULL == malloc_dest_buff))
    {
        FREE_P_SAFELY(malloc_src_buff);
        FREE_P_SAFELY(malloc_dest_buff);
        return -1;
    }
    

    MEMCPY(malloc_src_buff, src_data_addr_offset, len);
    int i=0;
#if 0 //debug
    for(i=0;i<len;i++)
    {
        soc_printf("malloc_buff[%d]=%d\n",i,malloc_src_buff[i]);
    }
#endif
    for(i=0;i<(len);i++)
    {
        malloc_dest_buff[i] = malloc_src_buff[len-1-i];
    }
    MEMCPY(dest_buff, malloc_dest_buff, len);

    FREE(malloc_dest_buff);
    FREE(malloc_src_buff);

    return ret;
}

BOOL keyupg_burnotp(UINT32* buf, UINT32 offset, INT32 len, UINT32 src_data_addr_offset, BOOL data_need_inverse)
{
    BOOL ret = FALSE;
    int oret = 0;
    UINT32 *malloc_buff;

    otp_init(NULL);

    MEMSET(buf, 0, len);
    otp_read(offset*4, buf, len) ;
    if(all_data_is_zero(buf, len))
    {
        if(data_need_inverse)
        {
            malloc_buff = (UINT32*)MALLOC(sizeof(UINT32)*len);
            if(NULL == malloc_buff)
            {
                return FALSE;
            }
            oret = data_inverse(src_data_addr_offset, malloc_buff, len);
            if(oret < 0)
            {
                FREE(malloc_buff);
                return FALSE;
            }
            otp_write_mp(malloc_buff, offset*4, len);
            FREE(malloc_buff);
        }
        else
        {
            otp_write_mp(src_data_addr_offset, offset*4, len);
        }

        ret = TRUE;
    }
    //test the data
    MEMSET(buf, 0, len);
    otp_read(offset*4, buf, len) ;

    //judge whether burnotp successfully
    if(data_need_inverse)
    {
        malloc_buff = (UINT32*)MALLOC(sizeof(UINT32)*len);
        if(NULL == malloc_buff)
        {
            return FALSE;
        }
        oret = data_inverse(src_data_addr_offset, malloc_buff, len);
        if(oret < 0)
        {
            FREE(malloc_buff);
            return FALSE;
        }
        MEMCPY(src_data_addr_offset,malloc_buff,len);
        FREE(malloc_buff);
    }
    /*
    int i;
    for(i=0;i<len;i++)
    {
        if( *(buf+i)!=*(src_data_addr_offset+i))
            break;
    }
    if(i != len)
    {
        ret = FALSE;//upgrade fail
    }
    else
    {
        ret = TRUE;//data success(upg success, or data is the same as before)
    }
    */
    if(MEMCMP(buf, src_data_addr_offset, len)!=0)
    {
        ret = FALSE;//upgrade fail
    }
    else
    {
        ret = TRUE;//data success(upg success, or data is the same as before)
    }


    return ret;

}

//not judge for all 0x00
BOOL keyupg_burnotp_2(UINT32* buf, UINT32 offset, INT32 len, UINT32 src_data_addr_offset, BOOL data_need_inverse)
{
    BOOL ret = FALSE;
    int oret = 0;
    UINT32 *malloc_buff;

    otp_init(NULL);

    MEMSET(buf, 0, len);
    otp_read(offset*4, buf, len) ;
    //if(all_data_is_zero(buf, len))
    {
        if(data_need_inverse)
        {
            malloc_buff = (UINT32*)MALLOC(sizeof(UINT32)*len);
            if(NULL == malloc_buff)
            {
                return FALSE;
            }
            oret = data_inverse(src_data_addr_offset, malloc_buff, len);
            if(oret < 0)
            {
                FREE(malloc_buff);
                return FALSE;
            }
            otp_write_mp(malloc_buff, offset*4, len);
            FREE(malloc_buff);
        }
        else
        {
            otp_write_mp(src_data_addr_offset, offset*4, len);
        }

        ret = TRUE;
    }
    //test the data
    MEMSET(buf, 0, len);
    otp_read(offset*4, buf, len) ;

    //judge whether burnotp successfully
    if(data_need_inverse)
    {
        malloc_buff = (UINT32*)MALLOC(sizeof(UINT32)*len);
        if(NULL == malloc_buff)
        {
            return FALSE;
        }
        oret = data_inverse(src_data_addr_offset, malloc_buff, len);
        if(oret < 0)
        {
            FREE(malloc_buff);
            return FALSE;
        }
        MEMCPY(src_data_addr_offset,malloc_buff,len);
        FREE(malloc_buff);
    }
    /*
    int i;
    for(i=0;i<len;i++)
    {
        if( *(buf+i)!=*(src_data_addr_offset+i))
            break;
    }
    if(i != len)
    {
        ret = FALSE;//upgrade fail
    }
    else
    {
        ret = TRUE;//data success(upg success, or data is the same as before)
    }*/
    if(MEMCMP(buf, src_data_addr_offset, len)!=0)
    {
        ret = FALSE;//upgrade fail
    }
    else
    {
        ret = TRUE;//data success(upg success, or data is the same as before)
    }

    return ret;

}

static BOOL keyupg_burnotp_config(void)
{
    BOOL ret = FALSE;
    UINT32 malloc_otp_buff[16];
    UINT32 buf_0x03=0;
    UINT32 buf_0x5f=0;

    otp_init(NULL);

//////////bootloader sign & ejtag protect///////////////
    //config 0x3
    UINT32 is_enable_bootrom = 0x01<<1;//enable
    UINT32 is_close_ejtag = 0x00<<11;//not close
    UINT32 is_debug_protect = 0x01<<0;//enable debug protect
    UINT32 is_seebootrom_enable = 0x01<<8;
    UINT32 is_allow_cpu_read_1 = 0x01<<4;//not allow cpu read otp
    otp_read(0x03*4, &buf_0x03, 4) ;
    buf_0x03|=(is_enable_bootrom|is_close_ejtag|is_debug_protect|is_seebootrom_enable|is_allow_cpu_read_1);
    buf_0x03&=0xfffff7ff;
    ret = keyupg_burnotp_2(malloc_otp_buff, 0x03, 4, &buf_0x03, FALSE);

    //config 0x5f
    UINT32 ejtag_otp = 0x01<<2;
    UINT32 is_allow_cpu_read_2 = 0x01<<30;
    UINT32 is_allow_cpu_read_3 = 0x01<<0;
    otp_read(0x5f*4, &buf_0x5f, 4) ;
    buf_0x5f|=(ejtag_otp|is_allow_cpu_read_2|is_allow_cpu_read_3);
    ret = keyupg_burnotp_2(malloc_otp_buff, 0x5f, 4, &buf_0x5f, FALSE);


    return ret;

}

BOOL check_usb_is_active()
{
    int vol_num, tot_vol_num = 0;
    int dm_ret;
    BOOL ret = TRUE;

    dm_ret = dm_ioctl(DM_CMD1_GET_VOL_NUM, DM_CMD_PARAM(MNT_TYPE_USB, 0, 0), &vol_num, sizeof(int));
    if ((dm_ret == 0) && (vol_num > 0))
        tot_vol_num += vol_num;

    dm_ret = dm_ioctl(DM_CMD1_GET_VOL_NUM, DM_CMD_PARAM(MNT_TYPE_SD, 0, 0), &vol_num, sizeof(int));
    if ((dm_ret == 0) && (vol_num > 0))
        tot_vol_num += vol_num;

    dm_ret = dm_ioctl(DM_CMD1_GET_VOL_NUM, DM_CMD_PARAM(MNT_TYPE_IDE, 0, 0), &vol_num, sizeof(int));
    if ((dm_ret == 0) && (vol_num > 0))
        tot_vol_num += vol_num;

    if (tot_vol_num <= 0)
    {
        ret = FALSE;
    }

#ifdef DVR_PVR_SUPPORT
    if (api_pvr_is_record_active())
    {
        ret = FALSE;
    }
#endif

    return ret;
}

static RET_CODE get_usb_file_len(unsigned char* upg_file_name,INT32* buffer_len)
{
    char dir_str[USBUPG_FILE_NAME_LENGTH];
    FILE *fp_handle;
    INT32 file_length=0;

    snprintf(dir_str,USBUPG_FILE_NAME_LENGTH,"/c/%s",upg_file_name);
    fp_handle = fopen(dir_str,"rb");
    if(fp_handle == NULL)
        return RET_FAILURE;

    fseek(fp_handle,0,SEEK_END);
    file_length = (unsigned long)ftell(fp_handle);
    *buffer_len = file_length;

    fclose(fp_handle);

    return RET_SUCCESS;
}

static RET_CODE usb_read_file(unsigned char* upg_file_name, void *file_buffer,INT32* buffer_size)
{
    char dir_str[USBUPG_FILE_NAME_LENGTH];
    FILE *fp_handle;
    INT32 file_length=0;

    if(file_buffer == NULL)
    {
        return RET_FAILURE;
    }

    snprintf(dir_str, USBUPG_FILE_NAME_LENGTH,"/c/%s",upg_file_name);
    fp_handle = fopen(dir_str,"rb");
    if(fp_handle == NULL)
        return RET_FAILURE;

    fseek(fp_handle,0,SEEK_END);
    file_length = (unsigned long)ftell(fp_handle);

    //if(file_length > buffer_size)
    //{
        //file_length=buffer_size;
    //}
    *buffer_size = file_length;

    fseek(fp_handle,0,SEEK_SET);
    //fread(file_buffer,sizeof(char),file_length,fp_handle);
    fread(file_buffer,sizeof(char),file_length,fp_handle);
    fclose(fp_handle);

    return RET_SUCCESS;

}

static RET_CODE usb_write_log(unsigned char* file_name, void *file_buffer,INT32* buffer_size)
{
    char dir_str[USBUPG_FILE_NAME_LENGTH];
    FILE *fp_handle;

    snprintf(dir_str,USBUPG_FILE_NAME_LENGTH,"/c/%s",file_name);
    fp_handle = fopen(dir_str,"a+b");
    if(fp_handle == NULL)
        return RET_FAILURE;

    fwrite(file_buffer, sizeof(char), buffer_size, fp_handle);
    fclose(fp_handle);

    osal_task_sleep(100);
    fs_sync("/c");

    return RET_SUCCESS;
}

static BOOL usb_file_rename(char* source_file_path, char* new_file_path)
{
    char src_name[128], des_name[128];
    snprintf(src_name,128,"/c/%s",source_file_path);
    snprintf(des_name,128,"/c/%s",new_file_path);
    fs_rename(src_name, des_name);

    return TRUE;
}

static RET_CODE find_min_name_file(char* minname_file_name, int minname_buf_size, UPG_FILENODE *upg_filelist, \
    UINT16 upg_node_num)
{
    strncpy(minname_file_name, upg_filelist[0].upg_file_name, minname_buf_size-1);

    int i=0;
    for(i=0;i<upg_node_num;i++)
    {
        if(strcmp(minname_file_name, upg_filelist[i].upg_file_name)>0)
        {
            strncpy(minname_file_name, upg_filelist[i].upg_file_name, minname_buf_size);
        }
    }

    return RET_SUCCESS;
}

static RET_CODE usb_upg_file_init(void)
{
    upg_filelist = (UPG_FILENODE *)MALLOC(sizeof(UPG_FILENODE) * UPG_MAX_FILENUM_SUPPORT);
    if(upg_filelist == NULL)
    {
        return RET_FAILURE;
    }
    else
    {
        upg_node_num = 0;//initlize node buffer
        MEMSET(upg_filelist,0x0, sizeof(UPG_FILENODE) * UPG_MAX_FILENUM_SUPPORT);

        return RET_SUCCESS;
    }

    return RET_SUCCESS;
}

static RET_CODE usb_upg_file_de_init(void)
{
    if(upg_filelist == NULL)
    {
        return RET_FAILURE;
    }
    else
    {
        FREE(upg_filelist);

        return RET_SUCCESS;
    }

    return RET_SUCCESS;
}

static RET_CODE usb_get_file_with_extends(char* upgfile_extends, UPG_FILENODE *upg_filelist, UINT16* upg_node_num)
{
    INT32 ret = RET_SUCCESS;

    UINT16 str_len;
    int dir_fd;
    struct dirent *dent;
    char usbupg_fsbuff[sizeof(struct dirent) + FILE_NAME_SIZE];
    UINT16 upg_node_num_tmp = 0;

    dir_fd = fs_opendir("/c");
    if(dir_fd < 0)/*USB disk root dir open failure*/
    {
        return RET_FAILURE;
    }

    dent = ( struct dirent *) usbupg_fsbuff;
    upg_node_num_tmp = 0;//initlize node buffer
    MEMSET(upg_filelist,0x0, sizeof(UPG_FILENODE) * UPG_MAX_FILENUM_SUPPORT);
    while(1)
    {
        if (fs_readdir(dir_fd,dent) <= 0)
            break;/*Tail of the dir files*/

        if(S_ISDIR(dent->d_type))
        {
            continue;
        }
        else
        {
            str_len = STRLEN(dent->d_name);
            if(!strncasecmp((dent->d_name + str_len - 3),upgfile_extends,3))
            {
                strncpy(upg_filelist[upg_node_num_tmp].upg_file_name, dent->d_name, 127);
                upg_filelist[upg_node_num_tmp].upg_file_name[127] = 0;

                upg_node_num_tmp++;
                if(upg_node_num_tmp > UPG_MAX_FILENUM_SUPPORT)
                    break;//check usbupg files if overflow
            }
        }
    }

    fs_closedir(dir_fd);

    *upg_node_num = upg_node_num_tmp;
    if(upg_node_num_tmp == 0)
        ret = RET_FAILURE;

    return ret;
}


static RET_CODE usb_get_min_name_file_with_extends(char* upgfile_extends, char* minname_file_name, int minname_buf_size)
{
    RET_CODE ret = RET_FAILURE;

    usb_upg_file_init();

    ret = usb_get_file_with_extends(upgfile_extends, upg_filelist, &upg_node_num);

    if(RET_SUCCESS==ret)
    {
        ret = find_min_name_file(minname_file_name, minname_buf_size, upg_filelist, upg_node_num);
    }

    usb_upg_file_de_init();

    return ret;
}

static RET_CODE usb_get_file_with_chief_name(char* upgfile_chiefname, int compare_len, UPG_FILENODE *upg_filelist, \
    UINT16* upg_node_num)
{
    INT32 ret = RET_SUCCESS;

    UINT16 str_len;
    int dir_fd;
    struct dirent *dent;
    char usbupg_fsbuff[sizeof(struct dirent) + FILE_NAME_SIZE];
    UINT16 upg_node_num_tmp = 0;

    dir_fd = fs_opendir("/c");
    if(dir_fd < 0)/*USB disk root dir open failure*/
    {
        return RET_FAILURE;
    }

    dent = ( struct dirent *) usbupg_fsbuff;
    upg_node_num_tmp = 0;//initlize node buffer
    MEMSET(upg_filelist,0x0, sizeof(UPG_FILENODE) * UPG_MAX_FILENUM_SUPPORT);
    while(1)
    {
        if (fs_readdir(dir_fd,dent) <= 0)
            break;/*Tail of the dir files*/

        if(S_ISDIR(dent->d_type))
        {
            continue;
        }
        else
        {
            str_len = STRLEN(dent->d_name);
            if(!strncasecmp((dent->d_name),upgfile_chiefname,compare_len))
            {
                strncpy(upg_filelist[upg_node_num_tmp].upg_file_name, dent->d_name, 127);
                upg_filelist[upg_node_num_tmp].upg_file_name[127] = 0;

                upg_node_num_tmp++;
                if(upg_node_num_tmp > UPG_MAX_FILENUM_SUPPORT)
                    break;//check usbupg files if overflow
            }
        }
    }

    fs_closedir(dir_fd);

    *upg_node_num = upg_node_num_tmp;
    if(upg_node_num_tmp == 0)
        ret = RET_FAILURE;

    return ret;
}


static RET_CODE usb_get_min_name_file_with_chief_name(char* upgfile_chiefname, int compare_len,
    char* minname_file_name, int minname_buf_size)
{
    RET_CODE ret = RET_FAILURE;

    usb_upg_file_init();

    ret = usb_get_file_with_chief_name(upgfile_chiefname, compare_len, upg_filelist, &upg_node_num);
    
    if(RET_SUCCESS==ret)
    {
        ret = find_min_name_file(minname_file_name, minname_buf_size, upg_filelist, upg_node_num);
    }

    usb_upg_file_de_init();

    return ret;
}

BOOL check_security_upg_file_valid(void)
{
    BOOL ret = FALSE;

    char file_name[128];
    if((usb_get_min_name_file_with_chief_name("Encrypted_data",FILENAME_COMPARE_LENGTH,file_name)==RET_SUCCESS)||\
        (usb_get_min_name_file_with_chief_name("product_sabbat_dual",FILENAME_COMPARE_LENGTH,file_name)==RET_SUCCESS))
    {
        ret = TRUE;
    }

    return ret;
}

static char lower[] = "0123456789abcdefghijklmnopqrstuvwxyz";
static char * strrev(char *s)
{
    int j,k,c;
    for(k=0;s[k] != 0;k++);
    for(j=0,k--;j<k;j++,k--) {
    c = s[j];
    s[j] = s[k];
    s[k] = c;
    }
    return s;
}
static char *itoa(int n, char *s, int radix)
{
    unsigned int uval;
    int i, sign;

    if((sign = (n < 0)) && (LENGTH_O_ECIMAL == radix))
    uval = -n;
    else
    uval = n;

    i=0;
    do
    {
      s[i++] = lower[uval % radix];
      uval /= radix;
    }while(uval>0);

    if (sign)
    s[i++] = '-';
    s[i] = '\0';
    return strrev(s);
}


char* get_chip_id_name(void)
{
    char chip_id_name[128] = {0};

#if 0
    UINT32 chip_id;

    chip_id = sys_ic_get_chip_id();

    if(ALI_M3327==chip_id)
    {
        strcpy(chip_id_name,"ALI_M3327");
    }
    else if(ALI_M3329E==chip_id)
    {
        strcpy(chip_id_name,"ALI_M3329E");
    }
    else if(ALI_M3327C==chip_id)
    {
        strcpy(chip_id_name,"ALI_M3327C");
    }
    else if(ALI_M3202==chip_id)
    {
        strcpy(chip_id_name,"ALI_M3202");
    }
    else if(ALI_M3101==chip_id)
    {
        strcpy(chip_id_name,"ALI_M3101");
    }
    else if(ALI_M3501==chip_id)
    {
        strcpy(chip_id_name,"ALI_M3501");
    }
    else if(ALI_S3601==chip_id)
    {
        strcpy(chip_id_name,"ALI_S3601");
    }
    else if(ALI_S3602==chip_id)
    {
        strcpy(chip_id_name,"ALI_S3602");
    }
    else if(ALI_S3602F==chip_id)
    {
        strcpy(chip_id_name,"ALI_S3606");
    }
    else
    {
        strcpy(chip_id_name, "UnknowChip");
    }
#else
    UINT32 chipid[2];
    char tmp[128];
    INT32 t_len = 0;

    otp_init(NULL);
    MEMSET(chipid, 0, 8);
    otp_read(0x00*4, chipid, 8) ;
    MEMSET(tmp,0,128);
    itoa(chipid[0], tmp, 16);
    strncpy(chip_id_name, tmp, 127);
    strncat(chip_id_name, ",", 1);
    itoa(chipid[1], tmp, 16);
    t_len = 128 - strlen(chip_id_name);
    strncat(chip_id_name, tmp, t_len-1);
#endif

    return chip_id_name;

}

void get_upgrade_msg(char* msg, INT32 msg_buf_len)
{
    char str[1024] = {0};
    INT32 t_len = 0;

    strncpy(str, get_chip_id_name(), 1023);

    strncat(str, "\nupdated file:", 16);
    t_len = 1024 - strlen(str);
    strncat(str, usb_upg_msg, t_len - 1);

    strncpy(msg, str, msg_buf_len-1);
    msg[msg_buf_len-1] = 0;
}

enum otp_upgrade_type
{
    EJTAG_UPG=0,
    PVR_UPG=1,
    HDCPKEY_UPG=2,
    ROOT_UPG=3,
    CONFIG_UPG  =4,
    MAX_OTP_UPG,
};

enum flash_upgrade_type
{
    BLOADER_UPG,
    MAINCODE_UPG,
    HDCPKEY_FLASH_UPG,
    CIPLUSKEY_UPG,
    SEECODE_UPG,
    ALLCODE_UPG,
    MAX_FLASH_UPG,
};


void encrypted_data_proc(void)
{
    char* encrypt_addr=ENCRYPT_PACK_ADDR;
    char* decrypt_addr=DECRYPT_PACK_ADDR;
    UINT8*  fw_addr=NULL;
    int fw_size=0, bloader_size=0, encryptdata_size=0;
    int ret;
    UINT32 chunk_id,chunk_len,chunk_addr;
    date_time local_time;
    char tmp_str[128];
    char  tmp[128];
    UINT32 *malloc_otp_buff;
    char file_name_fw[128],  file_name_key[128], file_rename[128];
    BOOL is_top_burn_success[MAX_OTP_UPG];
    BOOL is_flash_burn_success[MAX_FLASH_UPG];
    BOOL is_read_fw_file_success = FALSE;
    BOOL is_read_key_file_success = FALSE;
    INT32 *hdcpkey_fw_add;
    INT32 *cipluskey_fw_add;
    INT32 *hdcpkey_add;
    INT32 *cipluskey_add;
    const int NCRC=0x4352434e;//0x4e435243;
    INT32 t_len = 0;

///////////////////// prepair for Usb log msg ////////////////////////////
    MEMSET(tmp_str, 0, 128);

    get_local_time(&local_time);

    itoa(local_time.year, tmp, 10);
    strncpy(tmp_str, tmp, 127);
    strncat(tmp_str, "-", 1);
    itoa(local_time.month, tmp, 10);
    t_len = 128 - strlen(tmp_str);
    strncat(tmp_str, tmp, t_len-1);
    t_len = 128 - strlen(tmp_str);
    strncat(tmp_str, "-", t_len-1);
    itoa(local_time.day, tmp, 10);
    t_len = 128 - strlen(tmp_str);
    strncat(tmp_str, tmp, t_len-1);
    t_len = 128 - strlen(tmp_str);
    strncat(tmp_str, " ", t_len-1);
    itoa(local_time.hour, tmp, 10);
    t_len = 128 - strlen(tmp_str);
    strncat(tmp_str, tmp, t_len-1);
    t_len = 128 - strlen(tmp_str);
    strncat(tmp_str, ":", t_len-1);
    itoa(local_time.min, tmp, 10);
    t_len = 128 - strlen(tmp_str);
    strncat(tmp_str, tmp, t_len-1);
    t_len = 128 - strlen(tmp_str);
    strncat(tmp_str, "\r\n", t_len-1);
    usb_write_log(SECURITY_LOG_FILE_NAME, tmp_str, strlen(tmp_str));

    strncpy(tmp_str, get_chip_id_name(), 127);
    t_len = 128 - strlen(tmp_str);
    strncat(tmp_str, "\r\n", t_len-1);
    usb_write_log(SECURITY_LOG_FILE_NAME, tmp_str, strlen(tmp_str));

    strncpy(usb_upg_msg,"\0", 1);


///////////////////// FW data /////////////////////////////////////

    if(usb_get_min_name_file_with_chief_name("product_sabbat_dual",FILENAME_COMPARE_LENGTH,file_name_fw)==RET_SUCCESS)
    {
        //====>step3 read fw from usb
        get_usb_file_len(file_name_fw,&fw_size);
        fw_addr = (UINT8 *)MALLOC(sizeof(UINT8)*fw_size);
        if(NULL == fw_addr)
        {
            libc_printf("err:malloc failed");
            return;
        }
        ret = usb_read_file(file_name_fw, fw_addr, &fw_size);
        if(ret == RET_SUCCESS)
        {
            is_read_fw_file_success = TRUE;
        }
    }
    if(usb_get_min_name_file_with_chief_name("Encrypted_data",FILENAME_COMPARE_LENGTH,file_name_key)==RET_SUCCESS)
    {
        ret = usb_read_file(file_name_key, encrypt_addr,&encryptdata_size);
        if(ret == RET_SUCCESS)
        {
            decrypt_pack_data(encrypt_addr, decrypt_addr,encryptdata_size, pwd);
            is_read_key_file_success = TRUE;
        }
    }

    //fw
    if(is_read_fw_file_success)
    {
        strncpy(tmp_str, "--->", 127);
        t_len = 128 - strlen(tmp_str);
        strncat(tmp_str, file_name_fw, t_len-1);
        t_len = 128 - strlen(tmp_str);
        strncat(tmp_str, " upgrading\r\n", t_len-1);
        usb_write_log(SECURITY_LOG_FILE_NAME, tmp_str,strlen(tmp_str));

        //burn allcode & hdcpkey & ciplus key
        //read hdcpkey & cipluskey from keyfile,merge to fw file
        if(is_read_key_file_success)
        {
            //read hdcp key to fw file
            UINT32 hdcp_chuck_id = HDCPKEY_CHUNK_ID;
            UINT32 hdcp_len;
            UINT32 hdcp_crc;
            hdcpkey_add = HDCP_KEY_ADDR;
            chunk_init(fw_addr, fw_size);
            hdcpkey_fw_add = (UINT32)chunk_goto(&hdcp_chuck_id,0xFFFFFFFF,1) + CHUNK_HEADER_SIZE;
            MEMCPY(hdcpkey_fw_add, hdcpkey_add, HDCP_KEY_SIZE);//data
            hdcp_len = data_change_for_sto(HDCP_KEY_SIZE+CHUNK_SUBHEADER_SIZE);//len
            MEMCPY((UINT32)hdcpkey_fw_add-CHUNK_HEADER_SIZE+4, &hdcp_len,4);
            if(0!=MEMCMP((UINT32)hdcpkey_fw_add-CHUNK_HEADER_SIZE+12, &NCRC, 4))//crc
            {
                hdcp_crc = mg_table_driven_crc(0xFFFFFFFF, (UINT32)hdcpkey_fw_add+16, \
                                                                        HDCP_KEY_SIZE+CHUNK_SUBHEADER_SIZE);
                MEMCPY((UINT32)hdcpkey_fw_add-CHUNK_HEADER_SIZE+12, &hdcp_crc,4);
            }

            //read ciplus key to fw file
            UINT32 ciplus_chuck_id = CIPLUSKEY_CHUNK_ID;
            UINT32 ciplus_len;
            UINT32 ciplus_crc;
            cipluskey_add = CIPLUS_KEY_ADDR;
            chunk_init(fw_addr, fw_size);
            cipluskey_fw_add = (UINT32)chunk_goto(&ciplus_chuck_id,0xFFFFFFFF,1) + CHUNK_HEADER_SIZE;
            MEMCPY(cipluskey_fw_add, cipluskey_add, CIPLUS_KEY_SIZE);//data
            ciplus_len = data_change_for_sto(CIPLUS_KEY_SIZE+CHUNK_SUBHEADER_SIZE);//len
            MEMCPY((UINT32)cipluskey_fw_add-CHUNK_HEADER_SIZE+4, &ciplus_len,4);
            if(0!=MEMCMP((UINT32)cipluskey_fw_add-CHUNK_HEADER_SIZE+12, &NCRC, 4))//crc
            {
                ciplus_crc = mg_table_driven_crc(0xFFFFFFFF, (UINT32)cipluskey_fw_add+16, \
                                                                        CIPLUS_KEY_SIZE+CHUNK_SUBHEADER_SIZE);
                MEMCPY((UINT32)cipluskey_fw_add-CHUNK_HEADER_SIZE+12, &hdcp_crc,4);
            }

            is_flash_burn_success[HDCPKEY_FLASH_UPG] = TRUE;
            is_flash_burn_success[CIPLUSKEY_UPG] = TRUE;
        }
        //burn allcode
        UINT32 sector_count=0;
        UINT32 param;
        osal_interrupt_disable();
        for(sector_count=0;sector_count< ((fw_size-1)/SECTOR_SIZE+1);sector_count++)
        {
#ifdef WATCH_DOG_SUPPORT
            dog_set_time(0, 0);
#endif
            //param = 0x00+sector_count*SECTOR_SIZE;
        //  param = param << 10;    /* Lower 10 bits of LSB is length in K bytes*/
            //param += SECTOR_SIZE >> 10;
            ///sto_io_control((struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0), STO_DRIVER_SECTOR_ERASE, param);
            UINT32 tmp_param[2];
            tmp_param[0] = 0x00+sector_count*SECTOR_SIZE;
            tmp_param[1] = SECTOR_SIZE >>10;
            param= (UINT32)tmp_param;
            //Summic add for check the >=4M Flash
            sto_io_control((struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0),STO_DRIVER_SECTOR_ERASE_EXT,param);
        }
        osal_interrupt_enable();

        sto_lseek((struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0),0x00,STO_LSEEK_SET) ;
        sto_write((struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0),fw_addr, fw_size);
        is_flash_burn_success[ALLCODE_UPG] = TRUE;

        if(is_flash_burn_success[ALLCODE_UPG] == TRUE)
        {
            usb_write_log(SECURITY_LOG_FILE_NAME, "      fw upgrade complete!\r\n",\
                                    strlen( "      fw upgrade complete!\r\n"));
            t_len = 256 - strlen(usb_upg_msg);
            strncat(usb_upg_msg,"\n", t_len-1);
            t_len = 256 - strlen(usb_upg_msg);
            strncat(usb_upg_msg,file_name_fw, t_len-1);
        }
        else
        {
            //fw burn err
            usb_write_log(SECURITY_LOG_FILE_NAME, "      fw upgrade err!\r\n",strlen( "      fw upgrade err!\r\n"));
            strncpy(usb_upg_msg,"fw Burn Err! Reburn fw!", 255);
            FREE(fw_addr);
            return;
        }
    }
    FREE(fw_addr);

    //keys
    if(is_read_key_file_success)
    {
        strncpy(tmp_str, "--->", 4);
        t_len = 128 - strlen(tmp_str);
        if (t_len > strlen(file_name_key))
        {
            strncat(tmp_str, file_name_key, t_len-1);
        }
        t_len = 128 - strlen(tmp_str);
        if (t_len > strlen(" upgrading\r\n"))
        {
            strncat(tmp_str, " upgrading\r\n", t_len-1);
        }
        usb_write_log(SECURITY_LOG_FILE_NAME, tmp_str,strlen(tmp_str));

        //burn keys
        //burn hdcp & ciplus key
        if((is_flash_burn_success[HDCPKEY_FLASH_UPG] !=TRUE)||(is_flash_burn_success[CIPLUSKEY_UPG] !=TRUE))
        {
            //hdcp & cipluskey write  complete
            //write hdcp key
            chunk_id = HDCPKEY_CHUNK_ID;
            if(keyupg_burnflash(chunk_id, HDCP_KEY_ADDR,HDCP_KEY_SIZE) == TRUE)
            {
                usb_write_log(SECURITY_LOG_FILE_NAME, "      hdcp key upgrade complete!\r\n",\
                                        strlen( "      hdcp key upgrade complete!\r\n"));
                is_flash_burn_success[HDCPKEY_FLASH_UPG] = TRUE;
            }
            else
            {
                usb_write_log(SECURITY_LOG_FILE_NAME, "      hdcp key upgrade err!\r\n",\
                                        strlen( "      hdcp key upgrade err!\r\n"));
                is_flash_burn_success[HDCPKEY_FLASH_UPG] = FALSE;
            }
            //write ci+ key
            chunk_id = CIPLUSKEY_CHUNK_ID;
            if(keyupg_burnflash(chunk_id, CIPLUS_KEY_ADDR,CIPLUS_KEY_SIZE) == TRUE)
            {
                usb_write_log(SECURITY_LOG_FILE_NAME, "ci+ key upgrade complete!\r\n",\
                                        strlen( "ci+ key upgrade complete!\r\n"));
                is_flash_burn_success[CIPLUSKEY_UPG] = TRUE;
            }
            else
            {
                usb_write_log(SECURITY_LOG_FILE_NAME, "ci+ key upgrade err!\r\n",strlen( "ci+ key upgrade err!\r\n"));
                is_flash_burn_success[CIPLUSKEY_UPG] = FALSE;
            }
        }
        //burn otp key
        if((is_flash_burn_success[HDCPKEY_FLASH_UPG] ==TRUE)&&(is_flash_burn_success[CIPLUSKEY_UPG] ==TRUE))
        {
            malloc_otp_buff = (UINT32*)MALLOC(sizeof(UINT32)*0x200);
            if(NULL == malloc_otp_buff)
            {
                libc_printf("err:malloc failed");
                return;
            }
            MEMSET(malloc_otp_buff, 0, (sizeof(UINT32)*0x200));
            //====>step3 write ejtag/pvr/hdcp_protect/root_public key to otp
            //root_public key to otp
            if(keyupg_burnotp(malloc_otp_buff, ROOT_OTP_ADDR, ROOT_PUBLIC_KEY_SIZE, ROOT_PUBLIC_KEY_ADDR, FALSE))
            {
                usb_write_log(SECURITY_LOG_FILE_NAME, "      root_public key upgrade complete!\r\n",\
                                        strlen( "      root_public key upgrade complete!\r\n"));
                is_top_burn_success[ROOT_UPG] = TRUE;
            }
            else
            {
                usb_write_log(SECURITY_LOG_FILE_NAME, "      root_public key upgrade error!\r\n",\
                                        strlen( "      root_public key upgrade error!\r\n"));
                is_top_burn_success[ROOT_UPG] = FALSE;
            }
            //hdcp protect key to otp
            if(keyupg_burnotp(malloc_otp_buff, HDCP_OTP_ADDR, HDCP_PROTECT_KEY_SIZE, HDCP_PROTECT_KEY_ADDR, TRUE))
            {
                usb_write_log(SECURITY_LOG_FILE_NAME, "      hdcp_protect key upgrade complete!\r\n",\
                                        strlen( "      hdcp_protect key upgrade complete!\r\n"));
                is_top_burn_success[HDCPKEY_UPG] = TRUE;
            }
            else
            {
                usb_write_log(SECURITY_LOG_FILE_NAME, "      hdcp_protect key upgrade error!\r\n",\
                                        strlen( "      hdcp_protect key upgrade error!\r\n"));
                is_top_burn_success[HDCPKEY_UPG] = FALSE;
            }
            //pvr.key to otp
            if(keyupg_burnotp(malloc_otp_buff, PVR_OTP_ADDR, PVR_KEY_SIZE, PVR_KEY_ADDR, FALSE))
            {
                usb_write_log(SECURITY_LOG_FILE_NAME, "      pvr key upgrade complete!\r\n",\
                                        strlen( "      pvr key upgrade complete!\r\n"));
                is_top_burn_success[PVR_UPG] = TRUE;
            }
            else
            {
                usb_write_log(SECURITY_LOG_FILE_NAME, "      pvr key upgrade error!\r\n",\
                                        strlen( "      pvr key upgrade error!\r\n"));
                is_top_burn_success[PVR_UPG] = FALSE;
            }
            //ejtag.key to otp
            if(keyupg_burnotp(malloc_otp_buff, EJTAG_OTP_ADDR, EJTAG_KEY_SIZE, EJTAG_KEY_ADDR, FALSE))
            {
                usb_write_log(SECURITY_LOG_FILE_NAME, "      ejtag key upgrade complete!\r\n",\
                                        strlen( "      ejtag key upgrade complete!\r\n"));
                is_top_burn_success[EJTAG_UPG] = TRUE;
            }
            else
            {
                usb_write_log(SECURITY_LOG_FILE_NAME, "      ejtag key upgrade error!\r\n",\
                                        strlen( "      ejtag key upgrade error!\r\n"));
                is_top_burn_success[EJTAG_UPG] = FALSE;
            }

            //to set otp config
            if(is_top_burn_success[EJTAG_UPG]&&is_top_burn_success[HDCPKEY_UPG]&&is_top_burn_success[PVR_UPG]
                &&is_top_burn_success[ROOT_UPG])
            {
#ifndef NOT_BURN_OPT_CONFIG_TEST
                if(keyupg_burnotp_config())
#else
                if(1)
#endif
                {
                    usb_write_log(SECURITY_LOG_FILE_NAME,"otp_config complete!\r\n",strlen("otp_config complete!\r\n"));
                    is_top_burn_success[CONFIG_UPG] = TRUE;

                    strncpy(file_rename,"_", 1);
                    t_len = 128 - strlen(file_rename);
                    if (t_len > strlen(file_name_key))
                    {
                        strncat(file_rename, file_name_key, t_len-1);
                    }
                    t_len = 128 - strlen(file_rename);
                    if (t_len > strlen(".bck"))
                    {
                        strncat(file_rename, ".bck", t_len-1);
                    }
                    usb_file_rename(file_name_key, file_rename);
                }
                else
                {
                    usb_write_log(SECURITY_LOG_FILE_NAME, "otp_config error!\r\n",strlen( "otp_config error!\r\n"));
                    is_top_burn_success[CONFIG_UPG] = FALSE;
                }
            }
            else
            {
                usb_write_log(SECURITY_LOG_FILE_NAME, "otp data burn error,pls reburn!\r\n",\
                                        strlen( "otp data burn error,pls reburn!\r\n"));
                is_top_burn_success[CONFIG_UPG] = FALSE;
            }


            FREE(malloc_otp_buff);

            if(is_top_burn_success[CONFIG_UPG])
            {
                t_len = 256 - strlen(usb_upg_msg);
                if (t_len > 1)
                {
                    strncat(usb_upg_msg,"\n", 1);
                }
                t_len = 256 - strlen(usb_upg_msg);
                if (t_len > strlen(file_name_key))
                {
                    strncat(usb_upg_msg,file_name_key, t_len - 1);
                }
            }
            else
            {
                strncpy(usb_upg_msg,"\nOtp Burn Err!Reburn Otp!", (256-1));
            }
        }

    }

    usb_write_log(SECURITY_LOG_FILE_NAME, "=====================\r\n\r\n",strlen( "=====================\r\n\r\n"));

}

#endif

