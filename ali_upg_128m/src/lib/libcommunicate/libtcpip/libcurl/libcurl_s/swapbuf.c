/**
 * swapbuf for data transfer from media source to rtp writer.
 * This model is ALi DMX data transfer model, It just like pipe.
 *
 * It is safe only at the use case: one-reader vs one-writer
 *
 * Create by Davy Wu
 */

#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include "swapbuf.h"


struct _SWAPBUF_t {
    UINT32 buffSiz;
    INT32 dataCount; // for debug
    INT32 readCount; // for debug
    UINT32 read_idx; //unit
    UINT32 write_idx; //unit
    UINT8 *buffer;
};

#define SWB_PR(...) do {}while(0)
//libc_printf


swapbuf_t* swapbuf_new(UINT32 bufsiz)
{
    swapbuf_t* swb = NULL;
    if (bufsiz <=0 )
        goto exit_fn;
    swb = (swapbuf_t*)malloc(sizeof(swapbuf_t));

    if (swb == NULL)
        goto exit_fn;

    MEMSET(swb, 0, sizeof(swapbuf_t));

    swb->buffSiz = bufsiz;
    swb->buffer  = (UINT8*)malloc(bufsiz);

    if (swb->buffer == NULL) {
        free(swb);
        goto exit_fn;
    }
exit_fn:
    return swb;
}
void swapbuf_free(swapbuf_t* swb)
{
    if (swb == NULL)
        goto exit_fn;

    if (swb->buffer)
        free(swb->buffer);

    free(swb);
exit_fn:
    return;
}
void swapbuf_reset(swapbuf_t* swb)
{
    if (swb != NULL) {
        swb->read_idx = swb->write_idx = 0;
        swb->dataCount = 0;
        swb->readCount = 0;
        MEMSET(swb->buffer, 0, swb->buffSiz);
    }
}



/**
 * swapbuf_get_freecnt:
 * @swb: get the operating swapbuf's total free unit count.
 */
UINT32 swapbuf_get_freecnt(swapbuf_t*swb)
{
    UINT32 ret = 0;
    if (swb == NULL) {
        return ret;
    }

    if (swb->write_idx >= swb->read_idx) {
        ret = swb->buffSiz - (swb->write_idx - swb->read_idx);
    }
    else {
        ret = (swb->read_idx - swb->write_idx);
    }

    return ret;
}

/**
 * swapbuf_get_datacnt:
 * @swb: get the operating swapbuf's total data unit count.
 */
UINT32 swapbuf_get_datacnt(swapbuf_t*swb)
{
    UINT32 ret = 0;
    if (swb == NULL) {
        return ret;
    }

    if (swb->write_idx >= swb->read_idx) {
        ret = (swb->write_idx - swb->read_idx);
    }
    else {
        ret = swb->buffSiz - (swb->read_idx- swb->write_idx);
    }

    return ret;
}

void swapbuf_read_request(swapbuf_t*swb, UINT32 rUnitCnt, UINT8** r_addr, UINT32 *o_unit_cnt)
{
    INT32 unitFreeCnt = 0, unitDatacnt = 0;
    UINT32 write_idx, read_idx;

    if (swb==NULL || r_addr==NULL || o_unit_cnt == NULL) {
        goto exit_fn;
    }

    write_idx = swb->write_idx;
    read_idx  = swb->read_idx;
    unitDatacnt = write_idx - read_idx;

    if (unitDatacnt >= 0) {
        unitFreeCnt = swb->buffSiz - unitDatacnt;
    }
    else {
        unitDatacnt = swb->buffSiz + unitDatacnt;
        unitFreeCnt = swb->buffSiz - unitDatacnt;
    }

    *o_unit_cnt = 0;
    *r_addr = NULL;
    if (unitDatacnt <= 0) {
        goto exit_fn; // no data
    }

    if (write_idx > read_idx) {
        if (unitDatacnt >= rUnitCnt) {
            *o_unit_cnt = rUnitCnt;
        }
        else {
            *o_unit_cnt = unitDatacnt;
        }
        *r_addr = swb->buffer + read_idx;
    }
    else { // w<r
        if ((swb->buffSiz-read_idx) >= rUnitCnt) {
            *o_unit_cnt = rUnitCnt;
        }
        else {
            *o_unit_cnt = (swb->buffSiz-read_idx);
        }
        *r_addr = swb->buffer + swb->read_idx;
    }
exit_fn:
    SWB_PR("rd: fcnt %d, rcnt %d\n", unitFreeCnt, *o_unit_cnt);

    return;
}

void swapbuf_read_update(swapbuf_t*swb, UINT32 uUnitCnt)
{
    INT32 unitFreeCnt = 0, unitDatacnt = 0;
    UINT32 write_idx, read_idx;

    if (swb == NULL)
        goto exit_fn;

    write_idx = swb->write_idx;
    read_idx  = swb->read_idx;
    unitDatacnt = write_idx - read_idx;

    if (unitDatacnt >= 0) {
        unitFreeCnt = swb->buffSiz - unitDatacnt;
    }
    else {
        unitDatacnt = swb->buffSiz + unitDatacnt;
        unitFreeCnt = swb->buffSiz - unitDatacnt;
    }

    if (uUnitCnt > unitDatacnt) {
        goto exit_fn;
    }

    swb->readCount += uUnitCnt;
    swb->read_idx = (swb->read_idx + uUnitCnt)%swb->buffSiz;

exit_fn:
    SWB_PR("r..up fnct %d, ucnt %d\n", unitFreeCnt, uUnitCnt);
    return;
}

void swapbuf_write_request(swapbuf_t*swb, UINT32 wUnitCnt, UINT8** w_addr, UINT32 *o_unit_cnt)
{
    INT32 unitFreeCnt = 0, unitDatacnt = 0;
    UINT32 write_idx, read_idx;

    if (swb==NULL || w_addr==NULL || o_unit_cnt == NULL) {
        goto exit_fn;
    }

    write_idx = swb->write_idx;
    read_idx  = swb->read_idx;

    unitDatacnt = write_idx - read_idx;

    if (unitDatacnt >= 0) {
        unitFreeCnt = swb->buffSiz - unitDatacnt;
    }
    else {
        unitDatacnt = swb->buffSiz + unitDatacnt;
        unitFreeCnt = swb->buffSiz - unitDatacnt;
    }

    *w_addr = NULL;
    *o_unit_cnt = 0;
    if (unitFreeCnt <= 1) { // keep one empty
        goto exit_fn;
    }

    if (write_idx >= read_idx) { // w>r
        if ((swb->buffSiz - write_idx) >= wUnitCnt) {
            *o_unit_cnt = wUnitCnt;
        }
        else {
            *o_unit_cnt = (swb->buffSiz - swb->write_idx);
        }
        if (*o_unit_cnt > (unitFreeCnt - 1))
            *o_unit_cnt = (unitFreeCnt - 1);
        *w_addr = swb->buffer + write_idx;
    }
    else {// r>w
        if ((unitFreeCnt -1) >= wUnitCnt)
            *o_unit_cnt = wUnitCnt;
        else
            *o_unit_cnt = (unitFreeCnt -1);
        *w_addr = swb->buffer + write_idx;
    }
exit_fn:
    SWB_PR("wr: fcnt %d, wcnt %d\n", unitFreeCnt, *o_unit_cnt);
    return;
}

void swapbuf_write_update(swapbuf_t*swb, UINT32 uUnitCnt)
{
    INT32 unitFreeCnt = 0, unitDatacnt = 0;
    UINT32 write_idx, read_idx;
    if (swb == NULL)
        goto exit_fn;

    write_idx = swb->write_idx;
    read_idx  = swb->read_idx;
    unitDatacnt = write_idx - read_idx;

    if (unitDatacnt >= 0) {
        unitFreeCnt = swb->buffSiz - unitDatacnt;
    }
    else {
        unitDatacnt = swb->buffSiz + unitDatacnt;
        unitFreeCnt = swb->buffSiz - unitDatacnt;
    }

    if (uUnitCnt >= unitFreeCnt ) {
        goto exit_fn;
    }

    swb->write_idx = (swb->write_idx + uUnitCnt) % swb->buffSiz;
    swb->dataCount += uUnitCnt;

exit_fn:
    SWB_PR("w..up fcnt %d, ucnt %d\n",unitFreeCnt, uUnitCnt);
    return;
}

