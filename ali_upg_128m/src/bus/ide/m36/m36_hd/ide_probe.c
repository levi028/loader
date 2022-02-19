#include <types.h>
#include <sys_config.h>

#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libc/alloc.h>

#include <osal/osal.h>

#include <bus/ide/ide.h>
#include <bus/ide/hdreg.h>

#include "ide_reg.h"
#include "ide_iops.h"

extern void SELECT_DRIVE(ide_drive_t *drive);
extern void ide_s3601_set_pio_mode_id(ide_drive_t* drive,UINT8 modeid);
extern void ide_s3601_set_interrupt_mask(UINT8 mask);
extern void ide_s3601_compute_timing(UINT32 clock);
extern void ide_fixstring (UINT8 *s, const INT32 bytecount, const INT32 byteswap);
extern UINT8 ide_get_rw_mode(void);

hwif_t ide_hwif;

//INT32 int_count = 0;


/*
* Description: Wait IDE status until it's not busy,
*            then make sure Master HDD is ready.
*/
static INT32 wait_hwif_ready(struct hwif_s *hwif)
{
    INT32 rc;

    IDE_PRINTF("wait_hwif_ready()=>Check IDE M&S Before Probing %s.\n", hwif->name);

    /* Let HW settle down a bit from whatever init state we
     * come from
     */
    osal_delay(2000);

    /* Wait for BSY bit to go away, spec timeout is 30 seconds,
     * I know of at least one disk who takes 31 seconds, I use 35
     * here to be safe
     */
    if(ALI_M3329E==sys_ic_get_chip_id())
    {
        rc = ide_wait_not_busy(hwif, 150000);
        if (rc)
        {
            IDE_PRINTF("wait_hwif_ready()=>Return Err=>%d\n", rc);
            return rc;
        }
        IDE_PRINTF("wait_hwif_ready()=>Device Not Bsy Fst.\n");
    }
    else
        rc = ide_wait_not_busy(hwif, 350000);

    if(rc!=0)
        return !RET_SUCCESS;
    /* Now make sure both master & slave are ready */
    SELECT_DRIVE(&hwif->drives[0]);
    osal_delay(1);
    hwif->OUTB(8, hwif->io_ports[IDE_CONTROL_OFFSET]);
    osal_delay(2000);
    rc = ide_wait_not_busy(hwif, 100000);
    IDE_PRINTF("wait_hwif_ready()=>rc from ide_wait_not_busy() for Master=>Return %d.\n", rc);

/*
    //not support Slave HDD currently here.
    SELECT_DRIVE(&hwif->drives[1]);
    hwif->OUTB(8, hwif->io_ports[IDE_CONTROL_OFFSET]);
    osal_delay(2000);
    rc = ide_wait_not_busy(hwif, 100000);
    IDE_PRINTF("wait_hwif_ready()=>rc from ide_wait_not_busy() for Slave=>Return %d.\n", rc);
*/
    /* Exit function with master reselected (let's be sane) */
    SELECT_DRIVE(&hwif->drives[0]);

    return rc;

}


/* DMASET */
#define DMA_EN                0x01
#define DMA_IOEN            0x02
#define DMA_RDSTB            0x04

static OSAL_ID ide_sema_id = OSAL_INVALID_ID;
OSAL_ID ide_flag_id = OSAL_INVALID_ID;

volatile unsigned long g_interrupt_num=18;
//volatile unsigned long g_interrupt_flag[1000];
//volatile unsigned long g_hsr_num=0;
//volatile unsigned long g_reg_hsr_num=0;


void ide_hsr(UINT32 flag)
{
    //IDE_PRINTF("%s()=>isr flag is 0x%X \r\n", __FUNCTION__, flag);
    //flag=g_interrupt_flag[g_interrupt_num-1];

    //ASSERT(g_interrupt_flag[g_hsr_num]==flag);
    //ASSERT(g_interrupt_flag[g_interrupt_num-1]==flag);
    //g_hsr_num++;

#if 1
    if (IDE_INTR_ATA_INT2B == (flag&IDE_INTR_ATA_INT2B)) {
        osal_flag_set(ide_flag_id, IDE_FLAG_ATA_INT2B);
        //libc_printf("h");
    } else if (IDE_INTR_TXEND_INT == (flag&IDE_INTR_TXEND_INT)) {
        osal_flag_set(ide_flag_id, IDE_FLAG_TXEND_INT);
    } else if (IDE_INTR_TSTOP_INT == (flag&IDE_INTR_TSTOP_INT)) {
        osal_flag_set(ide_flag_id, IDE_FLAG_TSTOP_INT);
    }else if (IDE_INTR_SYNC_ATAINTRQ == (flag&IDE_INTR_SYNC_ATAINTRQ)) {
        osal_flag_set(ide_flag_id, IDE_FLAG_SYNC_ATAINTRQ);
    }else if (IDE_INTR_CLR_TOINT == (flag&IDE_INTR_CLR_TOINT)) {
        osal_flag_set(ide_flag_id, IDE_FLAG_CLR_TOINT);
    } else {
        ASSERT(0);
    }
#else
    osal_flag_set(ide_flag_id, flag);
#endif

}


/*
*    Check interrupt status, Return interrupt flags.
*/
UINT32 hwif_check_int(hwif_t *hwif)
{
    UINT32 flag=0;
    BYTE status;

    status= IDE_GET_BYTE(S3601_IDE_ISR);
    if(status&IDE_INTR_ATA_INT2B)
        flag|=IDE_INTR_ATA_INT2B;
    if(status&IDE_INTR_TXEND_INT)
        flag|=IDE_INTR_TXEND_INT;
    if(status&IDE_INTR_TSTOP_INT)
        flag|=IDE_INTR_TSTOP_INT;
    if(status&IDE_INTR_SYNC_ATAINTRQ)
        flag|=IDE_INTR_SYNC_ATAINTRQ;
    if(status&IDE_INTR_CLR_TOINT)
        flag|=IDE_INTR_CLR_TOINT;

    return flag;
}


/*
*    Only ATA transfer end Interrupt is being used here.
*/
void ide_interrupt_lsr(UINT32 para)
{
    hwif_t *hwif = (hwif_t *)para;
    ide_drive_t *drive = &hwif->drives[0];
    UINT32 flag;
    UINT32 rw_mode;

    flag=hwif_check_int(hwif);

    //libc_printf("-%x",flag);

    //Clear IDE status.
    (void) hwif->INB(IDE_STATUS_REG);

    //Clear Interrupt Status.
    IDE_SET_BYTE(S3601_IDE_ISR,0x1F);

    rw_mode=ide_get_rw_mode();
    if(rw_mode==IDE_PIO_BUR_MODE)
    {
        //Enable PIO burst transfer. ATACR: bit16.
        IDE_SET_DWORD(S3601_IDE_ATACR, IDE_GET_DWORD(S3601_IDE_ATACR)&(~0x00010000));
    }

    //Clear Control register
    //*(unsigned long*)0xB8000030=(*(unsigned long*)0xB8000030)&(~0x400);    // clear bit 10

    //IDE_PRINTF("%s()=>isr flag is 0x%X \r\n", __FUNCTION__, flag);

    ASSERT(flag!=0);
    //if(0 != flag) {    //may have problem, if bit 3(0x20) have not been mask for PIOB read operation
    if(flag&IDE_INTR_TXEND_INT){
/*
    if(flag&IDE_INTR_TSTOP_INT){
    if(flag&IDE_INTR_SYNC_ATAINTRQ){
        g_reg_hsr_num++;
        int_count ++;
        (volatile UINT8)hwif->stat = intr;
        1osal_interrupt_register_hsr(ide_hsr, flag);
        osal_interrupt_register_hsr(ide_hsr, flag|(g_reg_hsr_num<<16));
*/
        osal_interrupt_register_hsr(ide_hsr, flag);
    }
}


/**
 *    drive_is_flashcard    -    check for compact flash
 *    @drive: drive to check
 *
 *    CompactFlash cards and their brethern pretend to be removable
 *    hard disks, except:
 *         (1) they never have a slave unit, and
 *        (2) they don't have doorlock mechanisms.
 *    This test catches them, and is invoked elsewhere when setting
 *    appropriate config bits.
 *
 *    FIXME: This treatment is probably applicable for *all* PCMCIA (PC CARD)
 *    devices, so in linux 2.3.x we should change this to just treat all
 *    PCMCIA  drives this way, and get rid of the model-name tests below
 *    (too big of an interface change for 2.4.x).
 *    At that time, we might also consider parameterizing the timeouts and
 *    retries, since these are MUCH faster than mechanical drives. -M.Lord
 */

static inline int drive_is_flashcard (ide_drive_t *drive)
{
    struct hd_driveid *id = drive->id;

    if (drive->removable) {
        if (id->config == 0x848a) return 1;    /* CompactFlash */
        if (MEMCMP(id->model, "KODAK ATA_FLASH", 15)    /* Kodak */
         || MEMCMP(id->model, "Hitachi CV", 10)    /* Hitachi */
         || MEMCMP(id->model, "SunDisk SDCFB", 13)    /* old SanDisk */
         || MEMCMP(id->model, "SanDisk SDCFB", 13)    /* SanDisk */
         || MEMCMP(id->model, "HAGIWARA HPC", 12)    /* Hagiwara */
         || MEMCMP(id->model, "LEXAR ATA_FLASH", 15)    /* Lexar */
         || MEMCMP(id->model, "ATA_FLASH", 9))    /* Simple Tech */
        {
            return 1;    /* yes, it is a flash memory card */
        }
    }
    return 0;    /* no, it is not a flash memory card */
}



/**
 *    do_identify    -    identify a drive
 *    @drive: drive to identify
 *    @cmd: command used
 *
 *    Called when we have issued a drive identify command to
 *    read and parse the results. This function is run with
 *    interrupts disabled.
 */
static inline void do_identify (ide_drive_t *drive, UINT8 cmd)
{
    hwif_t *hwif = HWIF(drive);
    int bswap = 1;
    struct hd_driveid *id;

    id = drive->id;
    /* read 512 bytes of id info */
    hwif->ata_input_data(drive, id, SECTOR_SIZE);

    drive->id_read = 1;

    /*
     *  WIN_IDENTIFY returns little-endian info,
     *  WIN_PIDENTIFY *usually* returns little-endian info.
     */
    if (cmd == WIN_PIDENTIFY) {
        if ((id->model[0] == 'N' && id->model[1] == 'E') /* NEC */
         || (id->model[0] == 'F' && id->model[1] == 'X') /* Mitsumi */
         || (id->model[0] == 'P' && id->model[1] == 'i'))/* Pioneer */
            /* Vertos drives may still be weird */
            bswap ^= 1;
    }
    ide_fixstring(id->model,     sizeof(id->model),     bswap);
    ide_fixstring(id->fw_rev,    sizeof(id->fw_rev),    bswap);
    ide_fixstring(id->serial_no, sizeof(id->serial_no), bswap);
    /* we depend on this a lot! */
    id->model[sizeof(id->model)-1] = '\0';
    IDE_PRINTF("do_identify()=>Device Info=>name:%s, model:%s, serial_no:%s, type:", drive->name, id->model, id->serial_no);
    drive->present = 1;
    drive->dead = 0;

    /*
     * Check for an ATAPI device
     */
    if (cmd == WIN_PIDENTIFY) {
        UINT8 type = (id->config >> 8) & 0x1f;
        IDE_PRINTF("ATAPI ");
        switch (type) {
            case ide_floppy:
                type = ide_cdrom;
            case ide_cdrom:
                drive->removable = 1;
                IDE_PRINTF("CD/DVD-ROM");
                break;
            case ide_tape:
                IDE_PRINTF("TAPE");
                break;
            case ide_optical:
                IDE_PRINTF("OPTICAL");
                drive->removable = 1;
                break;
            default:
                IDE_PRINTF("UNKNOWN (type %d)", type);
                break;
        }
        IDE_PRINTF(".\n");
        drive->media = type;
        return;
    }

    /*
     * Not an ATAPI device: looks like a "regular" hard disk
     */
    if (id->config & (1<<7))
        drive->removable = 1;

    if (drive_is_flashcard(drive))
        drive->is_flash = 1;

    drive->media = ide_disk;
    IDE_PRINTF("%s.\n", (drive->is_flash) ? "CFA" : "ATA" );
    return;

}


/*
 *    actual_try_to_identify    -    send ata/atapi identify
 *    @drive: drive to identify
 *    @cmd: command to use
 *
 *    try_to_identify() sends an ATA(PI) IDENTIFY request to a drive
 *    and waits for a response.  It also monitors irqs while this is
 *    happening, in hope of automatically determining which one is
 *    being used by the interface.
 *
 *    Returns:    0  device was identified
 *            1  device timed-out (no response to identify request)
 *            2  device aborted the command (refused to identify itself)
 *    Step1: Send the IDENTIFY DEVICE command.
 *    Step2: Receive the IDENTIFY DEVICE data.
 */
static INT32 actual_try_to_identify (ide_drive_t *drive, UINT8 cmd)
{
    hwif_t *hwif = HWIF(drive);
    INT32 rc;
    unsigned long hd_status;
    unsigned long timeout;
    UINT8 s = 0, a = 0;

    /* take a deep breath */
    osal_delay(50000);

    if (IDE_CONTROL_REG) {
        a = hwif->INB(IDE_ALTSTATUS_REG);
        s = hwif->INB(IDE_STATUS_REG);
        if ((a ^ s) & ~INDEX_STAT) {
            IDE_PRINTF("%s: probing with STATUS(0x%02x) instead of "
                "ALTSTATUS(0x%02x)\n", drive->name, s, a);
            /* ancient Seagate drives, broken interfaces */
            hd_status = IDE_STATUS_REG;
        } else {
            /* use non-intrusive polling */
            hd_status = IDE_ALTSTATUS_REG;
        }
    } else
        hd_status = IDE_STATUS_REG;

    /* set features register for atapi
     * identify command to be sure of reply
     */
    if ((cmd == WIN_PIDENTIFY))
        /* disable dma & overlap */
        hwif->OUTB(0, IDE_FEATURE_REG);

    /* ask drive for ID */
    hwif->OUTB(cmd, IDE_COMMAND_REG);

    timeout = ((cmd == WIN_IDENTIFY) ? WAIT_WORSTCASE : WAIT_PIDENTIFY) / 2;
    timeout += osal_get_tick();
    do {
        if (time_after(osal_get_tick(), timeout)) {
            /* drive timed-out */
            return 1;
        }
        /* give drive a breather */
        osal_delay(50000);
    } while ((hwif->INB(hd_status)) & BUSY_STAT);

    /* wait for IRQ and DRQ_STAT */
    osal_delay(50000);
    if (OK_STAT((hwif->INB(IDE_STATUS_REG)), DRQ_STAT, BAD_R_STAT)) {
        //unsigned long flags;
        /* drive returned ID */
        do_identify(drive, cmd);
        /* drive responded with ID */
        rc = 0;
        /* clear drive IRQ */
        (void) hwif->INB(IDE_STATUS_REG);
        //local_irq_restore(flags);
    } else {
        /* drive refused ID */
        rc = 2;
    }
    return rc;
}


/*
 *    try_to_identify    -    try to identify a drive
 *    @drive: drive to probe
 *    @cmd: command to use
 *
 *    Issue the identify command and then do IRQ probing to
 *    complete the identification when needed by finding the
 *    IRQ the drive is attached to
 */
static INT32 try_to_identify (ide_drive_t *drive, UINT8 cmd)
{
    hwif_t *hwif = HWIF(drive);
    INT32 retval;
    /*
     * Disable device irq unless we need to
     * probe for it. Otherwise we'll get spurious
     * interrupts during the identify-phase that
     * the irq handler isn't expecting.
     */
    hwif->OUTB(10, IDE_CONTROL_REG);        // disable interrupt

    retval = actual_try_to_identify(drive, cmd);

    hwif->OUTB(8, IDE_CONTROL_REG);        // enable interrupt

    return retval;
}


/*
 *    do_probe        -    probe an IDE device
 *    @drive: drive to probe
 *    @cmd: command to use
 *
 *    do_probe() has the difficult job of finding a drive if it exists,
 *    without getting hung up if it doesn't exist, without trampling on
 *    ethernet cards, and without leaving any IRQs dangling to haunt us later.
 *
 *    If a drive is "known" to exist (from CMOS or kernel parameters),
 *    but does not respond right away, the probe will "hang in there"
 *    for the maximum wait time (about 30 seconds), otherwise it will
 *    exit much more quickly.
 *
 *    Returns:
 *        0  device was identified
 *        1  device timed-out (no response to identify request)
 *        2  device aborted the command (refused to identify itself)
 *        3  bad status from device (possible for ATAPI drives)
 *        4  probe was not attempted because failure was obvious
 *
 *    Step 1: Check IDE status ag & ag, make sure it's all right.
 *    Step 2: Try to identify a device.
 */
static INT32 do_probe (ide_drive_t *drive, UINT8 cmd)
{
    INT32 rc;
    UINT8 value;
    hwif_t *hwif = HWIF(drive);

    if (drive->present) {
        /* avoid waiting for inappropriate probes */
        if ((drive->media != ide_disk) && (cmd == WIN_IDENTIFY))
            return 4;
    }
/*
    IDE_PRINTF("probing for %s: present=%d, media=%d, probetype=%s\n",
        drive->name, drive->present, drive->media,
        (cmd == WIN_IDENTIFY) ? "ATA" : "ATAPI");
*/

    /* needed for some systems
     * (e.g. crw9624 as drive0 with disk as slave)
     */
    osal_delay(50000);
    SELECT_DRIVE(drive);

    UINT32 i = 0;
    for(i=1;i<100;i++)
    {
        osal_task_sleep(100);         //for ARC772 ATA->SATA bridge
        value=hwif->INB(IDE_SELECT_REG);
        if(value == drive->select.all)
        {
            IDE_PRINTF("do_probe()=>have waited Write-Read-Compare Select Reg for %d ms.\n", i*100);
            break;
        }
    }
    if(i > 99)
    {
        IDE_PRINTF("do_probe()=>have waited Write-Read-Compare Select Reg time out.\n");
        drive->present = 0;
        return 1;
    }
    if ((value=hwif->INB(IDE_SELECT_REG)) != drive->select.all && !drive->present) {
        if (drive->select.b.unit != 0) {
            /* exit with drive0 selected */
            SELECT_DRIVE(&hwif->drives[0]);
            /* allow BUSY_STAT to assert & clear */
            osal_delay(50000);
        }
        /* no i/f present: mmm.. this should be a 4 -ml */
        PRINTF("do_probe()=>Select Reg Value is %02x\n", value);
        return 3;
    }

    for(i=1;i<100;i++)
    {
        osal_task_sleep(100);         //for ARC772 ATA->SATA bridge
        value=hwif->INB(IDE_STATUS_REG);
        //if((value|0x80) != 0x80)
        if((value&0x80) != 0x80)
        {
            IDE_PRINTF("do_probe()=>have waited Not Bsy of Status Reg for %d ms.\n", i*100);
            break;
        }
    }
    if(i >= 99)
    {
        IDE_PRINTF("do_probe()=>have waited Write-Read-Compare Select Reg time out 2.\n");
        drive->present = 0;
        return 1;
    }
    if (OK_STAT((hwif->INB(IDE_STATUS_REG)), READY_STAT, BUSY_STAT) ||
        drive->present || cmd == WIN_PIDENTIFY) {
        /* send cmd and wait */
        if ((rc = try_to_identify(drive, cmd))) {
            /* failed: try again */
            rc = try_to_identify(drive,cmd);
        }
        if (hwif->INB(IDE_STATUS_REG) == (BUSY_STAT|READY_STAT))
            return 4;

        if (rc == 1 && cmd == WIN_PIDENTIFY) {
            unsigned long timeout;
            IDE_PRINTF("do_probe()=>Identify Packet Dev=>%s not response (status = 0x%02x), "
                "resetting drive\n", drive->name,
                hwif->INB(IDE_STATUS_REG));
            osal_delay(5000);
            hwif->OUTB(drive->select.all, IDE_SELECT_REG);
            osal_delay(5000);
            hwif->OUTB(WIN_SRST, IDE_COMMAND_REG);
            timeout = osal_get_tick();
            while (((hwif->INB(IDE_STATUS_REG)) & BUSY_STAT) &&
                   time_before(osal_get_tick(), timeout + WAIT_WORSTCASE))
                osal_task_sleep(50);
            rc = try_to_identify(drive, cmd);
        }
        if (rc == 1)
            IDE_PRINTF("do_probe()=>Identify Dev=>%s not response (status = 0x%02x)\n",
                drive->name, hwif->INB(IDE_STATUS_REG));
        /* ensure drive irq is clear */
        (void) hwif->INB(IDE_STATUS_REG);
    } else {
        /* not present or maybe ATAPI */
        rc = 3;
    }
    if (drive->select.b.unit != 0) {
        /* exit with drive0 selected */
        SELECT_DRIVE(&hwif->drives[0]);
        osal_delay(50000);
        /* ensure drive irq is clear */
        (void) hwif->INB(IDE_STATUS_REG);
    }
    return rc;
}


/**
 *    probe_for_drives    -    upper level drive probe
 *    @drive: drive to probe for
 *
 *    probe_for_drive() tests for existence of a given drive using do_probe()
 *    and presents things to the user as needed.
 *
 *    Returns:    0  no device was found
 *            1  device was found (note: drive->present might
 *               still be 0)
 */
static inline UINT8 probe_for_drive (ide_drive_t *drive)
{
    /*
     *    In order to keep things simple we have an id
     *    block for all drives at all times. If the device
     *    is pre ATA or refuses ATA/ATAPI identify we
     *    will add faked data to this.
     *
     *    Also note that 0 everywhere means "can't do X"
     */
    int rv;

    IDE_PRINTF("\n");

    drive->id = (struct hd_driveid *)MALLOC(SECTOR_WORDS *4);
    drive->id_read = 0;
    if(drive->id == NULL)
    {
        IDE_PRINTF("probe_for_drive()=>MALLOC (hd_driveid *)drive->id Failed.\n");
        return 0;
    }
    MEMSET(drive->id, 0, SECTOR_WORDS * 4);
    STRCPY((void *)(drive->id->model), "UNKNOWN");

    /* skip probing? */
    if (!drive->noprobe)
    {
        /* if !(success||timed-out) */
        IDE_PRINTF("probe_for_drive()=>Identify Dev.\n");
        if ((rv = do_probe(drive, WIN_IDENTIFY)) >= 2) {
            /* look for ATAPI device */
            if(rv == 2)
                IDE_PRINTF("Identify Dev=>Return 2=>device aborted the command (refused to identify itself).\n");
            else if(rv == 3)
                IDE_PRINTF("Identify Dev=>Return 3=>bad status from device (possible for ATAPI drives).\n");
            else
                IDE_PRINTF("Identify Dev=>Return 4=>probe was not attempted because failure was obvious.\n");

            IDE_PRINTF("probe_for_drive()=>Identify Packet Dev.\n");
            rv = do_probe(drive, WIN_PIDENTIFY);
        }
        else
        {
            if(rv == 0)
                IDE_PRINTF("Identify Dev=>Return 0=>device was identified.\n");
            else
                IDE_PRINTF("Identify Dev=>Return 1=>device timed-out (no response to identify request).\n");
        }

        if (!drive->present)
            /* drive not found */
            return 0;

        /* identification failed? */
        if (!drive->id_read) {
            if (drive->media == ide_disk) {
                IDE_PRINTF("%s: non-IDE drive, CHS=%d/%d/%d\n",
                    drive->name, drive->cyl,
                    drive->head, drive->sect);
            } else if (drive->media == ide_cdrom) {
                IDE_PRINTF("%s: ATAPI cdrom (?)\n", drive->name);
            } else {
                /* nuke it */
                IDE_PRINTF("%s: Unknown device on bus refused identification. Ignoring.\n", drive->name);
                drive->present = 0;
            }
        }
        /* drive was found */
    }
    if(!drive->present)
        return 0;
    /* The drive wasn't being helpful. Add generic info only */
    return drive->present;
}


#define IDE_CAPACITY_128G    0x0fffffff    // (128*1000*1000*1000/512 - 1)
#define IDE_CAPACITY_140G    273437500    // (140*1000*1000*1000/512)
#define IDE_CAPACITY_160G    312500000    // (160*1000*1000*1000/512)
#define IDE_CAPACITY_180G    351562500    // (180*1000*1000*1000/512)
#define IDE_CAPACITY_200G    390625000    // (200*1000*1000*1000/512)
#define IDE_CAPACITY_400G    781250000    // (400*1000*1000*1000/512)


#define IDE_1GB_SECTORS    (1000*1000*1000)/512
UINT8 *r_buf, *w_buf;


INT8 ide_capacity_check(UINT32 lba)
{
    int i;

    hdd_write_sector(lba, w_buf, 1);
    hdd_read_sector(lba, r_buf, 1);

    for(i=0;i<512;i++)
    {
        if(w_buf[i] != r_buf[i])
            break;
    }
    if (i == 512)
    {
        //IDE_PRINTF("%ld->\n", lba);
        return 1;
    }
    else
    {
        //IDE_PRINTF("<-%ld\n", lba);
        return -1;
    }

}


UINT32 ide_capacity_identify()
{
    UINT32 t0;
    int i;
    INT8 dir;
    UINT32 min, max, mid;

    r_buf = MALLOC(512+0x1f);
    r_buf = (UINT8*)(((UINT32)r_buf + 0x1f) & (~0x1f));
    w_buf = MALLOC(512+0x1f);
    w_buf = (UINT8*)(((UINT32)w_buf + 0x1f) & (~0x1f));

    for(i=0;i<512;i++)
    {
        asm volatile("mfc0 %0, $9" :"=r"(t0));
        w_buf[i] = (t0%0xff);
    }

    IDE_PRINTF("<=======HDD Capacity Identify=======>\n");
/*
    dir = ide_capacity_check(IDE_CAPACITY_140G);
    if (dir == 1)
    {
        dir = ide_capacity_check(IDE_CAPACITY_160G);
        if (dir == 1)
        {
            dir = ide_capacity_check(IDE_CAPACITY_180G);
            if (dir == 1)
            {
                dir = ide_capacity_check(IDE_CAPACITY_200G);
                if (dir == 1)
                {
                    min = IDE_CAPACITY_200G;
                }
                else
                    min = IDE_CAPACITY_180G;
            }
            else
                min = IDE_CAPACITY_160G;
        }
        else
            min = IDE_CAPACITY_140G;
    }

    max = min + IDE_1GB_SECTORS;
*/
    min = IDE_CAPACITY_128G;
    max = IDE_CAPACITY_400G;
    IDE_PRINTF("min(%ld)->max(%ld)\n", min, max);

    mid = (max - min)/2;
    while (mid != 0)
    {
        mid = (max - min)/2;
        dir = ide_capacity_check(mid + min);
        if (dir == 1)
            min += mid;
        else
            max -= mid;
    }

    IDE_PRINTF("<=======LBA: %ld=======>\n", min+1);

    MEMSET(r_buf, 0, 512); r_buf = NULL; FREE(r_buf);
    MEMSET(w_buf, 0, 512); w_buf = NULL; FREE(w_buf);

    return min+1;

}

void ide_s3601_uninit()
{
    hwif_t *hwif = &ide_hwif;

    if (osal_mutex_delete(hwif->semaphore)!=RET_SUCCESS)
    {
        //asm("sdbbp");
        SDBBP();
    }
    if (osal_semaphore_delete(ide_sema_id)!=RET_SUCCESS)
    //if ((ide_sema_id = osal_semaphore_create(1))==OSAL_INVALID_ID)
    {
        //asm("sdbbp");
        SDBBP();
    }

    if (osal_flag_delete(ide_flag_id)!=RET_SUCCESS)
    {
        //asm("sdbbp");
        SDBBP();
    }

    if (osal_interrupt_unregister_lsr(g_interrupt_num, ide_interrupt_lsr) )
    {
        //asm("sdbbp");
        SDBBP();
    }

}

INT32 ide_s3601_init()
{
    extern void idedisk_attach(ide_drive_t *);
    INT32 i, unit;
    hwif_t *hwif = &ide_hwif;
    OSAL_ID mutex_id;
    //UINT8 *r_buf, *w_buf;
    ide_drive_t *drive = &hwif->drives[0];
    UINT8 probe_success = 0;

    IDE_PRINTF("%s()=>\n", __FUNCTION__);

    //init mutex for ATA and CI Share Pin.
    ide_mutex_init();

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

    MEMSET(hwif, 0, sizeof(hwif_t));
    STRCPY((void *)(hwif->name), "ide0");
    for(i=0; i<IDE_NR_PORTS; i++)
    {
        hwif->io_ports[i] = i+8;
    }
    hwif->io_ports[IDE_CONTROL_OFFSET] = 0x16;
    hwif->io_ports[IDE_IRQ_OFFSET] = 0x00;
    for(i=0; i<MAX_DRIVES; i++)
    {
        hwif->drives[i].select.all = 0xa0;
        hwif->drives[i].select.b.unit = i;
        hwif->drives[i].hwif = hwif;
        hwif->drives[i].next = &hwif->drives[MAX_DRIVES-1-i];
        hwif->drives[i].ready_stat = READY_STAT|SEEK_STAT;
        hwif->drives[i].ctl = 8;
        STRCPY((void *)(hwif->drives[i].name), "hda");
        hwif->drives[i].name[2]+=i;
    }
    m3327_hwif_iops(hwif);
    default_hwif_transport(hwif);

    //ATA SW Reset. ATACR: bit12
    IDE_SET_DWORD(S3601_IDE_ATACR, IDE_GET_DWORD(S3601_IDE_ATACR)&0xFFFFEFFF); // clear 0
    osal_delay(3);
    IDE_SET_DWORD(S3601_IDE_ATACR, IDE_GET_DWORD(S3601_IDE_ATACR)|0x00001000); // set 1
    osal_delay(10);

    //Compute timings for supported r/w modes.
    ide_s3601_compute_timing(S3601_IDE_OPERATION_CLK);

    //Disable all interrupts
    ide_s3601_set_interrupt_mask(0x1F);

    // set path to cpu
    IDE_SET_PATH_CPU();

    //set r/w mode PIO-4
    //ide_s3601_set_PIOB_mode_id(4);

    //named semaphore but mutex for real.
    if ((mutex_id = osal_mutex_create())==OSAL_INVALID_ID)
    {
        IDE_PRINTF("%s()=>mutex create failure!\n", __FUNCTION__);
        return RET_FAILURE;
    }
    hwif->semaphore = mutex_id;

    if ((ide_sema_id = osal_semaphore_create(1))==OSAL_INVALID_ID)
    {
        IDE_PRINTF("%s()=>semaphore create failure!\n", __FUNCTION__);
        return RET_FAILURE;
    }

    if ((ide_flag_id = osal_flag_create(0))==OSAL_INVALID_ID) {
        IDE_PRINTF("%s()=>flag create failure!\n", __FUNCTION__);
        return RET_FAILURE;
    }

    if(ALI_M3329E==sys_ic_get_chip_id())
        g_interrupt_num = 12;
    else
        g_interrupt_num = 18;
    if (osal_interrupt_register_lsr(g_interrupt_num, ide_interrupt_lsr, (UINT32)hwif)==OSAL_E_FAIL)
    {
        IDE_PRINTF("%s()=>IDR register failure!\n", __FUNCTION__);
        return RET_FAILURE;
    }


    if (wait_hwif_ready(hwif))
    {
        IDE_PRINTF("ide_s3601_init()=>wait hwif ready timeout before probe!\n");
        return RET_FAILURE ;
    }

    //not support Slave HDD currently here.
    //for(unit = 0; unit < MAX_DRIVES; unit++) {
    for(unit = 0; unit < 1; unit++)
    {
        if (probe_for_drive(drive))
        {
            probe_success = 1;
            if (NULL == drive->id)
                return RET_FAILURE;
            idedisk_attach(drive);

            if ((drive->id)&&(drive->id->max_multsect!=0)) 
            {
                drive->mult_count = drive->id->max_multsect;//>16? 16: drive->id->max_multsect;
            }
            else
            {
                drive->mult_count = 1;
            }

            //IDE_PRINTF("ide_s3601_init()=>drive->mult_count: %d\n", drive->mult_count);

            //ide_exec_command(drive, WIN_SETMULT, 0, drive->mult_count);
            //ide_exec_command(drive, WIN_SETFEATURES, SETFEATURES_XFER, XFER_PIO_4);
            //ide_exec_command(drive, WIN_SETFEATURES, SETFEATURES_EN_WCACHE, 0);
            //ide_exec_command(drive, WIN_SETFEATURES, SETFEATURES_EN_AAM, 0xFE);

        }
     }

    IDE_SET_DWORD(S3601_IDE_ATACR, IDE_GET_DWORD(S3601_IDE_ATACR)&0xFFFFEFFF); // clear 0
    osal_delay(100);
    IDE_SET_DWORD(S3601_IDE_ATACR, IDE_GET_DWORD(S3601_IDE_ATACR)|0x00001000); // set 1

    if(0==probe_success)
    {
        return !RET_SUCCESS;
    }
    if(drive->capacity64 == IDE_CAPACITY_128G)
        drive->capacity64 = ide_capacity_identify();

    //Set default r/w mode: PIO-0
    ide_s3601_set_pio_mode_id(&hwif->drives[0], 0);

    IDE_PRINTF("ide_s3601_init()=>End!\n");
    if(hwif->drives[0].attach == 1)
    {
        return RET_SUCCESS;
    }
    else
    {
        return !RET_SUCCESS;
    }
}
