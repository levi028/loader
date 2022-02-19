/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: version_app.c
*
* Description:
*     the file is for showing ali application version string
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include "version.h"

static char app_ver_banner[] = "Application version " APP_VER "@" APPLABEL "by" COMPILE_BY "\n";
static char sdk_ver_banner[] = "<< " SDKLABEL " >>" "\n";

char *get_app_ver(void)
{
    return app_ver_banner;
}

char *get_sdk_ver(void)
{
    return sdk_ver_banner;
}
