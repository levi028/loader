/*****************************************************************************
*   Copyright (C)2015 Ali Corporation. All Rights Reserved.
*
*   File:   This file contains C3505 DVBS2 specify function for specify hardware
*
*
*    Description:
*    History: 'refer to update_history.txt'
*******************************************************************************/       


#include "nim_dvbs_c3505.h"

const UINT8 c3505_map_beta_active_buf_normal[32] =              
{
	0x00,   //  						 //index 0, do not use
	0x01,   // 1/4 of QPSK  	  //1
	0x01,   // 1/3  					//2
	0x01,   // 2/5  					//3
	0x01,   // 1/2  					//4
	0x01,   // 3/5  					//5
	0x01,   // 2/3  					//6
	0x01,   // 3/4  					//7
	0x01,   // 4/5  					//8
	0x01,   // 5/6  					//9
	0x01,   // 8/9  					//a
	0x01,   // 9/10 					//b
	0x01,   // 3/5 of 8PSK  		 //c
	0x01,   // 2/3  					//d
	0x01,   // 3/4  					//e
	0x01,   // 5/6  					//f
	0x01,   // 8/9  					//10
	0x01,   // 9/10 					//11
	0x01,   // 2/3 of 16APSK		//12
	0x01,   // 3/4  					//13
	0x01,   // 4/5  					//14
	0x01,   // 5/6  					//15
	0x01,   // 8/9  					//16
	0x01,   // 9/10 					//17
	0x01,   // for 32 APSK, dont use
	0x01,   // 4/5	          			//0x19
	0x01,   // 5/6  					//0x1a
	0x01,   // 8/9  					//0x1b
	0x01,   // 9/10 					//0x1c
	0x01, 
	0x01, 
	0x01                
};

const UINT8 c3505_map_beta_active_buf_short[32] =            
{                                               
	0x00,   //  						//index                
	0x01,   // 1/4 of QPSK  			//0x1                                  
	0x01,   // 1/3  					//0x2                                   
	0x01,   // 2/5  					//0x3                                   
	0x01,   // 1/2  					//0x4                                   
	0x01,   // 3/5  					//0x5                                   
	0x01,   // 2/3  					//0x6                                   
	0x01,   // 3/4  					//0x7                                   
	0x01,   // 4/5  					//0x8                                   
	0x01,   // 5/6  					//0x9                                   
	0x01,   // 8/9  					//0xa                                   
	0x01,   // 9/10 					//0xb                                   
	0x01,   // 3/5 of 8PSK  			//0xc                                   
	0x01,   // 2/3  					//0xd                                   
	0x01,   // 3/4  					//0xe                                   
	0x01,   // 5/6  					//0xf                                   
	0x01,   // 8/9  					//0x10                                  
	0x01,   // 9/10 					//0x11                                  
	0x01,   // 2/3 of 16APSK			//0x12                                  
	0x01,   // 3/4  					//0x13                                  
	0x01,   // 4/5  					//0x14                                  
	0x01,   // 5/6  					//0x15                                  
	0x01,   // 8/9  					//0x16                                  
	0x01,   // 9/10 					//0x17                                  
	0x01,   // 3/4 of 32APSK 			//0x18                                  
	0x01,   // 4/5	          			//0x19                                  
	0x01,   // 5/6  					//0x1a                                  
	0x01,   // 8/9  					//0x1b                                  
	0x01,   // 9/10 					//0x1c                                  
	0x01,                                                        
	0x01,                                                        
	0x01                                                         
};  

const UINT8 c3505_map_beta_buf_normal[32] =                     
{
	188,   //  205,		// index 0, do not use
	188,   //  205,		// 1/4 of QPSK    //1
	190,   //  230,		// 1/3  					//2
	205,   //  205,		// 2/5  					//3
	205,   //  205,		// 1/2  					//4
	180,   //  180,		// 3/5  					//5
	160,   //  180,		// 2/3  					//6  180->175 fisher 20150723
	180,   //  180,		// 3/4  					//7
	155,   //  180,		// 4/5  					//8
	168,   //  180,		// 5/6  					//9
	150,   //  155,		// 8/9  					//a
	150,   //  155,		// 9/10 					//b

	180,   //  180,		// 3/5  					//5
	165,   //  180,		// 2/3  					//6  180->175 fisher 20150723
	170,   // 3/4  					//0xe                           
	// 160 is better from yiping sim data 2017/12/04
	160, // 180,   //  180,		// 5/6 					//7
	150,   //  155,		// 8/9  					//a
	150,   //  155,		// 9/10 					//b
	165,   // 2/3 of 16APSK			//0x12                  180->175 fisher 20150723                   
	180,   // 3/4  					//0x13                          
	180,   // 4/5  					//0x14                          
	170,   // 5/6  					//0x15                          
	140,   //  155,		// 9/10 					//11   155->150 fisher 20150723
	155,   //  180,		// 3/4  					//13
	160,   //  155,		// 5/6  					//15
	160,   //  155,		// 8/9  					//16
	140,   // 5/6  					//0x1a                  
	140,   // 8/9  					//0x1b                  
	140,   // 9/10 					//0x1c                  
	180,
	180,
	180                       
};   


const UINT8 c3505_map_beta_short_adaptive_en[32] = 
{
    0,  //  index 0, do not use                                                                                                 
    0,  //  1/4 of QPSK  
    0,  //  1/3  			
    0,  //  2/5  			
    0,  //  1/2  			
    0,  //  3/5  			
    0,  //  2/3  			
    0,  //  3/4  			
    0,  //  4/5  			
    0,  //  5/6  			
    0,  //  8/9  			
    0,  //  9/10 			                                                                                                         
    0,  //  3/5 of 8PSK		
    0,  //  2/3  			
    0,  //  3/4  			
    0,  //  3/4  			
    0,  //  8/9  			
    0,  //  9/10 			
    0,  //  2/3 of 16APSK
    0,  //  3/4  			
    0,  //  4/5  			
    0,  //  5/6  			
    0,  //  9/10 			
    0,  //  3/4  			
    0,  //  5/6  			
    0,  //  8/9  			
    0,  //  5/6  			
    0,  //  8/9  			
    0,  //  9/10 			
    0,                   
    0,                
    0
};


const UINT8 c3505_map_beta_normal_adaptive_en[32] = 
{
    0,  //  index 0, do not use                                                                                                 
    0,  //  1/4 of QPSK  
    0,  //  1/3  			
    0,  //  2/5  			
    0,  //  1/2  			
    0,  //  3/5  			
    0,  //  2/3  			
    0,  //  3/4  			
    0,  //  4/5  			
    0,  //  5/6  			
    0,  //  8/9  			
    0,  //  9/10 			                                                                                                         
    1,  //  3/5 of 8PSK		
    1,  //  2/3  			
    1,  //  3/4  			
    1,  //  5/6  			
    0,  //  8/9  			
    0,  //  9/10 			
    0,  //  2/3 of 16APSK
    0,  //  3/4  			
    0,  //  4/5  			
    0,  //  5/6  			
    0,  //  9/10 			
    0,  //  3/4  			
    0,  //  5/6  			
    0,  //  8/9  			
    0,  //  5/6  			
    0,  //  8/9  			
    0,  //  9/10 			
    0,                   
    0,                
    0
};


const UINT8 c3505_map_beta_buf_normal_extra[32][3] =                     
{
    {188, 188, 188},   //  205,		// index 0, do not use                                                                                                 
    {188, 188, 188},   //  205,		// 1/4 of QPSK    //1                                                                                                  
    {190, 190, 190},   //  230,		// 1/3  					//2                                                                                        
    {205, 205, 205},   //  205,		// 2/5  					//3                                                                                        
    {205, 205, 205},   //  205,		// 1/2  					//4                                                                                        
    {180, 180, 180},   //  180,		// 3/5  					//5                                                                                        
    {160, 160, 160},   //  180,		// 2/3  					//6  180->175 fisher 20150723                                                              
    {180, 180, 180},   //  180,		// 3/4  					//7                                                                                        
    {155, 155, 155},   //  180,		// 4/5  					//8                                                                                        
    {168, 168, 168},   //  180,		// 5/6  					//9                                                                                        
    {150, 150, 150},   //  155,		// 8/9  					//a                                                                                        
    {150, 150, 150},   //  155,		// 9/10 					//b                                                                                                                                                                                                                                           
    {180, 135, 100},   //  180,		// 3/5  					//5                                                                                        
    {165, 130, 100},   //  165,       // 2/3  					//6  180->175 fisher 20150723 , 130: paladin for impluse noise improve 20170525
    {170, 140, 100},   //  3/4  					//0xe                                                                                                      
	// 160 is better from yiping sim data 2017/12/04	
    {160, 140, 100},	 //{180, 120, 100},   //  180,		// 5/6  					//7                                                                                
    {150, 150, 150},   //  155,		// 8/9  					//a                                                                                        
    {150, 150, 150},   //  155,		// 9/10 					//b                                                                                        
    {165, 165, 165},   //  2/3 of 16APSK			//0x12                  180->175 fisher 20150723                                                           
    {180, 180, 180},   //  3/4  					//0x13                                                                                                     
    {180, 180, 180},   //  4/5  					//0x14                                                                                                     
    {170, 170, 170},   //  5/6  					//0x15                                                                                                     
    {140, 140, 140},   //  155,		// 9/10 					//11   155->150 fisher 20150723                                                            
    {155, 155, 155},   //  180,		// 3/4  					//13                                                                                       
    {160, 160, 160},   //  155,		// 5/6  					//15                                                                                       
    {160, 160, 160},   //  155,		// 8/9  					//16                                                                                       
    {140, 140, 140},   //  5/6  					//0x1a                                                                                                     
    {140, 140, 140},   //  8/9  					//0x1b                                                                                                     
    {140, 140, 140},   //  9/10 					//0x1c                                                                                                     
    {180, 180, 180},                                                                                                                                       
    {180, 180, 180},                                                                                                                                       
    {180, 180, 180},
};   

const UINT16 c3505_map_beta_normal_snr_thres[32][2] =                     
{
    {5000, 10000},  //  index 0, do not use                                                                                                 
    {5000, 10000},  //  1/4 of QPSK  
    {5000, 10000},  //  1/3  			
    {5000, 10000},  //  2/5  			
    {5000, 10000},  //  1/2  			
    {5000, 10000},  //  3/5  			
    {5000, 10000},  //  2/3  			
    {5000, 10000},  //  3/4  			
    {5000, 10000},  //  4/5  			
    {5000, 10000},  //  5/6  			
    {5000, 10000},  //  8/9  			
    {5000, 10000},  //  9/10 			                                                                                                         
    {630,  730},  //  3/5 of 8PSK		
    //800,  900,  //  2/3  		
    {750,  850},  //  2/3  	
    {860,  960},  //  3/4  			
    {960,  1060},  //  5/6  			
    {5000, 10000},  //  8/9  			
    {5000, 10000},  //  9/10 			
    {5000, 10000},  //  2/3 of 16APSK
    {5000, 10000},  //  3/4  			
    {5000, 10000},  //  4/5  			
    {5000, 10000},  //  5/6  			
    {5000, 10000},  //  9/10 			
    {5000, 10000},  //  3/4  			
    {5000, 10000},  //  5/6  			
    {5000, 10000},  //  8/9  			
    {5000, 10000},  //  5/6  			
    {5000, 10000},  //  8/9  			
    {5000, 10000},  //  9/10 			
    {5000, 10000},                   
    {5000, 10000},                
    {5000, 10000},                 
}; 

const UINT8 c3505_map_beta_buf_short[32] =       
{                                                  
	160,   //  155,		// 9/10 					//17
	150,   // 1/4 of QPSK  			//0x1                               
	140,   // 1/3  					//0x2                               
	145,   //5/6
	140,   // 1/2  					//0x4                               
	140,   // 3/5  					//0x5                               
	135,   // 8/9
	125,   // 3/4  					//0x7                               
	120,   // 4/5  					//0x8                               
	115,   // 5/6  					//0x9                               
	115,   // 8/9  					//0xa                               
	115,   // 9/10 					//0xb                               
	115,   // 3/5 of 8PSK  			//0xc                               
	125,   // 2/3  					//0xd                               
	130,   // 9/10
	135, 
	115,   // 8/9  					//0x10                              
	115,   // 9/10 					//0x11                              
	125,   // 2/3 of 16APSK			//0x12                              
	120,   // 3/4  					//0x13                              
	125,   // 4/5  					//0x14                              
	115,   // 5/6  					//0x15                              
	115,   // 8/9  					//0x16                              
	115,   // 9/10 					//0x17                              
	120,   // 3/4 of 32APSK			//0x18                              
	115,   // 4/5  					//0x19                              
	125,   // 5/6  					//0x1a                              
	120,   // 8/9  					//0x1b                              
	120,   // 9/10 					//0x1c                              
	180,                                             
	180,                                             
	180                                              
};

const UINT16 c3505_demap_noise_normal[32] =                     
{
	0x136a,   //  						//index                        
	0x16b,  	// 1/4 of QPSK  		//1
	0x1d5,  	// 1/3  					//2
	0x246,  	// 2/5  					//3
	0x311,  	// 1/2  					//4
	0x413,  	// 3/5  					//5
	0x4fa,  	// 2/3  					//6
	0x62b,  	// 3/4  					//7
	0x729,  	// 4/5  					//8
	0x80c,  	// 5/6  					//9
	0xa2a,  	// 8/9  					//a
	0xab2,  	// 9/10 					//b
	0x8a9,  	// 3/5 of 8PSK  		 //c
	0xb31,  	// 2/3  					//d
	0xf1d,   // 3/4 					 //e
	0x1501, 	// 5/6  					//f
	0x1ca5, 	  // 8/9					  //10
	0x1e91, 	  // 9/10   				  //11
	0x133b, 	  // 2/3 of 16APSK  	  //12
	0x199a, 	  // 3/4					  //13
	0x1f08, 	  // 4/5					  //14
	0x234f, 	  // 5/6					  //15
	0x2fa1, 	  // 8/9					  //16
	0x3291, 	   // 9/10  				   //17
	0x2d83,   // 3/4 of 32APSK			//0x18                        
	0x381c,   // 4/5  					//0x19       
	0x4104,   // 5/6  					//0x1a       
	0x5b06,   // 8/9  					//0x1b       
	0x6186,   // 9/10 					//0x1c       
	0x00, 0x00, 0x00                
}; 

const UINT16 c3505_demap_noise_short[32] =          
{
	0x136a,   //  						//index                           
	0x113,    // 1/4 of QPSK  			//0x1                         
	0x1da,    // 1/3  					//0x2                         
	0x246,    // 2/5  					//0x3                         
	0x2C3,    // 1/2  					//0x4                         
	0x456,    // 3/5  					//0x5                         
	0x524,    // 2/3  					//0x6                         
	0x671,    // 3/4  					//0x7                         
	0x76f,    // 4/5  					//0x8                         
	0x85c,    // 5/6  					//0x9                         
    0xab2,    // 8/9  					//0xa                         
	0x00,   	 // for 32 APSK, dont use
	0x965,    // 3/5 of 8PSK  			//0xc                         
	0xb79,    // 2/3  					//0xd                         
	0xfa2,    // 3/4  					//0xe                         
	0x1643,   // 5/6  					//0xf                         
	0x1e1e,   // 8/9  					//0x10                        
	0x00, 	        					//0x11                        
	0x142d,   // 2/3 of 16APSK			//0x12                        
	0x1ac5,   // 3/4  					//0x13                        
	0x2000,   // 4/5  					//0x14                        
	0x25ed,   // 5/6  					//0x15                        
	0x3483,   // 8/9  					//0x16                        
    0x00, 	        					//0x17                        
	0x3159,   // 3/4 of 32APSK			//0x18                    
	0x3c3c,   // 4/5  					//0x19                    
	0x456c,   // 5/6  					//0x1a                        
	0x5d17,   // 8/9  					//0x1b                        
    0x00,               				//0x1c                        
	0x00, 0x00, 0x00                                   
};

const UINT8 c3505_snr_tab[177] =
{
	0, 1, 3, 4, 5, 7, 8,\
	9, 10, 11, 13, 14, 15, 16, 17,\
	18, 19, 20, 21, 23, 24, 25, 26,\
	26, 27, 28, 29, 30, 31, 32, 33,\
	34, 35, 35, 36, 37, 38, 39, 39,\
	40, 41, 42, 42, 43, 44, 45, 45,\
	46, 47, 47, 48, 49, 49, 50, 51,\
	51, 52, 52, 53, 54, 54, 55, 55,\
	56, 57, 57, 58, 58, 59, 59, 60,\
	61, 61, 62, 62, 63, 63, 64, 64,\
	65, 66, 66, 67, 67, 68, 68, 69,\
	69, 70, 70, 71, 72, 72, 73, 73,\
	74, 74, 75, 76, 76, 77, 77, 78,\
	79, 79, 80, 80, 81, 82, 82, 83,\
	84, 84, 85, 86, 86, 87, 88, 89,\
	89, 90, 91, 92, 92, 93, 94, 95,\
	96, 96, 97, 98, 99, 100, 101, 102,\
	102, 103, 104, 105, 106, 107, 108, 109,\
	110, 111, 113, 114, 115, 116, 117, 118,\
	119, 120, 122, 123, 124, 125, 127, 128,\
	129, 131, 132, 133, 135, 136, 138, 139,\
	141, 142, 144, 145, 147, 148, 150, 152,\
	153, 155
};

const UINT8 c3505_snr_dbx10_tab[SNR_TAB_SIZE] = 
{
	30, 40, 50,60, 65, 70, 75, 80, 85, 90, \
	95, 100, 105,110,115,120,140,160,180
};
// 1st column : >=thr, switch to lower snr, also use for initial search
// 2nd column: <=thr, switch to high snr
const UINT16 c3505_snr_thr[SNR_TAB_SIZE*2] = {
	                    956, 1060, // 3
					    837, 893, // 4
					    713, 775, // 5
					    690, 725, // 6,  8SPK start**
	      				630, 677, // 6.5	
	      				573, 615, // 7		
	      				520, 559, // 7.5  
	      				470, 508, // 8    
	      				426, 460, // 8.5  
	      				386, 415, // 9    // 16 APSK start
	      				347, 376, // 9.5  
	      				313, 339, // 10   
	      				281, 303, // 10.5 //
	      				253, 275,  // 11  
	      				228, 246,  // 11.5
	      				205, 222, // 12   **
	      				113, 144, // 14
	      				74, 91, // 16
	      				0, 60 // 18
					};

// head gain have 3 bit, in SW Tab, using 4bit for better understanding
//{IRS[4:0],  PRS[3:0], HEAD_GAIN[3:0],FRAC[3:0],}
// for 8PSK 3/5  add by grace
const UINT32 c3505_cr_para_8psk_3f5[PSK8_TAB_SIZE] = { 
       		0x12930,  //6
       		0x12930,  //6.5
       		0x12900,  //7
       		0x11800,  //7.5
       		0x1070a,  //8
       		0x10705,  //8.5
       		0x10705,  //9
       		0x10705,  //9.5
       		0x10705,  //10
			0x10705,  // 10.5
			0x10705,  // 11
			0x10705,  // 11.5
			0x10700,  // 12
			0x10700   // 14
		};

// for 8PSK other coderate 9/10,8/9,5/6,3/4,2/3  add by grace
const UINT32 c3505_cr_para_8psk_others[PSK8_TAB_SIZE] = { 
			0x12930, // 6
			0x12910, // 6.5
			0x12900, // 7
			0x11900, // 7.5
			0x11800, // 8
			0x11800, // 8.5
			0x1070a, // 9
			0x1070a, // 9.5
			0x10705, // 10
			0x10705,  // 10.5
			0x1070a, // 11
			0x10705, // 11.5
			0x10700, // 12
			0x10700  // 14
		};

// Begin: TAB for New ADPT Add by Hongyu //
/*
1) S2_CODE_RATE_TABLE size = 28*30 = 840 bits
index = {code_rate}, 28 index
value = { //30 bits
        S2_AWGN_NOISE,             //10 bits, EST_NOISE (MSBs) level to apply AWGN coefficients
        S2_AWGN_COEF_SET,  //3 bit, select one set from the AWGN_COEF table
        S2_PON_IRS_DELTA,  //3 bit signed, subtract from AWGN IRS when Pilot On
        S2_PON_PRS_DELTA,  //3 bit signed, subtract from AWGN PRS when Pilot On
        S2_LOOP_COEF_SET,  //3 bit, select one set from the CLIP_PED table
        S2_CLIP_PED_SET,   //3 bit, select one set from the CLIP_PED table
        S2_AVG_PED_SET,    //3 bit, select one set from the AVG_PED table
        S2_FORCE_OLD_CR,   //1 bit, only affect pilot off
        S2_LLR_SHIFT ,             //1 bit
        } 
code_rate = PL_MODCOD_TYPE[6:2]-1;

*/
const UINT16 c3505_s2_awgn_noise[28]={ // 10bits
// 
    0x1ff,//"QPSK 1/4", // 1 <=0.4dB
	0x1ff,//"QPSK 1/3", // 2
	0x1ff,//"QPSK 2/5", // 3
    0x005,//0x1ed,//"QPSK 1/2", // 4 1.7dB
	0x1bc,//0x1c3,//"QPSK 3/5", // 5 2.6dB
	0x1ff,//"QPSK 2/3", // 6 3.6dB
	0x1ff,//"QPSK 3/4", // 7 4.5dB
	0x1ff,//"QPSK 4/5", // 8 5.2dB
	0x1ff,//"QPSK 5/6", // 9 5.6dB
	0x1ff,//"QPSK 8/9", // 10
	0x1ff,//"QPSK 9/10", // 11 I set to 0x1ff because the value is small to 0x115 at the AWGN, but at the phase noise
			// when the CR is locked, the EST_NOISE is high to 0x15a, and so that the widerloop cannot be openecd.
			// and then CR will be unlocked.
			// this setting does not effect the AWGN performance.
    // symbol rate 6500below:   up
    0xcd,  //"8PSK 3/5", 6.5dB
	0xb9, //"8PSK 2/3", // 13 7.5dB
	0xa5,  //"8PSK 3/4", // 14 8.6dB
	0x90, //"8PSK 5/6", // 15 9.7dB
	0x83,//"8PSK 8/9", // 16 11.1dB phase noise
	0x83,//"8PSK 9/10", // 17 11.1dB(Phase noise)
	0x5f,//"16APSK 2/3", // 18 11.5dB
	0x59,//"16APSK 3/4", // 19 12.0dB
	0x56,//"16APSK 4/5", // 20 12.3dB0
	0x57,//"16APSK 5/6", // 21 12.2dB
	0x47,//"16APSK 8/9", // 22 13.6dB(phase noise)
	0x46,//"16APSK 9/10", // 23 13.7dB(phase noise)
	0x35,//"32APSK 3/4", // 24 14.3dB(phase noise)
	0x31,//"32APSK 4/5", // 25 15.3dB(phase noise)
	0x30,//"32APSK 5/6", // 26 15.5dB(phase noise)
	0x26,//"32APSK 8/9", // 27 16.70dB
	0x22//"32APSK 9/10", // 28 17.3dB
	};

const UINT16 c3505_s2_awgn_noise_extra[28]={ // 10bits
	// 6.5<=symbol_rate<=33M
    0x1ff,//"QPSK 1/4", // 1 <=0.4dB
	0,//"QPSK 1/3", // 2
	0,//"QPSK 2/5", // 3
    0,//x190,//"QPSK 1/2",3.0dB awgn for wider loop enable
	0x1ff,//"QPSK 3/5", // 5 same to QPSK 8/9
	0x1ff,//"QPSK 2/3", // 6
	0x1ff,//"QPSK 3/4", // 7
	0x1ff,//"QPSK 4/5", // 8
	0x1ff,//"QPSK 5/6", // 9
	0x1ff,//"QPSK 8/9", // 10
	0x1ff,//"QPSK 9/10", // 11 I set to 0x1ff because the value is small to 0x115 at the AWGN, but at the phase noise
			// when the CR is locked, the EST_NOISE is high to 0x15a, and so that the widerloop cannot be openecd.
			// and then CR will be unlocked.
			// this setting does not effect the AWGN performance.
    // symbol rate 6500below:   up
    0xc0, //"8PSK 3/5", // 12 6.5dB
	0xb0, //"8PSK 2/3", // 13 7.5dB
	0x9e, //"8PSK 3/4", // 14 8.6dB
	0x8c, //"8PSK 5/6", // 15 9.7dB
	0x7a,//"8PSK 8/9", // 16 11.1dB phase noise
	0x7a,//"8PSK 9/10", // 17 11.1dB(Phase noise)
	0x5b,//"16APSK 2/3", // 18 11.5dB
	0x55,//"16APSK 3/4", // 19 12.0dB
	0x54,//"16APSK 4/5", // 20 12.3dB0
	0x54,//"16APSK 5/6", // 21 12.2dB
	0x47,//"16APSK 8/9", // 22 13.6dB(phase noise)
	0x43,//"16APSK 9/10", // 23 14.0 awgn    //(dont use)->13.7dB(phase noise)
	0x2c,//"32APSK 3/4", // 24 14.3dB(phase noise)//modify by jalone for phase noise performance
	0x2c,//"32APSK 4/5", // 25 15.3dB(phase noise)
	0x2c,//"32APSK 5/6", // 26 15.5dB(phase noise)
	0x23,//"32APSK 8/9", // 27 16.70dB	
	0x23//"32APSK 9/10", // 28 17.3dB
	};
const UINT8 c3505_s2_awgn_coef_set[28]={//3	//3bit
	0,//"QPSK 1/4", // 1
	0,//"QPSK 1/3", // 2
	0,//"QPSK 2/5", // 3
    0x2,//0x1,//"QPSK 1/2", // 4
	0x6,//"QPSK 3/5", // 5
	0x7,//"QPSK 2/3", // 6
	0x0,//"QPSK 3/4", // 7
	0x2,//"QPSK 4/5", // 8  0x0////
	0,//"QPSK 5/6", // 9
	0,//"QPSK 8/9", // 10
	0x1,//"QPSK 9/10", // 11
	0x0,//"8PSK 3/5", // 12
	0x3,//"8PSK 2/3", // 13
	0x3,//"8PSK 3/4", // 14
	0x3,//"8PSK 5/6", // 15
	0x2,//"8PSK 8/9", // 16
	0x0,//"8PSK 9/10", // 17
	0x2,//"16APSK 2/3", // 18
	0x0,//"16APSK 3/4", // 19
	0x1,//"16APSK 4/5", // 20
	0x2,//"16APSK 5/6", // 21
	0,//"16APSK 8/9", // 22
	0x0,//"16APSK 9/10", // 23
	0,//"32APSK 3/4", // 24
	0x0,//"32APSK 4/5", // 25
	0,//"32APSK 5/6", // 26
	0x2,//"32APSK 8/9", // 27
	0x2//"32APSK 9/10", // 28
	};
const UINT8 c3505_s2_awgn_coef_set_extra[28]={//3	//3bit
	0,//"QPSK 1/4", // 1 
	0,//"QPSK 1/3", // 2
	0,//"QPSK 2/5", // 3
    0x2,//0x1,20130703//"QPSK 1/2", // 4
	0x1,//"QPSK 3/5", // 5
	0x6,//"QPSK 2/3", // 6
	0x6,//"QPSK 3/4", // 7
	0x1,//"QPSK 4/5", // 8
	0x6,//"QPSK 5/6", // 9
	0x6,//"QPSK 8/9", // 10
	0x6,//"QPSK 9/10", // 11
	0x1,//"8PSK 3/5", // 12
#ifdef IMPLUSE_NOISE_IMPROVED
	0x6,//"8PSK 2/3", // 13	// For burst noise improve by hyman 2017-04-27
#else
    0x3,//"8PSK 2/3", // 13
#endif
	0x3,//"8PSK 3/4", // 14
	0x3,//"8PSK 5/6", // 15
	0x2,//"8PSK 8/9", // 16
	0x0,//"8PSK 9/10", // 17
	0x4,//"16APSK 2/3", // 18
	0x0,//"16APSK 3/4", // 19
	0x2,//"16APSK 4/5", // 20
	0x2,//"16APSK 5/6", // 21
	0x1,//"16APSK 8/9", // 22
	0x1,//"16APSK 9/10", // 23
	0,//"32APSK 3/4", // 24
	0x0,//"32APSK 4/5", // 25
	0,//"32APSK 5/6", // 26
	0x0,//"32APSK 8/9", // 27
	0x5//"32APSK 9/10", // 28
	};
const UINT8 c3505_s2_pon_irs_delta[28]={//4	//3bit 
	0,//"QPSK 1/4", // 1
	0,//"QPSK 1/3", // 2
	0,//"QPSK 2/5", // 3
    0x0,//"QPSK 1/2", // 4
	0x7,//"QPSK 3/5", // 5
	0x1,//"QPSK 2/3", // 6
	0,//"QPSK 3/4", // 7
	0,//"QPSK 4/5", // 8
	0,//"QPSK 5/6", // 9
	0,//"QPSK 8/9", // 10
	0,//"QPSK 9/10", // 11
	0x7,//"8PSK 3/5", // 12
	0x0,//"8PSK 2/3", // 13
	0x0,//"8PSK 3/4", // 14
	0x0,//"8PSK 5/6", // 15
	0x7,//"8PSK 8/9", // 16
	0x0,//"8PSK 9/10", // 17
	0x0,//"16APSK 2/3", // 18
	0x0,//"16APSK 3/4", // 19
	0x0,//"16APSK 4/5", // 20
	0,//"16APSK 5/6", // 21
	0,//"16APSK 8/9", // 22
	0,//"16APSK 9/10", // 23
	0x7,//"32APSK 3/4", // 24
	0x7,//"32APSK 4/5", // 25
	0,//"32APSK 5/6", // 26
	0x0,//"32APSK 8/9", // 27
	0x1//"32APSK 9/10", // 28
	};
const UINT8 c3505_s2_pon_irs_delta_extra[28]={//4	//3bit 
	0,//"QPSK 1/4", // 1
	0,//"QPSK 1/3", // 2
	0,//"QPSK 2/5", // 3
    0x6,//0x0,//"QPSK 1/2", // 4
	0x1,//"QPSK 3/5", // 5
	0,//"QPSK 2/3", // 6
	0,//"QPSK 3/4", // 7
	0,//"QPSK 4/5", // 8
	0,//"QPSK 5/6", // 9
	0,//"QPSK 8/9", // 10
	0,//"QPSK 9/10", // 11
	0x7,//"8PSK 3/5", // 12
	0x0,//"8PSK 2/3", // 13
	0x0,//"8PSK 3/4", // 14
	0x7,//"8PSK 5/6", // 15
	0x7,//"8PSK 8/9", // 16
	0x0,//"8PSK 9/10", // 17
	0x7,//"16APSK 2/3", // 18
	0x7,//"16APSK 3/4", // 19
	0x7,//"16APSK 4/5", // 20
	0,//"16APSK 5/6", // 21
	0,//"16APSK 8/9", // 22
	0,//"16APSK 9/10", // 23
	0x7,//"32APSK 3/4", // 24
	0x7,//"32APSK 4/5", // 25
	0x7,//"32APSK 5/6", // 26
	0x7,//"32APSK 8/9", // 27
	0x7//"32APSK 9/10", // 28
	};

const UINT8 c3505_s2_pon_prs_delta[28]={ // 3bit 
	0,//"QPSK 1/4", // 1
	0,//"QPSK 1/3", // 2
	0,//"QPSK 2/5", // 3
    0x0,//"QPSK 1/2", // 4
	0x7,//"QPSK 3/5", // 5
	0x7,//"QPSK 2/3", // 6
	0,//"QPSK 3/4", // 7
	0,//"QPSK 4/5", // 8
	0,//"QPSK 5/6", // 9
	0,//"QPSK 8/9", // 10
	0,//"QPSK 9/10", // 11
	0x7,//"8PSK 3/5", // 12
	0x0,//"8PSK 2/3", // 13
	0x0,//"8PSK 3/4", // 14
	0x0,//"8PSK 5/6", // 15
	0x7,//"8PSK 8/9", // 16
	0x0,//"8PSK 9/10", // 17
	0x0,//"16APSK 2/3", // 18
	0x0,//"16APSK 3/4", // 19
	0x0,//"16APSK 4/5", // 20
	0,//"16APSK 5/6", // 21
	0,//"16APSK 8/9", // 22
	0,//"16APSK 9/10", // 23
	0x7,//"32APSK 3/4", // 24
	0x7,//"32APSK 4/5", // 25
	0,//"32APSK 5/6", // 26
	0x0,//"32APSK 8/9", // 27
	0x1//"32APSK 9/10", // 28
	};
const UINT8 c3505_s2_pon_prs_delta_extra[28]={ // 3bit 
	0,//"QPSK 1/4", // 1
	0,//"QPSK 1/3", // 2
	0,//"QPSK 2/5", // 3
    0x6,//0x0,//"QPSK 1/2", // 4
	0x7,//"QPSK 3/5", // 5
	0,//"QPSK 2/3", // 6
	0,//"QPSK 3/4", // 7
	0,//"QPSK 4/5", // 8
	0,//"QPSK 5/6", // 9
	0,//"QPSK 8/9", // 10
	0,//"QPSK 9/10", // 11
	0x7,//"8PSK 3/5", // 12
	0x0,//"8PSK 2/3", // 13
	0x0,//"8PSK 3/4", // 14
	0x7,//"8PSK 5/6", // 15
	0x7,//"8PSK 8/9", // 16
	0x0,//"8PSK 9/10", // 17
	0x7,//"16APSK 2/3", // 18
	0x7,//"16APSK 3/4", // 19
	0x7,//"16APSK 4/5", // 20
	0,//"16APSK 5/6", // 21
	0x1,//"16APSK 8/9", // 22
	0x7,//"16APSK 9/10", // 23
	0x7,//"32APSK 3/4", // 24
	0x7,//"32APSK 4/5", // 25
	0,//"32APSK 5/6", // 26
	0x0,//"32APSK 8/9", // 27
	0x1//"32APSK 9/10", // 28
	};
const UINT8 c3505_s2_loop_coef_set[28]={ //3//3bit 
	0,//"QPSK 1/4", // 1
	0,//"QPSK 1/3", // 2
	0,//"QPSK 2/5", // 3
    0x1,//"QPSK 1/2", // 4
	0x7,//"QPSK 3/5", // 5
	0x4,//"QPSK 2/3", // 6
	0x4,//"QPSK 3/4", // 7
	0x4,//"QPSK 4/5", // 8
	0x4,//"QPSK 5/6", // 9
	0x4,//"QPSK 8/9", // 10
	0x1,//"QPSK 9/10", // 11
	0x6,//"8PSK 3/5", // 12
	0x6,//"8PSK 2/3", // 13
	0x5,//"8PSK 3/4", // 14
	0x5,//"8PSK 5/6", // 15
	0x7,//"8PSK 8/9", // 16
	0x5,//"8PSK 9/10", // 17
	0,//"16APSK 2/3", // 18
	0,//"16APSK 3/4", // 19
	0x3,//"16APSK 4/5", // 20
	0x2,//"16APSK 5/6", // 21
	0,//"16APSK 8/9", // 22
	0x4,//"16APSK 9/10", // 23
	0x0,//"32APSK 3/4", // 24
	0,//"32APSK 4/5", // 25
	0,//"32APSK 5/6", // 26
	0,//"32APSK 8/9", // 27
	0//"32APSK 9/10", // 28
};
const UINT8 c3505_s2_loop_coef_extra[28]={ //3//3bit 
	0,//"QPSK 1/4", // 1
	0,//"QPSK 1/3", // 2
	0,//"QPSK 2/5", // 3
    0x1,//"QPSK 1/2", // 4
	0x7,//"QPSK 3/5", // 5
	0x1,//"QPSK 2/3", // 6
	0x1,//"QPSK 3/4", // 7
	0x7,//"QPSK 4/5", // 8
	0x1,//"QPSK 5/6", // 9
	0x1,//"QPSK 8/9", // 10
	0x1,//"QPSK 9/10", // 11
	0x7,//"8PSK 3/5", // 12
#ifdef IMPLUSE_NOISE_IMPROVED	
	0x2,//"8PSK 2/3", // 13	// For burst noise improve by hyman 2017-04-27
#else
    0x5,//"8PSK 2/3", // 13
#endif
	0x2,//"8PSK 3/4", // 14
	0x5,//"8PSK 5/6", // 15
	0x7,//"8PSK 8/9", // 16
	0x5,//"8PSK 9/10", // 17
	0x3,//"16APSK 2/3", // 18
	0,//"16APSK 3/4", // 19
	0,//"16APSK 4/5", // 20
	0x2,//"16APSK 5/6", // 21
	0,//"16APSK 8/9", // 22
	0x1,//"16APSK 9/10", // 23
	0x0,//"32APSK 3/4", // 24
    0x0,//"32APSK 4/5", // 25  
    0x0,//"32APSK 5/6", // 26 
	0,//"32APSK 8/9", // 27
	0x3//"32APSK 9/10", // 28
};
const UINT8 c3505_s2_clip_ped_set[28]={ //3//3bit 
	0,//"QPSK 1/4", // 1
	0,//"QPSK 1/3", // 2
	0,//"QPSK 2/5", // 3
    0x4,//"QPSK 1/2", // 4
	0x2,//"QPSK 3/5", // 5
	0x4,//"QPSK 2/3", // 6
	0x4,//"QPSK 3/4", // 7
	0x4,//"QPSK 4/5", // 8
	0x4,//"QPSK 5/6", // 9
	0x4,//"QPSK 8/9", // 10
	0x2,//"QPSK 9/10", // 11
	0x0,//"8PSK 3/5", // 12
	0,//"8PSK 2/3", // 13
	0,//"8PSK 3/4", // 14
	0x0,//"8PSK 5/6", // 15
	1,//"8PSK 8/9", // 16
	0x2,//"8PSK 9/10", // 17
	0,//"16APSK 2/3", // 18
	0,//"16APSK 3/4", // 19
	0x2,//"16APSK 4/5", // 20
	0,//"16APSK 5/6", // 21
	0,//"16APSK 8/9", // 22
	0,//"16APSK 9/10", // 23
	0x2,//"32APSK 3/4", // 24
	0,//"32APSK 4/5", // 25
	0,//"32APSK 5/6", // 26
	0x0,//"32APSK 8/9", // 27
	0//"32APSK 9/10", // 28
};
const UINT8 c3505_s2_clip_ped_set_extra[28]={ //3//3bit 
	0,//"QPSK 1/4", // 1
	0,//"QPSK 1/3", // 2
	0,//"QPSK 2/5", // 3
    0x2,//"QPSK 1/2", // 4
	0x4,//"QPSK 3/5", // 5
	0x2,//"QPSK 2/3", // 6
	0x2,//"QPSK 3/4", // 7
	0x4,//"QPSK 4/5", // 8
	0x2,//"QPSK 5/6", // 9
	0x2,//"QPSK 8/9", // 10
	0x2,//"QPSK 9/10", // 11
	0,//"8PSK 3/5", // 12
	0,//"8PSK 2/3", // 13
	0,//"8PSK 3/4", // 14
	0,//"8PSK 5/6", // 15
	1,//"8PSK 8/9", // 16
	0x2,//"8PSK 9/10", // 17
	0x0,//"16APSK 2/3", // 18
	0x2,//"16APSK 3/4", // 19
	0x2,//"16APSK 4/5", // 20
	0,//"16APSK 5/6", // 21
	0,//"16APSK 8/9", // 22
	0,//"16APSK 9/10", // 23
	0x2,//"32APSK 3/4", // 24
	0,//"32APSK 4/5", // 25
	0,//"32APSK 5/6", // 26
	0x0,//"32APSK 8/9", // 27
	0//"32APSK 9/10", // 28
};

const UINT8 c3505_s2_avg_ped_set[28]={ //3//3bit 
	0,//"QPSK 1/4", // 1
	0,//"QPSK 1/3", // 2
	0,//"QPSK 2/5", // 3
    0x7,//"QPSK 1/2", // 4
	0x5,//"QPSK 3/5", // 5
	0x5,//"QPSK 2/3", // 6
	0x5,//"QPSK 3/4", // 7
	0x5,//"QPSK 4/5", // 8
	0x5,//"QPSK 5/6", // 9
	0x5,//"QPSK 8/9", // 10
	0x7,//"QPSK 9/10", // 11
	0,//"8PSK 3/5", // 12
	0x1,//"8PSK 2/3", // 13
	0x1,//"8PSK 3/4", // 14
	0x2,//"8PSK 5/6", // 15
	0x3,//"8PSK 8/9", // 16
	0x3,//"8PSK 9/10", // 17
	0,//"16APSK 2/3", // 18
	0x2,//"16APSK 3/4", // 19
	0,//"16APSK 4/5", // 20
	0,//"16APSK 5/6", // 21
	0x2,//"16APSK 8/9", // 22
	0x2,//"16APSK 9/10", // 23
	0x5,//"32APSK 3/4", // 24
	0x5,//"32APSK 4/5", // 25
	0x5,//"32APSK 5/6", // 26
	0x5,//"32APSK 8/9", // 27
	0x5//"32APSK 9/10", // 28
};

const UINT8 c3505_s2_avg_ped_set_extra[28]={ //3//3bit 
    0,//"QPSK 1/4", // 1
    0,//"QPSK 1/3", // 2
    0,//"QPSK 2/5", // 3
    0x7,//"QPSK 1/2", // 4
    0x5,//"QPSK 3/5", // 5
    0x5,//"QPSK 2/3", // 6
    0x5,//"QPSK 3/4", // 7
    0x6,//"QPSK 4/5", // 8
    0x5,//"QPSK 5/6", // 9
    0x5,//"QPSK 8/9", // 10
    0x5,//"QPSK 9/10", // 11
    0,//"8PSK 3/5", // 12
    0x1,//"8PSK 2/3", // 13
#if (NIM_OPTR_ACM == ACM_CCM_FLAG)
    0x7,//"8PSK 3/4", // 14     // For acm pn performance by hyman
#else
    0x1,//"8PSK 3/4", // 14
#endif
    0x6,//"8PSK 5/6", // 15
    0x3,//"8PSK 8/9", // 16
    0x3,//"8PSK 9/10", // 17
    0,//"16APSK 2/3", // 18
    0x7,//"16APSK 3/4", // 19     //0x2->0x7   20150714 fisher
    0,//"16APSK 4/5", // 20
    0,//"16APSK 5/6", // 21
    0x2,//"16APSK 8/9", // 22
    0x2,//"16APSK 9/10", // 23
    0x7,//"32APSK 3/4", // 24    //0x5->0x7   20150714 fisher
    0x7,//"32APSK 4/5", // 25   //0x5->0x7   20150716 fisher
    0x7,//"32APSK 5/6", // 26  //0x5->0x7   20150716 fisher
    0x5,//"32APSK 8/9", // 27
    0x5//"32APSK 9/10", // 28
};

const UINT8 c3505_s2_force_old_cr[28]={ 
	 0,//"QPSK 1/4", // 1
	 0,//"QPSK 1/3", // 2
	 0,//"QPSK 2/5", // 3
	 0,//"QPSK 1/2", // 4
	 0,//"QPSK 3/5", // 5
	 0x1,//"QPSK 2/3", // 6
	 0,//"QPSK 3/4", // 7
	 0,//"QPSK 4/5", // 8
	 0,//"QPSK 5/6", // 9
	 0,//"QPSK 8/9", // 10
	 0,//"QPSK 9/10", // 11
	 0,//"8PSK 3/5", // 12
	 //1,//"8PSK 2/3", // 13	 // Edit by hyman 2015-09-22 
	 0,//"8PSK 2/3", // 13
	 0,//"8PSK 3/4", // 14
	 0,//"8PSK 5/6", // 15
	 1,//"8PSK 8/9", // 16
	 0,//"8PSK 9/10", // 17
	 0,//"16APSK 2/3", // 18
	 0,//"16APSK 3/4", // 19
	 0,//"16APSK 4/5", // 20
	 0,//"16APSK 5/6", // 21
	 0,//"16APSK 8/9", // 22
	 0,//"16APSK 9/10", // 23
	 0,//"32APSK 3/4", // 24
	 0,//"32APSK 4/5", // 25
	 0,//"32APSK 5/6", // 26
	 0,//"32APSK 8/9", // 27
	 0//"32APSK 9/10", // 28
};

const UINT8 c3505_s2_llr_shift[28]={ 
	0,//"QPSK 1/4", // 1
	0,//"QPSK 1/3", // 2
	0,//"QPSK 2/5", // 3
    0,//"QPSK 1/2", // 4
	0,//"QPSK 3/5", // 5
	0,//"QPSK 2/3", // 6
	0,//"QPSK 3/4", // 7
	0,//"QPSK 4/5", // 8
	0,//"QPSK 5/6", // 9
	0,//"QPSK 8/9", // 10
	0,//"QPSK 9/10", // 11
	1,//"8PSK 3/5", // 12
	1,//"8PSK 2/3", // 13
	1,//"8PSK 3/4", // 14
	1,//"8PSK 5/6", // 15
	1,//"8PSK 8/9", // 16
	1,//"8PSK 9/10", // 17
	1,//"16APSK 2/3", // 18
	1,//"16APSK 3/4", // 19
	1,//"16APSK 4/5", // 20
	1,//"16APSK 5/6", // 21
	1,//"16APSK 8/9", // 22
	1,//"16APSK 9/10", // 23
	1,//"32APSK 3/4", // 24
	1,//"32APSK 4/5", // 25
	1,//"32APSK 5/6", // 26
	1,//"32APSK 8/9", // 27
	1//"32APSK 9/10", // 28
};

// here CODE_RATE_TABLE and other 4 tables, i implement it through like below and dont define these table 
// but its subtables
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
2) AWGN_COEF_TABLE size = 8 * 17 = 136 bits  //AVG_PED and Wider_Loop and disabled for AWGN, SYMBOL_BEFORE_HEAD only affect pilot off
index = {AWGN_COEF_SET} 8 index
value = {
        IRS,                            //5 bits, IRS_TRA_S2 for AWGN
        IRS_FRACTION,           //2 bits
        PRS,                            //4 bits
        PRS_FRACTION,           //2 bits
        HEAD_PED_GAIN,          //3 bits
        SYMBOL_BEFORE_HEAD      //1 bit, only affect pilot off with AWGN SNR
        }
*/
const UINT8 c3505_symbol_before_head[8]={ 
	//						-AWGN_COEF_SET			
		1,				// 8psk 3/5;
		1,				// 8psk 2/3; 
		1,				//
		1,				//
		0,				// for DVBS
		0,				// dont use
		1,				// dotn use
		0				//
	};
const UINT8 c3505_head_ped_gain[8]={ 
		//						-AWGN_COEF_SET			
		0x1,			//  8psk 3/5;
		0x1,			//	8psk 2/3;
		0x1,				//	010
		0x1,				//	011
		0,				//  for DVBS
		0,				//	101
		0,				//	110
		0x1				//	111
	};
const UINT8 c3505_prs_fraction[8]= { 
		//						-AWGN_COEF_SET			
		0x3,//0x0				//  8psk 3/5;
		0,				//	8psk 2/3;
		0,				//	010
		0x2,///0				//	011
		0,				//  for DVBS
		0,				//	101
		0,				//	110
		0				//	111	
};
const UINT8 c3505_prs[8]= {
		//						-AWGN_COEF_SET			
		0x7,			//  8psk 3/5;
		0x9,			//	8psk 2/3;
		0x8,				//	010
		0x7,				//	011
		0x8,				//  for DVBS
		0x0,				//	101
		0x9,				//	110
		0x7				//	111
};
const UINT8 c3505_irs_fraction[8]= { 
		//						-AWGN_COEF_SET			
		0x3,	//0x0			//  8psk 3/5;
		0,				//	8psk 2/3;
		0,				//	010
		0x2,//0				//	011
		0,				//  for DVBS
		0,				//	101
		0,				//	110
		0				//	111
};
const UINT8 c3505_irs[8]= {
		//						-AWGN_COEF_SET			
		0x13,			//  8psk 3/5;
		0x12,			//	8psk 2/3;
		0x11,				//	010
		0x10,				//	011
		0x11,				//  for DVBS
		0x0,				//	101
		0x12,				//	110
		0x13				//	111
};
//----------6.5M to 33M begin--------------------------
const UINT8 c3505_symbol_before_head_extra[8]= { 
	//						-AWGN_COEF_SET			
		1,				// 8psk 3/5;
		1,				// 8psk 2/3; 
		1,				//
		1,				//
		1,				// dont use
		1,				// dont use
		1,				// dotn use
		0				//
	};
const UINT8 c3505_head_ped_gain_extra[8]= { 
		//						-AWGN_COEF_SET			
		0x1,			//  8psk 3/5;
		0x1,			//	8psk 2/3;
		0x1,				//	010
		0x1,				//	011
		0,				//  100
		0,				//	101
		0,				//	110
		0x0				//	111
	};
const UINT8 c3505_prs_fraction_extra[8]={ 
		//						-AWGN_COEF_SET			
		0x3,//0x0				//  8psk 3/5;
		0,				//	8psk 2/3;
		0,				//	010
		0x2,///0				//	011
		0,				//  100
		0,				//	101
		0,				//	110
		0				//	111	
};
const UINT8 c3505_prs_extra[8]={ 
		//						-AWGN_COEF_SET			
		0x7,			//  8psk 3/5;
		0x9,			//	8psk 2/3;
		0x8,				//	010
		0x7,				//	011
		0xa,				//  100
		0xb,				//	101
		0x9,				//	110
		0x7				//	111
};
const UINT8 c3505_irs_fraction_extra[8]={ 
		//						-AWGN_COEF_SET			
		0x3,	//0x0			//  8psk 3/5;
		0,				//	8psk 2/3;
		0,				//	010
		0x2,//0				//	011
		0,				//  100
		0,				//	101
		0,				//	110
		0				//	111
};
const UINT8 c3505_irs_extra[8]={ 
		//						-AWGN_COEF_SET			
		0x13,			//  8psk 3/5;
		0x12,			//	8psk 2/3;
		0x11,				//	010
		0x10,				//	011
		0x13,				//  100
		0x14,				//	101
		0x12,				//	110
		0x13				//	111
};

/*
3) LOOP_COEF_TABLE size = 8 * 19 = 152 bits
index = {LOOP_COEF_SET} 8 index
value = {
        HEAD_GAIN_SNR_DELTA,    //3 bits, SNR_DELTA (dB) to disable HEAD_PED_GAIN, if HEAD_PED_GAIN is big, disable it result in big bandwidth increase
        MAX_SNR_DELTA,                  //3 bits, clip SNR_DELTA so that the bandwidth doesn't become too big for high SNR_DELTA
        IRS_STEP,                               //3 bits, IRS step for each SNR_DELTA, have 2 bits fraction (same as IRS_FRACTION)
        PRS_STEP,                               //3 bits
        WIDER_SNR_DELTA,                //3 bits, SNR_DELTA to enable WIDER_LOOP, =7 means WIDER_LOOP always disabled
        WIDER_IRS_DELTA,                //2 bits
        WIDER_PRS_DELTA                 //2 bits
        }
*/
const UINT8 c3505_wider_prs_delta[8]={
//						-LOOP_COEF_SET			
		0x2,				// dont touch
		0x2,				//dont touch
		0,				//	dont touch
		0x2,				//	011
		2,				//  dont touch
		0,				//	101
		0,				//	110
		0				//	dont touch
	};
const UINT8 c3505_wider_irs_delta[8]={
//						-LOOP_COEF_SET			
		0x2,				//  dont touch
		0x2,				//	dont touch
		0,				//	dont touch
		0x2,				//	011
		2,				//  dont touch
		0,				//	101
		0,				//	110
		0				//	dont touch
	};
const UINT8 c3505_wider_snr_delta[8] = { 
//						-LOOP_COEF_SET			
		0x7,				//  dont touch
		0x0,				//dont touch
		0x7,				//	dont touch
		0x5,				//	011
		0x7,				//  dont touch
		0x7,				//	101
		0x7,				//	110
		0x7				//	dont touch
	};
const UINT8 c3505_prs_step[8] = {
//						-LOOP_COEF_SET			
		0x3,			//  dont touch
		0x0,			//	dont touch
		0x4,			//	dont touch
		0x4,			//	011 1*7
		0x0,				//  dont touch
		0x6,				//	101 6*1
		0x2,				//	110 2*3
		0x7				//	dont touch
	};
const UINT8 c3505_irs_step[8] = { 
//						-LOOP_COEF_SET			
		0x3 ,			//  dont touch
		0,			//	dont touch
		0x4,				//	dont touch
		0x4,				//	011
		0x0,				//  dont touch
		0x6,				//	101
		0x2,				//	110
		0x7				//	dont touch
	};
const UINT8 c3505_max_snr_delta[8]={
//						-LOOP_COEF_SET			
		0x1,			//  dont touch
		0x1,			//	dont touch
		0x1,				//	dont touch
		0x2,				//	011 1*7
		0x1,				//  dont touch
		0x1,				//	101 6*1
		0x3,				//	110 2*3
		0x1				//	dont touch
	};
const UINT8 c3505_head_gain_snr_delta[8]={ 
//						-LOOP_COEF_SET			
		0x4,			// dont touch
		0x0,			//	dont touch
		0x2,				//	dont touch
		0x2,				//	011
		0x2,				//  dont touch
		0x2,				//	101
		0x2,				//	110
		0x2				//	dont touch
	};
/*
4) CLIP_PED_TABLE, size = 8*23 = 184 bits
index = {CLIP_PED_SET}, 8 index
value = {
        CLIP_MULT_STEP, //5 bits, step to increase CLIP_PED1 and 2 if SNR_DELTA > 0
        CLIP_PED1_EN,   //1 bit
        CLIP_PED1,              //8 bits
        CLIP_PED2_EN,   //1 bit
        CLIP_PED2               //8 bits
        }
*/
const UINT8 c3505_clip_ped2[8]={ 
//						-CLIP_PED_SET			
			0x73,		//  8psk 3/5
			0x80,		//	8psk 2/3
			0x95,		//	010
			0x20,			//	011
			0x73,			//  100
			0,			//	101
			0,			//	110
			0			//	111
	
	};
const UINT8 c3505_clip_ped2_en[8]={
//						-CLIP_PED_SET			
			0x1,		//  8psk 3/5
			0x1,		//	8psk 2/3
			0x0,			//	010
			0x1,			//	011
			0,			//  100
			0,			//	101
			0,			//	110
			0			//	111
	
	};
const UINT8 c3505_clip_ped1[8]={
//						-CLIP_PED_SET			
			0x37,		//  8psk 3/5
			0x90,		//	8psk 2/3
			0x50,			//	8psk 3/4
			0x30,			//	011
			0x37,			//  100
			0,			//	101
			0,			//	110
			0			//	111
	
	};
const UINT8 c3505_clip_ped1_en[8]={
//						-CLIP_PED_SET			
			0x1,		//  8psk 3/5
			0x1,		//	8psk 2/3
			0x0,			//	8psk 3/4
			0x1,			//	011
			0,			//  100
			0,			//	101
			0,			//	110
			0			//	111
	
	};
 const UINT8 c3505_clip_mult_step[8]={
//						-CLIP_PED_SET			
			0x0,		//  8psk 3/5
			0x0,		//	8psk 2/3
			0,			//	010
			0x0f,			//	011
			0,			//  100
			0,			//	101
			0,			//	110
			0			//	111
	
	};

/*
5) AVG_PED_TABLE, size = 8*26 = 224 bits
index = {code_rate}, 8 index
value = {
        AVG_SNR_DELTA,  //3 bits SNR_DELTA to apply AVG_PED
        AVG_MULT_STEP,  //5 bits, step to increase AVG_PED1 and 2 if SNR_DELTA > AVG_SNR_DELTA
        AVG_PED1_EN,    //1 bit 
        AVG_PED1,               //8 bits
        AVG_PED2_en,    //1 bit
        AVG_PED2,               //8 bits
        }
*/
const UINT8 c3505_avg_ped2[8]={
//						-AVG_PED_SET			
		0x32,			//  8psk 3/5
		0x18,			//	8psk 2/3
		0x70,				//	010
		0x05,				//	011
		0x46,				//  100
		0x32,				//	101
		0,				//	110
		0				//	111
	};
const UINT8 c3505_avg_ped2_en[8]={
//						-AVG_PED_SET			
		0x1,			//  8psk 3/5
		0x1,				//	8psk 2/3
		1,				//	010
		1,				//	011
		1,				//  100
		1,				//	101
		0,				//	110
		0				//	111
	};
const UINT8 c3505_avg_ped1[8]={
//						-AVG_PED_SET			
		0x96,			//  8psk 3/5
		0x30,			//	8psk 2/3
		0xe0,				//	010
		0x0a,				//	011
		0x96,				//  100
		0x96,				//	101
		0,				//	110
		0				//	111
	};
const UINT8 c3505_avg_ped1_en[8]={
//						-AVG_PED_SET			
		0x1,			//  8psk 3/5
		0x1,			//	8psk 2/3
		1,				//	010
		1,				//	011
		1,				//  100
		1,				//	101
		0,				//	110
		0				//	111
	};
const UINT8 c3505_avg_mult_step[8]={
//						-AVG_PED_SET			
		0x0,			//  8psk 3/5
		0x0,			//	8psk 2/3
		0,				//	010
		0,				//	011
		0,				//  100
		0,				//	101
		0,				//	110
		0				//	111
	};
const UINT8 c3505_avg_snr_delta[8]={
//						-AVG_PED_SET			
		0x1,			//  8psk 3/5
		0x1,			//	8psk 2/3
		0,				//	
		0,				//	
		0,				//  
		0,				//	
		0,				//	
		0				//	
	};
/*
1) S_CODE_RATE_TABLE size = 6*30 = 180 bits
index = {code_rate}, 6 index
value = { //30 bits
        S_AWGN_NOISE,             //10 bits, EST_NOISE (MSBs) level to apply AWGN coefficients
        S_AWGN_COEF_SET,  //3 bit, select one set from the AWGN_COEF table
        S_PON_IRS_DELTA,  //3 bit signed, subtract from AWGN IRS when Pilot On
        S_PON_PRS_DELTA,  //3 bit signed, subtract from AWGN PRS when Pilot On
        S_LOOP_COEF_SET,  //3 bit, select one set from the CLIP_PED table
        S_CLIP_PED_SET,   //3 bit, select one set from the CLIP_PED table
        S_AVG_PED_SET,    //3 bit, select one set from the AVG_PED table
        S_FORCE_OLD_CR,   //1 bit, only affect pilot off
        S_LLR_SHIFT              //1 bit
        } 
code_rate = PL_MODCOD_TYPE[6:2]-1;

*/
const UINT16 c3505_s_awgn_noise[6]={// dont need to change 
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0
	};

const UINT8 c3505_s_awgn_coef_set[6]={ 
	0x2, // 4-> 2 fisher 20150730 
	0x2, // 4-> 2 fisher 20150730
	0x2, // 4-> 2 fisher 20150730
	0x2, // 4-> 2 fisher 20150730
	0x2, // 4-> 2 fisher 20150730
	0x2 // 4-> 2 fisher 20150730
	};

const UINT8 c3505_s_pon_irs_delta[6]={//dont need to change 
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0
	};

const UINT8 c3505_s_pon_prs_delta[6]={//dont need to change 
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0
	};

const UINT8 c3505_s_loop_coef_set[6]={
	0x4,
	0x4,
	0x4,
	0x4,
	0x4,
	0x4

	};

const UINT8 c3505_s_clip_ped_set[6]={
	0x2,
	0x2,
	0x2,
	0x2,
	0x2,
	0x2
	};

const UINT8 c3505_s_avg_ped_set[6]={
	0x6,
	0x6,
	0x6,
	0x6,
	0x6,
	0x6
	};

#if 0
const UINT8 c3505_s_force_old_cr[6]={ 
	0x1,
	0x1,
	0x1,
	0x1,
	0x1,
	0x1
	};
#else
const UINT8 c3505_s_force_old_cr[6]={ 
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0
	};
#endif


const UINT8 c3505_s_llr_shift[6]={ 
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0
	};

// 00: 135m, 01:96m 10:112m 11:XTAL 27
const UINT8 ADC_SAMPLE_FREQ[4] = {135, 96, 112, 27};


/*****************************************************************************
*  INT32 nim_C3505_set_32apsk_target(struct nim_device *dev)
*  
*
* Arguments:
*  Parameter1: struct nim_device *dev 
*
* Return Value: INT32
*****************************************************************************/
/******************************************************************************
* fuction describle:
*	this fuction is used to initial the value of the S2_32APSK_TARGET
*	because of test results show that perfomance of c3503 fpga is better when disabled then enabled, 
*	this function dont need to be called. 
*
*
*				 FORCE_OLD_DEMAP_AGC3 = reg_cre0[6];		init:	0
*					 DIRECT_POWER_EN= reg_creb[3],			init:	1
*
*					 S2_Q8PSK_SKIP_DATA,
*					 S2_16APSK_SKIP_DATA_PON=reg_creb[1],	init:	0
*					 S2_16APSK_SKIP_DATA_POFF= reg_creb[2], init:	0
*					 S2_32APSK_SKIP_DATA_PON= reg_cre9[4],	init:	0
*					 S2_32APSK_SKIP_DATA_POFF= reg_cre9[5], 	init:	0
*		   [6:0]   S2_16APSK_TARGET= reg_crec[6:0], 	init:	0x1d
*		   [6:0]   S2_32APSK_TARGET=reg_crea[6:0],		init:	0x1d
*		   [6:0]   S_TARGET= reg_cre7[6:0], 			init:	0x30
*		   [6:0]   S2_TARGET= reg_cre8[6:0],			init:	0x29
******************************************************************************/
INT32 nim_c3505_set_32apsk_target(struct nim_device *dev)
{
	UINT8 rdata = 0;

	//FORCE_OLD_DEMAP_AGC3
	nim_reg_read(dev,0xe0, &rdata, 1);
	rdata &= 0xbf;
	nim_reg_write(dev,0xe0, &rdata,1);

	// DIRECT_POWER_EN S2_16APSK_SKIP_DATA_PON S2_16APSK_SKIP_DATA_POFF
	nim_reg_read(dev,0xeb,&rdata,1);
	rdata &=0xf6;
	rdata |=0x08;
	nim_reg_write(dev,0xeb,&rdata,1);

	//S2_32APSK_SKIP_DATA_PON  S2_32APSK_SKIP_DATA_POFF
	nim_reg_read(dev,0xe9,&rdata,1);
	rdata &=0xcf;
	nim_reg_write(dev,0xe9,&rdata,1);

	//S2_16APSK_TARGET
	nim_reg_read(dev,0xec,&rdata,1);
	rdata |=0x1d;
	nim_reg_write(dev,0xec,&rdata,1);

	//S2_32APSK_TARGET
	nim_reg_read(dev,0xea,&rdata,1);
	rdata |=0x1d;
	nim_reg_write(dev,0xea,&rdata,1);

	// S_TARGET
	nim_reg_read(dev,0xe7,&rdata,1);
	rdata |=0x30;
	nim_reg_write(dev,0xe7,&rdata,1);

	// S2_TARGET
	nim_reg_read(dev,0xe8,&rdata,1);
	rdata |=0x29;
	nim_reg_write(dev,0xe8,&rdata,1);
	
	return SUCCESS;
}

/*****************************************************************************
* INT32 nim_C3505_set_dynamic_power(struct nim_device *dev, UINT32 *RsUbc)
* Get bit error ratio
*
* Arguments:
* Parameter1:
* Key word: power_ctrl
* Return Value: INT32
*****************************************************************************/
INT32 nim_C3505_set_dynamic_power(struct nim_device *dev, UINT8 snr)
{
	UINT8 coderate;
	UINT32 ber;
	static UINT32 ber_sum = 0;  // store the continuous ber
	static UINT32 last_ber_sum = 0;
	static UINT32 cur_ber_sum = 0;
	static UINT32 ber_thres = 0x180;
	static UINT8 cur_max_iter = 50; //  variable can not auto reset at channel change???
	static UINT8 snr_bak = 0;
	static UINT8 last_max_iter = 50;
	static int cnt3 = 0;

	if (cnt3 >= 3)
	{
		last_ber_sum = cur_ber_sum;
		cur_ber_sum = ber_sum;
		cnt3 = 0;
		ber_sum = 0;
	}
	
	nim_c3505_get_ber(dev, &ber);
	nim_c3505_reg_get_code_rate(dev, &coderate);
	ber_sum += ber;
	cnt3 ++;

	if (coderate < 0x04)	  // 1/4 rate
		ber_thres = 0x120;
	else
		ber_thres = 0x70;
	if (cur_max_iter == 50)
	{
		if (ber_sum >= ber_thres)
		{
			if (snr > snr_bak)
				snr_bak = snr;
			cur_max_iter -= 15;
		}
	}
	else if (cur_max_iter < 50)
	{
		if (((cur_ber_sum + 0x80) < last_ber_sum) || (snr > (snr_bak + 2)))
		{
			cur_max_iter += 15;
			snr_bak = 0;
			cnt3 = 0;
			ber_sum = 0;
			last_ber_sum = 0;
			cur_ber_sum = 0;
		}
		else if (ber_sum > 3 * ber_thres)
		{
			cur_max_iter -= 15;
			if ((coderate < 0x04) && (cur_max_iter < 35))
				cur_max_iter = 35;
			else if (cur_max_iter < 20)
				cur_max_iter = 20;
		}
	}

	if (cur_max_iter != last_max_iter)
	{
		NIM_PRINTF("----change cur_max_iter to %d----\n\n", cur_max_iter);
		nim_reg_write(dev, RC1_DVBS2_FEC_LDPC+1, &cur_max_iter, 1);
		last_max_iter = cur_max_iter;
	}

	return SUCCESS;
}


 /*****************************************************************************
 *   INT32 nim_c3505_set_ldpc_iter_para(struct nim_device *dev, UINT16 most_cnt, UINT8 unchg_thld0, UINT8 unchg_thld1, UINT8 pass_thld)

*
* Arguments:
*	Parameter1: struct nim_device *dev
*	Parameter2: UINT16 most_cnt
*	Parameter2: UINT8 unchg_thld0 //4 bit
*	Parameter2: UINT8 pass_thld //4 bit
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_set_ldpc_iter_para(struct nim_device *dev, UINT16 most_cnt, \
	UINT8 unchg_thld0, UINT8 unchg_thld1,UINT8 pass_thld)
{
	UINT8 data = 0, temp = 0;

	data = (UINT8) most_cnt&0xff;
	nim_reg_write(dev, RC1_DVBS2_FEC_LDPC+1, &data, 1);

	temp = (UINT8) ( (most_cnt>>8)&0x03 );
	temp <<= 4;
	nim_reg_read(dev, RC1_DVBS2_FEC_LDPC, &data, 1);
	data |= temp;
	nim_reg_write(dev, RC1_DVBS2_FEC_LDPC, &data, 1);

	nim_reg_read(dev, RC0_BIST_LDPC_REG, &data, 1);
	data = unchg_thld0;
	nim_reg_write(dev, RC0_BIST_LDPC_REG, &data, 1);

	data = (unchg_thld1&0xf << 4)|(pass_thld&0xf);
	nim_reg_write(dev, RC6_DVBS2_FEC_LDPC, &data, 1);

	return SUCCESS;
}
/*****************************************************************************
* 
 INT32 nim_C3505_set_LDPC_iter_cnt_record_last(struct nim_device *dev)
* Description: to record the last ldpc iter times
*
* Arguments:
*  Parameter1: struct nim_device *dev*  
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_set_ldpc_iter_cnt_record_last(struct nim_device *dev)
{
   UINT8 data;
   // read LDPC iteration number, maybe once, maybe the max, according to reg_crc1[3]
   nim_reg_read(dev, RC1_DVBS2_FEC_LDPC, &data, 1);
   data &= 0xf7;
   nim_reg_write(dev, RC1_DVBS2_FEC_LDPC, &data, 1);
 
   return SUCCESS;
}

/*****************************************************************************
 INT32 nim_C3505_set_LDPC_iter_cnt_record_max(struct nim_device *dev)
* Description: to record the max ldpc iter times
*
* Arguments:
*  Parameter1: struct nim_device *dev*  
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_set_ldpc_iter_cnt_record_max(struct nim_device *dev)
{
   UINT8 data;
   // read LDPC iteration number, maybe once, maybe the max, according to reg_crc1[3]
   nim_reg_read(dev, RC1_DVBS2_FEC_LDPC, &data, 1);
    data |= 0x08;
   nim_reg_write(dev, RC1_DVBS2_FEC_LDPC, &data, 1);
   return SUCCESS;
}

/*****************************************************************************
*  INT32 nim_C3505_ldpc_keep_working_enable(struct nim_device *dev)
* Description: set ldpc to work until a new frame start
*  Arguments:
*  Parameter1: struct nim_device *dev
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_ldpc_keep_working_enable(struct nim_device *dev)
{
	UINT8 data = 0;
	nim_reg_read(dev, R12d_LDPC_SEL, &data, 1);
	data |= 0x40;
	nim_reg_write(dev, R12d_LDPC_SEL, &data, 1);

	return SUCCESS;
}

/*****************************************************************************
*  INT32 nim_C3505_ldpc_keep_working_disable(struct nim_device *dev)
* Description: set ldpc back to the normal mode
*  Arguments:
*  Parameter1: struct nim_device *dev
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_ldpc_keep_working_disable(struct nim_device *dev)
{
	UINT8 data;
	nim_reg_read(dev, R12d_LDPC_SEL, &data, 1);
	data &= 0xbf;
	nim_reg_write(dev, R12d_LDPC_SEL, &data, 1);

	return SUCCESS;
}

/*****************************************************************************
*  INT32 nim_C3505_tso_soft_cbr_on (struct nim_device *dev, UINT32 bitrate)
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 bitrate
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_tso_soft_cbr_on (struct nim_device *dev, UINT32 bitrate)
{
	UINT8 data;

	if (bitrate > 512)
		bitrate = 512;
	data = (bitrate&0x0f)<<4;
	nim_reg_write(dev, RF0_HW_TSO_CTRL + 3, &data, 1);
	data = ((bitrate>>4)&0x1f) | 0x80;
	nim_reg_write(dev, RF0_HW_TSO_CTRL + 4, &data, 1);

	return SUCCESS;
}

/*****************************************************************************
*  INT32 nim_C3505_tso_soft_cbr_off (struct nim_device *dev)
* 
*
* Arguments:
*  Parameter1: struct nim_device *dev
* 
*
* Return Value: INT32
*****************************************************************************/
INT32 nim_c3505_tso_soft_cbr_off (struct nim_device *dev)
{
	UINT8 data;

	nim_reg_read(dev, RF0_HW_TSO_CTRL + 4, &data, 1);
	data = data & 0x7f;
	nim_reg_write(dev, RF0_HW_TSO_CTRL + 4, &data, 1);

	return SUCCESS;
}

/*****************************************************************************
* INT32 nim_c3505_LA_cfg(struct nim_device *dev)
* 
* Arguments:
*  
* Return Value: UINT8
*****************************************************************************/
INT32 nim_c3505_pl_sync_cfg(struct nim_device *dev, UINT8 pl_sync_type)
{
	UINT8 data = 0 ;
	
    if (1 == pl_sync_type)
    {
        // DVB-S2 PL angle update high limit for new plsync	
    	data = 0x3c;
    	//data = 0x5a; // For some tp lock speedup
    	nim_reg_write(dev,0x2e, &data, 1);

    	data = 0xc4 ;
    	nim_reg_write(dev,0x1a8, &data, 1);
    	
    	data = 0x89 ;
    	nim_reg_write(dev,0x1a9, &data, 1);
    	
    	data = 0xbb ;
    	nim_reg_write(dev,0x1aa, &data, 1);

    	data = 0x4b ;
    	nim_reg_write(dev,0x1ac, &data, 1);
    	
    // increase the search range in CCM for new plsync 
    	data = 0xff ;
    	nim_reg_write(dev,0x1b4, &data, 1);
    // Check range setting
    	data = 0xc8 ;
    	nim_reg_write(dev,0x1b7, &data, 1);
    // find flag setting for new plsync	
    	data = 0x82 ;
    	nim_reg_write(dev,0x1b9, &data, 1);
    	// For new pl
    	data = 0x4f ;
    	nim_reg_write(dev, R2A_PL_BND_CTRL, &data, 1);	
    	data = 0x2d ;
    	nim_reg_write(dev, R2A_PL_BND_CTRL + 1, &data, 1);	
    }
    else
    {
        // enable old plsync	
    	nim_reg_read(dev,0x1b6, &data, 1);
    	data |= 0x08 ;
    	nim_reg_write(dev,0x1b6, &data, 1);
    	
    	// Same with 3503, Optimize PL time band value for reduce lock time of low symbol TP 
    	data = 0x41 ;
    	nim_reg_write(dev, R2A_PL_BND_CTRL, &data, 1);	
    	data = 0x01 ;
    	nim_reg_write(dev, R2A_PL_BND_CTRL + 1, &data, 1);	
    	// end
    }

	return SUCCESS ;
}

