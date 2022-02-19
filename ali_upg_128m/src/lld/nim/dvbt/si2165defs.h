#ifndef    _STDDEFS_H_
#define    _STDDEFS_H_
/*#define NULL  0 */
#define EMB_L0_API                    /* empty */
#define EMB_L1_DRIVER_CORE            /* empty */
#define EMB_Si2165_EVB_API            /* empty */
#define EMB_L1_DVB_T_DEMOD_API        /* empty */
#define EMB_L1_DVB_C_DEMOD_API        /* empty */
#define EMB_L1_MULTISTANDARD_API    /* empty */
#define EMB_L1_COMMON_DEMOD_API     /* empty */
#define EMB_L2_Si2165_EVB_API        /* empty */
#define SLAB_WINAPI                 /* empty */
#define WINAPI                         /* empty */
#define CUSTOMER_API                /* empty */

//typedef unsigned char U8;
//typedef unsigned int  U32;
typedef unsigned char bool;
//typedef unsigned char BYTE;

typedef enum CONNECTION_TYPE    {
        SIMU = 0
        #ifdef USB_Capability
            , USB
        #endif /* USB_Capability */
        #ifdef CUSTOMER_Capability
            , CUSTOMER
        #endif /* CUSTOMER_Capability */
}  CONNECTION_TYPE;

typedef struct L0_Context {
    unsigned char   address;
    int nim_id;
    UINT32 i2c_id;
    int             indexSize;
    CONNECTION_TYPE connectionType;
    bool            trackWrite;
    bool            trackRead;
    char           *interfaceType;
    int             parallelPortNb;
    int             ioAddress;
    int             bitrate;
    int             timeOut;
    int             connectionHandle;
    #ifdef USB_Capability
        void           *usb;
        bool            usbOpen;
    #endif /* USB_Capability */
    #ifdef CUSTOMER_Capability
        bool            customerOpen;
    #endif /* CUSTOMER_Capability */
} L0_Context;

typedef struct {
  L0_Context *i2c;
  void       *specific;
  char         readAfterWrite;
  char       *nickname;
  void       *shareMem;
  void       *sharePtr;
  void       *mutexPtr;
  unsigned int currentPage;
} L1_Context;

#if 0 //def TRACES
    #define PRINTF       printf
    #define TRACES_PRINTF                printf
    /* <porting> replace this by your own error handling when it is not possible to allocate memory */
    #define ALLOCATION_ERROR_MESSAGE     printf
    #define TREAT_ERROR_MESSAGE          printf
    #define TRACES_ERR(EXP)   do { printf ("\n -!!!!!- TRACES_ERROR:   %-30s , line %5d : " #EXP "\n\n", __FILE__, __LINE__); } while (0)
    #define TRACES_TRACE(EXP) do { printf (" --- %-30s , line %5d : " #EXP "\n", __FILE__, __LINE__); } while (0)
    #define TRACES_SHOW(EXP)  do { printf (" ... " #EXP "\n"); } while (0)
    #define TRACES_USE(EXP)   do { printf ("" #EXP "\n"); } while (0)
    #define TRACE(EXP) do { printf (" --- %-30s , line %5d : " #EXP "\n", __FILE__, __LINE__); }     while (0)
#else
    #define ALLOCATION_ERROR_MESSAGE     /* empty */
    #define TREAT_ERROR_MESSAGE          /* empty */
    #define TRACES_PRINTF     /* empty */
    #define TRACES_ERR(EXP)   /* empty */
    #define TRACES_TRACE(EXP) /* empty */
    #define TRACES_SHOW(EXP)  /* empty */
    #define TRACES_USE(EXP)   /* empty */
    #define TRACE(EXP)          /* empty */
#endif

#ifdef USB_Capability
    #include "windows.h"
    typedef enum
    {
       RET_OK = 0,
       RET_KO = 2
    } NN6_RET;

    HANDLE USBMutex;
    bool shareCreated;
    bool USB_ownership;

    typedef struct mem_shared_struct
     {
       /* This receives a pointer within the current process at which the
       // shared memory is located.
       // The same shared memory may reside at different addresses in other
       // processes which share it.*/

           void *    location;
           HANDLE    hFileMapping;
    } mem_shared_struct;

    mem_shared_struct   *token;

#endif /* USB_Capability */
#endif /* _STDDEFS_H_ */
