/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
*    disclosed to unauthorized individual.
*    File:     fast_crc.h
*
*    Description:This file contains all functions definition of
*                Fast CRC code/decode functions.
*    NOTE: The type of int must be 32bits weight.
*
*    History:
*           Date            Athor        Version          Reason
*        ============    =============    =========    =================
*    1.    2000.7.13       Justin Wu       Ver 0.1    Create file.
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __MG_CRC_H
#define __MG_CRC_H

#ifdef __cplusplus
extern "C"
{
#endif

/********************************** Var announce *************************************/
/* FCS fast lookup table:    in    "fastCRC.c"*/
//extern unsigned int MG_CRC_Table[256];

/********************************** Function announce *******************************/
/* From "len" length "buffer" data get CRC, "crc" is preset value of crc*/
unsigned int mg_compute_crc(register unsigned int crc,register unsigned char *bufptr,register int len);

/* Setup fast CRC compute table*/
void mg_setup_crc_table(void);

/* Fast CRC compute*/
unsigned int mg_table_driven_crc(register unsigned int crc,register unsigned char *bufptr,register int len);

/* FCS coder */
void mg_fcs_coder(unsigned char *puc_in_data,int len);

/* FCS decoder */
int mg_fcs_decoder(unsigned char *puc_in_data,int len);

#ifdef __cplusplus
}
#endif

#endif  /* __MG_CRC_H */
/**************************************** END *************************************/
