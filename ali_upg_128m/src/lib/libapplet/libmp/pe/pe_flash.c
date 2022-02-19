

#include <types.h>
#include <sys_config.h>
#include <retcode.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>
#include <hld/sto/sto_dev.h>
#include <api/libchunk/chunk.h>
#include <api/libc/string.h>
#include <api/libc/printf.h>
#include <api/libfs2/stdio.h>
#include <api/libfs2/types.h>
#include <api/libmp/pe.h>
#include "pe_flash.h"

#ifdef FD_PATH_HEADER

typedef struct
{
    struct sto_device *sto;
    int          devID;
    unsigned int chunkID;
    unsigned int chunklen;
    unsigned int chunkaddr;

    unsigned int data_len;
    unsigned int cur_off;
    unsigned int err;
    ID           rlock;
} flashdata_hdl;

#define FDDEBUG 0
#if FDDEBUG
#define FD_DEBUG(f,...) libc_printf("(%s:%d):"f, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define FD_DEBUG(...)
#endif

pe_cache_cb_fp fd_cb =
{
    .file_open  = (pe_cache_open_fp)fd_open,
    .file_read  = (pe_cache_read_fp)fd_read,
    .file_tell  = (pe_cache_ftell_fp)fd_tell,
    .file_eof   = (pe_cache_feof_fp)fd_eof,
    .file_seek  = (pe_cache_seek_fp)fd_seek,
    .file_close = (pe_cache_fclose_fp)fd_close
};

static UINT8 get_chunk_info(UINT32 id, UINT32 *addr, UINT32 *len)
{
    INT32 ret = 0;
    CHUNK_HEADER chuck_hdr;

    MEMSET(&chuck_hdr, 0, sizeof(CHUNK_HEADER));
    ret = sto_get_chunk_header(id, &chuck_hdr);
    if (0 == ret)
    {
        return 0;
    }
    *addr = (UINT32)sto_chunk_goto(&id, 0xFFFFFFFF, 1) + CHUNK_HEADER_SIZE;
    *len = chuck_hdr.len - CHUNK_HEADER_SIZE + CHUNK_NAME;
    return 1;
}

int is_flashdata(char *path)
{
    return (path && MEMCMP(path, FD_PATH_HEADER, strlen(FD_PATH_HEADER)) == 0);
}

// path format: flshchk://dev id:chuank id:data length
UINT32 fd_open(char *path, char *mode)
{
    int ret = 0;
    flashdata_hdl *hdl  = NULL;

    if (path == NULL || !is_flashdata(path))
    {
        return 0;
    }

    hdl = (flashdata_hdl *)MALLOC(sizeof(flashdata_hdl));
    if (hdl == NULL)
    {
        return 0;
    }

    MEMSET(hdl, 0, sizeof(flashdata_hdl));
    hdl->rlock = OSAL_INVALID_ID;

    hdl->rlock = osal_mutex_create();
    if (hdl->rlock == OSAL_INVALID_ID)
    {
        fd_close(hdl);
        FD_DEBUG("create mutex failed\n");
        return 0;
    }

    ret = sscanf(path + strlen(FD_PATH_HEADER), "%d:%x:%x",
                 &hdl->devID, &hdl->chunkID, &hdl->data_len);
    if (ret == -1)
    {
        fd_close(hdl);
        FD_DEBUG("sscanf failed: %s\n", path);
        return 0;
    }

    hdl->sto = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, hdl->devID);
    if (hdl->sto == NULL)
    {
        fd_close(hdl);
        FD_DEBUG("find STO device failed, devID:%d\n", hdl->devID);
        return 0;
    }

    if (get_chunk_info((UINT32)hdl->chunkID, (UINT32*)&hdl->chunkaddr, (UINT32*)&hdl->chunklen) == 0)
    {
        fd_close(hdl);
        FD_DEBUG("Get Chunk failed, chunkID:%d\n", hdl->chunkID);
        return 0;
    }
    if (hdl->data_len == 0) { hdl->data_len = hdl->chunklen; }

    FD_DEBUG("filename: %s\nstoID: %d, chunkID:0x%x, chunkAddr:0x%x, chunklen:0x%x, datalen:0x%x\n",
             path, hdl->devID, hdl->chunkID, hdl->chunkaddr, hdl->chunklen, hdl->data_len);
    return (UINT32)hdl;
}

void fd_close(void *file)
{
    flashdata_hdl *hdl = NULL;

    if (file)
    {
        hdl = (flashdata_hdl *)file;
        if (hdl->rlock != OSAL_INVALID_ID)
        {
            osal_mutex_delete(hdl->rlock);
        }
        FREE(hdl);
    }
}

size_t fd_read(void *ptr, size_t size, size_t nmemb, void *file)
{
    flashdata_hdl *hdl = NULL;
    size_t len = size * nmemb;
    int ret = 0;

    if (file == NULL || ptr == NULL || len == 0)
    {
        return 0;
    }

    hdl = (flashdata_hdl *)file;
    if (hdl->err) { return 0; }

    osal_mutex_lock(hdl->rlock, OSAL_WAIT_FOREVER_TIME);
    if (hdl->cur_off + len > hdl->chunklen)
    {
        len = hdl->chunklen - hdl->cur_off;
    }
    if (hdl->cur_off + len > hdl->data_len)
    {
        len = hdl->data_len - hdl->cur_off;
    }

    if (len > 0)
    {
        ret = sto_get_data(hdl->sto, ptr, hdl->chunkaddr + hdl->cur_off, len);
        if (ret > 0)
        {
            hdl->cur_off += ret;
        }
        else
        {
            hdl->err = 1;
            len = 0;
        }
    }
    osal_mutex_unlock(hdl->rlock);
    //FD_DEBUG("read data:%d\n", len);
    return len;
}

size_t fd_write(void *ptr, size_t size, size_t nmemb, void *file)
{
    libc_printf("unsupport!!\n");
    return 0;
}

int fd_seek(void *file, off_t offset, int whence)
{
    int ret = 0;
    flashdata_hdl *hdl = NULL;
    UINT32 off = (UINT32) offset;

    if (file == NULL)
    {
        return -1;
    }

    hdl = (flashdata_hdl *)file;
    osal_mutex_lock(hdl->rlock, OSAL_WAIT_FOREVER_TIME);
    switch (whence)
    {
        case SEEK_SET:
            if (off <= hdl->data_len)
            {
                hdl->cur_off = off;
            }
            else
            {
                ret = -1;
            }
            break;
        case SEEK_CUR:
            if (hdl->cur_off + off <= hdl->data_len)
            {
                hdl->cur_off += off;
            }
            else
            {
                ret = -1;
            }
            break;
        case SEEK_END:
            hdl->cur_off = hdl->data_len;
            break;
        default:
            ret = -1;
            break;
    }
    osal_mutex_unlock(hdl->rlock);

    FD_DEBUG("seek: whence:%d, offset:%d, ret:%d\n", whence, offset, ret);
    return ret;
}

off_t fd_tell(void *file)
{
    flashdata_hdl *hdl = NULL;

    if (file == NULL)
    {
        return 0;
    }

    hdl = (flashdata_hdl *)file;
    return (off_t)hdl->cur_off;
}

int fd_eof(void *file)
{
    int eof = 0;
    flashdata_hdl *hdl = NULL;

    if (file == NULL)
    {
        return 1;
    }

    hdl = (flashdata_hdl *)file;
    eof = ((hdl->cur_off == hdl->data_len)
           || (hdl->err)
           || (hdl->cur_off == hdl->chunklen));
    FD_DEBUG("eof: %d\n", eof);
    return eof;
}

extern void pecache_reg_filehdl(pe_cache_cb_fp *hdl);
void fd_reg_filehdl()
{
    pecache_reg_filehdl(&fd_cb);
}
void fd_unreg_filehdl()
{
    pecache_reg_filehdl(NULL);
}

#endif

