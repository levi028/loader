#ifndef _GACAS_UPG_H_
#define _GACAS_UPG_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "gos_config.h"
#include "gacas_loader_db.h"


#define GACAS_UPG_BUFFER_MAX_LEN 		0x800000//(GACAS_FLASH_TOTAL_SIZE+0x80)
#define GACAS_UPG_FLASH_LOCK_SIZE 		0x80000
#define GACAS_UPG_OPERATOR_NAME		"Gospell"
#define GACAS_UPG_OPERATOR_NAME_LEN	7
#define GACAS_UPG_TITLE_OTA				"Upgrade By OTA"
#define GACAS_UPG_TITLE_USB				"Upgrade By USB"
#define GAUSB_FILE_NAME 					"Ali3711c_DDR128M_HD_en.abs"    
#define GAUSB_FILE_DIR 					"/mnt/uda1/Ali3711c_DDR128M_HD_en.abs"    

typedef void (*gacas_upg_show_process)(UINT32);
void gacas_upg_set_burning_progress(void);
INT32 gacas_upg_burn_flash_by_addr(UINT32 upg_flash_addr,UINT32 upg_flash_size,UINT32 upg_file_addr,UINT32 upg_file_size,UINT8 b_save_loader);
INT32 gacas_upg_burn_flash_by_chunk(UINT8 b_save_loader);
UINT8 gacas_upg_get_updata_chunk_num(void);
INT32 gacas_upg_add_updata_chunk_id(UINT32 chunk_id);
INT32 gacas_upg_set_updata_chunk_id(UINT32 *p_chunk_id,UINT8 chunk_num);
INT32 gacas_upg_encrypt_chunk(UINT32 chunk_id,UINT8 b_data_encypt);
INT32 gacas_upg_check_file(UINT32 data_addr,UINT32 data_len);
void gacas_upg_init(gacas_upg_show_process progress_show);
void gacas_upg_flash_lock(void);
void gacas_upg_flash_unlock(void);
#ifdef __cplusplus
 }
#endif

#endif

