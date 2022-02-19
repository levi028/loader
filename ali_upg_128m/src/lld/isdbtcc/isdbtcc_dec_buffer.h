#ifndef __ISDBTCC_DEC_BUFFER_H__
#define __ISDBTCC_DEC_BUFFER_H__


#include <sys_config.h>
#include <basic_types.h>

#define PKT_SIZE    184

#ifdef __cplusplus
extern "C"
{
#endif
//API call by Isdbtcc_dec.c
INT32 isdbtcc_dec_sbf_create(void);
INT32 isdbtcc_dec_sbf_wr_req(UINT32 * p_size, UINT8 ** pp_data);
void isdbtcc_dec_sbf_wr_update(UINT32 size);
INT32 isdbtcc_dec_sbf_rd_req(UINT32 * p_size, UINT8 ** pp_data);
void isdbtcc_dec_sbf_rd_update(UINT32 size);
INT32 isdbtcc_dec_hdr_buf_create(void);
INT32 isdbtcc_dec_hdr_buf_wr_req(UINT32* p_data);
void isdbtcc_dec_hdr_buf_wr_update();
INT32 isdbtcc_dec_hdr_buf_rd_req(UINT32 * p_data);
void isdbtcc_dec_hdr_buf_rd_update();

#ifdef __cplusplus
}
#endif


#endif /* __ISDBTCC_DEC_BUFFER_H__ */



