  /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: media_player.c
*
*    Description: This file describes media player operations.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <types.h>
#include <retcode.h>
#include <sys_config.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/sci/sci.h>
#include <hld/hld_dev.h>
#include <osal/osal_timer.h>

#include <api/libmp/pe.h>
#include <api/libfs2/dirent.h>
#include <api/libfs2/errno.h>
#include <api/libfs2/fcntl.h>
#include <api/libfs2/stat.h>
#include <api/libfs2/statvfs.h>
#include <api/libfs2/stdio.h>
#include <api/libfs2/unistd.h>

#include <api/libmp/media_player_api.h>
#include "media_player.h"
#include "play_list.h"
#include "utils.h"

#define MIN_DEVICE_ARRAY_SIZE       30
/**********************Global  Variables***********************************/
static unsigned char first_init = 1;

static BOOL m_release_flag = TRUE;


/*****************************************************************************
 * Function: media_player_set_release
 * Description: 
 *	Set if PE can be really released when calling media_player_release(). 
 *  In Ali applied scene, media_player_init() is called when U-disk is plugged  in, 
 *  media_player_release() is called when U-disk is pulled out, but some other 
 *  applications(network player, etc) may not want to really release resource of 
 *  media player when U-disk is pulled out, so it should call  media_player_set_release(TRUE) 
 *  when enter network player, and call media_player_set_release(FALSE) when exit network player.
 *
 * Input: 
 *      Para 1: BOOL flag, TRUE: media_player_release() should really release media player,
                           FALSE: media_player_release() should NOT release media player.
 * Output: 
 *      None
 * 
 * Returns: void
*****************************************************************************/
void media_player_set_release(BOOL flag)
{
    m_release_flag = flag;
}

/*****************************************************************************
 * Function: media_player_get_release
 * Description: 
 *	Get flag that if the PE can be release when call media_player_release().
 *
 * Input: 
 *      None
 * Output: 
 *      None
 * 
 * Returns: BOOL
*****************************************************************************/
BOOL media_player_get_release(void)
{
    return m_release_flag;
}

/////////////////////////////////////////////////////////////////////////////
// media_player_init
//
// Description:
//
// Arguments:
//
// Return Value://
/////////////////////////////////////////////////////////////////////////////
RET_CODE media_player_init(pe_config *pe_config)
{
    int                            ret = -1;

    MP_DEBUG("====>>media_player_init()\n");

    if (first_init)
    {
        if (!pe_config)
        {
            MP_DEBUG("Invalid parameters!\n");
            MP_DEBUG("<<====media_player_init()\n");
            return RET_FAILURE;
        }

        ret  = pe_init(&pe_config->music, &pe_config->image, &pe_config->video);
        if (-1 == ret)
        {
            MP_DEBUG("Pe init failed!\n");
            MP_DEBUG("<<====media_player_init()\n");
            return RET_FAILURE;
        }

        first_init = 0;
    }
    MP_DEBUG("<<====media_player_init()\n");

    return RET_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////
// media_player_release
//
// Description:
//
// Arguments:
//
// Return Value:
//
/////////////////////////////////////////////////////////////////////////////
#if 1
void media_player_release(void)
{
	MP_DEBUG("====>>media_player_release()\n");
    if (first_init)
    {
        return;
    }
	if (!media_player_get_release())
	{
		MP_DEBUG("<<====media_player_release()\n");
		return;
	}
	pe_cleanup();
	first_init = 1;
	MP_DEBUG("<<====media_player_release()\n");
}
#else
void media_player_release()
{
    int                            device_number = 0;

    MP_DEBUG("====>>media_player_release()\n");

    if (first_init)
    {
        return;
    }
    device_number = get_stroage_device_number(STORAGE_TYPE_ALL);
	if ((device_number < 0) || media_player_is_dlna())
    {
        MP_DEBUG("<<====media_player_release()\n");
        return ;
    }

	if (device_number == 0)
    {
        pe_cleanup();
        first_init = 1;
        MP_DEBUG("clean up PE!\n");
    }

    MP_DEBUG("<<====media_player_release()\n");

}
#endif
/////////////////////////////////////////////////////////////////////////////
// get_stroage_device_number
//
// Description:
//
// Arguments:
//
// Return Value:
//
/////////////////////////////////////////////////////////////////////////////
int get_stroage_device_number(storage_device_type devicetype)
{
    int                                dirfd = -1;
    struct dirent                    *entry = NULL;
    char                            buff[sizeof(struct dirent) + MAX_FILE_NAME_SIZE] = {0};
    int                                device_number = 0;
    int                                usb_device = 0;
    int sd_device = 0;
    int hd_device = 0;
    int sata_device = 0;
    int dlna_device_number = 0;
    char                                i = 0;
    char                             usb_device_number = 0;
    char sd_device_number = 0;
    char hd_device_number = 0;
    char sata_device_number = 0;
    __MAYBE_UNUSED__ e_device_type type = 0;


    MP_DEBUG("====>>get_stroagedevice_number\n");

    device_number = 0;
    usb_device_number = 0;
    sd_device_number = 0;
    hd_device_number = 0;
    dlna_device_number = 0;
    sata_device_number = 0;

    usb_device =0;
    sd_device = 0;
    hd_device = 0;
    sata_device = 0;
    entry = (struct dirent *)buff;
    if ((dirfd = fs_opendir(FS_ROOT_DIR)) < 0)
    {
        MP_DEBUG( "dir: error opening: %s\n", FS_ROOT_DIR);
        MP_DEBUG("<<====get_stroagedevice_number\n");
        return -1;
    }

    while (1)
    {
        if (fs_readdir(dirfd, entry) <= 0)
        {
            break;
        }

        if (S_ISDIR(entry->d_type)) //dir
        {
			if (strncmp(entry->d_name, "ud", 2) == 0)
			{
				for (i='a'; i<='z'; ++i)
            {
					if ((*(entry->d_name+2)) == i)
					{
                    usb_device |= (0x1<<(i-0x61));
                    break;
					}
				}
			}
			else if (strncmp(entry->d_name, "sd", 2) == 0)
			{
				for (i='a'; i<='z'; ++i)
				{
					if ((*(entry->d_name+2)) == i)
					{
                    sd_device |= (0x1<<(i-0x61));
                    break;
					}
				}
			}
			else if (strncmp(entry->d_name, "hd", 2) == 0)
			{
				for (i='a'; i<='z'; ++i)
				{
					if ((*(entry->d_name+2)) == i)
					{
                    hd_device |= (0x1<<(i-0x61));
                    break;
					}
				}
			}
			else if (strncmp(entry->d_name, "dlna", 2) == 0)
			{
                    dlna_device_number = 1;
			}
			else if (strncmp(entry->d_name, "sh", 2) == 0)
			{
				for (i='a'; i<='z'; ++i)
				{
					if ((*(entry->d_name+2)) == i)
					{
                    sata_device |= (0x1<<(i-0x61));
                    break;
					}
				}
             }
        }
    }

    fs_closedir(dirfd);

    for (i=0; i<32; ++i)
    {
        if ( usb_device & (0x1 << i))
        {
            usb_device_number++;
        }

        if ( sd_device & (0x1 << i))
        {
            sd_device_number++;
        }

        if ( hd_device & (0x1 << i))
        {
            hd_device_number++;
        }

        if ( sata_device & (0x1 << i))
        {
            sata_device_number++;
        }
    }

    switch(devicetype)
    {
        case STORAGE_TYPE_ALL:
        device_number = usb_device_number + sd_device_number + hd_device_number + sata_device_number
          + dlna_device_number;
            break;
        case STORAGE_TYPE_USB:
        device_number = usb_device_number;
            break;
        case STORAGE_TYPE_SD:
        device_number = sd_device_number;
            break;
        case STORAGE_TYPE_HDD:
        device_number = hd_device_number;
            break;
        case STORAGE_TYPE_SATA:
        device_number = sata_device_number;
              break;
        default:
            break;
    }
    
    MP_DEBUG("device type is %d, deice_number is %d\n", devicetype, device_number);

    MP_DEBUG("<<====get_stroagedevice_number\n");

    return device_number;

}
/////////////////////////////////////////////////////////////////////////////
// get_current_stroage_device
//
// Description:
//
// Arguments:
//
// Return Value:
//
/////////////////////////////////////////////////////////////////////////////
RET_CODE get_current_stroage_device(storage_device_type devicetype, char *device_array,
    unsigned char *device_array_size)
{
    int                                dirfd = -1;
    struct dirent                        *entry = NULL;
    char                                buff[sizeof(struct dirent) + MAX_FILE_NAME_SIZE] = {0};
    int                                usb_device = 0;
    int sd_device = 0;
    int hd_device = 0;
    int sata_device = 0;
    char                                i = 0;
    __MAYBE_UNUSED__ e_device_type type = 0;

    MP_DEBUG("====>>get_current_stroage_device\n");
    //MEMSET(buff, 0, sizeof(buff));
    if ((!device_array) || (!device_array_size) || (*device_array_size < MIN_DEVICE_ARRAY_SIZE))
    {
        MP_DEBUG("Invalid Parameters!\n");
        MP_DEBUG("<<====get_current_stroage_device\n");
        return RET_FAILURE;
    }

    usb_device =0;
    sd_device = 0;
    hd_device = 0;
    sata_device = 0;

    MEMSET(device_array, 0, *device_array_size);

    entry = (struct dirent *)buff;

    if ((dirfd = fs_opendir(FS_ROOT_DIR)) < 0)
    {
        MP_DEBUG( "dir: error opening: %s\n", FS_ROOT_DIR);
        MP_DEBUG("<<====get_current_stroage_device\n");
        return RET_FAILURE;
    }

    while (1)
    {
        if (fs_readdir(dirfd, entry) <= 0)
        {
            break;
        }

        if (S_ISDIR(entry->d_type)) //dir
        {
			if (strncmp(entry->d_name, "ud", 2) == 0)
			{
				 i = *(entry->d_name + 2);
				 if ((i < 'a') || (i > 'z'))
            {
					MP_DEBUG("impossiable error!\n");
					continue;
				 }
                    if (!(usb_device & (0x01 << (i-0x61))))
                    {
                        if ((STORAGE_TYPE_ALL == devicetype) || (STORAGE_TYPE_USB == devicetype))
                        {
                            strncat(device_array, entry->d_name, 3);
                            *device_array_size += 3;
                        }

                        usb_device |= (0x1<<(i-0x61));
                    }
			}
			else if (strncmp(entry->d_name, "sd", 2) == 0)
			{
				 i = *(entry->d_name + 2);
				 if ((i < 'a') || (i > 'z'))
				 {
					MP_DEBUG("impossiable error!\n");
					continue;
				 }

                     if (!(sd_device & (0x01 << (i-0x61))))
                     {
                        if ((STORAGE_TYPE_ALL == devicetype) || (STORAGE_TYPE_SD == devicetype))
                        {
                            strncat(device_array, entry->d_name, 3);
                            *device_array_size += 3;
                        }

                        sd_device |= (0x1<<(i-0x61));
                     }
			}
			else if (strncmp(entry->d_name, "hd", 2) == 0)
			{
				 i = *(entry->d_name + 2);
				 if ((i < 'a') || (i > 'z'))
				 {
					MP_DEBUG("impossiable error!\n");
					continue;
				 }

                     if (!(hd_device & (0x01 << (i-0x61))))
                     {
                        if ((STORAGE_TYPE_ALL == devicetype) || (STORAGE_TYPE_HDD == devicetype))
                        {
                            strncat(device_array, entry->d_name, 3);
                            *device_array_size += 3;
                        }

                        hd_device |= (0x1<<(i-0x61));
                     }
			}
			else if (strncmp(entry->d_name, "sh", 2) == 0)
			{
				 i = *(entry->d_name + 2);
				 if ((i < 'a') || (i > 'z'))
				 {
					MP_DEBUG("impossiable error!\n");
					continue;
				 }

                     if (!(sata_device & (0x01 << (i-0x61))))
                     {
                        if ((STORAGE_TYPE_ALL == devicetype) || (STORAGE_TYPE_HDD == devicetype))
                        {
                            strncat(device_array, entry->d_name, 3);
                            *device_array_size += 3;
                        }

                        sata_device |= (0x1<<(i-0x61));
                     }
				
			}		}

    }

    fs_closedir(dirfd);

    MP_DEBUG("<<====get_current_stroage_device\n");
    return RET_SUCCESS;
}
/////////////////////////////////////////////////////////////////////////////
// get_current_stroage_device1
//
// Description:
//
// Arguments:
//
// Return Value:
//
/////////////////////////////////////////////////////////////////////////////
RET_CODE get_current_stroage_device1(storage_device_type devicetype, char *device_array,
    unsigned char *device_array_size)
{
    int                                dirfd = -1;
    struct dirent                    *entry = NULL;
    char                            buff[sizeof(struct dirent) + MAX_FILE_NAME_SIZE] = {0};
    int                                usb_device = 0;
    int sd_device = 0;
    int hd_device = 0;
    int sata_device = 0;
    int dlna_device = 0;
    char                                i = 0;
    char                                temp[4] = {0};
    __MAYBE_UNUSED__ e_device_type type = 0;

    MP_DEBUG("====>>get_current_stroage_device\n");

	if ((!device_array) || (!device_array_size) || (*device_array_size < 30))
    {
        MP_DEBUG("Invalid Parameters!\n");
        MP_DEBUG("<<====get_current_stroage_device\n");
        return RET_FAILURE;
    }

    usb_device =0;
    sd_device = 0;
    dlna_device = 0;
    hd_device = 0;
    sata_device = 0;

    MEMSET(device_array, 0, *device_array_size);
    *device_array_size = 0;

    entry = (struct dirent *)buff;

    if ((dirfd = fs_opendir(FS_ROOT_DIR)) < 0)
    {
        MP_DEBUG( "dir: error opening: %s\n", FS_ROOT_DIR);
        MP_DEBUG("<<====get_current_stroage_device\n");
        return RET_FAILURE;
    }

    while (1)
    {
        if (fs_readdir(dirfd, entry) <= 0)
        {
            break;
        }

        if (S_ISDIR(entry->d_type)) //dir
        {
			if (strncmp(entry->d_name, "ud", 2) == 0)
			{
				 i = *(entry->d_name + 2);
				 if ((i < 'a') || (i > 'z'))
                {
					MP_DEBUG("impossiable error!\n");
					continue;
				 }
                    usb_device |= (0x1<<(i-0x61));
			}
			else if (strncmp(entry->d_name, "sd", 2) == 0)
			{
				 i = *(entry->d_name + 2);
				 if ((i < 'a') || (i > 'z'))
				 {
					MP_DEBUG("impossiable error!\n");
					continue;
				 }

                    sd_device |= (0x1<<(i-0x61));
			}
			else if (strncmp(entry->d_name, "dlna", 2) == 0)
			{
				dlna_device = 1;
			}			
			else if (strncmp(entry->d_name, "hd", 2) == 0)
			{
				 i = *(entry->d_name + 2);
				 if ((i < 'a') || (i > 'z'))
				 {
					MP_DEBUG("impossiable error!\n");
					continue;
				 }

                    hd_device |= (0x1<<(i-0x61));
			}
			else if (strncmp(entry->d_name, "sh", 2) == 0)
			{
				 i = *(entry->d_name + 2);
				 if ((i < 'a') || (i > 'z'))
				 {
					MP_DEBUG("impossiable error!\n");
					continue;
				 }

                    sata_device |= (0x1<<(i-0x61));
                }
        }

    }

    fs_closedir(dirfd);

    temp[0] = 'u';
    temp[1] = 'd';
    temp[3] = 0;
    for (i='a'; i<='z'; ++i)
    {
        if ( usb_device & (0x1 << (i - 0x61)))
        {
            if ((STORAGE_TYPE_ALL == devicetype) || (STORAGE_TYPE_USB == devicetype))
            {
                temp[2] = i;
                strncat(device_array, temp, 3);
                *device_array_size += 3;
            }
        }

    }

    temp[0] = 's';
    temp[1] = 'd';
    temp[3] = 0;
    for (i='a'; i<='z'; ++i)
    {
        if ( sd_device & (0x1 << (i - 0x61)))
        {
            if ((STORAGE_TYPE_ALL == devicetype) || (STORAGE_TYPE_SD == devicetype))
            {
                temp[2] = i;
                strncat(device_array, temp, 3);
                *device_array_size += 3;
            }
        }

    }

    temp[0] = 'h';
    temp[1] = 'd';
    temp[3] = 0;
    for (i='a'; i<='z'; ++i)
    {
        if ( hd_device & (0x1 << (i - 0x61)))
        {
            if ((STORAGE_TYPE_ALL == devicetype) || (STORAGE_TYPE_HDD == devicetype))
            {
                temp[2] = i;
                strncat(device_array, temp, 3);
                *device_array_size += 3;
            }
        }
    }

    temp[0] = 's';
    temp[1] = 'h';
    temp[3] = 0;
    for (i='a'; i<='z'; ++i)
    {
        if ( sata_device & (0x1 << (i - 0x61)))
        {
            if ((STORAGE_TYPE_ALL == devicetype) || (STORAGE_TYPE_HDD == devicetype))
            {
                temp[2] = i;
                strncat(device_array, temp, 3);
                *device_array_size += 3;
            }
        }
    }

    if(dlna_device)
    {
        strncat(device_array, "dlna", 3);
        *device_array_size += 3;
    }

    MP_DEBUG("<<====get_current_stroage_device\n");
    return RET_SUCCESS;
}
/////////////////////////////////////////////////////////////////////////////
// get_stroage_device_partition_number
//
// Description:
//
// Arguments:
//
// Return Value:
//
/////////////////////////////////////////////////////////////////////////////
RET_CODE get_stroage_device_partition_number(char *device_name, unsigned char *partition_number)
{
    int                                dirfd = -1;
    struct dirent                        *entry = NULL;
    char                                buff[sizeof(struct dirent) + MAX_FILE_NAME_SIZE] = {0};

    MP_DEBUG("====>>get_stroage_device_partition_number\n");

    if ((!device_name) || (!partition_number))
    {
        MP_DEBUG("Invalid Parameters!\n");
        MP_DEBUG("<<====get_stroage_device_partition_number\n");
        return RET_FAILURE;
    }

    *partition_number = 0;

    entry = (struct dirent *)buff;

    if ((dirfd = fs_opendir(FS_ROOT_DIR)) < 0)
    {
        MP_DEBUG( "dir: error opening: %s\n", FS_ROOT_DIR);
        MP_DEBUG("<<====get_stroage_device_partition_number\n");
        return RET_FAILURE;
    }

    while(1)
    {
        if (fs_readdir(dirfd, entry) <= 0)
        {
            break;
        }

        if (S_ISDIR(entry->d_type)) //dir
        {
            if (0 == strncmp(entry->d_name, device_name, 3))
            {
                *partition_number = *partition_number + 1;
            }
        }

    }

    fs_closedir(dirfd);

    MP_DEBUG("<<====get_stroage_device_partition_number\n");
    return RET_SUCCESS;
}
