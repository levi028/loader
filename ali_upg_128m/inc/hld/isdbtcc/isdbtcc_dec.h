#ifndef    __HLD_ISDBTCC_DEC_H__
#define __HLD_ISDBTCC_DEC_H__

#include <basic_types.h>
#include <hld/osd/osddrv.h>
#include <hld/sdec/sdec.h>
#include <hld/snd/snd.h>

#define ISDBTCC_DEC_CMD_START                            0x00000001
#define ISDBTCC_DEC_CMD_STOP                              0x00000002
#define ISDBTCC_DEC_CMD_PAUSE                            0x00000004
#define ISDBTCC_DEC_MSG_START_OK                      0x00000100
#define ISDBTCC_DEC_MSG_STOP_OK                        0x00000200
#define ISDBTCC_DEC_MSG_PAUSE_OK                      0x00000400
#define ISDBTCC_DEC_MSG_DATA_AVAILABLE          0x00010000
//#define ISDBTCC_DEC_CMD_SHOWON                   0x00020000
//#define ISDBTCC_DEC_CMD_SHOWOFF                   0x00040000
//#define ISDBTCC_DEC_MSG_SHOWON_OK               0x01000000
//#define ISDBTCC_DEC_MSG_SHOWOFF_OK               0x02000000

#define ISDBTCC_DEC_STATE_IDLE        0
#define ISDBTCC_DEC_STATE_PLAY        1
#define ISDBTCC_DEC_STATE_PAUSE    2

struct isdbtcc_priv
{
    void *isdbtcc_priv_addr;
    void (* stream_parse_cb)(void);
    BOOL (* display_entry_cb)(void);
    INT32 (*isdbtcc_draw_char)(UINT8, UINT16, UINT16, UINT8);
    BOOL (*isdbtcc_region_is_created)(UINT8);
    INT32 (*isdbtcc_create_region)(UINT8, struct osdrect *);
    INT32 (*isdbtcc_region_show)(UINT8, UINT8);
    INT32 (*isdbtcc_delete_region)(UINT8);
    INT32 (*isdbtcc_get_region_addr)(UINT8 ,UINT16 , UINT32 *);
    UINT8 status;
};

struct ccdec_data_hdr
{
    BOOL b_info_valid;

    UINT8 u_pes_start;
    // Bit map to specify which parameter in the structure is valid: "1" valid, "0" not invlid .
    // Bit0: STC_ID section valid or not;
    // bit1: Data discontinue or not
    // Bit2: PTS section valid or not;
    // Bit3: uVobuNum and uVobuBaseTime section valid or not.
    // Bit4: Video trick mode or not

    UINT8    u_ctrl_byte;
    UINT8    u_stc_id;    // System timer counter identify:0: STC 0;1: STC 1;
    UINT32    u_pts;    // Corresponding PTS value for a data block.

    UINT32 u_data_size;     // data size
    UINT8* pu_start_point;
};




#ifdef __cplusplus
extern "C"
{
#endif

INT32 isdbtcc_dec_open(struct sdec_device *dev);
INT32 isdbtcc_dec_close(struct sdec_device *dev);
INT32  isdbtcc_dec_start(struct sdec_device *dev,UINT16,UINT16);
INT32  isdbtcc_dec_stop(struct sdec_device *dev);
INT32  isdbtcc_dec_pause(struct sdec_device *dev);

//INT32  isdbtcc_dec_showonoff(struct sdec_device *dev,BOOL bOn);
INT32 isdbtcc_dec_request_write(struct sdec_device *dev,UINT32 u_size_requested,struct control_block* p_tdata_ctrl_blk,UINT8** ppu_data,UINT32* pu_size_got);
void isdbtcc_dec_update_write(struct sdec_device *dev,UINT32 data_size);

INT32 isdbtcc_dec_attach(struct sdec_feature_config * cfg_param);
void icc_draw_char1(UINT16 x, UINT16 y, UINT8 fg_color, UINT8 bg_color,UINT16 character);

BOOL get_isdbtcc_dec_status();
#ifdef __cplusplus
}
#endif

#endif /*__HLD_ISDBTCC_DEC_H__*/

