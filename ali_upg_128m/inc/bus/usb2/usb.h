/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: usb.h
*
*    Description: This head file contains usb define application process APIs.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __USB_NEW_H__
#define __USB_NEW_H__

#include <basic_types.h>

#ifdef __cplusplus
extern "C"
{
#endif

//#define USB_DEBUG
//#define USB_DEBUG_ERR     //if debug ,please open this MACRO
//#define USB_DRIVER_WIFI_SUPPORT
//#ifndef USBD_INFO_PRINTF
//#define USBD_INFO_PRINTF libc_printf
//#define USBD_INFO_PRINTF soc_printf
//#define USBD_INFO_PRINTF(...)    do{}while(0)
//#endif

#define USB_COMMAND_TIMEOUT  3
#define MAX_HANDLE_NUM             32
#define USB_HOST0_EN       0x1
#define USB_HOST1_EN       0x2

#define if_event_port_id             UINT32
#define IF_REAR_PROT                1         //usb port default for M3601E/M3601S
#define IF_FRONT_PROT            0
#define USB_OPT_VID          0x1a0a

#define USB_PRODUCT_NAME 256
#define USB_MANUFACTURER_NAME 256
#define USB_SERIAL_NUMBER_NAME 256

struct  usb_dev_info_ex
{
    UINT16 bcd_usb;
    UINT8  b_device_class;
    UINT8  b_device_sub_class;
    UINT8  b_device_protocol;
    UINT16 id_vendor;
    UINT16 id_product;
    UINT16 bcd_device;
};

struct  usb_disk_info_ex
{
    UINT16 bcd_usb;
    UINT16 id_vendor;
    UINT16 id_product;
    UINT8  i_serial_number;
    UINT8 subclass;
    UINT8 protocol;
    UINT8 max_lun;
    UINT32 phy_lbn0[8];
    UINT64 disk_size[8];
    UINT32 blk_size[8];
    UINT32 speed;    //real speed
    UINT32 which_port;
    UINT32 host_id;
    char product_name[USB_PRODUCT_NAME+1];	
    char manufacturer_name[USB_MANUFACTURER_NAME+1];
    char serial_number_name[USB_SERIAL_NUMBER_NAME+1];    
};

struct usb_debug_info
{
    struct usb_disk_info_ex disk;
    UINT32 read_retry_counter;
    UINT32 write_retry_counter;
    UINT32 stall_err;
    UINT32 rsp_err;
    UINT32 tog_err;
    UINT32 pid_err;
    UINT32 utmi_err;
    UINT32 crc_err;
    UINT32 unexpct_err;
    UINT32 usb_speed;
};

typedef void* USBD_NODE_ID, * p_usbd_node_id;

/*****************************************************************************
 * Function: usbd_m36f_attach
 * Description:
 *   process for USB device attach
 * Input:
 *   Para 1: UINT8 host_en :    host_en is set by caller to choose the port attach
 *                    for M3606, host_en value should be USB_HOST0_EN or USB_HOST1_EN or
 *           USB_HOST0_EN|USB_HOST1_EN
 *   Para 2: void(*notify)(UINT32): A callback for the upper layer. It will
 *           inform upper layer the usb device is connected to or removed from
 *           the port.
 *   Para 3: void(*mount)(UINT32): A callback for the upper layer to mount
 *           device.
 *   Para 4: void(*umount)(UINT32): A callback for the upper layer to umount
 *           device.
 * Output:
 *      None
 *
 * Returns:
 *   RET_SUCCESS = 0    : successful.
 *   !RET_SUCCESS    : failure.
*****************************************************************************/
RET_CODE usbd_m36f_attach(UINT32 host_en ,  void(*notify) (UINT32),
                          void(*mount) (UINT32), void(*umount) (UINT32)  );

/*****************************************************************************
 * Function: usbd_attach_ex
 * Description:
 *   process for USB device attach
 * Input:
 *   Para 1: void(*notify)(UINT32): A callback for the upper layer. It will
 *           inform upper layer the usb device is connected to or removed from
 *           the port.
 *   Para 2: void(*mount)(UINT32): A callback for the upper layer to mount
 *           device.
 *   Para 3: void(*umount)(UINT32): A callback for the upper layer to umount
 *           device.
 * Output:
 *      None
 *
 * Returns:
 *   RET_SUCCESS = 0    : successful.
 *   !RET_SUCCESS    : failure.
*****************************************************************************/
RET_CODE usbd_attach_ex(void(*notify)(UINT32), void(*mount)(UINT32),
                        void(*umount)(UINT32));

/*****************************************************************************
 * Function: usbd_device_ready_ex
 * Description:
 *   process for USB device attach info
 * Input:
 *   Para 1: struct USBD_NODE_ID
 *   Para 2: lun
 *
 *
 * Returns:
 *   RET_SUCCESS = 0    : successful.
 *   !RET_SUCCESS    : failure.
*****************************************************************************/
RET_CODE usbd_device_ready_ex(USBD_NODE_ID node_id,UINT32 lun);

/*****************************************************************************
 * Function: usbd_device_ready_ex
 * Description:
 *   process for USB device attach info
 * Input:
 *   Para 1: struct USBD_NODE_ID
 *   Para 2: struct usb_disk_info *disk_info: pointer to point usb disk info
 *           structure.
 *
 * Returns:
 *   RET_SUCCESS = 0    : successful.
 *   !RET_SUCCESS    : failure.
*****************************************************************************/
INT32  usbd_get_disk_info_ex(USBD_NODE_ID node_id,
                                    struct usb_disk_info_ex *disk_info);

/*****************************************************************************
 * Function: usbd_device_get_disk_count_ex
 * Description:
 *   usbd_device_get_disk_count -  To get a count of the device that connected
 *   on the port.
 *
 * Input:
 *   Para 1: struct USBD_NODE_ID
 *
 * Returns:
 *   RET_SUCCESS = 0    : successful.
 *   !RET_SUCCESS    : failure.
*****************************************************************************/
UINT32 usbd_device_get_disk_count_ex(USBD_NODE_ID node_id);


/*****************************************************************************
 * Function: usbd_device_get_disk_image
 * Description:
 *   To return a bitmap of mounted device (the index number of device is
 *   pointed by position of bitmap).
 *
 * Input:
 *   Para 1: struct USBD_NODE_ID
 *
 * Returns:
 *   RET_SUCCESS = 0    : successful.
 *   !RET_SUCCESS    : failure.
*****************************************************************************/
UINT32 usbd_device_get_disk_image_ex(USBD_NODE_ID node_id);

/*****************************************************************************
 * Function: usbd_get_disk_size_ex
 * Description:
 *   To get the disk size of the usb device.
 * Input:
 *   Para 1: struct USBD_NODE_ID
 *   Para 2: lun
 * 
 *
 * Returns:
 *   RET_SUCCESS = 0    : successful.
 *   !RET_SUCCESS    : failure.
*****************************************************************************/
INT32 usbd_get_disk_size_ex(USBD_NODE_ID node_id,UINT8 lun);
INT64 usbd_get_disk_size_ex_16(USBD_NODE_ID node_id,UINT8 lun);

/*****************************************************************************
 * Function: usbd_device_read_ex
 * Description:
 *   To read data form usb mass-storage device.
 * Input:
 *   Para 1: struct USBD_NODE_ID
 *   Para 2: sub lun - Index number of device.
 *   Para 3: lbn - local block number (Sector number), point to the beginning.
 *   Para 4: buffer: A pointer to point a buffer where will save the data that
 *           read from usb mass-storage device.
 *     Para 5: sector_count - Total sector count that will be read.
 *
 * Returns:
 *   RET_SUCCESS = 0    : successful.
 *   !RET_SUCCESS    : failure.
*****************************************************************************/
INT32 usbd_device_read_ex(USBD_NODE_ID node_id,UINT8 sub_lun, UINT32 lbn, 
                                    UINT8* buffer, UINT32 sector_count);
INT32 usbd_device_read_ex_16(USBD_NODE_ID node_id,UINT8 sub_lun, UINT64 lbn, 
                                     UINT8* buffer, UINT32 sector_count);


/*****************************************************************************
 * Function: usbd_device_write_ex
 * Description:
 *   To write data form usb mass-storage device.
 * Input:
 *   Para 1: struct USBD_NODE_ID
 *   Para 2: sub lun - Index number of device.
 *   Para 3: lbn - local block number (Sector number), point to the beginning.
 *   Para 4: buffer: A pointer to point a buffer where will save the data that
 *           read from usb mass-storage device.
 *     Para 5: sector_count - Total sector count that will be write.
 *
 * Returns:
 *   RET_SUCCESS = 0    : successful.
 *   !RET_SUCCESS    : failure.
*****************************************************************************/
INT32 usbd_device_write_ex(USBD_NODE_ID node_id, UINT8 sub_lun, UINT32 lbn,
                                 UINT8* buffer, UINT32 sector_count);
INT32 usbd_device_write_ex_16(USBD_NODE_ID node_id, UINT8 sub_lun, UINT64 lbn, 
                                 UINT8* buffer, UINT32 sector_count);

/**************************************************************************************
 get_usb_storage_connection_infor -  get number of usb mass-storage device.
 
 Returns:
    0	: no device on bus
    xx : xx device(s) on bus
 ***************************************************************************************/
INT32 get_usb_storage_connection_infor(void);


/*****************************************************************************
 * Function: usbd_power_down
 * Description:
 *   To power donw usb
 * Input:
 *   Para 1: struct USBD_NODE_ID
 *
 * Returns:
 *   none
*****************************************************************************/
void usbd_power_down(USBD_NODE_ID node_id);

/*****************************************************************************
 * Function: usbd_detach_ex
 * Description:
 *   process for USB device deattach
 * Input:
 *   Para 1: struct USBD_NODE_ID
 * Output:
 *      None
 *
 * Returns:
 *   RET_SUCCESS = 0    : successful.
 *   !RET_SUCCESS    : failure.
*****************************************************************************/
RET_CODE usbd_detach_ex();

/*****************************************************************************
 * Function: usbd_detach_ex
 * Description:
 *   process for USB device safely remove
 * Input:
 *   Para 1: struct USBD_NODE_ID
 * Output:
 *      None
 *
 * Returns:
 *      None
*****************************************************************************/
void usbd_safely_remove_ex( USBD_NODE_ID node_id );

/*****************************************************************************
 * Function: usb_device_rw_status
 * Description:
 *   To get usb device in read/write state
 * Input:
 *   Para 1: struct USBD_NODE_ID
 * Output:
 *      None
 *
 * Returns:
 *     0 : idle
 *     1 : in read/write state
*****************************************************************************/
UINT32 usb_device_rw_status( UINT32 node_id );

/*****************************************************************************
 * Function: usbd_check_lun_dev_ready_ex
 * Description:
 *   To get usb device ready state
 * Input:
 *   Para 1: struct USBD_NODE_ID
 *   Para 2: lun : logical unit number
 * Output:
 *      None
 *
 * Returns:
 *     !RET_SUCCESS : failure
 *     RET_SUCCESS : ready
*****************************************************************************/
INT32 usbd_check_lun_dev_ready_ex( USBD_NODE_ID node_id, UINT8 lun );

/*****************************************************************************
 * Function: usbd_get_dev_max_lun_ex
 * Description:
 *   To get usb device logical unit number
 * Input:
 *   Para 1: struct USBD_NODE_ID
 * Output:
 *      None
 *
 * Returns:
 *     numbers of lun
*****************************************************************************/
INT32 usbd_get_dev_max_lun_ex( USBD_NODE_ID node_id );

/*****************************************************************************
 * Function: test_get_dev_descriptor
 * Description:
 *   To get usb device descriptor
 * Input:
 *   Para 1: base_addr : to point a buffer where will save the data
 * Output:
 *      None
 *
 * Returns:
 *     numbers of lun
*****************************************************************************/
void test_get_dev_descriptor(UINT32 base_addr);

/*****************************************************************************
 * Function: wait_ep0_finish
 * Description:
 *   To wait ep0 finish
 * Input:
 *   Para 1: base_addr : to point a buffer where will save the data
 * Output:
 *      None
 *
 * Returns:
 *     numbers of lun
*****************************************************************************/
INT32 wait_ep0_finish(UINT32 base_addr);


typedef void * p_device_descriptor_t  ; //point to struct usb_device_descriptor

typedef enum
{
    IF_NO_EVENT_ASSIGN = 0,        /* No Event to be Notified */
    IF_UI_EVENT_TYPE,            /* UI Event Type to be Notified */
    IF_USBD_EVENT_TYPE,            /* USBD Event Type to be Notified */
} if_event_type;


typedef enum
{
    IF_NO_SUB_EVENT_ASSIGN = 0,        /* No Event to be Notified */
    IF_UI_ENTER_SUB_TYPE,            /* UI Event Type to be Notified */
    IF_USBD_SHOW_TPL_TYPE ,
} if_event_sub_type;

typedef enum
{
    IF_SUB_EVENT_TYPE ,
}if_event_vid ;


typedef enum
{
     pid_test_se0         =    0x0101,
     pid_test_j            =     0x0102,
      pid_test_k             =    0x0103,
     pid_test_packet        =    0x0104,
     pid_test_suspend     =    0x0106,
     pid_test_get_desc     =    0x0107,
     pid_test_get_desc_data = 0x0108,
      pid_test_bus_enumeration  = 0x0109,
}if_event_pid ;



typedef enum
{
    USBD_MASS_DEV            ,
    USBD_HUB_DEV            ,
    USBD_NOT_SUPPORT_DEV        ,
    USBD_UNKNOWN_DEV        ,
}if_event_dev_type;

typedef struct
{
    if_event_type                event_type;
    if_event_sub_type                sub_type ;
    if_event_vid                    vid;
    if_event_pid                    pid ;
    if_event_port_id                port_id ;
    if_event_dev_type            dev_type ;
    p_device_descriptor_t        dev_desc_pt    ;
}if_evt_msg_t ;

enum if_test_flag
{
    IF_TEST_NULL = 0,
    IF_TEST_SE0,
    IF_TEST_J,
    IF_TEST_K,
    IF_TEST_PACKET,
    IF_TEST_SUSPEND,
    IF_TEST_GET_DESC,
    IF_TEST_GET_DESC_DATA,
    IF_TEST_STOP,
    IF_TEST_ENTER,
};

/*****************************************************************************
 * Function: usb_test_enter
 * Description: dynamic remove usb device and disable usbd int
 *                      UI should call this function when enter usb_logo_test show
 * Input:
 *
 * Output:
 *      None
 *
 *  Returns:
 *   RET_SUCCESS = 0    : successful.
 *   !RET_SUCCESS        : failure.
*****************************************************************************/
RET_CODE usb_test_enter(void);     // for UI

/*****************************************************************************
 * Function: usb_test_set_port
 * Description: select test port by UI
 *                       UI should call this function before any test action
 * Input:
 *       Para 1: port_id : to point port number
 *  Returns:
 *   RET_SUCCESS = 0    : successful.
 *   !RET_SUCCESS        : failure.
*****************************************************************************/
RET_CODE usb_test_set_port(if_event_port_id port_id) ;

/*****************************************************************************
 * Function: usb_test_start
 * Description: Send UI msg to usb_test_task  , call by UI
 * Input:
 *       Para 1: vid : verdor_pid / USB VID which shows during enumeration
 *       Para 2: pid : event_pid / USB PID which shows during enumeration
 *  Returns:
 *   RET_SUCCESS = 0    : successful.
 *   !RET_SUCCESS        : failure.
*****************************************************************************/
RET_CODE usb_test_start(UINT32 vid, UINT32 pid);  // for UI

/*****************************************************************************
 * Function: usb_test_stop
 * Description: stop current sub-projetc test, call by UI
 * Input:
 *       None
 *  Returns:
 *   RET_SUCCESS = 0    : successful.
 *   !RET_SUCCESS        : failure.
*****************************************************************************/
RET_CODE usb_test_stop(void);

/*****************************************************************************
 * Function: usb_test_exit
 * Description: stop test, call by UI
 * Input:
 *       None
 *  Returns:
 *   RET_SUCCESS = 0    : successful.
 *   !RET_SUCCESS        : failure.
*****************************************************************************/
RET_CODE usb_test_exit(void);

/*****************************************************************************
 * Function: usb_test_start
 * Description: Send UI enum msg to usb_test_task  , call by UI. *
 * Input:
 *       Para 1: dev_descriptor : usb_device_desctiptor.
 *  Returns:
 *   RET_SUCCESS = 0    : successful.
 *   !RET_SUCCESS        : failure.
*****************************************************************************/
RET_CODE usb_test_bus_enum( p_device_descriptor_t dev_descriptor);

 /*****************************************************************************
 * Function: usb_test_reg_usbd_call_back
 * Description: reg callback  for usb.c
 * Input:
 *       Para 1: RET_CODE (* )(IfEvtMsg_t) : function call
 *  Returns:
 *   None
*****************************************************************************/
void usb_test_reg_usbd_call_back( RET_CODE (* )(if_evt_msg_t))  ;

 /*****************************************************************************
 * Function: usb_test_bus_notify_ex
 * Description: USBD send msg to usb_logo_test
 * Input:
 *       Para 1: RET_CODE (* )(IfEvtMsg_t) : function call
 *  Returns:
 *   None
*****************************************************************************/
RET_CODE usb_test_bus_notify_ex(if_evt_msg_t) ;  // call by USBD

/*****************************************************************************
 * Function: usb_test_bus_notify
 * Description: USBD send msg to usb_logo_test
 * Input:
 *       Para 1: RET_CODE (* )(IfEvtMsg_t) : function call
 *  Returns:
 *   None
*****************************************************************************/
RET_CODE usb_test_bus_notify(if_evt_msg_t);  // do nothing

/*****************************************************************************
 * Function: usb_test_reg_ui_call_back
 * Description: eg callback for usb_logo_test.c
 * Input:
 *       Para 1/2: functions of callback
 *  Returns:
 *   None
*****************************************************************************/
void usb_test_reg_ui_call_back(void(* )(UINT16 *), void(* )(char *, char *));

/*****************************************************************************
 * Function: usb_test_show_string
 * Description: usb_test_show_string
 * Input:
 *       Para 1: display string
 *  Returns:
 *   None
*****************************************************************************/
void usb_test_show_string(UINT16* len_display_str);

/*****************************************************************************
 * Function: usb_test_show_tpl
 * Description: usb test show VID/PID
 * Input:
 *       Para 1: VID
 *       Para 2: PID
 *  Returns:
 *   None
*****************************************************************************/
void usb_test_show_tpl(UINT16 VID,UINT16 PID) ;

/*****************************************************************************
 * Function: usb_test_task_overload
 * Description: usb test task ues
 * Input:
 *       Para 1: param1
 *       Para 2: param2
 *  Returns:
 *   None
*****************************************************************************/
void usb_test_task_overload(UINT32 param1, UINT32 param2);

/*****************************************************************************
 * Function: usb_test_show_tpl
 * Description: usb test show text
 * Input:
 *       Para 1: text string
 *  Returns:
 *   None
*****************************************************************************/
void usb_test_show_txt(char* txt);

/*****************************************************************************
 * Function: usb_test_init
 * Description: API for usb_logo_task init
 * Input:
 *
 *  Returns:
 *   RET_SUCCESS = 0    : successful.
 *   !RET_SUCCESS    : failure.
*****************************************************************************/
RET_CODE usb_test_init(void);

/*****************************************************************************
 * Function: test_reset_bus
 * Description: API for usb bus reset
 * Input:
 *
 *  Returns:
 *   None
*****************************************************************************/
void test_reset_bus(UINT32 base_addr);

void usb_inhibit_rw(int inhibit);
void force_usb_suspend(void);
BOOL usb_rw_completed(void);

#ifdef __cplusplus
}
#endif

#endif/*__USB_H__*/

