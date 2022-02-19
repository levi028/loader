
#ifndef _USB_3GDONGLE_API_H_
#define _USB_3GDONGLE_API_H_

#ifdef USB3G_DONGLE_SUPPORT
#include <api/libtcpip/lwip/sio.h>
#include "hld/ali_3g_mdm/ali_3g_mdm_api.h"

#define MCC_LEN 3
#define MNC_LEN 3

enum USB3G_DONGLE_EVENT
{
    USB3G_DONGLE_HOTPLUG_IN,    //a new dongle plugin
    USB3G_DONGLE_HOTPLUG_OUT,   //a dongle plugout
    USB3G_DONGLE_STATE_CHANGED, //dongle state has been changed. pls see enum usb3gdongle_state
    USB3G_DONGLE_DO_CONNECT,    //the dongle start to do connect. This event will trigger before executing the connection.
};

#define MAX_DONGLE_PORT_CNT 8  // max support 8 ports in one dongle

//usb 3g dongle port informaton
typedef struct
{
        char  valid;        //dongle port valid flag
        char  used;         //this port used to dailup
        char  res[2];
        int   hdl;         //handle of modem port, get from dev->open()
        void *private;     //the related ali_3g_modem_device.
} usb3gdongle_port;

#define APN_SIZE   128
#define PHONE_LEN  32
#define USR_LEN    128
#define PWD_LEN    128

//ISP service information
struct isp_info
{
    struct isp_info *next;
    char  isp_code[MCC_LEN + MNC_LEN + 1];  /*ISP code,consist of MCC+MNC
                                             *MCC:Mobile Country Code, 3 bytes
                                             *MNC:Mobile Network Code, 2~3 bytes
                                             *    European norm:2 bytes
                                             *    North America norm: 3 bytes
                                             */
    char  apn[APN_SIZE];        //access point name
    char  phone_num[PHONE_LEN]; //phone number used to dialup
    char  user[USR_LEN];        //user name
    char  passwd[PWD_LEN];      //password
};

//configuration of dongle
typedef struct
{
    struct isp_info isp; // ISP info
    int  baud_rate;     //baud rate of modem, see hld/ali_3g_mdm/termios.h
} usb3gdongle_config;

//dongle state
enum usb3gdongle_state
{
    dongle_state_none,         //invalid dongle
    dongle_state_idle,         //dongle is idle.
    dongle_state_connectting,  //do connectting
    dongle_state_connect_err,  //connect failed
    dongle_state_disconnectting,  //disconnectting
    dongle_state_connected,    //already connected
};

struct usb3gdongle
{
    struct usb3gdongle *next;
    ID     mutex;
    //
    char valid;
    char ready;
    char simready;
    char do_connect;
    //
    int  identify;

    short vendor_id;  //vendor id of this dongle
    short device_id;  //device id of this dongle

    char vendor_info[32];  //vendor name of this dongle
    char product_info[32]; //product name of this dongle
    char serial_no[32];    //serial number
    //
    char imsi[MCC_LEN + MNC_LEN + 10 + 1]; //IMSI of SIM Card
    int  state;
    int  pppfd;
    //
    usb3gdongle_port   ports[MAX_DONGLE_PORT_CNT];
    usb3gdongle_config config;
    struct siofd sio;
} ;

///
typedef void (*usb3g_dongle_cb)(enum USB3G_DONGLE_EVENT event, struct usb3gdongle *dongle);


/******************************************************************************
 * Function: modem_sendat
 * Description: -
 *    send a AT command to modem
 * Input:
 *    modem   : usb3gdongle_port
 *    atcmd   : AT command string
 *    response: used as a input parameter, it stores the expectant response value,
 *              in this case, res_len will be ignored.
 *              eg.
 *                 modem_sendat(modem, "AT\r", "OK", 0);
 *                 if receive "OK" after send "AT\r", it will return 1 else return 0.
 * Output:
 *    response: used as a output parameter, it used to receive response data, it
 *              must be clear before call this function.
 *              in this case, res_len must be the length of the buffer: resonse.
 *              eg.
 *                 memset(buff, 0, sizoef(buff));
 *                 modem_sendat(modem, "AT+CGMI\r", buff, sizeof(buff));
 *                 it used to get the vendor id of modem, the "buff" used to save the response data.
 * Returns:
 *    return 1 if success, else return 0.
 ******************************************************************************/
extern int modem_sendat(usb3gdongle_port *modem, char *atcmd, char *response, int res_len);

/******************************************************************************
 * Function: usb3g_dongle_isready
 * Description: -
 *    check the dongle if is ready
 * Return:
 *    return dongle->ready.
 ******************************************************************************/
extern int usb3g_dongle_isready(struct usb3gdongle *dongle);

/******************************************************************************
 * Function: usb3g_dongle_simready
 * Description: -
 *    check SIM card if is ready
 * Return:
 *    return dongle->simready.
 ******************************************************************************/
extern int usb3g_dongle_simready(struct usb3gdongle *dongle);

/******************************************************************************
 * Function: usb3g_get_dongle_vendor_info
 * Description: -
 *    get the vendor info of this dongle.
 *    if the dongle->vendor is emptry, this function will read vendor info from dongle.
 *    please check dongle->vendor when this function returned, maybe can not read
 *    vendor info from dongle.
 * Return:
 *    return dongle->vendor.
 ******************************************************************************/
extern char *usb3g_get_dongle_vendor_info(struct usb3gdongle *dongle);

/******************************************************************************
 * Function: usb3g_get_dongle_product_info
 * Description: -
 *    get the product info of this dongle.
 *    if the dongle->product is emptry, this function will read product info from dongle.
 *    please check dongle->product when this function returned, maybe can not
 *    read product info from dongle.
 * Return:
 *    return dongle->product.
 ******************************************************************************/
extern char *usb3g_get_dongle_product_info(struct usb3gdongle *dongle);

/******************************************************************************
 * Function: usb3g_get_dongle_signal_strength
 * Description: -
 *    get signal strength of this dongle
 * Return:
 *   return 1 if success, else return 0;
 ******************************************************************************/
extern int usb3g_get_dongle_signal_strength(struct usb3gdongle *dongle, int *quality);

/******************************************************************************
 * Function: usb3g_get_dongle_sim_imsi
 * Description: -
 *    get IMSI of SIM card in dongle.
 *    if the dongle->imsi is emptry, this function will read IMSI from dongle.
 *    please check dongle->imsi when this function returned, maybe can not
 *    read IMSI from dongle.
 * Return:
 *    return dongle->imsi.
 ******************************************************************************/
extern char *usb3g_get_dongle_sim_imsi(struct usb3gdongle *dongle);

/******************************************************************************
 * Function: usb3g_start_dail
 * Description: -
 *    start dialup and do ppp connect on the dongle.
 * Return:
 *   return 1 if start dialup success.
 *   return 0 if dongle is not ready , or sim err, do not set APN or phone number.
 ******************************************************************************/
extern int usb3g_start_dail(struct usb3gdongle *dongle);

/******************************************************************************
 * Function: usb3g_stop_dail
 * Description: -
 *    stop dail connect on the dongle.
 ******************************************************************************/
extern int usb3g_stop_dail(struct usb3gdongle *dongle);

/******************************************************************************
 * Function: usb3g_disconnect_all_dongle
 * Description: -
 *    disconnect all dongles
 ******************************************************************************/
extern int usb3g_disconnect_all_dongle();

/******************************************************************************
 * Function: usb3g_get_dongle_by_dev
 * Description: -
 *    find the dongle by modem dev,
 * Input:
 *    dev: ali_3g_modem_dev device.
 ******************************************************************************/
extern struct usb3gdongle *usb3g_get_dongle_by_dev(struct ali_3g_modem_dev *dev);


/******************************************************************************
 * Function: usb3g_get_dongle_by_index
 * Description: -
 *    get usb 3g dongle by index.
 *    index is start form 1, but can not Exceed the count of dongle.
 ******************************************************************************/
extern struct usb3gdongle *usb3g_get_dongle_by_index(int index);


/******************************************************************************
 * Function: usb3g_reg_isp_info
 * Description: -
 *    register a ISP service information to dongle manager
 * Input:
 *    isp_code : ISP code, consists: MCC+MNC
 *               MCC:Mobile Country Code, 3 bytes
 *               MNC:Mobile Network Code, 2~3 bytes
 *    apn      : access point name, can be NULL.
 *    phone_num: phone number for dialup.
 *    usr      : username, can be NULL.
 *    passwd   : password, can be NULL.
 ******************************************************************************/
extern int usb3g_reg_isp_info(const char isp_code[MCC_LEN + MNC_LEN], char *phone_num,
                              char *apn, char *usr, char *passwd);

/******************************************************************************
 * Function: usb3g_unreg_isp_info
 * Description: -
 *    delete a ISP service information from dongle manager,
 * Input:
 *    isp_code : ISP code, consists: MCC+MNC
 *               MCC:Mobile Country Code, 3 bytes
 *               MNC:Mobile Network Code, 2~3 bytes
 ******************************************************************************/
extern int usb3g_unreg_isp_info(char *isp_code);

/******************************************************************************
 * Function: usb3g_get_isp_service
 * Description: -
 *    get ISP service information by ISP code
 * Input:
 *    isp_code : ISP code, consists: MCC+MNC
 *               MCC:Mobile Country Code, 3 bytes
 *               MNC:Mobile Network Code, 2~3 bytes
 ******************************************************************************/
extern struct isp_info *usb3g_get_isp_service(char *isp_code);

/******************************************************************************
 * Function: usb3g_get_isp_service_by_imsi
 * Description: -
 *    get ISP service information by IMSI
 * Input:
 *    imsi : IMSI of SIM card
 ******************************************************************************/
extern struct isp_info *usb3g_get_isp_service_by_imsi(char *imsi);

/******************************************************************************
 * Function: usb3g_dongle_count
 * Description: -
 *    get the count of usb3g dongles
 ******************************************************************************/
extern int usb3g_dongle_count();

/******************************************************************************
 * Function: usb3g_attach
 * Description: -
 *    struct usb3gdongle module init, needs to be done only once after boot up.
 * Input:
 *    callback: usb dongle event callback.
 ******************************************************************************/
extern void usb3g_attach(usb3g_dongle_cb callback);

/******************************************************************************
 * Function: usb3g_pcap
 * Description: - 
 *    start/stop capture usb 3G data 
 * Input: 
 *    save_dir: the directory for saving captured data.
 *    start   : 1: start capture, 0: stop capture.
 * Returns: 
 *    return file save path if start success, else return NULL.
 * -------------------------------------------------
 ******************************************************************************/
extern char *usb3g_pcap(char *save_dir, UINT8 start);
#endif

#endif
