/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: cache.h
*
*    Description: declaration of cache api
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _CACHE_H_
#define _CACHE_H_


#define ALLOW_WRITES  1
#define NO_WRITES     0

extern  int   init_block_cache(int heap_size, int flags);
extern  void  shutdown_block_cache(void);

extern  void  force_cache_flush(int dev, int prefer_log_blocks);
extern  int   flush_blocks(int dev, off_t bnum, int nblocks);
extern  int   flush_device(int dev, int warn_locked);

extern  int   init_cache_for_device(int fd, off_t max_blocks);
extern  int   remove_cached_device_blocks(int dev, int allow_write);

extern  void *get_block(int dev, off_t bnum, int bsize);
extern  void *get_empty_block(int dev, off_t bnum, int bsize);
extern  int   release_block(int dev, off_t bnum);
extern  int   mark_blocks_dirty(int dev, off_t bnum, int nblocks);


extern  int  cached_read(int dev, off_t bnum, void *data,
                             off_t num_blocks, int bsize);
extern  int  cached_write(int dev, off_t bnum, const void *data,
                              off_t num_blocks, int bsize);
extern  int  cached_write_locked(int dev, off_t bnum, const void *data,
                                     off_t num_blocks, int bsize);
extern  int  set_blocks_info(int dev, off_t *blocks, int nblocks,
                                 void (*func)(off_t bnum, size_t nblocks, void *arg),
                                 void *arg);

extern  size_t read_phys_blocks (int fd, off_t bnum, void *data,
                                     unsigned int num_blocks, int bsize);
size_t write_phys_blocks( int dev, off_t block_num, const void *p_buffer, unsigned  int n_block_count, int bsize );


ssize_t read_pos( int fd, off_t _pos, void *data, size_t nbytes );
ssize_t write_pos( int fd, off_t _pos, const void *data, size_t nbytes );


#endif /* _CACHE_H_ */
