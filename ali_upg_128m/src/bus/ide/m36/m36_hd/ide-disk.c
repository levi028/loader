#include <types.h>
#include <sys_config.h>
#include <retcode.h>

#include <osal/osal.h>
#include <api/libc/string.h>
#include <api/libc/printf.h>

#include <errno.h>

#include <bus/ide/ide.h>
#include <hld/hld_dev.h>
#include <hld/cic/cic.h>

#include "ide_iops.h"
#include "ide_reg.h"

#define SECTOR_BITS    9        /* log2(SECTOR_SIZE) */

extern hwif_t ide_hwif;

#define shift_nr 1000
#define shift_start_sector  0x10000
#define shift_end_sector  0x10000+188*350

UINT32 g_shift_write_sector = shift_start_sector;
UINT32 g_shift_read_sector = shift_start_sector;
UINT32 g_shift_restart;

//ATA Transfer Mode & Mode ID.
UINT8 g_ide_rw_mode;
UINT8 g_ide_mode_id;
extern void ide_s3601_set_interrupt_mask(UINT8 mask);
extern INT32 ide_s3601_wait_rw_over(ide_drive_t *drive, INT32 direction, UINT32 timeout,UINT32 flagmode);
extern INT32 ide_wait_stat (ide_startstop_t *startstop, ide_drive_t *drive,
    UINT8 good, UINT8 bad, UINT32 timeout);
extern void SELECT_DRIVE(ide_drive_t *drive);


#define WAIT_NOT    10000
#define IDE_ERROR_TIMEOUT    7
//#define TEST_IDE_PERFORMANCE

//LBA Addressing Mode.
INT    lba_bit_mode = LBA_48;

//mutex id for CI & ATA DMA op.
struct ide_mutex ide_mutex_id = {
    .lock = NULL,
    .unlock = NULL,
};

void ide_mutex_init(void)
{
    if(ALI_M3329E==sys_ic_get_chip_id())
    {
        UINT32 pin_mux = *((volatile UINT32 *)0xb8000010);
        if(0!=(pin_mux&0x3))
        {
            struct cic_device * cic = NULL;
            cic = (struct cic_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_CIC);
            if(cic)
            {
                struct cic_req_mutex req_param;
                MEMSET(&req_param, 0, sizeof(struct cic_req_mutex));
                cic->do_ioctl(cic, CIC_DRIVER_REQMUTEX, (UINT32)(&req_param));
                ide_mutex_id.lock = req_param.cic_enter_mutex;
                ide_mutex_id.unlock = req_param.cic_exit_mutex;
            }
        }
    }
}


void ide_set_rw_mode(UINT8 mode,UINT8 id)
{
    g_ide_rw_mode=mode;
    g_ide_mode_id=id;
}

inline UINT8 ide_get_rw_mode(void)
{
    return g_ide_rw_mode;
}

inline UINT8 ide_get_rw_modeid(void)
{
    return g_ide_mode_id;
}


/*
*    Only used by ide_read/write_sector() to Get specific flag acording to transfer mode.
*/
UINT32 ide_get_flag_mode(UINT8 rwmode)
{
    UINT32 flag;
    switch(rwmode)
    {
        case IDE_PIO_BUR_MODE:
            flag=IDE_FLAG_ATA_INT2B;
            break;
        case IDE_DMA_MODE:
            flag=IDE_FLAG_ATA_INT2B;
            break;
        case IDE_UDMA_MODE:
            flag=IDE_FLAG_ATA_INT2B;
            break;
        default:
            flag=IDE_FLAG_ATA_INT2B;
            break;
    }
    return flag;
}

/*
 * lba_capacity_is_ok() performs a sanity check on the claimed "lba_capacity"
 * value for this drive (from its reported identification information).
 *
 * Returns:    1 if lba_capacity looks sensible
 *            0 otherwise
 *
 * It is called only once for each drive.
 */
static int lba_capacity_is_ok (struct hd_driveid *id)
{
    unsigned long lba_sects, chs_sects, head, tail;

    /*
     * The ATA spec tells large drives to return
     * C/H/S = 16383/16/63 independent of their size.
     * Some drives can be jumpered to use 15 heads instead of 16.
     * Some drives can be jumpered to use 4092 cyls instead of 16383.
     */
    if ((id->cyls == 16383
         || (id->cyls == 4092 && id->cur_cyls == 16383)) &&
        id->sectors == 63 &&
        (id->heads == 15 || id->heads == 16) &&
        (id->lba_capacity >= (unsigned int)id->heads*16383*63))
        return 1;

    lba_sects   = id->lba_capacity;
    chs_sects   = id->cyls * id->heads * id->sectors;

    /* perform a rough sanity check on lba_sects:  within 10% is OK */
    if ((lba_sects - chs_sects) < chs_sects/10)
        return 1;

    /* some drives have the word order reversed */
    head = ((lba_sects >> 16) & 0xffff);
    tail = (lba_sects & 0xffff);
    lba_sects = (head | (tail << 16));
    if ((lba_sects - chs_sects) < chs_sects/10) {
        id->lba_capacity = lba_sects;
        return 1;    /* lba_capacity is (now) good */
    }

    return 0;    /* lba_capacity value may be bad */
}


/*
 * Useful Only If HPA feature set is supported & LBA48 NOT supported.
 *
 * Queries for true maximum capacity of the drive.
 * Returns maximum LBA address (> 0) of the drive, 0 if failed.
 */
static unsigned long idedisk_read_native_max_address(ide_drive_t *drive)
{
    ide_task_t args;
    unsigned long addr = 0;

    /* Create IDE/ATA command request structure */
    MEMSET(&args, 0, sizeof(ide_task_t));
    args.tf_register[IDE_SELECT_OFFSET]    = 0x40;
    args.tf_register[IDE_COMMAND_OFFSET]    = WIN_READ_NATIVE_MAX;
    args.command_type            = IDE_DRIVE_TASK_NO_DATA;
    //args.handler                = &task_no_data_intr;
    /* submit command request */
    //ide_raw_taskfile(drive, &args, NULL);

    /* if OK, compute maximum address value */
    if ((args.tf_register[IDE_STATUS_OFFSET] & 0x01) == 0) {
        addr = ((args.tf_register[IDE_SELECT_OFFSET] & 0x0f) << 24)
             | ((args.tf_register[  IDE_HCYL_OFFSET]       ) << 16)
             | ((args.tf_register[  IDE_LCYL_OFFSET]       ) <<  8)
             | ((args.tf_register[IDE_SECTOR_OFFSET]       ));
        addr++;    /* since the return value is (maxlba - 1), we add 1 */
    }
    return addr;
}


/*
 * Useful Only If HPA feature set is supported & LBA48 supported.
 */
static unsigned long long idedisk_read_native_max_address_ext(ide_drive_t *drive)
{
    unsigned long long addr = 0;
    UINT8 stats;
    //int i;
    //INT32 ret;
    ide_startstop_t startstop;

    hwif_t *hwif = HWIF(drive);
    SELECT_DRIVE(drive);

    hwif->OUTB(2|drive->ctl, IDE_CONTROL_REG); /* disable interrupt */
    //hwif->OUTB(0x40, IDE_SECTOR_REG);
    hwif->OUTB(drive->select.all, IDE_SELECT_REG);
    hwif->OUTB(0x27, IDE_COMMAND_REG);
    osal_delay(1);

    if (ide_wait_stat(&startstop, drive, 0, BUSY_STAT, 4000)==0) {
        stats = hwif->INB(IDE_STATUS_REG);
        if(OK_STAT(stats, READY_STAT, BAD_R_STAT|DRQ_STAT))
        {
            if((hwif->INB(IDE_STATUS_OFFSET) & 0x01) == 0)
            {
                UINT32 high = (hwif->INB(IDE_HCYL_OFFSET) << 16) |
                                   (hwif->INB(IDE_LCYL_OFFSET) <<  8) |
                                    hwif->INB(IDE_SECTOR_OFFSET);

                UINT32 low = (hwif->INB(IDE_HCYL_OFFSET) << 16) |
                                   (hwif->INB(IDE_LCYL_OFFSET) <<  8) |
                                    hwif->INB(IDE_SECTOR_OFFSET);
                addr = ((unsigned long long)high << 24) | low;
                addr++;    /* since the return value is (maxlba - 1), we add 1 */
                return addr;
            }
        }
        IDE_PRINTF("%s()=>stat: 0x%x.\n", __FUNCTION__, stats);
        osal_task_sleep(1);

    }
    IDE_PRINTF("%s()=>stat: 0x%x.\n", __FUNCTION__, stats);
    return addr;
}


/*
 * Useful Only If HPA feature set is supported & LBA48 NOT supported.
 *
 * Sets maximum virtual LBA address of the drive.
 * Returns new maximum virtual LBA address (> 0) or 0 on failure.
 */
static unsigned long idedisk_set_max_address(ide_drive_t *drive, unsigned long addr_req)
{
    ide_task_t args;
    unsigned long addr_set = 0;

    addr_req--;
    /* Create IDE/ATA command request structure */
    MEMSET(&args, 0, sizeof(ide_task_t));
    args.tf_register[IDE_SECTOR_OFFSET]    = ((addr_req >>  0) & 0xff);
    args.tf_register[IDE_LCYL_OFFSET]    = ((addr_req >>  8) & 0xff);
    args.tf_register[IDE_HCYL_OFFSET]    = ((addr_req >> 16) & 0xff);
    args.tf_register[IDE_SELECT_OFFSET]    = ((addr_req >> 24) & 0x0f) | 0x40;
    args.tf_register[IDE_COMMAND_OFFSET]    = WIN_SET_MAX;
    args.command_type            = IDE_DRIVE_TASK_NO_DATA;
    //args.handler                = &task_no_data_intr;
    /* submit command request */
    //ide_raw_taskfile(drive, &args, NULL);
    /* if OK, read new maximum address value */
    if ((args.tf_register[IDE_STATUS_OFFSET] & 0x01) == 0) {
        addr_set = ((args.tf_register[IDE_SELECT_OFFSET] & 0x0f) << 24)
             | ((args.tf_register[  IDE_HCYL_OFFSET]       ) << 16)
             | ((args.tf_register[  IDE_LCYL_OFFSET]       ) <<  8)
             | ((args.tf_register[IDE_SECTOR_OFFSET]       ));
        addr_set++;
    }
    return addr_set;
}


/*
 * Useful Only If HPA feature set is supported & LBA48 supported.
 */
static unsigned long long idedisk_set_max_address_ext(ide_drive_t *drive, unsigned long long addr_req)
{
    ide_task_t args;
    unsigned long long addr_set = 0;

    addr_req--;
    /* Create IDE/ATA command request structure */
    MEMSET(&args, 0, sizeof(ide_task_t));
    args.tf_register[IDE_SECTOR_OFFSET]    = ((addr_req >>  0) & 0xff);
    args.tf_register[IDE_LCYL_OFFSET]    = ((addr_req >>= 8) & 0xff);
    args.tf_register[IDE_HCYL_OFFSET]    = ((addr_req >>= 8) & 0xff);
    args.tf_register[IDE_SELECT_OFFSET]      = 0x40;
    args.tf_register[IDE_COMMAND_OFFSET]    = WIN_SET_MAX_EXT;
    args.hob_register[IDE_SECTOR_OFFSET]    = (addr_req >>= 8) & 0xff;
    args.hob_register[IDE_LCYL_OFFSET]    = (addr_req >>= 8) & 0xff;
    args.hob_register[IDE_HCYL_OFFSET]    = (addr_req >>= 8) & 0xff;
    args.hob_register[IDE_SELECT_OFFSET]    = 0x40;
    //args.hobRegister[IDE_CONTROL_OFFSET_HOB]= (drive->ctl|0x80);
    args.command_type            = IDE_DRIVE_TASK_NO_DATA;
    //args.handler                = &task_no_data_intr;
    /* submit command request */
    //ide_raw_taskfile(drive, &args, NULL);
    /* if OK, compute maximum address value */
    if ((args.tf_register[IDE_STATUS_OFFSET] & 0x01) == 0) {
        UINT32 high = (args.hob_register[IDE_HCYL_OFFSET] << 16) |
               (args.hob_register[IDE_LCYL_OFFSET] <<  8) |
                args.hob_register[IDE_SECTOR_OFFSET];
        UINT32 low  = ((args.tf_register[IDE_HCYL_OFFSET])<<16) |
               ((args.tf_register[IDE_LCYL_OFFSET])<<8) |
                (args.tf_register[IDE_SECTOR_OFFSET]);
        addr_set = ((unsigned long long)high << 24) | low;
        addr_set++;
    }
    return addr_set;
}

static UINT32 sectors_to_mb(unsigned long long n)
{
    UINT32 value=(UINT32)((n<<9)/1000000);
    return value;
}


/*
 * The same here.
 */
static inline int idedisk_supports_lba48(const struct hd_driveid *id)
{
    return (id->command_set_2 & 0x0400) && (id->cfs_enable_2 & 0x0400)
           && id->lba_capacity_2;
}


/*
 * Bits 10 of command_set_1 and cfs_enable_1 must be equal,
 * so on non-buggy drives we need test only one.
 * However, we should also check whether these fields are valid.
 */
static inline int idedisk_supports_hpa(const struct hd_driveid *id)
{
    return (id->command_set_1 & 0x0400) && (id->cfs_enable_1 & 0x0400);
}


/*
 * Useful Only If HPA feature set is supported.
 */
static inline void idedisk_check_hpa(ide_drive_t *drive)
{
    unsigned long long capacity, set_max;
    int lba48 = idedisk_supports_lba48(drive->id);

    capacity = drive->capacity64;
    if (lba48)
        set_max = idedisk_read_native_max_address_ext(drive);
    else
        set_max = idedisk_read_native_max_address(drive);

    if (set_max <= capacity)
        return;

    IDE_PRINTF("%s: Host Protected Area detected.\n"
             "\tcurrent capacity is %llu sectors (%llu MB)\n"
             "\tnative  capacity is %llu sectors (%llu MB)\n",
             drive->name,
             capacity, sectors_to_mb(capacity),
             set_max, sectors_to_mb(set_max));

    if (lba48)
        set_max = idedisk_set_max_address_ext(drive, set_max);
    else
        set_max = idedisk_set_max_address(drive, set_max);
    if (set_max) {
        drive->capacity64 = set_max;
        IDE_PRINTF("%s: Host Protected Area disabled.\n",
                 drive->name);
    }
}


/*
 * drive->addressing:
 *    0: 28-bit
 *    1: 48-bit
 *    2: 48-bit capable doing 28-bit
 */
static int set_lba_addressing(ide_drive_t *drive, int arg)
{
    drive->addressing =  0;

    //if (HWIF(drive)->no_lba48)
    //    return 0;

    if (!idedisk_supports_lba48(drive->id))
                return -EIO;
    drive->addressing = arg;
    return 0;
}


static int write_cache(ide_drive_t *drive, int arg)
{
    ide_task_t args;
    //int err=0;

    if (!ide_id_has_flush_cache(drive->id))
        return 1;

    MEMSET(&args, 0, sizeof(ide_task_t));
    args.tf_register[IDE_FEATURE_OFFSET]    = (arg) ?
            SETFEATURES_EN_WCACHE : SETFEATURES_DIS_WCACHE;
    args.tf_register[IDE_COMMAND_OFFSET]    = WIN_SETFEATURES;
    args.command_type            = IDE_DRIVE_TASK_NO_DATA;
    //args.handler                = &task_no_data_intr;

    //err = ide_raw_taskfile(drive, &args, NULL);

    //if (err)
    //    return err;

    drive->wcache = arg;
    return 0;
}



/*
 * Compute drive->capacity, the full capacity of the drive
 * Called with drive->id != NULL.
 *
 * To compute capacity, this uses either of
 *
 *    1. CHS value set by user       (whatever user sets will be trusted)
 *    2. LBA value from target drive (require new ATA feature)
 *    3. LBA value from system BIOS  (new one is OK, old one may break)
 *    4. CHS value from system BIOS  (traditional style)
 *
 * in above order (i.e., if value of higher priority is available,
 * reset will be ignored).
 */
static void init_idedisk_capacity (ide_drive_t  *drive)
{
    struct hd_driveid *id = drive->id;
    /*
     * If this drive supports the Host Protected Area feature set,
     * then we may need to change our opinion about the drive's capacity.
     */
    int hpa = idedisk_supports_hpa(id);

    if (idedisk_supports_lba48(id)) {
        /* drive speaks 48-bit LBA */
        IDE_PRINTF("%s()=>LBA 48Bit-Mode Supported.\n", __FUNCTION__);
        drive->select.b.lba = 1;
        lba_bit_mode = LBA_48;
        drive->capacity64 = id->lba_capacity_2;
        if (hpa)
            idedisk_check_hpa(drive);
    } else if ((id->capability & 2) && lba_capacity_is_ok(id)) {
        IDE_PRINTF("%s()=>LBA 48Bit-Mode Not Supported.\n", __FUNCTION__);
        /* drive speaks 28-bit LBA */
        drive->select.b.lba = 1;
        lba_bit_mode = LBA_28;
        drive->capacity64 = id->lba_capacity;
        if (hpa)
            idedisk_check_hpa(drive);
    } else {
        /* drive speaks boring old 28-bit CHS */
        drive->capacity64 = drive->cyl * drive->head * drive->sect;
    }
}

/*
 * API called By FS to get LBA number.
 */
static unsigned long long idedisk_capacity (ide_drive_t *drive)
{
    return drive->capacity64 - drive->sect0;
}

unsigned long long hdd_disk_capacity (void)
{
    return idedisk_capacity (&ide_hwif.drives[0]);
}

static void idedisk_setup (ide_drive_t *drive)
{
    struct hd_driveid *id = drive->id;
    unsigned long long capacity;
    int barrier;

    if (drive->id_read == 0)
        return;

    /*
     * CompactFlash cards and their brethern look just like hard drives
     * to us, but they are removable and don't have a doorlock mechanism.
     */
    if (drive->removable && !(drive->is_flash)) {
        /*
         * Removable disks (eg. SYQUEST); ignore 'WD' drives
         */
        if (id->model[0] != 'W' || id->model[1] != 'D') {
            drive->doorlocking = 1;
        }
    }

    //check if support LBA48.
    (void)set_lba_addressing(drive, 1);
    if (drive->addressing == 1) {
        //int max_s = 2048;
    }

    //IDE_PRINTF("%s: max request size: %dKiB\n", drive->name, drive->queue->max_sectors / 2);

    /* Extract geometry if we did not already have one for the drive */
    if (!drive->cyl || !drive->head || !drive->sect) {
        drive->cyl = drive->bios_cyl = id->cyls;
        drive->head = drive->bios_head = id->heads;
        drive->sect = drive->bios_sect = id->sectors;
    }

    /* Handle logical geometry translation by the drive */
    if ((id->field_valid & 1) && id->cur_cyls &&
        id->cur_heads && (id->cur_heads <= 16) && id->cur_sectors) {
        drive->cyl  = id->cur_cyls;
        drive->head = id->cur_heads;
        drive->sect = id->cur_sectors;
    }

    /* Use physical geometry if what we have still makes no sense */
    if (drive->head > 16 && id->heads && id->heads <= 16) {
        drive->cyl  = id->cyls;
        drive->head = id->heads;
        drive->sect = id->sectors;
    }

    /* calculate drive capacity, and select LBA if possible */
    init_idedisk_capacity (drive);

    /* limit drive capacity to 137GB if LBA48 cannot be used */
    if (drive->addressing == 0 && drive->capacity64 > 1ULL << 28) {
        IDE_PRINTF("%s()=>%s: ATA device not support LBA48=>Capacity:%llu sectors(%llu MB)=>Limit to 137G.\n",
               __FUNCTION__, drive->name, (unsigned long long)drive->capacity64,
               sectors_to_mb(drive->capacity64));
        drive->capacity64 = 1ULL << 28;
    }

    if (/*drive->hwif->no_lba48_dma && */drive->addressing) {
        IDE_PRINTF("%s()=>%s: ATA device support LBA48=>Capacity:%llu sectors(%llu MB).\n",
                   __FUNCTION__, drive->name, (unsigned long long)drive->capacity64,
                   sectors_to_mb(drive->capacity64));
        if (drive->capacity64 > 1ULL << 28)
            IDE_PRINTF("%s()=>drive->addressing = 1.\n",__FUNCTION__);
        else
        {
            drive->addressing = 0;
            IDE_PRINTF("%s()=>drive->addressing = 0.\n",__FUNCTION__);
        }
    }

    /*
     * if possible, give fdisk access to more of the drive,
     * by correcting bios_cyls:
     */
    capacity = idedisk_capacity (drive);
    if (!drive->forced_geom) {

        if (idedisk_supports_lba48(drive->id)) {
            /* compatibility */
            drive->bios_sect = 63;
            drive->bios_head = 255;
        }

        if (drive->bios_sect && drive->bios_head) {
            unsigned int cap0 = capacity; /* truncate to 32 bits */
            unsigned int cylsz, cyl;

            if (cap0 != capacity)
                drive->bios_cyl = 65535;
            else {
                cylsz = drive->bios_sect * drive->bios_head;
                cyl = cap0 / cylsz;
                if (cyl > 65535)
                    cyl = 65535;
                if (cyl > drive->bios_cyl)
                    drive->bios_cyl = cyl;
            }
        }
    }

    {
        IDE_PRINTF("%s()=>%s capacity:%u sectors(%u MB)",
             __FUNCTION__, drive->name, (UINT32)capacity, sectors_to_mb(capacity));
    }

    /* Only print cache size when it was specified */
    if (id->buf_size)
        IDE_PRINTF(" w/%dKiB Cache", id->buf_size/2);

    IDE_PRINTF(", CHS=%d/%d/%d",
           drive->bios_cyl, drive->bios_head, drive->bios_sect);
    //if (drive->using_dma)
    //    ide_dma_verbose(drive);
    IDE_PRINTF("\n");

    drive->mult_count = 0;

    drive->no_io_32bit = id->dword_io ? 1 : 0;

    /* write cache enabled? */
    if ((id->csfo & 1) || (id->cfs_enable_1 & (1 << 5)))
        drive->wcache = 1;

    write_cache(drive, 1);

    /*
     * We must avoid issuing commands a drive does not understand
     * or we may crash it. We check flush cache is supported. We also
     * check we have the LBA48 flush cache if the drive capacity is
     * too large. By this time we have trimmed the drive capacity if
     * LBA48 is not available so we don't need to recheck that.
     */
    barrier = 0;
    if (ide_id_has_flush_cache(id))
        barrier = 1;
    if (drive->addressing == 1) {
        /* Can't issue the correct flush ? */
        if (capacity > (1ULL << 28) && !ide_id_has_flush_cache_ext(id))
            barrier = 0;
    }

    libc_printf("%s()=>%s cache flushes %ssupported\n",
        __FUNCTION__, drive->name, barrier ? "" : "not ");

}


//Print IDENTIFY DEVICE data.
static void ata_identify_device (ide_drive_t *drive)
{
    //UINT16 *id = (UINT16 *)drive->id;
    UINT16 i;

    IDE_PRINTF("\n%s()=>IDENTIFY DEVICE data: \n", __FUNCTION__);
    for ( i = 0; i < 256; i ++)
    {
        //IDE_PRINTF("word %03d: 0x%04x    ", i, id[i]);
        if ((i+1) % 4 == 0)
            IDE_PRINTF("\n\r");
    }
    IDE_PRINTF("\n\n");

}


void idedisk_attach(ide_drive_t *drive)
{
    idedisk_setup(drive);

    ata_identify_device(drive);

    if ((!drive->head || drive->head > 16) && !drive->select.b.lba) {
        IDE_PRINTF("%s: INVALID GEOMETRY: %d PHYSICAL HEADS?\n",
            drive->name, drive->head);
        drive->attach = 0;
    } else {
        drive->attach = 1;
        IDE_PRINTF("%s()=>%s is attached.\n", __FUNCTION__, drive->name);
    }
}


/*
*    Acording to ATA protocol, DRQ should be checked.
*    But actually, We don't care it, Data Transfer is still OKay.
*
*    Here, just check status ready & seek & not busy.
*    By Mao
*/
INT32 ide_wait_okay(ide_drive_t *drive, UINT32 time)
{
    UINT32 i;
    UINT8 data;
    hwif_t *hwif = HWIF(drive);

    if(0 == time) {
        data = hwif->INB(IDE_ALTSTATUS_REG);
        return (data&0x08) && (!(data&0x80)) ? 1 : 0;
    }

    for(i=0; i<time; i++) {
        data = hwif->INB(IDE_ALTSTATUS_REG);
        //IDE_PRINTF("-0x%x", data);
        //if((data&DRQ_STAT) && (!(data&BUSY_STAT))) {
        if((data&READY_STAT) && (data&SEEK_STAT) && (!(data&BUSY_STAT))) {
            return TRUE;
        }
        osal_task_sleep(1);
    }
    return 0;
}

#if 0
INT32 ide_wait_dma_over(ide_drive_t *drive, INT32 direction)
{
    INT32 i_error;
    UINT32 flag;
    UINT8 status1;
    extern ID ide_flag_id;
    hwif_t *hwif = HWIF(drive);
    if (direction == RQ_FLAG_DATA_IN) {
        if (OSAL_E_TIMEOUT == osal_flag_wait(&flag, ide_flag_id, IDE_FLAG_DMA, TWF_ANDW|TWF_CLR, 10000)) {
            IDE_PRINTF("wait RDMA time out!\n");
            return 0xFF;
        } else {
            status1 = hwif->INB(IDE_STATUS_REG);
            while((*(volatile unsigned long *)(0xb8000038))&0x10) {
                status1 = hwif->INB(IDE_STATUS_REG);
            }
            return status1;
        }
    } else if (direction == RQ_FLAG_DATA_OUT) {
        if (OSAL_E_TIMEOUT == osal_flag_wait(&flag, ide_flag_id, IDE_FLAG_WDMA, TWF_ANDW|TWF_CLR, 10000)) {
            IDE_PRINTF("wait WDMA time out!\n");
            return 0xFF;
        }
        //status1=hwif->INB(IDE_STATUS_REG);
        return SUCCESS;
    }
}
#endif


INT32 ide_error(ide_drive_t *drive, UINT8 *fun)
{
    int rv;
    hwif_t *hwif = HWIF(drive);

    //modified on 2008.05.14 for timeshifting while using ATA to SATA bridge.
    do{

        hwif->OUTB(2|drive->ctl, IDE_CONTROL_REG); /* disable interrupt */
        hwif->OUTB(0, IDE_FEATURE_REG);
        hwif->OUTB(0, IDE_NSECTOR_REG);
        hwif->OUTB(0, IDE_SECTOR_REG);
        hwif->OUTB(0, IDE_LCYL_REG);
        hwif->OUTB(0, IDE_HCYL_REG);
        hwif->OUTB(drive->select.all, IDE_SELECT_REG);

        //hwif->OUTB(0x08, IDE_COMMAND_REG);
        hwif->OUTB(WIN_IDLEIMMEDIATE, IDE_COMMAND_REG);

        osal_delay(10);     /* for 400ns */

        rv = ide_wait_status_not(HWIF(drive), (WAIT_NOT/2), (BUSY_STAT|ERR_STAT));
        hwif->OUTB(drive->ctl, IDE_CONTROL_REG);
    }
    while(0 !=rv );

    return 0;
}


/*
*Func name: ide_read_sector
*Desc: Read nums of sectors of IDE device.
*Arguments:    ->drive: IDE drive(Master or Slave, only Master supported here)
*            ->block: Right position of IDE device where Des data will be read from.
*            ->sector_nr: Read sector length.
*            ->buffer: Des data buffer address.
*/
#if (1)    //lba 48
INT32 ide_read_sector(ide_drive_t *drive, unsigned long long block, UINT16 sector_nr, UINT8 *buffer)
#else    // lba28
INT32 ide_read_sector(ide_drive_t *drive, UINT32 block, UINT16 sector_nr, UINT8 *buffer)
#endif
{
    ide_startstop_t startstop=ide_started;
    INT32 i, multi_sector;
    INT32 multi_sector_tmp;
    hwif_t *hwif = HWIF(drive);
    UINT8 tasklets[10];
    UINT8 rw_mode;

    RET_CODE ret = SUCCESS;

#ifdef TEST_IDE_PERFORMANCE
    UINT32 time_beg;
    UINT32 time_end;
    UINT32 read_speed;
#endif
#ifdef TEST_IDE_PERFORMANCE
        time_beg=osal_get_tick();
#endif

    //if (block>=drive->capacity64)
        //return -EINVAL;

    SELECT_DRIVE(drive);
    if (IDE_CONTROL_REG)
    {
        // bit 1(nIEN):0->enable int rq to host, 1->disable it.
        hwif->OUTB(0x08, IDE_CONTROL_REG);
        //hwif->OUTB(0x0A, IDE_CONTROL_REG);
    }

    ide_wait_stat(&startstop, drive, drive->ready_stat, BUSY_STAT, WAIT_READY);
    if (startstop==ide_stopped)
    {
        ide_error(drive, (UINT8 *)__FUNCTION__);
        //return ERR_FAILUE; //next rw op maybe success.
        return ERR_FAILUE;
    }

if (lba_bit_mode == LBA_48)    //lba48
{

    tasklets[0] = 0;
    tasklets[1] = 0;
    tasklets[2] = (UINT8)(sector_nr&0xFF);    // low 8 bits
    tasklets[3] = (UINT8)(sector_nr>>8);    // high 8 bits
    tasklets[4] = (UINT8) block;
    tasklets[5] = (UINT8) (block>>8);
    tasklets[6] = (UINT8) (block>>16);
    tasklets[7] = (UINT8) (block>>24);
    tasklets[8] = (UINT8) (block>>32);
    tasklets[9] = (UINT8) (block>>40);

/*
    IDE_PRINTF("%s: 0x%02x%02x 0x%02x%02x%02x%02x%02x%02x\n",
        drive->name, tasklets[3], tasklets[2],
        tasklets[9], tasklets[8], tasklets[7],
        tasklets[6], tasklets[5], tasklets[4]);
*/
    hwif->OUTB(tasklets[1], IDE_FEATURE_REG);
    hwif->OUTB(tasklets[3], IDE_NSECTOR_REG);
    hwif->OUTB(tasklets[7], IDE_SECTOR_REG);
    hwif->OUTB(tasklets[8], IDE_LCYL_REG);
    hwif->OUTB(tasklets[9], IDE_HCYL_REG);

    hwif->OUTB(tasklets[0], IDE_FEATURE_REG);
    hwif->OUTB(tasklets[2], IDE_NSECTOR_REG);
    hwif->OUTB(tasklets[4], IDE_SECTOR_REG);
    hwif->OUTB(tasklets[5], IDE_LCYL_REG);
    hwif->OUTB(tasklets[6], IDE_HCYL_REG);
    hwif->OUTB(0x00|drive->select.all,IDE_SELECT_REG);

    osal_delay(1);

    rw_mode=ide_get_rw_mode();
    if(rw_mode==IDE_PIO_MODE)    // PIO
    {
        hwif->OUTB(0x24, IDE_COMMAND_REG);
        //hwif->OUTB(WIN_MULTREAD_EXT, IDE_COMMAND_REG);
    }
    else if(rw_mode==IDE_PIO_BUR_MODE)    // PIO burst
    {
        hwif->OUTB(0x24, IDE_COMMAND_REG);
        //hwif->OUTB(WIN_MULTREAD_EXT, IDE_COMMAND_REG);
    }
    else        // dma/udma
    {
        hwif->OUTB(WIN_READDMA_EXT, IDE_COMMAND_REG);
    }
}
else    // lba28
{

    tasklets[0] = 0;
    tasklets[1] = 0;
    tasklets[2] = (UINT8)(sector_nr&0xFF);
    tasklets[3] = 0;
    tasklets[4] = (UINT8) block;
    tasklets[5] = (UINT8) (block>>8);
    tasklets[6] = (UINT8) (block>>16);
    tasklets[7] = (UINT8) (block>>24);
    tasklets[8] = (task_ioreg_t) 0;
    tasklets[9] = (task_ioreg_t) 0;
/*
    IDE_PRINTF("%s: 0x%02x%02x 0x%02x%02x%02x%02x%02x%02x\n",
        drive->name, tasklets[3], tasklets[2],
        tasklets[9], tasklets[8], tasklets[7],
        tasklets[6], tasklets[5], tasklets[4]);
*/
    hwif->OUTB(tasklets[0], IDE_FEATURE_REG);
    hwif->OUTB(tasklets[2], IDE_NSECTOR_REG);
    hwif->OUTB(tasklets[4], IDE_SECTOR_REG);
    hwif->OUTB(tasklets[5], IDE_LCYL_REG);
    hwif->OUTB(tasklets[6], IDE_HCYL_REG);
    hwif->OUTB((tasklets[7]&0x0f)|drive->select.all,IDE_SELECT_REG);

    osal_delay(10);

    rw_mode=ide_get_rw_mode();
    if(rw_mode==IDE_PIO_MODE)    // PIO
    {
        hwif->OUTB(0x20, IDE_COMMAND_REG);
        //hwif->OUTB(WIN_MULTREAD, IDE_COMMAND_REG);
    }
    else if(rw_mode==IDE_PIO_BUR_MODE)    // PIO burst
    {
        hwif->OUTB(0x20, IDE_COMMAND_REG);
        //hwif->OUTB(WIN_MULTREAD, IDE_COMMAND_REG);
    }
    else        // dma/udma
    {
        hwif->OUTB(WIN_READDMA, IDE_COMMAND_REG);
    }

}


    /* advice from ZH, Wen Liu: NTFS require this line before xfer. */
    osal_cache_invalidate(buffer, SECTOR_SIZE*sector_nr);
    for(i=0; i<sector_nr; i+=multi_sector)
    {

        if (!ide_wait_okay(drive, WAIT_OKAY))
        {
            ide_error(drive, (UINT8 *)__FUNCTION__);
            return ERR_FAILUE;
        }

        multi_sector = (drive->mult_count<sector_nr-i)? drive->mult_count: sector_nr - i;        // for WIN_MULTREAD(_EXT)



        if(rw_mode==IDE_PIO_MODE)
        {
#if S3601_SWAP_PIO_R_DATA
            //start swap for read
//            ide_s3601_set_swap_ata(1);
#endif
            //hwif->ata_input_data(drive, buffer+SECTOR_SIZE*i, SECTOR_SIZE);
            //hwif->ata_input_data(drive, buffer+SECTOR_SIZE*i, SECTOR_SIZE*multi_sector);

            for(multi_sector_tmp=1;multi_sector_tmp<=sector_nr;multi_sector_tmp++)
            {
                hwif->ata_input_data(drive, buffer, SECTOR_SIZE);
                buffer=buffer+512;
                osal_delay(20);

                if (!ide_wait_okay(drive, WAIT_OKAY))
                {
                    ide_error(drive, (UINT8 *)__FUNCTION__);
                    return ERR_FAILUE;
                }
            }

#if S3601_SWAP_PIO_R_DATA
            //stop swap for read
            ide_s3601_set_swap_ata(0);
#endif

        }
        else
        {
            if(rw_mode==IDE_PIO_BUR_MODE)
            {    // reduce interrupt times
                for(multi_sector_tmp=1;multi_sector_tmp<=sector_nr;multi_sector_tmp++)
                {
                    ide_s3601_set_interrupt_mask(0x8);    // for PIO Burst , disable device interrupt
                    multi_sector=sector_nr;

                    osal_task_dispatch_off();
                    hwif->start_dma(drive, ((UINT32)buffer)&0x0FFFFFFF,
                        SECTOR_SIZE, RQ_FLAG_DATA_IN,ide_get_rw_mode());
                    osal_task_dispatch_on();

                    ide_s3601_wait_rw_over(drive, RQ_FLAG_DATA_IN,10000,ide_get_flag_mode(ide_get_rw_mode()));

                    //osal_delay(20);
                    IDE_SET_PATH_CPU();
                    //osal_delay(30);

                    if (!ide_wait_okay(drive, WAIT_OKAY))
                    {
                        ide_error(drive, (UINT8 *)__FUNCTION__);
                        return ERR_FAILUE;
                    }
                    buffer=buffer+512;
                }

            }
            else
            {
                //ide_s3601_set_interrupt_mask(0x0);    // enable all interrupt
                ide_s3601_set_interrupt_mask(0x08);        // enabling device interrupt will bring problem in some type hd-disk.
                multi_sector=sector_nr;

                hwif->start_dma(drive, ((UINT32)buffer+(i<<SECTOR_BITS))&0x0FFFFFFF,
                    SECTOR_SIZE*multi_sector, RQ_FLAG_DATA_IN,ide_get_rw_mode());
                ret = ide_s3601_wait_rw_over(drive, RQ_FLAG_DATA_IN,10000,ide_get_flag_mode(ide_get_rw_mode()));
                // set CPU path
                IDE_SET_PATH_CPU();
            }

#if S3601_SWAP_DATA
            //stop swap for read
            ide_s3601_set_swap_ata(0);
#endif

            ide_s3601_set_interrupt_mask(0x1F);    // disable all interrupt
        }//if(rw_mode==IDE_PIO_MODE)


#ifdef TEST_IDE_PERFORMANCE
        time_end=osal_get_tick();
        if(time_end!=time_beg)
        {
            read_speed=(multi_sector*SECTOR_SIZE*1000)/(time_end-time_beg);
            IDE_PRINTF("read speed(Bytes/S): %d \r\n",read_speed);
        }
#endif
        //ide_wait_not_busy(HWIF(drive), WAIT_READY);
        HWIF(drive)->INB(IDE_STATUS_REG);
    }

    return ret;
}


UINT32 ide_try_read_buff(void)
{
    UINT32 rd = g_shift_read_sector;
    UINT32 wr = g_shift_write_sector;
    UINT32 rem_data;
    if(rd>wr)
        rem_data = rd - wr;
    else if(wr>rd)
        rem_data = wr - rd;
    else
        rem_data = 0;
    return rem_data;
}

INT32 hdd_read_sector(UINT32 block,  UINT8 *buffer,UINT16 sector_nr)
{
    INT32 read_return;
    IDE_PRINTF("R%x",block);

    if(ide_hwif.drives[0].attach == 1)
    {
        //is not semaphore here, but mutex! used to protect against IDE r/w re-access.
        if(ide_hwif.semaphore!= OSAL_INVALID_ID)
            osal_mutex_lock(ide_hwif.semaphore, OSAL_WAIT_FOREVER_TIME);
        else
            IDE_PRINTF("%s()=>(ide_hwif.semaphore == OSAL_INVALID_ID)\n", __FUNCTION__);

        if(ide_mutex_id.lock)
            ide_mutex_id.lock();
        read_return =  ide_read_sector(&ide_hwif.drives[0], block,  sector_nr, buffer);
        if(ide_mutex_id.unlock)
            ide_mutex_id.unlock();

        IDE_PRINTF("-%x",sector_nr);

        if(ide_hwif.semaphore != OSAL_INVALID_ID)
            osal_mutex_unlock(ide_hwif.semaphore);
    }
    else
    {
        read_return = !RET_SUCCESS;
    }

    IDE_PRINTF("-%x ",read_return);
     return read_return;
}


INT32 ide_read_buff( UINT16 sector_nr, UINT8 *buffer)
{
    //static UINT32 block = shift_start_sector;
    UINT32 rd = g_shift_read_sector;
    UINT32 wr = g_shift_write_sector;
    UINT32 rem_data;
    if(rd>wr)
        rem_data = rd - wr;
    else if(wr>rd)
        rem_data = wr - rd;
    else
        rem_data = 0;
    if(rem_data < (188*2))
        return 0;


    if(ide_hwif.semaphore != OSAL_INVALID_ID)
        osal_mutex_lock(ide_hwif.semaphore, OSAL_WAIT_FOREVER_TIME);
    else
        IDE_PRINTF("%s()=>(ide_hwif.semaphore == OSAL_INVALID_ID)\n", __FUNCTION__);

    ide_read_sector(&ide_hwif.drives[0], rd,  sector_nr, buffer);

    if(ide_hwif.semaphore != OSAL_INVALID_ID)
        osal_mutex_unlock(ide_hwif.semaphore);


    rd += sector_nr;
    if(rd >= shift_end_sector)
        rd = shift_start_sector;
    g_shift_read_sector = rd;
    return 1;
}


/*
*Func name: ide_write_sector
*Desc: Write nums of sectors of IDE device.
*Arguments:    ->drive: IDE drive(Master or Slave, only Master supported here)
*            ->block: Right position of IDE device where Source data will be written to.
*            ->sector_nr: Writen sector length.
*            ->buffer: Source data buffer address.
*/
#if (1)    // lba 48
INT32 ide_write_sector(ide_drive_t *drive, unsigned long long block, UINT16 sector_nr, UINT8 *buffer)
#else    // lba 28
INT32 ide_write_sector(ide_drive_t *drive, UINT32 block, UINT16 sector_nr, UINT8 *buffer)
#endif
{
    ide_startstop_t startstop=ide_started;
    INT32 i;
    INT32 multi_sector_tmp;
    INT32 multi_sector;
    hwif_t *hwif = HWIF(drive);
    UINT8 tasklets[10];
    UINT8 rw_mode;

    RET_CODE ret = SUCCESS;

#ifdef TEST_IDE_PERFORMANCE
    UINT32 time_beg;
    UINT32 time_end;
    UINT32 write_speed;
#endif

#ifdef TEST_IDE_PERFORMANCE
        time_beg=osal_get_tick();
#endif

    //if (block>=drive->capacity64)
        //return -EINVAL;

    SELECT_DRIVE(drive);
    if (IDE_CONTROL_REG)
    {
        // bit 1(nIEN):0->enable int rq to host, 1->disable it.
        hwif->OUTB(0x08, IDE_CONTROL_REG);
        //hwif->OUTB(0x0A, IDE_CONTROL_REG);
    }

    ide_wait_stat(&startstop, drive, drive->ready_stat, BUSY_STAT, WAIT_READY);
    if (startstop==ide_stopped)
    {
        ide_error(drive, (UINT8 *)__FUNCTION__);
        //return ERR_FAILUE; //next rw op maybe success.
        return ERR_FAILUE;
    }

if(lba_bit_mode == LBA_48)    // lba48
{
    tasklets[0] = 0;
    tasklets[1] = 0;
    tasklets[2] = (UINT8)(sector_nr&0xFF);    // low 8 bits
    tasklets[3] = (UINT8)(sector_nr>>8);    // hight 8 bits
    tasklets[4] = (UINT8) block;
    tasklets[5] = (UINT8) (block>>8);
    tasklets[6] = (UINT8) (block>>16);
    tasklets[7] = (UINT8) (block>>24);
    tasklets[8] = (UINT8) (block>>32);
    tasklets[9] = (UINT8) (block>>40);

/*
    IDE_PRINTF("%s: 0x%02x%02x 0x%02x%02x%02x%02x%02x%02x\n", drive->name,
        tasklets[3], tasklets[2], tasklets[9], tasklets[8],
        tasklets[7], tasklets[6], tasklets[5], tasklets[4]);
*/
    hwif->OUTB(tasklets[1], IDE_FEATURE_REG);
    hwif->OUTB(tasklets[3], IDE_NSECTOR_REG);
    hwif->OUTB(tasklets[7], IDE_SECTOR_REG);
    hwif->OUTB(tasklets[8], IDE_LCYL_REG);
    hwif->OUTB(tasklets[9], IDE_HCYL_REG);

    hwif->OUTB(tasklets[0], IDE_FEATURE_REG);
    hwif->OUTB(tasklets[2], IDE_NSECTOR_REG);
    hwif->OUTB(tasklets[4], IDE_SECTOR_REG);
    hwif->OUTB(tasklets[5], IDE_LCYL_REG);
    hwif->OUTB(tasklets[6], IDE_HCYL_REG);
    hwif->OUTB(0x00|drive->select.all,IDE_SELECT_REG);

    osal_delay(1);

    rw_mode=ide_get_rw_mode();
    if(rw_mode==IDE_PIO_MODE)    // PIO
    {
        hwif->OUTB(0x34, IDE_COMMAND_REG);
        //hwif->OUTB(WIN_MULTWRITE_EXT, IDE_COMMAND_REG);
    }
    else if(rw_mode==IDE_PIO_BUR_MODE)    // PIO burst
    {
        hwif->OUTB(0x34, IDE_COMMAND_REG);
        //hwif->OUTB(WIN_MULTWRITE_EXT, IDE_COMMAND_REG);
    }
    else        // dma/udma
    {
        hwif->OUTB(WIN_WRITEDMA_EXT, IDE_COMMAND_REG);
    }

}
else    // lba28
{

    tasklets[0] = 0;
    tasklets[1] = 0;
    tasklets[2] = (UINT8)(sector_nr&0xFF);
    tasklets[3] = 0;
    tasklets[4] = (UINT8) block;
    tasklets[5] = (UINT8) (block>>8);
    tasklets[6] = (UINT8) (block>>16);
    tasklets[7] = (UINT8) (block>>24);
    tasklets[8] = (task_ioreg_t) 0;
    tasklets[9] = (task_ioreg_t) 0;
/*
    IDE_PRINTF("%s: 0x%02x%02x 0x%02x%02x%02x%02x%02x%02x\n",
        drive->name, tasklets[3], tasklets[2],
        tasklets[9], tasklets[8], tasklets[7],
        tasklets[6], tasklets[5], tasklets[4]);
*/
    hwif->OUTB(tasklets[0], IDE_FEATURE_REG);
    hwif->OUTB(tasklets[2], IDE_NSECTOR_REG);
    hwif->OUTB(tasklets[4], IDE_SECTOR_REG);
    hwif->OUTB(tasklets[5], IDE_LCYL_REG);
    hwif->OUTB(tasklets[6], IDE_HCYL_REG);
    hwif->OUTB((tasklets[7]&0x0f)|drive->select.all,IDE_SELECT_REG);

    osal_delay(10);

    rw_mode=ide_get_rw_mode();
    if(rw_mode==IDE_PIO_MODE)    // PIO
    {
        hwif->OUTB(0x30, IDE_COMMAND_REG);
        //hwif->OUTB(WIN_MULTWRITE, IDE_COMMAND_REG);
    }
    else if(rw_mode==IDE_PIO_BUR_MODE)    // PIO burst
    {
        hwif->OUTB(0x30, IDE_COMMAND_REG);
        //hwif->OUTB(WIN_MULTWRITE, IDE_COMMAND_REG);
    }
    else        // dma/udma
    {
        hwif->OUTB(WIN_WRITEDMA, IDE_COMMAND_REG);
    }

}


    osal_cache_flush(buffer, sector_nr*SECTOR_SIZE);
    for(i=0; i<sector_nr; i+=multi_sector)
    {
        //hwif->INB(IDE_STATUS_REG);

        if (!ide_wait_okay(drive, WAIT_OKAY))
        {
            ide_error(drive, (UINT8 *)__FUNCTION__);
            return ERR_FAILUE;
        }

        multi_sector = (drive->mult_count<sector_nr-i)? drive->mult_count: sector_nr - i;        // for WIN_MULTWRITE(_EXT)


        if(rw_mode==IDE_PIO_MODE)
        {
#if S3601_SWAP_DATA        // swap is invalid for PIO write
            //start swap for write
            //ide_s3601_set_chd_swap_atafifo(1);
#endif
            //hwif->ata_output_data(drive, buffer+(i<<SECTOR_BITS), SECTOR_SIZE*multi_sector);
            //070819-jacky for write pio 512byte intrq issue!
            for(multi_sector_tmp=1;multi_sector_tmp<=multi_sector;multi_sector_tmp++)
            {
                hwif->ata_output_data(drive, buffer, SECTOR_SIZE);
                buffer=buffer+512;

                if (!ide_wait_okay(drive, WAIT_OKAY))
                {
                    ide_error(drive, (UINT8 *)__FUNCTION__);
                    return ERR_FAILUE;
                }
            }
#if S3601_SWAP_DATA
            //stop swap for write
            //ide_s3601_set_chd_swap_atafifo(0);
#endif
        }
        else
        {
            if( rw_mode==IDE_PIO_BUR_MODE)
            {    // reduce interrupt times
                for(multi_sector_tmp=1;multi_sector_tmp<=sector_nr;multi_sector_tmp++)
                {
                    ide_s3601_set_interrupt_mask(0x8);  // enabling device interrupt will bring problem in some type hd-disk.
                    multi_sector=sector_nr;

                    osal_task_dispatch_off();
                    hwif->start_dma(drive, ((UINT32)buffer)&0x0FFFFFFF,
                        SECTOR_SIZE, RQ_FLAG_DATA_OUT,ide_get_rw_mode());
                    osal_task_dispatch_on();

                    ide_s3601_wait_rw_over(drive, RQ_FLAG_DATA_OUT,10000,ide_get_flag_mode(ide_get_rw_mode()));

                    //osal_delay(20);
                    IDE_SET_PATH_CPU();
                    //osal_delay(30);

                    if (!ide_wait_okay(drive, WAIT_OKAY))
                    {
                        ide_error(drive, (UINT8 *)__FUNCTION__);
                        return ERR_FAILUE;
                    }
                    buffer=buffer+512*multi_sector;
                }
            }
            else
            {

                //ide_s3601_set_interrupt_mask(0x0);    // enable all interrupt
                ide_s3601_set_interrupt_mask(0x08);  // enabling device interrupt will bring problem in some type hd-disk.
                multi_sector=sector_nr;

                hwif->start_dma(drive, ((UINT32)buffer+(i<<SECTOR_BITS))&0x0FFFFFFF,
                    SECTOR_SIZE*multi_sector, RQ_FLAG_DATA_OUT,ide_get_rw_mode());
                ret = ide_s3601_wait_rw_over(drive, RQ_FLAG_DATA_OUT,10000,ide_get_flag_mode(ide_get_rw_mode()));

                //set CPU path
                IDE_SET_PATH_CPU();

#if S3601_SWAP_DATA
                //stop swap for write
                ide_s3601_set_chd_swap_atafifo(0);
#endif
            }

            ide_s3601_set_interrupt_mask(0x1F);    // disable all interrupt
        }//if(rw_mode==IDE_PIO_MODE)


#ifdef TEST_IDE_PERFORMANCE
        time_end=osal_get_tick();
        if(time_end!=time_beg)
        {
            write_speed=(multi_sector*SECTOR_SIZE*1000)/(time_end-time_beg);
            IDE_PRINTF("write speed(Bytes/S): %d \r\n",write_speed);
        }
#endif

        HWIF(drive)->INB(IDE_STATUS_REG);
        //ide_wait_not_busy(hwif, WAIT_READY);
    }

    return ret;
}

INT32 hdd_write_sector(UINT32 block, UINT8 *buffer, UINT16 sector_nr)
{
    INT32 write_return;
    IDE_PRINTF("W%x",block);

    if(ide_hwif.drives[0].attach == 1)
    {
        //is not semaphore here, but mutex! used to protect against IDE r/w re-access.
        if(ide_hwif.semaphore != OSAL_INVALID_ID)
            osal_mutex_lock(ide_hwif.semaphore, OSAL_WAIT_FOREVER_TIME);
        else
            IDE_PRINTF("%s()=>(ide_hwif.semaphore == OSAL_INVALID_ID)\n", __FUNCTION__);

        if(ide_mutex_id.lock)
            ide_mutex_id.lock();
        write_return =  ide_write_sector(&ide_hwif.drives[0], block, sector_nr, buffer);
        if(ide_mutex_id.unlock)
            ide_mutex_id.unlock();

        IDE_PRINTF("-%x",sector_nr);
        if(ide_hwif.semaphore != OSAL_INVALID_ID)
            osal_mutex_unlock(ide_hwif.semaphore);
    }
    else
    {
        IDE_PRINTF("%s()=>IDE drive not attached.\n", __FUNCTION__);
        write_return = !RET_SUCCESS;
    }

    IDE_PRINTF("-%x ",write_return);
    return write_return;
}

INT32 ide_write_buff( UINT16 sector_nr, UINT8 *buffer)
{
    //static UINT32 block = shift_start_sector;
    UINT32 rd = g_shift_read_sector;
    UINT32 wr = g_shift_write_sector;
    UINT32 rem_space;
    if(rd>wr)
        rem_space = rd - wr;
    else
        rem_space = shift_end_sector - wr + rd ;
    if(rem_space < (UINT32)(sector_nr*2))
        return RET_FAILURE;

    if(ide_hwif.semaphore != OSAL_INVALID_ID)
        osal_mutex_lock(ide_hwif.semaphore, OSAL_WAIT_FOREVER_TIME);
    else
        IDE_PRINTF("%s()=>(ide_hwif.semaphore == OSAL_INVALID_ID)\n", __FUNCTION__);

    ide_write_sector(&ide_hwif.drives[0], wr, sector_nr, buffer);

    if(ide_hwif.semaphore != OSAL_INVALID_ID)
        osal_mutex_unlock(ide_hwif.semaphore);


    wr += sector_nr;
    if(wr>=shift_end_sector)
        wr = shift_start_sector;
    g_shift_write_sector = wr;

    return RET_SUCCESS;
}

INT32 ide_exec_command(ide_drive_t *drive, UINT8 cmd, UINT8 feature, UINT8 nsector)
{
    INT32 ret;
    ide_startstop_t startstop;
    UINT8 stat;
    hwif_t *hwif = HWIF(drive);
    SELECT_DRIVE(drive);
    hwif->OUTB(2|drive->ctl, IDE_CONTROL_REG); /* disable interrupt */
    hwif->OUTB(feature, IDE_FEATURE_REG);
    hwif->OUTB(nsector, IDE_NSECTOR_REG);
    hwif->OUTB(0, IDE_SECTOR_REG);
    hwif->OUTB(0, IDE_LCYL_REG);
    hwif->OUTB(0, IDE_HCYL_REG);
    hwif->OUTB(drive->select.all, IDE_SELECT_REG);
    hwif->OUTB(cmd, IDE_COMMAND_REG);
    osal_delay(1);     /* for 400ns */
    if (ide_wait_stat(&startstop, drive, 0, BUSY_STAT, 4000)==0) {
        stat = hwif->INB(IDE_ALTSTATUS_REG);
        if (stat&(DRQ_STAT|ERR_STAT|WRERR_STAT)) {
            ret = 2;
        } else {
            ret = 0;
        }
    } else {
        ret = 1;
    }
    hwif->OUTB(drive->ctl, IDE_CONTROL_REG);
    ide_wait_not_busy(HWIF(drive), WAIT_READY);
    return ret;
}


