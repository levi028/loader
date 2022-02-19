#include <dp8051xp.h>
#include <stdio.h>
#include <intrins.h>
#include "sys.h"
#include "pmu_cec.h"
//=========================================================================================================//

#ifdef HDMI_CEC_SUPPORT
static UINT8 g_cec_wk = 0;
UINT8 g_cec_wake_up_fn[CEC_WKUP_GRP_ZIZE][CEC_CMD_SZIE_BYTE] =
{
	{0x00,0x44,0x6D,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
};

void pmu_cec_init(void)
{
	UINT8 i = 0, j = 0, log_addr = 0;

	/*set cec logic addr.*/
	CEC_SET_LOGADD(LOGIC_ADDR);
#ifdef PMU_MCU_DEBUG
	printf("cec_set_logaddr=%0bx\n", PMU_READ_BYTE(LOGIC_ADDR_REG));
#endif

	/*set cec div.*/
	CEC_SET_DIV(CEC_DIV);
#ifdef PMU_MCU_DEBUG
	printf("cec_set_iv=%0bx\n", PMU_READ_BYTE(DIVIDER_REG));
#endif

	/*set cec frq.*/
	CEC_SET_FRQ(CEC_FREQ);
#ifdef PMU_MCU_DEBUG
	printf("cec_set_freq=%0bx\n", PMU_READ_BYTE(FRQ_RES_REG));
#endif

	/*set logic address.*/
	log_addr =CEC_GET_LOGADDR();
#ifdef PMU_MCU_DEBUG
	printf("cec_logic_addr=%0bx\n", CEC_GET_LOGADDR());
#endif

	/*set wakeup cmd.*/
	for(j=0; j<CEC_WKUP_GRP_ZIZE; j++)
	{
	#ifdef PMU_MCU_DEBUG
		printf("\n");
	#endif

		for(i=0; i<CEC_CMD_SZIE_BYTE; i++)
		{
			if((i==0) && (j==0))/*only config first grp.*/
			{
				g_cec_wake_up_fn[j][i] = ((g_cec_wake_up_fn[j][i])&0xf0) | (log_addr&0x0f);
			}
			PMU_WRITE_BYTE((CEC_WKUP_CMD1_REG+CEC_CMD_SZIE_BYTE*j+i), g_cec_wake_up_fn[j][i]);

		#ifdef PMU_MCU_DEBUG
			printf("grp=%0bx, index=%0bx, cmd reg =%0bx\n", j, i, PMU_READ_BYTE(CEC_WKUP_CMD1_REG+CEC_CMD_SZIE_BYTE*j+i));
		#endif
		}
	}

	/*set cee irp.*/
	CEC_SYS_INT_IPR();

	/*enable cec interrupt.*/
	CEC_SYS_INT_ENABLE();

#ifdef PMU_MCU_DEBUG
	printf("cec interrupt reg =%0bx\n",PMU_READ_BYTE(SYS_REG_IER));
#endif
}

char get_cec_cmd(void)
{
	char ret = ERROR;
	if(g_cec_wk == 1)
	{
		ret = SUCCESS;
	}

	return ret;
}

void set_get_cec_cmd(char status)
{
	if(status == 1)
	{
		g_cec_wk =1;
	}
	else
	{
		g_cec_wk =0;
	}
}
#endif
