/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_quickscan.h
*
*    Description:
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _WIN_QUICKSCAN_H_
#define _WIN_QUICKSCAN_H_
#ifdef __cplusplus
extern "C"
{
#endif

extern CONTAINER  quicksrch_item1;    /* Scan Mode  by channel or by freq*/
extern CONTAINER  quicksrch_item2;      /* Channel No.*/
extern CONTAINER  quicksrch_item3;        /* ÆµÂÊ*/
extern CONTAINER  quicksrch_item4;        /* ·ûºÅÂÊ*/
extern CONTAINER  quicksrch_item5;        /* QAM */
extern CONTAINER  quicksrch_item6;     /* ÍøÂçËÑË÷*/
extern CONTAINER  quicksrch_item7;        /* ¿ªÊ¼ËÑË÷ */

extern TEXT_FIELD   quicksrch_txt1;
extern TEXT_FIELD   quicksrch_txt2;
extern TEXT_FIELD   quicksrch_txt3;
extern TEXT_FIELD   quicksrch_txt4;
extern TEXT_FIELD   quicksrch_txt5;
extern TEXT_FIELD   quicksrch_txt6;
extern TEXT_FIELD   quicksrch_txt7;  /* ¿ªÊ¼ËÑË÷ */

extern MULTISEL    quicksrch_sel1;  /* Scan Mode  by channel or by freq*/
extern MULTISEL    quicksrch_sel2;      /* Channel No.*/
extern EDIT_FIELD  quicksrch_edt3;  /* ÆµÂÊ*/
extern EDIT_FIELD  quicksrch_edt4;  /* ·ûºÅÂÊ*/
extern MULTISEL    quicksrch_sel5;  /* QAM */
extern MULTISEL    quicksrch_sel6;  /* ÍøÂçËÑË÷*/

#ifdef __cplusplus
}  /* End of the 'extern "C"' block */
#endif
#endif
