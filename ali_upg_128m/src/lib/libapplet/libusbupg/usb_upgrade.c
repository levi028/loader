/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: usb_upgrade.c
*
*    Description: this file describes upgrade operations by u disk
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <types.h>
#include <retcode.h>
#include <sys_config.h>
#include <asm/chip.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/sci/sci.h>
#include <hld/hld_dev.h>
#include <osal/osal_timer.h>
#include <api/libchunk/chunk.h>
#include <api/libc/fast_crc.h>

#include <api/libfs2/stat.h>
#include <api/libfs2/dirent.h>
#include <api/libfs2/statvfs.h>
#include <api/libfs2/unistd.h>
#include <api/libfs2/stdio.h>
#include <api/libupg/usb_upgrade.h>

/*
* for declaration sd_mount
*/
#include <api/libmp/pe.h>
#include <api/libfs2/fs_main.h>

#if 1
#define USBUPG_PRINTF(...) do{}while(0)
#else
#define USBUPG_PRINTF libc_printf
#endif

static USBUPG_FILENODE *usbupg_filelist=NULL;
static UINT16 usbupg_node_num = 0;
static CHUNK_HEADER *usbupg_blockinfo=NULL;
static UINT16 usbupg_block_num = 0;
static UINT8 usbupg_bloader = 0;
static UINT32 bootarea_len = 0;


/*
    Get real upgrade chunk list
*/
static inline void usb_upg_free(void *buffer)
{
    if(NULL != buffer)
    {
        free(buffer);
        buffer = NULL;
    }
}
static int usb_get_upg_chunklist(CHUNK_LIST *chunk_list,UINT32 *fiter_chunk_array,UINT8 flter_cnt)
{
    CHUNK_LIST *plist = NULL;
    unsigned long id = 0;
    unsigned char name[16] = {0};
    unsigned long count = 0;
    unsigned long temp_cnt = 0;
    unsigned long addr = 0;
    unsigned long i = 0;
    unsigned long j = 0;
    struct sto_device *sto = NULL;
    INT32 ret4sto = -1;

    sto = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
    ASSERT(sto != NULL);
    if(!(chunk_list&&fiter_chunk_array))
    {
        return 0;
    }
    count = sto_chunk_count(0, 0);
    if(!count)
    {
        return 0;
    }
    plist = chunk_list;
    for(i=1; i<=count; i++)
    {
        addr = sto_chunk_goto(&id, 0, i);
        if(addr & 0x80000000)
        {
            return 0;
        }
        ret4sto = sto_get_data(sto, name, addr + CHUNK_NAME, 16);
		if(0==ret4sto)
		{
			;
		}
        for(j = 0; j<flter_cnt; j++)
        {
            if(fiter_chunk_array[j] == id)
            {
                break;
            }
        }
        if(j >=flter_cnt)
        {
            temp_cnt++;
            plist->id = id;
            plist->upg_flag = 0;
            plist++;
        }
    }
    return temp_cnt;
}
RET_CODE usbupg_block_init(UINT16 *block_num)
{
    UINT16 i = 0;
    UINT16 block_cnt = 0;
    UINT16 block_cnt_upg = 0;
    int ret4int = -1;
    CHUNK_LIST *chunk_tmp_list = NULL;
    UINT32 filter_chunk[2] = {CADATA_ID,0};
    UINT16 temp_block_num = 0;

    if (NULL == block_num)
    {
        return RET_FAILURE;
    }

#ifdef _NV_PROJECT_SUPPORT_
    *block_num = 1;
#else

    get_boot_total_area_len(&bootarea_len);

    block_cnt = sto_chunk_count(0,0);

    if(block_cnt <= 0 )
    {
        *block_num = 0;
        return RET_FAILURE;
    }
    else
    {
        if(bootarea_len > 0)
        {
            
        }
        chunk_tmp_list = malloc(sizeof(CHUNK_LIST) * block_cnt);
        if(NULL == chunk_tmp_list)
        {
            return RET_FAILURE;
        }
    }

    MEMSET((UINT8*)chunk_tmp_list,0,sizeof(CHUNK_LIST) * block_cnt);
    block_cnt_upg = usb_get_upg_chunklist(chunk_tmp_list,filter_chunk,sizeof(filter_chunk)/sizeof(filter_chunk[0]));
    if(!block_cnt_upg)
    {
        usb_upg_free(chunk_tmp_list);
        return RET_FAILURE;
    }
    usbupg_blockinfo = (CHUNK_HEADER *)malloc(sizeof(CHUNK_HEADER) * block_cnt_upg);
    if(NULL == usbupg_blockinfo)
    {
        if(chunk_tmp_list)
        {
            usb_upg_free(chunk_tmp_list);
        }
        return RET_FAILURE;
    }
    else
    {
        usbupg_block_num = block_cnt_upg;
        *block_num = (block_cnt_upg + 1);
        for(i = 0; i < block_cnt_upg; i++)
        {
            ret4int = sto_get_chunk_header((chunk_tmp_list + i)->id,(usbupg_blockinfo + i));
            if(!ret4int)
            {
                usb_upg_free(chunk_tmp_list);
                return RET_FAILURE;
            }
        }
        if(chunk_tmp_list)
        {
            usb_upg_free(chunk_tmp_list);
        }
    }
#endif

    usbupg_filelist = (USBUPG_FILENODE *)malloc(sizeof(USBUPG_FILENODE) * USBUPG_MAX_FILENUM_SUPPORT);
    if(NULL == usbupg_filelist)
    {
        usbupg_block_num = 0;
        usb_upg_free(usbupg_blockinfo);
        return RET_FAILURE;
    }
    else
    {
        usbupg_node_num = 0;//initlize node buffer
        MEMSET(usbupg_filelist,0x0, sizeof(USBUPG_FILENODE) * USBUPG_MAX_FILENUM_SUPPORT);
    }

#ifndef _NV_PROJECT_SUPPORT_
#ifndef USBUPG_MEMCFG_SUPPORT
    //for 3503 not show memcfg upg in menu
    if(ALI_S3503==sys_ic_get_chip_id())
    {
        temp_block_num = *block_num;
        *block_num = temp_block_num - 1;
        usbupg_block_num -= 1;
        for(i = 2; i < block_cnt-1; i++)
        {
            MEMCPY((usbupg_blockinfo+i), (usbupg_blockinfo+i+1), sizeof(CHUNK_HEADER));
        }
    }
#endif
#endif

    return RET_SUCCESS;
}

RET_CODE usbupg_get_blockinfo(UINT16 block_idx,char *str_blockname,UINT32 *pchunk_offset)
{
    RET_CODE ret = RET_FAILURE;
    UINT8 size = 20;

    if(!(str_blockname && pchunk_offset))
    {
        return RET_FAILURE;
    }
    if(0 == block_idx)
    {
        strncpy(str_blockname, "AllCode", size - 1);
        *pchunk_offset = SYS_FLASH_SIZE;
        if(0 == usbupg_bloader)
        {
            // not include bootloader
            if(bootarea_len)
            {
                *pchunk_offset -= bootarea_len;
            }
            else
            {
                *pchunk_offset -= usbupg_blockinfo[0].offset;
            }
        }
        ret = RET_SUCCESS;
    }
    else if(block_idx < usbupg_block_num)
    {
        MEMCPY(str_blockname,(usbupg_blockinfo + block_idx - 1)->name,16);
        *pchunk_offset = (usbupg_blockinfo + block_idx - 1)->offset;//block length,include chunk header
        ret = RET_SUCCESS;
    }
    else
    {
        MEMCPY(str_blockname,(usbupg_blockinfo + block_idx - 1)->name,16);
        *pchunk_offset = (usbupg_blockinfo + block_idx - 1)->len + CHUNK_NAME;//block length,include chunk header
        ret = RET_SUCCESS;
    }
    return ret;
}

RET_CODE usbupg_block_free(void)
{
    if(usbupg_filelist != NULL)
    {
        usbupg_node_num = 0;//initlize node buffer
        usb_upg_free(usbupg_filelist);
    }
    if(usbupg_blockinfo!= NULL)
    {
        usbupg_block_num = 0;//initlize node buffer
        usb_upg_free(usbupg_blockinfo);
    }
    return RET_SUCCESS;
}

RET_CODE usbupg_get_file_filter(UINT16 block_idx, UINT8 *upgfile_extends, UINT8 *upgfile_filters)
{
    UINT32 id_filter = 0;
    UINT8 s_size = 4;

    if(!(upgfile_extends && upgfile_filters))
    {
        return RET_FAILURE;
    }
    if(0 == block_idx)//all code upgrade
    {
        id_filter = 0xFFFF0000;
    }
    else
    {
        id_filter = ((usbupg_blockinfo + block_idx - 1)->id) & 0xFFFF0000;
    }
    switch(id_filter)
    {
    case USBUPG_LOGO_ID:
        strncpy((char *)upgfile_filters, "", s_size-1);
        strncpy((char *)upgfile_extends, "M2V", s_size-1);
        break;
    case USBUPG_DFLTDB_ID:
        //default db
        strncpy((char *)upgfile_filters, "DDB", s_size-1);
        strncpy((char *)upgfile_extends,"BIN", s_size-1);
        break;
    case USBUPG_USERDB_ID:
        strncpy((char *)upgfile_filters, "UDB", s_size-1);
        strncpy((char *)upgfile_extends, "BIN", s_size - 1);
        break;
    case USBUPG_KEYDATA_ID:
        strncpy((char *)upgfile_filters, "KDT", s_size - 1);
        strncpy((char *)upgfile_extends, "BIN", s_size - 1);
        break;
    case USBUPG_MAINCODE_ID:
        strncpy((char *)upgfile_filters, "MCD", s_size - 1);
        strncpy((char *)upgfile_extends, "BIN", s_size - 1);
        break;
    case USBUPG_SEECODE_ID:
        strncpy((char *)upgfile_filters, "SCD", s_size - 1);
        strncpy((char *)upgfile_extends, "BIN", s_size - 1);
        break;
    case USBUPG_OTALOADER_ID:
        strncpy((char *)upgfile_filters, "SLD", s_size - 1);
        strncpy((char *)upgfile_extends, "BIN", s_size - 1);
        break;
    case USBUPG_BOOTLOADER_ID:
        strncpy((char *)upgfile_filters, "BLD", s_size - 1);
        strncpy((char *)upgfile_extends, "ABS", s_size - 1);
        break;
    case USBUPG_ALLCODE_ID:
        strncpy((char *)upgfile_filters, "ALL", s_size - 1);
        strncpy((char *)upgfile_extends, "ABS", s_size - 1);
        break;
#ifdef HDCP_IN_FLASH
    case USBUPG_HDCPKEY_ID:
        strncpy((char *)upgfile_filters, "", s_size - 1);
        strncpy((char *)upgfile_extends, "KEY", s_size - 1);
        break;
#endif
#ifdef USBUPG_MEMCFG_SUPPORT
    case USBUPG_MEMCFG_ID:
        strncpy((char *)upgfile_filters, "", s_size - 1);
        strncpy((char *)upgfile_extends, "ABS", s_size - 1);
        break;
#endif
    default:
        //user db
        strncpy((char *)upgfile_filters, "", s_size - 1);
        strncpy((char *)upgfile_extends, "ABS", s_size - 1);
        break;
    }

    return RET_SUCCESS;
}

RET_CODE usbupg_create_filelist(UINT16 block_idx,UINT16 *usbupg_nodenum)
{
    UINT16 str_len = 0;
    int dir_fd = -1;
    struct dirent *dent = NULL;
    char upgfile_extends[3 + 1] = {0};
    char upgfile_filters[3 + 1] = {0};
    USBUPG_BLOCK_TYPE block_type = USBUPG_OTHER;
    
    

#ifdef USBUPG_UPGFILE_FILTER
    unsigned long str_len_three = 3;
#endif

    if(!usbupg_nodenum)
    {
        return RET_FAILURE;
    }
    block_type = usbupg_get_block_type(block_idx);
    usbupg_get_file_filter(block_idx, (UINT8 *)upgfile_extends, (UINT8 *)upgfile_filters);
    if(sd_mount)
    {
        dir_fd = fs_opendir("/mnt/sda1");
    }
    else
    {
        dir_fd = fs_opendir("/mnt/uda1");
       }
    if(dir_fd < 0)/*USB disk root dir open failure*/
    {
        *usbupg_nodenum = 0;
        return RET_FAILURE;
    }
    dent = ( struct dirent *) malloc(sizeof(char)*(sizeof(struct dirent) + FILE_NAME_SIZE));
    if(NULL == dent)
    {
        USBUPG_PRINTF("ERROR: MALLOC struct dirent failed!\n");
        return RET_FAILURE;
    }
    usbupg_node_num = 0;//initlize node buffer
    MEMSET(usbupg_filelist,0x0, sizeof(USBUPG_FILENODE) * USBUPG_MAX_FILENUM_SUPPORT);
    while(1)
    {
        if (fs_readdir(dir_fd,dent) <= 0)
        {
            break;/*Tail of the dir files*/
        }
        if(S_ISDIR(dent->d_type))
        {
            continue;
        }
        else
        {
            str_len = strlen(dent->d_name);
            if(!strncasecmp((dent->d_name + str_len - 3),upgfile_extends,3))
            {
#ifdef USBUPG_UPGFILE_FILTER
                if((str_len_three == strlen(upgfile_filters))
                        && (strncasecmp(dent->d_name,upgfile_filters,3) != 0 ))
                {
                    continue;//filter failure
                }
#endif/*Using prefix name to distiguish different upgrade type,eg. all*.abs for allcode*/

                usbupg_filelist[usbupg_node_num].upg_file_type = block_type;
                strncpy((usbupg_filelist[usbupg_node_num].upg_file_name),dent->d_name, USBUPG_FILE_NAME_LENGTH-1);
                usbupg_node_num++;
                if(usbupg_node_num > USBUPG_MAX_FILENUM_SUPPORT)
                {
                    break;//check usbupg files if overflow
                }
            }
        }
    }
    fs_closedir(dir_fd);
    *usbupg_nodenum = usbupg_node_num;
    if(NULL != dent)
    {
        usb_upg_free(dent);
    }
    if(0 == usbupg_node_num)
    {
        return RET_FAILURE;
    }
    else
    {
        return RET_SUCCESS;
    }
}

RET_CODE usbupg_get_filenode(PUSBUPG_FILENODE usbupg_node,UINT16 usbupg_nodeidx)
{
    if(0 == usbupg_node_num)
    {
        return RET_FAILURE;
    }
    else
    {
        MEMCPY(usbupg_node,(usbupg_filelist+usbupg_nodeidx),sizeof(USBUPG_FILENODE));
        return RET_SUCCESS;
    }
}

static USBUPG_ERROR_CODE check_upg_err(char *pointer, UINT32 data_id)
{
    USBUPG_ERROR_CODE ret = USBUPG_FILEREAD_NO_ERROR;
    UINT32 rotation_16 = 16;
    UINT32 rotation_24 = 24;

    if (pointer != 0)
    {
        if ((((data_id >> rotation_16) & 0xFF) + ((data_id >> rotation_24) & 0xFF)) != 0xFF)
        {
            ret = USBUPG_FIELREAD_ERROR_READERROR;
        }
    }
    return ret;
}

RET_CODE usbupg_readfile(UINT16 block_idx,UINT16 usbupg_fileidx,void *file_buffer,
                                                INT32 buffer_size,usbupg_prgrs_disp progress_disp)
{
    char dir_str[USBUPG_FILE_NAME_LENGTH] = {0};
    UINT32 i = 0;
    UINT32 fileblocks_num = 0;
    UINT32 chunk_crc = 0;
    UINT32 chunk_len = 0;
    int file_ret = 0;
    INT32 file_length = 0;
    void *adr_bak = NULL;
    FILE *fp_handle = NULL;
    char *tptr = NULL;
    char *pointer = NULL;
    UINT32 data_id=0;
    UINT32 data_crc=0;
    INT32 data_len=0;
    INT32 data_off=0;
    UINT32 crc=0;
    USBUPG_ERROR_CODE upg_err_check= USBUPG_FILEREAD_NO_ERROR;
    UINT32 cnt = 0;

    if((0 == usbupg_node_num) || (NULL == file_buffer))
    {
        return USBUPG_FILEREAD_ERROR_NOFILE;
    }
    else
    {
        if(0 == sd_mount)
        {
            snprintf(dir_str,USBUPG_FILE_NAME_LENGTH,"/mnt/uda1/%s",usbupg_filelist[usbupg_fileidx].upg_file_name);
        }
        else
        {
            snprintf(dir_str,USBUPG_FILE_NAME_LENGTH, "/c/%s",usbupg_filelist[usbupg_fileidx].upg_file_name);
        }
        USBUPG_PRINTF("[%s(%d)] dir_str=%s\n", __FUNCTION__, __LINE__, dir_str);
        fp_handle = fopen(dir_str,"rb");
        if(NULL == fp_handle)
        {
            return USBUPG_FIELREAD_ERROR_READERROR;
        }
        adr_bak = file_buffer;
        MEMSET(adr_bak,0xff,buffer_size);//clear buffer
        if(block_idx > 1)//non-allcode upgrade,should reserve a CHUNK_HEADER size
        {
            file_buffer = file_buffer + USBUPG_CHUNK_SIZE;
            buffer_size= buffer_size - USBUPG_CHUNK_SIZE;
        }
        file_ret = fseek(fp_handle,0,SEEK_END);
        file_length = (unsigned long)ftell(fp_handle);
        USBUPG_PRINTF("[%s(%d)] file_length=0x%x\n", __FUNCTION__, __LINE__, file_length);
        if(file_ret < 0)
        {
            file_ret = fclose(fp_handle);
            return USBUPG_FIELREAD_ERROR_READERROR;
        }
        else
        {
            if(0 == block_idx)    // allcode upgrade
            {
                if(0 == usbupg_bloader)
                {
                    //skip bootloader
                    if(bootarea_len)
                    {
                        file_length -= bootarea_len;
                    }
                    else
                    {
                        file_length -= usbupg_blockinfo[0].offset;
                    }
                }
            }
            if(file_length<0)
            {
                file_ret = fclose(fp_handle);
                return USBUPG_FIELREAD_ERROR_READERROR;
            }
            if(file_length > buffer_size)
            {
                file_ret = fclose(fp_handle);
                return USBUPG_FIELREAD_ERROR_OVERFLOW;
            }
            else
            {
                if(0 == (file_length % SECTOR_SIZE))
                {
                    fileblocks_num = file_length/SECTOR_SIZE;
                }
                else
                {
                    fileblocks_num = file_length/SECTOR_SIZE + 1;
                }
                if((0 == block_idx) && (0 == usbupg_bloader))
                {
                    //skip bootloader
                    if(bootarea_len)
                    {
                        file_ret = fseek(fp_handle,bootarea_len,SEEK_SET);//set pos at head
                    }
                    else
                    {
                        file_ret = fseek(fp_handle,usbupg_blockinfo[0].offset,SEEK_SET);//set pos at head
                    }
                }
                else
                {
                    file_ret = fseek(fp_handle,0,SEEK_SET);//set pos at head
                }
                if(file_ret)
                {
                    file_ret = fclose(fp_handle);
                    return USBUPG_FIELREAD_ERROR_READERROR;
                }
                for(i = 0; i < fileblocks_num; i++)
                {
                    file_ret = (int)fread(file_buffer,SECTOR_SIZE,1,fp_handle);
                    file_buffer = file_buffer +SECTOR_SIZE;//move buffer pointer
                    if(progress_disp != NULL)
                    {
                        progress_disp(i *100 / fileblocks_num);
                    }
                    if(file_ret <= 0)
                    {
                        file_ret = fclose(fp_handle);
                        return USBUPG_FIELREAD_ERROR_READERROR;
                    }
                }//read a sector size each time
                if(progress_disp != NULL)
                {
                    progress_disp(100);
                }
                if(block_idx != 0)
                {
                    if(block_idx != 1)//except BOOTLOADER,for bootloader no crc etc.
                    {
                        MEMCPY(adr_bak,(usbupg_blockinfo + block_idx - 1), sizeof(CHUNK_HEADER));
                        chunk_len = (file_length + USBUPG_CHUNKCRC_OFFSET);
                        chunk_crc = NO_CRC;
                        if (NO_CRC != (usbupg_blockinfo + block_idx - 1)->crc)
                        {
                            chunk_crc =mg_table_driven_crc(0xFFFFFFFF,(adr_bak + 4*sizeof(UINT32)), chunk_len);
                        }
                        store_long((adr_bak + CHUNK_ID),(usbupg_blockinfo + block_idx - 1)->id);
                        store_long((adr_bak + CHUNK_LENGTH),chunk_len);
                        store_long((adr_bak + CHUNK_OFFSET),(usbupg_blockinfo + block_idx - 1)->offset);
                        store_long((adr_bak + CHUNK_CRC),chunk_crc);
                        /*length include all file data length and part of the chunk head,except for id,len,offset,crc*/
                    }
                    else//for bootloader special case
                    {
                        //TODO:should get system time and set in CHUNK_HEADER??
                        tptr = (char *)((usbupg_blockinfo + block_idx - 1)->name);
                        MEMCPY((adr_bak + CHUNK_NAME),tptr,16);
                        tptr = (char *)((usbupg_blockinfo + block_idx - 1)->version);
                        MEMCPY((adr_bak + CHUNK_VERSION),tptr,16);
                        tptr = (char *)((usbupg_blockinfo + block_idx - 1)->time);
                        MEMCPY((adr_bak + CHUNK_TIME),tptr,16);
                        tptr = (char *)((usbupg_blockinfo + block_idx - 1)->reserved);
                        MEMCPY((adr_bak + CHUNK_RESERVED),tptr,(sizeof(CHUNK_HEADER)-CHUNK_RESERVED));
                    }
                }
                file_ret = fclose(fp_handle);

#ifdef _NV_PROJECT_SUPPORT_
                return file_length;
#else
            pointer = adr_bak;

                if (NULL == pointer)
                {
                    return USBUPG_FIELREAD_ERROR_READERROR;
                }
                cnt= 0;
                do
                {
                    data_id  = fetch_long((unsigned char*)(pointer + CHUNK_ID));
                    data_len = fetch_long((unsigned char*)(pointer + CHUNK_LENGTH));
                    data_off = fetch_long((unsigned char*)(pointer + CHUNK_OFFSET));
                    data_crc = fetch_long((unsigned char*)(pointer + CHUNK_CRC));
                    crc = 0;
                    if((0 == data_off)||((UINT32)pointer ==(UINT32)(adr_bak + buffer_size)))
                    {
                        break;
                    }

                    // not check bootloader chunk id
                    if((0 == cnt) && (1 == usbupg_bloader))
                    {
                        upg_err_check = USBUPG_FILEREAD_NO_ERROR;                      
                    }
                    else
                    {
                        upg_err_check = check_upg_err(pointer, data_id);
                    }
                    cnt++;
                    if(USBUPG_FIELREAD_ERROR_READERROR == upg_err_check)
                    {
                        return upg_err_check;
                    }
                    if(block_idx > 1)//non-allcode upgrade,should reserve a CHUNK_HEADER size
                    {
                        buffer_size= buffer_size + USBUPG_CHUNK_SIZE;
                    }
                    if (data_off > buffer_size)
                    {
                        return USBUPG_FIELREAD_ERROR_READERROR;
                    }
                    if (data_len > data_off)
                    {
                        return USBUPG_FIELREAD_ERROR_READERROR;
                    }
					if(data_crc == crc)
					{
						;
					}
#if ((!defined(_CAS9_CA_ENABLE_))&&(!defined(CA_C2000A)))//no need to  check CRC because maincode/seecode will be verified
                    if (data_crc != NO_CRC)
                    {

                        crc = mg_table_driven_crc(0xFFFFFFFF, (UINT8*)pointer + 16, data_len);
                        //free(buffer);
                        if (data_crc != crc)
                        {
                            return USBUPG_FIELREAD_ERROR_READERROR;
                        }
                    }
#endif
                    pointer += data_off;
                    if ((UINT32)pointer >(UINT32)(adr_bak + buffer_size))
                    {
                        return USBUPG_FIELREAD_ERROR_READERROR;
                    }
                }while(1);
#endif //_NV_PROJECT_SUPPORT_

                return USBUPG_FILEREAD_NO_ERROR;
            }
        }
    }
}

static BOOL all_data_is_0xff(UINT8 *addr, UINT32 len)
{
    UINT32 i = 0;

    if(!addr)
    {
        return FALSE;
    }
    for (i = 0; i < len; i++)
    {
        if (addr[i] != 0xff)
        {
            return FALSE;
        }
    }
    return TRUE;
}

static void init_block_addr(UINT16 block_idx, INT32 *f_block_addr)
{
    if(!f_block_addr)
    {
        return ;
    }
    switch(block_idx)
    {
    case 0:    //allcode upgrade
        if(0 == usbupg_bloader)
        {// not include bootloader, skip loader
            if(bootarea_len)
            {
                *f_block_addr = bootarea_len;
            }
            else
            {
                *f_block_addr = usbupg_blockinfo[0].offset;
            }
        }
        else
        {
            *f_block_addr = 0x0;
        }
        break;
    case 1:     //bootloader
        *f_block_addr = 0x0;
        break;
    default:
        *f_block_addr = (INT32)sto_chunk_goto(&((usbupg_blockinfo + block_idx - 1)->id), 0xFFFFFFFF, 1);
        break;
    }
}

BOOL usbupg_burnflash(UINT16 block_idx,UINT8 *buffer_adr,UINT32 buffer_size,usbupg_prgrs_disp progress_disp)
{
    UINT8 *malloc_sector_buff = NULL;    //read flash data to sdram,to compare if need write
    UINT32 param = 0;
    INT32 f_block_addr = 0;
    INT32 burn_size = 0;
    struct sto_device *f_dev = NULL;
    BOOL need_erase = TRUE;
    INT32 head_sector_adr = 0;
    INT32 tail_sector_adr = 0;
    INT32 usbupg_blk_endadr = 0;
    INT32 memcpy_src_adr = 0;
    INT32 memcpy_dst_adr = 0;
    UINT32 memcpy_len = 0;
    UINT32 tmp_param[2] = {0};
    INT32 ret4sto = 0;

	if(0 == ret4sto)
	{
		;
	}
	if(0 == usbupg_blk_endadr)
	{
		;
	}
	if(!buffer_adr)
    {
        return FALSE;
    }
    burn_size = (INT32)buffer_size;
    if(progress_disp != NULL)
    {
        progress_disp(0);
    }
    init_block_addr(block_idx, &f_block_addr);
    if (NULL == (f_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0)))
    {
        USBUPG_PRINTF("ERROR: Not found Flash device!\n");
        return FALSE;
    }
    if (sto_open(f_dev) != SUCCESS)
    {
        USBUPG_PRINTF("ERROR: sto_open failed!\n");
        return FALSE;
    }
    if (NULL == (malloc_sector_buff = (UINT8*)malloc(SECTOR_SIZE)))
    {
        USBUPG_PRINTF("ERROR: malloc failed!\n");
        return FALSE;
    }
    if(((f_block_addr % SECTOR_SIZE) != 0) || ((buffer_size % SECTOR_SIZE) !=0))
    {
        head_sector_adr = f_block_addr - f_block_addr % SECTOR_SIZE;//get last sector head adr
        tail_sector_adr = (f_block_addr + buffer_size + SECTOR_SIZE) - (f_block_addr + buffer_size)%SECTOR_SIZE;
        usbupg_blk_endadr = f_block_addr + buffer_size;
        while(head_sector_adr < tail_sector_adr)
        {//write a sector,using malloc_sector_buff
            if (sto_lseek(f_dev, head_sector_adr, STO_LSEEK_SET) != head_sector_adr)
            {
                USBUPG_PRINTF("ERROR: sto_lseek failed!\n");
                usb_upg_free(malloc_sector_buff);
                return FALSE;
            }
            if (sto_read(f_dev, malloc_sector_buff, SECTOR_SIZE) != SECTOR_SIZE)
            {
                USBUPG_PRINTF("ERROR: sto_read failed!\n");
                usb_upg_free(malloc_sector_buff);
                return FALSE;
            }
            need_erase = !all_data_is_0xff(malloc_sector_buff, SECTOR_SIZE);
            if(head_sector_adr <= f_block_addr)  //copy upgrade data here
            {
                memcpy_len = (head_sector_adr + SECTOR_SIZE - f_block_addr);
                if(memcpy_len > buffer_size)
                {
                    memcpy_len = buffer_size;
                }
                memcpy_src_adr = (INT32)buffer_adr;
                memcpy_dst_adr = (INT32)(malloc_sector_buff + f_block_addr - head_sector_adr);
                MEMCPY((void *)memcpy_dst_adr,(void *)memcpy_src_adr,memcpy_len);
                buffer_adr = buffer_adr + memcpy_len;
                buffer_size = buffer_size - memcpy_len;
            }
            else
            {
                if(buffer_size >= SECTOR_SIZE)
                {
                    memcpy_len = SECTOR_SIZE;
                }
                else
                {
                    memcpy_len = buffer_size;
                }
                memcpy_src_adr = (INT32)buffer_adr;
                memcpy_dst_adr = (INT32)malloc_sector_buff;
                MEMCPY((void *)memcpy_dst_adr,(void *)memcpy_src_adr,memcpy_len);
                buffer_adr = buffer_adr + memcpy_len;
                buffer_size = buffer_size - memcpy_len;
            }
            if (need_erase)
            {
                tmp_param[0] = head_sector_adr;
                tmp_param[1] = SECTOR_SIZE>>10;
                param = (UINT32)tmp_param;
                if(sto_io_control(f_dev, STO_DRIVER_SECTOR_ERASE_EXT, param) != SUCCESS)
                {
                    USBUPG_PRINTF("ERROR: erase flash memory failed!\n");
                    usb_upg_free(malloc_sector_buff);
                    return FALSE;
                }
            }
            if (!all_data_is_0xff(malloc_sector_buff, SECTOR_SIZE))
            {
                if (sto_lseek(f_dev,head_sector_adr,STO_LSEEK_SET) != head_sector_adr)
                {
                    USBUPG_PRINTF("ERROR: sto_lseek failed!\n");
                    usb_upg_free(malloc_sector_buff);
                    return FALSE;
                }
                if(sto_write(f_dev,(UINT8 *)malloc_sector_buff, SECTOR_SIZE)!=SECTOR_SIZE)
                {
                    USBUPG_PRINTF("ERROR: sto_write failed!\n");
                    usb_upg_free(malloc_sector_buff);
                    return FALSE;
                }
            }
            head_sector_adr += SECTOR_SIZE;
            if(progress_disp != NULL)
            {
                progress_disp((burn_size -buffer_size)*100/burn_size);
            }
            if(0 == buffer_size)
            {
                break;
            }
        }
    }//for block upgrade
    else
    {
        if (sto_lseek(f_dev, f_block_addr, STO_LSEEK_SET) != f_block_addr)
        {
            USBUPG_PRINTF("ERROR: sto_lseek failed!\n");
            usb_upg_free(malloc_sector_buff);
            return FALSE;
        }
        if (sto_read(f_dev, malloc_sector_buff, SECTOR_SIZE) != SECTOR_SIZE)
        {
            USBUPG_PRINTF("ERROR: sto_read failed!\n");
            usb_upg_free(malloc_sector_buff);
            return FALSE;
        }
        while(buffer_size)
        {
            ret4sto = sto_get_data(f_dev, malloc_sector_buff, f_block_addr, SECTOR_SIZE);
            if(0 != MEMCMP(malloc_sector_buff, buffer_adr, SECTOR_SIZE))
            {
                need_erase = !all_data_is_0xff(malloc_sector_buff, SECTOR_SIZE);
                if (need_erase)
                {
                    tmp_param[0] = f_block_addr;
                    tmp_param[1] = SECTOR_SIZE>>10;
                    param = (UINT32)tmp_param;
                    if(sto_io_control(f_dev, STO_DRIVER_SECTOR_ERASE_EXT, param) != SUCCESS)
                    {
                        USBUPG_PRINTF("ERROR: erase flash memory failed!\n");
                        usb_upg_free(malloc_sector_buff);
                        return FALSE;
                    }
                }
                if (!all_data_is_0xff(buffer_adr, SECTOR_SIZE))
                {
                    if (sto_lseek(f_dev,(INT32)f_block_addr,STO_LSEEK_SET) != (INT32)f_block_addr)
                    {
                        USBUPG_PRINTF("ERROR: sto_lseek failed!\n");
                        usb_upg_free(malloc_sector_buff);
                        return FALSE;
                    }
                    if(sto_write(f_dev, buffer_adr, SECTOR_SIZE)!=SECTOR_SIZE)
                    {
                        USBUPG_PRINTF("ERROR: sto_write failed!\n");
                        usb_upg_free(malloc_sector_buff);
                        return FALSE;
                    }
                }
            }
            f_block_addr +=SECTOR_SIZE;
            buffer_adr +=SECTOR_SIZE;
            buffer_size -= SECTOR_SIZE;
            if(progress_disp != NULL)
            {
                progress_disp((burn_size -buffer_size)*100/burn_size);
            }
        }
    }
    if(progress_disp != NULL)
    {
        progress_disp(100);
    }
    usb_upg_free(malloc_sector_buff);
    return TRUE;
}

USBUPG_BLOCK_TYPE usbupg_get_block_type(UINT16 block_idx)
{
    UINT32 id_filter = 0;
    USBUPG_BLOCK_TYPE block_type = USBUPG_OTHER;

    if(0 == block_idx)//all code upgrade
    {
        id_filter = 0xFFFF0000;
    }
    else
    {
        id_filter = ((usbupg_blockinfo + block_idx - 1)->id) & 0xFFFF0000;
    }
    switch(id_filter)
    {
    case USBUPG_LOGO_ID:
        block_type = USBUPG_LOGO;
        break;
    case USBUPG_DFLTDB_ID:
        block_type = USBUPG_DFLTDB;
        break;
    case USBUPG_USERDB_ID:
        block_type = USBUPG_USERDB;
        break;
#ifdef HDCP_IN_FLASH
    case USBUPG_HDCPKEY_ID    :
        block_type = USBUPG_HDCPKEY;
        break;
#endif
    case USBUPG_KEYDATA_ID:
        block_type = USBUPG_KEYDB;
        break;
    case USBUPG_MAINCODE_ID:
        block_type = USBUPG_MAINCODE;
        break;
    case USBUPG_SEECODE_ID:
        block_type = USBUPG_SEECODE;
        break;
    case USBUPG_OTALOADER_ID:
        block_type = USBUPG_SECONDLOADER;
        break;
    case USBUPG_BOOTLOADER_ID:
        block_type = USBUPG_BOOTLOADER;
        break;
    case USBUPG_ALLCODE_ID:
        block_type = USBUPG_ALLCODE;
        break;
#ifdef USBUPG_MEMCFG_SUPPORT
    case USBUPG_MEMCFG_ID:
        block_type = USBUPG_MEMCFG;
        break;
#endif
    default:
        block_type = USBUPG_OTHER;
        break;
    }

    return block_type;
}

RET_CODE usbupg_dump_flash(UINT32 block_idx,UINT8 *str_file,usbupg_prgrs_disp progress_disp)
{
    /*block_idx parameter reserved for extended usage*/
    UINT16 i = 0;
    UINT8 *p_buff = NULL;
    UINT8 str_path[64] = {0};
    FILE *fp_dump = NULL;
    struct sto_device *f_dev = NULL;
    int ret4int = 0;
    INT32 ret4sto = 0;
    UINT32 read_size = 0;
    UINT32 dump_size = 0;
    UINT32 start_addr= 0;
    CHUNK_HEADER ch_header;
    int fop_ret = -1;

	if(0 == fop_ret)
	{
		;
	}
	if(0 == ret4sto)
	{
		;
	}
    if(!str_file)
    {
        return RET_FAILURE;
    }
    MEMSET(&ch_header, 0, sizeof(CHUNK_HEADER));
    snprintf((char *)str_path,64,"/c/%s",str_file);
    fp_dump = fopen((const char *)str_path,"w");
    if (NULL == fp_dump)
    {
        return USBUPG_FILE_NOT_EXIST;
    }
    f_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
    if (NULL == f_dev)
    {
        fop_ret = fclose(fp_dump);
        return USBUPG_FILE_NOT_EXIST;
    }
    p_buff = malloc(SECTOR_SIZE);
    if(NULL == p_buff)
    {
        fop_ret= fclose(fp_dump);
        return USBUPG_BUFFER_MALLOC_FAILURE;
    }
    progress_disp(0);
    if(0 == block_idx)
    {
        ret4sto = sto_lseek(f_dev,0, STO_LSEEK_SET);
        for(i = 0; i < (SYS_FLASH_SIZE/SECTOR_SIZE); i++)
        {
            MEMSET(p_buff,0x0,SECTOR_SIZE);
            ret4sto = sto_read(f_dev,p_buff,SECTOR_SIZE);
            fwrite(p_buff,1,SECTOR_SIZE, fp_dump);
            progress_disp((i*100)/(SYS_FLASH_SIZE/SECTOR_SIZE));
        }
    }
    else
    {
        start_addr= sto_chunk_goto(&block_idx,0xFFFFFFFF,1);
        ret4int = sto_get_chunk_header(block_idx, &ch_header);
        if(!ret4int)
        {   
            free(p_buff);
            p_buff = NULL;
            fop_ret = fclose(fp_dump);
            return USBUPG_FLASH_WR_ERROR;
        }
        dump_size = (0 == ch_header.offset)?ch_header.len +0x10:ch_header.offset;
        dump_size = dump_size - CHUNK_HEADER_SIZE;
        read_size = dump_size;
        ret4sto = sto_lseek(f_dev,start_addr+CHUNK_HEADER_SIZE,STO_LSEEK_SET);
        for(i = 0; i< dump_size/SECTOR_SIZE; ++i)
        {
            MEMSET(p_buff,0x0,SECTOR_SIZE);
            if(read_size > SECTOR_SIZE)
            {
                ret4sto = sto_read(f_dev,p_buff,SECTOR_SIZE);
                fwrite(p_buff,1,SECTOR_SIZE, fp_dump);
            }
            else
            {
                ret4sto = sto_read(f_dev,p_buff,read_size);
                fwrite(p_buff,1,read_size, fp_dump);
                break;
            }
            read_size -= SECTOR_SIZE;
            progress_disp((i*100)/(dump_size/SECTOR_SIZE));
        }
    }
    progress_disp(100);
    free(p_buff);
    p_buff = NULL;
    fop_ret = fclose(fp_dump);
    ret4int = fs_sync("/c");
    return RET_SUCCESS;
}

RET_CODE usbupg_match_file_structure(UINT16 absfile_idx)
{
    char dir_str[USBUPG_FILE_NAME_LENGTH] = {0};
    UINT8 i = 0;    //,chunk_cnt;
    UINT32 file_size = 0;
    UINT32 buff_offset = 0;
    UINT32 chunk_offset = 0;
    FILE *fp_handle = NULL;
    CHUNK_HEADER chunk_node;
    int fop_ret = -1;
    
    MEMSET(&chunk_node, 0, sizeof(CHUNK_HEADER));
    if(0 == usbupg_node_num)
    {
        /*No abs file*/
        return USBUPG_CHUNK_CNT_MISMATCH;
    }
    else
    {
        snprintf(dir_str, USBUPG_FILE_NAME_LENGTH, "/c/%s",usbupg_filelist[absfile_idx].upg_file_name);
        fp_handle = fopen(dir_str,"rb");
        if(NULL == fp_handle)
        {
            return USBUPG_FILEREAD_ERROR_NOFILE;
        }
        fop_ret = fseek(fp_handle,0,SEEK_END);
        if(fop_ret)
        {
            fclose(fp_handle);
            return USBUPG_FILE_WR_ERROR;
        }
        file_size = ftell(fp_handle);
        if((file_size < SYS_FLASH_SIZE/4) || (file_size > SYS_FLASH_SIZE))
        {
            fclose(fp_handle);
            return USBUPG_FILE_SIZE_MISMATCH;/*Invalid file*/
        }

        buff_offset = 0;
        for(i = 0; i < usbupg_block_num; i++)
        {
            MEMSET(&chunk_node,0x0,sizeof(CHUNK_HEADER));
            fop_ret = fseek(fp_handle,buff_offset,SEEK_SET);
            if(fop_ret)
            {
                fclose(fp_handle);
                return USBUPG_FILE_WR_ERROR;
            }
            file_size = fread(&chunk_node,sizeof(CHUNK_HEADER),1,fp_handle);/*actual read file bytes*/
            chunk_offset = fetch_long((unsigned char *)&(chunk_node.offset));
            buff_offset += usbupg_blockinfo[i].offset;

            USBUPG_PRINTF("buff_offset is %x  file %x,flash%x.\n",buff_offset,chunk_offset,usbupg_blockinfo[i].offset);
            if(chunk_offset != usbupg_blockinfo[i].offset)
            {
                fclose(fp_handle);
                return USBUPG_CHUNK_SIZE_MISMATCH;/*Can not directly reference,for little endian in MIPS*/
            }
        }

        fclose(fp_handle);
    }

    return RET_SUCCESS;
}

void set_upgrade_bloader_flag(UINT8 flag)
{
    usbupg_bloader = flag;
}

RET_CODE usbupg_allcode_burn(UINT16 absfile_idx,usbupg_prgrs_disp progress_disp)
{
    char dir_str[USBUPG_FILE_NAME_LENGTH] = {0};
    UINT8 i = 0;
    UINT8 sector_cnt = 0;
    UINT8 *p_sector = NULL;
    UINT32 file_len = 0;
    UINT32 sto_param = 0;
    FILE *fp_handle = NULL;
    struct sto_device *f_dev = NULL;
    INT32 ret4sto = -1;
    UINT32 tmp_param[2] = {0};
    int fop_ret = -1;

    snprintf(dir_str, USBUPG_FILE_NAME_LENGTH, "/c/%s",usbupg_filelist[absfile_idx].upg_file_name);
    fp_handle = fopen(dir_str,"rb");
    if(NULL == fp_handle)
    {
        return USBUPG_FILEREAD_ERROR_NOFILE;
    }
    p_sector  = (UINT8 *)malloc(SECTOR_SIZE);
    if (NULL == p_sector)
    {
        fop_ret = fclose(fp_handle);
        return USBUPG_BUFFER_MALLOC_FAILURE;
    }
    fop_ret = fseek(fp_handle,0,SEEK_END);
    if(fop_ret != 0)
    {
        fop_ret = fclose(fp_handle);
        free(p_sector);
        p_sector = NULL;
        return USBUPG_FILE_WR_ERROR;
    }
    file_len = ftell(fp_handle);
    if((file_len < SYS_FLASH_SIZE/4) || (file_len > SYS_FLASH_SIZE))
    {
        fop_ret = fclose(fp_handle);
        free(p_sector);
        p_sector = NULL;
        return USBUPG_FILE_SIZE_MISMATCH;
    }

    if (NULL == (f_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0)))
    {
        USBUPG_PRINTF("ERROR: Not found Flash device!\n");
        free(p_sector);
        p_sector = NULL;
        fclose(fp_handle);
        return USBUPG_FLASH_WR_ERROR;
    }

    if (progress_disp)
    {
        progress_disp(0);
    }
    if(0 == (usbupg_blockinfo[0].offset%SECTOR_SIZE))  //jump bootloader
    {
        i = (usbupg_blockinfo[0].offset)/SECTOR_SIZE;
    }
    else
    {
        i = 0;
    }
    if(usbupg_bloader)
    {
        i = 0;
    }
    fop_ret = fseek(fp_handle,i*SECTOR_SIZE,SEEK_SET);
    if(fop_ret != 0)
    {
        fop_ret = fclose(fp_handle);
        free(p_sector);
        p_sector = NULL;
        return USBUPG_FILE_WR_ERROR;
    }
    ret4sto = sto_lseek(f_dev,0,STO_LSEEK_SET);
    sector_cnt = (0 == (file_len % SECTOR_SIZE))?(file_len>>16):((file_len>>16) + 1);

    while(i < sector_cnt)
    {
        MEMSET(p_sector,0xFF,SECTOR_SIZE);
        // fix for >4M flash
        tmp_param[0] = i*SECTOR_SIZE;
        tmp_param[1] = SECTOR_SIZE>>10;
        sto_param = (UINT32)tmp_param;
        if(sto_io_control(f_dev, STO_DRIVER_SECTOR_ERASE_EXT, sto_param) != SUCCESS)
        {
            free(p_sector);
            p_sector = NULL;
            fop_ret = fclose(fp_handle);
            USBUPG_PRINTF("ERROR: erase flash memory failed!\n");
            return USBUPG_FLASH_WR_ERROR;
        }
        ret4sto = sto_lseek(f_dev,i*SECTOR_SIZE,STO_LSEEK_SET);
        ret4sto = sto_write(f_dev,p_sector, SECTOR_SIZE);
        if(SECTOR_SIZE != ret4sto)
        {
            free(p_sector);
            p_sector = NULL;
            fop_ret = fclose(fp_handle);
            return RET_FAILURE;
        }
        if (progress_disp)
        {
            progress_disp((i*100)/sector_cnt);
        }
        i++;
    }
    progress_disp(100);

    free(p_sector);
    p_sector = NULL;
    fop_ret = fclose(fp_handle);
    return RET_SUCCESS;
}

static void usbupg_get_block_offset(UINT16 block_idx,UINT32 *p_offset,UINT32 *p_len)
{
    UINT8 i = 0;

    if((!p_offset) || (!p_len))
    {
        return ;
    }
    *p_offset = 0;
    if(0 == block_idx)
    {
        *p_len = SYS_FLASH_SIZE;
        return;
    }
    else
    {
        *p_len = 0;
    }

    for(i = 0; i < usbupg_block_num; i++)
    {
        if(i == block_idx-1)
        {
            *p_len = usbupg_blockinfo[i].offset;
            break;
        }
        *p_offset += usbupg_blockinfo[i].offset;
    }

    if(block_idx == usbupg_block_num)
    {
        //user db
        *p_len = usbupg_blockinfo[block_idx-1].len;
    }
}

RET_CODE usbupg_readfile_block(UINT16 block_idx,UINT16 absfile_idx,void *file_buffer,UINT32 buffer_size)
{
    char dir_str[USBUPG_FILE_NAME_LENGTH] = {0};
    UINT32 fp_offset = 0;
    UINT32 fp_len = 0;
    FILE *fp_handle = NULL;
    int fop_ret = 0;
    
    if(NULL == file_buffer)
    {
        return RET_FAILURE;
    }
    usbupg_get_block_offset(block_idx,&fp_offset,&fp_len);
    if(buffer_size < fp_len)
    {
        return USBUPG_BUFFER_MALLOC_FAILURE;
    }

    snprintf(dir_str,USBUPG_FILE_NAME_LENGTH,"/c/%s",usbupg_filelist[absfile_idx].upg_file_name);
    fp_handle = fopen(dir_str,"rb");
    if(NULL == fp_handle)
    {
        return USBUPG_FILEREAD_ERROR_NOFILE;
       }
    fop_ret = fseek(fp_handle,fp_offset,SEEK_SET);
    if(!fop_ret)
    {
        fclose(fp_handle);
        return RET_FAILURE;
    }
    fop_ret = fclose(fp_handle);
    if(!fop_ret)
    {
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

