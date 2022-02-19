/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 Copyright (C)
*
*    File:    ci_link.c
*
*    Description:    This file contains all globe micros and functions declare
*                     of CI stack - link layer.
*    History:
*               Date                Athor           Version            Reason
*        ==========     ========   =======   ==========================
*    1.    Nov.3.2004        Justin Wu       Ver 0.1           Create file.
*
*****************************************************************************/

#include <sys_config.h>
#include <types.h>
#include <retcode.h>
#include <hld/hld_dev.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/alloc.h>
#include <api/libc/string.h>
#include <hld/cic/cic.h>

#include "ci_stack.h"


//CAM_RW_DELAY * CAM_RW_TIMEOUT shall = 4s, 20100715 SmarDTV CI+ CAM
//20100715, for SmarDTV CI+ CAM Test Suite:
//Emergency SW download test case: CAM_RW_DELAY = min 20, max 40
#ifdef CI_PLUS_SUPPORT
#define CAM_RW_DELAY    40
#define CAM_RW_TIMEOUT  100
#else    //CI_SUPPORT
#define CAM_RW_DELAY    100
#define CAM_RW_TIMEOUT  40
#endif

static INT32 ci_link_read_cis(struct ci_link_layer *link, int slot);
static INT32 ci_link_config_card(struct ci_link_layer *link, int slot);

static INT32 ci_ph_read(struct cic_device *dev, int slot, UINT16 *size, UINT8 *buffer);
static INT32 ci_ph_write(struct cic_device *dev, int slot, UINT16 size, UINT8 *buffer, UINT8 reg_bit);

static INT32 parse_device_oa(struct cistpl_tuple *tuple, struct cistpl_device_o *device_o);
static INT32 parse_vers1(struct cistpl_tuple *tuple, struct cistpl_vers_1 *vers_1);
static INT32 parse_manfid(struct cistpl_tuple *tuple, struct cistpl_manfid *manfid);
static INT32 parse_config(struct cistpl_tuple *tuple, struct cistpl_config *config);
static INT32 parse_cftable_entry(struct cistpl_tuple *tuple, struct cistpl_cftable_entry *entry);

#undef le16_to_cpu
#if (SYS_CPU_ENDIAN == ENDIAN_BIG)
#define le16_to_cpu(a)        ((((a) >> 8) & 0xff) | (((a) & 0xff) << 8)))
#else
#define le16_to_cpu(a)        (a)
#endif

static const UINT8 mantissa[] = {
    10, 12, 13, 15, 20, 25, 30, 35,
    40, 45, 50, 55, 60, 70, 80, 90
};

static const UINT32 exponent[] = {
    1, 10, 100, 1000, 10000, 100000, 1000000, 10000000
};

#define abs(a)            ((a)>0 ? (a) : -(a))

/* Convert an Extended Speed Byte to a Time in Nanoseconds */
#define SPEED_CVT(v)        (mantissa[(((v) >> 3) & 15) - 1] * exponent[(v) & 7] / 10)
/* Convert a Power Byte to a Current in 0.1 Microamps */
#define POWER_CVT(v)    (mantissa[((v) >> 3) & 15] * exponent[(v) & 7] / 10)
#define POWER_SCALE(v)    (exponent[(v) & 7])

/* CAM Monitor: Cyclic Detect CAM Status */
static void ci_cam_monitor(struct ci_link_layer *link, void *dummy)
{
    struct cic_io_command_signal signal_param;
    UINT32 i = 0;

    MEMSET(&signal_param, 0x0, sizeof(struct cic_io_command_signal));
    /* Detect CAM status */
    for (i = 0; i < CI_MAX_SLOT_NUM; i++)
    {
        // Check CAM Hardware Status
        signal_param.slot = i;
        signal_param.signal = CIC_CARD_DETECT;
        cic_io_control(link->slot[i].cic_dev, CIC_DRIVER_TSIGNAL, (UINT32)&signal_param);
        // Status Change
        if (link->slot[i].cam_present_status != signal_param.status)
        {
            // New Status
            link->slot[i].cam_present_status = signal_param.status;
            // Notify CI Stack of CAM Status Change
            api_ci_msg_to_stack(i, API_MSG_CAMHW);
        }
    }
    // Set Timer for Cyclic Detect CAM
    ci_set_timer(CI_DURATION_CAM_MONITOR, (ci_timer_handler)ci_cam_monitor, link, NULL);
}

/* Link Layer Attach */
INT32 ci_link_attach(struct ci_link_layer *link)
{
    UINT32 i = 0;
    struct cic_device *cic_dev = NULL;

    // Open All CI Devices
    if ((cic_dev = (struct cic_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_CIC)) == NULL)
    {
        LINK_PRINTF("ci_link_attach: Can't find CI device!\n");
        return ERR_NO_DEV;
    }
    if (cic_open(cic_dev, api_ci_cam_to_stack) != SUCCESS)
    {
        LINK_PRINTF("ci_link_attach: Open CI device failue!\n");
        ASSERT(0);
        return ERR_FAILUE;
    }

    // Initialize All Slot of CI Link Layer
    for (i = 0; i < CI_MAX_SLOT_NUM; i++)
    {
        link->slot[i].cic_dev = cic_dev;
        link->slot[i].state = CAM_DETACH;
        link->slot[i].cam_present_status = 0;
        link->slot[i].ca_pmt_cnt = 0;
        /* Initialize Flag, and Clear it */
        if ((link->slot[i].flag_id = osal_flag_create(0)) == OSAL_INVALID_ID)
        {
            /* Close All CI Devices */
            for (i = 0; i < CI_MAX_SLOT_NUM; i++)
            {
                if (link->slot[i].cic_dev != NULL)
                {
                    cic_close(link->slot[i].cic_dev);
                    link->slot[i].cic_dev = NULL;
                }
            }
            return ERR_FAILUE;
        }
    }

    // Set Timer for Cyclic Detect CAM
    ci_set_timer(CI_DURATION_CAM_MONITOR, (ci_timer_handler)ci_cam_monitor, link, NULL);

    return SUCCESS;
}

/* Link Layer Detach */
INT32 ci_link_detach(struct ci_link_layer *link)
{
    int i;

    /* Close All CI Devices */
     for (i = 0; i < CI_MAX_SLOT_NUM; i++)
    {
        // Close CI Device
        if (link->slot[i].cic_dev != NULL)
        {
            cic_close(link->slot[i].cic_dev);
            link->slot[i].cic_dev = NULL;
        }
        // Delete Flag
        if (link->slot[i].flag_id != OSAL_INVALID_ID)
        {
            osal_flag_delete(link->slot[i].flag_id);
            link->slot[i].flag_id = OSAL_INVALID_ID;
        }
    }

    return SUCCESS;
}

/* Check Whether CAM Pull Out Or Not */
static INT32 ci_link_cam_pullout(struct cic_device *dev, int slot, int reset, int timeout)
{
    struct cic_io_command_signal signal_param;
    static UINT8 signal[CI_MAX_SLOT_NUM], signal_cnt[CI_MAX_SLOT_NUM];

       MEMSET(&signal_param, 0, sizeof(struct cic_io_command_signal));
    /* Detect CAM status */
    signal_param.slot = slot;
    signal_param.signal = CIC_CARD_DETECT;
    cic_io_control(dev, CIC_DRIVER_TSIGNAL, (UINT32)&signal_param);

    // If Reset, then Get New CAM Status, Or Keep Old CAM Status
    if (reset)
    {
        signal[slot] = signal_param.status;
        signal_cnt[slot] = 0;
    }

    // Check CAM Status Change Or Not
    if (signal[slot] != signal_param.status)    /* Status Change, Get New Status, Reset Counter */
    {
        signal_cnt[slot] = 0;
        signal[slot] = signal_param.status;
    }
    else                                    /* Status Not Change, Keep Old Status, Increment Counter*/
    {
        signal_cnt[slot]++;
    }

    // Check Counter to Determine Whether CAM Pull Out Or Not
    if (signal[slot] == 0 && signal_cnt[slot] >= timeout)
    {
        LINK_PRINTF("ci_link_cam_pullout:[slot %d] CAM out!\n", slot);
        return SUCCESS;
    }

    return ERR_FAILUE;
}

/*
 *  @Pass CAM
 */

extern UINT8 slot_status;
extern UINT8 nim_ts_type[];

void ci_link_cam_pass(struct ci_link_layer *link, int slot, UINT8 pass)
{
    struct cic_io_command_signal signal_param;
    struct ci_slot *tp = &(link->slot[slot]);

    LINK_PRINTF("%s: set stream pass CAM\n", __FUNCTION__);

    signal_param.slot = slot;
    signal_param.signal = CIC_EMSTREAM;
    signal_param.status = pass;    /* Pass CAM */
    cic_io_control(tp->cic_dev, CIC_DRIVER_SSIGNAL, (UINT32)&signal_param);

    if (pass)
    {
        slot_status |= 1 << slot;
        nim_ts_type[slot] = 3;
    }
    else
    {
        slot_status &= (~(1 << slot));
        nim_ts_type[slot] = 2;
    }
}


/* For Support Multi-Thread like function in One Task to Enhance the Stack
   Performance, we Use Timer Based FSM. */
/* Detect CAM Pull Out or Plug In */
INT32 ci_link_cam_detect(struct ci_link_layer *link, int slot)
{
    struct ci_slot *tp;
    struct cic_io_command_signal signal_param;
    UINT16 state;

       MEMSET(&signal_param, 0, sizeof(struct cic_io_command_signal));
    state = (slot >> 8);        /* Bit[31:8] is State Index */
    slot &= 0xff;                /* Bit[7:0] is Real Slot Index */
    tp = &(link->slot[slot]);

    if (state < 3)
    {
        // Check CAM Status
        signal_param.slot = slot;
        signal_param.signal = CIC_CARD_DETECT;
        cic_io_control(tp->cic_dev, CIC_DRIVER_TSIGNAL, (UINT32)&signal_param);

        // Check Whether CAM Status Change Or Not
        if (link->slot[slot].cam_present_status == signal_param.status)    /* Status Not Change*/
        {
            state++;
        }
        else                                                        /* Status Change */
        {
            link->slot[slot].cam_present_status = signal_param.status;
            state = 0;
        }

        // Set Timer for Cyclic Detect CAM
        ci_set_timer(100, (ci_timer_handler)ci_link_cam_detect, (void *)link, (void *)((state << 8) | slot));
        return SUCCESS;
    }

    switch (link->slot[slot].cam_present_status)
    {
        case 0:        /* CAM Pull Out */
            LINK_PRINTF("ci_link_cam_detect:[slot %d] CAM pull out\n", slot);
            switch (link->slot[slot].state)
            {
            case CAM_DETACH:
            case CAM_STACK_DETACH:
                /* Do Noting */
                break;
            case CAM_ATTACH:
            case CAM_STACK_ATTACH:
                link->slot[slot].state = CAM_STACK_DETACH;
                api_ci_msg_to_stack(slot, API_MSG_CAMOUT);    /* Notify Stack of CAM Out */
                api_ci_msg_to_app(slot, API_MSG_CAMOUT);    /* Notify App of CAM Out */
                LINK_PRINTF("ci_link_cam_detect:[slot %d] CAM pull out send msg done\n", slot);
                break;
            }
            break;
        case 3:        /* CAM Plug In */
            LINK_PRINTF("ci_link_cam_detect:[slot %d] CAM plug in\n", slot);
            switch (link->slot[slot].state)
            {
            case CAM_DETACH:
                link->slot[slot].state = CAM_ATTACH;

                api_ci_msg_to_stack(slot, API_MSG_CAMIN);    /* Notify Stack of CAM In */
                api_ci_msg_to_app(slot, API_MSG_CAMIN);        /* Notify App of CAM In */
                LINK_PRINTF("ci_link_cam_detect:[slot %d] CAM plug in send msg done\n", slot);
                break;
            case CAM_STACK_DETACH:
            case CAM_ATTACH:
            case CAM_STACK_ATTACH:
                /* Do Noting */
                break;
            }
            break;
        default :
            LINK_PRINTF("ci_link_cam_detect:[slot %d] CD1/CD2 issue! Do noting!\n", slot);
            break;
    }

    return SUCCESS;
}

/* For support multi-thread like function in one task to enhance the stack
   performance, we use timer based FSM. */
/* CAM Attach */
INT32 ci_link_cam_attach(struct ci_link_layer *link, int slot)
{
    struct ci_slot *tp;
    struct cic_io_command_signal signal_param;
    UINT16 state;
    /* CAM Card Reset Delay */
    /* fix BUG11552: AlphaCryptLight Need More Gap */
    /* For Some Cards, 100ms maybe Not Enough, so Need Set cam_reset_delay
       to be 2000ms */
    static INT16 reset_delay = 100;

    state = (slot >> 8);        /* Bit[31:8] is State Index */
    slot &= 0xff;                /* Bit[7:0] is Real Slot Index */
    tp = &(link->slot[slot]);

    // Select Slot
    signal_param.slot = slot;
    signal_param.signal = CIC_SLOTSEL;
    signal_param.status = 1;
    cic_io_control(tp->cic_dev, CIC_DRIVER_SSIGNAL, (UINT32)&signal_param);

    // Check CAM Pull Out
    if (ci_link_cam_pullout(tp->cic_dev, slot, 1, 1) == SUCCESS)
    {
        state = 1000;        /* Flag to Exit State Match */
    }

    if (state == 0)
    {
        // Clear Reset Slot
        signal_param.signal = CIC_RSTSLOT;
        signal_param.status = 0;
        cic_io_control(tp->cic_dev, CIC_DRIVER_SSIGNAL, (UINT32)&signal_param);
        // Enable Slot
        signal_param.signal = CIC_ENSLOT;
        signal_param.status = 1;
        cic_io_control(tp->cic_dev, CIC_DRIVER_SSIGNAL, (UINT32)&signal_param);
        // Switch to Attribut Memory Space
        signal_param.signal = CIC_IOMEM;
        signal_param.status = 0;
        cic_io_control(tp->cic_dev, CIC_DRIVER_SSIGNAL, (UINT32)&signal_param);
        state++;
        /* Keep 300ms, after that, CAM Attach Again */
        ci_set_timer(300, (ci_timer_handler)ci_link_cam_attach, (void *)link, (void*)((state << 8) | slot));
        return SUCCESS;
    }
    else if (state == 1)
    {
        // Reset Slot
        signal_param.signal = CIC_RSTSLOT;
        signal_param.status = 1;    //1: reset
        cic_io_control(tp->cic_dev, CIC_DRIVER_SSIGNAL, (UINT32)&signal_param);
        state++;
        /* Keep >15ms. (3nd party design: 100mS, Digisat sugest 200ms for ZetaCAM) After that, CAM Attach Again */
        ci_set_timer(200, (ci_timer_handler)ci_link_cam_attach, (void *)link, (void*)((state << 8) | slot));
        return SUCCESS;
    }
    else if (state == 2)
    {
        // Clear Reset Slot
        signal_param.signal = CIC_RSTSLOT;
        signal_param.status = 0;
        cic_io_control(tp->cic_dev, CIC_DRIVER_SSIGNAL, (UINT32)&signal_param);
        state++;
        /* Keep >20ms, after that, CAM Attach Again */
        ci_set_timer(reset_delay, (ci_timer_handler)ci_link_cam_attach, (void *)link, (void*)((state << 8) | slot));
        return SUCCESS;
    }
    else if (state > 2 && state < 50)
    {
        // Check CAM Card Ready Or Not
        signal_param.signal = CIC_CARD_READY;
        cic_io_control(tp->cic_dev, CIC_DRIVER_TSIGNAL, (UINT32)&signal_param);

        if (signal_param.status)        /* CAM Card Ready */
        {
            /* Default Reset Delay Value */
            reset_delay = 100;

            /* Read CIS. If Some Error then Return. */
            if (ci_link_read_cis(link, slot) == SUCCESS)
            {
                /* Config Card. If isn't DVB CI Card then Return. */
                if (ci_link_config_card(link, slot) == SUCCESS)
                {
                    // Switch to IO Space
                    signal_param.signal = CIC_IOMEM;
                    signal_param.status = 1;
                    cic_io_control(tp->cic_dev, CIC_DRIVER_SSIGNAL, (UINT32)&signal_param);

                    // Notify Stack of Buffer Size Negotiation
                    api_ci_msg_to_stack(slot, API_MSG_NEGO);

                    return SUCCESS;
                }
                else
                {
                    LINK_PRINTF("ci_link_cam_attach:[slot %d] This card isn't DVB CI card!\n", slot);
                }
            }
            else
            {
                LINK_PRINTF("ci_link_cam_attach:[slot %d] Read CIS error!\n", slot);
            }
            state = 1000;        /* Flag to Exit State Match */
        }
        if (state < 50)
        {
            state++;
            /* After Some Time, CAM Attach Again */
            ci_set_timer(100, (ci_timer_handler)ci_link_cam_attach, (void *)link, (void *)((state << 8) | slot));
            return SUCCESS;
        }
        else
        {
            /* fix BUG11552: AlphaCryptLight Need More Gap */
            /* for Some Cards, 100ms maybe Not Enough, so Need Set cam_reset_delay
               to be 2000ms */
            if (reset_delay == 100)
            {
                reset_delay = 2000;
            }
            else
            {
                reset_delay = 100;
            }
            LINK_PRINTF("ci_link_cam_attach:[slot %d] reset delay = %d!\n", slot,reset_delay);

            LINK_PRINTF("ci_link_cam_attach:[slot %d] wait CIC_CARD_READY timeoutr!\n", slot);
        }
    }

    link->slot[slot].state = CAM_STACK_DETACH;
    api_ci_msg_to_stack(slot, API_MSG_CAMOUT);    /* Notify Stack of CAM Out */
    api_ci_msg_to_app(slot, API_MSG_CAMOUT);    /* Notify App of CAM Out */

    return ERR_FAILUE;
}

/* CAM Detach */
INT32 ci_link_cam_detach(struct ci_link_layer *link, int slot)
{
    struct ci_slot *tp = &(link->slot[slot]);
    struct cic_io_command_signal signal_param;

    // Stream Bypass CI
    signal_param.slot = slot;
    signal_param.signal = CIC_EMSTREAM;
    signal_param.status = 0;
    cic_io_control(tp->cic_dev, CIC_DRIVER_SSIGNAL, (UINT32)&signal_param);
    // Switch to Attribut Memory Space
    signal_param.signal = CIC_IOMEM;
    signal_param.status = 0;
    cic_io_control(tp->cic_dev, CIC_DRIVER_SSIGNAL, (UINT32)&signal_param);
    // Disable Slot
    signal_param.signal = CIC_ENSLOT;
    signal_param.status = 0;
    cic_io_control(tp->cic_dev, CIC_DRIVER_SSIGNAL, (UINT32)&signal_param);

    return SUCCESS;
}

/* Send LPDU to CAM */
INT32 ci_link_send_data(struct ci_link_layer *link, int slot, int len, UINT8 *data)
{
    struct ci_slot *tp = &(link->slot[slot]);
    int l;
    struct cic_io_command_signal    signal_param;

    // Select Slot
    signal_param.slot = slot;
    signal_param.signal = CIC_SLOTSEL;
    signal_param.status = 1;
    cic_io_control(tp->cic_dev, CIC_DRIVER_SSIGNAL, (UINT32)&signal_param);

    len -= 2;                                /* 2 Bytes Information Head (SlotID & TCID) */
    tp->buffer[0] = data[1];                    /* LPDU TC_ID */
    data += 2;
    while (len > 0)
    {
        l = len;
        tp->buffer[1] = 0;                        /* LPDU L Flag */
        if (l > tp->buffer_size - 2)
        {
            tp->buffer[1] = CI_LINK_M_FLAG;    /* LPDU M Flag */
            l = tp->buffer_size - 2;
        }
        MEMCPY(&(tp->buffer[2]), data, l);        /* LPDU Data */

        // Write LPDU to Data Buffer 
        if (ci_ph_write(tp->cic_dev, slot, l + 2, tp->buffer, 0) != SUCCESS)
        {
                   break;
            }
        len -= l;
        data += l;
    }

    return len != 0 ? ERR_FAILUE : SUCCESS;
}

/* Receive LPDU from CAM */
INT32 ci_link_recv_data(struct ci_link_layer *link, int slot, int *len, UINT8 *data)
{
    struct ci_slot *tp = &(link->slot[slot]);
    UINT8 *p, ml = CI_LINK_M_FLAG;
    UINT16 l;
    struct cic_io_command_signal signal_param;

    // Select Slot
    signal_param.slot = slot;
    signal_param.signal = CIC_SLOTSEL;
    signal_param.status = 1;
    cic_io_control(tp->cic_dev, CIC_DRIVER_SSIGNAL, (UINT32)&signal_param);

    *len = 2;
    data[0] = slot;                            /* Link Slot */
    p = data + 2;
     while (ml)
     {
         // Read LPDU from Data Buffer
         l = tp->buffer_size;
        if (ci_ph_read(tp->cic_dev, slot, &l, tp->buffer) != SUCCESS)
        {
            return ERR_FAILUE;
        }
         data[1] = tp->buffer[0];                /* Link TC_ID */
         ml = (tp->buffer[1] & CI_LINK_M_FLAG);    /* LPDU M/L flag */
         if (l >= 2)
         {
             l -= 2;
            *len += l;
            if (*len > CI_MAX_TPDU_SIZE)
            {
                LINK_PRINTF("ci_link_recv_data:[slot %d] CAM data %d too long!\n", slot,*len);
                return ERR_FAILUE;
            }
            MEMCPY(p, tp->buffer + 2, l);        /* TPDU Data */
            p += l;
        }
        else
        {
            LINK_PRINTF("ci_link_recv_data:[slot %d] CAM error!\n", slot);
            return ERR_FAILUE;
        }
    };

    return *len == 0 ? ERR_FAILUE : SUCCESS;
}

#if CIPLUS_DEBUG_INFO_LINK
static void dump_cis(struct ci_link_layer *link)
{
    struct ci_slot *tp = &(link->slot[0]);
    struct cistpl_device_o        *device = &(tp->device_infor);
    //struct cistpl_vers_1        *version = &(tp->version_infor);
    //struct cistpl_manfid        *manid = &(tp->manid_infor);
    //struct cistpl_config        *config = &(tp->config_infor);
    struct cistpl_cftable_entry    *cfgtable = &(tp->cfgtable_infor);
    int i;

    /* Dump device OA/OC configuration */
    LINK_PRINTF("ci_link_config_card: Device OA/OC information...\n");
    LINK_PRINTF("\t\t mwait: %0x\n", device->mwait);
    LINK_PRINTF("\t\t Vcc: %0x\n", device->vcc_used);
    for (i = 0; i < device->device.ndev; i++)
    {
        LINK_PRINTF("\t\t Device %d: type: %0x\n", i, device->device.dev[i].type);
        LINK_PRINTF("\t\t Device %d: wp: %0x\n", i, device->device.dev[i].wp);
        LINK_PRINTF("\t\t Device %d: speed: %0x\n", i, device->device.dev[i].speed);
        LINK_PRINTF("\t\t Device %d: size: %0x\n", i, device->device.dev[i].size);
    }

    /* Dump power configuration */
    if (cfgtable->vcc.present != 0)
    {
        LINK_PRINTF("ci_link_config_card: VCC power configuration...\n");
        LINK_PRINTF("\t\t Nom V: %d\n", cfgtable->vcc.param[0]);
        LINK_PRINTF("\t\t Min V: %d\n", cfgtable->vcc.param[1]);
        LINK_PRINTF("\t\t Max V: %d\n", cfgtable->vcc.param[2]);
        LINK_PRINTF("\t\t Sta I: %d\n", cfgtable->vcc.param[3]);
        LINK_PRINTF("\t\t Avg I: %d\n", cfgtable->vcc.param[4]);
        LINK_PRINTF("\t\t Pea I: %d\n", cfgtable->vcc.param[5]);
        LINK_PRINTF("\t\t Pdw I: %d\n", cfgtable->vcc.param[6]);
    }
    if (cfgtable->vpp1.present != 0)
    {
        LINK_PRINTF("ci_link_config_card: Vpp1 power configuration...\n");
        LINK_PRINTF("\t\t Nom V: %d\n", cfgtable->vcc.param[0]);
        LINK_PRINTF("\t\t Min V: %d\n", cfgtable->vcc.param[1]);
        LINK_PRINTF("\t\t Max V: %d\n", cfgtable->vcc.param[2]);
        LINK_PRINTF("\t\t Sta I: %d\n", cfgtable->vcc.param[3]);
        LINK_PRINTF("\t\t Avg I: %d\n", cfgtable->vcc.param[4]);
        LINK_PRINTF("\t\t Pea I: %d\n", cfgtable->vcc.param[5]);
        LINK_PRINTF("\t\t Pdw I: %d\n", cfgtable->vcc.param[6]);
    }
    if (cfgtable->vpp2.present != 0)
    {
        LINK_PRINTF("ci_link_config_card: Vpp2 power configuration...\n");
        LINK_PRINTF("\t\t Nom V: %d\n", cfgtable->vcc.param[0]);
        LINK_PRINTF("\t\t Min V: %d\n", cfgtable->vcc.param[1]);
        LINK_PRINTF("\t\t Max V: %d\n", cfgtable->vcc.param[2]);
        LINK_PRINTF("\t\t Sta I: %d\n", cfgtable->vcc.param[3]);
        LINK_PRINTF("\t\t Avg I: %d\n", cfgtable->vcc.param[4]);
        LINK_PRINTF("\t\t Pea I: %d\n", cfgtable->vcc.param[5]);
        LINK_PRINTF("\t\t Pdw I: %d\n", cfgtable->vcc.param[6]);
    }
    /* Dump timming configuration */
    LINK_PRINTF("ci_link_config_card: Timing configuration...\n");
    LINK_PRINTF("\t\t Wait %d, WS %d\n",  cfgtable->timing.wait, cfgtable->timing.waitscale);
    LINK_PRINTF("\t\t Redy %d, RS %d\n",  cfgtable->timing.ready, cfgtable->timing.rdyscale);
    LINK_PRINTF("\t\t Rsvd %d, RS %d\n",  cfgtable->timing.reserved, cfgtable->timing.rsvscale);
    /* Dump IO configuration */
    LINK_PRINTF("ci_link_config_card: IO configuration...\n");
    for (i = 0; i < cfgtable->io.nwin; i++)
    {
        LINK_PRINTF("\t\t Base 0x%08x, Len 0x%08x\n", cfgtable->io.win[i].base,cfgtable->io.win[i].len);
    }
    /* Dump IRQ configuration */
    LINK_PRINTF("ci_link_config_card: IRQ configuration...\n");
    LINK_PRINTF("\t\t IRQInfo1 0x%08x, IRQInfo2 0x%08x\n",  cfgtable->irq.irqinfo1, cfgtable->irq.irqinfo2);
    /* Dump MEM configuration */
    LINK_PRINTF("ci_link_config_card: MEM configuration...\n");
    for (i = 0; i < cfgtable->mem.nwin; i++)
    {
        LINK_PRINTF("\t\t Len 0x%08x, cadd 0x%08x, hadd 0x%08x\n", cfgtable->mem.win[i].len,cfgtable->mem.win[i].card_addr,cfgtable->mem.win[i].host_addr);
    }
}
#endif
/*
 *     Name        :       ali_tolower()
 *    Description    :       Convert str to low case for comparation.
 *    Parameter    :    str_src, str_len
 *    Return        :    str_dst, ret value
 */
static UINT8 ali_tolower(char *str_dst, char *str_src, UINT32 str_len)
{
    UINT32 i = 0;
    if((NULL == str_dst) || (NULL == str_src) || (0 == str_len))
        return 1;

    if(((UINT32)abs(str_dst - str_src)) < str_len)
    {
        // TO DO .....
    }

    for(i = 0;i < str_len;i++)
    {
        if(str_src[i] >= 'A' && str_src[i] <= 'Z')
            str_dst[i] = str_src[i] + 0x20;
        else
            str_dst[i] = str_src[i];
    }

    return 0;
}

/*
 *     Name        :       ci_link_read_cis()
 *    Description    :       M3327 CAM Card Information Structure process.
 *    Parameter    :    struct ci_link_layer *link,    int slot
 *    Return        :    INT32                : Return value
 */
static INT32 ci_link_read_cis(struct ci_link_layer *link, int slot)
{
    struct ci_slot *tp = &(link->slot[slot]);
    struct cic_io_command_memrw     memrw_param;
    struct cistpl_tuple tuple;
    UINT8  buf[255] = {0};
    UINT32 cis_addr = 0;
    int  i;

    char *str_dst = NULL;
    UINT8 str_conv = -1;

    MEMSET(&memrw_param, 0, sizeof(struct cic_io_command_memrw));
    MEMSET(&tuple, 0, sizeof(struct cistpl_tuple));
    /* Clear All Old CIS Informations */
    MEMSET(&tp->device_infor, 0, sizeof(struct cistpl_device_o));
    MEMSET(&tp->version_infor, 0, sizeof(struct cistpl_vers_1));
    MEMSET(&tp->manid_infor, 0, sizeof(struct cistpl_manfid));
    MEMSET(&tp->config_infor, 0, sizeof(struct cistpl_config));
    MEMSET(&tp->cfgtable_infor, 0, sizeof(struct cistpl_cftable_entry));

    tuple.tuple_data = buf;
    tuple.tuple_data_max = sizeof(buf);

    /* From CI Standard, Only a Few Metaformat (Tuples) are MUST:
     * CISTPL_DEVICE_OA/CISTPL_DEVICE_OC: For Device Information (VCC/Timing);
     * CISTPL_VERS_1: For Card Version and Name String;
     * CISTPL_MANFID: For Manufactory ID and Revision;
     * CISTPL_CONFIG: For Configuration Register;
     * CISTPL_CFTABLE_ENTRY: For Card HW Configuration and Information;
     *        STCE_EV: Environment Description String;
     *        STCE_PD: Physical Device Name String;
     * CISTPL_END: End Chain Tuple.
     */

    memrw_param.slot = slot;
    do
    {
        /* Test CAM Card Ready or Not */
        if (ci_link_cam_pullout(tp->cic_dev, slot, 1, 1) == SUCCESS)
        {
            return ERR_FAILUE;
        }
        /* Read Tuple Code */
        memrw_param.addr = cis_addr;
        memrw_param.size = 1;
        memrw_param.buffer = &(tuple.tuple_code);
        if (cic_io_control(tp->cic_dev, CIC_DRIVER_READMEM, (UINT32)&memrw_param) != SUCCESS)
        {
            return ERR_FAILUE;
        }
        /* If it is End of Tuple Chain, then Return */
        if (tuple.tuple_code == CISTPL_END)
        {
            return SUCCESS;
        }
        /* Read Tuple Length */
        memrw_param.addr = ++cis_addr;
        memrw_param.size = 1;
        memrw_param.buffer = &(tuple.tuple_link);
        if (cic_io_control(tp->cic_dev, CIC_DRIVER_READMEM, (UINT32)&memrw_param) != SUCCESS)
        {
            return ERR_FAILUE;
        }
        tuple.tuple_data_len = tuple.tuple_link;
        if (tuple.tuple_data_len > tuple.tuple_data_max)
        {
            LINK_PRINTF("ci_link_read_cis:[slot %d] CAM too long tuple\n", slot);
            return ERR_FAILUE;
        }
        /* Read Tuple Data Out to Buffer */
        memrw_param.addr = ++cis_addr;
        memrw_param.size = tuple.tuple_data_len;
        memrw_param.buffer = tuple.tuple_data;
        if (cic_io_control(tp->cic_dev, CIC_DRIVER_READMEM, (UINT32)&memrw_param) != SUCCESS)
        {
            return ERR_FAILUE;
        }
        cis_addr += tuple.tuple_data_len;

        switch (tuple.tuple_code)
        {
        case CISTPL_DEVICE_OA :
        case CISTPL_DEVICE_OC :
            LINK_PRINTF("ci_link_read_cis: Found a CISTPL_DEVICE_OA/OC=0x%02x\n", tuple.tuple_code);
            parse_device_oa(&tuple, &tp->device_infor);
            break;
        case CISTPL_VERS_1 :
            LINK_PRINTF("ci_link_read_cis: Found a CISTPL_VERS_1=0x%02x\n", tuple.tuple_code);
            parse_vers1(&tuple, &tp->version_infor);

            str_dst = NULL;
            if(tuple.tuple_data_len)
                str_dst = (char *)MALLOC(254); // Same as version info str len

            if(NULL != str_dst)
                str_conv = ali_tolower(str_dst, tp->version_infor.str, tuple.tuple_data_len);

            if(0 == str_conv)
            {
                for(i=0; i < tuple.tuple_data_len - 8; i++)
                {
                    if(0 == ali_memcmp(str_dst + i, "ciplus=1", 8))
                    {
                        link->slot[slot].version_infor.compatible = 1;
                        LINK_PRINTF("ci_link_read_cis:[slot %d] CICAM compatible --> ciplus=1\n", slot);
                        break;
                    }
                }
            }

            if (str_dst != NULL) {
                FREE(str_dst);
                str_dst = NULL;
            }

            UINT8 DTV_CAM[] = {0x53, 0x6D, 0x61, 0x72, 0x44, 0x54, 0x56, 0x00, 0x44, 0x56, 0x42, 0x20, 0x43, 0x41, 0x20, 0x4D,
                                0x6F, 0x64, 0x75, 0x6C, 0x65};    //SmarDTV.DVB CA Module
            //20100803, patch for DTV CI+ Test CAM
            if(0 == ali_memcmp(tp->version_infor.str, DTV_CAM, 21))
            {
                link->slot[slot].version_infor.compatible = 1;
                LINK_PRINTF("ci_link_read_cis:[slot %d] DTV CICAM --> ciplus=1\n", slot);
                break;
            }
            break;
        case CISTPL_MANFID :
            LINK_PRINTF("ci_link_read_cis: Found a CISTPL_MANFID=0x%02x\n", tuple.tuple_code);
            parse_manfid(&tuple, &tp->manid_infor);
            break;
        case CISTPL_CONFIG :
            LINK_PRINTF("ci_link_read_cis: Found a CISTPL_CONFIG=0x%02x\n", tuple.tuple_code);
            parse_config(&tuple, &tp->config_infor);
            break;
        case CISTPL_CFTABLE_ENTRY:
            LINK_PRINTF("ci_link_read_cis: Found a CISTPL_CFTABLE_ENTRY=0x%02x\n", tuple.tuple_code);
            if (tuple.tuple_data_len > 2 + 11 + 17)        /* Magic Number (???) */
            {
                parse_cftable_entry(&tuple, &tp->cfgtable_infor);
            }
            break;
        case CISTPL_END:
            LINK_PRINTF("ci_link_read_cis: Found a CISTPL_END=0x%02x\n", tuple.tuple_code);
            return SUCCESS;
        case CISTPL_NO_LINK:
            LINK_PRINTF("ci_link_read_cis: Found a CISTPL_NO_LINK=0x%02x\n", tuple.tuple_code);
            break;
        default:
            LINK_PRINTF("ci_link_read_cis: Found a un-process tuple=0x%02x\n", tuple.tuple_code);
            break;
        }
        /* Dump Tuple */
        LINK_PRINTF("Tuple Code: %02x, Tuple Link: %02x\n", tuple.tuple_code, tuple.tuple_link);
        for (i = 0; i < tuple.tuple_data_len; i++)
        {
            LINK_PRINTF("%02x ", tuple.tuple_data[i]);
        }
        LINK_PRINTF("\n\n");
    } while (1);

    return SUCCESS;
}

/*
 *     Name        :       ci_link_config_card()
 *    Description    :       M3327 CAM Card configurat process.
 *    Parameter    :    struct ci_slot *slot    : CIC device
 *    Return        :    INT32                    : Return value
 */
static INT32 ci_link_config_card(struct ci_link_layer *link, int slot)
{
    struct ci_slot *tp = &(link->slot[slot]);
    struct cic_io_command_memrw     memrw_param;
    //struct cistpl_device_o        *device = &(tp->device_infor);
    struct cistpl_vers_1        *version = &(tp->version_infor);
    struct cistpl_manfid        *manid = &(tp->manid_infor);
    struct cistpl_config        *config = &(tp->config_infor);
    struct cistpl_cftable_entry    *cfgtable = &(tp->cfgtable_infor);
    int i;
    UINT8  tmp = 0;

    MEMSET(&memrw_param, 0, sizeof(struct cic_io_command_memrw));
#if CIPLUS_DEBUG_INFO_LINK
    dump_cis(link);
#endif

    /* Is DVB CI Card? If not, then Return with Error */
    switch (config->cif_infor.ifn)
    {
    case IFN_DVBCI:
        LINK_PRINTF("ci_link_config_card:[slot %d] CAM This is a DVB CI card!\n", slot);
        break;
    case IFN_OPENCABLEPOD:
        LINK_PRINTF("ci_link_config_card:[slot %d] CAM This is a OpenCable POD card!\n", slot);
        return ERR_FAILUE;
    case IFN_ZOOMVIDEO:
        LINK_PRINTF("ci_link_config_card:[slot %d] CAM This is a Zoom Video card!\n", slot);
        return ERR_FAILUE;
    default:
        LINK_PRINTF("ci_link_config_card:[slot %d] CAM This is a unknow card!\n", slot);
        return ERR_FAILUE;
    }

    for (i = 0; i < config->cif_infor.ns; i++)
    {
        LINK_PRINTF("ci_link_config_card:[slot %d] CAM Inferface[%d]:%s\n", slot, i,
          &(config->cif_infor.str[config->cif_infor.ofs[i]]));
    }

    /* DVB CI PC Card Version should be 5.0. If not, then Return with Error */
    if (version->major != 5 || version->minor != 0)
    {
        LINK_PRINTF("ci_link_config_card:[slot %d] CAM card version error!\n", slot);
        return ERR_FAILUE;
    }
    for (i = 0; i < version->ns; i++)
    {
        LINK_PRINTF("ci_link_config_card:[slot %d] CAM version[%d]:%s\n", slot, i,
          &(version->str[version->ofs[i]]));
    }

    if((manid->manf) || (manid->card))
    {
        LINK_PRINTF("ci_link_config_card:[slot %d] CAM Manufacturer ID: 0x%02x, Card S/N: 0x%02x\n",
                slot, manid->manf, manid->card);
    }

    /* Setup PCMCIA Configuration Registers */
    if (config->base > 0x0fee)
    {
        LINK_PRINTF("ci_link_config_card: PCMCIA config register base invailid!\n");
        return ERR_FAILUE;
    }
    LINK_PRINTF("ci_link_config_card:[slot %d] CI config register address is 0x%04x\n", slot, config->base);

    /* Cfg Optoin Register: Enable the CAM Function */
    if (config->rmask[0] & 1)
    {
        memrw_param.slot = slot;
        memrw_param.addr = (config->base >> 1);
        memrw_param.size = 1;
        memrw_param.buffer = &tmp;
        /* Get Configuration Register Value */
        if (cic_io_control(tp->cic_dev, CIC_DRIVER_READMEM, (UINT32)&memrw_param) != SUCCESS)
        {
            return ERR_FAILUE;
        }
        /* Enable CAM */
        LINK_PRINTF("ci_link_config_card:[slot %d] config index value 0x%04x\n", slot, cfgtable->index);
        LINK_PRINTF("ci_link_config_card:[slot %d] config register value 0x%04x\n", slot, tmp);
        /* CAM Cards Support List:
         * ATsky CAM (9M), NagraVision, Irdeto (SCM CAM), Viaccess (SCM CAM).
         */
        tmp = (cfgtable->index | (tmp & 0x3f));
        memrw_param.buffer = &tmp;
        if (cic_io_control(tp->cic_dev, CIC_DRIVER_WRITEMEM, (UINT32)&memrw_param) != SUCCESS)
        {
            return ERR_FAILUE;
        }
        LINK_PRINTF("ci_link_config_card:[slot %d] config register setup 0x%04x\n", slot, tmp);
    }

    return SUCCESS;
}

/* For Support Multi-Thread like Function in One Task to Enhance the Stack
   Performance, we Use Timer based FSM */
/* Buffer Size Negotiation with CAM Card Process in Link Layer according to CI Spec. */
INT32 ci_link_negotiation(struct ci_link_layer *link, int slot)
{
    struct ci_slot *tp;
    struct cic_io_command_signal     signal_param;
    struct cic_io_command_iorw     iorw_param;
    UINT8     data[2] = {0};
    UINT16     bsize;
    UINT16     state;

    state = (slot >> 8);        /* Bit[31:8] is State Index */
    slot &= 0xff;                /* Bit[7:0] is Real Slot Index */
    tp = &(link->slot[slot]);

    // Select Slot
    signal_param.slot = slot;
    signal_param.signal = CIC_SLOTSEL;
    signal_param.status = 1;
    cic_io_control(tp->cic_dev, CIC_DRIVER_SSIGNAL, (UINT32)&signal_param);

    // Set I/O Param
    iorw_param.slot = slot;
    iorw_param.reg = CIC_CSR;    /* Command/Status Register */
    iorw_param.buffer = data;

    // Check CAM Pull Out or Not
    if (ci_link_cam_pullout(tp->cic_dev, slot, 1, 1) == SUCCESS)
    {
        state = 1000;            /* Flag to Exit State Match */
    }

    if (state == 0)
    {
        /* Step 1: Reset the CAM Card Interface */
        data[0] = CI_REG_RS;
        cic_io_control(tp->cic_dev, CIC_DRIVER_WRITEIO, (UINT32)&iorw_param);

        state++;
        ci_set_timer(200, (ci_timer_handler)ci_link_negotiation, (void *)link, (void *)((state << 8) | slot));
        return SUCCESS;
    }
    else if (state == 1)
    {
        /* Additional Step for ATsky CAM: Clear Reset CAM Card Interface*/
        data[0] = 0x00;
        cic_io_control(tp->cic_dev, CIC_DRIVER_WRITEIO, (UINT32)&iorw_param);

        state++;
        ci_set_timer(0, (ci_timer_handler)ci_link_negotiation, (void *)link, (void *)((state << 8) | slot));
        return SUCCESS;
    }
    else if (state > 1 && state <= 100)
    {
        /* Step 2: Wait Interface Ready */
        cic_io_control(tp->cic_dev, CIC_DRIVER_READIO, (UINT32)&iorw_param);
        if (data[0] & CI_REG_FR)
        {
            LINK_PRINTF("ci_link_negotiation:[slot %d] Reset IFX wait FREE %dmS!\n", slot, (state - 2) * 100);

            state = 101;        /* Ok, Jump to Next State */
            ci_set_timer(0, (ci_timer_handler)ci_link_negotiation, (void *)link, (void *)((state << 8) | slot));
            /* Begin Negotiation of Transfer Buffer Size */
            /* Step 3: Get the CAM Card Buffer Size */
            // Notify CAM Card of Provide its Maximum Buffer Size
            data[0] = CI_REG_SR;
            cic_io_control(tp->cic_dev, CIC_DRIVER_WRITEIO, (UINT32)&iorw_param);
            return SUCCESS;
        }
        if (state == 100)
        {
            LINK_PRINTF("ci_link_negotiation:[slot %d] Reset wait FREE timeout!\n", slot);
            state = 1000;        /* Flag to Exit State Match */
        }
        if (state < 100)
        {
            state++;
            ci_set_timer(100, (ci_timer_handler)ci_link_negotiation, (void *)link, (void *)((state << 8) | slot));
            return SUCCESS;
        }
    }
    else if (state > 100 && state <= 200)
    {
        // Wait for CAM Card Send Max Buffer Size
        cic_io_control(tp->cic_dev, CIC_DRIVER_READIO, (UINT32)&iorw_param);
        if (data[0] & CI_REG_DA)
        {
            LINK_PRINTF("ci_link_negotiation:[slot %d] Get size wait DATA %dmS!\n", slot, (state - 101) * 100);
            // Read Max Buffer Size of CAM Card
            bsize = 2;
            ci_ph_read(tp->cic_dev, slot, &bsize, data);
            if (bsize != 2)
            {
                LINK_PRINTF("ci_link_negotiation:[slot %d] Buffer data size is not 2: %d!\n", slot, bsize);
                state = 1000;    /* Flag to exit state matchin */
            }
            else
            {
                // Select Minimum Buffer Size to be Used
                bsize = (data[0] << 8) | data[1];
                LINK_PRINTF("ci_link_negotiation:[slot %d] Transfer buffer size of CAM: %x\n", slot, bsize);
                tp->buffer_size = (bsize > CI_LINK_BUFFER_SIZE ? CI_LINK_BUFFER_SIZE : bsize);
                LINK_PRINTF("ci_link_negotiation:[slot %d] Transfer buffer size: %x\n", slot, tp->buffer_size);

                state = 201;    /* Ok, Jump to Next State */
                ci_set_timer(0, (ci_timer_handler)ci_link_negotiation, (void *)link, (void *)((state << 8) | slot));
                /* Step 4: Set the CAM Card Buffer Size to be Used */
                data[0] = CI_REG_SW;
                cic_io_control(tp->cic_dev, CIC_DRIVER_WRITEIO, (UINT32)&iorw_param);
                return SUCCESS;
            }
        }
        if (state == 200)
        {
            state = 1000;        /* Flag to Exit State Match */
        }
        if (state < 200)
        {
            state++;
            ci_set_timer(100, (ci_timer_handler)ci_link_negotiation, (void *)link, (void *)((state << 8) | slot));
            return SUCCESS;
        }
    }
    else if (state > 200 && state <= 300)
    {
        // Wait CAM Card Ready
        cic_io_control(tp->cic_dev, CIC_DRIVER_READIO, (UINT32)&iorw_param);
        if (data[0] & CI_REG_FR)
        {
            LINK_PRINTF("ci_link_negotiation:[slot %d] Set size wait FREE %dmS!\n", slot, (state - 201) * 100);

            /* Keep Few Time for Some Card(board) Need Some Gap between Read Size and Write Size */
            state = 301;        /* Ok, Jump to Next State */
            ci_set_timer(100, (ci_timer_handler)ci_link_negotiation, (void *)link, (void *)((state << 8) | slot));
            return SUCCESS;
        }
        if (state == 300)
        {
            LINK_PRINTF("ci_link_negotiation:[slot %d] Set size wait FREE timeout!\n", slot);
            state = 1000;        /* Flag to exit state matchin */
        }
        if (state < 300)
        {
            state++;
            ci_set_timer(100, (ci_timer_handler)ci_link_negotiation, (void *)link, (void *)((state << 8) | slot));
            return SUCCESS;
        }
    }
    else if (state == 301)
    {
        // Send Buffer Size to be Used to CAM Card
        data[0] = ((tp->buffer_size >> 8) & 0xff);
        data[1] = (tp->buffer_size & 0xff);
        ci_ph_write(tp->cic_dev, slot, 2, data, CI_REG_SW);

        data[0] = 0;
        cic_io_control(tp->cic_dev, CIC_DRIVER_WRITEIO, (UINT32)&iorw_param);

        LINK_PRINTF("ci_link_negotiation:[slot %d] Shakhands OK!\n", slot);
        /* Add a few Gap between Shakehands and Init TC for Copatible with Ultimate CAM(SMT reported) */
        ci_set_timer(10, (ci_timer_handler)api_ci_msg_to_stack, (void *)slot, (void *)API_MSG_CTC);
        return SUCCESS;
    }

    // Notify Stack and App of CAM Out
    link->slot[slot].state = CAM_STACK_DETACH;
    api_ci_msg_to_stack(slot, API_MSG_CAMOUT);
    api_ci_msg_to_app(slot, API_MSG_CAMOUT);
    return ERR_FAILUE;
}

INT32 ci_reset_cam(struct ci_link_layer *link, int slot)
{
    struct ci_slot *tp;
    struct cic_io_command_signal signal_param;

    tp = &(link->slot[slot]);

    // Select Slot
    signal_param.slot = slot;
    signal_param.signal = CIC_SLOTSEL;
    signal_param.status = 1;
    cic_io_control(tp->cic_dev, CIC_DRIVER_SSIGNAL, (UINT32)&signal_param);

    // Reset Slot
    signal_param.signal = CIC_RSTSLOT;
    signal_param.status = 1;    //1: reset
    cic_io_control(tp->cic_dev, CIC_DRIVER_SSIGNAL, (UINT32)&signal_param);
    osal_task_sleep(200);

    // Select Slot
    signal_param.slot = slot;
    signal_param.signal = CIC_SLOTSEL;
    signal_param.status = 1;
    cic_io_control(tp->cic_dev, CIC_DRIVER_SSIGNAL, (UINT32)&signal_param);

    // Clear Reset Slot
    signal_param.signal = CIC_RSTSLOT;
    signal_param.status = 0;
    cic_io_control(tp->cic_dev, CIC_DRIVER_SSIGNAL, (UINT32)&signal_param);

    return SUCCESS;

}

INT32 ci_rs_reset_cam(struct ci_link_layer *link, int slot)
{
    struct ci_slot *tp;
    struct cic_io_command_signal     signal_param;
    struct cic_io_command_iorw     iorw_param;
    UINT8     data[2];

    tp = &(link->slot[slot]);

    // Select Slot
    signal_param.slot = slot;
    signal_param.signal = CIC_SLOTSEL;
    signal_param.status = 1;
    cic_io_control(tp->cic_dev, CIC_DRIVER_SSIGNAL, (UINT32)&signal_param);

    // Set I/O Param
    iorw_param.slot = slot;
    iorw_param.reg = CIC_CSR;    /* Command/Status Register */
    iorw_param.buffer = data;

    /* Step 1: Reset the CAM Card Interface */
    data[0] = CI_REG_RS;
    cic_io_control(tp->cic_dev, CIC_DRIVER_WRITEIO, (UINT32)&iorw_param);

    osal_task_sleep(200);

    /* Additional Step for ATsky CAM: Clear Reset CAM Card Interface*/
    data[0] = 0x00;
    cic_io_control(tp->cic_dev, CIC_DRIVER_WRITEIO, (UINT32)&iorw_param);

    return SUCCESS;
}

/* Read Data from CAM Card */
static INT32 ci_ph_read(struct cic_device *dev, int slot, UINT16 *size, UINT8 *buffer)
{
    struct cic_io_command_iorw iorw_param;
    int timeout;
    int bsize;
    UINT8 data[1] = {0};
    struct cic_io_command_signal signal_param;

    // Set Param
    iorw_param.slot = slot;
    iorw_param.reg = CIC_CSR;
    iorw_param.buffer = data;

    /* Check CAM Pull Out Or Not */
    ci_link_cam_pullout(dev, slot, 1, 2);

    /* Wait Data Available */
//    for (timeout = 40; timeout > 0; timeout--)
    for (timeout = CAM_RW_TIMEOUT; timeout > 0; timeout--)
    {
        // Wait Data Available
        cic_io_control(dev, CIC_DRIVER_READIO, (UINT32)&iorw_param);
        /*************************/
        //Check IIR bit, whether reset CAM
        if (data[0] & CI_REG_IIR)
        {
            LINK_PRINTF("ci_ph_read:[slot %d] IIR bit 1!\n", slot);
            // Select Slot
            signal_param.slot = slot;
            signal_param.signal = CIC_SLOTSEL;
            signal_param.status = 1;
            cic_io_control(dev, CIC_DRIVER_SSIGNAL, (UINT32)&signal_param);

            // Reset Slot
            signal_param.signal = CIC_RSTSLOT;
            signal_param.status = 1;    //1: reset
            cic_io_control(dev, CIC_DRIVER_SSIGNAL, (UINT32)&signal_param);
            osal_task_sleep(200);

            // Select Slot
            signal_param.slot = slot;
            signal_param.signal = CIC_SLOTSEL;
            signal_param.status = 1;
            cic_io_control(dev, CIC_DRIVER_SSIGNAL, (UINT32)&signal_param);

            // Clear Reset Slot
            signal_param.signal = CIC_RSTSLOT;
            signal_param.status = 0;
            cic_io_control(dev, CIC_DRIVER_SSIGNAL, (UINT32)&signal_param);

            return ERR_FAILUE;
        }
        /*************************/
        if (data[0] & CI_REG_DA)
        {
            break;
        }

        // Check CAM Pull Out Or Not
        if (ci_link_cam_pullout(dev, slot, 0, 2) == SUCCESS)
        {
            return ERR_FAILUE;
        }
//        osal_task_sleep(100);
        //osal_task_sleep(20);    //SmarDTV CI+ CAM performace requirement 100715
        osal_task_sleep(CAM_RW_DELAY);
    }

    /* Read Data Error */
    if (timeout == 0)
    {
        LINK_PRINTF("ci_ph_read:[slot %d] card not ready %02x!\n", slot, data[0]);
        return ERR_FAILUE;
    }

    /* Get Data Size */
    iorw_param.reg = CIC_SIZEMS;
    cic_io_control(dev, CIC_DRIVER_READIO, (UINT32)&iorw_param);
    bsize = (data[0] << 8);
    iorw_param.reg = CIC_SIZELS;
    cic_io_control(dev, CIC_DRIVER_READIO, (UINT32)&iorw_param);
    bsize |= data[0];
    if (bsize > *size)
    {
        LINK_PRINTF("ci_ph_read:[slot %d] Data longer than link buffer!\n", slot);
        return ERR_FAILUE;
    }
    *size = bsize;

    /* Read Data from CI Card */
    cic_read(dev, slot, bsize, buffer);

    /* Clear Command Register */
    iorw_param.reg = CIC_CSR;
    data[0] = 0;
    cic_io_control(dev, CIC_DRIVER_WRITEIO, (UINT32)&iorw_param);

#if CIPLUS_DEBUG_INFO_LINK
    {
        int i;
        LINK_PRINTF("ci_ph_read:[slot %d] size %d <-\n", slot, *size);
        for (i = 0; i < *size; i++)
            LINK_PRINTF("%02x ", buffer[i]);
        LINK_PRINTF("\n");
    }
#endif
    return SUCCESS;
}

/* Write Data to CAM Card */
static INT32 ci_ph_write(struct cic_device *dev, int slot, UINT16 size, UINT8 *buffer, UINT8 reg_bit)
{
    struct cic_io_command_iorw     iorw_param;
    int timeout;
    UINT8 data[1] = {0};
    struct cic_io_command_signal signal_param;

       MEMSET(&iorw_param, 0, sizeof(struct cic_io_command_iorw));
       MEMSET(&signal_param, 0, sizeof(struct cic_io_command_signal));
#if CIPLUS_DEBUG_INFO_LINK
    {
        int i;
        LINK_PRINTF("ci_ph_write:[slot %d] size %d ->\n", slot, size);
        for (i = 0; i < size; i++)
            LINK_PRINTF("%02x ", buffer[i]);
        LINK_PRINTF("\n");
    }
#endif

    /* Check CAM Free (Can Send Data to CAM Or Not) */
    iorw_param.slot = slot;
    iorw_param.reg = CIC_CSR;
    iorw_param.buffer = data;
    cic_io_control(dev, CIC_DRIVER_READIO, (UINT32)&iorw_param);
    /************************************/
    if (data[0] & CI_REG_IIR)
    {
        LINK_PRINTF("ci_ph_write:[slot %d] IIR bit 1!\n", slot);
        // Select Slot
        signal_param.slot = slot;
        signal_param.signal = CIC_SLOTSEL;
        signal_param.status = 1;
        cic_io_control(dev, CIC_DRIVER_SSIGNAL, (UINT32)&signal_param);

        // Reset Slot
        signal_param.signal = CIC_RSTSLOT;
        signal_param.status = 1;    //1: reset
        cic_io_control(dev, CIC_DRIVER_SSIGNAL, (UINT32)&signal_param);
        osal_task_sleep(200);

        // Select Slot
        signal_param.slot = slot;
        signal_param.signal = CIC_SLOTSEL;
        signal_param.status = 1;
        cic_io_control(dev, CIC_DRIVER_SSIGNAL, (UINT32)&signal_param);

        // Clear Reset Slot
        signal_param.signal = CIC_RSTSLOT;
        signal_param.status = 0;
        cic_io_control(dev, CIC_DRIVER_SSIGNAL, (UINT32)&signal_param);

        return ERR_FAILUE;
    }
    /************************************/
    if (data[0] & CI_REG_DA)
    {
        LINK_PRINTF("ci_ph_write:[slot %d] Collision with read when write!\n", slot);
        return ERR_FAILUE;
    }

    /* Check CAM Pull Out Or Not */
    ci_link_cam_pullout(dev, slot, 1, 2);

//fix SmartDTV Media gurad(SN:11410743900001) + CA card(SN:12800685567) issue.
//36 CI cam timing & performance issue.
    if (ALI_S3602==sys_ic_get_chip_id()
        || ALI_S3602F==sys_ic_get_chip_id())
    {
        osal_task_sleep(5);    //min for SmarDTV CAM card, Emergency SW download test case - 20100715
//        osal_task_sleep(10);        //for SmarDTV CAM card!!! 20100429(CAM Hw compability/time issue!)
//        osal_task_sleep(20);
    }

    /* Wait Module Free */
//    for (timeout = 40; timeout > 0; timeout--)
    for (timeout = CAM_RW_TIMEOUT; timeout > 0; timeout--)
    {
        // Host Control before Start Data Write Sequence
        data[0] = CI_REG_HC | reg_bit;    //CI_REG_SW is 1 when init
        cic_io_control(dev, CIC_DRIVER_WRITEIO, (UINT32)&iorw_param);

        // Wait Module Free to Accept Data from Host
        cic_io_control(dev, CIC_DRIVER_READIO, (UINT32)&iorw_param);
        if ((data[0] & CI_REG_FR) == CI_REG_FR)
        {
            break;
        }

        // Clear Host Control after Wait Fail
        data[0] = 0;
        cic_io_control(dev, CIC_DRIVER_WRITEIO, (UINT32)&iorw_param);

        // Check CAM Pull Out Or Not
        if (ci_link_cam_pullout(dev, slot, 0, 2) == SUCCESS)
        {
            return ERR_FAILUE;
        }
//        osal_task_sleep(100);
        //osal_task_sleep(20);    //SmarDTV CI+ CAM performace requirement 100715
        osal_task_sleep(CAM_RW_DELAY);
    }

    /* Write Data Error */
    if (timeout == 0)
    {
        LINK_PRINTF("ci_ph_write:[slot %d] card not ready %02x!\n", slot, data[0]);
        return ERR_FAILUE;
    }

    /* Set Data Size */
    iorw_param.reg = CIC_SIZEMS;
    data[0] = ((size >> 8) & 0xff);
    cic_io_control(dev, CIC_DRIVER_WRITEIO, (UINT32)&iorw_param);
    iorw_param.reg = CIC_SIZELS;
    data[0] = (size & 0xff);
    cic_io_control(dev, CIC_DRIVER_WRITEIO, (UINT32)&iorw_param);

    /* Write Data to CI card */
    cic_write(dev, slot, size, buffer);

    /* Clear Command Register */
    iorw_param.reg = CIC_CSR;
    data[0] = 0;
    cic_io_control(dev, CIC_DRIVER_WRITEIO, (UINT32)&iorw_param);

    return SUCCESS;
}

 /* Parse Device Other Information */
static INT32 parse_device_oa(struct cistpl_tuple *tuple, struct cistpl_device_o *device_o)
{
    struct cistpl_device *device = &device_o->device;
    int i;
    UINT8 *p, *q, scale;

        p = (UINT8 *)tuple->tuple_data;
        q = p + tuple->tuple_data_len;

    /* Other Conditions Info */
    device_o->mwait = *p & 1;            /* Support WAIT Signal or not */
    device_o->vcc_used = (*p >> 1) & 3;    /* Vcc to be Used */

    /* Jump Off Addition Bytes */
    while (*p & 0x80)
    {
        if (++p == q)
        {
            return ERR_FAILUE;
        }
    }
    p++;

    /* Parse Device Informatiom */
        device->ndev = 0;
        for (i = 0; i < CISTPL_MAX_DEVICES; i++)
        {
        if (*p == 0xff)
        {
            break;
        }
        device->dev[i].type = (*p >> 4) & 0xf;        /* Card Device Type */
        device->dev[i].wp = (*p & 0x08) ? 1 : 0;        /* Write Protect Switch */
        switch (*p & 0x07)                        /* Card Device Speed */
        {
        case 0: device->dev[i].speed = 0; break;
        case 1: device->dev[i].speed = 250; break;
        case 2: device->dev[i].speed = 200; break;
        case 3: device->dev[i].speed = 150; break;
        case 4: device->dev[i].speed = 100; break;
        case 7:
                if (++p == q)
                {
                    return ERR_FAILUE;
                }
                device->dev[i].speed = SPEED_CVT(*p);
                while (*p & 0x80)
                {
                if (++p == q)
                {
                    return ERR_FAILUE;
                }
                }
            break;
        default:
                return ERR_FAILUE;
        }

        if (++p == q)
        {
            return ERR_FAILUE;
        }

        if (*p == 0xff)
        {
            break;
        }

        scale = *p & 7;    /* Device Size: Device Memory Address Range */
        if (scale == 7)
        {
            return ERR_FAILUE;
        }
        device->dev[i].size = ((*p >> 3) + 1) * (512 << (scale * 2));
        device->ndev++;
        if (++p == q)
        {
            break;
        }
        }

        return SUCCESS;
}

/* Parse Strings */
static INT32 parse_strings(UINT8 *p, UINT8 *q, int max, char *s, UINT8 *ofs, UINT8 *found)
{
        int i, j, ns;

        if (p == q)
    {
        return ERR_FAILUE;
        }

        ns = 0; j = 0;
        for (i = 0; i < max; i++)
    {
        if (*p == 0xff)
        {
            break;
        }
        ofs[i] = j;    /* Starting Point of Every String */
        ns++;
        for (;;)
        {
                s[j++] = (*p == 0xff) ? '\0' : *p;    /* Return Strings */
                if ((*p == '\0') || (*p == 0xff))
            {
                break;
                }
                if (++p == q)
            {
                return ERR_FAILUE;
                }
        }
        if ((*p == 0xff) || (++p == q))
        {
            s[j] = '\0';
            break;
        }
        }
        if (found)
    {
        *found = ns;        /* Number of Strings */
        return SUCCESS;
        }
    else
    {
        return (ns == max) ? SUCCESS : ERR_FAILUE;
        }
}

/* Parse Version Level 1 Information */
static INT32 parse_vers1(struct cistpl_tuple *tuple, struct cistpl_vers_1 *vers_1)
{
        UINT8 *p, *q;

        p = (UINT8 *)tuple->tuple_data;
        q = p + tuple->tuple_data_len;

        vers_1->major = *p; p++;
        vers_1->minor = *p; p++;
        if (p >= q)
    {
        return ERR_FAILUE;
    }

        return parse_strings(p, q, CISTPL_VERS_1_MAX_PROD_STRINGS, vers_1->str, vers_1->ofs, &vers_1->ns);
}

/* Parse Manufacturer ID Information */
static INT32 parse_manfid(struct cistpl_tuple *tuple, struct cistpl_manfid *manfid)
{
        UINT16 *p;

        if (tuple->tuple_data_len < 4)
        {
        return ERR_FAILUE;
        }
        p = (UINT16 *)tuple->tuple_data;
        manfid->manf = le16_to_cpu(p[0]);    /* PC Card's Manufacturer */
        manfid->card = le16_to_cpu(p[1]);        /* Card Identifier and Revision Information */
        return SUCCESS;
}

/* Parse Config Information */
static INT32 parse_config(struct cistpl_tuple *tuple, struct cistpl_config *config)
{
        int rasz, rmsz, i, ifnsz;
        UINT8 *p, *q;

        p = (UINT8 *)tuple->tuple_data;
        q = p + tuple->tuple_data_len;
        rasz = *p & 0x03;            /* Number of Bytes of TPCC_RADR - 1 */
        rmsz = (*p & 0x3c) >> 2;    /* Number of Bytes TPCC_RMSK field - 1 */
        if (tuple->tuple_data_len < rasz + rmsz + 4)
        {
        return ERR_FAILUE;
        }
        config->last_idx = *(++p);    /* Index Number of Final Entry in the Card Configuration Table */
        p++;
        config->base = 0;        /* TPCC_RADR: Configuration Registers Base Address */
        for (i = 0; i <= rasz; i++)
        {
        config->base += p[i] << (8 * i);
        }
        p += rasz + 1;
        for (i = 0; i < 4; i++)        /* TPCC_RMSK: Configuration Registers Present Mask */
    {
        config->rmask[i] = 0;
    }
        for (i = 0; i <= rmsz; i++)
    {
        config->rmask[i >> 2] += p[i] << (8 * (i % 4));
    }
        config->subtuples = tuple->tuple_data_len - (rasz + rmsz + 4);    /* Data Length of Sub-Tuples */

        /* Note: Add Code for Sub-Tuple of CCSTPL_CIF in DVB CAM Applicatoins */
    if (config->subtuples != 0)
    {
        p += (rmsz + 1);
        if (*p == CCSTPL_CIF)
        {
            p += 2;
            ifnsz = (*p >> 6) & 3;        /* STCI_IFN_SIZE: Number of Bytes in Custom Interface ID Number */
            config->cif_infor.ifn = 0;    /* Interface ID Number */
            for (i = 0; i <= ifnsz; i++)
            {
                config->cif_infor.ifn += p[i] << (8 * i);
            }
            p += (ifnsz + 1);
            /* Interface Description Strings */
            parse_strings(p, q, CCSTPL_CIF_MAX_PROD_STRINGS, config->cif_infor.str, config->cif_infor.ofs, &(config->cif_infor.ns));
            config->cif_infor.ns++;    /* Interface Description String Number */
        }
    }

        return SUCCESS;
}

/* Parse Power Description Structure of Config Table Entry Information */
static UINT8 * parse_power(UINT8 *p, UINT8 *q, struct cistpl_power *pwr)
{
        int i;
        UINT32 scale;

        if (p == q)
    {
        return NULL;
    }
        pwr->present = *p;
        pwr->flags = 0;
        p++;

    /* Param Definitions */
        for (i = 0; i < 7; i++)
        {
        if (pwr->present & (1 << i))
        {
                if (p == q)
            {
                return NULL;
            }
                pwr->param[i] = POWER_CVT(*p);
                scale = POWER_SCALE(*p);
                while (*p & 0x80)
            {
                if (++p == q)
                {
                    return NULL;
                }
                if ((*p & 0x7f) < 100)    /* Binary Value for Next Two Decimal Digits to Right of Current Value */
                    {
                        pwr->param[i] += (*p & 0x7f) * scale / 100;
                }
                else if (*p == 0x7d)    /* High Impedance OK During Sleep or Power-Down only */
                    {
                        pwr->flags |= CISTPL_POWER_HIGHZ_OK;
                }
                else if (*p == 0x7e)    /* Zero Value */
                    {
                        pwr->param[i] = 0;
                }
                else if (*p == 0x7f)    /* High Impedance Required */
                    {
                        pwr->flags |= CISTPL_POWER_HIGHZ_REQ;
                }
                else
                        return NULL;
                }
                p++;
        }
        }
        return p;
}

/* Parse Configuration Timing Information of Config Table Entry Information */
static UINT8 * parse_timing(UINT8 *p, UINT8 *q, struct cistpl_timing *timing)
{
        UINT8 scale;

        if (p == q)
    {
        return NULL;
    }

    scale = *p;
    if ((scale & 3) != 3)    /* MAX Wait Time and Wait Scale */
    {
        if (++p == q)
        {
            return NULL;
        }
        timing->wait = SPEED_CVT(*p);
        timing->waitscale = exponent[scale & 3];
        }
    else
    {
        timing->wait = 0;
    }

    scale >>= 2;
        if ((scale & 7) != 7)    /* MAX Ready Time and Ready Scale */
    {
        if (++p == q)
        {
            return NULL;
        }
        timing->ready = SPEED_CVT(*p);
        timing->rdyscale = exponent[scale & 7];
        }
    else
    {
        timing->ready = 0;
    }

    scale >>= 3;
        if (scale != 7)            /* Reserved Time Definition and Reserved Scale */
    {
        if (++p == q)
        {
            return NULL;
        }
        timing->reserved = SPEED_CVT(*p);
        timing->rsvscale = exponent[scale];
        }
    else
    {
        timing->reserved = 0;
    }

    p++;
        return p;
}

/* Parse I/O Space Addresses Required For This Configuration of Config Table Entry Information */
static UINT8 * parse_io(UINT8 *p, UINT8 *q, struct cistpl_io *io)
{
        int i, j, bsz, lsz;

        if (p == q)
    {
        return NULL;
    }

    io->flags = *p;
        if (!(*p & 0x80))    /* Only I/O Space Definition Byte */
    {
        io->nwin = 1;
        io->win[0].base = 0;
        io->win[0].len = (1 << (io->flags & CISTPL_IO_LINES_MASK));
        return p+1;
        }

    if (++p == q)
    {
        return NULL;
    }

    /* Range Descriptor Byte follows I/O Space Definition Byte */
        io->nwin = (*p & 0x0f) + 1;    /* Number of I/O Address Ranges */
        bsz = (*p & 0x30) >> 4;        /* Number of  Bytes which are Used to Encode Start of I/O Address Block */
        if (bsz == 3)
    {
        bsz++;
    }
        lsz = (*p & 0xc0) >> 6;            /* Number of Bytes which are Used to Encode Length of I/O Address Block */
        if (lsz == 3)
    {
        lsz++;
    }
        p++;

    /* I/O Address Range Description */
        for (i = 0; i < io->nwin; i++)
    {
        io->win[i].base = 0;
        io->win[i].len = 1;
        /* Start of I/O Address Block */
        for (j = 0; j < bsz; j++, p++)
        {
                if (p == q)
            {
                return NULL;
            }
                io->win[i].base += *p << (j * 8);
        }
        /* Length of I/O Address Block */
        for (j = 0; j < lsz; j++, p++)
        {
                if (p == q)
            {
                return NULL;
            }
                io->win[i].len += *p << (j * 8);
        }
        }
        return p;
}

/* Parse Memory Space Description Structure of Config Table Entry Information */
static UINT8 * parse_mem(UINT8 *p, UINT8 *q, struct cistpl_mem *mem)
{
        int i, j, asz, lsz, has_ha;
        UINT32 len, ca, ha;

        if (p == q)
    {
        return NULL;
    }

        mem->nwin = (*p & 0x07) + 1;        /* Number of Window Descriptor */
        lsz = (*p & 0x18) >> 3;                /* Length Field Size */
        asz = (*p & 0x60) >> 5;            /* Card Address Field Size */
        has_ha = (*p & 0x80);                /* Host Address is Present or not */
        if (++p == q)
    {
        return NULL;
        }

        for (i = 0; i < mem->nwin; i++)
    {
        // Length
        len = ca = ha = 0;
        for (j = 0; j < lsz; j++, p++)
        {
                if (p == q)
            {
                return NULL;
                }
                len += *p << (j * 8);
        }
        // Card Address
        for (j = 0; j < asz; j++, p++)
        {
                if (p == q)
            {
                return NULL;
            }
                ca += *p << (j * 8);
        }
        // Host Address
        if (has_ha)
        {
                for (j = 0; j < asz; j++, p++)
            {
                if (p == q)
                {
                    return NULL;
                }
                ha += *p << (j * 8);
                }
        }
        mem->win[i].len = len << 8;
        mem->win[i].card_addr = ca << 8;
        mem->win[i].host_addr = ha << 8;
        }
        return p;
}

#define IRQ_INFO2_VALID        0x10        /* MASK bit */

/* Parse Interrupt Request Description Structure of Config Table Entry Information */
static UINT8 * parse_irq(UINT8 *p, UINT8 *q, struct cistpl_irq *irq)
{
        if (p == q)
    {
        return NULL;
    }
        irq->irqinfo1 = *p;                        /* Interrupt Request Byte 0 */
    p++;
        if (irq->irqinfo1 & IRQ_INFO2_VALID)
    {
        if (p + 2 > q)
        {
            return NULL;
        }
        irq->irqinfo2 = (p[1] << 8) + p[0];        /* Interrupt Request Byte 1 and 2 */
        p += 2;
        }
        return p;
}

/* Parse Config Table Entry Information */
static INT32 parse_cftable_entry(struct cistpl_tuple *tuple, struct cistpl_cftable_entry *entry)
{
        UINT8 *p, *q, features;

        p = tuple->tuple_data;
        q = p + tuple->tuple_data_len;
    entry->index = *p & 0x3f;        /* Configuration-Entry-Number */
        entry->flags = 0;
        if (*p & 0x40)                    /* Default Value for Entry */
        {
        entry->flags |= CISTPL_CFTABLE_DEFAULT;
        }
        if (*p & 0x80)                    /* Interface Configuration Present or not */
    {
        if (++p == q)
        {
            return ERR_FAILUE;
        }
        if (*p & 0x10)                /* BVDs Active */
            {
                entry->flags |= CISTPL_CFTABLE_BVDS;
        }
        if (*p & 0x20)                /* Write Protect (WP) Status Active*/
            {
                entry->flags |= CISTPL_CFTABLE_WP;
        }
        if (*p & 0x40)                /* READY Status Active */
            {
                entry->flags |= CISTPL_CFTABLE_RDYBSY;
        }
        if (*p & 0x80)                /* M Wait Required */
            {
                entry->flags |= CISTPL_CFTABLE_MWAIT;
        }
        entry->interface = *p & 0x0f;        /* Interface Type */
        }
    else
    {
        entry->interface = 0;
    }

        /* Process Optional Features */
        if (++p == q)
    {
        return ERR_FAILUE;
    }
        features = *p;                    /* Feature Selection Byte */
    p++;

        /* Power Options */
        if ((features & 3) > 0)
        {
        p = parse_power(p, q, &entry->vcc);        /* VCC Power-Description-Structure */
        if (p == NULL)
        {
            return ERR_FAILUE;
        }
        }
    else
    {
        entry->vcc.present = 0;
    }
        if ((features & 3) > 1)
    {
        p = parse_power(p, q, &entry->vpp1);        /* Vpp1 Power-Description-Structure */
        if (p == NULL)
        {
            return ERR_FAILUE;
        }
        }
    else
    {
        entry->vpp1.present = 0;
    }
        if ((features & 3) > 2)
    {
        p = parse_power(p, q, &entry->vpp2);        /* Vpp2 Power-Description-Structure */
        if (p == NULL)
        {
            return ERR_FAILUE;
        }
        }
    else
    {
        entry->vpp2.present = 0;
    }

        /* Timing Options */
        if (features & 0x04)
    {
        p = parse_timing(p, q, &entry->timing);
        if (p == NULL)
        {
            return ERR_FAILUE;
        }
        }
    else
    {
        entry->timing.wait = 0;
        entry->timing.ready = 0;
        entry->timing.reserved = 0;
        }

        /* I/O Space Options */
        if (features & 0x08)
    {
        p = parse_io(p, q, &entry->io);
        if (p == NULL)
        {
            return ERR_FAILUE;
        }
        }
    else
    {
        entry->io.nwin = 0;
    }

        /* Interrupt Options */
        if (features & 0x10)
    {
        p = parse_irq(p, q, &entry->irq);
        if (p == NULL)
        {
            return ERR_FAILUE;
        }
        }
    else
    {
        entry->irq.irqinfo1 = 0;
    }

    /* Memory Space Option */
        switch (features & 0x60)
    {
            case 0x00:
            entry->mem.nwin = 0;
            break;
            case 0x20:
            /* the address must be Even, because (UINT16 *)p
             * otherwise, STB will reboot
             */
            if (((UINT32)p) & 0x01)
                return ERR_FAILUE;

            entry->mem.nwin = 1;
            entry->mem.win[0].len = le16_to_cpu(*(UINT16 *)p) << 8;
            entry->mem.win[0].card_addr = 0;
            entry->mem.win[0].host_addr = 0;
            p += 2;
            if (p > q)
            {
                return ERR_FAILUE;
            }
            break;
            case 0x40:
            /* the address must be Even, because (UINT16 *)p
             * otherwise, STB will reboot
             */
            if (((UINT32)p) & 0x01)
                return ERR_FAILUE;
            entry->mem.nwin = 1;
            entry->mem.win[0].len = le16_to_cpu(*(UINT16 *)p) << 8;
            entry->mem.win[0].card_addr = le16_to_cpu(*(UINT16 *)(p + 2)) << 8;
            entry->mem.win[0].host_addr = 0;
            p += 4;
            if (p > q)
            {
                return ERR_FAILUE;
            }
            break;
            case 0x60:
            p = parse_mem(p, q, &entry->mem);
            if (p == NULL)
            {
                return ERR_FAILUE;
            }
            break;
        }

        /* Misc Option */
        if (features & 0x80)
    {
        if (p == q)
        {
            return ERR_FAILUE;
        }
        entry->flags |= (*p << 8);
        while (*p & 0x80)
        {
                if (++p == q)
            {
                return ERR_FAILUE;
            }
        }
        p++;
        }

        entry->subtuples = q - p;

        return SUCCESS;
}
