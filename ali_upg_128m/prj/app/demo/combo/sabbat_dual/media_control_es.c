/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: media_control_es.c
*
*    Description:
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/#include <sys_config.h>

#ifdef ES_PLAYER_TEST
#include <hld/decv/decv_media_player.h>
#include <hld/deca/deca.h>
#include <api/libmp/es_player_api.h>
#include <api/libfs2/stat.h>
#include <api/libfs2/dirent.h>
#include <api/libfs2/statvfs.h>
#include <api/libfs2/unistd.h>
#include <api/libfs2/stdio.h>
#endif

#ifdef ES_PLAYER_TEST
/**********************es player demo**************************************/
/* AWindH264.h264 every frame size */
static UINT32 frame_size[] =
{
    140898,    3221,    5601,    24314,    56859,    13455,    31754,    20593,    37285,    24512,
    45513,    22875,    26959,    24867,    25709,    25544,    41039,    26306,    28103,    27525,
    31509,    31950,    32494,    31606,    32534,    30997,    33557,    30466,    31926,    30601,
    32070,    31377,    34337,    33151,    72224,    32803,    31760,    32173,    29849,    29729,
    31286,    34497,    28756,    31877,    26168,    38159,    31117,    30298,    34097,    29890,
    32814,    31903,    31717,    30299,    30272,    31933,    33917,    40881,    57614,    50281,
    53449,    51582,    51822,    70068,    19019,    17418,    17878,    41741,    58781,    55272,
    53929,    51239,    55766,    46969,    48376,    49092,    47846,    56016,    48733,    43065,
    53618,    48955,    47887,    43727,    44588,    45241,    47634,    49321,    49213,    48939,
    55397,    48487,    47949,    48112,    43635,    41967,    17470,    13006,    30445,    43769,
    33192,    36328,    37141,    36789,    40140,    37656,    38214,    38765,    38422,    37963,
    38689,    40374,    39831,    35833,    33850,    34239,    33545,    33390,    37730,    36207,
    34742,    33485,    34030,    33115,    34534,    32871,    34562,    33289,    34045,    33279,
    32165,    46935,    31645,    37689,    33466,    36873,    37800,    37675,    35564,    34055,
    36163,    35650,    35029,    34398,    35439,    37189,    35526,    37572,    35883,    12852,
    51773,    42115,    41209,    40383,    10253,    21816,    82328,    59178,    74559,    76526,
    64621,    74136,    65358,    72711,    62381,    66354,    83045,    63550,    74507,    68286,
    70764,    76701,    47276,    52644,    48681,    47419,    45430,    46449,    44826,    40315,
    39680,    42816,    40722,    38388,    38395,    40235,    45069,    43949,    42892,    42067,
    74516,    35540,    40648,    34165,    37245,    35810,    41224,    36549,    43633,    36290,
    43129,    55940,    37423,    41867,    36490,    41791,    36300,    38107,    54002,    39779,
    32497,    35487,    31254,    35332,    30092,    32876,    28979,    32891,    29140,    42809,
    35625,    38537,    34145,    39045,    34715,    37759,    26430,    30976,    39227,    41263,
    36558,    30011,    28512,    42410,    36408,    40168,    33973,    39834,    36699,    39807,
    36940,    42007,    36035,    40694,    42851,    36026,    35333,    32375,    38582,    34569,
    106677,    18704,    23609,    27483,    27394,    29854,    33704,    29037,    36075,    32132,
    80940,    35433,    32625,    37239,    38036,    34660,    35237,    45922,    35670,    32172,
    46012,    32892,    38029,    36483,    36445,    34996,    38824,    39203,    40594,    38780,
    42705,    43292,    41418,    26154,    26042,    23153,    26578,    21605,    23912,    25694,
    28466,    25824,    32095,    17983,    17867,    22655,    19500,    22991,    22584,    31883,
    31296,    30275,    23878,    18512,    20611,    27405,    23377,    26159,    18300,    18155,
    17718,    22682,    22646,    23612,    22211,    25955,    24418,    61618,    20273,    26422,
    25340,    26468,    25169,    35241,    33468,    34811,    24798,    35357,    26098,    29837,
    30076,    30261,    31932,    33327,    33957,    19257,    13174,    9335,    27508,    47650,
    26811,    23252,    20611,    18409,    18403,    35694,    15947,    21221,    16614,    21872,
    19401,    18578,    20042,    18075,    17918,    18065,    18695,    18114,    18273,    18727,
    20564,    20560,    19307,    19622,    24157,    17887,    28926,    24461,    25411,    24485,
    23355,    20908,    20002,    27319,    21098,    22673,    23393,    22795,    22894,    23197,
    28430,    24903,    21493,    22491,    22136,    21060,    28322,    26128,    25914,    27021,
    26556,    24644,    24376,    24687,    23819,    23286,    22434,    21192,    21732,    21278,
    26403,    22890,    24324,    23363,    24991,    48108,    30689,    27967,    23789,    24398,
    22604,    22578,    22396,    23333,    23774,    29517,    23679,    23980,    24151,    24306,
    24644,    30258,    28626,    14782,    32393,    27229,    23170,    24025,    22047,    21900,
    24021,    24225,    25875,    23171,    24091,    21691,    23418,    22876,    25141,    24914,
    24898,    23567,    24765,    24346,    25966,    24652,    24954,    25333,    27487,    26644,
    26136,    25891,    26038,    25391,    24475,    25680,    28987,    26890,    25573,    24680,
    16570,    33923,    24882,    20958,    20166,    25506,    22515,    25236,    21431,    22143,
    24737,    24024,    27606,    26139,    23549,    23334,    26079,    28095,    28946,    29768,
    29848,    31312,    31573,    31256,    29727,    32398,    34128,    30880,    35847,    31239,
    36534,    38048,    41573,    50263,    48334,    44396,    47738,    46497,    57139,    61498,
    186560,    41680,    48173,    39611,    39122,    37449,    43130,    45281,    51980,    43839,
    44601,    42473,    47284,    44962,    47340,    49996,    47783,    55460,    45404,    46914,
    47625,    54124,    35531,    46038,    43008,    40684,    45179,    45435,    43597,    38346,
    38170,    37339,    39833,    38332,    40379,    32157,    20606,    24500,    23644,    28795,
    25830,    24025,    24499,    39453,    31761,    32134,    33709,    30138,    38658,    33409,
    33364,    37063,    33540,    39082,    32640,    36665,    35588,    34639,    34574,    34371,
    33917,    34836,    32132,    34210,    32146,    33564,    34057,    34483,    33948,    39266,
    38188,    41919,    39484,    41286,    40523,    41263,    41184,    39216,    41731,    39476,
    37742,    37878,    41017,    42105,    40426,    38965,    41270,    41855,    37662,    38685,
    38790,    41713,    39844,    39257,    35159,    37805,    38546,    38083,    34684,    32702,
    37421,    40869,    34174,    34954,    33104,    41281,    34325,    43301,    40106,    40345,
    39526,    38712,    35733,    35140,    35311,    34999,    41515,    40757,    40568,    34701,
    30537,    29253,    31035,    29510,    37561,    31419,    27550,    32937,    34653,    29348,
    24496,    23706,    21391,    19997,    28700,    23781,    28914,    25997,    26034,    22151,
    23233,    23064,    24891,    22794,    23966,    23846,    28505,    29002,    66331,    34869,
    36639,    41253,    40836,    38045,    38265,    37128,    44203,    40417,    38632,    35710,
    41584,    42156,    37983,    41446,    47146,
};

static char *m_video_file_name = NULL;
static FILE *video_fp = NULL;
static UINT8 *video_frame = NULL;

static void es_player_video_task(UINT32 para1, UINT32 para2)
{
    ali_video_config *dec_config = (ali_video_config *)para1;
    unsigned long long file_size;
    RET_CODE ret = RET_SUCCESS;
    UINT32 i = 0, free_size = 0, valid_size = 0, total_size = 0;
    UINT32 read_size, need_read = 1;
    UINT32 packet_size;
    ali_video_decoder_status status;
    struct av_packet pkt;
    const UINT16 video_valid_size = 1024;

    ali_sbm_info pe_info;
    MEMSET(&pe_info, 0, sizeof(ali_sbm_info));

    //fill the sbm config
    pe_info.mem_start = __MM_VIDEO_FILE_BUF_ADDR;
    pe_info.mem_size = __MM_VIDEO_FILE_BUF_LEN;

    #if (SYS_SDRAM_SIZE == 128 || SYS_SDRAM_SIZE == 64)
    // video file buffer is seperated with frame buffer
    #if (__MM_FB_TOP_ADDR > __MM_VIDEO_FILE_BUF_ADDR)
    pe_info.mem_size = __MM_FB_TOP_ADDR - __MM_VIDEO_FILE_BUF_ADDR;
    #endif
    pe_info.decoder_start = __MM_FB_BOTTOM_ADDR;
    pe_info.decv_buf_size = __MM_FB_TOP_ADDR - __MM_FB_BOTTOM_ADDR;
    #endif

//h.264
    dec_config->codec_tag = h264;
    dec_config->extradata = NULL;
    dec_config->extradata_size = 0;
    dec_config->frame_rate = 25000; /* param parser from container */
    dec_config->width = 1280;
    dec_config->height = 800;
    dec_config->sample_aspect_ratio.num = 1;
    dec_config->sample_aspect_ratio.den = 1;
    dec_config->preview = 0;

    if(dec_config->preview)
    {
        dec_config->src_rect.x = 32;
        dec_config->src_rect.y = 96;
        dec_config->src_rect.w = 656;
        dec_config->src_rect.h = 2688;
        dec_config->dst_rect.x = 430;
        dec_config->dst_rect.y = 475;
        dec_config->dst_rect.w = 165;
        dec_config->dst_rect.h = 710;
    }
    m_video_file_name = NULL;

    if(m_video_file_name == NULL)
    {
       m_video_file_name = "/mnt/uda1/es_player/AWindH264.mp4";
    }

    MEMSET(&pkt, 0, sizeof(pkt));

    video_fp = fopen(m_video_file_name, "rb");
    if(video_fp == NULL)
    {
        libc_printf("Open file %s fail\n", m_video_file_name);
        return -1;
    }

    fseek(video_fp, 0, SEEK_END);
    file_size = ftell(video_fp);
    fseek(video_fp, 0, SEEK_SET);
    if(file_size <= 0)
    {
        libc_printf("File size error %lld\n", file_size);
        return -1;
    }

    //fflush(fp);
    libc_printf("File size %lld", file_size);

    video_frame = (UINT8 *)MALLOC(0x100000);
    if(video_frame == NULL)
    {
        libc_printf("Malloc frame buffer fail\n");
        return -1;
    }

    ret = ali_video_decoder_open(dec_config, pe_info);
    if(ret < 0)
    {
        libc_printf("open video decoder fail\n");
        return -1;
    }

    ali_video_decoder_set_sync(dec_config, FALSE); //video free run, don't avsync

    ret = ali_sbm_show_total_size(VPKT_DATA_SBM_IDX, &total_size);
    if(ret < 0)
    {
        libc_printf("sbm ioctl fail\n");
    }

    while(1)
    {
        ret = ali_sbm_show_free_size(VPKT_DATA_SBM_IDX, &free_size);
        if(ret < 0)
        {
            libc_printf("sbm ioctl fail\n");
        }

        ret = ali_sbm_show_valid_size(VPKT_DATA_SBM_IDX, &valid_size);
        if(ret < 0)
        {
            libc_printf("sbm ioctl fail\n");
        }

        if((dec_config->extradata_size > 0) && (dec_config->extradata))
        {
            dec_config->extradata = NULL;
            i++;
        }

        if(need_read)
        {
            /* parse a complete frame */
          read_size = fread(video_frame, 1, frame_size[i], video_fp);
          packet_size = frame_size[i];
        }

        if(read_size == packet_size)
        {
            if((free_size > read_size) && (valid_size < total_size*4/5))
            {
                pkt.pts = AV_NOPTS_VALUE;
                pkt.dts = AV_NOPTS_VALUE;
                pkt.size = read_size;
                ret = ali_video_decoder_write_header(&pkt);
                if(ret == 0)
                {
                    do
                    {
                        ret = ali_video_decoder_write(video_frame, read_size);

                        if(ret < 0)
                        {
                            libc_printf("Write video packet data fail\n");
                            osal_task_sleep(10);
                        }
                    } while (ret < 0);
                }
                else
                {
                    libc_printf("Write video packet header fail\n");
                    goto sleep;
                }

                if(++i >= ARRAY_SIZE(frame_size))
                    break;

                need_read = 1;
            }
            else
            {
                ali_video_decoder_get_status(dec_config, &status);
                //libc_printf("video decoder status, width: %d, height: %d \n", status.width, status.height);
                need_read = 0;
            }
        }
        else
        {
            if(valid_size <= video_valid_size)
            {
                libc_printf("Read video frame error %d %d\n", read_size, frame_size[i]);
                goto task_exit;
            }
        }
sleep:
       osal_task_sleep(5);
    }

task_exit:
    ali_video_decoder_close(dec_config);

    fclose(video_fp);
    video_fp = NULL;
    FREE(video_frame);
    video_frame = NULL;

    libc_printf("=====Exit playing file %s=====", m_video_file_name);
}

RET_CODE win_es_player_video_init(ali_video_config * dec_config, OSAL_ID    tsk_id)
{
    OSAL_T_CTSK t_ctsk;

    //create es player task
    t_ctsk.itskpri = OSAL_PRI_HIGH;
    t_ctsk.stksz = 0x4000;
    t_ctsk.quantum = 10;
    t_ctsk.name[0] = 'e';
    t_ctsk.name[1] = 's';
    t_ctsk.name[2] = 'p';
    t_ctsk.para1 = (UINT32)dec_config;
    t_ctsk.task = es_player_video_task;
    *(UINT32 *)tsk_id = osal_task_create(&t_ctsk);
    if(OSAL_INVALID_ID == *(UINT32 *)tsk_id)
    {
        return -1;
    }
}

RET_CODE win_es_player_video_close(ali_video_config * dec_config, OSAL_ID    tsk_id)
{
    if(OSAL_INVALID_ID != tsk_id)
    {
        ali_video_decoder_close(dec_config);
        osal_task_delete(tsk_id);
    }

    if(video_fp != NULL)
    {
        fclose(video_fp);
        video_fp = NULL;
    }

    if(video_frame != NULL)
    {
        FREE(video_frame);
        video_frame = NULL;
    }
}

static UINT32 audio_frame_size[] =
{
    17,20,10,10,38,65,31,30,139,31,31,31,31,31,31,31,31,31,31,31,31,31,
    31,31,31,31,31,31,31,31,31,108,613,557,451,465,455,459,465,459,472,462,
    467,467,471,487,465,462,476,478,483,485,437,438,456,458,460,479,387,416,
    475,482,491,474,605,477,469,451,440,442,442,455,465,464,469,448,466,476,
    483,490,492,475,474,489,478,482,464,475,443,464,604,466,467,470,444,444,
    437,472,498,502,503,494,505,507,515,493,482,567,456,475,439,463,465,467,
    491,510,509,507,487,483,495,489,491,498,477,464,446,480,511,526,529,481,
    480,474,472,478,463,494,509,518,477,456,482,505,512,491,490,497,508,477,
    478,489,506,514,511,480,472,474,487,502,501,489,482,473,486,511,514,526,
    525,481,462,451,450,495,493,507,476,487,487,466,475,481,493,479,480,483,
    483,477,501,491,512,516,460,480,485,480,500,465,445,441,499,503,518,494,
    510,496,496,495,492,484,492,493,499,503,501,506,504,495,498,508,499,488,
    500,481,488,483,479,496,514,467,461,468,466,466,475,470,470,461,482,480,
    482,523,484,486,480,467,474,477,487,441,469,483,487,482,475,481,505,499,
    491,481,486,498,502,500,496,494,503,502,501,507,494,491,487,481,478,486,
    489,486,496,498,483,489,489,480,478,469,478,471,479,484,466,462,492,475,
    475,464,454,483,493,503,494,510,467,460,474,494,489,492,486,480,484,480,
    471,470,490,579,563,462,486,461,463,478,484,473,433,434,452,474,455,467,
    465,464,463,462,492,491,499,493,585,481,493,472,484,502,493,507,496,487,
    480,485,476,476,478,472,476,473,477,477,479,483,483,479,477,467,454,453,
    457,487,513,523,530,540,481,481,477,476,456,437,447,485,499,502,500,498,
    501,492,492,502,490,492,493,475,487,478,473,484,479,476,465,451,468,484,
    488,496,507,495,506,493,484,471,473,487,493,491,510,518,517,523,491,495,
    496,476,469,477,491,478,481,488,474,427,423,492,621,597,469,481,399,441,
    456,488,500,485,489,488,489,481,482,474,448,474,487,515,502,474,475,483,
    476,458,436,462,478,493,487,501,598,493,478,471,476,465,465,478,467,455,
    475,468,578,453,472,471,453,469,484,487,468,465,448,445,473,494,504,505,
    503,525,511,495,499,488,486,471,488,478,475,483,469,490,479,467,479,467,
    472,470,475,479,508,506,486,477,474,472,482,475,451,473,482,464,480,463,
    455,483,577,461,483,463,464,496,503,502,494,438,469,486,492,494,475,489,
    477,491,478,487,479,497,592,490,488,501,482,493,493,486,458,435,428,487,
    482,488,494,522,513,493,488,476,481,493,480,474,444,452,475,497,496,630,
    470,475,447,446,477,465,490,498,499,493,480,480,479,486,490,495,483,481,
    467,486,479,479,467,618,481,486,471,479,482,470,463,468,479,480,495,485,
    510,488,475,494,511,516,516,483,476,461,460,455,473,495,508,500,507,505,
    503,488,491,479,489,502,493,492,494,483,486,483,476,597,506,508,504,505,
    497,634,479,439,434,443,443,438,413,414,428,438,483,511,508,493,507,502,
    483,486,495,491,473,480,456,497,545,528,532,481,488,475,467,458,452,466,
    499,499,498,499,500,491,495,497,498,510,495,495,467,433,470,462,487,505,
    498,482,496,488,492,506,483,487,438,456,487,487,505,513,500,487,479,478,
    503,504,496,493,515,544,488,483,485,488,480,489,475,433,425,491,500,535,
    494,474,443,473,479,443,455,602,497,499,493,487,485,482,484,497,498,499,
    496,485,483,479,468,479,466,489,506,491,510,516,499,406,410,415,405,404,
    426,427,430,450,470,488,490,505,508,490,485,486,485,502,485,492,493,613,
    591,479,481,487,481,480,466,456,453,431,472,483,492,495,485,496,463,481,
    473,443,499,582,496,494,471,479,504,478,482,455,438,428,383,439,606,482,
    486,472,455,449,457,456,470,470,583,587,475,480,451,469,511,500,475,471,
    445,438,468,497,502,493,462,409,399,425,458,482,487,476,469,473,475,463,
    488,480,484,484,487,479,471,480,496,498,494,471,467,473,473,483,478,465,
    438,451,487,597,480,486,467,481,474,481,490,480,479,468,461,464,486,498,
    531,533,497,437,457,452,458,487,498,469,466,467,481,474,464,468,484,479,
    436,417,479,505,509,526,477,481,471,470,469,465,482,476,483,492,487,478,
    479,481,485,470,466,489,496,498,490,492,497,489,502,489,496,502,492,489,
    483,472,485,492,496,472,489,488,464,471,476,485,470,480,591,475,474,473,
    482,473,480,470,595,463,462,448,469,465,461,573,446,447,434,446,446,460,
    534,461,447,409,435,444,436,459,548,459,464,452,443,470,597,611,515,514,
    482,483,469,448,629,593,516,501,454,473,484,477,630,519,510,510,515,499,
    506,512,512,512,505,521,520,509,511,509,503,493,510,509,499,493,511,514,
    506,500,509,507,506,493,620,512,513,508,510,502,513,507,497,511,507,502,
    494,503,498,498,493,499,500,498,501,498,504,494,499,497,495,502,491,498,
    485,476,495,509,495,506,498,515,501,501,506,493,497,493,485,484,488,480,
    485,501,491,499,485,492,483,473,479,472,464,485,507,496,492,483,474,474,
    480,470,472,476,486,477,473,583,621,512,492,464,483,481,476,477,480,489,
    495,482,488,493,603,631,515,514,486,474,490,603,475,481,453,433,453,470,
    463,625,504,487,501,518,477,471,471,452,425,455,487,478,502,502,507,523,
    503,517,503,512,518,501,502,514,506,500,492,504,512,508,506,499,504,502,
    501,513,508,510,511,506,505,504,506,504,517,516,505,497,503,505,507,516,
    517,508,507,511,511,514,512,507,510,498,492,492,510,503,511,501,495,479,
    495,510,492,491,496,484,463,475,492,485,493,475,479,455,430,478,483,491,
    629,513,516,509,498,500,496,483,486,494,496,502,501,496,511,497,494,483,
    499,513,499,500,624,507,510,500,510,504,505,510,603,504,496,494,488,496,
    498,494,481,478,499,518,483,463,478,462,474,514,507,511,512,510,510,506,
    500,503,511,503,497,507,514,508,503,498,505,502,503,497,496,502,496,502,
    504,508,509,514,517,497,483,497,500,501,492,500,508,513,514,512,498,483,
    506,489,491,491,506,512,505,506,495,509,507,508,503,505,615,504,514,478,
    473,481,487,496,493,500,507,482,472,480,481,484,513,513,502,487,492,498,
    617,495,481,472,472,481,480,488,519,518,498,496,490,491,494,501,496,499,
    496,489,484,464,611,508,498,475,489,479,494,497,495,491,496,502,506,510,
    499,587,509,512,493,495,496,500,506,507,502,497,485,479,477,620,637,526,
    522,510,487,492,491,600,479,476,479,481,467,482,514,532,494,495,491,486,
    499,500,509,503,506,485,500,506,491,588,511,517,483,498,497,498,512,635,
    521,517,496,507,515,499,603,500,504,480,483,488,498,510,505,497,486,504,
    508,509,518,622,608,522,523,502,482,485,623,595,474,483,475,480,475,503,
    510,529,510,503,493,499,495,502,510,508,502,507,508,513,628,628,515,508,
    506,517,504,506,515,501,504,491,482,482,494,492,467,475,482,513,514,520,
    512,515,506,517,512,507,506,510,505,482,473,494,504,516,510,511,503,521,
    508,516,508,514,521,508,504,506,516,506,506,506,503,506,489,491,498,508,
    513,625,568,489,486,486,488,500,486,595,513,517,506,508,488,507,508,517,
    512,510,509,492,500,503,611,498,492,483,489,494,490,495,613,505,491,475,
    481,500,494,481,465,473,470,446,475,499,602,466,472,475,464,459,481,497,
    487,491,478,481,495,480,496,492,439,444,464,494,495,472,493,492,480,479,
    491,494,505,605,596,511,515,510,511,500,498,507,505,510,507,490,469,465,
    502,494,502,508,507,505,511,508,504,497,504,499,496,498,499,499,501,508,
    490,493,504,497,517,497,488,494,501,488,493,515,505,491,496,499,484,514,
    499,501,497,495,486,502,498,504,485,610,607,495,493,472,492,506,489,478,
    496,501,499,484,484,488,495,506,503,501,498,491,497,493,486,495,504,488,
    487,479,481,497,513,511,504,494,484,465,580,486,463,476,468,609,600,477,
    472,580,475,477,475,471,480,485,482,482,453,446,471,603,592,491,514,514,
    522,514,514,506,511,504,508,507,501,513,509,498,520,509,502,517,518,501,
    511,512,501,486,490,498,490,501,509,511,520,510,504,517,504,511,515,508,
    517,503,510,519,503,504,502,504,516,510,506,510,514,
};

static char *m_audio_file_name = NULL;
static FILE *audio_fp = NULL;
static UINT8 *audio_frame = NULL;

static void es_player_audio_task(UINT32 para1, UINT32 para2)
{
    ali_audio_config *dec_config = (ali_audio_config *)para1;
    unsigned long long file_size;
    RET_CODE ret = RET_SUCCESS;
    UINT32 i = 0, free_size = 0, valid_size = 0, total_size = 0;
    UINT32 read_size, need_read = 1;
    struct av_packet pkt;
    const UINT16 audio_valid_size = 1024;

    ali_sbm_info pe_info;
    MEMSET(&pe_info, 0, sizeof(ali_sbm_info));

    //fill the sbm config
    pe_info.mem_start = __MM_VIDEO_FILE_BUF_ADDR;
    pe_info.mem_size = __MM_VIDEO_FILE_BUF_LEN;

    #if (SYS_SDRAM_SIZE == 128 || SYS_SDRAM_SIZE == 64)
    // video file buffer is seperated with frame buffer
    #if (__MM_FB_TOP_ADDR > __MM_VIDEO_FILE_BUF_ADDR)
    pe_info.mem_size = __MM_FB_TOP_ADDR - __MM_VIDEO_FILE_BUF_ADDR;
    #endif
    pe_info.decoder_start = __MM_FB_BOTTOM_ADDR;
    pe_info.decv_buf_size = __MM_FB_TOP_ADDR - __MM_FB_BOTTOM_ADDR;
    #endif

    dec_config->codec_id = 0x15002;
    dec_config->av_sync_mode = 1;
    dec_config->av_sync_unit = 0;
    dec_config->bits_per_coded_sample = 32;
    dec_config->sample_rate = 48000;
    dec_config->channels = 2;
    dec_config->bit_rate = 0;

    m_audio_file_name = NULL;

    if(m_audio_file_name == NULL)
    {
        m_audio_file_name = "/mnt/uda1/es_player/Test_48000.dat";
    }

    MEMSET(&pkt, 0, sizeof(pkt));

    audio_fp = fopen(m_audio_file_name, "rb");
    if(audio_fp == NULL)
    {
        libc_printf("Open file %s fail\n", m_audio_file_name);
        return -1;
    }

    fseek(audio_fp, 0, SEEK_END);
    file_size = ftell(audio_fp);
    fseek(audio_fp, 0, SEEK_SET);
    if(file_size <= 0)
    {
        libc_printf("File size error %lld\n", file_size);
        return -1;
    }

    //fflush(fp);
    libc_printf("File size %lld", file_size);

    audio_frame = (UINT8 *)MALLOC(0x100000);
    if(audio_frame == NULL)
    {
        libc_printf("Malloc frame buffer fail\n");
        return -1;
    }

    ret = ali_audio_decoder_open(dec_config, pe_info);
    if(ret < 0)
    {
        libc_printf("open audio decoder fail\n");
        return -1;
    }

    ret = ali_sbm_show_total_size(APKT_DATA_SBM_IDX, &total_size);
    if(ret < 0)
    {
        libc_printf("sbm ioctl fail\n");
    }

    while(1)
    {
        ret = ali_sbm_show_free_size(APKT_DATA_SBM_IDX, &free_size);
        if(ret < 0)
        {
            libc_printf("sbm ioctl fail\n");
        }

        ret = ali_sbm_show_valid_size(APKT_DATA_SBM_IDX, &valid_size);
        if(ret < 0)
        {
            libc_printf("sbm ioctl fail\n");
        }

        if(need_read)
        {
            /* parse a complete frame */
            read_size = fread(audio_frame, 1, audio_frame_size[i], audio_fp);
        }

        if(read_size == audio_frame_size[i])
        {
            if((free_size > read_size) && (valid_size < total_size*4/5))
            {
                pkt.pts = AV_NOPTS_VALUE;
                pkt.dts = AV_NOPTS_VALUE;
                pkt.size = read_size;

                if(i == 0)
                {
                    UINT32 base_time = (pkt.pts==AV_NOPTS_VALUE)?0:pkt.pts;
                    ret = ali_audio_decore_ioctl(dec_config, DECA_DECORE_SET_BASE_TIME, &base_time, &base_time);
                    if(ret < 0)
                    {
                        libc_printf("deca decore set base time fail\n");
                    }
                }

                ret = ali_audio_decoder_write_header(&pkt);
                if(ret == 0)
                {
                    do
                    {
                        ret = ali_audio_decoder_write(audio_frame, read_size);

                        if(ret < 0)
                        {
                            libc_printf("Write audio packet data fail\n");
                            osal_task_sleep(10);
                        }
                    } while (ret < 0);

                }
            else
            {
                libc_printf("Write audio packet header fail\n");
                goto sleep;
            }

            if(++i >= ARRAY_SIZE(audio_frame_size))
                break;

            need_read = 1;
        }
        else
        {
            need_read = 0;
        }
    }
    else
    {
        if(valid_size <= audio_valid_size)
        {
            libc_printf("Read audio frame error %d %d\n", read_size, audio_frame_size[i]);
            goto task_exit;
        }
    }
sleep:
       osal_task_sleep(5);
    }

task_exit:
    ali_audio_decoder_close(dec_config);

    fclose(audio_fp);
    audio_fp = NULL;
    FREE(audio_frame);
    audio_frame = NULL;

    libc_printf("=====Exit playing file %s=====", m_audio_file_name);
}

RET_CODE win_es_player_audio_init(ali_audio_config *dec_config, OSAL_ID    tsk_id)
{
    OSAL_T_CTSK t_ctsk;

    //create es player task
    t_ctsk.itskpri = OSAL_PRI_HIGH;
    t_ctsk.stksz = 0x4000;
    t_ctsk.quantum = 10;
    t_ctsk.name[0] = 'e';
    t_ctsk.name[1] = 's';
    t_ctsk.name[2] = 'p';
    t_ctsk.para1 = (UINT32)dec_config;
    t_ctsk.task = es_player_audio_task;
    *(UINT32 *)tsk_id = osal_task_create(&t_ctsk);
    if(OSAL_INVALID_ID == *(UINT32 *)tsk_id)
    {
        return -1;
    }
}

RET_CODE win_es_player_audio_close(ali_audio_config * dec_config, OSAL_ID tsk_id)
{

    if(OSAL_INVALID_ID != tsk_id)
    {
        ali_audio_decoder_close(dec_config);
        osal_task_delete(tsk_id);
    }

    if(audio_fp != NULL)
    {
        fclose(audio_fp);
        audio_fp = NULL;
    }

    if(audio_frame != NULL)
    {
        FREE(audio_frame);
        audio_frame = NULL;
    }
}
/*****************************************end********************************/
#endif

