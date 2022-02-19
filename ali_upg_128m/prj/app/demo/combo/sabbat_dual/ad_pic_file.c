/***************************************************************************************************
 * brief:     feature "osd display picture" Components
 * detail:    support some components for "osd display picture"
 *            1. buffer manage
 *            2. file operation
 * author:    saicheong.yuen
 * date:      2014-9-26
 ***************************************************************************************************/

#include <sys_config.h>

#ifdef PIP_PNG_GIF

#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <hld/ge/ge.h>
#include <hld/osd/osddrv_dev.h>
#include <api/libimagedec/imagedec.h>
#include <hld/dis/vpo.h>
#include <api/libfs2/stdio.h>
#include <api/libmp/pe.h>

/* "ad_pic_conf.h" must be front, such as here, follow ad_pic_*.h */
#include "ad_pic_conf.h"

#ifdef    AD_PIC_FILE_DEBUG
#define   AD_PIC_FILE_PRINTF	libc_printf
#else
#define   AD_PIC_FILE_PRINTF(...)	do{}while(0)
#endif

/***************************************************************************************************
 * brief:     feature "osd display picture" Component: buffer manage
 * detail:    the component only manager the buffer for "osd display picture"
 * author:    saicheong.yuen
 * date:      2014-9-26
 ***************************************************************************************************/
typedef struct ad_pic_bufmgr_s
{
    UINT8          *deccode_buf;
    INT32          deccode_buflen;
    UINT8          *picture_buf;
    INT32          picture_buflen;
    UINT8          *fileoper_buf;
    INT32          fileoper_buflen;   
} ad_pic_bufmgr_t;


ad_pic_bufmgr_t ad_pic_bufmgr_obj;

#ifdef  OSD_SHOW_JPG_FOR_32M

int ad_pic_bufmgr_init()
{
    ad_pic_bufmgr_obj.deccode_buf     = (UINT8 *)(__MM_PVR_VOB_BUFFER_ADDR);
    ad_pic_bufmgr_obj.deccode_buflen  = 0x30000;
    ad_pic_bufmgr_obj.picture_buf     = ad_pic_bufmgr_obj.deccode_buf + ad_pic_bufmgr_obj.deccode_buflen;
    ad_pic_bufmgr_obj.picture_buf     = (UINT8 *)(((UINT32)(ad_pic_bufmgr_obj.picture_buf) + 3) & 0xFFFFFFFC);
    ad_pic_bufmgr_obj.picture_buflen  = 0x10000;
    ad_pic_bufmgr_obj.fileoper_buf    = ad_pic_bufmgr_obj.picture_buf + ad_pic_bufmgr_obj.picture_buflen;
    ad_pic_bufmgr_obj.fileoper_buflen = 0x8000;

    return RET_SUCCESS;
}

#define AD_PIC_FB_FRMLEN  (736*576*3/2)

int ad_pic_using = 0;

int ad_pic_bufmgr_init_logo()
{
    ad_pic_bufmgr_obj.fileoper_buf    = (__MM_FB_START_ADDR+__MM_FB_LEN - 0x30000);
    ad_pic_bufmgr_obj.fileoper_buflen = 0x30000;

    ad_pic_bufmgr_obj.deccode_buf     = ad_pic_bufmgr_obj.fileoper_buf - 0x60000;
    ad_pic_bufmgr_obj.deccode_buflen  = 0x60000;

    if (ad_pic_using == 0)
    {
        ad_pic_bufmgr_obj.picture_buf     = (UINT8 *)(ad_pic_bufmgr_obj.deccode_buf - AD_PIC_FB_FRMLEN);
        ad_pic_bufmgr_obj.picture_buflen  = AD_PIC_FB_FRMLEN; //0x100000
        ad_pic_using = 1;
    }
    else
    {
        ad_pic_bufmgr_obj.picture_buf     = (UINT8 *)(ad_pic_bufmgr_obj.deccode_buf - AD_PIC_FB_FRMLEN*2);
        ad_pic_bufmgr_obj.picture_buflen  = AD_PIC_FB_FRMLEN;
        ad_pic_using = 0;
    }    

    return RET_SUCCESS;
}

#else

int ad_pic_bufmgr_init()
{
    //ad_pic_bufmgr_obj.deccode_buf     = (UINT8 *)((__MM_MP_BUFFER_ADDR+0x10000) & ~(3<<29));
    ad_pic_bufmgr_obj.deccode_buf     = (UINT8 *)(__MM_PVR_VOB_BUFFER_ADDR);
    ad_pic_bufmgr_obj.deccode_buflen  = 0x480000;
    ad_pic_bufmgr_obj.picture_buf     = ad_pic_bufmgr_obj.deccode_buf + ad_pic_bufmgr_obj.deccode_buflen;
    ad_pic_bufmgr_obj.picture_buf     = (UINT8 *)(((UINT32)(ad_pic_bufmgr_obj.picture_buf) + 3) & 0xFFFFFFFC);
    ad_pic_bufmgr_obj.picture_buflen  = 0x100000;
    ad_pic_bufmgr_obj.fileoper_buf    = ad_pic_bufmgr_obj.picture_buf + ad_pic_bufmgr_obj.picture_buflen;
    ad_pic_bufmgr_obj.fileoper_buflen = 0x100000;

    return RET_SUCCESS;
}

#define AD_PIC_FB_FRMLEN  (1280*720*3/2)

int ad_pic_using = 0;

int ad_pic_bufmgr_init_logo()
{
    //ad_pic_bufmgr_obj.deccode_buf     = (UINT8 *)((__MM_MP_BUFFER_ADDR+0x10000) & ~(3<<29));
    ad_pic_bufmgr_obj.deccode_buf     = (UINT8 *)(__MM_PVR_VOB_BUFFER_ADDR+__MM_PVR_VOB_BUFFER_LEN-0x200000);
    ad_pic_bufmgr_obj.deccode_buflen  = 0xb00000;
    ad_pic_bufmgr_obj.fileoper_buf    = ad_pic_bufmgr_obj.deccode_buf + ad_pic_bufmgr_obj.deccode_buflen;
    ad_pic_bufmgr_obj.fileoper_buflen = 0x100000;


    if (ad_pic_using == 0)
    {
        ad_pic_bufmgr_obj.picture_buf     = (UINT8 *)(ad_pic_bufmgr_obj.fileoper_buf + ad_pic_bufmgr_obj.fileoper_buflen);
        ad_pic_bufmgr_obj.picture_buflen  = AD_PIC_FB_FRMLEN; //0x100000
        ad_pic_using = 1;
    }
    else
    {
        ad_pic_bufmgr_obj.picture_buf     = (UINT8 *)(ad_pic_bufmgr_obj.fileoper_buf + ad_pic_bufmgr_obj.fileoper_buflen + AD_PIC_FB_FRMLEN);
        ad_pic_bufmgr_obj.picture_buflen  = AD_PIC_FB_FRMLEN;
        ad_pic_using = 0;
    }   


    return RET_SUCCESS;
}
#endif

int ad_pic_bufmgr_get_decodebuffer(UINT8** decode_buf,  INT32* deccode_buflen)
{
    *decode_buf     = ad_pic_bufmgr_obj.deccode_buf;
    *deccode_buflen = ad_pic_bufmgr_obj.deccode_buflen;
    return RET_SUCCESS;
}

int ad_pic_bufmgr_get_picturebuffer(UINT8** picture_buf, INT32* picture_buflen)
{
    *picture_buf    = ad_pic_bufmgr_obj.picture_buf;
    *picture_buflen = ad_pic_bufmgr_obj.picture_buflen;
    return RET_SUCCESS;
}

int ad_pic_bufmgr_get_fileoperbuffer(UINT8** fileoper_buf, INT32* fileoper_buflen)
{
    *fileoper_buf    = ad_pic_bufmgr_obj.fileoper_buf;
    *fileoper_buflen = ad_pic_bufmgr_obj.fileoper_buflen;
    return RET_SUCCESS;
}

/***************************************************************************************************
 * brief:     feature "osd display picture" Component: draw picture
 * detail:    None
 * author:    saicheong.yuen
 * date:      2014-9-26
 ***************************************************************************************************/
// no thing now




/***************************************************************************************************
 * brief:     feature "osd display picture" Component: file operation
 * detail:    the component only manager the file operation
 *            1. file operation by pe cache
 *            2. file operation by memory
 * author:    saicheong.yuen
 * date:      2014-9-26
 ***************************************************************************************************/

#define AP_PIC_PECACHE_BLK_SIZE (64*1024)
#define AP_PIC_PECACHE_IMG_SIZE (AP_PIC_PECACHE_BLK_SIZE*2)

static int ad_pic_fop_pecache_open(file_h fh, const char* filename);
static int ad_pic_fop_pecache_close(file_h fh);
static int ad_pic_fop_pecache_read(file_h fh, BYTE *buf, UINT32 size);
static int ad_pic_fop_pecache_seek(file_h fh, long offset, UINT32 origin);
static int ad_pic_fop_pecache_tell(file_h fh);

static int ad_pic_fop_mem_open(file_h fh, const char* filename);
static int ad_pic_fop_mem_close(file_h fh);
static int ad_pic_fop_mem_read(file_h fh, BYTE *buf, UINT32 size);
static int ad_pic_fop_mem_seek(file_h fh, long offset, UINT32 origin);
static int ad_pic_fop_mem_tell(file_h fh);


typedef struct ad_pic_fop_s
{
    int (*file_open)(file_h fh, const char* filename);
    int (*file_close)(file_h fh);
    int (*file_read)(file_h fh, BYTE *buf, UINT32 size);
    int (*file_seek)(file_h fh, long offset, UINT32 origin);
    int (*file_tell)(file_h fh);
    int (*file_attach_buff)(file_h fh, const char* bufffile, UINT32 bufffile_len);
    int (*file_detach_buff)(file_h fh);
} ad_pic_fop_t;

typedef struct ad_pic_fhdl_s
{
    int           valid;
    int           pecache_id;
    UINT8         *pecache_buf;
    FILE          *mem_fp;  
    UINT8         *mem_addr;
    INT32         mem_pos;
    UINT32        mem_filesz;
    ad_pic_fop_t  *fop;
} ad_pic_fhdl_t;

ad_pic_fop_t ad_pic_fop_pecache =
{
    ad_pic_fop_pecache_open,
    ad_pic_fop_pecache_close,
    ad_pic_fop_pecache_read,
    ad_pic_fop_pecache_seek,
    ad_pic_fop_pecache_tell,
    NULL,
    NULL,
};

ad_pic_fop_t ad_pic_fop_mem =
{
    ad_pic_fop_mem_open,
    ad_pic_fop_mem_close,
    ad_pic_fop_mem_read,
    ad_pic_fop_mem_seek,
    ad_pic_fop_mem_tell,
    NULL,
    NULL,
};

static ad_pic_fhdl_t     g_ad_pic_fhdls[2];
/* 0 is reserved. 0 is unable to be used. */
//#define           AD_PIC_HDL_ALLOC   1
#define           AD_PIC_HDL_ALLOC   1


/***************************************************************************************************
 * brief:     file operation by pe cache
 * detail:    methods
 *            1. ad_pic_fop_pecache_open
 *            2. ad_pic_fop_pecache_close
 *            3. ad_pic_fop_pecache_read
 *            4. ad_pic_fop_pecache_seek
 *            5. ad_pic_fop_pecache_tell
 * author:    saicheong.yuen
 * date:      2014-9-26
 ***************************************************************************************************/

/* file operation implementer: open, using pe cache.*/
static int ad_pic_fop_pecache_open(file_h fh, const char* filename)
{
    UINT8         *fileoper_buf;
    INT32         fileoper_buflen; 
    int           pecache_id = -1;
    //UINT8         *pecache_buf = NULL;
    ad_pic_fhdl_t *phdl;
    phdl = &g_ad_pic_fhdls[fh];

    ad_pic_bufmgr_get_fileoperbuffer(&fileoper_buf, &fileoper_buflen);

    phdl->pecache_buf = fileoper_buf;
    if (phdl->pecache_buf == NULL)
    {
        AD_PIC_FILE_PRINTF("%s %d: file buffer is NULL\n", __FILE__, __LINE__);
        return RET_FAILURE;
    }
    pecache_id = pe_cache_open((char *)filename, phdl->pecache_buf, AP_PIC_PECACHE_IMG_SIZE, AP_PIC_PECACHE_BLK_SIZE);

    if (pecache_id < 0)
    {
        AD_PIC_FILE_PRINTF("%s %d: pecache_id fail\n", __FILE__, __LINE__);
        return RET_FAILURE;
    }

    phdl->pecache_id = pecache_id;
    return RET_SUCCESS;
}

/* file operation implementer: close, using pe cache.*/
static int ad_pic_fop_pecache_close(file_h fh)
{
    ad_pic_fhdl_t *phdl;
    phdl = &g_ad_pic_fhdls[fh];

    pe_cache_close(phdl->pecache_id);
    phdl->pecache_id = -1;

    return RET_SUCCESS;
}

/* file operation implementer: read, using pe cache.*/
static int ad_pic_fop_pecache_read(file_h fh, BYTE *buf, UINT32 size)
{
    UINT32        ret = 0;
    int           pecache_id;
    ad_pic_fhdl_t *phdl;

    phdl = &g_ad_pic_fhdls[fh];

    if (size == 0)
        return 0;

    pecache_id = phdl->pecache_id;
 
    if (pecache_id < 0)
    {
        AD_PIC_FILE_PRINTF("%s(): read pecache fail!!!\n", __FUNCTION__);
        return 0;
    }

    do 
    {
        ret = pe_cache_read(pecache_id, buf, size, 0xFFFFFFFF);
        if ((ret > 0) && (ret <= size))
        {
            break;
        }
        else if(ret > size)
        {
            ret = 0;
            break;
        }
        if (pe_cache_check_eof(pecache_id)) 
        {
            break;
        }
        osal_task_sleep(100);
    } while(ret == 0);
    return (int)ret;
}

/* file operation implementer: seek, using pe cache.*/
static int ad_pic_fop_pecache_seek(file_h fh, long offset, UINT32 origin)
{
    int           ret = RET_SUCCESS;
    int           pecache_id;
    ad_pic_fhdl_t *phdl;

    phdl = &g_ad_pic_fhdls[fh];

    pecache_id = phdl->pecache_id;
    if (pecache_id < 0)
        return RET_FAILURE;

    ret = pe_cache_seek(pecache_id, (off_t)offset, origin);
    if (ret == RET_FAILURE)
    {
        AD_PIC_FILE_PRINTF("%s(): seek pecache fail!!!\n", __FUNCTION__);
        return RET_FAILURE;
    }
    return RET_SUCCESS;

}

/* file operation implementer: tell, using pe cache.*/
static int ad_pic_fop_pecache_tell(file_h fh)
{
    int           offset = 0;
    int           pecache_id;
    ad_pic_fhdl_t *phdl;

    phdl = &g_ad_pic_fhdls[fh];

    pecache_id = phdl->pecache_id;
    if (pecache_id < 0)
        return 0;

    offset = pe_cache_tell(pecache_id);
    return offset;
}


/***************************************************************************************************
 * brief:     file operation by memory
 * detail:    methods
 *            1. ad_pic_fop_mem_open
 *            2. ad_pic_fop_mem_close
 *            3. ad_pic_fop_mem_read
 *            4. ad_pic_fop_mem_seek
 *            5. ad_pic_fop_mem_tell
 * author:    saicheong.yuen
 * date:      2014-9-26
 ***************************************************************************************************/
 
/* file operation implementer: open, using mem.*/
static int ad_pic_fop_mem_open(file_h fh, const char* filename)
{
    UINT8          *fileoper_buf;
    INT32          fileoper_buflen; 

    ad_pic_fhdl_t *phdl;

    phdl = &g_ad_pic_fhdls[fh];

    //UINT32 readsize=0;
    phdl->mem_fp = fopen(filename, "rb");
    if(phdl->mem_fp == NULL)
    {
        AD_PIC_FILE_PRINTF(" file handle is null\n");
        return RET_FAILURE;
    }

    ad_pic_bufmgr_get_fileoperbuffer(&fileoper_buf, &fileoper_buflen);
    #if 0
    phdl->mem_addr = phdl->file_buf;
    phdl->mem_filesz = fread(phdl->mem_addr, 1, AP_PIC_BUF_FILELEN, phdl->mem_fp);
    #endif

    phdl->mem_addr = fileoper_buf;
    phdl->mem_filesz = fread(phdl->mem_addr, 1, fileoper_buflen, phdl->mem_fp);
    if ((0 == phdl->mem_filesz) || (((UINT32)fileoper_buflen) == phdl->mem_filesz))
    {
        AD_PIC_FILE_PRINTF(" file size is zero or too big! (%d)\n", phdl->mem_filesz);
        return RET_FAILURE;
    }
    AD_PIC_FILE_PRINTF(" file size is %d\n",  phdl->mem_filesz);
    phdl->mem_pos = 0;

    return RET_SUCCESS;
};

/* file operation implementer: close, using mem.*/
static int ad_pic_fop_mem_close(file_h fh)
{
    ad_pic_fhdl_t *phdl;
    
    phdl = &g_ad_pic_fhdls[fh];

    fclose(phdl->mem_fp);
    return 0;
};

/* file operation implementer: read, using mem.*/
static int ad_pic_fop_mem_read(file_h fh, BYTE *buf, UINT32 size)
{
	UINT32 read_pos;
	UINT32 read_len;
    ad_pic_fhdl_t *phdl;
    
    phdl = &g_ad_pic_fhdls[fh];

	read_pos = phdl->mem_pos+size;
	if(read_pos > phdl->mem_filesz)
	{
		read_len = phdl->mem_filesz - phdl->mem_pos;
		MEMCPY((void *)buf, (void *)(phdl->mem_addr + phdl->mem_pos), read_len);
		phdl->mem_pos += read_len;
		return read_len;
	}
	else
	{
		MEMCPY((void *)buf, (void *)(phdl->mem_addr + phdl->mem_pos), size);
		phdl->mem_pos += size;
		return size;
	}
};

/* file operation implementer: seek, using mem.*/
static int ad_pic_fop_mem_seek(file_h fh, long offset, UINT32 origin)
{
    ad_pic_fhdl_t *phdl;
        
    phdl = &g_ad_pic_fhdls[fh];

	if(origin == 0)
	{
		phdl->mem_pos = (UINT32)offset;
		return RET_SUCCESS;
	}
	return RET_FAILURE;
};

/* file operation implementer: tell, using mem.*/
static int ad_pic_fop_mem_tell(file_h fh)
{
    ad_pic_fhdl_t *phdl;
        
    phdl = &g_ad_pic_fhdls[fh];

	return phdl->mem_pos;
};

/***************************************************************************************************
 * brief:     file operation adapter, API
 * detail:    methods
 *            1. ad_pic_file_init
 *            2. ad_pic_file_exit
 *            3. ad_pic_file_read
 *            4. ad_pic_file_seek
 *            5. ad_pic_file_tell
 * author:    saicheong.yuen
 * date:      2014-9-26
 ***************************************************************************************************/

/* file operation adapter: read */
int ad_pic_file_read(file_h fh, BYTE *buf, UINT32 size)
{
    ad_pic_fhdl_t *phdl;
    phdl = &g_ad_pic_fhdls[fh];

    return  phdl->fop->file_read(fh, buf, size);
}

/* file operation adapter: seek */
int ad_pic_file_seek(file_h fh, long offset, UINT32 origin)
{
    ad_pic_fhdl_t *phdl;
    phdl = &g_ad_pic_fhdls[fh];

    return phdl->fop->file_seek(fh, offset, origin);
}

/* file operation adapter: tell */
int ad_pic_file_tell(file_h fh)
{
    ad_pic_fhdl_t *phdl;
    phdl = &g_ad_pic_fhdls[fh];

    return phdl->fop->file_tell(fh);
}

static int ad_pic_file_getfop(file_h fh, char *filename)
{
    ad_pic_fhdl_t *phdl;
    phdl = &g_ad_pic_fhdls[fh];

#ifndef ENABLE_PE_CACHE
    phdl->fop = &ad_pic_fop_mem;
    AD_PIC_FILE_PRINTF("selet the mem file op!\n");
	return RET_SUCCESS;
#else
	char *ext;
	ext = strrchr(filename, '.');
	if (ext)
	{
		if((!strncasecmp(ext, ".png", 4)) || (!strncasecmp(ext, ".PNG", 4)))
		{  
		    //phdl->fop = &ad_pic_fop_pecache;
            phdl->fop = &ad_pic_fop_mem;
            AD_PIC_FILE_PRINTF("selet the pecache file op!\n");
			return RET_SUCCESS;
		}
		if((!strncasecmp(ext, ".gif", 4)) || (!strncasecmp(ext, ".GIF", 4)))
		{       
		    phdl->fop = &ad_pic_fop_mem;
            AD_PIC_FILE_PRINTF("selet the mem file op!\n");
			return RET_SUCCESS;
		}
        if((!strncasecmp(ext, ".jpg", 4)) || (!strncasecmp(ext, ".JPG", 4)))
        {         
            phdl->fop = &ad_pic_fop_pecache;
            AD_PIC_FILE_PRINTF("selet the pecache file op!\n");
            return RET_SUCCESS;;
        }
        if((!strncasecmp(ext, ".jpeg", 4)) || (!strncasecmp(ext, ".JPEG", 4)))
        {
            phdl->fop = &ad_pic_fop_pecache;
            AD_PIC_FILE_PRINTF("selet the pecache file op!\n");
            return RET_SUCCESS;
        }
        if((!strncasecmp(ext, ".bmp", 4)) || (!strncasecmp(ext, ".BMP", 4)))
        {
            phdl->fop = &ad_pic_fop_pecache;
            //phdl->fop = &ad_pic_fop_mem;
            AD_PIC_FILE_PRINTF("selet the pecache file op!\n");
            return RET_SUCCESS;
        }
	}
    phdl->fop = &ad_pic_fop_pecache;
    return RET_FAILURE;
#endif
}


/* file operation adapter: open */
int ad_pic_file_init(file_h *fh, const char* filename)
{
    int ret;
    ad_pic_fhdl_t *phdl;
    phdl = &g_ad_pic_fhdls[AD_PIC_HDL_ALLOC];

    ret = ad_pic_file_getfop(AD_PIC_HDL_ALLOC, (char *)filename);
    if (RET_SUCCESS != ret)
    {
        AD_PIC_FILE_PRINTF("%s %d: get file oper failed\n", __FILE__, __LINE__);
        return RET_FAILURE;
    }

    ret = phdl->fop->file_open((file_h)AD_PIC_HDL_ALLOC, filename);
    if (RET_SUCCESS != ret)
    {
        AD_PIC_FILE_PRINTF("%s %d: open file failed\n", __FILE__, __LINE__);
        return RET_FAILURE;
    }

    phdl->valid = 1;
    *fh = AD_PIC_HDL_ALLOC;
    return RET_SUCCESS;
}

/* file operation adapter: close */
int ad_pic_file_exit(file_h fh)
{
    int ret;
    ad_pic_fhdl_t *phdl;
    phdl = &g_ad_pic_fhdls[fh];

    ret = phdl->fop->file_close(fh);
    if (RET_SUCCESS != ret)
        return RET_FAILURE;    

    phdl->valid = 0;
    return RET_SUCCESS;
}


/***************************************************************************************************
 * brief:     file operation by memory
 * detail:    methods
 *            1. ad_pic_bufffop_mem_open
 *            2. ad_pic_bufffop_mem_close
 *            3. ad_pic_bufffop_mem_read
 *            4. ad_pic_bufffop_mem_seek
 *            5. ad_pic_bufffop_mem_tell
 * author:    saicheong.yuen
 * date:      2014-11-11
 ***************************************************************************************************/

/* file operation implementer: read, using mem.*/
static int ad_pic_bufffop_mem_read(file_h fh, BYTE *buf, UINT32 size)
{
	UINT32 read_pos;
	UINT32 read_len;
    ad_pic_fhdl_t *phdl;
    
    phdl = &g_ad_pic_fhdls[fh];

	read_pos = phdl->mem_pos+size;
	if(read_pos > phdl->mem_filesz)
	{
		read_len = phdl->mem_filesz - phdl->mem_pos;
		MEMCPY((void *)buf, (void *)(phdl->mem_addr + phdl->mem_pos), read_len);
		phdl->mem_pos += read_len;
		return read_len;
	}
	else
	{
		MEMCPY((void *)buf, (void *)(phdl->mem_addr + phdl->mem_pos), size);
		phdl->mem_pos += size;
		return size;
	}
};

/* file operation implementer: seek, using mem.*/
static int ad_pic_bufffop_mem_seek(file_h fh, long offset, UINT32 origin)
{
    ad_pic_fhdl_t *phdl;
        
    phdl = &g_ad_pic_fhdls[fh];

	if(origin == 0)
	{
		phdl->mem_pos = (UINT32)offset;
		return RET_SUCCESS;
	}
	return RET_FAILURE;
};

/* file operation implementer: tell, using mem.*/
static int ad_pic_bufffop_mem_tell(file_h fh)
{
    ad_pic_fhdl_t *phdl;
        
    phdl = &g_ad_pic_fhdls[fh];

	return phdl->mem_pos;
};

static int ad_pic_bufffop_mem_attach(file_h fh, const char* bufffile, UINT32 bufffile_len)
{
    ad_pic_fhdl_t *phdl;
    phdl = &g_ad_pic_fhdls[fh];

    phdl->mem_addr = (UINT8 *)bufffile;
    phdl->mem_filesz = bufffile_len;
    if (0 == phdl->mem_filesz)
    {
        AD_PIC_FILE_PRINTF("ERR: file size is zero or too big!\n");
        return RET_FAILURE;
    }
    AD_PIC_FILE_PRINTF("INFO: mem_addr=0x%x file size is %d\n",  phdl->mem_addr, phdl->mem_filesz);
    phdl->mem_pos = 0;

    return RET_SUCCESS;
}

static int ad_pic_bufffop_mem_detach(file_h fh)
{
#if 0
    ad_pic_fhdl_t *phdl;
    phdl = &g_ad_pic_fhdls[fh];
    phdl->mem_addr = NULL;
    phdl->mem_filesz = 0;
    phdl->mem_pos = 0;
#endif
    return RET_SUCCESS;
}

ad_pic_fop_t ad_pic_fop_buffmem =
{
    NULL,
    NULL,
    ad_pic_bufffop_mem_read,
    ad_pic_bufffop_mem_seek,
    ad_pic_bufffop_mem_tell,
    ad_pic_bufffop_mem_attach,
    ad_pic_bufffop_mem_detach,
};


/* file operation adapter: open */
int ad_pic_bufffile_init(file_h *fh, const char* bufffile, UINT32 bufffile_len)
{
    int ret;
    ad_pic_fhdl_t *phdl;
    phdl = &g_ad_pic_fhdls[AD_PIC_HDL_ALLOC];

    phdl->fop = &ad_pic_fop_buffmem;

    ret = phdl->fop->file_attach_buff((file_h)AD_PIC_HDL_ALLOC, bufffile, bufffile_len);
    if (RET_SUCCESS != ret)
    {
        AD_PIC_FILE_PRINTF("ERR: %s %d: open file failed\n", __FILE__, __LINE__);
        return RET_FAILURE;
    }

    phdl->valid = 1;
    *fh = AD_PIC_HDL_ALLOC;
    return RET_SUCCESS;
}

int ad_pic_bufffile_exit(file_h fh)
{
    int ret;
    ad_pic_fhdl_t *phdl;
    phdl = &g_ad_pic_fhdls[fh];

    ret = phdl->fop->file_detach_buff(fh);
    if (RET_SUCCESS != ret)
        return RET_FAILURE;    

    phdl->valid = 0;
    return RET_SUCCESS;

}


#endif

