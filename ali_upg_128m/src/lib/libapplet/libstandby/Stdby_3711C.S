//=====================================================================================//
#include <sys_config.h>
#include <hal/mips.h>
//=====================================================================================//

#define SYS_IC_NB_BASE_H                                                     (0xB800)
#define SYS_IC_NB_SMER                                                         (0x68)
#define SYS_IC_NB_EISR                                                          (0x30)
#define SYS_IC_NB_EIMR                                                          (0x38)
#define SYS_IC_NB_EIMR2                                                        (0x3C)
#define SYS_IC_SB_IRC_BITIE                                                   (0x80000) 
#define SYS_IC_SB_WATCHDOG                                                (0x18500)
#define SB_WATCHDOG_CTR                                                     (0x4)
#define SB_WATCHDOG_TIMER                                                  (0x0)
#define TIME_WAKE_DISABLE                                                   (1)
#define ALI_POWER_CTRL_REG                                                 (0x68)
#define ALI_CPU_STANDBY_ENABLE                                           (0x5)
#define GLOBAL_INT_ENABLE                                                    (0x801)

/*Define one second ticks.*/
#define TIME_COMPARE_1S                                                     0x66FF30/*(27000000/4).*/
#define MMFC0(rt, rd, sel) \
	.word (0x40000000) | (rt<<16) | (rd<<11) | sel

#define MMTC0(rt, rd, sel) \
	.word (0x40800000) | (rt<<16) | (rd<<11) | sel

#define STANDBY_POS(x) ((x) * 4)
#define STANDBY_OFS_AT STANDBY_POS(1)
#define STANDBY_OFS_V0 STANDBY_POS(2)
#define STANDBY_OFS_V1 STANDBY_POS(3)
#define STANDBY_OFS_A0 STANDBY_POS(4)
#define STANDBY_OFS_A1 STANDBY_POS(5)
#define STANDBY_OFS_A2 STANDBY_POS(6)
#define STANDBY_OFS_A3 STANDBY_POS(7)
#define STANDBY_OFS_T0 STANDBY_POS(8)
#define STANDBY_OFS_T1 STANDBY_POS(9)
#define STANDBY_OFS_T2 STANDBY_POS(10)
#define STANDBY_OFS_T3 STANDBY_POS(11)
#define STANDBY_OFS_T4 STANDBY_POS(12)
#define STANDBY_OFS_T5 STANDBY_POS(13)
#define STANDBY_OFS_T6 STANDBY_POS(14)
#define STANDBY_OFS_T7 STANDBY_POS(15)
#define STANDBY_OFS_S0 STANDBY_POS(16)
#define STANDBY_OFS_S1 STANDBY_POS(17)
#define STANDBY_OFS_S2 STANDBY_POS(18)
#define STANDBY_OFS_S3 STANDBY_POS(19)
#define STANDBY_OFS_S4 STANDBY_POS(20)
#define STANDBY_OFS_S5 STANDBY_POS(21)
#define STANDBY_OFS_S6 STANDBY_POS(22)
#define STANDBY_OFS_S7 STANDBY_POS(23)
#define STANDBY_OFS_T8 STANDBY_POS(24)
#define STANDBY_OFS_T9 STANDBY_POS(25)
#define STANDBY_OFS_K0 STANDBY_POS(26)
#define STANDBY_OFS_K1 STANDBY_POS(27)
#define STANDBY_OFS_GP STANDBY_POS(28)
#define STANDBY_OFS_FP STANDBY_POS(29)
#define STANDBY_OFS_HI STANDBY_POS(30)
#define STANDBY_OFS_LO STANDBY_POS(31)

#define STANDBY_SAVE_SOME \
		subu	sp, 128; \
		.set 	noat; \
		sw $1, STANDBY_OFS_AT(sp); \
		sw v0, STANDBY_OFS_V0(sp); \
		sw v1, STANDBY_OFS_V1(sp); \
		sw a0, STANDBY_OFS_A0(sp); \
		sw a1, STANDBY_OFS_A1(sp); \
		sw a2, STANDBY_OFS_A2(sp); \
		sw a3, STANDBY_OFS_A3(sp); \
		sw t0, STANDBY_OFS_T0(sp); \
		sw t1, STANDBY_OFS_T1(sp); \
		sw t2, STANDBY_OFS_T2(sp); \
		sw t3, STANDBY_OFS_T3(sp); \
		sw t4, STANDBY_OFS_T4(sp); \
		sw t5, STANDBY_OFS_T5(sp); \
		sw t6, STANDBY_OFS_T6(sp); \
		sw t7, STANDBY_OFS_T7(sp); \
		sw s0, STANDBY_OFS_S0(sp); \
		sw s1, STANDBY_OFS_S1(sp); \
		sw s2, STANDBY_OFS_S2(sp); \
		sw s3, STANDBY_OFS_S3(sp); \
		sw s4, STANDBY_OFS_S4(sp); \
		sw s5, STANDBY_OFS_S5(sp); \
		sw s6, STANDBY_OFS_S6(sp); \
		sw s7, STANDBY_OFS_S7(sp); \
		sw t8, STANDBY_OFS_T8(sp); \
		sw t9, STANDBY_OFS_T9(sp); \
		sw k0, STANDBY_OFS_K0(sp); \
		sw k1, STANDBY_OFS_K1(sp); \
		sw gp, STANDBY_OFS_GP(sp); \
		sw fp, STANDBY_OFS_FP(sp); \
		mfhi	k0; \
		sw	k0, STANDBY_OFS_HI(sp); \
		mflo	k0; \
		sw	k0, STANDBY_OFS_LO(sp); \
		.set at; \
		nop

#define STANDBY_LOAD_SOME \
		.set 	noat; \
		lw k0, STANDBY_OFS_HI(sp); \
		mthi	k0; \
		lw k0, STANDBY_OFS_LO(sp); \
		mtlo	k0; \
		lw $1, STANDBY_OFS_AT(sp); \
		lw v0, STANDBY_OFS_V0(sp); \
		lw v1, STANDBY_OFS_V1(sp); \
		lw a0, STANDBY_OFS_A0(sp); \
		lw a1, STANDBY_OFS_A1(sp); \
		lw a2, STANDBY_OFS_A2(sp); \
		lw a3, STANDBY_OFS_A3(sp); \
		lw t0, STANDBY_OFS_T0(sp); \
		lw t1, STANDBY_OFS_T1(sp); \
		lw t2, STANDBY_OFS_T2(sp); \
		lw t3, STANDBY_OFS_T3(sp); \
		lw t4, STANDBY_OFS_T4(sp); \
		lw t5, STANDBY_OFS_T5(sp); \
		lw t6, STANDBY_OFS_T6(sp); \
		lw t7, STANDBY_OFS_T7(sp); \
		lw s0, STANDBY_OFS_S0(sp); \
		lw s1, STANDBY_OFS_S1(sp); \
		lw s2, STANDBY_OFS_S2(sp); \
		lw s3, STANDBY_OFS_S3(sp); \
		lw s4, STANDBY_OFS_S4(sp); \
		lw s5, STANDBY_OFS_S5(sp); \
		lw s6, STANDBY_OFS_S6(sp); \
		lw s7, STANDBY_OFS_S7(sp); \
		lw t8, STANDBY_OFS_T8(sp); \
		lw t9, STANDBY_OFS_T9(sp); \
		lw k0, STANDBY_OFS_K0(sp); \
		lw k1, STANDBY_OFS_K1(sp); \
		lw gp, STANDBY_OFS_GP(sp); \
		lw fp, STANDBY_OFS_FP(sp); \
		addu sp, 128; \
		.set at; \
		nop

  #define STANDBY_SAVE_PARA \
		.set noat; \
		subu sp, 8; \
		sw a0, 4(sp); \
		sw a1, 0(sp); \
		.set at; \
		nop

  #define STANDBY_LOAD_PARA \
		.set noat; \
		lw a1, 0(sp); \
		lw a0, 4(sp); \
		addu sp,8; \
		.set at; \
		nop

#define STANDBY_SAVE_RA \
		subu sp,	4; \
		sw ra, 0(sp); \
		nop

#define STANDBY_LOAD_RA \
		lw ra, 0(sp); \
		addu sp,	4; \
		nop

#define STANDBY_SAVE_EPC \
		subu	 sp,	4; \
		mfc0 k0,	C0_EPC; \
		HB; \
		nop; \
		sw k0, 0(sp); \
		nop

#define STANDBY_LOAD_EPC \
		lw k0, 0(sp); \
		mtc0 k0, C0_EPC; \
		HB; \
		nop; \
		addu sp,	4; \
		nop

#define STANDBY_PUSH(x) \
		subu	 sp,	4; \
		sw x, (sp); \
		nop

#define STANDBY_POP(x) \
		lw x, (sp); \
		addu sp,	4; \
		nop
//=====================================================================================//

	.set noreorder
	.set noat
#--------------------------------------------------------------------
# Function_Name :  void sys_cpu_standby_exception_enter(void)
# Description       :  handle the interrupt in sdk for standby mode
#                             save general regs into stack, then recovery them. 
# Arguments         : None
# Return Value      : None
#--------------------------------------------------------------------
	.text
	.align 12
	.byte 0
	.align 8
	.byte 0
	.align 7
	.globl sys_cpu_standby_exception_enter
	.ent sys_cpu_standby_exception_enter
sys_cpu_standby_exception_enter:
	/*Backup general regs.*/
	STANDBY_SAVE_PARA/*save prarameters.*/
	STANDBY_SAVE_RA/*push ra.*/
	STANDBY_SAVE_SOME/*push gr.*/
	STANDBY_SAVE_EPC/*push epc.*/

	/*If there is some unexpect exeception occured, STB reboot.*/
	mfc0        t1, C0_CAUSE
	nop
	nop
	nop
	nop
	li             t0, 0x7C/*Bit2~Bit6 descripted ExcCode.*/
	and         t1, t0
	bnez        t1, exit_exception
	nop

	mfc0        t1, C0_SR/*disable globle interrupt.*/
	nop
	nop
	nop
	nop

	lui           t3, 0xFFFF
	ori           t3, 0xFFFE

	and         t2, t1, t3
	nop
	mtc0       t2, C0_SR
	nop
	nop
	nop
	nop

	jal            stdby_uart_debug
	li              a0, 'A'
	nop
 
	jal           sys_ir_standby_exception/*Standby interrupt process function.*/
	nop

	jal            stdby_uart_debug
	li              a0, 'B'
	nop

	/*relative context restore.*/
	STANDBY_LOAD_EPC
	STANDBY_LOAD_SOME
	STANDBY_LOAD_RA
	STANDBY_LOAD_PARA

#if 0/*Add for test.*/
loop2:
	b            loop2
	nop
#endif

	.set mips3
	eret/*Return epc address to go on run.*/
	.set mips2
	nop
	nop
	.end sys_cpu_standby_exception_enter

# ------------------------------------------------------------------------------
# void fill_st_code_into_icache(void)
# This function is fill the standby code stage into i-cache.
# i-cache size will define by the chipset.M3503C is 16k.
# @return none:
# @used t0, t1
# ------------------------------------------------------------------------------
	.globl fill_st_code_into_icache
	.ent fill_st_code_into_icache
fill_st_code_into_icache:
	la              t0, standby_run_in_dram
	lui            t1, 0xA000
	ori             t1, 0x0000

	or             t0, t1
	j               t0
	nop

standby_run_in_dram:
	/*load code to i cache.*/
	la            t0, __STANDBY_CODE_START
	la            t1, __STANDBY_CODE_END
	lui            t3, 0x8FFF
	ori           t3, 0xFFFF

	and         t0, t3
	and         t1, t3
	nop

	.set          mips3
1:
	cache       0x14, 0x0(t0)
	cache       0x1C, 0x0(t0)
	sync
	.set          mips1
	addiu        t0, 16
	nop
	bltu          t0, t1, 1b
	nop

	la             t0, jump_to_cache
	lui            t1, 0x8FFF
	ori            t1, 0xFFFF
	
	and          t0, t1
	j              t0
	nop
	.end fill_st_code_into_icache

# ------------------------------------------------------------------------------
# void fill_st_data_into_dcache(void)
# This function is load the standby data stage into d-cache.
# d-cache size will define by the chipset.M3503C is 16k.
# @return none:
# @used t0, t1,t2,t3
# ------------------------------------------------------------------------------
	.globl fill_st_data_into_dcache
	.ent fill_st_data_into_dcache
fill_st_data_into_dcache:
	la              t0, __STANDBY_DATA_START
	la              t1, __STANDBY_DATA_END
	li               t3, 1
	nop

1:
	lw             t2, 0(t0)
	andi           t2, 0x3
	addu          t3, t2
	addiu         t0, 4
	nop
	bltu           t0, t1, 1b
	nop

	jr              ra
	nop
	.end fill_st_data_into_dcache

/*************************************************************
CPU standby main process: cpu_standby_process
*************************************************************/
	.globl cpu_standby_process
	.ent cpu_standby_process
cpu_standby_process:
	jal            fill_st_code_into_icache
	nop

jump_to_cache:
	jal            fill_st_data_into_dcache
	nop

	la             t0,  __STANDBY_DATA_SP
	move        sp, t0

	jal            stdby_uart_debug
	li              a0, '1'

	move        a0, s7
	move        a1, s5
	jal            sys_prestandby
	nop

	jal            stdby_uart_debug
	li              a0, '2'

	jal            soc_enter_real_standby
	nop

	jal            stdby_uart_debug
	li              a0, '3'

	jal            mem_enter_standby
	nop

	jal            stdby_uart_debug
	li              a0, '4'

	lui            t0, 0xB800
	sw           zero, SYS_IC_NB_EIMR(t0)/*Disable interrupt----0xB8000038.*/
	sw           zero, SYS_IC_NB_EIMR2(t0)/*Disable interrupt----0xB800003C.*/
	nop

	lui            t1, 0x8/*Enable ir intrrupt.*/ 
	ori           t1, 0x0000
	sw           zero, 0x28(t0)/*Also set interrupt polarity.*/
	sw           t1, 0x28(t0)

	jal            stdby_uart_debug
	li              a0, '5'

	jal            enable_cpu_globle_interrupt/*Enable globle interrupt.*/
	nop

	jal           stdby_uart_debug
	li              a0, '6'

	jal            Set_C0_COUNT_1S
	nop

	jal           stdby_uart_debug
	li              a0, '7'

loop:
	jal           disable_cpu_globle_interrupt/*Disable globle interrupt.*/
	nop

	mfc0        t0, C0_COUNT
	nop
	nop
	nop
	nop
	mfc0        t1, C0_COMPARE
	nop
	nop
	nop
	nop
	subu        t0, t1, t0
	bgez        t0, slot
	nop

	/*check time to exit standby.*/
	jal           stby_update_time
	nop

	/*update count.*/
	lui            t2, 0x66
	ori            t2, 0xFF30

	mfc0        t1, C0_COMPARE
	nop
	nop
	nop
	nop
	addu        t1, t2
	mtc0        t1, C0_COMPARE
	nop
	nop
	nop
	nop

slot:
	jal           disable_cpu_globle_interrupt/*Disable globle interrupt.*/
	nop

	jal           slot_process
	nop

	jal           enable_cpu_globle_interrupt/*Enable globle interrupt.*/
	nop
	nop
	nop

	lui            t0, SYS_IC_NB_BASE_H
	sw           zero, SYS_IC_NB_EIMR(t0)/*Disable interrupt----0xB8000038.*/
	sw           zero, SYS_IC_NB_EIMR2(t0)/*Disable interrupt----0xB800003C.*/
	nop
	lui            t1, 0x8/*Enable ir intrrupt.*/
	sw           t1, SYS_IC_NB_EIMR(t0)
	nop
	nop
	nop

	/*delay some time.*/
	li              t1, 0x2000
1:
	subu         t1, 1
	nop
	bgez         t1, 1b
	nop

	j             loop
	nop
	.end cpu_standby_process

/*************************************************************
CPU exit standby and wait for rebooting again.
*************************************************************/
	.globl exit_standby
	.ent exit_standby
exit_standby:
	li             s8, 0xDEADBEAD/*Clear standby flag.*/
	lui           t0, SYS_IC_NB_BASE_H

	/*Clear all interrupts.*/
	sw           zero, 0x38(t0)
	sw           zero, 0x3C(t0)
	nop
	nop

	jal            stdby_uart_debug
	li              a0, 'Z'

	/*WDT reboot.*/
	lui            t0, 0xB801
	ori            t0, 0x8500

	lui            t1, 0xFFFF
	ori            t1, 0xFF00
	sw            t1, SB_WATCHDOG_TIMER(t0)
	li              t1, 0x64  
	sb            t1, SB_WATCHDOG_CTR(t0)
	nop
        
1:      
	b             1b
	nop
	nop
	.end exit_standby

/*************************************************************
CPU reboot when some unexpect exception occured.
*************************************************************/
	.globl exit_exception
	.ent exit_exception
exit_exception:
	mfc0        t1, C0_CAUSE
	move       s8, t1/*Record exception cause for analysis in next step.*/
	nop
	nop
	nop

	/*Clear all interrupts.*/
	lui            t0, SYS_IC_NB_BASE_H
	sw           zero, 0x38(t0)
	sw           zero, 0x3C(t0)
	nop
	nop

	/*When uart output 'eee', it means that some unexpect exeception occured!.*/
	jal            stdby_uart_debug
	li              a0, 'e'
	jal            stdby_uart_debug
	li              a0, 'e'
	jal            stdby_uart_debug
	li              a0, 'e'

	/*WDT reboot.*/
	lui            t0, 0xB801
	ori            t0, 0x8500
	lui            t1, 0xFFFF
	ori            t1, 0xFF00
	sw            t1, SB_WATCHDOG_TIMER(t0)
	li              t1, 0x64  
	sb            t1, SB_WATCHDOG_CTR(t0)
	nop
        
1:      
	b             1b
	nop
	nop
	.end exit_exception

/*************************************************************
Set C0_SR to enable globle interrupt
*************************************************************/
	.globl enable_cpu_globle_interrupt
	.ent enable_cpu_globle_interrupt
enable_cpu_globle_interrupt:
	mfc0         t1, C0_SR/*Enable globle interrupt.*/
	nop
	nop
	nop
	nop
	li              t3, GLOBAL_INT_ENABLE
	or            t2, t1, t3
	nop
	mtc0         t2, C0_SR
	nop
	nop
	nop
	nop

	jr              ra
	nop
	.end enable_cpu_globle_interrupt

/*************************************************************
void stdby_uart_debug(UINT32 data)
a0: print data
print debug information
*************************************************************/
	.globl stdby_uart_debug
	.ent stdby_uart_debug
stdby_uart_debug:
	lui            t0, 0xB801
	ori            t0, 0x8300
	sb            a0, 0(t0)
	nop

	/*delay some time to wait for uart output.*/
	li              t1, 0x2000
1:
	subu         t1, 1
	nop
	bgez         t1, 1b
	nop

	jr             ra
	nop
	.end stdby_uart_debug

/*************************************************************
set ticks to count one second.
*************************************************************/
	.globl Set_C0_COUNT_1S
	.ent Set_C0_COUNT_1S
Set_C0_COUNT_1S:
	/*Initialize time count.*/
	lui            t0, 0x66
	ori            t0, 0xFF30

	mtc0        zero, C0_COUNT/*Setup CP0 counter register.*/
	nop
	nop
	nop
	nop
	mtc0        t0, C0_COMPARE
	nop
	nop
	nop
	nop

	jr             ra
	nop
	.end Set_C0_COUNT_1S

#if 0/*Reserved for debug.*/
	.globl temp_storage
	.ent temp_storage
temp_storage:
	move       v1, ra
	move       s8,  a0

	move       ra, v1
	jr             ra
	nop
	.end temp_storage
#endif

/*************************************************************
Set current time.
*************************************************************/
	.globl stdby_save_time
	.ent stdby_save_time
stdby_save_time:
	move       s5,  a0
	nop
	nop
	nop

	jr             ra
	nop
	.end stdby_save_time

/*************************************************************
Set C0_SR to disable globle interrupt
*************************************************************/
	.globl disable_cpu_globle_interrupt
	.ent disable_cpu_globle_interrupt
disable_cpu_globle_interrupt:
	mfc0        t1, C0_SR/*disable globle interrupt.*/
	nop
	nop
	nop
	nop

	lui            t3, 0xFFFF
	ori            t3, 0xFFFE

	and         t2, t1, t3
	nop
	mtc0       t2, C0_SR
	nop
	nop
	nop
	nop

	jr            ra
	nop
	.end disable_cpu_globle_interrupt

/*************************************************************
CPU enter standby mode.
*************************************************************/
	.globl soc_enter_real_standby
	.ent soc_enter_real_standby
soc_enter_real_standby:
	lui           t0, SYS_IC_NB_BASE_H
	lw          t1, ALI_POWER_CTRL_REG(t0)
	ori          t1, ALI_CPU_STANDBY_ENABLE
	sw          t1, ALI_POWER_CTRL_REG(t0)
	nop

	/*Delay some time.*/
	li            t1, 0x2000
1:     
	bnez       t1, 1b
	addiu      t1, -1
	nop

	jr           ra
	nop
	.end soc_enter_real_standby

/*************************************************************
Set mem into low power status.
*************************************************************/
	.globl mem_enter_standby
	.ent mem_enter_standby
mem_enter_standby:
	/*Step1:DDR enter self-refresh mode.*/
	lui            t0, 0xB800
	ori            t0, 0x1000
	lw            t1, 0x30(t0)
	li             t2, (1<<15)
	or           t1, t2
	sw           t1, 0x30(t0)
	nop
	nop
	nop

	lui            t0, 0xB800
	ori            t0, 0x1000
	lw            t1, 0x30(t0)
	lui            t2, 0x7FFF
	ori            t2, 0xFFFF/*bit31 is cleared to '0'.*/
	and         t1, t2
	sw           t1, 0x30(t0)
	nop
	nop
	nop

	/*Step2:DDR PHY enter low power mode.*/
	lui            t0, 0xB803
	ori            t0, 0xE040
	lui            t1, 0x0004
	ori            t1, 0x0004
	sw           t1, 0(t0)
	nop

	lui            t1, 0x1104
	ori            t1, 0x1104
	sw           t1, 4(t0)
	nop

	lui            t1, 0x1107
	ori            t1, 0x1107
	sw           t1, 8(t0)
	nop

	lui            t1, 0x0007
	ori            t1, 0x00C0
	sw           t1, 0xC(t0)
	nop

	jr            ra
	nop
	.end    mem_enter_standby
	.set at
	.set reorder

.data
irq_status_save:
.space 256

