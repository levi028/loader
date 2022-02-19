/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
*    disclosed to unauthorized individual.
*
*    File: smartcard_t1.h
*
*    Description: Head file of smartcard t=1 protocol.
*
*    History:
*      Date          Author          Version         Comment
*      ====          ======          =======         =======
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*    PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __SMARTCARD_T1_H__
#define __SMARTCARD_T1_H__


#include <types.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct t1_buf
{
    UINT8*        base;
    UINT32        head, tail, size;
    UINT32        overrun;
} t1_buf_t;

/********************************************
T1 related Macro define
*********************************************/
/* I block */
#define T1_I_SEQ_SHIFT        6

/* R block */
#define T1_IS_ERROR(pcb)    ((pcb) & 0x0F)
#define T1_EDC_ERROR        0x01
#define T1_OTHER_ERROR        0x02
#define T1_R_SEQ_SHIFT        4

/* S block stuff */
#define T1_S_IS_RESPONSE(pcb)    ((pcb) & T1_S_RESPONSE)
#define T1_S_TYPE(pcb)        ((pcb) & 0x0F)
#define T1_S_RESPONSE        0x20
#define T1_S_RESYNC        0x00
#define T1_S_IFS        0x01
#define T1_S_ABORT        0x02
#define T1_S_WTX        0x03

#define swap_nibbles(x) ( (x >> 4) | ((x & 0xF) << 4) )

#ifndef TRUE
#define TRUE    1
#define FALSE   0
#endif


#define NAD     0
#define PCB     1
#define LEN     2
#define DATA    3


#define CPU_CLK_308     308
#define CPU_CLK_300     300
#define CPU_CLK_270     270
#define CPU_CLK_180     180
#define CPU_CLK_90      90

#define TX_RX_THLD        4000//MS

#define MAX_LENGTH        256    //max response length
#define CMD_LENGTH        5
#define CLA_OFFSET        0
#define INS_OFFSET        1
#define P1_OFFSET        2
#define P2_OFFSET        3
#define P3_OFFSET        4

/* internal state, do not mess with it. */
/* should be != DEAD after reset/init */
enum
{
    SENDING,
    RECEIVING,
    RESYNCH,
    DEAD,
/* behavior */
    GOTO,
    BREAK,
    CONTINUE,
    RESYNC,
    ERROR,
    DONE
};

/********************************************
T1 related Macro define ending
*********************************************/
 /* T=1 protocol constants */
#define T1_I_BLOCK            0x00
#define T1_R_BLOCK            0x80
#define T1_S_BLOCK            0xC0
#define T1_MORE_BLOCKS        0x20
#define T1_BUFFER_SIZE        (3 + 254 + 2)

enum
{
    IFD_PROTOCOL_RECV_TIMEOUT = 0x0000,
    IFD_PROTOCOL_T1_BLOCKSIZE,
    IFD_PROTOCOL_T1_CHECKSUM_CRC,
    IFD_PROTOCOL_T1_CHECKSUM_LRC,
    IFD_PROTOCOL_T1_IFSC,
    IFD_PROTOCOL_T1_IFSD,
    IFD_PROTOCOL_T1_STATE,
    IFD_PROTOCOL_T1_MORE
};

/* T1 protocol private*/
typedef struct
{
    INT32        state;      /*internal state*/

    UINT8        ns;            /* reader side  Send sequence number */
    UINT8        nr;            /* card side  RCV sequence number*/
    UINT32        ifsc;
    UINT32        ifsd;

    UINT8        wtx;        /* block waiting time extention*/
    UINT32        retries;
    INT32        rc_bytes;     /*checksum bytes, 1 byte for LRC, 2 for CRC*/

    UINT32         BGT;
    UINT32         BWT;
    UINT32         CWT;

    UINT32        (*checksum)(UINT8 *data, UINT32 len, UINT8 *rc);

    INT8        more;        /* more data bit */
    UINT8        previous_block[4];    /* to store the last R-block */
    UINT8        sdata[T1_BUFFER_SIZE];
    UINT8        rdata[T1_BUFFER_SIZE];
} t1_state_t;


/* Smart card T=1 operation */
extern void t1_buf_init(t1_buf_t *, void *, UINT32);
extern void t1_buf_set(t1_buf_t *, void *, UINT32);
extern INT32 t1_buf_get(t1_buf_t *, void *, UINT32);
extern INT32 t1_buf_put(t1_buf_t *, const void *, UINT32);
extern INT32 t1_buf_putc(t1_buf_t *, INT32  );
extern UINT32 t1_buf_avail(t1_buf_t * );
extern void *t1_buf_head(t1_buf_t * );
extern INT32 t1_transceive(struct smc_device*, UINT8 , const void *, UINT32 , void *, UINT32 );
extern INT32 t1_xcv(struct smc_device *, UINT8*, UINT32 , UINT8 *, UINT32 , UINT32 *);
extern INT32 t1_negociate_ifsd(struct smc_device*, UINT32 , INT32 );
extern INT32 t1_init(t1_state_t *);
extern void t1_set_checksum(t1_state_t *, UINT8 );
extern INT32 t1_set_param(t1_state_t *, INT32 , INT32 );


#ifdef __cplusplus
}
#endif
#endif

