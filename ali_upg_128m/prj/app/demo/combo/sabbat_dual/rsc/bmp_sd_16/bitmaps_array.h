//reduce the space for ota loader
#ifndef _BUILD_OTA_E_
#include "Im_Black.h"
#include "Im_Game_Tetris_Block_01.h"
#include "Im_Game_Tetris_Block_02.h"
#include "Im_Game_Tetris_Block_03.h"
#include "Im_Game_Tetris_Block_04.h"
#include "Im_Game_Tetris_Block_05.h"
#include "Im_Game_Tetris_Block_06.h"
#include "Im_Game_Tetris_Block_07.h"
#include "Im_Game_Tetris_Block_08.h"
#include "Im_White.h"
#include "Im_Arrow_02_Left.h"
#include "Im_Arrow_02_Right.h"
#include "Im_Arrow_02_Select_Left.h"
#include "Im_Arrow_02_Select_Right.h"
#include "Im_Button_EPG_01_Left.h"
#include "Im_Button_EPG_01_Middle.h"
#include "Im_Button_EPG_01_Right.h"
#include "Im_Button_EPG_02_L.h"
#include "Im_Button_EPG_02_M.h"
#include "Im_Button_EPG_02_R.h"
#include "Im_Button_Mediaplay_01_L.h"
#include "Im_Button_Mediaplay_01_M.h"
#include "Im_Button_Mediaplay_01_R.h"
#else
#define im_black NULL
#define im_game_tetris_block_01 NULL
#define im_game_tetris_block_02 NULL
#define im_game_tetris_block_03 NULL
#define im_game_tetris_block_04 NULL
#define im_game_tetris_block_05 NULL
#define im_game_tetris_block_06 NULL
#define im_game_tetris_block_07 NULL
#define im_game_tetris_block_08 NULL
#define im_white NULL
#define im_arrow_02_left NULL
#define im_arrow_02_right NULL
#define im_arrow_02_select_left NULL
#define im_arrow_02_select_right NULL
#define im_button_epg_01_left NULL
#define im_button_epg_01_middle NULL
#define im_button_epg_01_right NULL
#define im_button_epg_02_l NULL
#define im_button_epg_02_m NULL
#define im_button_epg_02_r NULL
#define im_button_mediaplay_01_l NULL
#define im_button_mediaplay_01_m NULL
#define im_button_mediaplay_01_r NULL
#endif
#include "Im_Button_POP_01_L.h"
#include "Im_Button_POP_01_M.h"
#include "Im_Button_POP_01_M_08.h"
#include "Im_Button_POP_01_R.h"
#include "Im_Button_POP_01_R_09.h"
#include "Im_Button_POP_02_L.h"
#include "Im_Button_Select_L.h"
#include "Im_Button_Select_M.h"
#include "Im_Button_Select_R.h"
#include "Im_Button_Short_Left.h"
#include "Im_Button_Short_Middle.h"
#include "Im_Button_Short_Right.h"
#include "Im_Button_Short_Select_Left.h"
#include "Im_Button_Short_Select_Middle.h"
#include "Im_Button_Short_Select_Right.h"

#ifndef _BUILD_OTA_E_
#include "Im_Mediaplayer.h"
#include "Im_Mediaplayer_On.h"
#include "Im_Mediaplayer_Select.h"
#include "Im_Channel.h"
#include "Im_ChannelList_MediaPlay.h"
#include "Im_ChannelList_PVR.h"
#include "Im_ChannelList_Radio.h"
#include "Im_ChannelList_TV.h"
#include "Im_Channel_On.h"
#include "Im_Channel_Select.h"
#include "Im_ColorButton_Blue.h"
#include "Im_ColorButton_Blue_Select.h"
#include "Im_ColorButton_Gray.h"
#include "Im_ColorButton_Gray_Select.h"
#include "Im_ColorButton_Green.h"
#include "Im_ColorButton_Green_Select.h"
#include "Im_ColorButton_Red.h"
#include "Im_ColorButton_Red_Select.h"
#include "Im_ColorButton_Yellow.h"
#include "Im_ColorButton_Yellow_Select.h"
#include "Im_EPG.h"
#include "Im_EPG_ColorButton_Blue.h"
#include "Im_EPG_ColorButton_Green.h"
#include "Im_EPG_ColorButton_Red.h"
#include "Im_EPG_ColorButton_Yellow.h"
#include "Im_EPG_Line_01.h"
#include "Im_EPG_OnTime.h"
#include "Im_EPG_Text_Bg_01.h"
#include "Im_EPG_Time_Back.h"
#include "Im_EPG_Time_Back_Select.h"
#include "Im_EPG_Time_Day_Back.h"
#include "Im_EPG_Time_Day_Back_Select.h"
#include "Im_EPG_Time_Day_Forward.h"
#include "Im_EPG_Time_Day_Forward_Select.h"
#include "Im_EPG_Time_HaHo_Back.h"
#include "Im_EPG_Time_HaHo_Back_Select.h"
#include "Im_EPG_Time_HaHo_Forward.h"
#include "Im_EPG_Time_HaHo_Forward_Select.h"
#include "Im_EPG_Time_HaHo_Help.h"
#include "Im_EPG_Time_HaHo_Help_Select.h"
#include "Im_EPG_Time_OnTi.h"
#include "Im_EPG_Time_OnTi_Select.h"
#include "Im_EPG_Time_TwHo_Back.h"
#include "Im_EPG_Time_TwHo_Back_Select.h"
#include "Im_EPG_Time_TwHo_Forward.h"
#include "Im_EPG_Time_TwHo_Forward_Select.h"
#include "Im_Game.h"
#include "Im_Game_On.h"
#include "Im_Game_Select.h"
#include "Im_Game_Win_01_Bottom.h"
#include "Im_Game_Win_01_Left.h"
#include "Im_Game_Win_01_LeftBottom.h"
#include "Im_Game_Win_01_LeftTop.h"
#include "Im_Game_Win_01_Right.h"
#include "Im_Game_Win_01_RightBottom.h"
#include "Im_Game_Win_01_RightTop.h"
#include "Im_Game_Win_01_Top.h"
#include "Im_Group_Left.h"
#include "Im_Group_Right.h"
#include "Im_HD.h"
#include "Im_HDD_Info_01.h"
#include "Im_HDD_Info_02.h"
#include "Im_HDD_Info_03.h"
#include "Im_HDD_Info_04.h"
#include "Im_HDD_Info_05.h"
#include "Im_Icon_TVInformation.h"
#include "Im_Information_Day.h"
#include "Im_Information_Icon_6.h"
#include "Im_Information_Icon_6_09.h"
#include "Im_Information_Icon_EPG.h"
#include "Im_Information_Icon_Left.h"
#include "Im_Information_Icon_Money.h"
#include "Im_Information_Icon_Right.h"
#include "Im_Information_Icon_Stereo.h"
#include "Im_Information_Icon_SubTitle.h"
#include "Im_Information_Icon_TTX.h"
#include "Im_Information_PVR.h"
#include "Im_Information_Radio.h"
#include "Im_Information_REC.h"
#include "Im_Information_Time.h"
#include "Im_Information_TV.h"
#include "Im_Installation.h"
#include "Im_Installation_On.h"
#include "Im_Installation_Select.h"
#include "Im_Keyboard_Bar_Left.h"
#include "Im_Keyboard_Bar_Middle.h"
#include "Im_Keyboard_Bar_Right.h"
#include "Im_Keyboard_Item.h"
#include "Im_Keyboard_Item_big.h"
#include "Im_Keyboard_Item_big_Select.h"
#include "Im_Keyboard_Item_Select.h"
#include "Im_Loading.h"
#include "Im_Lock.h"
#include "Im_Mainmenu_Icon_Default.h"
#include "Im_Mainmenu_LeftWin_Bottom.h"
#include "Im_Mainmenu_LeftWin_Left.h"
#include "Im_Mainmenu_LeftWin_LeftBot.h"
#include "Im_Mainmenu_LeftWin_LeftTop.h"
#include "Im_Mainmenu_LeftWin_Right.h"
#include "Im_Mainmenu_LeftWin_RightTo.h"
#include "Im_Mainmenu_LeftWin_Righ_13.h"
#include "Im_Mainmenu_LeftWin_Top.h"
#else
#define im_mediaplayer NULL
#define im_mediaplayer_on NULL
#define im_mediaplayer_select NULL
#define im_channel NULL
#define im_channel_list_media_play NULL
#define im_channel_list_pvr NULL
#define im_channel_list_radio NULL
#define im_channel_list_tv NULL
#define im_channel_on NULL
#define im_channel_select NULL
#define im_color_button_blue NULL
#define im_color_button_blue_select NULL
#define im_color_button_gray NULL
#define im_color_button_gray_select NULL
#define im_color_button_green NULL
#define im_color_button_green_select NULL
#define im_color_button_red NULL
#define im_color_button_red_select NULL
#define im_color_button_yellow NULL
#define im_color_button_yellow_select NULL
#define im_epg NULL
#define im_epg_color_button_blue NULL
#define im_epg_color_button_green NULL
#define im_epg_color_button_red NULL
#define im_epg_color_button_yellow NULL
#define im_epg_line_01 NULL
#define im_epg_on_time NULL
#define im_epg_text_bg_01 NULL
#define im_epg_time_back NULL
#define im_epg_time_back_select NULL
#define im_epg_time_day_back NULL
#define im_epg_time_day_back_select NULL
#define im_epg_time_day_forward NULL
#define im_epg_time_day_forward_select NULL
#define im_epg_time_ha_ho_back NULL
#define im_epg_time_ha_ho_back_select NULL
#define im_epg_time_ha_ho_forward NULL
#define im_epg_time_ha_ho_forward_select NULL
#define im_epg_time_ha_ho_help NULL
#define im_epg_time_ha_ho_help_select NULL
#define im_epg_time_on_ti NULL
#define im_epg_time_on_ti_select NULL
#define im_epg_time_tw_ho_back NULL
#define im_epg_time_tw_ho_back_select NULL
#define im_epg_time_tw_ho_forward NULL
#define im_epg_time_tw_ho_forward_select NULL
#define im_game NULL
#define im_game_on NULL
#define im_game_select NULL
#define im_game_win_01_bottom NULL
#define im_game_win_01_left NULL
#define im_game_win_01_left_bottom NULL
#define im_game_win_01_left_top NULL
#define im_game_win_01_right NULL
#define im_game_win_01_right_bottom NULL
#define im_game_win_01_right_top NULL
#define im_game_win_01_top NULL
#define im_group_left NULL
#define im_group_right NULL
#define im_hd NULL
#define im_hdd_info_01 NULL
#define im_hdd_info_02 NULL
#define im_hdd_info_03 NULL
#define im_hdd_info_04 NULL
#define im_hdd_info_05 NULL
#define im_icon_tvinformation NULL
#define im_information_day NULL
#define im_information_icon_6 NULL
#define im_information_icon_6_09 NULL
#define im_information_icon_epg NULL
#define im_information_icon_left NULL
#define im_information_icon_money NULL
#define im_information_icon_right NULL
#define im_information_icon_stereo NULL
#define im_information_icon_sub_title NULL
#define im_information_icon_ttx NULL
#define im_information_pvr NULL
#define im_information_radio NULL
#define im_information_rec NULL
#define im_information_time NULL
#define im_information_tv NULL
#define im_installation NULL
#define im_installation_on NULL
#define im_installation_select NULL
#define im_keyboard_bar_left NULL
#define im_keyboard_bar_middle NULL
#define im_keyboard_bar_right NULL
#define im_keyboard_item NULL
#define im_keyboard_item_big NULL
#define im_keyboard_item_big_select NULL
#define im_keyboard_item_select NULL
#define im_loading NULL
#define im_lock NULL
#define im_mainmenu_icon_default NULL
#define im_mainmenu_left_win_bottom NULL
#define im_mainmenu_left_win_left NULL
#define im_mainmenu_left_win_left_bot NULL
#define im_mainmenu_left_win_left_top  NULL
#define im_mainmenu_left_win_right  NULL
#define im_mainmenu_left_win_right_to  NULL
#define im_mainmenu_left_win_righ_13  NULL
#define im_mainmenu_left_win_top  NULL
#endif

#include "Im_MainMenu_RightWin_Bottom.h"
#include "Im_MainMenu_RightWin_Left.h"
#include "Im_MainMenu_RightWin_LeftBo.h"
#include "Im_MainMenu_RightWin_LeftTo.h"
#include "Im_MainMenu_RightWin_Right.h"
#include "Im_MainMenu_RightWin_RightB.h"
#include "Im_MainMenu_RightWin_RightT.h"
#include "Im_MainMenu_RightWin_Top.h"

#ifndef _BUILD_OTA_E_
#include "Im_MainMenu_SearchWin_01_Bottom.h"
#include "Im_MainMenu_SearchWin_01_Left.h"
#include "Im_MainMenu_SearchWin_01_LeftBottom.h"
#include "Im_MainMenu_SearchWin_01_LeftTop.h"
#include "Im_MainMenu_SearchWin_01_Right.h"
#include "Im_MainMenu_SearchWin_01_RightBottom.h"
#include "Im_MainMenu_SearchWin_01_RightTop.h"
#include "Im_MainMenu_SearchWin_01_Top.h"
#include "Im_MainMenu_SearchWin_02_Bottom.h"
#include "Im_MainMenu_SearchWin_02_Left.h"
#include "Im_MainMenu_SearchWin_02_LeftBottom.h"
#include "Im_MainMenu_SearchWin_02_LeftTop.h"
#include "Im_MainMenu_SearchWin_02_Right.h"
#include "Im_MainMenu_SearchWin_02_RightBottom.h"
#include "Im_MainMenu_SearchWin_02_RightTop.h"
#include "Im_MainMenu_SearchWin_02_Top.h"
#else
#define im_main_menu_search_win_01_bottom NULL
#define im_main_menu_search_win_01_left  NULL
#define im_main_menu_search_win_01_left_bottom  NULL
#define im_main_menu_search_win_01_left_top  NULL
#define im_main_menu_search_win_01_right  NULL
#define im_main_menu_search_win_01_right_bottom  NULL
#define im_main_menu_search_win_01_right_top  NULL
#define im_main_menu_search_win_01_top  NULL
#define im_main_menu_search_win_02_bottom  NULL
#define im_main_menu_search_win_02_left  NULL
#define im_main_menu_search_win_02_left_bottom  NULL
#define im_main_menu_search_win_02_left_top  NULL
#define im_main_menu_search_win_02_right  NULL
#define im_main_menu_search_win_02_right_bottom  NULL
#define im_main_menu_search_win_02_right_top  NULL
#define im_main_menu_search_win_02_top  NULL
#endif

#include "Im_MainMenu_SearchWin_03_Bottom.h"
#include "Im_MainMenu_SearchWin_03_Left.h"
#include "Im_MainMenu_SearchWin_03_LeftBottom.h"
#include "Im_MainMenu_SearchWin_03_LeftTop.h"
#include "Im_MainMenu_SearchWin_03_Right.h"
#include "Im_MainMenu_SearchWin_03_RightBottom.h"
#include "Im_MainMenu_SearchWin_03_RightTop.h"
#include "Im_MainMenu_SearchWin_03_Top.h"

#ifndef _BUILD_OTA_E_
#include "Im_MediaPlay_Bar_01_Middle.h"
#include "Im_MediaPlay_Control_Backward.h"
#include "Im_MediaPlay_Control_Backward_Hi.h"
#include "Im_MediaPlay_Control_Forward.h"
#include "Im_MediaPlay_Control_Forward_Hi.h"
#include "Im_MediaPlay_Control_NewBack.h"
#include "Im_MediaPlay_Control_NewBack_Hi.h"
#include "Im_MediaPlay_Control_NewFast.h"
#include "Im_MediaPlay_Control_NewFast_Hi.h"
#include "Im_MediaPlay_Control_Pause.h"
#include "Im_MediaPlay_Control_Pause_Hi.h"
#include "Im_MediaPlay_Control_Play.h"
#include "Im_MediaPlay_Control_Play_Hi.h"
#include "Im_MediaPlay_Control_Stop.h"
#include "Im_MediaPlay_Control_Stop_Hi.h"
#include "Im_MediaPlay_Exit.h"
#include "Im_MediaPlay_GroupArrow_L.h"
#include "Im_MediaPlay_GroupArrow_R.h"
#include "Im_MediaPlay_HelpButton_AB.h"
#include "Im_MediaPlay_HelpButton_EXIT.h"
#include "Im_MediaPlay_HelpButton_I.h"
#include "Im_MediaPlay_HelpButton_LR.h"
#include "Im_MediaPlay_HelpButton_MP.h"
#include "Im_MediaPlay_HelpButton_PIP.h"
#include "Im_MediaPlay_HelpButton_SWAP.h"
#include "Im_MediaPlay_Icon_01.h"
#include "Im_Mediaplay_Loading_01.h"
#include "Im_Mediaplay_Loading_02.h"
#include "Im_Mediaplay_Loading_03.h"
#include "Im_Mediaplay_Loading_04.h"
#include "Im_Mediaplay_Loading_05.h"
#include "Im_Mediaplay_Loading_06.h"
#include "Im_Mediaplay_Loading_07.h"
#include "Im_Mediaplay_Loading_08.h"
#include "Im_Mediaplay_Loading_09.h"
#include "Im_Mediaplay_Loading_10.h"
#include "Im_Mediaplay_Loading_11.h"
#include "Im_Mediaplay_Loading_12.h"
#include "Im_MediaPlay_POPWin_01_B.h"
#include "Im_MediaPlay_POPWin_01_L.h"
#include "Im_MediaPlay_POPWin_01_LB.h"
#include "Im_MediaPlay_POPWin_01_LT.h"
#include "Im_MediaPlay_POPWin_01_R.h"
#include "Im_MediaPlay_POPWin_01_RB.h"
#include "Im_MediaPlay_POPWin_01_RT.h"
#include "Im_MediaPlay_POPWin_01_T.h"
#include "Im_MediaPlay_RateBg_01.h"
#include "Im_Mediaplay_Rotate.h"
#include "Im_MediaPlay_TitleLine_01.h"
#include "Im_MediaPlay_TopBg_Icon_Music.h"
#include "Im_MediaPlay_TopBg_Icon_Music_Gray.h"
#include "Im_MediaPlay_TopBg_Icon_Music_S.h"
#include "Im_MediaPlay_TopBg_Icon_Photo.h"
#include "Im_MediaPlay_TopBg_Icon_Photo_Gray.h"
#include "Im_MediaPlay_TopBg_Icon_Photo_S.h"
#include "Im_MediaPlay_TopBg_Icon_Record.h"
#include "Im_MediaPlay_TopBg_Icon_Record_Gray.h"
#include "Im_MediaPlay_TopBg_Icon_Record_S.h"
#include "Im_MediaPlay_TopBg_Icon_Video.h"
#include "Im_MediaPlay_TopBg_Icon_Video_Gray.h"
#include "Im_MediaPlay_TopBg_Icon_Video_S.h"
#include "Im_MediaPlay_USBBar_L.h"
#include "Im_MediaPlay_USBBar_M.h"
#include "Im_MediaPlay_USBBar_R.h"
#include "Im_MotionL_01.h"
#include "Im_MotionL_02.h"
#include "Im_MotionL_03.h"
#include "Im_MotionL_04.h"
#include "Im_MotionL_05.h"
#include "Im_MotionR_01.h"
#include "Im_MotionR_02.h"
#include "Im_MotionR_03.h"
#include "Im_MotionR_04.h"
#include "Im_MotionR_05.h"
#include "Im_MP3_BMP.h"
#include "Im_MP3_Icon_BMP.h"
#include "Im_MP3_Icon_Folder.h"
#include "Im_MP3_Icon_JPG.h"
#include "Im_MP3_Icon_Mem_HardD.h"
#include "Im_MP3_Icon_Mem_SD.h"
#include "Im_MP3_Icon_Mem_USB.h"
#include "Im_MP3_Icon_MP3.h"
#include "Im_MP3_Icon_MPG.h"
#include "Im_MP3_Icon_OGG.h"
#include "Im_MP3_Icon_FLAC.h"
#include "Im_MP3_Icon_WAV.h"
#include "Im_MP3_Icon_Radom_Folder.h"
#include "Im_MP3_Icon_Radom_List.h"
#include "Im_MP3_Icon_Repeat_0.h"
#include "Im_MP3_Icon_Repeat_0_Hi.h"
#include "Im_MP3_Icon_Repeat_1.h"
#include "Im_MP3_Icon_Repeat_Folder.h"
#include "Im_MP3_Icon_Repeat_List.h"
#include "Im_MP3_Photo.h"
#include "Im_MP3_Video.h"
#include "Im_Mselect.h"
#include "Im_MultiButton_Left.h"
#include "Im_MultiButton_Middle.h"
#include "Im_MultiButton_Right.h"
#include "Im_Mute.h"
#include "Im_Mute_S.h"
#include "Im_Number.h"
#include "Im_Number_S.h"
#include "Im_Orange_Arrow_S.h"
#include "Im_Pause.h"
#include "Im_Pause_S.h"
#include "Im_PVR.h"
#include "Im_PVR_Dot.h"
#include "Im_PVR_Dot_Orange.h"
#include "Im_PVR_Icon_Backward.h"
#include "Im_PVR_Icon_Bookmark.h"
#include "Im_PVR_Icon_Forward.h"
#include "Im_PVR_Icon_Pause.h"
#include "Im_PVR_Icon_Play.h"
#include "Im_PVR_Icon_REC.h"
#include "Im_PVR_Icon_Record.h"
#include "Im_PVR_Icon_SlowBackward.h"
#include "Im_PVR_Icon_Slowforward.h"
#include "Im_PVR_Icon_Step.h"
#include "Im_PVR_Icon_Stop.h"
#include "Im_PVR_Icon_Timeshift.h"
#include "Im_PVR_On.h"
#include "Im_PVR_REC_Win_Left.h"
#include "Im_PVR_REC_Win_Middle.h"
#include "Im_PVR_REC_Win_Right.h"
#include "Im_PVR_Select.h"
#include "Im_Receive.h"
#include "Im_Rename_Pencil.h"
#include "Im_Satellite.h"
#include "Im_Satellite_line_1.h"
#include "Im_Satellite_line_2.h"
#include "Im_Satellite_line_3.h"
#include "Im_Satellite_line_4.h"
#include "Im_Satellite_line_5.h"
#else
#define im_media_play_bar_01_middle NULL
#define im_media_play_control_backward NULL
#define im_media_play_control_backward_hi NULL
#define im_media_play_control_forward NULL
#define im_media_play_control_forward_hi NULL
#define im_media_play_control_new_back NULL
#define im_media_play_control_new_back_hi NULL
#define im_media_play_control_new_fast NULL
#define im_media_play_control_new_fast_hi NULL
#define im_media_play_control_pause NULL
#define im_media_play_control_pause_hi NULL
#define im_media_play_control_play NULL
#define im_media_play_control_play_hi NULL
#define im_media_play_control_stop NULL
#define im_media_play_control_stop_hi NULL
#define im_media_play_exit NULL
#define im_media_play_group_arrow_l NULL
#define im_media_play_group_arrow_r NULL
#define im_media_play_help_button_ab NULL
#define im_media_play_help_button_exit NULL
#define im_media_play_help_button_i NULL
#define im_media_play_help_button_lr NULL
#define im_media_play_help_button_mp NULL
#define im_media_play_help_button_pip NULL
#define im_media_play_help_button_swap NULL
#define im_media_play_icon_01 NULL
#define im_mediaplay_loading_01 NULL
#define im_mediaplay_loading_02 NULL
#define im_mediaplay_loading_03 NULL
#define im_mediaplay_loading_04 NULL
#define im_mediaplay_loading_05 NULL
#define im_mediaplay_loading_06 NULL
#define im_mediaplay_loading_07 NULL
#define im_mediaplay_loading_08 NULL
#define im_mediaplay_loading_09 NULL
#define im_mediaplay_loading_10 NULL
#define im_mediaplay_loading_11 NULL
#define im_mediaplay_loading_12 NULL
#define im_media_play_popwin_01_b NULL
#define im_media_play_popwin_01_l NULL
#define im_media_play_popwin_01_lb NULL
#define im_media_play_popwin_01_lt NULL
#define im_media_play_popwin_01_r NULL
#define im_media_play_popwin_01_rb NULL
#define im_media_play_popwin_01_rt NULL
#define im_media_play_popwin_01_t NULL
#define im_media_play_rate_bg_01 NULL
#define im_mediaplay_rotate NULL
#define im_media_play_title_line_01 NULL
#define im_media_play_top_bg_icon_music NULL
#define im_media_play_top_bg_icon_music_gray NULL
#define im_media_play_top_bg_icon_music_s NULL
#define im_media_play_top_bg_icon_photo NULL
#define im_media_play_top_bg_icon_photo_gray NULL
#define im_media_play_top_bg_icon_photo_s NULL
#define im_media_play_top_bg_icon_record NULL
#define im_media_play_top_bg_icon_record_gray NULL
#define im_media_play_top_bg_icon_record_s NULL
#define im_media_play_top_bg_icon_video NULL
#define im_media_play_top_bg_icon_video_gray NULL
#define im_media_play_top_bg_icon_video_s NULL
#define im_media_play_usbbar_l NULL
#define im_media_play_usbbar_m NULL
#define im_media_play_usbbar_r NULL
#define im_motion_l_01 NULL
#define im_motion_l_02 NULL
#define im_motion_l_03 NULL
#define im_motion_l_04 NULL
#define im_motion_l_05 NULL
#define im_motion_r_01 NULL
#define im_motion_r_02 NULL
#define im_motion_r_03 NULL
#define im_motion_r_04 NULL
#define im_motion_r_05 NULL
#define im_mp3_bmp NULL
#define im_mp3_icon_bmp NULL
#define im_mp3_icon_folder NULL
#define im_mp3_icon_jpg NULL
#define im_mp3_icon_mem_hard_d NULL
#define im_mp3_icon_mem_sd NULL
#define im_mp3_icon_mem_usb NULL
#define im_mp3_icon_mp3 NULL
#define im_mp3_icon_mpg NULL
#define im_mp3_icon_ogg NULL
#define im_mp3_icon_flac NULL
#define im_mp3_icon_wav NULL
#define im_mp3_icon_radom_folder NULL
#define im_mp3_icon_radom_list NULL
#define im_mp3_icon_repeat_0 NULL
#define im_mp3_icon_repeat_0_hi NULL
#define im_mp3_icon_repeat_1 NULL
#define im_mp3_icon_repeat_folder NULL
#define im_mp3_icon_repeat_list NULL
#define im_mp3_photo NULL
#define im_mp3_video NULL
#define im_mselect NULL
#define im_multi_button_left NULL
#define im_multi_button_middle NULL
#define im_multi_button_right NULL
#define im_mute NULL
#define im_mute_s NULL
#define im_number NULL
#define im_number_s NULL
#define im_orange_arrow_s NULL
#define im_pause NULL
#define im_pause_s NULL
#define im_pvr NULL
#define im_pvr_dot NULL
#define im_pvr_dot_orange NULL
#define im_pvr_icon_backward NULL
#define im_pvr_icon_bookmark NULL
#define im_pvr_icon_forward NULL
#define im_pvr_icon_pause NULL
#define im_pvr_icon_play NULL
#define im_pvr_icon_rec NULL
#define im_pvr_icon_record NULL
#define im_pvr_icon_slow_backward NULL
#define im_pvr_icon_slowforward NULL
#define im_pvr_icon_step NULL
#define im_pvr_icon_stop NULL
#define im_pvr_icon_timeshift NULL
#define im_pvr_on NULL
#define im_pvr_rec_win_left NULL
#define im_pvr_rec_win_middle NULL
#define im_pvr_rec_win_right NULL
#define im_pvr_select NULL
#define im_receive NULL
#define im_rename_pencil NULL
#define im_satellite NULL
#define im_satellite_line_1 NULL
#define im_satellite_line_2 NULL
#define im_satellite_line_3 NULL
#define im_satellite_line_4 NULL
#define im_satellite_line_5 NULL
#endif

#include "Im_Scroll_Bar_Arrow_Bottom.h"
#include "Im_Scroll_Bar_Arrow_Top.h"
#include "Im_Scroll_Bar_Bg.h"
#include "Im_Scroll_Scroll_Bottom.h"
#include "Im_Scroll_Scroll_Middle.h"
#include "Im_Scroll_Scroll_Top.h"
#include "Im_Select_Enter.h"
#include "Im_Select_Left.h"
#include "Im_Select_List.h"
#include "Im_Select_Right.h"
#include "Im_SignalBg_Left.h"
#include "Im_SignalBg_Middle.h"
#include "Im_SignalBg_Right.h"
#include "Im_Signal_Blue_Left.h"
#include "Im_Signal_Blue_Middle.h"
#include "Im_Signal_Blue_Right.h"
#include "Im_Signal_Gray_Left.h"
#include "Im_Signal_Gray_Middle.h"
#include "Im_Signal_Gray_Right.h"
#include "Im_Signal_Green_Left.h"
#include "Im_Signal_Green_Middle.h"
#include "Im_Signal_Green_Right.h"
#include "Im_Signal_No.h"
#include "Im_Signal_Orange_Left.h"
#include "Im_Signal_Orange_Middle.h"
#include "Im_Signal_Orange_Right.h"
#include "Im_Singal_Bar_E_Bg.h"
#include "Im_Singal_Bar_E_Left.h"
#include "Im_Singal_Bar_E_Right.h"

#ifndef _BUILD_OTA_E_
#include "Im_SList_Win_Bottom.h"
#include "Im_SList_Win_Left.h"
#include "Im_SList_Win_LeftBottom.h"
#include "Im_SList_Win_LeftTop.h"
#include "Im_SList_Win_LeftTop_NA.h"
#include "Im_SList_Win_Right.h"
#include "Im_SList_Win_RightBottom.h"
#include "Im_SList_Win_RightTop.h"
#include "Im_SList_Win_RightTop_NA.h"
#include "Im_SList_Win_Top.h"
#include "Im_Spectram_02_Blue_M.h"
#include "Im_Spectram_02_Blue_T.h"
#include "Im_Spectram_Blue.h"
#include "Im_Spectram_Gray.h"
#include "Im_Spectram_Red.h"
#include "Im_Spectram_Yellow.h"
#include "Im_System.h"
#include "Im_System_On.h"
#include "Im_System_Select.h"
#else
#define im_slist_win_bottom NULL
#define im_slist_win_left NULL
#define im_slist_win_left_bottom NULL
#define im_slist_win_left_top NULL
#define im_slist_win_left_top_na NULL
#define im_slist_win_right NULL
#define im_slist_win_right_bottom NULL
#define im_slist_win_right_top NULL
#define im_slist_win_right_top_na NULL
#define im_slist_win_top NULL
#define im_spectram_02_blue_m NULL
#define im_spectram_02_blue_t NULL
#define im_spectram_blue NULL
#define im_spectram_gray NULL
#define im_spectram_red NULL
#define im_spectram_yellow NULL
#define im_system NULL
#define im_system_on NULL
#define im_system_select NULL
#endif

#include "Im_Title_Bg.h"
#include "Im_Title_Left.h"
#include "Im_Title_Right.h"

#ifndef _BUILD_OTA_E_
#include "Im_Tools.h"
#include "Im_Tools_On.h"
#include "Im_Tools_Select.h"
#include "Im_TV_Del.h"
#include "Im_TV_Favorite.h"
#include "Im_TV_Lock.h"
#include "Im_TV_Move.h"
#include "Im_TV_Skip.h"
#include "Im_USB.h"
#include "Im_Volume.h"
#include "Im_Volume_Bg.h"
#include "Im_Volume_Left.h"
#include "Im_Volume_Right.h"
#else
#define im_tools NULL
#define im_tools_on NULL
#define im_tools_select NULL
#define im_tv_del NULL
#define im_tv_favorite NULL
#define im_tv_lock NULL
#define im_tv_move NULL
#define im_tv_skip NULL
#define im_usb NULL
#define im_volume NULL
#define im_volume_bg NULL
#define im_volume_left NULL
#define im_volume_right NULL
#endif

#include "Im_Window_Bottom_Bg.h"
#include "Im_Window_Bottom_Right.h"
#include "Im_Window_LeftBottom.h"
#include "Im_Window_Middle_Left.h"
#include "Im_Window_Middle_Right.h"
#include "Im_Window_Title_Bg.h"
#include "Im_Window_Title_left.h"
#include "Im_Window_Title_Right.h"

#ifndef _BUILD_OTA_E_
#include "Im_Win_04_B.h"
#include "Im_Win_04_L.h"
#include "Im_Win_04_LB.h"
#include "Im_Win_04_LT.h"
#include "Im_Win_04_R.h"
#include "Im_Win_04_RB.h"
#include "Im_Win_04_RT.h"
#include "Im_Win_04_T.h"
#include "Im_Win_05_B.h"
#include "Im_Win_05_L.h"
#include "Im_Win_05_LB.h"
#include "Im_Win_05_LT.h"
#include "Im_Win_05_R.h"
#include "Im_Win_05_RB.h"
#include "Im_Win_05_RT.h"
#include "Im_Win_05_T.h"
#include "Im_Win_06_B.h"
#include "Im_Win_06_L.h"
#include "Im_Win_06_LB.h"
#include "Im_Win_06_R.h"
#include "Im_Win_06_RB.h"
#include "Im_Win_07_B.h"
#include "Im_Win_07_L.h"
#include "Im_Win_07_LB.h"
#include "Im_Win_07_LT.h"
#include "Im_Win_07_R.h"
#include "Im_Win_07_RB.h"
#include "Im_Win_07_RT.h"
#include "Im_Win_07_T.h"
#include "Im_Win_08_B.h"
#include "Im_Win_08_L.h"
#include "Im_Win_08_LB.h"
#include "Im_Win_08_LT.h"
#include "Im_Win_08_R.h"
#include "Im_Win_08_RB.h"
#include "Im_Win_08_RT.h"
#include "Im_Win_08_T.h"
#include "Im_Win_PV_B.h"
#include "Im_Win_PV_L.h"
#include "Im_Win_PV_LB.h"
#include "Im_Win_PV_LT.h"
#include "Im_Win_PV_R.h"
#include "Im_Win_PV_RB.h"
#include "Im_Win_PV_RT.h"
#include "Im_Win_PV_T.h"
#include "Im_Zoom_Down.h"
#include "Im_Zoom_In.h"
#include "Im_Zoom_Left.h"
#include "Im_Zoom_Right.h"
#include "Im_Zoom_Up.h"
#include "information_26.h"
#include "Media_Player_Pic_06.h"
#include "Im_Conax.h"
#include "Im_Conax_On.h"
#include "Im_Conax_Select.h"
#include "Im_Email_S.h"
#else
#define im_win_04_b NULL
#define im_win_04_l NULL
#define im_win_04_lb NULL
#define im_win_04_lt NULL
#define im_win_04_r NULL
#define im_win_04_rb NULL
#define im_win_04_rt NULL
#define im_win_04_t NULL
#define im_win_05_b NULL
#define im_win_05_l NULL
#define im_win_05_lb NULL
#define im_win_05_lt NULL
#define im_win_05_r NULL
#define im_win_05_rb NULL
#define im_win_05_rt NULL
#define im_win_05_t NULL
#define im_win_06_b NULL
#define im_win_06_l NULL
#define im_win_06_lb NULL
#define im_win_06_r NULL
#define im_win_06_rb NULL
#define im_win_07_b NULL
#define im_win_07_l NULL
#define im_win_07_lb NULL
#define im_win_07_lt NULL
#define im_win_07_r NULL
#define im_win_07_rb NULL
#define im_win_07_rt NULL
#define im_win_07_t NULL
#define im_win_08_b NULL
#define im_win_08_l NULL
#define im_win_08_lb NULL
#define im_win_08_lt NULL
#define im_win_08_r NULL
#define im_win_08_rb NULL
#define im_win_08_rt NULL
#define im_win_08_t NULL
#define im_win_pv_b NULL
#define im_win_pv_l NULL
#define im_win_pv_lb NULL
#define im_win_pv_lt NULL
#define im_win_pv_r NULL
#define im_win_pv_rb NULL
#define im_win_pv_rt NULL
#define im_win_pv_t NULL
#define im_zoom_down NULL
#define im_zoom_in NULL
#define im_zoom_left NULL
#define im_zoom_right NULL
#define im_zoom_up NULL
#define information_26 NULL
#define media_player_pic_06 NULL
#define im_conax NULL
#define im_conax_on NULL
#define im_conax_select NULL
#define im_email_s NULL
#endif
#include "Im_Ali_Demo.h"

#include "Im_Ca.h"
#include "Im_Ca_On.h"
#include "Im_Ca_Select.h"
typedef struct
{
    unsigned short    w;
    unsigned short    h;
    unsigned long    bit_count;
    unsigned long    bmp_size;
    unsigned long    data_size;
    const unsigned char*    data;
}bitmap_infor_t;

bitmap_infor_t bitmaps_infor[]=
{
    {28 , 28 , 16, 1568 , 335  , im_black},
    {20 , 20 , 16, 800  , 711  , im_game_tetris_block_01},
    {20 , 20 , 16, 800  , 565  , im_game_tetris_block_02},
    {20 , 20 , 16, 800  , 732  , im_game_tetris_block_03},
    {20 , 20 , 16, 800  , 558  , im_game_tetris_block_04},
    {20 , 20 , 16, 800  , 582  , im_game_tetris_block_05},
    {20 , 20 , 16, 800  , 712  , im_game_tetris_block_06},
    {20 , 20 , 16, 800  , 598  , im_game_tetris_block_07},
    {20 , 20 , 16, 800  , 592  , im_game_tetris_block_08},
    {28 , 28 , 16, 1568 , 742  , im_white},
    {28 , 28 , 16, 1568 , 129  , im_arrow_02_left},
    {28 , 28 , 16, 1570 , 132  , im_arrow_02_right},
    {28 , 28 , 16, 1568 , 129  , im_arrow_02_select_left},
    {28 , 28 , 16, 1568 , 132  , im_arrow_02_select_right},
    {30 , 30 , 16, 1800 , 291  , im_button_epg_01_left},
    {30 , 30 , 16, 1800 , 158  , im_button_epg_01_middle},
    {30 , 30 , 16, 1800 , 293  , im_button_epg_01_right},
    {30 , 30 , 16, 1800 , 235  , im_button_epg_02_l},
    {30 , 30 , 16, 1800 , 100  , im_button_epg_02_m},
    {30 , 30 , 16, 1800 , 225  , im_button_epg_02_r},
    {30 , 30 , 16, 1800 , 1590 , im_button_mediaplay_01_l},
    {30 , 30 , 16, 1800 , 1573 , im_button_mediaplay_01_m},
    {30 , 30 , 16, 1800 , 1590 , im_button_mediaplay_01_r},
    {30 , 30 , 16, 1800 , 293  , im_button_pop_01_l},
    {30 , 30 , 16, 1800 , 158  , im_button_pop_01_m},
    {30 , 30 , 16, 1800 , 100  , im_button_pop_01_m_08},
    {30 , 30 , 16, 1800 , 288  , im_button_pop_01_r},
    {30 , 30 , 16, 1800 , 100  , im_button_pop_01_r_09},
    {30 , 30 , 16, 1800 , 228  , im_button_pop_02_l},
    {32 , 32 , 16, 2048 , 562  , im_button_select_l},
    {32 , 32 , 16, 2048 , 416  , im_button_select_m},
    {32 , 32 , 16, 2048 , 566  , im_button_select_r},
    {32 , 32 , 16, 2048 , 316  , im_button_short_left},
    {32 , 32 , 16, 2048 , 163  , im_button_short_middle},
    {32 , 32 , 16, 2048 , 324  , im_button_short_right},
    {32 , 32 , 16, 2048 , 591  , im_button_short_select_left},
    {32 , 32 , 16, 2048 , 480  , im_button_short_select_middle},
    {32 , 32 , 16, 2048 , 583  , im_button_short_select_right},
    {64 , 52 , 16, 6656 , 3910 , im_mediaplayer},
    {64 , 52 , 16, 6656 , 5166 , im_mediaplayer_on},
    {64 , 52 , 16, 6656 , 5166 , im_mediaplayer_select},
    {64 , 52 , 16, 6656 , 3315 , im_channel},
    {43 , 43 , 16, 3784 , 2364 , im_channel_list_media_play},
    {43 , 43 , 16, 3784 , 2474 , im_channel_list_pvr},
    {43 , 43 , 16, 3784 , 2109 , im_channel_list_radio},
    {43 , 43 , 16, 3784 , 2248 , im_channel_list_tv},
    {64 , 52 , 16, 6656 , 5336 , im_channel_on},
    {64 , 52 , 16, 6656 , 5328 , im_channel_select},
    {68 , 34 , 16, 4624 , 695  , im_color_button_blue},
    {68 , 34 , 16, 4624 , 1067 , im_color_button_blue_select},
    {68 , 34 , 16, 4624 , 645  , im_color_button_gray},
    {68 , 34 , 16, 4624 , 770  , im_color_button_gray_select},
    {68 , 34 , 16, 4624 , 687  , im_color_button_green},
    {68 , 34 , 16, 4624 , 838  , im_color_button_green_select},
    {68 , 34 , 16, 4624 , 695  , im_color_button_red},
    {68 , 34 , 16, 4624 , 1059 , im_color_button_red_select},
    {68 , 34 , 16, 4624 , 707  , im_color_button_yellow},
    {68 , 34 , 16, 4624 , 1067 , im_color_button_yellow_select},
    {43 , 43 , 16, 3784 , 2917 , im_epg},
    {24 , 24 , 16, 1152 , 377  , im_epg_color_button_blue},
    {24 , 24 , 16, 1152 , 382  , im_epg_color_button_green},
    {24 , 24 , 16, 1152 , 361  , im_epg_color_button_red},
    {24 , 24 , 16, 1152 , 380  , im_epg_color_button_yellow},
    {2  , 30 , 16, 120  , 3    , im_epg_line_01},
    {10 , 10 , 16, 200  , 132  , im_epg_on_time},
    {32 , 32 , 16, 2048 , 1518 , im_epg_text_bg_01},
    {24 , 24 , 16, 1152 , 943  , im_epg_time_back},
    {24 , 24 , 16, 1152 , 991  , im_epg_time_back_select},
    {24 , 24 , 16, 1152 , 1031 , im_epg_time_day_back},
    {24 , 24 , 16, 1152 , 1057 , im_epg_time_day_back_select},
    {24 , 24 , 16, 1152 , 1034 , im_epg_time_day_forward},
    {24 , 24 , 16, 1152 , 1066 , im_epg_time_day_forward_select},
    {24 , 24 , 16, 1152 , 955  , im_epg_time_ha_ho_back},
    {24 , 24 , 16, 1152 , 1003 , im_epg_time_ha_ho_back_select},
    {24 , 24 , 16, 1152 , 923  , im_epg_time_ha_ho_forward},
    {24 , 24 , 16, 1152 , 1001 , im_epg_time_ha_ho_forward_select},
    {24 , 24 , 16, 1152 , 997  , im_epg_time_ha_ho_help},
    {24 , 24 , 16, 1152 , 1006 , im_epg_time_ha_ho_help_select},
    {24 , 24 , 16, 1152 , 983  , im_epg_time_on_ti},
    {24 , 24 , 16, 1152 , 1007 , im_epg_time_on_ti_select},
    {24 , 24 , 16, 1152 , 996  , im_epg_time_tw_ho_back},
    {24 , 24 , 16, 1152 , 1050 , im_epg_time_tw_ho_back_select},
    {24 , 24 , 16, 1152 , 987  , im_epg_time_tw_ho_forward},
    {24 , 24 , 16, 1152 , 1039 , im_epg_time_tw_ho_forward_select},
    {64 , 52 , 16, 6656 , 2249 , im_game},
    {64 , 52 , 16, 6656 , 2805 , im_game_on},
    {64 , 52 , 16, 6656 , 2568 , im_game_select},
    {48 , 48 , 16, 4608 , 274  , im_game_win_01_bottom},
    {48 , 48 , 16, 4608 , 912  , im_game_win_01_left},
    {48 , 48 , 16, 4608 , 919  , im_game_win_01_left_bottom},
    {48 , 48 , 16, 4608 , 987  , im_game_win_01_left_top},
    {48 , 48 , 16, 4608 , 936  , im_game_win_01_right},
    {48 , 48 , 16, 4608 , 959  , im_game_win_01_right_bottom},
    {48 , 48 , 16, 4608 , 1008 , im_game_win_01_right_top},
    {48 , 48 , 16, 4608 , 374  , im_game_win_01_top},
    {24 , 24 , 16, 1152 , 292  , im_group_left},
    {24 , 24 , 16, 1152 , 320  , im_group_right},
    {72 , 60 , 16, 8640 , 3928 , im_hd},
    {16 , 36 , 16, 1152 , 234  , im_hdd_info_01},
    {16 , 36 , 16, 1152 , 240  , im_hdd_info_02},
    {16 , 36 , 16, 1152 , 246  , im_hdd_info_03},
    {16 , 36 , 16, 1152 , 253  , im_hdd_info_04},
    {16 , 36 , 16, 1152 , 260  , im_hdd_info_05},
    {40 , 40 , 16, 3200 , 1544 , im_icon_tvinformation},
    {24 , 24 , 16, 1152 , 343  , im_information_day},
    {34 , 24 , 16, 1632 , 663  , im_information_icon_6},
    {34 , 24 , 16, 1632 , 675  , im_information_icon_6_09},
    {32 , 32 , 16, 2048 , 1206 , im_information_icon_epg},
    {34 , 24 , 16, 1632 , 594  , im_information_icon_left},
    {34 , 24 , 16, 1632 , 637  , im_information_icon_money},
    {34 , 24 , 16, 1632 , 590  , im_information_icon_right},
    {34 , 24 , 16, 1632 , 822  , im_information_icon_stereo},
    {32 , 32 , 16, 2048 , 1266 , im_information_icon_sub_title},
    {32 , 32 , 16, 2048 , 1239 , im_information_icon_ttx},
    {43 , 43 , 16, 3784 , 2490 , im_information_pvr},
    {43 , 43 , 16, 3784 , 2177 , im_information_radio},
    {58 , 32 , 16, 3712 , 841  , im_information_rec},
    {24 , 24 , 16, 1152 , 379  , im_information_time},
    {43 , 43 , 16, 3784 , 2244 , im_information_tv},
    {64 , 52 , 16, 6656 , 3289 , im_installation},
    {64 , 52 , 16, 6656 , 5010 , im_installation_on},
    {64 , 52 , 16, 6656 , 5002 , im_installation_select},
    {28 , 40 , 16, 2240 , 50   , im_keyboard_bar_left},
    {28 , 40 , 16, 2240 , 27   , im_keyboard_bar_middle},
    {28 , 40 , 16, 2240 , 44   , im_keyboard_bar_right},
    {36 , 36 , 16, 2592 , 130  , im_keyboard_item},
    {78 , 36 , 16, 5616 , 144  , im_keyboard_item_big},
    {78 , 36 , 16, 5616 , 102  , im_keyboard_item_big_select},
    {36 , 36 , 16, 2592 , 72   , im_keyboard_item_select},
    {60 , 60 , 16, 7200 , 2373 , im_loading},
    {60 , 60 , 16, 7200 , 2293 , im_lock},
    {24 , 32 , 16, 1536 , 433  , im_mainmenu_icon_default},
    {32 , 32 , 16, 2048 , 201  , im_mainmenu_left_win_bottom},
    {32 , 32 , 16, 2048 , 608  , im_mainmenu_left_win_left},
    {32 , 32 , 16, 2048 , 544  , im_mainmenu_left_win_left_bot},
    {32 , 32 , 16, 2048 , 710  , im_mainmenu_left_win_left_top},
    {32 , 32 , 16, 2048 , 27   , im_mainmenu_left_win_right},
    {32 , 32 , 16, 2048 , 236  , im_mainmenu_left_win_right_to},
    {32 , 32 , 16, 2048 , 181  , im_mainmenu_left_win_righ_13},
    {32 , 32 , 16, 2048 , 248  , im_mainmenu_left_win_top},
    {32 , 32 , 16, 2048 , 199  , im_main_menu_right_win_bottom},
    {32 , 32 , 16, 2048 , 386  , im_main_menu_right_win_left},
    {32 , 32 , 16, 2048 , 364  , im_main_menu_right_win_left_bo},
    {32 , 32 , 16, 2048 , 548  , im_main_menu_right_win_left_to},
    {32 , 32 , 16, 2048 , 608  , im_main_menu_right_win_right},
    {32 , 32 , 16, 2048 , 568  , im_main_menu_right_win_right_b},
    {32 , 32 , 16, 2048 , 719  , im_main_menu_right_win_right_t},
    {32 , 32 , 16, 2048 , 236  , im_main_menu_right_win_top},
    {32 , 32 , 16, 2048 , 27   , im_main_menu_search_win_01_bottom},
    {32 , 32 , 16, 2048 , 608  , im_main_menu_search_win_01_left},
    {32 , 32 , 16, 2048 , 608  , im_main_menu_search_win_01_left_bottom},
    {32 , 32 , 16, 2048 , 710  , im_main_menu_search_win_01_left_top},
    {32 , 32 , 16, 2048 , 27   , im_main_menu_search_win_01_right},
    {32 , 32 , 16, 2048 , 27   , im_main_menu_search_win_01_right_bottom},
    {32 , 32 , 16, 2048 , 236  , im_main_menu_search_win_01_right_top},
    {32 , 32 , 16, 2048 , 248  , im_main_menu_search_win_01_top},
    {32 , 32 , 16, 2048 , 27   , im_main_menu_search_win_02_bottom},
    {32 , 32 , 16, 2048 , 27   , im_main_menu_search_win_02_left},
    {32 , 32 , 16, 2048 , 27   , im_main_menu_search_win_02_left_bottom},
    {32 , 32 , 16, 2048 , 252  , im_main_menu_search_win_02_left_top},
    {32 , 32 , 16, 2048 , 608  , im_main_menu_search_win_02_right},
    {32 , 32 , 16, 2048 , 608  , im_main_menu_search_win_02_right_bottom},
    {32 , 32 , 16, 2048 , 719  , im_main_menu_search_win_02_right_top},
    {32 , 32 , 16, 2048 , 238  , im_main_menu_search_win_02_top},
    {32 , 32 , 16, 2048 , 201  , im_main_menu_search_win_03_bottom},
    {32 , 32 , 16, 2048 , 608  , im_main_menu_search_win_03_left},
    {32 , 32 , 16, 2048 , 544  , im_main_menu_search_win_03_left_bottom},
    {32 , 32 , 16, 2048 , 608  , im_main_menu_search_win_03_left_top},
    {32 , 32 , 16, 2048 , 608  , im_main_menu_search_win_03_right},
    {32 , 32 , 16, 2048 , 570  , im_main_menu_search_win_03_right_bottom},
    {32 , 32 , 16, 2048 , 608  , im_main_menu_search_win_03_right_top},
    {32 , 32 , 16, 2048 , 27   , im_main_menu_search_win_03_top},
    {12 , 12 , 16, 288  , 6    , im_media_play_bar_01_middle},
    {24 , 24 , 16, 1152 , 321  , im_media_play_control_backward},
    {24 , 24 , 16, 1152 , 321  , im_media_play_control_backward_hi},
    {24 , 24 , 16, 1152 , 321  , im_media_play_control_forward},
    {24 , 24 , 16, 1152 , 321  , im_media_play_control_forward_hi},
    {24 , 24 , 16, 1152 , 219  , im_media_play_control_new_back},
    {24 , 24 , 16, 1152 , 219  , im_media_play_control_new_back_hi},
    {24 , 24 , 16, 1152 , 219  , im_media_play_control_new_fast},
    {24 , 24 , 16, 1152 , 219  , im_media_play_control_new_fast_hi},
    {24 , 24 , 16, 1152 , 259  , im_media_play_control_pause},
    {24 , 24 , 16, 1152 , 259  , im_media_play_control_pause_hi},
    {24 , 24 , 16, 1152 , 115  , im_media_play_control_play},
    {24 , 24 , 16, 1152 , 115  , im_media_play_control_play_hi},
    {24 , 24 , 16, 1152 , 99   , im_media_play_control_stop},
    {24 , 24 , 16, 1152 , 99   , im_media_play_control_stop_hi},
    {56 , 30 , 16, 3360 , 759  , im_media_play_exit},
    {32 , 32 , 16, 2048 , 122  , im_media_play_group_arrow_l},
    {32 , 32 , 16, 2048 , 122  , im_media_play_group_arrow_r},
    {26 , 18 , 16, 936  , 475  , im_media_play_help_button_ab},
    {26 , 18 , 16, 936  , 633  , im_media_play_help_button_exit},
    {26 , 18 , 16, 936  , 259  , im_media_play_help_button_i},
    {26 , 18 , 16, 936  , 377  , im_media_play_help_button_lr},
    {26 , 18 , 16, 936  , 608  , im_media_play_help_button_mp},
    {26 , 18 , 16, 936  , 475  , im_media_play_help_button_pip},
    {26 , 18 , 16, 936  , 633  , im_media_play_help_button_swap},
    {16 , 16 , 16, 512  , 175  , im_media_play_icon_01},
    {74 , 74 , 16, 10952, 5463 , im_mediaplay_loading_01},
    {74 , 74 , 16, 10952, 5301 , im_mediaplay_loading_02},
    {74 , 74 , 16, 10952, 5379 , im_mediaplay_loading_03},
    {74 , 74 , 16, 10952, 5631 , im_mediaplay_loading_04},
    {74 , 74 , 16, 10952, 5759 , im_mediaplay_loading_05},
    {74 , 74 , 16, 10952, 5808 , im_mediaplay_loading_06},
    {74 , 74 , 16, 10952, 5524 , im_mediaplay_loading_07},
    {74 , 74 , 16, 10952, 5297 , im_mediaplay_loading_08},
    {74 , 74 , 16, 10952, 5390 , im_mediaplay_loading_09},
    {74 , 74 , 16, 10952, 5586 , im_mediaplay_loading_10},
    {74 , 74 , 16, 10952, 5732 , im_mediaplay_loading_11},
    {74 , 74 , 16, 10952, 5725 , im_mediaplay_loading_12},
    {56 , 56 , 16, 6272 , 194  , im_media_play_popwin_01_b},
    {56 , 56 , 16, 6272 , 952  , im_media_play_popwin_01_l},
    {56 , 56 , 16, 6272 , 897  , im_media_play_popwin_01_lb},
    {56 , 56 , 16, 6272 , 894  , im_media_play_popwin_01_lt},
    {56 , 56 , 16, 6272 , 1134 , im_media_play_popwin_01_r},
    {56 , 56 , 16, 6272 , 998  , im_media_play_popwin_01_rb},
    {56 , 56 , 16, 6272 , 1003 , im_media_play_popwin_01_rt},
    {56 , 56 , 16, 6272 , 194  , im_media_play_popwin_01_t},
    {12 , 12 , 16, 288  , 70   , im_media_play_rate_bg_01},
    {46 , 46 , 16, 4232 , 713  , im_mediaplay_rotate},
    {16 , 43 , 16, 1376 , 349  , im_media_play_title_line_01},
    {43 , 43 , 16, 3784 , 2358 , im_media_play_top_bg_icon_music},
    {43 , 43 , 16, 3784 , 1564 , im_media_play_top_bg_icon_music_gray},
    {43 , 43 , 16, 3784 , 1566 , im_media_play_top_bg_icon_music_s},
    {43 , 43 , 16, 3784 , 2761 , im_media_play_top_bg_icon_photo},
    {43 , 43 , 16, 3784 , 1766 , im_media_play_top_bg_icon_photo_gray},
    {43 , 43 , 16, 3784 , 1828 , im_media_play_top_bg_icon_photo_s},
    {43 , 43 , 16, 3784 , 2405 , im_media_play_top_bg_icon_record},
    {43 , 43 , 16, 3784 , 1623 , im_media_play_top_bg_icon_record_gray},
    {43 , 43 , 16, 3784 , 1733 , im_media_play_top_bg_icon_record_s},
    {43 , 43 , 16, 3784 , 2313 , im_media_play_top_bg_icon_video},
    {43 , 43 , 16, 3784 , 1701 , im_media_play_top_bg_icon_video_gray},
    {43 , 43 , 16, 3784 , 1752 , im_media_play_top_bg_icon_video_s},
    {24 , 30 , 16, 1440 , 231  , im_media_play_usbbar_l},
    {24 , 30 , 16, 1440 , 21   , im_media_play_usbbar_m},
    {24 , 30 , 16, 1440 , 231  , im_media_play_usbbar_r},
    {120, 30 , 16, 7200 , 1086 , im_motion_l_01},
    {120, 30 , 16, 7200 , 1106 , im_motion_l_02},
    {120, 30 , 16, 7200 , 1109 , im_motion_l_03},
    {120, 30 , 16, 7200 , 1110 , im_motion_l_04},
    {120, 30 , 16, 7200 , 1106 , im_motion_l_05},
    {120, 30 , 16, 7200 , 1099 , im_motion_r_01},
    {120, 30 , 16, 7200 , 1117 , im_motion_r_02},
    {120, 30 , 16, 7200 , 1120 , im_motion_r_03},
    {120, 30 , 16, 7200 , 1126 , im_motion_r_04},
    {120, 30 , 16, 7200 , 1111 , im_motion_r_05},
    {100, 100, 16, 20000, 2912 , im_mp3_bmp},
    {28 , 20 , 16, 1120 , 553  , im_mp3_icon_bmp},
    {28 , 20 , 16, 1120 , 260  , im_mp3_icon_folder},
    {28 , 20 , 16, 1120 , 445  , im_mp3_icon_jpg},
    {28 , 20 , 16, 1120 , 513  , im_mp3_icon_mem_hard_d},
    {28 , 20 , 16, 1120 , 523  , im_mp3_icon_mem_sd},
    {28 , 20 , 16, 1120 , 556  , im_mp3_icon_mem_usb},
    {28 , 20 , 16, 1120 , 549  , im_mp3_icon_mp3},
    {28 , 20 , 16, 1120 , 553  , im_mp3_icon_mpg},
    {28 , 20 , 16, 1120 , 494  , im_mp3_icon_ogg},
    {72 , 30 , 16, 4320 , 850  , im_mp3_icon_radom_folder},
    {72 , 30 , 16, 4320 , 904  , im_mp3_icon_radom_list},
    {24 , 24 , 16, 1152 , 199  , im_mp3_icon_repeat_0},
    {24 , 24 , 16, 1152 , 199  , im_mp3_icon_repeat_0_hi},
    {72 , 30 , 16, 4320 , 356  , im_mp3_icon_repeat_1},
    {72 , 30 , 16, 4320 , 613  , im_mp3_icon_repeat_folder},
    {72 , 30 , 16, 4320 , 726  , im_mp3_icon_repeat_list},
    {100, 100, 16, 20000, 4676 , im_mp3_photo},
    {100, 100, 16, 20000, 4354 , im_mp3_video},
    {20 , 20 , 16, 800  , 131  , im_mselect},
    {32 , 32 , 16, 2048 , 241  , im_multi_button_left},
    {20 , 32 , 16, 1280 , 30   , im_multi_button_middle},
    {32 , 32 , 16, 2048 , 231  , im_multi_button_right},
    {44 , 44 , 16, 3872 , 2070 , im_mute},
    {30 , 30 , 16, 1800 , 1346 , im_mute_s},
    {123, 44 , 16, 10912, 2549 , im_number},
    {84 , 30 , 16, 5040 , 1534 , im_number_s},
    {28 , 28 , 16, 1568 , 891  , im_orange_arrow_s},
    {44 , 44 , 16, 3872 , 2124 , im_pause},
    {30 , 30 , 16, 1800 , 1343 , im_pause_s},
    {64 , 52 , 16, 6656 , 3030 , im_pvr},
    {6  , 16 , 16, 192  , 113  , im_pvr_dot},
    {6  , 16 , 16, 192  , 113  , im_pvr_dot_orange},
    {24 , 24 , 16, 1152 , 291  , im_pvr_icon_backward},
    {10 , 10 , 16, 200  , 64   , im_pvr_icon_bookmark},
    {24 , 24 , 16, 1152 , 291  , im_pvr_icon_forward},
    {24 , 24 , 16, 1152 , 267  , im_pvr_icon_pause},
    {24 , 24 , 16, 1152 , 143  , im_pvr_icon_play},
    {24 , 24 , 16, 1152 , 148  , im_pvr_icon_rec},
    {10 , 10 , 16, 200  , 64   , im_pvr_icon_record},
    {24 , 24 , 16, 1152 , 331  , im_pvr_icon_slow_backward},
    {24 , 24 , 16, 1152 , 331  , im_pvr_icon_slowforward},
    {24 , 24 , 16, 1152 , 331  , im_pvr_icon_step},
    {24 , 24 , 16, 1152 , 123  , im_pvr_icon_stop},
    {10 , 10 , 16, 200  , 64   , im_pvr_icon_timeshift},
    {64 , 52 , 16, 6656 , 3273 , im_pvr_on},
    {40 , 150, 16, 12000, 3321 , im_pvr_rec_win_left},
    {40 , 150, 16, 12000, 1296 , im_pvr_rec_win_middle},
    {40 , 150, 16, 12000, 3367 , im_pvr_rec_win_right},
    {64 , 52 , 16, 6656 , 3081 , im_pvr_select},
    {58 , 58 , 16, 6728 , 3730 , im_receive},
    {24 , 24 , 16, 1152 , 667  , im_rename_pencil},
    {150, 54 , 16, 16200, 4338 , im_satellite},
    {60 , 60 , 16, 7200 , 178  , im_satellite_line_1},
    {60 , 60 , 16, 7200 , 383  , im_satellite_line_2},
    {60 , 60 , 16, 7200 , 665  , im_satellite_line_3},
    {60 , 60 , 16, 7200 , 1029 , im_satellite_line_4},
    {60 , 60 , 16, 7200 , 1500 , im_satellite_line_5},
    {12 , 12 , 16, 288  , 136  , im_scroll_bar_arrow_bottom},
    {12 , 12 , 16, 288  , 131  , im_scroll_bar_arrow_top},
    {12 , 12 , 16, 288  , 6    , im_scroll_bar_bg},
    {12 , 12 , 16, 288  , 102  , im_scroll_scroll_bottom},
    {12 , 12 , 16, 288  , 91   , im_scroll_scroll_middle},
    {12 , 12 , 16, 288  , 158  , im_scroll_scroll_top},
    {26 , 32 , 16, 1664 , 734  , im_select_enter},
    {26 , 32 , 16, 1664 , 555  , im_select_left},
    {26 , 32 , 16, 1664 , 553  , im_select_list},
    {26 , 32 , 16, 1664 , 525  , im_select_right},
    {16 , 16 , 16, 512  , 382  , im_signal_bg_left},
    {16 , 16 , 16, 512  , 386  , im_signal_bg_middle},
    {16 , 16 , 16, 512  , 394  , im_signal_bg_right},
    {12 , 12 , 16, 288  , 203  , im_signal_blue_left},
    {12 , 12 , 16, 288  , 164  , im_signal_blue_middle},
    {12 , 12 , 16, 288  , 208  , im_signal_blue_right},
    {12 , 12 , 16, 288  , 236  , im_signal_gray_left},
    {12 , 12 , 16, 288  , 221  , im_signal_gray_middle},
    {12 , 12 , 16, 288  , 239  , im_signal_gray_right},
    {12 , 12 , 16, 288  , 122  , im_signal_green_left},
    {12 , 12 , 16, 288  , 54   , im_signal_green_middle},
    {12 , 12 , 16, 288  , 118  , im_signal_green_right},
    {56 , 56 , 16, 6272 , 3726 , im_signal_no},
    {12 , 12 , 16, 288  , 64   , im_signal_orange_left},
    {12 , 12 , 16, 288  , 6    , im_signal_orange_middle},
    {12 , 12 , 16, 288  , 64   , im_signal_orange_right},
    {40 , 170, 16, 13600, 933  , im_singal_bar_e_bg},
    {40 , 170, 16, 13600, 3473 , im_singal_bar_e_left},
    {40 , 170, 16, 13600, 3525 , im_singal_bar_e_right},
    {60 , 60 , 16, 7200 , 3839 , im_slist_win_bottom},
    {60 , 60 , 16, 7200 , 1140 , im_slist_win_left},
    {60 , 60 , 16, 7200 , 3839 , im_slist_win_left_bottom},
    {60 , 60 , 16, 7200 , 2165 , im_slist_win_left_top},
    {60 , 60 , 16, 7200 , 2100 , im_slist_win_left_top_na},
    {60 , 60 , 16, 7200 , 1144 , im_slist_win_right},
    {60 , 60 , 16, 7200 , 3965 , im_slist_win_right_bottom},
    {60 , 60 , 16, 7200 , 2198 , im_slist_win_right_top},
    {60 , 60 , 16, 7200 , 2141 , im_slist_win_right_top_na},
    {60 , 60 , 16, 7200 , 1449 , im_slist_win_top},
    {28 , 6  , 16, 336  , 6    , im_spectram_02_blue_m},
    {28 , 6  , 16, 336  , 116  , im_spectram_02_blue_t},
    {28 , 4  , 16, 224  , 172  , im_spectram_blue},
    {28 , 4  , 16, 224  , 66   , im_spectram_gray},
    {28 , 6  , 16, 336  , 51   , im_spectram_red},
    {28 , 6  , 16, 336  , 123  , im_spectram_yellow},
    {64 , 52 , 16, 6656 , 3288 , im_system},
    {64 , 52 , 16, 6656 , 4603 , im_system_on},
    {64 , 52 , 16, 6656 , 4601 , im_system_select},
    {40 , 48 , 16, 3840 , 352  , im_title_bg},
    {40 , 48 , 16, 3840 , 1056 , im_title_left},
    {40 , 48 , 16, 3840 , 1056 , im_title_right},
    {64 , 52 , 16, 6656 , 2616 , im_tools},
    {64 , 52 , 16, 6656 , 4478 , im_tools_on},
    {64 , 52 , 16, 6656 , 4470 , im_tools_select},
    {22 , 22 , 16, 968  , 633  , im_tv_del},
    {22 , 22 , 16, 968  , 547  , im_tv_favorite},
    {22 , 22 , 16, 968  , 607  , im_tv_lock},
    {22 , 22 , 16, 968  , 509  , im_tv_move},
    {22 , 22 , 16, 968  , 549  , im_tv_skip},
    {80 , 60 , 16, 9600 , 4483 , im_usb},
    {32 , 44 , 16, 2816 , 1220 , im_volume},
    {32 , 44 , 16, 2816 , 324  , im_volume_bg},
    {32 , 44 , 16, 2816 , 960  , im_volume_left},
    {32 , 44 , 16, 2816 , 997  , im_volume_right},
    {40 , 40 , 16, 3200 , 206  , im_window_bottom_bg},
    {40 , 40 , 16, 3200 , 729  , im_window_bottom_right},
    {40 , 40 , 16, 3200 , 655  , im_window_left_bottom},
    {40 , 40 , 16, 3200 , 560  , im_window_middle_left},
    {40 , 40 , 16, 3200 , 640  , im_window_middle_right},
    {40 , 40 , 16, 3200 , 169  , im_window_title_bg},
    {40 , 40 , 16, 3200 , 640  , im_window_title_left},
    {40 , 40 , 16, 3200 , 707  , im_window_title_right},
    {40 , 40 , 16, 3200 , 154  , im_win_04_b},
    {40 , 40 , 16, 3200 , 414  , im_win_04_l},
    {40 , 40 , 16, 3200 , 548  , im_win_04_lb},
    {40 , 40 , 16, 3200 , 544  , im_win_04_lt},
    {40 , 40 , 16, 3200 , 400  , im_win_04_r},
    {40 , 40 , 16, 3200 , 544  , im_win_04_rb},
    {40 , 40 , 16, 3200 , 598  , im_win_04_rt},
    {40 , 40 , 16, 3200 , 182  , im_win_04_t},
    {16 , 16 , 16, 512  , 89   , im_win_05_b},
    {16 , 16 , 16, 512  , 166  , im_win_05_l},
    {16 , 16 , 16, 512  , 239  , im_win_05_lb},
    {16 , 16 , 16, 512  , 254  , im_win_05_lt},
    {16 , 16 , 16, 512  , 166  , im_win_05_r},
    {16 , 16 , 16, 512  , 240  , im_win_05_rb},
    {16 , 16 , 16, 512  , 259  , im_win_05_rt},
    {16 , 16 , 16, 512  , 121  , im_win_05_t},
    {40 , 40 , 16, 3200 , 247  , im_win_06_b},
    {40 , 40 , 16, 3200 , 764  , im_win_06_l},
    {40 , 40 , 16, 3200 , 738  , im_win_06_lb},
    {40 , 40 , 16, 3200 , 760  , im_win_06_r},
    {40 , 40 , 16, 3200 , 755  , im_win_06_rb},
    {16 , 16 , 16, 512  , 11   , im_win_07_b},
    {16 , 16 , 16, 512  , 11   , im_win_07_l},
    {16 , 16 , 16, 512  , 28   , im_win_07_lb},
    {16 , 16 , 16, 512  , 25   , im_win_07_lt},
    {16 , 16 , 16, 512  , 11   , im_win_07_r},
    {16 , 16 , 16, 512  , 25   , im_win_07_rb},
    {16 , 16 , 16, 512  , 28   , im_win_07_rt},
    {16 , 16 , 16, 512  , 11   , im_win_07_t},
    {24 , 24 , 16, 1152 , 15   , im_win_08_b},
    {24 , 24 , 16, 1152 , 15   , im_win_08_l},
    {24 , 24 , 16, 1152 , 15   , im_win_08_lb},
    {24 , 24 , 16, 1152 , 15   , im_win_08_lt},
    {24 , 24 , 16, 1152 , 15   , im_win_08_r},
    {24 , 24 , 16, 1152 , 15   , im_win_08_rb},
    {24 , 24 , 16, 1152 , 15   , im_win_08_rt},
    {24 , 24 , 16, 1152 , 15   , im_win_08_t},
    {8  , 8  , 16, 128  , 83   , im_win_pv_b},
    {8  , 8  , 16, 128  , 105  , im_win_pv_l},
    {8  , 8  , 16, 128  , 125  , im_win_pv_lb},
    {8  , 8  , 16, 128  , 125  , im_win_pv_lt},
    {8  , 8  , 16, 128  , 100  , im_win_pv_r},
    {8  , 8  , 16, 128  , 120  , im_win_pv_rb},
    {8  , 8  , 16, 128  , 121  , im_win_pv_rt},
    {8  , 8  , 16, 128  , 68   , im_win_pv_t},
    {10 , 6  , 16, 120  , 44   , im_zoom_down},
    {21 , 21 , 16, 924  , 477  , im_zoom_in},
    {6  , 10 , 16, 120  , 82   , im_zoom_left},
    {6  , 10 , 16, 120  , 87   , im_zoom_right},
    {10 , 6  , 16, 120  , 39   , im_zoom_up},
    {30 , 30 , 16, 1800 , 1347 , information_26},
    {52 , 30 , 16, 3120 , 735  , media_player_pic_06},
    {150, 40 , 16, 12000, 7935 , IM_ALI_DEMO_B16},
    {64 , 52 , 16, 6656 , 2776 , im_conax},
    {64 , 52 , 16, 6656 , 4432 , im_conax_on},
    {64 , 52 , 16, 6656 , 4430 , im_conax_select},
    {48 , 48 , 16, 4608 , 2486 , im_email_s},
    {28 , 20 , 16, 1120 , 549  , im_mp3_icon_flac},
    {64 , 52 , 16, 6656 , 2776 , im_ca},
    {64 , 52 , 16, 6656 , 4432 , im_ca_on},
    {64 , 52 , 16, 6656 , 4430 , im_ca_select},
    {28 , 20 , 16, 1120 , 549  , im_mp3_icon_wav},
};
