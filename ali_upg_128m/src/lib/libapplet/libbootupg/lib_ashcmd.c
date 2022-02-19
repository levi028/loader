/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2002 Copyright (C)
 *
 *  File: lib_ashcmd.c
 *
 *  Description: This file contains all functions definition ALi shell
 *                 command functions.
 *  History:
 *      Date        Author      Version  Comment
 *      ====        ======      =======  =======
 *  1   2004.12.13  Hong Zhang  MTW_3327 Initial
 *  2   2005.1.31   Hong Zhang 128B header
 *  3   2005.2.16   Hong Zhang  Comext Initial
 *  4   2006.5.16   Justin Wu   Update STO chunck support, and clean up code.
 ****************************************************************************/

#include <types.h>
#include <retcode.h>
#include <sys_config.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libc/fast_crc.h>
#include <api/libchunk/chunk.h>
#include <bus/sci/sci.h>
#include <bus/dog/dog.h>
#include <bus/flash/flash.h>
#include <hld/hld_dev.h>

#include "lib_ash.h"

#ifndef ENABLE_EROM
#ifdef _DEBUG_VERSION_
#define DEBUG
#define SH_PRINTF   soc_printf
#else
#define SH_PRINTF(...)
#endif

#define PACKAGE_SIZE    1024
#if (SYS_SDRAM_SIZE == 2)
#define IMAGE_START     0x80080000
#else
#define IMAGE_START     0x80200000
#define ZIP_BUFFER    0x80400000
#endif

#define LZMA_BASE_SIZE    1846
#define LZMA_LIT_SIZE    768
#define BUFFER_SIZE     ((LZMA_BASE_SIZE + (LZMA_LIT_SIZE << (0 + 2))) * sizeof(UINT16))

#ifdef PANEL_DISPLAY
extern struct pan_device* panel_dev;
#endif
extern unsigned char g_crc_bad;
extern struct sto_device *flash_dev;

static unsigned long start_addr;
static unsigned char *image_free;

#define TRANSFER_RETRY_TIMES 3
#ifdef UPGRADE_ONCE
static BOOL receive_comtest_falg = TRUE;//benjamin add.when upgrade via host which is in stanby mode,only allow upgrade once.
                                        //it's special case,only for smt
#endif

extern int un7zip(UINT8 *, UINT8 *, UINT8 *);

static int cmd_address(unsigned int argc, unsigned char *argv[]);
static int cmd_burn(unsigned int argc, unsigned char *argv[]);
static int cmd_comtest(unsigned int argc, unsigned char *argv[]);
static int cmd_move(unsigned int argc, unsigned char *argv[]);
static int cmd_reboot(unsigned int argc, unsigned char *argv[]);
static int cmd_transfer(unsigned int argc, unsigned char *argv[]);
static int cmd_transferraw(unsigned int argc, unsigned char *argv[]);
static int cmd_version(unsigned int argc, unsigned char *argv[]);
static int cmd_dump(unsigned int argc, unsigned char *argv[]);
static int cmd_set(unsigned int argc, unsigned char *argv[]);


#ifdef DTTM
void cmd_diag_sw_number(unsigned int argc, unsigned char *argv[]);
void cmd_diag_hw_number(unsigned int argc, unsigned char *argv[]);
#endif

#if (SYS_SDRAM_SIZE == 2)
static UINT32 total_transfer_size;
static UINT32 total_burn_size;
static UINT32 transfer_size;
static UINT32 burn_size;
#endif


/* Command <-> function map list */
struct ash_cmd lib_ash_command[] =
{
    {"address", cmd_address},
    {"burn", cmd_burn},
    {"comtest", cmd_comtest},
    {"move", cmd_move},
    {"reboot", cmd_reboot},
    {"transfer", cmd_transfer},
    {"transferraw", cmd_transferraw},
    {"version", cmd_version},
    {"dump", cmd_dump},
    {"set", cmd_set}
};


UINT32 boot_uart_id = SCI_FOR_RS232;
void boot_uart_set_id(UINT32 uart_id)
{
    boot_uart_id = uart_id;
    return;
}

UINT32 boot_uart_get_id()
{
    return boot_uart_id;
}

#ifdef DEBUG
void dump(unsigned char *p, int l)
{
    int i = 0;

    while (l > 0)
    {
        if (i % 16 == 0)
        {
            SH_PRINTF("\n%08X:", (unsigned long)p);
        }
        SH_PRINTF(" %02X", *p++);
        i++;
        l--;
    }
    SH_PRINTF("\n");
}
#endif

#ifdef PANEL_DISPLAY
static const unsigned char down_str[] = { '_', '-' };
static const unsigned char burn_str[] = { '-', '=' };
static const unsigned char dup_str[] = { '1', '=' };
static unsigned int led_display_type;
#ifdef PANEL_DISPLAY_ICON
// Add dot to config icon of panel display
static const unsigned char dot_str[] = { '.' };
static void led_show(unsigned char *patn)
{
    unsigned int i;
    unsigned char led_dispay_str[5];

    for (i = 0; i < 4; i++)
    {
        if(i>0)
            led_dispay_str[i+1] = patn[(i + led_display_type) % 2];
        else
            led_dispay_str[i] = patn[(i + led_display_type) % 2];
    }
    led_dispay_str[1] = dot_str[0];
    led_display_type++;
    pan_display(panel_dev, led_dispay_str, 5);
}
#else
static void led_show(unsigned char *patn)
{
    unsigned int i;
    unsigned char led_dispay_str[4];

    for (i = 0; i < 4; i++)
        led_dispay_str[i] = patn[(i + led_display_type) % 2];
    led_display_type++;
    pan_display(panel_dev, led_dispay_str, 4);
}
#endif
#endif

static void dump_str(unsigned char *p)
{
    while (*p)
        LIB_ASH_OC(*p++);
}


#ifdef DTTM
static int cmd_diag_entry(unsigned int argc, unsigned char *argv[])
{
    LIB_ASH_OC('@');
    return 0;
}

extern UINT8 s_ver[16];
void cmd_diag_sw_number(unsigned int argc, unsigned char *argv[])
{
#if 1
    LIB_ASH_OC(0x0a);
    LIB_ASH_OC(0x0d);
    LIB_ASH_OC('6');
    LIB_ASH_OC('3');
    LIB_ASH_OC('0');
    LIB_ASH_OC('1');
    LIB_ASH_OC(0x0a);
    LIB_ASH_OC(0x0d);
    LIB_ASH_OC('0');
    LIB_ASH_OC('x');

    LIB_ASH_OC('0');
    LIB_ASH_OC(s_ver[0]);
    LIB_ASH_OC('0');
    LIB_ASH_OC(s_ver[1]);
    LIB_ASH_OC(' ');
    LIB_ASH_OC('0');
    LIB_ASH_OC('x');
    LIB_ASH_OC('0');
    LIB_ASH_OC(s_ver[2]);
    LIB_ASH_OC('0');
    LIB_ASH_OC(s_ver[3]);
    LIB_ASH_OC(' ');
    LIB_ASH_OC('0');
    LIB_ASH_OC('x');
    LIB_ASH_OC('0');
    LIB_ASH_OC(s_ver[4]);
    LIB_ASH_OC('0');
    LIB_ASH_OC(s_ver[5]);
    LIB_ASH_OC(' ');
    LIB_ASH_OC('0');
    LIB_ASH_OC('x');
    LIB_ASH_OC('0');
    LIB_ASH_OC(s_ver[6]);
    LIB_ASH_OC('0');
    LIB_ASH_OC(s_ver[7]);
    LIB_ASH_OC(' ');
    LIB_ASH_OC('I');
    LIB_ASH_OC('B');
    LIB_ASH_OC('O');
    LIB_ASH_OC('Z');
    LIB_ASH_OC('-');
    LIB_ASH_OC('-');
    LIB_ASH_OC('-');
    LIB_ASH_OC('-');
    LIB_ASH_OC(' ');
    LIB_ASH_OC('>');
    LIB_ASH_OC('0');
    LIB_ASH_OC('0');
    LIB_ASH_OC('0');
    LIB_ASH_OC('0');
    LIB_ASH_OC(' ');
    LIB_ASH_OC('@');
    LIB_ASH_OC(0x0a);
    LIB_ASH_OC(0x0d);

#else
    LIB_ASH_OC(0x0a);
    LIB_ASH_OC(0x0d);
    LIB_ASH_OC('6');
    LIB_ASH_OC('3');
    LIB_ASH_OC('0');
    LIB_ASH_OC('1');
    LIB_ASH_OC(0x0a);
    LIB_ASH_OC(0x0d);
    LIB_ASH_OC('0');
    LIB_ASH_OC('x');
    LIB_ASH_OC('0');
    LIB_ASH_OC('0');
    LIB_ASH_OC('0');
    LIB_ASH_OC('4');
    LIB_ASH_OC(' ');
    LIB_ASH_OC('0');
    LIB_ASH_OC('x');
    LIB_ASH_OC('0');
    LIB_ASH_OC('2');
    LIB_ASH_OC('0');
    LIB_ASH_OC('3');
    LIB_ASH_OC(' ');
    LIB_ASH_OC('0');
    LIB_ASH_OC('x');
    LIB_ASH_OC('0');
    LIB_ASH_OC('1');
    LIB_ASH_OC('0');
    LIB_ASH_OC('2');
    LIB_ASH_OC(' ');
    LIB_ASH_OC('0');
    LIB_ASH_OC('x');
    LIB_ASH_OC('0');
    LIB_ASH_OC('1');
    LIB_ASH_OC('0');
    LIB_ASH_OC('2');
    LIB_ASH_OC(' ');
    LIB_ASH_OC('I');
    LIB_ASH_OC('B');
    LIB_ASH_OC('O');
    LIB_ASH_OC('Z');
    LIB_ASH_OC('-');
    LIB_ASH_OC('-');
    LIB_ASH_OC('-');
    LIB_ASH_OC('-');
    LIB_ASH_OC(' ');
    LIB_ASH_OC('>');
    LIB_ASH_OC('0');
    LIB_ASH_OC('0');
    LIB_ASH_OC('0');
    LIB_ASH_OC('0');
    LIB_ASH_OC(' ');
    LIB_ASH_OC('@');
    LIB_ASH_OC(0x0a);
    LIB_ASH_OC(0x0d);
#endif
}
void cmd_diag_hw_number(unsigned int argc, unsigned char *argv[])
{
    LIB_ASH_OC('6');
    LIB_ASH_OC('8');
    LIB_ASH_OC('0');
    LIB_ASH_OC('1');
    LIB_ASH_OC(0x0a);
    LIB_ASH_OC(0x0d);
    LIB_ASH_OC('0');
    LIB_ASH_OC('x');
    LIB_ASH_OC('0');
    LIB_ASH_OC(s_ver[4]);
    LIB_ASH_OC('0');
    LIB_ASH_OC(s_ver[5]);
    LIB_ASH_OC(' ');
    LIB_ASH_OC('0');
    LIB_ASH_OC('x');
    LIB_ASH_OC('0');
    LIB_ASH_OC(s_ver[6]);
    LIB_ASH_OC('0');
    LIB_ASH_OC(s_ver[7]);
    LIB_ASH_OC(' ');
    LIB_ASH_OC('>');
    LIB_ASH_OC('0');
    LIB_ASH_OC('0');
    LIB_ASH_OC('0');
    LIB_ASH_OC('0');
    LIB_ASH_OC(' ');
    LIB_ASH_OC('@');
    LIB_ASH_OC(0x0a);
    LIB_ASH_OC(0x0d);
}
#endif


static int cmd_comtest(unsigned int argc, unsigned char *argv[])
{
    unsigned int data_len;
#ifdef UPGRADE_ONCE
    if(receive_comtest_falg == FALSE)
        return -1;
#endif

#ifdef PANEL_DISPLAY
#if (SYS_SDRAM_SIZE != 2)
    pan_display(panel_dev,  "conn", 4);
#else
    pan_display(panel_dev,  "UP9", 4);
#endif
#endif

    if (argc != 2)
    {
#ifdef PANEL_DISPLAY
        pan_display(panel_dev,  "r101", 4);
#endif
        SH_PRINTF("Usage: comtest <data_num>\n");
        SH_PRINTF("       Then begin loopback <data_num> charactor to host.\n");
        return -1;
    }

    data_len = ATOI(argv[1]);

    while (data_len--)
        LIB_ASH_OC(LIB_ASH_IC());

    return 0;
}


static int cmd_version(unsigned int argc, unsigned char *argv[])
{
    unsigned int i, j;
    unsigned long of;
    unsigned int n;
    unsigned int data_len;
    unsigned long crc;
    unsigned long chid;
    unsigned long len;
    unsigned long crc1;
    unsigned char buf[CHUNK_HEADER_SIZE];
    unsigned int n_version = 0xb722;

    //LIB_ASH_OC(argc & 0xFF);
    //LIB_ASH_OC(argv[1][0]);
    //LIB_ASH_OC(argv[1][1]);
    if( argc >= 2 && argv[1][0] == 0x2d && argv[1][1] == 0x72)//"-r"
    {
#ifdef TRANSFER_FORMAT2_SUPPORT
        LIB_ASH_OC((n_version >> 8) & 0xFF);
        LIB_ASH_OC( n_version & 0xFF);
#endif
    }


    /* collect information */
    n = sto_chunk_count(0, 0);
    of = sto_chunk_goto(&chid, 0, n);
    len = sto_fetch_long(of + CHUNK_LENGTH);

    /* send Length */
    data_len = n * CHUNK_HEADER_SIZE + 2;
    LIB_ASH_OC((data_len >> 8) & 0xFF);
    LIB_ASH_OC(data_len & 0xFF);

    crc = 0xFFFFFFFF;
    /* send Flash ID */
    LIB_ASH_OC(flash_id_val);
    crc = mg_table_driven_crc(crc, &flash_id_val, 1);
    /* send Chunk Status */
    LIB_ASH_OC(g_crc_bad);
    crc = mg_table_driven_crc(crc, &g_crc_bad, 1);
    /* send Chunks */
    for (i = 1; i <= n; i++)
    {
        of = sto_chunk_goto(&chid, 0, i);
        sto_get_data(flash_dev, buf, of, CHUNK_HEADER_SIZE);
        data_len = sto_fetch_long(of + CHUNK_LENGTH);
        crc1 = mg_table_driven_crc(0xFFFFFFFF, (unsigned char *) \
              SYS_FLASH_BASE_ADDR + of + 16, data_len);
        store_long(buf + CHUNK_CRC, crc1);
        for (j = 0; j < CHUNK_HEADER_SIZE; j++)
            LIB_ASH_OC(buf[j]);
        crc = mg_table_driven_crc(crc, buf, CHUNK_HEADER_SIZE);
    }
    /* send CRC */
    LIB_ASH_OC((crc >> 24) & 0xFF);
    LIB_ASH_OC((crc >> 16) & 0xFF);
    LIB_ASH_OC((crc >> 8) & 0xFF);
    LIB_ASH_OC(crc & 0xFF);

    /* flag for state invalid */
    start_addr = 0xFFFFFFFF;

    return 0;
}
#ifdef TRANSFER_FORMAT2_SUPPORT
static int cmd_dump(unsigned int argc, unsigned char *argv[])
{
    unsigned long crc;
    unsigned char *p = NULL;
    unsigned int i,j,k,cxx_cmd = 0;
    unsigned int page_num = 0;
    unsigned int len = 0;
    unsigned char blank_pck[8];
    unsigned char ack_buff[9],head, answer_pnum;
    int dump_start, dump_len;

    blank_pck[0] = 'B';
    blank_pck[1] = 'L';
    blank_pck[2] = 'A';
    blank_pck[3] = 'N';
    blank_pck[4] = 'K';
    blank_pck[5] = 'P';
    blank_pck[6] = 'C';
    blank_pck[7] = 'K';

#ifdef PANEL_DISPLAY
    pan_display(panel_dev, "dup ", 4);
#endif

   if (argc == 3)
    {
//#ifdef PANEL_DISPLAY
//        pan_display(panel_dev,  "err1", 4);
//#endif
//        SH_PRINTF("Usage: dump <start> <offset>\n");
        dump_start = ATOI(argv[1]);
        dump_len = ATOI(argv[2]);
    }
   else
   {
       dump_start = 0;
       dump_len = 0;
   }

   if( dump_start + dump_len > flash_dev->totol_size )
   {
#ifdef PANEL_DISPLAY
        pan_display(panel_dev,  "err2", 4);
#endif
        return -1;
   }

   if( dump_len == 0 )
   {
            /* send flash length*/
       dump_len = flash_dev->totol_size - dump_start;
        LIB_ASH_OC((dump_len >> 24) & 0xFF);
        LIB_ASH_OC((dump_len >> 16) & 0xFF);
        LIB_ASH_OC((dump_len >> 8) & 0xFF);
        LIB_ASH_OC(dump_len & 0xFF);
   }

   p = ((unsigned char *)FLASH_BASE) + dump_start;

    i = 0;
    while (i < dump_len)
    {
        if (sci_read_tm(boot_uart_id, &head, 500 * 1000) != SUCCESS)
        {
            continue;
        }
        if(head != 'H')
        {
            if (head == 'c')
            {
                cxx_cmd++;
                if (cxx_cmd > 4)
                    return -1;
            }
            else if (head != '\r')
            {
                cxx_cmd = 0;
            }
            continue;
        }

        len = PACKAGE_SIZE;
        if(dump_len - i < PACKAGE_SIZE)
            len = dump_len - i;

        /* send page number */
        LIB_ASH_OC((page_num >> 24) & 0xFF);
        LIB_ASH_OC((page_num >> 16) & 0xFF);
        LIB_ASH_OC((page_num >> 8) & 0xFF);
        LIB_ASH_OC(page_num & 0xFF);

        /* send page length */
        LIB_ASH_OC((len >> 24) & 0xFF);
        LIB_ASH_OC((len >> 16) & 0xFF);
        LIB_ASH_OC((len >> 8) & 0xFF);
        LIB_ASH_OC(len & 0xFF);

        crc = mg_table_driven_crc(0xFFFFFFFF, (unsigned char*)&page_num,4);
        LIB_ASH_OC((crc >> 24) & 0xFF);
            LIB_ASH_OC((crc >> 16) & 0xFF);
            LIB_ASH_OC((crc >> 8) & 0xFF);
            LIB_ASH_OC(crc & 0xFF);
        crc = mg_table_driven_crc(0xFFFFFFFF, (unsigned char*)&len,4);
        LIB_ASH_OC((crc >> 24) & 0xFF);
           LIB_ASH_OC((crc >> 16) & 0xFF);
            LIB_ASH_OC((crc >> 8) & 0xFF);
            LIB_ASH_OC(crc & 0xFF);
        if (sci_read_tm(boot_uart_id, &answer_pnum, 500 * 1000) != SUCCESS)
            {
                    continue;
            }
        if(answer_pnum != 'G')
        {
            continue;
        }


        crc = 0xFFFFFFFF;
        /* check if the packet is full of 0xFF,then send the blank packet to host */
        for(j=0;j<len;j++)
        {
            if(*((UINT8 *)(p+page_num*PACKAGE_SIZE+j)) != 0xFF)
                break;
        }
        if(j != len)
        {
            for(k=0;k<len;k++)
            {
                LIB_ASH_OC(*(p+page_num*1024+k));
                crc = mg_table_driven_crc(crc, p+page_num*1024+k,1);
            }
        }
        else/* send blank packet */
        {
            for(k=0;k<8;k++)
            {
                LIB_ASH_OC(*(blank_pck+k));
            }
            crc = mg_table_driven_crc(0xFFFFFFFF, blank_pck,8);

        }
        /* send CRC */
        LIB_ASH_OC((crc >> 24) & 0xFF);
        LIB_ASH_OC((crc >> 16) & 0xFF);
        LIB_ASH_OC((crc >> 8) & 0xFF);
        LIB_ASH_OC(crc & 0xFF);
        /* wait acknowledge from host */
        for(k=0;k<9;k++)
        {
            if(sci_read_tm(boot_uart_id,ack_buff+k,50*1000) != SUCCESS)
                break;
        }
        if(k == 9)
        {
            if(mg_table_driven_crc(0xFFFFFFFF, ack_buff,5) != fetch_long(ack_buff+5))
                continue;
            if(ack_buff[0] == 'E')
                continue;
            if(ack_buff[0] == 'O')
            {
                if(fetch_long(ack_buff+1) == page_num)
                    page_num++;
                else
                    continue;
            }
        }
        else
            continue;

#ifdef PANEL_DISPLAY
        if (i % PACKAGE_SIZE == 0)
        {
            led_show((unsigned char *)dup_str);
        }
#endif
        i += PACKAGE_SIZE;
    }

   return 0;
}
#else
#if (SYS_SDRAM_SIZE == 2)
static INT32 send_data( INT8 *buff, INT32 len )
{
    INT32 i;

    for( i=0; i<len; i++ )
    {
        sci_write(boot_uart_id,buff[i]);
    }

    return(i);
}


static int cmd_dump(unsigned int argc, unsigned char *argv[])
{
    unsigned long addr,len,l,crc;
    int i,ulen,mem_mode;
    unsigned int err_count;
    int last_error;
    unsigned char answer;
    char crc_buffer[4];
       char *data_buffer;
       data_buffer = (char *)IMAGE_START;

#ifdef PANEL_DISPLAY
    pan_display(panel_dev, "dup ", 4);
#endif

    if (argc != 4)
    {
        LIB_ASH_OC('E');
        return !SUCCESS;
    }

    LIB_ASH_OC('O');

    addr = ATOI(argv[1]);
    len = ATOI(argv[2]);
    mem_mode = ATOI(argv[3]);

    l = 0;
    err_count = 0;
    //soc_printf("addr = %x,len = %x\n",addr,len);
    while(l<len && err_count <10)
    {
        ulen = ((l+PACKAGE_SIZE) <= len)? PACKAGE_SIZE : (len - l);
        if((addr & 0xa0000000) == 0)
            addr = 0xa0000000;
        else
            addr |= 0x80000000;

        if(mem_mode == 1)
        {
            for(i = 0; i < ulen; i++)
                data_buffer[i] = *(unsigned char *)(addr + l + i);
        }
        else if(mem_mode == 2)
        {
            addr &= (~0x1);
            for(i = 0; i < ulen; i += 2)
                *((unsigned short *)(&(data_buffer[i]))) = *(unsigned short *)(addr + l + i);
        }
        else if(mem_mode == 4)
        {
            addr &= (~0x3);
            for(i = 0; i < ulen; i += 4)
                *((unsigned long *)(&(data_buffer[i]))) = *(unsigned long *)(addr + l + i);
        }

        crc = mg_table_driven_crc(0xFFFFFFFF, (UINT8 *)data_buffer, ulen);
        for(i = 0; i < 4; i++)
            crc_buffer[i] = (crc>>((3-i)*8))&0xFF;

        /* send one package*/
        if(ulen != send_data(data_buffer, ulen))
        {
            return !SUCCESS;
        }

        /* send 4 byte CRC*/
        if(4 != send_data(crc_buffer, 4))
        {
            return !SUCCESS;
        }
        //soc_printf("crc = %x\n",crc);
        //soc_printf("read snswer!\n");
        answer = 0;
        if(sci_read_tm(boot_uart_id,&answer,1000*500)!=SUCCESS)
        {
            return !SUCCESS;
        }
        if(answer == 'O')
        {
            l += ulen;
            err_count = 0;
        }
        else if(answer == 'E')
        {
            err_count++;
            continue;
        }
        else
        {
            //soc_printf("no echo!\n");
            return !SUCCESS;
        }
       }
    if(err_count>=10)
    {
        return !SUCCESS;
    }
    return SUCCESS;

}
#else
static int cmd_dump(unsigned int argc, unsigned char *argv[])
{
    unsigned long crc;
    unsigned char *p = (unsigned char *)FLASH_BASE;
    unsigned int i;

#ifdef PANEL_DISPLAY
    pan_display(panel_dev, "dup ", 4);
#endif

    /* send flash length*/
    LIB_ASH_OC((flash_dev->totol_size>> 24) & 0xFF);
    LIB_ASH_OC((flash_dev->totol_size >> 16) & 0xFF);
    LIB_ASH_OC((flash_dev->totol_size >> 8) & 0xFF);
    LIB_ASH_OC(flash_dev->totol_size & 0xFF);

    /* wait for host answer*/
    if (LIB_ASH_IC() != 'O')
    {
#ifdef PANEL_DISPLAY
        pan_display(panel_dev,  "r801", 4);
#endif
        return -1;
    }

    crc = 0xFFFFFFFF;
    i = 0;
    while (i < (UINT32)(flash_dev->totol_size))
    {
        LIB_ASH_OC(*p);
        crc = mg_table_driven_crc(crc, p++, 1);
#ifdef PANEL_DISPLAY
        if (i % PACKAGE_SIZE == 0)
        {
            led_show((unsigned char *)dup_str);
        }
#endif
        i++;
    }

    /* send CRC */
    LIB_ASH_OC((crc >> 24) & 0xFF);
    LIB_ASH_OC((crc >> 16) & 0xFF);
    LIB_ASH_OC((crc >> 8) & 0xFF);
    LIB_ASH_OC(crc & 0xFF);

    return 0;
}
#endif
#endif

static int cmd_address(unsigned int argc, unsigned char *argv[])
{
    unsigned int sector_no;
    unsigned long temp_start;
    unsigned int len;

#if (SYS_SDRAM_SIZE == 2)
    if (argc < 2)
#else
    if (argc != 2)
#endif
    {
#ifdef PANEL_DISPLAY
        pan_display(panel_dev,  "r301", 4);
#endif
        SH_PRINTF("Usage: address <start_address>\n");
        return -1;
    }

    start_addr = ATOI(argv[1]);
    image_free = (unsigned char *)IMAGE_START;
#if (SYS_SDRAM_SIZE == 2)
    transfer_size = 0;
    burn_size = 0;
    total_transfer_size = 0;
    total_burn_size = 0;
    if (4 == argc)
    {
        total_transfer_size = ATOI(argv[2]);
        total_burn_size = ATOI(argv[3]);
    }
#endif

    sector_no = flash_sector_align(start_addr);
    if (sector_no >= flash_sectors_val)
    {
#ifdef PANEL_DISPLAY
        pan_display(panel_dev,  "r302", 4);
#endif
        SH_PRINTF("Error: Address greater than flash size. (Flash ID: %02X)\n", flash_id_val);
        LIB_ASH_OC('E');
        return -1;
    }

    temp_start = flash_sector_start(sector_no);
    if (temp_start < start_addr)
    {
        len = start_addr - temp_start;
        sto_get_data(flash_dev, image_free, temp_start , len);
        image_free += len;
        start_addr = temp_start;
    }

    LIB_ASH_OC('O');
    return 0;
}


static int cmd_move(unsigned int argc, unsigned char *argv[])
{
    unsigned long of;
    unsigned long chid;
    unsigned long len;
    unsigned long offset;

    if (argc != 3)
    {
#ifdef PANEL_DISPLAY
        pan_display(panel_dev,  "r401", 4);
#endif
        SH_PRINTF("Usage: move <id> <offset>\n");
        return -1;
    }
    if (start_addr == 0xFFFFFFFF)
    {
#ifdef PANEL_DISPLAY
        pan_display(panel_dev,  "r402", 4);
#endif
        SH_PRINTF("Error: Start address hasn't been initialized.\n");
        LIB_ASH_OC('E');
        return -1;
    }

    chid = ATOI(argv[1]);
    offset = ATOI(argv[2]);

    of = sto_chunk_goto(&chid, 0xFFFFFFFF, 1);
    if (of & 0x80000000)
    {
#ifdef PANEL_DISPLAY
        pan_display(panel_dev,  "r403", 4);
#endif
        SH_PRINTF("Error: Can not found specified Chunk ID. (ID: %08X)\n", chid);
        LIB_ASH_OC('E');
        return -1;
    }

    if (offset)
        len = offset;
    else
        len = sto_fetch_long(of + CHUNK_LENGTH) + 16;
    sto_get_data(flash_dev, image_free, of, len);
    store_long(image_free + CHUNK_OFFSET, offset);
    image_free += len;

    LIB_ASH_OC('O');
    return 0;
}


#ifdef TRANSFER_FORMAT2_SUPPORT
static int cmd_transfer(unsigned int argc, unsigned char *argv[])
{
    unsigned int i;
    unsigned int data_len;
    unsigned long crc;
    unsigned long len;
    unsigned long l;
    unsigned long offset;
    unsigned long package_num;
    unsigned char *p;
    unsigned char info[9];
    unsigned char err_state;
    unsigned char cxx_cmd;
#if (SYS_SDRAM_SIZE == 2)
    unsigned char msg[10];
    unsigned long progress;
#endif

    if (argc != 2)
    {
#ifdef PANEL_DISPLAY
        pan_display(panel_dev,  "r501", 4);
#endif
        SH_PRINTF("Usage: transfer <data_length>\n");
        return -1;
    }
    if (start_addr == 0xFFFFFFFF)
    {
#ifdef PANEL_DISPLAY
        pan_display(panel_dev,  "r502", 4);
#endif
        SH_PRINTF("Error: Start address hasn't been initialized.\n");
        LIB_ASH_OC('E');
        return -1;
    }

#ifdef PANEL_DISPLAY
#if (SYS_SDRAM_SIZE != 2)
    pan_display(panel_dev, "trn ", 4);
#endif
    led_display_type = 0;
#endif

    len = ATOI(argv[1]);

    cxx_cmd = 0;

    data_len = 0;
    while (data_len < len)
    {
        /* get header char as sub-command */
        info[0] = 0;
        if (sci_read_tm(boot_uart_id, info, 500 * 1000) != SUCCESS)
        {
            continue;
        }
        if (info[0] == 'H')
        {
            /* transfer head information */
            for (i = 0; i < 8; i++)
            {
                if (sci_read_tm(boot_uart_id, info + 1 + i, 500 * 1000) != SUCCESS)
                {
                    break;
                }
            }
            if (i != 8)
            {
                continue;
            }
            if (mg_table_driven_crc(0xFFFFFFFF, info + 1, 4) != \
                  fetch_long(info + 5))
            {
                continue;
            }
            package_num = fetch_long(info + 1);
            /* acknowledge for got head information */
            LIB_ASH_OC('C');
        }
        else
        {
            if (info[0] == 'c')
            {
                cxx_cmd++;
                if (cxx_cmd > 7)
                {
                    SH_PRINTF("Error: Might in comtest.\n");
                    LIB_ASH_OC('E');
                    return -1;
                }
            }
            else if (info[0] != '\r')
            {
                cxx_cmd = 0;
            }

            /* ignore */
            continue;
        }

        p = image_free + package_num * PACKAGE_SIZE;
        l = len - data_len;
        if (l > PACKAGE_SIZE)
            l = PACKAGE_SIZE;
        /* transfer one data block + CRC */
        for (i = 0; i < l + 4; i++)
        {
            if (sci_read_tm(boot_uart_id, p + i, 500 * 1000) != SUCCESS)
            {
                err_state = 1;
                break;
            }
        }
        if (i == l + 4)
        {
            err_state = mg_table_driven_crc(0xFFFFFFFF, p, l) != \
                  fetch_long(p + l);
#ifdef PANEL_DISPLAY
#if (SYS_SDRAM_SIZE == 2)
            transfer_size += l;
            if (total_transfer_size > 0)
            {
                progress = transfer_size*100/total_transfer_size;
                if (progress < 100)
                    sprintf(msg,"d%.2d",progress);
                else
                    sprintf(msg,"100 ");

                pan_display(panel_dev,  msg, 4);
            }
            else
                led_show((unsigned char *)down_str);
#else
            led_show((unsigned char *)down_str);
#endif
#endif
        }

        if (err_state == 0)
        {
            data_len = p + l - image_free;
            /* Acknowledge for packet OK */
            info[0] = 'O';
            store_long(info + 1, package_num);
            store_long(info + 5, mg_table_driven_crc(0xFFFFFFFF, \
                  info + 1, 4));
            for (i = 0; i < 9; i++)
            {
                LIB_ASH_OC(info[i]);
            }
        }
        else
        {
            /* Acknowledge for packet Error */
            LIB_ASH_OC('E');
        }
    }

    if (data_len == len)
    /* transfer succeed */
    {
        p += l;
        offset = fetch_long(image_free + CHUNK_OFFSET);
        if (offset == 0)
            offset = fetch_long(image_free + CHUNK_LENGTH) + 16;
        /* if offset > transfer length, fill gap with old data */
        if (offset > len)
        {
            l = offset - len;
            i = p - (unsigned char *)IMAGE_START + start_addr;
            sto_get_data(flash_dev, p, i, l);
        }
        image_free += offset;
    }

    return 0;
}

#else
#define     UNZIP_IN_BUF                0xa0100000//0xa0000200
#define     UNZIP_TEMP_BUF              0xa01a0000
#define     TRANSER_RAW_DATA            0
#define     TRANSER_ZIP_SECTION_DATA    1
#define     TRANSER_ZIP_ALL_DATA        2
// command format: 1. transfer raw_length (transfer raw data.)
//                 2. transfer C|A zip_length raw_length (transfer compress zip code data)
//               C - zip compress data; A - transfer all compress firmware data one time.
//               zip_length - zip compress data length
//               raw_length -  the raw data length
static int cmd_transfer(unsigned int argc, unsigned char *argv[])
{
    unsigned int i;
    unsigned long l;
    unsigned char *p;
    unsigned int data_len;
    unsigned int err_count;
    unsigned int last_error;
    unsigned long crc;
    unsigned long len;
    unsigned long raw_len=0;
    unsigned long offset;
    unsigned char c_transfer_type;
#if (SYS_SDRAM_SIZE == 2)
    unsigned char msg[10];
    unsigned long progress;
#endif

#if (SYS_SDRAM_SIZE == 2)
    if (argc < 2)
#else
    if (argc != 2)
#endif
    {
#ifdef PANEL_DISPLAY
        pan_display(panel_dev,  "r501", 4);
#endif
        SH_PRINTF("Usage: transfer <data_length>\n");
        return -1;
    }

    c_transfer_type = TRANSER_RAW_DATA;
    if (argc == 2)
    {
        len = ATOI(argv[1]);
    }
    else if (argc == 4)
    {
        if (argv[1][0] == 'C')
            c_transfer_type = TRANSER_ZIP_SECTION_DATA;
        else if (argv[1][0] == 'A')
            c_transfer_type = TRANSER_ZIP_ALL_DATA;
        else
            return -1;
        len = ATOI(argv[2]);
        raw_len = ATOI(argv[3]);
    }
    else
        return -1;

    if (start_addr == 0xFFFFFFFF)
    {
#ifdef PANEL_DISPLAY
        pan_display(panel_dev,  "r502", 4);
#endif
        SH_PRINTF("Error: Start address hasn't been initialized.\n");
        LIB_ASH_OC('E');
        return -1;
    }

#ifdef PANEL_DISPLAY
#if (SYS_SDRAM_SIZE != 2)
    pan_display(panel_dev, "trn ", 4);
#endif
    led_display_type = 0;
#endif

    data_len = 0;
    last_error = 0;
    err_count = 0;

    if (c_transfer_type != TRANSER_RAW_DATA)
        p = (unsigned char*)(UNZIP_IN_BUF);
    else
        p = image_free;
    while (data_len < len&& err_count < TRANSFER_RETRY_TIMES)
    {
        l = len - data_len;
        if (l > PACKAGE_SIZE)
            l = PACKAGE_SIZE;
        /* Acknowledge for next data block */
        LIB_ASH_OC(last_error ? 'E' : 'O');
        /* transfer one data block + CRC */
        for (i = 0; i < l + 4; i++)
        {
            if (sci_read_tm(boot_uart_id, p + i, 1500 * 1000) != SUCCESS)
            {
                err_count++;
                break;
            }
        }
        if (i == l + 4)
        {
            crc = fetch_long(p + l);
            last_error = (crc != mg_table_driven_crc(0xFFFFFFFF, \
                  p, l));
            if (!last_error)
            {
                p += l;
                data_len += l;
                err_count = 0;
            #if (SYS_SDRAM_SIZE == 2)
                transfer_size += l;
            #endif
            }
    #ifdef PANEL_DISPLAY
        #if (SYS_SDRAM_SIZE == 2)
            if (total_transfer_size > 0)
            {
                if (c_transfer_type != TRANSER_RAW_DATA)
                    progress = transfer_size*100/len;
                else
                    progress = transfer_size*100/total_transfer_size;
                if (progress < 100)
                    sprintf(msg,"d%.2d",progress);
                else
                    sprintf(msg,"100 ");

                pan_display(panel_dev,  msg, 4);
            }
            else
                led_show((unsigned char *)down_str);
        #else
            led_show((unsigned char *)down_str);
        #endif
    #endif
        }
        else
            last_error = 1;
    }

    if (c_transfer_type != TRANSER_RAW_DATA)
    {
        int ret;
        ret = un7zip((unsigned char *)UNZIP_IN_BUF, image_free, (unsigned char *)UNZIP_TEMP_BUF);
        if (ret != 0)
        {
            LIB_ASH_OC('E');
            return -1;
        }
    }

    if (data_len == len)
    /* transfer succeed */
    {
        offset = fetch_long(image_free + CHUNK_OFFSET);
        if (offset == 0)
            offset = fetch_long(image_free + CHUNK_LENGTH) + 16;
        /* if offset > transfer length, fill gap with old data */
        if (c_transfer_type != TRANSER_RAW_DATA)
        {
            if ((TRANSER_ZIP_SECTION_DATA == c_transfer_type) && (offset > raw_len))
            {
                l = offset - raw_len;
                i = image_free+raw_len - (unsigned char *)IMAGE_START + start_addr;
                sto_get_data(flash_dev, p, i, l);
            }
        }
        else
        {
            if (offset > len)
            {
                l = offset - len;
                i = p - (unsigned char *)IMAGE_START + start_addr;
                sto_get_data(flash_dev, p, i, l);
            }
        }
        if (TRANSER_ZIP_ALL_DATA == c_transfer_type)
            image_free += raw_len;
        else
            image_free += offset;
    }

    LIB_ASH_OC('O');
    return 0;
}
#endif

static int cmd_set(unsigned int argc, unsigned char *argv[])
{
    unsigned long len;
    char c;

     if (argc != 3)
    {
#ifdef PANEL_DISPLAY
        pan_display(panel_dev,  "r901", 4);
#endif
        SH_PRINTF("Usage: set <char> <length>\n");
        return -1;
    }

     if (start_addr == 0xFFFFFFFF)
    {
#ifdef PANEL_DISPLAY
        pan_display(panel_dev,  "r902", 4);
#endif
        SH_PRINTF("Error: Start address hasn't been initialized.\n");
        LIB_ASH_OC('E');
        return -1;
    }

    c =  *(argv[1]);
    len = ATOI(argv[2]);

    MEMSET(image_free, c, len);

    image_free += len;

    LIB_ASH_OC('O');

}

#ifdef DTTM
#define SVIZ_X_SOH            0x01
#define SVIZ_X_STX            0x02
#define SVIZ_X_EOT            0x04
#define SVIZ_X_ACK            0x06
#define SVIZ_X_NAK            0x15
#define SVIZ_X_CAN            0x18
#define SVIZ_X_C                0x43
#define SVIZ_X_BLOCK_SIZE    0x400//1024 bytes
#define SVIZ_X_MAX_RETRIES    0x03


const static UINT16 crc_table[256] = { /* As generated */
    0x0000,  0x1021,  0x2042,  0x3063,  0x4084,  0x50a5,  0x60c6,  0x70e7,
    0x8108,  0x9129,  0xa14a,  0xb16b,  0xc18c,  0xd1ad,  0xe1ce,  0xf1ef,
    0x1231,  0x0210,  0x3273,  0x2252,  0x52b5,  0x4294,  0x72f7,  0x62d6,
    0x9339,  0x8318,  0xb37b,  0xa35a,  0xd3bd,  0xc39c,  0xf3ff,  0xe3de,
    0x2462,  0x3443,  0x0420,  0x1401,  0x64e6,  0x74c7,  0x44a4,  0x5485,
    0xa56a,  0xb54b,  0x8528,  0x9509,  0xe5ee,  0xf5cf,  0xc5ac,  0xd58d,
    0x3653,  0x2672,  0x1611,  0x0630,  0x76d7,  0x66f6,  0x5695,  0x46b4,
    0xb75b,  0xa77a,  0x9719,  0x8738,  0xf7df,  0xe7fe,  0xd79d,  0xc7bc,
    0x48c4,  0x58e5,  0x6886,  0x78a7,  0x0840,  0x1861,  0x2802,  0x3823,
    0xc9cc,  0xd9ed,  0xe98e,  0xf9af,  0x8948,  0x9969,  0xa90a,  0xb92b,
    0x5af5,  0x4ad4,  0x7ab7,  0x6a96,  0x1a71,  0x0a50,  0x3a33,  0x2a12,
    0xdbfd,  0xcbdc,  0xfbbf,  0xeb9e,  0x9b79,  0x8b58,  0xbb3b,  0xab1a,
    0x6ca6,  0x7c87,  0x4ce4,  0x5cc5,  0x2c22,  0x3c03,  0x0c60,  0x1c41,
    0xedae,  0xfd8f,  0xcdec,  0xddcd,  0xad2a,  0xbd0b,  0x8d68,  0x9d49,
    0x7e97,  0x6eb6,  0x5ed5,  0x4ef4,  0x3e13,  0x2e32,  0x1e51,  0x0e70,
    0xff9f,  0xefbe,  0xdfdd,  0xcffc,  0xbf1b,  0xaf3a,  0x9f59,  0x8f78,
    0x9188,  0x81a9,  0xb1ca,  0xa1eb,  0xd10c,  0xc12d,  0xf14e,  0xe16f,
    0x1080,  0x00a1,  0x30c2,  0x20e3,  0x5004,  0x4025,  0x7046,  0x6067,
    0x83b9,  0x9398,  0xa3fb,  0xb3da,  0xc33d,  0xd31c,  0xe37f,  0xf35e,
    0x02b1,  0x1290,  0x22f3,  0x32d2,  0x4235,  0x5214,  0x6277,  0x7256,
    0xb5ea,  0xa5cb,  0x95a8,  0x8589,  0xf56e,  0xe54f,  0xd52c,  0xc50d,
    0x34e2,  0x24c3,  0x14a0,  0x0481,  0x7466,  0x6447,  0x5424,  0x4405,
    0xa7db,  0xb7fa,  0x8799,  0x97b8,  0xe75f,  0xf77e,  0xc71d,  0xd73c,
    0x26d3,  0x36f2,  0x0691,  0x16b0,  0x6657,  0x7676,  0x4615,  0x5634,
    0xd94c,  0xc96d,  0xf90e,  0xe92f,  0x99c8,  0x89e9,  0xb98a,  0xa9ab,
    0x5844,  0x4865,  0x7806,  0x6827,  0x18c0,  0x08e1,  0x3882,  0x28a3,
    0xcb7d,  0xdb5c,  0xeb3f,  0xfb1e,  0x8bf9,  0x9bd8,  0xabbb,  0xbb9a,
    0x4a75,  0x5a54,  0x6a37,  0x7a16,  0x0af1,  0x1ad0,  0x2ab3,  0x3a92,
    0xfd2e,  0xed0f,  0xdd6c,  0xcd4d,  0xbdaa,  0xad8b,  0x9de8,  0x8dc9,
    0x7c26,  0x6c07,  0x5c64,  0x4c45,  0x3ca2,  0x2c83,  0x1ce0,  0x0cc1,
    0xef1f,  0xff3e,  0xcf5d,  0xdf7c,  0xaf9b,  0xbfba,  0x8fd9,  0x9ff8,
    0x6e17,  0x7e36,  0x4e55,  0x5e74,  0x2e93,  0x3eb2,  0x0ed1,  0x1ef0
};

UINT16 gw_msg_checksum(
  UINT8 *    p_msg_data,
  UINT16    msg_length )
{
  UINT16 crc_check = 0;

//  GSL_Dprintf2(COMPID, GSL_MASK_FREE2, "", "_Gw_MsgChecksum - Entry");

  while (msg_length > 0)
  {
    crc_check = (UINT16)((crc_check<<8) ^ crc_table[(UINT16)((UINT32)crc_check>>8) ^ *p_msg_data++]);
    msg_length--;
  }
  //GSL_Dprintf2(COMPID, GSL_MASK_FREE2, "", "_Gw_MsgChecksum - Exit");

  return crc_check;
}


void cmd_dttm_transferraw(void)
{
    unsigned int i;
    unsigned long l;
    unsigned int data_len;
    unsigned int err_count;
    unsigned int last_error;
    UINT16 crc;
    unsigned long len;
    unsigned long offset;
    UINT32 block_number = 0;
    UINT8 ch;
    unsigned char *p;

#if 0
    len = ATOI(argv[1]);
#else
    len = 0x140000;
#endif

soc_printf("cmd_transferraw, len = %x\n",len);
    data_len = 0;
    last_error = 0;
    err_count = 0;

    p = (unsigned char *)(0x80200000);//*(volatile UINT8 *)(0x80200000);//image_free;

    while (1)
    {
        l = PACKAGE_SIZE;
        /* transfer one data block + CRC */
        if (sci_read_tm(boot_uart_id, &ch, 1500 * 1000) != SUCCESS)
            continue;
        //soc_printf("ch1 = %x\n",ch);
        if (SVIZ_X_STX == ch)
        {
                for (i = 0; i < l + 4; i++)
                {
                    if (i == 0 || i == 1)
                    {
                        if (sci_read_tm(boot_uart_id, &ch, 1500 * 1000) != SUCCESS)
                        {
                        //soc_printf("not success i = %x\n",i);
                            err_count++;
                            break;
                        }
                    }
                    else if (sci_read_tm(boot_uart_id, p + i - 2, 1500 * 1000) != SUCCESS)
                    {
                        soc_printf("not success i = %x\n",i);
                        err_count++;
                        break;
                    }
                }
                //soc_printf("end of trans i = %x l = %x\n",i,l);
                if (i == l + 4)
                {
                    //soc_printf("lenght is right\n");
                    //crc = fetch_long(p + l);
                    //last_error = (crc != MG_Table_Driven_CRC(0xFFFFFFFF, p, l));
                    crc = gw_msg_checksum(p,1024);
                    //soc_printf("crc = %x %x\n",(crc >> 8),(crc & 0xff));
                    if(((crc >> 8) != *(p + l)) || ((crc & 0xff) != *(p + l +1)))
                    {
                        last_error = 1;
                    }
                    else
                    {
                        last_error = 0;
                    }
                    //soc_printf("last_error = %x\n",last_error);
                    if (!last_error)
                    {
                        p += l;
                        err_count = 0;
                        /* Acknowledge for next data block */
                        soc_printf("ack\n");
                        LIB_ASH_OC(SVIZ_X_ACK);
                    }
                    else
                    {
                        soc_printf("1nak\n");
                        LIB_ASH_OC(SVIZ_X_NAK);
                    }
                }
                else
                {
                    /* Acknowledge for next data block */
                    soc_printf("2nak\n");
                    LIB_ASH_OC(SVIZ_X_NAK);
                    last_error = 1;
                }
        }
        else if (SVIZ_X_EOT == ch)
        {
            soc_printf("SVIZ_X_EOT ack\n");
            LIB_ASH_OC(SVIZ_X_ACK);
            break;
        }
    }
    soc_printf("transfer finish\n");
}
#endif

#define RETURNCODE(code) { \
    if(buffer != NULL) FREE(buffer);    \
    return code;                \
}
static int cmd_transferraw(unsigned int argc, unsigned char *argv[])
{
    unsigned int i;
    unsigned long l;
    unsigned char *p;
    unsigned char *pos = NULL;
    unsigned int data_len;
    unsigned int err_count;
    unsigned int last_error;
    unsigned long crc;
    unsigned long len;
    unsigned long offset;
    unsigned int zip = 0;
    unsigned int real_len = 0;
    UINT8 *buffer =  NULL;

    if (argc == 2)
    {
        len = ATOI(argv[1]);
        p = image_free;
    }
    else if(argc == 4 && strcmp(argv[1], "-z") == 0)
    {
        real_len = ATOI(argv[2]);
        len = ATOI(argv[3]);
        zip = 1;

        p = (unsigned char *)ZIP_BUFFER;
        pos = p;

        buffer = MALLOC(BUFFER_SIZE);
          if (buffer == NULL)
          {
              SH_PRINTF("Error: No Buffer to compress tranfer data.\n");
              LIB_ASH_OC('E');
              RETURNCODE(-1);
          }

    }
    else
    {
#ifdef PANEL_DISPLAY
        pan_display(panel_dev,  "r501", 4);
#endif
        SH_PRINTF("Usage: transfer <data_length>\n");
        RETURNCODE(-1);
    }

    if (start_addr == 0xFFFFFFFF)
    {
#ifdef PANEL_DISPLAY
        pan_display(panel_dev,  "r502", 4);
#endif
        SH_PRINTF("Error: Start address hasn't been initialized.\n");
        LIB_ASH_OC('E');
        RETURNCODE(-1);
    }
#ifdef PANEL_DISPLAY
    pan_display(panel_dev, "trn ", 4);
    led_display_type = 0;
#endif

    data_len = 0;
    last_error = 0;
    err_count = 0;
    // read transfer data
    while (data_len < len && err_count < TRANSFER_RETRY_TIMES)
    {
        l = len - data_len;
        if (l > PACKAGE_SIZE)
            l = PACKAGE_SIZE;
        /* Acknowledge for next data block */
        LIB_ASH_OC(last_error ? 'E' : 'O');
        /* transfer one data block + CRC */
        for (i = 0; i < l + 4; i++)
        {
            if (sci_read_tm(boot_uart_id, p + i, 1500 * 1000) != SUCCESS)
            {
                err_count++;
                break;
            }
        }
        if (i == l + 4)
        {
            crc = fetch_long(p + l);
            last_error = (crc != mg_table_driven_crc(0xFFFFFFFF, \
                  p, l));
            if (!last_error)
            {
                p += l;
                data_len += l;
                err_count = 0;
            }
#ifdef PANEL_DISPLAY
            led_show((unsigned char *)down_str);
#endif
        }
        else
            last_error = 1;
    }

    if (data_len == len)
    /* transfer succeed */
    {
        if(zip == 0)
            image_free += len;
        else  //unzip
        {

            if(un7zip((UINT8 *)pos, (UINT8 *)image_free, buffer) == 0)
                image_free += real_len;
            else
            {
                SH_PRINTF("Error: Compress tranfer data error.\n");
                LIB_ASH_OC('E');
                RETURNCODE(-1);
            }
        }
    }
    LIB_ASH_OC('O');
    RETURNCODE(0);
}

#ifdef DTTM
void cmd_dttm_burn(void)
{
    unsigned char *p;
    unsigned int sector_no;
    unsigned long temp_start;
    unsigned long temp_end;
    unsigned int len;
    unsigned int ret;
    unsigned char strbuf[16];


    /* Begin burn: Erase sector */
    temp_end = 0x150000;

    p = (unsigned char *)IMAGE_START + 26 + 0x10000;

    sector_no = flash_sector_align(0x10000);

    temp_start = 0x10000;

    while (temp_start < temp_end)
    {
        len =0x10000;
        switch (flash_verify(temp_start, p, len))
        {
        case 2:
            flash_erase_sector(temp_start);
        case 1:
            ret = flash_copy(temp_start, p, len);
            if (ret == 0)
                ret = flash_verify(temp_start, p, len);

        }
        p += len;
        temp_start += len;
    }

#ifdef WATCH_DOG_SUPPORT
    dog_stop(0);
#endif
    sys_watchdog_reboot();
}
#endif

static int cmd_burn(unsigned int argc, unsigned char *argv[])
{
#ifdef DEBUG
    unsigned int i;
#endif
    unsigned char *p;
    unsigned int sector_no;
    unsigned long temp_start;
    unsigned long temp_end;
    unsigned int len;
    unsigned int ret;
    unsigned char strbuf[16];

    if (start_addr == 0xFFFFFFFF || image_free == \
          (unsigned char *)IMAGE_START)
    {
#ifdef PANEL_DISPLAY
        pan_display(panel_dev,  "r601", 4);
#endif
        SH_PRINTF("Error: Nothing to burn.\n");
        LIB_ASH_OC('E');
        return -1;
    }

    LIB_ASH_OC('O');
#ifdef PANEL_DISPLAY
    pan_display(panel_dev, "bun ", 4);
    led_display_type = 0;
#endif

#ifdef HDCP_IN_FLASH
    // Check burn data include Bootloader sector or not.
    if( start_addr  < 0x10000 )
    {
        UINT8     *hdcp_internal_keys;
        CHUNK_HEADER hdcp_chuck_hdr;
        UINT32 hdcp_chunk_offset_addr;
        UINT32 hdcp_chuck_id = HDCPKEY_CHUNK_ID;
        if(sto_get_chunk_header(hdcp_chuck_id,&hdcp_chuck_hdr)!=0)
        {
            // read back the hdcp key from flash
            hdcp_internal_keys = (UINT8 *)MALLOC(sizeof(UINT8)*(hdcp_chuck_hdr.offset));
            hdcp_chunk_offset_addr = sto_chunk_goto(&hdcp_chuck_id,0xFFFFFFFF,1);
            sto_get_data( (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0), hdcp_internal_keys,
                                            hdcp_chunk_offset_addr, hdcp_chuck_hdr.offset);
            // restore back the hdcp key to all code image
            MEMCPY(IMAGE_START +hdcp_chunk_offset_addr,hdcp_internal_keys,hdcp_chuck_hdr.offset);
            FREE(hdcp_internal_keys);
        }
    }
#endif
    /* Begin burn: Erase sector */
    temp_end = image_free - (unsigned char *)IMAGE_START + \
          start_addr;
    if (temp_end < (UINT32)(flash_dev->totol_size))
    {
        sector_no = flash_sector_align(temp_end);
        temp_start = flash_sector_start(sector_no);
        if (temp_start < temp_end)
        {
            len = temp_start + flash_sector_size(sector_no) - \
                  temp_end;
            sto_get_data(flash_dev, image_free, temp_end, len);
            temp_end += len;
        }
    }
    else
        temp_end = flash_dev->totol_size;

    p = (unsigned char *)IMAGE_START;
    sector_no = flash_sector_align(start_addr);
    temp_start = start_addr;
    while (temp_start < temp_end)
    {
        len = flash_sector_size(sector_no++);
        switch (flash_verify(temp_start, p, len))
        {
        case 2:
            flash_erase_sector(temp_start);
        case 1:
            ret = flash_copy(temp_start, p, len);
            if (ret == 0)
                ret = flash_verify(temp_start, p, len);
            if (ret == 0)
                sprintf(strbuf, "B%04dK\n", temp_start >> 10);
            else
                sprintf(strbuf, "E%07d\n", temp_start);
            dump_str(strbuf);
#ifdef PANEL_DISPLAY
#if (SYS_SDRAM_SIZE == 2)
            unsigned long progress;
            if (total_burn_size > 0)
            {
                burn_size = temp_start;
                progress = burn_size*100/total_burn_size;
                if (progress < 100)
                    sprintf(strbuf,"b%.2d",progress);
                else
                    sprintf(strbuf,"100 ");
                pan_display(panel_dev,  strbuf, 4);
            }
            else
                led_show((unsigned char *)burn_str);
#else
            led_show((unsigned char *)burn_str);
#endif
#endif
        }
        p += len;
        temp_start += len;
    }

#ifdef DEBUG
    for (i = 0; i < flash_sectors_val; i++)
        dump(FLASH_OFF2PTR(flash_sector_start(i)), 16);
#endif
    LIB_ASH_OC('F');

#ifdef PANEL_DISPLAY
#if (SYS_SDRAM_SIZE != 2)
    pan_display(panel_dev, "end-", 4);
#endif
#endif
#ifdef UPGRADE_ONCE
    receive_comtest_falg = FALSE;
#endif

#if (SYS_SDRAM_SIZE == 2)
    sys_watchdog_reboot();
#endif
    return 0;
}


static int cmd_reboot(unsigned int argc, unsigned char *argv[])
{
    LIB_ASH_OC('O');
#ifdef PANEL_DISPLAY
    pan_display(panel_dev, "off", 4);
#endif
    sys_ic_exit_standby();
#ifdef PANEL_DISPLAY
    pan_close((struct pan_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_PAN));
#endif
#ifdef WATCH_DOG_SUPPORT
    dog_stop(0);
#endif
    sys_watchdog_reboot();
    return 0;
}
#endif

