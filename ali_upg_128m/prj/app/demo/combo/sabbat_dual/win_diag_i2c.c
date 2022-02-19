/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_diag_i2c.c
*
*    Description: part of the diagnosis application
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include "win_diagnosis.h"

static PRESULT i2c_write_read_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2);
static PRESULT i2c_con_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2);
static PRESULT i2c_num_edf_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

//extern UINT16 diag_unistr[DIAG_STR_CNT][DIAG_STR_LENGTH];

static UINT16 edf_i2c_id_str[9]={0};
static UINT16 edf_i2c_addr_str[9]={0};

static UINT16 edf_i2c_wdata_str[65]={0};
static UINT16 edf_i2c_wlen_str[3]={0};

static UINT16 txt_i2c_rdata_str[65]={0};
static UINT16 edf_i2c_rlen_str[3]={0};


/***************************/
/*THE TXT LABELS IN THE PAGE*/
/***************************/

/*The Title "Diagnosis I2C" */
DEF_TEXTFIELD(txt_i2c_title, &diag_i2c, \
      &txt_i2c_id, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
      0, 0, 0, 0, 0, \
      TXT_DIAG_LEFT, TXT_DIAG_TOP, TXT_DIAG_TITLE_WIDTH, TXT_DIAG_HEIGHT, \
      WSTL_DIAG_TEXT, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, \
      NULL, NULL, \
      C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, \
      0, diag_unistr[STR_I2C_TITLE_ID]);

/*The label " I2C id" */
DEF_TEXTFIELD(txt_i2c_id, &diag_i2c, \
      &edf_i2c_id, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
      0, 0, 0, 0, 0, \
      TXT_DIAG_LEFT, TXT_DIAG_TOP+TXT_DIAG_VGAP,\
      TXT_DIAG_LBL_WIDTH, TXT_DIAG_HEIGHT, \
      WSTL_DIAG_TEXT, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, \
      NULL, NULL, \
      C_ALIGN_RIGHT|C_ALIGN_VCENTER, 0, 0, \
      0, diag_unistr[STR_LBL_I2CID_ID]);

/*The label "Address: 0x" */
DEF_TEXTFIELD(txt_i2c_addr, &diag_i2c, \
      &edf_i2c_addr, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
      0, 0, 0, 0, 0, \
      TXT_DIAG_LEFT+TXT_DIAG_LBL_GAP*2, \
      TXT_DIAG_TOP+TXT_DIAG_VGAP, TXT_DIAG_LBL_WIDTH, TXT_DIAG_HEIGHT, \
      WSTL_DIAG_TEXT, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, \
      NULL, NULL, \
      C_ALIGN_RIGHT|C_ALIGN_VCENTER, 0, 0, \
      0, diag_unistr[STR_LBL_ADDR_ID]);

/*The label "WriteData" */
DEF_TEXTFIELD(txt_i2c_write_data, &diag_i2c,\
      &edf_i2c_wdata, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
      0, 0, 0, 0, 0, \
      TXT_DIAG_LEFT, TXT_DIAG_TOP+TXT_DIAG_VGAP*2, \
      TXT_DIAG_LBL_WIDTH, TXT_DIAG_HEIGHT, \
      WSTL_DIAG_TEXT, WSTL_NOSHOW_IDX,\
      WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, \
      NULL, NULL, \
      C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, \
      0, diag_unistr[STR_LBL_WDATA_ID]);

/*The label "Wlength" */
DEF_TEXTFIELD(txt_i2c_wlength, &diag_i2c, &edf_i2c_wlen,\
      C_ATTR_ACTIVE, C_FONT_DEFAULT, \
      0, 0, 0, 0, 0, \
      LBL_DATA_LEN_LEFT, TXT_DIAG_TOP+TXT_DIAG_VGAP*2,\
      TXT_DIAG_LBL_WIDTH, TXT_DIAG_HEIGHT, \
      WSTL_DIAG_TEXT, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, \
      NULL, NULL, \
      C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, \
      0, diag_unistr[STR_LBL_WLEN_ID]);

/*The label "ReadData" */
DEF_TEXTFIELD(txt_i2c_read_data, &diag_i2c,\
      &txt_i2c_rdata, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
      0, 0, 0, 0, 0, \
      TXT_DIAG_LEFT, TXT_DIAG_TOP+TXT_DIAG_VGAP*3, \
      TXT_DIAG_LBL_WIDTH, TXT_DIAG_HEIGHT, \
      WSTL_DIAG_TEXT, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, \
      NULL, NULL, \
      C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, \
      0, diag_unistr[STR_LBL_RDATA_ID]);

/*The text field for I2C Read Data*/
DEF_TEXTFIELD(txt_i2c_rdata, &diag_i2c, &txt_i2c_rlength,\
    C_ATTR_ACTIVE, C_FONT_DEFAULT, \
    0, 0, 0, 0, 0, \
    TXT_DIAG_LEFT+TXT_DIAG_LBL_GAP, TXT_DIAG_TOP+TXT_DIAG_VGAP*3,\
    EDF_DATA_WIDTH, TXT_DIAG_HEIGHT, \
    WSTL_DIAG_TEXT, WSTL_DIAG_BG_02, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, \
    NULL, NULL,
    C_ALIGN_LEFT|C_ALIGN_VCENTER, 2, 0, \
    0, txt_i2c_rdata_str);

/*The label "Rlength" */
DEF_TEXTFIELD(txt_i2c_rlength, &diag_i2c, \
      &edf_i2c_rlen, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
      0, 0, 0, 0, 0, \
      LBL_DATA_LEN_LEFT, TXT_DIAG_TOP+TXT_DIAG_VGAP*3,\
      TXT_DIAG_LBL_WIDTH, TXT_DIAG_HEIGHT, \
      WSTL_DIAG_TEXT, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, \
      NULL, NULL, \
      C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, \
      0, diag_unistr[STR_LBL_RLEN_ID]);

/***************************/
/*THE EDIT FIELDS IN THE PAGE*/
/***************************/

/*The edit field for I2C ID*/
DEF_EDITFIELD(edf_i2c_id, &diag_i2c,\
    &txt_i2c_addr, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
    21, 21, 21, 27, 22, \
    TXT_DIAG_LEFT+TXT_DIAG_LBL_GAP, \
    TXT_DIAG_TOP+TXT_DIAG_VGAP, TXT_DIAG_LBL_WIDTH, TXT_DIAG_HEIGHT, \
    WSTL_DIAG_TEXT, WSTL_DIAG_BG_02, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, \
    diag_edf_keymap, diag_hex_edf_callback,
    C_ALIGN_CENTER|C_ALIGN_VCENTER, 0, 0, \
    EDIT_STATUS|NORMAL_EDIT_MODE, "s8", \
    8, CURSOR_NORMAL, \
    NULL, NULL, edf_i2c_id_str);

/*The edit field for I2C address*/
DEF_EDITFIELD(edf_i2c_addr, &diag_i2c,\
    &txt_i2c_write_data, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
    22, 22, 22, 21, 23, \
    TXT_DIAG_LEFT+TXT_DIAG_LBL_GAP*3 , \
    TXT_DIAG_TOP+TXT_DIAG_VGAP, \
    EDF_NUM_WIDTH, TXT_DIAG_HEIGHT, \
    WSTL_DIAG_TEXT, WSTL_DIAG_BG_02, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, \
    diag_edf_keymap, diag_hex_edf_callback,
    C_ALIGN_CENTER|C_ALIGN_VCENTER, 0, 0, \
    EDIT_STATUS|NORMAL_EDIT_MODE, "s2", \
    2, CURSOR_NORMAL, \
    NULL, NULL, edf_i2c_addr_str);

/*The edit field for I2C Write Data*/
DEF_EDITFIELD(edf_i2c_wdata, &diag_i2c,\
    &txt_i2c_wlength, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
    23, 23, 23, 22, 24, \
    TXT_DIAG_LEFT+TXT_DIAG_LBL_GAP, \
    TXT_DIAG_TOP+TXT_DIAG_VGAP*2,\
    EDF_DATA_WIDTH, TXT_DIAG_HEIGHT, \
    WSTL_DIAG_TEXT, WSTL_DIAG_BG_02, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, \
    diag_edf_keymap, diag_hex_edf_callback,
    C_ALIGN_LEFT|C_ALIGN_VCENTER, 2, 0, \
    EDIT_STATUS|NORMAL_EDIT_MODE, "s64", \
    64, CURSOR_NORMAL, \
    NULL, NULL, edf_i2c_wdata_str);

/*The edit field for I2C Write Data length*/
DEF_EDITFIELD(edf_i2c_wlen, &diag_i2c, &txt_i2c_read_data,\
    C_ATTR_ACTIVE, C_FONT_DEFAULT, \
    24, 24, 24, 23, 25, \
    LBL_DATA_LEN_LEFT+TXT_DIAG_LBL_GAP, \
    TXT_DIAG_TOP+TXT_DIAG_VGAP*2,\
    EDF_NUM_WIDTH, TXT_DIAG_HEIGHT, \
    WSTL_DIAG_TEXT, WSTL_DIAG_BG_02,\
    WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, \
    diag_edf_keymap, i2c_num_edf_callback,
    C_ALIGN_CENTER|C_ALIGN_VCENTER, 2, 0, \
    EDIT_STATUS|NORMAL_EDIT_MODE, "s2", \
    2, CURSOR_NORMAL, \
    NULL, NULL, edf_i2c_wlen_str);

/*The edit field for I2C Read Data length*/
DEF_EDITFIELD(edf_i2c_rlen, &diag_i2c, \
    &btn_i2c_writeread, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
    25, 25, 25, 24, 26, \
    LBL_DATA_LEN_LEFT+TXT_DIAG_LBL_GAP, TXT_DIAG_TOP+TXT_DIAG_VGAP*3,\
    EDF_NUM_WIDTH, TXT_DIAG_HEIGHT, \
    WSTL_DIAG_TEXT, WSTL_DIAG_BG_02, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, \
    diag_edf_keymap, i2c_num_edf_callback,
    C_ALIGN_CENTER|C_ALIGN_VCENTER, 2, 0, \
    EDIT_STATUS|NORMAL_EDIT_MODE, "s2", \
    2, CURSOR_NORMAL, \
    NULL, NULL, edf_i2c_rlen_str);


/*************************/
/*THE BOTTONS IN THE PAGE*/
/*************************/

/*Two Botton  "Write/Read" */
DEF_TEXTFIELD(btn_i2c_writeread, &diag_i2c,\
      &btn_i2c_exit, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
      26, 27, 27, 25, 27, \
      BTN_DIAG_LEFT, TXT_DIAG_TOP+TXT_DIAG_VGAP*4, \
      TXT_DIAG_LBL_WIDTH, TXT_DIAG_HEIGHT, \
      WSTL_DIAG_BUTTON_01, WSTL_DIAG_BUTTON_02, \
      WSTL_DIAG_BUTTON_03, WSTL_NOSHOW_IDX, \
      diag_btn_keymap, i2c_write_read_callback, \
      C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, \
      0, diag_unistr[STR_BTN_W_R_ID]);

/*Two Botton  "Exit" */
DEF_TEXTFIELD(btn_i2c_exit, &diag_i2c, NULL, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
      27, 26, 26, 25, 21, \
      BTN_DIAG_LEFT+TXT_DIAG_LBL_GAP, TXT_DIAG_TOP+TXT_DIAG_VGAP*4 ,\
      TXT_DIAG_LBL_WIDTH, TXT_DIAG_HEIGHT, \
      WSTL_DIAG_BUTTON_01, WSTL_DIAG_BUTTON_02,\
      WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, \
      diag_btn_keymap, diag_exit_callback, \
      C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, \
      0, diag_unistr[STR_BTN_EXIT_ID]);

/*******************/
/*The Frame Container*/
/*******************/
DEF_CONTAINER(diag_i2c, NULL, NULL, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
      0, 0, 0, 0, 0, \
      WIN_DIAG_LEFT, WIN_DIAG_TOP, WIN_DIAG_WIDTH, WIN_DIAG_HEIGHT, \
      WSTL_DIAG_BG_02, WSTL_DIAG_BG_02, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, \
      diag_con_keymap, i2c_con_callback, \
      &txt_i2c_title, 21, 0);

/* End of Objects Defination */


static PRESULT i2c_num_edf_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    return ret;
}

static PRESULT i2c_write_read_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8   unact=0;
    INT32 ret_val=0;
    UINT32 i2c_id=0;
    UINT8 i2c_addr=0;
    UINT8 pdata_buf[32]={0};
    UINT16 wlen=0;
    UINT16 rlen=0;
    //PRESULT obj_ret = PROC_PASS;
    UINT32 str_ret=0;
	if(0 == str_ret)
	{
		;
	}
    switch (event)
    {
    case EVN_UNKNOWN_ACTION:
     unact = (VACTION)(param1>>16);
        if(VACT_ENTER == unact)
        {
        i2c_id = com_uni_str2hex(edf_i2c_id_str);
        i2c_addr = (UINT8)com_uni_str2hex(edf_i2c_addr_str);
        wlen = com_uni_str2int(edf_i2c_wlen_str);
        rlen = com_uni_str2int(edf_i2c_rlen_str);

        if( (wlen>NUM_ZERO) && (rlen>NUM_ZERO))
        {
            com_uni_str2hex_arr(pdata_buf, edf_i2c_wdata_str, wlen);
            ret_val = i2c_write_read(i2c_id, i2c_addr, pdata_buf, wlen, rlen);
            if(ret_val!=SUCCESS)
            {
                libc_printf("I2C write_read error. error code: %d",ret_val);
                return ret_val;
            }

            str_ret=com_hex_arr2uni_str(txt_i2c_rdata_str, pdata_buf, rlen);
        }
        else if(wlen>0)
        {
            com_uni_str2hex_arr(pdata_buf, edf_i2c_wdata_str, wlen);
            ret_val = i2c_write(i2c_id, i2c_addr, pdata_buf, wlen);
            if(ret_val!=SUCCESS)
            {
                libc_printf("I2C write error. error code: %d",ret_val);
                return ret_val;
            }
            txt_i2c_rdata_str[0] = 0;//str_ret=ComAscStr2Uni("", txt_i2c_rdata_str);
        }
        else if(rlen>0)
        {
            ret_val = i2c_read(i2c_id, i2c_addr, pdata_buf, wlen);
            if(SUCCESS != ret_val)
            {
                libc_printf("I2C read error. error code: %d",ret_val);
                return ret_val;
            }
            str_ret=com_hex_arr2uni_str(txt_i2c_rdata_str, pdata_buf, rlen);
        }

        osd_draw_object((POBJECT_HEAD)&txt_i2c_rdata, C_UPDATE_ALL);

        }
        break;
    default:
            break;
    }

    return ret;
}

static PRESULT i2c_con_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    //PRESULT obj_ret = PROC_PASS;
    UINT32 str_ret = 0;
	if(0 == str_ret)
	{
		;
	}
    switch (event)
    {
    case EVN_PRE_OPEN:
    str_ret=com_asc_str2uni((UINT8 *)"00000000", edf_i2c_id_str);
    str_ret=com_asc_str2uni((UINT8 *)"00", edf_i2c_addr_str);
    str_ret=com_asc_str2uni((UINT8 *)"0", edf_i2c_wlen_str);
    str_ret=com_asc_str2uni((UINT8 *)"0", edf_i2c_rlen_str);
    edf_i2c_wdata_str[0] = 0;//str_ret=ComAscStr2Uni("", edf_i2c_wdata_str);
    txt_i2c_rdata_str[0] = 0;//str_ret=ComAscStr2Uni("", txt_i2c_rdata_str);
        break;

    case EVN_POST_CLOSE:
    osd_draw_object((POBJECT_HEAD)&win_diag, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
        break;
   default:
     break;
    }
    return ret;
}


