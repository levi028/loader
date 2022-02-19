/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: ui_debug.c
*
*    Description:
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/

#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>


#define COMMEMCOPY  ali_memcpy
#define COMMEMCMP   ali_memcmp
#define COMMEMMOVE  ali_memmove
#define COMSTRCOPY  ali_strcpy
#define COMSTRCMP(cmp1, cmp2)   (!ali_strcmp((UINT8 *)(cmp1), (UINT8 *)(cmp2)))
#define COMSTRLEN   ali_strlen
#define COMMEMSET   ali_memset
#define COMMEMCOPY3 ali_memcpy
#define COMSTRCAT    strcat

#define UDG_PRINTF  libc_printf
#define DBG_DUMP_DATA               0
#define DBG_PRINT_WSTRING           0
#define DBG_DUMP_BITMAP             0
#define DBG_TRACE_EIT_EVENT         0
#ifdef DISABLE_PRINTF
#define DBG_PRINT_MEMORY_LAYOUT     0
#else
#define DBG_PRINT_MEMORY_LAYOUT     1
#endif
#define DBG_COUNTRY_BAND            0
#define DBG_WATCH_POINT             0

#define PRINT_NUM_ITEM  2
#define CONTENT_RESERVED_LEN    12
#define REPLACE_START_POS       (CONTENT_RESERVED_LEN-1)
#ifndef __MM_HIGHEST_ADDR
#define __MM_HIGHEST_ADDR       0xa8000000
#define __MM_VOID_BUFFER_ADDR   __MM_HIGHEST_ADDR - 0x100000
#endif
#define VOID_LEN (__MM_HIGHEST_ADDR-__MM_VOID_BUFFER_ADDR)
#define MAX_REGION      20
#define MAX_BAND_NUMBER 10

#define MIN_UNIT_BITMAP_WIDTH   4
#define LENGTH_O_ECIMAL      10
#ifdef CAS9_VSC
#define SEE_CODE_SIZE    0x1C0000
#define __MM_SEE_CODE_START_ADDR (__MM_VSC_DATA_START_ADDR+__MM_VSC_DATA_LEN)
#define __MM_SEE_HEAP_START_ADDR (__MM_SEE_CODE_START_ADDR+SEE_CODE_SIZE)
#endif

#if DBG_DUMP_DATA
void dump_data(BYTE *lp_buf, DWORD length, char *name)
{
    int i, j = 0, k;
    char label[9], ch;
    for(i=0; name && name[i] && i<8; i++)
        label[i] = name[i];
    for(; i<8; i++)
        label[i] = ' ';
    label[8] = 0;
    UDG_PRINTF("\n %s |  0  1  2  3  4  5  6  7   8  9  A  B  C  D  E  F |\n", label);
    UDG_PRINTF("----------|--------------------------------------------------|------------------\n");
//    UDG_PRINTF(" 00000000 | 00 11 22 33 44 55 66 77  88 99 AA BB CC DD EE FF | 123456789ABCDEF\n");
    for(i=0; i<length/LENGTH_O_HEX; i++)
    {
        j = i*LENGTH_O_HEX;
//        UDG_PRINTF(" %08x | ", j);
//        for(k=0; k<8; k++)
//            UDG_PRINTF("%02x ", lpBuf[j+k]);
//        for(; k<16; k++)
//            UDG_PRINTF(" %02x", lpBuf[j+k]);
//        UDG_PRINTF(" | ");
        UDG_PRINTF(" %08x | %02x %02x %02x %02x %02x %02x %02x %02x  %02x %02x %02x %02x %02x %02x %02x %02x | ",
                    (j), lp_buf[j], lp_buf[j+1], lp_buf[j+2], lp_buf[j+3],
                         lp_buf[j+4], lp_buf[j+5], lp_buf[j+6], lp_buf[j+7],
                         lp_buf[j+8], lp_buf[j+9], lp_buf[j+10], lp_buf[j+11],
                         lp_buf[j+12], lp_buf[j+13], lp_buf[j+14], lp_buf[j+15]);
        for(k=0; k<LENGTH_O_HEX; k++, j++)
        {
            if((BYTE)lp_buf[j] < 0x20 || (BYTE)lp_buf[j] > 0x7E)
                ch = '.';
            else
                ch = lp_buf[j];
            UDG_PRINTF("%c", ch);
        }
        UDG_PRINTF("\n");
    }
    if(length%LENGTH_O_HEX)
    {
        UDG_PRINTF(" %08x |", j);
        for(i=0; i<length%LENGTH_O_HEX && i<LENGTH_O_HEX_HALF; i++)
            UDG_PRINTF(" %02x", lp_buf[j+i]);
        UDG_PRINTF(" ");
        if(length%LENGTH_O_HEX >= LENGTH_O_HEX_HALF)
        {
            for(;i<length%LENGTH_O_HEX; i++)
                UDG_PRINTF(" %02x", lp_buf[j+i]);
        }
        for(;i<LENGTH_O_HEX;i++)
            UDG_PRINTF("   ");
        UDG_PRINTF(" | ");
        for(i=0; i<length%LENGTH_O_HEX; i++)
        {
            if((BYTE)lp_buf[j+i] < 0x20 || (BYTE)lp_buf[j+i] > 0x7E)
                ch = '.';
            else
                ch = lp_buf[j+i];
            UDG_PRINTF("%c", ch);
        }
        UDG_PRINTF("\n");
    }
    UDG_PRINTF("--------------------------------------------------------------------------------\n");
}
#endif

#if DBG_PRINT_WSTRING
void dbg_print_wstring(PCWSTR p_str)
{
    while(*p_str)
    {
        if((*p_str>=0x20) && (*p_str<=0x7E))
            UDG_PRINTF("%c", *p_str);
        else
            UDG_PRINTF("?");
        p_str++;
    }
    UDG_PRINTF("\n");
}
#endif

#if DBG_DUMP_BITMAP
void dump_bitmap(UINT8 *p_buf, INT16 u_length, INT16 u_width, INT16 u_height, UINT8 u_factor)
{
    INT16 i, j;
    UINT8 u_color;
    UINT32 u_offset = u_length>>u_factor;

#if 1
    if(u_width%MIN_UNIT_BITMAP_WIDTH|| u_factor)
    {
        UDG_PRINTF("%s: Invalid Format\n", __FUNCTION__);
        return;
    }
    for(i=0; i<u_height; i++)
    {
        for(j=i*u_width; j<(i+1)*u_width; j+=4)
        {
            UDG_PRINTF("%02x%02x%02x%02x\n", p_buf[j+3], p_buf[j+2], p_buf[j+1], p_buf[j]);
        }
    }
#else
    UDG_PRINTF("   |");
    for(i=0; i<u_width; i++)
        UDG_PRINTF(" %02d", i);
    UDG_PRINTF("\n");
    UDG_PRINTF("---|");
    for(i=0; i<u_width; i++)
        UDG_PRINTF("---");
    UDG_PRINTF("\n");

    for(i=0; i<u_height; i++)
    {
        UDG_PRINTF("%02d |", i);
        for(j=0; j<u_width; j++)
        {
            u_color = p_buf[u_offset+(j>>u_factor)];
            if(u_factor)
            {
                if(!(j&0x01))
                    u_color >>= 4;
                u_color &= 0x0F;
            }
            UDG_PRINTF(" %02x", u_color);
        }
        UDG_PRINTF("\n");
        u_offset += (u_length>>u_factor);
    }
#endif
}
#endif

#if DBG_TRACE_EIT_EVENT
UINT32 dbg_total_size;
static void trace_eit_event(eit_event_info_t *ep)
{
    UDG_PRINTF("eit_event_info tracing: ep=0x%x, size=%d\n", ep, dbg_total_size);
    if(ep->eit_ct_desc)
        UDG_PRINTF("    eit_ct_desc=0x%x\n", ep->eit_ct_desc);
    if(ep->eit_pr_desc)
        UDG_PRINTF("    eit_pr_desc=0x%x\n", ep->eit_pr_desc);

    int i;
    if(ep->eit_sht_desc)
    {
        UDG_PRINTF("    eit_sht_desc=0x%x\n", ep->eit_sht_desc);
        UDG_PRINTF("       event_name=0x%x\n", ep->eit_sht_desc->event_name);
        UDG_PRINTF("       text_desc=0x%x\n", ep->eit_sht_desc->text_desc);
        eit_short_desc_t *psht = ep->eit_sht_desc->next;
        i = 0;
        while(psht)
        {
            UDG_PRINTF("    [%d]next=0x%x\n", i++, psht);
            UDG_PRINTF("       event_name=0x%x\n", psht->event_name);
            UDG_PRINTF("       text_desc=0x%x\n", psht->text_desc);
            psht = psht->next;
        }
    }

    if(ep->eit_ext_desc)
    {
        UDG_PRINTF("    eit_ext_desc=0x%x\n", ep->eit_ext_desc);
        UDG_PRINTF("       text_char=0x%x\n", ep->eit_ext_desc->text_char);
        eit_ext_desc_t *pext = ep->eit_ext_desc->next;
        i = 0;
        while(pext)
        {
            UDG_PRINTF("    [%d]next=0x%x\n", ++i, pext);
            UDG_PRINTF("       text_char=0x%x\n", pext->text_char);
            pext = pext->next;
        }
    }
}
#endif

#if DBG_PRINT_MEMORY_LAYOUT
static void ul2hex_str(DWORD val, PCHAR str, BOOL f_ignore_zero, BOOL f_capital)
{
    int i, j;
    char num, chr = 'A';
    BOOL f_non_zero = FALSE;

    if(!f_capital)
        chr = 'a';
    for(i=0, j=0; i<8; i++)
    {
        num = (val >> ((7-i)*4)) & 0x0F;
        if(!f_non_zero && f_ignore_zero)
        {
            if(!num)
                continue;
            else
                f_non_zero = TRUE;
        }
        if(num < LENGTH_O_ECIMAL)
            str[j++] = '0' + num;
        else
            str[j++] = chr + num - 0x0A;
    }
    if(!j)
        str[j++] = (char)'0';
    str[j] = 0;
}

typedef struct MEMORY_CONFIG
{
    DWORD address;
    DWORD size;
    char name[32]; 
}MCFG;
static void print_range(MCFG * cfg)
{
    char *sz_top     = "+------------------------------+";
    char *sz_content = "          |                              |";
    char out_buf[64], temp_buf[30], num_buf[10];
    int i, j, len, output_len, start_pos;

    DWORD address = cfg->address;
    DWORD size = cfg->size;
    char *name = cfg->name;

    UINT32 s_len = 0;
    int t_size = 0;

    size &= 0x0fffffff;
#ifndef DUAL_ENABLE
    DWORD number[PRINT_NUM_ITEM] = {size, (address)};
#else
    DWORD number[PRINT_NUM_ITEM] = {size, (address+size)};
#endif
    char *sz_prefix[PRINT_NUM_ITEM] = {"Size=0x", "Next=0x"};

    if(NULL == name)
    {
        UDG_PRINTF("          %s\n", sz_top);
        return;
    }

#ifndef DUAL_ENABLE
    UDG_PRINTF("0x%08x", address+size);
#else
    UDG_PRINTF("0x%08x", address);
#endif
    UDG_PRINTF("%s\n", sz_top);
    output_len = COMSTRLEN(sz_content) - CONTENT_RESERVED_LEN;

    COMSTRCOPY(out_buf, sz_content);
    len = COMSTRLEN(name);
    if(len > output_len)
        len = output_len;
    start_pos = REPLACE_START_POS + (output_len - len)/2;
    for(i=0; i<len; i++)
        out_buf[i+start_pos] = name[i];
    UDG_PRINTF("%s\n", out_buf);

    t_size = 30;
    for(j=0; j<PRINT_NUM_ITEM; j++)
    {
        COMSTRCOPY(out_buf, sz_content);
        COMSTRCOPY(temp_buf, sz_prefix[j]);
        ul2hex_str(number[j], num_buf, FALSE, FALSE);
        //COMSTRCAT(TempBuf, NumBuf);
        s_len = STRLEN(temp_buf);
        if ((t_size - s_len) > strlen(num_buf))
        {
            strncat(temp_buf, num_buf, (t_size-1-s_len));
            len = COMSTRLEN(temp_buf);
            if(len > output_len)
                len = output_len;
        }
        start_pos = REPLACE_START_POS + (output_len - len)/2;
        for(i=0; i<len; i++)
            out_buf[i+start_pos] = temp_buf[i];
        UDG_PRINTF("%s\n", out_buf);
    }
}

static void print_buffer_range(DWORD address, DWORD size, char *name)
{
    char *sz_top     = "+------------------------------+";
    char *sz_content = "          |                              |";
    char out_buf[64], temp_buf[30], num_buf[10];
    int i, j, len, output_len, start_pos;

    UINT32 s_len = 0;
    int t_size = 0;

    size &= 0x0fffffff;
#ifndef DUAL_ENABLE
    DWORD number[PRINT_NUM_ITEM] = {size, (address)};
#else
    DWORD number[PRINT_NUM_ITEM] = {size, (address+size)};
#endif
    char *sz_prefix[PRINT_NUM_ITEM] = {"Size=0x", "Next=0x"};

    if(NULL == name)
    {
        UDG_PRINTF("          %s\n", sz_top);
        return;
    }

#ifndef DUAL_ENABLE
    UDG_PRINTF("0x%08x", address+size);
#else
    UDG_PRINTF("0x%08x", address);
#endif
    UDG_PRINTF("%s\n", sz_top);
    output_len = COMSTRLEN(sz_content) - CONTENT_RESERVED_LEN;

    COMSTRCOPY(out_buf, sz_content);
    len = COMSTRLEN(name);
    if(len > output_len)
        len = output_len;
    start_pos = REPLACE_START_POS + (output_len - len)/2;
    for(i=0; i<len; i++)
        out_buf[i+start_pos] = name[i];
    UDG_PRINTF("%s\n", out_buf);

    t_size = 30;
    for(j=0; j<PRINT_NUM_ITEM; j++)
    {
        COMSTRCOPY(out_buf, sz_content);
        COMSTRCOPY(temp_buf, sz_prefix[j]);
        ul2hex_str(number[j], num_buf, FALSE, FALSE);
        //COMSTRCAT(TempBuf, NumBuf);
        s_len = STRLEN(temp_buf);
        if ((t_size - s_len) > strlen(num_buf))
        {
            strncat(temp_buf, num_buf, (t_size-1-s_len));
            len = COMSTRLEN(temp_buf);
            if(len > output_len)
                len = output_len;
        }
        start_pos = REPLACE_START_POS + (output_len - len)/2;
        for(i=0; i<len; i++)
            out_buf[i+start_pos] = temp_buf[i];
        UDG_PRINTF("%s\n", out_buf);
    }
}


#ifdef FSC_SUPPORT
void print_fcc_memory(void)
{    
    libc_printf("FCC Buffer Info\n");
    libc_printf("__MM_FSC_BUFFER_ADDR=0x%08x,__MM_FSC_BUFFER_LEN=0x%08x\n",\
        __MM_FSC_BUFFER_ADDR,__MM_FSC_BUFFER_LEN);

    libc_printf("__MM_DMX_REC_LEN=0x%08x,__MM_DMX_AVP_LEN=0x%08x,DMX_DMA_BUFFER_LEN=0x%08x\n",\
        __MM_DMX_REC_LEN,__MM_DMX_AVP_LEN,DMX_DMA_BUFFER_LEN);

    libc_printf("__MM_DMX_MAIN_BLK_LEN=0x%08x,FSC_CACHE_BUFFER_PART1_LEN=0x%08x\n",\
        __MM_DMX_MAIN_BLK_LEN,FSC_CACHE_BUFFER_PART1_LEN);
    
    libc_printf("__MM_DMX0_DMA_START_ADDR=0x%08x\n",__MM_DMX0_DMA_START_ADDR);
    libc_printf("__MM_DMX1_DMA_START_ADDR=0x%08x\n",__MM_DMX1_DMA_START_ADDR);
    libc_printf("__MM_DMX2_DMA_START_ADDR=0x%08x\n",__MM_DMX2_DMA_START_ADDR);

    libc_printf("__MM_DMX0_MAIN_BLK_START_ADDR=0x%08x\n",__MM_DMX0_MAIN_BLK_START_ADDR);
    libc_printf("__MM_DMX1_MAIN_BLK_START_ADDR=0x%08x\n",__MM_DMX1_MAIN_BLK_START_ADDR);
    libc_printf("__MM_DMX2_MAIN_BLK_START_ADDR=0x%08x\n",__MM_DMX2_MAIN_BLK_START_ADDR);


    libc_printf("__MM_PVR_VOB_BUFFER_ADDR=0x%08x,__MM_PVR_VOB_BUFFER_LEN=0x%08x\n",\
        __MM_PVR_VOB_BUFFER_ADDR,__MM_PVR_VOB_BUFFER_LEN); 

    libc_printf("\n\n");

}
#endif

#ifdef _S3811_
void print_memory_layout()
{
    extern UINT heap_start;
    //UDG_PRINTF("\n======== Memory Layout ==========\n");

    print_buffer_range(__MM_HIGHEST_ADDR, 0, "Total Memory");
    print_buffer_range(__MM_VOID_BUFFER_ADDR,VOID_LEN, "Void Memory");
    print_buffer_range(__MM_FB_TOP_ADDR, 0, "Frame buffer top memory");
        print_buffer_range(__MM_VBV_START_ADDR,__MM_VBV_LEN , "VBV");
    print_buffer_range(__MM_MAF_START_ADDR,__MM_MAF_LEN , "__MM_MAF_START_ADDR ");
    print_buffer_range(__MM_FB_START_ADDR,__MM_FB_LEN , "__MM_FB_START_ADDR ");

#ifdef AVC_SUPPORT_UNIFY_MEM
    print_buffer_range(AVC_VBV_ADDR, AVC_VBV_LEN , "AVC VBV");
    print_buffer_range(AVC_CMD_QUEUE_ADDR, AVC_CMD_QUEUE_LEN, " AVC_CMD_QUEUE_ADDR");
    print_buffer_range(AVC_MB_COL_ADDR, AVC_MB_COL_LEN, " AVC_MB_COL_ADDR");
    print_buffer_range(AVC_MB_NEI_ADDR, AVC_MB_NEI_LEN, "AVC_MB_NEI_ADDR ");
    print_buffer_range(AVC_MEM_ADDR, AVC_MEM_LEN, "AVC_MB_NEI_ADDR ");
#else
    print_buffer_range(AVC_VBV_ADDR, AVC_VBV_LEN , "AVC VBV");
    print_buffer_range(AVC_FB_ADDR, AVC_FB_LEN , "AVC FB");
    print_buffer_range(AVC_DVIEW_ADDR, AVC_DVIEW_LEN , "AVC DVIEW");
    print_buffer_range(AVC_MV_ADDR, AVC_MV_LEN , "AVC MV");
    print_buffer_range(AVC_CMD_QUEUE_ADDR, AVC_CMD_QUEUE_LEN , "AVC CMD QUEUE");
    print_buffer_range(AVC_MB_COL_ADDR, AVC_MB_COL_LEN, " AVC_MB_COL_ADDR");
    print_buffer_range(AVC_MB_NEI_ADDR, AVC_MB_NEI_LEN, "AVC_MB_NEI_ADDR ");
    print_buffer_range(AVC_LAF_FLAG_BUF_ADDR, AVC_LAF_FLAG_BUF_LEN , "AVC_LAF_FLAG_BUF_ADDR");
    print_buffer_range(AVC_LAF_RW_BUF_ADDR, AVC_LAF_RW_BUF_LEN , "AVC_LAF_RW_BUF_ADDR");
#endif
    print_buffer_range(__MM_FB_BOTTOM_ADDR, 0, "__MM_FB_BOTTOM_ADDR ");

    print_buffer_range(__MM_PVR_VOB_BUFFER_ADDR, __MM_PVR_VOB_BUFFER_LEN, "VOB cache");
    print_buffer_range(__MM_AUTOSCAN_DB_BUFFER_ADDR, __MM_AUTOSCAN_DB_BUFFER_LEN, "AUTOSCAN DB ");

    print_buffer_range(__MM_DMX_AVP_START_ADDR,__MM_SI_VBV_OFFSET + __MM_DMX_DATA_LEN + __MM_DMX_AVP_LEN , "DMX AVP ");
    print_buffer_range(__MM_DMX_REC_START_ADDR,__MM_DMX_REC_LEN , "DMX_REC_Start ");
    print_buffer_range(__MM_DMX_CPU_BLK_ADDR, __MM_DMX_BLK_BUF_LEN,"__MM_DMX_CPU_BLK_ADDR");
    print_buffer_range(__MM_DMX_SEE_BLK_ADDR,__MM_DMX_BLK_BUF_LEN , "__MM_DMX_SEE_BLK_ADDR ");
    print_buffer_range(__MM_TTX_SUB_PAGE_BUF_ADDR, __MM_TTX_SUB_PAGE_LEN, "TTX_SUB_PAGE ");
    print_buffer_range(__MM_TTX_P26_NATION_BUF_ADDR,__MM_TTX_P26_NATION_LEN , " TTX_P26_NATION");
    print_buffer_range(__MM_TTX_P26_DATA_BUF_ADDR, __MM_TTX_P26_DATA_LEN, "TTX_P26_DATA_ ");
    print_buffer_range(__MM_TTX_BS_START_ADDR,__MM_TTX_BS_LEN , "__MM_TTX_BS_START_ADDR ");
    print_buffer_range(__MM_TTX_PB_START_ADDR,__MM_TTX_PB_LEN , "__MM_TTX_PB_START_ADDR ");
    print_buffer_range(__MM_EPG_BUFFER_START ,__MM_EPG_BUFFER_LEN, "EPG_BUFFER ");

    print_buffer_range(__MM_GE_START_ADDR, __MM_GE_LEN, "GE ");
    print_buffer_range(__MM_OSD_BK_ADDR2, __MM_OSD2_LEN, "OSD LAYER2 ");
    print_buffer_range(__MM_OSD_BK_ADDR1,__MM_OSD1_LEN , "OSD LAYER1 ");
    print_buffer_range(__MM_VCAP_FB_ADDR, __MM_VCAP_FB_SIZE, "__MM_VCAP_FB_ADDR ");
#ifdef OSD_VSRC_SUPPORT
    print_buffer_range(__MM_OSD_VSRC_MEM_ADDR, OSD_VSRC_MEM_MAX_SIZE, "OSD VSRC ");
#endif
    print_buffer_range(__MM_SUB_BS_START_ADDR,__MM_SUB_BS_LEN , "SUB_BS_START_ ");
    print_buffer_range(__MM_SUB_HW_DATA_ADDR,__MM_SUB_HW_DATA_LEN , "__MM_SUB_HW_DATA_ADDR ");
    print_buffer_range(__MM_SUB_PB_START_ADDR,__MM_SUB_PB_LEN , "SUB_PB_START ");

#if(SYS_SDRAM_SIZE == 128)
    print_buffer_range(__MM_COFDM_S3811_ISDBT_ADDR,__MM_COFDM_S3811_ISDBT_BUF_LEN , "__MM_COFDM_S3811_ISDBT_ADDR ");
#endif
#ifdef ISDBT_CC
    print_buffer_range(__MM_ISDBTCC_BS_START_ADDR,__MM_ISDBTCC_BS_LEN , "__MM_ISDBTCC_BS_START_ADDR ");
    print_buffer_range(__MM_ISDBTCC_PB_START_ADDR,__MM_ISDBTCC_PB_LEN , "__MM_ISDBTCC_PB_START_ADDR ");
    print_buffer_range(__MM_LWIP_MEM_ADDR,__MM_LWIP_MEM_LEN , "__MM_LWIP_MEM_ADDR ");
#else
    print_buffer_range(__MM_LWIP_MEM_ADDR,__MM_LWIP_MEM_LEN , "__MM_LWIP_MEM_ADDR ");
#endif
    print_buffer_range(__MM_LWIP_MEMP_ADDR, __MM_LWIP_MEMP_LEN,"__MM_LWIP_MEMP_ADDR");
    print_buffer_range(__MM_USB_START_ADDR, __MM_USB_DMA_LEN," USB_START_");
    print_buffer_range(__MM_CPU_DBG_MEM_ADDR, __MM_DBG_MEM_LEN, "CPU debug buffer ");

    print_buffer_range(__MM_MP_BUFFER_ADDR, __MM_MP_BUFFER_LEN, "__MM_MP_BUFFER_ADDR");

    print_buffer_range(heap_start, (__MM_HEAP_TOP_ADDR&0x0fffffff)-(heap_start&0x0fffffff), "Heap");
    print_buffer_range(0, heap_start, "Code");

    print_buffer_range(0, 0, NULL);
}
#else
#ifndef _USE_32M_MEM_
void print_memory_layout()
{
    extern UINT heap_start;
    //UDG_PRINTF("\n======== Memory Layout ==========\n");

    print_buffer_range(0, heap_start, "Code");
    print_buffer_range(heap_start, (__MM_HEAP_TOP_ADDR&0x0fffffff)-(heap_start&0x0fffffff), "Heap");

    print_buffer_range(__MM_AUTOSCAN_DB_BUFFER_ADDR, __MM_AUTOSCAN_DB_BUFFER_LEN, "AUTOSCAN DB ");
    print_buffer_range(__MM_DMX_AVP_START_ADDR,__MM_SI_VBV_OFFSET + __MM_DMX_DATA_LEN + __MM_DMX_AVP_LEN , "DMX AVP ");
    print_buffer_range(__MM_VBV_START_ADDR,__MM_VBV_LEN , "VBV");
    print_buffer_range(__MM_EPG_BUFFER_START ,__MM_EPG_BUFFER_LEN, "EPG_BUFFER ");
    print_buffer_range(__MM_USB_START_ADDR, __MM_USB_DMA_LEN," USB_START_");
    print_buffer_range(__MM_SUB_PB_START_ADDR,__MM_SUB_PB_LEN , "SUB_PB_START ");
    print_buffer_range(__MM_SUB_BS_START_ADDR,__MM_SUB_BS_LEN , "SUB_BS_START_ ");
    print_buffer_range(__MM_TTX_P26_DATA_BUF_ADDR, __MM_TTX_P26_DATA_LEN, "TTX_P26_DATA_ ");
    print_buffer_range(__MM_TTX_P26_NATION_BUF_ADDR,__MM_TTX_P26_NATION_LEN , " TTX_P26_NATION");
    print_buffer_range(__MM_TTX_SUB_PAGE_BUF_ADDR, __MM_TTX_SUB_PAGE_LEN, "TTX_SUB_PAGE ");
    print_buffer_range(__MM_TTX_PB_START_ADDR, __MM_TTX_PB_LEN, "TTX_PB_START ");
    print_buffer_range(__MM_TTX_BS_START_ADDR, __MM_TTX_BS_LEN, "TTX_BS_START ");
    print_buffer_range(__MM_DMX_REC_START_ADDR,__MM_DMX_REC_LEN , "DMX_REC_Start ");

    print_buffer_range(AVC_LAF_FLAG_BUF_ADDR, AVC_LAF_FLAG_BUF_LEN, "AVC_LAF_FLAG_BUF_ADDR ");
    print_buffer_range(AVC_LAF_RW_BUF_ADDR,AVC_LAF_RW_BUF_LEN , "AVC_LAF_RW_BUF_ADDR ");
    print_buffer_range(AVC_CMD_QUEUE_ADDR, AVC_CMD_QUEUE_LEN, " AVC_CMD_QUEUE_ADDR");
    print_buffer_range(AVC_MB_NEI_ADDR, AVC_MB_NEI_LEN, "AVC_MB_NEI_ADDR ");
    print_buffer_range(AVC_MB_COL_ADDR, AVC_MB_COL_LEN, " AVC_MB_COL_ADDR");
    print_buffer_range(AVC_MV_ADDR, AVC_MV_LEN, "AVC_MV_ADDR ");
    print_buffer_range(AVC_DVIEW_ADDR,AVC_DVIEW_LEN , "AVC_DVIEW_ADDR ");
    print_buffer_range(AVC_FB_ADDR, AVC_FB_LEN, "AVC_FB_ADDR ");
    print_buffer_range(AVC_VBV_ADDR, AVC_VBV_LEN , "AVC VBV");
    print_buffer_range(__MM_FB_TOP_ADDR, 0, "Frame buffer top memory");

//    print_buffer_range(__MM_OSD_VSRC_MEM_ADDR, OSD_VSRC_MEM_MAX_SIZE, "OSD VSRC ");
    print_buffer_range(__MM_OSD_BK_ADDR2, __MM_OSD2_LEN, "OSD LAYER2 ");
    print_buffer_range(__MM_OSD_BK_ADDR1,__MM_OSD1_LEN , "OSD LAYER1 ");
    print_buffer_range(__MM_GE_START_ADDR, __MM_GE_LEN, "GE ");
    print_buffer_range(__MM_PVR_VOB_BUFFER_ADDR, __MM_PVR_VOB_BUFFER_LEN, "VOB cache");
    print_buffer_range(__MM_VOID_BUFFER_ADDR,VOID_LEN, "Void Memory");
    print_buffer_range(__MM_HIGHEST_ADDR, 0, "Total Memory");

    print_buffer_range(0, 0, NULL);
}
#endif
#endif

#ifdef _SUPPORT_64M_MEM
void print_memory_layout_single()
{
    extern UINT heap_start;
    //UDG_PRINTF("\n======== Memory Layout ==========\n");

    //print_buffer_range(__MM_SHARE_BASE_ADDR,__MM_SHARED_MEM_LEN , "__MM_SHARE_BASE_ADDR ");
    print_buffer_range(__MM_HIGHEST_ADDR, 0, "Total Memory");
    print_buffer_range(__MM_VOID_BUFFER_ADDR, __MM_VOID_BUFFER_LEN , "void buffer to customer");
    libc_printf("\n\nshare memory mapping\n\n");

    print_buffer_range(__MM_VBV_START_ADDR,__MM_VBV_LEN , "__MM_VBV_START_ADDR ");
    print_buffer_range(__MM_MAF_START_ADDR,__MM_MAF_LEN , "__MM_MAF_START_ADDR ");
    print_buffer_range(__MM_FB_START_ADDR,__MM_FB_LEN , "__MM_FB_START_ADDR ");
    print_buffer_range(__MM_PVR_VOB_BUFFER_ADDR,__MM_PVR_VOB_BUFFER_LEN , "VOB cache");

    print_buffer_range(AVC_VBV_ADDR, AVC_VBV_LEN , "AVC VBV");
    print_buffer_range(AVC_CMD_QUEUE_ADDR, AVC_CMD_QUEUE_LEN, " AVC_CMD_QUEUE_ADDR");
    print_buffer_range(AVC_MB_COL_ADDR, AVC_MB_COL_LEN, " AVC_MB_COL_ADDR");
    print_buffer_range(AVC_MB_NEI_ADDR, AVC_MB_NEI_LEN, "AVC_MB_NEI_ADDR ");
    print_buffer_range(AVC_MEM_ADDR, AVC_MEM_LEN, "AVC_MEM_ADDR ");
    print_buffer_range(__MM_AUTOSCAN_DB_BUFFER_ADDR, __MM_AUTOSCAN_DB_BUFFER_LEN,"__MM_AUTOSCAN_DB_BUFFER_ADDR");

    print_buffer_range(__MM_DMX_AVP_START_ADDR,__MM_SI_VBV_OFFSET + __MM_DMX_DATA_LEN + __MM_DMX_AVP_LEN , "DMX AVP ");
    print_buffer_range(__MM_DMX_REC_START_ADDR, __MM_DMX_REC_LEN,"__MM_DMX_REC_START_ADDR");
    print_buffer_range(__MM_DMX_CPU_BLK_ADDR, __MM_DMX_BLK_BUF_LEN,"__MM_DMX_CPU_BLK_ADDR");

    print_buffer_range(__MM_DMX_SEE_BLK_ADDR,__MM_DMX_BLK_BUF_LEN , "__MM_DMX_SEE_BLK_ADDR ");

    print_buffer_range(__MM_TTX_SUB_PAGE_BUF_ADDR, __MM_TTX_SUB_PAGE_LEN, "TTX_SUB_PAGE ");
    print_buffer_range(__MM_TTX_P26_NATION_BUF_ADDR,__MM_TTX_P26_NATION_LEN , " TTX_P26_NATION");
    print_buffer_range(__MM_TTX_P26_DATA_BUF_ADDR, __MM_TTX_P26_DATA_LEN, "TTX_P26_DATA_ ");
    print_buffer_range(__MM_TTX_BS_START_ADDR, __MM_TTX_BS_LEN, "TTX_BS_START ");
    print_buffer_range(__MM_TTX_PB_START_ADDR, __MM_TTX_PB_LEN, "TTX_PB_START ");

    print_buffer_range(__MM_EPG_BUFFER_START ,__MM_EPG_BUFFER_LEN, "EPG_BUFFER ");
    print_buffer_range(__MM_OSD_BK_ADDR2, __MM_OSD2_LEN, "__MM_OSD_BK_ADDR2 ");
    print_buffer_range(__MM_OSD_BK_ADDR1, __MM_OSD1_LEN, "__MM_OSD_BK_ADDR1 ");
    print_buffer_range(__MM_VCAP_FB_ADDR, __MM_VCAP_FB_SIZE, "__MM_VCAP_FB_ADDR ");
    print_buffer_range(__MM_OSD_VSRC_MEM_ADDR, OSD_VSRC_MEM_MAX_SIZE, "__MM_OSD_VSRC_MEM_ADDR ");
    //print_buffer_range(__MM_GE_START_ADDR, __MM_GE_LEN, "__MM_GE_START_ADDR ");
    print_buffer_range(__MM_SUB_BS_START_ADDR,__MM_SUB_BS_LEN , "SUB_BS_START_ ");
    print_buffer_range(__MM_SUB_PB_START_ADDR,__MM_SUB_PB_LEN , "SUB_PB_START ");

    libc_printf("\n\nmain memory mapping\n\n");
    print_buffer_range(heap_start, (__MM_HEAP_TOP_ADDR&0x0fffffff)-(heap_start&0x0fffffff), "Heap");
    print_buffer_range(0, heap_start, "Code");

    libc_printf("\n\n MP buffer\n\n");
    print_buffer_range(__MM_MP_BUFFER_ADDR,__MM_MP_BUFFER_LEN , "MM_MP_BUFFER ");


    print_buffer_range(0, 0, NULL);

}

#else

#ifdef _USE_32M_MEM_
void print_memory_layout_dual()
{
    extern UINT heap_start;
    //UDG_PRINTF("\n======== Memory Layout ==========\n");

    libc_printf("\n\nmain memory mapping\n\n");

    print_buffer_range(0, heap_start, "Code");
    print_buffer_range(heap_start, (__MM_HEAP_TOP_ADDR&0x0fffffff)-(heap_start&0x0fffffff), "Heap");

    print_buffer_range(__MM_CPU_DBG_MEM_ADDR, __MM_DBG_MEM_LEN, "CPU debug buffer ");
    print_buffer_range(__MM_DCII_SUB_BS_START_ADDR, __MM_DCII_SUB_BS_LEN, "DCII");

    print_buffer_range(__MM_VCAP_FB_ADDR, __MM_VCAP_FB_SIZE, "__MM_VCAP_FB_ADDR ");
    print_buffer_range(__MM_OSD_BK_ADDR1, __MM_OSD_LEN, "__MM_OSD_BK_ADDR1 ");

    print_buffer_range(__MM_NIM_BUFFER_ADDR, __MM_NIM_BUFFER_LEN, "nim J83B buffer ");
    print_buffer_range(__MM_OSD_BK_ADDR3, __MM_OSD2_LEN, "__MM_OSD_BK_ADDR3 ");
    print_buffer_range(__MM_EPG_BUFFER_START ,__MM_EPG_BUFFER_LEN, "EPG_BUFFER ");
    print_buffer_range(__MM_DMX_CPU_BLK_ADDR, __MM_DMX_BLK_BUF_LEN,"__MM_DMX_CPU_BLK_ADDR");
    print_buffer_range(__MM_DMX_REC_START_ADDR, __MM_DMX_REC_LEN,"__MM_DMX_REC_START_ADDR");
    print_buffer_range(__MM_DMX_AVP_START_ADDR,__MM_SI_VBV_OFFSET + __MM_DMX_DATA_LEN + __MM_DMX_AVP_LEN , "DMX AVP ");
    print_buffer_range(__MM_AUTOSCAN_DB_BUFFER_ADDR, __MM_AUTOSCAN_DB_BUFFER_LEN,"__MM_AUTOSCAN_DB_BUFFER_ADDR");
    print_buffer_range(__MM_PVR_VOB_BUFFER_ADDR,__MM_PVR_VOB_BUFFER_LEN , "VOB cache");
    print_buffer_range(AVC_LAF_RW_BUF_ADDR,AVC_LAF_RW_BUF_LEN , "AVC_LAF_RW_BUF_ADDR ");
    print_buffer_range(AVC_MB_NEI_ADDR, AVC_MB_NEI_LEN, "AVC_MB_NEI_ADDR ");
    print_buffer_range(AVC_MB_COL_ADDR, AVC_MB_COL_LEN, " AVC_MB_COL_ADDR");
    print_buffer_range(AVC_MV_ADDR, AVC_MV_LEN, "AVC_MV_ADDR ");
    print_buffer_range(AVC_DVIEW_ADDR,AVC_DVIEW_LEN , "AVC_DVIEW_ADDR ");
    print_buffer_range(AVC_FB_ADDR, AVC_FB_LEN, "AVC_FB_ADDR ");
    print_buffer_range(__MM_FB_TOP_ADDR, 0, "Frame buffer top memory");
    print_buffer_range(__MM_OSD_VSRC_MEM_ADDR, OSD_VSRC_MEM_MAX_SIZE, "__MM_OSD_VSRC_MEM_ADDR ");

    libc_printf("\n\nprivate memory mapping\n\n");
#ifdef DUAL_ENABLE
    print_buffer_range(__MM_SEE_DBG_MEM_ADDR,__MM_DBG_MEM_LEN , "SEE debug");
#endif
    print_buffer_range(__MM_SUBT_ATSC_SEC_ADDR,__MM_SUBT_ATSC_SEC_LEN , "SUB_ATSC_ADDR");
    print_buffer_range(__MM_SUB_PB_START_ADDR,__MM_SUB_PB_LEN , "SUB_PB_START ");
    print_buffer_range(__MM_SUB_HW_DATA_ADDR,__MM_SUB_HW_DATA_LEN , "__MM_SUB_HW_DATA_ADDR ");
    print_buffer_range(__MM_SUB_BS_START_ADDR,__MM_SUB_BS_LEN , "SUB_BS_START_ ");
    print_buffer_range(__MM_OSD_BK_ADDR2, __MM_OSD1_LEN, "__MM_OSD_BK_ADDR2 ");
    print_buffer_range(AVC_LAF_FLAG_BUF_ADDR, AVC_LAF_FLAG_BUF_LEN, "AVC_LAF_FLAG_BUF_ADDR ");
    print_buffer_range(AVC_CMD_QUEUE_ADDR, AVC_CMD_QUEUE_LEN, "AVC_CMD_QUEUE_ADDR ");
#ifdef DUAL_ENABLE
    print_buffer_range(__MM_SEE_MP_BUF_ADDR,__MM_SEE_MP_BUF_LEN , " __MM_SEE_MP_BUF_ADDR");
#endif
    print_buffer_range(__MM_TTX_PB_START_ADDR, __MM_TTX_PB_LEN, "TTX_PB_START ");
    print_buffer_range(__MM_TTX_BS_START_ADDR, __MM_TTX_BS_LEN, "TTX_BS_START ");
    print_buffer_range(__MM_TTX_P26_DATA_BUF_ADDR, __MM_TTX_P26_DATA_LEN, "TTX_P26_DATA_ ");
    print_buffer_range(__MM_TTX_P26_NATION_BUF_ADDR,__MM_TTX_P26_NATION_LEN , " TTX_P26_NATION");
    print_buffer_range(__MM_TTX_SUB_PAGE_BUF_ADDR, __MM_TTX_SUB_PAGE_LEN, "TTX_SUB_PAGE ");
    print_buffer_range(__MM_DMX_SEE_BLK_ADDR,__MM_DMX_BLK_BUF_LEN , "__MM_DMX_SEE_BLK_ADDR ");
    print_buffer_range(AVC_VBV_ADDR,AVC_VBV_LEN , "AVC_VBV_ADDR ");
    print_buffer_range(__MM_VBV_START_ADDR,__MM_VBV_LEN , "__MM_VBV_START_ADDR ");

    libc_printf("\n\nshare memory mapping\n\n");
#ifdef DUAL_ENABLE
    print_buffer_range(__MM_SHARE_BASE_ADDR,__MM_SHARED_MEM_LEN , "__MM_SHARE_BASE_ADDR ");
    print_buffer_range(__MM_VOID_BUFFER_ADDR, __MM_VOID_BUFFER_LEN , "void buffer to customer");
#endif
    print_buffer_range(__MM_HIGHEST_ADDR, 0, "Total Memory");

    print_buffer_range(0, 0, NULL);

#ifndef H264_32M_SUPPORT
    print_buffer_range(__MM_FB0_Y_START_ADDR, __MM_FB0_Y_LEN, "FB0_Y");
    print_buffer_range(__MM_FB0_C_START_ADDR, __MM_FB0_C_LEN, "FB0_C");
    print_buffer_range(__MM_FB1_Y_START_ADDR, __MM_FB1_Y_LEN, "FB1_Y");
    print_buffer_range(__MM_FB1_C_START_ADDR, __MM_FB1_C_LEN, "FB1_C");
    print_buffer_range(__MM_FB2_Y_START_ADDR, __MM_FB2_Y_LEN, "FB2_Y");
    print_buffer_range(__MM_FB2_C_START_ADDR, __MM_FB2_C_LEN, "FB2_C");

    print_buffer_range(__MM_MP_BUFFER_ADDR, __MM_MP_BUFFER_LEN, "MP_BUFF");
    print_buffer_range(__MM_VIDEO_FILE_BUF_ADDR, __MM_VIDEO_FILE_BUF_LEN, "video_file_buff");
#endif
    libc_printf("SYS_DRAM_SIZE=%d\n", SYS_SDRAM_SIZE);
}

void print_memory_layout_dual_32m()
{
    extern UINT heap_start;
    //UDG_PRINTF("\n======== Memory Layout ==========\n");

    libc_printf("\n\nmain memory mapping\n\n");

    print_buffer_range(0, heap_start, "Code");
    print_buffer_range(heap_start, (__MM_HEAP_TOP_ADDR&0x0fffffff)-(heap_start&0x0fffffff), "Heap");

    print_buffer_range(__MM_CPU_DBG_MEM_ADDR, __MM_DBG_MEM_LEN, "CPU debug buffer ");

    print_buffer_range(__MM_DCII_SUB_BS_START_ADDR, __MM_DCII_SUB_BS_LEN, "DCII");
    print_buffer_range(__MM_OSD_BK_ADDR1, __MM_OSD_LEN, "__MM_OSD_BK_ADDR1 ");
    print_buffer_range(__MM_NIM_BUFFER_ADDR, __MM_NIM_BUFFER_LEN, "nim J83B buffer ");
    print_buffer_range(__MM_DMX_CPU_BLK_ADDR, __MM_DMX_BLK_BUF_LEN,"__MM_DMX_CPU_BLK_ADDR");
    print_buffer_range(__MM_DMX_AVP_START_ADDR,__MM_SI_VBV_OFFSET + __MM_DMX_DATA_LEN + __MM_DMX_AVP_LEN , "DMX AVP ");
    print_buffer_range(__MM_PVR_VOB_BUFFER_ADDR,__MM_PVR_VOB_BUFFER_LEN , "VOB cache");

    print_buffer_range(__OTA_ADDR, __OTA_ADDR_LEN, "__OTA_ADDR ");
    print_buffer_range(__MM_AUTOSCAN_DB_BUFFER_ADDR, __MM_AUTOSCAN_DB_BUFFER_LEN,"__MM_AUTOSCAN_DB_BUFFER_ADDR");

    print_buffer_range(__MM_OSD_BK_ADDR3, __MM_OSD2_LEN, "__MM_OSD_BK_ADDR3 ");
    print_buffer_range(__MM_EPG_BUFFER_START ,__MM_EPG_BUFFER_LEN, "EPG_BUFFER ");

    print_buffer_range(__MM_DVW_START_ADDR, __MM_DVW_LEN, "__MM_DVW_ADDR");
    print_buffer_range(__MM_FB_START_ADDR, __MM_FB_LEN, "__MM_FB_ADDR ");
    print_buffer_range(__MM_FB_TOP_ADDR, 0, "Frame buffer top memory");
    print_buffer_range(__MM_OSD_VSRC_MEM_ADDR, OSD_VSRC_MEM_MAX_SIZE, "__MM_OSD_VSRC_MEM_ADDR ");

    libc_printf("\n\nprivate memory mapping\n\n");
#ifdef CAS9_VSC
    print_buffer_range(__MM_VSC_CODE_START_ADDR,__MM_VSC_CODE_LEN , "VSC Code");
    print_buffer_range(__MM_VSC_DATA_START_ADDR,__MM_VSC_DATA_LEN , "VSC Data");
    print_buffer_range(__MM_SEE_CODE_START_ADDR, SEE_CODE_SIZE , "SEE Code");
    print_buffer_range(__MM_SEE_HEAP_START_ADDR,__MM_SEE_DBG_MEM_ADDR-__MM_SEE_HEAP_START_ADDR , "SEE Heap");
#endif
#ifdef DUAL_ENABLE
    print_buffer_range(__MM_SEE_DBG_MEM_ADDR,__MM_DBG_MEM_LEN , "SEE debug");
#endif
    print_buffer_range(__MM_SUBT_ATSC_SEC_ADDR,__MM_SUBT_ATSC_SEC_LEN , "SUB_ATSC_ADDR");
    print_buffer_range(__MM_SUB_PB_START_ADDR,__MM_SUB_PB_LEN , "SUB_PB_START ");
    print_buffer_range(__MM_SUB_BS_START_ADDR,__MM_SUB_BS_LEN , "SUB_BS_START_ ");
    print_buffer_range(__MM_OSD_BK_ADDR2, __MM_OSD1_LEN, "__MM_OSD_BK_ADDR2 ");
    print_buffer_range(__MM_TTX_PB_START_ADDR, __MM_TTX_PB_LEN, "TTX_PB_START ");
    print_buffer_range(__MM_TTX_BS_START_ADDR, __MM_TTX_BS_LEN, "TTX_BS_START ");
    print_buffer_range(__MM_TTX_P26_DATA_BUF_ADDR, __MM_TTX_P26_DATA_LEN, "TTX_P26_DATA_ ");
    print_buffer_range(__MM_TTX_P26_NATION_BUF_ADDR,__MM_TTX_P26_NATION_LEN , " TTX_P26_NATION");
    print_buffer_range(__MM_TTX_SUB_PAGE_BUF_ADDR, __MM_TTX_SUB_PAGE_LEN, "TTX_SUB_PAGE ");
    print_buffer_range(__MM_DMX_SEE_BLK_ADDR,__MM_DMX_BLK_BUF_LEN , "__MM_DMX_SEE_BLK_ADDR ");
    print_buffer_range(__MM_MAF_START_ADDR,__MM_MAF_LEN , "__MM_MAF_ADDR ");
    print_buffer_range(__MM_VBV_START_ADDR,__MM_VBV_LEN , "__MM_VBV_START_ADDR ");

    libc_printf("\n\nshare memory mapping\n\n");
#ifdef DUAL_ENABLE
    print_buffer_range(__MM_SHARE_BASE_ADDR,__MM_SHARED_MEM_LEN , "__MM_SHARE_BASE_ADDR ");
    print_buffer_range(__MM_VOID_BUFFER_ADDR, __MM_VOID_BUFFER_LEN , "void buffer to customer");
#endif
    print_buffer_range(__MM_HIGHEST_ADDR, 0, "Total Memory");

    print_buffer_range(0, 0, NULL);
    libc_printf("SYS_DRAM_SIZE=%d\n", SYS_SDRAM_SIZE);
}

#else
#ifdef _S3281_
void print_memory_layout_dual()
{
    extern UINT heap_start;
    //UDG_PRINTF("\n======== Memory Layout ==========\n");

    libc_printf("\n\nmain memory mapping\n\n");

    print_buffer_range(0, heap_start, "Code");
    print_buffer_range(heap_start, (__MM_HEAP_TOP_ADDR&0x0fffffff)-(heap_start&0x0fffffff), "Heap");

    print_buffer_range(__MM_CPU_DBG_MEM_ADDR, __MM_DBG_MEM_LEN, "CPU debug buffer ");
    print_buffer_range(__MM_DCII_SUB_BS_START_ADDR, __MM_DCII_SUB_BS_LEN, "DCII");
    print_buffer_range(__MM_USB_START_ADDR, __MM_USB_DMA_LEN,"USB_START_");
    print_buffer_range(__MM_LWIP_MEMP_ADDR, __MM_LWIP_MEMP_LEN,"__MM_LWIP_MEMP_ADDR");
    print_buffer_range(__MM_LWIP_MEM_ADDR, __MM_LWIP_MEM_LEN,"__MM_LWIP_MEM_ADDR");
    print_buffer_range(__MM_VCAP_FB_ADDR, __MM_VCAP_FB_SIZE, "__MM_VCAP_FB_ADDR ");
    print_buffer_range(__MM_OSD_BK_ADDR1, __MM_OSD_LEN, "__MM_OSD_BK_ADDR1 ");
    print_buffer_range(__MM_GE_START_ADDR, __MM_GE_LEN, "__MM_GE_START_ADDR ");
    print_buffer_range(__MM_NIM_BUFFER_ADDR, __MM_NIM_BUFFER_LEN, "nim J83B buffer ");
    print_buffer_range(__MM_OSD_BK_ADDR3, __MM_OSD2_LEN, "__MM_OSD_BK_ADDR3 ");
    print_buffer_range(__MM_EPG_BUFFER_START ,__MM_EPG_BUFFER_LEN, "EPG_BUFFER ");
    print_buffer_range(__MM_DMX_CPU_BLK_ADDR, __MM_DMX_BLK_BUF_LEN,"__MM_DMX_CPU_BLK_ADDR");
    print_buffer_range(__MM_DMX_REC_START_ADDR, __MM_DMX_REC_LEN,"__MM_DMX_REC_START_ADDR");
    print_buffer_range(__MM_DMX_AVP_START_ADDR,__MM_SI_VBV_OFFSET + __MM_DMX_DATA_LEN + __MM_DMX_AVP_LEN , "DMX AVP ");
    print_buffer_range(__MM_AUTOSCAN_DB_BUFFER_ADDR, __MM_AUTOSCAN_DB_BUFFER_LEN,"__MM_AUTOSCAN_DB_BUFFER_ADDR");
    print_buffer_range(__MM_PVR_VOB_BUFFER_ADDR,__MM_PVR_VOB_BUFFER_LEN , "VOB cache");
    print_buffer_range(AVC_LAF_RW_BUF_ADDR,AVC_LAF_RW_BUF_LEN , "AVC_LAF_RW_BUF_ADDR ");
    print_buffer_range(AVC_MB_NEI_ADDR, AVC_MB_NEI_LEN, "AVC_MB_NEI_ADDR ");
    print_buffer_range(AVC_MB_COL_ADDR, AVC_MB_COL_LEN, " AVC_MB_COL_ADDR");
    print_buffer_range(AVC_MV_ADDR, AVC_MV_LEN, "AVC_MV_ADDR ");
    print_buffer_range(AVC_DVIEW_ADDR,AVC_DVIEW_LEN , "AVC_DVIEW_ADDR ");
    print_buffer_range(AVC_FB_ADDR, AVC_FB_LEN, "AVC_FB_ADDR ");
    print_buffer_range(__MM_FB_TOP_ADDR, 0, "Frame buffer top memory");
    print_buffer_range(__MM_OSD_VSRC_MEM_ADDR, OSD_VSRC_MEM_MAX_SIZE, "__MM_OSD_VSRC_MEM_ADDR ");

    libc_printf("\n\nprivate memory mapping\n\n");
#ifdef DUAL_ENABLE
    print_buffer_range(__MM_SEE_DBG_MEM_ADDR,__MM_DBG_MEM_LEN , "SEE debug");
#endif
    print_buffer_range(__MM_SUBT_ATSC_SEC_ADDR,__MM_SUBT_ATSC_SEC_LEN , "SUB_ATSC_ADDR");
    print_buffer_range(__MM_SUB_PB_START_ADDR,__MM_SUB_PB_LEN , "SUB_PB_START ");
    print_buffer_range(__MM_SUB_HW_DATA_ADDR,__MM_SUB_HW_DATA_LEN , "__MM_SUB_HW_DATA_ADDR ");
    print_buffer_range(__MM_SUB_BS_START_ADDR,__MM_SUB_BS_LEN , "SUB_BS_START_ ");
    print_buffer_range(__MM_OSD_BK_ADDR2, __MM_OSD1_LEN, "__MM_OSD_BK_ADDR2 ");
    print_buffer_range(AVC_LAF_FLAG_BUF_ADDR, AVC_LAF_FLAG_BUF_LEN, "AVC_LAF_FLAG_BUF_ADDR ");
    print_buffer_range(AVC_CMD_QUEUE_ADDR, AVC_CMD_QUEUE_LEN, "AVC_CMD_QUEUE_ADDR ");
#ifdef DUAL_ENABLE
    print_buffer_range(__MM_SEE_MP_BUF_ADDR,__MM_SEE_MP_BUF_LEN , " __MM_SEE_MP_BUF_ADDR");
#endif
    print_buffer_range(__MM_TTX_PB_START_ADDR, __MM_TTX_PB_LEN, "TTX_PB_START ");
    print_buffer_range(__MM_TTX_BS_START_ADDR, __MM_TTX_BS_LEN, "TTX_BS_START ");
    print_buffer_range(__MM_TTX_P26_DATA_BUF_ADDR, __MM_TTX_P26_DATA_LEN, "TTX_P26_DATA_ ");
    print_buffer_range(__MM_TTX_P26_NATION_BUF_ADDR,__MM_TTX_P26_NATION_LEN , " TTX_P26_NATION");
    print_buffer_range(__MM_TTX_SUB_PAGE_BUF_ADDR, __MM_TTX_SUB_PAGE_LEN, "TTX_SUB_PAGE ");
    print_buffer_range(__MM_DMX_SEE_BLK_ADDR,__MM_DMX_BLK_BUF_LEN , "__MM_DMX_SEE_BLK_ADDR ");
    print_buffer_range(AVC_VBV_ADDR,AVC_VBV_LEN , "AVC_VBV_ADDR ");
    print_buffer_range(__MM_VBV_START_ADDR,__MM_VBV_LEN , "__MM_VBV_START_ADDR ");

    libc_printf("\n\nshare memory mapping\n\n");
#ifdef DUAL_ENABLE
    print_buffer_range(__MM_SHARE_BASE_ADDR,__MM_SHARED_MEM_LEN , "__MM_SHARE_BASE_ADDR ");
    print_buffer_range(__MM_VOID_BUFFER_ADDR, __MM_VOID_BUFFER_LEN , "void buffer to customer");
#endif
    print_buffer_range(__MM_HIGHEST_ADDR, 0, "Total Memory");

    print_buffer_range(0, 0, NULL);


    print_buffer_range(__MM_FB0_Y_START_ADDR, __MM_FB0_Y_LEN, "FB0_Y");
    print_buffer_range(__MM_FB0_C_START_ADDR, __MM_FB0_C_LEN, "FB0_C");
    print_buffer_range(__MM_FB1_Y_START_ADDR, __MM_FB1_Y_LEN, "FB1_Y");
    print_buffer_range(__MM_FB1_C_START_ADDR, __MM_FB1_C_LEN, "FB1_C");
    print_buffer_range(__MM_FB2_Y_START_ADDR, __MM_FB2_Y_LEN, "FB2_Y");
    print_buffer_range(__MM_FB2_C_START_ADDR, __MM_FB2_C_LEN, "FB2_C");

    print_buffer_range(__MM_MP_BUFFER_ADDR, __MM_MP_BUFFER_LEN, "MP_BUFF");
    print_buffer_range(__MM_VIDEO_FILE_BUF_ADDR, __MM_VIDEO_FILE_BUF_LEN, "video_file_buff");

    libc_printf("SYS_DRAM_SIZE=%d\n", SYS_SDRAM_SIZE);
}
#else
#if (defined _M3505_ || defined _M3702_)
static MCFG mc[] =
{
    {0, 0, "Code"},
    {0, 0, "Heap"},
    {__MM_CPU_DBG_MEM_ADDR, __MM_DBG_MEM_LEN, "CPU debug buffer "},
    {__MM_ISDBTCC_PB_START_ADDR,__MM_ISDBTCC_PB_LEN , "__MM_ISDBTCC_PB_START_ADDR "},
    {__MM_ISDBTCC_BS_START_ADDR,__MM_ISDBTCC_BS_LEN , "__MM_ISDBTCC_BS_START_ADDR "},
    {__MM_SUBT_ATSC_SEC_ADDR,__MM_SUBT_ATSC_SEC_LEN , "SUB_ATSC_ADDR"},
    {__MM_DCII_SUB_BS_START_ADDR, __MM_DCII_SUB_BS_LEN, "DCII"},
    {__MM_SUB_PB_START_ADDR,__MM_SUB_PB_LEN , "SUB_PB_START "},
    {__MM_SUB_HW_DATA_ADDR,__MM_SUB_HW_DATA_LEN , "__MM_SUB_HW_DATA_ADDR "},
    {__MM_SUB_BS_START_ADDR,__MM_SUB_BS_LEN , "SUB_BS_START_ "},
    {__MM_ATSC_CC_PB_START_ADDR,__MM_ATSC_CC_PB_LEN , "__MM_ATSC_CC_PB_START_ADDR "},
    {__MM_ATSC_CC_BS_START_ADDR,__MM_ATSC_CC_BS_LEN , "__MM_ATSC_CC_BS_START_ADDR "},
    {__MM_TTX_PB_START_ADDR, __MM_TTX_PB_LEN, "TTX_PB_START "},
    {__MM_TTX_BS_START_ADDR, __MM_TTX_BS_LEN, "TTX_BS_START "},
    {__MM_TTX_P26_DATA_BUF_ADDR, __MM_TTX_P26_DATA_LEN, "TTX_P26_DATA_ "},
    {__MM_TTX_P26_NATION_BUF_ADDR,__MM_TTX_P26_NATION_LEN , " TTX_P26_NATION"},
    {__MM_TTX_SUB_PAGE_BUF_ADDR, __MM_TTX_SUB_PAGE_LEN, "TTX_SUB_PAGE "},
    {__MM_USB_START_ADDR, __MM_USB_DMA_LEN,"USB_START_"},
    {__MM_LWIP_MEMP_ADDR, __MM_LWIP_MEMP_LEN,"__MM_LWIP_MEMP_ADDR"},
    {__MM_LWIP_MEM_ADDR, __MM_LWIP_MEM_LEN,"__MM_LWIP_MEM_ADDR"},
    {__MM_OSD_VSRC_MEM_ADDR, OSD_VSRC_MEM_MAX_SIZE, "__MM_OSD_VSRC_MEM_ADDR "},
    {__MM_VCAP_FB_ADDR, __MM_VCAP_FB_SIZE, "__MM_VCAP_FB_ADDR "},
    {__MM_OSD_BK_ADDR1, __MM_OSD1_LEN, "__MM_OSD_BK_ADDR1 "},
    {__MM_GE_START_ADDR, __MM_GE_LEN, "__MM_GE_START_ADDR "},
    {__MM_EPG_BUFFER_START ,__MM_EPG_BUFFER_LEN, "EPG_BUFFER "},
    {__MM_DMX_CPU_BLK_ADDR, __MM_DMX_BLK_BUF_LEN,"__MM_DMX_CPU_BLK_ADDR"},
    {__MM_DMX_REC_START_ADDR, __MM_DMX_REC_LEN,"__MM_DMX_REC_START_ADDR"},
    {__MM_DMX_AVP_START_ADDR,__MM_SI_VBV_OFFSET + __MM_DMX_DATA_LEN + __MM_DMX_AVP_LEN , "DMX AVP "},
    {__MM_AUTOSCAN_DB_BUFFER_ADDR, __MM_AUTOSCAN_DB_BUFFER_LEN,"__MM_AUTOSCAN_DB_BUFFER_ADDR"},
    {__MM_PVR_VOB_BUFFER_ADDR,__MM_PVR_VOB_BUFFER_LEN , "VOB cache"},
    {AVC_LAF_RW_BUF_ADDR,AVC_LAF_RW_BUF_LEN , "AVC_LAF_RW_BUF_ADDR "},
    {AVC_MB_NEI_ADDR, AVC_MB_NEI_LEN, "AVC_MB_NEI_ADDR "},
    {AVC_MB_COL_ADDR, AVC_MB_COL_LEN, " AVC_MB_COL_ADDR"},
    {AVC_MV_ADDR, AVC_MV_LEN, "AVC_MV_ADDR "},
    {AVC_DVIEW_ADDR,AVC_DVIEW_LEN , "AVC_DVIEW_ADDR "},
    {AVC_FB_ADDR, AVC_FB_LEN, "AVC_FB_ADDR "},
    {HEVC_INNER_AUX_ADDR,HEVC_INNER_AUX_LEN , "HEVC_INNER_AUX_ADDR "},
    {HEVC_INNER_ES_ADDR, HEVC_INNER_ES_LEN, "HEVC_INNER_ES_ADDR "},
    {HEVC_LAF_ADDR, HEVC_LAF_LEN, " HEVC_LAF_ADDR"},
    {HEVC_MB_NEI_ADDR, HEVC_MB_NEI_LEN, "HEVC_MB_NEI_ADDR "},
    {HEVC_MV_ADDR,HEVC_MV_LEN , "HEVC_MV_ADDR "},
    {HEVC_DVIEW_ADDR, HEVC_DVIEW_LEN, "HEVC_DVIEW_ADDR "},
    {HEVC_FB_ADDR, HEVC_FB_LEN, "HEVC_FB_ADDR "},
    {__MM_FB_TOP_ADDR, 0, "Frame buffer top memory"},
    {__MM_VIDEO_FILE_BUF_ADDR, __MM_VIDEO_FILE_BUF_LEN, "Media buffer"},      
};

static MCFG sc[] =
{
    {__MM_SEE_DBG_MEM_ADDR,__MM_DBG_MEM_LEN , "SEE debug"},
    {__MM_OSD_BK_ADDR2, __MM_OSD2_LEN, "__MM_OSD_BK_ADDR2 "},
    {AVC_LAF_FLAG_BUF_ADDR, AVC_LAF_FLAG_BUF_LEN, "AVC_LAF_FLAG_BUF_ADDR "},
    {AVC_CMD_QUEUE_ADDR, AVC_CMD_QUEUE_LEN, "AVC_CMD_QUEUE_ADDR "},
    {HEVC_CMD_QUEUE_BUF_ADDR, HEVC_CMD_QUEUE_LEN, "HEVC_CMD_QUEUE_BUF_ADDR "},
    {HEVC_EP_QUEUE_BUF_ADDR, HEVC_EP_QUEUE_LEN, "HEVC_EP_QUEUE_BUF_ADDR "},
    {HEVC_PICTURE_SYNTAX_BUF_ADDR, HEVC_PICTURE_SYNTAX_LEN, "HEVC_PICTURE_SYNTAX_BUF_ADDR "},
    {HEVC_VBV_ADDR, HEVC_VBV_LEN, "HEVC_VBV_ADDR "},
    {__MM_SEE_MP_BUF_ADDR,__MM_SEE_MP_BUF_LEN , " __MM_SEE_MP_BUF_ADDR"},
    {__MM_DMX_SEE_BLK_ADDR,__MM_DMX_BLK_BUF_LEN , "__MM_DMX_SEE_BLK_ADDR "},
    {AVC_VBV_ADDR,AVC_VBV_LEN , "AVC_VBV_ADDR "},
    {__MM_MAF_START_ADDR,__MM_MAF_LEN , "__MM_MAF_START_ADDR "},
    {__MM_VBV_START_ADDR,__MM_VBV_LEN , "__MM_VBV_START_ADDR "},
};

void print_memory_layout_dual_sort()
{
    UINT8 i,n,j,k;
    MCFG tmp;
    
    extern UINT heap_start;
    //UDG_PRINTF("\n======== Memory Layout ==========\n");
    mc[0].size=heap_start;
    mc[1].address=heap_start;
    mc[1].size= (__MM_HEAP_TOP_ADDR&0x0fffffff)-(heap_start&0x0fffffff);
    n = sizeof(mc)/sizeof(MCFG);
    for(i=0;i<n;i++)
        mc[i].address &= 0x8FFFFFFF;
    for(j=0;j<n;j++)
    {
        for(i=0;i<n-j;i++)
        {
            if(mc[i].address>mc[i+1].address)
            {
                memcpy(&tmp,&mc[i],sizeof(MCFG));
                memcpy(&mc[i],&mc[i+1],sizeof(MCFG));
                memcpy(&mc[i+1],&tmp,sizeof(MCFG));
            }
        }
    }
    libc_printf("\n\nmain memory mapping\n\n");
    for(i=0;i<n;i++)
    {
        if(mc[i].size!=0)
            print_range(&mc[i]);
    }


    n = sizeof(sc)/sizeof(MCFG);
    for(i=0;i<n;i++)
        sc[i].address &= 0x8FFFFFFF;
    for(j=0;j<n;j++)
    {
        for(i=0;i<n-j;i++)
        {
            if(sc[i].address>sc[i+1].address)
            {
                memcpy(&tmp,&sc[i],sizeof(MCFG));
                memcpy(&sc[i],&sc[i+1],sizeof(MCFG));
                memcpy(&sc[i+1],&tmp,sizeof(MCFG));
            }
        }
    }
    libc_printf("\n\nprivate memory mapping\n\n");
    for(i=0;i<n;i++)
    {
        if(sc[i].size!=0)
            print_range(&sc[i]);
    }
    

    libc_printf("\n\nshare memory mapping\n\n");

    print_buffer_range(__MM_SHARE_BASE_ADDR,__MM_SHARED_MEM_LEN , "__MM_SHARE_BASE_ADDR ");
    print_buffer_range(__MM_VOID_BUFFER_ADDR, __MM_VOID_BUFFER_LEN , "void buffer to customer");

    print_buffer_range(__MM_HIGHEST_ADDR, 0, "Total Memory");

    print_buffer_range(0, 0, NULL);

}
//3505
void print_memory_layout_dual()
{
    extern UINT heap_start;
    //UDG_PRINTF("\n======== Memory Layout ==========\n");

    libc_printf("\n\nmain memory mapping\n\n");

    print_buffer_range(0, heap_start, "Code");
    print_buffer_range(heap_start, (__MM_HEAP_TOP_ADDR&0x0fffffff)-(heap_start&0x0fffffff), "Heap");

    print_buffer_range(__MM_CPU_DBG_MEM_ADDR, __MM_DBG_MEM_LEN, "CPU debug buffer ");

    print_buffer_range(__MM_ISDBTCC_PB_START_ADDR,__MM_ISDBTCC_PB_LEN , "__MM_ISDBTCC_PB_START_ADDR ");
    print_buffer_range(__MM_ISDBTCC_BS_START_ADDR,__MM_ISDBTCC_BS_LEN , "__MM_ISDBTCC_BS_START_ADDR ");
    print_buffer_range(__MM_SUBT_ATSC_SEC_ADDR,__MM_SUBT_ATSC_SEC_LEN , "SUB_ATSC_ADDR");
    print_buffer_range(__MM_DCII_SUB_BS_START_ADDR, __MM_DCII_SUB_BS_LEN, "DCII");

    print_buffer_range(__MM_SUB_PB_START_ADDR,__MM_SUB_PB_LEN , "SUB_PB_START ");
    print_buffer_range(__MM_SUB_HW_DATA_ADDR,__MM_SUB_HW_DATA_LEN , "__MM_SUB_HW_DATA_ADDR ");
    print_buffer_range(__MM_SUB_BS_START_ADDR,__MM_SUB_BS_LEN , "SUB_BS_START_ ");

    print_buffer_range(__MM_ATSC_CC_PB_START_ADDR,__MM_ATSC_CC_PB_LEN , "__MM_ATSC_CC_PB_START_ADDR ");
    print_buffer_range(__MM_ATSC_CC_BS_START_ADDR,__MM_ATSC_CC_BS_LEN , "__MM_ATSC_CC_BS_START_ADDR ");

    print_buffer_range(__MM_TTX_PB_START_ADDR, __MM_TTX_PB_LEN, "TTX_PB_START ");
    print_buffer_range(__MM_TTX_BS_START_ADDR, __MM_TTX_BS_LEN, "TTX_BS_START ");
    print_buffer_range(__MM_TTX_P26_DATA_BUF_ADDR, __MM_TTX_P26_DATA_LEN, "TTX_P26_DATA_ ");
    print_buffer_range(__MM_TTX_P26_NATION_BUF_ADDR,__MM_TTX_P26_NATION_LEN , " TTX_P26_NATION");
    print_buffer_range(__MM_TTX_SUB_PAGE_BUF_ADDR, __MM_TTX_SUB_PAGE_LEN, "TTX_SUB_PAGE ");

    print_buffer_range(__MM_USB_START_ADDR, __MM_USB_DMA_LEN,"USB_START_");

    print_buffer_range(__MM_LWIP_MEMP_ADDR, __MM_LWIP_MEMP_LEN,"__MM_LWIP_MEMP_ADDR");
    print_buffer_range(__MM_LWIP_MEM_ADDR, __MM_LWIP_MEM_LEN,"__MM_LWIP_MEM_ADDR");
    print_buffer_range(__MM_OSD_VSRC_MEM_ADDR, OSD_VSRC_MEM_MAX_SIZE, "__MM_OSD_VSRC_MEM_ADDR ");
    print_buffer_range(__MM_VCAP_FB_ADDR, __MM_VCAP_FB_SIZE, "__MM_VCAP_FB_ADDR ");
    print_buffer_range(__MM_OSD_BK_ADDR1, __MM_OSD1_LEN, "__MM_OSD_BK_ADDR1 ");

    print_buffer_range(__MM_GE_START_ADDR, __MM_GE_LEN, "__MM_GE_START_ADDR ");
    print_buffer_range(__MM_EPG_BUFFER_START ,__MM_EPG_BUFFER_LEN, "EPG_BUFFER ");
    print_buffer_range(__MM_DMX_CPU_BLK_ADDR, __MM_DMX_BLK_BUF_LEN,"__MM_DMX_CPU_BLK_ADDR");
    print_buffer_range(__MM_DMX_REC_START_ADDR, __MM_DMX_REC_LEN,"__MM_DMX_REC_START_ADDR");
    print_buffer_range(__MM_DMX_AVP_START_ADDR,__MM_SI_VBV_OFFSET + __MM_DMX_DATA_LEN + __MM_DMX_AVP_LEN , "DMX AVP ");

    print_buffer_range(__MM_AUTOSCAN_DB_BUFFER_ADDR, __MM_AUTOSCAN_DB_BUFFER_LEN,"__MM_AUTOSCAN_DB_BUFFER_ADDR");
    print_buffer_range(__MM_PVR_VOB_BUFFER_ADDR,__MM_PVR_VOB_BUFFER_LEN , "VOB cache");

    print_buffer_range(AVC_LAF_RW_BUF_ADDR,AVC_LAF_RW_BUF_LEN , "AVC_LAF_RW_BUF_ADDR ");
    print_buffer_range(AVC_MB_NEI_ADDR, AVC_MB_NEI_LEN, "AVC_MB_NEI_ADDR ");
    print_buffer_range(AVC_MB_COL_ADDR, AVC_MB_COL_LEN, " AVC_MB_COL_ADDR");
    print_buffer_range(AVC_MV_ADDR, AVC_MV_LEN, "AVC_MV_ADDR ");
    print_buffer_range(AVC_DVIEW_ADDR,AVC_DVIEW_LEN , "AVC_DVIEW_ADDR ");
    print_buffer_range(AVC_FB_ADDR, AVC_FB_LEN, "AVC_FB_ADDR ");


    print_buffer_range(HEVC_INNER_AUX_ADDR,HEVC_INNER_AUX_LEN , "HEVC_INNER_AUX_ADDR ");
    print_buffer_range(HEVC_INNER_ES_ADDR, HEVC_INNER_ES_LEN, "HEVC_INNER_ES_ADDR ");
    print_buffer_range(HEVC_LAF_ADDR, HEVC_LAF_LEN, " HEVC_LAF_ADDR");
    print_buffer_range(HEVC_MB_NEI_ADDR, HEVC_MB_NEI_LEN, "HEVC_MB_NEI_ADDR ");
    print_buffer_range(HEVC_MV_ADDR,HEVC_MV_LEN , "HEVC_MV_ADDR ");
    print_buffer_range(HEVC_DVIEW_ADDR, HEVC_DVIEW_LEN, "HEVC_DVIEW_ADDR ");
    print_buffer_range(HEVC_FB_ADDR, HEVC_FB_LEN, "HEVC_FB_ADDR ");

    
    print_buffer_range(__MM_FB_TOP_ADDR, 0, "Frame buffer top memory");
    print_buffer_range(__MM_VIDEO_FILE_BUF_ADDR, __MM_VIDEO_FILE_BUF_LEN, "Media buffer");

    libc_printf("\n\nprivate memory mapping\n\n");
#ifdef DUAL_ENABLE
    print_buffer_range(__MM_SEE_DBG_MEM_ADDR,__MM_DBG_MEM_LEN , "SEE debug");
#endif
    print_buffer_range(__MM_OSD_BK_ADDR2, __MM_OSD2_LEN, "__MM_OSD_BK_ADDR2 ");
    print_buffer_range(AVC_LAF_FLAG_BUF_ADDR, AVC_LAF_FLAG_BUF_LEN, "AVC_LAF_FLAG_BUF_ADDR ");
    print_buffer_range(AVC_CMD_QUEUE_ADDR, AVC_CMD_QUEUE_LEN, "AVC_CMD_QUEUE_ADDR ");


    print_buffer_range(HEVC_CMD_QUEUE_BUF_ADDR, HEVC_CMD_QUEUE_LEN, "HEVC_CMD_QUEUE_BUF_ADDR ");
    print_buffer_range(HEVC_EP_QUEUE_BUF_ADDR, HEVC_EP_QUEUE_LEN, "HEVC_EP_QUEUE_BUF_ADDR ");
    print_buffer_range(HEVC_PICTURE_SYNTAX_BUF_ADDR, HEVC_PICTURE_SYNTAX_LEN, "HEVC_PICTURE_SYNTAX_BUF_ADDR ");
    print_buffer_range(HEVC_VBV_ADDR, HEVC_VBV_LEN, "HEVC_VBV_ADDR ");


#ifdef DUAL_ENABLE
    print_buffer_range(__MM_SEE_MP_BUF_ADDR,__MM_SEE_MP_BUF_LEN , " __MM_SEE_MP_BUF_ADDR");
#endif


    print_buffer_range(__MM_DMX_SEE_BLK_ADDR,__MM_DMX_BLK_BUF_LEN , "__MM_DMX_SEE_BLK_ADDR ");
    print_buffer_range(AVC_VBV_ADDR,AVC_VBV_LEN , "AVC_VBV_ADDR ");


    
    print_buffer_range(__MM_MAF_START_ADDR,__MM_MAF_LEN , "__MM_MAF_START_ADDR ");
    print_buffer_range(__MM_VBV_START_ADDR,__MM_VBV_LEN , "__MM_VBV_START_ADDR ");

    libc_printf("\n\nshare memory mapping\n\n");
#ifdef DUAL_ENABLE
    print_buffer_range(__MM_SHARE_BASE_ADDR,__MM_SHARED_MEM_LEN , "__MM_SHARE_BASE_ADDR ");
    print_buffer_range(__MM_VOID_BUFFER_ADDR, __MM_VOID_BUFFER_LEN , "void buffer to customer");
#endif
    print_buffer_range(__MM_HIGHEST_ADDR, 0, "Total Memory");

    print_buffer_range(0, 0, NULL);

}

#ifdef  VFB_SUPPORT
//3505 VFB
void print_memory_layout_dual_3505_vfb()
{
    extern UINT heap_start;
    //UDG_PRINTF("\n======== Memory Layout ==========\n");

    libc_printf("\n\nmain memory mapping\n\n");

    print_buffer_range(0, heap_start, "Code");
    print_buffer_range(heap_start, (__MM_HEAP_TOP_ADDR&0x0fffffff)-(heap_start&0x0fffffff), "Heap");

    print_buffer_range(__MM_CPU_DBG_MEM_ADDR, __MM_DBG_MEM_LEN, "CPU debug buffer ");

    print_buffer_range(__MM_ISDBTCC_PB_START_ADDR,__MM_ISDBTCC_PB_LEN , "__MM_ISDBTCC_PB_START_ADDR ");
    print_buffer_range(__MM_ISDBTCC_BS_START_ADDR,__MM_ISDBTCC_BS_LEN , "__MM_ISDBTCC_BS_START_ADDR ");
    print_buffer_range(__MM_SUBT_ATSC_SEC_ADDR,__MM_SUBT_ATSC_SEC_LEN , "SUB_ATSC_ADDR");
    print_buffer_range(__MM_DCII_SUB_BS_START_ADDR, __MM_DCII_SUB_BS_LEN, "DCII");

    print_buffer_range(__MM_SUB_PB_START_ADDR,__MM_SUB_PB_LEN , "SUB_PB_START ");
    print_buffer_range(__MM_SUB_HW_DATA_ADDR,__MM_SUB_HW_DATA_LEN , "__MM_SUB_HW_DATA_ADDR ");
    print_buffer_range(__MM_SUB_BS_START_ADDR,__MM_SUB_BS_LEN , "SUB_BS_START_ ");

    print_buffer_range(__MM_ATSC_CC_PB_START_ADDR,__MM_ATSC_CC_PB_LEN , "__MM_ATSC_CC_PB_START_ADDR ");
    print_buffer_range(__MM_ATSC_CC_BS_START_ADDR,__MM_ATSC_CC_BS_LEN , "__MM_ATSC_CC_BS_START_ADDR ");

    print_buffer_range(__MM_TTX_PB_START_ADDR, __MM_TTX_PB_LEN, "TTX_PB_START ");
    print_buffer_range(__MM_TTX_BS_START_ADDR, __MM_TTX_BS_LEN, "TTX_BS_START ");
    print_buffer_range(__MM_TTX_P26_DATA_BUF_ADDR, __MM_TTX_P26_DATA_LEN, "TTX_P26_DATA_ ");
    print_buffer_range(__MM_TTX_P26_NATION_BUF_ADDR,__MM_TTX_P26_NATION_LEN , " TTX_P26_NATION");
    print_buffer_range(__MM_TTX_SUB_PAGE_BUF_ADDR, __MM_TTX_SUB_PAGE_LEN, "TTX_SUB_PAGE ");

    print_buffer_range(__MM_USB_START_ADDR, __MM_USB_DMA_LEN,"USB_START_");

    print_buffer_range(__MM_LWIP_MEMP_ADDR, __MM_LWIP_MEMP_LEN,"__MM_LWIP_MEMP_ADDR");
    print_buffer_range(__MM_LWIP_MEM_ADDR, __MM_LWIP_MEM_LEN,"__MM_LWIP_MEM_ADDR");
    print_buffer_range(__MM_OSD_VSRC_MEM_ADDR, OSD_VSRC_MEM_MAX_SIZE, "__MM_OSD_VSRC_MEM_ADDR ");
    print_buffer_range(__MM_OSD_BK_ADDR2_MAIN, __MM_OSD2_LEN, "__MM_OSD_BK_ADDR2 ");    
    print_buffer_range(__MM_OSD_BK_ADDR1, __MM_OSD1_LEN, "__MM_OSD_BK_ADDR1 ");

    print_buffer_range(__MM_GE_START_ADDR, __MM_GE_LEN, "__MM_GE_START_ADDR ");
    print_buffer_range(__MM_EPG_BUFFER_START ,__MM_EPG_BUFFER_LEN, "EPG_BUFFER ");
    print_buffer_range(__MM_DMX_CPU_BLK_ADDR, __MM_DMX_BLK_BUF_LEN,"__MM_DMX_CPU_BLK_ADDR");
    print_buffer_range(__MM_DMX_REC_START_ADDR, __MM_DMX_REC_LEN,"__MM_DMX_REC_START_ADDR");
    print_buffer_range(__MM_DMX_AVP_START_ADDR,__MM_SI_VBV_OFFSET + __MM_DMX_DATA_LEN + __MM_DMX_AVP_LEN , "DMX AVP ");

    print_buffer_range(__MM_AUTOSCAN_DB_BUFFER_ADDR, __MM_AUTOSCAN_DB_BUFFER_LEN,"__MM_AUTOSCAN_DB_BUFFER_ADDR");
    print_buffer_range(__MM_PVR_VOB_BUFFER_ADDR,__MM_PVR_VOB_BUFFER_LEN , "VOB cache");
 
    libc_printf("---FB for HEVC\n");    
    print_buffer_range(HEVC_FB_ADDR, HEVC_FB_LEN, "HEVC_FB_ADDR "); 
    print_buffer_range(HEVC_DVIEW_ADDR, HEVC_DVIEW_LEN, "HEVC_DVIEW_ADDR ");
    print_buffer_range(HEVC_CMD_QUEUE_BUF_ADDR,HEVC_CMD_QUEUE_LEN , "HEVC_CMD_QUEUE_BUF_ADDR ");
    print_buffer_range(HEVC_EP_QUEUE_BUF_ADDR,HEVC_EP_QUEUE_LEN , "HEVC_EP_QUEUE_BUF_ADDR ");
    print_buffer_range(HEVC_PICTURE_SYNTAX_BUF_ADDR,HEVC_PICTURE_SYNTAX_LEN , "HEVC_PICTURE_SYNTAX_BUF_ADDR ");
    print_buffer_range(HEVC_INNER_AUX_ADDR,HEVC_INNER_AUX_LEN , "HEVC_INNER_AUX_ADDR ");
    print_buffer_range(HEVC_INNER_ES_ADDR, HEVC_INNER_ES_LEN, "HEVC_INNER_ES_ADDR ");
    print_buffer_range(HEVC_LAF_ADDR, HEVC_LAF_LEN, " HEVC_LAF_ADDR");
    print_buffer_range(HEVC_MB_NEI_ADDR, HEVC_MB_NEI_LEN, "HEVC_MB_NEI_ADDR ");
    print_buffer_range(HEVC_MV_ADDR,HEVC_MV_LEN , "HEVC_MV_ADDR ");     

    libc_printf("---FB for AVC\n");
    print_buffer_range(AVC_FB_ADDR, AVC_FB_LEN, "AVC_FB_ADDR ");
    print_buffer_range(AVC_DVIEW_ADDR,AVC_DVIEW_LEN , "AVC_DVIEW_ADDR ");    
    print_buffer_range(AVC_LAF_FLAG_BUF_ADDR,AVC_LAF_FLAG_BUF_LEN , "AVC_LAF_FLAG_BUF_ADDR ");
    print_buffer_range(AVC_CMD_QUEUE_ADDR,AVC_CMD_QUEUE_LEN , "AVC_CMD_QUEUE_ADDR ");
    print_buffer_range(AVC_LAF_RW_BUF_ADDR,AVC_LAF_RW_BUF_LEN , "AVC_LAF_RW_BUF_ADDR ");
    print_buffer_range(AVC_MB_NEI_ADDR, AVC_MB_NEI_LEN, "AVC_MB_NEI_ADDR ");
    print_buffer_range(AVC_MB_COL_ADDR, AVC_MB_COL_LEN, " AVC_MB_COL_ADDR");
    print_buffer_range(AVC_MV_ADDR, AVC_MV_LEN, "AVC_MV_ADDR ");    

    libc_printf("---FB for MPEG2\n");  
    print_buffer_range(__MM_FB_START_ADDR, __MM_FB_LEN, "MPEG2_FB_ADDR ");
    print_buffer_range(__MM_MAF_START_ADDR, __MM_MAF_LEN, "__MM_MAF_START_ADDR ");
    

    libc_printf("---FB End\n");
    print_buffer_range(__MM_FB_TOP_ADDR, 0, "Frame buffer top memory");
    print_buffer_range(__MM_VCAP_FB_ADDR, __MM_VCAP_FB_SIZE, "__MM_VCAP_FB_ADDR ");
    //print_buffer_range(__MM_MAIN_TOP_ADDR, 0, "__MM_MAIN_TOP_ADDR ");

    print_buffer_range(0, 0, NULL);
    

    libc_printf("\n\nprivate memory mapping\n\n");
#ifdef DUAL_ENABLE
    print_buffer_range(__MM_SEE_DBG_MEM_ADDR,__MM_DBG_MEM_LEN , "SEE debug");
#endif
    print_buffer_range(__MM_OSD_BK_ADDR2, __MM_OSD2_LEN, "__MM_OSD_BK_ADDR2 ");    

#ifdef DUAL_ENABLE
    print_buffer_range(__MM_SEE_MP_BUF_ADDR,__MM_SEE_MP_BUF_LEN , " __MM_SEE_MP_BUF_ADDR");
#endif

    print_buffer_range(__MM_DMX_SEE_BLK_ADDR,__MM_DMX_BLK_BUF_LEN , "__MM_DMX_SEE_BLK_ADDR ");
    
    libc_printf("---VBV Start\n");    
    print_buffer_range(__MM_VBV_START_ADDR,__MM_VBV_LEN , "__MM_VBV_START_ADDR ");
    print_buffer_range(AVC_VBV_ADDR,AVC_VBV_LEN , "AVC_VBV_ADDR ");
    print_buffer_range(HEVC_VBV_ADDR, HEVC_VBV_LEN, "HEVC_VBV_ADDR ");
    
    print_buffer_range(0, 0, NULL);

    libc_printf("\n\nshare memory mapping\n\n");
#ifdef DUAL_ENABLE
    print_buffer_range(__MM_SHARE_BASE_ADDR,__MM_SHARED_MEM_LEN , "__MM_SHARE_BASE_ADDR ");
    print_buffer_range(__MM_VOID_BUFFER_ADDR, __MM_VOID_BUFFER_LEN , "void buffer to customer");
#endif
    print_buffer_range(__MM_HIGHEST_ADDR, 0, "Total Memory");

    print_buffer_range(0, 0, NULL);

    libc_printf("\n\nSpecial Item\n\n");
    print_buffer_range(__MM_VIDEO_FILE_BUF_ADDR, __MM_VIDEO_FILE_BUF_LEN, "Media buffer");
    
    print_buffer_range(0, 0, NULL);

}

//3702 VFB
void print_memory_layout_dual_3702_vfb()
{
    extern UINT heap_start;
    //UDG_PRINTF("\n======== Memory Layout ==========\n");

    libc_printf("\n\nmain memory mapping\n\n");

    print_buffer_range(0x80100000, heap_start-0x80100000, "Code");
    print_buffer_range(heap_start, (__MM_HEAP_TOP_ADDR&0x0fffffff)-(heap_start&0x0fffffff), "Heap");

    print_buffer_range(__MM_CPU_DBG_MEM_ADDR, __MM_DBG_MEM_LEN, "CPU debug buffer ");

    print_buffer_range(__MM_ISDBTCC_PB_START_ADDR,__MM_ISDBTCC_PB_LEN , "__MM_ISDBTCC_PB_START_ADDR ");
    print_buffer_range(__MM_ISDBTCC_BS_START_ADDR,__MM_ISDBTCC_BS_LEN , "__MM_ISDBTCC_BS_START_ADDR ");
    print_buffer_range(__MM_SUBT_ATSC_SEC_ADDR,__MM_SUBT_ATSC_SEC_LEN , "SUB_ATSC_ADDR");
    print_buffer_range(__MM_DCII_SUB_BS_START_ADDR, __MM_DCII_SUB_BS_LEN, "DCII");

    print_buffer_range(__MM_SUB_PB_START_ADDR,__MM_SUB_PB_LEN , "SUB_PB_START ");
    print_buffer_range(__MM_SUB_HW_DATA_ADDR,__MM_SUB_HW_DATA_LEN , "__MM_SUB_HW_DATA_ADDR ");
    print_buffer_range(__MM_SUB_BS_START_ADDR,__MM_SUB_BS_LEN , "SUB_BS_START_ ");

    print_buffer_range(__MM_ATSC_CC_PB_START_ADDR,__MM_ATSC_CC_PB_LEN , "__MM_ATSC_CC_PB_START_ADDR ");
    print_buffer_range(__MM_ATSC_CC_BS_START_ADDR,__MM_ATSC_CC_BS_LEN , "__MM_ATSC_CC_BS_START_ADDR ");

    print_buffer_range(__MM_TTX_PB_START_ADDR, __MM_TTX_PB_LEN, "TTX_PB_START ");
    print_buffer_range(__MM_TTX_BS_START_ADDR, __MM_TTX_BS_LEN, "TTX_BS_START ");
    print_buffer_range(__MM_TTX_P26_DATA_BUF_ADDR, __MM_TTX_P26_DATA_LEN, "TTX_P26_DATA_ ");
    print_buffer_range(__MM_TTX_P26_NATION_BUF_ADDR,__MM_TTX_P26_NATION_LEN , " TTX_P26_NATION");
    print_buffer_range(__MM_TTX_SUB_PAGE_BUF_ADDR, __MM_TTX_SUB_PAGE_LEN, "TTX_SUB_PAGE ");

    print_buffer_range(__MM_USB_START_ADDR, __MM_USB_DMA_LEN,"USB_START_");

    print_buffer_range(__MM_LWIP_MEMP_ADDR, __MM_LWIP_MEMP_LEN,"__MM_LWIP_MEMP_ADDR");
    print_buffer_range(__MM_LWIP_MEM_ADDR, __MM_LWIP_MEM_LEN,"__MM_LWIP_MEM_ADDR");
    print_buffer_range(__MM_OSD_VSRC_MEM_ADDR, OSD_VSRC_MEM_MAX_SIZE, "__MM_OSD_VSRC_MEM_ADDR ");
    print_buffer_range(__MM_OSD_BK_ADDR2_MAIN, __MM_OSD2_LEN, "__MM_OSD_BK_ADDR2 ");    
    print_buffer_range(__MM_OSD_BK_ADDR1, __MM_OSD1_LEN, "__MM_OSD_BK_ADDR1 ");

    print_buffer_range(__MM_GE_START_ADDR, __MM_GE_LEN, "__MM_GE_START_ADDR ");
    print_buffer_range(__MM_EPG_BUFFER_START ,__MM_EPG_BUFFER_LEN, "EPG_BUFFER ");
    print_buffer_range(__MM_DMX_CPU_BLK_ADDR, __MM_DMX_BLK_BUF_LEN,"__MM_DMX_CPU_BLK_ADDR");
    print_buffer_range(__MM_DMX_REC_START_ADDR, __MM_DMX_REC_LEN,"__MM_DMX_REC_START_ADDR");
    print_buffer_range(__MM_DMX_AVP_START_ADDR,__MM_SI_VBV_OFFSET + __MM_DMX_DATA_LEN + __MM_DMX_AVP_LEN , "DMX AVP ");

    print_buffer_range(__MM_AUTOSCAN_DB_BUFFER_ADDR, __MM_AUTOSCAN_DB_BUFFER_LEN,"__MM_AUTOSCAN_DB_BUFFER_ADDR");
    print_buffer_range(__MM_PVR_VOB_BUFFER_ADDR,__MM_PVR_VOB_BUFFER_LEN , "VOB cache");
    print_buffer_range(__MM_NIM_BUFFER_ADDR,__MM_NIM_BUFFER_LEN , "NIM buffer");
    print_buffer_range(__MM_SND_DMA_BUFFER_ADDR,SND_DMA_BUFFER_LEN, "SND_DMA_BUFFER");

    libc_printf("---VBV Start\n");    
    print_buffer_range(__MM_VBV_START_ADDR,__MM_VBV_LEN , "__MM_VBV_START_ADDR ");
    print_buffer_range(AVC_VBV_ADDR,AVC_VBV_LEN , "AVC_VBV_ADDR ");
    print_buffer_range(HEVC_VBV_ADDR, HEVC_VBV_LEN, "HEVC_VBV_ADDR ");
 
    libc_printf("---FB for HEVC\n");    
    print_buffer_range(HEVC_FB_ADDR, HEVC_FB_LEN, "HEVC_FB_ADDR "); 
    print_buffer_range(HEVC_DVIEW_ADDR, HEVC_DVIEW_LEN, "HEVC_DVIEW_ADDR ");
    print_buffer_range(HEVC_CMD_QUEUE_BUF_ADDR,HEVC_CMD_QUEUE_LEN , "HEVC_CMD_QUEUE_BUF_ADDR ");
    print_buffer_range(HEVC_EP_QUEUE_BUF_ADDR,HEVC_EP_QUEUE_LEN , "HEVC_EP_QUEUE_BUF_ADDR ");
    print_buffer_range(HEVC_PICTURE_SYNTAX_BUF_ADDR,HEVC_PICTURE_SYNTAX_LEN , "HEVC_PICTURE_SYNTAX_BUF_ADDR ");
    print_buffer_range(HEVC_INNER_AUX_ADDR,HEVC_INNER_AUX_LEN , "HEVC_INNER_AUX_ADDR ");
    print_buffer_range(HEVC_INNER_ES_ADDR, HEVC_INNER_ES_LEN, "HEVC_INNER_ES_ADDR ");
    print_buffer_range(HEVC_LAF_ADDR, HEVC_LAF_LEN, " HEVC_LAF_ADDR");
    print_buffer_range(HEVC_MB_NEI_ADDR, HEVC_MB_NEI_LEN, "HEVC_MB_NEI_ADDR ");
    print_buffer_range(HEVC_MV_ADDR,HEVC_MV_LEN , "HEVC_MV_ADDR ");     

    libc_printf("---FB for AVC\n");
    print_buffer_range(AVC_FB_ADDR, AVC_FB_LEN, "AVC_FB_ADDR ");
    print_buffer_range(AVC_DVIEW_ADDR,AVC_DVIEW_LEN , "AVC_DVIEW_ADDR ");    
    print_buffer_range(AVC_LAF_FLAG_BUF_ADDR,AVC_LAF_FLAG_BUF_LEN , "AVC_LAF_FLAG_BUF_ADDR ");
    print_buffer_range(AVC_CMD_QUEUE_ADDR,AVC_CMD_QUEUE_LEN , "AVC_CMD_QUEUE_ADDR ");
    print_buffer_range(AVC_LAF_RW_BUF_ADDR,AVC_LAF_RW_BUF_LEN , "AVC_LAF_RW_BUF_ADDR ");
    print_buffer_range(AVC_MB_NEI_ADDR, AVC_MB_NEI_LEN, "AVC_MB_NEI_ADDR ");
    print_buffer_range(AVC_MB_COL_ADDR, AVC_MB_COL_LEN, " AVC_MB_COL_ADDR");
    print_buffer_range(AVC_MV_ADDR, AVC_MV_LEN, "AVC_MV_ADDR ");    

    libc_printf("---FB for MPEG2\n");  
    print_buffer_range(__MM_FB_START_ADDR, __MM_FB_LEN, "MPEG2_FB_ADDR ");
    print_buffer_range(__MM_MAF_START_ADDR, __MM_MAF_LEN, "__MM_MAF_START_ADDR ");
    

    libc_printf("---FB End\n");
    print_buffer_range(__MM_FB_TOP_ADDR, 0, "Frame buffer top memory");
    print_buffer_range(__MM_VCAP_FB_ADDR, __MM_VCAP_FB_SIZE, "__MM_VCAP_FB_ADDR ");
    //print_buffer_range(__MM_MAIN_TOP_ADDR, 0, "__MM_MAIN_TOP_ADDR ");

    print_buffer_range(0, 0, NULL);
    

    libc_printf("\n\nprivate memory mapping\n\n");
#ifdef DUAL_ENABLE
    print_buffer_range(__MM_SEE_DBG_MEM_ADDR,__MM_DBG_MEM_LEN , "SEE debug");
#endif
    print_buffer_range(__MM_OSD_BK_ADDR2, __MM_OSD2_LEN, "__MM_OSD_BK_ADDR2 ");    

#ifdef DUAL_ENABLE
    print_buffer_range(__MM_SEE_MP_BUF_ADDR,__MM_SEE_MP_BUF_LEN , " __MM_SEE_MP_BUF_ADDR");
#endif

    print_buffer_range(__MM_DMX_SEE_BLK_ADDR,__MM_DMX_BLK_BUF_LEN , "__MM_DMX_SEE_BLK_ADDR ");
    print_buffer_range(__MM_PRIV_DEC_ADDR, __MM_PRIV_DEC_LEN, "PRIV_DEC");
 
    
    print_buffer_range(0, 0, NULL);

    libc_printf("\n\nshare memory mapping\n\n");
#ifdef DUAL_ENABLE
    print_buffer_range(__MM_SHARE_BASE_ADDR,__MM_SHARED_MEM_LEN , "__MM_SHARE_BASE_ADDR ");
    print_buffer_range(__MM_VOID_BUFFER_ADDR, __MM_VOID_BUFFER_LEN , "void buffer to customer");
#endif
    print_buffer_range(__MM_HIGHEST_ADDR, 0, "Total Memory");

    print_buffer_range(0, 0, NULL);

    libc_printf("\n\nSpecial Item\n\n");
    print_buffer_range(__MM_VIDEO_FILE_BUF_ADDR, __MM_VIDEO_FILE_BUF_LEN, "Media buffer");
    
    print_buffer_range(0, 0, NULL);

}

#endif
#elif ((defined(_M3503D_)||defined(_M3711C_)))
#if defined(VFB_SUPPORT)
//3503D VFB
void print_memory_layout_dual_3503d_vfb()
{
    extern UINT heap_start;
    //UDG_PRINTF("\n======== Memory Layout ==========\n");

    libc_printf("\n\nmain memory mapping\n\n");

    print_buffer_range(0, heap_start, "Code");
    print_buffer_range(heap_start, (__MM_HEAP_TOP_ADDR&0x0fffffff)-(heap_start&0x0fffffff), "Heap");

    print_buffer_range(__MM_CPU_DBG_MEM_ADDR, __MM_DBG_MEM_LEN, "CPU debug buffer ");

    print_buffer_range(__MM_ISDBTCC_PB_START_ADDR,__MM_ISDBTCC_PB_LEN , "__MM_ISDBTCC_PB_START_ADDR ");
    print_buffer_range(__MM_ISDBTCC_BS_START_ADDR,__MM_ISDBTCC_BS_LEN , "__MM_ISDBTCC_BS_START_ADDR ");
    print_buffer_range(__MM_SUBT_ATSC_SEC_ADDR,__MM_SUBT_ATSC_SEC_LEN , "SUB_ATSC_ADDR");
    print_buffer_range(__MM_DCII_SUB_BS_START_ADDR, __MM_DCII_SUB_BS_LEN, "DCII");

    print_buffer_range(__MM_SUB_PB_START_ADDR,__MM_SUB_PB_LEN , "SUB_PB_START ");
    print_buffer_range(__MM_SUB_HW_DATA_ADDR,__MM_SUB_HW_DATA_LEN , "__MM_SUB_HW_DATA_ADDR ");
    print_buffer_range(__MM_SUB_BS_START_ADDR,__MM_SUB_BS_LEN , "SUB_BS_START_ ");

    print_buffer_range(__MM_ATSC_CC_PB_START_ADDR,__MM_ATSC_CC_PB_LEN , "__MM_ATSC_CC_PB_START_ADDR ");
    print_buffer_range(__MM_ATSC_CC_BS_START_ADDR,__MM_ATSC_CC_BS_LEN , "__MM_ATSC_CC_BS_START_ADDR ");

    print_buffer_range(__MM_TTX_PB_START_ADDR, __MM_TTX_PB_LEN, "TTX_PB_START ");
    print_buffer_range(__MM_TTX_BS_START_ADDR, __MM_TTX_BS_LEN, "TTX_BS_START ");
    print_buffer_range(__MM_TTX_P26_DATA_BUF_ADDR, __MM_TTX_P26_DATA_LEN, "TTX_P26_DATA_ ");
    print_buffer_range(__MM_TTX_P26_NATION_BUF_ADDR,__MM_TTX_P26_NATION_LEN , " TTX_P26_NATION");
    print_buffer_range(__MM_TTX_SUB_PAGE_BUF_ADDR, __MM_TTX_SUB_PAGE_LEN, "TTX_SUB_PAGE ");

    print_buffer_range(__MM_USB_START_ADDR, __MM_USB_DMA_LEN,"USB_START_");

    print_buffer_range(__MM_LWIP_MEMP_ADDR, __MM_LWIP_MEMP_LEN,"__MM_LWIP_MEMP_ADDR");
    print_buffer_range(__MM_LWIP_MEM_ADDR, __MM_LWIP_MEM_LEN,"__MM_LWIP_MEM_ADDR");
    print_buffer_range(__MM_OSD_VSRC_MEM_ADDR, OSD_VSRC_MEM_MAX_SIZE, "__MM_OSD_VSRC_MEM_ADDR ");
    print_buffer_range(__MM_OSD_BK_ADDR2_MAIN, __MM_OSD2_LEN, "__MM_OSD_BK_ADDR2 ");    
    print_buffer_range(__MM_OSD_BK_ADDR1, __MM_OSD1_LEN, "__MM_OSD_BK_ADDR1 ");

    print_buffer_range(__MM_GE_START_ADDR, __MM_GE_LEN, "__MM_GE_START_ADDR ");
    print_buffer_range(__MM_EPG_BUFFER_START ,__MM_EPG_BUFFER_LEN, "EPG_BUFFER ");
    print_buffer_range(__MM_DMX_CPU_BLK_ADDR, __MM_DMX_BLK_BUF_LEN,"__MM_DMX_CPU_BLK_ADDR");
    print_buffer_range(__MM_DMX_REC_START_ADDR, __MM_DMX_REC_LEN,"__MM_DMX_REC_START_ADDR");
    print_buffer_range(__MM_DMX_AVP_START_ADDR,__MM_SI_VBV_OFFSET + __MM_DMX_DATA_LEN + __MM_DMX_AVP_LEN , "DMX AVP ");

    print_buffer_range(__MM_AUTOSCAN_DB_BUFFER_ADDR, __MM_AUTOSCAN_DB_BUFFER_LEN,"__MM_AUTOSCAN_DB_BUFFER_ADDR");
    print_buffer_range(__MM_PVR_VOB_BUFFER_ADDR,__MM_PVR_VOB_BUFFER_LEN , "VOB cache");
#if 0 
    libc_printf("---FB for HEVC\n");    
    print_buffer_range(HEVC_FB_ADDR, HEVC_FB_LEN, "HEVC_FB_ADDR "); 
    print_buffer_range(HEVC_DVIEW_ADDR, HEVC_DVIEW_LEN, "HEVC_DVIEW_ADDR ");
    print_buffer_range(HEVC_CMD_QUEUE_BUF_ADDR,HEVC_CMD_QUEUE_LEN , "HEVC_CMD_QUEUE_BUF_ADDR ");
    print_buffer_range(HEVC_EP_QUEUE_BUF_ADDR,HEVC_EP_QUEUE_LEN , "HEVC_EP_QUEUE_BUF_ADDR ");
    print_buffer_range(HEVC_PICTURE_SYNTAX_BUF_ADDR,HEVC_PICTURE_SYNTAX_LEN , "HEVC_PICTURE_SYNTAX_BUF_ADDR ");
    print_buffer_range(HEVC_INNER_AUX_ADDR,HEVC_INNER_AUX_LEN , "HEVC_INNER_AUX_ADDR ");
    print_buffer_range(HEVC_INNER_ES_ADDR, HEVC_INNER_ES_LEN, "HEVC_INNER_ES_ADDR ");
    print_buffer_range(HEVC_LAF_ADDR, HEVC_LAF_LEN, " HEVC_LAF_ADDR");
    print_buffer_range(HEVC_MB_NEI_ADDR, HEVC_MB_NEI_LEN, "HEVC_MB_NEI_ADDR ");
    print_buffer_range(HEVC_MV_ADDR,HEVC_MV_LEN , "HEVC_MV_ADDR ");     
#endif
    libc_printf("---FB for AVC\n");
    print_buffer_range(AVC_FB_ADDR, AVC_FB_LEN, "AVC_FB_ADDR ");
    print_buffer_range(AVC_DVIEW_ADDR,AVC_DVIEW_LEN , "AVC_DVIEW_ADDR ");    
    print_buffer_range(AVC_LAF_FLAG_BUF_ADDR,AVC_LAF_FLAG_BUF_LEN , "AVC_LAF_FLAG_BUF_ADDR ");
    print_buffer_range(AVC_CMD_QUEUE_ADDR,AVC_CMD_QUEUE_LEN , "AVC_CMD_QUEUE_ADDR ");
    print_buffer_range(AVC_LAF_RW_BUF_ADDR,AVC_LAF_RW_BUF_LEN , "AVC_LAF_RW_BUF_ADDR ");
    print_buffer_range(AVC_MB_NEI_ADDR, AVC_MB_NEI_LEN, "AVC_MB_NEI_ADDR ");
    print_buffer_range(AVC_MB_COL_ADDR, AVC_MB_COL_LEN, " AVC_MB_COL_ADDR");
    print_buffer_range(AVC_MV_ADDR, AVC_MV_LEN, "AVC_MV_ADDR ");    

    libc_printf("---FB for MPEG2\n");  
    print_buffer_range(__MM_FB_START_ADDR, __MM_FB_LEN, "MPEG2_FB_ADDR ");
    print_buffer_range(__MM_MAF_START_ADDR, __MM_MAF_LEN, "__MM_MAF_START_ADDR ");
    

    libc_printf("---FB End\n");
    print_buffer_range(__MM_FB_TOP_ADDR, 0, "Frame buffer top memory");
    print_buffer_range(__MM_VCAP_FB_ADDR, __MM_VCAP_FB_SIZE, "__MM_VCAP_FB_ADDR ");
    //print_buffer_range(__MM_MAIN_TOP_ADDR, 0, "__MM_MAIN_TOP_ADDR ");

    print_buffer_range(0, 0, NULL);
    

    libc_printf("\n\nprivate memory mapping\n\n");
#ifdef DUAL_ENABLE
    print_buffer_range(__MM_SEE_DBG_MEM_ADDR,__MM_DBG_MEM_LEN , "SEE debug");
#endif
    print_buffer_range(__MM_OSD_BK_ADDR2, __MM_OSD2_LEN, "__MM_OSD_BK_ADDR2 ");    

#ifdef DUAL_ENABLE
    print_buffer_range(__MM_SEE_MP_BUF_ADDR,__MM_SEE_MP_BUF_LEN , " __MM_SEE_MP_BUF_ADDR");
#endif

    print_buffer_range(__MM_DMX_SEE_BLK_ADDR,__MM_DMX_BLK_BUF_LEN , "__MM_DMX_SEE_BLK_ADDR ");
    
    libc_printf("---VBV Start\n");    
    print_buffer_range(__MM_VBV_START_ADDR,__MM_VBV_LEN , "__MM_VBV_START_ADDR ");
    print_buffer_range(AVC_VBV_ADDR,AVC_VBV_LEN , "AVC_VBV_ADDR ");
    //print_buffer_range(HEVC_VBV_ADDR, HEVC_VBV_LEN, "HEVC_VBV_ADDR ");
    
    print_buffer_range(0, 0, NULL);

    libc_printf("\n\nshare memory mapping\n\n");
#ifdef DUAL_ENABLE
    print_buffer_range(__MM_SHARE_BASE_ADDR,__MM_SHARED_MEM_LEN , "__MM_SHARE_BASE_ADDR ");
    print_buffer_range(__MM_VOID_BUFFER_ADDR, __MM_VOID_BUFFER_LEN , "void buffer to customer");
#endif
    print_buffer_range(__MM_HIGHEST_ADDR, 0, "Total Memory");

    print_buffer_range(0, 0, NULL);

    libc_printf("\n\nSpecial Item\n\n");
    print_buffer_range(__MM_VIDEO_FILE_BUF_ADDR, __MM_VIDEO_FILE_BUF_LEN, "Media buffer");
    
    print_buffer_range(0, 0, NULL);

}

void print_memory_layout_dual_c3711c_new_map()
{
    extern UINT heap_start;
    //UDG_PRINTF("\n======== Memory Layout ==========\n");

    libc_printf("\n\nmain memory mapping\n\n");

    //print_buffer_range(0, heap_start, "Code");
    print_buffer_range(__MM_PRIV_HIGHEST_ADDR, (__MM_HEAP_TOP_ADDR- __MM_PRIV_HIGHEST_ADDR), "Main code and heap buffer");
    print_buffer_range(__MM_CPU_DBG_MEM_ADDR, __MM_DBG_MEM_LEN, "CPU debug buffer ");
    print_buffer_range(__MM_CPU_DBG_MEM_ADDR +  __MM_DBG_MEM_LEN, __MM_USB_START_ADDR - 
        (__MM_CPU_DBG_MEM_ADDR +  __MM_DBG_MEM_LEN), "TTX/Subt/CC mix buffer ");    
    print_buffer_range(__MM_ISDBTCC_PB_START_ADDR,__MM_ISDBTCC_PB_LEN , "__MM_ISDBTCC_PB_START_ADDR ");
    print_buffer_range(__MM_ISDBTCC_BS_START_ADDR,__MM_ISDBTCC_BS_LEN , "__MM_ISDBTCC_BS_START_ADDR ");
    print_buffer_range(__MM_SUBT_ATSC_SEC_ADDR,__MM_SUBT_ATSC_SEC_LEN , "SUB_ATSC_ADDR");
    print_buffer_range(__MM_DCII_SUB_BS_START_ADDR, __MM_DCII_SUB_BS_LEN, "DCII");

    print_buffer_range(__MM_SUB_PB_START_ADDR,__MM_SUB_PB_LEN , "SUB_PB_START ");
    print_buffer_range(__MM_SUB_HW_DATA_ADDR,__MM_SUB_HW_DATA_LEN , "__MM_SUB_HW_DATA_ADDR ");
    print_buffer_range(__MM_SUB_BS_START_ADDR,__MM_SUB_BS_LEN , "SUB_BS_START_ ");
    print_buffer_range(__MM_ATSC_CC_PB_START_ADDR,__MM_ATSC_CC_PB_LEN , "__MM_ATSC_CC_PB_START_ADDR ");
    print_buffer_range(__MM_ATSC_CC_BS_START_ADDR,__MM_ATSC_CC_BS_LEN , "__MM_ATSC_CC_BS_START_ADDR ");
    print_buffer_range(__MM_TTX_PB_START_ADDR, __MM_TTX_PB_LEN, "TTX_PB_START ");
    print_buffer_range(__MM_TTX_BS_START_ADDR, __MM_TTX_BS_LEN, "TTX_BS_START ");
    print_buffer_range(__MM_TTX_P26_DATA_BUF_ADDR, __MM_TTX_P26_DATA_LEN, "TTX_P26_DATA_ ");
    print_buffer_range(__MM_TTX_P26_NATION_BUF_ADDR,__MM_TTX_P26_NATION_LEN , " TTX_P26_NATION");
    print_buffer_range(__MM_TTX_SUB_PAGE_BUF_ADDR, __MM_TTX_SUB_PAGE_LEN, "TTX_SUB_PAGE ");

    print_buffer_range(__MM_USB_START_ADDR, __MM_USB_DMA_LEN,"__MM_USB_START_ADDR");
    print_buffer_range(__MM_LWIP_MEMP_ADDR, __MM_LWIP_MEMP_LEN,"__MM_LWIP_MEMP_ADDR");
    print_buffer_range(__MM_LWIP_MEM_ADDR, __MM_LWIP_MEM_LEN,"__MM_LWIP_MEM_ADDR");
    print_buffer_range(__MM_OSD_VSRC_MEM_ADDR, OSD_VSRC_MEM_MAX_SIZE, "__MM_OSD_VSRC_MEM_ADDR ");
    print_buffer_range(__MM_OSD_BK_ADDR2_MAIN, __MM_OSD2_LEN, "__MM_OSD_BK_ADDR2 ");    
    print_buffer_range(__MM_OSD_BK_ADDR1, __MM_OSD1_LEN, "__MM_OSD_BK_ADDR1 ");

    print_buffer_range(__MM_GE_START_ADDR, __MM_GE_LEN, "__MM_GE_START_ADDR ");
    print_buffer_range(__MM_EPG_BUFFER_START ,__MM_EPG_BUFFER_LEN, "EPG_BUFFER ");
    print_buffer_range(__MM_DMX_CPU_BLK_ADDR, __MM_DMX_BLK_BUF_LEN,"__MM_DMX_CPU_BLK_ADDR");
    print_buffer_range(__MM_DMX_REC_START_ADDR, __MM_DMX_REC_LEN,"__MM_DMX_REC_START_ADDR");
    print_buffer_range(__MM_DMX_AVP_START_ADDR,__MM_SI_VBV_OFFSET + __MM_DMX_DATA_LEN + __MM_DMX_AVP_LEN , "DMX AVP ");

    print_buffer_range(__MM_AUTOSCAN_DB_BUFFER_ADDR, __MM_AUTOSCAN_DB_BUFFER_LEN,"__MM_AUTOSCAN_DB_BUFFER_ADDR");
    print_buffer_range(__MM_PVR_VOB_BUFFER_ADDR,__MM_PVR_VOB_BUFFER_LEN , "VOB cache");
    print_buffer_range(__MM_NIM_BUFFER_ADDR,__MM_NIM_BUFFER_LEN , "__MM_NIM_BUFFER_ADDR");
    libc_printf("---FB for AVC\n");
    print_buffer_range(AVC_FB_ADDR, AVC_FB_LEN, "AVC_FB_ADDR ");
    print_buffer_range(AVC_DVIEW_ADDR,AVC_DVIEW_LEN , "AVC_DVIEW_ADDR ");    
    print_buffer_range(AVC_LAF_FLAG_BUF_ADDR,AVC_LAF_FLAG_BUF_LEN , "AVC_LAF_FLAG_BUF_ADDR ");
    print_buffer_range(AVC_CMD_QUEUE_ADDR,AVC_CMD_QUEUE_LEN , "AVC_CMD_QUEUE_ADDR ");
    print_buffer_range(AVC_LAF_RW_BUF_ADDR,AVC_LAF_RW_BUF_LEN , "AVC_LAF_RW_BUF_ADDR ");
    print_buffer_range(AVC_MB_NEI_ADDR, AVC_MB_NEI_LEN, "AVC_MB_NEI_ADDR ");
    print_buffer_range(AVC_MB_COL_ADDR, AVC_MB_COL_LEN, " AVC_MB_COL_ADDR");
    print_buffer_range(AVC_MV_ADDR, AVC_MV_LEN, "AVC_MV_ADDR ");    

    libc_printf("---FB for MPEG2\n");  
    print_buffer_range(__MM_FB_START_ADDR, __MM_FB_LEN, "MPEG2_FB_ADDR ");
    print_buffer_range(__MM_MAF_START_ADDR, __MM_MAF_LEN, "__MM_MAF_START_ADDR ");
    libc_printf("---FB End\n");

    print_buffer_range(__MM_FB_BOTTOM_ADDR, __MM_FB_TOP_ADDR - __MM_FB_BOTTOM_ADDR, "video Frame buffer lenth ");
    print_buffer_range(__MM_FB_TOP_ADDR, 0, "Frame buffer top memory");
    print_buffer_range(__MM_VCAP_FB_ADDR, __MM_VCAP_FB_SIZE, "__MM_VCAP_FB_ADDR ");
    print_buffer_range(__MM_MAIN_TOP_ADDR, 0, "__MM_MAIN_TOP_ADDR ");
    print_buffer_range(__MM_VOID_BUFFER_ADDR, __MM_VOID_BUFFER_LEN , "void buffer to customer");
    print_buffer_range(0, 0, NULL);
    
#ifdef DUAL_ENABLE
    libc_printf("\n\nprivate memory mapping\n\n");
    print_buffer_range(__MM_VSC_BUFFER_ADDR,__MM_VSC_BUF_LEN, "__MM_VSC_BUFFER_ADDR");
    print_buffer_range(__MM_SEE_BL_BUFFER_ADDR,__MM_SEE_BL_BUF_LEN, "__MM_SEE_BL_BUFFER_ADDR");
    print_buffer_range(__MM_PRIV_BOTTOM_ADDR,__MM_SEE_DBG_MEM_ADDR - __MM_PRIV_BOTTOM_ADDR , "SEE code and heap buffer");
    print_buffer_range(__MM_SEE_DBG_MEM_ADDR,__MM_DBG_MEM_LEN , "SEE debug");
    print_buffer_range(__MM_OSD_BK_ADDR2, __MM_OSD2_LEN, "__MM_OSD_BK_ADDR2 "); 
    print_buffer_range(__MM_SEE_MP_BUF_ADDR,__MM_SEE_MP_BUF_LEN , " __MM_SEE_MP_BUF_ADDR");\
    print_buffer_range(__MM_DMX_SEE_BLK_ADDR,__MM_DMX_SEE_BLK_BUF_LEN , " __MM_DMX_SEE_BLK_ADDR");   
    print_buffer_range(VDEC_PRIVATE_START_ADDR,__MM_REAL_PRIVATE_TOP_ADDR - VDEC_PRIVATE_START_ADDR , " VBV buffer length");
    print_buffer_range(__MM_PRIV_DEC_ADDR,__MM_PRIVATE_TOP_ADDR - __MM_PRIV_DEC_ADDR , " __MM_PRIV_DEC_ADDR(DD+ buffer)");
    print_buffer_range(__MM_PRIVATE_TOP_ADDR, 0, "Private Top Memory");
#endif
    print_buffer_range(0, 0, NULL);
    libc_printf("\n\nshare memory mapping\n\n");
#ifdef DUAL_ENABLE
    print_buffer_range(__MM_SHARE_BASE_ADDR,__MM_SHARED_MEM_LEN , "__MM_SHARE_BASE_ADDR ");
#endif 
    print_buffer_range(0, 0, NULL);
    libc_printf("\n\nSpecial Item\n\n");
    print_buffer_range(__MM_VIDEO_FILE_BUF_ADDR, __MM_VIDEO_FILE_BUF_LEN, "Media buffer");
    print_buffer_range(__MM_SHARE_BASE_ADDR - __MM_RESERV_SECIAL_BUF_LEN, __MM_RESERV_SECIAL_BUF_LEN, "Reserved buffer");
    print_buffer_range(0, 0, NULL);
    print_buffer_range(__MM_HIGHEST_ADDR, 0, "Total Memory");
    libc_printf("\n\n buffer end \n\n");
}

#endif

#else
void print_memory_layout_dual()
{
    extern UINT heap_start;
    //UDG_PRINTF("\n======== Memory Layout ==========\n");

    libc_printf("\n\nmain memory mapping\n\n");

    print_buffer_range(0, heap_start, "Code");
    print_buffer_range(heap_start, (__MM_HEAP_TOP_ADDR&0x0fffffff)-(heap_start&0x0fffffff), "Heap");

    print_buffer_range(__MM_CPU_DBG_MEM_ADDR, __MM_DBG_MEM_LEN, "CPU debug buffer ");
    print_buffer_range(__MM_EPG_BUFFER_START ,__MM_EPG_BUFFER_LEN, "EPG_BUFFER ");
    print_buffer_range(__MM_USB_START_ADDR, __MM_USB_DMA_LEN,"USB_START_");
    print_buffer_range(__MM_DMX_CPU_BLK_ADDR, __MM_DMX_BLK_BUF_LEN,"__MM_DMX_CPU_BLK_ADDR");

    print_buffer_range(__MM_LWIP_MEMP_ADDR, __MM_LWIP_MEMP_LEN,"__MM_LWIP_MEMP_ADDR");
    print_buffer_range(__MM_LWIP_MEM_ADDR, __MM_LWIP_MEM_LEN,"__MM_LWIP_MEM_ADDR");
    print_buffer_range(__MM_AUTOSCAN_DB_BUFFER_ADDR, __MM_AUTOSCAN_DB_BUFFER_LEN,"__MM_AUTOSCAN_DB_BUFFER_ADDR");
    print_buffer_range(__MM_OSD_VSRC_MEM_ADDR, OSD_VSRC_MEM_MAX_SIZE, "__MM_OSD_VSRC_MEM_ADDR ");
    print_buffer_range(__MM_VCAP_FB_ADDR, __MM_VCAP_FB_SIZE, "__MM_VCAP_FB_ADDR ");
    print_buffer_range(__MM_OSD_BK_ADDR1, __MM_OSD1_LEN, "__MM_OSD_BK_ADDR1 ");
    print_buffer_range(__MM_GE_START_ADDR, __MM_GE_LEN, "__MM_GE_START_ADDR ");
    print_buffer_range(__MM_PVR_VOB_BUFFER_ADDR,__MM_PVR_VOB_BUFFER_LEN , "VOB cache");
    print_buffer_range(__MM_DMX_REC_START_ADDR, __MM_DMX_REC_LEN,"__MM_DMX_REC_START_ADDR");
    print_buffer_range(__MM_DMX_AVP_START_ADDR,__MM_SI_VBV_OFFSET + __MM_DMX_DATA_LEN + __MM_DMX_AVP_LEN , "DMX AVP ");

    print_buffer_range(AVC_LAF_RW_BUF_ADDR,AVC_LAF_RW_BUF_LEN , "AVC_LAF_RW_BUF_ADDR ");
    print_buffer_range(AVC_MB_NEI_ADDR, AVC_MB_NEI_LEN, "AVC_MB_NEI_ADDR ");
    print_buffer_range(AVC_MB_COL_ADDR, AVC_MB_COL_LEN, " AVC_MB_COL_ADDR");
    print_buffer_range(AVC_MV_ADDR, AVC_MV_LEN, "AVC_MV_ADDR ");
    print_buffer_range(AVC_DVIEW_ADDR,AVC_DVIEW_LEN , "AVC_DVIEW_ADDR ");
    print_buffer_range(AVC_FB_ADDR, AVC_FB_LEN, "AVC_FB_ADDR ");
    print_buffer_range(__MM_FB_TOP_ADDR, 0, "Frame buffer top memory");
    print_buffer_range(__MM_VIDEO_FILE_BUF_ADDR, __MM_VIDEO_FILE_BUF_LEN, "Media buffer");

    libc_printf("\n\nprivate memory mapping\n\n");
#ifdef DUAL_ENABLE
    print_buffer_range(__MM_SEE_DBG_MEM_ADDR,__MM_DBG_MEM_LEN , "SEE debug");
#endif
    print_buffer_range(__MM_SUB_PB_START_ADDR,__MM_SUB_PB_LEN , "SUB_PB_START ");
    print_buffer_range(__MM_SUB_HW_DATA_ADDR,__MM_SUB_HW_DATA_LEN , "__MM_SUB_HW_DATA_ADDR ");
    print_buffer_range(__MM_SUB_BS_START_ADDR,__MM_SUB_BS_LEN , "SUB_BS_START_ ");
    print_buffer_range(__MM_OSD_BK_ADDR2, __MM_OSD2_LEN, "__MM_OSD_BK_ADDR2 ");
    print_buffer_range(AVC_LAF_FLAG_BUF_ADDR, AVC_LAF_FLAG_BUF_LEN, "AVC_LAF_FLAG_BUF_ADDR ");
    print_buffer_range(AVC_CMD_QUEUE_ADDR, AVC_CMD_QUEUE_LEN, "AVC_CMD_QUEUE_ADDR ");
#ifdef DUAL_ENABLE
    print_buffer_range(__MM_SEE_MP_BUF_ADDR,__MM_SEE_MP_BUF_LEN , " __MM_SEE_MP_BUF_ADDR");
#endif
    print_buffer_range(__MM_TTX_P26_DATA_BUF_ADDR, __MM_TTX_P26_DATA_LEN, "TTX_P26_DATA_ ");
    print_buffer_range(__MM_TTX_P26_NATION_BUF_ADDR,__MM_TTX_P26_NATION_LEN , " TTX_P26_NATION");
    print_buffer_range(__MM_TTX_SUB_PAGE_BUF_ADDR, __MM_TTX_SUB_PAGE_LEN, "TTX_SUB_PAGE ");
    print_buffer_range(__MM_TTX_PB_START_ADDR, __MM_TTX_PB_LEN, "TTX_PB_START ");
    print_buffer_range(__MM_TTX_BS_START_ADDR, __MM_TTX_BS_LEN, "TTX_BS_START ");
    print_buffer_range(__MM_DMX_SEE_BLK_ADDR,__MM_DMX_BLK_BUF_LEN , "__MM_DMX_SEE_BLK_ADDR ");
    print_buffer_range(AVC_VBV_ADDR,AVC_VBV_LEN , "AVC_VBV_ADDR ");
    print_buffer_range(__MM_VBV_START_ADDR,__MM_VBV_LEN , "__MM_VBV_START_ADDR ");

    libc_printf("\n\nshare memory mapping\n\n");
#ifdef DUAL_ENABLE
    print_buffer_range(__MM_SHARE_BASE_ADDR,__MM_SHARED_MEM_LEN , "__MM_SHARE_BASE_ADDR ");
    print_buffer_range(__MM_VOID_BUFFER_ADDR, __MM_VOID_BUFFER_LEN , "void buffer to customer");
#endif
    print_buffer_range(__MM_HIGHEST_ADDR, 0, "Total Memory");

    print_buffer_range(0, 0, NULL);

}

#endif


#endif
#endif
#endif


#endif

#if SUPPORT_AUTO_TEST
static BOOL m_f_auto_test = FALSE;
static UINT32 m_dw_start_time, m_dw_duration;

static void set_time_stamp(UINT32 ts)
{
    m_dw_start_time = ts;
    m_dw_duration = ((ts/10)%10 + 1)*50;
}

void switch_auto_test()
{
    m_f_auto_test = m_f_auto_test ? FALSE : TRUE;
    if(m_f_auto_test)
        set_time_stamp(os_get_tick_count());
}

void auto_test_generator()
{
    UINT32 ts, code = NULL_VKEY;

    if(m_f_auto_test)
    {
        ts = os_get_tick_count();
        if((ts - m_dw_start_time) < m_dw_duration)
            return;
        switch(ts%10)
        {
            case 0:
            case 6:
                code = V_KEY_UP;
                break;
            case 1:
            case 7:
                code = V_KEY_DOWN;
                break;
            case 2:
            case 8:
                code = V_KEY_1;
                break;
            case 3:
            case 9:
                code = V_KEY_1;//2;
                break;
            case 4:
                code = V_KEY_FAV;//V_KEY_P_UP;
                break;
//            case 5:
//                code = V_KEY_P_DOWN;
//                break;
            default:
                break;
        }
        if(NULL_VKEY != code)
        {
            fw_send_msg_to_ui(CTL_MSG_TYPE_KEY, CTRL_MSG_SUBTYPE_KEY_IR, code);
        }
        set_time_stamp(ts);
    }
}

BOOL com_uni_str_cmp_n(PCWSTR p_str1, PCWSTR p_str2, UINT16 n_len)
{
    UINT16 i, tmp;

    if((NULL == p_str1) || (NULL == p_str2))
    {
        return FALSE;
    }
 
    if(!n_len)
    {
        n_len = com_uni_str_len(p_str1);
        tmp = com_uni_str_len(p_str2);
        if(n_len != tmp)
            return FALSE;
    }

    for (i=0; i<n_len; i++)
    {
        if(p_str1[i] != p_str2[i])
            return FALSE;
    }
    return TRUE;
}

#endif

#if DBG_COUNTRY_BAND
void print_country_band()
{
typedef band_param      REGION_BAND[MAX_BAND_NUMBER];
    unsigned long chid = CHID_COUNTRYBAND;
    REGION_BAND *band;
    band_param *param;
    int i, j;

    band = (REGION_BAND *)(chunk_goto(&chid, 0xFFFFFFFF, 1) + 128);
    for(i=0; i<MAX_REGION; i++)
    {
        param = (band_param *)band[i];
        if(param->start_freq)
            UDG_PRINTF("======== Region[%d]=0x%x ========\n", i, param);
        for(j=0; j<MAX_BAND_NUMBER && param->start_freq; j++, param++)
        {
            UDG_PRINTF("[%d]%d, %d, %d, %d, %d, %d\n", j,
                    param->start_freq, param->end_freq, param->bandwidth,
                    (param->band_type&0x80)?1:0, param->band_type&0x7f, (param->band_type>>8)&0xFF);
        }
    }
}
#endif

#if DBG_WATCH_POINT
// set watch point for m3501
#
# void sys_set_watch_point(UINT32 addr)
#
        .globl    sys_set_watch_point
        .ent    sys_set_watch_point
sys_set_watch_point:
        mtc0    a0, $18
        nop
        jr      ra
        nop
        .end    sys_set_watch_point


// mask = bit0:store, bit1:load, bit2:fetch
void dbg_set_watch_point(UINT32 addr, UINT32 mask)
{
    addr &= 0xfffffff8;
    addr |= mask;
    sys_set_watch_point(addr);
}


#endif
