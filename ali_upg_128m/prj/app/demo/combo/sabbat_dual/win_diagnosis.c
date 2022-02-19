/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_diagnosis.c
*
*    Description:   The root menu of Diagnosis Application
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include "win_diagnosis.h"

#ifdef _CAS9_CA_ENABLE_
static char *diag_str[] =
{
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " ",
    " "
};
#else
static char *diag_str[] =
{
    "Diagnosis I2C",
    "I2CTypeId:0x:",
    "Address: 0x",
    "WriteData 0x:",
    "WLength:",
    "ReadData 0x:",
    "RLength:",
    "Write/Read",
    "Exit",
    "Diagnosis App",
    "Register W&R",
    "I2C Bus W&R"
};
#endif

UINT16 diag_unistr[DIAG_STR_CNT][DIAG_STR_LENGTH]={{0,},};


static PRESULT diag_register_callback(POBJECT_HEAD obj, \
                      VEVENT event, UINT32 param1, UINT32 param2);
static PRESULT diag_i2c_callback(POBJECT_HEAD obj, \
                 VEVENT event, UINT32 param1, UINT32 param2);
static PRESULT diag_con_callback(POBJECT_HEAD obj, \
                 VEVENT event, UINT32 param1, UINT32 param2);


#define BTN_DIAG_CENTER         174
#define BTN_ROOT_DIAG_WIDTH     TXT_DIAG_LBL_WIDTH*2
#define BTN_DIAG_LEFT_EX        (WIN_DIAG_LEFT +            \
                 (WIN_DIAG_WIDTH - BTN_ROOT_DIAG_WIDTH -20 )/2)



/***************************/
/*THE TXT LABELS IN THE PAGE*/
/***************************/

/*The Title "Diagnosis I2C" */
DEF_TEXTFIELD(txt_diag_title, &win_diag,\
          &btn_diag_register, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
          0, 0, 0, 0, 0, \
          TXT_DIAG_LEFT, TXT_DIAG_TOP+TXT_DIAG_VGAP, \
          TXT_DIAG_TITLE_WIDTH, TXT_DIAG_HEIGHT, \
          WSTL_TEXT_04_8BIT, DIAG_WSTL_NO_SHOW, \
          DIAG_WSTL_NO_SHOW, DIAG_WSTL_NO_SHOW, \
          NULL, NULL, \
          C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, \
          0, diag_unistr[STR_DIAG_TITLE_ID]);




/*************************/
/*THE BOTTONS IN THE PAGE*/
/*************************/

/*Two Botton  "Register" */
DEF_TEXTFIELD(btn_diag_register, &win_diag, \
          &btn_diag_i2c, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
          11, 13, 12, 13, 12, \
          BTN_DIAG_LEFT_EX, TXT_DIAG_TOP+TXT_DIAG_VGAP*3,\
          BTN_ROOT_DIAG_WIDTH, TXT_DIAG_HEIGHT, \
          WSTL_DIAG_BUTTON_01, WSTL_DIAG_BUTTON_02, \
          WSTL_DIAG_BUTTON_03, DIAG_WSTL_NO_SHOW, \
          diag_btn_keymap, diag_register_callback, \
          C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, \
          0, diag_unistr[STR_BTN_REGISTER_ID]);

/*Two Botton  "I2C" */
DEF_TEXTFIELD(btn_diag_i2c, &win_diag, &btn_diag_exit, \
          C_ATTR_ACTIVE, C_FONT_DEFAULT, \
          12, 11, 13, 11, 13, \
          BTN_DIAG_LEFT_EX, TXT_DIAG_TOP+TXT_DIAG_VGAP*5, \
          BTN_ROOT_DIAG_WIDTH, TXT_DIAG_HEIGHT, \
          WSTL_DIAG_BUTTON_01, WSTL_DIAG_BUTTON_02, \
          WSTL_DIAG_BUTTON_03, DIAG_WSTL_NO_SHOW, \
          diag_btn_keymap, diag_i2c_callback, \
          C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, \
          0, diag_unistr[STR_BTN_I2C_ID]);

/*Two Botton  "Exit" */
DEF_TEXTFIELD(btn_diag_exit, &win_diag, NULL, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
          13, 12, 11, 12, 11, \
          BTN_DIAG_LEFT_EX, TXT_DIAG_TOP+TXT_DIAG_VGAP*7 ,\
          BTN_ROOT_DIAG_WIDTH, TXT_DIAG_HEIGHT, \
          WSTL_DIAG_BUTTON_01, WSTL_DIAG_BUTTON_02, \
          DIAG_WSTL_NO_SHOW, DIAG_WSTL_NO_SHOW, \
          diag_btn_keymap, diag_exit_callback, \
          C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, \
          0, diag_unistr[STR_BTN_EXIT_ID]);


/*******************/
/*The Frame Container*/
/*******************/
DEF_CONTAINER(win_diag, NULL, NULL, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
          0, 0, 0, 0, 0, \
          WIN_DIAG_LEFT, WIN_DIAG_TOP, WIN_DIAG_WIDTH, WIN_DIAG_HEIGHT, \
          WSTL_DIAG_BG_02, WSTL_DIAG_BG_02, DIAG_WSTL_NO_SHOW, DIAG_WSTL_NO_SHOW, \
          diag_con_keymap, diag_con_callback, \
          &txt_diag_title, 11, 0);


/* End of Objects Defination */


VACTION diag_btn_keymap(POBJECT_HEAD obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch (key)
    {
    case V_KEY_ENTER:
        act = VACT_ENTER;
        break;
    default:
        break;
    }

    return act;
}

VACTION diag_edf_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act=0;


    switch(key)
    {
    case V_KEY_0:   case V_KEY_1:   case V_KEY_2:   case V_KEY_3:
    case V_KEY_4:   case V_KEY_5:   case V_KEY_6:   case V_KEY_7:
    case V_KEY_8:   case V_KEY_9:
        act = key - V_KEY_0 + VACT_NUM_0;
        break;
    case V_KEY_LEFT:
        act = VACT_EDIT_LEFT;
        break;
    case V_KEY_RIGHT:
        act = VACT_EDIT_RIGHT;
        break;
    case V_KEY_RED:
        act = VACT_DEL;
        break;

    default:
        act = VACT_PASS;
        break;
    }

    return act;
}


PRESULT diag_hex_edf_callback(POBJECT_HEAD pobj, \
                  VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    //UINT32  set_flag=1;
    //mobile_input_type_t mobile_input_type;

    //MEMSET(&mobile_input_type,0,sizeof(mobile_input_type_t));
    //mobile_input_set_caps(set_flag);
    mobile_input_type.type = MOBILE_INPUT_HEX;

    switch(event)
    {
    case EVN_KEY_GOT:
        ret = mobile_input_proc((EDIT_FIELD*)pobj,\
                    (VACTION)(param1>>16), param1 & 0xFFFF, param2);
        break;
    default:
        break;
    }

    return ret;
}
static PRESULT diag_register_callback(POBJECT_HEAD obj, \
                      VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8   unact=0;
    PRESULT obj_ret=PROC_PASS;

	if(PROC_PASS == obj_ret)
	{
		;
	}
	switch (event)
    {
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
        if(VACT_ENTER == unact)
        {
            //submenu_save_focus_id();
            obj_ret=osd_obj_open((POBJECT_HEAD)&diag_register,\
                        MENU_OPEN_TYPE_MENU);
            menu_stack_push((POBJECT_HEAD)&diag_register);
            //OSD_DrawObject((POBJECT_HEAD)&diag_register, C_UPDATE_ALL);

        }
        break;
    default:
        break;
    }

    return ret;
}

static PRESULT diag_i2c_callback(POBJECT_HEAD obj, \
                 VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8   unact=0;
    PRESULT obj_ret=PROC_PASS;
	
	if(PROC_PASS == obj_ret)
	{
		;
	}
    switch (event)
    {
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
        if(VACT_ENTER == unact)
        {
            //submenu_save_focus_id();
            obj_ret=osd_obj_open((POBJECT_HEAD)&diag_i2c, MENU_OPEN_TYPE_MENU+1);
            menu_stack_push((POBJECT_HEAD)&diag_i2c);

        }
        break;
    default:
        break;
    }

    return ret;
}

PRESULT diag_exit_callback(POBJECT_HEAD obj, \
               VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8   unact=0;

    switch (event)
    {
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
        if(VACT_ENTER == unact)
        {
            ret = PROC_LEAVE;
        }
        break;
    default:
        break;
    }

    return ret;

}

VACTION diag_con_keymap(POBJECT_HEAD obj, UINT32 key)
{
    VACTION act=0;

        act = osd_container_key_map(obj, key);

    return act;
}


static PRESULT diag_con_callback(POBJECT_HEAD obj, \
                 VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8 i=0;
    UINT32 str_ret=0;

	if(0 == str_ret)
	{
		;
	}
	switch (event)
    {
    case EVN_PRE_OPEN:
        for(i=0;i<DIAG_STR_CNT;++i)
        {
            str_ret=com_asc_str2uni((UINT8 *)diag_str[i], diag_unistr[i]);
        }
        break;

    case EVN_POST_CLOSE:
        break;
    default:
        break;
    }
    return ret;
}

/****************************************************************************
 *
 *  Function name: ComUniStr2Hex
 *
 *  Description:
    Convert the Hex string (Unicode String)
    into a hex number(UINT32).
    *
    *  Parameters: string : The Unicode string buffer.
    *
    *  Return:    The hex number (UINT32).
    *
    *
    ****************************************************************************/
UINT32 com_uni_str2hex(const UINT16 *string)
{
    UINT8 i=0;
    UINT8 len=0;
    UINT8 c=0;
    UINT32 val=0;

    val = 0;
    len = com_uni_str_len(string);

    for(i=0;i<len;i++)
    {
#if(SYS_CPU_ENDIAN==ENDIAN_LITTLE)
        c = (UINT8)(string[i] >> 8);
#else
        c = (UINT8)(string[i]);
#endif
        if((c>='0' )&& (c<='9'))
        {
            val = val*16 + c - '0';
        }
        if((c>='a' )&&( c<='f'))
        {
            val = val*16 + 10 + c - 'a';
        }
        if((c>='A') &&( c<='F'))
        {
            val = val*16 + 10 + c - 'A';
        }
    }

    return val;
}

/****************************************************************************
 *
 *  Function name: ComHexArr2UniStr
 *
 *  Description: Convert the Integer array (UINT8[]) into hex string.
 *
 *  Parameters: pString : The Unicode string buffer.
 *                     pData : The integer array (UINT8[]).
 *                     len: The array length.
 *
 *  Return:    The string length
 *
 ****************************************************************************/
UINT32 com_hex_arr2uni_str(UINT16 *pstring, const UINT8 *pdata, UINT16 len)
{
    char str[128]={0};
    UINT8 str_len = 128;

    UINT8 i=0;

    for(i=0;i<len;++i)
    {
        //sprintf(str+i*3,"%02X ",pdata[i]);
        snprintf(str + i * 3, str_len - i * 3, "%02X ",pdata[i]);
    }

    str[i*3 + 1] = '\0';

    return com_asc_str2uni((UINT8*)str,pstring);
}
/****************************************************************************
 *
 *  Function name: ComUniStr2HexArr
 *
 *  Description:
    Convert the Hex string (Unicode String)
    into integer array(UINT8[]).
    *
    *  Parameters: pData : The head pointer of the UINT8 array.
    *                     string : The Unicode string buffer.
    *                     length: The array length.
    *  Return:    void.
    *  Note: If the hex string is longer than the array,
    the additional part will be ignored.
    *           If the hex string is shorter,
             0 will be complemented in front of the hex number.
         *
         ****************************************************************************/

void com_uni_str2hex_arr(UINT8 *pdata, const UINT16 *string, UINT8 length)
{
    UINT8 i=0;
    UINT8 j=0;
    UINT8 strlen=0;
    UINT8 len=0;
    UINT8 ch=0;
    UINT8 cl=0;
    UINT32 val=0;
    UINT8 temp=0;

    strlen = com_uni_str_len(string);
    len = (UINT8)( strlen/2 );
    if(length<=len)
    {
        len = length;
        for(i=0;i<len*2;i+=2)
        {
#if(SYS_CPU_ENDIAN==ENDIAN_LITTLE)
            ch = (UINT8)(string[i] >> 8);
            cl = (UINT8)(string[i+1] >> 8);
#else
            ch = (UINT8)(string[i]);
            cl = (UINT8)(string[i+1]);
#endif
            if((ch>='0' )&&( ch<='9'))
            {
                val =  ch - '0';
            }
            else if((ch>='a' )&& (ch<='f'))
            {
                val = 10 + ch - 'a';
            }
            else if((ch>='A' )&&( ch<='F'))
            {
                val = 10 + ch - 'A';
            }
            if((cl>='0' )&& (cl<='9'))
            {
                val =  val*16 + cl - '0';
            }
            else if((cl>='a') &&( cl<='f'))
            {
                val = val*16 + 10 + cl - 'a';
            }
            else if((cl>='A' )&&( cl<='F'))
            {
                val = val*16 + 10 + cl - 'A';
            }
            pdata[i/2] = val;
        }
    }
    else
    {
        len = (UINT8)( (strlen+1)/2 );
        i = 0;
        for(j=0;j<length-len;++j)
        {
            pdata[j]=0x00;
        }
        temp=strlen%2;
        if( NUM_ONE == temp )
        {
#if(SYS_CPU_ENDIAN==ENDIAN_LITTLE)
            cl = (UINT8)(string[0] >> 8);
#else
            cl = (UINT8)(string[0]);
#endif

            if((cl>='0' )&&( cl<='9'))
            {
                pdata[j] = cl - '0';
            }
            else if((cl>='a' )&&( cl<='f'))
            {
                pdata[j] = 10 + cl - 'a';
            }
            else if((cl>='A' )&& (cl<='F'))
            {
                pdata[j] = 10 + cl - 'A';
            }

            i=1;
        }
        
        //for(;i<strlen;i+=2)
        while (i < strlen)
        {
#if(SYS_CPU_ENDIAN==ENDIAN_LITTLE)
            ch = (UINT8)(string[i] >> 8);
            cl = (UINT8)(string[i+1] >> 8);
#else
            ch = (UINT8)(string[i]);
            cl = (UINT8)(string[i+1]);
#endif
            if((ch>='0' )&& (ch<='9'))
            {
                val =  ch - '0';
            }
            else if((ch>='a' )&&( ch<='f'))
            {
                val = 10 + ch - 'a';
            }
            else if((ch>='A' )&& (ch<='F'))
            {
                val = 10 + ch - 'A';
            }

            if((cl>='0') && (cl<='9'))
            {
                val =  val*16 + cl - '0';
            }
            else if((cl>='a' )&&( cl<='f'))
            {
                val = val*16 + 10 + cl - 'a';
            }
            else if((cl>='A' )&& (cl<='F'))
            {
                val = val*16 + 10 + cl - 'A';
            }
            pdata[(i+1)/2+j] = val;

            i += 2;
        }
    }
}

/****************************************************************************
 *
 *  Function name: ComHex2UniStr
 *
 *  Description: Convert the Integer into hex string (less than 8 Characters).
 *
 *  Parameters: pBuffer : The Unicode string buffer.
 *                     num : The integer(UINT32).
 *                     len: The string length.
 *
 *  Return:    The string length
 *
 ****************************************************************************/
UINT32 com_hex2uni_str(UINT16 *pbuffer, UINT32 num, UINT32 len)
{
    char str[20]={0};
    UINT32 str_len=0;

    sprintf(str,"%02lX%06lX",(num&0xFF000000)>>24,num&0x00FFFFFF);
    //snprintf(str, 20, "%02lu%06lu",(num&0xFF000000)>>24,num&0x00FFFFFF);
    if(len >0)
    {
        str_len = STRLEN(str);

        if( len< str_len)
        {
            str[len] = '\0';
        }
        else if(len > str_len )
        {
            MEMSET(str,'0',len-str_len);
            sprintf(&str[len-str_len],"%02lX%06lX",(num&0xFF000000)>>24,num&0x00FFFFFF);
            //snprintf(&str[len-str_len], len-str_len, "%02lu%06lu",(num&0xFF000000)>>24,num&0x00FFFFFF);
        }
    }

    return com_asc_str2uni((UINT8*)str,pbuffer);
}


