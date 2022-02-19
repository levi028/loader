/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 Copyright (C)
*
*    File:    ci_trans.c
*
*    Description:    This file contains all globe micros and functions declare
*                     of CI stack - transport layer.
*    History:
*               Date                 Athor          Version          Reason
*        ==========     ========   ======   ==========================
*    1.    Nov.15.2004       Justin Wu      Ver 0.1         Create file.
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

static void ci_trans_conn_init(struct ci_transport_connect *tc, UINT8 slot, UINT8 tcid);
static INT32 ci_trans_recv_data(struct ci_transport_connect *tc);
static INT32 ci_trans_send_tpdu(struct ci_transport_connect *tc, UINT8 tag, int len, UINT8 *data);
static INT32 ci_trans_recv_tpdu(struct ci_transport_connect *tc);
static UINT8 ci_get_tpdu_status(struct ci_tpdu *tpdu);

/* Transmission Layer Attach */
INT32 ci_trans_attach(struct ci_transport_layer *trans)
{
    int i;

    // Initialize All Transmission Connections
    for (i = 0; i < CI_MAX_TRANSCON_NUM; i++)
    {
        ci_trans_conn_init(&(trans->connect[i]), i, 0);
        trans->connect[i].link = &(trans->link);
    }

    // Link Layer Attach
    return ci_link_attach(&(trans->link));
}

/* Transmission Layer Detach */
INT32 ci_trans_detach(struct ci_transport_layer *trans)
{
    // Link Layer Detach
    return ci_link_detach(&(trans->link));
}

/* Open Transmission Connection */
struct ci_transport_connect * ci_trans_open_conn(struct ci_transport_layer *trans, int slot)
{
    int i, j;

    for (i = 0; i < CI_MAX_TRANSCON_NUM; i++)
    {
        // Check Transmission Connection Status
        if (trans->connect[i].state == TRANS_STATE_IDLE)
        {
            TRAN_PRINTF("Creating connection: slot %d, tcid %d\n", slot, i + 1);
            // Initialize Transmission Connection
            ci_trans_conn_init(&(trans->connect[i]), slot, i + 1);

            // Send Create Transmission Connection Request
            if (ci_trans_send_tpdu(&(trans->connect[i]), TRANS_TAG_CREAT_TC, 0, NULL) == SUCCESS)
            {
                // Modify Transmission Connection Status
                trans->connect[i].state = TRANS_STATE_CREATION;

                // Receive Transmission Connection Reply
                if (ci_trans_recv_tpdu(&(trans->connect[i])) == TRANS_TAG_CTC_REPLY)
                {
                    trans->connect[i].trans_polling_cnt = 0;
                    trans->connect[i].trans_polling_fail_cnt = 0;
                    TRAN_PRINTF("Creat connection success\n");
                    return &(trans->connect[i]);
                    /* The following is a workaround for CAMs that don't quite follow the specs... */
                }
                // If Receive Transmission Connection Reply First Fail, then Receive Reply Twice
                else
                {
                    for (j = 0; j < CI_MAX_CONNECT_RETRIES; j++)
                    {
                        TRAN_PRINTF("CAM: retrying to establish connection\n");
                        if (ci_trans_recv_tpdu(&(trans->connect[i])) == TRANS_TAG_CTC_REPLY)
                        {
                            TRAN_PRINTF("CAM: connection established\n");
                            return &(trans->connect[i]);
                        }
                    }
                }
            }
            break;
        }
    }
    TRAN_PRINTF("Creat connection failue\n");
    return NULL;
}

/* Close Transmission Connection */
INT32 ci_trans_close_conn(struct ci_transport_layer *trans, int tcid)
{
    struct ci_transport_connect *tc = &(trans->connect[tcid - 1]);

    TRAN_PRINTF("ci_trans_close_conn: tcid = %08X\n", tcid);

    // Check Transmission Connection Status
    if (tc->state != TRANS_STATE_IDLE)
    {
        // Send Delete Transmission Connection Request
        if(ci_trans_send_tpdu(tc, TRANS_TAG_DELETE_TC, 0, NULL) != SUCCESS)
        {
            return ERR_FAILUE;
        }
        // Delete Transmission Connection
        ci_trans_conn_init(tc, tc->slot, tc->tcid);
        return SUCCESS;
    }
    else
    {
        TRAN_PRINTF("ci_trans_close_conn: unknown tcid id: %d\n", tcid);
    }
    return ERR_FAILUE;
}

/* Close Transmission Connection Directly */
static INT32 ci_trans_close_conn_d(struct ci_transport_layer *trans, int tcid)
{
    struct ci_transport_connect *tc = &(trans->connect[tcid - 1]);

    TRAN_PRINTF("ci_trans_close_conn: tcid = %08X\n", tcid);
    // Check Transmission Connection Status
    if (tc->state != TRANS_STATE_IDLE)
    {
        // Delete Transmission Connection
        ci_trans_conn_init(tc, tc->slot, tc->tcid);
        return SUCCESS;
    }
    else
    {
        TRAN_PRINTF("ci_trans_close_conn: unknown tcid id: %d\n", tcid);
    }
    return ERR_FAILUE;
}

/* Close All Transmission Connections */
int ci_trans_close_all_conn(struct ci_transport_layer *trans, int slot)
{
    int i, result = 0;

    for (i = 0; i < CI_MAX_TRANSCON_NUM; i++)
    {
        if (trans->connect[i].state != TRANS_STATE_IDLE && trans->connect[i].slot == slot)
        {
            ci_trans_close_conn(trans, trans->connect[i].tcid);
            result++;
        }
    }
    return result;
}

/* Close All Transmission Connection Directly */
int ci_trans_close_all_conn_d(struct ci_transport_layer *trans, int slot)
{
    int i, result = 0;

    for (i = 0; i < CI_MAX_TRANSCON_NUM; i++)
    {
        if (trans->connect[i].state != TRANS_STATE_IDLE && trans->connect[i].slot == slot)
        {
            ci_trans_close_conn_d(trans, trans->connect[i].tcid);
            result++;
        }
    }
    return result;
}

/* Process for Transmission Connection */
struct ci_transport_connect * ci_trans_process(struct ci_transport_layer *trans, int slot)
{
    struct ci_transport_connect *tc = NULL;
    UINT32 i = 0;

    for (i = 0; i < CI_MAX_TRANSCON_NUM; i++)
    {
        tc = &(trans->connect[i]);
        if (tc->slot == slot)
        {
            switch (tc->state)
            {
            case TRANS_STATE_CREATION:        /* Need Polling in Creation state */
            case TRANS_STATE_ACTIVE:        /* Polling when Free in Active state */
                if (!tc->data_available)        /* If No data Available, then Polling */
                {
                    /* Polling */
                    trans->connect[i].trans_polling_cnt++;
                    // Send T_Data_Last with Null Data Field to CAM for Polling
                    if (ci_trans_send_tpdu(tc, TRANS_TAG_DATA_LAST, 0, NULL) == SUCCESS)
                    {
                        // Receive TPDU from CAM
                        if (ci_trans_recv_tpdu(tc) == ERR_FAILUE)
                        {
                            TRAN_PRINTF("ci_trans_process %d: TC[%d] polling failue at recieve %d!\n", slot, i, trans->connect[i].trans_polling_cnt);
                            /* We Continue Here for Compability Reason.
                               If the CAM be Sure Dead, it will Polling Timeout */
                            // If Fail Counter Beyond Max Polling Retry Times, then Return Null (Receive No TPDU)
                            if (++trans->connect[i].trans_polling_fail_cnt > CI_MAX_POLLING_RETRIES)
                            {
                                return NULL;
                            }
                        }
                        else
                        {
                            trans->connect[i].trans_polling_fail_cnt = 0;
                            if ((trans->connect[i].trans_polling_cnt % 100) == 0)
                            {
                                TRAN_PRINTF("ci_trans_process %d: TC[%d] polling success %d!\n", slot, i, trans->connect[i].trans_polling_cnt);
                            }
                        }
                    }
                    else
                    {
                        TRAN_PRINTF("ci_trans_process %d: TC[%d] polling failue at send polling %d!\n", slot, i, trans->connect[i].trans_polling_cnt);
                        /* We Continue Here for Compability Reason.
                           If the CAM be Sure Dead, it will Polling Timeout */
                        // If Fail Counter Beyond Max Polling Retry Times, then Return Null (Receive No TPDU)
                        if (++trans->connect[i].trans_polling_fail_cnt > CI_MAX_POLLING_RETRIES)
                        {
                            return NULL;
                        }
                    }
                }
                // Check Last Received Object from CAM
                switch (tc->last_response)
                {
                case TRANS_TAG_REQUEST_TC:        /* Do Nothing, Reserved for Future */
                    TRAN_PRINTF("ci_trans_process %d: CAM request new TC!\n", slot);
                    break;
                case TRANS_TAG_DATA_MORE:
                case TRANS_TAG_DATA_LAST:
                case TRANS_TAG_CTC_REPLY:        /* Only For CREATION State */
                case TRANS_TAG_SB:
                    if (tc->data_available)
                    {
                        TRAN_PRINTF("ci_trans_process %d: recieve data...\n", slot);
                        // Request Receive Data from CAM
                        if (ci_trans_recv_data(tc) == ERR_FAILUE)
                        {
                            TRAN_PRINTF("ci_trans_process %d: recieve data error %d\n", slot, trans->connect[i].trans_polling_fail_cnt);
                            // If Fail Counter Beyond Max Polling Retry Times, then Return Null (Receive No Data)
                            if (++trans->connect[i].trans_polling_fail_cnt > CI_MAX_POLLING_RETRIES)
                            {
                                return NULL;
                            }
                            else
                            {
                                /* We Don't Return NULL here for Compability Reason.
                                   This Make it Can Try to Get Data at Next Loop.
                                   If the CAM be Sure Dead, it will Polling Timeout */
                                return (struct ci_transport_connect *)-1;
                            }
                        }
#if CIPLUS_DEBUG_INFO_TRANS
                        for (i  = 0; i  < tc->tpdu.size; i ++)
                        {
                            TRAN_PRINTF(" %02x", tc->tpdu.tpdu.data[i ]);
                        }
                        TRAN_PRINTF("\n");
#endif
                    }
                    break;
                case ERR_FAILUE:
                default:
                    //XXX Tc->state = stIDLE;//XXX Init()???
                    /* We Don't Return NULL here for Compability Reason.
                       This Make it can Try to Get Data at Next Loop.
                       If the CAM be Sure Dead, it will Polling Timeout */
                    TRAN_PRINTF("ci_trans_process %d: last_response %d\n", slot, tc->last_response);
                    return (struct ci_transport_connect *)-1;
                    break;
                }
                //This will Only Work with One Transport Connection Per Slot!
                return tc;
                break;
            default: ;
            }
        }
    }
    return (struct ci_transport_connect *)-1;
}

/* Send SPDU in TPDU */
INT32 ci_trans_send_spdu(struct ci_transport_connect *tc, int len, UINT8 *data)
{
    UINT8 tag = TRANS_TAG_DATA_LAST;    /* C_TPDU Tag */
    int l = len;

    while (tc->state == TRANS_STATE_ACTIVE && len > 0)
    {
        if (l > CI_MAX_TPDU_DATA_SIZE)    /* Fragment */
        {
            tag = TRANS_TAG_DATA_MORE;
            l = CI_MAX_TPDU_DATA_SIZE;
        }
        // If Send SPDU Fail or don't Receive TAG_SB, then Exit Loop
        if (ci_trans_send_tpdu(tc, tag, l, data) != SUCCESS ||ci_trans_recv_tpdu(tc) != TRANS_TAG_SB)
        {
                   break;
            }
        len -= l;
        data += l;
    }

    return len != 0 ? ERR_FAILUE : SUCCESS;
}

/* Get SPDU from TPDU */
UINT8 * ci_trans_get_spdu(struct ci_transport_connect *tc, int *len)
{
    UINT8 *data;

    if (tc->tpdu.size)
    {
        data = ci_get_length(tc->tpdu.tpdu.data + 3, len);    /* Jump to length */
        if (*len)
        {
            (*len)--;        /* The first byte is always the tcid, jump over it */
            return data + 1;
        }
    }
    return NULL;
}

/* Send Receive Data Request to CAM */
static INT32 ci_trans_recv_data(struct ci_transport_connect *tc)
{
    // Request Receive Data from CAM
    if (ci_trans_send_tpdu(tc, TRANS_TAG_RCV, 0, NULL) == SUCCESS)
    {
        /* Do noting, the date will process at later time out of process */
        // Receive TPDU
        return ci_trans_recv_tpdu(tc);
    }
    return ERR_FAILUE;
}

/* Initialize Transmission Connection */
static void ci_trans_conn_init(struct ci_transport_connect *tc, UINT8 slot, UINT8 tcid)
{
    tc->slot = slot;
    tc->tcid = tcid;
    tc->state = TRANS_STATE_IDLE;
    tc->last_response = ERR_FAILUE;
    tc->data_available = 0;
}

/* Send TPDU */
static INT32 ci_trans_send_tpdu(struct ci_transport_connect *tc, UINT8 tag, int len, UINT8 *data)
{
    struct ci_tpdu *tpdu = &(tc->tpdu);
    UINT8 *p;

    tpdu->size = 0;                        /* Reset Link Length */
    tpdu->tpdu.data[0] = tc->slot;             /* Link Slot */
    tpdu->tpdu.data[1] = tc->tcid;             /* Link TC_ID */
    tpdu->tpdu.data[2] = tag;                 /* TPDU TAG */
    switch (tag)
    {
    case TRANS_TAG_RCV:
    case TRANS_TAG_CREAT_TC:
    case TRANS_TAG_DELETE_TC:
    case TRANS_TAG_DTC_REPLY:
        tpdu->tpdu.data[3] = 1;             /* TPDU Length */
        tpdu->tpdu.data[4] = tc->tcid;        /* TPDU TC_ID */
        tpdu->size = 5;                    /* Link Length */
        break;
    case TRANS_TAG_NEW_TC:
    case TRANS_TAG_TC_ERROR:
        if (len == 1)
        {
            tpdu->tpdu.data[3] = 2;         /* TPDU Length */
            tpdu->tpdu.data[4] = tc->tcid;    /* TPDU TC_ID */
                tpdu->tpdu.data[5] = data[0];    /* TPDU New TC_ID or Error Code */
                tpdu->size = 6;                /* Link Length */
        }
        else
        {
                TRAN_PRINTF("ERROR: illegal data length for TPDU tag 0x%02X: %d\n", tag, len);
         }
            break;
    case TRANS_TAG_DATA_LAST:
    case TRANS_TAG_DATA_MORE:
        if (len <= CI_MAX_TPDU_DATA_SIZE)
        {
                p = tpdu->tpdu.data + 3;                    /* Seek to TPDU Length Field */
                p = ci_set_length(p, len + 1);
                if (NULL != p)
                    *p++ = tc->tcid;                            /* TPDU TC_ID */
                else
                    return RET_FAILURE;
                if (len != 0)                                /* TPDU data */
                {
                MEMCPY(p, data, len);
                }
                tpdu->size = len + (p - tpdu->tpdu.data);        /* Link Length */
            }
        else
        {
                TRAN_PRINTF("ERROR: illegal data length for TPDU tag 0x%02X: %d\n", tag, len);
        }
        break;
    default:
        TRAN_PRINTF("ERROR: unknown TPDU tag: 0x%02X\n", tag);
        break;    //090415 add
    }
#ifdef NEED_TPDU_IN_OUT_INFO
        TRAN_PRINTF("%s : tpdu size=%d, data= ",__FUNCTION__, tpdu->size);
        INT32 i;
        for(i=0;i<tpdu->size; i++)
        {
            TRAN_PRINTF("%02x ", tpdu->tpdu.data[i]);
        }
        TRAN_PRINTF("\n");
#endif
    // Send TPDU Data in Link Layer
    return ci_link_send_data(tc->link, tc->slot, tpdu->size, tpdu->tpdu.data);
}

/* Receive TPDU */
static INT32 ci_trans_recv_tpdu(struct ci_transport_connect *tc)
{
    struct ci_tpdu *tpdu = &(tc->tpdu);

    // Receive Data in Link Layer
    if (ci_link_recv_data(tc->link, tc->slot, &(tpdu->size), tpdu->tpdu.data) == SUCCESS &&
        tpdu->tpdu.info.tcid == tc->tcid)
    {
#ifdef NEED_TPDU_IN_OUT_INFO
            TRAN_PRINTF("%s : tpdu size=%d, data= ",__FUNCTION__, tpdu->size);
            INT32 i;
            for(i=0;i<tpdu->size; i++)
            {
                    TRAN_PRINTF("%02x ", tpdu->tpdu.data[i]);
            }
            TRAN_PRINTF("\n");
#endif
        switch (tc->state)
        {
        case TRANS_STATE_IDLE:
            /* Do Noting according to CI Spec. */
                    TRAN_PRINTF("ci_trans_recv_TPDU: TRANS_STATE_IDLE stat, do nothing\n");
            break;
        case TRANS_STATE_CREATION:
            /* Only Process CTC_REPLY Tag Input according to CI Spec. */
                    TRAN_PRINTF("ci_trans_recv_TPDU: TRANS_STATE_CREATION stat\n");
            if (tpdu->tpdu.info.tag == TRANS_TAG_CTC_REPLY)
            {
                tc->data_available = ci_get_tpdu_status(tpdu) & CI_DATA_INDICATOR;
                tc->last_response = tpdu->tpdu.info.tag;
                tc->state = TRANS_STATE_ACTIVE;
                        TRAN_PRINTF("ci_trans_recv_TPDU: TC created, slot %d\n", tc->slot);
            }
            break;
        case TRANS_STATE_ACTIVE:
            /* Only Process DATA_MORE, DATA_LAST, REQUEST_TC, DELET_TC, SB Tags
               Input according to CI Spec. */
            switch (tpdu->tpdu.info.tag)
            {
                    case TRANS_TAG_SB:
                    case TRANS_TAG_DATA_LAST:
                    case TRANS_TAG_DATA_MORE:
                    case TRANS_TAG_REQUEST_TC:
                        /* Leave for Upper Layer Process -> Process() */
                        break;
                    case TRANS_TAG_DELETE_TC:
                        TRAN_PRINTF("ci_trans_recv_TPDU: TRANS_TAG_DELETE_TC start\n");
                // Send Delete Transmission Connection Reply to CAM
                if (ci_trans_send_tpdu(tc, TRANS_TAG_DTC_REPLY, 0, NULL) != SUCCESS)
                        {
                            return ERR_FAILUE;
                        }
                        TRAN_PRINTF("ci_trans_recv_TPDU: TRANS_TAG_DELETE_TC done\n");
                // Delete Transmission Connection
                ci_trans_conn_init(tc, tc->slot, tc->tcid);
                break;
            default:
                        TRAN_PRINTF("ci_trans_recv_TPDU: unknow tag!\n");
                return ERR_FAILUE;
            }
            // Get Data Status and Tag
            tc->data_available = ci_get_tpdu_status(tpdu) & CI_DATA_INDICATOR;
            tc->last_response = tpdu->tpdu.info.tag;
            break;
        case TRANS_STATE_DELETION:
            /* Only Process DTC_REPLY Tag Input according to CI Spec. */
                    TRAN_PRINTF("ci_trans_recv_TPDU: TRANS_STATE_ACTIVE stat\n");
            if (tpdu->tpdu.info.tag == TRANS_TAG_DTC_REPLY)
            {
                        TRAN_PRINTF("ci_trans_recv_TPDU: host TC deleted, slot %d\n", tc->slot);
                // Delete Transmission Connection
                ci_trans_conn_init(tc, tc->slot, tc->tcid);
                // Get Data Status and Tag
                tc->data_available = ci_get_tpdu_status(tpdu) & CI_DATA_INDICATOR;
                tc->last_response = tpdu->tpdu.info.tag;
            }
            break;
           }
    }
    else
    {
        TRAN_PRINTF("ci_trans_recv_TPDU: Read failed: slot %d, tcid %d\n\n", tc->slot, tc->tcid);
        return ERR_FAILUE;
    }
    return tc->last_response;
}

/* Get TPDU Status: Data Availabel Or Not */
static UINT8 ci_get_tpdu_status(struct ci_tpdu *tpdu)
{
    if (tpdu->size >= 4 && tpdu->tpdu.data[tpdu->size - 4] == TRANS_TAG_SB &&
        tpdu->tpdu.data[tpdu->size - 3] == 2)
    {
        //Get SB_value
        return tpdu->tpdu.data[tpdu->size - 1];
        }
    return 0;
}
