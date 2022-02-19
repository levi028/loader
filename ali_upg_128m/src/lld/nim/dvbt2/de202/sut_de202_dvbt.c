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
//#include <stdio.h>
//#include <stdlib.h>
#include <string.h>

#include "userdef.h"

#include "sut_de202_dvbt.h"
#include <api/libc/printf.h>


/*************************************************
 stdout function for debug
**************************************************/
//#define DBG_PRINT_OUT
#ifdef  DBG_PRINT_OUT
#define DBG_PRIN   libc_printf
#else
#define DBG_PRINT(...)
#endif

#if 0
/*************************************************
 I2C Access Block ID
**************************************************/
static BLOCKID tuner_block_id = TUNER_TERR;
static BLOCKID demod_block_id = DEMOD_DVBT;
#endif
/******************************
 I2C Access Block ID
 ******************************/
 BLOCKID tuner_block_id = TUNER_TERR;
 BLOCKID demod_block_id = DEMOD_DVBT;

#if 0
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
        cmd_init(argc - 1, argv);
    } else if (strcmp(argv[0], "t_wakeup") == 0) {
        cmd_init_dvbt(argc - 1, argv);
    } else if (strcmp(argv[0], "t_tune") == 0) {
        cmd_tune_dvbt(argc - 1, argv);
    } else if (strcmp(argv[0], "t_sleep") == 0) {
        cmd_sleep_dvbt(argc - 1, argv);
    } else if (strcmp(argv[0], "t_monitor") == 0) {
        cmd_get_dvbtstat(argc - 1, argv);
    } else if (strcmp(argv[0], "t_tsout") == 0) {
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
#endif

/******************************************************************************
 * cmd_init
 *-----------------------------------------------------------------------------
 * 
 *****************************************************************************/
static int cmd_init(int argc, char **argv)
{
    libc_printf("Enter cmd_init\n");
 
    /* tuner init */
    tuner_init();
    /* demod sleep */
    dvbt_demod_init();
    return 0;
}

/******************************************************************************
 * cmd_dvbt_wakeup_dvbt
 *-----------------------------------------------------------------------------
 * 
 *****************************************************************************/
static int cmd_init_dvbt(int argc, char **argv)
{
    DBG_PRINT("Enter cmd_init_dvbt\n");
    /* tuner awake */
    dvbt_tuner_wakeup();
    /* demod wakeup */
    dvbt_demod_wakeup();
    return 0;
}

/******************************************************************************
 * cmd_tune_dvbt
 *-----------------------------------------------------------------------------
 *
 *****************************************************************************/
static int cmd_tune_dvbt(int argc, char **argv)
{
    T_DigitalTuningParam    param;
    T_TPS_LOCK              TPS_Lock;
    T_TS_LOCK               TS_Lock;
    INT32                   offset;
    DBG_PRINT("Enter cmd_tune_dvbt\n");

    if(argc > 1) param.m_frequency = atol(argv[1]);
    else param.m_frequency = 666000;

    if (argc > 2) param.m_BandWidth = atoi(argv[2]);
    else param.m_BandWidth = 3;

    if (argc > 3) param.m_hierarchy = atoi(argv[3]);
    else param.m_hierarchy = 0;

    /* demod setting */
    dvbt_demod_setparam(&param );

    /* tuner setting */
    dvbt_tuner_tune( &param );

    /* Lock Sequence */
    LockSequence( param.m_BandWidth, &TPS_Lock, &TS_Lock, &offset);

    libc_printf("Tuning Freq   : %ld [kHz]\n",param.m_frequency);
    libc_printf("CarrierOffset : %ld [kHz]\n",offset); 
    libc_printf("TPS_Lock      : %d {0:unknown, 1:lock, 2:unlock}\n",(INT16)TPS_Lock); 
    libc_printf("TS_Lock       : %d {0:unknown, 1:lock, 2:unlock}\n",(INT16)TS_Lock); 

    /* In case of carrier offset > threshold, Go back to tuner setting */
    if( ( offset > OFFSET_THRESHOLD )||( -1 * offset > OFFSET_THRESHOLD ) ) {
        libc_printf("Go back to tuner setting with compensated frequency (%ld[kHz]) ---- \n", ( param.m_frequency + offset ) );

        param.m_frequency += offset;

        dvbt_tuner_tune( &param );

        /* wait DVB-T lock */
        LockSequence( param.m_BandWidth, &TPS_Lock, &TS_Lock, &offset);

        libc_printf("Tuning Freq   : %ld [kHz]\n",param.m_frequency);
        libc_printf("CarrierOffset : %ld [kHz]\n",offset); 
        libc_printf("TPS_Lock      : %d {0:unknown, 1:lock, 2:unlock}\n",(INT16)TPS_Lock); 
        libc_printf("TS_Lock       : %d {0:unknown, 1:lock, 2:unlock}\n",(INT16)TS_Lock); 
    }
    return 0;
}

/******************************************************************************
 * cmd_sleep
 *-----------------------------------------------------------------------------
 *
 *****************************************************************************/
static int cmd_sleep_dvbt(int argc, char **argv)
{
    DBG_PRINT("Enter cmd_sleep_dvbt\n");
   
    /* demod sleep */
    dvbt_demod_sleep();
    
    /* tuner sleep */
    dvbt_tuner_sleep();
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
    
    dvbt_tsout(ts_ena);
    return 0;
}

/******************************************************************************
 * cmd_get_dvbtstat
 *-----------------------------------------------------------------------------
 *
 *****************************************************************************/
static int cmd_get_dvbtstat(int argc, char **argv)
{
    UINT8   SyncStat;
    UINT8   TSLockStat;
    UINT16  IFAGCOut;
    INT16   RSSI;                           /* 2010-05-21 add */
    INT32   Offset;
    INT16   CN_q2;
    UINT32  BitError;
    UINT32  Period;
    UINT16  RSError;
    T_TPSInfo Info;
    T_DigitalTuningParam param;
    UINT16  SSI = 0;                            /* 2010-10-15 add */
    UINT16  SQI = 0;                            /* 2010-10-15 add */

    if (argc > 1) param.m_BandWidth = atoi(argv[1]);
    else param.m_BandWidth = 3;

    dvbt_get_syncstat(&SyncStat, &TSLockStat);
    dvbt_get_ifagc(&IFAGCOut);
    dvbt_get_rssi(&RSSI);                   /* 2010-05-21 add */
    dvbt_get_CarrierOffset(param.m_BandWidth, &Offset);
    dvbt_get_CN(&CN_q2);
    dvbt_get_PreRSBer(&BitError, &Period);
    dvbt_get_TPSInfo(&Info);
    dvbt_get_RSError(&RSError);
    /* SyncStat */
    libc_printf("\n--------------------------------\n");
    if( SyncStat == 6 ){
        libc_printf("TPS Locked | ");
    }else{
        libc_printf("Not TPS Locked(%d) | ", SyncStat);
    }
    if( TSLockStat == 1 ){
        libc_printf("TS Locked\n");
    }else{
        libc_printf("TS Not Locked\n");
    }
    /* IFAGC { 0.0374 * IIFAGC_OUT - 101.52 } */
    libc_printf("IFAGCOut(0...4095): %04u \n", IFAGCOut);
    libc_printf("RSSI [dBm]        : %4d.%01d \n", RSSI/10, abs(RSSI % 10) );    /* 2010-05-21 add */
    libc_printf("TPS Info \n");

    /* Mode */
    switch(Info.mode){
    case 0:
        libc_printf("  -----mode        = <     2k mode    >\n"); break;
    case 1:
        libc_printf("  -----mode        = <     8k mode    >\n"); break;
    default:
        libc_printf("  -----mode        = < Unknown mode(%d) >\n", Info.mode); break;
    }
    
    
    /* Guard */
    switch(Info.guard){
    case 0:
        libc_printf("  -----GI          = <      1/32      >\n"); break;
    case 1:
        libc_printf("  -----GI          = <      1/16      >\n"); break;
    case 2:
        libc_printf("  -----GI          = <      1/8       >\n"); break;
    case 3:
        libc_printf("  -----GI          = <      1/4       >\n"); break;
    default:
        libc_printf("  -----GI          = < Unknown guard(%d) >\n", (INT16)Info.guard); break;
    }

    /* hierarchy */
    switch(Info.hierarchy){
    case 0:
        libc_printf("  -----hierarchy   = <Non hierarchical>\n"); break;
    case 1:
        libc_printf("  -----hierarchy   = <      alpha = 1 >\n"); break;
    case 2:
        libc_printf("  -----hierarchy   = <      alpha = 2 >\n"); break;
    case 3:
        libc_printf("  -----hierarchy   = <      alpha = 4 >\n"); break;
    default:
        libc_printf("  -----hierarchy   = < Unknown Hir(%d) >\n", (INT16)Info.hierarchy); break;
    }
    
    /* Modulation */
    switch(Info.constellation){
    case 0:
        libc_printf("  -----Modulation  = <      QPSK      >\n"); break;
    case 1:
        libc_printf("  -----Modulation  = <      16QAM     >\n"); break;
    case 2:
        libc_printf("  -----Modulation  = <      64QAM     >\n"); break;
    default:
        libc_printf("  -----Modulation  = < Unknown Mod(%d) >\n", (INT16)Info.constellation); break;
    }

    /* rateHP */
    switch(Info.rateHP){
    case 0:
        libc_printf("  -----CodeRate HP = <      1/2       >\n"); break;
    case 1:
        libc_printf("  -----CodeRate HP = <      2/3       >\n"); break;
    case 2:
        libc_printf("  -----CodeRate HP = <      3/4       >\n"); break;
    case 3:
        libc_printf("  -----CodeRate HP = <      5/6       >\n"); break;
    case 4:
        libc_printf("  -----CodeRate HP = <      7/8       >\n"); break;
    default:
        libc_printf("  -----CodeRate HP = < Unknown CR(%d) >\n", (INT16)Info.rateHP); break;
    }

    /* rateLP */
    switch(Info.rateLP){
    case 0:
        libc_printf("  -----CodeRate LP = <      1/2       >\n"); break;
    case 1:
        libc_printf("  -----CodeRate LP = <      2/3       >\n"); break;
    case 2:
        libc_printf("  -----CodeRate LP = <      3/4       >\n"); break;
    case 3:
        libc_printf("  -----CodeRate LP = <      5/6       >\n"); break;
    case 4:
        libc_printf("  -----CodeRate LP = <      7/8       >\n"); break;
    default:
        libc_printf("  -----CodeRate LP = < Unknown CR(%d) >\n", (INT16)Info.rateLP); break;
    }

    /* Cell ID */
    libc_printf("  -----cell ID     = <       %2d       >\n",Info.cellID);

    /* CarrierOffset */
    libc_printf("CarrierOffset     = %ld kHz\n", Offset);
    /* CN */
    libc_printf("CN                = %d.%02d [dB] \n", ( CN_q2 / 4 ), ( 0x03 & CN_q2 ) * 25 );
    /* preRS-BER */
    libc_printf("PreRSBER :BitError=%8ld Period=%8ld BER=%e\n", BitError, Period, (float)BitError/(float)Period);
    /* RSError */
    libc_printf("RSError  :BitError=%8d \n", RSError);
    libc_printf("--------------------------------\n");
  
/* SSI/SQI */ /* 2010-10-15 add */
    dvbt_get_ssi_sqi(&SQI,&SSI);
    libc_printf("SSI %d\n",SSI);
    libc_printf("SQI %d\n",SQI);

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
    libc_printf("t_wakeup\n");
    libc_printf("        register setting for DVB-T receive\n");
    libc_printf("\n");
    libc_printf("t_tune [freq] [bandwidth] [Hier] : \"t_tune 666000 3 0\" \n");
    libc_printf("        DVB-T tuning\n");
    libc_printf("        freq        :center frequency [kHz] \n");
    libc_printf("        bandwidth   :band width setting 1:6MHz, 2:7MHz, 3:8MHz (6MBW is not supported)\n");
    libc_printf("        Heir        :Select Hierarchy   0:High 1:Low \n");
    libc_printf("\n");
    libc_printf("t_sleep\n");
    libc_printf("        sleep\n");
    libc_printf("t_tsout [tsout]\n");
    libc_printf("        tsout       : Tsout enable flag  0:disable Tsout, 1:enable Tsout\n");
    libc_printf("\n");
    libc_printf("t_monitor [bandwidth]\n");
    libc_printf("        bandwidth   :band width setting 1:6MHz, 2:7MHz, 3:8MHz (6MBW is not supported) \n");
    libc_printf("        get status(Lock-Status/IFAGC-Level/Carrier-OFFSET/CN/preRS-BER/TPS_info)\n");
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
 * show help
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
void tuner_init(void)
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
    iic_write_val( tuner_block_id, 0x03, 0xC0 );    /* 2010-05-21 add */

    dvbt_tuner_sleep();
}

/******************************************************************************
 * dvbt_tuner_wakeup
 *-----------------------------------------------------------------------------
 * Input        : none
 * Output       : none
 * Return       : none
 * Description  : Terrestrial Digital Awake
 *****************************************************************************/
void dvbt_tuner_wakeup(void)
{
    UINT8 taddr;
    UINT8 tdata[2];
    UINT8 data_length;

    DBG_PRINT("Enter dvbt_tuner_wakeup\n");
    
    taddr    = 0x14;
    tdata[0] = 0xFB;    /* 0x14 */
    tdata[1] = 0x0F;    /* 0x15 */
    data_length = 2;
    iic_write( tuner_block_id, taddr, tdata, data_length );

    return;

}

/******************************************************************************
 * dvbt_tuner_tune
 *-----------------------------------------------------------------------------
 * Input        : T_DigitalTuningParam* pDigParam
 * Output       : none
 * Return       : none
 * Description  : Terrestrial Digital Tuning
 *****************************************************************************/
void dvbt_tuner_tune(T_DigitalTuningParam* pDigParam)
{
    UINT8   taddr;
    UINT8   tdata[5];
    UINT8   data_length;
    UINT32  frequency;
    UINT8   bandWidthRegValue = 0x20;

    DBG_PRINT("Enter dvbt_tuner_tune\n");
    
    dvbt_setRfTunerParam( pDigParam->m_BandWidth );  /* set BANK 0x00 in this function*/
    
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

    frequency = pDigParam->m_frequency;
    
    switch( pDigParam->m_BandWidth )
    {
    //BWSelect
        case 1: /* 6MBW */
            /* 6MBW is currently not supported */
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
 * dvbt_tuner_sleep
 *-----------------------------------------------------------------------------
 * Input        : none
 * Output       : none
 * Return       : none
 * Description  : stop tuning
 *****************************************************************************/
void dvbt_tuner_sleep(void)
{
    UINT8 taddr;
    UINT8 tdata[2];
    UINT8 data_length;
    
    DBG_PRINT("Enter dvbt_tuner_sleep\n");

    taddr    = 0x14;
    tdata[0] = 0x00;    /* 0x14 */
    tdata[1] = 0x04;    /* 0x15 */
    data_length = 2;
    iic_write( tuner_block_id, taddr, tdata, data_length );

    return;
}

/******************************************************************************
 * dvbt_setRfTunerParam
 *-----------------------------------------------------------------------------
 * Input : UINT8  bandWidth
 * Output: none
 * Return: void
 *****************************************************************************/
void dvbt_setRfTunerParam(UINT8 bandWidth)
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
    if (bandWidth == 1 ) {  /* BW 6M */
        /* 6MBW is currently not supported */
        tdata[1] = 0x13;       /* 0x0C */
        tdata[2] = 0x20;       /* 0x0D */
        tdata[3] = 0xE5;//0x05;       /* 0x0E */ updaet 0n 20110720
        tdata[4] = 0x1C;//0x00;       /* 0x0F */
    } else if (bandWidth == 2 ) {         /* BW 7M */
        tdata[1] = 0x13;       /* 0x0C */ /* 2010-09-09 change */
        tdata[2] = 0x20;       /* 0x0D */
        tdata[3] = 0xED;       /* 0x0E */ /* 2010-05-21 change */
        tdata[4] = 0x00;       /* 0x0F */
    } else {                              /* BW 8M */
        tdata[1] = 0x13;       /* 0x0C */ /* 2010-09-09 change */
        tdata[2] = 0x20;       /* 0x0D */
        tdata[3] = 0xED;       /* 0x0E */ /* 2010-05-21 change */
        tdata[4] = 0x00;       /* 0x0F */
    }
    data_length = 5;
    iic_write( tuner_block_id, taddr, tdata, data_length );
    
    return ;
}

/******************************************************************************
 * dvbt_demod_init
 *-----------------------------------------------------------------------------
 * Input        : none
 * Output       : none
 * Return       : none
 * Description  : DVB demod init
 *****************************************************************************/
void dvbt_demod_init(void)
{
    DBG_PRINT("Enter dvbt_demod_init\n");

    iic_write_val( demod_block_id,  0x00 , 0x00 );
    iic_write_val( demod_block_id,  0xff , 0x1f );
    iic_write_val( demod_block_id,  0x85 , 0x00 );
    iic_write_val( demod_block_id,  0x81 , 0x00 );
    iic_write_val( demod_block_id,  0x80 , 0x00 );

    return;
}

/******************************************************************************
 * dvbt_demod_wakeup
 *-----------------------------------------------------------------------------
 * Input        : none
 * Output       : none
 * Return       : none
 * Description  : DVB demod init
 *****************************************************************************/
void dvbt_demod_wakeup(void)
{

    DBG_PRINT("Enter dvbt_demod_wakeup\n");

    iic_write_val( demod_block_id,  0x00 , 0x00 );
    iic_write_val( demod_block_id,  0x80 , 0x00 );
    iic_write_val( demod_block_id,  0x81 , 0x03 );
    iic_write_val( demod_block_id,  0x85 , 0x07 );

    iic_write_val( demod_block_id,  0xff , 0x09 );
    return;
}

/******************************************************************************
 * dvbt_demod_setparam
 *-----------------------------------------------------------------------------
 * Input        : T_DigitalTuningParam* pDigParam
 * Output       : none
 * Return       : none
 * Description  : Terrestrial Digital Tuning
 *****************************************************************************/
void dvbt_demod_setparam(T_DigitalTuningParam* pDigParam)
{
    INT16 i;

    DBG_PRINT("Enter dvbt_demod_setparam\n");

    // DVB-T Common
    iic_write_val( demod_block_id,  0x00 , 0x00 );  /* bank 0 */
    iic_write_val( demod_block_id,  0x81 , 0x03 );
    iic_write_val( demod_block_id,  0x85 , 0x07 );
    iic_write_val( demod_block_id,  0x80 , 0x00 );
    iic_write_val( demod_block_id,  0x70 , TS_SETTING_DVBT ); /* 2010-05-21 change */
    iic_write_val( demod_block_id,  0xff , 0x09 );

    iic_write_val( demod_block_id,  0x00 , 0x01 );  /* bank 1 */
    iic_write_val( demod_block_id,  0x6A , 0x50 );
    iic_write_val( demod_block_id,  0x48 , 0x30 );

    iic_write_val( demod_block_id,  0x00 , 0x04 );  /* bank 4 */
    iic_write_val( demod_block_id,  0x27 , 0x41 );

    switch( pDigParam->m_BandWidth )
    {
        case 1: /* 6MBW */
            /* 6MBW is currently not supported */
		for(i = 0; i < CORE_REGSETTING_MAX_ENTRY; i++ ) {
                iic_write_val( demod_block_id,  CoreRegSetting_6MBW[i].m_address, CoreRegSetting_6MBW[i].m_data);
            }
            iic_write_val( demod_block_id,  0x00 , 0x07 ); /* bank 7 */
            iic_write_val( demod_block_id,  0x38 , 0x00 );
            iic_write_val( demod_block_id,  0x39 , 0x02 );
            iic_write_val( demod_block_id,  0x3C , 0x05 );
            iic_write_val( demod_block_id,  0x3D , 0x05 );
            iic_write_val( demod_block_id,  0x44 , 0x91 );
            iic_write_val( demod_block_id,  0x45 , 0xa0 );	
            break;
        case 2: /* 7MBW */
            for(i = 0; i < CORE_REGSETTING_MAX_ENTRY; i++ ) {
                iic_write_val( demod_block_id,  CoreRegSetting_7MBW[i].m_address, CoreRegSetting_7MBW[i].m_data);
            }
            iic_write_val( demod_block_id,  0x00 , 0x07 ); /* bank 7 */
            iic_write_val( demod_block_id,  0x38 , 0x01 );
            iic_write_val( demod_block_id,  0x39 , 0x02 );
            iic_write_val( demod_block_id,  0x3C , 0x03 );
            iic_write_val( demod_block_id,  0x3D , 0x01 );
            iic_write_val( demod_block_id,  0x44 , 0x97 );
            iic_write_val( demod_block_id,  0x45 , 0x00 );
            break;
        case 3: /* 8MBW */
        default:
            for(i = 0; i < CORE_REGSETTING_MAX_ENTRY; i++ ) {
                iic_write_val( demod_block_id,  CoreRegSetting_8MBW[i].m_address, CoreRegSetting_8MBW[i].m_data);
            }
            iic_write_val( demod_block_id,  0x00 , 0x07 ); /* bank 7 */
            iic_write_val( demod_block_id,  0x38 , 0x01 );
            iic_write_val( demod_block_id,  0x39 , 0x02 );
            iic_write_val( demod_block_id,  0x3C , 0x05 );
            iic_write_val( demod_block_id,  0x3D , 0x05 );
            iic_write_val( demod_block_id,  0x44 , 0x91 );
            iic_write_val( demod_block_id,  0x45 , 0xA0 );
            break;
    }

    /* hierarchy */
    if( pDigParam->m_hierarchy == 0 ) {
        /* High Priority */
        iic_write_val( demod_block_id,  0x00 , 0x04 ); /* bank 4 */
        iic_write_val( demod_block_id,  0x10 , 0x00 );
        iic_write_val( demod_block_id,  0x00 , 0x00 ); /* bank 0 */
    } else {
        /* Low Priority */
        iic_write_val( demod_block_id,  0x00 , 0x04 ); /* bank 4 */
        iic_write_val( demod_block_id,  0x10 , 0x01 );
        iic_write_val( demod_block_id,  0x00 , 0x00 ); /* bank 0 */
    }

    return;
}

/******************************************************************************
 * dvbt_demod_start
 *-----------------------------------------------------------------------------
 * Input        : none
 * Output       : none
 * Return       : none
 * Description  : Terrestrial Digital Tuning
 *****************************************************************************/
void dvbt_demod_start(void)
{
  
    DBG_PRINT("Enter dvbt_demod_start\n");

    iic_write_val( demod_block_id,  0xfe , 0x01 );    /* DVB-T SRST */
    
    return;
    
}

/******************************************************************************
 * dvbt_demod_sleep
 *-----------------------------------------------------------------------------
 * Input        : none
 * Output       : none
 * Return       : none
 * Description  : DVB demod sleep
 *****************************************************************************/
void dvbt_demod_sleep(void)
{

    DBG_PRINT("Enter dvbt_demod_sleep\n");

    iic_write_val( demod_block_id,  0x00 , 0x00 );
    iic_write_val( demod_block_id,  0xff , 0x1f );
    iic_write_val( demod_block_id,  0x85 , 0x00 );
    iic_write_val( demod_block_id,  0x81 , 0x00 );
    iic_write_val( demod_block_id,  0x80 , 0x00 );

    return;

}

/******************************************************************************
 * dvbt_tsout
 *-----------------------------------------------------------------------------
 * Input        : UINT8 enable
 * Output       : none
 * Return       : none
 * Description  : Terrestrial Digital Tuning
 *****************************************************************************/
void dvbt_tsout( UINT8 enable )
{

    iic_write_val( demod_block_id,  0x00 , 0x00 );
    if( enable == 1 )
        iic_write_val( demod_block_id,  0xff , 0x08 );
    else
        iic_write_val( demod_block_id,  0xff , 0x09 );
    return;
}

/******************************************************************************
 * Lock Sequence
 *-----------------------------------------------------------------------------
 * Input : UINT8       bandwidth
 *       : T_TPS_LOCK* pTPS_Lock   TPS_Lock flag
 *       : T_TS_LOCK*  pTS_Lock    TS_Lock flag
 *       : INT32*      pOffset      Carrier Frequency Offset [kHz]
 * Return: void
 *****************************************************************************/
void LockSequence(UINT8 bandwidth, T_TPS_LOCK* pTPS_Lock, T_TS_LOCK* pTS_Lock, INT32* pOffset)
{
    /* state */
    T_LOCKSTAT LockState;
    /* initialization */
    LockState.m_state       = DVBT_LOCKSTAT_START;
    LockState.m_tslock      = TS_UNKNOWN;
    LockState.m_tpslock     = TPS_UNKNOWN;
    LockState.m_offset      = 0;                    /* carrier offset  */
    LockState.m_waittime    = 0;                    /* wait time       */
    LockState.m_seq_count   = 0;                    /* timeout counter */
    LockState.m_bandwidth   = bandwidth;            /* bandwidth       */
    /* Lock Sequence */
    while( LockState.m_state != DVBT_LOCKSTAT_END ) {
        Waitms( LockState.m_waittime );
        switch( LockState.m_state ) {
            case DVBT_LOCKSTAT_START:
                dvbt_lockseq_start(&LockState);
                break;
            case DVBT_LOCKSTAT_WAIT_TPS_LOCK:
                dvbt_lockseq_wait_tps_lock(&LockState);
                break;
            case DVBT_LOCKSTAT_WAIT_TS_LOCK:
                dvbt_lockseq_wait_ts_lock(&LockState);
                break;
            case DVBT_LOCKSTAT_END:
            default:
                break;
        }
    }
    *pOffset   = LockState.m_offset;
    *pTS_Lock  = LockState.m_tslock;
    *pTPS_Lock = LockState.m_tpslock;
    return;
}

/******************************************************************************
 * dvbt_lockseq_start
 *-----------------------------------------------------------------------------
 * Input        : 
 * Output       : T_LOCKSTAT* LockState
 * Return       : none
 *****************************************************************************/
void dvbt_lockseq_start(T_LOCKSTAT* pLockState)
{
    /* DVB-T Demod SRST */ 
    dvbt_demod_start();

    /* State updating */
    pLockState->m_state       = DVBT_LOCKSTAT_WAIT_TPS_LOCK;
    pLockState->m_tslock      = TS_UNKNOWN;
    pLockState->m_tpslock     = TPS_UNKNOWN;
    pLockState->m_offset      = 0;
    pLockState->m_waittime    = 100;
    pLockState->m_seq_count   = 0;
}

/******************************************************************************
 * dvbt_lockseq_wait_tps_lock
 *-----------------------------------------------------------------------------
 * Input        : 
 * Output       : T_LOCKSTAT* LockState
 * Return       : none
 *****************************************************************************/
void dvbt_lockseq_wait_tps_lock(T_LOCKSTAT* pLockState)
{
    T_TPS_LOCK TPS_Lock;
    T_TS_LOCK  TS_Lock;
    UINT8      Unlock_Flag;
    INT32      offset;
    
    /* Timeout counter */
    pLockState->m_seq_count   += 1;

    /* Read Demod Status */
    dvbt_GetDemodState(pLockState->m_bandwidth, &TPS_Lock, &TS_Lock, &Unlock_Flag, &offset);

    /* Check Demod Status */
    if( Unlock_Flag == 1 ) {
        /* State updating */
        pLockState->m_state       = DVBT_LOCKSTAT_END;
        pLockState->m_tslock      = TS_UNLOCK;
        pLockState->m_tpslock     = TPS_UNLOCK;
        pLockState->m_offset      = 0;
        pLockState->m_waittime    = 0;
        pLockState->m_seq_count   = 0;
        return;
    } else if( TPS_Lock == 1 ) {
        if( ( offset > (INT32)OFFSET_THRESHOLD )||( -1 * offset > (INT32)OFFSET_THRESHOLD ) ) {
            /* State updating */
            pLockState->m_state       = DVBT_LOCKSTAT_END;
            pLockState->m_tpslock     = TPS_LOCK;
            pLockState->m_offset      = offset;
            pLockState->m_waittime    = 0;
            pLockState->m_seq_count   = 0;
            return;
        } else {
            /* State updating */
            pLockState->m_state       = DVBT_LOCKSTAT_WAIT_TS_LOCK;
            pLockState->m_tpslock     = TPS_LOCK;
            pLockState->m_offset      = offset;
            pLockState->m_waittime    = 0;
            pLockState->m_seq_count   = 0;
	      libc_printf("TPS_LOCK \n");
            return;
        }
    }

    /* Check Timeout */
    if( pLockState->m_seq_count >= DVBT_LOCK_TIMEOUT ) {
        /* State updating */
        pLockState->m_state       = DVBT_LOCKSTAT_END;
        pLockState->m_tslock      = TS_UNLOCK;
        pLockState->m_tpslock     = TPS_UNLOCK;
        pLockState->m_offset      = 0;
        pLockState->m_waittime    = 0;
        pLockState->m_seq_count   = 0;
        return;                                             /* 2010-06-10 add */
    }

    pLockState->m_waittime    = LOCK_POLLING_PERIOD;        /* 2010-06-10 add */
    return;                                                 /* 2010-06-10 add */
}

/******************************************************************************
 * dvbt_lockseq_wait_ts_lock
 *-----------------------------------------------------------------------------
 * Input        : 
 * Output       : T_LOCKSTAT* LockState
 * Return       : none
 *****************************************************************************/
void dvbt_lockseq_wait_ts_lock(T_LOCKSTAT* pLockState)
{
    T_TPS_LOCK TPS_Lock;
    T_TS_LOCK  TS_Lock;
    UINT8      Unlock_Flag;
    INT32      offset;
    
    /* Timeout counter */
    pLockState->m_seq_count   += 1;

    /* Read Demod Status */
    dvbt_GetDemodState(pLockState->m_bandwidth, &TPS_Lock, &TS_Lock, &Unlock_Flag, &offset);

    /* Check TS Lock */
    if( TS_Lock == 1 ) {
        /* State updating */
        pLockState->m_state       = DVBT_LOCKSTAT_END;
        pLockState->m_tslock      = TS_LOCK;
        pLockState->m_waittime    = 0;
        pLockState->m_seq_count   = 0;
	libc_printf("TS_Lock \n");
        return;
    }

    /* Check Timeout */
    if( pLockState->m_seq_count >= DVBT_LOCK_TIMEOUT ) {
        /* State updating */
        pLockState->m_state       = DVBT_LOCKSTAT_END;
        pLockState->m_tslock      = TS_UNLOCK;
        pLockState->m_waittime    = 0;
        pLockState->m_seq_count   = 0;
        return;                                             /* 2010-06-10 add */
    }
    
    pLockState->m_waittime    = LOCK_POLLING_PERIOD;        /* 2010-06-10 add */
    return;                                                 /* 2010-06-10 add */
}

/******************************************************************************
 * dvbt_GetDemodState
 *-----------------------------------------------------------------------------
 * Input        : none
 * Output       : UINT16* TPS_Lock,
 *                UINT16* TS_Lock, 
 *                UINT16* Unlock_Flag,
 *                INT32*  offset
 * Return       : none
 *****************************************************************************/
void dvbt_GetDemodState(UINT8 bandwidth, T_TPS_LOCK* pTPS_Lock, T_TS_LOCK* pTS_Lock, UINT8* pUnlock_Flag, INT32* pOffset)
{
    UINT8 A_STATE;
    UINT8 B_STATE;
    UINT8 C_STATE;
    UINT8 rdata[4] = {0};
    UINT8 spectrum_inversion;
    INT32 data32;

    /* Bank 0 */
    iic_write_val( demod_block_id,  0x00, 0x00 );

    /* Lock Status Read */
    iic_read( demod_block_id, 0x10, rdata, 1 );
    A_STATE = 0x07 & rdata[0];

    iic_read( demod_block_id, 0x14, rdata, 1 );
    B_STATE    = ( 0x0f & rdata[0] );

    iic_write_val( demod_block_id,  0x00, 0x0a );
    iic_read( demod_block_id, 0x14, rdata, 1 );
    C_STATE    = ( 0x10 & rdata[0] ) >> 4;
    iic_write_val( demod_block_id,  0x00, 0x00 );

    /* TPS Lock */
    if( A_STATE == 6 ) {
        *pTPS_Lock   = 1;
	  libc_printf("pTPS_Lock\n");
    } else {
        *pTPS_Lock   = 0;
		  libc_printf("pTPS_ULock : A_STATE= %d\n",A_STATE);
    }

    /* TS Lock */
    iic_read( demod_block_id, 0x73, rdata, 1 );
    *pTS_Lock    = ( 0x08 & rdata[0] ) >> 3;
   if(*pTS_Lock==0)
   	libc_printf("TS_Lock\n");
   else
   	libc_printf("TS_UNLock\n");  
   	

    /* Unlock_Flag ( = no DVB-T siganl ) */
    if ((B_STATE >= 4) && !C_STATE && !(A_STATE >= 6)) {
        *pUnlock_Flag = 1;
		 libc_printf("no DVB-T siganl\n");
    } else {
        *pUnlock_Flag = 0;
		 libc_printf("Have  DVB-T siganl\n");
		
    }

    if( *pTPS_Lock == 1 ) {
        /* Carrier Frequency Offset */
        iic_read( demod_block_id, 0x4C, rdata, 4 );

        data32 = - dvb_Convert2SComplement(
                    ((UINT32)(rdata[0] & 0x1F) << 24) 
                  | ((UINT32)rdata[1] << 16)
                  | ((UINT32)rdata[2] << 8)
                  |  (UINT32)rdata[3], 29);

        switch(bandwidth){
            case 1:
                /* 6MBW is not supported */
                *pOffset = 0;
                break;
            case 2:
                *pOffset = data32 / 33554;
                break;
            case 3:
                *pOffset = data32 / 29360;
                break;
            default:
                *pOffset = 0;
                break;
        }

        iic_write_val( demod_block_id,  0x00, 0x07 );    /* bank 7 */
        iic_read( demod_block_id, 0xC6, rdata, 1 );        /* spectrum inversion flag */
        iic_write_val( demod_block_id,  0x00, 0x00 );    /* bank 0 */
        spectrum_inversion = rdata[0] & 0x01;

        if( spectrum_inversion == 1 ) {
            *pOffset *= -1;
        }
    } else {
        *pOffset = 0;
    }
    return;
}

/******************************************************************************
 * dvbt_get_syncstat
 *-----------------------------------------------------------------------------
 * Input        : none
 * Output       : UINT8* pSyncStat
 *              : UINT8* pTSLockStat
 * Return       : none
 *****************************************************************************/
void dvbt_get_syncstat(UINT8* pSyncStat, UINT8* pTSLockStat )
{
    UINT8 data = 0;
    iic_write_val( demod_block_id,  0x00, 0x00 );
    iic_read( demod_block_id, 0x10, &data, 1 );
    *pSyncStat    = 0x07 & data;
    iic_read( demod_block_id, 0x73, &data, 1 );
    *pTSLockStat  = ( 0x08 & data ) >> 3;
    return;
}

/******************************************************************************
 * dvbt_get_ifagc
 *-----------------------------------------------------------------------------
 * Input        : none
 * Output       : UINT16* pIFAGCOut
 * Return       : none
 *****************************************************************************/
void dvbt_get_ifagc(UINT16* pIFAGCOut )
{
    UINT8 rdata[2] = {0};

    iic_write_val( demod_block_id,  0x00, 0x00 );
    iic_read( demod_block_id, 0x26, rdata, 2 );
    *pIFAGCOut = ((UINT16)(rdata[0] & 0x0F) << 8) | (UINT16)rdata[1];
    return;
}

/* 2010-05-21 add */
/******************************************************************************
 * dvbt_get_rssi
 *-----------------------------------------------------------------------------
 * Input        : none
 * Output       : INT16* pRSSI ( x10 )
 * Return       : none
 * Description  :
 *****************************************************************************/
void dvbt_get_rssi(INT16* pRSSI )
{
    UINT8  rdata[2] = {0};
    UINT16 IFAGCOUT = 0;
    
    iic_write_val( demod_block_id, 0x00, 0x00 );
    iic_read( demod_block_id, 0x26, rdata, 2 );

    IFAGCOUT = ( ( (UINT16)(rdata[0] & 0x0F) << 8 ) )  
             | ( (UINT16)rdata[1] );
    
    *pRSSI = ( ( (INT32)IFAGCOUT * (INT32)AGC_A_T * (INT32)AGC_R ) / 10000 )
             - AGC_B_T;
    
    return;

}

/******************************************************************************
 * dvbt_get_CarrierOffset
 *-----------------------------------------------------------------------------
 * Input        : UINT8  bandwidth
 * Output       : INT32* pOffset
 * Return       : none
 *****************************************************************************/
void dvbt_get_CarrierOffset(UINT8 bandwidth, INT32* pOffset )
{
    INT32 data;
    UINT8 rdata[4] = {0};
    UINT8 spectrum_inversion;
    UINT8 SyncStat;
    UINT8 TSLockStat;

    dvbt_get_syncstat(&SyncStat,&TSLockStat);
    if( SyncStat != 6 ) {
        *pOffset = 0;
        libc_printf("!!!! CarrierOffset invalid (TPS_UnLock)\n");
        return;
    }

    iic_write_val( demod_block_id,  0x00, 0x00 );
    iic_read( demod_block_id, 0x4C, rdata, 4 );

    data = - dvb_Convert2SComplement(
                ((UINT32)(rdata[0] & 0x1F) << 24)
              | ((UINT32)rdata[1] << 16)
              | ((UINT32)rdata[2] << 8)
              |  (UINT32)rdata[3], 29);
    
    switch(bandwidth){
        case 1:
            /* 6MBW is not supported */
            *pOffset = 0;
            break;
        case 2:
            *pOffset = data / 33554;
            break;
        case 3:
            *pOffset = data / 29360;
            break;
        default:
            *pOffset = 0;
            break;
    }

    iic_write_val( demod_block_id,  0x00, 0x07 );    /* bank 7 */
    iic_read( demod_block_id, 0xC6, rdata, 1 );        /* spectrum inversion flag */
    iic_write_val( demod_block_id,  0x00, 0x00 );    /* bank 0 */
    spectrum_inversion = rdata[0] & 0x01;

    if( spectrum_inversion == 1 ) {
        *pOffset *= -1;
    }
    return;
}

/******************************************************************************
 * dvbt_get_CN
 *-----------------------------------------------------------------------------
 * Input        : none
 * Output       : UINT16* pCN_q2 :CN[dB] ( fixed point Q2-format )
 * Return       : none
 *****************************************************************************/
void dvbt_get_CN(INT16* pCN_q2 )
{
    UINT8   rdata[2] = {0};
    UINT16  CN_DATA = 0;

    iic_write_val( demod_block_id,  0x00, 0x00 );
    iic_read( demod_block_id, 0x2C, rdata, 2 );
    CN_DATA = ((UINT16)rdata[0] << 8) | (UINT16)rdata[1];

    /* CN [dB] = 10 * log{ CN_DATA / (32000 - CN_DATA) } + 33.10 */
    /* pCN_q2 is fixed point variable in Q2-format. */
    *pCN_q2 =   CnCalc( CN_DATA ) - CnCalc( 32000 - CN_DATA ) + 132;/* 132 = 33.1 * 4 (Q2 format & round) */

    return;
}

/******************************************************************************
 * dvbt_get_PreRSBer
 *-----------------------------------------------------------------------------
 * Input        : none
 * Output       : UINT32* pBitError
 *              : UINT32* pPeriod
 * Return       : none
 *****************************************************************************/
 void dvbt_get_PreRSBer(UINT32* pBitError, UINT32* pPeriod )
{
    INT16 i;
    UINT8 valid;

    dvbt_PreRSBer_ResetBer();

    for( i = 0; i < 10; i++ ){
        Waitms(100);
        dvbt_PreRSBer_GetBer(&valid, pBitError);
        if(valid != 0) {
            break;
        }
    }

    dvbt_PreRSBer_BerSize(pPeriod);

    return;
}

/******************************************************************************
 * dvbt_PreRSBer_ResetBer
 *-----------------------------------------------------------------------------
 * Input        : none
 * Output       : none
 * Return       : none
 *****************************************************************************/
void dvbt_PreRSBer_ResetBer(void)
{
    iic_write_val( demod_block_id,  0x00, 0x00 );
    iic_write_val( demod_block_id,  0x79, 0x01 );
    return;
}

/******************************************************************************
 * dvbt_PreRSBer_GetBer
 *-----------------------------------------------------------------------------
 * Input        : none
 * Output       : UINT8* valid      (0:invalid, 1:valid)
 *              : UINT32* pBitError
 * Return       : none
 *****************************************************************************/
void dvbt_PreRSBer_GetBer(UINT8* valid, UINT32* pBitError)
{
    UINT8 rdata[3] = {0};

    iic_write_val( demod_block_id,  0x00, 0x00 );
    iic_read( demod_block_id, 0x76, rdata, 3 );
    *valid      = ( rdata[2] & 0x80 ) >> 7;
    *pBitError  = ((UINT32)(rdata[2] & 0x0F) << 16) | ((UINT32)rdata[1] << 8) | (UINT32)rdata[0];
    return;
}

/******************************************************************************
 * dvbt_PreRSBer_BerSize
 *-----------------------------------------------------------------------------
 * Input        : none
 * Output       : UINT32* pPeriod
 * Return       : none
 *****************************************************************************/
void dvbt_PreRSBer_BerSize(UINT32* pPeriod )
{
    UINT8 rdata = 0;

    iic_write_val( demod_block_id,  0x00, 0x00 );
    iic_read( demod_block_id, 0x72, &rdata, 1 );
    *pPeriod = ((UINT32)1 << (rdata & 0x1F)) * 204 * 8;
    return;
}

/******************************************************************************
 * dvbt_get_RSError
 *-----------------------------------------------------------------------------
 * Input        : none
 * Output       : UINT16* pBitError
 * Return       : none
 *****************************************************************************/
void dvbt_get_RSError(UINT16* pBitError)
{
    UINT8 rdata[2] = {0};

    iic_write_val( demod_block_id,  0x00, 0x00 );
    iic_read( demod_block_id, 0xEA, rdata, 2 );
    *pBitError = ((UINT16)rdata[0] << 8) | (UINT16)rdata[1];
    return;
}

/******************************************************************************
 * dvbt_get_TPSInfo
 *-----------------------------------------------------------------------------
 * Input        : none
 * Output       : T_TPSInfo* pInfo
 * Return       : none
 *****************************************************************************/
void dvbt_get_TPSInfo(T_TPSInfo* pInfo)
{
    UINT8 rdata[5] = {0};

    iic_write_val( demod_block_id,  0x00, 0x00 );
    iic_read( demod_block_id, 0x2F, rdata, 5 );

    pInfo->constellation = (rdata[0] >> 6) & 0x03; /* 0x2F, bit[7:6] */
    pInfo->hierarchy = (rdata[0] >> 3) & 0x07;     /* 0x2F, bit[5:3] */
    pInfo->rateHP = rdata[0] & 0x07;               /* 0x2F, bit[2:0] */
    pInfo->rateLP = (rdata[1] >> 5) & 0x07;        /* 0x30, bit[7:5] */
    pInfo->guard = (rdata[1] >> 3) & 0x03;         /* 0x30, bit[4:3] */
    pInfo->mode = (rdata[1] >> 1) & 0x03;          /* 0x30, bit[2:1] */
    pInfo->cellID = (UINT16)(((UINT16)rdata[3] << 8) | (UINT16)rdata[4]); /* 0x32, 0x33 */

    return;
}

/* add 2010-10-15 */
/******************************************************************************
 * dvbt_get_ssi_sqi
 * The function is an implementation example of SSI/SQI calculation
 *-----------------------------------------------------------------------------
 * Input        : none
 * Output       : pSQI, pSSI [%]
 * Return       : none
 *****************************************************************************/
static void dvbt_get_ssi_sqi (UINT16* pSQI, UINT16* pSSI)
{
    T_TPSInfo tps_info;     /* TPS signaling information */
    UINT8 prioirty = 0;     /* priority of current receiving stream [0:HP/1:LP] */
    UINT16 cnreq_index = 0;
    UINT16 rate = 0;        /* code rate of current receiving stream */
    INT16 rssi = 0;         /* RSSI [dBm x 10] */
    INT16 Prel = 0 ;        /* Prel [dBm x 10] */
    INT16 Pref = 0 ;        /* reference signal level [dBm x 10] */
    UINT32 bit_error = 0 ;  /* num of Pre-RS bit error */
    UINT32 period = 0 ;     /* period of Pre-BCH bit error measurement [num of bits]*/
    INT16 cn = 0 ;          /* received CN [dB x 100] */
    INT16 cnReq = 0;        /* required CN [dB x 100] */
    INT16 cnRel = 0 ;       /* CNrel [dB x 100] */
    INT16 ber_10log10 = 0;  /* 10xlog10(BER) x 100 */
    INT16 BER_SQI = 0 ;     /* BER_SQI x 100 */
    UINT16 SQI = 0 ;        /* SQI x 100 */
    UINT16 SSI = 0;         /* SSI x 10 */
    
    DBG_PRINT("dvbt_get_ssi_sqi\n");

    /* Get TPS info */
    dvbt_get_TPSInfo(&tps_info);

    /* Get PreRS BER */
    dvbt_get_PreRSBer(&bit_error, &period);
    
    /* Get CNR */
    dvbt_get_CNdBx100(&cn);
    
    /* Get RSSI */
    dvbt_get_rssi(&rssi );
    
    /* Get receiving prioirty */
    iic_write_val( demod_block_id,  0x00, 0x04 );
    iic_read( demod_block_id, 0x10, &prioirty, 1 );    /* bank=0x04, subadr=0x10 */
    iic_write_val( demod_block_id,  0x00, 0x00 );
    prioirty &= 0x01;

    /* rate of current receiving stream */
    if(prioirty == 0) {
        rate = tps_info.rateHP;
    } else {
        rate = tps_info.rateLP;
    }
    
    /* pick up required CN and reference level */
    if(tps_info.hierarchy == 0)  {
        /* non hierarchal */
        cnReq = DVBT_ReqiredCNdBx100[tps_info.constellation][rate];
        Pref  = DVBT_RefRSSIx10[tps_info.constellation][rate];
    } else {
        /* hierarchal */
        cnreq_index = (tps_info.hierarchy - 1) * 2 + tps_info.constellation - 1;
        if( cnreq_index > 5 ) {
            /* illegal parameter */
            cnReq =  10000; /* Error : Set Required-CN  100dB  */
            Pref  = -10000; /* Error : Set Required-CN -100dBm */
            return;
        } else {
            cnReq = DVBT_ReqiredCNdBx100_hier[cnreq_index][prioirty][rate];
            Pref  = DVBT_RefRSSIx10_hier[cnreq_index][prioirty][rate];
        }
    }

    /* CNrel calculation */
    cnRel = cn - cnReq;
    
    /* BER_SQI calculation */
    /*************************************************************************
        BER_SQI = 0                  if         BER >10E-3
        BER_SQI = 20*LOG10(1/BER)-40 if 10E-7 < BER <= 10E-3
        BER_SQI = 100                if         BER <= 10E-7
     *************************************************************************/
    ber_10log10 = calc_10log10_x100(bit_error) - calc_10log10_x100(period);
    
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
static void dvbt_get_CNdBx100(INT16* pCNx100 )
{
    UINT8  rdata[2] = {0};
    UINT16 CN_DATA = 0;

    iic_write_val( demod_block_id,  0x00, 0x00 );
    iic_read( demod_block_id, 0x2C, rdata, 2 );
    CN_DATA = (rdata[0] << 8) | rdata[1];

    *pCNx100 = calc_10log10_x100( CN_DATA ) - calc_10log10_x100( 32000 - CN_DATA ) + 3310;

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
static UINT32 calc_10log10_x100(UINT32 input)
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
    result_10log10x100    = (UINT32)i * 301 + (UINT32)log10x100_table[ 0x1F & s ];
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
 *****************************************************************************/
UINT16 CnCalc( UINT16 input )
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
    output = 3 * 4 * i + CnTable[ s ];
    return output;
}

/******************************************************************************
 * History
 *
 * 2010-03-24 v0.10 initial version
 *
 * 2010-05-21 v0.20 
 *                  Tuner setting was revised. (tuner_init,dvbt_tuner_tune,setRfTunerParam)
 *                  Tuning wait time was changed. (cmd_tune_dvbt,dvbt_tuner_tune)
 *                  Demod setting was revised. (CoreRegSetting_8MBW,CoreRegSetting_7MBW)
 *
 * 2010-06-10 v0.21 
 *                  Lock Sequence was updated.
 *                       (dvbt_lockseq_start,dvbt_lockseq_wait_tps_lock,dvbt_lockseq_wait_ts_lock)
 *
 * 2010-07-23 v0.30 
 *                  Descriptions of DE204 was removed.
 *
 * 2010-09-09 v1.00 
 *                  DVB-T tuner setting has been updated.(setRfTunerParam)
 *
 * 2010-10-05 v2.00 
 *                  I2C access functions has been revised.
 *
 * 2010-10-15 v2.10 
 *                  SSI/SQI calculation has been added.
 *
 ******************************************************************************/
