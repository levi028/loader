/****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2002 Copyright (C)
 *
 *  File: ali_sbm.h
 *
 *  Description: ali share buffer memory for cpu & see access
 *s
 *  History:
 *      Date             Author         Version      Comment
 *      ======           ======          =====       =======
 *  1.  2011.08.03       Dylan.Yang     0.1.000     First version Created
 ****************************************************************************/
#ifndef __HLD_SBM_H__
#define __HLD_SBM_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define SBM_NUM                  10

#define SBM_REQ_OK	             0
#define SBM_REQ_FAIL	         1
#define SBM_REQ_BUSY	         2

#define SBM_CLOSED               0
#define SBM_CPU_READY            1
#define SBM_SEE_READY            2

#define SBM_MODE_NORMAL          0
#define SBM_MODE_PACKET          1

#define SBM_MUTEX_LOCK           0
#define SBM_SPIN_LOCK            1

#define SBMIO_CREATE_SBM         0
#define SBMIO_SHOW_VALID_SIZE    1
#define SBMIO_SHOW_FREE_SIZE     2

struct sbm_config
{
    unsigned int buffer_addr;
    unsigned int buffer_size;
    unsigned int block_size;
    unsigned int reserve_size;
    unsigned int wrap_mode;
    unsigned int lock_mode;
};

struct sbm_rw_desc 
{
    unsigned int valid_size;
	unsigned int read_pos;
	unsigned int write_pos;
};

struct sbm_rw_desc_pkt 
{
	unsigned int read_pos;
	unsigned int write_pos;
	unsigned int tmp_write_pos;
	unsigned int last_read_pos;
	unsigned char write_wrap_around;
    unsigned char read_wrap_around;
    unsigned int pkt_num;
    unsigned int valid_size;
};

struct sbm_desc
{
    struct sbm_config sbm_cfg;
    struct sbm_rw_desc *sbm_rw;
    int mutex;
    unsigned char status;
};

struct sbm_desc_pkt 
{
    struct sbm_config sbm_cfg;
    struct sbm_rw_desc_pkt *sbm_rw;
    int mutex;
    unsigned char status;
};

UINT32 ali_sbm_write(INT32 sbm_idx, const char *buf, UINT32 size);
int ali_sbm_create(INT32 sbm_idx, struct sbm_config sbm_init);
int ali_sbm_destroy(INT32 sbm_idx, INT32 sbm_mode);
int ali_sbm_reset(INT32 sbm_idx);
int ali_sbm_show_valid_size(INT32 sbm_idx, UINT32 *valid_size);
int ali_sbm_show_total_size(INT32 sbm_idx, UINT32 *total_size);
int ali_sbm_show_free_size(INT32 sbm_idx, UINT32 *free_size);

#ifdef __cplusplus
}
#endif

#endif
