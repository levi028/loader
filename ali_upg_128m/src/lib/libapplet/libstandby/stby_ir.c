#include "stby_ir.h"
//==============================================================================================//

unsigned char standby_bufferin = 0xFF;
unsigned char standby_bufferout = 0xFF;
unsigned char ir_buffer[512] = {0xFF};

//==============================================================================================//

enum irp_type
{
	IR_TYPE_NEC = 0,
	IR_TYPE_LAB,
	IR_TYPE_50560,
	IR_TYPE_KF,
	IR_TYPE_LOGIC,
	IR_TYPE_SRC,
	IR_TYPE_NSE,
	IR_TYPE_RC5,
	IR_TYPE_RC5_X,
};

enum ir_waveform
{
	IR_LEADING = 0,
	IR_SPECIAL,
	IR_DATA,
	IR_REPEAT_LEADING,
	IR_REPEAT_DATA,
	IR_STOP,
	IR_END,
	IR_NULL,
};

struct irc_pulse
{
	unsigned char type;
	unsigned short fst_half;
	unsigned short scd_half;
	unsigned short tolerance;
};

struct ir_attr
{
	unsigned int type;
	unsigned int bit_msb_first : 1;
	unsigned int byte_msb_first : 1;
	unsigned int pulse_invert : 1;
	unsigned int repeat_enable : 1;
	unsigned int ignore_lastpulse : 1;
	unsigned int pulse_prec;
	unsigned int pulse_max_width;
	struct irc_pulse pulse[7];
	enum ir_waveform *normal;
	enum ir_waveform *repeat;
};

struct irc_decoder
{
	struct ir_attr *attr;
	unsigned char first_half_got;
	unsigned char decode_step;
	unsigned char key_bit_cnt;
	unsigned short last_pulse_width;
	unsigned long key_code[4];
	unsigned long last_key_code[4];
};

enum ir_waveform ir_nec_normal[] =
{
	IR_LEADING,/*start bit.*/
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,/*8 bits command.*/
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,/*8 bits address.*/
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,/*8 bits ~command.*/
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,/*8 bits ~address.*/
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_END,
};

enum ir_waveform ir_nec_repeat[] =
{
	IR_REPEAT_LEADING,
	IR_END,
};

struct ir_attr ir_attr_nec =
{
	IR_TYPE_NEC,
	1,
	0,
	0,
	1,
	0,
	280,
	15000,
	{
		{IR_LEADING, PULSE_LOW | 9000, PULSE_HIGH | 4500, 500},/*start pulse.*/
		{IR_NULL, 0, 0, 0},
		{IR_DATA, PULSE_LOW | 560, PULSE_HIGH | 560, 200},/*logic '0'.*/
		{IR_DATA, PULSE_LOW | 560, PULSE_HIGH | 1680, 300},/*logic '1'.*/
		{IR_REPEAT_LEADING, PULSE_LOW | 9000, PULSE_HIGH | 2250, 500},
		{IR_REPEAT_DATA, PULSE_LOW | 560, PULSE_HIGH | 20000, 1000},
		{IR_STOP, PULSE_LOW | 500, PULSE_HIGH | 15000, 1000},
	},
	ir_nec_normal,
	ir_nec_repeat,
};

enum ir_waveform ir_lab[] =
{
	IR_LEADING,
	IR_SPECIAL,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA,
	IR_END,
};

struct ir_attr ir_attr_lab =
{
	IR_TYPE_LAB,
	1,
	0,
	0,
	0,
	0,
	140,
	15000,
	{
		{IR_LEADING, PULSE_LOW | 280, PULSE_HIGH | 7300, 140},
		{IR_SPECIAL, PULSE_LOW | 280, PULSE_HIGH | 6150, 1500},
		{IR_DATA, PULSE_LOW | 280, PULSE_HIGH | 4780, 500},
		{IR_DATA, PULSE_LOW | 280, PULSE_HIGH | 7300, 500},
		{IR_NULL, 0, 0, 0},
		{IR_NULL, 0, 0, 0},
		{IR_STOP, PULSE_LOW | 500, PULSE_HIGH | 15000, 1000},
	},
	ir_lab,
	0,
};

enum ir_waveform ir_50560[] =
{
	IR_LEADING,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_SPECIAL,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_SPECIAL,
	IR_END,
};

struct ir_attr ir_attr_50560 =
{
	IR_TYPE_50560,
	1,
	0,
	0,
	1,
	0,
	260,
	15000,
	{
		{IR_LEADING, PULSE_LOW | 8400, PULSE_HIGH | 4200, 500},
		{IR_SPECIAL, PULSE_LOW | 520, PULSE_HIGH | 4200, 500},
		{IR_DATA, PULSE_LOW | 520, PULSE_HIGH | 1050, 500},
		{IR_DATA, PULSE_LOW | 520, PULSE_HIGH | 2100, 500},
		{IR_NULL, 0, 0, 0},
		{IR_NULL, 0, 0, 0},
		{IR_STOP, PULSE_LOW | 500, PULSE_HIGH | 15000, 1000},
	},
	ir_50560,
	0,
};

enum ir_waveform ir_kf[] =
{
	IR_LEADING,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_END,
};

struct ir_attr ir_attr_kf =
{
	IR_TYPE_KF,
	0,
	1,
	0,
	0,
	0,
	190,
	20000,
	{
		{IR_LEADING, PULSE_LOW | 3640, PULSE_HIGH | 1800, 500},
		{IR_NULL, 0, 0, 0},
		{IR_DATA, PULSE_LOW | 380, PULSE_HIGH | 380, 150},
		{IR_DATA, PULSE_LOW | 380, PULSE_HIGH | 1350, 300},
		{IR_NULL, 0, 0, 0},
		{IR_NULL, 0, 0, 0},
		{IR_STOP, PULSE_LOW | 500, PULSE_HIGH | 20000, 1000},
	},
	ir_kf,
	0,
};

enum ir_waveform ir_logic[] =
{
	IR_LEADING,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_END,
};

struct ir_attr ir_attr_logic =
{
	IR_TYPE_LOGIC,
	0,
	0,
	0,
	0,
	0,
	125,
	20000,
	{
		{IR_LEADING, PULSE_LOW | 5000, PULSE_HIGH | 5000, 500},
		{IR_NULL, 0, 0, 0},
		{IR_DATA, PULSE_LOW | 250, PULSE_HIGH | 500, 100},
		{IR_DATA, PULSE_LOW | 500, PULSE_HIGH | 1000, 200},
		{IR_NULL, 0, 0, 0},
		{IR_NULL, 0, 0, 0},
		{IR_STOP, PULSE_LOW | 500, PULSE_HIGH | 20000, 1000},
	},
	ir_logic,
	0,
};

enum ir_waveform ir_src_normal[] =
{
	IR_LEADING,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_SPECIAL,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_END,
};

struct ir_attr ir_attr_src =
{
	IR_TYPE_SRC,
	1,
	0,
	0,
	1,
	0,
	280,
	60000,
	{
		{IR_LEADING, PULSE_LOW | 6000, PULSE_HIGH | 2000, 400},
		{IR_SPECIAL, PULSE_LOW | 3640, PULSE_HIGH | 3640, 400},
		{IR_DATA, PULSE_LOW | 560, PULSE_HIGH | 560, 200},
		{IR_DATA, PULSE_LOW | 560, PULSE_HIGH | 1120, 300},
		{IR_NULL, 0, 0, 0},
		{IR_NULL, 0, 0, 0},
		{IR_STOP, PULSE_LOW | 560, PULSE_HIGH | 20000, 1000},
	},
	ir_src_normal,
	0,
};

enum ir_waveform ir_nse_normal[] =
{
	IR_LEADING,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_LEADING,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_END,
};

enum ir_waveform ir_nse_repeat[] =
{
	IR_REPEAT_LEADING,
	IR_END,
};

struct ir_attr ir_attr_nse =
{
	IR_TYPE_NSE,
	1,
	0,
	0,
	1,
	0,
	280,
	60000,
	{
		{IR_LEADING, PULSE_LOW | 3640, PULSE_HIGH | 1800, 400},
		{IR_NULL, 0, 0, 0},
		{IR_DATA, PULSE_LOW | 560, PULSE_HIGH | 560, 200},
		{IR_DATA, PULSE_LOW | 560, PULSE_HIGH | 1120, 300},
		{IR_REPEAT_LEADING, PULSE_LOW | 3640, PULSE_HIGH | 3640, 500},
		{IR_REPEAT_DATA, PULSE_LOW | 560, PULSE_HIGH | 20000, 1000},
		{IR_STOP, PULSE_LOW | 560, PULSE_HIGH | 20000, 1000},
	},
	ir_nse_normal,
	ir_nse_repeat,
};

enum ir_waveform ir_rc5[] =
{
	IR_LEADING, IR_LEADING,/*S1 & S2 bit.*/
	IR_SPECIAL,/*toggle bit.*/
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,/*5 bits addr & 6 bits command.*/
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA, IR_DATA,
	IR_END,
};

struct ir_attr ir_attr_rc5 =
{
	IR_TYPE_RC5,/*type name.*/
	1,/*bits MSB.*/
	0,
	0,
	0,
	1,/*repeat enable.*/
	200,/*pulse prec.*/
	15000,/*pulse max width.*/
	{
		{IR_LEADING, PULSE_HIGH | 830, PULSE_LOW | 830, 200},/*leading logic.*/
		{IR_SPECIAL, PULSE_HIGH | 830, PULSE_LOW | 830, 200},/*toggle logic.*/
		{IR_DATA, PULSE_LOW | 830, PULSE_HIGH | 830, 200},/*data logic '0'.*/
		{IR_DATA, PULSE_HIGH | 830, PULSE_LOW | 830, 200},/*data logic '1'.*/
		{IR_NULL, 0, 0, 0},
		{IR_NULL, 0, 0, 0},
		{IR_STOP, PULSE_HIGH | 830, PULSE_LOW | 15000, 1000},
	},
	ir_rc5,
	0,
};

enum ir_waveform ir_rc5_x[] =
{
	IR_LEADING,
	IR_DATA,/*S1 & S2 bit.*/
	IR_DATA,
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,/*5 bits addr & 6 bits command.*/
	IR_DATA, IR_DATA, IR_DATA, IR_DATA,
	IR_DATA, IR_DATA,
};

struct ir_attr ir_attr_rc5_x =
{
	IR_TYPE_RC5,/*type name.*/
	1,/*bits MSB.*/
	0,
	0,
	0,
	1,/*repeat enable.*/
	200,/*pulse prec.*/
	15000,/*pulse max width.*/
	{
		{IR_LEADING, PULSE_HIGH | 830, PULSE_LOW | 830, 200},/*leading logic.*/
		{IR_SPECIAL, PULSE_HIGH | 830, PULSE_LOW | 830, 200},/*toggle logic.*/
		{IR_DATA, PULSE_LOW | 830, PULSE_HIGH | 830, 200},/*data logic '0'.*/
		{IR_DATA, PULSE_HIGH | 830, PULSE_LOW | 830, 200},/*data logic '1'.*/
		{IR_NULL, 0, 0, 0},
		{IR_NULL, 0, 0, 0},
		{IR_STOP, PULSE_HIGH | 830, PULSE_LOW | 15000, 1000},
	},
	ir_rc5_x,
	0,
};

struct irc_decoder irc_decoders_nec =
{
	&ir_attr_nec,//.attr = &ir_attr_nec,
	0,//.first_half_got = 0,
	0,//.decode_step = 0,
	0,//.key_bit_cnt = 0,
	0,//.last_pulse_width = 0,
	{0,},//.key_code = {0,},
	{0,},//.last_key_code = {0,},
};

struct irc_decoder irc_decoders_lab =
{
	&ir_attr_lab,//.attr = &ir_attr_lab,
	0,//.first_half_got = 0,
	0,//.decode_step = 0,
	0,//.key_bit_cnt = 0,
	0,//.last_pulse_width = 0,
	{0,},//.key_code = {0,},
	{0,},//.last_key_code = {0,},
};

struct irc_decoder irc_decoders_50560 =
{
	&ir_attr_50560,//.attr = &ir_attr_50560,
	0,//.first_half_got = 0,
	0,//.decode_step = 0,
	0,//.key_bit_cnt = 0,
	0,//.last_pulse_width = 0,
	{0,},//.key_code = {0,},
	{0,},//.last_key_code = {0,},
};

struct irc_decoder irc_decoders_kf =
{
	&ir_attr_kf,//.attr = &ir_attr_kf,
	0,//.first_half_got = 0,
	0,//.decode_step = 0,
	0,//.key_bit_cnt = 0,
	0,//.key_code = {0,},
	{0,},//.last_pulse_width = 0,
	{0,},//.last_key_code = {0,},
};

struct irc_decoder irc_decoders_logic =
{
	&ir_attr_logic,//.attr = &ir_attr_logic,
	0,//.first_half_got = 0,
	0,//.decode_step = 0,
	0,//.key_bit_cnt = 0,
	0,//.last_pulse_width = 0,
	{0,},//.key_code = {0,},
	{0,},//.last_key_code = {0,},
};

struct irc_decoder irc_decoders_src =
{
	&ir_attr_src,//.attr = &ir_attr_src,
	0,//.first_half_got = 0,
	0,//.decode_step = 0,
	0,//.key_bit_cnt = 0,
	0,//.last_pulse_width = 0,
	{0,},//.key_code = {0,},
	{0,},//.last_key_code = {0,},
};

struct irc_decoder irc_decoders_nse =
{
	&ir_attr_nse,//.attr = &ir_attr_nse,
	0,//.first_half_got = 0,
	0,//.decode_step = 0,
	0,//.key_bit_cnt = 0,
	0,//.last_pulse_width = 0,
	{0,},//.key_code = {0,},
	{0,},//.last_key_code = {0,},
};

struct irc_decoder irc_decoders_rc5 =
{
	&ir_attr_rc5,//.attr = &ir_attr_rc5,
	0,//.first_half_got = 0,
	0,//.decode_step = 0,
	0,//.key_bit_cnt = 0,
	0,//.last_pulse_width = 0,
	{0,},//.key_code = {0,},
	{0,},//.last_key_code = {0,},
};

struct irc_decoder irc_decoders_rx5_x =
{
	&ir_attr_rc5_x,//.attr = &ir_attr_rc5_x,
	0,//.first_half_got = 0,
	0,//.decode_step = 0,
	0,//.key_bit_cnt = 0,
	0,//.last_pulse_width = 0,
	{0,},//.key_code = {0,},
	{0,},//.last_key_code = {0,},
};

struct irc_decoder *irc_decoders[] =
{
	&irc_decoders_nec,
	&irc_decoders_lab,
	&irc_decoders_50560,
	&irc_decoders_kf,
	&irc_decoders_logic,
	&irc_decoders_src,
	&irc_decoders_nse,
	&irc_decoders_rc5,
	//&irc_decoders_rx5_x,
};

enum irc_decode_ret
{
	IRC_DECODE_SUCCEED,
	IRC_DECODE_DATA0,
	IRC_DECODE_DATA1,
	IRC_DECODE_FAIL,
	IRC_DECODE_STOP,
};
//=====================================================================================//

void COPY_CODE(unsigned long *dst_addr, unsigned long *src_addr)
{
	unsigned int i = 0;

	for(i=0; i<16; i++)
	{
		*((unsigned char *)dst_addr+i) = *((unsigned char *)src_addr+i);
	}
}

void CLEAR_CODE(unsigned int *clear_addr)
{
	unsigned int i = 0;

	for(i=0; i<16; i++)
	{
		*((unsigned char *)clear_addr+i) = 0;
	}
}

void stdby_reverse_code_bit(unsigned long *ir_code, unsigned long bit_cnt)
{
	unsigned char i = 0;
	unsigned long src_code[4] = {0};
	unsigned long dst_code[4] = {0};

	COPY_CODE(src_code, ir_code);

	for(i=0; i<bit_cnt; i++)
	{
		if(src_code[i/32] & (0x1 << (i % 32)))
		{
			dst_code[(bit_cnt-1-i)/32] |= (0x1 << ((bit_cnt - 1 - i) % 32));
		}
	}

	COPY_CODE(ir_code, dst_code);
}

void stdby_reverse_code_byte(unsigned long *ir_code, unsigned char bit_cnt)
{
	unsigned char i = 0;
	unsigned char src_code[16] = {0};
	unsigned char dst_code[16] = {0};
	unsigned char byte_cnt = (bit_cnt+7)/8;

	//sample
	//src: 58 59 4e 48 00 00 57 50
	//mips: 48 4e 59 58 50 57 00 00
	//rev: 57 50 58 59 4e 48 00 00
	//dst: 59 58 50 57 00 00 4e 48

	COPY_CODE((unsigned long *)src_code, ir_code);

	dst_code[0] = src_code[3];
	dst_code[1] = src_code[2];
	dst_code[2] = src_code[1];
	dst_code[3] = src_code[0];
	dst_code[4] = src_code[7];
	dst_code[5] = src_code[6];
	dst_code[6] = src_code[5];
	dst_code[7] = src_code[4];
	dst_code[8] = src_code[11];
	dst_code[9] = src_code[10];
	dst_code[10] = src_code[9];
	dst_code[11] = src_code[8];
	dst_code[12] = src_code[15];
	dst_code[13] = src_code[14];
	dst_code[14] = src_code[13];
	dst_code[15] = src_code[12];

	COPY_CODE((unsigned long *)src_code, (unsigned long *)dst_code);
	for(i=0; i<byte_cnt; i++)
	{
		dst_code[i] = src_code[byte_cnt-i-1];
	}

	src_code[0] = dst_code[3];
	src_code[1] = dst_code[2];
	src_code[2] = dst_code[1];
	src_code[3] = dst_code[0];
	src_code[4] = dst_code[7];
	src_code[5] = dst_code[6];
	src_code[6] = dst_code[5];
	src_code[7] = dst_code[4];
	src_code[8] = dst_code[11];
	src_code[9] = dst_code[10];
	src_code[10] = dst_code[9];
	src_code[11] = dst_code[8];
	src_code[12] = dst_code[15];
	src_code[13] = dst_code[14];
	src_code[14] = dst_code[13];
	src_code[15] = dst_code[12];

	COPY_CODE((unsigned long *)ir_code, (unsigned long *)src_code);
}

/****************************************************************************
* unsigned long irc_decode(unsigned long pulse_width)
* Description: Translate pulse width to IR code
* Arguments:
* struct irc_decoder *ird: IR decode structure
* unsigned long pulse_width: Input pulse width, in uS
* unsigned long pulse_polarity: Input pulse polarity, '1' = high level, '0' = low level.
* Return Value:
* INT32: Key code
****************************************************************************/
unsigned long stdby_irc_decode(struct irc_decoder *ird, unsigned long pulse_width, unsigned long pulse_polarity)
{
	unsigned long accum_pulse_width = 0;
	enum irc_decode_ret result = IRC_DECODE_FAIL;
	enum ir_waveform step;
	enum ir_waveform next_step;
	unsigned long last_key = 0;
	unsigned short step_temp = 0;
	unsigned long accum_pulse = 0;
	unsigned long tolerance_pulse = 0;
	unsigned long same_polarity = 0;
	unsigned long accum_pulse_1 = 0;
	unsigned long tolerance_pulse_1 = 0;
	unsigned long same_polarity_1 = 0;
	unsigned long half_pol = 0;
	unsigned long half_width = 0;

	/*Ignore burr pulses.*/
	if(pulse_width < ird->attr->pulse_prec)
	{
		ird->first_half_got = 0;
		return PAN_KEY_INVALID;
	}

	if(ird->first_half_got == 0)/*first_half_got is flag for dule-pulse senser.*/
	{
		if(ird->decode_step == 0)/*just check leading pulse.*/
		{
			if(PULSE_POL(ird->attr->pulse[0].fst_half))/*if leading start with high level.*/
			{
				half_pol = PULSE_POL(ird->attr->pulse[0].scd_half);
				half_width = PULSE_VALUE(ird->attr->pulse[0].scd_half);
			}
			else
			{
				half_pol = PULSE_POL(ird->attr->pulse[0].fst_half);
				half_width = PULSE_VALUE(ird->attr->pulse[0].fst_half);
			}

			/*Check leading code first half pulse polarity.*/
			if(half_pol ^ (pulse_polarity << 15))
			{
				return PAN_KEY_INVALID;
			}

			/*Check leading code first half pulse width*/
			if(!INRANGE(pulse_width, half_width, ird->attr->pulse[0].tolerance))
			{
				return PAN_KEY_INVALID;
			}
		}

		ird->first_half_got = 1;
		ird->last_pulse_width = pulse_width;
		if((ird->decode_step == 0) && (PULSE_POL(ird->attr->pulse[0].fst_half) ? 1 : 0))
		{
			ird->first_half_got = 0;
			ird->last_pulse_width = 0;
			ird->decode_step++;
		}

		return PAN_KEY_INVALID;
	}
	else
	{
		accum_pulse_width = ird->last_pulse_width + pulse_width;
		ird->last_pulse_width = 0;
		ird->first_half_got = 0;

		/*Process normal key.*/
		if(ird->attr->normal)
		{
			step = ird->attr->normal[ird->decode_step];
			next_step = ird->attr->normal[ird->decode_step + 1];
			if(step > IR_DATA)
			{
				step++;
			}

			accum_pulse = PULSE_VALUE(ird->attr->pulse[step].fst_half) + PULSE_VALUE(ird->attr->pulse[step].scd_half);
			tolerance_pulse = ird->attr->pulse[step].tolerance;
			same_polarity = !(PULSE_POL(ird->attr->pulse[step].scd_half) ^ (pulse_polarity << 15));

			if(ird->attr->type == IR_TYPE_RC5)
			{
				accum_pulse += PULSE_VALUE(ird->attr->pulse[next_step].fst_half);
				tolerance_pulse += PULSE_VALUE(ird->attr->pulse[next_step].fst_half);
			}

			if(ird->attr->normal[ird->decode_step] == IR_DATA)
			{
				accum_pulse_1 = PULSE_VALUE(ird->attr->pulse[IR_DATA + 1].fst_half) + PULSE_VALUE(ird->attr->pulse[IR_DATA + 1].scd_half);
				tolerance_pulse_1 = ird->attr->pulse[IR_DATA + 1].tolerance;
				same_polarity_1 = !(PULSE_POL(ird->attr->pulse[IR_DATA + 1].scd_half) ^ (pulse_polarity << 15));

				if(ird->attr->type == IR_TYPE_RC5)
				{
					accum_pulse_1 += PULSE_VALUE(ird->attr->pulse[IR_DATA + 1].fst_half);
					tolerance_pulse_1 += PULSE_VALUE(ird->attr->pulse[IR_DATA + 1].fst_half);
				}

				if((((ird->attr->ignore_lastpulse) && (ird->attr->normal[ird->decode_step + 1] == IR_END)) \
					|| INRANGE(accum_pulse_width, accum_pulse, tolerance_pulse)) && (same_polarity ? 1 : 0))
				{
					result = IRC_DECODE_DATA0;
				}
				else if((((ird->attr->ignore_lastpulse) && (ird->attr->normal[ird->decode_step + 1] == IR_END)) \
					|| INRANGE(accum_pulse_width, accum_pulse_1, tolerance_pulse_1)) && (same_polarity_1 ? 1 : 0))
				{
					result = IRC_DECODE_DATA1;
				}
				else
				{
					result = IRC_DECODE_FAIL;
				}
			}
			else
			{
				if(INRANGE(accum_pulse_width, accum_pulse, tolerance_pulse))
				{
					if((ird->attr->type == IR_TYPE_RC5) && (step == IR_SPECIAL))
					{
						result = IRC_DECODE_SUCCEED;
					}
					else if(same_polarity)
					{
						result = IRC_DECODE_SUCCEED;
					}
					else
					{
						result = IRC_DECODE_FAIL;
					}
				}
				else
				{
					result = IRC_DECODE_FAIL;
				}
			}

			if((result == IRC_DECODE_SUCCEED) || (result == IRC_DECODE_DATA0) \
				|| (result == IRC_DECODE_DATA1))
			{
				ird->decode_step++;
				if((result == IRC_DECODE_DATA0) || (result == IRC_DECODE_DATA1))
				{
					if(result == IRC_DECODE_DATA1)
					{
						ird->key_code[ird->key_bit_cnt/32] |= (0x1 << (ird->key_bit_cnt % 32));
					}

					ird->key_bit_cnt++;
				}

				if((ird->attr->type == IR_TYPE_RC5) && INRANGE(accum_pulse_width, accum_pulse, ird->attr->pulse[next_step].tolerance))
				{
					ird->first_half_got = 1;
					ird->last_pulse_width = PULSE_VALUE(ird->attr->pulse[next_step].fst_half);
				}

				/*deal with bit/byte reverse if need at last.*/
				if(ird->attr->normal[ird->decode_step] == IR_END)
				{
					last_key = PAN_KEY_INVALID;
					if(ird->attr->bit_msb_first)
					{
						stdby_reverse_code_bit(ird->key_code, ird->key_bit_cnt);
					}

					if(ird->attr->byte_msb_first)
					{
						stdby_reverse_code_byte(ird->key_code, ird->key_bit_cnt);
					}

					ird->decode_step = 0;
					ird->key_bit_cnt = 0;
					last_key = ird->last_key_code[0];
					COPY_CODE(ird->last_key_code, ird->key_code);
					CLEAR_CODE((unsigned int *)(ird->key_code));

					return ird->last_key_code[0];
				}

				return PAN_KEY_INVALID;
			}
			else if(result == IRC_DECODE_FAIL)
			{
				step_temp = ird->decode_step;
				ird->decode_step = 0;
				ird->key_bit_cnt = 0;
				CLEAR_CODE((unsigned int *)(ird->key_code));
				if(ird->attr->normal[step_temp] != IR_LEADING)
				{
					return PAN_KEY_INVALID;
				}
			}
			else
			{
				ird->decode_step = 0;
				ird->key_bit_cnt = 0;
				CLEAR_CODE((unsigned int *)(ird->key_code));
				return PAN_KEY_INVALID;
			}
		}
	}

	return ird->last_key_code[0];
}

/***************************************************************************
* unsigned long irc_pulse_to_code(unsigned long pulse_width)
* Description: Translate pulse width to IR code.
* From now on, irc_decode support fellow protocols :
* NEC LAB 50560 KF Logic SRC NSE RC5 RC6_mode1.
* Arguments:
* unsigned long pulse_width: Input pulse width, in uS
* unsigned long pulse_polarity: Input pulse polarity, '1' = high level, '0' = low level.
* Return Value:
* INT32: Key code
****************************************************************************/
unsigned long stdby_irc_pulse_to_code(unsigned long pulse_width, unsigned long pulse_polarity)
{
	unsigned char i = 0;
	unsigned long key_code = PAN_KEY_INVALID;

	for(i = 0; i< IRP_CNT; i++)
	{
		key_code = stdby_irc_decode(irc_decoders[i], pulse_width, pulse_polarity);
		if((key_code != PAN_KEY_INVALID) && (key_code != 0x0))
		{
			return key_code;
		}
	}

	return key_code; 
}

/***************************************************************************
* Function_Name: IR_GenerateCode
* Description: IR controler generate code
* Arguments: None
* Return Value: None
****************************************************************************/
void stdby_IR_GenerateCode(void)
{
	volatile unsigned int cur_code = 0, pulse_width = 0;
	volatile unsigned char pulse_polarity = 0;
	volatile unsigned char rlc_data = 0, i = 0;
	struct irc_decoder *ird;

	for(i=0; i<IRP_CNT; i++)
	{
		ird = irc_decoders[i];
		ird->first_half_got = 0;
		ird->decode_step = 0;
		ird->key_bit_cnt = 0;
		CLEAR_CODE((unsigned int *)(ird->key_code));
	}

	pulse_width = 0;
	while(standby_bufferin != standby_bufferout)
	{
		rlc_data = ir_buffer[standby_bufferout];

		/*The MSB bit is the status bit, LSB 7 bits is time ticks, If some one status is too long, mult-bytes maybe used.*/
		pulse_width += ((rlc_data & 0x7F) * CLK_CYC_US);/*Pulse width.*/
		pulse_polarity = ((rlc_data&0x80)>>7);
		standby_bufferout++;
		if(standby_bufferout == 0xFF)
		{
			standby_bufferout = 0;
		}
		
		/*Long pulse.*/
		if((!((rlc_data ^ ir_buffer[standby_bufferout]) & 0x80)) && (standby_bufferout != standby_bufferin))
		{
			continue;
		}

		cur_code = stdby_irc_pulse_to_code(pulse_width, pulse_polarity);
		if(STANDBY_IR_KEY == cur_code)
		{
			Exit_Standby_Process();
		}
		else if(0x0 == cur_code)
		{
			stdby_uart_debug('0');
		}
		else if(PAN_KEY_INVALID == cur_code)
		{
			stdby_uart_debug('F');
		}

		pulse_width = 0;
	}
}

/***************************************************************************
* Function_Name: IR_ISR
* Description: Receive the remote controller signal from Interrupt Requested.
* Arguments: None
* Return Value: None
****************************************************************************/
void sys_ir_standby_exception(void)
{
	volatile unsigned char status = 0, num = 0, num1 = 0;

	stdby_uart_debug('a');
	status = (READ_BYTE(IR_REG_ISR) & 3);
	if(status)
	{
		WRITE_BYTE(IR_REG_ISR, status);/*clear interrupt.*/
		stdby_uart_debug('b');
		do{
			num1 = num = (READ_BYTE(IR_REG_FIFOCTRL) & 0x7F);
			while(num > 0)
			{
				/*Put RLC to buffer.*/
				ir_buffer[standby_bufferin++] = READ_BYTE(IR_REG_RLCBYTE);
				if(standby_bufferin == 0xFF)
				{
					standby_bufferin = 0;
				}

				num--;
			}
		}while (num1 > 0);

		stdby_uart_debug('c');
		if(status == 0x02)
		{
			stdby_uart_debug('d');
			stdby_IR_GenerateCode();
			stdby_uart_debug('e');

			num = READ_BYTE(IR_REG_FIFOCTRL) & 0x7F;
			while(num > 0)
			{
				READ_BYTE(IR_REG_RLCBYTE);
				num--;
			}

			standby_bufferin = 0;
			standby_bufferout = 0;
		}
	}
	stdby_uart_debug('f');
}

void stdby_init_ir(void)
{
	standby_bufferin = 0;
	standby_bufferout = 0;
	ir_buffer[0] = 0x0;

#ifdef _M3702_
	WRITE_DW(NB_IB_ENABLE_REG1, 0);
	WRITE_DW(NB_IB_ENABLE_REG2, 0);
	WRITE_DW(NB_IB_ENABLE_REG3, 0);
	WRITE_DW(NB_IB_ENABLE_REG4, 0);
	WRITE_DW(NB_IB_ENABLE_REG5, 0);
	WRITE_DW(NB_IB_ENABLE_REG6, 0);
#else
	WRITE_DW(NB_IB_ENABLE_REG, 0);
	WRITE_DW((NB_IB_ENABLE_REG+4), 0);
#endif

	/*Clear CR.*/
	WRITE_BYTE(IR_REG_CFG, 0);

	/*Enable IRC.*/
	WRITE_BYTE(IR_REG_CFG, DEFAULT_IRC_VALUE);

	/*Init FIFO, threshould is 32 bytes.*/
	WRITE_BYTE(IR_REG_FIFOCTRL, IR_FIFO_SIZE);

	/*When the input signal keep at a status (H or L) for a long time,
	this means the previous transmission finished, the timeout interrupt 
	will assert if the interrupt is enable. The time limited is:
	(TIMETHR+1)*128*(work clock cycle).
	*/
	/*Timeout:6*128*CLK work.*/
	WRITE_BYTE(IR_REG_TIMETHR, IR_TIMETHR_VALUE);

	/*Noise filer:1*CLKwork.*/
	WRITE_BYTE(IR_REG_NOISETHR, NOISETHR_CLK);

	/*Clear all interrupt.*/
	WRITE_BYTE(IR_REG_ISR, IR_TIMEOUT_FIFO_INT);

	/*Enable all interrupt.*/
	WRITE_BYTE(IR_REG_IER, IR_TIMEOUT_FIFO_INT);
}
