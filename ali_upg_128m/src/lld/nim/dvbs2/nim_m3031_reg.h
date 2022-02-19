#ifndef __NIM_M3031_REG_H
#define __NIM_M3031_REG_H

#define ADDR_MASK_ID 0x00  //Unique identifier for RF 3031 IP. Increase by 1 on each tapeout. S3031a -> 0x01, S3031b -> 0x02,
#define ADDR_BB_RSSI_EN 0x10  //Baseband RSSI Enable
#define ADDR_BUF_EN 0x11  //Baseband Buffer Enable
#define ADDR_DGB_EN 0x12  //DGB Enable
#define ADDR_LNA_EN 0x13  //LNA Enable
#define ADDR_LPF_EN 0x14  //LPF Enam ble
#define ADDR_LT_EN 0x15  //LT Enable
#define ADDR_MIX_EN 0x16  //Mixer Enable
#define ADDR_PLL_EN 0x17  //PLL enable
#define ADDR_RF_RSSI_EN 0x18  //RF RSSI Enable
#define ADDR_VGA_EN 0x19  //VGA Enable
#define ADDR_BB_DBAND 0x1A  //Baseband RSSI Dead Band Control
#define ADDR_BB_MODE 0x1B  //Baseband Mode Selection
#define ADDR_BB_VRMS 0x1C  //Baseband RSSI VRMS setting
#define ADDR_BUF_DRIVE 0x1D  //Output Buffer Driving Strength (0=0.5mA, 1=1mA, 2=2mA, 3=4mA)
#define ADDR_DC_COMP_SW_OFFSET 0x1E  //Switch DC comparator polarity (Should NOT be merged with DC_INVERT_COMP)
#define ADDR_LNA_HPW 0x1F  //LNA High Power Mode
#define ADDR_LO_MUTE 0x20  //LO Mute (at squaring buffer)
#define ADDR_LPF_CUTOFF 0x21  //Cut Off Frequency selection
#define ADDR_LPF_TUNING 0x22  // Tune the RC time constant according to RC oscillator
#define ADDR_MIX_CAP 0x23  //MIX bandwidth selection
#define ADDR_MIX_CM 0x24  //MIX Common Mode 
#define ADDR_MIX_DC_CURRENT 0x25  //Set DC correction range on Mixer (value are TBD)
#define ADDR_PLL_CP 0x26  //Set Charge Pump Frequency. Values are TBD. 0 is Open loop for characterization purpose
#define ADDR_PLL_D 0x27  //Reference Divider. Fref = FXO/D (Valid range is 2:31))
#define ADDR_PLL_DIV_BIAS 0x28  //Divider Bias setting( increase current by 25% in CML dividers)
#define ADDR_PLL_FLTR 0X28  //
#define ADDR_PLL_M_MSB 0x29  //for Main Divider Fref=fvco/(16+M) (all values are valid) 
#define ADDR_PLL_M_LSB 0x2A  //Main Divider Fref=fvco/(16+M) (all values are valid)
#define ADDR_PLL_P 0x2B  //VCO Post Divider (0 -> %2, 1-> %4)
#define ADDR_REG_A 0x2C  //Reserved Register A
#define ADDR_REG_B 0x2D  //Reserved Register B
#define ADDR_REG_C 0x2E  //Reserved Register C
#define ADDR_RF_DBAND 0x2F  //RF RSSI Dead Band
#define ADDR_RF_VRMS 0x30  //RF RSSI VRMS 
#define ADDR_VCO_RCFREQ 0x31  //Allow to measure VCO usign RCFREQ (0 --> LPF Oscillator, 1 --> VCO reference frequency)
#define ADDR_VCO_VRMS 0x32  //VCO RSSI VRMS (0 -> 0.75Vpp, 1 -> 1Vpp, 2 - > 1.25Vpp, 3-> 1.5Vpp)
#define ADDR_XO_AMP 0x33  //Control XO amplitude (0--> 800mV, 1 -> 700mV, 2 -> 600mV, 3 -> 400mV)
#define ADDR_VGA_COMB_GAIN 0x35
#define ADDR_RSSI 0x40  //DC comparator I
#define ADDR_RSSI_STCK1 0x41  //DC comparator I
#define ADDR_RSSI_STCK2 0x42  //Monitor B (Reserved)
#define ADDR_TOP_1 0x50  //Each sub-block function on bit, RC_FREQ.
#define ADDR_MO_TOP_FSM 0x51  //Current FSM status code.
#define ADDR_TOP_3 0x52  //
#define ADDR_TOP_4 0x53  //
#define ADDR_MAN_RFAGC 0x54  //RFAGC control. 1:FSM stop. 0: FSM active. 0->1 holding. 1->0 restoring
#define ADDR_RFAGC_LGT_MIN 0x55  //The RFAGC coarse gain bottom index
#define ADDR_RFAGC_LGT_MAX 0x56  //The RFAGC coarse gain top index
#define ADDR_RFAGC_LGT_START 0x57  //The RFAGC coarse gain start point/index.
#define ADDR_MO_GAIN_C_INDEX 0x58  //When TUNE_RFAGC == 0, internal coarse gain index. Else, manual mode coarse gain setting,RG_GAIN_C_INDEX.
#define ADDR_MO_GAIN_F_INDEX 0x59  //When TUNE_RFAGC == 0, internal fine gain index. Else, manual mode fine gain setting,RG_GAIN_F_INDEX.
#define ADDR_RFAGC_SPEED 0x5a  //The RFAGC loop speed controller.
#define ADDR_RFAGC_8 0x5b  //RFAGC gain table entry selection
#define ADDR_RFAGC_9 0x5c  //This register is used to convey data when write operation or reflects the value of chosen entry when read operation
#define ADDR_RFAGC_10 0x5d  //The middle 8 bits of chosen entry
#define ADDR_RFAGC_11 0x5e  //The highest 2 bits of chosen entry
#define ADDR_RG_GAIN_C_INDEX 0x5f  //From register, as the manual setting of the coarse gain index. to select the corrsponding gain table.
#define ADDR_RG_GAIN_F_INDEX 0x60  //From register, as the manual setting of the fine gain index/value.
#define ADDR_RFAGC_14 0x61  //
#define ADDR_RFAGC_15 0x62  //
#define ADDR_DC_INIT_1 0x63  //From register, configures the manual mode seletction. 0: use the feedback value from register for DC compasation(settle is pull high).  1: manual calculation of the DC compasation value(settle is pull low).
#define ADDR_DC_INIT_2 0x64  // 3: 540 Tu â‰?160us.
#define ADDR_RG_MIX_DC_I_LOW 0x65  //From register, to manual set the MIX_DC_I_LOW.
#define ADDR_RG_MIX_DC_Q_LOW 0x66  //From register, to manual set the MIX_DC_Q_LOW.
#define ADDR_RG_MIX_DC_I_HIGH 0x67  //From register, to manual set the MIX_DC_I_HIGH.
#define ADDR_RG_MIX_DC_Q_HIGH 0x68  //From register, to manual set the MIX_DC_Q_HIGH.
#define ADDR_RG_VGA_DC_I 0x69  //From register, to manual set the VGA_DC_I.
#define ADDR_RG_VGA_DC_Q 0x6a  //From register, to manual set the VGA_DC_Q.
#define ADDR_MO_MIX_DC_I_LOW 0x6b  //To register, to report MIX_DC_I_LOW.
#define ADDR_MO_MIX_DC_Q_LOW 0x6c  //To register, to report MIX_DC_Q_LOW.
#define ADDR_MO_MIX_DC_I_HIGH 0x6d  //To register, to report MIX_DC_I_HIGH.
#define ADDR_MO_MIX_DC_Q_HIGH 0x6e  //To register, to report MIX_DC_Q_HIGH.
#define ADDR_MO_VGA_DC_I 0x6f  //To register, to report VGA_DC_I.
#define ADDR_MO_VGA_DC_Q 0x70  //To register, to report VGA_DC_Q.
#define ADDR_DC_INIT_15 0x71  //To register, to report DC_COMP_I.
#define ADDR_DC_INIT_16 0x72  //
#define ADDR_DC_INIT_17 0x73  //
#define ADDR_MO_OSC_FREQ_L 0x74  //monitor the OSC_FREQ by Register, low 8bit.
#define ADDR_MO_OSC_FREQ_H 0x75  //monitor the OSC_FREQ by Register, uppper 2-bit.
#define ADDR_RC_FREQ_3 0x76  //
#define ADDR_RC_FREQ_4 0x77  //
#define ADDR_BBAGC_1 0x78  //BBAGC manual control flag 0->Autocontrol, 1->Manual Control
#define ADDR_BBAGC_2 0x79  //Manually Set the DGB gain 0~55.
#define ADDR_BBAGC_3 0x7a  //Report of the DGB gain 0~55.
#define ADDR_BBAGC_4 0x7b  //
#define ADDR_BBAGC_5 0x7c  //
#define ADDR_MO_VCO_AC 0x7d  //To register:
#define ADDR_PLL_INIT_2 0x7e  //if RG_AMPSET_MAN =1:
#define ADDR_PLL_INIT_3 0x7f  //From the I2C , register
#define ADDR_PLL_INIT_WAIT 0x80  //From the I2C , register
#define ADDR_PLL_DAC_HIGH 0x81  //From the I2C , register
#define ADDR_PLL_DAC_LOW 0x82  //From the I2C , register
#define ADDR_RG_VCO_DAC_MAN 0x83  //From the I2C , register
#define ADDR_PLL_INIT_8 0x84  //From the I2C , register
#define ADDR_MO_VCO_DAC 0x85  //To register:
#define ADDR_MO_VCO_BAND 0x86  //To register:
#define ADDR_PLL_INIT_11 0x87  //
#define ADDR_PLL_INIT_12 0x88  //

#endif

