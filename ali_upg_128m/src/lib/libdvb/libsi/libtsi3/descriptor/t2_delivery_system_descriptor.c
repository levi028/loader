/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: t2_delivery_system_descriptor.c
*
*    Description: parse T2 delivery system descriptor
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <types.h>
#include <sys_config.h>
#include <api/libtsi/si_types.h>
#include <api/libtsi/si_descriptor.h>
#include <api/libsi/si_module.h>
#include <api/libsi/si_nit.h>
#include <api/libsi/si_utility.h>
#include <api/libsi/t2_delivery_system_descriptor.h>

#if defined(DVBT2_SUPPORT)

typedef struct
{
//  UINT8 descriptor_tag;           //0x7F
//  UINT8 descriptor_length;        //>=4 bytes

    UINT8 descriptor_tag_extension; //0x04

    UINT8 plp_id;
    UINT8 t2_system_id[2];

//  Other parameters for (descriptor_length > 4) are regardless now.
}__attribute__((packed)) t2_delivery_system_descriptor;


//#define DVBT2_PRINTF          libc_printf
#define DVBT2_PRINTF(...)       do{}while(0)

INT32 t2_delivery_system_descriptor_parser(UINT8 tag, UINT8 length, UINT8 *data, void *priv)
{
    UINT8 min_length = 4;
    UINT16 i = 0;
    UINT16 t2_system_id = 0;
    struct nit_section_info *n_info = NULL;
    t2_delivery_system_descriptor *t2_desc = NULL;

    if((data == NULL) || (priv == NULL))
    {
        return !SI_SUCCESS;
    }

    n_info = (struct nit_section_info *)priv;
    t2_desc = (t2_delivery_system_descriptor *)data;

    if ((tag != EXTENSION_DESCRIPTOR) || (length < min_length) || \
        (t2_desc->descriptor_tag_extension != T2_DELIVERY_SYSTEM_DESCRIPTOR))
    {
        return SI_PERROR;
    }

    t2_system_id = (t2_desc->t2_system_id[0]<<8) | (t2_desc->t2_system_id[1]);

    for(i=0; i<n_info->t2_info_num; ++i)
    {
        if( n_info->t2_info[i].plp_id == t2_desc->plp_id && n_info->t2_info[i].t2_system_id == t2_system_id )
        {
            break;              //The mapping is exist already.
        }
    }

    if(i == n_info->t2_info_num)
    {
        n_info->t2_info_num++;  //Append a mapping.
    }
    else
    {
        DVBT2_PRINTF("Duplicate mapping t2_info[%d]: onid = 0x%X, tsid = 0x%X, plp_id = 0x%X, t2_system_id = 0x%X\n",
                    i,
                    n_info->t2_info[i].onid,
                    n_info->t2_info[i].tsid
                    n_info->t2_info[i].plp_id,
                    n_info->t2_info[i].t2_system_id,
                    );
    }

        n_info->t2_info[i].plp_id = t2_desc->plp_id;
        n_info->t2_info[i].t2_system_id = t2_system_id;
        n_info->t2_info[i].onid = n_info->onid;
        n_info->t2_info[i].tsid = n_info->tsid;
        DVBT2_PRINTF("t2_info[%d]: onid = 0x%X, tsid = 0x%X, plp_id = 0x%X, t2_system_id = 0x%X\n",
                    i,
                    n_info->t2_info[i].onid,
                    n_info->t2_info[i].tsid
                    n_info->t2_info[i].plp_id,
                    n_info->t2_info[i].t2_system_id,
                    );

    return SI_SUCCESS;
}

#endif

