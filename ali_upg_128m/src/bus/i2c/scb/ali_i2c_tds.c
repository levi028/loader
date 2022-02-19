#include "ali_i2c_common.h"
#include <bus/i2c/i2c.h>

#include <sys_config.h>
#include <types.h>
#include <retcode.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <asm/chip.h>

#define I2C_TOTAL_TYPE_NUM              3
#define I2C_SCB_NUM                	4

UINT32 g_scb_sys_chip_id = 0;
UINT32 g_scb_sys_chip_ver = 0;

#if defined(ALI_SEE)
//struct i2c_device i2c_dev[I2C_TOTAL_TYPE_NUM] = {{0}};
//struct i2c_device_ext i2c_dev_ext[I2C_TOTAL_TYPE_NUM] = {{0}};
struct i2c_device i2c_dev[I2C_TOTAL_TYPE_NUM];
struct i2c_device_ext i2c_dev_ext[I2C_TOTAL_TYPE_NUM];
#endif

static struct ali_i2c i2c_scb[I2C_SCB_NUM] = {
		{
			.base = 0xB8018200,
			.irq = 26,
		},
		{
			.base = 0xB8018700,
			.irq = 33,
		},
		{
			.base = 0xB8018B00,
			.irq = 34,
		},
		{
			.base = 0xB8018D00,
			.irq = 51,
		},		
};

#if 0// secure coding
static inline struct ali_i2c *get_ali_i2c(UINT32 id)
{
	int num = sizeof (i2c_scb) / sizeof (i2c_scb[0]);

	if (id >= num) {
		I2C_ERR("id out of range\n");
		return NULL;
	}

	return &i2c_scb[id];
}
#else
#define get_ali_i2c(id) ({						\
			UINT32 num = sizeof (i2c_scb) / sizeof (i2c_scb[0]); \
			if (id >= num) {				\
				I2C_ERR("id out of range\n");		\
				return ERR_PARA;			\
			}						\
			&i2c_scb[id];					\
		})
#endif

static void ali_i2c_isr(UINT32 param)
{
	__ali_i2c_isr((struct ali_i2c *)param);
}

INT32 i2c_scb_read(UINT32 id, UINT8 slv_addr, UINT8 *data, int len)
{
	INT32 ret = 0;
	struct i2c_msg msgs[1];
	struct ali_i2c *i2c = get_ali_i2c(id);

	if ((len < 0) || (NULL == data)) {
		I2C_DBG("error arguments\n");
		return ERR_PARA;
	}
	
	memset(msgs, 0x0, sizeof (msgs));
	
	msgs[0].addr = slv_addr >> 1;
	msgs[0].buf = data;
	msgs[0].len = len;
	msgs[0].flags = I2C_M_RD;

	__ali_i2c_lock(i2c);	
	i2c->msgs = msgs;
	i2c->nmsgs = 1;
	ret = __ali_i2c_start_xfer(i2c);
	__ali_i2c_unlock(i2c);
	return 0 == ret ? SUCCESS : !SUCCESS;
}

INT32 i2c_scb_write(UINT32 id, UINT8 slv_addr, UINT8 *data, int len)
{
	INT32 ret = 0;
	struct i2c_msg msgs[1];
	struct ali_i2c *i2c = get_ali_i2c(id);

	if ((len < 0) || (NULL == data)) {
		I2C_DBG("error arguments\n");
		return ERR_PARA;
	}
	
	memset(msgs, 0x0, sizeof (msgs));
	
	msgs[0].addr = slv_addr >> 1;
	msgs[0].buf = data;
	msgs[0].len = len;
	
	__ali_i2c_lock(i2c);
	i2c->msgs = msgs;
	i2c->nmsgs = 1;
	ret = __ali_i2c_start_xfer(i2c);
	__ali_i2c_unlock(i2c);
	return 0 == ret ? SUCCESS : !SUCCESS;
}

INT32 i2c_scb_write_plus_read(UINT32 id, UINT8 slv_addr, UINT8 *data, int wlen, int rlen)
{
	INT32 ret = 0;
	struct i2c_msg msgs[2];
	struct ali_i2c *i2c = get_ali_i2c(id);

	if (NULL == data) {
		I2C_DBG("error arguments\n");
		return ERR_PARA;
	}
	
	memset(msgs, 0x0, sizeof (msgs));
	
	msgs[0].addr = slv_addr >> 1;
	msgs[0].buf = data;
	msgs[0].len = wlen;

	msgs[1].addr = slv_addr >> 1;
	msgs[1].buf = data;
	msgs[1].len = rlen;
	msgs[1].flags = I2C_M_RD;
	
	__ali_i2c_lock(i2c);
	i2c->msgs = msgs;
	i2c->nmsgs = 2;
	i2c->is_standalone = 1;
	ret = __ali_i2c_start_xfer(i2c);
	__ali_i2c_unlock(i2c);
	return 0 == ret ? SUCCESS : !SUCCESS;
}

static INT32 __i2c_scb_write_read(UINT32 id, UINT8 slv_addr, UINT8 *data, int wlen, int rlen, int is_std_read)
{
	INT32 ret = 0;
	struct i2c_msg msgs[2];
	struct ali_i2c *i2c = get_ali_i2c(id);

	if ((rlen < 0) || (NULL == data)) {
		I2C_DBG("error arguments\n");
		return ERR_PARA;
	}

	if (0 == wlen) {
		return i2c_scb_read(id, slv_addr, data, rlen);
	}
	
	memset(msgs, 0x0, sizeof (msgs));
	
	msgs[0].addr = slv_addr >> 1;
	msgs[0].buf = data;
	msgs[0].len = wlen;

	msgs[1].addr = slv_addr >> 1;
	msgs[1].buf = data;
	msgs[1].len = rlen;
	msgs[1].flags = I2C_M_RD;
	
	__ali_i2c_lock(i2c);
	i2c->msgs = msgs;
	i2c->nmsgs = 2;
	i2c->is_std_read = is_std_read;
	ret = __ali_i2c_start_xfer(i2c);
	__ali_i2c_unlock(i2c);
	return 0 == ret ? SUCCESS : !SUCCESS;
}

INT32 i2c_scb_write_read(UINT32 id, UINT8 slv_addr, UINT8 *data, int wlen, int rlen)
{
	if (NULL == data) {
		I2C_ERR("data ptr is NULL\n");
		return ERR_PARA;
	}

	return __i2c_scb_write_read(id, slv_addr, data, wlen, rlen, 0);
}

INT32 i2c_scb_write_read_std(UINT32 id, UINT8 slv_addr, UINT8 *data, int wlen, int rlen)
{
	if (NULL == data) {
		I2C_ERR("data ptr is NULL\n");
		return ERR_PARA;
	}

	return __i2c_scb_write_read(id, slv_addr, data, wlen, rlen, 1);
}

static INT32 i2c_scb_eddc_read_without_lock(UINT32 id, UINT8 segment_ptr, UINT8 slv1_addr, UINT8 slv2_addr, UINT8 *data, int rlen)
{
	INT32 ret = 0;
	struct i2c_msg msgs[1];
	struct ali_i2c *i2c = get_ali_i2c(id);

	if (NULL == data) {
		I2C_ERR("data ptr is NULL\n");
		return ERR_PARA;
	}
	
	memset(msgs, 0x0, sizeof (msgs));
	
	msgs[0].addr = slv2_addr >> 1;
	msgs[0].buf = data;
	msgs[0].len = rlen;
	msgs[0].flags = I2C_M_RD;
	
	i2c->msgs = msgs;
	i2c->nmsgs = 1;

	i2c->ddc_addr = slv1_addr;
	i2c->ddc_seg_ptr = segment_ptr;	
	i2c->is_ddc_read = 1;

	ret = __ali_i2c_start_xfer(i2c);
	return 0 == ret ? SUCCESS : !SUCCESS;
}

INT32 i2c_scb_eddc_read(UINT32 id, UINT8 segment_ptr, UINT8 slv1_addr, UINT8 slv2_addr, UINT8 *data, int rlen)
{
	INT32 ret = 0;
	struct ali_i2c *i2c = get_ali_i2c(id);

	if (NULL == data) {
		I2C_ERR("data ptr is NULL\n");
		return ERR_PARA;
	}

	__ali_i2c_lock(i2c);
	ret = i2c_scb_eddc_read_without_lock(id, segment_ptr, slv1_addr, slv2_addr, data, rlen);
	__ali_i2c_unlock(i2c);
	return ret;
}

static INT32 i2c_scb_write_write_read_without_lock(UINT32 id, UINT8 segment_ptr, UINT8 slv1_addr, UINT8 slv2_addr, UINT8 *data, int rlen)
{
	UINT32 count = 0;
	UINT8 segment_point = 0;
	UINT32 word_offset = 0;
	UINT32 r_lens = 0;
	UINT32 compare_len = 0;
	UINT8 *data_buffer = NULL;
	UINT32 temp = 0;
	UINT32 num_256 = 256;

	if ((rlen < 0) || (NULL == data)) {
		return ERR_PARA;
	}

	segment_point = segment_ptr;
	word_offset = data[0];
	compare_len = 128;
	data_buffer = data;
	count = rlen;

	while (count > 0) {
		r_lens = (count > compare_len) ? compare_len : count;

		if (word_offset == num_256) {
			segment_point++;
			word_offset = 0;
		} else if ((word_offset + r_lens) > num_256) {
			r_lens = 256 - word_offset;
		}

		*data_buffer = word_offset;
		temp = i2c_scb_eddc_read_without_lock(id, segment_point, slv1_addr, slv2_addr, data_buffer, r_lens);
		if (temp != SUCCESS) {
			I2C_ERR("read error!\n");
			return !SUCCESS;
		}

		word_offset += r_lens;
		data_buffer += r_lens;
		count -= r_lens;
	}

	return SUCCESS;
}

INT32 i2c_scb_write_write_read(UINT32 id, UINT8 segment_ptr, UINT8 slv1_addr, UINT8 slv2_addr, UINT8 *data, int rlen)
{
	INT32 ret = 0;
	struct ali_i2c *i2c = get_ali_i2c(id);

	if (NULL == data) {
		I2C_ERR("data ptr is NULL\n");
		return ERR_PARA;
	}

	__ali_i2c_lock(i2c);
	ret = i2c_scb_write_write_read_without_lock(id, segment_ptr, slv1_addr, slv2_addr, data, rlen);
	__ali_i2c_unlock(i2c);
	return ret;
}

INT32 i2c_scb_mode_set(UINT32 id, int bps, int en)
{
	struct ali_i2c *i2c = get_ali_i2c(id);

	if (bps <= 0) {
		return ERR_FAILUE;
	}

	if (400000 < bps) {
		I2C_INFO("bps(%d) out of range\n", bps);
		bps = 400000;
	}

	i2c->bus_clk_rate = bps;
	__ali_i2c_init(i2c);

	i2c->enable_int = 0;
#if 1
	if (sys_ic_get_chip_id() >= ALI_S3503) {
		i2c->enable_int = 1;
		if (OSAL_E_OK != osal_interrupt_register_lsr(i2c->irq, ali_i2c_isr, (UINT32)i2c)) {
			I2C_ERR("register isr fail\n");
			return !SUCCESS;
		}
	}
#endif

	return SUCCESS;
}

// secure coding: add static
static INT32 i2c_scb_reset(UINT32 id)
{
	struct ali_i2c *i2c = get_ali_i2c(id);

	__ali_i2c_deinit(i2c);
	return SUCCESS;
}

__ATTRIBUTE_REUSE_ INT32 i2c_scb_attach(int dev_num)
{
	g_scb_sys_chip_id = sys_ic_get_chip_id();
	g_scb_sys_chip_ver = sys_ic_get_rev_id();

	if (ALI_S3811 == g_scb_sys_chip_id) {
		dev_num = 2;
	} else if (ALI_S3281 == g_scb_sys_chip_id) {
		dev_num = 2;
		i2c_scb[0].base = 0xB8018200;
		i2c_scb[1].base = 0xB8018700;
		i2c_scb[0].irq = 26;
		i2c_scb[1].irq = 33;
	} else if (ALI_C3701 == g_scb_sys_chip_id) {
		dev_num = 2;
		i2c_scb[0].base = 0xB8018200;
		i2c_scb[1].base = 0xB8018700;
		i2c_scb[0].irq = 26;
		i2c_scb[1].irq = 33;
	} else if (ALI_S3503 == g_scb_sys_chip_id) {
		dev_num = 3;
		i2c_scb[0].base = 0xB8018200;
		i2c_scb[1].base = 0xB8018700;
		i2c_scb[2].base = 0xB8018b00;
		i2c_scb[0].irq = 26;
		i2c_scb[1].irq = 33;
		i2c_scb[2].irq = 34;
	} else if (ALI_S3821 == g_scb_sys_chip_id) {
		dev_num = 4;
		i2c_scb[0].base = 0xB8018200;
		i2c_scb[1].base = 0xB8018700;
		i2c_scb[2].base = 0xB8018b00;
		i2c_scb[3].base = 0xB8018d00;
		i2c_scb[0].irq = 26;
		i2c_scb[1].irq = 33;
		i2c_scb[2].irq = 34;
		i2c_scb[3].irq = 51;
	} else if (ALI_C3505 == g_scb_sys_chip_id) {
		dev_num = 4;
		i2c_scb[0].base = 0xB8018200;
		i2c_scb[1].base = 0xB8018700;
		i2c_scb[2].base = 0xB8018b00;
		i2c_scb[3].base = 0xB8018d00;
		i2c_scb[0].irq = 26;
		i2c_scb[1].irq = 33;
		i2c_scb[2].irq = 34;
		i2c_scb[3].irq = 51;
	} 
#if defined(ALI_SEE)
	else if (ALI_C3702 == g_scb_sys_chip_id) {
		dev_num = 4;
		i2c_scb[0].base = 0xb8018200;
		i2c_scb[1].base = 0xb8018700;
		i2c_scb[2].base = 0xb8018b00;
		i2c_scb[3].base = 0xb8018d00;
		i2c_scb[0].irq = 8 + 20;
		i2c_scb[1].irq = 8 + 21;
		i2c_scb[2].irq = 8 + 22;
		i2c_scb[3].irq = 8 + 23;
	} else if (ALI_C3922 == g_scb_sys_chip_id) {
		dev_num = 3;
		i2c_scb[0].base = 0xB8018200;
		i2c_scb[1].base = 0xB8018700;
		i2c_scb[2].base = 0xB8018b00;
		i2c_scb[0].irq = 8 + 20;
		i2c_scb[1].irq = 8 + 21;
		i2c_scb[2].irq = 8 + 22;
	} 
#endif
	else if ((ALI_M3329E == g_scb_sys_chip_id)) {
		if (g_scb_sys_chip_ver >= IC_REV_5) {
			dev_num = 1;
		}
	}
#if defined(ALI_SEE)
	if ((1 == sys_ic_is_m3202()) && (I2C_SCB_NUM > 1)) {
		i2c_scb[1].irq = 25;
	}
#endif
	if (I2C_SCB_NUM < dev_num) {
		I2C_ERR("I2C_SCB_NUM < dev_num\n");
		return ERR_FAILURE;
	}

	i2c_dev[I2C_TYPE_SCB >> 16].mode_set = i2c_scb_mode_set;
	i2c_dev[I2C_TYPE_SCB >> 16].read = i2c_scb_read;
	i2c_dev[I2C_TYPE_SCB >> 16].write = i2c_scb_write;
	i2c_dev[I2C_TYPE_SCB >> 16].write_read = i2c_scb_write_read;
	i2c_dev[I2C_TYPE_SCB >> 16].write_read_std = i2c_scb_write_read_std;
	i2c_dev[I2C_TYPE_SCB >> 16].write_write_read = i2c_scb_write_write_read;
	i2c_dev_ext[I2C_TYPE_SCB >> 16].write_plus_read = i2c_scb_write_plus_read;

	return SUCCESS;
}

int i2c_scb_read_req_register_cb(UINT32 id, void (*read_req_cb)(void *read_req_cb_data), void *read_req_cb_data)
{
	struct ali_i2c *i2c = get_ali_i2c(id);

	if (NULL == read_req_cb) {
		I2C_ERR("read_req_cb is NULL\n");
		return -1;
	}

	if (NULL == read_req_cb_data) {
		I2C_DBG("read_req_cb_data is NULL\n");
	}

	return __ali_i2c_read_req_register_cb(i2c, read_req_cb, read_req_cb_data);
}

// secure coding: add static
static int i2c_scb_read_req_unregister_cb(UINT32 id)
{
	struct ali_i2c *i2c = get_ali_i2c(id);
	
	return __ali_i2c_read_req_unregister_cb(i2c);
}

