/******************************************************************************
* Notice:		Copyright(2014) Gospell R&D center of Chengdu 
*               		All Rights Reserved.
* File Name:		gacas_for_nsc.c
* Description:		无卡管理模块，提供无卡数据处理相关外部接口函数。
* Date:			2014-7-30
* Author:			Yuj
* History:		2014-7-20,Created
******************************************************************************/
/******************************************************************************
*                        Head File Reference Area
******************************************************************************/
#include "gos_config.h"
#ifdef GOS_CONFIG_BOOT
//#include "bootloader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#undef GACAS_U8	
#define GACAS_U8 		unsigned char

#undef GACAS_S8 
#define GACAS_S8  	char

#undef GACAS_U16
#define GACAS_U16	unsigned short

#undef GACAS_S16
#define GACAS_S16	short

#undef GACAS_U32
#define GACAS_U32	unsigned int

#undef GACAS_S32
#define GACAS_S32	int

#undef GACAS_U64
#define GACAS_U64	unsigned long long

#undef GACAS_S64
#define GACAS_S64	long long

#undef GACAS_BOOL
#define GACAS_BOOL	char

#undef GACAS_FLOAT 
#define GACAS_FLOAT 	float

#undef GACAS_RESULT
#define GACAS_RESULT long

#undef GACAS_TRUE
#define GACAS_TRUE 1

#undef GACAS_FALSE
#define GACAS_FALSE 0

#undef GACAS_SUCCESS
#define GACAS_SUCCESS 0

#undef GACAS_FAIL
#define GACAS_FAIL -1

#undef GACAS_MEMSET
#define GACAS_MEMSET	memset

#undef GACAS_MEMCPY
#define GACAS_MEMCPY	memcpy

#undef GACAS_MEMCMP
#define GACAS_MEMCMP	memcmp

#undef GACAS_SPRINTF
#define GACAS_SPRINTF  	sprintf	

#undef GACAS_SNPRINTF
#define GACAS_SNPRINTF  	snprintf	

#undef GACAS_STRCAT
#define GACAS_STRCAT	strcat

#undef GACAS_STRLEN
#define GACAS_STRLEN	strlen

#undef GACAS_STRCPY
#define GACAS_STRCPY 	strcpy

#undef GACAS_RAND
#define GACAS_RAND		rand

#undef GACAS_FUN
#define GACAS_FUN		
#undef GACAS_VAR
#define GACAS_VAR		

#undef GACAS_NULL
#define GACAS_NULL			((void *)0)			

typedef struct
{
	GACAS_U32 erk[64];     /* encryption round keys */
	GACAS_U32 drk[64];     /* decryption round keys */
	GACAS_S32 nr;             /* number of rounds */
}gacas_aes_context_t;
/* forward S-box & tables */

GACAS_VAR static GACAS_U32 gacas_aes_FSb[256];
GACAS_VAR static GACAS_U32 gacas_aes_FT0[256]; 
GACAS_VAR static GACAS_U32 gacas_aes_FT1[256]; 
GACAS_VAR static GACAS_U32 gacas_aes_FT2[256]; 
GACAS_VAR static GACAS_U32 gacas_aes_FT3[256]; 

/* reverse S-box & tables */

GACAS_VAR static GACAS_U32 gacas_aes_RSb[256];
GACAS_VAR static GACAS_U32 gacas_aes_RT0[256];
GACAS_VAR static GACAS_U32 gacas_aes_RT1[256];
GACAS_VAR static GACAS_U32 gacas_aes_RT2[256];
GACAS_VAR static GACAS_U32 gacas_aes_RT3[256];

/* round constants */

GACAS_VAR static GACAS_U32 gacas_aes_RCON[10];

/* tables generation flag */

GACAS_VAR static int gacas_aes_do_init = 1;
GACAS_VAR static int gacas_aes_KT_init = 1;

GACAS_VAR static GACAS_U32 gacas_aes_KT0[256];
GACAS_VAR static GACAS_U32 gacas_aes_KT1[256];
GACAS_VAR static GACAS_U32 gacas_aes_KT2[256];
GACAS_VAR static GACAS_U32 gacas_aes_KT3[256];

GACAS_FUN GACAS_U32 gacas_aes_gen_rotr8(GACAS_U32 x)
{
	return (((x<<24)&0xFFFFFFFF)|((x&0xFFFFFFFF)>>8));
}
GACAS_FUN GACAS_U8 gacas_aes_gen_xtime(GACAS_U8 x)
{
	return ((x<<1)^ ((x&0x80)?0x1B:0x00));
}
GACAS_FUN void gacas_aes_gen_tables( void )
{
	int i;
	GACAS_U8 x, y;
	GACAS_U8 pow[256];
	GACAS_U8 log[256];
	/* compute pow and log tables over GF(2^8) */
	for( i = 0, x = 1; i < 256; i++, x ^= gacas_aes_gen_xtime( x ) )
	{
		pow[i] = x;
		log[x] = i & 0xFF;
	}
	/* calculate the round constants */
	for( i = 0, x = 1; i < 10; i++, x = gacas_aes_gen_xtime( x ) )
	{
		gacas_aes_RCON[i] = (GACAS_U32) x << 24;
	}
	/* generate the forward and reverse S-boxes */
	gacas_aes_FSb[0x00] = 0x63;
	gacas_aes_RSb[0x63] = 0x00;

	for( i = 1; i < 256; i++ )
	{
		x = pow[255 - log[i]];

		y = x;  y = ( y << 1 ) | ( y >> 7 );
		x ^= y; y = ( y << 1 ) | ( y >> 7 );
		x ^= y; y = ( y << 1 ) | ( y >> 7 );
		x ^= y; y = ( y << 1 ) | ( y >> 7 );
		x ^= y ^ 0x63;

		gacas_aes_FSb[i] = x;
		gacas_aes_RSb[x] = i;
	}
    /* generate the forward and reverse tables */
	for( i = 0; i < 256; i++ )
	{
		x = (unsigned char) gacas_aes_FSb[i]; y = gacas_aes_gen_xtime( x );

		gacas_aes_FT0[i] =   (GACAS_U32) ( x ^ y ) ^
		( (GACAS_U32) x <<  8 ) ^
		( (GACAS_U32) x << 16 ) ^
		( (GACAS_U32) y << 24 );

		gacas_aes_FT0[i] &= 0xFFFFFFFF;

		gacas_aes_FT1[i] = gacas_aes_gen_rotr8( gacas_aes_FT0[i] );
		gacas_aes_FT2[i] = gacas_aes_gen_rotr8( gacas_aes_FT1[i] );
		gacas_aes_FT3[i] = gacas_aes_gen_rotr8( gacas_aes_FT2[i] );

		y = (unsigned char) gacas_aes_RSb[i];

		gacas_aes_RT0[i] = 	((GACAS_U32)((0x0B&&y)?pow[(log[0x0B]+log[y])% 255]:0))^
				((GACAS_U32)((0x0D&&y)?pow[(log[0x0D]+log[y])% 255]:0)<< 8)^
				((GACAS_U32)((0x09&&y)?pow[(log[0x09]+log[y])% 255]:0)<< 16)^
				((GACAS_U32)((0x0E&&y)?pow[(log[0x0E]+log[y])% 255]:0)<< 24);
		gacas_aes_RT0[i] &= 0xFFFFFFFF;

		gacas_aes_RT1[i] = gacas_aes_gen_rotr8( gacas_aes_RT0[i] );
		gacas_aes_RT2[i] = gacas_aes_gen_rotr8( gacas_aes_RT1[i] );
		gacas_aes_RT3[i] = gacas_aes_gen_rotr8( gacas_aes_RT2[i] );
	}
}

/* AES key scheduling routine */

GACAS_FUN int gacas_aes_set_key( gacas_aes_context_t *ctx, GACAS_U8 *key, int nbits )
{
	int i;
	GACAS_U32 *RK, *SK;

	if( gacas_aes_do_init )
	{
		gacas_aes_gen_tables();
		gacas_aes_do_init = 0;
	}

	switch( nbits )
	{
		case 128: ctx->nr = 10; break;
		case 192: ctx->nr = 12; break;
		case 256: ctx->nr = 14; break;
		default : return( 1 );
	}

	RK = ctx->erk;

	for( i = 0; i < (nbits >> 5); i++ )
	{
		//GET_UINT32( RK[i], key, i * 4 );
		RK[i] = ((GACAS_U32)(key)[(i*4)]<<24)|((GACAS_U32)(key)[(i*4)+1]<<16)|((GACAS_U32) (key)[(i*4)+2]<<8)|((GACAS_U32)(key)[(i*4)+3]);
	}
    	/* setup encryption round keys */
	switch( nbits )
	{
		case 128:
			for( i = 0; i < 10; i++, RK += 4 )
			{
				RK[4]  = RK[0] ^ gacas_aes_RCON[i] ^
					( gacas_aes_FSb[ (GACAS_U8) ( RK[3] >> 16 ) ] << 24 ) ^
					( gacas_aes_FSb[(GACAS_U8)( RK[3] >>  8 ) ] << 16 ) ^
					( gacas_aes_FSb[(GACAS_U8)( RK[3]       ) ] <<  8 ) ^
					( gacas_aes_FSb[(GACAS_U8)( RK[3] >> 24 ) ]       );

				RK[5]  = RK[1] ^ RK[4];
				RK[6]  = RK[2] ^ RK[5];
				RK[7]  = RK[3] ^ RK[6];
			}
			break;
		case 192:
			for( i = 0; i < 8; i++, RK += 6 )
			{
				RK[6]  = RK[0] ^ gacas_aes_RCON[i] ^
					( gacas_aes_FSb[(GACAS_U8)( RK[5] >> 16 ) ] << 24 ) ^
					( gacas_aes_FSb[(GACAS_U8)( RK[5] >>  8 ) ] << 16 ) ^
					( gacas_aes_FSb[(GACAS_U8)( RK[5]       ) ] <<  8 ) ^
					( gacas_aes_FSb[(GACAS_U8)( RK[5] >> 24 ) ]       );

				RK[7]  = RK[1] ^ RK[6];
				RK[8]  = RK[2] ^ RK[7];
				RK[9]  = RK[3] ^ RK[8];
				RK[10] = RK[4] ^ RK[9];
				RK[11] = RK[5] ^ RK[10];
			}
			break;
		case 256:
			for( i = 0; i < 7; i++, RK += 8 )
			{
				RK[8]  = RK[0] ^ gacas_aes_RCON[i] ^
				( gacas_aes_FSb[(GACAS_U8)( RK[7] >> 16 ) ] << 24 ) ^
				( gacas_aes_FSb[(GACAS_U8)( RK[7] >>  8 ) ] << 16 ) ^
				( gacas_aes_FSb[(GACAS_U8)( RK[7]       ) ] <<  8 ) ^
				( gacas_aes_FSb[(GACAS_U8)( RK[7] >> 24 ) ]       );

				RK[9]  = RK[1] ^ RK[8];
				RK[10] = RK[2] ^ RK[9];
				RK[11] = RK[3] ^ RK[10];

				RK[12] = RK[4] ^
				( gacas_aes_FSb[(GACAS_U8)( RK[11] >> 24 ) ] << 24 ) ^
				( gacas_aes_FSb[(GACAS_U8)( RK[11] >> 16 ) ] << 16 ) ^
				( gacas_aes_FSb[(GACAS_U8)( RK[11] >>  8 ) ] <<  8 ) ^
				( gacas_aes_FSb[(GACAS_U8)( RK[11]       ) ]       );

				RK[13] = RK[5] ^ RK[12];
				RK[14] = RK[6] ^ RK[13];
				RK[15] = RK[7] ^ RK[14];
			}
			break;
	}
    /* setup decryption round keys */
	if( gacas_aes_KT_init )
	{
		for( i = 0; i < 256; i++ )
		{
			gacas_aes_KT0[i] = gacas_aes_RT0[ gacas_aes_FSb[i] ];
			gacas_aes_KT1[i] = gacas_aes_RT1[ gacas_aes_FSb[i] ];
			gacas_aes_KT2[i] = gacas_aes_RT2[ gacas_aes_FSb[i] ];
			gacas_aes_KT3[i] = gacas_aes_RT3[ gacas_aes_FSb[i] ];
		}
		gacas_aes_KT_init = 0;
	}
	SK = ctx->drk;
	*SK++ = *RK++;
	*SK++ = *RK++;
	*SK++ = *RK++;
	*SK++ = *RK++;
	for( i = 1; i < ctx->nr; i++ )
	{
		RK -= 8;

		*SK++ = gacas_aes_KT0[(GACAS_U8)( *RK >> 24 ) ] ^
			gacas_aes_KT1[(GACAS_U8)( *RK >> 16 ) ] ^
			gacas_aes_KT2[(GACAS_U8)( *RK >>  8 ) ] ^
			gacas_aes_KT3[(GACAS_U8)( *RK       ) ]; RK++;

		*SK++ = gacas_aes_KT0[(GACAS_U8)( *RK >> 24 ) ] ^
			gacas_aes_KT1[(GACAS_U8)( *RK >> 16 ) ] ^
			gacas_aes_KT2[(GACAS_U8)( *RK >>  8 ) ] ^
			gacas_aes_KT3[(GACAS_U8)( *RK       ) ]; RK++;

		*SK++ = gacas_aes_KT0[(GACAS_U8)( *RK >> 24 ) ] ^
			gacas_aes_KT1[(GACAS_U8)( *RK >> 16 ) ] ^
			gacas_aes_KT2[(GACAS_U8)( *RK >>  8 ) ] ^
			gacas_aes_KT3[(GACAS_U8)( *RK       ) ]; RK++;

		*SK++ = gacas_aes_KT0[(GACAS_U8)( *RK >> 24 ) ] ^
			gacas_aes_KT1[(GACAS_U8)( *RK >> 16 ) ] ^
			gacas_aes_KT2[(GACAS_U8)( *RK >>  8 ) ] ^
			gacas_aes_KT3[(GACAS_U8)( *RK       ) ]; RK++;
	}
	RK -= 8;
	*SK++ = *RK++;
	*SK++ = *RK++;
	*SK++ = *RK++;
	*SK++ = *RK++;
    	return( 0 );
}

/* AES 128-bit block encryption routine */
GACAS_FUN void gacas_aes_fround(GACAS_U32 *RK,GACAS_U32 *X0,GACAS_U32 *X1,GACAS_U32 *X2,GACAS_U32 *X3,GACAS_U32 Y0,GACAS_U32 Y1,GACAS_U32 Y2,GACAS_U32 Y3) 
{                                           
	*X0 = RK[0]^gacas_aes_FT0[(GACAS_U8)(Y0>>24)]^gacas_aes_FT1[(GACAS_U8)(Y1>>16)]^gacas_aes_FT2[(GACAS_U8)(Y2>>8)]^gacas_aes_FT3[(GACAS_U8)(Y3)];
	*X1 = RK[1]^gacas_aes_FT0[(GACAS_U8)(Y1>>24)]^gacas_aes_FT1[(GACAS_U8)(Y2>>16)]^gacas_aes_FT2[(GACAS_U8)(Y3>>8)]^gacas_aes_FT3[(GACAS_U8)(Y0)];
	*X2 = RK[2]^gacas_aes_FT0[(GACAS_U8)(Y2>>24)]^gacas_aes_FT1[(GACAS_U8)(Y3>>16)]^gacas_aes_FT2[(GACAS_U8)(Y0>>8)]^gacas_aes_FT3[(GACAS_U8)(Y1)];
	*X3 = RK[3]^gacas_aes_FT0[(GACAS_U8)(Y3>>24)]^gacas_aes_FT1[(GACAS_U8)(Y0>>16)]^gacas_aes_FT2[(GACAS_U8)(Y1>>8)]^gacas_aes_FT3[(GACAS_U8)(Y2)];
}
GACAS_FUN void gacas_aes_encrypt( gacas_aes_context_t *ctx, GACAS_U8 input[16], GACAS_U8 output[16] )
{
	GACAS_U32 *RK, X0, X1, X2, X3, Y0=0, Y1=0, Y2=0, Y3=0;

	RK = ctx->erk;

	//GET_UINT32( X0, input,  0 ); 
	X0 = ((GACAS_U32)(input)[(0*4)]<<24)|((GACAS_U32)(input)[(0*4)+1]<<16)|((GACAS_U32) (input)[(0*4)+2]<<8)|((GACAS_U32)(input)[(0*4)+3]);
	X0 ^= RK[0];
	//GET_UINT32( X1, input,  4 );
	X1 = ((GACAS_U32)(input)[(1*4)]<<24)|((GACAS_U32)(input)[(1*4)+1]<<16)|((GACAS_U32) (input)[(1*4)+2]<<8)|((GACAS_U32)(input)[(1*4)+3]);
	X1 ^= RK[1];
	//GET_UINT32( X2, input,  8 );
	X2 = ((GACAS_U32)(input)[(2*4)]<<24)|((GACAS_U32)(input)[(2*4)+1]<<16)|((GACAS_U32) (input)[(2*4)+2]<<8)|((GACAS_U32)(input)[(2*4)+3]);
	X2 ^= RK[2];
	//GET_UINT32( X3, input, 12 ); 
	X3 = ((GACAS_U32)(input)[(3*4)]<<24)|((GACAS_U32)(input)[(3*4)+1]<<16)|((GACAS_U32) (input)[(3*4)+2]<<8)|((GACAS_U32)(input)[(3*4)+3]);
	X3 ^= RK[3];

	RK += 4;
	gacas_aes_fround(RK, &Y0, &Y1, &Y2, &Y3, X0, X1, X2, X3 );       /* round 1 */
	RK += 4;
	gacas_aes_fround(RK, &X0, &X1, &X2, &X3, Y0, Y1, Y2, Y3 );       /* round 2 */
	RK += 4;
	gacas_aes_fround(RK, &Y0, &Y1, &Y2, &Y3, X0, X1, X2, X3 );       /* round 3 */
	RK += 4;
	gacas_aes_fround(RK, &X0, &X1, &X2, &X3, Y0, Y1, Y2, Y3 );       /* round 4 */
	RK += 4;
	gacas_aes_fround(RK, &Y0, &Y1, &Y2, &Y3, X0, X1, X2, X3 );       /* round 5 */
	RK += 4;
	gacas_aes_fround(RK, &X0, &X1, &X2, &X3, Y0, Y1, Y2, Y3 );       /* round 6 */
	RK += 4;
	gacas_aes_fround(RK, &Y0, &Y1, &Y2, &Y3, X0, X1, X2, X3 );       /* round 7 */
	RK += 4;
	gacas_aes_fround(RK, &X0, &X1, &X2, &X3, Y0, Y1, Y2, Y3 );       /* round 8 */
	RK += 4;
	gacas_aes_fround(RK, &Y0, &Y1, &Y2, &Y3, X0, X1, X2, X3 );       /* round 9 */

	if( ctx->nr > 10 )
	{
		RK += 4;
		gacas_aes_fround(RK, &X0, &X1, &X2, &X3, Y0, Y1, Y2, Y3 );   /* round 10 */
		RK += 4;
		gacas_aes_fround(RK, &Y0, &Y1, &Y2, &Y3, X0, X1, X2, X3 );   /* round 11 */
	}

	if( ctx->nr > 12 )
	{
		RK += 4;
		gacas_aes_fround(RK, &X0, &X1, &X2, &X3, Y0, Y1, Y2, Y3 );   /* round 12 */
		RK += 4;
		gacas_aes_fround(RK, &Y0, &Y1, &Y2, &Y3, X0, X1, X2, X3 );   /* round 13 */
	}

	/* last round */

	RK += 4;

	X0 = RK[0] ^ ( gacas_aes_FSb[(GACAS_U8)( Y0 >> 24 ) ] << 24 ) ^
	             ( gacas_aes_FSb[(GACAS_U8)( Y1 >> 16 ) ] << 16 ) ^
	             ( gacas_aes_FSb[(GACAS_U8)( Y2 >>  8 ) ] <<  8 ) ^
	             ( gacas_aes_FSb[(GACAS_U8)( Y3       ) ]       );

	X1 = RK[1] ^ ( gacas_aes_FSb[(GACAS_U8)( Y1 >> 24 ) ] << 24 ) ^
	             ( gacas_aes_FSb[(GACAS_U8)( Y2 >> 16 ) ] << 16 ) ^
	             ( gacas_aes_FSb[(GACAS_U8)( Y3 >>  8 ) ] <<  8 ) ^
	             ( gacas_aes_FSb[(GACAS_U8)( Y0       ) ]       );

	X2 = RK[2] ^ ( gacas_aes_FSb[(GACAS_U8)( Y2 >> 24 ) ] << 24 ) ^
	             ( gacas_aes_FSb[(GACAS_U8)( Y3 >> 16 ) ] << 16 ) ^
	             ( gacas_aes_FSb[(GACAS_U8)( Y0 >>  8 ) ] <<  8 ) ^
	             ( gacas_aes_FSb[(GACAS_U8)( Y1       ) ]       );

	X3 = RK[3] ^ ( gacas_aes_FSb[(GACAS_U8)( Y3 >> 24 ) ] << 24 ) ^
	             ( gacas_aes_FSb[(GACAS_U8)( Y0 >> 16 ) ] << 16 ) ^
	             ( gacas_aes_FSb[(GACAS_U8)( Y1 >>  8 ) ] <<  8 ) ^
	             ( gacas_aes_FSb[(GACAS_U8)( Y2       ) ]       );

	(output)[(0*4)+0] = (GACAS_U8)(X0>> 24);
	(output)[(0*4)+1] = (GACAS_U8)(X0>>16);
	(output)[(0*4)+2] = (GACAS_U8)(X0>>8);
	(output)[(0*4)+3] = (GACAS_U8)(X0); 
	(output)[(1*4)+0] = (GACAS_U8)(X1>> 24);
	(output)[(1*4)+1] = (GACAS_U8)(X1>>16);
	(output)[(1*4)+2] = (GACAS_U8)(X1>>8);
	(output)[(1*4)+3] = (GACAS_U8)(X1); 
	(output)[(2*4)+0] = (GACAS_U8)(X2>> 24);
	(output)[(2*4)+1] = (GACAS_U8)(X2>>16);
	(output)[(2*4)+2] = (GACAS_U8)(X2>>8);
	(output)[(2*4)+3] = (GACAS_U8)(X2); 
	(output)[(3*4)+0] = (GACAS_U8)(X3>> 24);
	(output)[(3*4)+1] = (GACAS_U8)(X3>>16);
	(output)[(3*4)+2] = (GACAS_U8)(X3>>8);
	(output)[(3*4)+3] = (GACAS_U8)(X3); 

	//PUT_UINT32( X0, output,  0 );
	//PUT_UINT32( X1, output,  4 );
	//PUT_UINT32( X2, output,  8 );
	//PUT_UINT32( X3, output, 12 );
}

/* AES 128-bit block decryption routine */
GACAS_FUN void gacas_aes_rround(GACAS_U32 *RK,GACAS_U32 *X0,GACAS_U32 *X1,GACAS_U32 *X2,GACAS_U32 *X3,GACAS_U32 Y0,GACAS_U32 Y1,GACAS_U32 Y2,GACAS_U32 Y3) 
{                                           
	*X0 = RK[0]^gacas_aes_RT0[(GACAS_U8)(Y0>>24)]^gacas_aes_RT1[(GACAS_U8)(Y3>>16)]^gacas_aes_RT2[(GACAS_U8)(Y2>>8)]^gacas_aes_RT3[(GACAS_U8)(Y1)];  
	*X1 = RK[1]^gacas_aes_RT0[(GACAS_U8)(Y1>>24)]^gacas_aes_RT1[(GACAS_U8)(Y0>>16)]^gacas_aes_RT2[(GACAS_U8)(Y3>>8)]^gacas_aes_RT3[(GACAS_U8)(Y2)];
	*X2 = RK[2]^gacas_aes_RT0[(GACAS_U8)(Y2>>24)]^gacas_aes_RT1[(GACAS_U8)(Y1>>16)]^gacas_aes_RT2[(GACAS_U8)(Y0>>8)]^gacas_aes_RT3[(GACAS_U8)(Y3)];
	*X3 = RK[3]^gacas_aes_RT0[(GACAS_U8)(Y3>>24)]^gacas_aes_RT1[(GACAS_U8)(Y2>>16)]^gacas_aes_RT2[(GACAS_U8)(Y1>>8)]^gacas_aes_RT3[(GACAS_U8)(Y0)];
}
GACAS_FUN void gacas_aes_decrypt( gacas_aes_context_t *ctx, GACAS_U8 input[16], GACAS_U8 output[16] )
{
	GACAS_U32 *RK, X0, X1, X2, X3, Y0=0, Y1=0, Y2=0, Y3=0;

	RK = ctx->drk;

	//GET_UINT32( X0, input,  0 ); 
	X0 = ((GACAS_U32)(input)[(0*4)]<<24)|((GACAS_U32)(input)[(0*4)+1]<<16)|((GACAS_U32) (input)[(0*4)+2]<<8)|((GACAS_U32)(input)[(0*4)+3]);
	X0 ^= RK[0];
	//GET_UINT32( X1, input,  4 );
	X1 = ((GACAS_U32)(input)[(1*4)]<<24)|((GACAS_U32)(input)[(1*4)+1]<<16)|((GACAS_U32) (input)[(1*4)+2]<<8)|((GACAS_U32)(input)[(1*4)+3]);
	X1 ^= RK[1];
	//GET_UINT32( X2, input,  8 );
	X2 = ((GACAS_U32)(input)[(2*4)]<<24)|((GACAS_U32)(input)[(2*4)+1]<<16)|((GACAS_U32) (input)[(2*4)+2]<<8)|((GACAS_U32)(input)[(2*4)+3]);
	X2 ^= RK[2];
	//GET_UINT32( X3, input, 12 ); 
	X3 = ((GACAS_U32)(input)[(3*4)]<<24)|((GACAS_U32)(input)[(3*4)+1]<<16)|((GACAS_U32) (input)[(3*4)+2]<<8)|((GACAS_U32)(input)[(3*4)+3]);
	X3 ^= RK[3];

	RK += 4;
	gacas_aes_rround(RK, &Y0, &Y1, &Y2, &Y3, X0, X1, X2, X3 );       /* round 1 */
	RK += 4;
	gacas_aes_rround(RK, &X0, &X1, &X2, &X3, Y0, Y1, Y2, Y3 );       /* round 2 */
	RK += 4;
	gacas_aes_rround(RK, &Y0, &Y1, &Y2, &Y3, X0, X1, X2, X3 );       /* round 3 */
	RK += 4;
	gacas_aes_rround(RK, &X0, &X1, &X2, &X3, Y0, Y1, Y2, Y3 );       /* round 4 */
	RK += 4;
	gacas_aes_rround(RK, &Y0, &Y1, &Y2, &Y3, X0, X1, X2, X3 );       /* round 5 */
	RK += 4;
	gacas_aes_rround(RK, &X0, &X1, &X2, &X3, Y0, Y1, Y2, Y3 );       /* round 6 */
	RK += 4;
	gacas_aes_rround(RK, &Y0, &Y1, &Y2, &Y3, X0, X1, X2, X3 );       /* round 7 */
	RK += 4;
	gacas_aes_rround(RK, &X0, &X1, &X2, &X3, Y0, Y1, Y2, Y3 );       /* round 8 */
	RK += 4;
	gacas_aes_rround(RK, &Y0, &Y1, &Y2, &Y3, X0, X1, X2, X3 );       /* round 9 */
	if( ctx->nr > 10 )
	{
		RK += 4;
		gacas_aes_rround(RK, &X0, &X1, &X2, &X3, Y0, Y1, Y2, Y3 );   /* round 10 */
		RK += 4;
		gacas_aes_rround(RK, &Y0, &Y1, &Y2, &Y3, X0, X1, X2, X3 );   /* round 11 */
	}

	if( ctx->nr > 12 )
	{
		RK += 4;
		gacas_aes_rround(RK, &X0, &X1, &X2, &X3, Y0, Y1, Y2, Y3 );   /* round 12 */
		RK += 4;
		gacas_aes_rround(RK, &Y0, &Y1, &Y2, &Y3, X0, X1, X2, X3 );   /* round 13 */
	}
    /* last round */

	RK += 4;

	X0 = RK[0] ^ ( gacas_aes_RSb[(GACAS_U8)( Y0 >> 24 ) ] << 24 ) ^
	             ( gacas_aes_RSb[(GACAS_U8)( Y3 >> 16 ) ] << 16 ) ^
	             ( gacas_aes_RSb[(GACAS_U8)( Y2 >>  8 ) ] <<  8 ) ^
	             ( gacas_aes_RSb[(GACAS_U8)( Y1       ) ]       );

	X1 = RK[1] ^ ( gacas_aes_RSb[(GACAS_U8)( Y1 >> 24 ) ] << 24 ) ^
	             ( gacas_aes_RSb[(GACAS_U8)( Y0 >> 16 ) ] << 16 ) ^
	             ( gacas_aes_RSb[(GACAS_U8)( Y3 >>  8 ) ] <<  8 ) ^
	             ( gacas_aes_RSb[(GACAS_U8)( Y2       ) ]       );

	X2 = RK[2] ^ ( gacas_aes_RSb[(GACAS_U8)( Y2 >> 24 ) ] << 24 ) ^
	             ( gacas_aes_RSb[(GACAS_U8)( Y1 >> 16 ) ] << 16 ) ^
	             ( gacas_aes_RSb[(GACAS_U8)( Y0 >>  8 ) ] <<  8 ) ^
	             ( gacas_aes_RSb[(GACAS_U8)( Y3       ) ]       );

	X3 = RK[3] ^ ( gacas_aes_RSb[(GACAS_U8)( Y3 >> 24 ) ] << 24 ) ^
	             ( gacas_aes_RSb[(GACAS_U8)( Y2 >> 16 ) ] << 16 ) ^
	             ( gacas_aes_RSb[(GACAS_U8)( Y1 >>  8 ) ] <<  8 ) ^
	             ( gacas_aes_RSb[(GACAS_U8)( Y0       ) ]       );

	(output)[(0*4)+0] = (GACAS_U8)(X0>> 24);
	(output)[(0*4)+1] = (GACAS_U8)(X0>>16);
	(output)[(0*4)+2] = (GACAS_U8)(X0>>8);
	(output)[(0*4)+3] = (GACAS_U8)(X0); 
	(output)[(1*4)+0] = (GACAS_U8)(X1>> 24);
	(output)[(1*4)+1] = (GACAS_U8)(X1>>16);
	(output)[(1*4)+2] = (GACAS_U8)(X1>>8);
	(output)[(1*4)+3] = (GACAS_U8)(X1); 
	(output)[(2*4)+0] = (GACAS_U8)(X2>> 24);
	(output)[(2*4)+1] = (GACAS_U8)(X2>>16);
	(output)[(2*4)+2] = (GACAS_U8)(X2>>8);
	(output)[(2*4)+3] = (GACAS_U8)(X2); 
	(output)[(3*4)+0] = (GACAS_U8)(X3>> 24);
	(output)[(3*4)+1] = (GACAS_U8)(X3>>16);
	(output)[(3*4)+2] = (GACAS_U8)(X3>>8);
	(output)[(3*4)+3] = (GACAS_U8)(X3); 
	
	//PUT_UINT32( X0, output,  0 );
	//PUT_UINT32( X1, output,  4 );
	//PUT_UINT32( X2, output,  8 );
	//PUT_UINT32( X3, output, 12 );
}
GACAS_FUN GACAS_S32 gacas_aes_128_encrypt(GACAS_U32 data_len,GACAS_U8* p_data_in,GACAS_U8* p_data_out,GACAS_U8 *p_key )
{
	gacas_aes_context_t lCT;
	GACAS_U8* ptr_in = GACAS_NULL;
	GACAS_U8* ptr_out = GACAS_NULL;
	GACAS_U8 buf[16];
	GACAS_U32 i;
	
	gacas_aes_set_key( &lCT, p_key, 128 );
	ptr_in = p_data_in;
	ptr_out = p_data_out;
	for(i = 0; i < data_len/16; i++)
	{
		gacas_aes_encrypt( &lCT, ptr_in, buf);
		ptr_in += 16;
		GACAS_MEMCPY((char*)ptr_out,(char*) buf, sizeof(buf));
		ptr_out += 16;
	}
	return 0;
}
GACAS_FUN GACAS_S32 gacas_aes_128_decrypt(GACAS_U32 data_len,GACAS_U8* p_data_in,GACAS_U8* p_data_out,GACAS_U8 *p_key )
{
	gacas_aes_context_t lCT;
	GACAS_U8* ptr_in = GACAS_NULL;
	GACAS_U8* ptr_out = GACAS_NULL;
	GACAS_U8 buf[16];
	GACAS_U32 i;
	
	gacas_aes_set_key( &lCT, p_key, 128 );
	ptr_in = p_data_in;
	ptr_out = p_data_out;
	for(i = 0; i < data_len/16; i++)
	{
		gacas_aes_decrypt( &lCT, ptr_in, buf);
		ptr_in += 16;
		GACAS_MEMCPY((char*)ptr_out, (char*)buf, sizeof(buf));
		ptr_out += 16;
	}
	return 0;
}
INT32 gacas_aes_ecb_encypt(UINT8 *key,UINT8 *data_in,UINT8 *data_out,UINT32 data_len)
{
	//aes_pure_ecb_crypt(key,data_in,data_out,data_len,DSC_ENCRYPT);
	return gacas_aes_128_encrypt(data_len,data_in,data_out,key);
}
INT32 gacas_aes_ecb_decypt(UINT8 *key,UINT8 *data_in,UINT8 *data_out,UINT32 data_len)
{
	//aes_pure_ecb_crypt(key,data_in,data_out,data_len,DSC_DECRYPT);
	return gacas_aes_128_decrypt(data_len,data_in,data_out,key);
}
#endif
