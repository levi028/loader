#ifndef __panel_H__
#define __panel_H__
#include "sys.h"
//======================================================================================================================//

#define FIRST_SCAN_GOING                             (0x0)
#define FIRST_SCAN_DONE                              (0x1)
#define INVALID_PANEL_KEY1                         (0x0)
#define INVALID_PANEL_KEY2                         (0xFF)

//ch455 led map as follow:
//{'.', 0x80},
//{'0', 0x3f}, {'1', 0x06}, {'2', 0x5b}, {'3', 0x4f},
//{'4', 0x66}, {'5', 0x6d}, {'6', 0x7d}, {'7', 0x07},
//{'8', 0x7f}, {'9', 0x6f}, {'a', 0x77}, {'A', 0x77},
//{'b', 0x7c}, {'B', 0x7c}, {'c', 0x39}, {'C', 0x39},
//{'d', 0x5e}, {'D', 0x5e}, {'e', 0x79}, {'E', 0x79},
//{'f', 0x71}, {'F', 0x71}, {'g', 0x6f}, {'G', 0x3d},
//{'h', 0x76}, {'H', 0x76}, {'i', 0x04}, {'I', 0x30},
//{'j', 0x0e}, {'J', 0x0e}, {'l', 0x38}, {'L', 0x38},
//{'n', 0x54}, {'N', 0x37}, {'o', 0x5c}, {'O', 0x3f},
//{'p', 0x73}, {'P', 0x73}, {'q', 0x67}, {'Q', 0x67},
//{'r', 0x50}, {'R', 0x77}, {'s', 0x6d}, {'S', 0x6d},
//{'t', 0x78}, {'T', 0x31}, {'u', 0x3e}, {'U', 0x3e},
//{'y', 0x6e}, {'Y', 0x6e}, {'z', 0x5b}, {'Z', 0x5b},
//{':', 0x80}, {'-', 0x40}, {'_', 0x08}, {' ', 0x00},

/*FD650 bitmap table.*/
/*=========================SEGMENT NOTE=======================================
 ----SEG0----
 |                   |
 SEG6             SEG1
 |                   |
 ----SEG5----
 |                   |
 SEG4            SEG2
 |                   |
 ----SEG3----
***************SEGMENT NOTE***************/

enum SHOW_TYPE
{
    /*Panel show ' OFF'.*/
    SHOW_OFF = 0,

    /*Panel show current time.*/
    SHOW_TIME,

    /*Panel show '    '.*/
    SHOW_BANK,

    /*Panel default show ' OFF'.*/
    SHOW_DEFAULT,

    /*just keep panel on, do not changed the content.*/
    SHOW_NO_CHANGE,
};
//======================================================================================================================//

void panel_init(void);
void show_panel(enum SHOW_TYPE show_type);
void panel_process(void);
#endif
