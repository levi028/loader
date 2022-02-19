#ifndef __NIM_TEST_H__
#define __NIM_TEST_H__

#include <bus/sci/sci.h>

//#define NIM_TEST_SUPPORT

#if 0//it shoud define this macro for ETree and CTree, and it should comment this macro for STree 
#define nim_di_seq_c_operate nim_DiSEqC_operate
#endif

#define NIM_GET_BYTE(i)             (*(volatile UINT8 *)(i))
#define NIM_SET_BYTE(i,d)          (*(volatile UINT8 *)(i)) = (d)

#define C3501_BASE_ADDR             0xb8003000
#define C3503_BASE_ADDR             0xb8028000
#define C3505_BASE_ADDR             0xb804c000
#define SYS_WTD_ADDR                0xb8018504

#define NIM_TEST_SEND_CHAR(a)		sci_write(SCI_FOR_RS232, a)
#define NIM_TEST_RECV_CHAR()		sci_read(SCI_FOR_RS232)

struct str_nim_test_cmd
{
    unsigned char *command;
    unsigned char *param;    
    unsigned char *help_info; 
    int (*handler)(unsigned int argc, unsigned char *argv[]);
};

void nim_test_init();

int nim_test_get_chip_type(unsigned int argc, unsigned char *argv[]);
int nim_test_help(unsigned int argc, unsigned char *argv[]);
int nim_test_stop(unsigned int argc, unsigned char *argv[]);
int nim_test_get_channel_info(unsigned int argc, unsigned char *argv[]);
int nim_test_reg_read(unsigned int argc, unsigned char *argv[]);
int nim_test_reg_write(unsigned int argc, unsigned char *argv[]);
int nim_test_diseqc(unsigned int argc, unsigned char *argv[]);
int nim_diseqc_test_on(unsigned int argc, unsigned char *argv[]);
int nim_diseqc_test_off(unsigned int argc, unsigned char *argv[]);
int nim_test_polar(unsigned int argc, unsigned char *argv[]);
int nim_test_22k(unsigned int argc, unsigned char *argv[]);
int nim_test_diseqc_write_bytes(unsigned int argc, unsigned char *argv[]);
int nim_test_toneburst(unsigned int argc, unsigned char *argv[]);
int nim_test_envelope(unsigned int argc, unsigned char *argv[]);
int nim_test_combined_transmission(unsigned int argc, unsigned char *argv[]);


#endif
