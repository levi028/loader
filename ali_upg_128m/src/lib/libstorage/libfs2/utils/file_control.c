/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: file_control.c
*
*    Description: control api about file
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <osal/osal.h>
#include <asm/chip.h>
#include <api/libc/string.h>
#include <api/libfs2/stdio.h>
#include <api/libfs2/stat.h>
#include <api/libfs2/dirent.h>
#include <api/libfs2/errno.h>
#include <api/libfs2/statvfs.h>
#include <api/libfs2/unistd.h>
#include <api/libfs2/libfc.h>

#define NULL ((void *)0)
#if BUFSIZ < 4096
#undef BUFSIZ
#define BUFSIZ 4096
#endif

typedef struct
{
    int task_running;
    const char source[2048];
    const char dest[2048];
    int flags;
    void *callback;
} copy_parameter;

enum { GROWBY = 80 }; /* how large we will grow strings by */

static int copyfd_cancel_flag = -1;
static copy_parameter cp_para = {0, {0}, {0}, 0, NULL};

static inline void fc_free(void *buf)
{
    if(buf)
    {
        free(buf);
        buf = NULL;
    }
}
static off_t fc_full_fd_action(int src_fd, int dst_fd, off_t size, void *cp_callback)
{
    int status = -1;
    off_t total = -1;
    off_t file_total_size = -1;
    unsigned long cp_percent = 0;
    unsigned long pre_cp_percent = 0;
    cp_callback_func        callback = NULL;
    off_t err_code = -1; // error code that will be return when failed
    char *buffer = NULL;
    ssize_t write_value = -1;
    ssize_t read_value = -1;

    if (src_fd < 0)
    {
        return -1;
    }

    buffer = (char *)malloc(BUFSIZ);
    if(NULL == buffer)
    {
        FC_DEBUG("%s(): malloc failed!\n", __FUNCTION__);
        return -1;
    }

    if (!size)
    {
        size = BUFSIZ;
        status = 1; /* copy until eof */
    }

    callback = (cp_callback_func)cp_callback;
    pre_cp_percent = 0;
    cp_percent = 0;
    file_total_size = fs_lseek(src_fd, 0, SEEK_END);
    fs_lseek(src_fd, 0, SEEK_SET);

    while (1)
    {
        if (copyfd_cancel_flag)
        {
            status = -1;
            break;
        }

        read_value = fs_read(src_fd, buffer, size > BUFSIZ ? BUFSIZ : size);
        FC_DEBUG("read %d bytes!\n", read_value);

        if (!read_value)   /* eof - all done. */
        {
            status = 0;
            break;
        }
        if (read_value < 0)
        {
            FC_DEBUG("msg_read_error");
            break;
        }
        /* dst_fd == -1 is a fake, else... */
        if (dst_fd >= 0)
        {
            write_value = fs_write(dst_fd, buffer, read_value);

            if (write_value < read_value)
            {
                if (write_value < 0)
                {
                    err_code = write_value; // set error code to indicate fail reason
                }
                FC_DEBUG("msg_write_error: %d", write_value);
                break;
            }
        }
        total += read_value;
        cp_percent = (unsigned long)(total * 100 / file_total_size); // fix BUG34109
        if ((cp_percent != pre_cp_percent) && callback)
        {
            pre_cp_percent = cp_percent;
            callback(3, cp_percent);
        }
        if (status < 0)
        {
            size -= read_value;
            if (!size)
            {
                status = 0;
                break;
            }
        }
    }

    fc_free(buffer);
    return status ? err_code : total;
}

#if 0  //for clean warning
static off_t fc_copy_fd_size(int src_fd, int dst_fd, off_t size, void *callback)
{
    if (size)
    {
        return fc_full_fd_action(src_fd, dst_fd, size, callback);
    }
    return 0;
}
#endif

static off_t fc_copy_fd_eof(int src_fd, int dst_fd, void *callback)
{
    return fc_full_fd_action(src_fd, dst_fd, 0, callback);
}


static char *fc_last_char_is(const char *string, int chr)
{
    if (string)
    {
        string = strrchr(string, chr);
        if (string && !string[1])
        {
            return (char*)string;
        }
    }
    return NULL;
}
static void *fc_malloc(size_t size)
{
    void *ptr = malloc(size);

    if ((NULL == ptr) && (size != 0))
    {
        FC_DEBUG("msg_memory_exhausted");
    }
    return ptr;
}

static void *fc_realloc(void *ptr, size_t size)
{
    ptr = (void *)realloc(ptr, size);
    if ((NULL == ptr) && (size != 0))
    {
        FC_DEBUG("msg_memory_exhausted");
    }
    return ptr;
}

static char *fc_readlink(const char *path)
{
    char *buf = NULL;
    int bufsize = -1;
    int readsize = -1;

    if(!path)
    {
        return NULL;
    }
    do
    {
        buf = fc_realloc(buf, bufsize += GROWBY);
        if (NULL == buf)
        {
            return NULL;
        }
        readsize = fs_readlink(path, buf, bufsize); /* 1st try */
        if (-1 == readsize)
        {
            FC_DEBUG("%s", path);
            fc_free(buf);
            return NULL;
        }
    }
    while (bufsize < readsize + 1);

    buf[readsize] = '\0';

    return buf;
}

static char *fc_concat_path_file(const char *path, const char *filename)
{
    char *outbuf = NULL;
    char *lc = NULL;
    int len = -1;

    if (!path || !filename)
    {
        return NULL;
    }
    lc = fc_last_char_is(path, '/');   //调用了fc_last_char_is函数
    while ('/' == (*filename))
    {
        filename++;
    }
    outbuf = fc_malloc(strlen(path) + strlen(filename) + 1 + (NULL == lc));
    if (NULL == outbuf)
    {
        return NULL;
    }
    len = strlen(path) + strlen(filename) + 1 + (NULL == lc);
    snprintf(outbuf, len, "%s%s%s", path, (NULL == lc) ? "/" : "", filename);

    return outbuf;
}
static char *fc_concat_subpath_file(const char *path, const char *f)
{
    if(!f || !path)
    {
        return NULL;
    }

    if (('.' == *f) && ((!f[1]) || (('.' == f[1]) &&(!f[2]))))
    {
        return NULL;
    }
    return fc_concat_path_file(path, f);
}

static int fc_retry_overwrite(const char *dest, int flags)
{
    if(!dest)
    {
        FC_DEBUG("dest is null\n");
        return -1;
    }
    if (!(flags & (FILEUTILS_FORCE | FILEUTILS_INTERACTIVE)))
    {
        FC_DEBUG("'%s' exists\n", dest);
        return -1;
    }
    if (fs_unlink(dest) < 0)
    {
        FC_DEBUG("cannot remove '%s'", dest);
        return -1; // error
    }
    return 1; // ok (to try again)
}

static int copy_link_file(const char *source, const char *dest)
{
    char *lpath = NULL;

    if((!source) || (!dest))
    {
        FC_DEBUG("null pointer in %s\n", __FUNCTION__);
        return -1;
    }
    lpath = fc_readlink(source);
    if (fs_symlink(lpath, dest) < 0)
    {
        FC_DEBUG("cannot create symlink '%s'", dest);
        fc_free(lpath);
        return -1;
    }
    fc_free(lpath);
    return 0;
}

static int copy_file(const char *source, const char *dest, int flags, void *cp_callback)
{
    struct stat source_stat;
    struct stat dest_stat;
    int status = -1;
    int dp = -1;
    struct dirent *entry = NULL;
    signed char ovr = -1;
    char *buff = NULL;                                //malloc(sizeof(struct dirent) + sizeof(char)*PATH_MAX);
    cp_callback_func        callback = (cp_callback_func)cp_callback;
    int src_fd = -1;
    int dst_fd = -1;
    off_t lret = -1;
    int ret = -1;
    int (*lf)(const char *oldpath, const char *newpath) = NULL;
    char *new_source = NULL;
    char *new_dest = NULL;

    if((!source) || (!dest))
    {
        FC_DEBUG("sth is null\n");
        return -1;
    }
    MEMSET(&source_stat, 0, sizeof(source_stat));
    if (fs_stat(source, &source_stat) < 0)
    {
        if (flags & (FILEUTILS_MAKE_SOFTLINK | FILEUTILS_MAKE_HARDLINK))
        {
            goto make_links;
        }
        FC_DEBUG("cannot stat '%s'", source);
        return -1;
    }
    if (S_ISDIR(source_stat.st_mode))
    {
        if (fs_stat(dest, &dest_stat) < 0)
        {
            ret = f_mkdir((char *)dest, FILEUTILS_RECUR);
            if (ret < 0)
            {
                FC_DEBUG( "dir: make dest dir fail\n");
                return ret;
            }
        }
        if ((dp = fs_opendir(source)) < 0)
        {
            FC_DEBUG( "dir: error opening: %s\n", source);
            return -1;
        }
        buff = malloc(sizeof(struct dirent) + sizeof(char)*PATH_MAX);
        if(NULL == buff)
        {
            return -1;
        }
        entry = (struct dirent *)buff;
        while (fs_readdir(dp, entry) > 0)
        {
            if (copyfd_cancel_flag)
            {
                status = 0;
                break;
            }
            new_source = fc_concat_subpath_file(source, entry->d_name);
            if (NULL == new_source)
            {
                continue;
            }
            new_dest = fc_concat_path_file(dest, entry->d_name);
            if (NULL == new_dest)
            {
                fc_free(new_source);
                continue;
            }
            ret = copy_file(new_source, new_dest, flags, callback);
            if (ret < 0)
            {
                status = ret;
                FC_DEBUG("copy_file(%s, %s) return %d\n", new_source, new_dest, ret);
                if (status == -ENOSPC)
                {
                    fc_free(new_source);
                    fc_free(new_dest);
                    break; // no space, break it
                }
            }
            fc_free(new_source);
            fc_free(new_dest);
        }
        fc_free(buff);
        fs_closedir(dp);
        status = 0;
    }
    else if (flags & (FILEUTILS_MAKE_SOFTLINK | FILEUTILS_MAKE_HARDLINK))
    {
make_links:
        lf = (flags & FILEUTILS_MAKE_SOFTLINK) ? fs_symlink : fs_link;
        if (lf(source, dest) < 0)
        {
            ovr = fc_retry_overwrite(dest, flags);
            if (ovr <= 0)
            {
                return ovr;
            }
            if (lf(source, dest) < 0)
            {
                FC_DEBUG("cannot create link '%s'", dest);
                return -1;
            }
        }
        return 0;
    }
    else if (S_ISREG(source_stat.st_mode))
    {
        src_fd = fs_open(source, O_RDONLY, 0);
        if (-1 == src_fd)
        {
            FC_DEBUG("cannot open '%s'", source);
            return -1;
        }
        if ((fs_stat(dest, &dest_stat) < 0) || (flags&FILEUTILS_FORCE))
        {
            dst_fd = fs_open(dest, O_WRONLY | O_CREAT | O_TRUNC, source_stat.st_mode);
            if (-1 == dst_fd)
            {
                dst_fd = fs_open(dest, O_WRONLY | O_CREAT | O_TRUNC, source_stat.st_mode);
                if (-1 == dst_fd)
                {
                    FC_DEBUG("cannot open '%s'", dest);
                    fs_close(src_fd);
                    return -1;
                }
            }
            if (callback)
            {
                callback(2, (unsigned long)source);
            }
            lret = fc_copy_fd_eof(src_fd, dst_fd, callback);
            if (lret < 0)
            {
                if (copyfd_cancel_flag)
                {
                    fs_unlink(dest);
                    status = 0;
                }
                else
                {
                    status = (int)lret;     // set error code to indicate fail reason
                }
            }
            else
            {
                status = 0;
            }
            if (fs_close(dst_fd) < 0)
            {
                FC_DEBUG("cannot close '%s'", dest);
                status = -1;
            }
            if (fs_close(src_fd) < 0)
            {
                FC_DEBUG("cannot close '%s'", source);
                status = -1;
            }
        }
        else
        {
            if (fs_close(src_fd) < 0)
            {
                FC_DEBUG("cannot close '%s'", source);
                status = -1;
            }
            return 0;
        }
    }
    else if (S_ISBLK(source_stat.st_mode) || S_ISCHR(source_stat.st_mode)
             || S_ISSOCK(source_stat.st_mode)  || S_ISLNK(source_stat.st_mode))
    {
        ovr = fc_retry_overwrite(dest, flags);
        if (ovr <= 0)
        {
            return ovr;
        }
        if (S_ISLNK(source_stat.st_mode))
        {
              return copy_link_file(source, dest);
        }
    }
    else
    {
        FC_DEBUG("internal error: unrecognized file type");
        return -1;
    }
    fs_sync(dest);
    return status;
}

static void copy_task(unsigned int param1,unsigned int param2)
{
    const char *source = NULL;
    const char *dest = NULL;
    int flags = -1;
    int ret = -1;
    cp_callback_func callback = NULL;
    copy_parameter *parameter = (copy_parameter *)param1;
    char *path = malloc(sizeof(char)*PATH_MAX);
    int lenth = -1;
    int final = -1;
    int s_len = -1;

    if(NULL == path)
    {
        return;
    }
    MEMSET(path, 0x0, PATH_MAX);

    source = parameter->source;
    dest = parameter->dest;
    flags = parameter->flags;
    callback = parameter->callback;

    lenth = 0;
    while (source[lenth] != '\0')
    {
        if ('/' == source[lenth])
        {
            final = lenth;
        }
        lenth++;
    }
    strncpy(path, dest, PATH_MAX-1);
    s_len = strlen(path);
    strncat(path, (char *)(source + final), PATH_MAX-1-s_len);

    ret = copy_file(source, path, flags, callback);
    if (ret < 0)
    {
        if (callback)
        {
            callback(1, ret); // return failed reason
        }
        parameter->task_running = FALSE;
        if (NULL != path)
        {
            fc_free(path);
        }
        return;
    }

    fs_sync(dest);
    if (callback)
    {
        callback(0,0);
    }
    parameter->task_running = FALSE;
    if (NULL != path)
    {
        fc_free(path);
    }
}

int f_cp(const char *source, const char *dest, int flags, void *callback)
{
    OSAL_T_CTSK task_attribute;
    OSAL_ID task_id = 0;

    MEMSET(&task_attribute, 0, sizeof(OSAL_T_CTSK));
    if ((!source)||(!dest) || (!callback))
    {
        return -1;
    }

    if (TRUE == cp_para.task_running)
    {
        return -1;
    }

    copyfd_cancel_flag = 0;
    cp_para.task_running = TRUE;
    cp_para.flags = flags;
    cp_para.callback = callback;
    strncpy((char*)cp_para.source, (char*)source, (2048-1));
    strncpy((char*)cp_para.dest, (char*)dest, (2048-1));

    //start thread
    task_attribute.stksz    = 0xF000;//0x2000;
    task_attribute.quantum    = 10;
    task_attribute.itskpri    = OSAL_PRI_NORMAL;
    task_attribute.para1    = (unsigned long)(&cp_para);
    //task_attribute.para2    =;
    task_attribute.name[0]    = 'C';
    task_attribute.name[1]    = 'p';
    task_attribute.name[2]    = 'y';
    task_attribute.task = (FP)copy_task;
    task_id = osal_task_create(&task_attribute);
    if (OSAL_INVALID_ID == task_id)
    {
        cp_para.task_running = FALSE;
        FC_DEBUG("Copy task create fail!! \n");

        return -1;
    }
    return 0;
}


int f_cancel_cp(void)
{
    osal_task_dispatch_off();
    copyfd_cancel_flag = 1;
    osal_task_dispatch_on();
    osal_task_sleep(10);

    return 0;
}

int f_mkdir (char *path, int flags)
{
    char *s = NULL;
    char c = -1;
    struct stat st;
    int ret = -1;
    int status = -1;
    int total = 0;//strlen(path);
    int judge = -1;
    char *newpath = NULL;
    int lenth = -1;

    if(!path)
    {
        return -1;
    }
    total = strlen(path);
    s = path;
    MEMSET(&st, 0, sizeof(struct stat));
    do
    {
        c = 0;
        if (flags & FILEUTILS_RECUR)      /* Get the parent. */
        {
            /* Bypass leading non-'/'s and then subsequent '/'s. */
            while (*s)
            {
                if ('/' == *s)
                {
                    do
                    {
                        ++s;
                    }
                    while ('/' == *s);
                    c = *s;        /* Save the current char */
                    *s = 0;        /* and replace it with nul. */
                    break;
                }
                ++s;
            }
        }
        newpath = malloc(sizeof(char)*PATH_MAX);
        MEMSET(newpath, 0, PATH_MAX);
        if (NULL == newpath)
        {
            status = -1;
            break;
        }
        //strncpy(newpath, path);
        strncpy(newpath, path, PATH_MAX-1);
        lenth = strlen(newpath);
        if ( ('/' == *newpath) && (1 == lenth) )
        {
            *s = c;
            fc_free(newpath);
            continue;
        }
        if (total != lenth)
        {
            newpath[lenth-1] = '\0';
        }
        judge = strlen(newpath);
        if ((judge > 1) && ('/' == newpath[judge-1]))
        {
            newpath[judge-1] = '\0';
        }
        ret = fs_mkdir(newpath, 0);
        FC_DEBUG("current dir is %s, ret = %d \n",path,ret);
        if (ret < 0)
        {
            /* If we failed for any other reason than the directory
             * already exists, output a diagnostic and return -1.*/
            if ( (-17 == ret) || (-2 == ret) )
            {
                *s = c;
                if (total == lenth)
                {
                    FC_DEBUG(" %s create is finished!\n", path);
                    status = 0;
                    fc_free(newpath);
                    break;
                }
                else
                {
                    fc_free(newpath);
                    continue;
                }
            }
            if ( !(flags & FILEUTILS_RECUR)
                    || (fs_stat(path, &st) < 0 || !S_ISDIR(st.st_mode)))
            {
                FC_DEBUG("cannot create directory '%s'", path);
                status = ret; // set error code to indicate fail reason
                fc_free(newpath);
                break;
            }
        }
        *s = c;
        fc_free(newpath);
    }
    while (1);
    return status;
}



int f_rm(const char *path, int flags)
{
    int status = -1;
    int path_exists = -1;
    int dirfd = -1;
    int err = -1;
    struct stat st;
    struct stat path_stat;
    struct dirent *entry = NULL;
    char *syncpath = malloc(sizeof(char)*PATH_MAX);
    int final = -1;
    int lenth = -1;
    char *buff = NULL;
    char *dirname = NULL;
    int s_len = -1;

    MEMSET(&st, 0, sizeof(struct stat));
    MEMSET(&path_stat, 0, sizeof(struct stat));
    if(NULL == syncpath)
    {
        return -1;
    }
    if(!path)
    {
        fc_free(syncpath);
        return -1;
    }
    while (path[lenth] != '\0')
    {
        syncpath[lenth] = path[lenth];
        if ('/' == syncpath[lenth])
        {
            final = lenth;
        }
        lenth++;
    }
    syncpath[final] = '\0';

    if (fs_stat(path, &path_stat) < 0)
    {
        path_exists = 0;
    }

    if (!path_exists)
    {
        if (!(flags & FILEUTILS_FORCE))
        {
            FC_DEBUG("cannot remove '%s'", path);
            fc_free(syncpath);
            return -1;
        }
        fc_free(syncpath);
        return 0;
    }

    if (S_ISDIR(path_stat.st_mode))
    {

        if (!(flags & FILEUTILS_RECUR))
        {
            FC_DEBUG("%s: is a directory", path);
            fc_free(syncpath);
            return -1;
        }
        if ((dirfd = fs_opendir(path)) < 0)
        {
            FC_DEBUG( "dir: error opening: %s, return %d", path, dirfd);
            fc_free(syncpath);
            return -1;
        }

        dirname = malloc(sizeof(char) * PATH_MAX);
        if (NULL == dirname)
        {
            FC_DEBUG( "dir: error malloc");
            fc_free(syncpath);
            return -1;
        }
        MEMSET(dirname, 0, PATH_MAX);
        buff = malloc(sizeof(struct dirent) + sizeof(char)*PATH_MAX);
        if(NULL == buff)
        {
            FC_DEBUG( "dir: error malloc");
            fc_free(syncpath);
            fc_free(dirname);
            return -1;
        }
        entry = (struct dirent *)buff;
        while (1)
        {
            err = fs_readdir(dirfd, entry);
            if ( err <= 0 )
            {
                break;
            }

            strncpy(dirname, path, PATH_MAX-1);
            s_len = strlen(dirname);
            strncat(dirname, "/", PATH_MAX-1-s_len);

            if (('.' == entry->d_name[0]) && ((!entry->d_name[1]) || (('.' == entry->d_name[1]) && (!entry->d_name[2]))))
            {
                //err=fs_readdir(dirfd, entry);
                continue;
            }
            s_len = strlen(dirname);
            strncat(dirname, entry->d_name, PATH_MAX-1-s_len);

            err = fs_stat(dirname, &st);
            if (0 == err)
            {
                // dir entries may be change, after deleting a file.
                // Rewind dir to ensure all files in this fold are deleted.
                if (f_rm(dirname, flags) < 0)
                {
                    status = -1;
                }
                else
                {
                    fs_rewinddir(dirfd);
                }
            }

        }
        fc_free(dirname);
        fc_free(buff);
        if (fs_closedir(dirfd) < 0)
        {
            FC_DEBUG("cannot close '%s'", path);
            fc_free(syncpath);
            return -1;
        }

        if (fs_rmdir(path) < 0)
        {
            FC_DEBUG("cannot remove '%s'", path);
            fc_free(syncpath);
            return -1;
        }

    }
    else
    {

        if (fs_remove(path) < 0)
        {
            FC_DEBUG("cannot remove '%s'", path);
            fc_free(syncpath);
            return -1;
        }
        status = 0;
    }
    fs_sync(syncpath);
    fc_free(syncpath);
    return status;
}


int f_mv(const char *source, const char *dest, int flags)
{
    char *path = malloc(sizeof(char)*PATH_MAX);
    int lenth = -1;
    int final = -1;
    int s_len = -1;

    if (NULL == path)
    {
        return -1;
    }
    MEMSET(path, 0, PATH_MAX);
    if((!source)||(!dest))
    {
        fc_free(path);
        return -1;
    }
    while (source[lenth] != '\0')
    {
        if ('/' == source[lenth])
        {
            final = lenth;
        }
        lenth++;
    }
    //strcpy(path, dest);
    strncpy(path, dest, PATH_MAX-1);
    s_len = STRLEN(path);
    //strcat(path, (char *)(source + final));
    strncat(path, (char *)(source + final), PATH_MAX-1-s_len);

    if (fs_rename(source, path) < 0)
    {
        FC_DEBUG("fs_rename(source, path) < 0\n");

        if (copy_file(source, path, flags,NULL) < 0)
        {
            fc_free(path);
            FC_DEBUG("Can not move %s to %s\n", source, path);
            return -1;
        }
        if (f_rm(source, FILEUTILS_RECUR) < 0)
        {
            fc_free(path);
            FC_DEBUG("Can not remove %s\n", source);
            return -1;
        }
    }
    fc_free(path);
    fs_sync(dest);
    return 0;
}

