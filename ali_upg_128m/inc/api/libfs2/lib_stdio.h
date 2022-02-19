/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: lib_stdio.h
*
*    Description: declaration of lib_stdio
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef    _stdio_h_904fa35d_7831_47dc_a883_b4572786b47e
#define    _stdio_h_904fa35d_7831_47dc_a883_b4572786b47e

#include <stdarg.h>
#include <types.h>
#include <retcode.h>
#include <sys_config.h>
#include <osal/osal.h>
#include <api/libc/list.h>
#include <api/libfs2/types.h>

//__BEGIN_DECLS
#ifdef __cplusplus
extern "C"
{
#endif

#define fpos_t long long
#define FINDDATA_FILE_NAME_SIZE      64
#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

#define    __SLBF    0x0001        /* line buffered */
#define    __SNBF    0x0002        /* unbuffered */
#define    __SRD    0x0004        /* OK to read */
#define    __SWR    0x0008        /* OK to write */
    /* RD and WR are never simultaneously asserted */
#define    __SRW    0x0010        /* open for reading & writing */
#define    __SEOF    0x0020        /* found EOF */
#define    __SERR    0x0040        /* found error */
#define    __SMBF    0x0080        /* _buf is from malloc */
#define    __SAPP    0x0100        /* fdopen()ed in append mode */
#define    __SSTR    0x0200        /* this is an sprintf/snprintf string */
#define    __SOPT    0x0400        /* do fseek() optimization */
#define    __SNPT    0x0800        /* do not do fseek() optimization */
#define    __SOFF    0x1000        /* set iff _offset is in fact correct */
#define    __SMOD    0x2000        /* true => fgetln modified _p text */
#define    __SALC    0x4000        /* allocate string space dynamically */
#define    __SIGN    0x8000        /* ignore this file in _fwalk */

/*
 * The following three definitions are for ANSI C, which took them
 * from System V, which brilliantly took internal interface macros and
 * made them official arguments to setvbuf(), without renaming them.
 * Hence, these ugly _IOxxx names are *supposed* to appear in user code.
 *
 * Although numbered as their counterparts above, the implementation
 * does not rely on this.
 */
#define    _IOFBF    0        /* setvbuf should set fully buffered */
#define    _IOLBF    1        /* setvbuf should set line buffered */
#define    _IONBF    2        /* setvbuf should set unbuffered */

#define    BUFSIZ    1024        /* size of buffer used by setbuf */
#define    EOF    (-1)

/*
 * FOPEN_MAX is a minimum maximum, and is the number of streams that
 * stdio can provide without attempting to allocate further resources
 * (which could fail).  Do not use this for anything.
 */
                /* must be == _POSIX_STREAM_MAX <limits.h> */
#define    FOPEN_MAX    20    /* must be <= OPEN_MAX <sys/syslimits.h> */
#define    FILENAME_MAX    1024    /* must be <= PATH_MAX <sys/syslimits.h> */

/* System V/ANSI C; this is the wrong way to do this, do *not* use these. */
#define    l_tmpnam    1024    /* XXX must be == PATH_MAX */
#define    TMP_MAX        308915776

#ifndef SEEK_SET
#define    SEEK_SET    0    /* set file offset to offset */
#endif
#ifndef SEEK_CUR
#define    SEEK_CUR    1    /* set file offset to current plus offset */
#endif
#ifndef SEEK_END
#define    SEEK_END    2    /* set file offset to EOF plus offset */
#endif

/*
 * The __sfoo macros are here so that we can
 * define function versions in the C library.
 */
#define    __sgetc(p) (--(p)->_r < 0 ? __srget(p) : (int)(*(p)->_p++))

#define    __sfeof(p)    (((p)->_flags & __SEOF) != 0)
#define    __sferror(p)    (((p)->_flags & __SERR) != 0)
#define    __sclearerr(p)    ((void)((p)->_flags &= ~(__SERR|__SEOF)))
#define    __sfileno(p)    ((p)->_file)

/*
#define    feof(p)        ((!__isthreaded) ? __sfeof(p) : (feof)(p))
#define    ferror(p)    ((!__isthreaded) ? __sferror(p) : (ferror)(p))
#define    clearerr(p)    ((!__isthreaded) ? __sclearerr(p) : (clearerr)(p))

#define    fileno(p)    ((!__isthreaded) ? __sfileno(p) : (fileno)(p))
*/

/*
 * This global flag is non-zero when a process has created one
 * or more threads. It is used to avoid calling locking functions
 * when they are not required.
 */
//extern int    __isthreaded;

//#define    _FSTDIO            /* Define for new stdio with functions. */

/*
 * NB: to fit things in six character monocase externals, the stdio
 * code uses the prefix `__s' for stdio objects, typically followed
 * by a three-character attempt at a mnemonic.
 */

/* stdio buffers */
struct __ssbuf
{
    unsigned char *_base;
    int    _size;
};

/* hold a buncha junk that would grow the ABI */
struct __s_filex;

/*
 * stdio state variables.
 *
 * The following always hold:
 *
 *    if (_flags&(__SLBF|__SWR)) == (__SLBF|__SWR),
 *        _lbfsize is -_bf._size, else _lbfsize is 0
 *    if _flags&__SRD, _w is 0
 *    if _flags&__SWR, _r is 0
 *
 * This ensures that the getc and putc macros (or inline functions) never
 * try to write or read from a file that is in `read' or `write' mode.
 * (Moreover, they can, and do, automatically switch from read mode to
 * write mode, and back, on "r+" and "w+" files.)
 *
 * _lbfsize is used only to make the inline line-buffered output stream
 * code as compact as possible.
 *
 * _ub, _up, and _ur are used when ungetc() pushes back more characters
 * than fit in the current _bf, or when ungetc() pushes back a character
 * that does not match the previous one in _bf.  When this happens,
 * _ub._base becomes non-nil (i.e., a stream has ungetc() data iff
 * _ub._base!=NULL) and _up and _ur save the current values of _p and _r.
 */
typedef    struct __ss_file
{
    unsigned char *_p;    /* current position in (some) buffer */
    int    _r;        /* read space left for getc() */
    int    _w;        /* write space left for putc() */
    short    _flags;        /* flags, below; this FILE is free if 0 */
    short    _file;        /* fileno, if Unix descriptor, else -1 */
    struct    __ssbuf _bf;    /* the buffer (at least 1 byte, if !NULL) */
    int    _lbfsize;    /* 0 or -_bf._size, for inline putc */

    /* operations */
    void    *_cookie;    /* cookie passed to io functions */
    int    (*_close)(void *);
    int    (*_read)(void *, char *, int);
    fpos_t    (*_seek)(void *, fpos_t, int);
    int    (*_write)(void *, const char *, int);

    /* separate buffer for long sequences of ungetc() */
    struct    __ssbuf _ub;    /* ungetc buffer */
    struct __s_filex *_extra; /* additions to FILE to not break ABI */
    int    _ur;        /* saved _r when _r is counting ungetc data */

    /* tricks to meet minimum requirements even when malloc() fails */
    unsigned char _ubuf[3];    /* guarantee an ungetc() buffer */
    unsigned char _nbuf[1];    /* guarantee a getc() buffer */

    /* separate buffer for fgetln() when line crosses buffer boundary */
    struct    __ssbuf _lb;    /* buffer for fgetln() */

    /* Unix stdio files get aligned to block boundaries on fseek() */
    int    _blksize;    /* stat.st_blksize (may be != _bf._size) */
    fpos_t    _offset;    /* current lseek offset */
} FILE;

struct __s_date
{
    WORD    yyyy;
    BYTE    mm;
    BYTE    dd;
};

struct __s_time
{
    BYTE    hh;
    BYTE    mm;
    BYTE    ss;
};

typedef struct
{
    UINT8 is_dir;
    int    size;    /* if size < 0, than it is a directory */
    char    name[550];
    struct __s_date date;
    struct __s_time time;
}f_dirent;

typedef struct
{
    UINT32 id;
    char path[1024];
}DIR;

typedef struct
{
    unsigned short  attrib;         /* File attribute. */
    unsigned long  time_create;    /* Time of file creation */
    unsigned long  time_access;    /* Time of last file access (?L for FAT file systems) */
    unsigned long  time_write;     /* Time of last write to file */
    unsigned long  size;           /* Length of file in bytes */
    char    name[FINDDATA_FILE_NAME_SIZE];       /* Null-terminated name of matched file/directory, without the path */
}FINDDATA;

/*
 * Functions defined in ANSI C standard.
 */

FILE    *fopen(const char * __restrict, const char * __restrict);
int        fclose(FILE *);
size_t    fread(void * __restrict, size_t, size_t, FILE * __restrict);
size_t    fwrite(const void * __restrict, size_t, size_t, FILE * __restrict);
int        fseek(FILE *, off_t, int);
size_t    fread_std(void * __restrict, size_t, size_t, FILE * __restrict);
size_t    fwrite_std(const void * __restrict, size_t, size_t, FILE * __restrict);
int        fseek_std(FILE *, long, int);
int        fflush(FILE *);
void    rewind(FILE *);
int        remove(const char *);
int     frename(const char *old_file, const char *new_file);

int        fputs(const char * __restrict, FILE * __restrict);
char    *fgets(char * __restrict, int, FILE * __restrict);
int        fputc(int, FILE *);
int        fgetc(FILE *);
void    clearerr(FILE *);
int        feof(FILE *);
int        ferror(FILE *);
FILE    *fdopen(int, const char *);
int        fileno(FILE *);

int        fprintf(FILE * __restrict, const char * __restrict, ...);
int        vfprintf(FILE *stream, const char *format, va_list arg_ptr);

off_t    ftell(FILE *);
long    ftell_std(FILE *);
off_t    ftello(FILE *fp);

int        getc(FILE *);
int        putc(int, FILE *);

int        setvbuf(FILE * __restrict, char * __restrict, int, size_t);
int        ungetc(int, FILE *);


int        frmextname(char *path);
void     fgetdevname(const char *path, char *dev);

INT32     fsync(const char *path);

int     fmkdir(const char *path);

DIR     *fopendir(const char *path);
int     freaddir(DIR *dir, f_dirent *dest);
int     frewinddir(DIR *dir);
int     fclosedir(DIR *dir);

int     femptydir(const char *path);
int     femptydirre(const char *path);
int     frmdirempty(const char *path);
int     frmdir(const char *path);

int     ffindfirst(char *filespec, FINDDATA *find_data, int *handle);
int        ffindnext( int handle, FINDDATA *find_data);
int     ffindclose(int handle);

int     fscanf(FILE * __restrict, char const * __restrict, ...);
int     fseeko(FILE *fp, off_t offset, int whence);
int     fsetpos(FILE *, const fpos_t *);
void    setbuffer(FILE *, char *, int);
int     setlinebuf(FILE *);
int     vfscanf(FILE *stream, const char *format, va_list arg_ptr);


/*
 * Functions internal to the implementation.
 */
int    __srget(FILE *);
int    __swbuf(int, FILE *);



//__END_DECLS
#ifdef __cplusplus
}
#endif

#endif /* !_STDIO_H_904FA35D_7831_47dc_A883_B4572786B47E */



