 /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: time_zone_name.c
*
*    Description:   The define of time zone
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>

#include "time_zone_name.h"




#if 1

static struct time_zone_name time_zone_citys[] =
{
    {
        -10,0, 1,
            {"Honolulu"}
    },
    {
        -9,    0,  1,
            {"Anchorage"}
    },
    {
        -8,    0,  4,
            {
            "Vancouver",
            "San Francisco",
            "Seattle",
            "Los Angeles"
            }
    },
    {
        -7,    0,  3,
            {
            "Phoenix",
            "Edmonton",
            "Denver"
            }
    },
    {
        -6,    0,  12,
            {
            "San Salvador",
            "Mexico City",
            "Winnipeg",
            "Houston",
            "Minneapolis",
            "St. Paul",
            "Guatemala",
            "New Orleans",
            "Chicago",
            "Tegucigalpa",
            "Montgomery",
            "Managua"
            }
    },
    {
        -5,    0,  16,
            {
            "Lima",
            "Kingston",
            "Bogota",
            "Indianapolis",
            "Atlanta",
            "Detroit",
            "Havana",
            "Miami",
            "Toronto",
            "Nassau",
            "Washington DC",
            "Ottawa",
            "Philadelphia",
            "New York",
            "Montreal",
            "Boston"
            }
    },
    {
        -4,    0,  7,
            {
            "Santiago",
            "Santo Domingo",
            "La Paz",
            "Caracas",
            "San Juan",
            "Asuncion",
            "Halifax"
            }
    },
    {
        -3,    0,  5,
            {
            "Buenos Aires",
            "Montevideo",
            "Brasilia",
            "Sao Paulo",
            "Rio de Janeiro"
            }
    },
    {
        -4,    30, 1,
            {"St. John's"}
    },
    {
        0, 0,  5,
            {
            "Reykjavik",
            "Casablanca",
            "Lisbon",
            "Dublin",
            "London"
            }
    },
    {
        1, 0,  21,
            {
            "Lagos",
            "Algiers",
            "Madrid",
            "Barcelona",
            "Paris",
            "Brussels",
            "Amsterdam",
            "Geneva",
//          "Z¨¹rich",
            "Frankfurt",
            "Oslo",
            "Copenhagen",
            "Rome",
            "Berlin",
            "Prague",
            "Zagreb",
            "Vienna",
            "Stockholm",
            "Cape Town",
            "Budapest",
            "Belgrade",
            "Warsaw"
            }
    },
    {
        2, 0,  17,
            {
            "Johannesburg",
            "Harare",
            "Sofia",
            "Athens",
            "Tallinn",
            "Helsinki",
            "Bucharest",
            "Minsk",
            "Istanbul",
            "Kyiv",
            "Odesa",
            "Cairo",
            "Khartoum",
            "Ankara",
            "Jerusalem",
            "Beirut",
            "Amman"
            }
    },
    {
        3, 0,  9,
            {
            "Nairobi",
            "Addis Ababa",
            "Aden",
            "Riyadh",
            "Antananarivo",
            "Kuwait City",
            "Moscow",
            "Baghdad"
            }
    },
    {
        3, 30, 1,
            {"Tehran"}
    },
    {
        4, 1,  1,
            {"Dubai"}
    },
    {
        4, 30, 1,
            {"Kabul"}
    },
    {
        5, 0,  4,
            {
            "Karachi",
            "Tashkent",
            "Islamabad",
            "Lahore"
            }
    },
    {
        5, 30, 3,
            {
            "Mumbai",
            "New Delhi",
            "Kolkata"
            }
    },
    {
        5, 45, 1,
            {"Kathmandu"}
    },
    {
        6, 0,  1,
            {"Dhaka"}
    },
    {
        6, 30, 1,
            {"Yangon"}
    },
    {
        7, 0,  3,
            {
            "Bangkok",
            "Hanoi",
            "Jakarta"
            }
    },
    {
        8, 0,  8,
            {
            "Kuala Lumpur",
            "Singapore",
            "Hong Kong",
            "Perth",
            "Beijing",
            "Manila",
            "Shanghai",
            "Taipei"
            }
    },
    {
        9, 0,  2,
            {
            "Seoul",
            "Tokyo"
            }
    },
    {
        9, 30, 2,
            {
            "Darwin",
            "Adelaide"
            }
    },
    {
        10,    0,  5,
            {
            "Melbourne",
            "Canberra",
            "Sydney",
            "Brisbane",
            "Vladivostok"
            }
    },
    {
        12,    0,  6,
            {
            "Auckland",
            "Suva",
            "Kamchatka",
            "Anadyr",
            "Kiritimati"
            }
    },
    {
        12,    45, 1,
            {"Chatham Island"}
    },
};
#endif

#define TIME_ZONE_CNT (sizeof(time_zone_citys)/sizeof(time_zone_citys[0]))


struct time_zone_name *get_time_zone_name(INT32 hoffset, INT32 moffset)
{
    UINT32 i = 0;
    struct time_zone_name *time_zone = NULL;
    struct time_zone_name *ret = NULL;

    for(i=0;i<TIME_ZONE_CNT;i++)
    {
        time_zone = &time_zone_citys[i];
        if((time_zone->hoffset == hoffset)&&( time_zone->moffset == moffset))
        {
            break;
        }
    }

    if(i == TIME_ZONE_CNT)
    {
        ret = NULL;
    }
    else
    {
        ret = time_zone;
    }
    return ret;
}

