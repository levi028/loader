/*****************************************************************************
*    Copyright (c) 2015 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be 
     disclosed to unauthorized individual.    
*    File: chunk_hdr.c
*   
*    Description:    This file contains chunk header checking funtions of Ali SDK.
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
     PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <api/libchunk/chunk.h>
#include <hld/sto/sto.h>
#include <hld/sto/sto_dev.h>
#include <api/libchunk/chunk.h>
#include <api/libchunk/chunk_hdr.h>
#include <boot/boot_common.h>
#include <sys_parameters.h>

//mandatory
#if defined (_CAS9_CA_ENABLE_)
#include <api/librsa/rsa_verify.h>
#include <api/libstbinfo/stb_info_data.h>
#endif
#include <api/librsa/generate_hmac.h>

#define HDR_ALL_HMAC_OFFSET (0x80)
#define HDR_CHUNK_HMAC_OFFSET (HDR_ALL_HMAC_OFFSET+HMAC_OUT_LENGTH)

#define MAX_OTA_PARA_LEN 0x1FF0
//#define SW_HDR_DEBUG

#ifdef SW_HDR_DEBUG
#define HDR_PRINTF     libc_printf //soc_printf 
#else
#define HDR_PRINTF(...)     do{}while(0)
#endif

#if defined (_CAS9_CA_ENABLE_)
static RET_CODE _read_hmac_from_flash(const UINT8 type,UINT8 *hmac_data, const UINT8 hmac_len)
{
    RET_CODE ret=RET_FAILURE;
    UINT32 hmac_offset=0;
    struct sto_device *flash_dev = NULL;
    UINT32 chunk_id = 0;
    UINT32 chunk_offset =0;
      
    flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
    if((NULL == flash_dev) || (NULL==hmac_data) ||(hmac_len<HMAC_OUT_LENGTH))
    {
        return RET_FAILURE;
    }

    switch(type)
    {
        case HMAC_STATIC_HDRCHUNK:
            chunk_id=STB_CHUNK_STATIC_DATA_HDR_ID;
            hmac_offset=HDR_CHUNK_HMAC_OFFSET;
            break;
        case HMAC_STATIC_ALLHDR:
            chunk_id=STB_CHUNK_STATIC_DATA_HDR_ID;
            hmac_offset=HDR_ALL_HMAC_OFFSET;            
            break;
        case HMAC_DYNAMIC_HDRCHUNK:
            chunk_id=STB_CHUNK_STBINFO_HDR_ID;
            hmac_offset=HDR_CHUNK_HMAC_OFFSET;
            break;
        case HMAC_DYNAMIC_ALLHDR:
            chunk_id=STB_CHUNK_STBINFO_HDR_ID;
            hmac_offset=HDR_ALL_HMAC_OFFSET;
            break;
        default:
            HDR_PRINTF("_read_hmac_from_flash, type wrong %d\n", type);
            return ret;
            break;
    }

    chunk_offset = (UINT32)sto_chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    if((ERR_PARA == chunk_offset)||(ERR_FAILUE== chunk_offset))
    {
        HDR_PRINTF("_read_hmac_from_flash, sto_chunk_goto 0x%x fail\n", chunk_id);
        return RET_FAILURE;
    }
    
    ret = sto_get_data(flash_dev, hmac_data, chunk_offset+hmac_offset, hmac_len);
    if(hmac_len!=ret )
    {
        HDR_PRINTF("_read_hmac_from_flash, sto_get_data fail,ret %d\n", chunk_id, ret);
        ret=RET_FAILURE;           
    }
    else
        ret = RET_SUCCESS;  
    return ret;
}

static RET_CODE _cal_hmac1_hdrchunk_from_flash(UINT8 type, UINT8 *hmac_data, const UINT8 hmac_len)
{
    UINT8 chunk_data[HDR_CHUNK_HMAC_OFFSET]={0};
    RET_CODE ret=RET_FAILURE;
    struct sto_device *flash_dev = NULL;
    UINT32 chunk_id =0;
    UINT32 chunk_data_len =0;
    UINT32 chunk_offset = 0;
       
    flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
    if((NULL == flash_dev) || (hmac_len<HMAC_OUT_LENGTH) || (NULL==hmac_data))
    {
        return ret;
    }

    switch(type)
    {
        case HMAC_STATIC_HDRCHUNK:
            chunk_id=STB_CHUNK_STATIC_DATA_HDR_ID;
            chunk_data_len=HDR_CHUNK_HMAC_OFFSET;
            break; 
        case HMAC_DYNAMIC_HDRCHUNK:
            chunk_id=STB_CHUNK_STBINFO_HDR_ID;
            chunk_data_len=HDR_CHUNK_HMAC_OFFSET;
            break;
        default:
            HDR_PRINTF("_cal_hmac1_hdrchunk_from_flash, type wrong %d\n", type);
            return ret;
            break;
    }
    
    chunk_offset = (UINT32)sto_chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    if((ERR_PARA == chunk_offset)||(ERR_FAILUE== chunk_offset))
    {
        HDR_PRINTF("_cal_hmac1_hdrchunk_from_flash, sto_chunk_goto 0x%x fail\n", chunk_id);
        return RET_FAILURE;
    }
    
    ret = sto_get_data(flash_dev, (UINT8 *)chunk_data, chunk_offset, chunk_data_len);
    if(ret==chunk_data_len)
    {       
        ret=api_gen_hmac1_hdrchunk((UINT8 *)chunk_data, chunk_data_len, hmac_data);
    }
    else
    {
        HDR_PRINTF("_cal_hmac1_hdrchunk_from_flash, sto_get_data fail,ret %d\n", chunk_id, ret);
        ret = RET_FAILURE;
    }
   
    return ret;
}

static RET_CODE _cal_hmac2_allhdr_from_flash(UINT8 type, UINT8 *hmac_data, const UINT8 hmac_len)
{//caculate all static/dynamic hdr data HMAC
    RET_CODE ret = RET_FAILURE;
    UINT32 chunk_cnt=0;
    UINT32 chunk_cnt2=0;   
    UINT32 chunklist_base = 0;
    UINT32 chunklist_len=0;    
    UINT32 header_base = 0;    
    UINT32 header_len=0;
    UINT32 dwi=0;
    UINT32 static_hdr_end_idx =0;
    UINT32 dynamic_hdr_start_idx =0;
    //UINT32 chunk_offset= 0;
    UINT32 new_head_start = 0;
    UINT32 new_head_len = 0;
    struct sto_device *test_flash_dev = NULL;
    UINT32 data_base = 0;
    UINT32 data_len = 0;
    //UINT32 stb_chunk_id=0;
    UINT8 *key_data=NULL;

    test_flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
    if((NULL == test_flash_dev) || (hmac_len<HMAC_OUT_LENGTH) || (NULL==hmac_data))
    {
        return ret;
    }

    switch(type)
    {
        case HMAC_STATIC_ALLHDR:
            //stb_chunk_id=STB_CHUNK_STATIC_DATA_HDR_ID;
            break; 
        case HMAC_DYNAMIC_ALLHDR:
            //stb_chunk_id=STB_CHUNK_STBINFO_HDR_ID;
            break;
        default:
            HDR_PRINTF("_cal_hmac2_allhdr_from_flash, type wrong %d\n", type);
            return ret;
            break;
    }
    
    if(sys_ic_get_chip_id()>=ALI_C3505)
    {
         //for 3505
        new_head_start = UK_OFFSET_WITH_NEW_HEAD_N3;
        new_head_len = BL_CODE_OFFSET_2 - UK_OFFSET_WITH_NEW_HEAD_N3;
    }
    
    chunk_cnt=sto_chunk_count(0, 0);

    if(chunk_cnt==0)
    {
        HDR_PRINTF("_cal_hmac2_allhdr_from_flash, sto_chunk_count wrong\n");
        return RET_FAILURE;
    }
      
    chunklist_len=chunk_cnt* sizeof(CHUNK_LIST);      
    chunklist_base=(UINT32) MALLOC (chunklist_len);
    if(0==chunklist_base)
    {
        return RET_FAILURE;
    }        

	chunk_cnt2=sto_get_chunk_list((CHUNK_LIST *)chunklist_base);

    if(chunk_cnt2==0)
    {
        HDR_PRINTF("_cal_hmac2_allhdr_from_flash, sto_get_chunk_list wrong\n");
        FREE((UINT8 *)chunklist_base);  
        return RET_FAILURE;
    }   
        
    header_len=chunk_cnt2* sizeof(CHUNK_HEADER)+new_head_len;//add key area len
    header_base=(UINT32) MALLOC (header_len);
    if(0==header_base)
    {
        FREE((UINT8 *)chunklist_base);  
        return RET_FAILURE;
    }
   
    for(dwi=0;dwi<chunk_cnt2;dwi++)
	{
		if(0==sto_get_chunk_header( ((CHUNK_LIST *)chunklist_base+dwi)->id,(CHUNK_HEADER*)header_base+dwi))
        {   
            HDR_PRINTF("_cal_hmac2_allhdr_from_flash, sto_get_chunk_header fail\n");
			ret = RET_FAILURE;
            goto Exit;
		}
        
        if(OUC_CHUNK_ID==((CHUNK_LIST *)chunklist_base+dwi)->id)
	    {//static data hmac should include ouc loader chunk hdr
			static_hdr_end_idx=dwi;
		}
        if(STB_CHUNK_STBINFO_HDR_ID==((CHUNK_LIST *)chunklist_base+dwi)->id)
	    {
			dynamic_hdr_start_idx=dwi;
		}
					
    }
    
    if((static_hdr_end_idx== 0) && (type == HMAC_STATIC_ALLHDR))
    {
        ret = RET_FAILURE;
		HDR_PRINTF("fixed %d : %s===static_hdr_end_idx=%d====\n",__LINE__,__FUNCTION__,static_hdr_end_idx);
        goto Exit;//no hdr chunk
    }
    if((dynamic_hdr_start_idx== 0) && (type == HMAC_DYNAMIC_ALLHDR))
    {
        ret = RET_FAILURE;
		HDR_PRINTF("fixed %d : %s===dynamic_hdr_start_idx=%d====\n",__LINE__,__FUNCTION__,dynamic_hdr_start_idx);
        goto Exit;//no hdr chunk
    }

    //save chunk header to memory, then all chunk header read should not read flash again
    if(get_chunk_header_status() == 0)
    {
        store_chunk_header_data(header_base,chunk_cnt2* sizeof(CHUNK_HEADER));//Fu_20151006_advanced_chunk_header_verify
        set_chunk_header_status(1);
    }

    if(type == HMAC_DYNAMIC_ALLHDR)
    {
        data_base = header_base + dynamic_hdr_start_idx*sizeof(CHUNK_HEADER);
        data_len = (chunk_cnt2-dynamic_hdr_start_idx)*sizeof(CHUNK_HEADER);
        HDR_PRINTF("_cal_hmac2_allhdr_from_flash, data_base 0x%x, data_len 0x%x\n", data_base, data_len);
        ret=api_gen_hmac2_allhdr((UINT8 *)data_base, data_len, hmac_data);//gen all dynamic hdr hmac
        if(ret!=RET_SUCCESS)
        {
			ret = RET_FAILURE;
            goto Exit;
		}
    }
    else if(type == HMAC_STATIC_ALLHDR)
    {
        key_data = (UINT8 *)MALLOC(BL_CODE_OFFSET_2 - UK_OFFSET_WITH_NEW_HEAD_N3);
        if(key_data == NULL)
        {
            ret = RET_FAILURE;
            goto Exit;
        }
        MEMSET(key_data, 0, BL_CODE_OFFSET_2 - UK_OFFSET_WITH_NEW_HEAD_N3);
        
        ret = sto_get_data(test_flash_dev, (UINT8 *)key_data, new_head_start, new_head_len);
        if(ret!=new_head_len)
        {
            HDR_PRINTF("_cal_hmac2_allhdr_from_flash, sto_get_data fail %d\n", ret);
            ret = RET_FAILURE;
            FREE(key_data);
            goto Exit;
        }
        data_base = header_base;
        data_len = (static_hdr_end_idx+1)*sizeof(CHUNK_HEADER);
        MEMCPY(data_base+data_len, key_data,new_head_len);//add key data
        data_len += new_head_len;
        HDR_PRINTF("_cal_hmac2_allhdr_from_flash, data_base 0x%x, data_len 0x%x\n", data_base, data_len);
        ret=api_gen_hmac2_allhdr((UINT8 *)data_base, data_len, hmac_data);//gen all static hdr hmac
        if(ret!=RET_SUCCESS)
        {
			ret = RET_FAILURE;
            FREE(key_data);
            goto Exit;
		}
        FREE(key_data);
        
    }
    ret = RET_SUCCESS;

Exit:     
    FREE((UINT8 *)chunklist_base);
	FREE((UINT8 *)header_base); 
    return ret;
}

static RET_CODE _write_hmac_to_flash(UINT8 type, UINT8 *hmac_data, UINT32 hmac_len) 
{
    struct sto_device *sto_flash_dev=NULL;
    INT32 ret = RET_FAILURE; 
    UINT8 *app_flash_buffer = NULL;
    UINT32 hmac_offset = 0;
    UINT32 chunk_id = 0;
    UINT32 chunk_offset = 0;
    UINT8 temp_data[512]={0};
    UINT32 temp_len =0;
    
    if((NULL==hmac_data) || (HMAC_OUT_LENGTH!=hmac_len))
    {
        return RET_FAILURE;
    }

    MEMSET(temp_data, 0xFF, 512);
    switch(type)
    {
        case HMAC_STATIC_HDRCHUNK:
            chunk_id=STB_CHUNK_STATIC_DATA_HDR_ID;
            hmac_offset=HDR_CHUNK_HMAC_OFFSET;
            temp_len = hmac_len;
            break;
        case HMAC_STATIC_ALLHDR:
            chunk_id=STB_CHUNK_STATIC_DATA_HDR_ID;
            hmac_offset=HDR_ALL_HMAC_OFFSET;
            temp_len = 0x100;//add padding to invalid signature
            break;
        case HMAC_DYNAMIC_HDRCHUNK:
            chunk_id=STB_CHUNK_STBINFO_HDR_ID;
            hmac_offset=HDR_CHUNK_HMAC_OFFSET;
            temp_len = hmac_len;
            break;
        case HMAC_DYNAMIC_ALLHDR:
            chunk_id=STB_CHUNK_STBINFO_HDR_ID;
            hmac_offset=HDR_ALL_HMAC_OFFSET;
            temp_len = 0x200;//add padding to invalid signature
            break;
        default:
            HDR_PRINTF("_write_hmac_to_flash, type wrong %d\n", type);
            return ret;
            break;
    }

    MEMCPY(temp_data, hmac_data, hmac_len);
    chunk_offset = (UINT32)sto_chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    if((ERR_PARA == chunk_offset)||(ERR_FAILUE== chunk_offset))
    {
        HDR_PRINTF("_write_hmac_to_flash, sto_chunk_goto fail 0x%x\n", chunk_id);
        return RET_FAILURE;
    }
    
    sto_flash_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
    app_flash_buffer = (UINT8 *)MALLOC(64*1024);
    if(NULL==app_flash_buffer)
    {
        return RET_FAILURE;
    }

    sto_io_control(sto_flash_dev, STO_DRIVER_SECTOR_BUFFER, (UINT32)app_flash_buffer);
    sto_io_control(sto_flash_dev, STO_DRIVER_SET_FLAG, STO_FLAG_AUTO_ERASE|STO_FLAG_SAVE_REST);
    
    ret = sto_put_data(sto_flash_dev, chunk_offset+hmac_offset, temp_data, temp_len);
    sto_io_control(sto_flash_dev, STO_DRIVER_SECTOR_BUFFER, 0);
    sto_io_control(sto_flash_dev, STO_DRIVER_SET_FLAG,0);
    
    if(app_flash_buffer)
    {
        FREE(app_flash_buffer);
        app_flash_buffer=NULL;
    }

    if(ret!=temp_len)
    {
        HDR_PRINTF("_write_hmac_to_flash, sto_put_data fail 0x%x\n", ret);
        ret = RET_FAILURE;
    }
    else
        ret = RET_SUCCESS;
    return ret;
}

//Fu_20151006_advanced_chunk_header_verify
RET_CODE verify_hdrchunk_hmac(UINT8 type)
{
    RET_CODE ret = RET_FAILURE;
    UINT8 hmac_read[HMAC_OUT_LENGTH]={0};
    UINT8 hmac_cal[HMAC_OUT_LENGTH]={0};

    if(type > HMAC_DYNAMIC_HDRCHUNK)
        return ret;
    
    MEMSET(hmac_read,0,HMAC_OUT_LENGTH);
    MEMSET(hmac_cal,0,HMAC_OUT_LENGTH);
    
    if(RET_SUCCESS!=_read_hmac_from_flash(type,hmac_read,HMAC_OUT_LENGTH))
    {
        HDR_PRINTF("verify_hdrchunk_hmac, read_hmac fail\n");
        return ret;
    }
    
    if(RET_SUCCESS==_cal_hmac1_hdrchunk_from_flash(type, hmac_cal ,HMAC_OUT_LENGTH))
    {
        if(0==MEMCMP(hmac_read,hmac_cal,HMAC_OUT_LENGTH))
        {
            libc_printf("verify_hdrchunk_hmac %d success\n", type);
            ret=RET_SUCCESS;
        }
    }
           
    return ret;
}

RET_CODE verify_allhdr_hmac(UINT8 type)
{
    RET_CODE ret = RET_FAILURE;
    UINT8 hmac_read[HMAC_OUT_LENGTH]={0};
    UINT8 hmac_cal[HMAC_OUT_LENGTH]={0};

    if(type > HMAC_DYNAMIC_ALLHDR)
        return ret;
    
    MEMSET(hmac_read,0,HMAC_OUT_LENGTH);
    MEMSET(hmac_cal,0,HMAC_OUT_LENGTH);  

    if(RET_SUCCESS!=_read_hmac_from_flash(type,hmac_read,HMAC_OUT_LENGTH))
    {
        HDR_PRINTF("verify_allhdr_hmac, read_hmac fail\n");
        return ret;
    }
    
    if(RET_SUCCESS==_cal_hmac2_allhdr_from_flash(type, hmac_cal ,HMAC_OUT_LENGTH))
    {
        if(0==MEMCMP(hmac_read,hmac_cal,HMAC_OUT_LENGTH))
        {
            libc_printf("verify_allhdr_hmac %d success\n", type);
            ret=RET_SUCCESS;
        }
    }
    
    return ret;
}

RET_CODE update_allhdr_hmac(UINT8 type)
{//caculate all static/dynamic hdr data HMAC and write to static/dynamic chunk 
    RET_CODE ret = RET_FAILURE;
    UINT8 hmac_inner[HMAC_OUT_LENGTH]={0};
    UINT8 hmac_outter[HMAC_OUT_LENGTH]={0};
    UINT8 sub_type1 =0;//hdrchunk
    UINT8 sub_type2 =0;//allhdr
  
    if(type == HMAC_DYNAMIC_TOTAL)
    {
        sub_type1 = HMAC_DYNAMIC_HDRCHUNK;
        sub_type2 = HMAC_DYNAMIC_ALLHDR;
    }
    else if(type == HMAC_STATIC_TOTAL)
    {
        sub_type1 = HMAC_STATIC_HDRCHUNK;
        sub_type2 = HMAC_STATIC_ALLHDR;
    }
    else
    {
        HDR_PRINTF("update_allhdr_hmac, type err\n");
        return RET_FAILURE;
    }

    set_chunk_header_status(0);//force read chunk header from flash if need update hmac
    ret = _cal_hmac2_allhdr_from_flash(sub_type2, hmac_inner, HMAC_OUT_LENGTH);
    if(ret!=RET_SUCCESS)
    {
        HDR_PRINTF("update_allhdr_hmac, cal_hmac2_allhdr fail\n");
		ret = RET_FAILURE;
        goto Exit;
	}
    ret = _write_hmac_to_flash(sub_type2, hmac_inner, HMAC_OUT_LENGTH);
    if(ret!=RET_SUCCESS)
    {
        HDR_PRINTF("update_allhdr_hmac, write_hmac %d fail\n", sub_type2);
		ret = RET_FAILURE;
        goto Exit;
	}

    ret = _cal_hmac1_hdrchunk_from_flash(sub_type1, hmac_outter, HMAC_OUT_LENGTH);
    if(ret!=RET_SUCCESS)
    {
        HDR_PRINTF("update_allhdr_hmac, cal_hmac1 fail\n");
		ret = RET_FAILURE;
        goto Exit;
	}
    
    ret = _write_hmac_to_flash(sub_type1, hmac_outter, HMAC_OUT_LENGTH);
    if(ret!=RET_SUCCESS)
    {
        HDR_PRINTF("update_allhdr_hmac, write_hmac %d fail\n", sub_type1);
		ret = RET_FAILURE;
        goto Exit;
	}
    ret = RET_SUCCESS;

Exit:     
    return ret;
}

#if 0   
UINT8 is_hdr_missing(UINT32 total_size)
{
    UINT8 ret=FALSE;
    
    UINT8 bsecond_ota = FALSE;
    UINT8 main_hdr=FALSE;
    UINT8 see_hdr=FALSE;
#ifdef _CAS9_VSC_ENABLE_    
    UINT8 vsc_hdr=FALSE;
#endif
    UINT32 pointer = 0;
    UINT32 data_id = 0;
    UINT32 data_len = 0;
    UINT32 data_off = 0;
    UINT32 check_cnt=0; 

    HDR_PRINTF ( "[%s]-st\n",__FUNCTION__ );
    while ( pointer < total_size )
    {
        data_id = sto_fetch_long ( ( UINT32 ) pointer + CHUNK_ID );
        data_len = sto_fetch_long ( ( UINT32 ) pointer + CHUNK_LENGTH );
        data_off = sto_fetch_long ( ( UINT32 ) pointer + CHUNK_OFFSET );
        
        /*HDR_PRINTF("@pointer[%08X] id[%08X] len[%08X] off[%08X]\n", \
                              pointer, data_id, data_len, data_off);
         */

        if ((0 != data_len) && (data_len < data_off) )
        {
            switch(data_id)
            {
               case CHUNKID_SECLOADER:
                    bsecond_ota = TRUE;
                    HDR_PRINTF("Find OTA LOADER at 0x%x!\n", pointer);
                break;
                
               case CHUNKID_MAINCODE:
                    main_hdr=TRUE;
                    HDR_PRINTF("Find main code at 0x%x!\n", pointer);                    
                break;
               case CHUNKID_SEECODE:
                    see_hdr=TRUE;
                    HDR_PRINTF("Find see code at 0x%x!\n", pointer);                                    
                break;
                
                #ifdef _CAS9_VSC_ENABLE_                    
                   case CHUNKID_VSC_CODE:
                        vsc_hdr=TRUE;
                        HDR_PRINTF("Find vsc code at 0x%x!\n", pointer);                                    
                    break;
                #endif           
                default: break;
            }                  
        }        
        pointer += C_SECTOR_SIZE;
        
        check_cnt++;    
        if(check_cnt>=MAX_FLASH_SECTOR_CHECK_CUNT)
        {
            HDR_PRINTF("run serach too many times ,check_cnt(0x%x)!\n", check_cnt);            
            break;
        }
    }

    if(TRUE==bsecond_ota)
    {
        if((FALSE==main_hdr) || (FALSE==see_hdr))
        {
            ret=TRUE;
        }
        #ifdef _CAS9_VSC_ENABLE_
        if(FALSE==vsc_hdr )
        {
            ret=TRUE;
        }        
        #endif
    }
    return ret;
}
#endif 

RET_CODE verify_otapara_hmac()
{
    RET_CODE ret = RET_FAILURE;
    UINT8 hmac_read[HMAC_OUT_LENGTH]={0};
    UINT8 hmac_cal[HMAC_OUT_LENGTH]={0};
    struct sto_device *flash_dev = NULL;
    UINT32 chunk_id=CHUNKID_OTA_PARA;
    UINT32 chunk_offset = 0;
    UINT8 *ota_para=NULL;

    flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
    if(flash_dev == NULL)
        return RET_FAILURE;
    
    MEMSET(hmac_read,0,HMAC_OUT_LENGTH);
    MEMSET(hmac_cal,0,HMAC_OUT_LENGTH);
    
    chunk_offset = (UINT32)sto_chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    if((ERR_PARA == chunk_offset)||(ERR_FAILUE== chunk_offset))
    {
        HDR_PRINTF("verify_otapara_hmac, sto_chunk_goto %d fail\n", chunk_id);
        return RET_FAILURE;
    }

    ota_para = (UINT8 *)MALLOC(MAX_OTA_PARA_LEN+0x10+HMAC_OUT_LENGTH);
    if(NULL==ota_para)
    {
        return RET_FAILURE;
    }
    MEMSET(ota_para, 0, MAX_OTA_PARA_LEN+0x10+HMAC_OUT_LENGTH);
    
    ret = sto_get_data(flash_dev, ota_para, chunk_offset, MAX_OTA_PARA_LEN+0x10+HMAC_OUT_LENGTH);
    if((MAX_OTA_PARA_LEN+0x10+HMAC_OUT_LENGTH)!=ret )
    {
        HDR_PRINTF("verify_otapara_hmac, sto_get_data fail, ret %d\n", ret);
        FREE(ota_para);
        return RET_FAILURE;           
    } 
    MEMCPY(hmac_read, &ota_para[MAX_OTA_PARA_LEN+0x10], HMAC_OUT_LENGTH);

    ret=api_gen_hmac3_chunkdata(0,&ota_para[0], MAX_OTA_PARA_LEN+0x10, hmac_cal);
    if(RET_SUCCESS != ret)
    {
        FREE(ota_para);
        return RET_FAILURE;
    }

    if(0==MEMCMP(hmac_read,hmac_cal,HMAC_OUT_LENGTH))
    {
        libc_printf("verify_otapara_hmac success\n");
        ret=RET_SUCCESS;
    }
    else
        ret=RET_FAILURE;
   
    FREE(ota_para);
    return ret;
}

RET_CODE update_otapara_hmac(UINT8 *p_ota_para ,const UINT16 ota_para_len)
{//save new otapara, generate HMAC and write to flash
    UINT8 hmac_data[HMAC_OUT_LENGTH]={0};
    RET_CODE ret=RET_FAILURE;
    UINT32 chunk_id=CHUNKID_OTA_PARA;
    UINT32 chunk_offset = 0;
    struct sto_device *flash_dev = NULL;
    UINT8 *ota_para=NULL;
    UINT8 *app_flash_buffer = NULL;

    MEMSET(hmac_data,0x00,HMAC_OUT_LENGTH); 
    flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
    if((NULL==p_ota_para) || ((MAX_OTA_PARA_LEN-0x70)<ota_para_len) || (NULL == flash_dev))
    {
        return RET_FAILURE;
    }

    ota_para = (UINT8 *)MALLOC(MAX_OTA_PARA_LEN+0x10+HMAC_OUT_LENGTH);
    if(NULL==ota_para)
    {
        return RET_FAILURE;
    }
    MEMSET(ota_para, 0, MAX_OTA_PARA_LEN+0x10+HMAC_OUT_LENGTH);
    
    chunk_offset = (UINT32)sto_chunk_goto(&chunk_id, 0xFFFFFFFF, 1);
    if((ERR_PARA == chunk_offset)||(ERR_FAILUE== chunk_offset))
    {
        HDR_PRINTF("update_otapara_hmac, sto_chunk_goto %d fail\n", chunk_id);
        FREE(ota_para);
        return RET_FAILURE;
    }
    
    ret = sto_get_data(flash_dev, ota_para, chunk_offset, MAX_OTA_PARA_LEN+0x10);
    if((MAX_OTA_PARA_LEN+0x10)!=ret )
    {
        HDR_PRINTF("update_otapara_hmac, sto_get_data fail, ret %d\n", ret);
        FREE(ota_para);
        return RET_FAILURE;           
    } 

    MEMCPY(&ota_para[CHUNK_HEADER_SIZE], p_ota_para, ota_para_len);
   
    ret=api_gen_hmac3_chunkdata(0,&ota_para[0], MAX_OTA_PARA_LEN+0x10, hmac_data);
    if(RET_SUCCESS != ret)
    {
        FREE(ota_para);
        return RET_FAILURE;
    }

    MEMCPY(&ota_para[MAX_OTA_PARA_LEN+0x10], hmac_data, HMAC_OUT_LENGTH);

    app_flash_buffer = (UINT8 *)MALLOC(64*1024);
    if(NULL==app_flash_buffer)
    {   
        FREE(ota_para);
        return RET_FAILURE;
    }

    sto_io_control(flash_dev, STO_DRIVER_SECTOR_BUFFER, (UINT32)app_flash_buffer);
    sto_io_control(flash_dev, STO_DRIVER_SET_FLAG, STO_FLAG_AUTO_ERASE|STO_FLAG_SAVE_REST);
    
    ret = sto_put_data(flash_dev, chunk_offset+CHUNK_HEADER_SIZE, &ota_para[CHUNK_HEADER_SIZE], MAX_OTA_PARA_LEN-0x70+HMAC_OUT_LENGTH);
    sto_io_control(flash_dev, STO_DRIVER_SECTOR_BUFFER, 0);
    sto_io_control(flash_dev, STO_DRIVER_SET_FLAG,0);
    
    if(app_flash_buffer)
    {
        FREE(app_flash_buffer);
        app_flash_buffer=NULL;
    }

    if(ret!=(MAX_OTA_PARA_LEN-0x70+HMAC_OUT_LENGTH))
    {
        HDR_PRINTF("update_otapara_hmac, sto_put_data fail, ret %d\n", ret);
        ret = RET_FAILURE;
    }
    else
        ret = RET_SUCCESS;

    FREE(ota_para);
    return ret;
}
#endif

