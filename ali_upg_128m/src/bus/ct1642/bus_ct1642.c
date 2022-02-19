/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2005 Copyright (C)
*
*    File:    tsi_s3602.c
*
*    Description:    This file contains all globe micros and functions declare
*                     of S3602 TSI.
*    History:
*           Date            Athor        Version          Reason
*        ============    =============    =========    =================
*    1. Aug.19.2005       Goliath Peng      Ver 0.1    Create file.
*    2. 2006.12.1                    Bin   Wang                         Add  Card B
*    3. 2009.2.18            Eric Li               Ver 0.3      Make API clearly and friendly.
*****************************************************************************/

#include <sys_config.h>
#include <basic_types.h>
#include <retcode.h>
#include <hal/hal_common.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <hal/hal_gpio.h>
#include <hld/snd/snd.h>

static UINT32 cur_value = 0;
static int ct1642_clock_pin = 0;
static int ct1642_data_pin = 0;

#define GPIO_CLK_PIN       ct1642_clock_pin
#define GPIO_DATA_PIN      ct1642_data_pin
#define CT1642_PIN_NUM	   18

//#define Debug_CT1642
#ifdef Debug_CT1642
#define GPIO_CT1642_PRINTF libc_printf
#else
#define GPIO_CT1642_PRINTF(...) do{}while(0)
#endif

#define MAX_REQUEST_QUEUE_SIZE 10

typedef void (*run_loop_request_entry)(void *param);

typedef struct run_loop_request_queue_node {
	struct run_loop_request_queue_node *next;
	
	run_loop_request_entry request_entry;
	void *param;
} run_loop_request_queue_node_t;

typedef struct run_loop {
	ID sem;
	ID thread;
	char name[4];

	run_loop_request_entry request_entry;
	void *param;

	run_loop_request_queue_node_t *free_queue;
	run_loop_request_queue_node_t *busy_queue;

	int is_waiting;
} run_loop_t;

static void run_loop_thread_entry(DWORD param1, DWORD param2)
{
	run_loop_t *run_loop = (run_loop_t *)param1;
	run_loop_request_queue_node_t *busy_queue_head;
	run_loop_request_queue_node_t busy_queue_head_dup;
	
	while (1) {
		osal_interrupt_disable();

		if (NULL == run_loop->busy_queue) {
			run_loop->is_waiting = 1;
			osal_interrupt_enable();
			osal_semaphore_capture(run_loop->sem, OSAL_WAIT_FOREVER_TIME);
			osal_interrupt_disable();
		}
		run_loop->is_waiting = 0;
		
		busy_queue_head = run_loop->busy_queue;
		run_loop->busy_queue = run_loop->busy_queue->next;
		
		memcpy(&busy_queue_head_dup, busy_queue_head, sizeof (busy_queue_head_dup));

		// return current node to free queue
		busy_queue_head->next = run_loop->free_queue;
		run_loop->free_queue = busy_queue_head;
		
		osal_interrupt_enable();

		busy_queue_head_dup.request_entry(busy_queue_head_dup.param);
	}
}

static int run_loop_thread_create(run_loop_t *run_loop)
{
	OSAL_T_CTSK t_ctsk;

	memset(&t_ctsk, 0x0, sizeof (t_ctsk));
	t_ctsk.stksz = 0x4000;
	t_ctsk.quantum = 5;
	t_ctsk.itskpri = OSAL_PRI_NORMAL;
	memcpy(t_ctsk.name, run_loop->name, 3);
	t_ctsk.para1 = (UINT32)run_loop;
	t_ctsk.task = run_loop_thread_entry;
	
	run_loop->thread = osal_task_create(&t_ctsk);
	if(OSAL_INVALID_ID == run_loop->thread) {
		return -1;
	}

	return 0;
}

run_loop_t *run_loop_create(const char *name, size_t request_queue_size)
{
	run_loop_t *run_loop;
	size_t i;

	if (MAX_REQUEST_QUEUE_SIZE < request_queue_size) {
		libc_printf("request_queue_size too large\n");
		return NULL;
	}
	
	run_loop = (run_loop_t *)malloc(sizeof (*run_loop));
	if (NULL == run_loop) {
		goto fail_to_alloc_ctx;
	}
	memset(run_loop, 0x0, sizeof (*run_loop));

	run_loop->free_queue = (run_loop_request_queue_node_t *)malloc(sizeof (*(run_loop->free_queue)) * request_queue_size);
	if (NULL == run_loop->free_queue) {
		goto fail_to_alloc_request_queue;
	}
	for (i=0; i<request_queue_size-1; ++i) {
		run_loop->free_queue[i].next = &run_loop->free_queue[i+1];
	}
	run_loop->free_queue[i].next = NULL;
	run_loop->busy_queue = NULL;

	run_loop->sem = osal_semaphore_create(1);
	if (OSAL_INVALID_ID == run_loop->sem) {
		goto fail_to_create_sem;
	}
	osal_semaphore_capture(run_loop->sem, OSAL_WAIT_FOREVER_TIME);
	
	memcpy(run_loop->name, name, 3);
	if (run_loop_thread_create(run_loop)) {
		goto fail_to_create_thread;
	}

	return run_loop;

fail_to_create_thread:
	libc_printf("Fail to create run loop thread\n");
	osal_semaphore_delete(run_loop->sem);
fail_to_create_sem:
	libc_printf("Fail to create run loop sem\n");
	free(run_loop->free_queue);
fail_to_alloc_request_queue:
	libc_printf("Fail to alloc request queue\n");
	free(run_loop);
fail_to_alloc_ctx:
	libc_printf("Fail to alloc run loop ctx\n");
	return NULL;
}

static void run_loop_request_notify_hsr(DWORD param)
{
	osal_semaphore_release((ID)param);
}

int run_loop_request_post(run_loop_t *run_loop, run_loop_request_entry request_entry, void *param)
{
	if (NULL == run_loop) {
		return -1;
	}

	osal_interrupt_disable();
	
	if (NULL == run_loop->free_queue) {
		osal_interrupt_enable();
		libc_printf("##### Oops! [%s] run loop %s overflow\n", __FUNCTION__, run_loop->name);
		return -1;
	} else {
		run_loop_request_queue_node_t *free_queue_head;
		run_loop_request_queue_node_t *busy_queue_tail;

		free_queue_head = run_loop->free_queue;
		run_loop->free_queue = run_loop->free_queue->next;

		free_queue_head->next = NULL;
		free_queue_head->request_entry = request_entry;
		free_queue_head->param = param;
		
		if (NULL == run_loop->busy_queue) {
			run_loop->busy_queue = free_queue_head;
			if (run_loop->is_waiting) {
				if (OSAL_E_OK != osal_interrupt_register_hsr(run_loop_request_notify_hsr, (DWORD)run_loop->sem)) {                                                                                                                                         
					libc_printf("##### Oops! [%s]: register hsr error.\n", __FUNCTION__);
				}
			}
		} else {
			for (busy_queue_tail = run_loop->busy_queue; \
				NULL != busy_queue_tail->next; \
				busy_queue_tail = busy_queue_tail->next) {}
			busy_queue_tail->next = free_queue_head;
		}
	}
	
	osal_interrupt_enable();

	return 0;
}

void ct1642_set_clock_dat_pin(int clock_pin,int data_pin)
{
	ct1642_clock_pin = clock_pin;
	ct1642_data_pin = data_pin;
}

static inline int ct1642_write(int gpio_out_num, UINT8 value)
{
	int i = 0;
	int out_value = 0;
	int delay_time_us = 100;
	
	out_value = cur_value;
    out_value &= ~(1 << gpio_out_num);
	out_value |= value << gpio_out_num;
   
	//send_data
	for(i = 0; i < CT1642_PIN_NUM; i++) {	
		osal_delay(delay_time_us);
		HAL_GPIO_BIT_SET(GPIO_CLK_PIN, 0);
		value = (out_value >> i) & 0x01;
		HAL_GPIO_BIT_SET(GPIO_DATA_PIN, value);
		osal_delay(delay_time_us);
		HAL_GPIO_BIT_SET(GPIO_CLK_PIN, 1);
	}
	
	//save_data
    osal_delay(delay_time_us);
	HAL_GPIO_BIT_SET(GPIO_CLK_PIN, 1);
	HAL_GPIO_BIT_SET(GPIO_DATA_PIN, 0);
    osal_delay(delay_time_us);
	HAL_GPIO_BIT_SET(GPIO_DATA_PIN, 1);
    osal_delay(delay_time_us);
	
	//put_data
	HAL_GPIO_BIT_SET(GPIO_CLK_PIN, 0);
	HAL_GPIO_BIT_SET(GPIO_DATA_PIN, 0);
    osal_delay(delay_time_us);
	HAL_GPIO_BIT_SET(GPIO_DATA_PIN, 1);

	return out_value;
}

static void ct1642_write_request_entry(void *param)
{
	int gpio_out_num = (int)param & 0x7fffffff;
	UINT8 value = (((int)param) >> 31) & 0x1;
	cur_value = ct1642_write(gpio_out_num, value);
}

static run_loop_t *s_run_loop = NULL;

void ct1642_multi_gpio_set(INT8 multi_position, UINT8 value)
{
	int gpio_out_num = multi_position;

	if ((gpio_out_num >=0)||(gpio_out_num <= HAL_GPIO_NUM_INVALID)) {
		GPIO_CT1642_PRINTF("call ct1642_multi_gpio_set should set gpio_out_num < 0&&gpio_out_num > -100,please recheck your code!\n");
		return;
	}

	if(value > 0)
		value = 1;
	else
		value = 0;
	
	gpio_out_num = -gpio_out_num;
	if(gpio_out_num > 4)
		gpio_out_num += 6;
	gpio_out_num -= 1;

	if (NULL == s_run_loop) {
		s_run_loop = run_loop_create("MUT", MAX_REQUEST_QUEUE_SIZE);
		if (NULL == s_run_loop) {
			GPIO_CT1642_PRINTF("##### Oops! %s: ct1642 create run loop fail.\n", __FUNCTION__);
			return;
		}
	}

	gpio_out_num = (value << 31) | gpio_out_num;
	if (run_loop_request_post(s_run_loop, ct1642_write_request_entry, (void *)gpio_out_num)) {
		GPIO_CT1642_PRINTF("##### Oops! %s: ct1642 post request to run loop fail.\n", __FUNCTION__);
		return;
	}
}

/**************************************************************/
