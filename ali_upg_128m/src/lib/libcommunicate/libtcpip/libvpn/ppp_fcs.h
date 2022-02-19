
#ifndef _PPP_FSC_H_
#define _PPP_FSC_H_
#define PPPINITFCS16    0xffff
#define PPPGOODFCS16    0xf0b8
unsigned short pppfcs16(unsigned short fcs, void *cp, int len);

#endif

