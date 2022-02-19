/******************************************************
mxl_api.c
----------------------------------------------------
Rf IC control functions				

<Revision History>
'11/11/24 : OKAMOTO	Correct header file name. ("mxl_user_define.h"->"mxL_user_define.h")
'11/11/24 : OKAMOTO	Update to "MxL301RF_API_ Files_V9.4.7.0".
'11/11/18 : OKAMOTO	Set 3msec wait after 14byte writing.
'11/11/16 : OKAMOTO	Set 6msec wait after 14byte writing.
'11/10/11 : OKAMOTO	Implement "MxL_PowerUpSettings".
'11/10/06 : OKAMOTO	Select AGC external or internal.
'11/10/06 : OKAMOTO	Control AGC set point.
'11/09/30 : OKAMOTO	Correct unmatched type.
'11/09/30 : OKAMOTO	Implement "MxL_agc_gain_fix".
'11/06/22 : OKAMOTO	AGC mode selection.
'11/03/16 : OKAMOTO	Select IDAC setting in "MxL_Tuner_RFTune".
'11/03/16 : OKAMOTO	Select IDAC setting.
'11/03/16 : OKAMOTO	Delete code not used.
'11/02/22 : OKAMOTO	IF out selection.
'11/02/14 : OKAMOTO	Add new MxL_ERR_MSG "MxL_ERR_UNKNOWN_ID".
'11/02/10 : OKAMOTO	Omit MxL_Tuner_PostTune. This function is not used.
'11/02/10 : OKAMOTO	Not support MxL_TunerID_MxL202RF and MxL_TunerID_MxL302RF.
'11/02/09 : OKAMOTO	Correct build error.
----------------------------------------------------
Copyright(C) 2011 SHARP CORPORATION			
******************************************************/

/*

 Driver APIs for MxLxxxRF Tuner
 
 Copyright, Maxlinear, Inc.
 All Rights Reserved
 
 File Name:      MxL_API.c
 
 Version:    9.4.7.0
*/


/*#include "StdAfx.h"*/

#include "mxl_api.h"
/* '11/11/24 : OKAMOTO	Correct header file name. ("mxl_user_define.h"->"mxL_user_define.h")*/
#include "mxL_user_define.h"
/* For customer:  include appropriate tuner driver file */
/* #include "mxl302rf.h" */
#include "mxl301rf.h"
/* #include "mxl202rf.h" */

extern UINT32 MxL_I2C_Write(UINT8 DeviceAddr, UINT8* pArray, UINT32 count, UINT32 tuner_id);
extern UINT32 MxL_I2C_Read(UINT8 DeviceAddr, UINT8 Addr, UINT8* mData, UINT32 tuner_id);

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//																		   //
//							Tuner Functions								   //
//																		   //
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
MxL_ERR_MSG MxL_Set_Register(MxLxxxRF_TunerConfigS* myTuner, UINT8 RegAddr, UINT8 RegData)
{
	UINT32 Status=0;
	UINT8 pArray[2];
	pArray[0] = RegAddr;
	pArray[1] = RegData;
	
	Status = MxL_I2C_Write((UINT8)myTuner->I2C_Addr, pArray, 2,myTuner->tuner_id);
	if(Status) return MxL_ERR_SET_REG;

	return MxL_OK;
}

MxL_ERR_MSG MxL_Get_Register(MxLxxxRF_TunerConfigS* myTuner, UINT8 RegAddr, UINT8 *RegData)
{
	if(MxL_I2C_Read((UINT8)myTuner->I2C_Addr, RegAddr, RegData,myTuner->tuner_id))
		return MxL_ERR_GET_REG;
	return MxL_OK;

}

MxL_ERR_MSG MxL_Soft_Reset(MxLxxxRF_TunerConfigS* myTuner)
{
	UINT32 Status=0;
	UINT8 reg_reset;
	reg_reset = 0xFF;
	Status = MxL_I2C_Write((UINT8)myTuner->I2C_Addr, &reg_reset, 1,myTuner->tuner_id);
	if(Status)
		return (MxL_ERR_MSG)Status;

	return MxL_OK;
}


MxL_ERR_MSG MxL_Stand_By(MxLxxxRF_TunerConfigS* myTuner)
{
	UINT8 pArray[4];	/* a array pointer that store the addr and data pairs for I2C write	*/
	
	pArray[0] = 0x01;
	pArray[1] = 0x0;
	pArray[2] = 0x13;   //START_TUNE
	pArray[3] = 0x0;    //0:abort.

	if(MxL_I2C_Write((UINT8)myTuner->I2C_Addr, pArray, 4,myTuner->tuner_id))
		return MxL_ERR_OTHERS;

	return MxL_OK;
}

/* '11/10/11 : OKAMOTO	Implement "MxL_PowerUpSettings". */
MxL_ERR_MSG MxL_PowerUpSettings(MxLxxxRF_TunerConfigS* myTuner)
{
	UINT8 pArray[2];	/* a array pointer that store the addr and data pairs for I2C write	*/

	pArray[0] = 0x01;
	pArray[1] = 0x01;

	if(MxL_I2C_Write((UINT8)myTuner->I2C_Addr, pArray, 2,myTuner->tuner_id))
		return MxL_ERR_OTHERS;

	return MxL_OK;
}

MxL_ERR_MSG MxL_Wake_Up(MxLxxxRF_TunerConfigS* myTuner)
{
#if 0
	UINT8 pArray[2];	/* a array pointer that store the addr and data pairs for I2C write	*/

	pArray[0] = 0x01;
	pArray[1] = 0x01;

	if(MxL_I2C_Write((UINT8)myTuner->I2C_Addr, pArray, 2,myTuner->tuner_id))
		return MxL_ERR_OTHERS;
#else
	/* '11/10/11 : OKAMOTO	Implement "MxL_PowerUpSettings". */
	MxL_ERR_MSG Status = MxL_PowerUpSettings(myTuner);
	if(Status!=MxL_OK)
		return Status;
#endif
	if(MxL_Tuner_RFTune(myTuner))
		return MxL_ERR_RFTUNE;

	return MxL_OK;
}

MxL_ERR_MSG MxL_Tuner_Init(MxLxxxRF_TunerConfigS* myTuner)
{	
	UINT8 pArray[MAX_ARRAY_SIZE];	/* a array pointer that store the addr and data pairs for I2C write */
	UINT32 Array_Size = 0;				/* a integer pointer that store the number of element in above array */

	UINT32 Status;

	/* Soft reset tuner */
	if(MxL_Soft_Reset(myTuner))
		return MxL_ERR_INIT;

	/* perform initialization calculation */
	if(myTuner->TunerID == MxL_TunerID_MxL202RF)
	{
		/* Uncomment section below for MxL202RF */
		/*Status = MxL202RF_Init(pArray, &Array_Size, (UINT8)myTuner->Mode, (UINT32)myTuner->Xtal_Freq,
				(UINT32)myTuner->IF_Freq, (UINT8)myTuner->IF_Spectrum, (UINT8)myTuner->ClkOut_Setting, (UINT8)myTuner->ClkOut_Amp,
				(UINT8)myTuner->Xtal_Cap, (UINT8)myTuner->BW_MHz, (UINT8)myTuner->AGC);
		if(Status) //if failure
			return (MxL_ERR_MSG)Status;
		*/

		return MxL_ERR_INIT; /* '11/02/10 : Not support MxL_TunerID_MxL202RF and MxL_TunerID_MxL302RF. */
	}
	else if (myTuner->TunerID == MxL_TunerID_MxL301RF)
	{	
		/* Uncomment section below for MxL301RF */
		Status = MxL301RF_Init(pArray, &Array_Size, (UINT8)myTuner->Mode, (UINT32)myTuner->Xtal_Freq,
				(UINT32)myTuner->IF_Freq, (UINT8)myTuner->IF_Spectrum, (UINT8)myTuner->ClkOut_Setting, (UINT8)myTuner->ClkOut_Amp,
				(UINT8)myTuner->Xtal_Cap, (UINT8)myTuner->AGC, (UINT8)myTuner->IF_Path
				
				/* '11/10/06 : OKAMOTO	Select AGC external or internal. */
				,myTuner->bInternalAgcEnable
				);
		if(Status) //if failure
			return (MxL_ERR_MSG)Status;
		
	}
	else if (myTuner->TunerID == MxL_TunerID_MxL302RF)
	{	
		/* Uncomment section below for MxL302RF */
		/* Status = MxL302RF_Init(pArray, &Array_Size, (UINT8)myTuner->Mode, (UINT32)myTuner->Xtal_Freq,
				(UINT32)myTuner->IF_Freq, (UINT8)myTuner->IF_Spectrum, (UINT8)myTuner->ClkOut_Setting, (UINT8)myTuner->ClkOut_Amp,
				(UINT8)myTuner->Xtal_Cap, (UINT8)myTuner->AGC, (UINT8)myTuner->IF_Split, (UINT8)myTuner->IF_Path);
		if(Status) //if failure
			return (MxL_ERR_MSG)Status;
		*/

		return MxL_ERR_INIT; /* '11/02/10 : Not support MxL_TunerID_MxL202RF and MxL_TunerID_MxL302RF. */
	}

	/* perform I2C write here */
	if(MxL_I2C_Write((UINT8)myTuner->I2C_Addr, pArray, Array_Size,myTuner->tuner_id))
		return MxL_ERR_INIT;

	/* '11/10/06 : OKAMOTO	Control AGC set point. */
	{
		UINT8 pArray[2];
		pArray[0] = 0x2A;
		pArray[1] = myTuner->AGC_set_point;
		if(MxL_I2C_Write((UINT8)myTuner->I2C_Addr, pArray, sizeof(pArray),myTuner->tuner_id))
			return MxL_ERR_INIT;
	}

	MxL_Delay(1);	/* 1ms delay*/

	return MxL_OK;
}


/* '11/03/16 : OKAMOTO	Select IDAC setting. */
/*====================================================*
    MxL_idac_setting
   --------------------------------------------------
    Description     IDAC setting
    Argument        INT8 DeviceAddr
    				idac_setting	(Select IDAC setting.)
    				idac_hysterisis	(Set hysterisis in auto setting.)
    Return Value	UINT32(MxL_OK:Success, Others:Fail)
 *====================================================*/
MxL_ERR_MSG MxL_idac_setting(UINT8 DeviceAddr, MXL_IDAC_SETTING idac_setting, MxL_IDAC_HYSTERISIS idac_hysterisis, UINT8 dig_idac_code, UINT32 tuner_id)
{
	switch(idac_setting){
	case MxL_IDAC_SETTING_AUTO:
		{
			UINT8 Array[] = {
				0x0D, 0x00,
				0x0C, 0x67,
				0x6F, 0x89,
				0x70, 0x0C,
				0x6F, 0x8A,
				0x70, 0x0E,
				0x6F, 0x8B,
				0x70, 0x10,
			};

			if(idac_hysterisis>=MxL_IDAC_HYSTERISIS_MAX){
				return MxL_ERR_OTHERS;
			}else{
				UINT8 ui8_idac_hysterisis;
				ui8_idac_hysterisis = (UINT8)idac_hysterisis;
				Array[15] = Array[15]+ui8_idac_hysterisis;
			}
			return (MxL_ERR_MSG)MxL_I2C_Write(DeviceAddr, Array, sizeof(Array), tuner_id);
		}
		break;
	case MxL_IDAC_SETTING_MANUAL:
		if(dig_idac_code>=63){
			return MxL_ERR_OTHERS;
		}else{
			UINT8 Array[] = {0x0D, 0x0};
			Array[1] = 0xc0 + dig_idac_code;	//DIG_ENIDAC_BYP(0x0D[7])=1, DIG_ENIDAC(0x0D[6])=1
			return (MxL_ERR_MSG)MxL_I2C_Write(DeviceAddr, Array, sizeof(Array), tuner_id);
		}
		break;
	case MxL_IDAC_SETTING_OFF:
		return (MxL_ERR_MSG)MXL301_register_write_bit_name(DeviceAddr, MxL_BIT_NAME_DIG_ENIDAC, 0, tuner_id);		//0x0D[6]	0
		break;
	default:
		return MxL_ERR_OTHERS;
	}
}

/* '11/11/24 : OKAMOTO	Correct error C2057: 定数式が必要です。 */
#define tempArray_Size	6		/* size of tempoarary array, the last registers of pArray */

//MxL_ERR_MSG MxL_Tuner_RFTune(MxLxxxRF_TunerConfigS* myTuner, UINT32 RF_Freq_Hz, MxLxxxRF_BW_MHz BWMHz)
MxL_ERR_MSG MxL_Tuner_RFTune(MxLxxxRF_TunerConfigS* myTuner)
{
	UINT8 pArray[MAX_ARRAY_SIZE] = {0};	/* a array pointer that store the addr and data pairs for I2C write */
	UINT32 Array_Size = 0;				/* a integer pointer that store the number of element in above array */

	UINT32 Status = 0;
	UINT8 Data1 = 0, Data2 = 0;
	SINT16 Data = 0;
	UINT8 i = 0;
    INT32 RF_Input_Level = 0;

	/* '11/11/24 : OKAMOTO	Update to "MxL301RF_API_ Files_V9.4.7.0" */
#if 0	/* '11/11/24 : OKAMOTO	Correct error C2057: 定数式が必要です。 */
	const UINT32 tempArray_Size = 6;		/* size of tempoarary array, the last registers of pArray */
#endif
	UINT8 ptempArray[tempArray_Size];		/* temporary array */

	//Store information into struc
//	myTuner->RF_Freq_Hz = RF_Freq_Hz;
//	myTuner->BW_MHz = BWMHz;

	/* perform Channel Change calculation */
	if(myTuner->TunerID == MxL_TunerID_MxL202RF)
	{
		/* Uncomment section below for MxL202RF */
		/*Status = MxL202RF_RFTune(pArray,&Array_Size,RF_Freq_Hz,BWMHz, myTuner->Mode); 
		if (Status)
			return MxL_ERR_RFTUNE;
		*/

		return MxL_ERR_RFTUNE; /* '11/02/10 : Not support MxL_TunerID_MxL202RF and MxL_TunerID_MxL302RF. */
	}
	else if (myTuner->TunerID == MxL_TunerID_MxL301RF)
	{	
		/* Uncomment section below for MxL301RF */
		Status = MxL301RF_RFTune(pArray,&Array_Size,myTuner->RF_Freq_Hz,myTuner->BW_MHz, myTuner->Mode);
		if (Status)
			return MxL_ERR_RFTUNE;
		
	}
	else if (myTuner->TunerID == MxL_TunerID_MxL302RF)
	{
		/* Uncomment section below for MxL302RF */	
		/* Status = MxL302RF_RFTune(pArray,&Array_Size,RF_Freq_Hz,BWMHz, myTuner->Mode, myTuner->IF_Split);
		if (Status)
			return MxL_ERR_RFTUNE;
		*/

		return MxL_ERR_RFTUNE; /* '11/02/10 : Not support MxL_TunerID_MxL202RF and MxL_TunerID_MxL302RF. */
	}

#if 0
	/* perform I2C write here */
	if(MxL_I2C_Write((UINT8)myTuner->I2C_Addr, pArray, Array_Size,myTuner->tuner_id))
		return MxL_ERR_RFTUNE;
#else
	/* '11/11/24 : OKAMOTO	Update to "MxL301RF_API_ Files_V9.4.7.0" */
	/* Copy last sets of addr./data pairs into new array - added V9.4.7.0*/
	for (i = 0; i < tempArray_Size; i ++) 
	{
		ptempArray[i] = pArray[Array_Size - tempArray_Size + i];
	}

	Array_Size = Array_Size - tempArray_Size;
	
	/* perform I2C write here */
	if(MxL_I2C_Write((UINT8)myTuner->I2C_Addr, pArray, Array_Size, myTuner->tuner_id)) 
		return MxL_ERR_RFTUNE;

	/* Added 3 ms delay along with writing last sets of addr./data pairs - added V9.4.7.0 */
	MxL_Delay(3); 
	if(MxL_I2C_Write((UINT8)myTuner->I2C_Addr, ptempArray, tempArray_Size, myTuner->tuner_id)) 
		return MxL_ERR_RFTUNE;
#endif

	MxL_Delay(1); /* Added V9.2.1.0 */

	/* Register read-back based setting for Analog M/N split mode only */ 
	if (myTuner->TunerID == MxL_TunerID_MxL302RF && myTuner->Mode == MxL_MODE_ANA_MN && myTuner->IF_Split == MxL_IF_SPLIT_ENABLE)
	{
		MxL_Get_Register(myTuner, 0xE3, &Data1);
		MxL_Get_Register(myTuner, 0xE4, &Data2);
		Data = ((Data2&0x03)<<8) + Data1;
		if(Data >= 512) 
			Data = Data - 1024;
				
		if(Data < 20)
		{
			MxL_Set_Register(myTuner, 0x85, 0x43);
			MxL_Set_Register(myTuner, 0x86, 0x08);
		}
		else if (Data >= 20)
		{
			MxL_Set_Register(myTuner, 0x85, 0x9E);
			MxL_Set_Register(myTuner, 0x86, 0x0F);
		}
		
		for(i = 0; i<Array_Size; i+=2)
		{
			if(pArray[i] == 0x11)
				Data1 = pArray[i+1];
			if(pArray[i] == 0x12)
				Data2 = pArray[i+1];
		}
		MxL_Set_Register(myTuner, 0x11, Data1);
		MxL_Set_Register(myTuner, 0x12, Data2);
	}

//	if ( myTuner->TunerID == MxL_TunerID_MxL302RF)	
	if (myTuner->TunerID == MxL_TunerID_MxL301RF || myTuner->TunerID == MxL_TunerID_MxL302RF)	
		MxL_Set_Register(myTuner, 0x13, 0x01);  //START_TUNE: start. 4.2.8 Sequencer Settings, MxL301RF_Prog_Guide_Release_9.4.4.0.pdf.

	if (myTuner->TunerID == MxL_TunerID_MxL302RF && myTuner->Mode >= MxL_MODE_ANA_MN && myTuner->IF_Split == MxL_IF_SPLIT_ENABLE)
	{
		if(MxL_Set_Register(myTuner, 0x00, 0x01))
			return MxL_ERR_RFTUNE;
	}
	MxL_Delay(30);

	if((myTuner->Mode == MxL_MODE_DVBT) || (myTuner->Mode >= MxL_MODE_ANA_MN))
	{
		if(MxL_Set_Register(myTuner, 0x1A, 0x0D))   //4.2.9 Post-Tune Settings, MxL301RF_Prog_Guide_Release_9.4.4.0.pdf.
			return MxL_ERR_SET_REG;
	}
	if (myTuner->TunerID == MxL_TunerID_MxL302RF && myTuner->Mode >= MxL_MODE_ANA_MN && myTuner->IF_Split == MxL_IF_SPLIT_ENABLE)
	{
		if(MxL_Set_Register(myTuner, 0x00, 0x00))
			return MxL_ERR_RFTUNE;
	}

	/* '11/03/16 : OKAMOTO	Select IDAC setting in "MxL_Tuner_RFTune". */
	Status = MxL_idac_setting(myTuner->I2C_Addr, myTuner->idac_setting, myTuner->idac_hysterisis, myTuner->dig_idac_code, myTuner->tuner_id);
	if(Status!=MxL_OK){
		return (MxL_ERR_MSG)Status;
	}
    
    Status = MxL_if_out_select(myTuner->I2C_Addr, MxL_IF_OUT_1, myTuner->tuner_id);
	if(Status!=MxL_OK)
		return (MxL_ERR_MSG)Status;
    Status = MxL_agc_mode_select(myTuner->I2C_Addr, MxL_AGC_MODE_1, myTuner->tuner_id);
 	if(Status!=MxL_OK)
		return (MxL_ERR_MSG)Status;
    Status = MxL_Check_RF_Input_Power(myTuner, &RF_Input_Level);
	if(Status!=MxL_OK)
		return (MxL_ERR_MSG)Status;
//    libc_printf("RF_Input_Level = %d \n", RF_Input_Level);

	return MxL_OK;
}


MxL_ERR_MSG MxL_Check_ChipVersion(MxLxxxRF_TunerConfigS* myTuner, MxLxxxRF_ChipVersion* myChipVersion)
{	
	UINT8 Data = 0;
	if(MxL_Get_Register(myTuner, 0x17, &Data))
		return MxL_GET_ID_FAIL;
		
	switch(Data & 0x0F)
	{
	case 0x09: *myChipVersion=MxL_30xRF_V9; break;
	default: 
		*myChipVersion=MxL_UNKNOWN_ID;
		return MxL_ERR_UNKNOWN_ID;
	}	

	return MxL_OK;
}

MxL_ERR_MSG MxL_RFSynth_Lock_Status(MxLxxxRF_TunerConfigS* myTuner, BOOL* isLock)
{	
	UINT8 Data = 0;
	*isLock = FALSE; 

#if 0
    UINT8 val=0;
    if(MxL_Get_Register(myTuner, 0x17, &val)) //it's normal, version&0xF == 1001(MxL301RF_V3). MxL301RF_Prog_Guide_Release_9.4.4.0.pdf, p36.
    {
        libc_printf("fail to read version \n");
    }
    libc_printf("val = 0x%x \n",val);

    if(MxL_Get_Register(myTuner, 0x00, &val))
    {
        libc_printf("fail to read IF 0x00 \n");
    }
    libc_printf("IF val = 0x%x \n",val);
    
    if(MxL_Get_Register(myTuner, 0x21, &val))
    {
        libc_printf("fail to read MxL_BIT_NAME_AGC_MODE 0x21 \n");
    }
    libc_printf("MxL_BIT_NAME_AGC_MODE = 0x%x \n",val);
    
    if(MxL_Get_Register(myTuner, 0x15, &val))
    {
        libc_printf("fail to read MxL_BIT_NAME_AGC_GAIN_FIX 0x21 \n");
    }
    libc_printf("MxL_BIT_NAME_AGC_GAIN_FIX = 0x%x \n",val);
#endif
    
    
	if(MxL_Get_Register(myTuner, 0x16, &Data))
		return MxL_ERR_OTHERS;
    /*
	Data &= 0x0C;
	if (Data == 0x0C)
		*isLock = TRUE;  // RF Synthesizer is Lock
    */
    
    //Both RF and REF Synthesizer are Locked. MxL301RF_Prog_Guide_Release_9.4.4.0.pdf, p36.
	Data &= 0x0A;
	if (Data == 0x0A)
		*isLock = TRUE;  
    
	return MxL_OK;
}

MxL_ERR_MSG MxL_REFSynth_Lock_Status(MxLxxxRF_TunerConfigS* myTuner, BOOL* isLock)
{
	UINT8 Data = 0;
	*isLock = FALSE; 
	if(MxL_Get_Register(myTuner, 0x16, &Data))
		return MxL_ERR_OTHERS;
	Data &= 0x03;
	if (Data == 0x03)
		*isLock = TRUE;   /*REF Synthesizer is Lock */
	return MxL_OK;
}

MxL_ERR_MSG MxL_Check_RF_Input_Power(MxLxxxRF_TunerConfigS* myTuner, INT32* RF_Input_Level)
{	
	UINT8 RFin1 = 0, RFin2 = 0, RFOff1 = 0, RFOff2 = 0;
	INT32 RFin = 0, RFoff = 0;
	INT32 cal_factor = 0;

    if (MxL_Set_Register(myTuner, 0x14, 0x01))	
		return MxL_ERR_SET_REG;

	MxL_Delay(1);
	if(MxL_Get_Register(myTuner, 0x18, &RFin1))  /* LSBs */
		return MxL_ERR_SET_REG; 
	if(MxL_Get_Register(myTuner, 0x19, &RFin2))  /* MSBs */
		return MxL_ERR_SET_REG;	

	if(MxL_Get_Register(myTuner, 0xD6, &RFOff1))  /* LSBs */
		return MxL_ERR_SET_REG; 
	if(MxL_Get_Register(myTuner, 0xD7, &RFOff2))  /* MSBs */
		return MxL_ERR_SET_REG;	

#if 0
	RFin = (REAL32)(((RFin2 & 0x07) << 5) + ((RFin1 & 0xF8) >> 3) + ((RFin1 & 0x07) * 0.125));
	RFoff = (REAL32)(((RFOff2 & 0x0F) << 2) + ((RFOff1 & 0xC0) >> 6) + (((RFOff1 & 0x38)>>3) * 0.125));
	if(myTuner->Mode == MxL_MODE_DVBT)
		cal_factor = 113.;
	else if(myTuner->Mode == MxL_MODE_ATSC)
		cal_factor = 109.;
	else if(myTuner->Mode == MxL_MODE_CAB_STD)
		cal_factor = 110.;
	else
		cal_factor = 107.;
#endif
	RFin = (INT32)(((RFin2 & 0x07) << 5) + ((RFin1 & 0xF8) >> 3) + (((RFin1 & 0x07) * 125)/1000) );
	RFoff = (INT32)(((RFOff2 & 0x0F) << 2) + ((RFOff1 & 0xC0) >> 6) + ((((RFOff1 & 0x38)>>3) * 125)/1000) );
	if(myTuner->Mode == MxL_MODE_DVBT)
		cal_factor = 113;
	else if(myTuner->Mode == MxL_MODE_ATSC)
		cal_factor = 109;
	else if(myTuner->Mode == MxL_MODE_CAB_STD)
		cal_factor = 110;
	else
		cal_factor = 107;

	*RF_Input_Level = RFin - RFoff - cal_factor;

	return MxL_OK;
}

/* '11/02/22 : OKAMOTO	IF out selection. */
/*====================================================*
    MxL_if_out_select
   --------------------------------------------------
    Description     Control bits for IF1/2 output enabling.
    Argument        DeviceAddr	- MxL Tuner Device address
					if_out	IF output
    Return Value	UINT32(MxL_OK:Success, Others:Fail)
 *====================================================*/
MxL_ERR_MSG MxL_if_out_select(UINT8 DeviceAddr, MxL_IF_OUT if_out, UINT32 tuner_id)
{
	UINT8 if1_off_RegData = 0;
	UINT8 if2_off_RegData = 0;
	UINT8 main_to_if2_RegData = 0;

	UINT32 Status;
	
	switch(if_out){
	case MxL_IF_OUT_1:
		if1_off_RegData = 0;
		if2_off_RegData = 1;
		main_to_if2_RegData = 0;
		break;
	case MxL_IF_OUT_2:
		if1_off_RegData = 1;
		if2_off_RegData = 0;
		main_to_if2_RegData = 1;
		break;
	default:
		return MxL_ERR_OTHERS;
	}

	Status = MXL301_register_write_bit_name(DeviceAddr, MxL_BIT_NAME_IF1_OFF, if1_off_RegData, tuner_id);
	if(Status!=MxL_OK){
		return (MxL_ERR_MSG)Status;
	}
	Status = MXL301_register_write_bit_name(DeviceAddr, MxL_BIT_NAME_IF2_OFF, if2_off_RegData, tuner_id);
	if(Status!=MxL_OK){
		return (MxL_ERR_MSG)Status;
	}
	Status = MXL301_register_write_bit_name(DeviceAddr, MxL_BIT_NAME_MAIN_TO_IF2, main_to_if2_RegData, tuner_id);
	if(Status!=MxL_OK){
		return (MxL_ERR_MSG)Status;
	}
	
	return MxL_OK;
}

/* '11/06/22 : OKAMOTO	AGC mode selection. */
/*====================================================*
    MxL_agc_mode_select
   --------------------------------------------------
    Description     External VAGC pin selection, only work for DIG_AGCSEL=1.
    Argument        DeviceAddr	- MxL Tuner Device address
					agc_mode	
					(0= Select VAGC1 (Default), 1= Select VAGC2)
    Return Value	UINT32(MxL_OK:Success, Others:Fail)
 *====================================================*/
MxL_ERR_MSG MxL_agc_mode_select(UINT8 DeviceAddr, MxL_AGC_MODE agc_mode, UINT32 tuner_id)
{
	UINT32 Status = MXL301_register_write_bit_name(DeviceAddr, MxL_BIT_NAME_AGC_MODE, agc_mode, tuner_id);
	if(Status!=MxL_OK){
		return (MxL_ERR_MSG)Status;
	}
	
	return MxL_OK;
}

/* '11/09/30 : OKAMOTO	Implement "MxL_agc_gain_fix". */
/*====================================================*
    MxL_agc_gain_fix
   --------------------------------------------------
    Description     Fix AGC gain.
    Argument        DeviceAddr	- MxL Tuner Device address
					bFixGain(TRUE:Fix gain , FALSE:Not fix)	
    Return Value	UINT32(MxL_OK:Success, Others:Fail)
 *====================================================*/
MxL_ERR_MSG MxL_agc_gain_fix(UINT8 DeviceAddr, BOOL bFixGain, UINT32 tuner_id)
{
	UINT8	RegData = 0;
	if(bFixGain){
		RegData = 3;
	}
	return (MxL_ERR_MSG)MXL301_register_write_bit_name(DeviceAddr, MxL_BIT_NAME_AGC_GAIN_FIX, RegData, tuner_id);
}
