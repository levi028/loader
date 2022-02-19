
#include <types.h>
#include <retcode.h>
#include <sys_config.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/sci/sci.h>
#include <hld/hld_dev.h>
#include <osal/osal_timer.h>
#include <api/libc/list.h>
#include <api/libc/time.h>
#include <api/libfs2/dirent.h>
#include <api/libfs2/errno.h>
#include <api/libfs2/fcntl.h>
#include <api/libfs2/stat.h>
#include <api/libfs2/statvfs.h>
#include <api/libfs2/stdio.h>
#include <api/libfs2/unistd.h>
#include <api/libmp/media_player_api.h>
#include <api/libmp/mp_list.h>
#include <api/libfs2/libfc.h>

#include <api/../../src/lib/libapplet/libmp/lists/file_list.h>
#include <api/../../src/lib/libapplet/libmp/lists/play_list.h>
#include <api/../../src/lib/libapplet/libmp/lists/quick_sort.h>
#include <api/../../src/lib/libapplet/libmp/lists/utils.h>

#define MAX_VIDEO_FILE_NUM 128
#define MAX_MUSIC_FILE_NUM 1024	//128
#define MAX_IMAGE_FILE_NUM 1024
#define SCAN_TASK_STCKSIZE 0x8000
#define DEP_PATH_LEN   2048
#define DIRENT_BUF_LEN 2048
#define INDEX_FILE_PATH_LEN 128
#define SITI_PRJ
#define MEDIATYPE(t) (t==VIDEO_FILE_LIST?("Video"):(t==MUSIC_FILE_LIST?"Music":"Photo"))

//#define DEBUG_IN_FILE_LIST
#ifdef  DEBUG_IN_FILE_LIST
#define DEBUG_IN_FILE_LIST	libc_printf
#else
#define DEBUG_IN_FILE_LIST(...)	do{}while(0)
#endif

typedef struct
{
    char  dev[4];
    FILE *fp;
    int   filenum;
    int   max_filenum;
    int   deps;
    int   type;
    int   bypart;
    char  file[1024];
    char  dep_path[DEP_PATH_LEN];
    char  dirent_buf[DIRENT_BUF_LEN];
} filelst_scan;


FILE *cur_index_file = NULL;
/*
static char *strchr_i(char *s, char c, int index)
{
    int   i   = 0;
    char *ptr = s;

    while (*ptr)
    {
        if (*ptr == c) { i++; }
        if (i == index)
        {
            return ptr;
        }
        ptr++;
    }
    return NULL;
}
*/
static char *strchr_r(char *s, char c)
{
    int len = strlen(s) - 1;
    while (len >= 0)
    {
        if (s[len] == c) { break;}
        len--;
    }
    return (len >= 0 ? s + len : NULL);
}

static void filelst_scan_reset(filelst_scan *scan)
{
    if (scan)
    {
        scan->deps = 0;
        scan->fp = NULL;
        scan->filenum = 0;
        MEMSET(scan->file, 0, sizeof(scan->file));
        MEMSET(scan->dep_path, 0, sizeof(scan->dep_path));
    }
}

static int filelst_idxtable_get_offset(FILE *fp, UINT32 index)
{
    int ret = 0;
    UINT32 off = 0;

    ret = fseek(fp, index * 4, SEEK_SET);
    if (ret < 0)
    {
        return 0;
    }
    ret = fread((char *)&off, 1, 4, fp);
    if (ret < 0)
    {
        return 0;
    }
    return off;
}

static int filelst_idxtable_set_offset(FILE *fp, UINT32 index)
{
    int ret = 0;
    UINT32 off = 0;

    ret = fseek(fp, 0, SEEK_END);
    if (ret < 0)
    {
        return 0;
    }
    off = (UINT32)ftell(fp);
    ret = fseek(fp, index * 4, SEEK_SET);
    if (ret < 0)
    {
        return 0;
    }
    return fwrite((char *)&off, 1, 4, fp);
}

static int filelst_idxtable_get_num(FILE *fp)
{
    return filelst_idxtable_get_offset(fp, 0);
}

static int filelst_idxtable_set_num(FILE *fp, UINT32 num)
{
    int ret = 0;
    ret = fseek(fp, 0, SEEK_SET);
    if (ret < 0)
    {
        return 0;
    }
    return fwrite((char *)&num, 1, 4, fp);
}

static int filelst_idxtable_get_fname(FILE *fp, int index, char *path, int path_len)
{
    char *ptr = NULL;
    int ret = 0;
    UINT32 off = 0;

    off = filelst_idxtable_get_offset(fp, index);
    if (off == 0)
    {
        return 0;
    }
    ret = fseek(fp, off, SEEK_SET);
    if (ret < 0)
    {
        return 0;
    }
    ptr = fgets(path, path_len, fp);
    if (ptr)
    {
        path[strlen(path) - 1] = 0; //remove '\n'
        ret=1;
    }
    else
    {
        ret=0;
    }
    return (ret);
}

static int filelst_idxtable_set_fname(FILE *fp, char *filename)
{
    int ret = 0;

    ret = fseek(fp, 0, SEEK_END);
    if (ret < 0)
    {
        return 0;
    }
    fwrite(filename, 1, strlen(filename), fp);
    return fwrite("\n", 1, 1, fp);
}

static int filelst_init_index_table(FILE *fp, int filenum)
{
    int ret = 0;
    UINT32 len = 0;
    UINT32 off = 0;
    char buff[1024];

    if (fp == NULL)
    {
        return 0;
    }
    memset(buff, 0, 1024);
    len = (filenum + 1) * 4;

    do
    {
        off = len < 1024 ? len : 1024;
        ret = fwrite(buff, 1, off, fp);
        if (ret > 0)
        {
            len -= ret;
        }
        else
        {
            DEBUG_IN_FILE_LIST("fwrite err:%d\n", errno);
            break;
        }
    }
    while (len > 0);

    return ret;
}

static void filelst_update_index_table(FILE *fp, int file_index)
{
    filelst_idxtable_set_num(fp, file_index);
    filelst_idxtable_set_offset(fp, file_index);
}

RET_CODE filelst_idx_file_path(file_list_type type, char *dirpath, char *out, int out_len)
{
    char *index_file = NULL;

    switch (type)
    {
        case VIDEO_FILE_LIST:
            index_file = "alimpvideo.ini";
            break;
        case MUSIC_FILE_LIST:
            index_file = "alimpmusic.ini";
            break;
        case IMAGE_FILE_LIST:
            index_file = "alimpimage.ini";
            break;
        case MUSIC_FAVORITE_LIST:
            index_file = "alifavorite.ini";
            break;
        default:
            return RET_FAILURE;
    }
    snprintf(out, out_len, "%s/%s", dirpath, index_file);
    return RET_SUCCESS;
}

static int filelst_max_file_num(int type)
{
    int max_filenum = 0;

    switch (type)
    {
        case VIDEO_FILE_LIST:
            max_filenum = MAX_VIDEO_FILE_NUM;
            break;
        case MUSIC_FAVORITE_LIST:
        case MUSIC_FILE_LIST:
            max_filenum = MAX_MUSIC_FILE_NUM;
            break;
        case IMAGE_FILE_LIST:
            max_filenum = MAX_IMAGE_FILE_NUM;
            break;
        default:
            break;
    }
    return max_filenum;
}

static void filelst_get_1st_partition(char *dev, char *path)
{
    DIR *dir = NULL;

    sprintf(path, "/mnt/%s", dev);
    dir = fopendir(path);
    if (dir)
    {
        fclosedir(dir);
        return;
    }
    sprintf(path, "/mnt/%s%d", dev, 1);
    dir = fopendir(path);
    if (dir)
    {
        fclosedir(dir);
        return;
    }
    path[0] = 0;
}

int filelst_scan_save_video(filelst_scan *scan, char *filename)
{
    return filelst_idxtable_set_fname(scan->fp, scan->dep_path);
}

int filelst_scan_save_music(filelst_scan *scan, char *filename)
{
    return filelst_idxtable_set_fname(scan->fp, scan->dep_path);
}

int filelst_scan_save_image(filelst_scan *scan, char *filename)
{
    return filelst_idxtable_set_fname(scan->fp, scan->dep_path);
}


int filelst_scan_save_file(filelst_scan *scan, char *filename)
{
    int ret = 0;
    int dep_len = 0;

    dep_len = strlen(scan->dep_path);
    if ((scan->filenum + 1 > scan->max_filenum) || (dep_len + strlen(filename) > DEP_PATH_LEN - 1))
    {
        return 0;
    }
    scan->filenum++;
    snprintf(scan->dep_path + dep_len, DEP_PATH_LEN - dep_len, "/%s", filename);
    filelst_update_index_table(scan->fp, scan->filenum);
    switch (scan->type)
    {
        case VIDEO_FILE_LIST:
            ret = filelst_scan_save_video(scan, filename);
            break;
        case MUSIC_FILE_LIST:
            ret = filelst_scan_save_music(scan, filename);
            break;
        case IMAGE_FILE_LIST:
            ret = filelst_scan_save_image(scan, filename);
            break;
        default:
            break;
    }
    scan->dep_path[dep_len] = 0;
    return ret;
}

int filelst_scan_check_file(filelst_scan *scan, char *filename)
{
    int ret = 0;
    char *extname = NULL;
    int file_type = F_UNKOWN;

    extname = strchr_r(filename, '.');
    if (extname == NULL)
    {
        return ret;
    }
    file_type = file_map_type(scan->type, extname + 1);
    if (F_UNKOWN != file_type)
    {
        ret = filelst_scan_save_file(scan, filename);
    }
    return ret;
}

static int filelst_scan_dir(filelst_scan *scan, char *dirname, int recursive)
{
    int ret = 0;
    DIR *dir = NULL;
    int dep_len = 0;
    f_dirent *entry = NULL;

    scan->deps++;
    if (scan->deps > 400)
    {
        return 0;
    } /*dangerous!!! stack maybe overflow*/

    entry = (f_dirent *)scan->dirent_buf;
    dep_len = strlen(scan->dep_path);
    if ((dep_len + strlen(dirname) + 2) > DEP_PATH_LEN)
    {
        DEBUG_IN_FILE_LIST("Directory too deep!!!!\n");
        return 0;
    }

    if (dep_len > 0)
    {
        strcat(scan->dep_path, "/");
    }
    strcat(scan->dep_path, dirname);
    dir = fopendir(scan->dep_path);
    if (dir)
    {
        while (ret >= 0 && (scan->filenum < scan->max_filenum))
        {
            MEMSET(entry, 0, DIRENT_BUF_LEN);
            ret = freaddir(dir, entry);
            if (ret <= 0)
            {
                ret = (ret == -ENOENT) ? 0 : -1;
                break;
            }
            if (dirs_skipped(entry->name))
            {
                continue;
            }
            if (entry->is_dir && recursive)
            {
                ret = filelst_scan_dir(scan, entry->name, recursive);
            }
            else
            {
                ret = filelst_scan_check_file(scan, entry->name);
            }
        }
        fclosedir(dir);
    }
    else
    {
        DEBUG_IN_FILE_LIST("open dir:%s failed!!!\n", scan->dep_path);
    }
    scan->dep_path[dep_len] = 0; //reset path
    scan->deps--;
    return ret;
}

int filelst_scan_part(filelst_scan *scan, char *root)
{
    int ret = 0;

    //DEBUG_IN_FILE_LIST("SCAN:%s For %s\n", root, MEDIATYPE(scan->type));
    filelst_scan_reset(scan);
    filelst_idx_file_path(scan->type, root, scan->file, sizeof(scan->file) - 1);
    scan->fp = fopen(scan->file, "w+");
    if (scan->fp == NULL)
    {
        DEBUG_IN_FILE_LIST("Create Index File [%s] fail!!!\n", scan->file);
        return 0;
    }
    //DEBUG_IN_FILE_LIST("Create Index File [%s]\n", scan->file);
    filelst_init_index_table(scan->fp, scan->max_filenum);
    ret = filelst_scan_dir(scan, root, 1);
    fclose(scan->fp);
    fsync(scan->file);
    scan->fp = NULL;
    DEBUG_IN_FILE_LIST("SCAN:%s OVER!! %s Count:%d\n", root, MEDIATYPE(scan->type), scan->filenum);
    return ret;
}

int filelst_scan_image(filelst_scan *scan, char *root)
{
    int ret  = 0;
    DIR *dir = NULL;
    f_dirent *entry = NULL;
    char path[INDEX_FILE_PATH_LEN] = {0};
    filelst_scan *nscan = NULL;

    nscan = MALLOC(sizeof(filelst_scan));
    if (nscan == NULL)
    {
        return 0;
    }
    memcpy(nscan, scan, sizeof(filelst_scan));
    filelst_scan_reset(nscan);
    strcpy(nscan->dep_path, root);

    filelst_idx_file_path(nscan->type, root, nscan->file, sizeof(nscan->file) - 1);
    nscan->fp = fopen(nscan->file, "w+");
    if (nscan->fp == NULL)
    {
        DEBUG_IN_FILE_LIST("Create Index File [%s] fail!!!\n", nscan->file);
        return 0;
    }
    DEBUG_IN_FILE_LIST("Create Index File [%s]\n", nscan->file);
    filelst_init_index_table(nscan->fp, nscan->max_filenum);

    dir = fopendir(root);
    if (dir)
    {
        entry = (f_dirent *)scan->dirent_buf;
        while (ret >= 0)
        {
            MEMSET(entry, 0, DIRENT_BUF_LEN);
            ret = freaddir(dir, entry);
            if (ret <= 0)
            {
                break;
            }

            if (dirs_skipped(entry->name))
            {
                continue;
            }
            if (entry->is_dir)
            {
                snprintf(path, sizeof(path) - 1, "%s/%s", root, entry->name);
                ret = filelst_scan_part(scan, path);
            }
            else
            {
                ret = filelst_scan_check_file(nscan, entry->name);
            }
        }
        fclosedir(dir);
    }

    fclose(nscan->fp);
    fsync(nscan->file);
    FREE(nscan);
    return ret;
}

void filelst_scan_parts(filelst_scan *scan)
{
    int ret = 0;
    DIR *dir = NULL;
    f_dirent *entry = NULL;
    char path[12] = {0};

    dir = fopendir("/mnt");
    if (dir)
    {
        entry = (f_dirent *)scan->dirent_buf;
        while (ret >= 0)
        {
            MEMSET(entry, 0, DIRENT_BUF_LEN);
            ret = freaddir(dir, entry);
            if (ret <= 0)
            {
                break;
            }
            if (dirs_skipped(entry->name))
            {
                continue;
            }
            if (entry->is_dir && strstr(entry->name, scan->dev))
            {
                sprintf(path, "/mnt/%s", entry->name);
                ret = filelst_scan_dir(scan, path, 1);
            }
        }
        fclosedir(dir);
    }
}

void filelst_scan_by_device(filelst_scan *scan)
{
    char path[12] = {0};

    filelst_get_1st_partition(scan->dev, path);
    if (strlen(path) == 0)
    {
        DEBUG_IN_FILE_LIST("CAN NOT FIND FIRST PARTITION IN DEV:%s\n", scan->dev);
        FREE(scan);
        return;
    }

    //DEBUG_IN_FILE_LIST("SCAN DEV:%s, first partition:%s\n", scan->dev, path);
    filelst_idx_file_path(scan->type, path, scan->file, sizeof(scan->file) - 1);
    scan->fp = fopen(scan->file, "w+");
    if (scan->fp == NULL)
    {
        DEBUG_IN_FILE_LIST("Create Index File [%s] fail!!!\n", scan->file);
        return;
    }
    MEMSET(scan->dep_path, 0, DEP_PATH_LEN);
    filelst_init_index_table(scan->fp, scan->max_filenum);
    filelst_scan_parts(scan);
    fclose(scan->fp);
    fsync(scan->file);
    DEBUG_IN_FILE_LIST("SCAN DEV:%s OVER! %s, Count:%d\n", scan->dev, MEDIATYPE(scan->type), scan->filenum);
}

void filelst_scan_by_partition(filelst_scan *scan)
{
    int ret = 0;
    DIR *dir = NULL;
    f_dirent *entry = NULL;
    char path[12] = {0};

    dir = fopendir("/mnt");
    if (dir)
    {
        entry = (f_dirent *)scan->dirent_buf;
        while (ret >= 0)
        {
            MEMSET(entry, 0, DIRENT_BUF_LEN);
            ret = freaddir(dir, entry);
            if (ret <= 0)
            {
                break;
            }
            if (dirs_skipped(entry->name))
            {
                continue;
            }
            if (entry->is_dir && strstr(entry->name, scan->dev))
            {
                snprintf(path, 11, "/mnt/%s", entry->name);
#ifdef SITI_PRJ //for siti prj, special mode
                if (scan->type == IMAGE_FILE_LIST)
                {
                    filelst_scan_image(scan, path);
                    continue;
                }
#endif
                ret = filelst_scan_part(scan, path);
            }
        }
        fclosedir(dir);
    }
}

void filelst_scan_task(UINT32 param1, UINT32 param2)
{
    filelst_scan *scan = (filelst_scan *)param1;

    if (scan == NULL)
    {
        return;
    }

    if (scan->bypart)
    {
        filelst_scan_by_partition(scan);
    }
    else
    {
        filelst_scan_by_device(scan);
    }
    FREE(scan);
}

void filelst_scan_device(char *dev, int type)
{
    OSAL_T_CTSK ptask;

    filelst_scan *scan = MALLOC(sizeof(filelst_scan));
    if (scan == NULL)
    {
        return;
    }
    MEMSET(scan, 0, sizeof(filelst_scan));
    strncpy(scan->dev, dev, 3);
    scan->type = type;
    scan->max_filenum = filelst_max_file_num(type);
    scan->bypart = 1;
    ptask.task = (FP)filelst_scan_task;
    ptask.itskpri = OSAL_PRI_NORMAL;
    ptask.quantum = 10;
    ptask.stksz = SCAN_TASK_STCKSIZE;
    ptask.para1 = (int)scan;
    ptask.para2 =  0;
    ptask.name[0] = 's';
    ptask.name[1] = 'c';
    ptask.name[2] = 'a';
    if (OSAL_INVALID_ID == osal_task_create(&ptask))
    {
        FREE(scan);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
RET_CODE filelst_open_index_file(P_FILE_LIST pfile_list, char *dirpath)
{
    char path[INDEX_FILE_PATH_LEN] = {0};

    if ((pfile_list == NULL) || (dirpath == NULL))
    {
        return RET_FAILURE;
    }
    if (cur_index_file)
    {
        fclose(cur_index_file);
        cur_index_file = NULL;
    }
    filelst_idx_file_path(pfile_list->type, dirpath, path, INDEX_FILE_PATH_LEN - 1);
    cur_index_file = fopen(path, "r+");
    return (cur_index_file != NULL);
}

RET_CODE filelst_get_fileinfo(P_FILE_LIST pfile_list, int index, p_file_info pinfo)
{
    char *ptr = NULL;

    if ((pfile_list == NULL) || (cur_index_file == NULL) || (pinfo == NULL) || (index <= 0))
    {
        return RET_FAILURE;
    }

    filelst_idxtable_get_fname(cur_index_file, index, pinfo->path, MAX_DIR_NAME_SIZE - 1);
    ptr = strchr_r(pinfo->path, '/');
    if (ptr == NULL)
    {
        return RET_FAILURE;
    }

    //DEBUG_IN_FILE_LIST("GET FILE:%s\n", pinfo->path);
    *ptr = 0;
    strncpy(pinfo->name, ptr + 1, MAX_FILE_NAME_SIZE - 1);
    return RET_SUCCESS;
}

P_FILE_LIST_ITEM filelst_get_file_item(P_FILE_LIST pfile_list, int index)
{
    char *ptr = NULL;
    char *filename = NULL;
    P_FILE_LIST_ITEM item = NULL;

    if (pfile_list == NULL || cur_index_file == NULL)
    {
        return NULL;
    }

    filename = MALLOC(1024);
    if (filename == NULL) { return NULL; }

    MEMSET(filename, 0, 1024);
    filelst_idxtable_get_fname(cur_index_file, index, filename, 1023);

    ptr = strchr_r(filename, '/');
    if (ptr)
    {
        item = MALLOC(sizeof(FILE_LIST_ITEM) + strlen(ptr));
        if (item)
        {
            MEMSET(item, 0, sizeof(FILE_LIST_ITEM) + strlen(ptr));
            strcpy(item->name, ptr + 1);
            ptr = strchr_r(item->name, '.');
            if (ptr)
            {
                item->type = file_map_type(pfile_list->type, ptr + 1);
                item->index = index;
            }
        }
    }
    FREE(filename);
    DEBUG_IN_FILE_LIST("GET Item %d: %s\n", item->index, item->name);
    return item;
}

RET_CODE filelst_read_idxfile_to_filelist(file_list_handle handle, char *dirpath)
{
    P_FILE_LIST pfile_list = NULL;
    INT32 filenum = 0;
    P_FILE_LIST_ITEM pfile_list_item = NULL;
    int i = 0;

    pfile_list = (P_FILE_LIST) handle;
    if ((pfile_list == NULL) || (cur_index_file == NULL))
    {
        return RET_FAILURE;
    }

    filenum = filelst_idxtable_get_num(cur_index_file);
    for (i = pfile_list->filenum + 1; i <= filenum; i++)
    {
        pfile_list_item = filelst_get_file_item(pfile_list, i);
        if (pfile_list_item)
        {
            list_add_tail(&pfile_list_item->listpointer, &pfile_list->head);
            pfile_list->filenum++;
        }
    }
    return RET_SUCCESS;
}

char *filelst_get_fullpath(P_FILE_LIST_ITEM file_list_item)
{
    char *fullpath = NULL;
    RET_CODE ret = 0;

    if (cur_index_file == NULL)
    {
        return NULL;
    }

    fullpath = MALLOC(1024);
    if (fullpath)
    {
        MEMSET(fullpath, 0, 1024);
        ret = filelst_idxtable_get_fname(cur_index_file, file_list_item->index, fullpath, 1023);
        if (ret == 0)
        {
            FREE(fullpath);
            fullpath = NULL;
        }
        //DEBUG_IN_FILE_LIST("FullPath:%s\n", fullpath);
    }
    return fullpath;
}

char *filelst_create_newpath(char *oldpath, char *newname)
{
    char *ptr = NULL;
    char *newpath = NULL;
    char oldchr = 0;

    if (oldpath == NULL || newname == NULL)
    {
        return NULL;
    }
    newpath = MALLOC(strlen(oldpath) + strlen(newname));
    if (newpath)
    {
        ptr = strchr_r(oldpath, '/');
        if (ptr == NULL)
        {
            FREE(newpath);
            return NULL;
        }

        ptr++;
        oldchr = *ptr;
        *ptr = 0;
        strcpy(newpath, oldpath);
        *ptr = oldchr;
        strcat(newpath, newname);
    }
    return newpath;
}

void filelst_rename(P_FILE_LIST pfile_list, P_FILE_LIST_ITEM item, char *newpath)
{
    if (cur_index_file)
    {
        filelst_idxtable_set_offset(cur_index_file, item->index);
        filelst_idxtable_set_fname(cur_index_file, newpath);
    }
}

void filelst_delete_file(P_FILE_LIST pfile_list, int index)
{
    int filenum = 0;
    UINT32 offset = 0;
    int start = 0;

    if (cur_index_file == NULL || index < 1) { return; }

    filenum = filelst_idxtable_get_num(cur_index_file);
    if (filenum <= 0)
    {
        return;
    }
    for (start = index; start <= filenum; start++)
    {
        fseek(cur_index_file, (start + 1) * 4, SEEK_SET);
        fread((char *)&offset, 1, 4, cur_index_file);
        fseek(cur_index_file, (start) * 4, SEEK_SET);
        fwrite((char *)&offset, 1, 4, cur_index_file);
    }
    filenum--;
    DEBUG_IN_FILE_LIST("DELET:%d\n", index);
    filelst_idxtable_set_num(cur_index_file, filenum);

    fflush(cur_index_file);
}

int file_list_music_favorite_add(char *filename, char *dirpath)
{
    FILE *fp = NULL;
    char path[INDEX_FILE_PATH_LEN] = {0}, db_filename[1024] = {0};
    int filenum = 0, index = 0;
    BOOL found_same = FALSE;

    filelst_idx_file_path(MUSIC_FAVORITE_LIST, dirpath, path, INDEX_FILE_PATH_LEN - 1);
    fp = fopen(path, "r+");
    if (fp == NULL)
    {
        fp = fopen(path, "w+");
        if (fp)
        {
            filelst_init_index_table(fp, MAX_MUSIC_FILE_NUM);
        }
    }
    if (fp == NULL)
        return 0;

    filenum = filelst_idxtable_get_num(fp);
    if (filenum < MAX_MUSIC_FILE_NUM)
    {
        for (index = 0; index < filenum; index ++ )
        {
            MEMSET(db_filename, 0, sizeof(db_filename));
            filelst_idxtable_get_fname(fp, index + 1, db_filename, sizeof(db_filename));
            if (strcmp(db_filename, filename) == 0)
            {
                found_same = TRUE;
                break;
            }
        }

        if (FALSE == found_same)
        {
            filelst_update_index_table(fp, filenum + 1);
            filelst_idxtable_set_fname(fp, filename);
        }
    }
    fclose(fp);
    fsync(path);
    DEBUG_IN_FILE_LIST("favorite list[%s:%d] add:%s\n", dirpath, filenum + 1, filename);
    return 1;
}


int file_list_music_favorite_del(char *filename, char *dirpath)
{
    FILE *fp = NULL;
    char path[INDEX_FILE_PATH_LEN] = {0}, db_filename[1024] = {0};
    int filenum = 0, index = 0, start = 0;
    UINT32 offset = 0;
    BOOL found_same = FALSE;

    filelst_idx_file_path(MUSIC_FAVORITE_LIST, dirpath, path, INDEX_FILE_PATH_LEN - 1);
    fp = fopen(path, "r+");
    if (fp == NULL)
    {
        fp = fopen(path, "w+");
        if (fp)
        {
            filelst_init_index_table(fp, MAX_MUSIC_FILE_NUM);
        }
    }
    if (fp == NULL)
        return 0;

    filenum = filelst_idxtable_get_num(fp);
    if (filenum < MAX_MUSIC_FILE_NUM)
    {
        for (index = 0; index < filenum; index ++ )
        {
            MEMSET(db_filename, 0, sizeof(db_filename));
            filelst_idxtable_get_fname(fp, index + 1, db_filename, sizeof(db_filename));
            if (strcmp(db_filename, filename) == 0)
            {
                found_same = TRUE;
                break;
            }
        }

        if (TRUE == found_same)
        {
            for (start = (index + 1); start <= filenum; start ++)
            {
                fseek(fp, (start + 1) * 4, SEEK_SET);
                fread((char *)&offset, 1, 4, fp);
                fseek(fp, start * 4, SEEK_SET);
                fwrite((char *)&offset, 1, 4, fp);
            }

            filenum --;
            filelst_idxtable_set_num(fp, filenum);
        }
    }
    fclose(fp);
    fsync(path);
    DEBUG_IN_FILE_LIST("favorite list[%s:%d] add:%s\n", dirpath, filenum + 1, filename);
    return 1;
}


int file_list_music_favorite_get_count(char *dirpath)
{
    FILE *fp = NULL;
    char path[INDEX_FILE_PATH_LEN] = {0};
    __MAYBE_UNUSED__ int filenum = 0;

    filelst_idx_file_path(MUSIC_FAVORITE_LIST, dirpath, path, INDEX_FILE_PATH_LEN - 1);
    fp = fopen(path, "r");
    if (fp == NULL)
    {
        return 0;
    }
    filenum = filelst_idxtable_get_num(fp);
    fclose(fp);

    return filenum;
}


BOOL file_list_check_file_exist(file_list_type listtype, char *filename, char *dirpath)
{
    FILE *fp = NULL;
    char path[INDEX_FILE_PATH_LEN] = {0}, db_filename[1024] = {0};
    int filenum = 0, index = 0;
    BOOL found_same = FALSE;

    filelst_idx_file_path(MUSIC_FAVORITE_LIST, dirpath, path, INDEX_FILE_PATH_LEN - 1);
    fp = fopen(path, "r+");
    if (fp == NULL)
    {
        return FALSE;
    }

    filenum = filelst_idxtable_get_num(fp);
    for (index = 0; index < filenum; index ++ )
    {
        MEMSET(db_filename, 0, sizeof(db_filename));
        filelst_idxtable_get_fname(fp, index + 1, db_filename, sizeof(db_filename));
        if (strcmp(db_filename, filename) == 0)
        {
            found_same = TRUE;
            break;
        }
    }

    fclose(fp);
    fsync(path);

    return found_same;
}


void file_list_scan_device(char *dev)
{
    //filelst_scan_device(dev, VIDEO_FILE_LIST);
    filelst_scan_device(dev, MUSIC_FILE_LIST);
    filelst_scan_device(dev, IMAGE_FILE_LIST);
}

int file_list_has_index_file(P_FILE_LIST pfile_list, char *dirpath)
{
    char path[INDEX_FILE_PATH_LEN] = {0};
    FILE *fp = NULL;
    int ret = 0;

    filelst_idx_file_path(pfile_list->type, dirpath, path, INDEX_FILE_PATH_LEN - 1);
    fp = fopen(path, "r");
    if (fp)
    {
        ret = 1;
        fclose(fp);
    }
    return ret;
}

int file_list_get_filenum(file_list_handle handle, char *dirpath)
{
    P_FILE_LIST pfile_list = NULL;
    UINT32 filenum = 0;
    FILE *fp = NULL;
    char file_path[INDEX_FILE_PATH_LEN] = {0};

    pfile_list = (P_FILE_LIST) handle;
    if (pfile_list == NULL || dirpath == NULL)
    {
        return 0;
    }
    filelst_idx_file_path(pfile_list->type, dirpath, file_path, INDEX_FILE_PATH_LEN - 1);
    fp = fopen(file_path, "r");
    if (fp)
    {
        filenum = filelst_idxtable_get_num(fp);
        fclose(fp);
    }
    return filenum;
}


