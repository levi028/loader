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
#include "Im_MainMenu_RightWin_Bottom.h"
#include "Im_MainMenu_RightWin_Left.h"
#include "Im_MainMenu_RightWin_LeftBo.h"
#include "Im_MainMenu_RightWin_LeftTo.h"
#include "Im_MainMenu_RightWin_Right.h"
#include "Im_MainMenu_RightWin_RightB.h"
#include "Im_MainMenu_RightWin_RightT.h"
#include "Im_MainMenu_RightWin_Top.h"
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
#include "Im_MainMenu_SearchWin_03_Bottom.h"
#include "Im_MainMenu_SearchWin_03_Left.h"
#include "Im_MainMenu_SearchWin_03_LeftBottom.h"
#include "Im_MainMenu_SearchWin_03_LeftTop.h"
#include "Im_MainMenu_SearchWin_03_Right.h"
#include "Im_MainMenu_SearchWin_03_RightBottom.h"
#include "Im_MainMenu_SearchWin_03_RightTop.h"
#include "Im_MainMenu_SearchWin_03_Top.h"
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
#include "Im_Title_Bg.h"
#include "Im_Title_Left.h"
#include "Im_Title_Right.h"
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
#include "Im_Window_Bottom_Bg.h"
#include "Im_Window_Bottom_Right.h"
#include "Im_Window_LeftBottom.h"
#include "Im_Window_Middle_Left.h"
#include "Im_Window_Middle_Right.h"
#include "Im_Window_Title_Bg.h"
#include "Im_Window_Title_left.h"
#include "Im_Window_Title_Right.h"
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
#include "Im_ALI_Demo.h"

typedef struct
{
	unsigned short	w;
	unsigned short	h;
	unsigned long	bit_count;
	unsigned long	bmp_size;
	unsigned long	data_size;
	const unsigned char*	data;
}bitmap_infor_t;

bitmap_infor_t bitmaps_infor[]=
{
	{20 , 20 , 32, 1600 , 512  , Im_help_select},
	{84 , 26 , 32, 8736 , 6949 , Im_help_red},
	{84 , 26 , 32, 8736 , 7289 , Im_help_green},
	{84 , 26 , 32, 8736 , 7153 , Im_help_blue},
	{84 , 26 , 32, 8736 , 6922 , Im_help_yellow},
	{30 , 30 , 32, 3600 , 1486 , Im_help_clock},
	{84 , 26 , 32, 8736 , 7369 , Im_help_exit},
	{84 , 26 , 32, 8736 , 7279 , Im_help_fav},
	{84 , 26 , 32, 8736 , 7391 , Im_help_ok},
	{84 , 26 , 32, 8736 , 7210 , Im_help_store},
	{1280, 720, 32, 3686400, 1704904, Im_win_bg},
	{12 , 24 , 32, 1152 , 378  , Im_arrow_left},
	{12 , 24 , 32, 1152 , 378  , Im_arrow_right},
	{431, 64 , 32, 110336, 67160, Im_edit_item_sel},
	{31 , 26 , 32, 3224 , 2309 , Im_fav_icon},
	{336, 60 , 32, 80640, 52986, Im_lm_item_sel},
	{336, 60 , 32, 80640, 43759, Im_lm_item_unsel},
	{24 , 26 , 32, 2496 , 1606 , Im_lock_icon},
	{445, 238, 32, 423640, 135167, Im_mini_win_bg},
	{1280, 84 , 32, 430080, 124743, Im_mm_item_bg},
	{240, 84 , 32, 80640, 15889, Im_mm_item_sel},
	{2  , 66 , 32, 528  , 487  , Im_mm_vertical},
	{431, 2  , 32, 3448 , 2969 , Im_n_horizontal},
	{335, 133, 32, 178220, 46680, Im_lm_bg_down},
	{23 , 13 , 32, 1196 , 244  , Im_arrow_up},
	{23 , 13 , 32, 1196 , 322  , Im_arrow_down},
	{335, 486, 32, 651240, 191952, Im_lm_bg_up},
	{1280, 720, 32, 3686400, 1704904, Im_win_bg_trans},
	{28 , 31 , 32, 3472 , 1845 , Im_delete_icon},
	{30 , 31 , 32, 3720 , 993  , Im_move_icon},
	{24 , 31 , 32, 2976 , 915  , Im_skip_icon},
	{277, 63 , 32, 69804, 18442, Im_menu_logo},
	{336, 60 , 32, 80640, 46788, Im_lm_item_selected},
	{240, 84 , 32, 80640, 8179 , Im_mm_item_selected},
	{167, 80 , 32, 53440, 17575, Im_ws_progress_bg_left},
	{586, 19 , 32, 44536, 14427, Im_ws_progress_bg_top},
	{394, 298, 32, 469648, 110310, Im_ws_tvlist_bg},
	{394, 50 , 32, 78800, 39988, Im_ws_tvradio_title},
	{117, 56 , 32, 26208, 10452, Im_ws_search_bg},
	{394, 2  , 32, 3152 , 2997 , Im_srch_line},
	{394, 47 , 32, 74072, 11830, Im_ws_tvlist_item},
	{2  , 8  , 32, 64   , 35   , Im_ws_progress_unsel_bl},
	{2  , 20 , 32, 160  , 100  , Im_ws_progress_unsel_tl},
	{15 , 8  , 32, 480  , 424  , Im_ws_progress_unsel_lb},
	{15 , 8  , 32, 480  , 420  , Im_ws_progress_unsel_rb},
	{15 , 34 , 32, 2040 , 1888 , Im_ws_progress_unsel_rl},
	{15 , 34 , 32, 2040 , 1883 , Im_ws_progress_unsel_ll},
	{1  , 40 , 32, 160  , 161  , Im_ws_progress_sel},
	{310, 64 , 32, 79360, 24431, Im_sm_right_txt_down},
	{586, 64 , 32, 150016, 85851, Im_sm_left_txt_down},
	{896, 2  , 32, 7168 , 6059 , Im_sm_line},
	{6  , 14 , 32, 336  , 260  , Im_pwdialog_rt},
	{6  , 14 , 32, 336  , 261  , Im_pwdialog_lt},
	{2  , 14 , 32, 112  , 65   , Im_pwdialog_topline},
	{6  , 2  , 32, 48   , 49   , Im_pwdialog_leftline},
	{2  , 1  , 32, 8    , 5    , Im_pwdialog_buttomline},
	{6  , 2  , 32, 48   , 49   , Im_pwdialog_rightline},
	{6  , 1  , 32, 24   , 5    , Im_pwdialog_lb},
	{1  , 56 , 32, 224  , 223  , Im_weekday_bg},
	{2  , 58 , 32, 464  , 427  , Im_weekday_splitline},
	{30 , 60 , 32, 7200 , 2553 , Im_weekday_sel_left},
	{1  , 60 , 32, 240  , 239  , Im_weekday_sel_middle},
	{30 , 60 , 32, 7200 , 2553 , Im_weekday_sel_right},
	{430, 2  , 32, 3440 , 3060 , Im_sm_line2},
	{430, 56 , 32, 96320, 60848, Im_epg_item_sel},
	{30 , 31 , 32, 3720 , 1375 , Im_scrambling_flag},
	{30 , 31 , 32, 3720 , 1523 , Im_scrambling_dollar},
	{33 , 23 , 32, 3036 , 1062 , Im_mute_s},
	{14 , 16 , 32, 896  , 240  , Im_pause_s},
	{245, 56 , 32, 54880, 42845, Im_epg_item_sel_rt},
	{448, 64 , 32, 114688, 34652, Im_sm_timer_sel_right},
	{896, 2  , 32, 7168 , 6049 , Im_sm_timer_splitline},
	{1  , 62 , 32, 248  , 247  , Im_sm_navigate_bg},
//	{896, 438, 32, 1569792, 524065, Im_sm_timer_bg},
	{448, 64 , 32, 114688, 35718, Im_sm_timer_sel_left},
	{734, 1  , 32, 2936 , 2675 , Im_progname_h_split},
	{1  , 180, 32, 720  , 711  , Im_progname_v_split},
	{914, 180, 32, 658080, 234474, Im_progname_bg},
	{1  , 59 , 32, 236  , 235  , Im_progname_title_bg},
	{112, 110, 32, 49280, 26764, Im_prog_logo},
	{34 , 29 , 32, 3944 , 1434 , Im_volume_icon},
	{28 , 28 , 32, 3136 , 1067 , Im_volumebar_progress_icon},
	{6  , 28 , 32, 672  , 591  , Im_volumebar_sel_left},
	{1  , 28 , 32, 112  , 92   , Im_volumebar_sel_middle},
	{27 , 28 , 32, 3024 , 2889 , Im_volumebar_sel_right},
	{6  , 28 , 32, 672  , 596  , Im_volumebar_unsel_left},
	{1  , 28 , 32, 112  , 95   , Im_volumebar_unsel_middle},
	{6  , 28 , 32, 672  , 614  , Im_volumebar_unsel_right},
	{1  , 56 , 32, 224  , 219  , Im_popup_title_bg},
	{97 , 34 , 32, 13192, 9976 , Im_popup_button_unsel},
	{13 , 1  , 32, 52   , 53   , Im_popup_bg_leftline},
	{13 , 23 , 32, 1196 , 1122 , Im_popup_bg_lefttop},
	{13 , 1  , 32, 52   , 53   , Im_popup_bg_rightline},
	{13 , 23 , 32, 1196 , 1121 , Im_popup_bg_righttop},
	{1  , 23 , 32, 92   , 93   , Im_popup_bg_topline},
	{1  , 1  , 32, 4    , 5    , Im_popup_bg_mid},
	{97 , 33 , 32, 12804, 10555, Im_popup_button_sel},
	{1  , 27 , 32, 108  , 106  , Im_popup_bg_bottomline},
	{13 , 27 , 32, 1404 , 1371 , Im_popup_bg_leftbottom},
	{13 , 27 , 32, 1404 , 1368 , Im_popup_bg_rightbottom},
	{478, 50 , 32, 95600, 50202, Im_chan_list_sel},
	{478, 2  , 32, 3824 , 3398 , Im_chan_list_horizontal},
	{34 , 30 , 32, 4080 , 786  , Im_filelist_folder},
	{40 , 40 , 32, 6400 , 1591 , Im_N_MediaPlay_Control_Backward},
	{40 , 40 , 32, 6400 , 742  , Im_N_MediaPlay_Control_Backward_Hi},
	{40 , 40 , 32, 6400 , 1591 , Im_N_MediaPlay_Control_Forward},
	{40 , 40 , 32, 6400 , 742  , Im_N_MediaPlay_Control_Forward_Hi},
	{40 , 40 , 32, 6400 , 1415 , Im_N_MediaPlay_Control_NewBack},
	{40 , 40 , 32, 6400 , 642  , Im_N_MediaPlay_Control_NewBack_Hi},
	{40 , 40 , 32, 6400 , 1415 , Im_N_MediaPlay_Control_NewFast},
	{40 , 40 , 32, 6400 , 642  , Im_N_MediaPlay_Control_NewFast_Hi},
	{40 , 40 , 32, 6400 , 1505 , Im_N_MediaPlay_Control_Pause},
	{40 , 40 , 32, 6400 , 765  , Im_N_MediaPlay_Control_Pause_Hi},
	{40 , 40 , 32, 6400 , 1540 , Im_N_MediaPlay_Control_Play},
	{40 , 40 , 32, 6400 , 829  , Im_N_MediaPlay_Control_Play_Hi},
	{40 , 40 , 32, 6400 , 745  , Im_N_MediaPlay_Control_Stop},
	{40 , 40 , 32, 6400 , 385  , Im_N_MediaPlay_Control_Stop_Hi},
	{60 , 60 , 32, 14400, 8256 , Im_mail_gos},
	{60 , 60 , 32, 14400, 8117 , Im_mail_s},
	{34 , 25 , 32, 3400 , 945  , Im_N_information_pvr},
	{16 , 16 , 32, 1024 , 693  , Im_MediaPlay_Bar_02_Middle},
	{16 , 16 , 32, 1024 , 661  , Im_MediaPlay_RateBg_02},
	{6  , 20 , 32, 480  , 185  , Im_PVR2_Dot},
	{6  , 20 , 32, 480  , 185  , Im_PVR2_Dot_Orange}
};//Total size=5635928
