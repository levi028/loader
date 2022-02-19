/*-
 * Copyright (c) 1982, 1986, 1989, 1993
 *    The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *    @(#)errno.h    8.5 (Berkeley) 1/21/94
 * $FreeBSD: src/sys/sys/errno.h,v 1.28 2005/04/02 12:33:28 das Exp $
 */

#ifndef _SYS_ERRNO_H_
#define _SYS_ERRNO_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define    SUCCESS      0

#define    EPERM        1        /* Operation not permitted */
#define    ENOENT       2        /* No such file or directory */
#define    ESRCH        3        /* No such process */
#define    EINTR        4        /* Interrupted system call */
#define    EIO          5        /* Input/output error */
#define    ENXIO        6        /* Device not configured */
#define    E2BIG        7        /* Argument list too long */
#define    ENOEXEC      8        /* Exec format error */
#define    EBADF        9        /* Bad file descriptor */
#define    ECHILD       10        /* No child processes */
#define    EAGAIN       11  /* Try again */
#define    ENOMEM       12        /* Cannot allocate memory */
#define    EACCES       13        /* Permission denied */
#define    EFAULT       14        /* Bad address */
#define    ENOTBLK      15        /* Block device required */
#define    EBUSY        16        /* Device busy */
#define    EEXIST       17        /* File exists */
#define    EXDEV        18        /* Cross-device link */
#define    ENODEV       19        /* Operation not supported by device */
#define    ENOTDIR      20        /* Not a directory */
#define    EISDIR       21        /* Is a directory */
#define    EINVAL       22        /* Invalid argument */
#define    ENFILE       23        /* Too many open files in system */
#define    EMFILE       24        /* Too many open files */
#define    ENOTTY       25        /* Inappropriate ioctl for device */
#define    ETXTBSY      26        /* Text file busy */
#define    EFBIG        27        /* File too large */
#define    ENOSPC       28        /* No space left on device */
#define    ESPIPE       29        /* Illegal seek */
#define    EROFS        30        /* Read-only filesystem */
#define    EMLINK       31        /* Too many links */
#define    EPIPE        32        /* Broken pipe */

/* math software */
#define    EDOM         33        /* Numerical argument out of domain */
#define    ERANGE       34        /* Result too large */

/* non-blocking and interrupt i/o */
#define    EDEADLK      35  /* Resource deadlock would occur */
#define    ENAMETOOLONG 36  /* File name too long */
#define    ENOLCK       37  /* No record locks available */
#define    ENOSYS       38  /* Function not implemented */
#define    ENOTEMPTY    39  /* Directory not empty */
#define    ELOOP        40  /* Too many symbolic links encountered */
#define    EWOULDBLOCK  EAGAIN  /* Operation would block */

#define    ENOMSG       42  /* No message of desired type */
#define    EIDRM        43  /* Identifier removed */
#define    ECHRNG       44  /* Channel number out of range */
#define    EL2NSYNC     45  /* Level 2 not synchronized */
#define    EL3HLT       46  /* Level 3 halted */
#define    EL3RST       47  /* Level 3 reset */
#define    ELNRNG       48  /* Link number out of range */
#define    EUNATCH      49  /* Protocol driver not attached */
#define    ENOCSI       50  /* No CSI structure available */
#define    EL2HLT       51  /* Level 2 halted */
#define    EBADE        52  /* Invalid exchange */
#define    EBADR        53  /* Invalid request descriptor */
#define    EXFULL       54  /* Exchange full */
#define    ENOANO       55  /* No anode */
#define    EBADRQC      56  /* Invalid request code */
#define    EBADSLT      57  /* Invalid slot */

#define    EDEADLOCK    EDEADLK

#define    EBFONT       59  /* Bad font file format */
#define    ENOSTR       60  /* Device not a stream */
#define    ENODATA      61  /* No data available */
#define    ETIME        62  /* Timer expired */
#define    ENOSR        63  /* Out of streams resources */
#define    ENONET       64  /* Machine is not on the network */
#define    ENOPKG       65  /* Package not installed */
#define    EREMOTE      66  /* Object is remote */
#define    ENOLINK      67  /* Link has been severed */
#define    EADV         68  /* Advertise error */
#define    ESRMNT       69  /* Srmount error */
#define    ECOMM        70  /* Communication error on send */
#define    EPROTO       71  /* Protocol error */
#define    EMULTIHOP    72  /* Multihop attempted */
#define    EDOTDOT      73  /* RFS specific error */
#define    EBADMSG      74  /* Not a data message */
#define    EOVERFLOW    75  /* Value too large for defined data type */
#define    ENOTUNIQ     76  /* Name not unique on network */
#define    EBADFD       77  /* File descriptor in bad state */
#define    EREMCHG      78  /* Remote address changed */
#define    ELIBACC      79  /* Can not access a needed shared library */
#define    ELIBBAD      80  /* Accessing a corrupted shared library */
#define    ELIBSCN      81  /* .lib section in a.out corrupted */
#define    ELIBMAX      82  /* Attempting to link in too many shared libraries */
#define    ELIBEXEC     83  /* Cannot exec a shared library directly */
#define    EILSEQ       84  /* Illegal byte sequence */
#define    ERESTART     85  /* Interrupted system call should be restarted */
#define    ESTRPIPE     86  /* Streams pipe error */
#define    EUSERS       87  /* Too many users */


/* ipc/network software -- argument errors */
#define    ENOTSOCK           88        /* Socket operation on non-socket */
#define    EDESTADDRREQ       89        /* Destination address required */
#define    EMSGSIZE           90        /* Message too long */
#define    EPROTOTYPE         91        /* Protocol wrong type for socket */
#define    ENOPROTOOPT        92        /* Protocol not available */
#define    EPROTONOSUPPORT    93        /* Protocol not supported */
#define    ESOCKTNOSUPPORT    94        /* Socket type not supported */
#define    EOPNOTSUPP         95        /* Operation not supported */
#define    ENOTSUP            EOPNOTSUPP    /* Operation not supported */
#define    EPFNOSUPPORT   96        /* Protocol family not supported */
#define    EAFNOSUPPORT   97        /* Address family not supported by protocol family */
#define    EADDRINUSE     98        /* Address already in use */
#define    EADDRNOTAVAIL  99        /* Can't assign requested address */

/* ipc/network software -- operational errors */
#define    ENETDOWN       100        /* Network is down */
#define    ENETUNREACH    101        /* Network is unreachable */
#define    ENETRESET      102        /* Network dropped connection on reset */
#define    ECONNABORTED   103        /* Software caused connection abort */
#define    ECONNRESET     104        /* Connection reset by peer */
#define    ENOBUFS        105        /* No buffer space available */
#define    EISCONN        106        /* Socket is already connected */
#define    ENOTCONN       107        /* Socket is not connected */
#define    ESHUTDOWN      108        /* Can't send after socket shutdown */
#define    ETOOMANYREFS   109        /* Too many references: can't splice */
#define    ETIMEDOUT      110        /* Operation timed out */
#define    ECONNREFUSED   111        /* Connection refused */
#define    EHOSTDOWN      112  /* Host is down */
#define    EHOSTUNREACH   113  /* No route to host */
#define    EALREADY       114  /* Operation already in progress */
#define    EINPROGRESS    115  /* Operation now in progress */
#define    ESTALE         116  /* Stale NFS file handle */
#define    EUCLEAN        117  /* Structure needs cleaning */
#define    ENOTNAM        118  /* Not a XENIX named type file */
#define    ENAVAIL        119  /* No XENIX semaphores available */
#define    EISNAM         120  /* Is a named type file */
#define    EREMOTEIO      121  /* Remote I/O error */
#define    EDQUOT         122  /* Quota exceeded */

#define    ENOMEDIUM      123  /* No medium found */
#define    EMEDIUMTYPE    124  /* Wrong medium type */
#define    ECANCELED      125    /* Operation Canceled */
#define    ENOKEY         126    /* Required key not available */
#define    EKEYEXPIRED    127    /* Key has expired */
#define    EKEYREVOKED    128    /* Key has been revoked */
#define    EKEYREJECTED   129    /* Key was rejected by service */

#define    ELAST          130        /* Must be equal largest errno */
//////////////////////////////////////////////////////////////////////////

/* quotas & mush */
#define    EPROCLIM        200        /* Too many processes */
#define    EBADRPC         201        /* RPC struct is bad */
#define    ERPCMISMATCH    202        /* RPC version wrong */
#define    EPROGUNAVAIL    203        /* RPC prog. not avail */
#define    EPROGMISMATCH   204        /* Program version wrong */
#define    EPROCUNAVAIL    205        /* Bad procedure for program */
#define    EFTYPE          206        /* Inappropriate file type or format */
#define    EAUTH           207        /* Authentication error */
#define    ENEEDAUTH       208        /* Need authenticator */
#define    ENOATTR         210        /* Attribute not found */
#define    EDOOFUS         211        /* Programming error */

#define ENSROK    0       /* DNS server returned answer with no data */
#define ENSRNODATA  300   /* DNS server returned answer with no data */
#define ENSRFORMERR 301   /* DNS server claims query was misformatted */
#define ENSRSERVFAIL 302  /* DNS server returned general failure */
#define ENSRNOTFOUND 303  /* Domain name not found */
#define ENSRNOTIMP  304   /* DNS server does not implement requested operation */
#define ENSRREFUSED 305   /* DNS server refused query */
#define ENSRBADQUERY 306  /* Misformatted DNS query */
#define ENSRBADNAME 307   /* Misformatted domain name */
#define ENSRBADFAMILY 308 /* Unsupported address family */
#define ENSRBADRESP 309   /* Misformatted DNS reply */
#define ENSRCONNREFUSED 310 /* Could not contact DNS servers */
#define ENSRTIMEOUT 311   /* Timeout while contacting DNS servers */

#define ENSROF    312 /* End of file */
#define ENSRFILE  313 /* Error reading file */
#define ENSRNOMEM 314 /* Out of memory */
#define ENSRDESTRUCTION 315 /* Application terminated lookup */
#define ENSRQUERYDOMAINTOOLONG  316 /* Domain name is too long */
#define ENSRCNAMELOOP 317 /* Domain name is too long */
#define EGUIDPARTITION 318 /* GUID partition table & support over 2T HDD*/


#ifndef ERRNO
#define ERRNO
#endif //ERRNO


extern int errno;

#ifdef __cplusplus
}
#endif


#endif
