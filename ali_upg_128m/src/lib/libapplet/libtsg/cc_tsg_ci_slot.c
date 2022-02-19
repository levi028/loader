/*****************************************************************************
*    Copyright (C) 2008 ALi Corp. All Rights Reserved.
*
*    Company confidential and Properietary information.
*    This information may not be disclosed to unauthorized
*    individual.
*    File: cc_tsg_ci_slot.c
*
*    Description:
*
*    History:
*    Date           Athor        Version        Reason
*    ========       ========     ========       ========
*    2008/10/14
*
*****************************************************************************/
#include <sys_config.h>
#include <basic_types.h>
#include <retcode.h>
#include <mediatypes.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <hld/nim/nim_dev.h>
#include <hld/nim/nim.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>
#include <bus/tsi/tsi.h>
#include <api/libpvr/lib_pvr.h>
#include <api/libtsg/lib_tsg.h>
#include "cc_tsg_task.h"

#define CC_TSG_PRINTF(...)
//#define CC_TSG_PRINTF           libc_printf
#ifdef CI_SLOT_DYNAMIC_DETECT

#define CC_TSG_PRINTF2(...)

#define CC_CIC_MAX_DETECT_TIME  8
#define CC_CIC_MIN_DETECT_PKT   (2*1024)
#define CC_CIC_MAX_DETECT_PKT   (8*1024)
#define CC_CIC_MAX_FAKE_PKT     (CC_CIC_MIN_DETECT_PKT)


#define ENTER_CC_CIC_API()    do {osal_semaphore_capture(m_cc_cic_sema_id, TMO_FEVR); \
                            CC_TSG_PRINTF2("%s enter, %d\n", __FUNCTION__,__LINE__);}while(0)
#define LEAVE_CC_CIC_API()    do {osal_semaphore_release(m_cc_cic_sema_id);\
                            CC_TSG_PRINTF2("%s leave, %d\n", __FUNCTION__,__LINE__);}while(0)
static OSAL_ID m_cc_cic_sema_id = OSAL_INVALID_ID;

static struct dmx_device *m_tsg_ci_dmx_dev;

static UINT32 m_cc_tsg_video_scramb;
static UINT32 m_cc_tsg_audio_scramb;
static UINT8 m_tsg_fack_scrambled;
static UINT32 m_cc_tsg_scramb_flag;
static UINT32 m_tsg_dmx0_packet_num;
static UINT8 m_ci_slot_detect_flag;
static UINT8 m_ci_slot_current;
static UINT8 m_ci_descramble_ok;
static UINT8 m_ci_descramble_ok_slot;
static UINT8 m_ci_slot_ca_pmt_received;
static UINT32 m_cc_tsg_prog_id;
static BOOL m_cc_tsg_play_active;

#ifdef CC_TSG_DETECT_FAKE_SCRAMBLE
static UINT8 m_ci_slot_detect_dmx1_flag;
#endif

static UINT8 m_ci_slot_attached[CI_SLOT_NS];
static UINT8 m_ci_slot_passing[CI_SLOT_NS];
static enum CC_CIC_DESCRAMBLE_STATE m_ci_slot_descrambling[CI_SLOT_NS];

static UINT8 m_ci_slot_detect_times[CI_SLOT_NS];


static UINT8 m_tsg_task_reset_channel;
static UINT32 m_tsg_task_pause_count;

//BOOL cc_tsg_task_is_running(void);
void cc_tsg_ci_slot_task_start(UINT32 prog_id);
RET_CODE cc_tsg_ci_stop_task_stop(void);

static BOOL _cc_cic_is_stream_scrambled(struct dmx_device *dmx_dev)
{
    UINT8 scrabled = 0, scrable_typ = 0;

    if (dmx_io_control(dmx_dev, IS_AV_SCRAMBLED,(UINT32)(&scrable_typ)) == RET_SUCCESS )
    {
        if(scrable_typ & (VDE_TS_SCRBL|VDE_PES_SCRBL|AUD_TS_SCRBL|AUD_PES_SCRBL))
            scrabled = 1;
    }
    return scrabled;
}

#ifdef CC_TSG_DETECT_FAKE_SCRAMBLE
static BOOL _cc_cic_is_stream_scrambled_all(struct dmx_device *dmx_dev)
{
    UINT8 scrabled = 0, scrable_typ = 0;

    if (dmx_io_control(dmx_dev, IS_AV_SCRAMBLED,(UINT32)(&scrable_typ)) == RET_SUCCESS )
    {
        if (scrable_typ)
            scrabled = 1;
    }
    return scrabled;
}
#endif

static BOOL _cc_cic_slot_descrambling_except(UINT32 slot)
{
    UINT32 i;
    for (i = 0; i < CI_SLOT_NS; i++)
    {
        if (i != slot)
        {
            if (m_ci_slot_descrambling[i] == CC_CIC_DESCRAMBLE_OK)
                return TRUE;
            //#ifdef CC_USE_TSG_PLAYER
            if ( (pvr_get_with_tsg_using_status()) && (cc_tsg_task_is_running() || m_tsg_task_pause_count))
            //#endif
            {
                if (m_ci_slot_attached[i] && m_ci_slot_passing[i])
                {
                    if (!_cc_cic_is_stream_scrambled(m_tsg_ci_dmx_dev))
                    {
                        m_ci_slot_descrambling[i] = CC_CIC_DESCRAMBLE_OK;
                        return TRUE;
                    }
                }
            }
        }
    }

    return FALSE;
}

extern int default_route_tsi;
extern UINT8 default_tsi_polarity;
// before call this, please bypass all other slots
static BOOL _cc_cic_set_slot_passing(UINT32 slot, UINT32 pass)
{
    UINT8 ts_id;
    UINT8 ts_polar;

    if(m_cc_tsg_play_active)
    {
        ts_id = TSI_SPI_TSG;
        ts_polar = 0x03;
    }
    else
    {
        ts_id = default_route_tsi;
        ts_polar = default_tsi_polarity;
    }

    if (pass)
    {
        CC_TSG_PRINTF("passing slot %d, %s\n", slot,ts_id == TSI_SPI_TSG?"tsg p":"normal p");
        tsi_mode_set(ts_id, ts_polar&0x7f);
        dmx_io_control(m_tsg_ci_dmx_dev, IO_DMX_RESET, 0);
        api_set_nim_ts_type(3, 1<<slot);
        tsi_mode_set(ts_id, ts_polar|0x80);
        m_ci_slot_current = slot;
        m_ci_slot_passing[slot] = pass;
    }
    else
    {
        CC_TSG_PRINTF("bypass slot %d, %s\n", slot,ts_id == TSI_SPI_TSG?"tsg p":"normal p");
        tsi_mode_set(ts_id, ts_polar&0x7f);
        api_set_nim_ts_type(2, 1<<slot);
        tsi_mode_set(ts_id, ts_polar|0x80);
        m_ci_slot_passing[slot] = pass;
    }

    return TRUE;
}

static void _cc_cic_set_slot_descrambling(UINT32 slot, UINT32 descrambling)
{
    m_ci_slot_descrambling[slot] = descrambling;
}

static BOOL _cc_cic_slot_attached_except(UINT32 slot)
{
    UINT32 i;
    for (i = 0; i < CI_SLOT_NS; i++)
    {
        if (i != slot)
        {
            if (m_ci_slot_attached[i])
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

static BOOL _cc_cic_set_slot_bypass_except(UINT32 slot)
{
    UINT32 i;
    for (i = 0; i < CI_SLOT_NS; i++)
    {
        if (i != slot)
        {
            if (m_ci_slot_passing[i])
            {
                _cc_cic_set_slot_passing(i, FALSE);
            }
        }
    }
    return TRUE;
}

static void _cc_cic_select_slot(void)
{
    if (m_ci_slot_attached[m_ci_slot_current])
    {
        if (m_ci_slot_passing[m_ci_slot_current] == 0)
        {
            _cc_cic_set_slot_passing(m_ci_slot_current, TRUE);
        }
    }
}

static void _cc_cic_start_detect(void)
{
#ifdef CC_TSG_DETECT_FAKE_SCRAMBLE
    if (cc_tsg_task_is_running())
    {
        m_ci_slot_detect_dmx1_flag = TRUE;
        m_cc_tsg_video_scramb = 0;
        m_cc_tsg_audio_scramb = 0;
        m_cc_tsg_scramb_flag = 0;
        m_tsg_dmx0_packet_num = 0;
    }
    else
    {
        m_ci_slot_detect_dmx1_flag = FALSE;
    }
#endif
//#ifndef CC_USE_TSG_PLAYER

    if(pvr_get_with_tsg_using_status() == 0)
    {
        if(!m_tsg_ci_dmx_dev)
        {
            m_tsg_ci_dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);
        }

        dmx_io_control(m_tsg_ci_dmx_dev, IO_DMX_GLEAR_PACKET_NUM, 0);
    }
//#endif
     _cc_cic_select_slot();

    if (m_ci_slot_passing[m_ci_slot_current])
    {
        m_tsg_dmx0_packet_num = 0;
        m_ci_slot_detect_flag = _cc_cic_slot_attached_except(m_ci_slot_current);
        //osal_flag_clear(g_cc_tsg_flagid, MID27_FLG_CC_CI_DESCRAMBLE);

        CC_TSG_PRINTF("using slot %d\n", m_ci_slot_current);
        if (m_ci_slot_detect_flag)
        {
            CC_TSG_PRINTF("detecting slot %d\n", m_ci_slot_current);
        }
    }

    if (pvr_r_get_record_all())
    {
#ifdef CC_TSG_DETECT_FAKE_SCRAMBLE
        m_ci_slot_detect_dmx1_flag = 0;
#endif
        m_ci_slot_detect_flag = 0;
    }
}

static BOOL _cc_cic_goto_other_slot(UINT32 slot)
{
    // goto other slot please
    UINT32 i;

    for (i = 0; i < CI_SLOT_NS; i++)
    {
        if (i == slot)
            continue;
        if (m_ci_slot_attached[i])
        {
            if (m_ci_slot_detect_times[i] < CC_CIC_MAX_DETECT_TIME)
            {
                m_ci_slot_detect_times[i]++;
                _cc_cic_set_slot_bypass_except(i);
                _cc_cic_set_slot_passing(i, TRUE);

                //if (m_ci_slot_descrambling[i] != CC_CIC_DESCRAMBLE_FAILED)
                {
                    // start detect new slot
                    m_ci_slot_descrambling[i] = CC_CIC_DESCRAMBLE_UNKNOWN;
                    //api_send_ca_pmt(i);
                    _cc_cic_start_detect();
                    break;
                }
            }
        }
    }
#if 0
    if (i == CI_SLOT_NS)
    {
        osal_flag_set(g_cc_tsg_flagid, MID27_FLG_CC_CI_DESCRAMBLE);
    }
#endif
    return (i < CI_SLOT_NS);
}

static void _cc_tsg_set_fake_scramble(BOOL b_fake)
{
    if (b_fake)
    {
        _cc_cic_set_slot_descrambling(0, CC_CIC_DESCRAMBLE_OK);
        _cc_cic_set_slot_descrambling(1, CC_CIC_DESCRAMBLE_OK);
        if (m_ci_slot_passing[m_ci_slot_current])
        {
            // fake scrambled bypass all slots
            _cc_cic_set_slot_passing(m_ci_slot_current, FALSE);
        }
        m_ci_slot_detect_flag = 0;
        //osal_flag_set(g_cc_tsg_flagid, MID27_FLG_CC_CI_DESCRAMBLE);
        m_tsg_fack_scrambled = 1;
        CC_TSG_PRINTF("record is fake scramble\n");
    }
}

BOOL api_set_ci_slot_attached(UINT32 slot, BOOL b_attach)
{
    if (m_ci_slot_attached[slot] == b_attach)
        return TRUE;

    UINT32 send_pmt = FALSE;

    ENTER_CC_CIC_API();

    m_ci_descramble_ok = 0;
    m_ci_descramble_ok_slot = 0;

    m_ci_slot_attached[slot] = b_attach;

    if (b_attach)
    {
        CC_TSG_PRINTF("slot %d attached %d\n", slot, osal_get_tick());
        m_ci_slot_detect_times[slot] = 0;
        if (m_ci_slot_descrambling[slot] == CC_CIC_DESCRAMBLE_OK)
            goto ATTACH_OK;
        m_ci_slot_descrambling[slot] = CC_CIC_DESCRAMBLE_UNKNOWN;

        if (_cc_cic_slot_descrambling_except(slot))
        {
            // other slot descrable OK;
            CC_TSG_PRINTF("slot %d is working well\n", 1 - slot);
        }
        else
        {
            if (m_ci_slot_attached[m_ci_slot_current] && m_ci_slot_passing[m_ci_slot_current] && m_ci_slot_detect_flag == 1)
            {
                ; // m_ci_slot_current is detecting
                CC_TSG_PRINTF("slot %d is detecting\n", slot);
            }
            else
            {
                _cc_cic_set_slot_bypass_except(slot);
                _cc_cic_set_slot_passing(slot, TRUE);
                // start detect new slot
                send_pmt = TRUE;
                //api_send_ca_pmt(slot);
                //_cc_cic_start_detect();
            }
        }
    }
    else
    {
        CC_TSG_PRINTF("slot %d dettached\n", slot);
        if (m_ci_slot_passing[slot])
        {
            _cc_cic_set_slot_passing(slot, FALSE);
            if (m_ci_slot_descrambling[slot] == CC_CIC_DESCRAMBLE_OK)
            {
                if (_cc_cic_slot_descrambling_except(slot))
                {
                }
                else
                {
                    // goto other slot please
                    send_pmt = TRUE;
                }
            }
            m_ci_slot_descrambling[slot] = CC_CIC_DESCRAMBLE_UNKNOWN;
        }
        m_ci_slot_detect_times[slot] = 0;
    }

ATTACH_OK:

    LEAVE_CC_CIC_API();

    if (send_pmt)
    {
        if (b_attach)
        {
            api_send_ca_pmt(slot);
            _cc_cic_start_detect();
        }
        else
        {
            if (_cc_cic_goto_other_slot(slot))
                api_send_ca_pmt(m_ci_slot_current);
        }
    }

    return TRUE;
}

BOOL api_send_ca_pmt_auto(void)
{
    BOOL ret = FALSE;

    if (m_ci_slot_passing[m_ci_slot_current])
    {
        api_send_ca_pmt(m_ci_slot_current);
        ret = TRUE;
    }

    return ret;
}

void api_set_ci_slot_valid(UINT32 slot, BOOL valid)
{
    if (m_ci_slot_detect_flag && m_ci_slot_current == slot)
    {
        ENTER_CC_CIC_API();
        if (valid)
        {
            if (m_tsg_dmx0_packet_num > CC_CIC_MIN_DETECT_PKT)
                m_tsg_dmx0_packet_num = CC_CIC_MIN_DETECT_PKT;
            m_ci_slot_ca_pmt_received = 1;
        }
        else
        {
            m_ci_slot_ca_pmt_received = 1;
            if (m_ci_slot_passing[slot] && slot == m_ci_slot_current)
            {
                _cc_cic_goto_other_slot(slot);
            }
            m_ci_slot_detect_flag = 0;
            m_ci_slot_detect_times[slot] = CC_CIC_MAX_DETECT_TIME;
            m_ci_slot_descrambling[slot] = CC_CIC_DESCRAMBLE_FAILED;
        }
        LEAVE_CC_CIC_API();
    }
}

#ifdef CC_TSG_DETECT_FAKE_SCRAMBLE
void cc_tsg_ci_slot_set_scramb_flag(UINT8 *buf, UINT32 len, UINT16 video_pid, UINT16 audio_pid)
{
    if (m_ci_slot_detect_dmx1_flag)
    {
        if (m_tsg_dmx0_packet_num <= CC_CIC_MAX_FAKE_PKT)
        {
            m_cc_tsg_scramb_flag = check_ts_scramble(buf, len, video_pid, audio_pid, &m_cc_tsg_video_scramb, &m_cc_tsg_audio_scramb);
        }
    }
}
#endif

void cc_tsi_dmx_config(UINT8 tsi_dmx_x, BOOL use_tsg, BOOL pass_ci)
{
    if (use_tsg)
        tsi_select(tsi_dmx_x, TSI_SPI_TSG);
    else
        tsi_select(tsi_dmx_x, default_route_tsi);

    if (pass_ci)
        tsi_dmx_src_select(tsi_dmx_x, TSI_TS_A);
    else
        tsi_dmx_src_select(tsi_dmx_x, TSI_TS_B);
}

void cc_tsg_set_tsg_play_flag(BOOL flag)
{
    m_cc_tsg_play_active = flag;
}

void cc_tsg_ci_slot_set_ca_pmt_received(UINT8 flag)
{
    m_ci_slot_ca_pmt_received = flag;
}

void cc_tsg_ci_slot_reset(void)
{
    UINT32 i;

    ENTER_CC_CIC_API();

    for (i = 0; i < CI_SLOT_NS; i++)
    {
        m_ci_slot_descrambling[i] = CC_CIC_DESCRAMBLE_UNKNOWN;
        m_ci_slot_detect_times[i] = 0;
    }

    _cc_cic_start_detect();

    if (m_ci_slot_detect_flag)
        m_ci_slot_ca_pmt_received = 0;
    else
        m_ci_slot_ca_pmt_received = 1;
    m_tsg_fack_scrambled = 0;

    LEAVE_CC_CIC_API();
}

void cc_tsg_ci_slot_start(void)
{
    if (m_ci_slot_detect_flag)
    {
        if (m_ci_descramble_ok)
        {
            CC_TSG_PRINTF("fixed to use slot %d\n", m_ci_descramble_ok_slot);
            if (m_ci_slot_current != m_ci_descramble_ok_slot)
            {
                _cc_cic_set_slot_passing(m_ci_slot_current, FALSE);
                _cc_cic_set_slot_passing(m_ci_descramble_ok_slot, TRUE);
            }
            _cc_cic_set_slot_descrambling(m_ci_descramble_ok_slot, CC_CIC_DESCRAMBLE_OK);
            m_ci_slot_detect_flag = 0;
#ifdef CC_TSG_DETECT_FAKE_SCRAMBLE
            m_ci_slot_detect_dmx1_flag = 0;
#endif
            m_ci_descramble_ok = 0;
        }
    }

}

void cc_tsg_ci_slot_pause(void)
{
    if (m_ci_slot_detect_flag == 0)
    {
        if (m_ci_slot_passing[m_ci_slot_current] && m_ci_slot_descrambling[m_ci_slot_current] == CC_CIC_DESCRAMBLE_OK)
        {
            m_ci_descramble_ok = 1;
            m_ci_descramble_ok_slot = m_ci_slot_current;
        }
    }
}

void cc_tsg_ci_slot_data_received(UINT32 pkt_num)
{
#ifdef CC_TSG_DETECT_FAKE_SCRAMBLE
    if (m_ci_slot_detect_flag || m_ci_slot_detect_dmx1_flag)
        m_tsg_dmx0_packet_num += pkt_num;
#else
    if (m_ci_slot_detect_flag)
        m_tsg_dmx0_packet_num += pkt_num;
#endif
}

void cc_tsg_ci_slot_detect(void)
{
    ENTER_CC_CIC_API();

#ifdef CC_TSG_DETECT_FAKE_SCRAMBLE
    if (m_ci_slot_detect_dmx1_flag)
    {
        if (m_tsg_dmx0_packet_num >= CC_CIC_MAX_FAKE_PKT)
        {
            if (m_cc_tsg_scramb_flag == 0)
            {
                _cc_tsg_set_fake_scramble(TRUE);
                CC_TSG_PRINTF("all slot descramble OK\n");
            }
            m_ci_slot_detect_dmx1_flag = 0;
        }
    }
#endif

    if (m_ci_slot_detect_flag)
    {
        if (m_tsg_dmx0_packet_num >= CC_CIC_MIN_DETECT_PKT)
        {
            UINT32 descram = _cc_cic_is_stream_scrambled(m_tsg_ci_dmx_dev);
            if (!descram)
            {
                _cc_cic_set_slot_descrambling(m_ci_slot_current, CC_CIC_DESCRAMBLE_OK);
                m_ci_slot_detect_flag = 0;
                //osal_flag_set(g_cc_tsg_flagid, MID27_FLG_CC_CI_DESCRAMBLE);
                CC_TSG_PRINTF("slot %d descramble OK\n", m_ci_slot_current);
            }
            else if (m_tsg_dmx0_packet_num >= CC_CIC_MAX_DETECT_PKT)
            {
                if (m_ci_slot_ca_pmt_received || (m_tsg_dmx0_packet_num >= CC_CIC_MAX_DETECT_PKT * 2))
                {
                    CC_TSG_PRINTF("slot %d descramble failed, try another\n", m_ci_slot_current);
                    _cc_cic_set_slot_descrambling(m_ci_slot_current, CC_CIC_DESCRAMBLE_FAILED);
                    if (!_cc_cic_goto_other_slot(m_ci_slot_current))
                    {
                        m_ci_slot_detect_flag = 0;
                    }
                }
            }
        }
    }
    LEAVE_CC_CIC_API();
}

BOOL cc_tsg_ci_slot_switch(BOOL b_forced)
{
    UINT32 i;
    BOOL ret = FALSE;

    ENTER_CC_CIC_API();

    if (m_ci_slot_attached[0] && m_ci_slot_attached[1])
    {
        if (m_ci_slot_passing[m_ci_slot_current])
        {
            if (m_ci_slot_descrambling[m_ci_slot_current] == CC_CIC_DESCRAMBLE_OK || b_forced)
            {
                if (m_ci_slot_descrambling[1-m_ci_slot_current] != CC_CIC_DESCRAMBLE_FAILED || m_ci_slot_detect_times[1-m_ci_slot_current] < CC_CIC_MAX_DETECT_TIME)
                {
                    _cc_cic_set_slot_passing(m_ci_slot_current, FALSE);
                    m_ci_slot_descrambling[m_ci_slot_current] = CC_CIC_DESCRAMBLE_UNKNOWN;
                    _cc_cic_set_slot_passing(1 - m_ci_slot_current, TRUE);
                    m_ci_slot_descrambling[m_ci_slot_current] = CC_CIC_DESCRAMBLE_UNKNOWN;
                    //api_send_ca_pmt_auto();
                    m_ci_slot_detect_flag = 1;
                    m_tsg_dmx0_packet_num = 0;
                    ret = TRUE;
                }
            }
        }
    }

    LEAVE_CC_CIC_API();

    return ret;
}

BOOL cc_tsg_ci_slot_select(UINT32 slot)
{
    UINT32 i;
    BOOL ret = FALSE;

    if (slot > 1)
        return FALSE;

    CC_TSG_PRINTF("selecting slot %d\n", slot);
    ENTER_CC_CIC_API();

    if (m_ci_slot_attached[0] && m_ci_slot_attached[1])
    {
        if (m_ci_slot_descrambling[slot] != CC_CIC_DESCRAMBLE_FAILED || m_ci_slot_detect_times[slot] < CC_CIC_MAX_DETECT_TIME)
        {

#ifdef CC_TSG_DETECT_FAKE_SCRAMBLE
            m_ci_slot_detect_dmx1_flag = 0;
#endif

            m_ci_slot_detect_flag = 0;
            if (slot != m_ci_slot_current)
            {
                if (m_ci_slot_passing[m_ci_slot_current])
                {
                    _cc_cic_set_slot_passing(m_ci_slot_current, FALSE);
                    m_ci_slot_descrambling[m_ci_slot_current] = CC_CIC_DESCRAMBLE_UNKNOWN;
                    _cc_cic_set_slot_passing(slot, TRUE);
                    ret = TRUE;
                }
            }
            m_ci_slot_descrambling[m_ci_slot_current] = CC_CIC_DESCRAMBLE_OK;
        }
    }

    LEAVE_CC_CIC_API();
    CC_TSG_PRINTF("selected slot %d\n", slot);

    return ret;
}

void cc_tsg_set_fake_scramble(BOOL b_fake)
{
    if (b_fake)
    {
           ENTER_CC_CIC_API();
        _cc_tsg_set_fake_scramble(TRUE);
        LEAVE_CC_CIC_API();
    }
}

BOOL cc_tsg_task_get_scramble(void)
{
    UINT32 ret = FALSE;

    ENTER_CC_CIC_API();

    UINT32 i;
    for (i = 0; i < CI_SLOT_NS; i++)
    {
        if (m_ci_slot_passing[i])
        {
            ret = TRUE;
            break;
        }
        if (m_ci_slot_descrambling[i] != CC_CIC_DESCRAMBLE_OK)
        {
            ret = TRUE;
            break;
        }
    }
    LEAVE_CC_CIC_API();

    return ret;
}

void cc_tsg_ci_slot_init()
{
    m_tsg_ci_dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);

    if (OSAL_INVALID_ID == m_cc_cic_sema_id)
    {
        m_cc_cic_sema_id = osal_semaphore_create(1);
        ASSERT(m_cc_cic_sema_id != OSAL_INVALID_ID);
    }

}

BOOL cc_tsg_ci_slot_is_detecting(void)
{
    BOOL ret;
    ENTER_CC_CIC_API();
    ret = m_ci_slot_detect_flag;
    LEAVE_CC_CIC_API();
    return ret;
}


void cc_tsg_task_pause(void)
{
    if (pvr_r_get_scramble() == 0)
    {
        if (m_tsg_task_pause_count == 0)
        {
            m_tsg_task_reset_channel = 0;
            if (cc_tsg_task_is_running())
            {
                CC_TSG_PRINTF("IO_DMX_PAUSE_PARSE, TRUE\n");
                dmx_io_control(m_tsg_ci_dmx_dev, IO_DMX_PAUSE_PARSE, TRUE);
                CC_TSG_PRINTF("CC TSG playing paused\n");
            }
        }
        m_tsg_task_pause_count++;
        return;
    }
    if (m_tsg_task_pause_count == 0)
    {
        m_tsg_task_reset_channel = 0;

        if (cc_tsg_task_is_running())
        {
            cc_tsg_ci_slot_pause();
//#ifdef CC_USE_TSG_PLAYER
            if(pvr_get_with_tsg_using_status())
                cc_tsg_task_stop0(FALSE);
            else
//#else
                cc_tsg_ci_stop_task_stop();
//#endif
            CC_TSG_PRINTF("CC TSG paused\n");
        }
    }
    m_tsg_task_pause_count++;
}

void cc_tsg_task_resume(BOOL b_start)
{
    if (m_tsg_task_pause_count == 0)
        return;

    m_tsg_task_pause_count = 0;
    CC_TSG_PRINTF("CC TSG resuming\n");

    if (b_start)
    {
        if (pvr_r_get_scramble())
        {
//#ifdef CC_USE_TSG_PLAYER
            if(pvr_get_with_tsg_using_status())
                cc_tsg_task_start(m_cc_tsg_prog_id);
            else
//#else
                cc_tsg_ci_slot_task_start(m_cc_tsg_prog_id);
//#endif
        }
        else
        {
            CC_TSG_PRINTF("IO_DMX_PAUSE_PARSE, FALSE\n");
            dmx_io_control(m_tsg_ci_dmx_dev, IO_DMX_PAUSE_PARSE, FALSE);
        }
    }
    else
    {
        if (pvr_r_get_scramble() == 0)
        {
            CC_TSG_PRINTF("IO_DMX_PAUSE_PARSE, FALSE\n");
            dmx_io_control(m_tsg_ci_dmx_dev, IO_DMX_PAUSE_PARSE, FALSE);
        }
    }
    m_tsg_task_reset_channel = 0;
}

UINT32 cc_tsg_task_get_pause_count()
{
    return m_tsg_task_pause_count;
}

void cc_tsg_task_set_pause_count(UINT32 cnt)
{
    m_tsg_task_pause_count = cnt;
}

BOOL cc_tsg_task_reset_channel(BOOL reset)
{
    //CC_TSG_PRINTF("%s\n", __FUNCTION__);

    if (m_tsg_task_pause_count == 0)
        return TRUE;

    m_tsg_task_reset_channel = reset;

    return TRUE;
}

BOOL cc_tsg_task_need_reset_channel(void)
{
    return m_tsg_task_reset_channel;
}

//#ifndef CC_USE_TSG_PLAYER
static UINT16 m_ci_slot_proc_start;

void get_dmx0_data_packet()
{
    if(!m_tsg_ci_dmx_dev)
    {
        m_tsg_ci_dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);
    }

    //set_packed_num(m_tsg_ci_dmx_dev, (UINT32)&m_tsg_dmx0_packet_num);
    dmx_io_control(m_tsg_ci_dmx_dev, IO_DMX_GET_PACKET_NUM, (UINT32)&m_tsg_dmx0_packet_num);
}

void cc_tsg_ci_slot_task_start(UINT32 prog_id)
{
    CC_TSG_PRINTF("dmx0 passing CI\n");

    tsi_dmx_src_select(TSI_DMX_0, TSI_TS_A);
    cc_tsg_ci_slot_reset();
    cc_tsg_ci_slot_start();
    if (m_cc_tsg_prog_id == prog_id)
        cc_tsg_ci_slot_set_ca_pmt_received(1);
    else
        m_cc_tsg_prog_id = prog_id;
    //CC_TSG_PRINTF("m_ci_slot_ca_pmt_received = %d\n",m_ci_slot_ca_pmt_received);
    m_ci_slot_proc_start = 1;

}

RET_CODE cc_tsg_ci_stop_task_stop(void)
{
    if(m_ci_slot_proc_start)
    {
        CC_TSG_PRINTF("dmx0 not pass CI\n");
        tsi_dmx_src_select(TSI_DMX_0, TSI_TS_B);
    }
    m_ci_slot_proc_start = 0;
}

void cc_tsg_ci_slot_task_proc()
{
    UINT32 len;
    UINT8 *buf = NULL;

    if(m_ci_slot_proc_start)
    {
        get_dmx0_data_packet();
        cc_tsg_ci_slot_detect();
    }
}

BOOL cc_tsg_task_is_running(void)
{
    if(pvr_get_with_tsg_using_status())
        return cc_tsg_task_using_tsg_is_running();
    else
        return m_ci_slot_proc_start;
}
//#endif

#else    //CI_SLOT_DYNAMIC_DETECT
extern UINT32 m_cc_tsg_prog_id;
static UINT8 m_tsg_task_reset_channel;
static UINT32 m_tsg_task_pause_count;
extern int default_route_tsi;
extern UINT8 default_tsi_polarity;

void cc_tsi_dmx_config(UINT8 tsi_dmx_x, BOOL use_tsg, BOOL pass_ci)
{
    CC_TSG_PRINTF("patch -> %s: dmx %d, tsg %d, ci %d\n",__FUNCTION__,tsi_dmx_x,use_tsg,pass_ci);
    if (use_tsg)
    {
        tsi_mode_set(TSI_SPI_TSG, default_tsi_polarity|0x80);
        tsi_select(tsi_dmx_x, TSI_SPI_TSG);
    }
    else
    {
        tsi_mode_set(default_route_tsi, default_tsi_polarity|0x80);
        tsi_select(tsi_dmx_x, default_route_tsi);
    }

    if (pass_ci)
    {
        tsi_dmx_src_select(tsi_dmx_x, TSI_TS_A);
    }
    else
        tsi_dmx_src_select(tsi_dmx_x, TSI_TS_B);

}

void cc_tsg_task_pause(void)
{
    struct dmx_device *dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);

     CC_TSG_PRINTF("patch -> %s\n",__FUNCTION__);
    if (pvr_r_get_scramble() == 0)
    {
        if (m_tsg_task_pause_count == 0)
        {
            m_tsg_task_reset_channel = 0;
            if (cc_tsg_task_is_running())
            {
                CC_TSG_PRINTF("IO_DMX_PAUSE_PARSE, TRUE\n");
                dmx_io_control(dmx_dev, IO_DMX_PAUSE_PARSE, TRUE);
                CC_TSG_PRINTF("CC TSG playing paused\n");
            }
        }
        m_tsg_task_pause_count++;
        return;
    }
    if (m_tsg_task_pause_count == 0)
    {
        m_tsg_task_reset_channel = 0;

        if (cc_tsg_task_is_running())
        {
            if(pvr_get_with_tsg_using_status())
                cc_tsg_task_stop0(FALSE);
            CC_TSG_PRINTF("CC TSG paused\n");
        }
    }
    m_tsg_task_pause_count++;
}

void cc_tsg_task_resume(BOOL b_start)
{
    struct dmx_device *dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);

    CC_TSG_PRINTF("patch -> %s: start %d\n",__FUNCTION__,b_start);
    if (m_tsg_task_pause_count == 0)
        return;

    m_tsg_task_pause_count = 0;
    CC_TSG_PRINTF("CC TSG resuming\n");

    if (b_start)
    {
        if (pvr_r_get_scramble())
        {
            if(pvr_get_with_tsg_using_status())
                cc_tsg_task_start(m_cc_tsg_prog_id);
        }
        else
        {
            CC_TSG_PRINTF("IO_DMX_PAUSE_PARSE, FALSE\n");
            dmx_io_control(dmx_dev, IO_DMX_PAUSE_PARSE, FALSE);
        }
    }
    else
    {
        if (pvr_r_get_scramble() == 0)
        {
            CC_TSG_PRINTF("IO_DMX_PAUSE_PARSE, FALSE\n");
                dmx_io_control(dmx_dev, IO_DMX_PAUSE_PARSE, FALSE);
        }
    }
    m_tsg_task_reset_channel = 0;
}

void cc_tsg_task_set_pause_count(UINT32 cnt)
{
    m_tsg_task_pause_count = cnt;
}

BOOL cc_tsg_task_reset_channel(BOOL reset)
{
    CC_TSG_PRINTF("patch -> %s: reset %d\n", __FUNCTION__,reset);

    if (m_tsg_task_pause_count == 0)
        return TRUE;

    m_tsg_task_reset_channel = reset;

    return TRUE;
}

BOOL cc_tsg_task_need_reset_channel(void)
{
    return m_tsg_task_reset_channel;
}

BOOL cc_tsg_task_is_running(void)
{
    if(pvr_get_with_tsg_using_status())
        return cc_tsg_task_using_tsg_is_running();

    return FALSE;
}

#endif

