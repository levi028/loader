/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2007 Copyright (C)
 *
 *  File: pvr_mgr_task.c
 *
 *  Description: This file describes pvr manager task.
 *
 *  History:
 *      Date            Author          Version         Comment
 *      ====            ======          =======         =======
 *  1.  2009.3.20       Dukula_Zhu      0.0.1
 *
 ****************************************************************************/
#include "pvr_mgr_data.h"

static ppvr_mgr_tsk_info l_pvr_mgr_tsk_info = NULL;

static INT32 __pvr_mgr_send_cmd(ppvr_mgr_cmd cmd, UINT32 tmout)
{
    INT32 ret = -1;

    if (l_pvr_mgr_tsk_info->msgbuf_id != OSAL_INVALID_ID)
    {
        ret = osal_msgqueue_send(l_pvr_mgr_tsk_info->msgbuf_id, cmd, sizeof(pvr_mgr_cmd), tmout);
    }
    return ret;
}

static INT32 __pvr_mgr_wait_cmd_finish(UINT32 cmd_bit, UINT32 tmout)
{
    INT32 ret = -1;
    UINT32 flgptn_after = 0;

    PDEBUG("%s %d\n", __FUNCTION__, cmd_bit);
    if (l_pvr_mgr_tsk_info->flag_id != OSAL_INVALID_ID)
    {
        ret = osal_flag_wait(&flgptn_after, l_pvr_mgr_tsk_info->flag_id, cmd_bit, OSAL_TWF_ANDW | OSAL_TWF_CLR, tmout);
    }
    return ret;
}

void _pvr_mgr_declare_cmd_finish(UINT32 cmd_bit)
{
    if (l_pvr_mgr_tsk_info->flag_id != OSAL_INVALID_ID)
    {
        osal_flag_set(l_pvr_mgr_tsk_info->flag_id, cmd_bit);
    }
    PDEBUG("%s %d\n", __FUNCTION__, cmd_bit);
}

static int __pvr_mgr_find_task(OSAL_ID tid)
{
    int i = -1;

    //1. if found, return the index of the task id!
    for (i = 0; i < PVR_MGR_CALLER_DEPTH; i++)
    {
        if (l_pvr_mgr_tsk_info->caller_task_ids[i] == tid)
        {
            return i;
        }
    }
    //2. else if there is empty! add it and return the pos
    for (i = 0; i < PVR_MGR_CALLER_DEPTH; i++)
    {
        if ((0 == l_pvr_mgr_tsk_info->caller_task_ids[i])
            || (OSAL_INVALID_ID == l_pvr_mgr_tsk_info->caller_task_ids[i]))
        {
            break;
        }
    }
    if (PVR_MGR_CALLER_DEPTH == i)
    {
        PERROR("No room for the task id = %d\n", tid);
        i = 0;
    }
    else
    {
        PDEBUG("add a new task id = %d @ %d\n", tid, i);
        l_pvr_mgr_tsk_info->caller_task_ids[i] = tid;
    }
    return i;
}

INT32 _pvr_mgr_send_message(PVR_HANDLE handle, UINT32 msg, UINT16 param16,
                            UINT32 param32, UINT32 param32_ext, BOOL sync)
{
    UINT32  result = 0;
    int id = -1;
    UINT32  flag = 0;
    pvr_mgr_cmd n_cmd;

    PDEBUG("#### Send message: 0x%X\n", msg);
    //get the task id
    id = osal_task_get_current_id();
    if (OSAL_INVALID_ID == id)
    {
        id = 0;
    }
    flag = 1 << (__pvr_mgr_find_task(id) + 20);
    MEMSET(&n_cmd, 0, sizeof(pvr_mgr_cmd));
    n_cmd.cmd = msg;
    n_cmd.param16 = param16;
    n_cmd.param32 = param32;
    n_cmd.param32_ext = param32_ext;
    n_cmd.p_result = &result;
    n_cmd.handle = handle;
    if (sync)
    {
        n_cmd.flag = flag;
    }
    n_cmd.sync = sync;
    if (SUCCESS != __pvr_mgr_send_cmd(&n_cmd, OSAL_WAIT_FOREVER_TIME))
    {
        PERROR("pvr send message error!\n");
        return result;
    }
    if (sync)
    {
        __pvr_mgr_wait_cmd_finish(flag, OSAL_WAIT_FOREVER_TIME);
    }
    return result;
}

void _pvr_mgr_frame(void)
{
    UINT32 cmd_size = 0;
    pvr_mgr_cmd cmd_in_queue;
    INT32 tmo = 10;

    while (1)
    {
        MEMSET(&cmd_in_queue, 0, sizeof(pvr_mgr_cmd));
        if ((l_pvr_mgr_tsk_info->msgbuf_id != OSAL_INVALID_ID) &&
            (E_OK == osal_msgqueue_receive((VP)&cmd_in_queue, (INT *)&cmd_size, l_pvr_mgr_tsk_info->msgbuf_id, tmo)))
        {
            PDEBUG("pvr_frame deal cmd %d\n", cmd_in_queue.cmd);
            _pvr_mgr_cmd_main(&cmd_in_queue);
        }
        else
        {
            osal_task_sleep(100);
        }
        if (OSAL_INVALID_ID == l_pvr_mgr_tsk_info->msgbuf_id)
        {
            break;
        }
    }
    PDEBUG("pvr_task exit!\n");
}

INT32 _pvr_mgr_task_delete(void)
{
    INT32 ret = SUCCESS;
    UINT16 i = 0;

    for (i = 0; i < PVR_MGR_CALLER_DEPTH; i++)
    {
        l_pvr_mgr_tsk_info->caller_task_ids[i] = OSAL_INVALID_ID;
    }
    osal_task_delete(l_pvr_mgr_tsk_info->task_id);
    l_pvr_mgr_tsk_info->task_id = OSAL_INVALID_ID;
    osal_flag_delete(l_pvr_mgr_tsk_info->flag_id);
    l_pvr_mgr_tsk_info->flag_id = OSAL_INVALID_ID;
    osal_msgqueue_delete(l_pvr_mgr_tsk_info->msgbuf_id);
    l_pvr_mgr_tsk_info->msgbuf_id = OSAL_INVALID_ID;
    l_pvr_mgr_tsk_info = NULL;
    return ret;
}

INT32 _pvr_mgr_task_init(void)
{
    INT32 ret = SUCCESS;
    UINT16 i = 0;
    OSAL_T_CTSK task;
    T_CMBF t_cmbf;

    l_pvr_mgr_tsk_info = &g_pvr_mgr_info->task_info;
    for (i = 0; i < PVR_MGR_CALLER_DEPTH; i++)
    {
        l_pvr_mgr_tsk_info->caller_task_ids[i] = OSAL_INVALID_ID;
    }
    //init manager task/flag/...
    l_pvr_mgr_tsk_info->flag_id = osal_flag_create(0);
    PDEBUG("pvr_flag_id = %d\n", l_pvr_mgr_tsk_info->flag_id);
    if (OSAL_INVALID_ID == l_pvr_mgr_tsk_info->flag_id)
    {
        PDEBUG("PVR: cre_flag pvr_flag_id failed in %s\n", __FUNCTION__);
        ret = ERR_FAILUE;
        return ret;
    }
    MEMSET(&t_cmbf, 0, sizeof(T_CMBF));
    MEMSET(&task, 0, sizeof(OSAL_T_CTSK));
    t_cmbf.bufsz =  sizeof(pvr_mgr_cmd) * 20;
    t_cmbf.maxmsz = sizeof(pvr_mgr_cmd);
    l_pvr_mgr_tsk_info->msgbuf_id = osal_msgqueue_create(&t_cmbf);
    PDEBUG("pvr_msgbuf_id = %d\n", l_pvr_mgr_tsk_info->msgbuf_id);
    if (OSAL_INVALID_ID == l_pvr_mgr_tsk_info->msgbuf_id)
    {
        PDEBUG("PVR: cre_mbf pvr_msgbuf_id failed in %s\n", __FUNCTION__);
        ret = ERR_FAILUE;
        return ret;
    }
    task.task = (FP)_pvr_mgr_frame;
    task.stksz = 0x4000;
    task.quantum = 10;
    task.itskpri = OSAL_PRI_NORMAL;
    task.name[0] = 'P';
    task.name[1] = 'M';
    task.name[2] = 'g';
    l_pvr_mgr_tsk_info->task_id = osal_task_create(&task);
    PDEBUG("pvr_task_id = %d\n", l_pvr_mgr_tsk_info->task_id);
    if (OSAL_INVALID_ID == l_pvr_mgr_tsk_info->task_id)
    {
        PDEBUG("PVR: cre_tsk pvr_task_id failed in %s\n", __FUNCTION__);
        ret = ERR_FAILUE;
    }
    return ret;
}


