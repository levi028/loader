#ifndef _ALI_3G_MDM_API_H_
#define _ALI_3G_MDM_API_H_

#include <sys_config.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <os/tds2/itron.h>
#include <osal/osal_timer.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>

#include "termios.h"
#include <hld/hld_dev.h>

#define MAX_TTY_NUM                 254

/* Ioctl command type define */
#define ALI_MDM_IOCTL_TCFLSH        0x5407

#define ALI_MDM_IOCTL_TCGETS        0x540d
#define ALI_MDM_IOCTL_TCSETS        0x540e
#define ALI_MDM_IOCTL_TCSETSW         0x540F
#define ALI_MDM_IOCTL_TCSETSF         0x5410

#define ALI_MDM_IOCTL_TIOCMBIS      0x5416
#define ALI_MDM_IOCTL_TIOCMBIC      0x5417


#define ALI_MDM_IOCTL_TIOCGSERIAL    0x5484
#define ALI_MDM_IOCTL_TIOCSSERIAL    0x5485

#define ALI_MDM_IOCTL_TIOCMGET      0x741d
#define ALI_MDM_IOCTL_TIOCMSET      0x741a

#define ALI_MDM_IOCTL_READABLE      0x5493

/* tcsetattr uses these */
#define TCSANOW     0        /* the change occurs immediately. */
#define TCSADRAIN   1    /* the change occurs after all output written to fd has been transmitted.
                                              This function should be used when changing parameters that affect output. */
#define TCSAFLUSH   2    /* the change occurs after all output written to the object referred by fd has been transmitted,
                                               and all input that has been received but not read will be discarded before the change is made. */

/* tcflush() and TCFLSH use these */
#define TCIFLUSH    0
#define TCOFLUSH    1
#define TCIOFLUSH   2


/********************************************************************
 * ALi 3G Modem device structure                                      *
 ********************************************************************/

struct ali_3g_modem_dev {
    struct hld_device             *next;                                    /*next device */
    INT32                     type;
    INT8                     name[HLD_MAX_NAME_SIZE];
    INT32                       flags;

    INT32                index;                      /* device index */
    UINT32                udev;
    UINT16                usb_vendor_id;
    UINT16                usb_product_id;
    INT8                     *product;                /* string to describe product */
    INT8                     *manufacturer;            /* string to describe manufacturer */
    INT8                     *serial;                    /* string to describe serial number */
    void                            *priv;                      /* pointer to private data (struct tty_driver) */
    BOOL                    notify;
    INT32  (*open)(struct ali_3g_modem_dev *dev);       /* return with file descriptor tty_fd */
    INT32  (*close)(INT32 ali_3g_modem_fd);       
    INT32  (*read)(INT32 ali_3g_modem_fd, char *buffer, UINT32 size);
    INT32  (*write)(INT32 ali_3g_modem_fd, char *buffer, UINT32 size);
    INT32  (*ioctl)(INT32 ali_3g_modem_fd, UINT32 cmd, UINT32 arg);
};

 /********************************************************************
  * Function to get ALi 3G Driver Vesrion Info                                 *
  ********************************************************************/
 char *ali_3g_modem_get_vesrion(void);

/********************************************************************
 * ALi 3G Modem Module Init function                                  *
 ********************************************************************/
RET_CODE ali_3g_modem_init(void);

/********************************************************************
 * API for register callback notify 3g modem plug in/out              *
 ********************************************************************/
#define MODEM_DEVICE_PLUG_IN        0x0001
#define MODEM_DEVICE_PLUG_OUT       0x0002

typedef int (*MDM_NOTIFY_CALL_FUN_PTR)(unsigned long event, void *mdm_dev);

void ali_3g_modem_dev_register_notify(MDM_NOTIFY_CALL_FUN_PTR);
void ali_3g_modem_dev_unregister_notify(MDM_NOTIFY_CALL_FUN_PTR);

/********************************************************************
 * API for register callback notify usb rs232 plug in/out              *
 ********************************************************************/
#define RS232_DEVICE_PLUG_IN        0x0003
#define RS232_DEVICE_PLUG_OUT       0x0004

typedef int (*RS232_NOTIFY_CALL_FUN_PTR)(unsigned long event, void *rs232_dev);

void ali_rs232_dev_register_notify(RS232_NOTIFY_CALL_FUN_PTR);
void ali_rs232_dev_unregister_notify(RS232_NOTIFY_CALL_FUN_PTR);


/********************************************************************
 * API for register callback to init porting usb serial driver              *
 ********************************************************************/
typedef int (*RS232_DRIVER_INIT_CALL_FUN_PTR)(void);
void ali_rs232_driver_init_register(RS232_DRIVER_INIT_CALL_FUN_PTR);

/********************************************************************
 * API for set/get 3g modem attribute                               *
 * Reference:                                                       *
 *   http://www.mkssoftware.com/docs/man3/tcsetattr.3.asp           *
 *   http://book.51cto.com/art/200711/59746.htm                     *
 *   http://linux.about.com/library/cmd/blcmdl3_tcgetattr.htm       *
 ********************************************************************/
int ali_3g_modem_tcgetattr(int fd, struct termios *termios_p);
int ali_3g_modem_tcsetattr(int fd, int optional_actions, const struct termios *termios_p);
int ali_3g_modem_tcflush (int fd, int queue_selector);

/********************************************************************
 * API for flush terminal data
 * Reference: http://www.mkssoftware.com/docs/man3/tcflush.3.asp
 * queue_selector:
 *      TCIFLUSH (flush data received but not read),
 *      TCOFLUSH (flush data written but not transmitted)
 *      TCIOFLUSH (flush both).
int ali_3g_modem_tcflush(int fd, int queue_selector);
********************************************************************/

/********************************************************************
 * API for set/get 3g modem in/out speed                            *
 ********************************************************************/
speed_t ali_3g_modem_cfgetispeed(const struct termios *termios_p);
speed_t ali_3g_modem_cfgetospeed(const struct termios *termios_p);
int ali_3g_modem_cfsetispeed(struct termios *termios_p, speed_t speed);
int ali_3g_modem_cfsetospeed(struct termios *termios_p, speed_t speed);
int ali_3g_modem_cfsetspeed (struct termios *termios_p, speed_t speed) ;


/********************************************************************
 * Functions for Customer to Add New 3G Dongle                            *
 ********************************************************************/
#define MATCH_NONE                    0x0000
#define MATCH_VENDOR                0x0001
#define MATCH_PRODUCT                0x0002
#define MATCH_INT_CLASS            0x0040
#define MATCH_INT_SUBCLASS        0x0080
#define MATCH_INT_PROTOCOL        0x0100

#define MATCH_DEVICE                (MATCH_VENDOR|MATCH_PRODUCT)
#define MATCH_INTERFACE            (MATCH_INT_CLASS|MATCH_INT_SUBCLASS|MATCH_INT_PROTOCOL)
#define MATCH_DEVICE_INTERFACE    (MATCH_DEVICE|MATCH_INTERFACE)
#define MATCH_VENDOR_INTERFACE    (MATCH_VENDOR|MATCH_INTERFACE)

struct dongle_info {
    char*             name;

    /* Before ModeSwitch, VendorID, Product ID */
    unsigned short    storage_vendor_id;
    unsigned short    storeage_product_id;

    /* After ModeSwitch, VendorID, Product ID, Intarface Info */
    unsigned short    modem_match_flag;
    unsigned short    modem_vendor_id;
    unsigned short    modem_product_id;
    unsigned char        modem_interface_class;
    unsigned char        modem_interface_sub_class;
    unsigned char        modem_interface_protocol;

    /* USB ModeSwitch Command */
    char*             modeswitch_cmd;

};

int ali_3g_modem_add_support(struct dongle_info new_dongle );

#define USBNET_DEVICE_PLUG_IN		0x0001
#define USBNET_DEVICE_PLUG_OUT   	0x0002
typedef int (*USBNET_NOTIFY_CALL_FUN_PTR)(unsigned long event, void *usbnet_dev);
void ali_usbnet_dev_register_notify(USBNET_NOTIFY_CALL_FUN_PTR notify_call);
void ali_usbnet_dev_unregister_notify(USBNET_NOTIFY_CALL_FUN_PTR notify_call);
#endif /*_ALI_3G_MDM_API_H_ */

