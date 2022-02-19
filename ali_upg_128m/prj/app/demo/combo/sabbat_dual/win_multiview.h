/****************************************************************************
*
*  ALi (Shanghai) Corporation, All Rights Reserved. 2003 Copyright (C)
*
*  File: win_multiview.h
*
*  Description: m/p
*
*  History:
*      Date                Author             Version       Comment
*      ====                ======             =======       =======
*  1.  2006.05.30      Sunny Yin         0.1.000       Initial
*
****************************************************************************/
#ifndef _WIN_MULTIVIEW_H_
#define _WIN_MULTIVIEW_H_

#define    MULTI_VIEW_X_COUNT        3
#define    MULTI_VIEW_COUNT            9

//#define    MULTI_VIEW_X_COUNT        2
//#define    MULTI_VIEW_COUNT            4

//OSD Frame Buffer position
#define    OSD_MULTI_LEFT            0
#define    OSD_MULTI_TOP            0
#define    OSD_MULTI_WIDTH            1110
#define    OSD_MULTI_HEIGHT        620//    384//(OSD_PAL == (x)) ? 510 : 426

#define    MV_MP_WIDTH            1110
#define    MV_MP_HEIGHT            620//384//(OSD_PAL == (x)) ? 510 : 426

#define    CNT_MV_L        (85)
#define    CNT_MV_T        (50)
#define    CNT_MV_W        1110
#define    CNT_MV_H        620//384

#if(9 == MULTI_VIEW_COUNT)
//vidoe position
#define    MV_VIDEO_LEFT                (CNT_MV_L+6)
#define    MV_VIDEO_TOP                (CNT_MV_T+4)

#define    MV_VIDEO_WIDTH                (370-2)//WIN_MV_RECT_WIDTH
#define    MV_VIDEO_HEIGHT                (207-38)//(WIN_MV_RECT_HEIGHT + WIN_MV_TXT_HEIGHT -2)
//#define    MV_VIDEO_HEIGHT                144//(WIN_MV_RECT_HEIGHT + WIN_MV_TXT_HEIGHT -2)
#define    TXT_MV_X_SHIFT            370//210//WIN_MV_RECT_WIDTH
#define    TXT_MV_Y_SHIFT            207//(WIN_MV_RECT_HEIGHT + WIN_MV_TXT_HEIGHT -2)

//Matrix Box
#define    MTB_MV_L        (CNT_MV_L+6)
#define    MTB_MV_T        (CNT_MV_T+TXT_MV_Y_SHIFT-38+4)
#define    MTB_MV_W        (TXT_MV_X_SHIFT-6)
#define    MTB_MV_H        38

#endif

//clock window
#define    BMP_LOAD_W            60
#define    BMP_LOAD_H            60
#define    BMP_LOAD_L            (MV_VIDEO_LEFT+MV_VIDEO_WIDTH/2-30)
#define    BMP_LOAD_T            (MV_VIDEO_TOP+MV_VIDEO_HEIGHT/2-30)

//signal bmp
#define    BMP_SIG_W            42
#define    BMP_SIG_H            30
#define    BMP_SIG_L            (MV_VIDEO_LEFT+MV_VIDEO_WIDTH/2-21)//((TXT_MV_X_SHIFT - BMP_SIG_W)>>1)
#define    BMP_SIG_T            BMP_LOAD_T//(MV_VIDEO_TOP+MV_VIDEO_HEIGHT/2-15)//10

#define    TXT_L            (BMP_SIG_L-(TXT_W-BMP_SIG_W)/2)
#define    TXT_T            (BMP_SIG_T+BMP_SIG_H)//10
//#define    TXT_L            BMP_LOAD_L//10
//#define    TXT_T            (BMP_LOAD_T+BMP_LOAD_H)//10
#define    TXT_W            (BMP_LOAD_W*3)//10
#define    TXT_H            36//28//10


#define    RESET_CALLBACK                0xFFFF
#define    INIT_CALLBACK                    0xFFF
#endif
