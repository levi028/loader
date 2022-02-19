/******************************************************************************
 *   $ Copyright $ Sony Corporation
 *-----------------------------------------------------------------------------
 *   File Name   : $File: sut_de202_dvbt_vXX.c $
 *   Modified    : $Date: 2010/10/15 $ By $Author:  $
 *   Revision    : $Revision: 2.10 $
 *   Description : SUT-DE202 Tuner Control Sample Code
 *                 This code is not include I2C communication function.
 *                 It is necessary to add codes for the I2C communication
 *                 properly to execute and compile the code.
 *-----------------------------------------------------------------------------
 * This program may contain information about unfinished products and is subject
 * to change without notice.
 * Sony cannot assume responsibility for any problems arising out of the
 * use of the program
 *****************************************************************************/
/******************************************************************************
 *   includes:
 *    - system includes
 *    - application includes
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "userdef.h"
#include "sut_de202_dvbt2.h"

 extern BLOCKID tuner_block_id ;
 extern BLOCKID demod_block_id ;
 
#if 0

/*************************************************
 I2C Read/Write
**************************************************/
static INT16 iic_write_val(INT16 id, UINT8 subadr, UINT8 val);
static INT16 iic_write(INT16 id, UINT8 subadr, UINT8 *data, UINT8 len);
static INT16 iic_read(INT16 id, UINT8 subadr, UINT8 *data, UINT8 len);

static INT16 write_tuner(
    UINT8 slv0,
    UINT8 slv1,
    UINT8 subadr,
    UINT8 *data,
    UINT8 len
);
static INT16 write_demod(
    UINT8 slv,
    UINT8 subadr,
    UINT8 *data,
    UINT8 len
);
static INT16 read_demod(
    UINT8 slv,
    UINT8 subadr,
    UINT8 *data,
    UINT8 len
);

/*************************************************
 Wait
**************************************************/
static void Waitms(UINT16 ms);

#endif
/******************************************************************************
 * main
 *-----------------------------------------------------------------------------
 * Input : INT16 argc, char* argv[]
 * Output: none
 * Return: INT16
 *****************************************************************************/
int main(int argc, char* argv[]) {
    INT16 i;

    if (argc < 2) {
        cmd_help(argc - 1, argv);
        return 1;

    }

/*------------------ I2C  Driver open ------------------------*/
/*    Add code to open I2C driver interface.                  */
/*------------------------------------------------------------*/


    /* cut first arg */
    for( i = 0; i < argc - 1; i++ ){ argv[i] = argv[i+1];}

    /* Search/Execute Command */
    if (strcmp(argv[0], "help") == 0) {
        cmd_help(argc - 1, argv);
    } else if (strcmp(argv[0], "init") == 0) {
        dvbt2_cmd_init(argc - 1, argv);
    } else if (strcmp(argv[0], "t2_wakeup") == 0) {
        cmd_init_dvbt2(argc - 1, argv);
    } else if (strcmp(argv[0], "t2_tune") == 0) {
        cmd_tune_dvbt2(argc - 1, argv);
    } else if (strcmp(argv[0], "t2_sleep") == 0) {
        cmd_sleep_dvbt2(argc - 1, argv);
    } else if (strcmp(argv[0], "t2_monitor") == 0) {
        cmd_get_dvbt2stat(argc - 1, argv);
    } else if (strcmp(argv[0], "t2_tsout") == 0) {
        cmd_tsout(argc - 1, argv);

    } else {
        cmd_error(argc - 1, argv);
    }

    /* 100ms wait for terminal display */
    Waitms(100);

/*------------------ I2C  Driver close -----------------------*/
/*    Add code to close I2C driver interface.                  */
/*------------------------------------------------------------*/

    return 0;
}

/******************************************************************************
 * cmd_init
 *-----------------------------------------------------------------------------
 * 
 *****************************************************************************/
static int dvbt2_cmd_init(int argc, char **argv)
{
    DBG_PRINT("Enter cmd_init\n");
 
    /* tuner init  */
    dvbt2_tuner_init();
    /* demod sleep */
    dvbt2_demod_init();
    return 0;
}

/******************************************************************************
 * cmd_dvbt2_wakeup_dvbt2
 *-----------------------------------------------------------------------------
 * 
 *****************************************************************************/
static int cmd_init_dvbt2(int argc, char **argv)
{
    DBG_PRINT("Enter cmd_init_dvbt2\n");
    /* tuner awake */
    dvbt2_tuner_wakeup();
    /* demod wakeup */
    dvbt2_demod_wakeup();
    return 0;
}

/******************************************************************************
 * cmd_tune_dvbt2
 *-----------------------------------------------------------------------------
 *
 *****************************************************************************/
static int cmd_tune_dvbt2(int argc, char **argv)
{
    T2_DigitalTuningParam   param;
    T2_DEM_LOCK             demLockResult = DEM_UNKNOWN;
    T2_TS_LOCK              tsLockResult = DVBT2_TS_UNKNOWN;
    INT32                   offset;
    
    DBG_PRINT("Enter cmd_tune_dvbt2\n");

    if(argc > 1) param.m_frequency = atoi(argv[1]);
    else param.m_frequency = 666000;

    if (argc > 2) param.m_BandWidth = atoi(argv[2]);
    else param.m_BandWidth = 3;

    if (argc > 3) param.m_MplpSelect = atoi(argv[3]);
    else param.m_MplpSelect = 0;

    if (argc > 4) param.m_MplpId = atoi(argv[4]);
    else param.m_MplpId = 0;

    if( ( param.m_BandWidth != 2 )&&( param.m_BandWidth != 3 ) ) {
        libc_printf("not supported bandwidth now \n");
        return -1;
    }
    
    /* demod setparam */
    dvbt2_demod_setparam(&param );

    dvbt2_tuner_tune( &param );

    /* 2010-10-05 LongEchoSeq_Ena is used to select DVB-T2 Acquisition Sequnece.
          LongEchoSeq_Ena = 0 : DVB-T2 Lock Sequence
          LongEchoSeq_Ena = 1 : DVB-T2 Long Echo Sequence
    */
    if (LongEchoSeq_Ena==0) {
        /* DVB-T2 Lock Sequence */
        /* wait DVB-T2 lock */
    demodLockSequence(&param, &demLockResult, &tsLockResult, &offset);    /* demod Lock Sequence */

    /* If carrier offset > threshold, Go back to tuner setting */
    if( ( offset > OFFSET_THRESHOLD )||( -1 * offset > OFFSET_THRESHOLD ) ) {
        libc_printf("Go back to tuner setting with compensated frequency (%ld[kHz]) ---- \n", ( param.m_frequency + offset ) );
        
        param.m_frequency += offset;

        dvbt2_tuner_tune( &param );

            /* wait DVB-T2 lock */
        demodLockSequence(&param, &demLockResult, &tsLockResult, &offset);    /* demod Lock Sequence */
    }
    } else {
        /* 2010-10-05 DVB-T2 Long Echo Sequence has been added */
        /* Demod SRST */
        dvbt2_demod_start();

        Waitms(100);

        /* wait DVB-T2 demod lock */
        t2_le_Seq(&demLockResult);
        
        if( demLockResult == DEM_LOCK ) {
            /* Check Carrier Offset */
            dvbt2_get_CarrierOffset(param.m_BandWidth, &offset);
            libc_printf("CarrierOffset %ld \n",offset);
            
            /* If carrier offset > threshold, Go back to tuner setting */
            if( ( offset > OFFSET_THRESHOLD )||( -1 * offset > OFFSET_THRESHOLD ) ) {
                libc_printf("Go back to tuner setting with compensated frequency (%ld[kHz]) ---- \n", ( param.m_frequency + offset ) );
        
                param.m_frequency += offset;

                dvbt2_demod_setparam(&param);

                dvbt2_tuner_tune(&param);
                
                /* Demod SRST */
                dvbt2_demod_start();

                Waitms(100);

                /* wait DVB-T2 demod lock */
                t2_le_Seq(&demLockResult);
                
                if( demLockResult == DEM_LOCK ) {
                    /* wait DVB-T2 TS lock */
                    t2_le_SeqTsLock(&tsLockResult);    
                } else {
                    /* Demod Unlock */
                }
            } else {
                /* wait DVB-T2 TS lock */
                t2_le_SeqTsLock(&tsLockResult);
            }
        }
        libc_printf("DemodLock %d\n",demLockResult);
        libc_printf("TSLock    %d\n",tsLockResult);
    }

    return 0;
}

/******************************************************************************
 * cmd_sleep
 *-----------------------------------------------------------------------------
 *
 *****************************************************************************/
static int cmd_sleep_dvbt2(int argc, char **argv)
{
    DBG_PRINT("Enter cmd_sleep_dvbt2\n");
   
    /* DVB demod sleep */
    dvbt2_demod_sleep();
    /* tuner sleep */
    dvbt2_tuner_sleep();
    return 0;
}


/******************************************************************************
 * cmd_get_dvbt2stat
 *-----------------------------------------------------------------------------
 *
 *****************************************************************************/
static int cmd_get_dvbt2stat(int argc, char **argv)
{
    UINT8   SyncStat;
    UINT8   TSLockStat;
    UINT16  IFAGCOut;
    INT16   RSSI;                           /* 2010-05-21 add */
    INT32   Offset;
    INT16   CN_q2;
    UINT32  PreBchBitError, PreBchBerPeriod;
    UINT32  PostBchFrameError, PostBchFerPeriod;
    T2_DigitalTuningParam   param;
    /* 2010-06-21 delete T2_DemodInfo Info : This data has been merged to L1PreData */
    UINT8           Current_PLP_ID;         /* 2010-05-21 add */
    PLPSEL_FLAG     PlpSelFlag;             /* 2010-05-21 add */
    UINT8   plpNum;
    UINT8   plpId[255];
    L1CHG_RECOVERY_FLAG L1ChgRecoveryFlag;  /* 2010-05-21 add */
    T2_L1PreInfo  L1PreData;                /* 2010-06-21 add */
    T2_L1PostInfo L1PostData;               /* 2010-06-21 add */
    UINT16  SSI;                            /* 2010-10-15 add */
    UINT16  SQI;                            /* 2010-10-15 add */
    
    if (argc > 1) param.m_BandWidth = atoi(argv[1]);
    else param.m_BandWidth = 3;

    if( ( param.m_BandWidth != 2 )&&( param.m_BandWidth != 3 ) ) {
        libc_printf("not supported bandwidth\n");
        return -1;
    }
    
    dvbt2_get_syncstat(&SyncStat, &TSLockStat);
    dvbt2_get_ifagc(&IFAGCOut);
    dvbt2_get_rssi(&RSSI);                  /* 2010-05-21 add */
    dvbt2_get_CarrierOffset(param.m_BandWidth, &Offset);
    dvbt2_get_CN(&CN_q2);
    dvbt2_get_PreBCHBer(&PreBchBitError, &PreBchBerPeriod);
    dvbt2_get_PostBCHFer(&PostBchFrameError, &PostBchFerPeriod);
    /* 2010-06-21 delete dvbt2_get_T2DemodInfo -> This monitoring function has been merged to dvbt2_get_T2L1PreInfo  */
    dvbt2_get_T2PLPSelFlag(&PlpSelFlag, &Current_PLP_ID);    /* 2010-05-21 add */
    dvbt2_get_PlpIdInfo(&plpNum, plpId);

    /* SyncStat */
    libc_printf("\n--------------------------------------------\n");
    if(SyncStat == 6){
        libc_printf("-----Demod       = <     LOCKED     >\n");
    }else{
        libc_printf("-----Demod       = <    UNLOCKED    >\n");
    }
    if(TSLockStat){
        libc_printf("-----TS OutPut   = <     LOCKED     >\n");
    }else{
        libc_printf("-----TS OutPut   = <    UNLOCKED    >\n");
    }
    /* IFAGC */

    libc_printf("-----IFAGC out   = <     %4d       >\n", IFAGCOut);
    libc_printf("-----RSSI [dBm]    <     %4d.%01d    >\n", RSSI/10, abs(RSSI % 10) ); /* 2010-05-21 add */

    /* CarrierOffset */
    libc_printf("--Cariier Offset = <     %4ld kHz   >\n", Offset);

    /* CN */
    libc_printf("-----CN          = <     %2d.%02d      >\n", ( CN_q2 / 4 ), ( 0x03 & CN_q2 ) * 25);
    
    
    /* PreBCH-Ber */
    libc_printf("-----Pre BCH-Ber = <   BitError=%ld Period=%ld >\n", PreBchBitError, PreBchBerPeriod);
    
    /* PostBCH-Fer */
    libc_printf("-----PostBCH-Fer = < FrameError=%ld Period=%ld >\n", PostBchFrameError, PostBchFerPeriod);

    /* 2010-06-21 demod info output has been removed. This monitoring was merged to dvbt2_print_T2L1PreInfo */

    dvbt2_print_PlpIdInfo(plpNum, plpId);

    /* PLP Sel Error Flag */ /* 2010-05-21 add */
    switch(PlpSelFlag){
    case PLPSEL_OK:
        libc_printf("-----PLP Sel     = <   PLP Sel OK    >\n"); break;
    case PLPSEL_ERROR:
        libc_printf("-----PLP Sel     = <   PLP Sel Error >\n"); break;
    default:
        libc_printf("-----PLP Sel     = <  Unknown PLP Sel Error Flag (%d) >\n", PlpSelFlag); break;
    }
    
    /* Current PLP_ID */ /* 2010-05-21 add */
    libc_printf("Current_PLP_ID   = <       %4d      >\n", Current_PLP_ID);

    /* L1 Change Recovery */ /* 2010-05-21 add */
    dvbt2_L1ChangeRecovery( &L1ChgRecoveryFlag );
    switch(L1ChgRecoveryFlag){
    case NO_CHANGE:
        libc_printf("-----L1Change    = < No Change       >\n"); break;
    case CHANGE_AND_SRST:
        libc_printf("-----L1Change    = < Change and SRST >\n"); break;
    default:
        libc_printf("-----L1Change    = < No Change  (%d) >\n", L1ChgRecoveryFlag); break;
    }

    /* L1 Pre/Post Signaling Data */
    dvbt2_get_T2L1PreInfo(&L1PreData);              /* 2010-06-21 add */
    dvbt2_get_T2L1PostInfo(&L1PostData);            /* 2010-06-21 add */

    dvbt2_print_T2L1PreInfo(&L1PreData);            /* 2010-06-21 add */
    dvbt2_print_T2L1PostInfo(&L1PostData);          /* 2010-06-21 add */

    /* SSI/SQI */ /* 2010-10-15 add */
    dvbt2_get_ssi_sqi(&SQI,&SSI);
    libc_printf("SSI %d\n",SSI);
    libc_printf("SQI %d\n",SQI);
    return 0;
}

/******************************************************************************
 * cmd_tsout
 *-----------------------------------------------------------------------------
 *
 *****************************************************************************/
static int cmd_tsout(int argc, char **argv)
{
    UINT8 ts_ena;
    if(argc > 1) ts_ena = atoi(argv[1]);
    else ts_ena = 0;
    dvbt2_tsout(ts_ena);
    return 0;
}

/******************************************************************************
 * cmd_help
 *-----------------------------------------------------------------------------
 * help
 *****************************************************************************/
static int cmd_help(int argc, char **argv)
{
    libc_printf("\n\n");

    libc_printf("------------------------------ \n");
    libc_printf("####### COMMANDS #######\n");
    libc_printf("init\n");
    libc_printf("        tuner initialization\n");
    libc_printf("\n");
    libc_printf("t2_wakeup\n");
    libc_printf("        register setting for DVB-T2 reveive\n");
    libc_printf("\n");
    libc_printf("t2_tune [freq] [bandwidth] [Mplp_enable] [Mplp_id]: \"t2_tune 666000 3 0 0\" \n");
    libc_printf("        DVB-T2 tuning\n");
    libc_printf("        freq        :center frequency [kHz] \n");
    libc_printf("        bandwidth   :band width setting 1:6MHz, 2:7MHz, 3:8MHz (supported 8MHz only)\n");
    libc_printf("        Mplp_enable :Select Mplp Enable 0:DefaultPLP  1:PLP selection\n");
    libc_printf("        Mplp_id     :Mplp_id [id number]\n");
    libc_printf("\n");
    libc_printf("t2_sleep\n");
    libc_printf("        sleep\n");
    libc_printf("t2_tsout [tsout]\n");
    libc_printf("        tsout       : Tsout enable flag  0:disable Tsout, 1:enable Tsout\n");
    libc_printf("\n");
    libc_printf("t2_monitor [bandwidth] \n");
    libc_printf("        bandwidth   :band width setting 1:6MHz, 2:7MHz, 3:8MHz (supported 8MHz only)\n");
    libc_printf("        get status(Lock-Status/IFAGC-Level/Carrier-OFFSET/CN/preBCH-BER/postBCH-FER/PLP_info)\n");
    libc_printf("\n");
    libc_printf("------------------------------ \n");
    libc_printf("SUT-DE202 Sample Code \n");
    libc_printf(" (C) Sony corporation\n ");
    libc_printf("     all right reserved.\n\n");

    return 0;
}

/******************************************************************************
 * cmd_error
 *-----------------------------------------------------------------------------
 * 
 *****************************************************************************/
static int cmd_error(int argc, char **argv)
{
    libc_printf("Command Error\n");
    cmd_help(argc, argv);
    return -1;
}


/******************************************************************************
 * tuner_init
 *-----------------------------------------------------------------------------
 * Input        : none
 * Output       : none
 * Return       : none
 * Description  : setting output ports HiZ, disable all functions.
 *****************************************************************************/
 void dvbt2_tuner_init(void)
{
    DBG_PRINT("Enter tuner_init\n");
    
    iic_write_val( tuner_block_id, 0x01, 0x29 );
    iic_write_val( tuner_block_id, 0x44, 0x07 );
    iic_write_val( tuner_block_id, 0x04, 0x40 );
    iic_write_val( tuner_block_id, 0x14, 0xFB );
    iic_write_val( tuner_block_id, 0x21, 0x18 );

    Waitms(100);

    iic_write_val( tuner_block_id, 0x07, 0x00 );    /* 2010-05-21 add */
    iic_write_val( tuner_block_id, 0x04, 0x00 );    /* 2010-05-21 add */
    iic_write_val( tuner_block_id, 0x03, 0xC0 );	/* 2010-05-21 add */

    dvbt2_tuner_sleep();
}

/******************************************************************************
 * dvbt2_tuner_wakeup
 *-----------------------------------------------------------------------------
 * Input        : none
 * Output       : none
 * Return       : none
 * Description  : Terrestrial Digital Awake
 *****************************************************************************/
 void dvbt2_tuner_wakeup(void)
{
    UINT8 taddr;
    UINT8 tdata[2];
    UINT8 data_length;

    DBG_PRINT("Enter dvbt2_tuner_wakeup\n");
    
    taddr    = 0x14;
    tdata[0] = 0xFB;    /* 0x14 */
    tdata[1] = 0x0F;    /* 0x15 */
    data_length = 2;
    iic_write( tuner_block_id, taddr, tdata, data_length );
    
    return;

}

/******************************************************************************
 * dvbt2_tuner_tune
 *-----------------------------------------------------------------------------
 * Input        : T2_DigitalTuningParam* pDigParam
 * Output       : none
 * Return       : none
 * Description  : Terrestrial Digital Tuning
 *****************************************************************************/
 void dvbt2_tuner_tune(T2_DigitalTuningParam* pDigParam)
{
    UINT8 taddr;
    UINT8 tdata[5];
    UINT8 data_length;
    INT32 frequency;
    UINT8 bandWidthRegValue = 0x20;

    DBG_PRINT("Enter dvbt2_tuner_tune\n");
    
    setRfTunerParam( pDigParam->m_BandWidth );  /* set BANK 0x00 in this function*/
    
    frequency = pDigParam->m_frequency;

#if 0
    taddr    = 0x03;                                        /* 2010-05-21 add */
    tdata[0] = 0xC4;                                        /* 2010-05-21 add */
    tdata[1] = 0x40;                                        /* 2010-05-21 add */
    data_length = 2;                                        /* 2010-05-21 add */
    iic_write( tuner_block_id, taddr, tdata, data_length ); /* 2010-05-21 add */
#endif

    taddr    = 0x03;
    tdata[0] = 0xC4;
    data_length = 1;
    iic_write( tuner_block_id, taddr, tdata, data_length );

    taddr    = 0x04;
    tdata[0] = 0x40;
    data_length = 1;
    iic_write( tuner_block_id, taddr, tdata, data_length );

    switch( pDigParam->m_BandWidth )
    {
        case 1: /* 6MBW */
			/* DVB-T2 6MBW is not supported */
            bandWidthRegValue = 0x00;
            break;
        case 2: /* 7MBW */
            bandWidthRegValue = 0x10;
            break;
        case 3: /* 8MBW */
        default:
            bandWidthRegValue = 0x20;
            break;
    }
    
    taddr = 0x10;
    
    tdata[0] = frequency & 0xff;
    tdata[1] = (frequency & 0xff00) >> 8;
    tdata[2] = bandWidthRegValue | ((frequency & 0x0f0000) >> 16); 
    tdata[3] = 0xff;
    tdata[4] = 0xff;
    data_length = 5;
   
    iic_write( tuner_block_id, taddr, tdata, data_length);

    Waitms(100);         /* 2010-05-21 add */

    iic_write_val( tuner_block_id, 0x0C, 0x03 );    //For meet DTG LTE SPEC, according to DE202 change_for_LTE_test.pdf.
    iic_write_val( tuner_block_id, 0x04, 0x00 );         /* 2010-05-21 add */
    iic_write_val( tuner_block_id, 0x03, 0xC0 );         /* 2010-05-21 add */

    return;
}





/******************************************************************************
 * dvbt2_tuner_sleep
 *-----------------------------------------------------------------------------
 * Input        : none
 * Output       : none
 * Return       : none
 * Description  : stop tuning
 *****************************************************************************/
void dvbt2_tuner_sleep(void)
{
    UINT8 taddr;
    UINT8 tdata[2];
    UINT8 data_length;

    DBG_PRINT("Enter dvbt2_tuner_sleep\n");

    taddr    = 0x14;
    tdata[0] = 0x00;    /* 0x14 */
    tdata[1] = 0x04;    /* 0x15 */
    data_length = 2;
    iic_write( tuner_block_id, taddr, tdata, data_length );
    
    return;
}

/******************************************************************************
 * setRfTunerParam
 *-----------------------------------------------------------------------------
 * Input : UINT8  bandWidth
 * Output: none
 * Return: void
 *
 *****************************************************************************/
void setRfTunerParam(UINT8 bandWidth)
{

    UINT8 taddr;
    UINT8 tdata[5];
    UINT8 data_length;

    taddr    = 0x05;
    tdata[0] = 0x00;           /* 0x05 */
    tdata[1] = 0x0A;           /* 0x06 */
    tdata[2] = 0x00;           /* 0x07 */
    tdata[3] = 0x0E;           /* 0x08 */
    data_length = 4;
    iic_write( tuner_block_id, taddr, tdata, data_length );

    taddr    = 0x0B;
    tdata[0] = 0x25;           /* 0x0B */
    tdata[1] = 0x13;           /* 0x0C */
    tdata[2] = 0x20;           /* 0x0D */
    if (bandWidth == 1 ) {                /* BW 6M */
        /*  6MBW is currently not supported */
        tdata[3] = 0x05;       /* 0x0E */
        tdata[4] = 0x00;       /* 0x0F */
    } else if (bandWidth == 2 ) {         /* BW 7M */
        tdata[3] = 0xED;       /* 0x0E */
        tdata[4] = 0x00;       /* 0x0F */
    } else {                              /* BW 8M */
        tdata[3] = 0xED;       /* 0x0E */
        tdata[4] = 0x00;       /* 0x0F */
    }
    data_length = 5;
    iic_write( tuner_block_id, taddr, tdata, data_length );

    return ;
}

/******************************************************************************
 * dvbt2_demod_init
 *-----------------------------------------------------------------------------
 * Input        : none
 * Output       : none
 * Return       : none
 * Description  : DVB demod init
 *****************************************************************************/
void dvbt2_demod_init(void)
{
    DBG_PRINT("Enter dvbt2_demod_init\n");

    iic_write_val( demod_block_id, 0x00 , 0x00 );
    iic_write_val( demod_block_id, 0xff , 0x1f );
    iic_write_val( demod_block_id, 0x85 , 0x00 );
    iic_write_val( demod_block_id, 0x81 , 0x00 );
    iic_write_val( demod_block_id, 0x80 , 0x00 );

    return;
}

/******************************************************************************
 * dvbt2_demod_wakeup
 *-----------------------------------------------------------------------------
 * Input        : none
 * Output       : none
 * Return       : none
 * Description  : DVB demod init
 *****************************************************************************/
void dvbt2_demod_wakeup(void)
{

    DBG_PRINT("Enter dvbt2_demod_wakeup\n");

    iic_write_val( demod_block_id, 0x00 , 0x00 );
    iic_write_val( demod_block_id, 0x80 , 0x02 );
    iic_write_val( demod_block_id, 0x81 , 0x20 );
    iic_write_val( demod_block_id, 0x85 , 0x07 );
    
    iic_write_val( demod_block_id, 0x00 , 0x20 );
    iic_write_val( demod_block_id, 0x69 , 0x01 );


    iic_write_val( demod_block_id, 0x00 , 0x00 );
    iic_write_val( demod_block_id, 0xff , 0x09 );
    return;
}

/******************************************************************************
 * dvbt2_demod_setparam
 *-----------------------------------------------------------------------------
 * Input        : T2_DigitalTuningParam* pDigParam
 * Output       : none
 * Return       : none
 * Description  : Terrestrial Digital Tuning
 *****************************************************************************/
void dvbt2_demod_setparam(T2_DigitalTuningParam* pDigParam)
{
    INT16 i;

    DBG_PRINT("Enter dvbt2_demod_setparam\n");

    /* T2 Common */
    for(i = 0; i < CORE_COMMON_REGSETTING_MAX_ENTRY; i++ ) {
        iic_write_val( demod_block_id, CoreRegSettingT2_COMMON[i].m_address, CoreRegSettingT2_COMMON[i].m_data);
    }
    
    switch( pDigParam->m_BandWidth )
    {
        case 1: /* 6MBW */
            /* 6MBW is not supported */
            break;
        case 2: /* 7MBW */
            for(i = 0; i < CORE_REGSETTING_MAX_ENTRY; i++ ) {
                iic_write_val( demod_block_id, CoreRegSettingT2_7MBW[i].m_address, CoreRegSettingT2_7MBW[i].m_data);
            }
            break;
        case 3: /* 8MBW */
        default:
            for(i = 0; i < CORE_REGSETTING_MAX_ENTRY; i++ ) {
                iic_write_val( demod_block_id, CoreRegSettingT2_8MBW[i].m_address, CoreRegSettingT2_8MBW[i].m_data);
            }
            
            break;
    }

 
    if( pDigParam->m_MplpSelect == 0 ) {
        /* default PLP */
        iic_write_val( demod_block_id, 0x00, 0x23 );
        iic_write_val( demod_block_id, 0xad, 0x00 );
        iic_write_val( demod_block_id, 0xaf, 0x00 );
        iic_write_val( demod_block_id, 0x00, 0x00 );
    } else {
        /* Selected PLP */
        iic_write_val( demod_block_id, 0x00, 0x23 );
        iic_write_val( demod_block_id, 0xad, 0x01 );
        iic_write_val( demod_block_id, 0xaf, pDigParam->m_MplpId );
        iic_write_val( demod_block_id, 0x00, 0x00 );
    }
    
    
    /* Prepare for T2 Tuning */
    iic_write_val( demod_block_id, 0x00, 0x00 );        /*Bank 0x00 */
    iic_write_val( demod_block_id, 0xff, 0x09 );
    
    
    return;
}

/******************************************************************************
 * dvbt2_demod_start
 *-----------------------------------------------------------------------------
 * Input        : none
 * Output       : none
 * Return       : none
 * Description  : Terrestrial Digital Tuning
 *****************************************************************************/
void dvbt2_demod_start(void)
{
  
    DBG_PRINT("Enter dvbt2_demod_start\n");
    
    iic_write_val( demod_block_id, 0x00, 0x00 );
    iic_write_val( demod_block_id, 0xfe, 0x01 );
    return;
    
}

/******************************************************************************
 * dvbt2_demod_sleep
 *-----------------------------------------------------------------------------
 * Input        : none
 * Output       : none
 * Return       : none
 * Description  : DVB demod sleep
 *****************************************************************************/
void dvbt2_demod_sleep(void)
{

    DBG_PRINT("Enter dvbt2_demod_sleep\n");

    iic_write_val( demod_block_id, 0x00, 0x00 );
    iic_write_val( demod_block_id, 0xff, 0x1f );
    iic_write_val( demod_block_id, 0x85, 0x00 );

    iic_write_val( demod_block_id, 0x00, 0x20 );
    iic_write_val( demod_block_id, 0x69, 0x00 );

    iic_write_val( demod_block_id, 0x00, 0x00 );
    iic_write_val( demod_block_id, 0x81, 0x00 );
    iic_write_val( demod_block_id, 0x80, 0x00 );

    return;

}


/******************************************************************************
 * dvbt2_tsout
 *-----------------------------------------------------------------------------
 * Input        : UINT8 enable
 * Output       : none
 * Return       : none
 * Description  : Terrestrial Digital Tuning
 *****************************************************************************/
void dvbt2_tsout( UINT8 enable )
{

    iic_write_val( demod_block_id, 0x00, 0x00 );

    if( enable == 1 )
        iic_write_val( demod_block_id, 0xff , 0x08 );
    else
        iic_write_val( demod_block_id, 0xff , 0x09 );
    return;
}


/******************************************************************************
 * demodLockSequence
 *-----------------------------------------------------------------------------
 * Input : T2_DigitalTuningParam*  pParam
 * Output: T2_DEM_LOCK*            pDem_Lock
 *       : T2_TS_LOCK*             pTs_Lock
 *       : INT32*                  pOffset
 * Return: 
 *****************************************************************************/
void demodLockSequence(T2_DigitalTuningParam * pParam, T2_DEM_LOCK* pDem_Lock, T2_TS_LOCK* pTs_Lock, INT32* pOffset)
{
    /* ------- state */
    T2_DemodSeqState LockState;

    DBG_PRINT("Enter demodLockSequence\n");

    /*---------------------- initialization  */
    LockState.m_state             = DEMOD_SEQ_START;
    LockState.m_dem_lock          = DEM_UNKNOWN;
    LockState.m_ts_lock           = DVBT2_TS_UNKNOWN;
    LockState.m_waittime          = 0;
    LockState.m_seq_count         = 0;
    LockState.m_spectrum_flag     = 0;
    LockState.m_frequency         = pParam->m_frequency;
    LockState.m_BandWidth         = pParam->m_BandWidth;
    LockState.m_offset            = 0;


    iic_write_val( demod_block_id, 0x00, 0x20 );        /*Bank 0x20 */

    /* Set Spectrum Normal */
    iic_write_val( demod_block_id, 0xb5, 0x00 );
    /* Initial TS mode  */
    iic_write_val( demod_block_id, 0xe0, 0x0f );
    iic_write_val( demod_block_id, 0xe1, 0xff );
    
    iic_write_val( demod_block_id, 0x00, 0x00 );        /*Bank 0x00 */

    while( LockState.m_state != DEMOD_SEQ_END ) {
        Waitms( LockState.m_waittime );
        switch( LockState.m_state ) {
            case DEMOD_SEQ_START:
                dvbt2_demodlock_start_seq( &LockState );
                break;
            case DEMOD_SEQ_WAIT_DEM_LOCK:
                dvbt2_demodlock_wait_demlock( &LockState );
                break;
            case DEMOD_SEQ_WAIT_TS_LOCK:
                 dvbt2_demodlock_wait_tslock( &LockState );
                break;
            default:
                break;
        }
    }
    *pOffset     = LockState.m_offset;
    *pDem_Lock   = LockState.m_dem_lock;
    *pTs_Lock    = LockState.m_ts_lock;

    dvbt2_output_demodResults(&LockState);

}

/******************************************************************************
 * dvbt2_demodlock_start_seq
 *-----------------------------------------------------------------------------
 * Input : 
 * Output: T2_DemodSeqState* pLockState
 * Return: 
 *****************************************************************************/
void dvbt2_demodlock_start_seq( T2_DemodSeqState* pLockState )
{

    DBG_PRINT("Enter dvbt2_demodlock_start_seq\n");

    /* DVB-T2 Demod SRST  */
    dvbt2_demod_start();

    pLockState->m_dem_lock     = DEM_UNKNOWN;
    pLockState->m_ts_lock      = DVBT2_TS_UNKNOWN;
    pLockState->m_waittime     = 100;
    pLockState->m_state        = DEMOD_SEQ_WAIT_DEM_LOCK;

    return;
}

/******************************************************************************
 * dvbt2_demodlock_wait_demlock
 *-----------------------------------------------------------------------------
 * Input : 
 * Output: T2_DemodSeqState* pLockState
 * Return: 
 *****************************************************************************/
void dvbt2_demodlock_wait_demlock( T2_DemodSeqState* pLockState )
{
    UINT8  rdata[4] = {0};
    UINT8  offset_polarity = 0;
    UINT32 carrier_reg = 0;
    INT32  carrier_data = 0;
    INT32  offset = 0;               /* CarrierOffset [kHz] */
    UINT8  detectLock   = 1;
    UINT8  seq_maxstate = 0;
    UINT8  seq_lockstate = 0;
    
    DBG_PRINT("Enter dvbt2_demodlock_wait_demlock\n");
    
    pLockState->m_seq_count++; 

    /*------------------------------------*/
    /*-----------Unlock check-------------*/
    /*------------------------------------*/

    if( pLockState->m_seq_count == T2_UNLOCK_STATUS_CHECK1_TIMER ) {
        
        iic_write_val( demod_block_id, 0x00, 0x20 );           /* Bank 0x20 */

        iic_read( demod_block_id, 0x57, &detectLock, 1);
        detectLock = ( (detectLock & 0x10) >> 4 );
        
        iic_read( demod_block_id, 0x17, &seq_maxstate, 1);
        seq_maxstate = seq_maxstate & 0x07;
        
        iic_write_val( demod_block_id, 0x00, 0x00 );           /* Bank 0x00 */
        
        if( ( detectLock == 0 ) && ( seq_maxstate <= 2 ) ) {
           if( pLockState->m_spectrum_flag == 0 ) {
               pLockState->m_dem_lock      = DEM_UNKNOWN;
               pLockState->m_state         = DEMOD_SEQ_START;
               pLockState->m_spectrum_flag = 1;
               pLockState->m_seq_count     = 0;
               pLockState->m_waittime      = 0;
               
               iic_write_val( demod_block_id, 0x00, 0x20 );    /* Bank 0x20 */
               iic_write_val( demod_block_id, 0xb5, 0x10 );
               iic_write_val( demod_block_id, 0x00, 0x00 );    /* Bank 0x00 */

               return;
           } else {
               /* UNLOCK Detected */
               pLockState->m_dem_lock      = DEM_UNLOCK;
               pLockState->m_state         = DEMOD_SEQ_END;
               pLockState->m_spectrum_flag = 0;
               pLockState->m_waittime     = 0;
               
               iic_write_val( demod_block_id, 0x00, 0x20 );    /* Bank 0x20 */
               iic_write_val( demod_block_id, 0xb5, 0x00 );
               iic_write_val( demod_block_id, 0x00, 0x00 );    /* Bank 0x00 */
               
               return ;
           }
        } else {
           ;
        }
        
    } else if( pLockState->m_seq_count == T2_UNLOCK_STATUS_CHECK2_TIMER ) {
        
        iic_write_val( demod_block_id, 0x00, 0x20 );           /* Bank 0x20 */
        
        iic_read( demod_block_id, 0x17, &seq_maxstate, 1);
        seq_maxstate = seq_maxstate & 0x07;
        
        iic_write_val( demod_block_id, 0x00, 0x00 );           /* Bank 0x00 */
        
        if( seq_maxstate <= 2 ) {
            /* UNLOCK Detected */ 
            pLockState->m_dem_lock  = DEM_UNLOCK;
            pLockState->m_state     = DEMOD_SEQ_END;
            pLockState->m_spectrum_flag = 0;
            pLockState->m_waittime     = 0;
               
            iic_write_val( demod_block_id, 0x00, 0x20 );    /* Bank 0x20 */
            iic_write_val( demod_block_id, 0xb5, 0x00 );
            iic_write_val( demod_block_id, 0x00, 0x00 );    /* Bank 0x00 */
            
            return;
        } else {
            ;
        }
    }

    /*------------------------------------*/
    /*-----------Lock check---------------*/
    /*------------------------------------*/
    iic_write_val( demod_block_id, 0x00, 0x20 );               /* Bank 0x20 */
    iic_read( demod_block_id, 0x10, &seq_lockstate, 1);
    seq_lockstate = seq_lockstate & 0x07;

    /*------------------------------------*/
    /*-----------CarrierOffset------------*/
    /*------------------------------------*/
    iic_read( demod_block_id, 0x4c, rdata, 4 );
    iic_read( demod_block_id, 0xb5, &offset_polarity, 1 );
    iic_write_val( demod_block_id, 0x00, 0x00 );               /* Bank 0x00 */

    carrier_reg = ( ( (UINT32)(rdata[0] & 0x0f) << 24 ) ) 
                | ( ( (UINT32)rdata[1] << 16) )
                | ( ( (UINT32)rdata[2] << 8 ) )
                | (   (UINT32)rdata[3] );
    offset_polarity  = ( offset_polarity & 0x10 ) >> 4;

    carrier_data = dvb_Convert2SComplement(carrier_reg, 28);

    if( offset_polarity == 0 ) {
        carrier_data = -1 * carrier_data;
    }

    switch(pLockState->m_BandWidth){
        case 1:
            /* T.B.D. */
            break;
        case 2:
            /* T.B.D. */
            break;
        case 3:
            offset = carrier_data / 117441;
            break;
        default:
            offset = 0;
            break;
    }

    if(seq_lockstate >= 6) {
        if( ( offset > OFFSET_THRESHOLD )||( -1 * offset > OFFSET_THRESHOLD ) ) {
            /* In case that carrier offset is larger than 100[kHz], Lock Sequence is stopped.*/
            pLockState->m_dem_lock  = DEM_LOCK;
            pLockState->m_state     = DEMOD_SEQ_END;
            pLockState->m_seq_count = 0;
            pLockState->m_waittime  = 0;
            pLockState->m_offset    = offset;
            return;
        } else {
            pLockState->m_dem_lock  = DEM_LOCK;
            pLockState->m_state     = DEMOD_SEQ_WAIT_TS_LOCK;
            pLockState->m_seq_count = 0;
            pLockState->m_waittime  = 0;
            pLockState->m_offset    = offset;
            return;
        }
    } else {
        if( pLockState->m_seq_count > T2_DEMOD_LOCK_CHECK_TIMEOUT ) {
            /* UNLOCK Detected */
            pLockState->m_dem_lock  = DEM_UNLOCK;
            pLockState->m_state     = DEMOD_SEQ_END;
            pLockState->m_spectrum_flag = 0;
            pLockState->m_waittime     = 0;
            iic_write_val( demod_block_id, 0x00, 0x20 );    /* Bank 0x20 */
            iic_write_val( demod_block_id, 0xb5, 0x00 );
            iic_write_val( demod_block_id, 0x00, 0x00 );    /* Bank 0x00 */
            return;
        } else {
            ;
        }
    }
    
    pLockState->m_waittime     = 100;
    return ;
    
}

/******************************************************************************
 * dvbt2_demodlock_wait_tslock
 *-----------------------------------------------------------------------------
 * Input : 
 * Output: T2_DemodSeqState* pLockState
 * Return: 
 *****************************************************************************/
void dvbt2_demodlock_wait_tslock( T2_DemodSeqState* pLockState )
{
    /* check TS lock  */
    UINT8 tslock_flag = 0, ts_rate_flag=0;
    
    DBG_PRINT("Enter dvbt2_demodlock_wait_tslock\n");
    
    pLockState->m_seq_count++;
    
    iic_write_val( demod_block_id, 0x00, 0x20 );           /* Bank 0x20 */

    iic_read( demod_block_id, 0x10, &tslock_flag, 1);
    tslock_flag = ( (tslock_flag & 0x20) >> 5 );
    
    
    if( tslock_flag == 1 ) {
        /* TS LOCK */
        pLockState->m_ts_lock   = DVBT2_TS_LOCK;
        pLockState->m_state     = DEMOD_SEQ_END;
        pLockState->m_waittime  = 0;
        
        iic_write_val( demod_block_id, 0x00, 0x3f );       /* Bank 0x3f */
        iic_read( demod_block_id, 0x5a, &ts_rate_flag, 1);
        ts_rate_flag = ( (ts_rate_flag & 0x80) >> 7 );
        
        if( ts_rate_flag == 0 ) {
            iic_write_val( demod_block_id, 0x00, 0x20 );   /* Bank 0x20 */
            iic_write_val( demod_block_id, 0xe0, 0x0f );
            iic_write_val( demod_block_id, 0xe1, 0xff );
        } else {
            iic_write_val( demod_block_id, 0x00, 0x20 );   /* Bank 0x20 */
            iic_write_val( demod_block_id, 0xe0, 0x00 );
            iic_write_val( demod_block_id, 0xe1, 0x00 );
        }
        
        return; 
    } else {
        if( pLockState->m_seq_count > T2_TS_LOCK_CHECK_TIMEOUT ) {
            /* TS UNLOCK Detected */
            pLockState->m_ts_lock   = DVBT2_TS_UNLOCK;
            pLockState->m_state     = DEMOD_SEQ_END;
            pLockState->m_waittime  = 0;
            return; 
        }
    }

    iic_write_val( demod_block_id, 0x00, 0x00 );   /* Bank 0x00 */
    
    pLockState->m_waittime     = 100;
    
    return;

}

/******************************************************************************
 * dvbt2_output_demodResults
 *-----------------------------------------------------------------------------
 * Input : T2_DemodSeqState* pLockState
 * Output: 
 * Return: 
 *****************************************************************************/
void dvbt2_output_demodResults( T2_DemodSeqState* pLockState ) {

    DBG_PRINT("Enter dvbt2_output_demodResults\n");

    if( pLockState->m_dem_lock == DEM_UNLOCK ) {
       libc_printf("== T2 Demod Unlock ==\n");
    } else if( ( pLockState->m_offset > OFFSET_THRESHOLD )||( -1 * pLockState->m_offset > OFFSET_THRESHOLD ) ) {
       libc_printf("== T2 Carrier Offset is over the threshold ==\n");
    } else if ( pLockState->m_ts_lock  != DVBT2_TS_LOCK ) {
       libc_printf("== T2 TS Unlock    ==\n");
    } else {
        libc_printf("== T2 State Locked ==\n");
        if( pLockState->m_spectrum_flag == 0 ) {
           libc_printf("== T2 spectrum Normal ==\n");
        } else {
           libc_printf("== T2 spectrum Invert ==\n");
        }       
    }
    libc_printf("== T2 Carrier Offset = %ld [kHz] ==\n",pLockState->m_offset);

   return;

}

/* 2010-05-21 */
/******************************************************************************
 * dvbt2_L1ChangeRecovery
 *-----------------------------------------------------------------------------
 * Input : 
 * Output: L1CHANGE_FLAG* pL1ChangeFlag
 * Return: 
 *****************************************************************************/
void dvbt2_L1ChangeRecovery( L1CHG_RECOVERY_FLAG* pL1ChgRecoveryFlag )
{
    UINT8 rdata = 0;
    UINT8 ts_lock;
    UINT8 l1_change;

    DBG_PRINT("Enter dvbt2_L1ChangeRecovery\n");

    iic_write_val( demod_block_id, 0x00, 0x20 );        /* Bank 0x20 */
    iic_read( demod_block_id, 0x10, &rdata, 1 );        /* Read TS Lock */
    ts_lock = ( 0x20 & rdata ) >> 5; 
    
    iic_write_val( demod_block_id, 0x00, 0x22 );        /* Bank 0x22 */
    iic_read( demod_block_id, 0x15, &rdata, 1 );        /* Read L1-Change */
    l1_change = ( 0x01 & rdata ); 

    if( ( ts_lock == 0 )&&( l1_change == 1 ) ) {
        /* TS Unlock && L1 Change. */
        *pL1ChgRecoveryFlag = CHANGE_AND_SRST;
        /* DVB-T2 Demod SRST  */
        dvbt2_demod_start();
    } else {
        /* Not TS-Unlock or Not L1-Change */
        *pL1ChgRecoveryFlag = NO_CHANGE;
    }

    return;
}

/******************************************************************************
 * dvbt2_get_syncstat
 *-----------------------------------------------------------------------------
 * Input        : none
 * Output       : UINT8* pSyncStat, UINT8* pTSLockStat
 * Return       : none
 * Description  :
 *****************************************************************************/
 void dvbt2_get_syncstat(UINT8* pSyncStat, UINT8* pTSLockStat )
{

    UINT8 data = 0;
/*--------------------------------------------------------------------
  Sync state monitor
--------------------------------------------------------------------*/
    iic_write_val( demod_block_id, 0x00, 0x20 );        /* Bank 0x20 */
    iic_read( demod_block_id, 0x10, &data, 1 );
    *pSyncStat    = 0x07 & data;
    *pTSLockStat  = ( 0x20 & data ) >> 5;
    iic_write_val( demod_block_id, 0x00, 0x00 );        /* Bank 0x00 */
    return;

}

/******************************************************************************
 * dvbt2_get_ifagc
 *-----------------------------------------------------------------------------
 * Input        : none
 * Output       : UINT16* pIFAGCOut
 * Return       : none
 * Description  :
 *****************************************************************************/
 void dvbt2_get_ifagc(UINT16* pIFAGCOut )
{
    UINT8 rdata[2] = {0};

    iic_write_val( demod_block_id, 0x00, 0x20 );        /* Bank 0x20 */
    
    iic_read( demod_block_id, 0x26, rdata, 2 );

    iic_write_val( demod_block_id, 0x00, 0x00 );        /* Bank 0x00 */
    *pIFAGCOut = ( ( (UINT16)(rdata[0] & 0x0F) << 8 ) )  
               | (   (UINT16)rdata[1] );
    return;

}

/* 2010-05-21 add */
/******************************************************************************
 * dvbt2_get_rssi
 *-----------------------------------------------------------------------------
 * Input        : none
 * Output       : INT16* pRSSI  ( x10 )
 * Return       : none
 * Description  :
 *****************************************************************************/
void dvbt2_get_rssi(INT16* pRSSI )
{
    UINT8  rdata[2] = {0};
    UINT16 IFAGCOUT = 0;
    
    iic_write_val( demod_block_id, 0x00, 0x20 );        /* Bank 0x20 */
    iic_read( demod_block_id, 0x26, rdata, 2 );

    IFAGCOUT = ( ( (UINT16)(rdata[0] & 0x0F) << 8 ) )  
             | ( (UINT16)rdata[1] );
    
    *pRSSI = ( ( (INT32)IFAGCOUT * (INT32)AGC_A_T2 * (INT32)AGC_R ) / 10000 )
             - AGC_B_T2;
    
    return;

}

/******************************************************************************
 * dvbt2_get_CarrierOffset
 *-----------------------------------------------------------------------------
 * Input        : UINT8 bandwidth
 * Output       : INT32* pOffset
 * Return       : none
 * Description  :
 *****************************************************************************/
void dvbt2_get_CarrierOffset(UINT8 bandwidth, INT32* pOffset )
{
    INT32  carrier_data;
    INT32  offset = 0;
    UINT8  rdata[4] = {0};
    UINT8  offset_polarity = 0;
    UINT8  SyncStat;
    UINT8  TSLockStat;
    UINT32 carrier_reg;

    dvbt2_get_syncstat(&SyncStat,&TSLockStat);
    if( SyncStat != 6 ) {
        *pOffset = 0;
        libc_printf("!!!! CarrierOffset invalid (Demod_UnLock)\n");
        return;
    }

    iic_write_val( demod_block_id, 0x00, 0x20 );        /* Bank 0x20 */
    iic_read( demod_block_id, 0x4c, rdata, 4 );
    iic_read( demod_block_id, 0xb5, &offset_polarity, 1 );
    iic_write_val( demod_block_id, 0x00, 0x00 );        /* Bank 0x00 */

    carrier_reg = ( ( (UINT32)(rdata[0] & 0x0f) << 24 ) ) 
                | ( ( (UINT32)rdata[1] << 16) )
                | ( ( (UINT32)rdata[2] << 8 ) )
                | (   (UINT32)rdata[3] );
    offset_polarity  = ( offset_polarity & 0x10 ) >> 4;

    carrier_data = dvb_Convert2SComplement(carrier_reg, 28);

    if( offset_polarity == 0 ) {
        carrier_data = -1 * carrier_data;
    }
    
    switch(bandwidth){
        case 1:
            /* T.B.D. */
            break;
        case 2:
            /* T.B.D. */
            break;
        case 3:
            offset = carrier_data / 117441;
            break;
        default:
            offset = 0;
            break;
    }

    *pOffset = offset;

    return;
}

/******************************************************************************
 * dvbt2_get_CN
 *-----------------------------------------------------------------------------
 * Input        : none
 * Output       : UINT16* pCN_by10 :CN[dB] ( Q2-format fixed pointrepresentation )
 * Return       : none
 * Description  :
 *****************************************************************************/
void dvbt2_get_CN(INT16* pCN_q2 )
{
    UINT8   rdata[2] = {0};
    UINT16  cn_Data;
    UINT8   SyncStat;
    UINT8   TSLockStat;

    dvbt2_get_syncstat(&SyncStat, &TSLockStat);
    if( SyncStat != 6 ) {
        *pCN_q2 = 0;
        libc_printf("!!!! CN invalid (Demod_UnLock)\n");
        return;
    }

    iic_write_val( demod_block_id, 0x00, 0x20 );        /* Bank 0x20 */
    iic_read( demod_block_id, 0x2c, rdata, 2 );
    iic_write_val( demod_block_id, 0x00, 0x00 );        /* Bank 0x00 */
    cn_Data = ( ( (UINT16)rdata[0] << 8) )
             |  ( (UINT16)rdata[1] );

    *pCN_q2 =  DVBT2_CnCalc( cn_Data / 4 );

    return;
}



/******************************************************************************
 * dvbt2_get_PreBCHBer
 *-----------------------------------------------------------------------------
 * Input        : none
 * Output       : UINT16* pBitError
 *              : UINT16* pPeriod
 * Return       : none
 * Description  :
 *****************************************************************************/
void dvbt2_get_PreBCHBer(UINT32* pBitError, UINT32* pPeriod )
{
    UINT8   rdata[3] = {0};
    UINT8   berMode1, berMode2;
    UINT16  number;

    iic_write_val( demod_block_id, 0x00, 0x20 );        /* Bank 0x20 */
    iic_read( demod_block_id, 0x75, rdata, 1 );

    if((rdata[0] & 0x01) == 0) {
        libc_printf("!!!! PreBCH BER Invalid\n");
        *pBitError = 0xFFFFFFFF;
    } else {

        iic_write_val( demod_block_id, 0x00, 0x24 );        /* Bank 0x24 */
        
        iic_read( demod_block_id, 0x41, rdata, 3 );
        *pBitError = ( ( (UINT32)(rdata[0] & 0x3f) << 16 ) ) 
                   | ( ( (UINT32)rdata[1] << 8) ) 
                   | (   (UINT32)rdata[2] );
    }

    iic_write_val( demod_block_id, 0x00, 0x22 );        /* Bank 0x22 */

    iic_read( demod_block_id, 0x5B, rdata, 1 );
    berMode1 = rdata[0] & 0x07;

    iic_read( demod_block_id, 0x5E, rdata, 1 );
    berMode2 = rdata[0] & 0x03;



    if ( berMode2 == 0 ) {
        switch ( berMode1 ) {
        case 0:  number =  7200; break;
        case 1:  number =  9720; break;
        case 2:  number = 10800; break;
        case 3:  number = 11880; break;
        case 4:  number = 12600; break;
        case 5:  number = 13320; break;
        default: number = 10800; break;
        }
    } else {
        switch ( berMode1 ) {
        case 0:  number = 32400; break;
        case 1:  number = 38880; break;
        case 2:  number = 43200; break;
        case 3:  number = 48600; break;
        case 4:  number = 51840; break;
        case 5:  number = 54000; break;
        default: number = 43200; break;
        }
    }

    iic_write_val( demod_block_id, 0x00, 0x20 );        /* Bank 0x20 */

    iic_read( demod_block_id, 0x72, rdata, 1 );
    *pPeriod = ( (UINT32)1 << (rdata[0] & 0xf) ) * number;

    iic_write_val( demod_block_id, 0x00, 0x00 );         /* Bank 0x00 */

    return;

}

/******************************************************************************
 * dvbt2_get_PostBCHFer
 *-----------------------------------------------------------------------------
 * Input        : none
 * Output       : UINT32* pFrameError
 *              : UINT32* pPeriod
 * Return       : none
 * Description  :
 *****************************************************************************/
void dvbt2_get_PostBCHFer(UINT32* pFrameError, UINT32* pPeriod )
{
    UINT8 rdata[3] = {0};

    iic_write_val( demod_block_id, 0x00, 0x20 );        /* Bank 0x20 */

    iic_read( demod_block_id, 0x75, rdata, 3 );

    if((rdata[0] & 0x01) == 0) {
        libc_printf("!!!! PostBCH FER Invalid\n");
        *pFrameError = 0xFFFFFFFF;
    } else {
        *pFrameError = ( ( (UINT16)(rdata[1] & 0x7F) << 8 )) 
                       | ( (UINT16)rdata[2] );
    }

    iic_read( demod_block_id, 0x72, rdata, 1 );
    *pPeriod = (UINT32)1 << (rdata[0] & 0xf);

    iic_write_val( demod_block_id, 0x00, 0x00 );         /* Bank 0x00 */

    return;

}

/* 2010-06-21 dvbt2_get_T2DemodInfo has been merged to dvbt2_get_T2L1PreInfo. */
/******************************************************************************
 * dvbt2_get_T2DemodInfo
 *-----------------------------------------------------------------------------
 * Input        : none
 * Output       : T2_DemodInfo* pInfo
 * Return       : none
 * Description  :
 *****************************************************************************/
void dvbt2_get_T2DemodInfo(T2_DemodInfo* pInfo)
{

    UINT8 rdata[7] = {0};

    iic_write_val( demod_block_id, 0x00, 0x22 );         /* Bank 0x22  */
    iic_read( demod_block_id, 0x20, rdata, 7 );


    pInfo->type    =   rdata[0];                       /* 0x20, bit[7:0] */
    pInfo->bwt_ext =   rdata[1] & 0x01;                /* 0x21, bit[0]   */
    pInfo->miso    =   rdata[2] & 0x07;                /* 0x22, bit[2:0] */
    pInfo->mixed   =   rdata[3] & 0x01;                /* 0x23, bit[0]   */
    pInfo->mode    = ( rdata[3] & 0x0E ) >> 1;         /* 0x23, bit[3:1] */

    pInfo->guard   =   rdata[5] & 0x07;                /* 0x25, bit[2:0] */
    pInfo->papr    =   rdata[6] & 0x0f;                /* 0x26, bit[3:0] */

    iic_read( demod_block_id, 0x30, rdata, 1 );
    pInfo->pp      =   rdata[0] & 0x0f;                /* 0x30, bit[3:0] */
    
    iic_read( demod_block_id, 0x39, rdata, 2 );
    pInfo->ndsym   = ( ( (UINT16)( rdata[0] & 0x0f ) << 8 ) ) 
                   | (   (UINT16)rdata[1] );       /* 0x39, bit[3:0]; 0x40, bit[7:0] */
    
    iic_write_val( demod_block_id, 0x00, 0x00 );         /* Bank 0x00  */
    
    
    return;

}

/* 2010-06-21 add */
/******************************************************************************
 * dvbt2_get_T2L1PreInfo
 *-----------------------------------------------------------------------------
 * Input        : none
 * Output       : T2_L1PreInfo* pInfo
 * Return       : none
 * Description  :
 *****************************************************************************/
void dvbt2_get_T2L1PreInfo(T2_L1PreInfo* pInfo) {

    unsigned char rdata[27] = {0};

    iic_write_val( demod_block_id, 0x00, 0x22 );        //Bank 0x22
    iic_read( demod_block_id, 0x20, rdata, sizeof(rdata)/sizeof(rdata[0]) );
    iic_write_val( demod_block_id, 0x00, 0x00 );        //Bank 0x00
 
    pInfo->type               = rdata[0];
    pInfo->bwt_ext            = rdata[1] & 0x01;
    pInfo->s1                 = rdata[2] & 0x07;
    pInfo->s2                 = rdata[3] & 0x0f;
    pInfo->guard              = rdata[5] & 0x07;
    pInfo->papr               = rdata[6] & 0x0f;
    pInfo->L1_mod             = rdata[7] & 0x0f;
    pInfo->pp                 = rdata[16] & 0x0f;
    pInfo->tx_id_avaliablity  = rdata[17];
    pInfo->cell_id            = (UINT16)( ( (UINT16)rdata[18] << 8 ) | (UINT16)rdata[19] );
    pInfo->network_id         = (UINT16)( ( (UINT16)rdata[20] << 8 ) | (UINT16)rdata[21] );
    pInfo->t2_system_id       = (UINT16)( ( (UINT16)rdata[22] << 8 ) | (UINT16)rdata[23] );
    pInfo->num_data_symbols   = (UINT16)( ( (UINT16)( rdata[25] & 0x0f ) << 8 ) | (UINT16)rdata[26] );
    return;
}

/* 2010-06-21 add */
/******************************************************************************
 * dvbt2_print_T2L1PreInfo
 *-----------------------------------------------------------------------------
 * Input        : none
 * Output       : T2_L1PreInfo* pInfo
 * Return       : none
 * Description  :
 *****************************************************************************/
void dvbt2_print_T2L1PreInfo(T2_L1PreInfo* pInfo) {
    libc_printf("===============================\n");
    libc_printf("=== T2 PreSignelling info  ====\n");
    libc_printf("===============================\n");
    libc_printf("L1-Pre TYPE[7:0]               %x\n",pInfo->type             );
    libc_printf("L1-Pre BWT_EXT                 %x\n",pInfo->bwt_ext          );
    libc_printf("L1-Pre S1[2:0]                 %x\n",pInfo->s1               );
    libc_printf("L1-Pre S2[3:0]                 %x\n",pInfo->s2               );
    libc_printf("L1-Pre GUARD_INTERVAL[2:0]     %x\n",pInfo->guard            );
    libc_printf("L1-Pre PAPR[3:0]               %x\n",pInfo->papr             );
    libc_printf("L1-Pre L1_MOD[3:0]             %x\n",pInfo->L1_mod           );
    libc_printf("L1-Pre PILOT_PATTERN[3:0]      %x\n",pInfo->pp               );
    libc_printf("L1-Pre TX_ID_AVAILABILITY[7:0] %x\n",pInfo->tx_id_avaliablity);
    libc_printf("L1-Pre CELL_ID[15:0]           %x\n",pInfo->cell_id          );
    libc_printf("L1-Pre NETWORK_ID[15:0]        %x\n",pInfo->network_id       );
    libc_printf("L1-Pre T2_SYSTEM_ID[15:0]      %x\n",pInfo->t2_system_id     );
    libc_printf("L1-Pre NUM_DATA_SYMBOLS[15:0]  %x\n",pInfo->num_data_symbols );
    return;
}

/* 2010-06-21 add */
/******************************************************************************
 * dvbt2_get_T2L1PostInfo
 *-----------------------------------------------------------------------------
 * Input        : none
 * Output       : T2_L1PostInfo* pInfo
 * Return       : none
 * Description  :
 *****************************************************************************/
void dvbt2_get_T2L1PostInfo(T2_L1PostInfo* pInfo) {
    unsigned char rdata[26] = {0};
      
    iic_write_val( demod_block_id, 0x00, 0x22 );        /* Bank 0x00 */
    iic_read(  demod_block_id, 0x45, rdata, sizeof(rdata)/sizeof(rdata[0]) );
    iic_write_val( demod_block_id, 0x00, 0x00 );        /* Bank 0x00 */
    
    /* rdata[0]..[2] : currently unused */
    pInfo->num_aux              = rdata[3] & 0x0f;
    pInfo->aux_config_rfu       = rdata[4];
    pInfo->fef_type             = rdata[10] & 0x0f;
    pInfo->fef_length           = (UINT32)
                                  ( ( (UINT32)( rdata[11] & 0x3f ) << 16 )
                                  | ( (UINT32)rdata[12] <<  8 )
                                  | ( (UINT32)rdata[13]) );
    pInfo->fef_interval         = rdata[14];
    
    //current plp
    pInfo->plp_id             = rdata[15];
    pInfo->plp_type           = rdata[16] & 0x07;
    pInfo->plp_code           = rdata[22] & 0x07;
    pInfo->plp_mod            = rdata[23] & 0x07;
    pInfo->plp_rotation       = rdata[24] & 0x01;
    pInfo->plp_fec_type       = rdata[25] & 0x03;
    return;
}

/* 2010-06-21 add */
/******************************************************************************
 * dvbt2_print_T2L1PostInfo
 *-----------------------------------------------------------------------------
 * Input        : none
 * Output       : T2_L1PostInfo* pInfo
 * Return       : none
 * Description  :
 *****************************************************************************/
void dvbt2_print_T2L1PostInfo(T2_L1PostInfo* pInfo) {
    libc_printf("===================================\n");
    libc_printf("=== T2 L1 Post Signalling Data ====\n");
    libc_printf("===================================\n"); 
    libc_printf("L1-Post NUM_AUX[3:0]           %x\n",pInfo->num_aux        );
    libc_printf("L1-Post AUX_CONFIG_RFU[7:0]    %x\n",pInfo->aux_config_rfu );
    libc_printf("L1-Post FEF_TYPE[3:0]          %x\n",pInfo->fef_type       );
    libc_printf("L1-Post FEF_LENGTH[21:0]       %lx\n",pInfo->fef_length    );
    libc_printf("L1-Post FEF_INTERVAL[7:0]      %x\n",pInfo->fef_interval   );
    libc_printf("--- Current PLP ---\n");
    libc_printf("L1-Post PLP_ID[7:0]            %x\n",pInfo->plp_id         );
    libc_printf("L1-Post PLP_TYPE[2:0]          %x\n",pInfo->plp_type       );
    libc_printf("L1-Post PLP_COD[2:0]           %x\n",pInfo->plp_code       );
    libc_printf("L1-Post PLP_MOD[2:0]           %x\n",pInfo->plp_mod        );
    libc_printf("L1-Post PLP_ROTATION           %x\n",pInfo->plp_rotation   );
    libc_printf("L1-Post PLP_FEC_TYPE[1:0]      %x\n",pInfo->plp_fec_type   );
    return ;
}

/* 2010-05-21 add */
/******************************************************************************
 * dvbt2_get_T2PLPSelFlag
 *-----------------------------------------------------------------------------
 * Input        : none
 * Output       : PLPSEL_FLAG*   pPlpIdError     : 0:No L1 Change, 1:L1 Change and SRST
 *                UINT8* pCurrent_PLP_ID         : Current PLP_ID
 * Return       : none
 * Description  :
 *****************************************************************************/
 void dvbt2_get_T2PLPSelFlag(PLPSEL_FLAG* pPlpSelFlag, UINT8* pCurrent_PLP_ID)
{

    UINT8 rdata = 0;

    iic_write_val( demod_block_id, 0x00, 0x22 );       /* Bank 0x22  */
    iic_read( demod_block_id, 0x13, &rdata, 1 );

    if( ( rdata & 0x01 ) == 0 ) {                      /* 0x13, bit[0] */
        *pPlpSelFlag   =   PLPSEL_OK;
    } else {
        *pPlpSelFlag   =   PLPSEL_ERROR;
    }

    iic_read( demod_block_id, 0x54, &rdata, 1 );       /* 0x54 */
    *pCurrent_PLP_ID = rdata;

    iic_write_val( demod_block_id, 0x00, 0x00 );       /* Bank 0x00  */

    return;

}

/******************************************************************************
 * UINT8 PlpIdNum, UINT8 *PlpId
 *-----------------------------------------------------------------------------
 * Input        : none
 * Output       : UINT8 PlpIdNum, UINT8 *PlpId
 * Return       : none
 * Description  :
 *****************************************************************************/
 void dvbt2_get_PlpIdInfo(UINT8 *PlpIdNum, UINT8 *PlpId)
{

    UINT8 rdata[32] = {0};
    UINT8 data_length, i;
    
    
    iic_write_val( demod_block_id, 0x00, 0x22 );         /* Bank 0x22  */
    iic_read( demod_block_id, 0x7f, rdata, 1 );
    
    *PlpIdNum = rdata[0];
    
    data_length = 32;    /* in case of I2c burst reading max is 32 byte. */
    
    /* PLP_ID_000 -- PLP_ID_031 */
    iic_read( demod_block_id, 0x80, rdata, data_length);
    for(i = 0; i < data_length ; i++){
       PlpId[i] = rdata[i];
    }
    
    /* PLP_ID_032 -- PLP_ID_063 */
    iic_read( demod_block_id, 0xA0, rdata, data_length);
    for(i = 0; i < data_length ; i++){
       PlpId[i + 32] = rdata[i];
    }
    
    /* PLP_ID_064 -- PLP_ID_095 */
    iic_read( demod_block_id, 0xC0, rdata, data_length);
    for(i = 0; i < data_length ; i++){
       PlpId[i + 64] = rdata[i];
    }
    
    /* PLP_ID_096 -- PLP_ID_127 */
    iic_read( demod_block_id, 0xE0, rdata, data_length);
    for(i = 0; i < data_length ; i++){
       PlpId[i + 96] = rdata[i];
    }
    
    iic_write_val( demod_block_id, 0x00, 0x23 );         /* Bank 0x23  */
    
    /* PLP_ID_128 -- PLP_ID_169 */
    iic_read( demod_block_id, 0x10, rdata, data_length);
    for(i = 0; i < data_length ; i++){
       PlpId[i + 128] = rdata[i];
    }
    
    /* PLP_ID_160 -- PLP_ID_191 */
    iic_read( demod_block_id, 0x30, rdata, data_length);
    for(i = 0; i < data_length ; i++){
       PlpId[i + 160] = rdata[i];
    }
    
    
    /* PLP_ID_192 -- PLP_ID_223 */
    iic_read( demod_block_id, 0x50, rdata, data_length);
    for(i = 0; i < data_length ; i++){
       PlpId[i + 192] = rdata[i];
    }
    
    
    /* PLP_ID_224 -- PLP_ID_254 */
    
    data_length = 31;       /* Length = 31 */
    
    iic_read( demod_block_id, 0x70, rdata, data_length);
    for(i = 0; i < data_length ; i++){
       PlpId[i + 224] = rdata[i];
    }

    iic_write_val( demod_block_id, 0x00, 0x00 );         /* Bank 0x00  */

    return;

}

/******************************************************************************
 * dvbt2_print_PlpIdInfo
 *-----------------------------------------------------------------------------
 * Input : void
 * Output: none
 * Return: void
 *****************************************************************************/
void dvbt2_print_PlpIdInfo(UINT8 PlpIdNum, UINT8 *plpId)
{

    INT16 i;
    INT16 j;
    libc_printf("\n");
    libc_printf("-----PLP ID NUM  = <      %3d       >\n", PlpIdNum);
    libc_printf("PLP_ID| ");
    for (i = 0 ; i < 16 ; i++) {
        libc_printf(" %2x " , i);
    }
    libc_printf("\n");
    libc_printf("------------------------------------------------------------------------\n");
    i = 0;
    while( i < 255 ) {
        libc_printf(" 0x%2x | ", i);
        for (j = 0 ; j < 0x10 ; j++) {
            if ( i < 255 ) {
                libc_printf(" %02x ", plpId[i]);
            }
            i++;
        }
        libc_printf("\n");
    }
    libc_printf("\n");

    return;
}

/* add 2010-10-15 */
/******************************************************************************
 * dvbt2_get_ssi_sqi
 * The function is an implementation example of SSI/SQI calculation
 *-----------------------------------------------------------------------------
 * Input        : none
 * Output       : pSQI, pSSI [%]
 * Return       : none
 *****************************************************************************/
void dvbt2_get_ssi_sqi (UINT16* pSQI, UINT16* pSSI)
{
    UINT8 data[2] = {0};
    UINT8 plp_mod = 0;    /* modulation of receiving PLP */
    UINT8 plp_cr = 0;     /* code rate of receiving PLP */
    INT16 rssi = 0;       /* RSSI [dBm x 10] */
    INT16 Prel = 0;       /* Prel [dBm x 10] */
    INT16 Pref = 0;       /* reference signal level [dBm x 10] */
    UINT32 bit_error = 0; /* num of Pre-BCH bit error */
    UINT32 period = 0;    /* period of Pre-BCH bit error measurement [num of bits]*/
    INT16 cn = 0 ;        /* received CN [dB x 100] */
    INT16 cnReq = 0;      /* required CN [dB x 100] */
    INT16 cnRel = 0;      /* CNrel [dB x 100] */
    INT16 ber_10log10 = 0;/* 10xlog10(BER)x100 */
    INT16 BER_SQI = 0 ;   /* BER_SQI x 100 */
    UINT16 SQI = 0 ;      /* SQI x 100 */
    UINT16 SSI = 0;       /* SSI x 10 */
    
    DBG_PRINT("dvbt2_get_ssi_sqi\n");

    /* Get CR/Mod info */
    iic_write_val( demod_block_id,  0x00, 0x22 );
    iic_read( demod_block_id, 0x5B, data, 2 );    /* bank=0x20, subadr=0x5B,0x5C */
    iic_write_val( demod_block_id,  0x00, 0x00 );
    plp_cr  = data[0] & 0x07;
    plp_mod = data[1] & 0x07;

    /* Get PreBCH BER */
    dvbt2_get_PreBCHBer(&bit_error, &period);
    
    /* Get CNR */
    dvbt2_get_CNdBx100(&cn);
    
    /* Get RSSI */
    dvbt2_get_rssi(&rssi );
    
    /* Get required CN */
    cnReq = DVBT2_ReqiredCNdBx100[plp_mod][plp_cr];

    /* CNrel calculation */
    cnRel = cn - cnReq;
    
    /* BER_SQI calculation */
    /*************************************************************************
        BER_SQI = 0                  if         BER >10E-3
        BER_SQI = 20*LOG10(1/BER)-40 if 10E-7 < BER <= 10E-3
        BER_SQI = 100                if         BER <= 10E-7
     *************************************************************************/
    ber_10log10 = dvbt2_calc_10log10_x100(bit_error) - dvbt2_calc_10log10_x100(period) + ADJUST_T2;  /* ADJUST_T2 = 4301 = 10log10(20000) x 100 */
    
    if ( (ber_10log10 <= -70 * 100)||(bit_error == 0) ) {
        /* BER <= 10E-7 */
        BER_SQI = 100*100 ;
    } else if (ber_10log10 > -30 * 100) {
        /* BER > 10E-3 */
        BER_SQI =  0;
    } else {
        BER_SQI = -2 * ber_10log10 - 40 * 100;
        /* dead code
        if (BER_SQI < 0)
		{
            BER_SQI = 0 ;
        }
        */
    }
    
    /* SQI calculation */
    /*************************************************************************
        SQI = 0                                 if          C/Nrel < -7 dB
        SQI = (((C/Nrel -3)/10) + 1) * BER_SQI  if -7 dB <= C/Nrel < +3 dB
        SQI = BER_SQI                           if          C/Nrel >= +3 dB
     *************************************************************************/
    if (cnRel >= 3 * 100) {
        SQI = BER_SQI;
    }
    else if (cnRel < -7 * 100) {
        SQI = 0;
    } else {
        SQI = (UINT16)( ( ( ( ( (INT32)cnRel - (INT32)3*100 ) / (INT32)10 ) + (INT32)1*100 ) * (INT32)BER_SQI ) / (INT32)100 );
    }
    
    /* SSI calculation */
    /*************************************************************************
        SSI = 0                         if           Prel < -15dB
        SSI = (2/3) * (Prel + 15)       if -15 dB <= Prel < 0dB
        SSI = 4 * Prel + 10             if   0 dB <= Prel < 20 dB
        SSI = (2/3) * (Prel - 20) + 90  if  20 dB <= Prel < 35 dB
        SSI = 100                       if           Prel >= 35 dB
     *************************************************************************/
    Pref = DVBT2_RefRSSIx10[plp_mod][plp_cr];
    Prel = rssi - Pref;
    if( Prel < -15*10 ) {
        SSI = 0;
    } else if( Prel < 0*10 ) {
        SSI = (2 * ( Prel + 15*10 ) ) / 3;
    } else if( Prel < 20*10 ) {
        SSI = 4 * Prel + 10*10;
    } else if( Prel < 35*10 ) {
        SSI = ((2 * ( Prel - 20*10 ) ) / 3) + 90*10;
    } else {
        SSI = 100*10;
    }
    
    /* set result */
    *pSQI = SQI / 100;
    *pSSI = SSI / 10;
    return;
}

/* add 2010-10-15 */
/******************************************************************************
 * dvbt_get_CNdBx100
 *-----------------------------------------------------------------------------
 * Input        : none
 * Output       : pCNx100 (CN x 100 [dB])
 * Return       : none
 *****************************************************************************/
static void dvbt2_get_CNdBx100(INT16* pCNx100 )
{
    UINT8  rdata[2] = {0};
    UINT16 CN_DATA;

    iic_write_val( demod_block_id,  0x00, 0x20 );
    iic_read( demod_block_id, 0x2C, rdata, 2 );
    iic_write_val( demod_block_id,  0x00, 0x00 );
    CN_DATA = (rdata[0] << 8) | rdata[1];

    *pCNx100 = dvbt2_calc_10log10_x100( CN_DATA ) - 602;

    return;
}

/* add 2010-10-15 */
/******************************************************************************
 * calc_10log10_x100
 *-----------------------------------------------------------------------------
 * Input        : input
 * Output       : none
 * Return       : 10log10(input) x 100
 *****************************************************************************/
static UINT32 dvbt2_calc_10log10_x100(UINT32 input)
{
    UINT16    i;
    UINT16    s = 0;
    UINT32   result_10log10x100;

    for( i = 0; i < 32; i ++ ) {
        s = ( s >> 1 ) + ( (UINT32)( input & 0x01 ) << 5 );
        input >>= 1;
        if( input == 0 ) {
            break;
        }
    }
    result_10log10x100    = (UINT32)i * 301 + (UINT32)dvbt2_log10x100_table[ 0x1F & s ];
    return result_10log10x100;
}

/******************************************************************************
 * dvb_Convert2SComplement
 *-----------------------------------------------------------------------------
 * Input        : UINT32 value, UINT32 bitlen
 * Output       : none
 * Return       : INT32
 *****************************************************************************/
/* Utility function */
#define MASKUPPER(n) ((n) == 0 ? 0 : 0xFFFFFFFFU << (32 - (n)))
#define MASKLOWER(n) ((n) == 0 ? 0 : 0xFFFFFFFFU >> (32 - (n)))
INT32 dvb_Convert2SComplement(UINT32 value, UINT32 bitlen)
{
    if(bitlen == 0 || bitlen >= 32){
        return (INT32)value;
    }

    if(value & ((UINT32)1 << (bitlen - 1))){
        /* minus value */
        return (INT32)(MASKUPPER(32 - bitlen) | value);
    }else{
        /* plus value */
        return (INT32)(MASKLOWER(bitlen) & value);
    }
}

/******************************************************************************
 * CnCalc
 *-----------------------------------------------------------------------------
 * Input        : input : anti-logarithm number ( 0< input < 0xFFFF)
 * Output       : none
 * Return       : logarithm number ( Q2-format fixed point )
 * Description  :
 *****************************************************************************/
UINT16 DVBT2_CnCalc( UINT16 input )
{
    UINT16 i;
    UINT16 s = 0;
    UINT16 output;
    
    for( i = 0; i < 16; i ++ ) {
        if(  (input >> ( i + 5 ) ) == 0 ) {
            s = 0x1F & (input >> i );
            break;
        }
    }
    output = 3 * 4 * i + DVBT2_CnTable[ s ];
    return output;
}

/* 2010-10-05 Add : DVB-T2 Long Echo Sequence */
/******************************************************************************
 * DVB-T2 Long Echo Sequence.
 *-----------------------------------------------------------------------------
 * Input        : input
 * Output       : none
 * Return       : 
 *****************************************************************************/
/******************************************
 Register Sub-Address
 ******************************************/
#define CXD2820R_B0_REGC_BANK                           0x00
#define CXD2820R_B27                                    0x27
#define CXD2820R_B27_FCS_ALPHA                          0xDD
#define CXD2820R_B20                                    0x20
#define CXD2820R_B20_SEQ_L1PRE_TO_CNT                   0x13
#define CXD2820R_B20_SEQ_NG_OCCURRED_MAX_STATE          0x17
#define CXD2820R_B27_GTHETA_OFST0                       0xE2
#define CXD2820R_B20_P1DET_STATE                        0x57
#define CXD2820R_B27_SYR_ACTMODE                        0xC9
#define CXD2820R_B27_CRL_OPEN                           0x69
#define CXD2820R_B20_WR_SMOOTH_SCALE_11_8               0xe0
#define CXD2820R_B20_SP_TS_LOCK                         0x10
#define CXD2820R_B20_TDA_INVERT                         0xb5

/******************************************
 Polling Interval
 ******************************************/
#define CXD2820_TUNE_POLL_INTERVAL          100

/******************************************
 Monitoring/Decision Timing
 ******************************************/
#define DVB_DEMOD_DVBT2_P1_WAIT             300     /**< DVB-T2 300ms before checking early no P1 (T2 or FEF) indication. */
#define DVB_DEMOD_DVBT2_T2_P1_WAIT          600     /**< DVB-T2 600ms before checking for P1 (T2 only) indication. */
#define DVB_DEMOD_DVBT2_WAIT_LOCK           5000    /**< DVB-T2 5000ms for demod lock indication. */
#define DVB_DEMOD_DVBT2_WAIT_TS_LOCK        5000    /**< DVB-T2 5000ms for TS lock. Assumes that demod core is locked. */
#define DVB_DEMOD_DVBT2_LE_WAIT_SYR         3000    /**< DVB-T2 3000ms timeout for TS lock on Long Echo sequence before trialling SYR. */
#define DVB_DEMOD_DVBT2_LE_WAIT_NO_OFST     3500    /**< DVB-T2 3500ms timeout for DMD lock when zero timing offset loop enabled. */

/******************************************
 Return Status
 ******************************************/
typedef enum {
    SONY_DVB_OK,                /**< Successful. */
    SONY_DVB_ERROR_ARG,         /**< Invalid argument (maybe software bug). */
    SONY_DVB_ERROR_I2C,         /**< I2C communication error. */
    SONY_DVB_ERROR_SW_STATE,    /**< Invalid software state. */
    SONY_DVB_ERROR_HW_STATE,    /**< Invalid hardware state. */
    SONY_DVB_ERROR_TIMEOUT,     /**< Timeout occured. */
    SONY_DVB_ERROR_UNLOCK,      /**< Failed to lock. */
    SONY_DVB_ERROR_RANGE,       /**< Out of range. */
    SONY_DVB_ERROR_NOSUPPORT,   /**< Not supported for current device. */
    SONY_DVB_ERROR_CANCEL,      /**< The operation is canceled. */
    SONY_DVB_ERROR_OTHER        /**< The operation failed. Enable tracing to determine details of the error. */
} sony_dvb_result_t;

/******************************************
 Spectrum Sence Status
 ******************************************/
typedef enum {
    DVB_DEMOD_SPECTRUM_NORMAL = 0,      /**< Spectrum normal sense. */
    DVB_DEMOD_SPECTRUM_INV              /**< Spectrum inverted. */    
} sony_dvb_demod_spectrum_sense_t ;

/******************************************
 Sequence Status
 ******************************************/
typedef enum {
    SONY_DVB_LE_SEQ_START = 0,
    SONY_DVB_LE_SEQ_P1,
    SONY_DVB_LE_SEQ_LOCK,
    SONY_DVB_LE_SEQ_CRL_OPEN_TRIAL,
    SONY_DVB_LE_SEQ_TIMING_TRIAL,
    SONY_DVB_LE_SEQ_SPECTRUM_TRIAL,
    SONY_DVB_LE_SEQ_UNKNOWN
} sony_dvb_le_seq_state_t ;

/******************************************
 Sequence State Struct
 ******************************************/
typedef struct sony_dvb_le_seq_t {
    sony_dvb_le_seq_state_t state ;
    UINT8 autoSpectrumDetect ;
    UINT32 waitTime ;
    UINT32 totalWaitTime ;
    UINT8 spectTrial ;
    UINT8 gThetaPOfst ;
    UINT8 gThetaMOfst ;
    UINT8 gThetaNoOfst ;
    UINT8 unlockDetDone ;
    UINT8 crlOpenTrial ;
    UINT8 complete ;
    T2_DEM_LOCK seq_result;
} sony_dvb_le_seq_t ;

/******************************************
 Lock Result
 ******************************************/
typedef enum {

    /* "Lock" condition not met, yet DEMOD_LOCK_RESULT_UNLOCKED also not reached. */
    DEMOD_LOCK_RESULT_NOTDETECT,

    /* "Lock" condition is found. */
    DEMOD_LOCK_RESULT_LOCKED,

    /* Demodulator has decided that no signal 
            was found or the signal was not the required system. */
    DEMOD_LOCK_RESULT_UNLOCKED
} sony_dvb_demod_lock_result_t;

/******************************************
 Function Pointer for sub-process
 ******************************************/
typedef sony_dvb_result_t (*sony_dvb_le_seq_func_t) (sony_dvb_le_seq_t* pSeq);

/* DVB-T2 Long Echo Sequence states. */
static sony_dvb_result_t t2_le_SeqWaitStart (sony_dvb_le_seq_t* pSeq);
static sony_dvb_result_t t2_le_SeqWaitP1 (sony_dvb_le_seq_t* pSeq);
static sony_dvb_result_t t2_le_SeqWaitDmdLock (sony_dvb_le_seq_t* pSeq);
static sony_dvb_result_t t2_le_SeqWaitTimingTrial (sony_dvb_le_seq_t* pSeq);
static sony_dvb_result_t t2_le_SeqWaitCrlOpenTrial (sony_dvb_le_seq_t* pSeq);
static sony_dvb_result_t t2_le_SeqWaitSpectrumTrial (sony_dvb_le_seq_t* pSeq);

sony_dvb_result_t dvb_demod_CheckTSLock (sony_dvb_demod_lock_result_t * pLock);
sony_dvb_result_t dvb_demod_t2_CheckP1Lock (sony_dvb_demod_lock_result_t * pLock);
sony_dvb_result_t dvb_demod_t2_SetCrlOpen (UINT8 crlOpen);
sony_dvb_result_t dvb_demod_t2_CheckT2P1Lock (sony_dvb_demod_lock_result_t * pLock);
sony_dvb_result_t dvb_demod_t2_SetSpectrumSense (sony_dvb_demod_spectrum_sense_t sense);
sony_dvb_result_t dvb_demod_t2_OptimiseTSOutput (void);
sony_dvb_result_t dvb_demod_SoftReset (void);
sony_dvb_result_t dvb_demod_monitorT2_SyncStat (UINT8 * pSyncStat, UINT8 * pTSLockStat);
sony_dvb_result_t dvb_demod_monitorT2_SpectrumSense (sony_dvb_demod_spectrum_sense_t* pSense);

/*------------------------------------------------------------------------------
  DVB-T2 Long Echo Sequence.
------------------------------------------------------------------------------*/
/******************************************************************************
 t2_le_Seq : Top function
 *****************************************************************************/
int t2_le_Seq (T2_DEM_LOCK* pResult)
{
    sony_dvb_result_t result = SONY_DVB_OK ;

    /* Setup sequencer with start values. */
    sony_dvb_le_seq_t seq = { 
        SONY_DVB_LE_SEQ_START, /* state */
        0,                              /* autoSpectrumDetect */
        0,                              /* waitTime */
        0,                              /* totalWaitTime */
        0,                              /* spectTrial */
        0,                              /* gThetaPOfst */
        0,                              /* gThetaMOfst */
        0,                              /* gThetaNoOfst */
        0,                              /* unlockDetDone */
        0,                              /* crlOpenTrial */
        0,                              /* complete */
        0                               /* seq_result */
        
    };
    
    /* Setup function pointer table for each sequence element. */
    sony_dvb_le_seq_func_t funcs[SONY_DVB_LE_SEQ_UNKNOWN] = { 
        t2_le_SeqWaitStart,             /* SONY_DVB_LE_SEQ_START */
        t2_le_SeqWaitP1,                /* SONY_DVB_LE_SEQ_P1 */
        t2_le_SeqWaitDmdLock,           /* SONY_DVB_LE_SEQ_LOCK */
        t2_le_SeqWaitCrlOpenTrial,      /* SONY_DVB_LE_SEQ_CRL_OPEN_TRIAL */
        t2_le_SeqWaitTimingTrial,       /* SONY_DVB_LE_SEQ_TIMING_TRIAL */
        t2_le_SeqWaitSpectrumTrial      /* SONY_DVB_LE_SEQ_SPECTRUM_TRIAL */
    };

    DBG_PRINT ("t2_le_Seq\n");

    *pResult = 0;

    /* Assign demod into sequencer. */
    seq.autoSpectrumDetect = 1;

    /* Run sequencer. */
    while ((!seq.complete) && (result == SONY_DVB_OK)) {

        /* Ensure FSM state has not been corrupted. */
        if (seq.state >= SONY_DVB_LE_SEQ_UNKNOWN) {
            return ((int)SONY_DVB_ERROR_OTHER);
        }

        /* Process state. */
        result = funcs[seq.state](&seq);

        /* Sanity check on overall wait time. */
        if ((result == SONY_DVB_OK) && (seq.totalWaitTime > 30*1000)) {
            return ((int)SONY_DVB_ERROR_TIMEOUT);
        }

    }

    *pResult = seq.seq_result;
    return ((int)result);
}

/******************************************************************************
 t2_le_Seq : t2_le_SeqWaitStart
 *****************************************************************************/
static sony_dvb_result_t t2_le_SeqWaitStart (sony_dvb_le_seq_t* pSeq)
{
    sony_dvb_result_t result = SONY_DVB_OK ;

    DBG_PRINT ("t2_le_SeqWaitStart\n");
    if ((!pSeq)) {
        return (SONY_DVB_ERROR_ARG);
    }

    pSeq->waitTime = 0;

    /* Don't reset on first sequencer run because SoftReset performed in dvb_demod_Tune(). */
    if (pSeq->totalWaitTime != 0) {
        result = dvb_demod_SoftReset ();
        if (result != SONY_DVB_OK) {
            return (result);
        }
    }

    /* Assign next sequence state. */
    pSeq->state = SONY_DVB_LE_SEQ_P1;

    return (result);
}

/******************************************************************************
 t2_le_Seq : t2_le_SeqWaitP1
 *****************************************************************************/
static sony_dvb_result_t t2_le_SeqWaitP1 (sony_dvb_le_seq_t* pSeq)
{
    sony_dvb_result_t result = SONY_DVB_OK ;
    sony_dvb_demod_lock_result_t lock = DEMOD_LOCK_RESULT_NOTDETECT ;

    DBG_PRINT ("t2_le_SeqWaitP1\n");
    if ((!pSeq)) {
        return (SONY_DVB_ERROR_ARG);
    }

    /* Sleep. */
    Waitms (CXD2820_TUNE_POLL_INTERVAL);

    /* Increment wait time. */
    pSeq->waitTime += CXD2820_TUNE_POLL_INTERVAL;
    pSeq->totalWaitTime += CXD2820_TUNE_POLL_INTERVAL;

    /* Check if need to do unlock checks. */
    if (!pSeq->unlockDetDone) {
        
        /* Check for T2/non-T2 P1 symbol. */
        if (pSeq->waitTime == DVB_DEMOD_DVBT2_P1_WAIT) {
            /* Check for any P1. */
            result = dvb_demod_t2_CheckP1Lock (&lock);
            if (result != SONY_DVB_OK) {
                return (result);
            }

            /* P1 detected. */
            if (lock == DEMOD_LOCK_RESULT_LOCKED) {
                
                if (pSeq->crlOpenTrial) {

                    /* OREG_CRL_OPEN = 0. */
                    result = dvb_demod_t2_SetCrlOpen (0);
                    if (result != SONY_DVB_OK) {
                        return (result);
                    }
                    
                    pSeq->unlockDetDone = 1;
                    pSeq->state = SONY_DVB_LE_SEQ_START ;
                }
                else {
                    pSeq->state = SONY_DVB_LE_SEQ_LOCK ;
                }
            }
            else {
                /* No P1 detected, trial CRL open. */
                pSeq->state = SONY_DVB_LE_SEQ_CRL_OPEN_TRIAL ;
            }

            return (result);
        }

        /* Check for T2 P1 lock only. */
        if (pSeq->waitTime == DVB_DEMOD_DVBT2_T2_P1_WAIT) {
            result = dvb_demod_t2_CheckT2P1Lock (&lock);
            if (result != SONY_DVB_OK) {
                return (result);
            }

            /* Indicate UNLOCK if no P1 (no T2). */
            if (lock != DEMOD_LOCK_RESULT_LOCKED) {
                pSeq->complete = 1;
                pSeq->seq_result = DEM_UNLOCK;
                return (SONY_DVB_ERROR_UNLOCK);
            }
        }
    }
    
    /* Assign next sequence state. */
    pSeq->state = SONY_DVB_LE_SEQ_LOCK ;

    return (result);
}

/******************************************************************************
 t2_le_Seq : t2_le_SeqWaitDmdLock
 *****************************************************************************/
static sony_dvb_result_t t2_le_SeqWaitDmdLock (sony_dvb_le_seq_t* pSeq)
{
    sony_dvb_result_t result = SONY_DVB_OK ;
    UINT8 tsLock = 0;
    UINT8 dmdState = 0;
    UINT8 fcsAlpha = 0;

    DBG_PRINT ("t2_le_SeqWaitDmdLock\n");
    if ((!pSeq)) {
        return (SONY_DVB_ERROR_ARG);
    }

    result = dvb_demod_monitorT2_SyncStat (&(dmdState), &(tsLock));
    if (result != SONY_DVB_OK) {
        return (result);
    }

    if (dmdState == 6) {

        /* Set Bank 0x27. */
        iic_write_val(demod_block_id, CXD2820R_B0_REGC_BANK, CXD2820R_B27);

        /* Read OREG_FCS_ALPHA_U. */
        iic_read(demod_block_id, CXD2820R_B27_FCS_ALPHA, &fcsAlpha, 1);
        
        if ((fcsAlpha & 0xF0) == 0) {
            iic_write_val(demod_block_id, CXD2820R_B27_FCS_ALPHA, fcsAlpha | 0x30);
        }

        /* Demodulator locked. */
        pSeq->complete = 1;
        pSeq->seq_result = DEM_LOCK;
        return (result);
    }

    if (pSeq->gThetaNoOfst) {
        if (pSeq->waitTime > DVB_DEMOD_DVBT2_LE_WAIT_NO_OFST) {
            /* Error - timeout. */
            pSeq->complete = 1;
            return (SONY_DVB_ERROR_TIMEOUT);
        }
    }
    else {
        if (pSeq->waitTime > DVB_DEMOD_DVBT2_WAIT_LOCK) {
            /* Error - timeout. */
            pSeq->complete = 1;
            pSeq->seq_result = DEM_UNLOCK;
            return (SONY_DVB_ERROR_TIMEOUT);
        }
    }

    /* Check L1 pre. */
    {
        UINT8 l1PreToCnt = 0 ;
        UINT8 seqMaxState = 0 ;

        /* Set Bank 0x20. */
        iic_write_val(demod_block_id, CXD2820R_B0_REGC_BANK, CXD2820R_B20);

        /* Read IREG_SEQ_L1PRE_TO_CNT */
        iic_read(demod_block_id, CXD2820R_B20_SEQ_L1PRE_TO_CNT, &l1PreToCnt, 1);

        if (l1PreToCnt >= 3) {

            /* Read IREG_SEQ_MAX_STATE */
            iic_read(demod_block_id, CXD2820R_B20_SEQ_NG_OCCURRED_MAX_STATE, &seqMaxState, 1);

            /* Check if move to timing trial state. */
            if ((seqMaxState & 0x07) <= 3) {
                pSeq->state = SONY_DVB_LE_SEQ_TIMING_TRIAL;
                return (SONY_DVB_OK);
            }
        }
        
        pSeq->state = SONY_DVB_LE_SEQ_P1;
    }

    return (result);
}

/******************************************************************************
 t2_le_Seq : t2_le_SeqWaitTimingTrial
 *****************************************************************************/
static sony_dvb_result_t t2_le_SeqWaitTimingTrial (sony_dvb_le_seq_t* pSeq)
{
    sony_dvb_result_t result = SONY_DVB_OK ;
    UINT8 data[] = { 0, 0 };
    UINT8 syrActMode = 0x03;      /* Set OREG_SYR_ACTMODE = 3. */
    UINT8 fcsAlphaU = 0x00;       /* Set OREG_FCS_ALPHA_U = 0. */

    DBG_PRINT ("t2_le_SeqWaitTimingTrial\n");
    if ((!pSeq)) {
        return (SONY_DVB_ERROR_ARG);
    }

    /* 
        offset = 250/(4096/2) * 16384 
        offset = 2000
    */
    if (pSeq->gThetaPOfst) {
        if (pSeq->gThetaMOfst) {
            if (pSeq->gThetaNoOfst) {

                /* Move to wait for P1 detection without adjustment. */
                pSeq->state = SONY_DVB_LE_SEQ_P1;
                return (SONY_DVB_OK);
            }
            else {
                /* OREG_GTHETA_OFST = 0us */
                data[0] = 0x00;
                data[1] = 0x00;
                fcsAlphaU = 0x30 ;      /* OREG_FCS_ALPHA_U = 3 */
                syrActMode = 0x07 ;     /* OREG_SYR_ACTMODE = 7 */
                pSeq->gThetaNoOfst = 1 ;
            }
        }
        else {
            /* OREG_GTHETA_OFST = -offset (-2000): -250us */
            DBG_PRINT ("t2_le_SeqWaitTimingTrial : GTHETA_OFST = -250\n");
            
            data[0] = 0x78;
            data[1] = 0x30;
            pSeq->gThetaMOfst = 1 ;
        }
    }
    else {
        /* OREG_GTHETA_OFST = +offset (2000) +250us  */
        DBG_PRINT ("t2_le_SeqWaitTimingTrial : GTHETA_OFST = +250\n");
        data[0] = 0x07;
        data[1] = 0xD0;
        pSeq->gThetaPOfst = 1;
    }
    pSeq->unlockDetDone = 1; 

    /* Set Bank 0x27. */
    iic_write_val(demod_block_id, CXD2820R_B0_REGC_BANK, CXD2820R_B27);
    
    /* Set GTHETA_OFST0/1. */
    iic_write(demod_block_id, CXD2820R_B27_GTHETA_OFST0, data, sizeof(data));

    /* Set OREG_SYR_ACTMODE = 3. */
    iic_write_val(demod_block_id, CXD2820R_B27_SYR_ACTMODE, syrActMode);

    /* Set OREG_FCS_ALPHA_U = 0. */
    iic_read(demod_block_id, CXD2820R_B27_FCS_ALPHA, data, 1);
    data[0] = ( data[0] & 0x0F ) | fcsAlphaU;
    iic_write_val(demod_block_id, CXD2820R_B27_FCS_ALPHA, data[0]);

    pSeq->state = SONY_DVB_LE_SEQ_START;

    return (result);
}

/******************************************************************************
 t2_le_Seq : t2_le_SeqWaitCrlOpenTrial
 *****************************************************************************/
static sony_dvb_result_t t2_le_SeqWaitCrlOpenTrial (sony_dvb_le_seq_t* pSeq)
{
    sony_dvb_result_t result = SONY_DVB_OK ;
    UINT8 data = 0 ;

    DBG_PRINT ("t2_le_SeqWaitCrlOpenTrial\n");
    if ((!pSeq)) {
        return (SONY_DVB_ERROR_ARG);
    }

    if (pSeq->crlOpenTrial) {

        result = dvb_demod_t2_SetCrlOpen (0);
        if (result != SONY_DVB_OK) {
            return (result);
        }

        pSeq->crlOpenTrial = 0;
        pSeq->state = SONY_DVB_LE_SEQ_SPECTRUM_TRIAL;
    }
    else {
        
        /* Set Bank 0x20. */
        iic_write_val(demod_block_id, CXD2820R_B0_REGC_BANK, CXD2820R_B20);

        /* Read IREG_ENP1DET. */
        iic_read(demod_block_id, CXD2820R_B20_P1DET_STATE, &data, 1);
        
        if ((data & 0x40) != 0) {
            /* No P1 detected. */
            pSeq->state = SONY_DVB_LE_SEQ_SPECTRUM_TRIAL;
        }
        else {
            /* P1 detected. */
            /* OREG_CRL_OPEN = 1 */
            dvb_demod_t2_SetCrlOpen (1);
            pSeq->crlOpenTrial = 1;
            pSeq->state = SONY_DVB_LE_SEQ_START;
        }
    }

    return (result);
}

/******************************************************************************
 t2_le_Seq : t2_le_SeqWaitSpectrumTrial
 *****************************************************************************/
static sony_dvb_result_t t2_le_SeqWaitSpectrumTrial (sony_dvb_le_seq_t* pSeq)
{
    sony_dvb_result_t result = SONY_DVB_OK ;
    sony_dvb_demod_spectrum_sense_t sense = DVB_DEMOD_SPECTRUM_NORMAL;

    DBG_PRINT ("t2_le_SeqWaitSpectrumTrial\n");
    if ((!pSeq)) {
        return (SONY_DVB_ERROR_ARG);
    }

    /* Check to see if user requested auto spectrum detection. */
    if (!pSeq->autoSpectrumDetect) {
        pSeq->complete = 1;
        pSeq->seq_result = DEM_UNLOCK;
        return (SONY_DVB_ERROR_UNLOCK);
    }

    /* Flip expected spectrum sense. */
    dvb_demod_monitorT2_SpectrumSense (&sense);

    dvb_demod_t2_SetSpectrumSense ( sense == DVB_DEMOD_SPECTRUM_NORMAL ? DVB_DEMOD_SPECTRUM_INV : DVB_DEMOD_SPECTRUM_NORMAL );
    
    if (pSeq->spectTrial == 1) {
        pSeq->complete = 1;
        pSeq->seq_result = DEM_UNLOCK;
        return (SONY_DVB_ERROR_UNLOCK);
    }
    pSeq->spectTrial = 1;
    pSeq->state = SONY_DVB_LE_SEQ_START;

    return (result);
}

/******************************************************************************
 t2_le_Seq : t2_le_SeqTsLock
 *****************************************************************************/
int t2_le_SeqTsLock (T2_TS_LOCK* pResult)
{
    sony_dvb_result_t result = SONY_DVB_OK ;
    sony_dvb_demod_lock_result_t lock = DEMOD_LOCK_RESULT_NOTDETECT;
    UINT16 index = 0;
    UINT8 data = 0;

    *pResult = DVBT2_TS_UNKNOWN;
    DBG_PRINT ("t2_le_SeqTsLock\n");

    /* Wait for TS lock */
    for (index = 0; index < (DVB_DEMOD_DVBT2_WAIT_TS_LOCK / CXD2820_TUNE_POLL_INTERVAL); index++) {

        dvb_demod_CheckTSLock (&lock);

        if (lock == DEMOD_LOCK_RESULT_LOCKED) {
            /* DVB-T2 TS locked, attempt to optimise the Transport Stream output. */
            dvb_demod_t2_OptimiseTSOutput () ;
        }

        if (lock == DEMOD_LOCK_RESULT_LOCKED) {

            /* Set Bank 0x27. */
            iic_write_val(demod_block_id, CXD2820R_B0_REGC_BANK, CXD2820R_B27);

            /* Read OREG_SYR_ACTMODE. */
            iic_read(demod_block_id, CXD2820R_B27_SYR_ACTMODE, &data, 1);

            if ((data & 0x07) == 3) {
                DBG_PRINT ("t2_le_SeqTsLock : Set SYR_ACTMODE 0x07\n");

                /* Set OREG_SYR_ACTMODE = 7. */
                iic_write_val(demod_block_id, CXD2820R_B27_SYR_ACTMODE, 0x07);
            }
        }
        else if (index == (DVB_DEMOD_DVBT2_LE_WAIT_SYR / CXD2820_TUNE_POLL_INTERVAL)) {
            DBG_PRINT ("t2_le_SeqTsLock : Set SYR_ACTMODE 0x03\n");
            
            /* Set Bank 0x27. */
            iic_write_val(demod_block_id, CXD2820R_B0_REGC_BANK, CXD2820R_B27);

            /* Set OREG_SYR_ACTMODE = 3. */
            iic_write_val(demod_block_id, CXD2820R_B27_SYR_ACTMODE, 0x03);
        }

        switch (lock) {
        case DEMOD_LOCK_RESULT_LOCKED:
            *pResult = DVBT2_TS_LOCK;
            return ((int)SONY_DVB_OK);
        case DEMOD_LOCK_RESULT_UNLOCKED:
            *pResult = DVBT2_TS_UNLOCK;
            return ((int)SONY_DVB_ERROR_UNLOCK);
        /* Intentional fall-through. */
        case DEMOD_LOCK_RESULT_NOTDETECT:
        default:
            break;              /* continue waiting... */
        }

        Waitms (CXD2820_TUNE_POLL_INTERVAL);
        result = SONY_DVB_ERROR_TIMEOUT;
    }

    *pResult = DVBT2_TS_UNLOCK;
    return ((int)result);
}

/******************************************************************************
 * dvb_demod_CheckTSLock
 *-----------------------------------------------------------------------------
 * Input        : none
 * Output       : sony_dvb_demod_lock_result_t * pLock
 * Return       : none
 * Description  : Check TS Lock
 *****************************************************************************/
sony_dvb_result_t dvb_demod_CheckTSLock (sony_dvb_demod_lock_result_t * pLock)
{

    sony_dvb_result_t result = SONY_DVB_OK;
    UINT8 syncState = 0, tsLock = 0;

    DBG_PRINT ("dvb_demod_CheckTSLock\n");

    dvbt2_get_syncstat(&syncState, &tsLock );
    
    /* Check DMD_OK (Demodulator Lock) & TS Lock. */
    *pLock = DEMOD_LOCK_RESULT_NOTDETECT;
    if ((syncState == 6) && (tsLock == 1)) {
        *pLock = DEMOD_LOCK_RESULT_LOCKED;
    }

    return (result);
}

/******************************************************************************
 * dvb_demod_t2_CheckP1Lock
 *-----------------------------------------------------------------------------
 * Input        : none
 * Output       : sony_dvb_demod_lock_result_t * pLock
 * Return       : none
 * Description  : 
 *****************************************************************************/
sony_dvb_result_t dvb_demod_t2_CheckP1Lock (sony_dvb_demod_lock_result_t * pLock)
{
    sony_dvb_result_t result = SONY_DVB_OK;

    /* P1 early detect. */
    UINT8 p1DetLock = 0 ;
    UINT8 seqMaxState = 0 ;

    /* Set Bank 0x20 */
    iic_write_val(demod_block_id, 0x00, CXD2820R_B20);
    iic_read(demod_block_id, CXD2820R_B20_P1DET_STATE, &p1DetLock, 1);
    iic_read(demod_block_id, CXD2820R_B20_SEQ_NG_OCCURRED_MAX_STATE, &seqMaxState, 1);
    
    *pLock = (((p1DetLock & 0x10) == 0) && ((seqMaxState & 0x07) <= 2)) ? 
        DEMOD_LOCK_RESULT_NOTDETECT : DEMOD_LOCK_RESULT_LOCKED  ;

    return (result);
}

/******************************************************************************
 * dvb_demod_t2_CheckT2P1Lock
 *-----------------------------------------------------------------------------
 * Input        : none
 * Output       : sony_dvb_demod_lock_result_t * pLock
 * Return       : none
 * Description  : 
 *****************************************************************************/
sony_dvb_result_t dvb_demod_t2_CheckT2P1Lock (sony_dvb_demod_lock_result_t * pLock)
{
    sony_dvb_result_t result = SONY_DVB_OK;
    UINT8 data = 0;

    DBG_PRINT ("dvb_demod_t2_CheckT2P1Lock\n");


    /* Set bank 0x20 */
    iic_write_val(demod_block_id, 0x00, CXD2820R_B20);

    iic_read(demod_block_id, CXD2820R_B20_SEQ_NG_OCCURRED_MAX_STATE, &data, 1);
    
    /* Detect T2 P1 if detected at least once since last soft reset. */
    *pLock = ((data & 0x07) <= 2) ? DEMOD_LOCK_RESULT_NOTDETECT : DEMOD_LOCK_RESULT_LOCKED ;

    return (result);
}

/******************************************************************************
 * dvb_demod_t2_SetCrlOpen
 *-----------------------------------------------------------------------------
 * Input        : none
 * Output       : UINT8 crlOpen
 * Return       : none
 * Description  : 
 *****************************************************************************/
sony_dvb_result_t dvb_demod_t2_SetCrlOpen (UINT8 crlOpen)
{
    sony_dvb_result_t result = SONY_DVB_OK;
    DBG_PRINT ("dvb_demod_t2_SetCrlOpen\n");
    
    /* Set Bank 0x27. */
    iic_write_val(demod_block_id, CXD2820R_B0_REGC_BANK, CXD2820R_B27);

    /* Set OREG_CRL_OPEN. */
    iic_write_val(demod_block_id, CXD2820R_B27_CRL_OPEN, crlOpen);

    return (result);
}

/******************************************************************************
 * dvb_demod_SoftReset
 *-----------------------------------------------------------------------------
 * Input        : none
 * Output       : none
 * Return       : none
 * Description  : 
 *****************************************************************************/
sony_dvb_result_t dvb_demod_SoftReset (void)
{
    sony_dvb_result_t result = SONY_DVB_OK;
    DBG_PRINT ("dvb_demod_SoftReset\n");


    /* Set bank 0 */
    iic_write_val(demod_block_id, 0x00, 0x00);

    /* Soft reset (rstgen_hostrst) */
    iic_write_val(demod_block_id, 0xFE, 0x01);

    return (result);
}

/******************************************************************************
 * dvb_demod_monitorT2_SyncStat
 *-----------------------------------------------------------------------------
 * Input        : none
 * Output       : UINT8 * pSyncStat
 *                UINT8 * pTSLockStat
 * Return       : none
 * Description  : 
 *****************************************************************************/
sony_dvb_result_t dvb_demod_monitorT2_SyncStat (UINT8 * pSyncStat, 
                                                UINT8 * pTSLockStat) 
{
    sony_dvb_result_t result = SONY_DVB_OK;
    DBG_PRINT ("dvb_demod_monitorT2_SyncStat\n");


    /* Set bank 0x20 */
    iic_write_val(demod_block_id, CXD2820R_B0_REGC_BANK, CXD2820R_B20);

    {
        UINT8 data = 0;
        iic_read(demod_block_id, CXD2820R_B20_SP_TS_LOCK, &data, sizeof (data));
        *pSyncStat = data & 0x07 ;
        *pTSLockStat = (data & 0x20) >> 5;
    }
    return (result);
}

/******************************************************************************
 * dvb_demod_t2_OptimiseTSOutput
 *-----------------------------------------------------------------------------
 * Input        : none
 * Output       : none
 * Return       : none
 * Description  : 
 *****************************************************************************/
sony_dvb_result_t dvb_demod_t2_OptimiseTSOutput (void)
{
   sony_dvb_result_t result = SONY_DVB_OK;
   UINT8 ts_rate_flag=0;

    iic_write_val(demod_block_id, 0x00, 0x3f );       /* Bank 0x3f */
    iic_read(demod_block_id, 0x5a, &ts_rate_flag, 1);
    ts_rate_flag = ( (ts_rate_flag & 0x80) >> 7 );
    
    if( ts_rate_flag == 0 ) {
        iic_write_val(demod_block_id, 0x00, 0x20 );   /* Bank 0x20 */
        iic_write_val(demod_block_id, 0xe0, 0x0f );
        iic_write_val(demod_block_id, 0xe1, 0xff );
    } else {
        iic_write_val(demod_block_id, 0x00, 0x20 );   /* Bank 0x20 */
        iic_write_val(demod_block_id, 0xe0, 0x00 );
        iic_write_val(demod_block_id, 0xe1, 0x00 );
    }
        
    return (result);
}

/******************************************************************************
 * dvb_demod_monitorT2_SpectrumSense
 *-----------------------------------------------------------------------------
 * Input        : none
 * Output       : sony_dvb_demod_spectrum_sense_t* pSense
 * Return       : none
 * Description  : 
 *****************************************************************************/
sony_dvb_result_t dvb_demod_monitorT2_SpectrumSense (sony_dvb_demod_spectrum_sense_t* pSense)
{
    sony_dvb_result_t result = SONY_DVB_OK;
    DBG_PRINT ("dvb_demod_monitorT2_SpectrumSense\n");

    {
        UINT8 data = 0 ;

        /* Set bank 0x20 */
        iic_write_val(demod_block_id, CXD2820R_B0_REGC_BANK, CXD2820R_B20);

        /* Read OREG_DNCNV_SRVS (bit 4) */
        iic_read(demod_block_id, CXD2820R_B20_TDA_INVERT, &data, sizeof(data));

        *pSense = (data & 0x10) ? DVB_DEMOD_SPECTRUM_INV : DVB_DEMOD_SPECTRUM_NORMAL ;
    }

    return (result);
}

/******************************************************************************
 * dvb_demod_t2_SetSpectrumSense
 *-----------------------------------------------------------------------------
 * Input        : none
 * Output       : sony_dvb_demod_spectrum_sense_t sense
 * Return       : none
 * Description  : 
 *****************************************************************************/
sony_dvb_result_t dvb_demod_t2_SetSpectrumSense (sony_dvb_demod_spectrum_sense_t sense)
{
    sony_dvb_result_t result = SONY_DVB_OK;
    UINT8 data = 0;
    
    DBG_PRINT ("dvb_demod_t2_SetSpectrumSense\n");
    
    {
        /* Set bank 0x20 */
        iic_write_val(demod_block_id, CXD2820R_B0_REGC_BANK, CXD2820R_B20);

        /* Set OREG_DNCNV_SRVS (bit 4) */
        iic_read(demod_block_id, CXD2820R_B20_TDA_INVERT, &data, 1);
        data = (data & 0xEF) | (UINT8) (sense == DVB_DEMOD_SPECTRUM_INV ? 0x10 : 0x00);
        iic_write_val(demod_block_id, CXD2820R_B20_TDA_INVERT, data);
    }
    return (result);
}


#if 0
/******************************************************************************
 * iic_write_val
 *-----------------------------------------------------------------------------
 * INT16              id      : Block ID 
 * UINT8    subadr  : sub address
 * UINT8    val     : data value
 * 
 * retrun                   : 
 *****************************************************************************/
static INT16 iic_write_val(INT16 id, UINT8 subadr, UINT8 val){
    iic_write(id, subadr, &val, 1);
    return 0;
}

/******************************************************************************
 * iic_write
 *-----------------------------------------------------------------------------
 * INT16              id      : Block ID 
 * UINT8    subadr  : sub address
 * UINT8*   data    : pointer to data buffer to send
 * UINT8    len     : data byte length to send
 * 
 * retrun                   : 
 *****************************************************************************/
static INT16 iic_write(INT16 id, UINT8 subadr, UINT8 *data, UINT8 len){
    switch( (BLOCKID)id ) {
        
        case TUNER_TERR:
            write_tuner( DVBT_SLAVE_ADDRESS, TUNER_SLAVE_ADDRESS, subadr, data, len );
            break;
        case DEMOD_DVBT:
            write_demod( DVBT_SLAVE_ADDRESS, subadr, data, len );
            break;
        default :
            break;
    }
    return 0;
}

/******************************************************************************
 * iic_read
 *-----------------------------------------------------------------------------
 * INT16              id      : Block ID
 * UINT8    subadr  : sub address
 * UINT8*   data    : pointer to data buffer to put
 * UINT8    len     : length
 * 
 * retrun                   : 
 *****************************************************************************/
static INT16 iic_read(INT16 id, UINT8 subadr, UINT8 *data, UINT8 len){
    switch( (BLOCKID)id ) {
        case TUNER_TERR:
            libc_printf("tuner read function is not implemented. \n");
            break;
        case DEMOD_DVBT:
            read_demod( DVBT_SLAVE_ADDRESS, subadr, data, len );
            break;
        default :
            break;
    }
    return 0;
}

/******************************************************************************
 * write_tuner
 *-----------------------------------------------------------------------------
 * UINT8    slv0    : Slave Address        (0xD8)
 * UINT8    slv1    : Tuner Slave Addressr (0xC0)
 * UINT8    subadr  : Sub Address
 * UINT8*   data    : pointer to data buffer
 * UINT8    len     : length
 * 
 * retrun                   : 
 *****************************************************************************/
static INT16 write_tuner(
    UINT8 slv0,
    UINT8 slv1,
    UINT8 subadr,
    UINT8 *data,
    UINT8 len
){
 //   struct iic_trq i2cMsg;  /* -> I2C structure is an example.*/
    UINT8 senddata[ MAX_LENGTH_TUNER_WRITE + 2];
    INT16 i;
    
    senddata[0] = slv1;
    senddata[1] = subadr;
    
    if( len <= MAX_LENGTH_TUNER_WRITE ) {
        for(i = 0; i < len; i++){
            senddata[i+2] = data[i];  
        }
#if 0
        i2cMsg.type    = IIC_TASK_WRITE;
        i2cMsg.addr    = slv0;
        i2cMsg.subtype = IIC_SUB_1B;
        i2cMsg.sub     = (UINT16)(0x09);
        i2cMsg.size    = len + 2;
        i2cMsg.buf     = senddata;
#else

#endif

        /*------------------------------------------------
            Add code here for I2C communication
        ------------------------------------------------*/

        return 0;
    } else {
        libc_printf("write_tuner error : length value is over MAX_LENGTH_TUNER_WRITE! \n");
        return 1;
    }
}

/******************************************************************************
 * write_demod
 *-----------------------------------------------------------------------------
 * UINT8    slv     : Slave Address of Demod (0xD8)
 * UINT8    subadr  : sub address
 * UINT8*   data    : pointer to data buffer
 * UINT8    len     : length
 * 
 * retrun                   : 
 *****************************************************************************/
static INT16 write_demod(
    UINT8 slv,
    UINT8 subadr,
    UINT8 *data,
    UINT8 len
){
#if 0
    struct iic_trq i2cMsg;  /* -> I2C structure is an example.*/

    i2cMsg.type    = IIC_TASK_WRITE;
    i2cMsg.addr    = slv;
    i2cMsg.subtype = IIC_SUB_1B;
    i2cMsg.sub     = (UINT16)subadr;
    i2cMsg.size    = len;
    i2cMsg.buf     = data;
#else

#endif

        /*------------------------------------------------
            Add code here for I2C communication
        ------------------------------------------------*/

    return 0;
}
    
/******************************************************************************
 * read_demod
 *-----------------------------------------------------------------------------
 * UINT8    slv     : Slave Address of Demod (0xD8)
 * UINT8    subadr  : sub address
 * UINT8*   data    : pointer to data buffer
 * UINT8    len     : length
 * 
 * retrun                   : 
 *****************************************************************************/
static INT16 read_demod(
    UINT8 slv,
    UINT8 subadr,
    UINT8 *data,
    UINT8 len
){
#if 0
    struct iic_trq i2cMsg;  /* -> I2C structure is an example.*/

    i2cMsg.type     = IIC_TASK_READ;
    i2cMsg.addr     = slv;
    i2cMsg.subtype  = IIC_SUB_1B;
    i2cMsg.sub      = (UINT16)subadr;
    i2cMsg.size     = len;
    i2cMsg.buf      = data;
#else

#endif

        /*------------------------------------------------
            Add code here for I2C communication
        ------------------------------------------------*/

    return 0;
}

/******************************************************************************
 * Waitms
 *-----------------------------------------------------------------------------
 * Input : INT16 ms
 * Output: none
 * Return: void
 *****************************************************************************/
static void Waitms(UINT16 ms) {

        /*------------------------------------------------
            Add code for wait ( milliseconds unit )
        ------------------------------------------------*/
        osal_task_sleep(ms);

    return;
}
#endif
/******************************************************************************
 * History
 *
 * 2010-03-24 v0.10 initial version
 *
 * 2010-05-21 v0.20 
 *                  DVB-T 7MBW setting was added.(CoreRegSettingT2_7MBW,dvbt2_tuner_tune,dvbt2_demod_setparam)
 *                  Demod setting was revised. (CoreRegSettingT2_COMMON)
 *                  Tuner setting was revised. (tuner_init,dvbt2_tuner_tune,setRfTunerParam)
 *                  Tuning wait time was changed. (cmd_tune_dvbt2,dvbt2_tuner_tune)
 *                  L1-Change recovery procedure was added. (dvbt2_L1ChangeRecovery)
 *                  PLP selection monitor was added. (dvbt2_get_T2PLPSelFlag)
 *
 * 2010-06-21 v0.21 
 *                  L1 Pre/Post Signalling Data was added. 
 *                     (dvbt2_get_T2L1PreInfo,dvbt2_print_T2L1PreInfo,
 *                      dvbt2_get_T2L1PostInfo,dvbt2_print_T2L1PostInfo)
 *                  Monitoring function was revised. (cmd_get_dvbt2stat)
 *
 * 2010-07-23 v0.30 
 *                  Descriptions of DE204 was removed.
 *
 * 2010-09-09 v1.00 
 *                  Revision number increment (There is no change on tuner/demod settings)
 *
 * 2010-10-05 v2.00 
 *                  DVB-T2 Long Echo Sequence has been appended.
 *
 * 2010-10-05 v2.00 
 *                  DVB-T2 Long Echo Sequence has been appended.
 *
 * 2010-10-15 v2.10 
 *                  SSI/SQI calculation has been added.
 *
 ******************************************************************************/

