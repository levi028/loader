#ifndef _WIN_MULTIVIEW_PASSWORD_H_
#define _WIN_MULTIVIEW_PASSWORD_H_

//typedef enum
//{
//    WIN_PWD_CHAN = 0,
//    WIN_PWD_MENU,
//}WIN_PWD_TYPE;

//typedef enum
//{
//    WIN_PWD_INPUT = 0,
//    WIN_PWD_EDIT,
//}WIN_PWD_INPUT_TYPE;

//typedef enum
//{
//    WIN_PWD_CALLED_UNKOWN = 0,
//    WIN_PWD_CALLED_PROG_PLAYING,
 //   WIN_PWD_CALLED_PVR
//}WIN_PWD_CALLED_MODULE;

BOOL win_mv_pwd_open(UINT32* vkey_exist_array,UINT32 vkey_exist_cnt);
BOOL win_mv_pwd_get_input(char* input_pwd);
//void win_set_pwd_caller(WIN_PWD_CALLED_MODULE pwd_by_rating);
//WIN_PWD_CALLED_MODULE win_get_pwd_caller();

#endif//_WIN_PASSWORD_H_

