#include <sys_config.h>
#include <retcode.h>
#include <types.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/i2c/i2c.h>
#include <osal/osal.h>
//#include <hld/nim/nim_dev.h>
#include <hld/nim/nim_tuner.h>

#include "tun_nm120.h"
#include "nmicmn.h"
#include "nmiioctl_cmn.h"

#if ((SYS_TUN_MODULE == NM120) || (SYS_TUN_MODULE == ANY_TUNER))

#define NIM_PRINTF(...)
//#define NIM_PRINTF libc_printf

static struct COFDM_TUNER_CONFIG_EXT *tuner_dev_cfg[MAX_TUNER_SUPPORT_NUM];
static UINT32 nm120_tuner_cnt = 0;
static UINT8 nm120_reg_ini[2]={0,0};
static UINT8 nm120_bw[2]={0,0};


/*****************************************************************************
* INT32 tun_nm120_init(struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config)
*
* Tuner nm120 Initialization
*
* Arguments:
*  Parameter1: struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config        : pointer for Tuner configuration structure
*
* Return Value: INT32            : Result
*****************************************************************************/
INT32 tun_nm120_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config)
{
    if ((ptrTuner_Config == NULL) || nm120_tuner_cnt >= MAX_TUNER_SUPPORT_NUM)
        return ERR_FAILUE;

    struct COFDM_TUNER_CONFIG_EXT *cfg;
    cfg = (struct COFDM_TUNER_CONFIG_EXT *)MALLOC(sizeof(struct COFDM_TUNER_CONFIG_EXT));
    if(cfg == NULL)
        return ERR_FAILUE;

    tuner_dev_cfg[nm120_tuner_cnt] = cfg;
    MEMCPY(cfg, ptrTuner_Config, sizeof(struct COFDM_TUNER_CONFIG_EXT));
    if (NULL != tuner_id)
        *tuner_id = nm120_tuner_cnt;

    nm120_reg_ini[nm120_tuner_cnt]=0;        // status bit for initialized nm120 register
    nm120_tuner_cnt++;

    return SUCCESS;
}

/*****************************************************************************
* INT32 tun_nm120_status(UINT8 *lock)
*
* Tuner read operation
*
* Arguments:
*  Parameter1: UINT8 *lock        : Phase lock status
*
* Return Value: INT32            : Result
*****************************************************************************/

INT32 tun_nm120_status(UINT32 tuner_id, UINT8 *lock)
{
    INT32 result;
    UINT8 data =0;
    struct COFDM_TUNER_CONFIG_EXT *ptrTuner_Config;

    if ((tuner_id>=nm120_tuner_cnt) || (tuner_id>=MAX_TUNER_SUPPORT_NUM))
        return ERR_FAILUE;
    ptrTuner_Config = tuner_dev_cfg[tuner_id];

    // nm120 has no PLL lock indicator, only need to wait 100ms, enough for lock
    osal_task_sleep(100);

    //result = ptrTuner_Config->Tuner_Read(ptrTuner_Config->i2c_type_id,ptrTuner_Config->cTuner_Base_Addr, &data, 1);
    *lock = 1;

    return SUCCESS;
}

/*****************************************************************************
* INT32 nim_nm120_control(UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd)
*
* Tuner write operation
*
* Arguments:
*  Parameter1: UINT32 freq        : Synthesiser programmable divider
*  Parameter2: UINT8 bandwidth        : channel bandwidth
*  Parameter3: UINT8 AGC_Time_Const    : AGC time constant
*  Parameter4: UINT8 *data        :
*
* Return Value: INT32            : Result
*****************************************************************************/
INT32 tun_nm120_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd)
{
    INT32 result;
    UINT8 tmp_bw=0;

    struct COFDM_TUNER_CONFIG_EXT *ptrTuner_Config;

    if (tuner_id >= nm120_tuner_cnt || tuner_id >= MAX_TUNER_SUPPORT_NUM)
    {
        return ERR_FAILUE;
    }
    ptrTuner_Config = tuner_dev_cfg[tuner_id];

    if(0==nm120_reg_ini[tuner_id])
    {
        tTnrInit cfg;
        MEMSET((void *)&cfg, 0, sizeof(tTnrInit));

        cfg.ldobypass = 0;
        cfg.xo = 24;
        cfg.i2c_adr = 0xce;
        cfg.init_bus_only = 0;
        result = nmi_drv_ctl(NMI_DRV_INIT, (void *)&cfg);

        if (0 != result)
        {
            return ERR_FAILUE;
        }

        nm120_reg_ini[tuner_id]=1;
    }


    tTnrTune tune;
    int spectrum = 1;
    //int if_outpur = 40;

    MEMSET((void *)&tune, 0, sizeof(tTnrTune));
    tune.rf = freq * 1000;
    tune.vif = 4570000;
    switch(bandwidth)
    {
        case 6:
            tune.std = nDVBT_6;//nDTV_6; 
            break;
        case 7:
            tune.std = nDVBT_7;//nDTV_7;
            break;
        case 8:
        default:
            tune.std = nDVBT_8;//nDTV_8;
            break;
    }
    tune.output = nIf;
/*
    if (nmi_drv_ctl(NMI_DRV_TUNE, &tune) < 0)
    {
        return ERR_FAILUE;
    }
    else
    {
        nmi_drv_ctl(NMI_DRV_INVERT_SPECTRUM, (void *)(&spectrum));
        nmi_drv_ctl(NMI_DRV_SET_IF_OUTPUT_VOLTAGE, (void *)(&if_outpur));
    }
*/

    //libc_printf("NMI120 bandwidth: %d !\n", tune.std);

    nmi_drv_ctl(NMI_DRV_TUNE, &tune);
    //nmi_drv_ctl(NMI_DRV_SET_IF_OUTPUT_VOLTAGE, (void *)(&if_outpur));

    return SUCCESS;
}


/******************************************************************************
**
**    ASIC Helper Functions
**
*******************************************************************************/

void nmi_log(char *str)
{
    libc_printf(str);
}

void nmi_delay(UINT32 msec)
{
    int i = 0;
    for (i=0; i<=msec; i++)
    {
        osal_delay(1000);
    }
}

UINT32 nmi_get_tick(void)
{
    UINT32 time;
    time = osal_get_tick();
    return time;
}

int nmi_bus_read(unsigned char DeviceAddr, unsigned char* pArray, unsigned long count)
{
    int status = 0;
    struct COFDM_TUNER_CONFIG_EXT *ptrTuner_Config;

    ptrTuner_Config = tuner_dev_cfg[0];

    status= ptrTuner_Config->tuner_read(I2C_FOR_TUNER, DeviceAddr, pArray, count);


    return (status);    //success
}


int nmi_bus_write(unsigned char DeviceAddr, unsigned char* pArray, unsigned long count)
{
    int status = 0;
    struct COFDM_TUNER_CONFIG_EXT *ptrTuner_Config;

    ptrTuner_Config = tuner_dev_cfg[0];

    status= ptrTuner_Config->tuner_write(I2C_FOR_TUNER, DeviceAddr, pArray, count);


    return (status);    //success
}

unsigned long nmi_bus_get_read_blksz(void)
{
    //return UINT_MAX;
    return 0;
}

unsigned long nmi_bus_get_write_blksz(void)
{
    return 14;
}


#endif

