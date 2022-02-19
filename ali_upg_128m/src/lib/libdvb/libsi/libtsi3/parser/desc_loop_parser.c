/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: desc_loop_parser.c
*
*    Description: loop parse descriptor in transport stream
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <types.h>
#include <sys_config.h>
#include <api/libtsi/si_types.h>
#include <api/libtsi/si_descriptor.h>
#include <api/libsi/desc_loop_parser.h>

struct descriptor
{
    UINT8 tag;
    UINT8 len;
    UINT8 data[0];
}__attribute__((packed));

#ifdef PVR_DYNAMIC_PID_MONITOR_SUPPORT
INT32 rec_desc_loop_parser(UINT32 param,UINT8 *data, INT32 len,
    struct desc_table *info, INT32 nr, UINT32 *stat, void *priv)
{
    INT16 i        = -1;
    INT16 l        = -1;
    INT16 h        = -1;
    INT16 m        = -1;
    INT32 desc_len = -1;
    INT32 ret      = -1;
    struct descriptor *desc = NULL;
    struct rec_desc_table *current = NULL;

    if((NULL == data) || (NULL == info) || (NULL == priv))
    {
        return !SI_SUCCESS;
    }

    for(i=0; i<len; i += sizeof(struct descriptor)+desc_len)
    {
        desc = (struct descriptor *)(data+i);
        desc_len = desc->len;
        l = -1;
        h = nr;
        do
        {
            m = (l+h)>>1;
            current = (struct rec_desc_table *)(info+m);
            if (current->tag>desc->tag)
            {
                h = m;
            }
            else if(current->tag<desc->tag)
            {
                l = m;
            }
            else
            {
                if (stat)
                {
                    *stat |= (1<<current->bit);
                }
                if (current->parser)
                {
                    ret = current->parser(param,desc->tag,desc->len, desc->data, priv);
                    if (ret != SI_SUCCESS)
                    {
                        return ret;
                    }
                }
                break;
            }
        }
        while (h-l>1);
    }
    return SI_SUCCESS;
}

#endif

INT32 desc_loop_parser(UINT8 *data, INT32 len, struct desc_table *descriptor_table,
    INT32 desc_num_in_table, UINT32 *stat, void *priv)
{
    INT16 i        = 0;
    INT16 k        = 0;
    INT32 desc_len = 0;
    INT32 ret      = 0;
    struct descriptor *desc = NULL;

    if((NULL == data) || (NULL == descriptor_table) || (NULL == priv))
    {
        return !SI_SUCCESS;
    }

    for(i=0; i<len; i += sizeof(struct descriptor)+desc_len)
    {
        desc = (struct descriptor *)(data+i);
        desc_len = desc->len;

        /* Use SEQUENTIAL SEARCH is more safer because BINARY SEARCH rely on the tag value
         * INCREASE in descriptor_table, but not all this function's users know about this.
         * And, the number of elements in descriptor table always small. */
        for (k=0; k<desc_num_in_table; k++)
        {
            /* check current descriptor received whether is a wanted one in table */
            if (desc->tag == descriptor_table[k].tag)
            {
                if (stat)
                {
                    *stat |= (1<<descriptor_table[k].bit);
                }

                if (descriptor_table[k].parser)
                {
                    ret = descriptor_table[k].parser(desc->tag, desc->len, desc->data, priv);

                    if (ret != SI_SUCCESS)
                    {
                        return ret;
                    }
                }
            }
        }
    }
    return SI_SUCCESS;
}

INT32 si_descriptor_parser(UINT8 *data, INT32 length, struct descriptor_info *info, INT32 nr)
{
    INT32 pos = 0;
    INT32 i   = 0;
    INT32 k   = 0;
    INT32 ret = 0;
    struct descriptor *desc = NULL;

    if((NULL == data) || (NULL == info))
    {
        return !SI_SUCCESS;
    }

    for(pos = 0; pos<length; pos+= desc->len+sizeof(struct descriptor))
    {
        desc = (struct descriptor *)(data+pos);
        for(k=0; k<nr; k++)
        {
            if ((desc->tag == info[k].descriptor_tag)&&(info[k].mask_len<=desc->len))
            {
                for(i=0; i<info[k].mask_len; i++)
                {
                    if ((desc->data[i]&info[k].mask[i]) != (info[k].value[i]&info[k].mask[i]))
                    {
                        break;
                    }
                }

                if (i == info[k].mask_len)
                {
                    ret = info[k].on_descriptor(desc->tag,desc->len, desc->data, info->priv);

                    if (ret != SI_SUCCESS)
                    {
                        return ret;
                    }

                    break;
                }
            }
        }
    }
    return SI_SUCCESS;
}


