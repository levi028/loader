
#ifndef __SYS_MEM_MAP_H
#define __SYS_MEM_MAP_H

#if ( SYS_SDRAM_SIZE == 2 )

#elif ( SYS_SDRAM_SIZE == 4 )

#elif (SYS_SDRAM_SIZE == 16)

#elif (SYS_SDRAM_SIZE == 32)

#elif (SYS_SDRAM_SIZE == 64)

#elif (SYS_SDRAM_SIZE == 128)
#ifndef DRAM_SPLIT
#error "DRAM_SPLIT macro error!!"
#else //#ifndef DRAM_SPLIT
#ifndef VFB_SUPPORT
#error "VFB_SUPPORT macro error!!"
#else// VFB_SUPPORT
//cas9 or VFB enable
/* memory mapping

== limitations
1. AVC and MPEG2 decoder both use about 30MB memory, MPEG2 requires one 32MB segment,
    so we put decoder buffer between [64MB, 96MB], start from 96MB and go down.
2. DMX dma buffer should under same 8M segment.
    __MM_DMX_AVP_START_ADDR, and __MM_DMX_REC_START_ADDR, must under same 8M segment,

    [94M,128M]  34M -- cpu memory -- video frame buffer
    [0x17B0400, 94M] 124M -- cpu memory -- private memory
    [0xB0400 , 0x17B0400] 23M -- see memory -- SEE SW
    [0x30400 , 0xB0400] 512KB -- see memory -- SEE BL.
    [0x400 , 0x30400] 192KB -- see memory -- VSC buffer.
    [0  ,0x400] 1KB --share memory -- 

*/
//#define AVC_SUPPORT_UNIFY_MEM
#ifdef SD_PVR
#ifdef AVC_SUPPORT_UNIFY_MEM
#undef AVC_SUPPORT_UNIFY_MEM
#endif
#endif

#ifdef AVC_SUPPORT_UNIFY_MEM
#ifdef MP_PREVIEW_SWITCH_SMOOTHLY
#undef MP_PREVIEW_SWITCH_SMOOTHLY
#endif

#ifdef DYNAMIC_RESOLUTION_SUPPORT
#undef DYNAMIC_RESOLUTION_SUPPORT
#endif

#define AVC_MEM_LEN		0x15bb000//0x1898400
#endif

#define __MM_RESERV_SECIAL_BUF_LEN  0x400 
#define __MM_SHARED_MEM_LEN         0x400
#define __MM_VSC_BUF_LEN            0x30000
#define __MM_SEE_BL_BUF_LEN         0x80000
#define __MM_SEE_MP_BUF_LEN         0      //0x00900000    //9M

#define __MM_PRIVATE_SHARE_LEN	0x17B0000//0x02000000//0x01e00000	//30M
#define __MM_TTXSUBT_S_LEN      0

#define __MM_VOID_BUFFER_LEN	0//0x00200000	//2M
	
#define __MM_PRIVATE_LEN        (__MM_PRIVATE_SHARE_LEN - __MM_SHARED_MEM_LEN)

/*******************************************************NIM BUFFER LEN************************************************************/
#ifdef DVBC_SUPPORT
#define __MM_NIM_DVBC_BUFFER_LEN         0x0// 0x200000//2M   No need J83B
#else
#define __MM_NIM_DVBC_BUFFER_LEN         0x0
#endif

#ifdef DVBT2_SUPPORT
#define __MM_NIM_DVBT2_TIME_DEINT_BUFFER_LEN         0x400000//4M
#define __MM_NIM_DVBT2_DJB_BUFFER_LEN                0x96000//600K
#else 
#define __MM_NIM_DVBT2_TIME_DEINT_BUFFER_LEN         0x0
#define __MM_NIM_DVBT2_DJB_BUFFER_LEN                0x0
#endif
#define __MM_NIM_DVBT2_BUFFER_LEN (__MM_NIM_DVBT2_TIME_DEINT_BUFFER_LEN + __MM_NIM_DVBT2_DJB_BUFFER_LEN)

#define __MM_NIM_BUFFER_LEN  (__MM_NIM_DVBT2_BUFFER_LEN + __MM_NIM_DVBC_BUFFER_LEN)


/*************************FSC BUFFER*******************/
#ifdef FSC_SUPPORT
    #ifdef FSC_SHARE_WITH_PVR            
    #define __MM_DMX_MAIN_BLK_LEN     ((47*1024)*87)// should align with 188*16//(0x400000)
    #else
    #define __MM_DMX_MAIN_BLK_LEN     ((47*1024)*60)
    #endif 

    #ifdef FSC_DMA_BUFFER_CACHE
    #define FSC_CACHE_BUFFER_PART1_LEN     (__MM_DMX_MAIN_BLK_LEN*2) 
    #else
    #define FSC_CACHE_BUFFER_PART1_LEN     (__MM_DMX_MAIN_BLK_LEN*1)
    #endif
#else
    #define __MM_DMX_MAIN_BLK_LEN       0
    #define FSC_CACHE_BUFFER_PART1_LEN  0
#endif

/************************************************************PVR BUFFER LEN*********************************************************/
#ifdef SD_PVR
    #define __MM_PVR_VOB_BUFFER_LEN        ((47*1024)*(75*2)+0x1000- __MM_NIM_BUFFER_LEN)//0x88a800//0x6fb000
#else
    #ifdef SAT2IP_SUPPORT
        #define __MM_MEDIA_PROV_BUFFER_LEN     0x400000
    #else
        #define __MM_MEDIA_PROV_BUFFER_LEN     0
    #endif
    #ifdef _AUI_
        /*As the MP module is sharing memory with PVR module, and the memory of 
        MP module will allocate 10M to SBM which is used to cache data of video file 
        in driver, and as the memory size of SBM can just use 6M to ensure the video 
        file play success, so the memory size of MP module can be reduced by 4M, 
        that is, the memory size of PVR module can be reduced by 4M.The heap space 
        of aui version will increase 4M.*/
                    
        /*the DMX module share its memory with media player module, 
        which can increase 3.23M memory size of MP module. As the MP module 
        needs a lot of memory, the PVR module is also share its memory with MP 
        module. As the DMX module dedicates 3.23M to MP module, the PVR module 
        can reduce the dedication of 3.23M. The heap space of aui version will increase 3.23M*/
	    #define __MM_PVR_VOB_BUFFER_LEN (((47*1024)*(150*3)+0x1000) - __MM_MEDIA_PROV_BUFFER_LEN - __MM_NIM_BUFFER_LEN - (4*1024*1024) \
                                                                                 - __MM_SI_VBV_OFFSET - __MM_DMX_DATA_LEN - __MM_DMX_AVP_LEN - __MM_DMX_REC_LEN - __MM_DMX_BLK_BUF_LEN)	
	#else
		#define __MM_PVR_VOB_BUFFER_LEN (((47*1024)*(150*2)+0x1000) - __MM_MEDIA_PROV_BUFFER_LEN - __MM_NIM_BUFFER_LEN - FSC_CACHE_BUFFER_PART1_LEN)	
	#endif
#endif

/************************************************************DECODER BUFFER*************************************************************/
#define __MM_FB_LEN_DEVIDED        0X32A000//(HD:1920X1152X1.5)
#ifndef SD_PVR
    #ifdef VIDEO_DEBLOCKING_SUPPORT
	#define __MM_FB_LEN			    0x10D8800//0x10CF200//0x19c6200
    #else
	#define __MM_FB_LEN			    0xddb800//0xdd2200//0XCA8000//0X9B4000//0X26D000//(16*SD>3*HD)
    #endif
    #define __MM_MAF_LEN            0x198C00//0xd0000//0X3000//((FLAG==4*4*3K) + VALUE ==120*72*32 *2(Y+C)>46*36*32 *2(Y+C) *4 )
#else
    #ifdef VIDEO_DEBLOCKING_SUPPORT
    #define __MM_FB_LEN                0x61BC00//0x10CF200//0x19c6200
    #else
    #define __MM_FB_LEN                0x308400//0xdd2200//0XCA8000//0X9B4000//0X26D000//(16*SD>3*HD)
    #endif
    #define __MM_MAF_LEN            0X8C000//0x198C00//0xd0000//0X3000//((FLAG==4*4*3K) + VALUE ==120*72*32 *2(Y+C)>46*36*32 *2(Y+C) *4 )
#endif
#define __MM_DVW_LEN            0

#define MAX_EXTRA_FB_NUM 3
#ifndef SD_PVR
    #define MAX_MB_WIDTH 120 //(1920/16)
    #define MAX_MB_HEIGHT 68 //(1088/16)
#else
    #define MAX_MB_WIDTH 46//45 //(720/16)
    #define MAX_MB_HEIGHT 36//36 //(576/16)
#endif

#ifdef H264_SUPPORT_MULTI_BANK
    #ifndef SD_PVR
        #define MAX_MB_STRIDE 120 //120 MB alignment to improve AVC performance
    #else
        #define MAX_MB_STRIDE 46//46 //120 MB alignment to improve AVC performance
    #endif
#define EXTRA_FB_SIZE 0x2000
#define ONE_FB_SIZE (((MAX_MB_STRIDE*MAX_MB_HEIGHT*256*3/2+EXTRA_FB_SIZE-1)&0xffffe000)+EXTRA_FB_SIZE)
#else
    #ifndef SD_PVR
        #define MAX_MB_STRIDE 120 //120 MB alignment to improve AVC performance
    #else
        #define MAX_MB_STRIDE 46//46 //120 MB alignment to improve AVC performance
    #endif

    #ifdef SD_PVR
        #define one_frm_y_size         (MAX_MB_STRIDE*((MAX_MB_HEIGHT+1)/2)*512)
        #define one_frm_c_size   (MAX_MB_STRIDE*((((MAX_MB_HEIGHT+1)/2)+1)/2)*512)
        #define ONE_FB_SIZE (one_frm_y_size + one_frm_c_size)
    #else
        #define ONE_FB_SIZE (MAX_MB_STRIDE*MAX_MB_HEIGHT*256*3/2)
    #endif

#endif

#define EXTRA_DV_FB_SIZE 0x3C00
#define ONE_DV_FB_SIZE (((MAX_MB_WIDTH*MAX_MB_HEIGHT*256*3/2)/4) + EXTRA_DV_FB_SIZE)
#define ONE_MV_SIZE 32*(MAX_MB_WIDTH*MAX_MB_HEIGHT) //522240

#ifdef DYNAMIC_RESOLUTION_SUPPORT
#ifndef SD_PVR
#define AVC_FB_LEN		ONE_FB_SIZE*(5+MAX_EXTRA_FB_NUM) //0x1700000

#ifndef MP_PREVIEW_SWITCH_SMOOTHLY
#define AVC_DVIEW_LEN   0
#else
#define AVC_DVIEW_LEN   ONE_DV_FB_SIZE*(5+MAX_EXTRA_FB_NUM) //0xb00000
#endif

#define AVC_MV_LEN		ONE_MV_SIZE*(5+MAX_EXTRA_FB_NUM) //0x37c800//0x2FD000
#else
#define const_frm_num   5
#define AVC_FB_LEN		ONE_FB_SIZE*(const_frm_num+MAX_EXTRA_FB_NUM) //0x1700000
#define AVC_DVIEW_LEN   0//  ONE_DV_FB_SIZE*(4+MAX_EXTRA_FB_NUM) //0xb00000
#define AVC_MV_LEN		ONE_MV_SIZE*(const_frm_num+MAX_EXTRA_FB_NUM) //0x37c800//0x2FD000
#endif //end of SD_PVR
#define AVC_MB_COL_LEN		0x18000 //0x11000
#else
#ifndef SD_PVR
#define AVC_FB_LEN      ONE_FB_SIZE*(4+MAX_EXTRA_FB_NUM) //0x1700000

#ifndef MP_PREVIEW_SWITCH_SMOOTHLY
#define AVC_DVIEW_LEN   0
#else
#define AVC_DVIEW_LEN   ONE_DV_FB_SIZE*(4+MAX_EXTRA_FB_NUM) //0xb00000
#endif

#define AVC_MV_LEN      ONE_MV_SIZE*(4+MAX_EXTRA_FB_NUM) //0x37c800//0x2FD000
#define AVC_MB_COL_LEN  0x15000
#else
#define const_frm_num   5
#define AVC_FB_LEN      ONE_FB_SIZE*(const_frm_num+MAX_EXTRA_FB_NUM) //0x1700000
#define AVC_DVIEW_LEN   0//  ONE_DV_FB_SIZE*(4+MAX_EXTRA_FB_NUM) //0xb00000
#define AVC_MV_LEN      ONE_MV_SIZE*(const_frm_num+MAX_EXTRA_FB_NUM) //0x37c800//0x2FD000
#define AVC_MB_COL_LEN  0x18000 // 0x11000 will cause overflow
#endif //end of SD_PVR
#endif // end of DYNAMIC_RESOLUTION_SUPPORT

#define AVC_MB_NEI_LEN        0xf000
#define AVC_CMD_QUEUE_LEN   0x10000 //In allegro test stream, this length could be bigger than 128k, however, in realistic, 0x10000 should be enough
#undef  AVC_LAF_RW_BUF_LEN
//#define AVC_LAF_RW_BUF_LEN ((MAX_MB_WIDTH*MAX_MB_HEIGHT)*32*2*2)
#define AVC_LAF_RW_BUF_LEN (((((MAX_MB_WIDTH*MAX_MB_HEIGHT)*48*2)+1023)&0x0ffffc00)*2)

//#define AVC_LAF_FLAG_BUF_LEN (0xc00*21)
#define AVC_LAF_FLAG_BUF_LEN (0xc00*22) //when enable dual output, we need 1 more laf buffer

#ifndef SD_PVR
    #define AVC_VBV_LEN        0x400000 //for CI+, must use 4MB. if this size is set to 0x200000, vbv buffer overflow could happen in 20061007d_0.ts
#else
    #define AVC_VBV_LEN        0x180000
#endif

#define HEVC_VBV_LEN           		0// 0x400000
#define HEVC_CMD_QUEUE_LEN      	0//  0x20000
#define HEVC_EP_QUEUE_LEN       	0//  0x20000
#define HEVC_PICTURE_SYNTAX_LEN  	0// 0x5000

#define HEVC_FB_LEN            		0// 0x2020000//0x1820000//0x1B24000//0x1807E00

#ifndef MP_PREVIEW_SWITCH_SMOOTHLY
#define HEVC_DVIEW_LEN          0
#else
#define HEVC_DVIEW_LEN         	 	0//  0x53ac00
#endif

#define HEVC_MV_LEN              0// 0x100000
#define HEVC_MB_NEI_LEN          0// 0xB000
#define HEVC_LAF_LEN             0// 0xD7400 //0x18F000
#define HEVC_INNER_ES_LEN        0// 0x300000 //max:0x733334
#define HEVC_INNER_AUX_LEN       0// 0x39000 

/*****************************************OSD ,DMX,TTX,*********************************************************************************/

#if 1 //ifdef FPGA_TEST
#define __MM_GMA1_LEN            0 // 0x1FA400 // 1920*1080 osd layer1
#define __MM_GMA2_LEN            0 // 0x1FA400//1920*1080  osd layer2
#define __MM_TEMP_BUF_LEN        0 // 0x100 //1920*1080*4 temp resize buffer
#define __MM_CMD_LEN            0 // 0x6DB0 // command buffer
#else
#define __MM_GMA1_LEN            0x1FA400 // 1920*1080 osd layer1
#define __MM_GMA2_LEN            0x1FA400//1920*1080  osd layer2
#define __MM_TEMP_BUF_LEN        0x100 //1920*1080*4 temp resize buffer
#define __MM_CMD_LEN            0x6DB0 // command buffer
#endif
#define __MM_GE_LEN                (__MM_GMA1_LEN+__MM_GMA2_LEN+__MM_TEMP_BUF_LEN+__MM_CMD_LEN) //0xBE45B0
#define __MM_OSD_LEN            0x65400 // 720*576
#ifndef SD_PVR
#ifdef _AUI_
#define __MM_OSD1_LEN           (1280*720*4)//(1280*720*2)//(1280*720*4)
#else
#define __MM_OSD1_LEN           (1280*720*2)//(1280*720*4)
#endif
#define __MM_OSD2_LEN            (1280*720*4)
#else
#ifdef _AUI_
#define __MM_OSD1_LEN           (1280*720*4)//(1008 * 640 * 2 + 256)
#else
#define __MM_OSD1_LEN           (1008 * 640 * 2 + 256)
#endif
#define __MM_OSD2_LEN           (720*576)
#endif

// CNX
#ifdef _AUI_
//AUI project share buffer with __MM_OSD_BK_ADDR2 which configured for SEE TTX/SUBT,
// But now it is no used anymore
#define AD_SEE_OSD2_SHARE_BUFFER
#endif

#ifdef AD_SEE_OSD2_SHARE_BUFFER
#define __MM_AD_DECA_MEM_LEN 0xF0000
#define __MM_AD_SND_MEM_LEN  0x34200
#else
#define __MM_AD_DECA_MEM_LEN 0
#define __MM_AD_SND_MEM_LEN 0
#endif


//#define OSD_VSRC_MEM_MAX_SIZE     0x400000    //300*1024
/*Close the macro, __MM_OSD_VSRC_MEM_ADDR, which is used to auxiliary drawing in OSD
 layer. As the aui do not use this but the GE to auxiliary drawing, closing this macro
 in aui version has no effect for aui, and the heap space of aui will increase 1.25M*/
#ifdef _AUI_
#define OSD_VSRC_MEM_MAX_SIZE     0//0x140000    //300*1024
#else
#define OSD_VSRC_MEM_MAX_SIZE    (1280*720*2) //0x140000    //300*1024
#endif

#ifndef SD_PVR
#define __MM_VBV_LEN            0x500000
#else
#define __MM_VBV_LEN            0x12C000//(HD = 8*SD > 4*SD)
#endif

#define __MM_DMX_SI_LEN            (32*188)//(16*188)
#ifdef SUPPORT_MULTI_SD_VIDEO
    #define EXTRA_VIDEO_NUM 3
    #define __MM_DMX_SI_TOTAL_LEN    (__MM_DMX_SI_LEN*(44-EXTRA_VIDEO_NUM))
#else
    #define __MM_DMX_SI_TOTAL_LEN    (__MM_DMX_SI_LEN*44)
#endif

#define __MM_SI_VBV_OFFSET        __MM_DMX_SI_TOTAL_LEN
#define __MM_DMX_DATA_LEN        (30*188)
#define __MM_DMX_PCR_LEN        (10*188)
#define __MM_DMX_AUDIO_LEN        (256*188)//(32*188)
#define __MM_DMX_VIDEO_LEN        (12*512*188)//(8*512*188)
#ifdef SUPPORT_MULTI_SD_VIDEO
    #define __MM_DMX_EXTRA_VIDEO_LEN (960*188)
    #define __MM_DMX_AVP_LEN        (__MM_DMX_VIDEO_LEN+__MM_DMX_EXTRA_VIDEO_LEN*EXTRA_VIDEO_NUM+__MM_DMX_AUDIO_LEN+__MM_DMX_PCR_LEN)
#else
    #define __MM_DMX_AVP_LEN        (__MM_DMX_VIDEO_LEN+__MM_DMX_AUDIO_LEN+__MM_DMX_PCR_LEN)
#endif
#define __MM_DMX_REC_LEN            (__MM_DMX_VIDEO_LEN)

#ifdef FSC_SUPPORT
#ifdef FSC_DMA_BUFFER_CACHE
//Case 1) use dma buffer to cache fsc data
#define __MM_DMX_BLK_BUF_LEN        0xbc000
#define __MM_DMX_SEE_BLK_BUF_LEN    0xbc000

#define DMX_DMA_BUFFER_LEN          __MM_DMX_MAIN_BLK_LEN
#if(DMX_DMA_BUFFER_LEN < __MM_DMX_AVP_LEN)
    #error "DMX_DMA_BUFFER_LEN too small"
#endif
#undef __MM_DMX_REC_LEN
#undef __MM_DMX_AVP_LEN
#define __MM_DMX_REC_LEN            DMX_DMA_BUFFER_LEN
#define __MM_DMX_AVP_LEN            DMX_DMA_BUFFER_LEN

#define __MM_FSC_BUFFER_LEN         (__MM_DMX_REC_LEN*1+1024)
#else
//Case 2) Use the SW (DMX task)to cache
#define __MM_DMX_BLK_BUF_LEN        __MM_DMX_MAIN_BLK_LEN
#define __MM_DMX_SEE_BLK_BUF_LEN    0xbc000

#define DMX_DMA_BUFFER_LEN          __MM_DMX_AVP_LEN
#define __MM_DMX_REC_LEN		    (__MM_DMX_AVP_LEN)
#define __MM_FSC_BUFFER_LEN         (__MM_DMX_MAIN_BLK_LEN*2 + __MM_DMX_REC_LEN*1+1024)
#endif
#else
//Case 3) Normal, and NO FSC.
#define __MM_DMX_BLK_BUF_LEN        0xbc000
#define __MM_DMX_SEE_BLK_BUF_LEN    __MM_DMX_BLK_BUF_LEN
#define __MM_FSC_BUFFER_LEN         0
#endif


/*The macros, __MM_SUB_BS_START_ADDR, __MM_SUB_HW_DATA_ADDR, 
__MM_SUB_PB_START_ADDR, __MM_DCII_SUB_BS_START_ADDR,
__MM_SUBT_ATSC_SEC_ADDR, of SUBTITLE are not used by aui, so cloing 
these macros in aui has no eddect for aui, and the heap space of aui will increase 0.697M*/
#ifdef _AUI_
#define __MM_SUB_BS_LEN            0//0xA000 //0X2800
#else
#define __MM_SUB_BS_LEN            0// 0xA000
#endif

#ifdef HD_SUBTITLE_SUPPORT
/*The macros, __MM_SUB_BS_START_ADDR, __MM_SUB_HW_DATA_ADDR, 
__MM_SUB_PB_START_ADDR, __MM_DCII_SUB_BS_START_ADDR,
__MM_SUBT_ATSC_SEC_ADDR, of SUBTITLE are not used by aui, so closing 
these macros in aui has no eddect for aui, and the heap space of aui will increase 0.697M*/
	#ifdef _AUI_
    #define __MM_SUB_PB_LEN            0//0xA0000 //0x50000//0X19000(1920X340 for hd subt)
	#else
	#define __MM_SUB_PB_LEN            0//0xA0000 //0x50000//0X19000(1920X340 for hd subt)
	#endif
#else
    #define __MM_SUB_PB_LEN           0//   0X19000
#endif

#ifdef SUPPORT_HW_SUBT_DECODE
#define __MM_SUB_HW_DATA_LEN 0xC000
#else
#define __MM_SUB_HW_DATA_LEN 0
#endif

// TTX
#if (TTX_ON==1)
    #define __MM_TTX_BS_LEN            0x5000//0X2800
    #ifdef TTX_SUB_PAGE
    /*The macros, _MM_TTX_SUB_PAGE_BUF_ADDR, __MM_TTX_P26_NATION_BUF_ADDR, __MM_TTX_P26_DATA_BUF_ADDR, 
	__MM_TTX_BS_START_ADDR, __MM_TTX_PB_START_ADDR of TTX module are not used by aui, so closing these macros
    in aui has no effect for aui. And the heap space of aui will increase 1.28M*/
    #ifdef _AUI_
    #define __MM_TTX_PB_LEN                0//0xCB200 //+80*1040 //0XCA800
    #define __MM_TTX_SUB_PAGE_LEN      0// 0x14500 //80*1040
    #else
    #define __MM_TTX_PB_LEN                0xCB200 //+80*1040 //0XCA800
    #define __MM_TTX_SUB_PAGE_LEN          0x14500 //80*1040
    #endif
    #else
    #define __MM_TTX_PB_LEN                0xCA800 //+80*1040 //
    #define __MM_TTX_SUB_PAGE_LEN       0 //80*1040
    #endif
    #ifdef SUPPORT_PACKET_26
    /*The macros, _MM_TTX_SUB_PAGE_BUF_ADDR, __MM_TTX_P26_NATION_BUF_ADDR, __MM_TTX_P26_DATA_BUF_ADDR, 
	__MM_TTX_BS_START_ADDR, __MM_TTX_PB_START_ADDR of TTX module are not used by aui, so closing these macros
    in aui has no effect for aui. And the heap space of aui will increase 1.28M*/
    #ifdef _AUI_
    #define __MM_TTX_P26_NATION_LEN     0//0x61A80 //25*40*400
    #define __MM_TTX_P26_DATA_LEN       0//0x3E8 //25*40
    #else
    #define __MM_TTX_P26_NATION_LEN     0x61A80 //25*40*400
    #define __MM_TTX_P26_DATA_LEN       0x3E8 //25*40
    #endif
    #else
    #define __MM_TTX_P26_NATION_LEN     0
    #define __MM_TTX_P26_DATA_LEN       0
    #endif
#else
    #define __MM_TTX_BS_LEN             0
    #define __MM_TTX_PB_LEN             0
    #define __MM_TTX_SUB_PAGE_LEN       0
    #define __MM_TTX_P26_NATION_LEN     0
    #define __MM_TTX_P26_DATA_LEN       0

#endif

#if (CC_ON==1)
#define OSD_CC_WIDTH   	  576//480//480//704//480//(CC_CHAR_W*40+16)//560//704(16bytes aligned)
#define OSD_CC_HEIGHT  	  390//360//300//360//300//450//(CC_CHAR_H*15)//500//570
#define CC_CHAR_HEIGHT		26

#define __MM_ATSC_CC_PB_RECT_LEN (OSD_CC_WIDTH*OSD_CC_HEIGHT)
#ifdef _AUI_
#define __MM_ATSC_CC_PB_LEN  0//(__MM_ATSC_CC_PB_RECT_LEN+OSD_CC_WIDTH*4*CC_CHAR_HEIGHT)
#define __MM_ATSC_CC_BS_LEN  0//0x1c00 // 1K Word CC Data, 1K Byte CC Field, 2K Word DTVCC Data
#else
#define __MM_ATSC_CC_PB_LEN  (__MM_ATSC_CC_PB_RECT_LEN+OSD_CC_WIDTH*4*CC_CHAR_HEIGHT)
#define __MM_ATSC_CC_BS_LEN  0x1c00 // 1K Word CC Data, 1K Byte CC Field, 2K Word DTVCC Data
#endif
#else
#undef __MM_ATSC_CC_PB_LEN
#undef __MM_ATSC_CC_BS_LEN
#define __MM_ATSC_CC_PB_LEN	0
#define __MM_ATSC_CC_BS_LEN	0
#endif

#define __MM_USB_DMA_LEN                0    // 0x10FFFF currently not use
#define __MM_EPG_BUFFER_LEN             0x100000
#define __MM_AUTOSCAN_DB_BUFFER_LEN     0x100000

#ifdef NETWORK_SUPPORT
#define STO_PROTECT_BY_MUTEX
#define __MM_LWIP_MEM_LEN               0x10000
#define __MM_LWIP_MEMP_LEN              0xd1f00//0xce200
#define __MM_XML_MEMP_LEN                0x100000 // 5M
#else
#define __MM_LWIP_MEM_LEN               0
#define __MM_LWIP_MEMP_LEN              0
#define __MM_XML_MEMP_LEN                0
#endif

#if(ISDBT_CC)
#define __MM_ISDBTCC_BS_LEN        0x8FC0
#define __MM_ISDBTCC_PB_LEN        0x7E900
#else
#define __MM_ISDBTCC_BS_LEN         0
#define __MM_ISDBTCC_PB_LEN         0
#endif

#ifdef DUAL_VIDEO_OUTPUT_USE_VCAP
#define __MM_VCAP_FB_SIZE               (736*576*2*4)
#else
#define __MM_VCAP_FB_SIZE               0
#endif
#define __MM_DBG_MEM_LEN   0x4000

/*************************************************MEMORY MAP ADDR*************************************************************/

#ifdef SD_PVR
    #define __MM_PRIV_HIGHEST_ADDR   0xA17B0400        //23M   
#else
    #define __MM_PRIV_HIGHEST_ADDR   0xA17B0400        //23M
#endif

#define __MM_SHARE_BASE_ADDR     (0xA0000000 + __MM_RESERV_SECIAL_BUF_LEN)
#define __MM_VSC_BUFFER_ADDR     (__MM_SHARE_BASE_ADDR + __MM_SHARED_MEM_LEN)
#define __MM_SEE_BL_BUFFER_ADDR  (__MM_VSC_BUFFER_ADDR + __MM_VSC_BUF_LEN)
#define __MM_PRIV_BOTTOM_ADDR    (__MM_SEE_BL_BUFFER_ADDR + __MM_SEE_BL_BUF_LEN)
//#define __MM_TTXSUB_S_TOP_ADDR      (__MM_PRIV_HIGHEST_ADDR)
//#define __MM_TTXSUB_S_START_ADDR    (__MM_TTXSUB_S_TOP_ADDR - __MM_TTXSUBT_S_LEN)
//#define __MM_VOID_BUFFER_ADDR       (__MM_TTXSUB_S_START_ADDR - __MM_VOID_BUFFER_LEN)    //23M
#define __MM_PRIVATE_TOP_ADDR       (__MM_PRIV_HIGHEST_ADDR)
#define __MM_PRIVATE_ADDR           (__MM_PRIVATE_TOP_ADDR-__MM_PRIVATE_LEN)     //0xA0000800//23M

#if (defined _CAS9_VSC_ENABLE_ || defined _C0700A_VSC_ENABLE_)
	#define __MM_VSC_CODE_START_ADDR	__MM_PRIVATE_ADDR
	#define __MM_VSC_DATA_START_ADDR (__MM_VSC_CODE_START_ADDR+__MM_VSC_CODE_LEN)
#endif

#ifdef SD_PVR
#else
    #define __MM_HIGHEST_ADDR           0xa8000000        //128M
    #define __MM_MAIN_TOP_ADDR          __MM_HIGHEST_ADDR        //128M
    #define __MM_VOID_BUFFER_ADDR       (__MM_MAIN_TOP_ADDR - __MM_VOID_BUFFER_LEN)    //23M
    #define __MM_VCAP_FB_ADDR           ((__MM_VOID_BUFFER_ADDR - __MM_VCAP_FB_SIZE)&0XFFFFFF00) // 256 bytes alignment
    #define __MM_FB_TOP_ADDR            __MM_VCAP_FB_ADDR //124MB
#endif

#ifdef SD_PVR
    #if(__MM_MAIN_TOP_ADDR != __MM_PRIVATE_ADDR)
    #error "__MM_PRIVATE_ADDR address error"
    #endif
#endif


// MPEG2
#define __MM_MAF_START_ADDR         ((__MM_FB_TOP_ADDR - __MM_MAF_LEN) & 0xFFFFFC00)
#define MPEG2_BUF_START_ADDR        ((__MM_MAF_START_ADDR - __MM_FB_LEN) & 0xFFFFFc00)

// AVC
#ifndef AVC_SUPPORT_UNIFY_MEM
#define AVC_MV_ADDR                 ((__MM_FB_TOP_ADDR - AVC_MV_LEN)&0xffffff00)                //256 bytes alignment
#define AVC_MB_COL_ADDR             ((AVC_MV_ADDR - AVC_MB_COL_LEN)&0xffffff00)                 //256 bytes alignment
#define AVC_MB_NEI_ADDR             ((AVC_MB_COL_ADDR - AVC_MB_NEI_LEN)&0xffffff00)             //256 bytes alignment
#define AVC_LAF_RW_BUF_ADDR         ((AVC_MB_NEI_ADDR - AVC_LAF_RW_BUF_LEN)&0xfffffc00)         //1024 bytes alignment
#define AVC_CMD_QUEUE_ADDR          ((AVC_LAF_RW_BUF_ADDR - AVC_CMD_QUEUE_LEN)&0xffffff00)      //256 bytes alignment
#define AVC_LAF_FLAG_BUF_ADDR       ((AVC_CMD_QUEUE_ADDR - AVC_LAF_FLAG_BUF_LEN)&0xfffffc00)    //1024 bytes alignment
#define AVC_DVIEW_ADDR              ((AVC_LAF_FLAG_BUF_ADDR -  AVC_DVIEW_LEN)&0xfffffe00)       //512 bytes alignment
#define AVC_BUF_START_ADDR          ((AVC_DVIEW_ADDR -  AVC_FB_LEN)&0xfffffe00)                 //512 bytes alignment
#else
#define AVC_MB_COL_ADDR 		    ((__MM_FB_TOP_ADDR - AVC_MB_COL_LEN - AVC_MV_LEN -AVC_CMD_QUEUE_LEN)&0xffffff00)  		//256 bytes alignment
#define AVC_MB_NEI_ADDR 			((AVC_MB_COL_ADDR - AVC_MB_NEI_LEN)&0xffffff00) 	//256 bytes alignment
#define AVC_BUF_START_ADDR 			((AVC_MB_NEI_ADDR - AVC_MEM_LEN)&0xffffff00)
#define AVC_DVIEW_ADDR 				0
#define AVC_MV_ADDR 				0
#define AVC_LAF_RW_BUF_ADDR			0
#define AVC_LAF_FLAG_BUF_ADDR       0
#define AVC_FB_ADDR					0
#endif

#if (MPEG2_BUF_START_ADDR < AVC_BUF_START_ADDR )
#define __MM_FB_BOTTOM_ADDR         MPEG2_BUF_START_ADDR
#else
#define __MM_FB_BOTTOM_ADDR         AVC_BUF_START_ADDR
#endif

#define __MM_FB_START_ADDR          __MM_FB_BOTTOM_ADDR
#define __MM_FB_ACT_LEN             (__MM_MAF_START_ADDR-__MM_FB_START_ADDR)

#ifdef AVC_SUPPORT_UNIFY_MEM
#undef  AVC_MEM_ADDR
#undef  AVC_MEM_LEN
#define AVC_MEM_ADDR                __MM_FB_BOTTOM_ADDR
#define AVC_MEM_LEN                 ((AVC_MB_NEI_ADDR - AVC_MEM_ADDR)&0xffffff00)
#else /* AVC_SUPPORT_UNIFY_MEM */
#define AVC_FB_ADDR                 __MM_FB_BOTTOM_ADDR
#define AVC_FB_ACT_LEN              (AVC_DVIEW_ADDR-AVC_FB_ADDR)

#define AVC_MEM_LEN                 0   //for compile error
#endif /* AVC_SUPPORT_UNIFY_MEM */


#ifndef SD_PVR
    #define __MM_NIM_BUFFER_ADDR            (__MM_FB_BOTTOM_ADDR - __MM_NIM_BUFFER_LEN)

    #ifdef FSC_SUPPORT
        #ifdef FSC_SHARE_WITH_PVR
            #define __MM_BUF_PVR_TOP_ADDR           __MM_NIM_BUFFER_ADDR
            #define __MM_FSC_BUFFER_TOP             ((__MM_BUF_PVR_TOP_ADDR)&0XFF800000)
        #else
            #define __MM_FSC_BUFFER_TOP             __MM_NIM_BUFFER_ADDR
            #define __MM_BUF_PVR_TOP_ADDR           (__MM_FSC_BUFFER_TOP - __MM_FSC_BUFFER_LEN)
        #endif
    #else
        #define __MM_BUF_PVR_TOP_ADDR           __MM_NIM_BUFFER_ADDR
    #endif
    
    #ifdef SAT2IP_SUPPORT//MEDIA_PROVIDER_USE_RING_BUFFER
        #define __MM_MEDIA_PROVIDER_BUFFER_ADDR (__MM_BUF_PVR_TOP_ADDR - __MM_MEDIA_PROV_BUFFER_LEN)
        #define __MM_PVR_VOB_BUFFER_ADDR        (__MM_MEDIA_PROVIDER_BUFFER_ADDR - __MM_PVR_VOB_BUFFER_LEN)
    #else
        #define __MM_PVR_VOB_BUFFER_ADDR    (__MM_BUF_PVR_TOP_ADDR - __MM_PVR_VOB_BUFFER_LEN)
    #endif

    #define __MM_DMX_AVP_START_ADDR        ((__MM_PVR_VOB_BUFFER_ADDR - __MM_SI_VBV_OFFSET - __MM_DMX_DATA_LEN - __MM_DMX_AVP_LEN)&0XFFFFFFF0)
    
    #define __MM_DMX_REC_START_ADDR        ((__MM_DMX_AVP_START_ADDR - __MM_DMX_REC_LEN)&0XFFFFFFF0)
    #define __MM_DMX_CPU_BLK_ADDR        ((__MM_DMX_REC_START_ADDR - __MM_DMX_BLK_BUF_LEN)&0XFFFFFFE0)
    #define __MM_EPG_BUFFER_START       (__MM_DMX_CPU_BLK_ADDR - __MM_EPG_BUFFER_LEN)

    #define __MM_GE_START_ADDR          ((__MM_EPG_BUFFER_START - __MM_GE_LEN)&0XFFFFFFE0)
    #define __MM_OSD_BK_ADDR1  		((__MM_GE_START_ADDR - __MM_OSD1_LEN)&0XFFFFFFE0)
    #define __MM_OSD_BK_ADDR2_MAIN      ((__MM_OSD_BK_ADDR1 - __MM_OSD2_LEN)&0XFFFFFFE0)
    //#define __MM_VCAP_FB_ADDR           ((__MM_OSD_BK_ADDR2_MAIN - __MM_VCAP_FB_SIZE)&0XFFFFFF00) // 256 bytes alignment
    #define __MM_OSD_VSRC_MEM_ADDR     ((__MM_OSD_BK_ADDR2_MAIN - OSD_VSRC_MEM_MAX_SIZE)&0XFFFFFFE0)//((__MM_VCAP_FB_ADDR - OSD_VSRC_MEM_MAX_SIZE)&0XFFFFFFE0)
    #define __MM_LWIP_MEM_ADDR          (__MM_OSD_VSRC_MEM_ADDR - __MM_LWIP_MEM_LEN)
   
#else
    #define __MM_GE_START_ADDR          ((__MM_FB_BOTTOM_ADDR - __MM_GE_LEN)&0XFFFFFFE0)
    #ifdef _M36F_SINGLE_CPU
        // for m36f single cpu mode(only test for ali internal)
        // move osd, vsrc, scap buffer to private buffer base.
        #define __MM_LWIP_MEM_ADDR      (__MM_GE_START_ADDR - __MM_LWIP_MEM_LEN)
    #else
        #define __MM_OSD_BK_ADDR1  		((__MM_GE_START_ADDR - __MM_OSD1_LEN)&0XFFFFFFE0)
        #if (defined(_CAS9_CA_ENABLE_) || defined(_C0200A_CA_ENABLE_) ||defined(_GEN_CA_ENABLE_) ||defined(_BC_CA_ENABLE_))
            #define __MM_OSD_BK_ADDR2_MAIN      ((__MM_OSD_BK_ADDR1 - __MM_OSD2_LEN)&0XFFFFFFE0)
            #define __MM_VCAP_FB_ADDR           ((__MM_OSD_BK_ADDR2_MAIN - __MM_VCAP_FB_SIZE)&0XFFFFFF00) // 256 bytes alignment
        #else
            #define __MM_VCAP_FB_ADDR           ((__MM_OSD_BK_ADDR1 - __MM_VCAP_FB_SIZE)&0XFFFFFF00) // 256 bytes alignment
        #endif
        #define __MM_LWIP_MEM_ADDR          (__MM_VCAP_FB_ADDR - __MM_LWIP_MEM_LEN)
     #endif
#endif

#define __MM_LWIP_MEMP_ADDR         (__MM_LWIP_MEM_ADDR - __MM_LWIP_MEMP_LEN)
#define __MM_USB_START_ADDR            ((__MM_LWIP_MEMP_ADDR - __MM_USB_DMA_LEN)&0XFFFFFFE0)

#ifndef SD_PVR
//add for TTX_TO_MAIN
#define __MM_TTX_SUB_PAGE_BUF_ADDR  (__MM_USB_START_ADDR - __MM_TTX_SUB_PAGE_LEN)
#define __MM_TTX_P26_NATION_BUF_ADDR (__MM_TTX_SUB_PAGE_BUF_ADDR - __MM_TTX_P26_NATION_LEN)
#define __MM_TTX_P26_DATA_BUF_ADDR  (__MM_TTX_P26_NATION_BUF_ADDR -  __MM_TTX_P26_DATA_LEN)
#define __MM_TTX_BS_START_ADDR    ((__MM_TTX_P26_DATA_BUF_ADDR - __MM_TTX_BS_LEN)&0XFFFFFFFC)
#define __MM_TTX_PB_START_ADDR    ((__MM_TTX_BS_START_ADDR - __MM_TTX_PB_LEN)&0XFFFFFFFC)

//add for ATSC_CC_TO_MAIN
#define __MM_ATSC_CC_PB_START_ADDR  ((__MM_TTX_PB_START_ADDR - __MM_ATSC_CC_PB_LEN)&0XFFFFFFFC)
#define __MM_ATSC_CC_BS_START_ADDR  ((__MM_ATSC_CC_PB_START_ADDR - __MM_ATSC_CC_BS_LEN)&0XFFFFFFFC)

#define __MM_SUB_BS_START_ADDR    ((__MM_ATSC_CC_BS_START_ADDR  - __MM_SUB_BS_LEN)&0XFFFFFFFC)
#define __MM_SUB_HW_DATA_ADDR ((__MM_SUB_BS_START_ADDR - __MM_SUB_HW_DATA_LEN)&0XFFFFFFF0)
#define __MM_SUB_PB_START_ADDR    ((__MM_SUB_HW_DATA_ADDR - __MM_SUB_PB_LEN)&0XFFFFFFFC)
//modify atsc subtitle on main cpu
#define __MM_DCII_SUB_BS_START_ADDR     (__MM_SUB_PB_START_ADDR - __MM_DCII_SUB_BS_LEN)
#define __MM_SUBT_ATSC_SEC_ADDR     (__MM_DCII_SUB_BS_START_ADDR - __MM_SUBT_ATSC_SEC_LEN)
//modify isdbt closed caption on main cpu
#define __MM_ISDBTCC_BS_START_ADDR ((__MM_SUBT_ATSC_SEC_ADDR - __MM_ISDBTCC_BS_LEN)&0XFFFFFFE0)
#define __MM_ISDBTCC_PB_START_ADDR ((__MM_ISDBTCC_BS_START_ADDR - __MM_ISDBTCC_PB_LEN)&0XFFFFFFE0)
#else
//add for TTX_TO_MAIN
#define __MM_TTX_SUB_PAGE_BUF_ADDR  (__MM_TTXSUB_S_TOP_ADDR - __MM_TTX_SUB_PAGE_LEN)
#define __MM_TTX_P26_NATION_BUF_ADDR (__MM_TTX_SUB_PAGE_BUF_ADDR - __MM_TTX_P26_NATION_LEN)
#define __MM_TTX_P26_DATA_BUF_ADDR  (__MM_TTX_P26_NATION_BUF_ADDR -  __MM_TTX_P26_DATA_LEN)
#define __MM_TTX_BS_START_ADDR    ((__MM_TTX_P26_DATA_BUF_ADDR - __MM_TTX_BS_LEN)&0XFFFFFFFC)
#define __MM_TTX_PB_START_ADDR    ((__MM_TTX_BS_START_ADDR - __MM_TTX_PB_LEN)&0XFFFFFFFC)

//add for ATSC_CC_TO_MAIN
#define __MM_ATSC_CC_PB_START_ADDR  ((__MM_TTX_PB_START_ADDR - __MM_ATSC_CC_PB_LEN)&0XFFFFFFFC)
#define __MM_ATSC_CC_BS_START_ADDR  ((__MM_ATSC_CC_PB_START_ADDR - __MM_ATSC_CC_BS_LEN)&0XFFFFFFFC)

#define __MM_SUB_BS_START_ADDR    ((__MM_ATSC_CC_BS_START_ADDR  - __MM_SUB_BS_LEN)&0XFFFFFFFC)
#define __MM_SUB_HW_DATA_ADDR ((__MM_SUB_BS_START_ADDR - __MM_SUB_HW_DATA_LEN)&0XFFFFFFF0)
#define __MM_SUB_PB_START_ADDR    ((__MM_SUB_HW_DATA_ADDR - __MM_SUB_PB_LEN)&0XFFFFFFFC)
//modify atsc subtitle on main cpu
#define __MM_DCII_SUB_BS_START_ADDR     (__MM_SUB_PB_START_ADDR - __MM_DCII_SUB_BS_LEN)
#define __MM_SUBT_ATSC_SEC_ADDR     (__MM_DCII_SUB_BS_START_ADDR - __MM_SUBT_ATSC_SEC_LEN)
//modify isdbt closed caption on main cpu
#define __MM_ISDBTCC_BS_START_ADDR ((__MM_SUBT_ATSC_SEC_ADDR - __MM_ISDBTCC_BS_LEN)&0XFFFFFFE0)
#define __MM_ISDBTCC_PB_START_ADDR ((__MM_ISDBTCC_BS_START_ADDR - __MM_ISDBTCC_PB_LEN)&0XFFFFFFE0)

#if ( __MM_ISDBTCC_PB_START_ADDR < __MM_TTXSUB_S_START_ADDR)
    #error "TTX ATSC len is bigger than __MM_TTXSUBT_S_LEN"
#endif
#endif

#ifndef SD_PVR
#define __MM_CPU_DBG_MEM_ADDR          (__MM_ISDBTCC_PB_START_ADDR - __MM_DBG_MEM_LEN)
#else
#define __MM_CPU_DBG_MEM_ADDR          (__MM_USB_START_ADDR - __MM_DBG_MEM_LEN)
#endif

#define __MM_AUTOSCAN_DB_BUFFER_ADDR    __MM_PVR_VOB_BUFFER_ADDR

#define __MM_NIM_BUFFER_TOP_ADDR		(__MM_NIM_BUFFER_ADDR + __MM_NIM_BUFFER_LEN)
#define __MM_NIM_DVBT2_BUFFER_ADDR        (__MM_NIM_BUFFER_TOP_ADDR - __MM_NIM_DVBT2_BUFFER_LEN)
#define __MM_NIM_DVBC_BUFFER_ADDR       (__MM_NIM_DVBT2_BUFFER_ADDR - __MM_NIM_DVBC_BUFFER_LEN)


//Allocate FSC buffer, share with PVR
#ifdef FSC_SUPPORT 
    #define __MM_FSC_BUFFER_ADDR    (__MM_FSC_BUFFER_TOP - __MM_FSC_BUFFER_LEN)

    #define __MM_DMX0_DMA_START_ADDR    (__MM_DMX_AVP_START_ADDR)
    #define __MM_DMX1_DMA_START_ADDR    (__MM_DMX_REC_START_ADDR)
    #define __MM_DMX2_DMA_START_ADDR    ((__MM_FSC_BUFFER_TOP - __MM_DMX_REC_LEN)&0XFFFFFFF0)
    //#define __MM_DMX3_DMA_START_ADDR	((__MM_DMX2_DMA_START_ADDR - __MM_DMX_REC_LEN)&0XFFFFFFF0)

    #define __MM_DMX0_MAIN_BLK_START_ADDR (__MM_DMX_CPU_BLK_ADDR )
    #ifdef FSC_DMA_BUFFER_CACHE
    #define __MM_DMX1_MAIN_BLK_START_ADDR   (__MM_DMX_CPU_BLK_ADDR )
    #define __MM_DMX2_MAIN_BLK_START_ADDR   (__MM_DMX_CPU_BLK_ADDR )
    #else
    #define __MM_DMX1_MAIN_BLK_START_ADDR	((__MM_DMX3_DMA_START_ADDR - __MM_DMX_BLK_BUF_LEN)&0XFFFFFFE0)
    #define __MM_DMX2_MAIN_BLK_START_ADDR   ((__MM_DMX1_MAIN_BLK_START_ADDR - __MM_DMX_BLK_BUF_LEN)&0XFFFFFFE0)
    #endif
    
#ifdef FSC_DMA_BUFFER_CACHE 
    #if(__MM_DMX2_DMA_START_ADDR < __MM_FSC_BUFFER_ADDR)
    #error "1) __MM_FSC_BUFFER_LEN too small for dmx2"
    #endif
    #if defined(__MM_DMX3_DMA_START_ADDR) && (__MM_DMX3_DMA_START_ADDR < __MM_FSC_BUFFER_ADDR)
    #error "2) __MM_FSC_BUFFER_LEN too small for dmx3"
    #endif
#else
#if(__MM_DMX2_MAIN_BLK_START_ADDR < __MM_FSC_BUFFER_ADDR)
    #error "__MM_FSC_BUFFER_ADDR too small"
#endif
#if(__MM_FSC_BUFFER_ADDR < __MM_PVR_VOB_BUFFER_ADDR)
    #error "PVR no enough buffer to share with fsc"
#endif
#endif
#endif


//end of main mem map

//Private mem map
/*
    In HW security mode, some see buffer must be shared by media player,
    so put them together on the top:
    VBV, DMX, TTX
*/

#ifdef _DD_SUPPORT
#define __MM_PRIV_DEC_LEN   0x20000
#define __MM_PRIV_DEC_ADDR (__MM_PRIVATE_TOP_ADDR- __MM_PRIV_DEC_LEN)
#else
#define __MM_PRIV_DEC_LEN   0x20000 //for 
#define __MM_PRIV_DEC_ADDR (__MM_PRIVATE_TOP_ADDR- __MM_PRIV_DEC_LEN)
#endif

#define __MM_REAL_PRIVATE_TOP_ADDR  __MM_PRIV_DEC_ADDR

#define __MM_VBV_START_ADDR         ((__MM_REAL_PRIVATE_TOP_ADDR - __MM_VBV_LEN)&0XFFFFFF00)
#define AVC_VBV_ADDR                ((__MM_REAL_PRIVATE_TOP_ADDR - AVC_VBV_LEN)&0XFFFFFF00)   //256 bytes alignment


#if (AVC_VBV_ADDR < __MM_VBV_START_ADDR)
#define VDEC_PRIVATE_START_ADDR     AVC_VBV_ADDR
#else
#define VDEC_PRIVATE_START_ADDR     __MM_VBV_START_ADDR
#endif

#define __MM_DMX_SEE_BLK_ADDR       ((VDEC_PRIVATE_START_ADDR - __MM_DMX_SEE_BLK_BUF_LEN)&0XFFFFFFE0)
#define __MM_DMX_SEE_BLK_START_ADDR (__MM_DMX_SEE_BLK_ADDR)


#define __MM_SEE_MP_BUF_ADDR    (__MM_DMX_SEE_BLK_ADDR -  __MM_SEE_MP_BUF_LEN)

//move to main
//#define AVC_CMD_QUEUE_ADDR  ((__MM_SEE_MP_BUF_ADDR - AVC_CMD_QUEUE_LEN)&0xffffff00)      //256 bytes alignment
//#define AVC_LAF_FLAG_BUF_ADDR   ((AVC_CMD_QUEUE_ADDR - AVC_LAF_FLAG_BUF_LEN)&0xfffffc00)  //1024 bytes alignment
#define __MM_OSD_BK_ADDR2       ((__MM_SEE_MP_BUF_ADDR - __MM_OSD2_LEN)&0XFFFFFFE0)//((AVC_LAF_FLAG_BUF_ADDR - __MM_OSD2_LEN)&0XFFFFFFE0)

// AD buffer share with __MM_OSD_BK_ADDR2, because in 3505, no ttx/subt in see anymore
#define __MM_AD_DECA_MEM_ADDR            ((__MM_SEE_MP_BUF_ADDR - __MM_AD_DECA_MEM_LEN)&0xFFFFFFE0)
#define __MM_AD_SND_MEM_ADDR            ((__MM_AD_DECA_MEM_ADDR - __MM_AD_SND_MEM_LEN)&0xFFFFFFE0)

#if (__MM_AD_SND_MEM_ADDR < __MM_OSD_BK_ADDR2)
#define __MM_SEE_DBG_MEM_ADDR      ( __MM_AD_SND_MEM_ADDR - __MM_DBG_MEM_LEN)
#else
#define __MM_SEE_DBG_MEM_ADDR      ( __MM_OSD_BK_ADDR2 - __MM_DBG_MEM_LEN)
#endif

// for jpeg decoder memmap
#ifdef SD_PVR
#define __MM_FB0_Y_LEN            (736*576 + 1024)
#define __MM_IMG_FB_Y_LEN        (736*576*2 + 1024)
#define __MM_IMG_FB_C_LEN        (__MM_IMG_FB_Y_LEN/2)
#else
#define __MM_FB0_Y_LEN            (1920*1088+1024)//(736*576+512)    //for high definition jpg decode
#endif
#define __MM_FB1_Y_LEN            __MM_FB0_Y_LEN
#define __MM_FB2_Y_LEN            __MM_FB0_Y_LEN

#define __MM_FB0_C_LEN            (__MM_FB0_Y_LEN/2)
#define __MM_FB1_C_LEN            __MM_FB0_C_LEN
#define __MM_FB2_C_LEN            __MM_FB0_C_LEN

#define __MM_FB3_Y_LEN            (736*576+1024)
#define __MM_FB3_C_LEN            (__MM_FB3_Y_LEN/2)
#define __MM_FB4_Y_LEN            __MM_FB3_Y_LEN
#define __MM_FB4_C_LEN            __MM_FB3_C_LEN
#define __MM_FB5_Y_LEN          __MM_FB3_Y_LEN
#define __MM_FB5_C_LEN          __MM_FB3_C_LEN
#define __MM_FB6_Y_LEN          __MM_FB3_Y_LEN
#define __MM_FB6_C_LEN          __MM_FB3_C_LEN

#ifdef SD_PVR   
    #define __MM_FB3_Y_START_ADDR    0 //((__MM_FB2_C_START_ADDR+__MM_FB2_C_LEN)&0XFFFFFE00)
    #define __MM_FB3_C_START_ADDR    0 //((__MM_FB3_Y_START_ADDR+__MM_FB3_Y_LEN)&0XFFFFFE00)
    #define __MM_FB4_Y_START_ADDR    0 //((__MM_FB3_C_START_ADDR+__MM_FB3_C_LEN)&0XFFFFFE00)
    #define __MM_FB4_C_START_ADDR    0 //((__MM_FB4_Y_START_ADDR+__MM_FB4_Y_LEN)&0XFFFFFE00)
    #define __MM_FB5_Y_START_ADDR    0 //((__MM_FB4_C_START_ADDR+__MM_FB4_C_LEN)&0XFFFFFE00)
    #define __MM_FB5_C_START_ADDR    0 //((__MM_FB5_Y_START_ADDR+__MM_FB5_Y_LEN)&0XFFFFFE00)
    #define __MM_FB6_Y_START_ADDR    0 //((__MM_FB5_C_START_ADDR+__MM_FB5_C_LEN)&0XFFFFFE00)
    #define __MM_FB6_C_START_ADDR    0 //((__MM_FB6_Y_START_ADDR+__MM_FB6_Y_LEN)&0XFFFFFE00)
    #define __MM_MP_BUFFER_LEN        (__MM_TTX_BS_START_ADDR - __MM_SEE_MP_BUF_ADDR)//(__MM_SEE_MP_BUF_LEN - __MM_FB0_Y_LEN * 3 - __MM_FB0_C_LEN * 3)
    #define __MM_MP_BUFFER_ADDR        __MM_SEE_MP_BUF_ADDR//(__MM_FB2_C_START_ADDR + __MM_FB2_C_LEN)
#else
    #define __MM_FB0_Y_START_ADDR    (__MM_SEE_MP_BUF_ADDR & 0xFFFFFE00)
    #define __MM_FB0_C_START_ADDR    (__MM_FB0_Y_START_ADDR+__MM_FB0_Y_LEN)

    #define __MM_FB1_Y_START_ADDR    ((__MM_FB0_C_START_ADDR+__MM_FB0_C_LEN)&0XFFFFFE00)
    #define __MM_FB1_C_START_ADDR    ((__MM_FB1_Y_START_ADDR+__MM_FB1_Y_LEN)&0XFFFFFE00)

    #define __MM_FB2_Y_START_ADDR    ((__MM_FB1_C_START_ADDR+__MM_FB1_C_LEN)&0XFFFFFE00)
    #define __MM_FB2_C_START_ADDR    ((__MM_FB2_Y_START_ADDR+__MM_FB2_Y_LEN)&0XFFFFFE00)

    #define __MM_FB3_Y_START_ADDR    0 //((__MM_FB2_C_START_ADDR+__MM_FB2_C_LEN)&0XFFFFFE00)
    #define __MM_FB3_C_START_ADDR    0 //((__MM_FB3_Y_START_ADDR+__MM_FB3_Y_LEN)&0XFFFFFE00)
    #define __MM_FB4_Y_START_ADDR    0 //((__MM_FB3_C_START_ADDR+__MM_FB3_C_LEN)&0XFFFFFE00)
    #define __MM_FB4_C_START_ADDR    0 //((__MM_FB4_Y_START_ADDR+__MM_FB4_Y_LEN)&0XFFFFFE00)
    #define __MM_FB5_Y_START_ADDR    0 //((__MM_FB4_C_START_ADDR+__MM_FB4_C_LEN)&0XFFFFFE00)
    #define __MM_FB5_C_START_ADDR    0 //((__MM_FB5_Y_START_ADDR+__MM_FB5_Y_LEN)&0XFFFFFE00)
    #define __MM_FB6_Y_START_ADDR    0 //((__MM_FB5_C_START_ADDR+__MM_FB5_C_LEN)&0XFFFFFE00)
    #define __MM_FB6_C_START_ADDR    0 //((__MM_FB6_Y_START_ADDR+__MM_FB6_Y_LEN)&0XFFFFFE00)
    //#define __MM_MP_BUFFER_LEN        (__MM_FB_TOP_ADDR - __MM_TTX_P26_DATA_BUF_ADDR)//(__MM_SEE_MP_BUF_LEN - __MM_FB0_Y_LEN * 3 - __MM_FB0_C_LEN * 3)
    //#define __MM_MP_BUFFER_ADDR        __MM_TTX_P26_DATA_BUF_ADDR//(__MM_FB2_C_START_ADDR + __MM_FB2_C_LEN)
#endif

// for satcodx memmap
#define __MM_DVW_START_ADDR         ((__MM_FB2_C_START_ADDR - 0X25FA0)&0XFFFFFFF0)

#ifdef SAT2IP_SUPPORT
    #define __MM_MP_BUFFER_LEN            (__MM_PVR_VOB_BUFFER_LEN+__MM_NIM_BUFFER_LEN+__MM_MEDIA_PROV_BUFFER_LEN)
#else
    /*the DMX module share its memory with media player module, 
    which can increase 3.23M memory size of MP module. As the MP module 
    needs a lot of memory, the PVR module is also share its memory with MP 
    module. As the DMX module dedicates 3.23M to MP module, the PVR module 
    can reduce the dedication of 3.23M. The heap space of aui version will increase 3.23M*/
	#ifdef _AUI_
        #define __MM_MP_BUFFER_LEN            (__MM_PVR_VOB_BUFFER_LEN+__MM_NIM_BUFFER_LEN +  __MM_SI_VBV_OFFSET + \
                                                                     __MM_DMX_DATA_LEN + __MM_DMX_AVP_LEN + __MM_DMX_REC_LEN + __MM_DMX_BLK_BUF_LEN)
    #else
		#define __MM_MP_BUFFER_LEN            (__MM_PVR_VOB_BUFFER_LEN+__MM_NIM_BUFFER_LEN)
	#endif
#endif
/*the DMX module share its memory with media player module, 
which can increase 3.23M memory size of MP module. As the MP module 
needs a lot of memory, the PVR module is also share its memory with MP 
module. As the DMX module dedicates 3.23M to MP module, the PVR module 
can reduce the dedication of 3.23M. The heap space of aui version will increase 3.23M*/
#ifdef _AUI_
    #define __MM_MP_BUFFER_ADDR          __MM_DMX_CPU_BLK_ADDR
#else
    #define __MM_MP_BUFFER_ADDR            __MM_PVR_VOB_BUFFER_ADDR
#endif


//end of Private mem map
#ifdef SEE_CPU
#define __MM_DBG_MEM_ADDR __MM_SEE_DBG_MEM_ADDR
#else
#define __MM_DBG_MEM_ADDR __MM_CPU_DBG_MEM_ADDR
#endif

#define __MM_HEAP_TOP_ADDR      __MM_DBG_MEM_ADDR

#endif //end of VFB_SUPPORT
#endif //end of DRAM_SPLIT


#endif //end of 128M config

#define __MM_VIDEO_FILE_BUF_ADDR    __MM_MP_BUFFER_ADDR
#define __MM_VIDEO_FILE_BUF_LEN     __MM_MP_BUFFER_LEN

#ifdef PLSN_SUPPORT
#define __MM_TABLE_LEN		0x319A00
#define __MM_CAPTURE_LEN	0x400000
#if (defined(M36F_SINGLE_CPU) && defined(SUPPORT_64M_MEM))
#define __MM_CAPTURE_ADDR 	(__MM_PVR_VOB_BUFFER_ADDR & 0x8FFFFFFF)
#define __MM_TABLE_ADDR		((__MM_VBV_START_ADDR&0x8FFFFFFF))
#else
#define __MM_CAPTURE_ADDR 	((__MM_PVR_VOB_BUFFER_ADDR + __MM_AUTOSCAN_DB_BUFFER_LEN) & 0x8FFFFFFF)
#define __MM_TABLE_ADDR		((__MM_CAPTURE_ADDR + __MM_CAPTURE_LEN) & 0x8FFFFFFF)
#endif
#endif

#endif //end of __SYS_MEM_MAP_H

