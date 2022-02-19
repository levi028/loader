/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
*    disclosed to unauthorized individual.
*
*    File: smartcard_t1.c
*
*    Description: This file contains all globe micros and functions declare
*                     of smartcard t=1 protocol.
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
#include <types.h>
#include <api/libc/string.h>
#include "smartcard.h"
#include "smartcard_t1.h"
#include "smartcard_txrx.h"
#include "smartcard_dev.h"

static UINT32 g_sent_length = 0;
static UINT32 g_last_send = 0;

/* Correct Table? */
static UINT16 crctab[256] =
{
    0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
    0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
    0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
    0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
    0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
    0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
    0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
    0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
    0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
    0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
    0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
    0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
    0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
    0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
    0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
    0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
    0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
    0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
    0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
    0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
    0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
    0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
    0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
    0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
    0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
    0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
    0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
    0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
    0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
    0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
    0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
    0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};
/***********************************************************************
 *
 *Below are functions about ISO/IEC 7816-3 Protocol T=1
 *The main interface for T1 is the function t1_transceive(), it is used to translate/receive
 *T1 data
 *
 *******************************************************************/
void t1_buf_init(t1_buf_t *bp, void *mem, UINT32 len)
{
    MEMSET(bp, 0, sizeof(*bp));
    bp->base = (UINT8*) mem;
    bp->size = len;
}

void t1_buf_set(t1_buf_t *bp, void *mem, UINT32 len)
{
    t1_buf_init(bp, mem, len);
    bp->tail = len;
}

INT32 t1_buf_get(t1_buf_t *bp, void *mem, UINT32 len)
{
    if (len > bp->tail - bp->head)
    {
        return -1;
    }
    if (mem)
    {
        MEMCPY(mem, bp->base + bp->head, len);
    }
    bp->head += len;
    return len;
}

INT32 t1_buf_put(t1_buf_t *bp, const void *mem, UINT32 len)
{
    if (len > bp->size - bp->tail)
    {
        bp->overrun = 1;
        return -1;
    }
    if (mem)
    {
        MEMCPY(bp->base + bp->tail, mem, len);
    }
    bp->tail += len;
    return len;
}

INT32 t1_buf_putc(t1_buf_t *bp, INT32 byte)
{
    UINT8 c = byte;

    return t1_buf_put(bp, &c, 1);
}

UINT32 t1_buf_avail(t1_buf_t *bp)
{
    return bp->tail - bp->head;
}

void *t1_buf_head(t1_buf_t *bp)
{
    return bp->base + bp->head;
}

/* Returns LRC of data.*/
static UINT32 t1_lrc(UINT8 *data, UINT32 len, UINT8 *rc)
{
    UINT8    lrc = 0;

    while (len--)
    {
        lrc ^= *data++;
    }

    if (rc)
    {
        *rc = lrc;
    }
    return 1;

}

/* Compute CRC of data.*/
static UINT32 t1_crc(UINT8 *data, UINT32 len, UINT8 *rc)
{
    UINT16 v = 0xFFFF;

    while (len--)
    {
        v = ((v >> 8) & 0xFF) ^ crctab[(v ^ *data++) & 0xFF];
    }

    if (rc)
    {
        rc[0] = (v >> 8) & 0xFF;
        rc[1] = v & 0xFF;
    }
    return 2;
}
/*choose the error check algorithm*/
void t1_set_checksum(t1_state_t *t1, UINT8 csum)
{
    switch (csum)
    {
    case IFD_PROTOCOL_T1_CHECKSUM_LRC:
        t1->rc_bytes = 1;
        t1->checksum = t1_lrc;
        break;
    case IFD_PROTOCOL_T1_CHECKSUM_CRC:
        t1->rc_bytes = 2;
        t1->checksum = t1_crc;
        break;
    default:
        break;
    }
}

 /* Set default T1 protocol parameters*/
static void t1_set_defaults(t1_state_t *t1)
{
    t1->retries  = 3;
    t1->ifsc     = ATR_DEFAULT_IFSC;
    t1->ifsd     = ATR_DEFAULT_IFSD;
    t1->nr      = 0;
    t1->ns      = 0;
    t1->wtx      = 0;
}

/* set parmaters for T1 protocol*/
INT32 t1_set_param(t1_state_t *t1, INT32 type, INT32 value)
{
    switch (type)
    {
        case IFD_PROTOCOL_T1_CHECKSUM_LRC:
        case IFD_PROTOCOL_T1_CHECKSUM_CRC:
            t1_set_checksum(t1, type);
            break;
        case IFD_PROTOCOL_T1_IFSC:
            t1->ifsc = value;
            break;
        case IFD_PROTOCOL_T1_IFSD:
            t1->ifsd = value;
            break;
        case IFD_PROTOCOL_T1_STATE:
            t1->state = value;
            break;
        case IFD_PROTOCOL_T1_MORE:
            t1->more = value;
            break;
        default:
            T1PRINTF("SMC T1: Unsupported parameter %d\n", type);
            return -1;
    }

    return RET_SUCCESS;
}

/* check the block type by PCB*/
static UINT32 t1_block_type(UINT8 pcb)
{
    switch (pcb & 0xC0)
    {
        case T1_R_BLOCK:
            return T1_R_BLOCK;
        case T1_S_BLOCK:
            return T1_S_BLOCK;
        default:
            return T1_I_BLOCK;
    }
}

/* set number sequnce for I/R block*/
static UINT32 t1_seq(UINT8 pcb)
{
    switch (pcb & 0xC0)
    {
        case T1_R_BLOCK:
            return (pcb >> T1_R_SEQ_SHIFT) & 1;
        case T1_S_BLOCK:
            return 0;
        default:
            return (pcb >> T1_I_SEQ_SHIFT) & 1;
    }
}

/* Build checksum*/
static UINT32 t1_compute_checksum(t1_state_t *t1,UINT8 *data, UINT32 len)
{
    return len + t1->checksum(data, len, data + len);
}

/* verify checksum*/
//return 1: verify ok
//         0: verify fail
static INT32 t1_verify_checksum(t1_state_t *t1, UINT8*rbuf,UINT32 len)
{
    unsigned char    csum[2];
    int        m = 0;
    int     n = 0;

    m = (INT32)len - t1->rc_bytes;
    n = t1->rc_bytes;

	T1PRINTF("SMC T1:line=%d, m=%d, n=%d, len=%d\n", __LINE__, m, n, len);
    if (m < 0)
    {
        return 0;
    }

    t1->checksum(rbuf, m, csum);
    if (!MEMCMP(rbuf + m, csum, n))
    {
        return 1;
    }

    return 0;
}
/*init T1 */
INT32 t1_init(t1_state_t *t1)
{
    t1_set_defaults(t1);
    t1_set_param(t1, IFD_PROTOCOL_T1_CHECKSUM_LRC, 0);
    t1_set_param(t1, IFD_PROTOCOL_T1_STATE, SENDING);
    t1_set_param(t1, IFD_PROTOCOL_T1_MORE, FALSE);

    return 0;
}

/*Detach t1 protocol*/

/*update the T1 block wait time when receiving S-wtx request*/
static void t1_update_bwt(t1_state_t *t1, UINT32 wtx)
{
    t1->BWT = wtx * t1->BWT;
    T1PRINTF("SMC T1: New timeout at WTX request: %d sec\n", t1->BWT);
}
static void t1_restore_bwt(struct smartcard_private *tp)
{
    if (tp != NULL)
    {
        /*BWT = (2^bwi*960 + 11)etu*/
        tp->T1.BWT= tp->first_cwt;
    }

}

/*construct the block*/
static UINT32 t1_build(t1_state_t *t1, UINT8 *block, UINT8 dad, UINT8 pcb, t1_buf_t *bp, UINT32 *lenp)
{
    UINT32 len = 0;
    INT8 more = FALSE;

    len = bp? t1_buf_avail(bp) : 0;
    if (len > t1->ifsc)
    {
        pcb |= T1_MORE_BLOCKS;
        len = t1->ifsc;
        more = TRUE;
    }

    /* Add the sequence number */
    switch (t1_block_type(pcb))
    {
        case T1_R_BLOCK:
            pcb |= t1->nr << T1_R_SEQ_SHIFT;
            break;
        case T1_I_BLOCK:
            pcb |= t1->ns << T1_I_SEQ_SHIFT;
            t1->more = more;
            T1PRINTF("SMC T1: more bit: %d\n", more);
            break;
        default:
            break;
    }

    block[0] = dad;
    block[1] = pcb;
    block[2] = len;

    if (len)
    {
        MEMCPY(block + 3, t1_buf_head(bp), len);
    }
    if (lenp)
    {
        *lenp = len;
    }

    len = t1_compute_checksum(t1, block, len + 3);

    /* memorize the last sent block */
    /* only 4 bytes since we are only interesed in R-blocks */
    MEMCPY(t1->previous_block, block, 4);

    return len;
}
/*reconstruct the last sent block*/
static UINT32 t1_rebuild(t1_state_t *t1, UINT8 *block)
{
    UINT8 pcb = t1->previous_block[1];

    /* copy the last sent block */
    if (T1_R_BLOCK == t1_block_type(pcb))
    {
        MEMCPY(block, t1->previous_block, 4);
    }
    else
    {
        T1PRINTF("SMC T1: previous block was not R-Block: %02X\n", pcb);
        return 0;
    }
    return 4;
}
/* Send/receive block*/
INT32 t1_xcv(struct smc_device *dev, UINT8*sblock, UINT32 slen, UINT8 *rblock, UINT32 rmax, UINT32 *ractual)
{
    INT32 n = 0;
    UINT32 rmax_int = 0;
    UINT32 m = 0;
    UINT32 actual=0;
    UINT8 dad = 0;
    UINT8 dad1 = 0;

    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    if(tp->T!=1)
    {
        return -1;
    }
    t1_state_t *t1 = &tp->T1;

    //T1PRINTF("sending block : %d bytes need to be send\n ", slen);

    if ((NULL == sblock)||(NULL == rblock))
    {
        return -1;
    }
    if (t1->wtx > 1)
    {
        /*set the new temporary timeout at WTX card request */
        t1_update_bwt(t1,t1->wtx);
    }

/************************************************************
  *Note: For some cards, the block head and body should be readed separately.
  *          If that happens, the below should be modified to write/read twice.
  ***********************************************************/
      dad = *sblock;
#if 1
    //n = smc_uart_T1_write(dev, slen, sblock);
    n = smc_dev_write(dev, sblock, slen, (INT16 *)(&actual));
    t1->wtx = 0;    /* reset to default value ??????????*/
    if (n != RET_SUCCESS)
    {
        T1PRINTF("SMC T1: SMC write error in t1_xcv!\n");
        return SMART_WRITE_ERROR;//return n;
    }
#else
    n = smc_uart_fifowrite(dev, sblock, slen, &actual);
    if ((n != RET_SUCCESS))//||(actual != slen))
    {
        T1PRINTF("SMC write error in t1_xcv!\n");
        return SMART_WRITE_ERROR;//return n;
    }
#endif
    /* Get the response en bloc */
    MEMSET(rblock, 0, rmax);
    rmax_int = rmax;
    //n = smc_uart_T1_read(dev, rmax_int, rblock, &actual);
    n = smc_dev_read(dev, rblock, rmax_int, (INT16 *)(&actual));
    rmax = rmax_int;

    if ((SMART_PARITY_ERROR == n)|| (SMART_NO_ANSWER == n))  //current not implemented for parity check
    {
        T1PRINTF("SMC T1: SMC read-no answer!\n");
        return n;
    }
    if ((SMART_NOT_INSERTED == n) ||(SMART_NOT_RESETED == n))
    {
        T1PRINTF("SMC T1: SMC read fetal error in t1_xcv!\n");
        return -1; //fetal error
    }

    //T1PRINTF("t1_xcv read ok, %d bytes got\n ", actual);
    #if 0
    for (i=0;i<actual; i++)
        libc_printf(" 0x%02x ", rblock[i]);
    libc_printf("\n");
    #endif
    dad1 = *rblock;
    invert(&dad1, 1);
    if ((actual > 0)&&(dad1 == dad))
    {
        m = rblock[2] + 3 + t1->rc_bytes;
        if (m < actual)
        {
            actual = m;
        }
    }

//    if (actual > 0)
//    {
//        T1PRINTF("t1_xcv read OK, %d bytes got!\n", actual);
//    }
    *ractual = actual;

    /* Restore initial timeout */
    t1_restore_bwt(tp);
    return RET_SUCCESS;
}

INT32 t1_negociate_ifsd(struct smc_device*dev, UINT32 dad, INT32 ifsd)
{
    t1_buf_t sbuf={NULL,0,0,0,0};
    UINT8 *sdata = NULL;
    UINT32 slen = 0;
    UINT32 retries = 0;
    UINT32 snd_len = 0;
    INT32 n = 0;
    UINT8 snd_buf[1];
    UINT32 actual =  0;
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;

    t1_state_t *t1 = &tp->T1;

    sdata = &(t1->sdata[0]);
    retries = t1->retries;

    /* S-block IFSD request */
    snd_buf[0] = ifsd;
    snd_len = 1;

    /* Initialize send/recv buffer */
    t1_buf_set(&sbuf, (void *) snd_buf, snd_len);

    while (TRUE)
    {
        /* Build the block */
        slen = t1_build(t1, sdata, dad, T1_S_BLOCK | T1_S_IFS, &sbuf, NULL);

        /* Send the block */
        n = t1_xcv(dev, sdata, slen, sdata, T1_BUFFER_SIZE, &actual);

        retries--;
        /* ISO 7816-3 Rule 7.4.2 */
        if (0 == retries)
        {
            goto error;
        }

        if (-1 == n)
        {
            T1PRINTF("SMC T1: fatal: transmit/receive failed\n");
            goto error;
        }

        if ((SMART_NO_ANSWER== n)    || (SMART_WRITE_ERROR == n)                            /* Parity error */
            || (sdata[DATA] != ifsd)                /* Wrong ifsd received */
            || (sdata[NAD] != swap_nibbles(dad))    /* wrong NAD */
            || (!t1_verify_checksum(t1, sdata, actual))    /* checksum failed */
            || (actual != (UINT32)4 + t1->rc_bytes)                /* wrong frame length */
            || (sdata[LEN] != 1)                    /* wrong data length */
            || (sdata[PCB] != (T1_S_BLOCK | T1_S_RESPONSE | T1_S_IFS))) /* wrong PCB */
        {
            continue;
        }
        /* no more error */
        goto done;
    }

done:
    #if 0
    for (i=0;i<actual;i++)
        T1PRINTF(" 0x%02x ", sdata[i]);
    #endif
    return RET_SUCCESS;

error:
    t1->state = DEAD;
    return -1;
}

static int t1_checksum_fail_process(struct smc_device *dev, UINT32 actual, UINT8 *rdata, UINT8 *sdata,  \
                                    UINT32 *retries, UINT8 dad, UINT32 *p_slen, t1_buf_t *sbuf)
{
    UINT32 i = 0;
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    t1_state_t *t1 = &tp->T1;

    T1PRINTF("SMC T1: smc t1: checksum failed\n");
    for (i = 0; i < actual; i++)
    {
        T1PRINTF(" 0x%02x ", rdata[i]);
    }

	/* special for VMX card, since VMX will not support R-Block and S-Block, resend I-Block */ 
    #if defined(_VMX_CA_STD_ENABLE_) || defined(_VMX_CA_ENABLE_)
    *p_slen = t1_build(t1, sdata, dad, T1_I_BLOCK, sbuf, &g_last_send); 
    return CONTINUE; 
    #endif
	
    /* ISO 7816-3 Rule 7.4.2 */
    if (0 == *retries)
    {
        return RESYNC;
    }

    /* ISO 7816-3 Rule 7.2 */
    if (T1_R_BLOCK == t1_block_type(t1->previous_block[PCB]))
    {
        T1PRINTF("SMC T1: Rule 7.2\n");
        *p_slen = t1_rebuild(t1, sdata);
        return CONTINUE;
    }

    *p_slen = t1_build(t1, sdata, dad, T1_R_BLOCK | T1_EDC_ERROR, NULL, NULL);
    return CONTINUE;
}

static int t1_bad_nad_process(struct smc_device *dev, UINT32 actual, UINT8 *rdata, UINT8 *sdata,  \
                                    UINT32 *retries, UINT32  *p_slen)
{
    UINT32 i = 0;
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    t1_state_t *t1 = &tp->T1;

    T1PRINTF("SMC T1: Bad NAD, retry\n");
    for (i = 0; i < actual; i++)
    {
        T1PRINTF(" 0x%02x ", rdata[i]);
    }
    /* ISO 7816-3 Rule 7.4.2 */
    if (0 == retries)
    {
        return RESYNC;
    }
    /* ISO 7816-3 Rule 7.2 */
    if (T1_R_BLOCK == t1_block_type(t1->previous_block[PCB]))
    {
        T1PRINTF("SMC T1: Rule 7.2\n");
        *p_slen = t1_rebuild(t1, sdata);
        return CONTINUE;
    }
    return CONTINUE;
}

static int t1_r_block_process(struct smc_device *dev, UINT8 *rdata, UINT8 *sdata, UINT8 pcb, \
                            UINT32 *retries, UINT8 dad, UINT32 *p_slen, t1_buf_t *sbuf)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    t1_state_t *t1 = &tp->T1;

    /* length != 0x00 (illegal), b6 of pcb is set */
    if ((rdata[LEN] != 0x00) || (pcb & 0x20))
    {
        T1PRINTF("SMC T1: R-Block required\n");
        /* ISO 7816-3 Rule 7.4.2 */
        if (0 == retries)
        {
            return RESYNC;
        }

        /* ISO 7816-3 Rule 7.2 */
        if (T1_R_BLOCK == t1_block_type(t1->previous_block[1]))
        {
            T1PRINTF("SMC T1: Rule 7.2\n");
            *p_slen = t1_rebuild(t1, sdata);
            return CONTINUE;
        }

        *p_slen = t1_build(t1, sdata, dad, T1_R_BLOCK | T1_OTHER_ERROR, NULL, NULL);
        return CONTINUE;
    }
    /* wrong sequence number & no bit more */
	T1PRINTF("SMC T1: t1->ns=%d, t1->more=%d\n", t1->ns, t1->more);
    if (((t1_seq(pcb) != t1->ns) && (!t1->more)))
    {
        T1PRINTF("SMC T1: received: %d, expected: %d, more: %d\n", \
            t1_seq(pcb), t1->ns, t1->more);

        /* ISO 7816-3 Rule 7.2 */
        if (T1_R_BLOCK == t1_block_type(t1->previous_block[PCB]))
        {
            T1PRINTF("SMC T1: Rule 7.2\n");
            *p_slen = t1_rebuild(t1, sdata);
            return CONTINUE;
        }

        T1PRINTF("SMC T1:line=%d, R-Block required\n", __LINE__);
        /* ISO 7816-3 Rule 7.4.2 */
        if (0 == retries)
        {
        	T1PRINTF("SMC T1: line=%d, 0 == retries\n", __LINE__);
            return RESYNC;
        }
        *p_slen = t1_build(t1, sdata, dad, T1_R_BLOCK | T1_OTHER_ERROR, NULL, NULL);
		T1PRINTF("SMC T1: line=%d, slen=%d, continue\n", __LINE__, *p_slen);
        return CONTINUE;
    }

    if (RECEIVING == t1->state)
    {
        /* ISO 7816-3 Rule 7.2 */
        if (T1_R_BLOCK == t1_block_type(t1->previous_block[1]))
        {
            T1PRINTF("SMC T1: Rule 7.2\n");
            *p_slen = t1_rebuild(t1, sdata);
            return CONTINUE;
        }

        T1PRINTF("RECEIVING == t1->state");
        *p_slen = t1_build(t1, sdata,dad, T1_R_BLOCK, NULL, NULL);
        return BREAK;
    }

    /* If the card terminal requests the next
     * sequence number, it received the previous
     * block successfully */
    if (t1_seq(pcb) != t1->ns)
    {
        t1_buf_get(sbuf, NULL, g_last_send);
        g_sent_length += g_last_send;
        g_last_send = 0;
        t1->ns ^= 1;
    }	
	else
	{	
		if (T1_R_BLOCK != t1_block_type(t1->previous_block[1]))
		{
			return CONTINUE;		
		}
	}

    /* If there's no data available, the ICC
     * shouldn't be asking for more */
    if (0 == t1_buf_avail(sbuf))
    {
    	T1PRINTF("SMC T1: line=%d, RESYNC\n", __LINE__);
        return RESYNC;
    }

    *p_slen = t1_build(t1, sdata, dad, T1_I_BLOCK, sbuf, &g_last_send);
	T1PRINTF("SMC T1: line=%d, return!\n", __LINE__);
    return 0;
}

static int t1_i_block_process(struct smc_device *dev, UINT8 *rdata, UINT8 *sdata, UINT8 pcb, \
                             UINT8 dad, UINT32 *p_slen, t1_buf_t *sbuf, t1_buf_t *rbuf)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    t1_state_t *t1 = &tp->T1;

    /* The first I-block sent by the ICC indicates
     * the last block we sent was received successfully. */
    if (SENDING == t1->state)
    {
        T1PRINTF("");
        t1_buf_get(sbuf, NULL, g_last_send);
        g_last_send = 0;
        t1->ns ^= 1;
    }

    t1->state = RECEIVING;
	
    /* If the block sent by the card doesn't match
     * what we expected it to send, reply with
     * an R block */
    if (t1_seq(pcb) != t1->nr)
    {
        T1PRINTF("SMC T1: wrong nr\n");
        *p_slen = t1_build(t1, sdata, dad, T1_R_BLOCK | T1_OTHER_ERROR, NULL, NULL);
        return CONTINUE;
    }

    t1->nr ^= 1;

    if (t1_buf_put(rbuf, rdata + 3, rdata[LEN]) < 0)
    {
        T1PRINTF("SMC T1: buffer overrun by %d bytes\n", rdata[LEN] - (rbuf->size - rbuf->tail));
        return ERROR;
    }

    if (0 == (pcb & T1_MORE_BLOCKS))
    {
    	T1PRINTF("SMC T1: line=%d, DONE!\n", __LINE__);
        return DONE;
    }

    *p_slen = t1_build(t1, sdata, dad, T1_R_BLOCK, NULL, NULL);
	T1PRINTF("SMC T1: line=%d, return 0!\n", __LINE__);
    return 0;
}

static int t1_goto_error(struct smc_device *dev)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    t1_state_t *t1 = &tp->T1;

    t1->state = DEAD;
    return !RET_SUCCESS;
}

static int t1_no_more_retry_error(struct smc_device *dev, UINT32 times)
{

    if(0 == times)
    {
        return t1_goto_error(dev);
    }

    return RET_SUCCESS;
}
static int t1_param_init(struct smc_device *dev, UINT32 snd_len)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    t1_state_t *t1 = &tp->T1;

    if (0 == snd_len)
    {
        return !RET_SUCCESS;
    }
    /* we can't talk to a dead card / reader. Reset it! */
    if (DEAD == t1->state)
    {
        T1PRINTF("SMC T1: T=1 state machine is DEAD. Reset the card first.\n");
        return !RET_SUCCESS;
    }
    t1->state = SENDING;

    return RET_SUCCESS;
}

static int t1_check_ret_n(struct smc_device *dev, UINT32 *retries, INT32 n)
{
    if ((SMART_NO_ANSWER == n)||(SMART_WRITE_ERROR == n))
    {
        if (0 == *retries)
        {
        	T1PRINTF("SMC T1: [%s %d], ERROR\n", __FUNCTION__, __LINE__);
            return ERROR;
        }
		T1PRINTF("SMC T1: [%s %d], CONTINUE!\n", __FUNCTION__, __LINE__);
        return CONTINUE; /* resend the command */
    }
    if (n < 0)
    {
        T1PRINTF("SMC T1: fatal: transmit/receive failed\n");
        return ERROR;
    }

    return 0;
}

static int t1_s_block_ifs(struct smc_device *dev, UINT8 *rdata, UINT8 *sdata, UINT8 pcb, \
                             UINT8 dad, UINT32 *p_slen, t1_buf_t *tbuf)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    t1_state_t *t1 = &tp->T1;

    if (rdata[LEN] != 1)
    {
        T1PRINTF("SMC T1: Wrong length: %d\n", rdata[LEN]);
        *p_slen = t1_build(t1, sdata, dad, T1_R_BLOCK | T1_OTHER_ERROR, NULL, NULL);
        return CONTINUE;
    }
    T1PRINTF("SMC T1: sent S-block with ifs=%u\n", rdata[DATA]);
    if (0 == rdata[DATA])
    {
    	T1PRINTF("SMC T1: [%s %d], RESYNC!\n", __FUNCTION__, __LINE__);
        return RESYNC;
    }
    t1->ifsc = rdata[DATA];
    t1_buf_putc(tbuf, rdata[DATA]);
    *p_slen = t1_build(t1, sdata, dad, T1_S_BLOCK | T1_S_RESPONSE | T1_S_TYPE(pcb), tbuf, NULL);
	T1PRINTF("SMC T1: [%s %d], return 0!\n", __FUNCTION__, __LINE__);
    return 0;
}

static int t1_s_block_wtx(struct smc_device *dev, UINT8 *rdata, UINT8 *sdata, UINT8 pcb, \
                             UINT8 dad, UINT32 *p_slen, t1_buf_t *tbuf)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    t1_state_t *t1 = &tp->T1;

    if (rdata[LEN] != 1)
    {
        T1PRINTF("SMC T1: Wrong length: %d\n", rdata[LEN]);
        *p_slen = t1_build(t1, sdata, dad, T1_R_BLOCK | T1_OTHER_ERROR, NULL, NULL);
        return CONTINUE;
    }
    T1PRINTF("SMC T1: sent S-block with wtx=%u\n", rdata[DATA]);
    t1->wtx = rdata[DATA];
    t1_buf_putc(tbuf, rdata[DATA]);
    *p_slen = t1_build(t1, sdata, dad, T1_S_BLOCK | T1_S_RESPONSE | T1_S_TYPE(pcb), tbuf, NULL);

    return 0;
}

static int t1_s_block_response(struct smc_device *dev, UINT8 *sdata, UINT8 pcb, \
                             UINT8 dad, UINT32 *p_slen, UINT32 *retries)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    t1_state_t *t1 = &tp->T1;

    if (T1_S_IS_RESPONSE(pcb))
    {
        if (0 == *retries)
        {
        	T1PRINTF("SMC T1: [%s %d], RESYNC\n", __FUNCTION__, __LINE__);
            return RESYNC;
        }
        /* ISO 7816-3 Rule 7.2 */
        if (T1_R_BLOCK == t1_block_type(t1->previous_block[PCB]))
        {
            T1PRINTF("SMC T1: Rule 7.2\n");
            *p_slen = t1_rebuild(t1, sdata);
            return CONTINUE;
        }
        T1PRINTF("SMC T1: wrong response S-BLOCK received\n");
        *p_slen = t1_build(t1, sdata,dad, T1_R_BLOCK | T1_OTHER_ERROR,NULL, NULL);
        return CONTINUE;
    }
    return 0;
}
/* Send an APDU through T=1, rcv_len usually the size of rcv_buf */
/* return the actual size readed to the rcv_buf */
INT32 t1_transceive(struct smc_device *dev, UINT8 dad, const void *snd_buf,  \
                   UINT32 snd_len, void *rcv_buf, UINT32 rcv_len)
{
    struct smartcard_private *tp = (struct smartcard_private *)dev->priv;
    t1_state_t *t1 = &tp->T1;
    t1_buf_t sbuf;
    t1_buf_t rbuf;
    t1_buf_t tbuf;
    UINT8    sblk[5];
    UINT32    slen = 0;
    UINT32 retries = 0;
    UINT32 resyncs = 0;
    UINT32 actual = 0;
    UINT8 *sdata = &(t1->sdata[0]);
    UINT8 *rdata = &(t1->rdata[0]);
    UINT8 pcb = 0;
    INT32 n = 0;
    INT32 ret = 0;

    MEMSET(&sbuf, 0x0, sizeof (sbuf));
    MEMSET(&rbuf, 0x0, sizeof (rbuf));
    MEMSET(&tbuf, 0x0, sizeof (tbuf));
	ret = t1_param_init(dev, snd_len);
    if(ret != RET_SUCCESS)
    {
        return ret;
    }
    retries = t1->retries+1;
    resyncs = 3;
    /* Initialize send/recv buffer */
    t1_buf_set(&sbuf, (void *) snd_buf, snd_len);
    t1_buf_init(&rbuf, rcv_buf, rcv_len);
    /* Send the first block */
    slen = t1_build(t1, sdata, dad, T1_I_BLOCK, &sbuf, &g_last_send);

    while (1)
    {
        retries--;
		T1PRINTF("[%s %d], retries=%d\n", __FUNCTION__, __LINE__, retries);
        ret = t1_no_more_retry_error(dev, retries);
        if (ret != RET_SUCCESS)
        {
            goto error;
        }
        
        n = t1_xcv(dev, sdata, slen, rdata, T1_BUFFER_SIZE, &actual);
        ret = t1_check_ret_n(dev, &retries, n);
		T1PRINTF("[%s %d], slen=%d, actual=%d, n=%d, ret=%d\n", __FUNCTION__, __LINE__, slen, actual, n, ret);
        if(ERROR == ret)
        {
            goto error;
        }
        else if(CONTINUE == ret)
        {
			slen = t1_build(t1, sdata, dad, T1_R_BLOCK | T1_OTHER_ERROR, NULL, NULL);//Rule7.1, Rule 7.2
            continue;
        }
             /* wrong NAD */                /* length == 0xFF (illegal) */
        if ((rdata[NAD] != swap_nibbles(dad)) || (0xFF == rdata[LEN]))
        {
            ret = t1_bad_nad_process(dev, actual, rdata, sdata, &retries, &slen);
            if(RESYNC == ret)
            {
                goto resync;
            }
            else if(CONTINUE == ret)
            {
                continue;
            }
        }
        if (!t1_verify_checksum(t1, rdata, actual))
        {
            ret = t1_checksum_fail_process(dev, actual, rdata, sdata, &retries, dad, &slen, &sbuf);
            if(RESYNC == ret)
            {
                goto resync;
            }
            else if(CONTINUE == ret)
            {
                continue;
            }
        }
        pcb = rdata[PCB];
        switch (t1_block_type(pcb))
        {
            case T1_R_BLOCK:				
				
                ret = t1_r_block_process(dev, rdata, sdata, pcb, &retries, dad, &slen, &sbuf);
                if (RESYNC == ret)
                {
                    goto resync;
                }
                else if (CONTINUE == ret)
                {
                    continue;
                }
                else if (BREAK == ret)
                {
                    break;
                }
                break;

            case T1_I_BLOCK:
                ret = t1_i_block_process(dev, rdata, sdata, pcb, dad, &slen, &sbuf, &rbuf);
                if (CONTINUE == ret)
                {
                    continue;
                }
                else if (ERROR == ret)
                {
                    goto error;
                }
                else if (DONE == ret)
                {
                    goto done;
                }
                break;

            case T1_S_BLOCK:
                if ((T1_S_IS_RESPONSE(pcb)) && (RESYNCH == t1->state))
                {
                    T1PRINTF("SMC T1: S-Block answer received\n");
                    t1->state = SENDING;
                    g_sent_length =0;
                    g_last_send = 0;
                    resyncs = 3;
                    retries = t1->retries;
                    t1_buf_init(&rbuf, rcv_buf, rcv_len);
                    slen = t1_build(t1, sdata, dad, T1_I_BLOCK, &sbuf, &g_last_send);
                    continue;
                }
                ret = t1_s_block_response(dev, sdata, pcb, dad, &slen, &retries);
                if (CONTINUE == ret)
                {
                    continue;
                }
                else if (RESYNC == ret)
                {
                    goto resync;
                }
                t1_buf_init(&tbuf, sblk, sizeof(sblk));
                T1PRINTF("SMC T1: S-Block request received\n");
                switch (T1_S_TYPE(pcb))
                {
                    case T1_S_RESYNC:
                        if (rdata[LEN] != 0)
                        {
                            T1PRINTF("SMC T1: Wrong length: %d\n", rdata[LEN]);
                            slen = t1_build(t1, sdata, dad, T1_R_BLOCK | T1_OTHER_ERROR, NULL, NULL);
                            continue;
                        }
                        T1PRINTF("SMC T1: Resync requested\n");/* the card is not allowed to send a resync. */
                        break;
                    case T1_S_ABORT:
                        if (rdata[LEN] != 0)
                        {
                            T1PRINTF("SMC T1: Wrong length: %d\n", rdata[LEN]);
                            slen = t1_build(t1, sdata, dad, T1_R_BLOCK | T1_OTHER_ERROR, NULL, NULL);
                            continue;
                        }
                        T1PRINTF("SMC T1: Abort requested\n");
                        break;
                    case T1_S_IFS:
                        ret = t1_s_block_ifs(dev, rdata, sdata, pcb, dad, &slen, &tbuf);
                        if (CONTINUE == ret)
                        {
                            continue;
                        }
                        else if (RESYNC == ret)
                        {
                            goto resync;
                        }
                        break;
                    case T1_S_WTX:
                        ret = t1_s_block_wtx(dev, rdata, sdata, pcb, dad, &slen, &tbuf);
                        if(CONTINUE == ret)
                        {
                            continue;
                        }
                        break;
                    default:
                        T1PRINTF("SMC T1: T=1: Unknown S block type 0x%02x\n", T1_S_TYPE(pcb));
                        break;
                }
                break;
            default:
                break;
        }
        /* Everything went just splendid */
        retries = t1->retries;
        continue;
resync:
        /* the number or resyncs is limited, too. ISO 7816-3 Rule 6.4 */
        t1_no_more_retry_error(dev, resyncs);
        resyncs--;
        t1->ns = 0;
        t1->nr = 0;
        slen = t1_build(t1, sdata, dad, T1_S_BLOCK|T1_S_RESYNC, NULL, NULL);
        t1->state = RESYNCH;
        t1->more = FALSE;
        retries = 1;
        continue;
    }
done:
    return t1_buf_avail(&rbuf);
error:
    t1->state = DEAD;
    return -1;
}
