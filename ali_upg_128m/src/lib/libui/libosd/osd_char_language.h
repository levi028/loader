/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: osd_char_language.h
*
*    Description: implement osd draw speical language(Inherited from osd lib).
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _OSD_CHAR_LANGUAGE_H_
#define _OSD_CHAR_LANGUAGE_H_
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef PERSIAN_SUPPORT
void xformer_capture_semaphore(void);
void xformer_release_semaphore(void);
void arabic_unistr_xformer(unsigned short* uni_str,BOOL convert_flag,BOOL mb_flag);
#endif

#ifdef __cplusplus
}
#endif
#endif


