/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_diag_register.c
*
*    Description:  register debug tools
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include "win_diagnosis.h"

#define TXT_DIAG_REG_LEFT   TXT_DIAG_LEFT //30
#define TXT_DIAG_REG_TOP    (WIN_DIAG_TOP + 460)
#define TXT_DIAG_REG_WIDTH  140
#define TXT_DIAG_REG_HEIGHT 40//28
#define TXT_DIAG_REG_GAP    30

#define TXT_DRAW_LEFT  (WIN_DIAG_LEFT+20)
#define TXT_DRAW_TOP   (WIN_DIAG_TOP+10)

#define DIAG_MAX_COLUM   4
#define DIAG_MAX_ROW  10
#define DIAG_BASE  4
/*
 *  Local functions & variables define
 */

static PRESULT register_read_callback(POBJECT_HEAD obj, \
               VEVENT event, UINT32 param1, UINT32 param2);
static PRESULT register_write_callback(POBJECT_HEAD obj, \
               VEVENT event, UINT32 param1, UINT32 param2);
static PRESULT register_con_callback(POBJECT_HEAD obj, \
               VEVENT event, UINT32 param1, UINT32 param2);


static void show_memory_data(void);
static void set_memory_data(void);



static UINT16 edit_saddr_str[9]={0};
static UINT16 edit_maddr_str[9]={0};
static UINT16 edit_value_str[9]={0};

#ifdef _CAS9_CA_ENABLE_
static UINT8 *txt_title_reg[]=
{
      (UINT8 *)" ",
      (UINT8 *)" ",
      (UINT8 *)" ",
      (UINT8 *)" ",
      (UINT8 *)" ",
      (UINT8 *)" "
};

#else
static UINT8 *txt_title_reg[]=
{
      (UINT8 *)"StartAddr:",
      (UINT8 *)"Read",
      (UINT8 *)"Exit",
      (UINT8 *)"ModifyAddr:",
      (UINT8 *)"Value:",
      (UINT8 *)"Write"
};
#endif

static UINT16 txt_title_str[6][12]={{0,},};


DEF_TEXTFIELD(txt_title_saddr, &diag_register, \
      &edit_saddr, C_ATTR_ACTIVE,C_FONT_DEFAULT, \
      0, 0, 0, 0, 0, \
      TXT_DIAG_REG_LEFT, TXT_DIAG_REG_TOP,\
      TXT_DIAG_REG_WIDTH, TXT_DIAG_REG_HEIGHT, \
      WSTL_DIAG_TEXT, WSTL_NOSHOW_IDX, \
      WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, \
      NULL, NULL, \
      C_ALIGN_RIGHT| C_ALIGN_VCENTER, 0, 0, \
      0, txt_title_str[0]);


DEF_EDITFIELD(edit_saddr,&diag_register,\
    &txt_title_read,C_ATTR_ACTIVE,C_FONT_DEFAULT, \
    1,0,0,6,2,\
    TXT_DIAG_REG_LEFT+TXT_DIAG_REG_WIDTH,\
     TXT_DIAG_REG_TOP , TXT_DIAG_REG_WIDTH, TXT_DIAG_REG_HEIGHT,\
    WSTL_DIAG_TEXT, WSTL_DIAG_BG_02,\
     WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX,   \
    diag_edf_keymap,diag_hex_edf_callback,  \
    C_ALIGN_CENTER| C_ALIGN_VCENTER, \
    0,0,NORMAL_EDIT_MODE|EDIT_STATUS,\
     "s8",8,CURSOR_NORMAL ,NULL,NULL,edit_saddr_str);

DEF_TEXTFIELD(txt_title_read, &diag_register, \
      &txt_title_exit, C_ATTR_ACTIVE,C_FONT_DEFAULT, \
      2, 0, 0, 1, 3, \
      TXT_DIAG_REG_LEFT+2*(TXT_DIAG_REG_WIDTH), \
      TXT_DIAG_REG_TOP, TXT_DIAG_REG_WIDTH, TXT_DIAG_REG_HEIGHT, \
      WSTL_DIAG_BUTTON_01, WSTL_DIAG_BUTTON_02, \
      WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, \
      diag_btn_keymap, register_read_callback,\
      C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, \
      0, txt_title_str[1]);

DEF_TEXTFIELD(txt_title_exit, &diag_register, \
      &txt_title_maddr, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
      3, 0, 0, 2, 4, \
      TXT_DIAG_REG_LEFT+3*(TXT_DIAG_REG_WIDTH), \
      TXT_DIAG_REG_TOP , TXT_DIAG_REG_WIDTH, TXT_DIAG_REG_HEIGHT, \
      WSTL_DIAG_BUTTON_01, WSTL_DIAG_BUTTON_02, \
      WSTL_NOSHOW_IDX, WSTL_DIAG_BUTTON_01, \
      diag_btn_keymap, diag_exit_callback, \
      C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, \
      0, txt_title_str[2]);

/*****************************************************************/

DEF_TEXTFIELD(txt_title_maddr, &diag_register,\
      &edit_maddr, C_ATTR_ACTIVE,C_FONT_DEFAULT, \
      0, 0, 0, 0, 0, \
      TXT_DIAG_REG_LEFT, TXT_DIAG_REG_TOP+TXT_DIAG_REG_HEIGHT,\
      TXT_DIAG_REG_WIDTH, TXT_DIAG_REG_HEIGHT, \
      WSTL_DIAG_TEXT, WSTL_NOSHOW_IDX, \
      WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, \
      NULL, NULL, \
      C_ALIGN_RIGHT| C_ALIGN_VCENTER, 0, 0, \
      0, txt_title_str[3]);

DEF_EDITFIELD(edit_maddr,&diag_register,\
    &txt_title_value,C_ATTR_ACTIVE,C_FONT_DEFAULT, \
    4,0,0,3,5,\
    TXT_DIAG_REG_LEFT+TXT_DIAG_REG_WIDTH,\
    TXT_DIAG_REG_TOP+TXT_DIAG_REG_HEIGHT , \
    TXT_DIAG_REG_WIDTH, TXT_DIAG_REG_HEIGHT,\
    WSTL_DIAG_TEXT, WSTL_DIAG_BG_02, \
    WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX,   \
    diag_edf_keymap,diag_hex_edf_callback,  \
    C_ALIGN_CENTER| C_ALIGN_VCENTER, 0,0,\
    NORMAL_EDIT_MODE|EDIT_STATUS, "s8",8,\
    CURSOR_NORMAL ,NULL,NULL,edit_maddr_str);

DEF_TEXTFIELD(txt_title_value, &diag_register,\
      &edit_value, C_ATTR_ACTIVE,C_FONT_DEFAULT, \
      0, 0, 0, 0, 0, \
      TXT_DIAG_REG_LEFT+2*TXT_DIAG_REG_WIDTH, \
      TXT_DIAG_REG_TOP+TXT_DIAG_REG_HEIGHT, \
      TXT_DIAG_REG_WIDTH, TXT_DIAG_REG_HEIGHT, \
      WSTL_DIAG_TEXT, WSTL_NOSHOW_IDX, \
      WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, \
      NULL, NULL, \
      C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, \
      0, txt_title_str[4]);

DEF_EDITFIELD(edit_value,&diag_register,\
    &txt_title_write,C_ATTR_ACTIVE,C_FONT_DEFAULT, \
    5,0,0,4,6,\
    TXT_DIAG_REG_LEFT+3*TXT_DIAG_REG_WIDTH, \
    TXT_DIAG_REG_TOP+TXT_DIAG_REG_HEIGHT ,\
     TXT_DIAG_REG_WIDTH, TXT_DIAG_REG_HEIGHT,\
    WSTL_DIAG_TEXT, WSTL_DIAG_BG_02, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX,   \
    diag_edf_keymap,diag_hex_edf_callback,  \
    C_ALIGN_CENTER| C_ALIGN_VCENTER, 0,0,\
    NORMAL_EDIT_MODE|EDIT_STATUS, "s8",8,\
    CURSOR_NORMAL ,NULL,NULL,edit_value_str);

DEF_TEXTFIELD(txt_title_write, &diag_register,NULL,\
      C_ATTR_ACTIVE,C_FONT_DEFAULT, \
      6, 0, 0, 5, 1, \
      TXT_DIAG_REG_LEFT+4*(TXT_DIAG_REG_WIDTH), \
      TXT_DIAG_REG_TOP+TXT_DIAG_REG_HEIGHT, \
      TXT_DIAG_REG_WIDTH, TXT_DIAG_REG_HEIGHT, \
      WSTL_DIAG_BUTTON_01, WSTL_DIAG_BUTTON_02, \
      WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX,  \
      diag_btn_keymap, register_write_callback,\
      C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, \
      0, txt_title_str[5]);

/**********************************************************************/
DEF_CONTAINER(diag_register, NULL, NULL, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
      0, 0, 0, 0, 0, \
      WIN_DIAG_LEFT, WIN_DIAG_TOP, WIN_DIAG_WIDTH, WIN_DIAG_HEIGHT, \
      WSTL_DIAG_BG_02, WSTL_DIAG_BG_02, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, \
      diag_con_keymap, register_con_callback, \
      &txt_title_saddr, 1, 0);


static PRESULT register_read_callback(POBJECT_HEAD obj,\
               VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8   unact=0;

    switch (event)
    {
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
        if (VACT_ENTER == unact)
        {
            show_memory_data();
        }
        break;
     default:
        break;
    }

    return ret;
}





static PRESULT register_write_callback(POBJECT_HEAD obj, \
               VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8   unact=0;

    switch (event)
    {
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
        if (VACT_ENTER == unact)
        {
            set_memory_data();
            show_memory_data();
        }
        break;
    default:
        break;
    }

    return ret;

}



static PRESULT register_con_callback(POBJECT_HEAD obj, \
               VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8 i=0;
    //PRESULT obj_ret = PROC_PASS;
    UINT32 str_ret=0;
	
	if(0 == str_ret)
	{
		;
	}
    switch (event)
    {
    case EVN_PRE_OPEN:
         for(i=0;i<6;i++)
     {
         str_ret=com_asc_str2uni(txt_title_reg[i], txt_title_str[i]);
     }

        osd_set_attr((POBJECT_HEAD)&txt_title_saddr, C_ATTR_ACTIVE);
        break;

    case EVN_POST_CLOSE:
    osd_draw_object((POBJECT_HEAD)&win_diag, \
            C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
        break;
     default:
        break;
    }
    return ret;
}

static void show_memory_data(void)
{

    UINT32 row=0;
    UINT32 col=0;
    UINT32 temp=0;
    UINT32 i=0;
    UINT16  regstr[16];
    char *row_title[4]={"00","04","08","0C"};
    TEXT_FIELD txt_register;
    UINT16 row_title_str[4];
    UINT16 col_title_str[/*maxrow*/10]={0};
    //PRESULT obj_ret=PROC_PASS;
    UINT32 str_ret=0;
    UINT8 *pstartaddr=NULL;
    UINT32  addr_str[/*maxrow*maxcol*/40]={0};
    UINT32  startaddr=0;
    UINT32 tempdst[/*maxrow*maxcol*4*/160]={0};
    UINT32 temp_addr = startaddr & 0xff;
    UINT32 temp_addr2 = 256 - temp_addr;
    UINT32 data_row = temp_addr2 >> 4;

	if(0 == str_ret)
	{
		;
	}
    MEMSET(&txt_register,0,sizeof(TEXT_FIELD));
    startaddr = com_uni_str2hex(edit_saddr_str);
    pstartaddr = (UINT8*)startaddr;

    if(NULL == pstartaddr)
    {
        ASSERT(0);
        return;
    }
    for(i=0;i<DIAG_MAX_ROW*DIAG_MAX_COLUM*DIAG_BASE;i++)
    {
        row = i / DIAG_MAX_COLUM / 4;

        if (row < data_row)
    {
        tempdst[i] =(UINT32) *pstartaddr;
    }
        else
    {
        tempdst[i] =(UINT32) 0;
    }
        pstartaddr++;
    }

    for(i=0;i<DIAG_MAX_ROW*DIAG_MAX_COLUM;i++)
    {
            addr_str[i] = tempdst[4*i]+(tempdst[4*i+1]<<8)\
        +(tempdst[4*i+2]<<16)+(tempdst[4*i+3]<<24);
        }

       for(i=1;i<=DIAG_MAX_COLUM;i++)
       {
            DEF_TEXTFIELD(txt_register, &diag_register,\
                NULL, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
                    0, 0, 0, 0, 0, \
                    TXT_DRAW_LEFT+i*(TXT_DIAG_REG_WIDTH), \
            TXT_DRAW_TOP, TXT_DIAG_REG_WIDTH, TXT_DIAG_REG_HEIGHT, \
                    WSTL_DIAG_TEXT, WSTL_NOSHOW_IDX,\
             WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, \
                     NULL, NULL, \
                    C_ALIGN_CENTER| C_ALIGN_VCENTER, 0, 0, \
                    0,row_title_str);
              osd_set_text_field_content(&txt_register,\
          STRING_ANSI, (UINT32)row_title[i-1]);
              osd_draw_object((POBJECT_HEAD)&txt_register, C_UPDATE_ALL);
       }

    for(i=1;i<=DIAG_MAX_ROW;i++)
       {
            if (i <= data_row)
        {
            temp = startaddr + 16*(i-1);
        }
            else
        {
            temp = 0;
        }
              str_ret=com_hex2uni_str(col_title_str,temp, 8);

            DEF_TEXTFIELD(txt_register, &diag_register,\
                NULL, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
                    0, 0, 0, 0, 0, \
                    TXT_DRAW_LEFT, TXT_DRAW_TOP+i*(TXT_DIAG_REG_HEIGHT),\
            TXT_DIAG_REG_WIDTH, TXT_DIAG_REG_HEIGHT, \
                     WSTL_DIAG_TEXT, WSTL_NOSHOW_IDX, \
             WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, \
                     NULL, NULL, \
                    C_ALIGN_CENTER| C_ALIGN_VCENTER, 0, 0, \
                    0,col_title_str);

            osd_draw_object((POBJECT_HEAD)&txt_register, C_UPDATE_ALL);
       }

    for(row=1;row<=DIAG_MAX_ROW;row++)
    {
        for(col=1;col<=DIAG_MAX_COLUM;col++)
        {
            str_ret=com_hex2uni_str(regstr, addr_str[(row-1)*DIAG_MAX_COLUM+col-1], 8);

            DEF_TEXTFIELD(txt_register, &diag_register, \
                  NULL, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
                  0, 0, 0, 0, 0, \
                  TXT_DRAW_LEFT+col*(TXT_DIAG_REG_WIDTH), \
                  TXT_DRAW_TOP+row*(TXT_DIAG_REG_HEIGHT), \
                  TXT_DIAG_REG_WIDTH, TXT_DIAG_REG_HEIGHT, \
                  WSTL_DIAG_TEXT, WSTL_NOSHOW_IDX, \
                  WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, \
                  NULL, NULL, \
                  C_ALIGN_CENTER| C_ALIGN_VCENTER, 0, 0, \
                  0,regstr);

            osd_draw_object((POBJECT_HEAD)&txt_register, C_UPDATE_ALL);

        }
    }
}

static void set_memory_data(void)
{
    UINT32 *modifyaddr=NULL;
    UINT32 value=0;

    modifyaddr =(UINT32 *)com_uni_str2hex(edit_maddr_str);
    value = com_uni_str2hex(edit_value_str);
    if(modifyaddr != NULL)
    {
        *modifyaddr = value;
    }
}


