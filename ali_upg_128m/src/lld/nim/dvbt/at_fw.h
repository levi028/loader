/**
 \file at_fw.h

 \brief  demod definition.
 \date   27/01/2011 Created by pillouxv

 \section section1 file description :
    Header file for structure and define definition

    Abilis Systems copyright 2005-2009
*/


#ifndef _at_fw_H_
#define _at_fw_H_

/*******************************/
/***** GLOBAL DEFINITION *******/
/*******************************/
/** Address size definition in number of byte */
#define ADDRESS_SIZE         (0x4)
/** Data table size definition in number of byte */
#define DATA_TABLE_SIZE     (0x20)
/**Demod table size in number of byte */
#define DEMOD_TABLE_SIZE     (0x00000000)
/**Firmware table size definition in number of byte */
#define FW_TABLE_SIZE         (0x00003252)

/*******************************/
/***** STRUCT DEFINITION *******/
/*******************************/
typedef struct {
 unsigned short length;
 unsigned char  address[ADDRESS_SIZE];
 unsigned char  data[DATA_TABLE_SIZE];
} fw_data_format;

#ifdef __cplusplus
extern "C" {
#endif
/* Firmware section table */
extern const fw_data_format fw_table[];
/* Demod section table */
extern const fw_data_format demod_table[];
#ifdef __cplusplus
}
#endif
#endif /* _at_fw_H_ */
