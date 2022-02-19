#include <sys_config.h>
#include <types.h>
#include <osal/osal.h>
#include <api/libc/alloc.h>
#include <api/libc/string.h>
#include <hal/hal_gpio.h>
#include <hld/hld_dev.h>
#include <hld/nim/nim_dev.h>
#include <hld/nim/nim.h>
#include <hld/nim/nim_tuner.h>
#include <bus/i2c/i2c.h>
#include <bus/tsi/tsi.h>
#include "si2165.h"

#if (SYS_DEM_MODULE == SI2165)
#define MAX_NIM    TUNER_NUMBER


#define SI2165_DEBUG_FLAG    0
#if(SI2165_DEBUG_FLAG)
#define printf soc_printf
#define NIM_SI2165_PRINTF    soc_printf//libc_printf
#else
#define printf(...)
#define NIM_SI2165_PRINTF(...)
#endif

#define APP_TUNER_OFDM_EN4020_SI2165 1
#define APP_TUNER_QAM_EN4020_SI2165 0
#define APP_TUNER_OFDM_MAX3580_SI2165 0

#if APP_TUNER_OFDM_EN4020_SI2165 || APP_TUNER_QAM_EN4020_SI2165
#include "en4020_tuner.h"
entr_config ctmp[MAX_NIM];

#endif

//#define TRACES
//#define RWTRACES

#define PAGE_ADDRESS        0xFF

/* Comment SILABS_I2C and Uncomment SERIT_I2C to swicth the communication method */
/* Using SILABS_I2C mode, you should be able to control our EVB                  */
#define SERIT_I2C




//#define COMMAND_LINE_APPLICATION

#define DIV_P                     pll_divp_prescaler_1
#define DEFAULT_DVB_RATE            9.1428
#define DEFAULT_FE_CLK             56.000
#define DEFAULT_DVB_RATE            9.1428
#define DEFAULT_BER_DEPTH        10000
#define DEMOD_ADDRESS             0xc8

#if APP_TUNER_OFDM_MAX3580_SI2165
#define CHIP_MODE                   chip_mode_pll_ext
#endif

#if APP_TUNER_OFDM_EN4020_SI2165
#define CHIP_MODE                   chip_mode_pll_xtal

#endif

#define REF_FREQUENCY               27
#define TUNER_XTAL_MHZ              REF_FREQUENCY
#define DIV_R                        2
#define DIV_N                       63
#define DIV_M                        7
#define AGC2_MIN                     105

#define AGC2_KACQ                   13
#define AGC2_KLOC                   16
#define KP_LOCK                       5
/* default KP_LOCK      5 */
#define CENTRAL_TAP                  10
/* default CENTRAL_TAP 10 */
#define TUNER_ADDRESS             0xc0


#if APP_TUNER_OFDM_MAX3580_SI2165
#define ZIF_MODE
#define ADC_SAMPLING_MODE          adc_sampling_mode_zif_ovr4
#define TUNER_IF_MHZ                   0

#define L1_RF_TUNER_Process              L1_RF_MAX3580_Process
#define L1_RF_TUNER_Init              L1_RF_MAX3580_Init
#define L1_RF_TUNER_InitAfterReset      L1_RF_MAX3580_InitAfterReset
#define L1_RF_TUNER_Get_Infos           L1_RF_MAX3580_Get_Infos
#define L1_RF_TUNER_Get_I2cChannel      L1_RF_MAX3580_Get_I2cChannel
#define L1_RF_TUNER_Get_IF              L1_RF_MAX3580_Get_IF
#define L1_RF_TUNER_Get_RF              L1_RF_MAX3580_Get_RF
#define L1_RF_TUNER_Get_minRF           L1_RF_MAX3580_Get_minRF
#define L1_RF_TUNER_Get_maxRF           L1_RF_MAX3580_Get_maxRF
#define L1_RF_TUNER_Tune              L1_RF_MAX3580_Tune
#define L1_RF_TUNER_Saw                  L1_RF_MAX3580_Saw
#define L1_RF_TUNER_Get_Saw              L1_RF_MAX3580_Get_Saw
#define L1_RF_TUNER_reference_frequency L1_RF_MAX3580_reference_frequency
#endif

#if APP_TUNER_OFDM_EN4020_SI2165 || APP_TUNER_QAM_EN4020_SI2165

#define IF_MODE
#define ADC_SAMPLING_MODE          adc_sampling_mode_if_ovr2
#define TUNER_IF_MHZ                   36

#endif



//L2_Context     *front_end[1];

static L1_Context *demod[MAX_NIM];
//static L1_Context *tuner[MAX_NIM];

#ifdef    SERIT_I2C
    #define semaphore_wait(x)
    #define semaphore_signal(x)

    #define i2cHandleTer DEMOD_ADDRESS
    #define i2cHandleTuner TUNER_ADDRESS
#endif /* SERIT_I2C */


//extern semaphore_t Sem_Si2165;

static ID NIM_I2C_SEMA_ID[MAX_NIM];

struct nim_si2165_private
{
    int nim_id;
    UINT32 i2c_id;
};

//////////////////////////////////////////////////////////////////////////////////////////
#define ST_ErrorCode_t UINT32
#define ST_NO_ERROR SUCCESS

INT32 STI2C_Read(UINT32 i2c_id, int slv_addr, UINT8 *Buffer_p, UINT32 MaxLen, UINT32 Timeout, UINT32 *ActLen_p)
{

    INT32      err = 0;
//    NIM_SI2165_PRINTF("STI2C_Read... %x, %x, %x\n", Handle, Buffer_p, MaxLen);

    osal_semaphore_capture(NIM_I2C_SEMA_ID[i2c_id & 0x01],TMO_FEVR);

    err = i2c_read(i2c_id ,slv_addr, Buffer_p, MaxLen);
    if(err != SUCCESS)
    {
        *ActLen_p = 0;
        NIM_SI2165_PRINTF("Error i2c R_r==> %d,i2c_id=%d\n",err,i2c_id);
    }else
    {
        *ActLen_p = MaxLen;
    }
    osal_task_sleep(1);
    osal_semaphore_release(NIM_I2C_SEMA_ID[i2c_id & 0x01]);
//    mdump(USER_SILVERK, "i2c R", Buffer_p, MaxLen);

    return( err );

}
#define BURST_PAIR_SZ   12
//////////////// use only for one byte register //////////////
INT32 i2c_write2(UINT32 id, int Handle, UINT8 *data, int len)
{
    UINT8 tmp[32];
    UINT32 reg_number, i,j, ii;
    UINT16 BurstPairs, RemainLen;
    INT32 err=0;

    RemainLen = (len-1) % BURST_PAIR_SZ;
    BurstPairs = (len-1) / BURST_PAIR_SZ;
    reg_number = data[0];

    for(i=0; i< BurstPairs; i++){
        tmp[0] = reg_number + i*(BURST_PAIR_SZ);
        for(j=0; j<BURST_PAIR_SZ; j++){
            tmp[1+j] = data[1+i*BURST_PAIR_SZ+j];
        }
        //mdump(USER_SILVERK, "w2-b", tmp, BURST_PAIR_SZ+1);
        err = i2c_write(id, Handle, tmp, BURST_PAIR_SZ+1);
        osal_task_sleep(1);
        if(err != SUCCESS)
        {
            NIM_SI2165_PRINTF("Error i2c W==> %d\n,i2c_id=%d\n",err,id);
        }
    }

    if(RemainLen > 0){
        tmp[0] = reg_number + i*(BURST_PAIR_SZ);
        for(j=0; j < RemainLen; j++){
            tmp[1+j] = data[1+i*BURST_PAIR_SZ+j];
        }
        //mdump(USER_SILVERK, "w2=r", tmp, RemainLen + 1);
        err = i2c_write(id, Handle, tmp, RemainLen + 1);
        if(err != SUCCESS)
        {
            NIM_SI2165_PRINTF("Error i2c W==> %d,i2c_id=%d\n",err,id);
        }
    }
    NIM_SI2165_PRINTF("SB\n");
    return err;
}
INT32 STI2C_Write(UINT32 i2c_id, int slv_addr, const UINT8 *Buffer_p, UINT32 NumberToWrite, UINT32 Timeout, UINT32 *ActLen_p)
{
    INT32          err = 0;
//    NIM_SI2165_PRINTF("STI2C_Write... %x, %x, %x\n", Handle, Buffer_p, NumberToWrite);

//    mdump(USER_SILVERK, "i2c W", Buffer_p, NumberToWrite);
    osal_semaphore_capture(NIM_I2C_SEMA_ID[i2c_id & 0x01],TMO_FEVR);
    if(NumberToWrite > BURST_PAIR_SZ){
        err = i2c_write2(i2c_id, slv_addr, Buffer_p, NumberToWrite);
    }
    else
        err = i2c_write(i2c_id, slv_addr, Buffer_p, NumberToWrite);
    if(err != SUCCESS)
    {
        NIM_SI2165_PRINTF("Error i2c W==> %d,i2c_id=%d\n",err,i2c_id);
    }
    //NIM_SI2165_PRINTF("MB\n");
    osal_semaphore_release(NIM_I2C_SEMA_ID[i2c_id & 0x01]);
    return( err );
 }

/* ============================================================================*/
#ifdef    SERIT_I2C
int Si2165_SetReg(UINT32 i2c_id, unsigned int addr,unsigned char *data, int byteCount)
{
    static UINT8 io_data[60];
    UINT8 i;
    UINT8 *dataptr;
    int status;
    UINT32 Actlen;
    ST_ErrorCode_t ErrCode=ST_NO_ERROR;


    /*  Expected traffic on the I2C lines: DEMOD_ADDRESS addr_msb addr_lsb data_byte[0] ... data_byte[byteCount-1] */

    status = 1;


    io_data[0] = (UINT8)((addr&0xff00)>>8);
    io_data[1] = (UINT8)((addr&0x00ff)>>0);

    for(i=0;i<byteCount;i++) {
        io_data[i+2]=(UINT8)data[i];
    }

    semaphore_wait(&FRONT_I2C_ACCESS);
    ErrCode = STI2C_Write(i2c_id, i2cHandleTer, io_data, 2+byteCount, 1000, &Actlen);// master i2c address 0xc8 include STI2C_Write

    if(ErrCode != ST_NO_ERROR)
    {
        status = 0;
        printf("[Si2165_SetReg]  I2C Write Error = 0x%x\n", ErrCode);
    }
    semaphore_signal(&FRONT_I2C_ACCESS);
    return Actlen;
}


#if APP_TUNER_OFDM_MAX3580_SI2165
int Max3580_SetReg(unsigned char addr,unsigned char *data, int byteCount)
{
    static UINT8 io_data[60];
    UINT8 i;
    UINT8 *dataptr;
    int status;
    UINT32 Actlen;
    unsigned char temp=0;


    ST_ErrorCode_t ErrCode=ST_NO_ERROR;;

    status = 1;

    io_data[0] = (UINT8)addr;

    for(i=0;i<byteCount;++i)
    {
        io_data[i+1]=(UINT8)*(data++);
        printf(" 0x%x", io_data[i+1]);

    }
    printf("\n");


    temp = 0x01;
    Si2165_SetReg(0x01,&temp,1);

    semaphore_wait(&FRONT_I2C_ACCESS);
    ErrCode = STI2C_Write(i2cHandleTuner, io_data, 1+byteCount, 1000, &Actlen);
    if(ErrCode != ST_NO_ERROR)
    {
        status = 0;
        printf("[Max3580_SetReg]  I2C Write Error = 0x%x\n", ErrCode);
    }
    semaphore_signal(&FRONT_I2C_ACCESS);

    temp = 0x00;
    Si2165_SetReg(0x01,&temp,1);

    return Actlen;
}
#endif

#endif /* SERIT_I2C */


#ifdef    SERIT_I2C
int Si2165_GetReg(UINT32 i2c_id, unsigned int addr,unsigned char *data, int byteCount)
{
    static UINT8 io_data[60];
    UINT8 i;
    UINT8 *dataptr;
    int status;
    ST_ErrorCode_t ErrCode=ST_NO_ERROR;
    UINT32 Actlen;

    status = 1;
    semaphore_wait(&FRONT_I2C_ACCESS);
    /*  Expected traffic on the I2C lines: DEMOD_ADDRESS addr_msb addr_lsb */

    io_data[0] = (UINT8)((addr&0xff00)>>8);
    io_data[1] = (UINT8)((addr&0x00ff)>>0);

    ErrCode = STI2C_Write(i2c_id, i2cHandleTer, io_data, 2, 1000, &Actlen); // master i2c address 0xc8 include STI2C_Write
    if(ErrCode != ST_NO_ERROR)
    {

        printf("[Si2165_GetReg]  Step 1:  I2C Write Errorr = 0x%x\n", ErrCode);
        status = 0;
    }
    else
    {
    /*  Expected traffic on the I2C lines: DEMOD_ADDRESS+1 data[0] ... data[byteCount-1]  (data bytes send by Si2165 followed by ACK until end) */
     // master i2c address 0xc8+1 include STI2C_Read
            ErrCode = STI2C_Read(i2c_id, i2cHandleTer, (UINT8 *)data, byteCount, 1000, &Actlen);
            if(ErrCode != ST_NO_ERROR)
            {
                  printf("[Si2165_GetReg] Step 2:  I2C Read Error = 0x%x\n", ErrCode);
            status = 0;
        }
    }
    semaphore_signal(&FRONT_I2C_ACCESS);
    return Actlen;
}

#if APP_TUNER_OFDM_MAX3580_SI2165
int Max3580_GetReg( unsigned char addr,unsigned char *data, int byteCount)
{
    static UINT8 io_data[60];
    UINT8 i;
    UINT8 *dataptr;
    int status;
    ST_ErrorCode_t ErrCode;
    UINT32 Actlen;
    unsigned char temp=0;

    status = 1;

    temp = 0x01;
    Si2165_SetReg(0x01,&temp,1);
    semaphore_wait(&FRONT_I2C_ACCESS);

    ErrCode = STI2C_WriteNoStop(i2cHandleTuner, &addr, 1, 1000, &Actlen);
    if(ErrCode != ST_NO_ERROR)
    {

        printf("[Max3580_GetReg]  Step 1:  I2C Write Errorr = 0x%x\n", ErrCode);
        status = 0;
    }
    else
    {
            ErrCode = STI2C_Read(i2cHandleTuner, (UINT8 *)data, byteCount, 1000, &Actlen);
            if(ErrCode != ST_NO_ERROR)
            {
                  printf("[Max3580_GetReg] Step 2:  I2C Read Error = 0x%x\n", ErrCode);
            status = 0;
        }
    }
    semaphore_signal(&FRONT_I2C_ACCESS);

    temp = 0x00;
    Si2165_SetReg(0x01,&temp,1);

    return Actlen;
}
#endif


#endif /* SERIT_I2C */
#if APP_TUNER_OFDM_EN4020_SI2165 || APP_TUNER_QAM_EN4020_SI2165
int system_wait(int time_ms) {
    task_delay(time_ms);
    return time_ms;
/*    clock_t ticks1, ticks2;
    ticks1=clock();
    ticks2=ticks1;
    while((ticks2*1000/CLOCKS_PER_SEC-ticks1*1000/CLOCKS_PER_SEC)<time_ms) {ticks2=clock();}

    return (int)ticks2*1000/CLOCKS_PER_SEC;
    */
}

ST_ErrorCode_t RegGetRegistersTuner (UINT32 i2c_id, unsigned char reg_addr, unsigned char cnt,unsigned char *getdata)
{

    unsigned char temp=0;
    ST_ErrorCode_t ErrCode = ST_NO_ERROR;
    static unsigned char     data;
    UINT32 Actlen;

    data = reg_addr;
    temp = 0x01;
    Si2165_SetReg(i2c_id, 0x01,&temp,1);
    task_delay(5);
    semaphore_wait(&FRONT_I2C_ACCESS);
    ErrCode = STI2C_Write(i2c_id, i2cHandleTuner, &data, 1, 1000, &Actlen);
    semaphore_signal(&FRONT_I2C_ACCESS);

    temp = 0x00;
    Si2165_SetReg(i2c_id, 0x01,&temp,1);
    task_delay(5);
    if(ErrCode != ST_NO_ERROR) {
            printf("\nError(0x%x) on RegGetOneRegisterTuner(0x%02x) write    data=0x%02x\n",ErrCode, reg_addr, data);
    }
    else {
        temp = 0x01;
        Si2165_SetReg(i2c_id, 0x01,&temp,1);
        task_delay(5);
        semaphore_wait(&FRONT_I2C_ACCESS);
        ErrCode = STI2C_Read(i2c_id, i2cHandleTuner, getdata, cnt, 1000, &Actlen);
        semaphore_signal(&FRONT_I2C_ACCESS);

        temp = 0x00;
        Si2165_SetReg(i2c_id, 0x01,&temp,1);
        task_delay(5);
        if(ErrCode != ST_NO_ERROR)
            printf("\nError(0x%x) on RegGetOneRegisterTuner(0x%02x) read  data=0x%02x\n",ErrCode, reg_addr, data);
    }
    return ErrCode;

}


 ST_ErrorCode_t RegSetRegistersTuner(UINT32 i2c_id, unsigned char reg_addr, unsigned char cnt,unsigned char *data)
{
    ST_ErrorCode_t ErrCode;
    UINT32 Actlen;

    static UINT8 DataBuf[256];
    int retry;
    int i;
    unsigned char temp=0;

    ErrCode = ST_NO_ERROR;
    DataBuf[0] = reg_addr;


    for (i=0; i < cnt; i++)
    DataBuf[i+1] = *(data+i);

    temp = 0x01;
    Si2165_SetReg(i2c_id, 0x01,&temp,1);
    task_delay(5);
    semaphore_wait(&FRONT_I2C_ACCESS);
    ErrCode = STI2C_Write(i2c_id, i2cHandleTuner, DataBuf, cnt+1, 1000, &Actlen);
    semaphore_signal(&FRONT_I2C_ACCESS);

    temp = 0x00;
    Si2165_SetReg(i2c_id, 0x01,&temp,1);
    task_delay(5);

        if(ErrCode != ST_NO_ERROR)
            printf("\nRegSetOneRegisterTuner:ERROR1>  (Reg = 0x%02x, Len = 0x%02x)\n",  DataBuf[0], cnt);

    return ErrCode;
}

ENTR_return entr_user_2wb_wr(UINT32 i2c_id, ENTR_uint8 b2wAdx, ENTR_uint8 icReg, ENTR_uint8 *data, ENTR_uint8 byteCnt)
{

    ST_ErrorCode_t ErrCode;
    UINT32 Actlen;

    static UINT8 DataBuf[256];
    int retry;
    int i;
    unsigned char temp=0;

    ErrCode = ST_NO_ERROR;
    DataBuf[0] = icReg;


    for (i=0; i < byteCnt; i++)
    DataBuf[i+1] = *(data+i);

    temp = 0x01;
    Si2165_SetReg(i2c_id, 0x01,&temp,1);

    task_delay(5);
    semaphore_wait(&FRONT_I2C_ACCESS);
    ErrCode = STI2C_Write(i2c_id, b2wAdx, DataBuf, byteCnt+1, 1000, &Actlen);
    semaphore_signal(&FRONT_I2C_ACCESS);

    temp = 0x00;
    Si2165_SetReg(i2c_id, 0x01,&temp,1);

    task_delay(5);
    if(ErrCode != ST_NO_ERROR)
        printf("\nRegSetOneRegisterTuner:ERROR>  (Reg = 0x%02x, Len = 0x%02x)\n",  DataBuf[0], byteCnt);

    if(ErrCode != ST_NO_ERROR)
        return ENTR_err_rw;
    else
        return ENTR_ok;
}

ENTR_return entr_user_2wb_rd(UINT32 i2c_id, ENTR_uint8 b2wAdx, ENTR_uint8 icReg, ENTR_uint8 *data, ENTR_uint8 byteCnt)
{

        ST_ErrorCode_t ErrCode;
        //static unsigned char    data;
        UINT32 Actlen;
        static UINT8 DataBuf[256];
        unsigned char temp=0;

        //return;

        temp = 0x01;
        Si2165_SetReg(i2c_id, 0x01,&temp,1);
        task_delay(5);
        semaphore_wait(&FRONT_I2C_ACCESS);
        ErrCode = STI2C_Write(i2c_id, b2wAdx, &icReg, 1, 1000, &Actlen);
        semaphore_signal(&FRONT_I2C_ACCESS);

        temp = 0x00;
        Si2165_SetReg(i2c_id, 0x01,&temp,1);
        task_delay(5);
        if(ErrCode != ST_NO_ERROR) {
                printf("\nError(0x%x) on RegGetOneRegisterTuner(0x%02x) write    data=0x%02x\n",ErrCode, icReg, data);
    }
    else {
            temp = 0x01;
            Si2165_SetReg(i2c_id, 0x01,&temp,1);
            task_delay(5);
            semaphore_wait(&FRONT_I2C_ACCESS);
            ErrCode = STI2C_Read(i2c_id, b2wAdx,DataBuf, byteCnt, 1000, &Actlen);
            semaphore_signal(&FRONT_I2C_ACCESS);

            temp = 0x00;
            Si2165_SetReg(i2c_id, 0x01,&temp,1);


            MEMCPY(data,DataBuf,byteCnt);

            task_delay(5);
            if(ErrCode != ST_NO_ERROR)
                printf("\nError(0x%x) on RegGetOneRegisterTuner(0x%02x) read  data=0x%02x\n",ErrCode, icReg, data);
        }

    if(ErrCode != ST_NO_ERROR)
        return ENTR_err_rw;
    else
        return ENTR_ok;
}
#endif

/************************************************************************************************************************
  ReadRegister function
  Use:        register read function
              Used to read a register based on its address, size, offset
  Parameter:  add              register address
  Parameter:  offset           register offset
  Parameter:  nbbit            register size in bits
  Parameter:  isSigned         register sign info
  Behavior:          This function uses the characteristics of the register to read its value from hardware
                       it reads the minimum number of bytes required to retrieve the register based on the number of bits and the offset
                       it masks all bits not belonging to the register before returning
                       it handles the sign-bit propagation for signed registers
  Returns: the value read. O if error during the read
************************************************************************************************************************/
EMB_L1_DRIVER_CORE int  SLAB_WINAPI L1_ReadRegister      (L1_Context *context, unsigned int add, unsigned char offset, unsigned char nbbit, unsigned int isSigned) {
    int iNbBytes, i, nbReadBytes, iNbPrefixBitsToCancel;
    int iVal;
    unsigned char  prefixByte, postfixByte;
    unsigned char* pbtDataBuffer;
    iNbBytes = (nbbit + offset + 7)/8;
    pbtDataBuffer    = (unsigned char*)malloc(iNbBytes*sizeof(unsigned char));
    iVal = 0;

    /* <Porting> <modification required> replace the L0_ReadBytes call by the byte-level Read function of your usual connection layer */
    nbReadBytes = L1_ReadBytes(context    , add, iNbBytes, &pbtDataBuffer);

    //for(i=0;i<nbReadBytes;i++)
    //{
    //    printf("[0x%02x]\n",pbtDataBuffer[i]);
    //}

    if (nbReadBytes != iNbBytes) {
        //printf("[L1_ReadRegister] iNbBytes = %d nbReadBytes = %d \n",iNbBytes,nbReadBytes);
        free(pbtDataBuffer);
        return 0;
    }

    for (i = 0; i < iNbBytes; i++) {iVal = iVal + ( pbtDataBuffer[i] << (8*(i)));}

    prefixByte  = pbtDataBuffer[iNbBytes - 1];
    iNbPrefixBitsToCancel = 8*(iNbBytes) - nbbit - offset;
    prefixByte  = prefixByte  >> (8 - iNbPrefixBitsToCancel);
    prefixByte  = prefixByte  << (8 - iNbPrefixBitsToCancel);
    postfixByte = pbtDataBuffer[iNbBytes-1];
    postfixByte = postfixByte << (8 - offset);
    postfixByte = postfixByte >> (8 - offset);

    iVal = -( prefixByte << (8*(iNbBytes - 1))) + ( iVal ) - postfixByte ;

    /* sign bit propagation, if required */
    if (isSigned) {
        iVal = (signed long)iVal >> offset;
        iVal = (signed long)iVal << (32-nbbit);
        iVal = (signed long)iVal >> (32-nbbit);
    } else {
        iVal = (unsigned long)iVal >> offset;
    }
    free(pbtDataBuffer);

    //printf("[L1_ReadRegister] iNbBytes = %d nbReadBytes = %d iVal = 0x%x\n",iNbBytes,nbReadBytes,iVal);
    return iVal;
}

/************************************************************************************************************************
  L1_ReadRegisterTrace function
  Use:         trace and read function
               Used to read a register based on its address, size, offset, with traces
  Parameter:    add              register address
  Parameter:    offset           register offset
  Parameter:    nbbit            register size in bits
  Behavior:          This function traces the register information, then calls L1_ReadRegister with all the register parameters
                     It is used only when tracing register reads is required
  Comments: generally activated by changing the definition of the CONTEXT_READ function in the proper header file
  Returns: the value written. O if error during the write
************************************************************************************************************************/
EMB_L1_DRIVER_CORE int  SLAB_WINAPI L1_ReadRegisterTrace (L1_Context *context, char* name, unsigned int add, unsigned char offset, unsigned char nbbit, unsigned int isSigned) {
    int res;
    int i;
    unsigned int iNbBytes;


    res = L1_ReadRegister (context, add, offset, nbbit, isSigned);
    iNbBytes = (nbbit + offset + 7)/8;
    printf(" << READ  %-20s                           : (0x%04x)", name, (int)res);
    printf("  @[0x%0x:%d", add+iNbBytes-1, (offset+nbbit-1)%8);
    if (iNbBytes == 1) {
       if (nbbit ==1) {
           printf("]\n");
       } else {
           printf("-%d]\n", offset);
       }
    } else {
       printf("]-[0x%0x:%d]\n", add, offset);
    }
    return res;
}
/************************************************************************************************************************
  L1_WriteRegister function
  Use:        register write function
              Used to write a register based on its address, size, offset
  Parameter:  add              register address
  Parameter:  offset           register offset
  Parameter:  nbbit            register size in bits
  Parameter:  alone            register loneliness info (1 if all other bits can be overwritten without checking)
  Parameter:  Value            the required value for the register
  Behavior:          This function uses all characteristics of the register to write its value in the hardware
                       if the required value is out of range, no operation is performed and an error code is returned
                       before writing, the current register value is retrieved (if required because of adjacent registers),
                          in order to preserve the contain of adjacent registers
                       it reads the minimum number of bytes required to retrieve the register based on:
                          the number of bits and the offset
                       it keeps all bits not belonging to the register intact
                       it handles the sign-bit propagation for signed registers
  Returns: the value written. O if error during the write
************************************************************************************************************************/
EMB_L1_DRIVER_CORE int  SLAB_WINAPI L1_WriteRegister    (L1_Context *context, unsigned int add, unsigned char offset, unsigned char nbbit, unsigned char alone, double Value) {
    unsigned int iNbBytes, iVal, i,nbWrittenBytes, iNbPrefixBitsToCancel, nbReadBytes;
    unsigned char prefixByte, postfixByte;
    unsigned char* pbtDataBuffer;
    iNbBytes = (nbbit + offset + 7)/8;
    pbtDataBuffer    = (unsigned char*)malloc(iNbBytes*sizeof(unsigned char));
      iNbPrefixBitsToCancel = 8*(iNbBytes) - nbbit - offset;
    /* removing sign-propagation bits for negative values of signed registers */
    Value = (unsigned long)Value << iNbPrefixBitsToCancel;
    Value = (unsigned long)Value >> iNbPrefixBitsToCancel;

    if (((iNbBytes*8 == nbbit) && (offset == 0)) || ((alone == 1))) {
        iVal = (unsigned long)Value << offset;
    } else {
        nbReadBytes = L1_ReadBytes(context    , add, iNbBytes, &pbtDataBuffer);
        if (nbReadBytes != iNbBytes) {
            free(pbtDataBuffer);
            return 0;
        }
        prefixByte  = pbtDataBuffer[iNbBytes-1];
        prefixByte  = prefixByte  >> (8 - iNbPrefixBitsToCancel);
        prefixByte  = prefixByte  << (8 - iNbPrefixBitsToCancel);

        postfixByte = pbtDataBuffer[0];
        postfixByte = postfixByte << (8 - offset);
        postfixByte = postfixByte >> (8 - offset);

        iVal = ( prefixByte << (8*(iNbBytes - 1))) + ( (unsigned long)Value << offset ) + postfixByte ;
    }

    for (i = 0 ; i < iNbBytes; i++ ) {pbtDataBuffer[i] = (BYTE)(iVal >> 8*(i));}

    nbWrittenBytes = L1_WriteBytes(context, add, iNbBytes, pbtDataBuffer);
    free(pbtDataBuffer);
    if (nbWrittenBytes != iNbBytes) return 0;
    return Value;
}

/************************************************************************************************************************
  L1_WriteRegisterTrace function
  Use:        register write and trace function
              Used to write a register based on its address, size, offset, with traces
  Parameter:  add              register address
  Parameter:  offset           register offset
  Parameter:  nbbit            register size in bits
  Parameter:  alone            register loneliness info (1 if all other bits can be overwritten without checking)
  Parameter:  Value            the required value for the register
  Behavior:          This function traces the register information, then calls L1_WriteRegister with all the register parameters
                     It is used only when tracing register writes is required
  Comments: generally activated by changing the definition of the CONTEXT_WRITE function in the proper header file
  Returns: the value written. O if error during the write
************************************************************************************************************************/
EMB_L1_DRIVER_CORE int  SLAB_WINAPI L1_WriteRegisterTrace (L1_Context *context, char* name, char* valtxt, unsigned int add, unsigned char offset, unsigned char nbbit, unsigned char alone, double Value) {
    int i;
    unsigned int iNbBytes;
    int res;
    iNbBytes = (nbbit + offset + 7)/8;
    printf(" >> WRITE %-20s %-25s : (0x%04x)", name, valtxt, (int)Value);
    printf("  @[0x%0x:%d", add+iNbBytes-1, (offset+nbbit-1)%8);
    if (iNbBytes == 1) {
        if (nbbit ==1) {
            printf("]\n");
        } else {
            printf("-%d]\n", offset);
        }
    } else {
        printf("]-[0x%0x:%d]\n", add, offset);
    }
    res = L1_WriteRegister (context, add, offset, nbbit, alone, Value);
    return res;
}
EMB_L1_DRIVER_CORE double  SLAB_WINAPI L1_ReadRegister8Bits      (L1_Context *context, unsigned int add, unsigned char offset, unsigned char nbbit, unsigned int isSigned) {
    int iNbBytes, i, nbReadBytes, iNbPrefixBitsToCancel;
    double iVal;
    unsigned char  prefixByte, postfixByte;
    unsigned char* pbtDataBuffer;
    iNbBytes = (nbbit + offset + 7)/8;
    pbtDataBuffer    = (unsigned char*)malloc(iNbBytes*sizeof(unsigned char));
    iVal = 0;

    /* <Porting> <modification required> replace the L0_ReadBytes call by the byte-level Read function of your usual connection layer */
    nbReadBytes = L1_ReadBytes8Bits(context    , add, iNbBytes, &pbtDataBuffer);

    if (nbReadBytes != iNbBytes) {
        free(pbtDataBuffer);
        return 0;
    }

    for (i = 0; i < iNbBytes; i++) {iVal = iVal + ( pbtDataBuffer[i] << (8*(i)));}

    prefixByte  = pbtDataBuffer[iNbBytes - 1];
    iNbPrefixBitsToCancel = 8*(iNbBytes) - nbbit - offset;
    prefixByte  = prefixByte  >> (8 - iNbPrefixBitsToCancel);
    prefixByte  = prefixByte  << (8 - iNbPrefixBitsToCancel);
    postfixByte = pbtDataBuffer[iNbBytes-1];
    postfixByte = postfixByte << (8 - offset);
    postfixByte = postfixByte >> (8 - offset);

    iVal = -( prefixByte << (8*(iNbBytes - 1))) + ( iVal ) - postfixByte ;

    /* sign bit propagation, if required */
    if (isSigned) {
        iVal = (signed long)iVal >> offset;
        iVal = (signed long)iVal << (32-nbbit);
        iVal = (signed long)iVal >> (32-nbbit);
    } else {
        iVal = (unsigned long)iVal >> offset;
    }

    free(pbtDataBuffer);
    return iVal;
}

/************************************************************************************************************************
  L1_ReadRegisterTrace8Bits function
  Use:         trace and read function
               Used to read a register based on its address, size, offset, with traces
  Parameter:    add              register address
  Parameter:    offset           register offset
  Parameter:    nbbit            register size in bits
  Behavior:          This function traces the register information, then calls L1_ReadRegister with all the register parameters
                     It is used only when tracing register reads is required
  Comments: generally activated by changing the definition of the CONTEXT_READ function in the proper header file
  Returns: the value written. O if error during the write
************************************************************************************************************************/
EMB_L1_DRIVER_CORE double  SLAB_WINAPI L1_ReadRegisterTrace8Bits (L1_Context *context, char* name, unsigned int add, unsigned char offset, unsigned char nbbit, unsigned int isSigned) {
    double res = L1_ReadRegister8Bits (context, add, offset, nbbit, isSigned);
    unsigned int iNbBytes;
    iNbBytes = (nbbit + offset + 7)/8;
    printf(" << READ  8 bits %-20s                           : %4.0f (0x%04x)", name, res, (int)res);
    printf("  @[0x%04x:%d", add+iNbBytes-1, (offset+nbbit-1)%8);
    if (iNbBytes == 1) {
       if (nbbit ==1) {
           printf("]\n");
       } else {
           printf("-%d]\n", offset);
       }
    } else {
       printf("]-[0x%04x:%d]\n", add, offset);
    }
    return res;
}
/************************************************************************************************************************
  L1_WriteRegister8Bits function
  Use:        register write function
              Used to write a register based on its address, size, offset
  Parameter:  add              register address
  Parameter:  offset           register offset
  Parameter:  nbbit            register size in bits
  Parameter:  alone            register loneliness info (1 if all other bits can be overwritten without checking)
  Parameter:  Value            the required value for the register
  Behavior:          This function uses all characteristics of the register to write its value in the hardware
                       if the required value is out of range, no operation is performed and an error code is returned
                       before writing, the current register value is retrieved (if required because of adjacent registers),
                          in order to preserve the contain of adjacent registers
                       it reads the minimum number of bytes required to retrieve the register based on:
                          the number of bits and the offset
                       it keeps all bits not belonging to the register intact
                       it handles the sign-bit propagation for signed registers
  Returns: the value written. O if error during the write
************************************************************************************************************************/
EMB_L1_DRIVER_CORE double  SLAB_WINAPI L1_WriteRegister8Bits    (L1_Context *context, unsigned int add, unsigned char offset, unsigned char nbbit, unsigned char alone, double Value) {
    unsigned int iNbBytes, iVal, i,nbWrittenBytes, iNbPrefixBitsToCancel, nbReadBytes;
    unsigned char prefixByte, postfixByte;
    unsigned char* pbtDataBuffer;
    iNbBytes = (nbbit + offset + 7)/8;
    pbtDataBuffer    = (unsigned char*)malloc(iNbBytes*sizeof(unsigned char));
      iNbPrefixBitsToCancel = 8*(iNbBytes) - nbbit - offset;
    /* removing sign-propagation bits for negative values of signed registers */
    Value = (unsigned long)Value << iNbPrefixBitsToCancel;
    Value = (unsigned long)Value >> iNbPrefixBitsToCancel;

    if (((iNbBytes*8 == nbbit) && (offset == 0)) || ((alone == 1))) {
        iVal = (unsigned long)Value << offset;
    } else {
        nbReadBytes = L1_ReadBytes8Bits(context    , add, iNbBytes, &pbtDataBuffer);
        if (nbReadBytes != iNbBytes) {
            free(pbtDataBuffer);
            return 0;
        }
        prefixByte  = pbtDataBuffer[iNbBytes-1];
        prefixByte  = prefixByte  >> (8 - iNbPrefixBitsToCancel);
        prefixByte  = prefixByte  << (8 - iNbPrefixBitsToCancel);

        postfixByte = pbtDataBuffer[0];
        postfixByte = postfixByte << (8 - offset);
        postfixByte = postfixByte >> (8 - offset);

        iVal = ( prefixByte << (8*(iNbBytes - 1))) + ( (unsigned long)Value << offset ) + postfixByte ;
    }

    for (i = 0 ; i < iNbBytes; i++ ) {pbtDataBuffer[i] = (BYTE)(iVal >> 8*(i));}

    nbWrittenBytes = L1_WriteBytes8Bits(context, add, iNbBytes, pbtDataBuffer);
    free(pbtDataBuffer);
    if (nbWrittenBytes != iNbBytes) return 0;
    return Value;
}

/************************************************************************************************************************
  L1_WriteRegisterTrace8Bits function
  Use:        register write and trace function
              Used to write a register based on its address, size, offset, with traces
  Parameter:  add              register address
  Parameter:  offset           register offset
  Parameter:  nbbit            register size in bits
  Parameter:  alone            register loneliness info (1 if all other bits can be overwritten without checking)
  Parameter:  Value            the required value for the register
  Behavior:          This function traces the register information, then calls L1_WriteRegister with all the register parameters
                     It is used only when tracing register writes is required
  Comments: generally activated by changing the definition of the CONTEXT_WRITE function in the proper header file
  Returns: the value written. O if error during the write
************************************************************************************************************************/
EMB_L1_DRIVER_CORE double  SLAB_WINAPI L1_WriteRegisterTrace8Bits (L1_Context *context, char* name, char* valtxt, unsigned int add, unsigned char offset, unsigned char nbbit, unsigned char alone, double Value) {
    unsigned int iNbBytes;
    double res;
    iNbBytes = (nbbit + offset + 7)/8;
    res = L1_WriteRegister8Bits (context, add, offset, nbbit, alone, Value);
    printf(" >> WRITE 8 bits %-20s %-25s : %4.0f (0x%04x)", name, valtxt, Value, (int)Value);
    printf("  @[0x%04x:%d", add+iNbBytes-1, (offset+nbbit-1)%8);
    if (iNbBytes == 1) {
        if (nbbit ==1) {
            printf("]\n");
        } else {
            printf("-%d]\n", offset);
        }
    } else {
        printf("]-[0x%04x:%d]\n", add, offset);
    }
    return res;
}

#ifdef __cplusplus
}
#endif

/**************************************************************************************************
  L1_ReadBytes8Bits function
  Use:  Layer 1 bytes read function
        Used to read bytes while taking into account the L0 limitation to 8 bits
  Parameter:    iI2CIndex        the start index for the read
  Parameter:    iNbBytes         the number of bytes to read
  Parameter:    pbtDataBuffer    a buffer to store the bytes read
  Behavior:          This function reads in 8 bits mode from the L0
                      if the current page value is different from the requested one, the page
regiter is set
                       then it reads the bytes using only the LSB byte of iI2CIndex
  Returns: 0 if error, otherwise the number of bytes read
**************************************************************************************************/
EMB_L1_DRIVER_CORE int     SLAB_WINAPI  L1_ReadBytes8Bits  (L1_Context* context, unsigned int iI2CIndex, int iNbBytes, BYTE **pbtDataBuffer) {
    unsigned char  pPageBuffer[1];
    unsigned char  requestedPage;
    requestedPage = (iI2CIndex & 0xFF00) >> 8;
    if (requestedPage != context->currentPage) {
      pPageBuffer[0] = requestedPage;
      context->currentPage = requestedPage;
      printf("changing to page 0x%02x\n",requestedPage);
#ifdef    SERIT_I2C
           L0_WriteBytes (context->i2c/*->address*/,  PAGE_ADDRESS, 1,   pPageBuffer);
#endif /* SERIT_I2C */
#ifdef    SILABS_I2C
           L0_WriteBytes(context->i2c, PAGE_ADDRESS,   1,        pPageBuffer);
#endif /* SILABS_I2C */
    }
#ifdef    SERIT_I2C
    return L0_ReadBytes  (context->i2c/*->address*/, iI2CIndex&0xff, iNbBytes, *pbtDataBuffer);
#endif /* SERIT_I2C */
#ifdef    SILABS_I2C
    return L0_ReadBytes(context->i2c, iI2CIndex&0xff, iNbBytes, pbtDataBuffer);
#endif /* SILABS_I2C */
}

/**************************************************************************************************
  L1_WriteBytes8Bits function
  Use:          Byte writing function taking into account the L0 limitation to 8 bit
                Used to manage page index if the bus is limited to 8 bits addressing
  Parameter:    iI2CIndex        the start index for the read
  Parameter:    iNbBytes         the number of bytes to read
  Parameter:    pbtDataBuffer    a buffer containing the bytes to write
  Behavior:     This function writes in 8 bits mode to the L0
                  if the current page value is different from the requested one, the page regiter
is set
                  then it writes the bytes using only the LSB byte of iI2CIndex
  Returns: 0 if error, otherwise the number of bytes written

***************************************************************************************************/
EMB_L1_DRIVER_CORE int     SLAB_WINAPI  L1_WriteBytes8Bits  (L1_Context* context, unsigned int iI2CIndex, int iNbBytes, BYTE *pbtDataBuffer) {
    unsigned char  pPageBuffer[1];
    unsigned char  requestedPage;
    requestedPage = (iI2CIndex & 0xFF00) >> 8;
    if (requestedPage != context->currentPage) {
      pPageBuffer[0] = requestedPage;
      context->currentPage = requestedPage;
      printf("changing to page 0x%02x\n",requestedPage);
#ifdef    SERIT_I2C
           L0_WriteBytes (context->i2c/*->address*/, PAGE_ADDRESS, 1,   pPageBuffer);
#endif /* SERIT_I2C */
#ifdef    SILABS_I2C
           L0_WriteBytes(context->i2c, PAGE_ADDRESS,   1,        pPageBuffer);
#endif /* SILABS_I2C */
    }
#ifdef    SERIT_I2C
    return L0_WriteBytes(context->i2c/*->address*/, iI2CIndex&0xff, iNbBytes, pbtDataBuffer);
#endif /* SERIT_I2C */
#ifdef    SILABS_I2C
    return L0_WriteBytes(context->i2c, iI2CIndex&0xff, iNbBytes, pbtDataBuffer);
#endif /* SILABS_I2C */
}

void store(char **dest, char *str) {
    *dest = (char*) malloc(sizeof(char)*(strlen(str)+1));
    strcpy(*dest,str);
}

bool L0_Connect (L0_Context* i2c, CONNECTION_TYPE connType) {
    PRINTF("L0_Connect: attempting to connect in mode %d context 0x%x\n",connType,i2c);
    #ifdef USB_Capability
    if (i2c->connectionType == USB) {
       i2c->usbOpen = 0;
       store(&(i2c->interfaceType), "unknown");
    }
    #endif /* USB_Capability */
    switch (connType) {
    #ifdef USB_Capability
        case USB:
            if (i2c->usbOpen == 0) {
               Cypress_USB_Open();
            }
            i2c->connectionType = USB;
            store(&(i2c->interfaceType),"USB");
            return 1;
            break;
    #endif /* USB_Capability */
    #ifdef SERIT_I2C
        case CUSTOMER:
            i2c->connectionType = CUSTOMER;
            store(&(i2c->interfaceType),"CUSTOMER");
            return 1;
            break;
    #endif /* SERIT_I2C */
    case SIMU:
        i2c->connectionType = SIMU;
        store(&(i2c->interfaceType),"SIMU");
        return 1;
        break;
    default:
        return 0;
        break;
    };
}

#ifdef SERIT_I2C
EMB_L0_API int SLAB_WINAPI SERIT_WriteBytes(L0_Context* i2c, unsigned int iI2CIndex, int iNbBytes, BYTE* pbtDataBuffer) {
    int nbWriteBytes;
    if(i2c->address == DEMOD_ADDRESS)
        nbWriteBytes = Si2165_SetReg(i2c->i2c_id, iI2CIndex,pbtDataBuffer,iNbBytes);
    else if(i2c->address == TUNER_ADDRESS)
#if APP_TUNER_OFDM_MAX3580_SI2165
        nbWriteBytes = Max3580_SetReg(iI2CIndex,pbtDataBuffer,iNbBytes);
#endif
#if APP_TUNER_OFDM_EN4020_SI2165
;
#endif
    return nbWriteBytes;
}
#endif /* SERIT_I2C */


#ifdef SERIT_I2C
EMB_L0_API int SLAB_WINAPI SERIT_ReadBytes (L0_Context* i2c, unsigned int iI2CIndex, int iNbBytes,BYTE** pbtDataBuffer) {
    int r,i,nbReadBytes,waitcount;
    BYTE *pbtIndexBuffer;
    unsigned char *    pucDataBuffer;
    if(i2c->address == DEMOD_ADDRESS)
        nbReadBytes = Si2165_GetReg(i2c->i2c_id, iI2CIndex, *pbtDataBuffer, iNbBytes);
    else if(i2c->address == TUNER_ADDRESS)
#if APP_TUNER_OFDM_MAX3580_SI2165
        nbReadBytes = Max3580_GetReg(iI2CIndex, *pbtDataBuffer, iNbBytes);
#endif
#if APP_TUNER_OFDM_EN4020_SI2165
;
#endif
    return nbReadBytes;
}
#endif /* SERIT_I2C */

EMB_L0_API int SLAB_WINAPI L0_ReadBytes (L0_Context* i2c, unsigned int iI2CIndex, int iNbBytes, BYTE** pbtDataBuffer) {
    int r,i,nbReadBytes,waitcount;
    BYTE *pbtIndexBuffer;
    unsigned char *    pucDataBuffer;

    nbReadBytes = SERIT_ReadBytes(i2c, iI2CIndex, iNbBytes, pbtDataBuffer);

#if 0
    pbtIndexBuffer = (BYTE*) malloc(i2c->indexSize*sizeof(BYTE));
    pucDataBuffer = (unsigned char*) malloc(iNbBytes*sizeof(unsigned char));

    if (i2c->trackRead==1) {
        printf("Layer0 reading from %#04x ",i2c->address);
        if (i2c->indexSize) printf("at index 0x");
    }
    for (i=0;i<i2c->indexSize;i++) {
        r = 8*(i2c->indexSize -1 -i);
        pbtIndexBuffer[i] = (unsigned char)((iI2CIndex & (0xff<<r) ) >> r);
        if (i2c->trackRead==1) printf("%02x",pbtIndexBuffer[i]);
    }
    if (i2c->trackRead==1) {
        printf(" <%s",i2c->interfaceType);
    }

    switch (i2c->connectionType)
        {
        #ifdef SILABS_I2C
        case USB:
          nbReadBytes = 0;
          waitcount = 0;
           if (RET_OK == Cypress_USB_ReadI2C(i2c->address>>1, i2c->indexSize, pbtIndexBuffer, iNbBytes, pucDataBuffer)) {
               nbReadBytes = iNbBytes;
               for ( i=0 ; i < nbReadBytes ; i++ ) {(*pbtDataBuffer)[i] = (BYTE) pucDataBuffer[i];}
           }
          break;
        #endif    /* SILABS_I2C */
        #ifdef SERIT_I2C
        case CUSTOMER:
            nbReadBytes = SERIT_ReadBytes(i2c, iI2CIndex, iNbBytes, pbtDataBuffer);
            break;
        #endif    /* SERIT_I2C */
        default:
            nbReadBytes = 0;
            break;
        };
    if (i2c->trackRead==1) {
        for ( i=0 ; i < nbReadBytes ; i++ ) {
            printf(" 0x%02x",(*pbtDataBuffer)[i]);
        }
        printf("\n");
    }
    free ( pbtIndexBuffer);
    free (pucDataBuffer);
#endif
    return nbReadBytes;
}

EMB_L0_API int SLAB_WINAPI L0_WriteBytes(L0_Context* i2c, unsigned int iI2CIndex, int iNbBytes, BYTE* pbtDataBuffer) {
    int r,i,nbWrittenBytes,waitcount;
    BYTE *pbtIndexBuffer;
    BYTE *pbtBuffer;
    unsigned char *    pucDataBuffer;

    nbWrittenBytes = SERIT_WriteBytes(i2c, iI2CIndex, iNbBytes, pbtDataBuffer);

    #if 0
    pbtIndexBuffer = (BYTE*) malloc(i2c->indexSize*sizeof(BYTE));
    pbtBuffer = (BYTE*) malloc((iNbBytes + i2c->indexSize)*sizeof(BYTE));
    pucDataBuffer = (unsigned char*) malloc(iNbBytes*sizeof(unsigned char));

    if (i2c->trackWrite==1) {
        printf("Layer0 writing   in %#04x ",i2c->address);
        if (i2c->indexSize) printf("at index 0x");
    }
    for (i=0;i<i2c->indexSize;i++) {
        r = 8*(i2c->indexSize -1 -i);
        pbtBuffer[i] = (unsigned char)((iI2CIndex & (0xff<<r) ) >> r);
        pbtIndexBuffer[i] = pbtBuffer[i];
        if (i2c->trackWrite==1) printf("%02x",pbtBuffer[i]);
    }
    if (i2c->trackWrite==1) {
        printf(" %s>",i2c->interfaceType);
    }

    for ( i=0 ; i < iNbBytes ; i++ ) {
        pbtBuffer[i+i2c->indexSize] = pbtDataBuffer[i];
           pucDataBuffer[i] = (unsigned char) pbtDataBuffer[i];
        if (i2c->trackWrite==1) printf(" 0x%02x",pbtDataBuffer[i]);
    }
    if (i2c->trackWrite==1) printf("\n");
       switch (i2c->connectionType)
       {
            #ifdef SILABS_I2C
              case USB:
                  TRACES_PRINTF("Using SILABS_I2C: USB connectivity\n");
                  nbWrittenBytes = 0;
                  pbtDataBuffer = &pbtBuffer[i2c->indexSize];
                   if (RET_OK == Cypress_USB_WriteI2C (i2c->address>>1, i2c->indexSize, pbtIndexBuffer, iNbBytes, pucDataBuffer)) {
                       nbWrittenBytes = iNbBytes + i2c->indexSize;
                   }
                  break;
            #endif  /* SILABS_I2C */
            #ifdef SERIT_I2C
                case CUSTOMER:
                  TRACES_PRINTF("Using SERIT_I2C: CUSTOMER connectivity\n");
                    nbWrittenBytes = SERIT_WriteBytes(i2c, iI2CIndex, iNbBytes, pbtBuffer);
                    break;
            #endif  /* SERIT_I2C */
            default:
                TRACES_PRINTF ("Using default connectivity (%d) ???? There must be something wrong\n", i2c->connectionType);
                TRACE (DEFAULTING To SIMU);
                nbWrittenBytes = i2c->indexSize;
                break;
    };

    free (pbtBuffer);
    free (pbtIndexBuffer);
    free (pucDataBuffer);
    TRACES_PRINTF("nbWrittenBytes %d\n",nbWrittenBytes);
    #endif

    return nbWrittenBytes - i2c->indexSize;
}

/**************************************************************************************************
      L1_ReadBytes function
      Use:    Layer 1 bytes read passthru function
            Used to replace the more complex version  with 8/16 bits addressing capability
      Parameter:    iI2CIndex         the start index for the read
      Parameter:    iNbBytes         the number of bytes to read
      Parameter:    pbtDataBuffer     a buffer to store the bytes read
      Behavior:     This function calls the corresponding LO function
      Returns: 0 if error, otherwise the number of bytes read
*************************************************************************************************/
EMB_L1_DRIVER_CORE int       SLAB_WINAPI    L1_ReadBytes  (L1_Context* context, unsigned int iI2CIndex    , int iNbBytes, BYTE **pbtDataBuffer) {
    if (L0_ReadBytes(context->i2c, iI2CIndex, iNbBytes, pbtDataBuffer)) {
        return iNbBytes;
    }
    return 0;
}

/**************************************************************************************************
  L1_WriteBytes function
  Use:          Byte writing passthru function to the L0
                Used to replace the more complex version with 8/16 bits addressing capability
  Parameter:    iI2CIndex        the start index for the read
  Parameter:    iNbBytes         the number of bytes to read
  Parameter:    pbtDataBuffer    a buffer containing the bytes to write
  Behavior:     This function calls the corresponding LO function
  Returns: 0 if error, otherwise the number of bytes written
***************************************************************************************************/
EMB_L1_DRIVER_CORE int     SLAB_WINAPI  L1_WriteBytes  (L1_Context* context, unsigned int iI2CIndex, int iNbBytes, BYTE *pbtDataBuffer) {
    if (L0_WriteBytes(context->i2c, iI2CIndex, iNbBytes, pbtDataBuffer)) {
        return iNbBytes;
    }
    return 0;
}

EMB_L0_API void   SLAB_WINAPI  L0_TrackRead  (L0_Context* i2c, bool track) {
    i2c->trackRead = track;
}

EMB_L0_API void   SLAB_WINAPI  L0_TrackWrite (L0_Context* i2c, bool track) {
    i2c->trackWrite = track;
}

/**************************************************************************************************
  L1_Demod_set_adc_regs function
  Use:        ADC setup function
              Used to set the ADC registers
  Comments:   The ADC registers need to be set differently depending on the revision.
               With Rev C, they are also modified at init by the firmware, and need to be re-
written.
  Parameter: *demod          the demod handle
  Registers:   revcode, adc_ri1,

***************************************************************************************************/
EMB_L1_COMMON_DEMOD_API void   SLAB_WINAPI L1_Demod_set_adc_regs    (L1_Context *demod) {
    switch ((int)L1_READ(demod, revcode))
    {
        case revcode_rev_b: {
            L1_WRITE(demod, adc_ri1,  0x48);
            L1_WRITE(demod, adc_ri3,  0x09);
            L1_WRITE(demod, adc_ri5,  0x40);
            L1_WRITE(demod, adc_ri6,  0x4c);
            break;
        }
        case revcode_rev_c: {
            L1_WRITE(demod, adc_ri1,  0x46);
            L1_WRITE(demod, adc_ri3,  0x00);
            L1_WRITE(demod, adc_ri5,  0x0a);
            L1_WRITE(demod, adc_ri6,  0xff);
            L1_WRITE(demod, adc_ri8,  0x70);
            break;
        }
        default:             {
            break;
        }
    }
}

/**************************************************************************************************
  L1_DVB_T_Reset function
  Use:        Demodulator reset function in DVB_T
              Used to reset the demodulator for DVB_T
  Returns:    1 if the demodulator has been correctly reset
  Comments:   It
  Parameter: *demod          the demod handle
  Registers: rst_all gp_reg0 start_synchro

**************************************************************************************************/
EMB_L1_DVB_T_DEMOD_API char   SLAB_WINAPI L1_DVB_T_Reset          (L1_Context *demod) {
    L1_WRITE(demod, rst_all       , rst_all_reset);
    L1_WRITE(demod, gp_reg0       , 0);
    L1_WRITE(demod, start_synchro , start_synchro_start);
    L1_Demod_set_adc_regs(demod);
    return 1;
}


/***********************************************************************************************************************
  L1_DVB_C_Reset function
  Use:        Demodulator reset function in DVB_C
              Used to reset the demodulator for DVB_C
  Returns:    1 if the demodulator has been correctly reset
  Comments:   It
  Parameter: *demod          the demod handle
  Registers: rst_all

***********************************************************************************************************************/
EMB_L1_DVB_C_DEMOD_API char   SLAB_WINAPI L1_DVB_C_Reset          (L1_Context *demod) {
    L1_WRITE(demod, rst_all       , rst_all_reset);
    L1_WRITE(demod, gp_reg0       , 0);
    L1_WRITE(demod, start_synchro , start_synchro_start);
    L1_Demod_set_adc_regs(demod);
    return 1;
}

/**************************************************************************************************
  L1_DVB_T_get_stream function
  Use:        DVB_T stream checking function
              Used to read the DVB_T stream value found in the TPS
  Returns:    the value of the stream register
  Registers:    stream
  Parameter:   *demod          the demod handle

***************************************************************************************************/
EMB_L1_DVB_T_DEMOD_API int    SLAB_WINAPI L1_DVB_T_get_stream        (L1_Context *demod) {
    return (int)L1_READ(demod, stream);
}

/**************************************************************************************************
  L1_DVB_T_get_stream function
  Use:        DVB_T FFT mode checking function
              Used to read the DVB_T FFT mode value found in the TPS
  Returns:    the value of the auto_fft_mode register
  Registers:    auto_fft_mode
  Parameter:   *demod          the demod handle

****************************************************************************************************/
EMB_L1_DVB_T_DEMOD_API int    SLAB_WINAPI L1_DVB_T_get_fft_mode      (L1_Context *demod) {
    return (int)L1_READ(demod, auto_fft_mode);
}

/**************************************************************************************************
  L1_DVB_T_get_guard_interval function
  Use:        DVB_T guard interval checking function
              Used to read the DVB_T guard interval value found in the TPS
  Returns:    the value of the auto_guard_int register
  Registers:    auto_guard_int
  Parameter:   *demod          the demod handle

***************************************************************************************************/
EMB_L1_DVB_T_DEMOD_API int    SLAB_WINAPI L1_DVB_T_get_guard_interval(L1_Context *demod) {
    return (int)L1_READ(demod, auto_guard_int);
}

/**************************************************************************************************
  L1_DVB_T_get_constellation function
  Use:        DVB_T constellation checking function
              Used to read the DVB_T constellation value found in the TPS
  Returns:    the value of the auto_constellation register
  Registers:    auto_constellation
  Parameter:   *demod          the demod handle

***************************************************************************************************/
EMB_L1_DVB_T_DEMOD_API int    SLAB_WINAPI L1_DVB_T_get_constellation (L1_Context *demod) {
    return (int)L1_READ(demod, auto_constellation);
}

/**************************************************************************************************
  L1_DVB_T_get_hierarchy function
  Use:        DVB_T hierarchy checking function
              Used to read the DVB_T hierarchy value found in the TPS
  Returns:    the value of the auto_hierarchy register
  Registers:    auto_hierarchy
  Parameter:   *demod          the demod handle

***************************************************************************************************/
EMB_L1_DVB_T_DEMOD_API int    SLAB_WINAPI L1_DVB_T_get_hierarchy     (L1_Context *demod) {
    return (int)L1_READ(demod, auto_hierarchy);
}

/**************************************************************************************************
  L1_DVB_T_get_code_rate_hp function
  Use:        DVB_T HP rate checking function
              Used to read the DVB_T HP rate value found in the TPS
  Returns:    the value of the auto_rate_hp register
  Registers:    auto_rate_hp
  Parameter:   *demod          the demod handle

***************************************************************************************************/
EMB_L1_DVB_T_DEMOD_API int    SLAB_WINAPI L1_DVB_T_get_code_rate_hp  (L1_Context *demod) {
    return (int)L1_READ(demod, auto_rate_hp);
}

/**************************************************************************************************
  L1_DVB_T_get_code_rate_lp function
  Use:        DVB_T LP rate checking function
              Used to read the DVB_T LP rate value found in the TPS
  Returns:    the value of the auto_rate_lp register
  Registers:    auto_rate_lp
  Parameter:   *demod          the demod handle

***************************************************************************************************/
EMB_L1_DVB_T_DEMOD_API int    SLAB_WINAPI L1_DVB_T_get_code_rate_lp  (L1_Context *demod) {
    return (int)L1_READ(demod, auto_rate_lp);
}

/**************************************************************************************************
  L1_DVB_T_get_status function
  Use:      DVB_T statusing function
            Used to retrieve the DVB_T status information in a structure
  Returns:      0 if the structure initialization fails, 1 otherwise
  Behavior:     Sets all members of the DVB_T_Status structure provided during function call to
the current
                 hardware values by calling the corresponding statusing functions and returns
  Parameter:   *demod          the demod handle
                **status        a DVB_T_Status structure used to store the status information

***************************************************************************************************/
EMB_L1_DVB_T_DEMOD_API char   SLAB_WINAPI L1_DVB_T_get_status       (L1_Context *demod,
DVB_T_Status  **status) {
    *status = (DVB_T_Status  *) malloc(sizeof(DVB_T_Status));
    if (*status == NULL ) return 0;
    (*status)->stream         = L1_DVB_T_get_stream         (demod);
    (*status)->fft_mode       = L1_DVB_T_get_fft_mode       (demod);
    (*status)->guard_interval = L1_DVB_T_get_guard_interval (demod);
    (*status)->constellation  = L1_DVB_T_get_constellation  (demod);
    (*status)->hierarchy      = L1_DVB_T_get_hierarchy      (demod);
    (*status)->code_rate_hp   = L1_DVB_T_get_code_rate_hp   (demod);
    (*status)->code_rate_lp   = L1_DVB_T_get_code_rate_lp   (demod);
    return 1;
}


/**************************************************************************************************
    L1_DVB_T_get_freq_offset  function
  Use:      DVB_T frequency offset checking function
            Used for computing the DVB_T frequency offset
  Formula:
    frequency_offset_Hz = FE_clk.freq_corr_t/2^29

  Returns:      the current frequency offset value in Hz for DVB_T
  Behavior:     reads freq_corr_t and then returns the corresponding frequency offset
  Parameter:   *demod          the demod handle
  Variable:   Si2165->symbol_rate_MHz   the driver-internal symbol rate.
  Variable:   Si2165->sampling_rate     the driver-internal sampling rate.
  Registers:    freq_corr_t

***************************************************************************************************/
EMB_L1_DVB_T_DEMOD_API double SLAB_WINAPI L1_DVB_T_get_freq_offset   (L1_Context *demod) {

    /* to access Si2165 specific parameters, cast demod->specific as a Si2165_Context */
    Si2165_Context *Si2165;
    double freq_corr;
    Si2165  = (Si2165_Context *) demod->specific;

    freq_corr = L1_READ(demod, freq_corr_t);
    /* Use 536870912.0 instead of pow(2,29), to avoid use of math.h */
    return (double)((Si2165->FE_clk*freq_corr*1000000.0)/536870912.0);
}
/***********************************************************************************************************************
    L1_DVB_C_get_freq_offset  function
  Use:      DVB_C frequency offset checking function
            Used for computing the DVB_C frequency offset
  Formula:

    frequency_offset_Hz = (FE_clk.freq_corr_c + dvb_rate.phase_cor_c) / 2^16

  Returns:      the current frequency offset value in Hz for DVB_C
  Behavior:     reads freq_corr_c and phase_cor_c and then returns the corresponding frequency
offset
  Parameter:   *demod          the demod handle
  Variable:    Si2165->FE_clk the driver-internal Front-End clock
  Variable:    Si2165->dvb_rate   the driver-internal DVB_rate
  Registers:    freq_corr_c phase_cor_c

***********************************************************************************************************************/
EMB_L1_DVB_C_DEMOD_API double SLAB_WINAPI L1_DVB_C_get_freq_offset   (L1_Context *demod) {

    /* to access Si2165 specific parameters, cast demod->specific as a Si2165_Context */
    Si2165_Context *Si2165;
    Si2165  = (Si2165_Context *) demod->specific;

    /* Use 65536.0 instead of pow(2,16), to avoid use of math.h */
    return (double)((Si2165->FE_clk*L1_READ(demod, freq_corr_c) + Si2165->dvb_rate*L1_READ(demod,phase_cor_c))/65536.0);
}


/**************************************************************************************************
    L1_Demod_get_freq_offset  function
  Use:      main frequency offset check function.
            Used for the selection of the correct get_freq_offset function for the current standard
  Returns:      the current freq offset value in Hz
  Parameter:   *demod          the demod handle
  Variable:   Si2165->standard the driver-internal standard

***************************************************************************************************/
EMB_L1_MULTISTANDARD_API double SLAB_WINAPI L1_Demod_get_freq_offset (L1_Context *demod) {

    /* to access Si2165 specific parameters, cast demod->specific as a Si2165_Context */
    Si2165_Context *Si2165;
    Si2165  = (Si2165_Context *) demod->specific;

    switch (Si2165->standard)
    {
        case standard_dvb_t: return L1_DVB_T_get_freq_offset(demod);
        case standard_dvb_c: return L1_DVB_C_get_freq_offset(demod);
    }
    return 0;
}

/**************************************************************************************************
  L1_Demod_set_if_freq_shift function
  Use:      IF frequency shift setup function
            Used to set the if_freq_shift register
  Returns:      the current if_freq_shift
  Formula:     if_freq_shift = digital_if_MHz.2^29/FE_clk
  Parameter:   *demod          the demod handle
  Registers:    if_freq_shift

***************************************************************************************************/
EMB_L1_COMMON_DEMOD_API double    SLAB_WINAPI L1_Demod_Set_if_freq_shift   (L1_Context *demod) {

    /* to access Si2165 specific parameters, cast demod->specific as a Si2165_Context */
    Si2165_Context *Si2165;
    Si2165  = (Si2165_Context *) demod->specific;

    /* To avoid a null FE_clk, set it to DEFAULT_FE_CLK if null */
    if (Si2165->FE_clk   == 0) {Si2165->FE_clk   = DEFAULT_FE_CLK;}

    /* Use 536870912.0 instead of pow(2,29), to avoid use of math.h */
    return L1_WRITE(demod, if_freq_shift, (Si2165->digital_if_MHz)*536870912.0/(Si2165->FE_clk) );
}

/**************************************************************************************************
  L1_Demod_Set_Digital_IF_MHz function
  Use:      digital IF setup function
            Used to set the digital IF value and call L1_Demod_Set_if_freq_shift
  Returns:      the current digital IF frequency
  Parameter:   *demod         the demod handle
                digital_if     the desired digital IF frequency value, in MHz

***************************************************************************************************/
EMB_L1_COMMON_DEMOD_API double   SLAB_WINAPI L1_Demod_Set_Digital_IF_MHz   (L1_Context *demod,double digital_if) {

    /* to access Si2165 specific parameters, cast demod->specific as a Si2165_Context */
    Si2165_Context *Si2165;
    Si2165  = (Si2165_Context *) demod->specific;

    Si2165->digital_if_MHz = digital_if;

    /* To avoid a null FE_clk, set it to DEFAULT_FE_CLK if null */
    if (Si2165->FE_clk   == 0) {Si2165->FE_clk   = DEFAULT_FE_CLK;}

    /* After changing the digital if, it is necessary to recompute the if freq shift */
    L1_Demod_Set_if_freq_shift(demod);

    return Si2165->digital_if_MHz;
}

/**************************************************************************************************
  L1_Demod_set_FE_clk function
  Use:      FE_clk setup function
            Used to set the Front-end clock value and update the necessary registers
  Returns:      the current FE_clk
  Equation: if (adc_sampling_mode = if_ovr4 or zif_ovr4), FE_clk = adc_clk
  Equation: if (adc_sampling_mode = if_ovr2 or zif_ovr2), FE_clk = adc_clk*2
  Comments:     Only non-null values are accepted, otherwise it uses the default value of FE_clk
  Parameter:   *demod          the demod handle
                fe_clk     the desired Front-End clock value
  Registers:    adc_sampling_mode

****************************************************************************************************/
EMB_L1_COMMON_DEMOD_API double   SLAB_WINAPI L1_Demod_Set_FE_clk   (L1_Context *demod) {
    int sampling_mode;

    /* to access Si2165 specific parameters, cast demod->specific as a Si2165_Context */
    Si2165_Context *Si2165;
    Si2165  = (Si2165_Context *) demod->specific;

    /* Select the FE_Clk formula depending on the sampling mode (register adc_sampling_mode) */
    sampling_mode = (int)L1_READ(demod, adc_sampling_mode);
    switch (sampling_mode)
    {
        case adc_sampling_mode_zif_ovr4: {
            Si2165->FE_clk = Si2165->adc_clk;
            break;
        }
        case adc_sampling_mode_if_ovr2: {
            Si2165->FE_clk = Si2165->adc_clk*2;
            break;
        }
        case adc_sampling_mode_zif_ovr2: {
            Si2165->FE_clk = Si2165->adc_clk*2;
            break;
        }
        default: {
            /* sampling_mode = adc_sampling_mode_if_ovr4 */
            Si2165->FE_clk = Si2165->adc_clk;
            break;
        }
    }
    return Si2165->FE_clk;
}

/**************************************************************************************************
  L1_Demod_set_oversamp function
  Use:      oversampling rate setup function
            Used to set the oversamp register
  Returns:      the current oversamp
  Formula:      oversamp = 2^23.(FE_clk/dvb_rate)
  Parameter:   *demod          the demod handle
  Registers:    oversamp

***************************************************************************************************/
EMB_L1_COMMON_DEMOD_API double    SLAB_WINAPI L1_Demod_Set_oversamp   (L1_Context *demod) {

    /* to access Si2165 specific parameters, cast demod->specific as a Si2165_Context */
    Si2165_Context *Si2165;
    Si2165  = (Si2165_Context *) demod->specific;

    /* To avoid division by 0, set the dvb_rate at DEFAULT_DVB_RATE if null */
    if (Si2165->dvb_rate == 0) {Si2165->dvb_rate = DEFAULT_DVB_RATE;}

    /* Use 8388608.0 instead of pow(2,23), to avoid use of math.h */
    return L1_WRITE(demod, oversamp,    (Si2165->FE_clk)*8388608.0/(Si2165->dvb_rate) );
}

/**************************************************************************************************
  L1_DVB_T_set_dvb_rate function
  Use:        dvb_rate setup function
              Used to set the dvb_rate variable to the proper value for DVB-T
  Formula:  dvb_rate = bandwidth_MHz.8/7
  Equation: dvb_rate = EQ_START bandwidth_MHz.8/7 EQ_END

  Returns:    the current dvb_rate
  Behavior:   Sets the Si2165->dvb_rate for future reference, then calls the
L1_Demod_Set_oversamp to update the value of the oversamp register
  Parameter:  *demod          the demod handle
  Variable:   Si2165->dvb_rate   the current dvb_rate value
  Registers: oversamp

***************************************************************************************************/
EMB_L1_DVB_T_DEMOD_API double   SLAB_WINAPI L1_DVB_T_Set_dvb_rate(L1_Context *demod) {

    /* to access Si2165 specific parameters, cast demod->specific as a Si2165_Context */
    Si2165_Context *Si2165;
    Si2165  = (Si2165_Context *) demod->specific;

    if (Si2165->bandwidth_MHz == 0) Si2165->bandwidth_MHz = DEFAULT_DVB_T_BANDWIDTH;
    Si2165->dvb_rate = (double)(Si2165->bandwidth_MHz*8.0/7.0);
    L1_Demod_Set_oversamp(demod);
    return Si2165->dvb_rate;
}

/***********************************************************************************************************************
  L1_DVB_C_set_dvb_rate function
  Use:        dvb_rate setup function
              Used to set the dvb_rate variable to the proper value for DVB-C
  Equation: dvb_rate = symbol_rate

  Returns:    the current dvb_rate
  Behavior:   Sets the Si2165->dvb_rate for future reference, then calls the
L1_Demod_Set_oversamp to update the value of the oversamp register
  Parameter:  *demod          the demod handle
  Variable:   Si2165->dvb_rate   the current dvb_rate value
  Registers: oversamp

***********************************************************************************************************************/
EMB_L1_DVB_C_DEMOD_API double   SLAB_WINAPI L1_DVB_C_Set_dvb_rate(L1_Context *demod) {

    /* to access Si2165 specific parameters, cast demod->specific as a Si2165_Context */
    Si2165_Context *Si2165;
    Si2165  = (Si2165_Context *) demod->specific;

    /* To avoid a null symbol rate, set it to DEFAULT_DVB_C_SYMBOL_RATE if null */
    if (Si2165->symbol_rate_MHz == 0) {
        Si2165->symbol_rate_MHz = DEFAULT_DVB_C_SYMBOL_RATE;
    }
    Si2165->dvb_rate = (double)(Si2165->symbol_rate_MHz);
    L1_Demod_Set_oversamp(demod);
    return Si2165->dvb_rate;
}

/**************************************************************************************************
  L1_Demod_set_dvb_rate function
  Use:        top-level dvb_rate setup function
              Used to call the proper dvb_rate setup function for the current standard
  Returns:    the current dvb_rate
  Parameter:  *demod          the demod handle
  Comment:    The dvb_rate is not provided to this function, as it needs to be computed from
standard-dependent variables by the underlying functions.
  Variable:   Si2165->standard the driver-internal standard
  Variable:   Si2165->dvb_rate the driver-internal DVB rate

***************************************************************************************************/
EMB_L1_COMMON_DEMOD_API double   SLAB_WINAPI L1_Demod_Set_dvb_rate(L1_Context *demod) {

    /* to access Si2165 specific parameters, cast demod->specific as a Si2165_Context */
    Si2165_Context *Si2165;
    Si2165  = (Si2165_Context *) demod->specific;

    switch (Si2165->standard)
    {
        case standard_dvb_t: return L1_DVB_T_Set_dvb_rate(demod);
        case standard_dvb_c: return L1_DVB_C_Set_dvb_rate(demod);
    }
    return Si2165->dvb_rate;
}

/**************************************************************************************************
  L1_DVB_T_set_bandwidth function
  Use:        DVB_T bandwidth control function
              Used to set the driver-internal symbol_rate for the required BW
  Equation:   bandwidth = CHANNEL_BW_MHz.100
  Returns:    the current bandwidth
  Parameter: *demod          the demod handle
                bw_MHz the desired bandwidth (in MHz)
  Variable:  Si2165->bandwidth_MHz      the desired channel bandwidth (in MHz)
  Behavior:   Sets the bandwidth according to the input bandwidth, and calls
               L1_DVB_T_Set_dvb_rate to compute dvb_rate and update the oversamp register
accordingly
  Registers: bandwidth
***************************************************************************************************/
EMB_L1_DVB_T_DEMOD_API int    SLAB_WINAPI L1_DVB_T_set_bandwidth    (L1_Context *demod, int bw_MHz
) {

    /* to access Si2165 specific parameters, cast demod->specific as a Si2165_Context */
    Si2165_Context *Si2165;
    Si2165  = (Si2165_Context *) demod->specific;

    Si2165->bandwidth_MHz = bw_MHz;
    if (Si2165->bandwidth_MHz == 0) Si2165->bandwidth_MHz = DEFAULT_DVB_T_BANDWIDTH;
    L1_WRITE(demod, bandwidth,  Si2165->bandwidth_MHz*100);
    L1_Demod_Set_dvb_rate(demod);
    return bw_MHz;
}
/***********************************************************************************************************************
  L1_DVB_C_set_symbol_rate function
  Use:        Symbol rate control function
              Used to set the DVB_C symbol_rate
  Returns:    the current symbol_rate
  Parameter: *demod           the demod handle
              symbol_rate      the desired symbol_rate (in MHz)
  Variable:  Si2165->symbol_rate_MHz the driver-internal symbol rate
  Behavior:   Logs the current symbol_rate and calls L1_Demod_set_sampling_rate to update the
necessary registers

***********************************************************************************************************************/
EMB_L1_DVB_C_DEMOD_API double SLAB_WINAPI L1_DVB_C_set_symbol_rate  (L1_Context *demod, double symbol_rate) {

    /* to access Si2165 specific parameters, cast demod->specific as a Si2165_Context */
    Si2165_Context *Si2165;
    Si2165  = (Si2165_Context *) demod->specific;

    Si2165->symbol_rate_MHz = symbol_rate;
    L1_Demod_Set_dvb_rate(demod);
    return Si2165->symbol_rate_MHz;
}
/**********************************************************************************************************************
  L1_DVB_C_set_constellation function
  Use:        DVB_C constellation control function
              Used to set the DVB_C constellation
  Returns:    the current constellation
  Parameter: *demod           the demod handle
              constel          the desired constellation
  Behavior:   Writes the constellation to the required value
  Registers: constellation

***********************************************************************************************************************/
EMB_L1_DVB_C_DEMOD_API int    SLAB_WINAPI L1_DVB_C_set_constellation(L1_Context *demod, int constel)
{
    return (int)L1_WRITE(demod, constellation, constel);
}

/**************************************************************************************************
  L1_DVB_T_Re_Init function
  Use:        Demodulator re-init function in DVB_T
              Used to start the demodulator and re-initialize it for DVB_T
  Returns:    1 if the demodulator has been correctly initialized
  Comments:   It should be called after each HW reset or power cycle, in DVB_T mode
              All registers that need to have their values set differently from the default values should
be set here
  Parameter: *demod          the demod handle
  Registers: addr_jump       gp_reg0

*************************************************************************************************/
EMB_L1_DVB_T_DEMOD_API char   SLAB_WINAPI L1_DVB_T_Re_Init          (L1_Context *demod) {
    int count;

    /* to access Si2165 specific parameters, cast demod->specific as a Si2165_Context */
    Si2165_Context *Si2165;
    Si2165  = (Si2165_Context *) demod->specific;

    L1_DVB_T_set_bandwidth      (demod, Si2165->bandwidth_MHz);

    /* setting the DSP start address for DVB_T */
    L1_WRITE(demod, dsp_mode      , dsp_mode_dvb_t);
    L1_WRITE(demod, addr_jump     , DVB_T_ADDR_JUMP);
    L1_DVB_T_Reset(demod);
    count = 0;
    while (L1_READ(demod, boot_done) != boot_done_done) {
        if (count++ > 5) {
            TRACES_ERR (Boot never completes !);
            /* This probably means that there is a communication error */
            return 0;
            break;
        }
    }
    return 1;
}

/***********************************************************************************************************************
  L1_DVB_C_Re_Init function
  Use:        Demodulator re-init function in DVB_C
              Used to start the demodulator and re-initialize it for DVB_C
  Returns:    1 if the demodulator has been correctly initialized
  Comments:   It should be called after each HW reset or power cycle, in DVB_C mode
              All registers that need to have their values set differently from the default values should
be set here
  Parameter: *demod          the demod handle
  Registers: addr_jump rst_all auto_algo

***********************************************************************************************************************/
EMB_L1_DVB_C_DEMOD_API char   SLAB_WINAPI L1_DVB_C_Re_Init          (L1_Context *demod) {
    /* to access Si2165 specific parameters, cast demod->specific as a Si2165_Context */
    Si2165_Context *Si2165;
    Si2165  = (Si2165_Context *) demod->specific;

    L1_DVB_C_set_symbol_rate(demod, Si2165->symbol_rate_MHz);
    L1_WRITE(demod, auto_algo,         auto_algo_dfe_init);
    L1_WRITE(demod, kp_lock,           KP_LOCK);
    L1_WRITE(demod, central_tap,       CENTRAL_TAP);
    /* setting the DSP start address for DVB_C (--MUST-- be 0x00000000 !) */
    L1_WRITE(demod, addr_jump,         DVB_C_ADDR_JUMP);
    L1_DVB_C_Reset(demod);
    return 1;
}


/**************************************************************************************************
  L1_Demod_reset function
  Use:        Demodulator 'software' reset function
              Used to restart all internal loops inside the demodulator
  Comments:   No demodulator setting is changed when resetting the demodulator using
L1_Demod_reset.
               The demodulator should relock if there is a valid signal.
  Parameter: *demod          the demod handle
  Registers:   rst_all

****************************************************************************************************/
EMB_L1_MULTISTANDARD_API void   SLAB_WINAPI L1_Demod_reset            (L1_Context *demod) {

    /* to access Si2165 specific parameters, cast demod->specific as a Si2165_Context */
    Si2165_Context *Si2165;
    Si2165  = (Si2165_Context *) demod->specific;

    switch (Si2165->standard)
    {
        case standard_dvb_t: L1_DVB_T_Reset(demod); break;
        case standard_dvb_c: L1_DVB_C_Reset(demod); break;
        default: return;
    }
}


/**************************************************************************************************
  L1_Demod_TS_mode function
  Use:        TS output mode selection function
              Used to set the Transport Stream interface in parellel or serial mode
  Behavior:   Sets the TS mode if 'serial' or 'parallel', or OFF otherwise
  Parameter:  *demod          the demod handle
  Parameter:  ts_mode         the required TS mode
  Returns:    the current_mode (2 means 'OFF')

**************************************************************************************************/
EMB_L1_COMMON_DEMOD_API int    SLAB_WINAPI L1_Demod_TS_mode        (L1_Context *demod, int ts_mode
) {
    switch (ts_mode)
    {
        case ts_data_mode_serial: {
                L1_WRITE(demod, ts_data_mode        , ts_data_mode_serial);
                /* Enabling only the necessary TS pins */
                L1_WRITE(demod, ts_val_tri          , ts_val_tri_normal);
                L1_WRITE(demod, ts_sync_tri         , ts_sync_tri_normal);
                L1_WRITE(demod, ts_clk_tri          , ts_clk_tri_normal);
                L1_WRITE(demod, ts_err_tri          , ts_err_tri_normal);
                L1_WRITE(demod, ts_data0_tri        , ts_data0_tri_normal);
                L1_WRITE(demod, ts_data1_tri        , ts_data1_tri_tristate);
                L1_WRITE(demod, ts_data2_tri        , ts_data2_tri_tristate);
                L1_WRITE(demod, ts_data3_tri        , ts_data3_tri_tristate);
                L1_WRITE(demod, ts_data4_tri        , ts_data4_tri_tristate);
                L1_WRITE(demod, ts_data5_tri        , ts_data5_tri_tristate);
                L1_WRITE(demod, ts_data6_tri        , ts_data6_tri_tristate);
                L1_WRITE(demod, ts_data7_tri        , ts_data7_tri_tristate);
                ts_mode = ts_data_mode_serial;
                break;
            }
        case ts_data_mode_parallel: {
                L1_WRITE(demod, ts_data_mode        , ts_data_mode_parallel);
                /* Enabling all TS pins */
                L1_WRITE(demod, ts_val_tri          , ts_val_tri_normal);
                L1_WRITE(demod, ts_sync_tri         , ts_sync_tri_normal);
                L1_WRITE(demod, ts_clk_tri          , ts_clk_tri_normal);
                L1_WRITE(demod, ts_err_tri          , ts_err_tri_normal);
                L1_WRITE(demod, ts_data0_tri        , ts_data0_tri_normal);
                L1_WRITE(demod, ts_data1_tri        , ts_data1_tri_normal);
                L1_WRITE(demod, ts_data2_tri        , ts_data2_tri_normal);
                L1_WRITE(demod, ts_data3_tri        , ts_data3_tri_normal);
                L1_WRITE(demod, ts_data4_tri        , ts_data4_tri_normal);
                L1_WRITE(demod, ts_data5_tri        , ts_data5_tri_normal);
                L1_WRITE(demod, ts_data6_tri        , ts_data6_tri_normal);
                L1_WRITE(demod, ts_data7_tri        , ts_data7_tri_normal);
                ts_mode = ts_data_mode_parallel;
                break;
            }
        default: {
             /* only ts_mode_serial (0) and ts_mode_parallel (1) are defined                 */
             /* in case another value is received, switch TS OFF and return 2 to signal this */
                /* Tristating all TS pins */
                L1_WRITE(demod, ts_val_tri          , ts_val_tri_tristate);
                L1_WRITE(demod, ts_sync_tri         , ts_sync_tri_tristate);
                L1_WRITE(demod, ts_clk_tri          , ts_clk_tri_tristate);
                L1_WRITE(demod, ts_err_tri          , ts_err_tri_tristate);
                L1_WRITE(demod, ts_data0_tri        , ts_data0_tri_tristate);
                L1_WRITE(demod, ts_data1_tri        , ts_data1_tri_tristate);
                L1_WRITE(demod, ts_data2_tri        , ts_data2_tri_tristate);
                L1_WRITE(demod, ts_data3_tri        , ts_data3_tri_tristate);
                L1_WRITE(demod, ts_data4_tri        , ts_data4_tri_tristate);
                L1_WRITE(demod, ts_data5_tri        , ts_data5_tri_tristate);
                L1_WRITE(demod, ts_data6_tri        , ts_data6_tri_tristate);
                L1_WRITE(demod, ts_data7_tri        , ts_data7_tri_tristate);
                ts_mode = 2;
                break;
            }
    }
    return ts_mode;
}


/**************************************************************************************************
  L1_Demod_Re_Init function
  Use:        demodulator re-init function
              Used to start the demodulator and re-initialize it for the current standard
  Returns:    1 if the demodulator has been correctly initialized
  Parameter:   *demod          the demod handle
  Comments:   It should be called after each HW reset or power cycle.
          The location of the digital IF carrier depends on:
          - The adc_sampling_mode
          - The spectral inversion.
        Typical values:
            adc_sampling_mode = 'if_ovr4', Tuner_IF = 36 MHz, adc_clk  = 60 MHz.

       In IF mode: if 'adc_sampling_mode' ==  if_ovr2 OR  if_ovr4.
           (register iq_adc_swap is 'not_swapped').

           NO spectral inversion:
             digital_if_MHz =     Tuner_IF - adc_clk.

           WITH spectral inversion:
             digital_if_MHz = -1*(Tuner_IF - adc_clk).

       In Low_IF or Zero_IF: if 'adc_sampling_mode' ==  zif_ovr2 OR  zif_ovr4.
           digital_if_MHz = Tuner_IF (close to 0).

           NO spectral inversion:
                iq_adc_swap is 'not_swapped'.

           WITH spectral inversion:
                iq_adc_swap is 'swapped'.


***************************************************************************************************/
  EMB_L1_MULTISTANDARD_API char   SLAB_WINAPI L1_Demod_Re_Init        (L1_Context *demod) {
    int    chipmode;
    int    count;
    int    div_l;
    double init_ok;

    /* to access Si2165 specific parameters, cast demod->specific as a Si2165_Context */
    Si2165_Context *Si2165;
    Si2165  = (Si2165_Context *) demod->specific;


    /* <porting> The SiLabs communication layer allows dynamic communication modeselection                        */
    /* <porting> This layer requires the connection type to be set to reconnect in case of HW reset or power-cycle */

#ifdef SILABS_I2C
    L0_Connect(demod->i2c, USB);
#else
    //L0_Connect(demod->i2c, CUSTOMER);
#endif  /* SILABS_I2C */

    /* system configuration */
    /*  -1- chip mode */
    L1_WRITE(demod, chip_mode,  CHIP_MODE);

    if ((chipmode = L1_READ(demod, chip_mode)) != CHIP_MODE) {
        /* <porting> Change this for the final error handling when it is not possible to set the chip_mode as required */
        TREAT_ERROR_MESSAGE("!*!*!*!*!    Board disconnected    !*!*!*!*!\n");
        //system("pause");
        return 0;
    }


    /*  -2- AGCs      */
    /* <porting> The RF agc may be kept in tristate if not used (depends on the RF design) */
    L1_WRITE(demod, agc_rf_tri, agc_rf_tri_tristate);
    L1_WRITE(demod, agc_if_tri       , agc_if_tri_normal);
    L1_WRITE(demod, agc2_buftype     , agc2_buftype_push_pull);
    L1_WRITE(demod, agc2_pola        , agc2_pola_non_inverted);
    L1_WRITE(demod, agc2_clkdiv      , 7);

    L1_WRITE(demod, agc2_max         , 0xFF);
    L1_WRITE(demod, agc2_min         , AGC2_MIN);
    L1_WRITE(demod, agc2_kacq        , AGC2_KACQ);
    L1_WRITE(demod, agc2_kloc        , AGC2_KLOC);
    L1_WRITE(demod, agc_unfreeze_thr , 3);
    L1_WRITE(demod, agc_alpha_acq, 16);
    switch (Si2165->standard)
    {
        case standard_dvb_c: {
            L1_WRITE(demod, agc_crestf_dbx8, 104);
            L1_WRITE(demod, aaf_crestf_dbx8, 104);
            L1_WRITE(demod, aci_crestf_dbx8,  80);
            break;
        }
        default: {
            L1_WRITE(demod, agc_crestf_dbx8, 120);
            L1_WRITE(demod, aaf_crestf_dbx8, 120);
            L1_WRITE(demod, aci_crestf_dbx8, 104);
            break;
        }
    }

    /*  -3- ADC       */
    L1_WRITE(demod, adc_sampling_mode, ADC_SAMPLING_MODE);

    /* PLL setup    */
    L1_WRITE(demod, pll_divr, DIV_R);
    L1_WRITE(demod, pll_divn, DIV_N);
    L1_WRITE(demod, pll_divm, DIV_M);
    switch (Si2165->standard)
    {
        case standard_dvb_c: {
            div_l = 14;
            break;
        }
        default: {
            div_l = 11;
             break;
        }
    }
    L1_WRITE(demod, pll_divl, div_l);
    L1_WRITE(demod, pll_divp, DIV_P);
    L1_WRITE(demod, pll_enable, pll_enable_enable);

    if (DIV_P == pll_divp_prescaler_1) {
        Si2165->FVCO    = REF_FREQUENCY*2*DIV_N*1/DIV_R;
    } else { /* pll_divp_prescaler_4 */
        Si2165->FVCO    = REF_FREQUENCY*2*DIV_N*4/DIV_R;
    }

    Si2165->adc_clk = Si2165->FVCO/(4*DIV_M);
    Si2165->sys_clk = Si2165->FVCO/(2*div_l);

    L1_Demod_Set_FE_clk(demod);

#ifdef IF_MODE
    /* IF mode:
       <porting> The IF setup depends on the RF design!
        if adc_sampling_mode 'if_ovr4' and iq_adc_swap 'not_swapped':
        NO spectral inversion: digital_if_MHz = Tuner_IF - adc_clk = 36 - 57 = -21 MHz
    */
       L1_Demod_Set_Digital_IF_MHz(demod, TUNER_IF_MHZ - Si2165->adc_clk);
#endif /* end of IF_MODE block */

#ifdef ZIF_MODE
    /*   In Low_IF or Zero_IF: if 'adc_sampling_mode' ==  zif_ovr2 OR  zif_ovr4.
           digital_if_MHz = Tuner_IF (close to 0).
    */
       L1_Demod_Set_Digital_IF_MHz(demod, TUNER_IF_MHZ);
#endif /* end of ZIF_MODE block */

    L1_WRITE(demod, iq_adc_swap, iq_adc_swap_not_swapped);

    /* Initialization procedure */
    L1_WRITE(demod, chip_init,  chip_init_init);
    L1_WRITE(demod, start_init, start_init_run);

    init_ok  = L1_READ(demod, init_done);
    count = 0;
    while (init_ok != init_done_completed) {
        init_ok  = L1_READ(demod, init_done);
        if (count++ > 5) {
            TRACES_ERR (Can not complete the initialization procedure!);
            /*  <porting> Treat here a possible communication error! */
            return 0;
            break;
        }
    }
    TRACES_PRINTF("count = %d\n", count);

    L1_WRITE(demod, chip_init, chip_init_func);
    L1_WRITE(demod, rst_all,   rst_all_reset);

    /* demodulation parameters */
    L1_Demod_set_standard       (demod, Si2165->standard);
    L1_Demod_set_ber_depth      (demod, DEFAULT_BER_DEPTH);
    /* By default, freq_sync_range = '0' (it works as if at '50' = 50kHz). Changing this to 100kHz */
    L1_WRITE(demod, freq_sync_range         , 200);

    /* GPIOs */
    /* On the EVB, we set the GPIOs to monitor the FEC state and the TS errors */
    /* D1 monitors the FEC lock state           */
    L1_WRITE(demod, fecl0_e         , fecl0_e_enable);
    L1_WRITE(demod, gp0_en          , gp0_en_enable);
    L1_WRITE(demod, gpio0_tri       , gpio0_tri_normal);
    L1_WRITE(demod, gp0_t           , gp0_t_cmos);
    L1_WRITE(demod, gp0_sel         , gp0_sel_interrupt);
/*    L1_WRITE(demod, gp0_o           , gp0_o_low);*/    /* OK by default, no need to set it */
/*    L1_WRITE(demod, gp0_deltasigma  , 0);*/            /* OK by default, no need to set it */
    L1_WRITE(demod, gp0_p           , gp0_p_non_inverted);
    /* D2 monitors the TS errors                */
    L1_WRITE(demod, sel_gpio_ts_err , sel_gpio_ts_err_ts_err);
    L1_WRITE(demod, ts_err_tri      , ts_err_tri_normal);

    /* start DSP      */
    switch (Si2165->standard)
    {
        case standard_dvb_c: {
            L1_DVB_C_Re_Init(demod);
            break;
        }
        default:             {
            L1_DVB_T_Re_Init(demod);
            break;
        }
    }
    L1_Demod_TS_mode(demod,ts_data_mode_parallel);
    L1_Demod_reset(demod);
    return 1;
}

/**************************************************************************************************
  L1_Demod_set_standard function
  Use:      Standard selection function
            Used to set the standard inside the demodulator
  Returns:      the integer value corresponding to the final standard
  Comments:     Only valid standard values are accepted, otherwise it uses the default standard (
DVT-T)
  Parameter:   *demod          the demod handle
  Parameter:   standard        an integer value corresponding to the desired standard
  Variable:   Si2165->standard  the driver-internal standard
  Registers:   standard

**************************************************************************************************/
EMB_L1_MULTISTANDARD_API int    SLAB_WINAPI L1_Demod_set_standard   (L1_Context *demod, int standard) {

    /* to access Si2165 specific parameters, cast demod->specific as a Si2165_Context */
    Si2165_Context *Si2165;
    Si2165  = (Si2165_Context *) demod->specific;

    switch (standard)
    {
        case standard_dvb_c: break;
        default: standard = standard_dvb_t; break;
    }
    Si2165->standard = standard;
    L1_WRITE(demod, standard, Si2165->standard);
    return Si2165->standard;
}

/**************************************************************************************************
  L1_Demod_set_ber_depth function
  Use:        Ber depth setup function
              Used to set the demodulator register ber_pkt to a new value to change the ber depth
  Returns:    the current ber depth
  Behavior:   Logs the current ber depth rate in Si2165->ber_depth, because it is useful to
compute the BER
  Comments:   The value set is the number of TS packets used for the internal computing of bit
errors.
              This is merely an 'alias' function from ber_depth to ber_pkt, as we want to provide
               a meaningful function name for the ber depth modification
  Parameter: *demod          the demod handle
               ber_depth      the integer value corresponding to the desired ber depth
  Registers:  ber_pkt

***************************************************************************************************/
EMB_L1_COMMON_DEMOD_API int    SLAB_WINAPI L1_Demod_set_ber_depth    (L1_Context *demod, int
ber_depth) {

    /* to access Si2165 specific parameters, cast demod->specific as a Si2165_Context */
    Si2165_Context *Si2165;
    Si2165  = (Si2165_Context *) demod->specific;

    /* Store the requested ber_depth value, because                  */
    /*  it is used for BER computing                                 */
    /* This will avoid reading the ber_depth register to compute BER */
    Si2165->ber_depth = (int)L1_WRITE(demod, ber_pkt,ber_depth);

    return Si2165->ber_depth;
}

/**************************************************************************************************
  L1_Demod_current_bandwidth function
  Use:        bandwidth retrieval function
              Used to return the bandwidth value stored in the Si2165 structure
  Returns:    the value of the Si2165->bandwidth_MHz

***************************************************************************************************/
EMB_L1_COMMON_DEMOD_API int    SLAB_WINAPI L1_Demod_current_bandwidth        (L1_Context *demod) {

    /* to access Si2165 specific parameters, cast demod->specific as a Si2165_Context */
    Si2165_Context *Si2165;
    Si2165  = (Si2165_Context *) demod->specific;

    return Si2165->bandwidth_MHz;
}

/**************************************************************************************************
  L1_Demod_current_standard function
  Use:        standard retrieval function
              Used to return the standard value stored in the Si2165 structure
              This avoids reading the standard register
  Returns:    the value of the Si2165-> standard

***************************************************************************************************/
EMB_L1_COMMON_DEMOD_API int    SLAB_WINAPI L1_Demod_current_standard        (L1_Context *demod) {

    /* to access Si2165 specific parameters, cast demod->specific as a Si2165_Context */
    Si2165_Context *Si2165;
    Si2165  = (Si2165_Context *) demod->specific;

    return Si2165->standard;
}

/**************************************************************************************************
  L1_DVB_T_get_demod_lock  function
  Use:      DVB_T demod lock check function
            Used for retrieving the DVB_T demodulator's lock status
  Returns:  1 if the demodulator is locked
  Behavior: reads demod_lock_t and then returns 1 only if it means 'locked'
  Registers:    demod_lock_t
  Parameter:   *demod          the demod handle

**************************************************************************************************/
EMB_L1_DVB_T_DEMOD_API char   SLAB_WINAPI L1_DVB_T_get_demod_lock   (L1_Context *demod) {
    double demod_lock;
    demod_lock = (int)L1_READ(demod, demod_lock_t);
    if (demod_lock == demod_lock_t_locked) return 1;
    return 0;
}

/***********************************************************************************************************************
  L1_DVB_C_get_demod_lock  function
  Use:      DVB_C demod lock check function
            Used for retrieving the DVB_C demodulator's lock status
  Returns:  1 if the demodulator is locked
  Behavior: reads demod_lock_c and then returns 1 only if it means 'locked'
  Registers:    demod_lock_c
  Parameter:   *demod          the demod handle

***********************************************************************************************************************/
EMB_L1_DVB_C_DEMOD_API char   SLAB_WINAPI L1_DVB_C_get_demod_lock   (L1_Context *demod) {
    double demod_lock;
    demod_lock = (int)L1_READ(demod, demod_lock_c);
    if (demod_lock == demod_lock_c_locked) return 1;
    return 0;
}

/**************************************************************************************************
    L1_Demod_get_demod_lock function
  Use:      main demod lock state check function.
            Used for the selection of the correct get_demod_lock function for the current standard
  Returns:      1 if the demodulator is locked, 0 otherwise
  Parameter:   *demod          the demod handle
  Variable:   Si2165->standard the driver-internal standard

**************************************************************************************************/
EMB_L1_MULTISTANDARD_API char   SLAB_WINAPI L1_Demod_get_demod_lock (L1_Context *demod) {

    /* to access Si2165 specific parameters, cast demod->specific as a Si2165_Context */
    Si2165_Context *Si2165;
    Si2165  = (Si2165_Context *) demod->specific;

    switch (Si2165->standard)
    {
        case standard_dvb_t: return L1_DVB_T_get_demod_lock(demod);
        case standard_dvb_c: return L1_DVB_C_get_demod_lock(demod);
    }
    return 0;
}


/**************************************************************************************************
  L1_Demod_SW_Init function
  Use:        software initialisation function
              Used to initialize the software context
  Returns:    1 if the software initialization has been correct
  Comments:   It should be called first and once only when starting the application
  Parameter:   *demod          the demod handle
  Parameter:    add            the initial demod address
  Parameter:    standard       the initial standard

***************************************************************************************************/
EMB_L1_COMMON_DEMOD_API char   SLAB_WINAPI L1_Demod_SW_Init           (L1_Context **ppDemod, unsigned int add, int standard) {
    L1_Context * demod;
    Si2165_Context *Si2165;
    /* demod context instantiation */
    *ppDemod   = (L1_Context *) malloc(sizeof(L1_Context));

    /* Checking proper context allocation (requires customer-specific error handling) */
    if (*ppDemod == NULL) {
        ALLOCATION_ERROR_MESSAGE("Could not allocate Si2165 context !\n");
        return 0;
    }
    /* for easier access to demod members, use a nicer name (only during this function) */
    demod = *ppDemod;

    demod->i2c = (L0_Context *) malloc(sizeof(L0_Context));
    /* Checking proper context allocation (requires customer-specific error handling) */
    if (demod->i2c == NULL) {
        ALLOCATION_ERROR_MESSAGE("Could not allocate demod i2c context !\n");
        return 0;
    }
    /* <Porting> <modification required> Use your Layer 0 functions to set the I2C Address */
    demod->i2c->address   = add;
    demod->i2c->indexSize =   2;

    /* to access Si2165 specific parameters, cast demod->specific as a Si2165_Context */
    demod->specific = (Si2165_Context *)malloc(sizeof(Si2165_Context ));
    Si2165  = (Si2165_Context *) demod->specific;
    /* demod default parameters setup */
    Si2165->ber_depth        = 0;
    Si2165->standard         = standard;
    Si2165->FVCO             = 0;
    Si2165->adc_clk          = 0;
    Si2165->sys_clk          = 0;
    Si2165->FE_clk           = 0;
    Si2165->dvb_rate         = 0;
    Si2165->digital_if_MHz   = 0;
    Si2165->symbol_rate_MHz  = 0;
    Si2165->bandwidth_MHz    = 0;
    Si2165->ber_depth        = 0;
    return 1;
}


/**************************************************************************************************
  L2_Si2165_EVB_SW_Init function
  Use:        software initialization function
              Used to initialize the Si2165 and tuner structures
  Behavior:   This function performs all the steps necessary to initialize the Si2165 and tuner
instances
  Parameter:  **ppFrontEnd, a pointer to the L2_Context context to be initialized
  Parameter:  demodAdd, the I2C address of the demod
  Parameter:  standard, the initial standard set in the demod
  Parameter:  tunerAdd, the I2C address of the tuner
  Comments:     It MUST be called first and once before using any other function.
                It can be used to build a multi-demod/multi-tuner application, if called several
times from the upper layer with different pointers and addresses
                After execution, all demod and tuner functions are accessible.
***************************************************************************************************/

EMB_L2_Si2165_EVB_API char WINAPI L2_Si2165_EVB_SW_Init (L2_Context  **ppFrontEnd, int demodAdd, int standard, int tunerAdd) {
    L2_Context  *front_end;

    /* Front-end structure initialization */
    *ppFrontEnd = (L2_Context  *) malloc(sizeof(L2_Context));

    /* for easier access to front_end members, use a nicer name (only during this function) */
    front_end = *ppFrontEnd;

    /* <porting> Checking proper context allocation (requires customer-specific error handling) */
    if (front_end == NULL) {
        /* <porting> replace this by your own eror handling when it is not possible to allocate the demod i2c context */
        ALLOCATION_ERROR_MESSAGE("Could not allocate front_end context !\n");
        return 0;
    }

    /* to make sure the specifics will be correctly initialized, set the pointer to NULL */
    front_end->specific = NULL;

    /* Si2165 initialization */
    L1_Demod_SW_Init(&(front_end->demod), demodAdd, standard);
    #ifdef SILABS_I2C
    L0_Connect   (front_end->demod->i2c, USB);
    #else
    L0_Connect   (front_end->demod->i2c, CUSTOMER);
    #endif  /* SILABS_I2C */

    /* Tuner initialization */
    /* <porting> replace L1_RF_TUNER_Init by the final tuner's initialization function */
    /* <porting> replace L0_Connect by whatever is needed to connect the tuner */
#if APP_TUNER_OFDM_MAX3580_SI2165
    L1_RF_TUNER_Init (&(front_end->tuner), tunerAdd);
    /* <Porting> <removable> Your final application will not have a simulation mode, hence it will always work in 'connected' mode */
    #ifdef SILABS_I2C
    L0_Connect   (front_end->tuner->i2c, USB);
    #else
    L0_Connect   (front_end->tuner->i2c, CUSTOMER);
    #endif  /* SILABS_I2C */
#endif

    return 1;
}

/**************************************************************************************************
  L2_Tune function
  Use:        tuner current frequency retrieval function
              Used to retrieve the current RF from the tuner's driver.
  Porting:    Replace the internal TUNER function calls by the final tuner's corresponding calls
  Comments:   If the tuner is connected via the demodulator's I2C switch, enabling/disabling the
i2c_passthru is required before/after tuning.
  Behavior:   This function closes the Si2165's I2C switch then tunes and finally reopens the I2C
switch
  Parameter:  *front_end->tuner, the tuner handle
***************************************************************************************************/
EMB_L2_Si2165_EVB_API int  WINAPI L2_Tune      (int nim_id, L2_Context   *front_end, int rf) {
    int res;
////    L1_WRITE(front_end->demod, i2c_passthru      , i2c_passthru_enabled);
    /* <porting> replace L1_RF_TUNER_Tune by whatever is needed to tune at rf (channel center frequency, in Hz) */
#if APP_TUNER_OFDM_MAX3580_SI2165
    res = L1_RF_TUNER_Tune     (front_end->tuner, rf);
#endif
#if APP_TUNER_OFDM_EN4020_SI2165 || APP_TUNER_QAM_EN4020_SI2165
    ctmp[nim_id].FchanHz = rf;//666000000;
    entr_set_chan(&ctmp[nim_id]);
#endif

////    L1_WRITE(front_end->demod, i2c_passthru      , i2c_passthru_disabled);
    return res;
}

/**************************************************************************************************
  tune function
  Use:        user-level tune function
              Used to have the user enter the desired tuning frequency, then call the L2_RF_Tune
function
  Behavior:   Asks the user to enter the tune freq, then calls the underlying tune function
***************************************************************************************************/
/*
void tune(L2_Context   *front_end) {
    int rf;
    printf("tuning frequency (in Hz)? ");
    scanf("%d",&rf);
    printf("\nTuning at %d\n",rf);
    L2_Tune(front_end, rf);
}
*/

/**************************************************************************************************
  L2_Get_maxRF function
  Use:        tuner maximum frequency retrieval function
              Used to retrieve the maximum possible RF from the tuner's driver.
  Porting:    Replace the internal TUNER function calls by the final tuner's corresponding calls
  Behavior:   This function does not need to activate the Si2165's I2C switch, as the required
value is part of the tuner's structure
  Parameter:  *front_end->tuner, the tuner handle
***************************************************************************************************/
EMB_L2_Si2165_EVB_API int  WINAPI L2_Get_maxRF (L2_Context   *front_end)   {
    /* <porting> replace L1_RF_TUNER_Get_maxRF by whatever is needed to retrieve the tuner's max RF */
#if APP_TUNER_OFDM_MAX3580_SI2165
    return L1_RF_TUNER_Get_maxRF (front_end->tuner);
#endif
#if APP_TUNER_OFDM_EN4020_SI2165 || APP_TUNER_QAM_EN4020_SI2165
    return (860000000);
#endif
}

/**************************************************************************************************
  L2_Scan_Table_Init function
  Use:      scan table initialization function
            Used to allocate the scan table structure
  Parameter:
***************************************************************************************************/
EMB_L2_Si2165_EVB_API Scan_Context*  WINAPI L2_Scan_Table_Init (L2_Context   *front_end) {
    Scan_Context *scan;
    if (front_end->specific == NULL) {
        /* scan context allocation */
        front_end->specific = (Scan_Context*) malloc(sizeof(Scan_Context));

        /* to access front end specific parameters, cast front_end->specific as a Scan_Context */
        scan  = (Scan_Context *) front_end->specific;
/*        TRACES_PRINTF("L2_Scan_Table_Init : front_end->specific == NULL, allocating memory       at 0x%x\n", scan);*/

        scan->carrier_count       = 0;
        scan->rf                  = (double*) malloc(sizeof(double)*scan->carrier_count);
        scan->standard            = (int*)    malloc(sizeof(int   )*scan->carrier_count);
        scan->carrier_details     = (void**)  malloc(sizeof(void* )*scan->carrier_count);

    } else {
        scan  = (Scan_Context *) front_end->specific;
/*        TRACES_PRINTF("L2_Scan_Table_Init : front_end->specific != NULL, returning previous pointer 0x%x\n", scan);*/
    }
    return scan;
}

/**************************************************************************************************
  L2_Scan_Table_Clear function
  Use:      scan table 'flush' function
            Used to remove all carriers from the scan table
  Returns: 1 if OK, 0 if no scan table exists
***************************************************************************************************/
EMB_L2_Si2165_EVB_API char      WINAPI L2_Scan_Table_Clear (L2_Context   *front_end) {
    /* to access front end specific parameters, cast front_end->specific as a Scan_Context */
    Scan_Context *scan;
    DVB_T_Status *Tstatus;
    DVB_C_Status *Cstatus;

    scan  = L2_Scan_Table_Init(front_end);

    if(scan != NULL) {
        int i;
        for (i=0; i < scan->carrier_count; i++) {
           switch (scan->standard[i]) {
                case standard_dvb_t: {
                     Tstatus = (DVB_T_Status *) scan->carrier_details[i];
                    free(Tstatus);
                    break;
                }
                case standard_dvb_c: {
                    Cstatus = (DVB_C_Status *) scan->carrier_details[i];
                    free(Cstatus);
                    break;
                }
                default:{}
           }
           free(scan->carrier_details[i]);
        }
        scan->carrier_count       = 0;
        scan->rf                  = (double*) realloc(scan->rf,              sizeof(double)*scan->carrier_count);
        scan->standard            = (int*)    realloc(scan->standard,        sizeof(int   )*scan->carrier_count);
        scan->carrier_details     = (void**)  realloc(scan->carrier_details, sizeof(void* )*scan->carrier_count);
        return 1;
    }
    return 0;
}


/**************************************************************************************************
  L2_Get_RF function
  Use:        tuner current frequency retrieval function
              Used to retrieve the current RF from the tuner's driver.
  Porting:    Replace the internal TUNER function calls by the final tuner's corresponding calls
  Behavior:   This function does not need to activate the Si2165's I2C switch, as the required
value is part of the tuner's structure
  Parameter:  *front_end->tuner, the tuner handle
***************************************************************************************************/
EMB_L2_Si2165_EVB_API int  WINAPI L2_Get_RF    (int nim_id, L2_Context   *front_end)   {
    /* <porting> replace L1_RF_TUNER_Get_RF by whatever is needed to retrieve the tuner's current RF */
#if APP_TUNER_OFDM_MAX3580_SI2165
    return L1_RF_TUNER_Get_RF   (front_end->tuner);
#endif
#if APP_TUNER_OFDM_EN4020_SI2165 || APP_TUNER_QAM_EN4020_SI2165
    return (ctmp[nim_id].FchanHz);
#endif
}


/**************************************************************************************************
  L2_Scan_Table_AddOneCarrier function
  Use:      carrier addition function
            Used to add a carrier in the scan table
  Returns: The index of the new carrier
***************************************************************************************************/
EMB_L2_Si2165_EVB_API int      WINAPI L2_Scan_Table_AddOneCarrier (L2_Context   *front_end) {
    /* to access front end specific parameters, cast front_end->specific as a Scan_Context */
    Scan_Context *scan;
    scan  = L2_Scan_Table_Init(front_end);
    scan->carrier_count       = scan->carrier_count + 1;
    scan->rf                  = (double*) realloc(scan->rf,              sizeof(double)*scan->carrier_count);
    scan->standard            = (int*)    realloc(scan->standard,        sizeof(int  ) *scan->carrier_count);
    scan->carrier_details     = (void**)  realloc(scan->carrier_details, sizeof(void*) *scan->carrier_count);
    scan->carrier_details     = (void**)  realloc(scan->carrier_details, sizeof(void*) *scan->carrier_count);
    return scan->carrier_count -1;
}


/**************************************************************************************************
  L2_Scan_DVB_T function
  Use:      DVB-T scan function
            Used to scan the RF band between the given limits
  Parameter: minRF the start frequency for the scan
  Parameter: maxRF the stop frequency for the scan
  Behavior: This function loops between minRF and maxRF and return the number of carriers found.
            Each carrier's data is stored in the scan table, with the corresponding DVB-T details.
            A timeout is used in the core loop to make sure it will not hang up.
            N.B.: This should never happen, as the demodulator should always return a
check_signal value different from 'searching' in less than a second.
            Nevertheless, it is useful when testing the code in 'simulation' mode.
***************************************************************************************************/
#if 0
EMB_L2_Si2165_EVB_API int       WINAPI L2_Scan_DVB_T (L2_Context   *front_end, double minRF,double maxRF) {
    /* to access front end specific parameters, cast front_end->specific as a Scan_Context */
    Scan_Context *scan;
    int scan_step, current_rf, min, max, corrected_rf, rounded_rf;
    int carrier_index, detected;
    int check_signal;
    int check_start_time;
    DVB_T_Status  *status;

    /* Initialize the scan table if not already done */
    scan  = L2_Scan_Table_Init(front_end);

    /* Do not scan if the scan table can not be initialized */
    if (scan == NULL) return 0;

    scan_step = L1_Demod_current_bandwidth(front_end->demod)*1000000;
    min       = (int)minRF;
    max       = (int)maxRF;
    detected  = 0;
    /* Widen the frequency locking range to be able to lock even with offsets */
    L1_WRITE(front_end->demod, freq_sync_range, 210);

   /* TRACES_PRINTF("scanning from %d to %d in steps of %d\n\n", min, max, scan_step);*/

    for (current_rf = min; current_rf <= max; current_rf = current_rf + scan_step) {
        L2_Tune(front_end, current_rf);
        /* <porting> remove the printf lines used to follow the scan process in the demonstration application*/
        TRACES_PRINTF("\r%100s\r%15d   ", " ", current_rf);
        /* Allow 20 ms for the AGCs to stabilize after tuning */
        system_wait(20);
        L1_WRITE(front_end->demod, start_synchro, start_synchro_start);
        /* Store the start time to be able to quit if the demod doesn't answer for any reason */
/*        check_start_time = time_now();*/
        while ( 1 ) {
            /*   Carrier detection loop. NOTE: The test order is important !   */
            if (L1_READ(front_end->demod, start_synchro) == start_synchro_running) {
                check_signal = (int)L1_READ(front_end->demod, check_signal);
                /* Stay in this loop as long as the demod says it is 'searching' */
                if (check_signal != check_signal_searching) break;
            }
            /* If 'searching' for more than 4 seconds, inform the user, try to re_init and abort if unsuccessful */
            /* <porting> This needs to be modified as required for the final application,        */
            /* <porting> depending on the expected behavior in such case                         */
            /* <porting> The demonstration API uses printf for important messages such as errors */
            if (/*time_now() - */check_start_time >= DVB_T_SCAN_TIMEOUT) {
               TREAT_ERROR_MESSAGE(  "\n!*!*!*!*!     DVB_T timeout HIT     !*!*!*!*!*! at %d\n", current_rf);
               if (L1_READ(front_end->demod, chip_mode) != chip_mode_pll_xtal) {
                   TREAT_ERROR_MESSAGE("!*!*!*!*!     Board disconnected, I will try to reconnect   !*!*!*!*!*!\n");
////////////////////system("pause");
                   L1_Demod_Re_Init(front_end->demod);
                   if (L1_READ(front_end->demod, chip_mode) != chip_mode_pll_xtal) {
                       TREAT_ERROR_MESSAGE("!*!*!*!*!     Board still disconnected, sorry  !*!*!*!*!*!\n");
                       return detected;
                    }
               }
               break;
            }
            /* To avoid taking all the Host processor's resources, leave 5 ms between each loop */
            system_wait(5);
        }
        /* If 'digital', a carrier has been detected */
        if (check_signal == check_signal_digital) {
            /* <porting> remove the printf lines used to follow the scan process in the demonstration application*/
            TRACES_PRINTF(" carrier detected\n");
            /* Make room in the scan table for the new carrier */
            carrier_index = L2_Scan_Table_AddOneCarrier(front_end);
            /* Retrieve the final rf using the demod's frequency offset */
            corrected_rf  = L2_Get_RF(front_end) - (int)L1_Demod_get_freq_offset(front_end->demod);
            /* Round the final rf in steps of 1/6 MHz (because it is what users expect)*/
            rounded_rf    = (int)((int)((corrected_rf/1e6*6.0) + 0.5)*1e6/6.0);
            /* Store the rf in the scan table */
            scan->rf[carrier_index]       = rounded_rf;
            /* Store the carrier standard in the scan table */
            scan->standard[carrier_index] = L1_Demod_current_standard(front_end->demod);
            /* Store the carrier status in the scan table */
            L1_DVB_T_get_status(front_end->demod, &status);
            scan->carrier_details[carrier_index] = (DVB_T_Status *)malloc(sizeof(DVB_T_Status));
            scan->carrier_details[carrier_index] = status;
            detected++;
        }
    }
    /* <porting> remove the printf lines used to follow the scan process in the demonstration application*/
    TRACES_PRINTF("\n");
    return detected;
}
#endif


#if 0
/***********************************************************************************************************************
  L1_DVB_C_get_constellation function
  Use:        DVB_C constellation checking function
              Used to read the DVB_C constellation
  Returns:    the value of the constellation register
  Registers:    constellation
  Parameter:   *demod          the demod handle

***********************************************************************************************************************/
EMB_L1_DVB_C_DEMOD_API int    SLAB_WINAPI L1_DVB_C_get_constellation(L1_Context *demod) {
    return (int)L1_READ(demod, constellation);}

/***********************************************************************************************************************
  L1_DVB_C_get_spectral_inversion function
  Use:        DVB_C spectral inversion checking function
              Used to read the DVB_C spectral inversion
  Returns:    the value of the spectral inversion
  Registers:    ps_ambig_out
  Parameter:   *demod          the demod handle

***********************************************************************************************************************/
EMB_L1_DVB_C_DEMOD_API int    SLAB_WINAPI L1_DVB_C_get_spectral_inversion(L1_Context *demod) {
    return (int)L1_READ(demod, ps_ambig_out);}

/***********************************************************************************************************************
  L1_DVB_C_get_status function
  Use:      DVB_C statusing function
            Used to retrieve the DVB_C status information in a structure
  Returns:      0 if the structure initialization fails, 1 otherwise
  Behavior:     Sets all members of the DVB_C_Status structure provided during function call to
the current
                 hardware values by calling the corresponding statusing functions and returns
  Parameter:   *demod          the demod handle
                **status        pointer to a DVB_C_Status structure used to store the status information

***********************************************************************************************************************/
EMB_L1_DVB_C_DEMOD_API char   SLAB_WINAPI L1_DVB_C_get_status       (L1_Context *demod, DVB_C_Status **status) {
    *status = (DVB_C_Status  *) malloc(sizeof(DVB_C_Status));
    if (*status == NULL ) return 0;
    (*status)->constellation      = L1_DVB_C_get_constellation      (demod);
    (*status)->spectral_inversion = L1_DVB_C_get_spectral_inversion (demod);
     return 1;
}

/************************************************************************************************************************
  L2_Scan_DVB_C function
  Use:      DVB-C scan function
            Used to scan the RF band between the given limits
  Parameter: minRF the start frequency for the scan
  Parameter: maxRF the stop frequency for the scan
  Behavior: This function loops between minRF and maxRF and return the number of carriers found.
            Each carrier's data is stored in the scan table, with the corresponding DVB-C details.
  Comments: If the tuner is connected via the demodulator's I2C switch, enabling/disabling the
i2c_passthru is required before/after tuning.
************************************************************************************************************************/
EMB_L2_Si2165_EVB_API int       WINAPI L2_Scan_DVB_C (L2_Context   *front_end, double minRF,double maxRF, int scan_step) {
    /* to access front end specific parameters, cast front_end->specific as a Scan_Context */
    Scan_Context *scan;
    int current_rf, min, max, corrected_rf, rounded_rf;
    int carrier_index,detected;
    int scan_state;
    int check_start_time;
    int scan_timeout;
    int i;
    int previous_lock;
    unsigned char locked;
    DVB_C_Status  *status;
    unsigned char constel_order[5]= {constellation_qam64, constellation_qam16, constellation_qam32,     constellation_qam128, constellation_qam256};

    /* Initialize the scan table if not already done */
    scan  = L2_Scan_Table_Init(front_end);

    /* Do not scan if the scan table can not be initialized */
    if (scan == NULL) return 0;

/*    scan_timeout = 550-50*L1_Demod_current_symbol_rate;*/

    //scan_timeout = 550 - 50*L1_Demod_current_symbol_rate(front_end->demod);
    //TRACES_PRINTF("timeout %d ms\n",scan_timeout);

    min       = (int)minRF;
    max       = (int)maxRF;
    detected  = 0;

    TRACES_PRINTF("scanning from %d to %d in steps of %d Hz\n\n", min, max, scan_step);

    for (current_rf = min; current_rf <= max; current_rf = current_rf + scan_step) {
        L2_Tune(front_end, current_rf);
        /* <porting> remove the printf lines used to follow the scan process in the demonstration application*/
        TRACES_PRINTF("\r%100s\r%15d   ", " ", current_rf);
        /* Allow 20 ms for the AGCs to stabilize after tuning */
        system_wait(20);
        /* Store the start time to be able to quit if the demod doesn't answer for any reason */
        while ( 1 ) {
            for (i=0; i<5 ; i++ ) {
                L1_WRITE(front_end->demod, constellation, constel_order[i]);
                L1_WRITE(front_end->demod, rst_demod,     rst_demod_reset);
                L1_WRITE(front_end->demod, rst_equal,     rst_equal_reset);
                L1_WRITE(front_end->demod, rst_fec,       rst_fec_reset);
                //check_start_time = system_time();
                /*byte_reg[EPB_ADDR_RST+OFFSET_BYTE_15_8]=(UINT8) (0xF1);*/ /*rst fec, equal, demod (active low)*/
                while (1) {
                    previous_lock = L1_READ(front_end->demod, fec_lock);
                    /* To avoid taking all the Host processor's resources, leave 5 ms between each loop */
                    system_wait(5);
                    if (L1_READ(front_end->demod, fec_lock)==fec_lock_locked && previous_lock == fec_lock_locked) {
                        locked = 1;
                        break; /* exit constellation loop output because locked */
                    }
                    //if ((system_time() - check_start_time) >= scan_timeout) {
                    //    locked = 0;
                    //    break; /* exit lock test loop because either locked or timeout reached */
                    }
                }
                if (L1_READ(front_end->demod, fec_lock)==fec_lock_locked) {
                    locked = 1;
                    break; /* exit constellation loop output because locked */
                } else {
                    locked = 0;
                }
            }
            break; /* exit loop for this frequency */
        }
        /* If 'locked', a carrier has been detected */
        if (locked) {
            /* <porting> remove the printf lines used to follow the scan process in the demonstration application*/
            TRACES_PRINTF(" carrier detected\n");
            /* Make room in the scan table for the new carrier */
            carrier_index = L2_Scan_Table_AddOneCarrier(front_end);
            /* Retrieve the final rf using the demod's frequency offset */
            corrected_rf  = L2_Get_RF(front_end) - (int)L1_Demod_get_freq_offset(front_end->demod);
            /* Round the final rf in steps of 1/6 MHz (because it is what users expect)*/
            rounded_rf    = (int)((int)((corrected_rf/1e6*6.0) + 0.5)*1e6/6.0);
            /* Store the rf in the scan table */
            scan->rf[carrier_index]       = rounded_rf;
            /* Store the carrier standard in the scan table */
            scan->standard[carrier_index] = L1_Demod_current_standard(front_end->demod);
            /* Store the carrier status in the scan table */
            L1_DVB_C_get_status(front_end->demod, &status);
            scan->carrier_details[carrier_index] = (DVB_C_Status *)malloc(sizeof(DVB_C_Status));
            scan->carrier_details[carrier_index] = status;
            detected++;
        }
    /* <porting> remove the printf lines used to follow the scan process in the demonstration application*/
    TRACES_PRINTF("\n");
    return detected;
}

/**************************************************************************************************
  L2_Scan function
  Use:      Scan function selection, depending on the current standard
            Used to call the scanning function for the current standard
  Parameter: minRF the start frequency for the scan
  Parameter: maxRF the stop frequency for the scan
  Behavior: This function clears the carriers table before scanning.
            This could be changed, if several scans are required (such as one scan per standard,
or scanning by RF band)
***************************************************************************************************/
EMB_L2_Si2165_EVB_API int       WINAPI L2_Scan (L2_Context   *front_end, double minRF, double maxRF, int scan_step) {
    TRACES_USE(L2_Scan);

    /* To start a new scan, clear the previous scan results */
    L2_Scan_Table_Clear(front_end);

    /* Select the scan routine depending on the current standard */
    switch (L1_Demod_current_standard(front_end->demod)) {
       // case standard_dvb_t: return L2_Scan_DVB_T(front_end, minRF, maxRF);
        case standard_dvb_c: return L2_Scan_DVB_C(front_end, minRF, maxRF, scan_step);
        default:{}
    }

    return 0;
}
#endif
/**************************************************************************************************
  L1_DVB_T_format_text_status function
  Use:      Text status formatting function
            Used to format the DVB_T demodulator status information in a text
  Porting:  Convenient during software development. Will generally not be part of the final
application.
  Returns:      The status text
  Behavior:     Fills a char array with the text-formatted status information
  Comments:     Called after retrieving the status information using L1_DVB_T_get_status, or to
display a carrier status after scanning
  Parameter:   status          the demod status structure
  Parameter:   formatted_status the text string to fill

***************************************************************************************************/
#ifdef    COMMAND_LINE_APPLICATION
EMB_L1_DVB_T_DEMOD_API char  SLAB_WINAPI L1_DVB_T_format_text_status  (DVB_T_Status  *status, char** formatted_status) {
    *formatted_status = (char*) malloc(sizeof(char)*1000);
    if (formatted_status == NULL) return 0;

    sprintf(*formatted_status, "--- DVB_T status ---\n");
    sprintf(*formatted_status, "%s stream          %3d", *formatted_status, status->stream);
    switch(status->stream)
    {
        case stream_hp:
            sprintf(*formatted_status, "%s : hp\n", *formatted_status); break;
        case stream_lp:
            sprintf(*formatted_status, "%s : lp\n", *formatted_status); break;
        default:
            sprintf(*formatted_status, "%s : UNKNOWN!\n", *formatted_status);
    }

    sprintf(*formatted_status, "%s fft_mode        %3d", *formatted_status, status->fft_mode);
    switch(status->fft_mode)
    {
        case auto_fft_mode_2k:
            sprintf(*formatted_status, "%s : 2k\n", *formatted_status); break;
        case auto_fft_mode_4k:
            sprintf(*formatted_status, "%s : 4k\n", *formatted_status); break;
        case auto_fft_mode_8k:
            sprintf(*formatted_status, "%s : 8k\n", *formatted_status); break;
        default:
            sprintf(*formatted_status, "%s : UNKNOWN!\n", *formatted_status);
    }

    sprintf(*formatted_status, "%s guard_interval  %3d", *formatted_status, status->guard_interval);
    switch(status->guard_interval)
    {
        case auto_guard_int_1_32:
            sprintf(*formatted_status, "%s : 1/32\n", *formatted_status); break;
        case auto_guard_int_1_16:
            sprintf(*formatted_status, "%s : 1/16\n", *formatted_status); break;
        case auto_guard_int_1_8:
            sprintf(*formatted_status, "%s : 1/8\n", *formatted_status); break;
        case auto_guard_int_1_4:
            sprintf(*formatted_status, "%s : 1/4\n", *formatted_status); break;
        default:
            sprintf(*formatted_status, "%s : UNKNOWN!\n", *formatted_status);
    }

    sprintf(*formatted_status, "%s constellation   %3d", *formatted_status, status->constellation);
    switch(status->constellation)
    {
        case auto_constellation_qpsk:
            sprintf(*formatted_status, "%s : QPSK\n", *formatted_status); break;
        case auto_constellation_qam16:
            sprintf(*formatted_status, "%s : QAM16\n", *formatted_status); break;
        case auto_constellation_qam64:
            sprintf(*formatted_status, "%s : QAM64\n", *formatted_status); break;
        default:
            sprintf(*formatted_status, "%s : UNKNOWN!\n", *formatted_status);
    }

    sprintf(*formatted_status, "%s hierarchy       %3d", *formatted_status, status->hierarchy);
    switch(status->hierarchy)
    {
        case auto_hierarchy_none:
            sprintf(*formatted_status, "%s : none\n", *formatted_status); break;
        case auto_hierarchy_alfa1:
            sprintf(*formatted_status, "%s : Alpha_1\n", *formatted_status); break;
        case auto_hierarchy_alfa2:
            sprintf(*formatted_status, "%s : Alpha_2\n", *formatted_status); break;
        case auto_hierarchy_alfa4:
            sprintf(*formatted_status, "%s : Alpha_4\n", *formatted_status); break;
        default:
            sprintf(*formatted_status, "%s : UNKNOWN!\n", *formatted_status);
    }

    sprintf(*formatted_status, "%s code_rate_hp    %3d", *formatted_status, status->code_rate_hp);
    switch(status->code_rate_hp)
    {
        case auto_rate_hp_1_2:
            sprintf(*formatted_status, "%s : 1/2\n", *formatted_status); break;
        case auto_rate_hp_2_3:
            sprintf(*formatted_status, "%s : 2/3\n", *formatted_status); break;
        case auto_rate_hp_3_4:
            sprintf(*formatted_status, "%s : 3/4\n", *formatted_status); break;
        case auto_rate_hp_5_6:
            sprintf(*formatted_status, "%s : 5/6\n", *formatted_status); break;
        case auto_rate_hp_7_8:
            sprintf(*formatted_status, "%s : 7/8\n", *formatted_status); break;
        default:
            sprintf(*formatted_status, "%s : UNKNOWN!\n", *formatted_status);
    }

    sprintf(*formatted_status, "%s code_rate_lp    %3d", *formatted_status, status->code_rate_lp);
    switch(status->code_rate_lp)
    {
        case auto_rate_lp_1_2:
            sprintf(*formatted_status, "%s : 1/2\n", *formatted_status); break;
        case auto_rate_lp_2_3:
            sprintf(*formatted_status, "%s : 2/3\n", *formatted_status); break;
        case auto_rate_lp_3_4:
            sprintf(*formatted_status, "%s : 3/4\n", *formatted_status); break;
        case auto_rate_lp_5_6:
            sprintf(*formatted_status, "%s : 5/6\n", *formatted_status); break;
        case auto_rate_lp_7_8:
            sprintf(*formatted_status, "%s : 7/8\n", *formatted_status); break;
        default:
            sprintf(*formatted_status, "%s : UNKNOWN!\n", *formatted_status);
    }
    TRACES_PRINTF("%s\n",*formatted_status);
    return 1;
}
#endif /* COMMAND_LINE_APPLICATION */
/***********************************************************************************************************************
  L1_DVB_C_format_text_status function
  Use:      text status formatting function
            Used to format the DVB_C demodulator status information in a text
  Porting:  Convenient during software development. Will generally not be part of the final
application.
  Returns:      The status text
  Behavior:     Fills a char array with the text-formatted status information
  Comments:     Called after retrieving the status information using L1_DVB_C_get_status, or to
display a carrier status after scanning
  Parameter:   status          the demod status structure
  Parameter:   formatted_status the text string to fill

***********************************************************************************************************************/
#ifdef    COMMAND_LINE_APPLICATION
EMB_L1_DVB_C_DEMOD_API char  SLAB_WINAPI L1_DVB_C_format_text_status  (DVB_C_Status  *status, char** formatted_status) {
    *formatted_status = (char*) malloc(sizeof(char)*1000);
    if (formatted_status == NULL) return 0;

    sprintf(*formatted_status, "--- DVB_C status ---\n");
    sprintf(*formatted_status, "%s constellation   %3d", *formatted_status, status->constellation);
    switch(status->constellation)
    {
        case constellation_qpsk:
            sprintf(*formatted_status, "%s : QPSK\n", *formatted_status); break;
        case constellation_qam16:
            sprintf(*formatted_status, "%s : QAM16\n", *formatted_status); break;
        case constellation_qam32:
            sprintf(*formatted_status, "%s : QAM32\n", *formatted_status); break;
        case constellation_qam64:
            sprintf(*formatted_status, "%s : QAM64\n", *formatted_status); break;
        case constellation_qam128:
            sprintf(*formatted_status, "%s : QAM128\n", *formatted_status); break;
        case constellation_qam256:
            sprintf(*formatted_status, "%s : QAM256n", *formatted_status); break;
        default:
            sprintf(*formatted_status, "%s : UNKNOWN!\n", *formatted_status);
    }

    TRACES_PRINTF("%s\n",*formatted_status);
    return 1;
}
#endif /* COMMAND_LINE_APPLICATION */

/**************************************************************************************************
  L2_Scan_Table_Infos function
  Use:      scan table display function
            Used to retrieve the carrier information from the scan table, based on the carrier
index
  Parameters: i The index of the carrier
  Returns: a text string containing the (textual) information for the selected carrier
  Comments: Mostly useful for debugging while building the application
***************************************************************************************************/
#ifdef    COMMAND_LINE_APPLICATION

EMB_L2_Si2165_EVB_API char     WINAPI L2_Scan_Table_Infos (L2_Context   *front_end, int i, char **text) {
    char* carrier_text;
    /* to access front end specific parameters, cast front_end->specific as a Scan_Context */
    Scan_Context *scan;

    char *formatted_status;
    scan  = L2_Scan_Table_Init(front_end);

    *text = (char*) malloc(sizeof(char)*20000);
    carrier_text = *text;
    sprintf(carrier_text, "no carrier at index %d: %d carriers logged\n",i, scan->carrier_count);
    if(scan != NULL) {
        if (i < scan->carrier_count) {
            switch (scan->standard[i]) {
                case standard_dvb_t: {
                    L1_DVB_T_format_text_status(scan->carrier_details[i], &formatted_status);
                    sprintf(carrier_text, "DVB-T at %f MHz\n %s", (scan->rf[i])/1000000.0, formatted_status);
                    break;
                }
                case standard_dvb_c: {
                    L1_DVB_C_format_text_status(scan->carrier_details[i], &formatted_status);
                    sprintf(carrier_text, "DVB-C at %f MHz\n %s", (scan->rf[i])/1000000.0, formatted_status);
                    break;
                }
                default:{
                    sprintf(carrier_text, "unknown standard: %d\n", scan->standard[0]);
                }
            }
            return 1;
        }
    }
    return 0;
}
#endif

/**************************************************************************************************
  scan function
  Use:        scan start function
              Used to show how to scan the RF and display the carriers detected
  Behavior:   Launches the autoscan from minrf to maxrf, then displays the parameters related to
each detected carrier
**************************************************************************************************/
#if 0
void scan (L2_Context   *front_end) {
    char * txt;
    int minRF, maxRF;
    int nbCarriers;
    int i;
    int scan_step;

    int startTime, stopTime;

    txt   = (char*) malloc(sizeof(char)*1000);
/*    startTime = time_now();*/

    /*printf("Locking Time is %d ms\n",time_minus(time_now(),locking_time)/(clock_t)(ST_GetClocksPerSecond()/1000));*/

    //  using L2_Get_minRF(front_end) may NOT work, if the minRF it returns is not down from the first channel by a multiple of the scan step 
    // i.e. if the first carrier is at 626000000 and minRF = 470000000, then (626000000 - minRF) = 19.5*8MHz 
    //                    this only works if minRF is set at 474000000, thus (626000000 - minRF) = 19.0*8MHz 

    minRF = 474000000;
    maxRF = L2_Get_maxRF(front_end);
    scan_step = 0;

    printf("\nScan between %d and %d\n",minRF, maxRF);
    printf("\nScan...\n\n");
    if (L1_Demod_current_standard(front_end->demod)==standard_dvb_c) {
        printf("scan_step (in Hz)? ");
        scanf("%d",&scan_step);
    }

/*    startTime = time_now();*/
    nbCarriers = L2_Scan(front_end, minRF, maxRF, scan_step);
    /*stopTime   = system_time();*/

/*    TRACES_PRINTF("\n\nscan finds %d carriers in %.2f s\n\n", nbCarriers, time_minus(time_now(),startTime)/(clock_t)(ST_GetClocksPerSecond()/1000));*/

    for (i = 0; i< nbCarriers; i++) {
        L2_Scan_Table_Infos(front_end, i, &txt);
        printf("carrier %d: %s\n", i, txt);
    }
}
#endif
/**************************************************************************************************
  L1_DVB_C_get_C_N_dB  function
  Use:      DVB_C get_C_N_dB function
            Used for retrieving the DVB_C C/N (in dB)
  Formula:

          C_N_dB =  10.log(2^24/c_n)

  Returns:  The current C/N value in dB. It returns 0 if c_n is null (to avoid a division by 0),
             knowing that a C/N of 0 indicates a bad reception
  Behavior: reads c_n and then computes the C/N value in dB.
  Registers:    c_n
  Parameter:   *demod          the demod handle

**************************************************************************************************/
EMB_L1_DVB_C_DEMOD_API double SLAB_WINAPI L1_DVB_C_get_C_N_dB       (L1_Context *demod) {
    double c_n;
    c_n = L1_READ(demod, c_n);
    if (c_n == 0) return 0;
    /* Use 16777216.0 instead of pow(2,24), to avoid use of math.h */
    return 10*log10( 16777216.0 / c_n );
}
/**************************************************************************************************
  L1_Demod_get_ber function
  Use:        BER retrieval function
              Used to compute the BER (Bit Error Rate) based on the ber_bit register
  Formula:

               BER = ber_bit / (ber_pkt.8.204)

  Variable:   Si2165->ber_depth    the current ber depth (set by L1_Demod_set_ber_depth)
  Behavior:   reads ber_bit and computes the BER value.
              If Si2165->ber_depth is null, attempt to set it by the L1_Demod_set_ber_depth
function.
              If the value is still null after an attempt to set it at DEFAULT_BER_DEPTH return 1
(the worst possible BER)
  Parameter: *demod          the demod handle
  Registers: ber_bit

**************************************************************************************************/
EMB_L1_COMMON_DEMOD_API double SLAB_WINAPI L1_Demod_get_ber(L1_Context *demod) {

    /* to access Si2165 specific parameters, cast demod->specific as a Si2165_Context */
    Si2165_Context *Si2165;
    Si2165  = (Si2165_Context *) demod->specific;

    if (Si2165->ber_depth == 0) {       /* if current ber_depth is 0, set it to DEFAULT_BER_DEPTH */
        L1_Demod_set_ber_depth(demod, DEFAULT_BER_DEPTH);
    }
    if (Si2165->ber_depth == 0) {       /* if HW value is still 0, do not compute to avoid a division by 0, return 1; */
        return 1;               /* a value of 1 would mean constant errors, which is what we want to return in case of errors */
    }
    /* Keep the formula as 8*208 to 'show' that we BER uses 'word_size*ts_packet_size' */
    return (double)L1_READ(demod, ber_bit)/(Si2165->ber_depth*8*208);
}


/* ==========================================================================*/


void Si2165Init(struct nim_device *dev)
{
    int nim_id;
    unsigned char data[1],read;
#if APP_TUNER_OFDM_EN4020_SI2165 || APP_TUNER_QAM_EN4020_SI2165
    ENTR_return retVal;
    ENTR_uint16 rev;
#endif

    nim_id = ((struct nim_si2165_private * )(dev->priv))->nim_id;
      /* demod instance initialization (to do only once) */
#if APP_TUNER_OFDM_EN4020_SI2165

    if(L1_Demod_SW_Init(&demod[nim_id], DEMOD_ADDRESS, standard_dvb_t)){
        demod[nim_id]->i2c->nim_id = nim_id;
        ctmp[nim_id].i2c_id = demod[nim_id]->i2c->i2c_id = ((struct nim_si2165_private * )(dev->priv))->i2c_id;
    }else{
        NIM_SI2165_PRINTF("Si2165Init error!!\n");
    }

#endif

#if APP_TUNER_QAM_EN4020_SI2165
    L1_Demod_SW_Init(&demod[nim_id], DEMOD_ADDRESS, standard_dvb_c);
#endif

    /* <Porting> <removable> Your final application will not have a simulation mode, hence it     will always work in 'connected' mode */
#ifdef SILABS_I2C
       L0_Connect   (demod[nim_id]->i2c, USB);
#else
    L0_Connect   (demod[nim_id]->i2c, CUSTOMER);
#endif  /* SILABS_I2C */

    L0_TrackWrite(demod[nim_id]->i2c, 1);
    L0_TrackRead (demod[nim_id]->i2c, 1);

    /* demod hardware initialization (to do after each HW reset or power-cycle) */
    L1_Demod_Re_Init(demod[nim_id]);

    /* tuner initialization */
#if APP_TUNER_OFDM_MAX3580_SI2165
    L1_RF_TUNER_Init(&tuner[nim_id], TUNER_ADDRESS);
    L0_Connect     (tuner[nim_id]->i2c, CUSTOMER);
#endif

#if APP_TUNER_OFDM_EN4020_SI2165 || APP_TUNER_QAM_EN4020_SI2165

    printf("[QamInit] en4020 init start\n" );

    /* paramters set by entr_active() */
    ctmp[nim_id].b2wAddress = ENTR_b2wAddress_C0;
    ctmp[nim_id].Fxtal = ENTR_Fxtal_24MHz;
    ctmp[nim_id].Fcomp = ENTR_Fcomp_1MHz;
    ctmp[nim_id].specInvSel = ENTR_specInvSel_on;
    ctmp[nim_id].ltaEnaSel = ENTR_ltaEnaSel_on;
    ctmp[nim_id].tunerCnt = ENTR_tunerCnt_single;

    /* parameters set by entr_set_chan() */
    /* atsc Mode */
    ctmp[nim_id].rfAttMode = ENTR_rfAttMode_seriesPin_shuntPin_internalAtt;

#if APP_TUNER_OFDM_EN4020_SI2165
    ctmp[nim_id].modulation = ENTR_modulation_DVB_T;
#endif

#if APP_TUNER_QAM_EN4020_SI2165
    c.modulation = ENTR_modulation_DVB_C;
#endif

    ctmp[nim_id].ifOutputSel = ENTR_ifOutputSel_1;
    ctmp[nim_id].agcCtrlMode = ENTR_agcCtrlMode_demod; /* tuner only pcb - ENTR_agcCtrlMode_internal */
    ctmp[nim_id].broadcastMedium = ENTR_broadcastMedium_ota;
    ctmp[nim_id].ifOutPutLevel = ENTR_ifOutPutLevel_0_dB_nominal;
    ctmp[nim_id].ifBw = ENTR_ifBw_8MHz;
    ctmp[nim_id].FifHz = 36000000;
    ctmp[nim_id].FchanHz = 243000000;
    printf("[QamInit] entr_code_revision\n" );

    semaphore_wait(&Sem_Si2165);
    retVal = entr_code_revision (&rev);
    semaphore_signal(&Sem_Si2165);

    if (retVal != ENTR_ok)
    {
        printf("\n[QamInit]  en4020 revision error = %d\n",rev);
    }
    semaphore_wait(&Sem_Si2165);
    retVal = entr_active (&ctmp[nim_id]);
    semaphore_signal(&Sem_Si2165);

    if (retVal != ENTR_ok)
    {
        printf("\n[QamInit]  en4020 active error = %d\n",retVal);
    }
    printf("[QamInit] en4020 init end\n" );

#endif


#if 0

    L1_WRITE(demod,ts_data_mode,ts_data_mode_parallel);
    L1_WRITE(demod,ts_data_dir,ts_data_dir_msb_first);
    L1_WRITE(demod,ts_data_parity,ts_data_parity_enabled);
    L1_WRITE(demod,ts_data_sync_overwr,ts_data_sync_overwr_enabled);
    L1_WRITE(demod,ts_tei,ts_tei_enabled);
    L1_WRITE(demod,ts_before_lock,ts_before_lock_active);

    L1_WRITE(demod,ts_clk_edge,ts_clk_edge_falling);
    L1_WRITE(demod,ts_clk_mode,ts_clk_mode_gapped);

    L1_WRITE(demod,ts_mux,ts_mux_ts);
    L1_WRITE(demod,sel_gpio_ts_err,sel_gpio_ts_err_ts_err);
    L1_WRITE(demod,sel_gpio_ts_data1,sel_gpio_ts_data1_ts_data);
    L1_WRITE(demod,sel_gpio_ts_data2,sel_gpio_ts_data2_ts_data);
    L1_WRITE(demod,sel_gpio_ts_data3,sel_gpio_ts_data3_ts_data);
    L1_WRITE(demod,sel_gpio_ts_data4,sel_gpio_ts_data4_ts_data);

    L1_WRITE(demod,ts_data0_tri,ts_data0_tri_normal);
    L1_WRITE(demod,ts_data1_tri,ts_data1_tri_normal);
    L1_WRITE(demod,ts_data2_tri,ts_data2_tri_normal);
    L1_WRITE(demod,ts_data3_tri,ts_data3_tri_normal);
    L1_WRITE(demod,ts_data4_tri,ts_data4_tri_normal);
    L1_WRITE(demod,ts_data5_tri,ts_data5_tri_normal);
    L1_WRITE(demod,ts_data6_tri,ts_data6_tri_normal);
    L1_WRITE(demod,ts_data7_tri,ts_data7_tri_normal);


    L1_WRITE(demod, ts_sync_pola,ts_sync_pola_active_high);
    L1_WRITE(demod, ts_val_pola,ts_val_pola_active_high);
    L1_WRITE(demod, ts_err_pola,ts_err_pola_active_high);


#endif

}

void Si2165TunerSet(int nim_id, int Freq,int Band,int QamSize) /* 474 8000*/
{
    Si2165_Context *Si2165;
#if APP_TUNER_OFDM_EN4020_SI2165 || APP_TUNER_QAM_EN4020_SI2165
    ENTR_return retVal;
#endif
    Si2165  = (Si2165_Context *) demod[nim_id]->specific;

    Si2165->bandwidth_MHz = Band/1000;
    Si2165->symbol_rate_MHz = Band/1000;

#if APP_TUNER_OFDM_EN4020_SI2165 || APP_TUNER_QAM_EN4020_SI2165

    L2_Tune(nim_id, NULL,Freq*1000000);
    //L1_DVB_T_set_bandwidth(demod, Si2165->bandwidth_MHz);
    //c.FchanHz = Freq*1000000;//666000000;
    //entr_set_chan(&c);


    /* If the requested standard is not the current standard, set it correctly and re_init */
    //if (L1_Demod_current_standard(front_end->demod) != scan->standard[i])
    if(0)
    {
        L1_Demod_set_standard(demod[nim_id], Si2165->standard);
        L1_Demod_Re_Init(demod[nim_id]);
    }

      switch (Si2165->standard)
      {

              case standard_dvb_t:
              {

                  //DVB_T_Status    *dvb_t_details;
                  //dvb_t_details = scan->carrier_details[i];
                  L1_DVB_T_set_bandwidth(demod[nim_id], Si2165->bandwidth_MHz);
                  //L1_DVB_T_set_stream    (front_end->demod, dvb_t_details->stream);

                  /* For DVB-T channels, reset is not required, the DSP will relock on its own */
                  break;
              }

              case standard_dvb_c:
              {

                  //DVB_C_Status    *dvb_c_details;
                  //dvb_c_details = scan->carrier_details[i];
                  L1_DVB_C_set_constellation(demod[nim_id], QamSize);
                  L1_DVB_C_set_symbol_rate    (demod[nim_id], Si2165->symbol_rate_MHz);

                  /* For DVB-C channels, reset is required (the DSP is not used for DVB-C) */
                  L1_Demod_reset            (demod[nim_id]);
                  break;
              }

              default :
                  printf("UNKNOWN standard (%d)\n",Si2165->standard);
              break;
    }




    //c.FchanHz = Freq*1000000;//666000000;
    //semaphore_wait(&Sem_Si2165);
    //retVal = entr_set_chan(&c);
    //semaphore_signal(&Sem_Si2165);

    //if (retVal != ENTR_ok)
    //{
    //    printf("\n[TerrTunerSet]  entr_set_chan = %d\n",retVal);
    //}
    //else
    //    printf("\n[TerrTunerSet]  entr_set_chan = %dMHz\n",Freq);

#endif

}

int Si2165LockCheck(int nim_id)
{
    int lock;

    //lock = L1_Demod_get_demod_lock(demod);
    lock = L1_READ(demod[nim_id],demod_lock_t);
   // printf("tps lock = %d\n",lock);
    //printf("tps lock = %d\n", L1_READ(demod,tps_lock));
    //printf("tps lock = %d\n", L1_READ(demod,freq_lock_t));
    //printf("tps lock = %d\n", L1_READ(demod,timing_lock_t));
    //printf("tps lock = %d\n", L1_READ(demod,fft_lock_t));

    //if(lock)NIM_SI2165_PRINTF("================> demod-[%d] locked\n", nim_id);
    return lock;
}

/* ==========================================================================*/
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
static int nim_attached_counter, nim_opened[MAX_NIM];
static UINT32 nim_frequency[MAX_NIM];
static UINT8 nim_code_rate[MAX_NIM];
static UINT16 nim_agc[MAX_NIM];
static UINT8 nim_snr[MAX_NIM];
static UINT32 nim_vbber[MAX_NIM];
static UINT8 nim_guard_interval[MAX_NIM];
static UINT8 nim_fft_mode[MAX_NIM];
static UINT8 nim_modulation[MAX_NIM];
static UINT8 nim_Inv[MAX_NIM];
static UINT8 nim_hier[MAX_NIM];
static UINT8 nim_priority[MAX_NIM];
static INT32 nim_freq_offset[MAX_NIM];
static UINT32 nim_autoscan_stop_flag[MAX_NIM];

 char  L1_DVB_T_format_text_status (DVB_T_Status  *status, char** formatted_status) {
    *formatted_status = (char*) malloc(sizeof(char)*1000);
    if (formatted_status == NULL) return 0;

    sprintf(*formatted_status, "--- DVB_T status ---\n");
    sprintf(*formatted_status, "%s stream          %3d", *formatted_status, status->stream);
    switch(status->stream)
    {
        case stream_hp:
            sprintf(*formatted_status, "%s : hp\n", *formatted_status); break;
        case stream_lp:
            sprintf(*formatted_status, "%s : lp\n", *formatted_status); break;
        default:
            sprintf(*formatted_status, "%s : UNKNOWN!\n", *formatted_status);
    }

    sprintf(*formatted_status, "%s fft_mode        %3d", *formatted_status, status->fft_mode);
    switch(status->fft_mode)
    {
        case auto_fft_mode_2k:
            sprintf(*formatted_status, "%s : 2k\n", *formatted_status); break;
        case auto_fft_mode_4k:
            sprintf(*formatted_status, "%s : 4k\n", *formatted_status); break;
        case auto_fft_mode_8k:
            sprintf(*formatted_status, "%s : 8k\n", *formatted_status); break;
        default:
            sprintf(*formatted_status, "%s : UNKNOWN!\n", *formatted_status);
    }

    sprintf(*formatted_status, "%s guard_interval  %3d", *formatted_status, status->guard_interval);
    switch(status->guard_interval)
    {
        case auto_guard_int_1_32:
            sprintf(*formatted_status, "%s : 1/32\n", *formatted_status); break;
        case auto_guard_int_1_16:
            sprintf(*formatted_status, "%s : 1/16\n", *formatted_status); break;
        case auto_guard_int_1_8:
            sprintf(*formatted_status, "%s : 1/8\n", *formatted_status); break;
        case auto_guard_int_1_4:
            sprintf(*formatted_status, "%s : 1/4\n", *formatted_status); break;
        default:
            sprintf(*formatted_status, "%s : UNKNOWN!\n", *formatted_status);
    }

    sprintf(*formatted_status, "%s constellation   %3d", *formatted_status, status->constellation);
    switch(status->constellation)
    {
        case auto_constellation_qpsk:
            sprintf(*formatted_status, "%s : QPSK\n", *formatted_status); break;
        case auto_constellation_qam16:
            sprintf(*formatted_status, "%s : QAM16\n", *formatted_status); break;
        case auto_constellation_qam64:
            sprintf(*formatted_status, "%s : QAM64\n", *formatted_status); break;
        default:
            sprintf(*formatted_status, "%s : UNKNOWN!\n", *formatted_status);
    }

    sprintf(*formatted_status, "%s hierarchy       %3d", *formatted_status, status->hierarchy);
    switch(status->hierarchy)
    {
        case auto_hierarchy_none:
            sprintf(*formatted_status, "%s : none\n", *formatted_status); break;
        case auto_hierarchy_alfa1:
            sprintf(*formatted_status, "%s : Alpha_1\n", *formatted_status); break;
        case auto_hierarchy_alfa2:
            sprintf(*formatted_status, "%s : Alpha_2\n", *formatted_status); break;
        case auto_hierarchy_alfa4:
            sprintf(*formatted_status, "%s : Alpha_4\n", *formatted_status); break;
        default:
            sprintf(*formatted_status, "%s : UNKNOWN!\n", *formatted_status);
    }

    sprintf(*formatted_status, "%s code_rate_hp    %3d", *formatted_status, status->code_rate_hp);
    switch(status->code_rate_hp)
    {
        case auto_rate_hp_1_2:
            sprintf(*formatted_status, "%s : 1/2\n", *formatted_status); break;
        case auto_rate_hp_2_3:
            sprintf(*formatted_status, "%s : 2/3\n", *formatted_status); break;
        case auto_rate_hp_3_4:
            sprintf(*formatted_status, "%s : 3/4\n", *formatted_status); break;
        case auto_rate_hp_5_6:
            sprintf(*formatted_status, "%s : 5/6\n", *formatted_status); break;
        case auto_rate_hp_7_8:
            sprintf(*formatted_status, "%s : 7/8\n", *formatted_status); break;
        default:
            sprintf(*formatted_status, "%s : UNKNOWN!\n", *formatted_status);
    }

    sprintf(*formatted_status, "%s code_rate_lp    %3d", *formatted_status, status->code_rate_lp);
    switch(status->code_rate_lp)
    {
        case auto_rate_lp_1_2:
            sprintf(*formatted_status, "%s : 1/2\n", *formatted_status); break;
        case auto_rate_lp_2_3:
            sprintf(*formatted_status, "%s : 2/3\n", *formatted_status); break;
        case auto_rate_lp_3_4:
            sprintf(*formatted_status, "%s : 3/4\n", *formatted_status); break;
        case auto_rate_lp_5_6:
            sprintf(*formatted_status, "%s : 5/6\n", *formatted_status); break;
        case auto_rate_lp_7_8:
            sprintf(*formatted_status, "%s : 7/8\n", *formatted_status); break;
        default:
            sprintf(*formatted_status, "%s : UNKNOWN!\n", *formatted_status);
    }
    TRACES_PRINTF("%s\n",*formatted_status);
    return 1;
}

char   f_si2165_get_status(L1_Context *demod, DVB_T_Status  *status) {
    if(status){
        status->stream         = L1_DVB_T_get_stream(demod);
        status->fft_mode       = L1_DVB_T_get_fft_mode(demod);
        status->guard_interval = L1_DVB_T_get_guard_interval(demod);
        status->constellation  = L1_DVB_T_get_constellation(demod);
        status->hierarchy      = L1_DVB_T_get_hierarchy(demod);
        status->code_rate_hp   = L1_DVB_T_get_code_rate_hp(demod);
        status->code_rate_lp   = L1_DVB_T_get_code_rate_lp(demod);
    }else
        return 0;
    return 1;
}

static INT32 f_si2165_open(struct nim_device *dev)
{
    int nim_id;

    nim_id = ((struct nim_si2165_private * )(dev->priv))->nim_id;
    NIM_SI2165_PRINTF("NIM ======> f_si2165_open(0x%x), nim_id: %d\n", dev, nim_id);

    if(nim_id >= MAX_NIM){
        NIM_SI2165_PRINTF("f_si2165_open Error!! nim_id: %d\n", nim_id);
        return ERR_FAILED;
    }
    if(nim_opened[nim_id]){
        NIM_SI2165_PRINTF("warning: nim-%d already opened!!", nim_id);
    }else{
        Si2165Init(dev);
        nim_opened[nim_id] = 1;
    }
    return SUCCESS;
}
static INT32 f_si2165_close(struct nim_device *dev)
{
    int nim_id;

    nim_id = ((struct nim_si2165_private * )(dev->priv))->nim_id;
    NIM_SI2165_PRINTF("f_si2165_close...%d\n", nim_id);

    if(!nim_opened[nim_id] ){
        NIM_SI2165_PRINTF("warning: nim-%d already closed!!", nim_id);
    }else{
        nim_opened[nim_id] = 0;
    }
     L1_Demod_reset(demod[nim_id]);

    return SUCCESS;
}
static INT32 f_si2165_disable(struct nim_device *dev)
{
    UINT8 data = 0;
    int nim_id;

    nim_id = ((struct nim_si2165_private * )(dev->priv))->nim_id;
    NIM_SI2165_PRINTF("f_si2165_disable...%d\n", nim_id);

    return SUCCESS;
}
static INT32 f_si2165_ioctl(struct nim_device *dev, INT32 cmd, UINT32 param)
{
    int nim_id;

    nim_id = ((struct nim_si2165_private * )(dev->priv))->nim_id;
    NIM_SI2165_PRINTF("f_si2165_ioctl...id:%d, cmd: %d, param:%d\n", nim_id, cmd, param);

    switch( cmd )
    {
        case NIM_DRIVER_STOP_ATUOSCAN:
            nim_autoscan_stop_flag[nim_id] = param;
        break;

        default:
        break;
    }
    return SUCCESS;
}

static INT32 f_si2165_channel_change(struct nim_device *dev, UINT32 freq, UINT32 bandwidth,
    UINT8 guard_interval, UINT8 fft_mode, UINT8 modulation, UINT8 fec, UINT8 usage_type, UINT8 inverse,UINT8 priority)
{
    UINT8 code_rate,Tuner_Retry=0,Can_Tuner_Retry=0;
    UINT8 lock=0;
    int nim_id;

    nim_id = ((struct nim_si2165_private * )(dev->priv))->nim_id;

    NIM_SI2165_PRINTF("==== f_si2165_channel_change ====:%d\n", nim_id);
    NIM_SI2165_PRINTF("fre: %d\n", freq);
    NIM_SI2165_PRINTF("bandwidth: %d\n", bandwidth);
    NIM_SI2165_PRINTF("guard interval: %d\n", guard_interval);
    NIM_SI2165_PRINTF("fft_mode: %d\n", fft_mode);
    NIM_SI2165_PRINTF("modulation:%d\n", modulation);
    NIM_SI2165_PRINTF("FEC: %d\n", fec);
    NIM_SI2165_PRINTF("usage_type: %d\n", usage_type);
    NIM_SI2165_PRINTF("inverse: %d\n", inverse);
    NIM_SI2165_PRINTF("priority: %d\n", priority);

    Si2165TunerSet(nim_id, freq/1000, bandwidth*1000, 64);
    nim_frequency[nim_id] = freq;

    return SUCCESS;
}
static INT32 f_si2165_channel_search(struct nim_device *dev, UINT32 freq,UINT32 bandwidth,
    UINT8 guard_interval, UINT8 fft_mode, UINT8 modulation, UINT8 fec, UINT8 usage_type, UINT8 inverse,UINT16 freq_offset,UINT8 priority)
{
    UINT8     i,data[1];
    INT32    chsearch=ERR_FAILED ,center_freq;
    INT32 freq_offset_reg;

     UINT32    Center_NPRO,Search_NPRO,tmp_freq;
    UINT32 First_Frequency;
    UINT8 num,freq_step;
    UINT8 j=0,k = 0;
    INT32 get_freq_offset;
    INT32 remainder;
    UINT16 temp_Step_Freq;
    int nim_id;

    NIM_SI2165_PRINTF("==== f_si2165_channel_search ====:%d, freq_offset:%d\n", nim_id, freq_offset);

    nim_id = ((struct nim_si2165_private * )(dev->priv))->nim_id;
    Si2165TunerSet(nim_id, freq/1000, bandwidth*1000, 64);
    return f_si2165_channel_change(dev, freq, bandwidth, guard_interval, fft_mode, modulation, fec, usage_type, inverse, priority);
}

//static UINT32 ber_val;

static INT32 f_si2165_get_lock(struct nim_device *dev, UINT8 *lock)
{
    UINT8 data=0;
    OSAL_ER    result;
    UINT32    flgptn;
    DVB_T_Status  status;

    int nim_id;
    nim_id = ((struct nim_si2165_private * )(dev->priv))->nim_id;
    //NIM_SI2165_PRINTF("f_si2165_get_lock nim_id: %d\n", nim_id);

    *lock = Si2165LockCheck(nim_id);

    //if(*lock==1)
        //f_si2165_get_BER(dev, &ber_val);


    #if(SI2165_DEBUG_FLAG)

        static UINT8 count=0;

        count++;
        if(count==20)
        {
            soc_printf("LK=%d \n",*lock);
            count=0;
        }

    #endif

/*
    f_si2165_get_status(demod[nim_id], &status);
    char *ptxt=NULL;
    L1_DVB_T_format_text_status(&status, &ptxt);

    NIM_SI2165_PRINTF("========= tuner %d status ==========\n", nim_id);
    if(ptxt){
        NIM_SI2165_PRINTF(ptxt);
        free(ptxt);
    }
*/
    return SUCCESS;
}
static INT32 f_si2165_get_freq(struct nim_device *dev, UINT32 *freq)
{
    INT32 freq_off=0;
    UINT8  data[3];
    INT32  tdata=0;
    OSAL_ER    result;
    UINT32    flgptn;
    int nim_id;
    nim_id = ((struct nim_si2165_private * )(dev->priv))->nim_id;
    *freq =     nim_frequency[nim_id];
    NIM_SI2165_PRINTF("f_si2165_get_freq nim_id: %d freq: %d khz\n", nim_id,     *freq);

    return SUCCESS;

}
static INT32 f_si2165_get_code_rate(struct nim_device *dev, UINT8* code_rate)
{
    int nim_id;

    //L1_DVB_T_get_code_rate_hp

    nim_id = ((struct nim_si2165_private * )(dev->priv))->nim_id;
    NIM_SI2165_PRINTF("f_si2165_get_code_rate nim_id: %d\n", nim_id);


    return SUCCESS;

}
static INT32 f_si2165_get_AGC(struct nim_device *dev, UINT16 *agc)
{
     UINT8        data[2];
     UINT16        agc_if=0;
     OSAL_ER    result;
     UINT32        flgptn;
      UINT16    agc_min,agc_max;
     int nim_id;

     nim_id = ((struct nim_si2165_private * )(dev->priv))->nim_id;
    // NIM_SI2165_PRINTF("f_si2165_get_AGC nim_id: %d\n", nim_id);
    *agc = nim_agc[nim_id];

    return SUCCESS;
}
static INT32 f_si2165_get_SNR(struct nim_device *dev, UINT8 *snr)
{
    UINT8    data;
    OSAL_ER result;
    UINT32     flgptn;
    int nim_id;

    nim_id = ((struct nim_si2165_private * )(dev->priv))->nim_id;
    //NIM_SI2165_PRINTF("f_si2165_get_SNR nim_id: %d\n", nim_id);
    *snr = nim_snr[nim_id];
    return SUCCESS;
}

static INT32 f_si2165_get_BER(struct nim_device *dev, UINT32 *vbber)
{
    UINT32 dwPeriod=0,dwCount=0;
    UINT32 temp,temp1;
    UINT32 ber;
    INT32 i;
    UINT8 data[2];
    OSAL_ER result;
    UINT32     flgptn;
    int nim_id;
    nim_id = ((struct nim_si2165_private * )(dev->priv))->nim_id;

    nim_vbber[nim_id] = L1_Demod_get_ber(demod[nim_id]);
    *vbber = nim_vbber[nim_id];

    return SUCCESS;
}
static INT32 f_si2165_get_GI(struct nim_device *dev, UINT8 *guard_interval)
{
    int nim_id;

    NIM_SI2165_PRINTF("f_si2165_get_GI nim_id: %d\n", nim_id);
    nim_id = ((struct nim_si2165_private * )(dev->priv))->nim_id;

    nim_guard_interval[nim_id] = (UINT8*)L1_DVB_T_get_guard_interval(demod[nim_id]);
    switch(L1_DVB_T_get_guard_interval(demod[nim_id]))
    {
        case auto_guard_int_1_32:
            nim_guard_interval[nim_id] = 32; break;
        case auto_guard_int_1_16:
            nim_guard_interval[nim_id] = 16; break;
        case auto_guard_int_1_8:
            nim_guard_interval[nim_id] = 8; break;
        case auto_guard_int_1_4:
            nim_guard_interval[nim_id] = 4; break;
        default:;
    }

    *guard_interval = nim_guard_interval[nim_id];

    return SUCCESS;
}
static INT32 f_si2165_get_fftmode(struct nim_device *dev, UINT8 *fft_mode)
{
    int nim_id;

    nim_id = ((struct nim_si2165_private * )(dev->priv))->nim_id;
    NIM_SI2165_PRINTF("f_si2165_get_fftmode nim_id: %d\n", nim_id);

    switch(L1_DVB_T_get_fft_mode(demod))
    {
        case auto_fft_mode_2k:
            nim_fft_mode[nim_id] = 2; break;
        case auto_fft_mode_4k:
            nim_fft_mode[nim_id] = 4; break;
        case auto_fft_mode_8k:
            nim_fft_mode[nim_id] = 8; break;
        default:;
    }
    *fft_mode = nim_fft_mode[nim_id];
    return SUCCESS;
}

static INT32 f_si2165_get_modulation(struct nim_device *dev, UINT8 *modulation)
{
    int nim_id;

    nim_id = ((struct nim_si2165_private * )(dev->priv))->nim_id;
    NIM_SI2165_PRINTF("f_si2165_get_modulation nim_id: %d\n", nim_id);

    switch(L1_DVB_T_get_constellation(demod[nim_id]))
    {
        case auto_constellation_qpsk:
            nim_modulation[nim_id] = 4;
            break;
        case auto_constellation_qam16:
            nim_modulation[nim_id] = 16;
            break;

        case auto_constellation_qam64:
            nim_modulation[nim_id] = 64;
            break;
        default:;
    }
    *modulation = nim_modulation[nim_id];

    return SUCCESS;
}
static INT32 f_si2165_get_specinv(struct nim_device *dev, UINT8 *Inv)
{
    UINT8    data;
    UINT32    flgptn;
    OSAL_ER    result;
    int nim_id;

    nim_id = ((struct nim_si2165_private * )(dev->priv))->nim_id;
    NIM_SI2165_PRINTF("f_si2165_get_specinv nim_id: %d\n", nim_id);
    *Inv = nim_Inv[nim_id];

    return SUCCESS;
}
static INT32 f_si2165_get_hier_mode(struct nim_device *dev, UINT8*hier)
{
    int nim_id;

    nim_id = ((struct nim_si2165_private * )(dev->priv))->nim_id;
    NIM_SI2165_PRINTF("f_si2165_get_hier_mode nim_id: %d\n", nim_id);
    *hier = nim_hier[nim_id];

    return SUCCESS;
}
INT8 f_si2165_priority(struct nim_device *dev, UINT8*priority)
{
    int nim_id;

    nim_id = ((struct nim_si2165_private * )(dev->priv))->nim_id;
    NIM_SI2165_PRINTF("f_si2165_priority nim_id: %d\n", nim_id);
    *priority = nim_priority[nim_id];

    return SUCCESS;
}
static INT32 f_si2165_get_freq_offset(struct nim_device *dev, INT32 *freq_offset)
{
    OSAL_ER    result;
    UINT32    flgptn;
    int nim_id;
    nim_id = ((struct nim_si2165_private * )(dev->priv))->nim_id;
    NIM_SI2165_PRINTF("f_si2165_get_freq_offset nim_id: %d\n", nim_id);
    *freq_offset = nim_freq_offset[nim_id];

    return SUCCESS;
}

INT32 f_si2165_attach(char *name, struct COFDM_TUNER_CONFIG_API *ptrCOFDM_Tuner, UINT32 i2c_id)
{

    struct nim_device *dev;
    struct nim_si2165_private * priv_mem;

    if(nim_attached_counter >= MAX_NIM)
    {
        NIM_SI2165_PRINTF("f_si2165_attach Error -- %d/n", nim_attached_counter);
        return ERR_NO_DEV;
    }
    if ((ptrCOFDM_Tuner == NULL))
    {
        NIM_SI2165_PRINTF("Tuner Configuration API structure is NULL!/n");
        return ERR_NO_DEV;
    }

    NIM_I2C_SEMA_ID[nim_attached_counter] = osal_semaphore_create(1);

    dev = (struct nim_device *)dev_alloc(name, HLD_DEV_TYPE_NIM, sizeof(struct nim_device));
    if (dev == NULL)
    {
        NIM_SI2165_PRINTF("Error: Alloc nim device error!\n");
        return ERR_NO_MEM;
    }

    /* Alloc structure space of private */
    priv_mem = (struct nim_si2165_private *)MALLOC(sizeof(struct nim_si2165_private));
    if ((void*)priv_mem == NULL)
    {
        dev_free(dev);
        NIM_SI2165_PRINTF("Alloc nim device prive memory error!/n");
        return ERR_NO_MEM;
    }
    priv_mem->i2c_id = i2c_id;
    priv_mem->nim_id = nim_attached_counter++;
    dev->priv = (void*)priv_mem;

    /* Function point init */

    dev->base_addr = DEMOD_ADDRESS;   //please check here
    dev->init = f_si2165_attach;
    dev->open = f_si2165_open;
    dev->stop = f_si2165_close;
    dev->disable=f_si2165_disable;
    dev->do_ioctl = f_si2165_ioctl;
    dev->channel_change = f_si2165_channel_change;
    dev->channel_search = f_si2165_channel_search;
    dev->get_lock = f_si2165_get_lock;
    dev->get_freq = f_si2165_get_freq;
    dev->get_FEC = f_si2165_get_code_rate;
    dev->get_AGC = f_si2165_get_AGC;
    dev->get_SNR = f_si2165_get_SNR;
    dev->get_BER = f_si2165_get_BER;
    dev->get_guard_interval = f_si2165_get_GI;
    dev->get_fftmode = f_si2165_get_fftmode;
    dev->get_modulation = f_si2165_get_modulation;
    dev->get_spectrum_inv = f_si2165_get_specinv;
    dev->get_HIER= f_si2165_get_hier_mode;
    dev->get_priority=f_si2165_priority;
    dev->get_freq_offset =    f_si2165_get_freq_offset;



    /* Add this device to queue */
    if (dev_register(dev) != SUCCESS)
    {
        NIM_SI2165_PRINTF("Error: Register nim device error!\n");
        FREE(priv_mem);
        dev_free(dev);
        return ERR_NO_DEV;
    }

    return SUCCESS;
}

#endif
