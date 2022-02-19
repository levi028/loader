/**
 * swapbuf for data transfer from media source to rtp writer.
 * This model is ALi DMX data transfer model, It just like pipe.
 *
 * It is safe only at the use case: one-reader vs one-writer
 *
 * Create by Davy Wu
 */

#ifndef _SWAP_BUF_H__
#define _SWAP_BUF_H__
#include <basic_types.h>
#include <sys_config.h>

typedef struct _SWAPBUF_t  swapbuf_t;

/**
 * swapbuf_new:
 * Try to malloc one swap buffer.
 * @bufsiz: the data buffer size.
 */
swapbuf_t* swapbuf_new(UINT32 bufsiz);

/**
 * swapbuf_free:
 * @swb: release one swapbuf, free data buffer space.
 */
void swapbuf_free(swapbuf_t* swb);


/**
 * swapbuf_get_datacnt:
 * @swb: get the operating swapbuf's total free unit count.
 */
UINT32 swapbuf_get_freecnt(swapbuf_t*swb);

/**
 * swapbuf_get_datacnt:
 * @swb: get the operating swapbuf's total data unit count.
 */
UINT32 swapbuf_get_datacnt(swapbuf_t*swb);

/**
 * swapbuf_request_read:
 * @dp: the operating swapbuf.
 * @rUnitCnt: The unit count want to read.
 * @r_addr: output address that can read data.
 * @o_unit_cnt: output, the unit count that can read
 */
void swapbuf_read_request(swapbuf_t*swb, UINT32 rUnitCnt, UINT8** r_addr, UINT32 *o_unit_cnt);

/**
 * swapbuf_update_read:
 * @dp: the operating swapbuf.
 * @uUnitCnt: the unit count going to update read indicator, it should be call after read data.
 */
void swapbuf_read_update(swapbuf_t*swb, UINT32 uUnitCnt);

/**
 * swapbuf_request_write:
 * @dp: the operating swapbuf.
 * @wUnitCnt: the unit count want to write.
 * @w_addr: output, the address that can write to.
 * @o_unit_cnt: output, the unit count that can write.
 */
void swapbuf_write_request(swapbuf_t*swb, UINT32 wUnitCnt, UINT8** w_addr, UINT32 *o_unit_cnt);

/**
 * swapbuf_update_write:
 * @dp: the operating swapbuf
 * @uUnitCnt: the unit count going to update write indicator, it should be call after write data.
 */
void swapbuf_write_update(swapbuf_t*swb, UINT32 uUnitCnt);

void swapbuf_reset(swapbuf_t* swb);

#endif /* _SWAP_BUF_H__ */
