#include <types.h>
#include <sys_config.h>
#include <bus/ide/ide.h>
#include <osal/osal.h>
#include <errno.h>

#include "ide_reg.h"
#include "ide_iops.h"


#define IDE_ATOM_OPTION                0

#if (IDE_ATOM_OPTION>0)
#define ATOM_ENTER()    osal_interrupt_disable()
#define ATOM_LEAVE()    osal_interrupt_enable()
#else
#define ATOM_ENTER()    do{}while(0)
#define ATOM_LEAVE()    do{}while(0)
#endif


// PIO register=pio_reg[mode][0],pio_reg[mode][1],pio_reg[mode][2],pio_reg[mode][3]
UINT32 g_pio_reg[5][4]={{0x04,0x19,0x30,0x21},
    {0x03,0x07,0x30,0x09},{0x02,0x03,0x30,0x05},
    {0x01,0x10,0x0D,0x12},{0x01,0x08,0x0B,0x0A}};
// PIO register=pio_data[mode][0],pio_data[mode][1],pio_data[mode][2],pio_data[mode][3]
UINT32 g_pio_data[5][4]={{0x04,0x48,0x1B,0x4A},
    {0x03,0x2A,0x14,0x2C},{0x02,0x17,0x10,0x19},
    {0x01,0x10,0x0D,0x12},{0x01,0x08,0x0B,0x0A}};

// dma register=0x00,dma[mode][0],dma[mode][1],dma[mode][2]
UINT32 g_dma[3][3]={{0x03,0x28,0x27},{0x02,0x0C,0x0D},{0x01,0x08,0x0B}};

// udma register
UINT32 g_udma1[6][7]={{0x1A,0xB,0x8,0xB,0x4,0x3,0x1},{0x14,0x7,0x8,0x7,0x4,0x3,0x1},
    {0x10,0x5,0x8,0x5,0x4,0x3,0x1},{0x10,0x3,0x8,0x3,0x4,0x3,0x1},
    {0x10,0x3,0x8,0x3,0x4,0x3,0x0},{0x0E,0x3,0x8,0x3,0x4,0x3,0x0}};
UINT32 g_udma2[6][2]={{0x18,0x13},{0x18,0x0D},{0x18,0x09},{0x10,0x07},{0x10,0x04},{0x0C,0x03}};

UINT32 g_pio_reg_new[5][4];
UINT32 g_pio_data_new[5][4];
UINT32 g_dma_new[3][3];
UINT32 g_udma1_new[6][7];
UINT32 g_udma2_new[6][2];

extern void ide_set_rw_mode(UINT8 mode,UINT8 id);
extern UINT8 ide_get_rw_modeid(void);

void ide_s3601_set_pio_data_timing(BYTE mode);

static UINT8 ide_s3601_inb(UINT8 port)
{
    UINT8 ret;
    ATOM_ENTER();
    IDE_SET_BYTE(S3601_IDE_AAR, (UINT8)(port&0x1F));
    ret = IDE_GET_WORD(S3601_IDE_DAR)&0xff;
    ATOM_LEAVE();
    return ret;
}

static UINT16 ide_s3601_inw(UINT8 port)
{
    UINT16 ret;
    ATOM_ENTER();
    IDE_SET_BYTE(S3601_IDE_AAR, (UINT8)(port&0x1F));
    ret = IDE_GET_WORD(S3601_IDE_DAR);
    ATOM_LEAVE();
    return ret;
}

static void ide_s3601_insw(UINT8 port, void *addr, UINT32 count)
{
    IDE_SET_BYTE(S3601_IDE_AAR, (UINT8)(port&0x1F));
    //ide_s3601_set_swap_ata(1);
    count = (count+1)>>1;
    while(count--) {
        *((UINT16 *)addr)=IDE_GET_WORD(S3601_IDE_DAR);
        addr += 2;
    }
    //ide_s3601_set_swap_ata(0);
}

static void ide_s3601_outb(UINT8 value, UINT8 port)
{
    ATOM_ENTER();
    IDE_SET_BYTE(S3601_IDE_AAR, (UINT8)(port&0x1F));
    IDE_SET_WORD(S3601_IDE_DAR, (UINT16)value);
    ATOM_LEAVE();
}

static void ide_s3601_outw(UINT16 value, UINT8 port)
{
    ATOM_ENTER();
    IDE_SET_BYTE(S3601_IDE_AAR, (UINT8)(port&0x1F));
    IDE_SET_WORD(S3601_IDE_DAR, value);
    ATOM_LEAVE();
}

static void ide_s3601_outsw(UINT8 port, void *addr, UINT32 count)
{
    IDE_SET_BYTE(S3601_IDE_AAR, (UINT8)(port&0x1F));
    //ide_s3601_set_swap_ata(1);
    count = (count+1)>>1;
    while(count--) {
        IDE_SET_WORD(S3601_IDE_DAR, *((UINT16 *)addr));
        addr += 2;
    }
    //ide_s3601_set_swap_ata(0);
}

static void ide_s3601_start_read(ide_drive_t *drive, UINT32 addr, UINT8 rwmode)
{

    if(rwmode== IDE_PIO_BUR_MODE)
    {
        ide_s3601_set_pio_data_timing(ide_get_rw_modeid());
        //set BURSTEN, bit 16 =1.
        IDE_SET_DWORD(S3601_IDE_ATACR, IDE_GET_DWORD(S3601_IDE_ATACR)|0x00010000);
    }
    else if (rwmode==IDE_DMA_MODE)
    {
        //ide_s3601_set_DMA_mode_id(ide_get_rw_modeid());
    }
    else if (rwmode==IDE_UDMA_MODE)
    {
        //ide_s3601_set_UDMA_mode_id(ide_get_rw_modeid());
        //set CLR_ATADSEL, bit 27=1, HW auto clear 0.
        IDE_SET_DWORD(S3601_IDE_ATACR, IDE_GET_DWORD(S3601_IDE_ATACR)|0x08000000);
    }
    else
    {
        //
    }

    //set ATAFIFO path.
    IDE_SET_PATH_FIFO();

#if S3601_SWAP_DATA
    //set swap for read.
    ide_s3601_set_swap_ata(1);
#endif

    //set destination address.
    IDE_SET_DWORD(S3601_IDE_ATABFDADR, addr);

    //start operation.
    if(rwmode==IDE_PIO_BUR_MODE)
    {
        //write AAR, fix hardware some bug.
        IDE_SET_BYTE(S3601_IDE_AAR, (UINT8)(0x8&0x1F));    // point to command block register
        //set PIOBRDSTB, bit 4=1.
        IDE_SET_BYTE(S3601_IDE_FSMCR, IDE_GET_BYTE(S3601_IDE_FSMCR)|0x00000010);
    }
    else if(rwmode==IDE_DMA_MODE)
    {
        //set DMARDSTB, bit 0=1.
        IDE_SET_BYTE(S3601_IDE_FSMCR, IDE_GET_BYTE(S3601_IDE_FSMCR)|0x00000001);
    }
    else if (rwmode==IDE_UDMA_MODE)
    {
        //set UDMARDSTB, bit 2=1.
        IDE_SET_BYTE(S3601_IDE_FSMCR, IDE_GET_BYTE(S3601_IDE_FSMCR)|0x00000004);
    }
    else
    {
        //
    }

    //later, ATAINT_2B, bit 0 will report transfer done.

}

static void ide_s3601_start_write(ide_drive_t *drive, UINT32 addr, UINT8 rwmode)
{
    //set ATAFIFO path.
    IDE_SET_PATH_FIFO();

#if S3601_SWAP_DATA
    //set swap for write.
    ide_s3601_set_chd_swap_atafifo(1);
#endif

    //set source address
    IDE_SET_DWORD(S3601_IDE_ATABFSADR, addr);

    //start operation.
    if(rwmode==IDE_PIO_BUR_MODE)
    {
        ide_s3601_set_pio_data_timing(ide_get_rw_modeid());
        // set BURSTEN, bit 16 =1.
        IDE_SET_DWORD(S3601_IDE_ATACR, IDE_GET_DWORD(S3601_IDE_ATACR)|0x00010000);

        //write AAR, fix hardware some bug.
        IDE_SET_BYTE(S3601_IDE_AAR, (UINT8)(0x8&0x1F));    // point to command block register

        //ide_s3601_set_PIOB_mode_id(ide_get_rw_modeid());
        //set PIOBWTSTB, bit 5=1.
        IDE_SET_BYTE(S3601_IDE_FSMCR, IDE_GET_BYTE(S3601_IDE_FSMCR)|0x00000020);
    }
    else if(rwmode==IDE_DMA_MODE)
    {
        //ide_s3601_set_DMA_mode_id(ide_get_rw_modeid());
        //set DMAWTSTB, bit 1=1.
        IDE_SET_BYTE(S3601_IDE_FSMCR, IDE_GET_BYTE(S3601_IDE_FSMCR)|0x00000002);
    }
    else if (rwmode==IDE_UDMA_MODE)
    {
        //ide_s3601_set_UDMA_mode_id(ide_get_rw_modeid());
        //set CLR_ATADSEL, bit 27=1, HW auto clear 0.
        IDE_SET_DWORD(S3601_IDE_ATACR, IDE_GET_DWORD(S3601_IDE_ATACR)|0x08000000);
        // set UDMAWTSTB, bit 3=1.
        IDE_SET_BYTE(S3601_IDE_FSMCR, IDE_GET_BYTE(S3601_IDE_FSMCR)|0x00000008);
    }
    else
    {
        //
    }

    //later, ATAINT_2B, bit 0 will report transfer done.
}

static INT32 ide_s3601_start_rw(ide_drive_t * drive, UINT32 addr, UINT32 count, INT32 direction, UINT8 rwmode)
{
    UINT32 value;

    // set byte count
    value=IDE_GET_DWORD(S3601_IDE_BC4TR);
    value=(value&0xFF000000)|(count&0x00FFFFFF);    // 3601 is little endian
    IDE_SET_DWORD(S3601_IDE_BC4TR, value);

    if (direction == RQ_FLAG_DATA_IN)
    {
        ide_s3601_start_read(drive, addr, rwmode);
    }
    else if (direction == RQ_FLAG_DATA_OUT)
    {
        ide_s3601_start_write(drive, addr, rwmode);
    }


    return SUCCESS;
}

INT32 ide_s3601_wait_rw_over(ide_drive_t *drive, INT32 direction, UINT32 timeout,UINT32 flagmode)
{
#ifdef _RD_DEBUG_
    UINT8 stat;
    hwif_t *hwif = HWIF(drive);
#endif
    UINT32 flag;
    extern OSAL_ID ide_flag_id;

    if (OSAL_E_TIMEOUT == osal_flag_wait(&flag, ide_flag_id, flagmode, TWF_ANDW|TWF_CLR, timeout)) {
        #ifdef _RD_DEBUG_
        stat = hwif->INB(IDE_ALTSTATUS_REG);
        libc_printf("=>Wait Int time out Stat(0x%x)=>", stat);
        #endif

        //HDD Physical errors may cause IDE bus data held to unknew value.
        //Only Reset IP can handle this issue.
        //if(stat == PHY_ERR_STAT)
        if(1)
        {
            //ATA HW Reset
            if (ALI_M3329E==sys_ic_get_chip_id())
            {
                //m3329e
                *(volatile unsigned long *)0xB8000060 |= 1<<7;
                osal_delay(100);
                *(volatile unsigned long *)0xB8000060 &= ~(1<<7);
                osal_delay(300);
            }
            else
            {
                //s3602
                *(volatile unsigned long *)0xB8000080 |= 1<<10;
                osal_delay(100);
                *(volatile unsigned long *)0xB8000080 &= ~(1<<10);
                osal_delay(300);
            }
            #ifdef _RD_DEBUG_
            stat = hwif->INB(IDE_ALTSTATUS_REG);
            IDE_PRINTF("=>After HW Reset Stat(0x%x)=>", stat);
            #endif

            //ide_error(drive, __FUNCTION__);
        }
        return ERR_FAILUE;
    }

    return SUCCESS;
}

static void ata_input_data(ide_drive_t *drive, void *data, UINT32 count)
{
    //HWIF(drive)->INSW(IDE_DATA_REG, data, count<<1);
    HWIF(drive)->INSW(IDE_DATA_REG, data, count);
}

static void ata_output_data(ide_drive_t *drive, void *data, UINT32 count)
{
    //HWIF(drive)->OUTSW(IDE_DATA_REG, data, count<<1);
    HWIF(drive)->OUTSW(IDE_DATA_REG, data, count);
}

void m3327_hwif_iops(hwif_t *hwif)
{
    hwif->INB = ide_s3601_inb;
    hwif->INW = ide_s3601_inw;
    hwif->OUTB = ide_s3601_outb;
    hwif->OUTW = ide_s3601_outw;
    hwif->INSW = ide_s3601_insw;
    hwif->OUTSW = ide_s3601_outsw;
}

void default_hwif_transport(hwif_t *hwif)
{
    hwif->ata_input_data = ata_input_data;
    hwif->ata_output_data = ata_output_data;
    hwif->start_dma = ide_s3601_start_rw;
}

UINT32 ide_read24(ide_drive_t *drive)
{
    UINT8 hcyl = HWIF(drive)->INB(IDE_HCYL_REG);
    UINT8 lcyl = HWIF(drive)->INB(IDE_LCYL_REG);
    UINT8 sect = HWIF(drive)->INB(IDE_SECTOR_REG);
    return (hcyl<<16)|(lcyl<<8)|sect;
}

void SELECT_DRIVE(ide_drive_t *drive)
{
    HWIF(drive)->OUTB(drive->select.all, IDE_SELECT_REG);
}

INT32 wait_for_ready (ide_drive_t *drive, int timeout)
{
    hwif_t *hwif    = HWIF(drive);
    UINT8 stat        = 0;

    while(--timeout) {
        stat = hwif->INB(IDE_ALTSTATUS_REG);
        if (!(stat & BUSY_STAT)) {
            if (drive->ready_stat == 0)
                break;
            else if ((stat & drive->ready_stat)||(stat & ERR_STAT))
                break;
        }
        osal_delay(1000);
    }
    if ((stat & ERR_STAT) || timeout <= 0) {
        if (stat & ERR_STAT) {
            IDE_PRINTF("%s: wait_for_ready, "
                "error status: %x\n", drive->name, stat);
        }
        return 1;
    }
    return 0;
}


//added on 2008.05.14 for timeshifting while using ATA to SATA bridge.
INT32 ide_wait_status_not(hwif_t *hwif, UINT32 timeout, UINT8 status)
{
    UINT8 stat = 0;
    UINT32 c = 0;

    while(timeout--)
    {
        osal_delay(1000);
        c ++;

        stat = hwif->INB(hwif->io_ports[IDE_CONTROL_OFFSET]);
        if ((stat & status) == 0)
        {
            IDE_PRINTF("%s()=>Have wait %d ms before success.\n", __FUNCTION__, c);
            return 0;
        }
        /*
         * Assume a value of 0xff means nothing is connected to
         * the interface and it doesn't implement the pull-down
         * resistor on D7.
         */
        if (stat == 0xff)
        {
            IDE_PRINTF("ide_wait_status_not()=>0x%02x Status Error.\n", status);
            return -ENODEV;
        }
    }

    IDE_PRINTF("ide_wait_status_not()=>0x%02x Time Out.\n", stat);
    return -EBUSY;
}


/*
* Description: Wait Alternate Status until it is not busy.
* Return:
*        ->0, success;
*        ->-ENODEV, status error;
*        ->-EBUSY, wait timeout but still busy.
*/
INT32 ide_wait_not_busy(hwif_t *hwif, UINT32 timeout)
{
    UINT8 stat = 0;
    while(timeout--) {
        /*
         * Turn this into a schedule() sleep once I'm sure
         * about locking issues (2.5 work ?).
         */
        osal_delay(1000);

        stat = hwif->INB(hwif->io_ports[IDE_CONTROL_OFFSET]);
        if ((stat & BUSY_STAT) == 0) {
            //IDE_PRINTF("stat: %02x\n", stat);
            return 0;
        }
        /*
         * Assume a value of 0xff means nothing is connected to
         * the interface and it doesn't implement the pull-down
         * resistor on D7.
         */
        if ((stat&0x01) == 0x01)
        {
            IDE_PRINTF("%s()=>Status Error.\n", __FUNCTION__);
            return -ENODEV;
        }
    }
    IDE_PRINTF("%s()=>Time Out.\n", __FUNCTION__);
    return -EBUSY;
}


/*
 * This routine busy-waits for the drive status to be not "busy".
 * It then checks the status for all of the "good" bits and none
 * of the "bad" bits, and if all is okay it returns 0.  All other
 * cases return 1 after invoking ide_error() -- caller should just return.
 *
 * This routine should get fixed to not hog the cpu during extra long waits..
 * That could be done by busy-waiting for the first jiffy or two, and then
 * setting a timer to wake up at half second intervals thereafter,
 * until timeout is achieved, before timing out.
 *
 *Step 1:Wait timeout if IDE status is bsy.
 *Step 2:Check IDE status again if it's "good" & not "bad".
 *
 */
INT32 ide_wait_stat (ide_startstop_t *startstop, ide_drive_t *drive,
    UINT8 good, UINT8 bad, UINT32 timeout)
{
    hwif_t *hwif = HWIF(drive);
    UINT8 stat;
    int i;

/*
    // bail early if we've exceeded max_failures
    if (drive->max_failures && (drive->failures > drive->max_failures)) {
        *startstop = ide_stopped;
        return 1;
    }
*/

    //Step 1.
    osal_delay(1);    /* spec allows drive 400ns to assert "BUSY" */
    if ((stat = hwif->INB(IDE_ALTSTATUS_REG)) & BUSY_STAT)
    {
        //IDE_PRINTF("%s()=>stat: 0x%x.\n", __FUNCTION__, stat);
        timeout += osal_get_tick();
        while ((stat = hwif->INB(IDE_ALTSTATUS_REG)) & BUSY_STAT)
        {
            if (time_after(osal_get_tick(), timeout))
            {
                /*
                 * One last read after the timeout in case
                 * heavy interrupt load made us not make any
                 * progress during the timeout..
                 */
                stat = hwif->INB(IDE_ALTSTATUS_REG);
                if (!(stat & BUSY_STAT))
                    break;

                *startstop = ide_stopped;//DRIVER(drive)->error(drive, "status timeout", stat);
                IDE_PRINTF("\n\n\n%s()=>timeout but Status still Bsy 0x%02x\n\n\n", __FUNCTION__,stat);
                return 1;
            }
        }
    }
    /*
     * Allow status to settle, then read it again.
     * A few rare drives vastly violate the 400ns spec here,
     * so we'll wait up to 10usec for a "good" status
     * rather than expensively fail things immediately.
     * This fix courtesy of Matthew Faupel & Niccolo Rigacci.
     */

    //IDE_PRINTF("%s()=>stat: 0x%x.\n", __FUNCTION__, stat);

    //Step 2.
    for (i = 0; i < 10; i++) {
        osal_delay(1);
        if (OK_STAT((stat = hwif->INB(IDE_ALTSTATUS_REG)), good, bad))
        {
            if (stat & 0x20)
                IDE_PRINTF("%s()=>ALT STATUS Device Fault/Stream Error: %02x", __FUNCTION__, stat);
            *startstop = ide_started;
            return 0;
        }
    }
    for (i = 0; i < 1000; i++) {    //once more,  for ARC772 quick standby / power off
        osal_task_sleep(1);
        if (OK_STAT((stat = hwif->INB(IDE_ALTSTATUS_REG)), good, bad))
        {
            if (stat & 0x20)
                IDE_PRINTF("%s()=>ALT STATUS Device Fault/Stream Error: %02x", __FUNCTION__, stat);
            *startstop = ide_started;
            return 0;
        }
    }
    IDE_PRINTF("\n\n\n%s()=>Status error  0x%02x\n\n\n", __FUNCTION__,stat);
    *startstop = ide_stopped; //DRIVER(drive)->error(drive, "status error", stat);
    return 1;
}


void ide_fixstring (UINT8 *s, const INT32 bytecount, const INT32 byteswap)
{
    UINT8 *p = s, *end = &s[bytecount & ~1]; /* bytecount must be even */

    if (byteswap) {
        /* convert from big-endian to host byte order */
        for (p = end ; p != s;) {
            UINT16 *pp = (unsigned short *) (p -= 2);
            *pp = ((*pp)>>8)|(((*pp)&0xFF)<<8);
        }
    }
    /* strip leading blanks */
    while (s != end && *s == ' ')
        ++s;
    /* compress internal blanks and strip trailing blanks */
    while (s != end && *s) {
        if (*s++ != ' ' || (s != end && *s && *s != ' '))
            *p++ = *(s-1);
    }
    /* wipe out trailing garbage */
    while (p != end)
        *p++ = '\0';
}


/*
* default is the timing while reference clock is 166M
* new is the timing computed acording to the input clock:
*
*    (new + 1) * 1000/clock = (default + 1) * 1000/166M
*    => new = (default + 1) * clock/166M - 1
*    => new = ((default + 1) * clock - 166M)/166M
*
*    if new += 0.5
*    => new = ((default + 1) * clock - 83M)/166M
*/
void ide_s3601_compute_timing(UINT32 clock)
{

    UINT32 i, j;
    UINT32 ref = 166000000; //166M
    UINT32 half_ref = 83000000; //83M
    UINT8 deta=0; //0;//10;//2;//4    // allowed error, if set deta=0 , will access 0xB8010000 fail

    //Avoid Overflow
    ref=ref/1000;
    half_ref=half_ref/1000;
    clock=clock/1000;

    for(i=0;i<5;i++)
    {
        for(j=0;j<4;j++)
        {
            if ((g_pio_reg[i][j]+1)*clock<half_ref)
            {
                g_pio_reg_new[i][j]=0;
            }
            else
            {
                g_pio_reg_new[i][j]=((g_pio_reg[i][j]+1)*clock-half_ref)/ref;
            }

            if((g_pio_data[i][j]+1)*clock<half_ref)
            {
                g_pio_data_new[i][j]=0;
            }
            else
            {
                g_pio_data_new[i][j]=((g_pio_data[i][j]+1)*clock-half_ref)/ref;
            }

            g_pio_reg_new[i][j]&=0xFF;
            g_pio_data_new[i][j]&=0xFF;
            g_pio_reg_new[i][j]+=deta;
            g_pio_data_new[i][j]+=deta;
        }
    }

    for(i=0;i<3;i++)
    {
        for(j=0;j<3;j++)
        {
            if ((g_dma[i][j]+1)*clock<half_ref)
            {
                g_dma_new[i][j]=0;
            }
            else
            {
                g_dma_new[i][j]=((g_dma[i][j]+1)*clock-half_ref)/ref;
            }

                g_dma_new[i][j]&=0xFF;
                g_dma_new[i][j]+=deta;
        }
    }

    for(i=0;i<6;i++)
    {
        for(j=0;j<7;j++)
        {
            if ((g_udma1[i][j]+1)*clock<half_ref)
            {
                g_udma1_new[i][j]=0;
            }
            else
            {
                g_udma1_new[i][j]=((g_udma1[i][j]+1)*clock-half_ref)/ref;
            }

            g_udma1_new[i][j]&=0xFF;
            g_udma1_new[i][j]+=deta;
        }
    }


    for(i=0;i<6;i++)
    {
        for(j=0;j<2;j++)
        {
            if ((g_udma2[i][j]+1)*clock<half_ref)
            {
                g_udma2_new[i][j]=0;
            }
            else
            {
                g_udma2_new[i][j]=((g_udma2[i][j]+1)*clock-half_ref)/ref;
            }

            g_udma2_new[i][j]&=0xFF;
            g_udma2_new[i][j]+=deta;
        }
    }

}


void ide_s3601_set_pio_reg_timing(BYTE mode)
{
    // PIO mode
    UINT32 value;

#if (SYS_CHIP_MODULE == ALI_S3602)
    // 061027-kb : make sure that ( g_pio_reg_new[mode][1] >= g_pio_reg_new[mode][0]+3)
    if ( g_pio_reg_new[mode][1]<(g_pio_reg_new[mode][0]+2))
    {
        g_pio_reg_new[mode][1]=g_pio_reg_new[mode][0]+2;
    }
#endif

    value=g_pio_reg_new[mode][0]<<24|g_pio_reg_new[mode][1]<<16|g_pio_reg_new[mode][2]<<8|g_pio_reg_new[mode][3];
#if 0
    //050905-kb test
    if (mode==0)
    {
        //value = 0x00020402;
        value = 0x00030503;
    }
#endif
    //IDE_PRINTF("S3601_IDE_PIOTCR orig = %08x \r\n", IDE_GET_DWORD(S3601_IDE_PIOTCR)); //kb test
    //IDE_PRINTF("ide_s3601_set_pio_reg_timing mode=%d , value = %08x\n", mode, value);

    IDE_SET_DWORD(S3601_IDE_PIOTCR, value);
}

void ide_s3601_set_pio_data_timing(BYTE mode)
{
    // PIO burst mode
    UINT32 value;

#if (SYS_CHIP_MODULE == ALI_S3602)
    // 061027-kb : make sure that ( g_pio_data_new[mode][1] >= g_pio_data_new[mode][0]+3)
    if ( g_pio_data_new[mode][1]<(g_pio_data_new[mode][0]+2))
    {
        g_pio_data_new[mode][1]=g_pio_data_new[mode][0]+2;
    }
#endif

    value=g_pio_data_new[mode][0]<<24|g_pio_data_new[mode][1]<<16|g_pio_data_new[mode][2]<<8|g_pio_data_new[mode][3];
    IDE_SET_DWORD(S3601_IDE_PIOTCR, value);
}

void ide_s3601_set_dma_timing(BYTE mode)
{
    // dma mode
    UINT32 value;

#if (SYS_CHIP_MODULE == ALI_S3602)
    // 061027-kb : make sure that ( g_dma_new[mode][1] >= g_dma_new[mode][0]+3)
    if ( g_dma_new[mode][1]<(g_dma_new[mode][0]+3))
    {
            g_dma_new[mode][1]=g_dma_new[mode][0]+3;
    }
#endif

    value=g_dma_new[mode][0]<<16|g_dma_new[mode][1]<<8|g_dma_new[mode][2];
#if 0    // for 12.288MHZ
    //050905-kb test
    if (mode==0)
    {
        value = 0x00000303;
    }
    if (mode==1)
    {
        value = 0x00000505;
    }
    if (mode==2)
    {
        value = 0x00000505;
    }
    //IDE_PRINTF("S3601_IDE_DMATCR orig = %08x \r\n", IDE_GET_DWORD(S3601_IDE_DMATCR)); //kb test
    //IDE_PRINTF("ide_s3601_set_dma_timing mode=%d , value = %08x \r\n", mode, value);
#endif
#if 0    // for 33MHZ
    if (mode==0)
    {
        value = 0x00010404;
    }
    if (mode==1)
    {
        value = 0x00010606;
    }
    if (mode==2)
    {
        value = 0x00010606;
    }
    //IDE_PRINTF("S3601_IDE_DMATCR orig = %08x \r\n", IDE_GET_DWORD(S3601_IDE_DMATCR)); //kb test
    //IDE_PRINTF("ide_s3601_set_dma_timing mode=%d , value = %08x \r\n", mode, value);
#endif

#if 0    // for 30MHZ , FPGA , munuaully adjust the timing counters
    if (mode==0)
    {
        value = 0x00010404;
    }
    if (mode==1)
    {
        value = 0x00000303;
    }
    if (mode==2)
    {
        value = 0x00000302;
    }
    //IDE_PRINTF("S3601_IDE_DMATCR orig = %08x \r\n", IDE_GET_DWORD(S3601_IDE_DMATCR)); //kb test
    //IDE_PRINTF("ide_s3601_set_dma_timing mode=%d , value = %08x \r\n", mode, value);
#endif

    IDE_SET_DWORD(S3601_IDE_DMATCR, value&0x00FFFFFF);
}

void ide_s3601_set_udma_timing(BYTE mode)
{
    // ultra dma mode
    UINT32 value;
#if 0    //1try to test
    if(mode<=4)
        g_udma1_new[mode][6]=1;
    else
        g_udma1_new[mode][6]=0;
#endif
//1 in s3602, these code will be modified for new offset
//1T4ENV[3:0], T4ACK[3:0], T4DVS[3:0] and T4RP[5:0] are extended 1 bit depth to fit highe mem_clk frequncy=198mhz
    value=g_udma1_new[mode][0]<<24|g_udma1_new[mode][1]<<20|
        g_udma1_new[mode][2]<<16|g_udma1_new[mode][3]<<12|
        g_udma1_new[mode][4]<<8|g_udma1_new[mode][5]<<4|
        g_udma1_new[mode][6];

    IDE_SET_DWORD(S3601_IDE_UDMATCR1, value);

#if (SYS_CHIP_MODULE == ALI_S3602)
    // 061027-kb : make sure that ( g_udma2_new[mode][1] >= (g_udma1_new[mode][6]+3))
    if ( g_udma2_new[mode][1]<(g_udma1_new[mode][6]+3))
    {
        g_udma2_new[mode][1]=g_udma1_new[mode][6]+3;
    }
#endif

    value=g_udma2_new[mode][0]<<8|g_udma2_new[mode][1];
    IDE_SET_DWORD(S3601_IDE_UDMATCR2, value&0x00001FFF);
}


/*
Mask: bit 4~0
    -> 1: disable
    -> 0: enable
*/
void ide_s3601_set_interrupt_mask(UINT8 mask)
{
    IDE_SET_BYTE(S3601_IDE_ICR, mask&0x1F);
}


void ide_s3601_enable_grant(UINT8 len)
{
    // set GRANT_LEN
    IDE_SET_BYTE(S3601_IDE_GLR,len);
    // set GRANT_EN, bit 20=1
    IDE_SET_DWORD(S3601_IDE_ATACR, IDE_GET_DWORD(S3601_IDE_ATACR)|0x00100000);
}

void ide_s3601_disable_grant()
{
    // set GRANT_EN, bit 20=0
    IDE_SET_DWORD(S3601_IDE_ATACR, IDE_GET_DWORD(S3601_IDE_ATACR)&0xFFEFFFFF);
}

INT32 ide_s3601_stop_transfer()
{
    UINT32 flag;
    extern OSAL_ID ide_flag_id;
    // set STOP_ATA, bit 24=1
    IDE_SET_DWORD(S3601_IDE_ATACR, IDE_GET_DWORD(S3601_IDE_ATACR)|0x01000000);

#if 0
    while(1)
    {
        UINT8 status;
        // polling TXEND_INT, bit 1=1
        status=IDE_GET_BYTE(S3601_IDE_ISR);
        if(status&0x01)
        {
            break;
        }
    }
#else
    if (OSAL_E_TIMEOUT == osal_flag_wait(&flag, ide_flag_id, IDE_FLAG_TXEND_INT, TWF_ANDW|TWF_CLR, 10000)) {
        IDE_PRINTF("wait WDMA time out!\n");
        return ERR_FAILUE;
    }
#endif
    return SUCCESS;
}

INT32 ide_s3601_pause_transfer()
{
    volatile UINT32 val;
    // set PAUSE_ATA, bit 29=1
    IDE_SET_DWORD(S3601_IDE_ATACR, IDE_GET_DWORD(S3601_IDE_ATACR)|0x20000000);

    while(1)
    {
        // polling ATATXPAUSE, bit 30=1
        val=IDE_GET_DWORD(S3601_IDE_ATACR);
        if(val&0x40000000)
        {
            break;
        }
    }

    return SUCCESS;
}

void ide_s3601_resume_transfer()
{
    // set PAUSE_ATA, bit 29=0
    IDE_SET_DWORD(S3601_IDE_ATACR, IDE_GET_DWORD(S3601_IDE_ATACR)&0xDFFFFFFF);

}

void ide_s3601_set_timeout(UINT16 time)
{
    // set TIMEOUT_CNT
    IDE_SET_WORD(S3601_IDE_ATATCR, time);
}


void ide_s3601_read_all_reg()
{
#ifdef _RD_DEBUG_
    UINT16 dar;
    BYTE aar;
    BYTE fsmcr;
    UINT32 atacr;
    UINT32 bc4tr;    // 24 bit
    BYTE glr;
    UINT32 tbcr;        // 24 bit
    BYTE gcr;
    UINT32 piotcr;
    UINT32 dmatcr;
    UINT32 udmatcr1;
    UINT32 udmatcr2;
    BYTE icr;
    BYTE isr;
    UINT16 atatcr;
    UINT16 pwcr;
    BYTE psr;
    BYTE res;
    UINT32 pbcr;
    BYTE sisr;
    UINT16 ata2ch_data;
    UINT16 ch2ata_data;
    BYTE piocsr;
    BYTE dmacsr;
    BYTE udmacsr;
    BYTE rdchdcsr;
    BYTE rsccsr;

    UINT32 atabfsadr;
    UINT32 atabfdadr;
    BYTE debugsel;
    BYTE ataffwptr;
    BYTE ataffrptr;
    BYTE ataffsr;
    BYTE ataffpftr;

    // read value

    dar=IDE_GET_WORD(S3601_IDE_DAR);
    aar=IDE_GET_BYTE(S3601_IDE_AAR);
    fsmcr=IDE_GET_BYTE(S3601_IDE_FSMCR);
    atacr=IDE_GET_DWORD(S3601_IDE_ATACR);
    bc4tr=IDE_GET_DWORD(S3601_IDE_BC4TR)&0x00FFFFFF;
    glr=IDE_GET_BYTE(S3601_IDE_GLR);
    tbcr=IDE_GET_DWORD(S3601_IDE_TBCR)&0x00FFFFFF;    // low 24 bit
    gcr=IDE_GET_BYTE(S3601_IDE_GCR);
    piotcr=IDE_GET_DWORD(S3601_IDE_PIOTCR);
    dmatcr=IDE_GET_DWORD(S3601_IDE_DMATCR);
    udmatcr1=IDE_GET_DWORD(S3601_IDE_UDMATCR1);
    udmatcr2=IDE_GET_DWORD(S3601_IDE_UDMATCR2);
    icr=IDE_GET_BYTE(S3601_IDE_ICR);
    isr=IDE_GET_BYTE(S3601_IDE_ISR);

    atatcr=IDE_GET_WORD(S3601_IDE_ATATCR);
    pwcr=IDE_GET_WORD(S3601_IDE_PWCR);
    psr=IDE_GET_BYTE(S3601_IDE_PSR);
    res=IDE_GET_BYTE(S3601_IDE_RESERVED);

    pbcr=IDE_GET_DWORD(S3601_IDE_PBCR);
    sisr=IDE_GET_BYTE(S3601_IDE_SISR);
    ata2ch_data=IDE_GET_WORD(S3601_IDE_ATA2CH_DATA);
    ch2ata_data=IDE_GET_WORD(S3601_IDE_CH2ATA_DATA);

    piocsr=IDE_GET_BYTE(S3601_IDE_PIOCSR);
    dmacsr=IDE_GET_BYTE(S3601_IDE_DMACSR);
    udmacsr=IDE_GET_BYTE(S3601_IDE_UDMACSR);
    rdchdcsr=IDE_GET_BYTE(S3601_IDE_RDCHDCSR);
    rsccsr=IDE_GET_BYTE(S3601_IDE_RSCCSR);

    atabfsadr=IDE_GET_DWORD(S3601_IDE_ATABFSADR);
    atabfdadr=IDE_GET_DWORD(S3601_IDE_ATABFDADR);

    debugsel=IDE_GET_BYTE(S3601_IDE_DEBUGSEL);
    ataffwptr=IDE_GET_BYTE(S3601_IDE_ATAFFWPTR);
    ataffrptr=IDE_GET_BYTE(S3601_IDE_ATAFFRPTR);
    ataffsr=IDE_GET_BYTE(S3601_IDE_ATAFFSR);
    ataffpftr=IDE_GET_BYTE(S3601_IDE_ATAFFPFTR);


    // printf value
    libc_printf("[0x00] dar: 0x%X \r\n",dar);
    libc_printf("[0x02] aar: 0x%X \r\n",aar);
    libc_printf("[0x03] fsmcr: 0x%X \r\n",fsmcr);
    libc_printf("[0x04] atacr: 0x%X \r\n",atacr);
    libc_printf("[0x08] bc4tr: 0x%X \r\n",bc4tr);
    libc_printf("[0x0B] glr: 0x%X \r\n",glr);
    libc_printf("[0x0C] tbcr: 0x%X \r\n",tbcr);
    libc_printf("[0x0F] gcr: 0x%X \r\n",gcr);
    libc_printf("[0x10] piotcr: 0x%X \r\n",piotcr);
    libc_printf("[0x14] dmatcr: 0x%X \r\n",dmatcr);
    libc_printf("[0x18] udmatcr1: 0x%X \r\n",udmatcr1);
    libc_printf("[0x1C] udmatcr2: 0x%X \r\n",udmatcr2);
    libc_printf("[0x20] icr: 0x%X \r\n",icr);
    libc_printf("[0x21] isr: 0x%X \r\n",isr);
    libc_printf("[0x22] atatcr: 0x%X \r\n",atatcr);

    libc_printf("[0x24] pwcr: 0x%X \r\n",pwcr);
    libc_printf("[0x26] psr: 0x%X \r\n",psr);
    libc_printf("[0x27] res: 0x%X \r\n",res);
    libc_printf("[0x28] pbcr: 0x%X \r\n",pbcr);
    libc_printf("[0x2B] sisr: 0x%X \r\n",sisr);
    libc_printf("[0x2C] ata2ch_data: 0x%X \r\n",ata2ch_data);
    libc_printf("[0x2E] ch2ata_data: 0x%X \r\n",ch2ata_data);
    libc_printf("[0x30] piocsr: 0x%X \r\n",piocsr);
    libc_printf("[0x31] dmacsr: 0x%X \r\n",dmacsr);
    libc_printf("[0x32] udmacsr: 0x%X \r\n",udmacsr);
    libc_printf("[0x33] rdchdcsr: 0x%X \r\n",rdchdcsr);
    libc_printf("[0x34] rsccsr: 0x%X \r\n",rsccsr);
    libc_printf("[0x80] atabfsadr: 0x%X \r\n",atabfsadr);
    libc_printf("[0x84] atabfdadr: 0x%X \r\n",atabfdadr);
    libc_printf("[0x88] debugsel: 0x%X \r\n",debugsel);

    libc_printf("[0x89] ataffwptr: 0x%X \r\n",ataffwptr);
    libc_printf("[0x8A] ataffrptr: 0x%X \r\n",ataffrptr);
    libc_printf("[0x8B] ataffsr: 0x%X \r\n",ataffsr);
    libc_printf("[0x8C] ataffsr: 0x%X \r\n",ataffpftr);
#endif
}

void ide_s3601_set_pio_mode_id(ide_drive_t* drive,UINT8 modeid)
{
    UINT8 value;
    value=0x08+modeid;
    ide_exec_command(drive, WIN_SETFEATURES, SETFEATURES_XFER, value);
    ide_s3601_set_pio_reg_timing(modeid);
    ide_set_rw_mode(IDE_PIO_MODE,modeid);
}

void ide_s3601_set_piob_mode_id(ide_drive_t* drive,UINT8 modeid)
{
    UINT8 value;
    value=0x08+modeid;        // the same with PIO mode
    //ide_s3601_set_pio_data_timing(modeid);    //it is more safe to set it later
    ide_exec_command(drive, WIN_SETFEATURES, SETFEATURES_XFER, value);
    ide_set_rw_mode(IDE_PIO_BUR_MODE,modeid);
}

void ide_s3601_set_dma_mode_id(ide_drive_t* drive,UINT8 modeid)
{
    UINT8 value;
    value=0x20+modeid;
    ide_exec_command(drive, WIN_SETFEATURES, SETFEATURES_XFER, value);
    ide_s3601_set_dma_timing(modeid);
    ide_set_rw_mode(IDE_DMA_MODE,modeid);
}

void ide_s3601_set_udma_mode_id(ide_drive_t* drive,UINT8 modeid)
{
    UINT8 value;
    //hwif_t *hwif = HWIF(drive);
    value=0x40+modeid;
    ide_exec_command(drive, WIN_SETFEATURES, SETFEATURES_XFER, value);
    ide_s3601_set_udma_timing(modeid);
    ide_set_rw_mode(IDE_UDMA_MODE,modeid);
}

void ide_s3601_set_swap_ata(UINT8 enable)
{
    // set swap for PIOB/DMA/UDMA read operation
    if(enable)
    {
        // set ATADSWAP, bit 18=1
        IDE_SET_DWORD(S3601_IDE_ATACR, IDE_GET_DWORD(S3601_IDE_ATACR)|0x00040000);
    }
    else
    {
        // set ATADSWAP, bit 18=0
        IDE_SET_DWORD(S3601_IDE_ATACR, IDE_GET_DWORD(S3601_IDE_ATACR)&0xFFFBFFFF);
    }

}


void ide_s3601_set_chd_swap_atafifo(UINT8 enable)
{
    // set swap for PIOB/DMA/UDMA write operation
    if(enable)
    {
        // set CHDSWAP, bit 19=1
        IDE_SET_DWORD(S3601_IDE_ATACR, IDE_GET_DWORD(S3601_IDE_ATACR)|0x00080000);
    }
    else
    {
        // set CHDSWAP, bit 19=0
        IDE_SET_DWORD(S3601_IDE_ATACR, IDE_GET_DWORD(S3601_IDE_ATACR)&0xFFF7FFFF);
    }

}


