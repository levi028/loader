/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "ali_i2c_common.h"

#ifndef HZ
#define HZ 1000
#endif

#if defined (ALI_TDS)

#include <osal/osal.h>
#define udelay 		osal_delay

#define ioread8(addr) 				(*((volatile UINT8 *)(addr)))
#define iowrite8(val, addr) 	(*((volatile UINT8 *)(addr)) = (val))

static void mutex_init(MUTEX *mutex)
{
	if (NULL == mutex)
		return;

	*mutex = osal_mutex_create();
}

static void mutex_lock(MUTEX *mutex)
{
	if (NULL == mutex)
		return;

	osal_mutex_lock(*mutex, OSAL_WAIT_FOREVER_TIME);
}

static void mutex_unlock(MUTEX *mutex)
{
	if (NULL == mutex)
		return;

	osal_mutex_unlock(*mutex);
}

static void init_completion(COMPLETION *completion)
{
	if (NULL == completion)
		return;

	*completion = osal_flag_create(0);
}

// NOTE: set flag in HSR is must, or TDS will halt
static void complete_hsr(UINT32 completion)
{
	osal_flag_set(completion, 1);
}

static void complete(COMPLETION *completion)
{
	if (NULL == completion)
		return;

	osal_interrupt_register_hsr((T_HSR_PROC_FUNC_PTR)complete_hsr, *completion);
}

static unsigned long wait_for_completion_timeout(COMPLETION *completion, unsigned long timeout)
{
	UINT32 flags = 0;
	OSAL_ER result = OSAL_E_OK;

	if (NULL == completion)
		return -1;

	//result = osal_flag_wait(&flags, *completion, 1, OSAL_TWF_ORW, timeout + 200);// 200MS is the estimate time from ISR to HSR
	result = osal_flag_wait(&flags, *completion, 1, OSAL_TWF_ORW, OSAL_WAIT_FOREVER_TIME);
	if (OSAL_E_OK != result) {
		I2C_ERR("wait for completion fail with result: %d\n", result);
		return (result == OSAL_E_TIMEOUT) ? 0 : -1;
	}

	return 1;
}

#define INIT_COMPLETION(completion) osal_flag_clear((completion), 1)

#define local_irq_save(flags) do { (void)flags; osal_interrupt_disable(); } while (0)

#define local_irq_restore(flags) do { (void)flags; osal_interrupt_enable(); } while (0)

#elif defined (ALI_UBOOT)

#define ioread8(x)			readb(x)
#define iowrite8(val, x)		writeb(val, x)

#define init_completion(...)
#define complete(...)
#define wait_for_completion_timeout(...) 1
#define INIT_COMPLETION(...)

#else


#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/delay.h>

#include <linux/platform_device.h>


#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/clk-provider.h>
#include <linux/version.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
#define INIT_COMPLETION(completion) reinit_completion(&(completion))
#endif

#endif

static inline u8 ali_reg_read(struct ali_i2c *i2c, unsigned long reg)
{
	if (NULL == i2c) {
		I2C_ERR("i2c is NULL\n");
		return 0;
	}

	return ioread8(i2c->base + reg);
}

static inline void ali_reg_write(struct ali_i2c *i2c, unsigned long reg, u8 val)
{
	if (NULL == i2c) {
		I2C_ERR("i2c is NULL\n");
		return;
	}

	iowrite8(val, i2c->base + reg);
}

static inline void ali_irq_dis(struct ali_i2c *i2c, u32 mask)
{
	u8 val = 0;

	if (NULL == i2c) {
		I2C_ERR("i2c is NULL\n");
		return;
	}

	val = ali_reg_read(i2c, I2C_IER_REG_OFFSET);
	ali_reg_write(i2c, I2C_IER_REG_OFFSET, val & (~mask));
}

static inline void ali_irq_en(struct ali_i2c *i2c, u32 mask)
{
	u8 val = 0;

	if (NULL == i2c) {
		I2C_ERR("i2c is NULL\n");
	}

	val = ali_reg_read(i2c, I2C_IER_REG_OFFSET);
	ali_reg_write(i2c, I2C_IER_REG_OFFSET, val | mask);
}

static inline void ali_irq_clr(struct ali_i2c *i2c, u32 mask)
{
	u8 val = 0;

	if (NULL == i2c) {
		I2C_ERR("i2c is NULL\n");
		return;
	}

	val = ali_reg_read(i2c, I2C_ISR_REG_OFFSET);
	ali_reg_write(i2c, I2C_ISR_REG_OFFSET, val | mask);
}

static inline void ali_all_irq_clr(struct ali_i2c *i2c)
{
	u8 val = 0;

	if (NULL == i2c) {
		I2C_ERR("i2c is NULL\n");
		return;
	}

	/* Disable All interrupt */
	//val = ali_reg_read(i2c, I2C_IER_REG_OFFSET);
	//val &= ~0xFF;
	ali_reg_write(i2c, I2C_IER_REG_OFFSET, 0);

	/* Disable Trigger interrupt */
	val = ali_reg_read(i2c, I2C_IER1_REG_OFFSET);
	val &= ~I2C_IER1_TRIGGER_MASK;
	ali_reg_write(i2c, I2C_IER1_REG_OFFSET, val);
}

static inline void ali_irq_clr_en(struct ali_i2c *i2c, u32 mask)
{
	if (NULL == i2c) {
		I2C_ERR("i2c is NULL\n");
		return;
	}

	ali_irq_clr(i2c, mask);
	ali_irq_en(i2c, mask);
}

static void ali_clear_rx_fifo(struct ali_i2c *i2c)
{
	if (NULL == i2c) {
		I2C_ERR("i2c is NULL\n");
		return;
	}

#if !defined(ALI_TDS)// on TDS, the following loop will cause OS halt, reason is unknown
	while (!(I2C_HSR_RFIFO_MASK & ali_reg_read(i2c, I2C_HSR_REG_OFFSET))) {
		ali_reg_read(i2c, I2C_FDR_REG_OFFSET);
	}
#endif
}

static void ali_i2c_read_fifo(struct ali_i2c *i2c)
{
	if (NULL == i2c) {
		I2C_ERR("i2c is NULL\n");
		return;
	}

	// NOTE: Do not output message here, or read/write operation will fail.
	while ((i2c->xfer_len < i2c->msgs->len) &&
	       (!(ali_reg_read(i2c, I2C_HSR_REG_OFFSET) & I2C_HSR_RFIFO_MASK))) {
		i2c->msgs->buf[i2c->xfer_len++] = ali_reg_read(i2c, I2C_FDR_REG_OFFSET);
	}
}

static void ali_i2c_write_fifo(struct ali_i2c *i2c)
{
	if (NULL == i2c) {
		I2C_ERR("i2c is NULL\n");
		return;
	}

	// NOTE: Do not output message here, or read/write operation will fail.
	while ((i2c->xfer_len < i2c->msgs->len) &&
	       (!(ali_reg_read(i2c, I2C_HSR_REG_OFFSET) & I2C_HSR_WFIFO_MASK))) {
		ali_reg_write(i2c, I2C_FDR_REG_OFFSET, i2c->msgs->buf[i2c->xfer_len++]);
	}
}

static inline void ali_i2c_wakeup(struct ali_i2c *i2c, int code)
{
	if (NULL == i2c) {
		I2C_ERR("i2c is NULL\n");
		return;
	}

	i2c->xfer_result.err = code;
	complete(&i2c->xfer_result.completion);
}

static inline void ali_i2c_enable_trig_lvl_int(struct ali_i2c *i2c, int is_enable)
{
	u8 val = 0;

	if (NULL == i2c) {
		I2C_ERR("i2c is NULL\n");
		return;
	}

	val = ali_reg_read(i2c, I2C_IER1_REG_OFFSET);
	if (is_enable) {
		val |= I2C_IER1_TRIGGER_MASK;
	} else {
		val &= ~I2C_IER1_TRIGGER_MASK;
	}
	ali_reg_write(i2c, I2C_IER1_REG_OFFSET, val);
}

static void ali_i2c_set_target_byte_count(struct ali_i2c *i2c, u16 bc)
{
	if (NULL == i2c) {
		I2C_ERR("i2c is NULL\n");
		return;
	}

#if 0       
	ali_reg_write(i2c, I2C_BCR_REG_OFFSET, (((bc & (~I2C_FCR_BC_MASK)) >> 5) & 0xFF));
	ali_reg_write(i2c, I2C_FCR_REG_OFFSET, I2C_FCR_FIFO_MASK | (bc & I2C_FCR_BC_MASK));
#else
	ali_reg_write(i2c, I2C_BCR_REG_OFFSET, (bc >> 5) & 0xFF);
	ali_reg_write(i2c, I2C_FCR_REG_OFFSET, I2C_FCR_FIFO_MASK | (bc & I2C_FCR_BC_MASK));
#endif
	I2C_DBG("target byte count: %d, hsr: 0x%x\n", bc, ali_reg_read(i2c, I2C_HSR_REG_OFFSET));
}

static inline void ali_i2c_start_transaction(struct ali_i2c *i2c, u8 cp)
{
	u8 val = 0;

	if (NULL == i2c) {
		I2C_ERR("i2c is NULL\n");
		return;
	}

	//val = ali_reg_read(i2c, I2C_HCR_REG_OFFSET);
	val = I2C_HCR_HOST_CONCTOLLER_MASK | I2C_HCR_START_TRANSACTION_MASK | I2C_HCR_DEVICE_NOT_EXIST_MASK | cp;
	ali_reg_write(i2c, I2C_HCR_REG_OFFSET, val);
}

static int wait_trig_lvl_int_timeout(struct ali_i2c *i2c)
{
	u8 isr1_status = 0;
	int i = I2C_FIFO_DEPTH * 2;

	if (NULL == i2c) {
		I2C_ERR("i2c is NULL\n");
		return -1;
	}

	while (--i) {
		isr1_status = ali_reg_read(i2c, I2C_ISR1_REG_OFFSET);
		if (isr1_status & I2C_IER1_TRIGGER_MASK) {
			return 0;
		}
		udelay(i2c->one_byte_time_us);
	}
	
	I2C_ERR("fail, wait timeout, isr1: 0x%x, hsr: 0x%x, isr: 0x%x\n", isr1_status,
		ali_reg_read(i2c, I2C_HSR_REG_OFFSET), ali_reg_read(i2c, I2C_ISR_REG_OFFSET));
	return -1;
}

static int wait_trans_done_timeout(struct ali_i2c *i2c)
{
	u8 status = 0;
	int i = I2C_FIFO_DEPTH * 2;

	if (NULL == i2c) {
		I2C_ERR("i2c is NULL\n");
		return -1;
	}

	while (--i) {
		status = ali_reg_read(i2c, I2C_ISR_REG_OFFSET);
		if (status & I2C_IER_TDIE_MASK) {
			return 0;
		} else if ((status & 0x0e) != 0) {
			I2C_ERR("fail, isr: 0x%x\n", status);
			return -1;
		}
		udelay(i2c->one_byte_time_us);
	}

	I2C_ERR("fail, wait timeout, isr: 0x%x\n", status);
	return -1;
}

static int ali_i2c_start_recv(struct ali_i2c *i2c)
{
	struct i2c_msg *msg = NULL;
	u8 cp = 0;

	if (NULL == i2c) {
		I2C_ERR("i2c is NULL\n");
		return -1;
	}

	msg = i2c->msgs;
	cp = (u8)(i2c->is_std_read ? I2C_HCR_STR : I2C_HCR_SER);

	ali_reg_write(i2c, I2C_SAR_REG_OFFSET, (msg->addr << 1) | I2C_READ_OPERATION);
	I2C_DBG("msg->addr: 0x%x, SAR read from reg:0x%x\n", (int)msg->addr, ali_reg_read(i2c, I2C_SAR_REG_OFFSET));

	if (i2c->is_ddc_read) {
		cp |= I2C_HCR_EDDC_OPERATION_MASK;
		ali_reg_write(i2c, I2C_SSAR_REG_OFFSET, msg->buf[0]);
		ali_reg_write(i2c, I2C_DAR_REG_OFFSET, i2c->ddc_addr);
		ali_reg_write(i2c, I2C_SPR_REG_OFFSET, i2c->ddc_seg_ptr);
	} else if (i2c->is_standalone) {
		cp = I2C_HCR_CAR;
	}
	
	ali_i2c_set_target_byte_count(i2c, msg->len);

	if (I2C_FIFO_DEPTH < i2c->msgs->len) {
		ali_reg_write(i2c, I2C_FTLR_REG_OFFSET, I2C_FIFO_RX_TRIG_LVL);
		if (i2c->enable_int) {
			ali_i2c_enable_trig_lvl_int(i2c, 1);
		}
	}

	ali_i2c_start_transaction(i2c, cp);

	if (!i2c->enable_int) {
		while (I2C_FIFO_DEPTH < i2c->msgs->len - i2c->xfer_len) {
			if (wait_trig_lvl_int_timeout(i2c)) {
				I2C_ERR("wait trigger level interrupt timeout\n");
				return -1;
			}
			
			ali_i2c_read_fifo(i2c);
			ali_reg_write(i2c, I2C_ISR1_REG_OFFSET, I2C_IER1_TRIGGER_MASK);
		}

		
		if (wait_trans_done_timeout(i2c)) {
			I2C_ERR("wait transaction done timeout\n");
			return -1;
		}

		ali_i2c_read_fifo(i2c);
	}

	return 0;
}

static int ali_i2c_start_send(struct ali_i2c *i2c)
{
	struct i2c_msg *msg = NULL;
	u8 val = 0;
	int i = 0;

	if (NULL == i2c) {
		I2C_ERR("i2c is NULL\n");
		return -1;
	}

	msg = i2c->msgs;

	if ((1 < i2c->nmsgs) && (!i2c->is_standalone)) {
		// write-read command protocol: just set SSAR* here
		ali_reg_write(i2c, I2C_SSAR_REG_OFFSET, msg->buf[0]);       
		val = (msg->len - 1) & I2C_SSAR_EN_REG_MASK;
		ali_reg_write(i2c, I2C_SSAR_EN_REG_OFFSET, val);
		for (i=0; i<val; ++i) {
			ali_reg_write(i2c, I2C_SSAR2_REG_OFFSET + i, msg->buf[1 + i]);
		}
	} else {
		ali_reg_write(i2c, I2C_SAR_REG_OFFSET, (msg->addr << 1) | I2C_WRITE_OPERATION);
		I2C_DBG("msg->addr: 0x%x, SAR read from reg:0x%x\n", (int)msg->addr, ali_reg_read(i2c, I2C_SAR_REG_OFFSET));
		//ali_reg_write(i2c, I2C_SSAR_REG_OFFSET, msg->buf[0]);

		ali_i2c_set_target_byte_count(i2c, msg->len);

		// fill data to FIFO 1st
		ali_i2c_write_fifo(i2c);

		if (i2c->xfer_len < i2c->msgs->len) {
			ali_reg_write(i2c, I2C_FTLR_REG_OFFSET, I2C_FIFO_TX_TRIG_LVL);
			if (i2c->enable_int) {
				ali_i2c_enable_trig_lvl_int(i2c, 1);
			}
		}

		ali_i2c_start_transaction(i2c, I2C_HCR_WT);

		if (!i2c->enable_int) {
			while (i2c->xfer_len < i2c->msgs->len) {
				if (wait_trig_lvl_int_timeout(i2c)) {
					I2C_ERR("wait trigger level interrupt timeout(c)\n");
					return -1;
				}
				
				ali_i2c_write_fifo(i2c);
				ali_reg_write(i2c, I2C_ISR1_REG_OFFSET, I2C_IER1_TRIGGER_MASK);
			}

			if (wait_trans_done_timeout(i2c)) {
				I2C_ERR("wait transaction timeout\n");
				return -1;
			}
		}
	}

	return 0;
}

static int ali_i2c_start_xfer_one_msg(struct ali_i2c *i2c)
{
	if (NULL == i2c) {
		I2C_ERR("i2c is NULL\n");
		return -1;
	}

        if (i2c->msgs->flags & I2C_M_RD) {
                return ali_i2c_start_recv(i2c);
        } else {
                return ali_i2c_start_send(i2c);
        }
}

static void ali_i2c_reset(struct ali_i2c *i2c)
{
	u8 val = 0;

	if (NULL == i2c) {
		I2C_ERR("i2c is NULL\n");
		return;
	}

	/* Host Controller Enable */
	val = ali_reg_read(i2c, I2C_HCR_REG_OFFSET);
	val |= I2C_HCR_HOST_CONCTOLLER_MASK;
	ali_reg_write(i2c, I2C_HCR_REG_OFFSET, val);

	/*Using 12M clock for I2C interface */
	val = ali_reg_read(i2c, I2C_DCR_REG_OFFSET);
	val &= I2C_IER_ARBLIE_MASK;
	ali_reg_write(i2c, I2C_DCR_REG_OFFSET, val);

	/* Set I2C Clock and Timing Regrister */
	val = I2C_12M_CLOCK_CONSTANT / i2c->bus_clk_rate;
	ali_reg_write(i2c, I2C_HPCC_REG_OFFSET, val - I2C_SCL_RT_CONSTANT);
	ali_reg_write(i2c, I2C_LPCC_REG_OFFSET, val - I2C_SCL_RT_CONSTANT);
	//ali_reg_write(i2c, I2C_HPCC_REG_OFFSET, val);
	//ali_reg_write(i2c, I2C_LPCC_REG_OFFSET, val);
	ali_reg_write(i2c, I2C_PSUR_REG_OFFSET, val);
	ali_reg_write(i2c, I2C_PHDR_REG_OFFSET, val);
	ali_reg_write(i2c, I2C_RSUR_REG_OFFSET, val);
	ali_reg_write(i2c, I2C_SHDR_REG_OFFSET, val);

	/* Flushes FIFO and Resets The FIFO Point */
	val = ali_reg_read(i2c, I2C_FCR_REG_OFFSET);
	val |= I2C_FCR_FIFO_MASK;// | I2C_IER_SSIE_MASK;
	ali_reg_write(i2c, I2C_FCR_REG_OFFSET, val);

	/* Make Sure RX FIFO is Empty */
	ali_clear_rx_fifo(i2c);

	/* Disable interrupt */
	ali_all_irq_clr(i2c);
}

void __ali_i2c_init(struct ali_i2c *i2c)
{
	if (NULL == i2c) {
		I2C_ERR("i2c is NULL\n");
		return;
	}

	ali_i2c_reset(i2c);

	i2c->one_byte_time_us = 1000000 / (i2c->bus_clk_rate / 8);

	mutex_init(&i2c->mutex);
	init_completion(&i2c->xfer_result.completion);
}

void __ali_i2c_deinit(struct ali_i2c *i2c)
{
	u8 val = 0;

	if (NULL == i2c) {
		I2C_ERR("i2c is NULL\n");
		return;
	}

	/* Flushes FIFO and Resets The FIFO Point */
	val = ali_reg_read(i2c, I2C_FCR_REG_OFFSET);
	val |= I2C_FCR_FIFO_MASK;
	ali_reg_write(i2c, I2C_FCR_REG_OFFSET, val);

	/* Disable interrupt */
	ali_all_irq_clr(i2c);
}

int __ali_i2c_start_xfer(struct ali_i2c *i2c)
{
	int err = 0;
	unsigned long flags = 0;
	unsigned long timeout = 0;

	if (NULL == i2c) {
		I2C_ERR("i2c is NULL\n");
		return -1;
	}

	I2C_DBG("nmsgs: %d\n", i2c->nmsgs);
	
	if (1 == i2c->nmsgs) {
		i2c->is_standalone = 1;
	}

	while (i2c->nmsgs) {
		I2C_DBG("isr: 0x%x, hsr: 0x%x, msg len: %d\n", ali_reg_read(i2c, I2C_ISR_REG_OFFSET),
			ali_reg_read(i2c, I2C_HSR_REG_OFFSET), i2c->msgs->len);

		ali_reg_write(i2c, I2C_HSR_REG_OFFSET, 0x00);
		ali_reg_write(i2c, I2C_ISR_REG_OFFSET, 0xff);
		ali_reg_write(i2c, I2C_ISR1_REG_OFFSET, I2C_IER1_TRIGGER_MASK);

		if (i2c->enable_int) {
			INIT_COMPLETION(i2c->xfer_result.completion);
			ali_irq_clr_en(i2c, I2C_IER_TDIE_MASK | I2C_IER_DNEE_MASK | I2C_IER_DBIE_MASK | I2C_IER_ARBLIE_MASK);
			//ali_irq_clr_en(i2c, I2C_IER_TDIE_MASK | I2C_IER_DNEE_MASK | I2C_IER_DBIE_MASK | I2C_IER_ARBLIE_MASK | I2C_IER_FEIE_MASK | I2C_IER_FFIE_MASK);
		}

		if (!i2c->enable_int) {
			local_irq_save(flags);
		}

		i2c->xfer_len = 0;
		err = ali_i2c_start_xfer_one_msg(i2c);

		if (!i2c->enable_int) {
			local_irq_restore(flags);
		}

		if (err) {
			I2C_ERR("xfer one msg fail, msg_len: %d, xfer_len: %d\n", i2c->msgs->len, i2c->xfer_len);
			break;
		}

		if (i2c->enable_int && ((1 == i2c->nmsgs) || (i2c->is_standalone))) {
			timeout = ((i2c->one_byte_time_us * i2c->msgs->len * HZ) / 1000000 + 1) * 10;
			if (0 == wait_for_completion_timeout(&i2c->xfer_result.completion, timeout)) {
				I2C_ERR("wait xfer complete timeout, nmsgs: %d, msg_len: %d, timeout: %lu\n", 
					i2c->nmsgs, i2c->msgs->len, timeout);
				err = -1;
				break;
			}

			err = i2c->xfer_result.err;
			if (err) {
				I2C_ERR("[interrupt] xfer one msg fail, msg_len: %d, xfer_len: %d\n", i2c->msgs->len, i2c->xfer_len);
				break;
			}
		}

		i2c->msgs++;
		i2c->nmsgs--;
	}

	i2c->is_standalone = 0;
	i2c->is_ddc_read = 0;
	//i2c->is_std_read = 0;

	return err;
}

int __ali_i2c_read_req_register_cb(struct ali_i2c *i2c, void (*read_req_cb)(void *read_req_cb_data), void *read_req_cb_data)
{
	//unsigned long flags;
	u8 val = 0;

	if (NULL == i2c) {
		I2C_ERR("i2c is NULL\n");
		return -1;
	}

	if (NULL == read_req_cb) {
		I2C_ERR("read_req_cb is NULL\n");
		return -1;
	}
	
	if (!i2c->enable_int) {
		I2C_ERR("Not in the interrupt mode.\n");
		return -1;
	}

	if (NULL == read_req_cb_data) {
		I2C_DBG("read_req_cb_data is NULL\n");
	}

	//spin_lock_irqsave(&i2c->lock, flags);
	mutex_lock(&i2c->mutex);
	// enable read request inetrrupt
	val = ali_reg_read(i2c, I2C_IER1_REG_OFFSET);
	val |= I2C_IER1_READ_REQ_MASK;
	ali_reg_write(i2c, I2C_IER1_REG_OFFSET, val);
	// enable read request detect
	val = ali_reg_read(i2c, I2C_DCR_REG_OFFSET);
	val |= I2C_DCR_READ_REQ_DETECT_MASK;
	ali_reg_write(i2c, I2C_DCR_REG_OFFSET, val);
	
	i2c->read_req_cb = read_req_cb;
	i2c->read_req_cb_data = read_req_cb_data;
	//spin_unlock_irqrestore(&i2c->lock, flags);
	mutex_unlock(&i2c->mutex);
	return 0;
}

int __ali_i2c_read_req_unregister_cb(struct ali_i2c *i2c)
{	
	//unsigned long flags;
	u8 val = 0;

	if (NULL == i2c) {
		I2C_ERR("i2c is NULL\n");
		return -1;
	}

	if (!i2c->enable_int) {
		I2C_ERR("No in the interrupt mode.\n");
		return -1;
	}
	
	//spin_lock_irqsave(&i2c->lock, flags);
	mutex_lock(&i2c->mutex);
	// disable read request inetrrupt
	val = ali_reg_read(i2c, I2C_IER1_REG_OFFSET);
	val &= ~I2C_IER1_READ_REQ_MASK;
	ali_reg_write(i2c, I2C_IER1_REG_OFFSET, val);
	// disable read request detect
	val = ali_reg_read(i2c, I2C_DCR_REG_OFFSET);
	val &= ~I2C_DCR_READ_REQ_DETECT_MASK;
	ali_reg_write(i2c, I2C_DCR_REG_OFFSET, val);

	i2c->read_req_cb = NULL;
	i2c->read_req_cb_data = NULL;
	//spin_unlock_irqrestore(&i2c->lock, flags);
	mutex_unlock(&i2c->mutex);
	return 0;
}

void __ali_i2c_isr(struct ali_i2c *i2c)
{
	u32 pend = 0, isr = 0, ier = 0;
	u32 pend1 = 0, isr1 = 0, ier1 = 0;
	u32 clr = 0, clr1 = 0;
	u32 val = 0;

	if (NULL == i2c) {
		I2C_ERR("i2c is NULL\n");
		return;
	}

	/* Get the interrupt Status from the ISR. There is no clearing of
	 * interrupts in the IPIF. Interrupts must be cleared at the source.
	 * To find which interrupts are pending; AND interrupts pending with
	 * interrupts masked.
	 */
	isr = ali_reg_read(i2c, I2C_ISR_REG_OFFSET);
	ier = ali_reg_read(i2c, I2C_IER_REG_OFFSET);
	pend = isr & ier;

	isr1 = ali_reg_read(i2c, I2C_ISR1_REG_OFFSET);
	ier1 = ali_reg_read(i2c, I2C_IER1_REG_OFFSET);
	pend1 = isr1 & ier1;

	/* Do not processes a devices interrupts if the device has no
	 * interrupts pending
	 */
	if ((!pend) && (!pend1))
		return;

	/* Service requesting interrupt */
	if ((pend & I2C_IER_ARBLIE_MASK) || (pend & I2C_IER_DNEE_MASK) || (pend & I2C_IER_DBIE_MASK)) {
		/* bus arbritration lost, or...
		 * Transmit error,TX error
		 */
		I2C_ERR("i2c IRQ error, pend: 0x%08x\n", pend);
		ali_i2c_reset(i2c);
		ali_i2c_wakeup(i2c, -1);
	} else if (pend1 & I2C_IER1_TRIGGER_MASK) {

		clr1 = I2C_IER1_TRIGGER_MASK;

		if (i2c->msgs->flags & I2C_M_RD) {
			ali_i2c_read_fifo(i2c);
		} else {
			ali_i2c_write_fifo(i2c);
		}
	} else if (pend1 & I2C_IER1_READ_REQ_MASK) {

		clr1 = I2C_IER1_READ_REQ_MASK;

		if (NULL != i2c->read_req_cb) {
			I2C_DBG("will invoke read request callback\n");
			i2c->read_req_cb(i2c->read_req_cb_data);
			I2C_DBG("did invoke read request callback\n");
		}
	} else if (pend & I2C_IER_TDIE_MASK) {
		/* IIC bus has transitioned to not busy */
		clr = I2C_IER_TDIE_MASK;
		/*Completing the current transaction,Disable IRQ */
		ali_irq_dis(i2c, I2C_IER_TDIE_MASK);
		// disable trigger level interrupt
		ali_i2c_enable_trig_lvl_int(i2c, 0);

		// read the tail from FIFO
		if (i2c->msgs->flags & I2C_M_RD) {
			ali_i2c_read_fifo(i2c);
		}

		// disable trigger level interrupt
		val = ali_reg_read(i2c, I2C_IER1_REG_OFFSET);
		ali_reg_write(i2c, I2C_IER1_REG_OFFSET, val & ~I2C_IER1_TRIGGER_MASK);

		if (i2c->xfer_len == i2c->msgs->len) {
			ali_i2c_wakeup(i2c, 0);
		} else {
			I2C_ERR("i2c->xfer_len != i2c->msgs->len, xfer_len: %d, msg_len: %d\n", i2c->xfer_len, i2c->msgs->len);
			ali_i2c_wakeup(i2c, -1);
		}
	} else {
		/* got IRQ which is not acked */
		I2C_ERR("got unexpected IRQ, pend:%08x, pend1: 0X%08x\n", pend, pend1);
		clr = pend;
		clr1 = pend1;
	}

	ali_reg_write(i2c, I2C_ISR_REG_OFFSET, clr);
	ali_reg_write(i2c, I2C_ISR1_REG_OFFSET, clr1);
}

void __ali_i2c_lock(struct ali_i2c *i2c)
{
	if (NULL == i2c) {
		I2C_ERR("i2c is NULL\n");
		return;
	}

	mutex_lock(&i2c->mutex);
}

void __ali_i2c_unlock(struct ali_i2c *i2c)
{
	if (NULL == i2c) {
		I2C_ERR("i2c is NULL\n");
		return;
	}

	mutex_unlock(&i2c->mutex);
}
