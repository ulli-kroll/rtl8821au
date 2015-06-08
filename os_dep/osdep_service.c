/******************************************************************************
 *
 * Copyright(c) 2007 - 2012 Realtek Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/


#define _OSDEP_SERVICE_C_

#include <drv_types.h>
#include <rtw_debug.h>

#define RT_TAG	('1178')

/*
* Translate the OS dependent @param error_code to OS independent RTW_STATUS_CODE
* @return: one of RTW_STATUS_CODE
*/
inline int RTW_STATUS_CODE(int error_code)
{
	if (error_code >= 0)
		return _SUCCESS;

	switch (error_code) {
	/*
	 * case -ETIMEDOUT:
	 * 	return RTW_STATUS_TIMEDOUT;
	 */
	default:
		return _FAIL;
	}
}

u32 rtw_atoi(u8 *s)
{
	int num = 0, flag = 0;
	int i;

	for (i = 0; i <= strlen(s); i++) {
		if (s[i] >= '0' && s[i] <= '9')
			num = num * 10 + s[i] - '0';
		else if (s[0] == '-' && i == 0)
			flag = 1;
		else
			break;
	 }

		if (flag == 1)
			num = num * -1;

	 return num;

}

void *_rtw_vmalloc(u32 sz)
{
	u8 	*pbuf;
	pbuf = vmalloc(sz);

	return pbuf;
}

void *_rtw_zvmalloc(u32 sz)
{
	u8 *pbuf;

	pbuf = _rtw_vmalloc(sz);
	if (pbuf != NULL)
		memset(pbuf, 0, sz);

	return pbuf;
}

inline void _rtw_vmfree(void *pbuf)
{
	vfree(pbuf);
}

void *_rtw_malloc(u32 sz)
{
	u8 *pbuf = NULL;

	pbuf = kmalloc(sz, in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);

	return pbuf;
}


void *_rtw_zmalloc(u32 sz)
{
	u8 *pbuf = _rtw_malloc(sz);

	if (pbuf != NULL) {
		memset(pbuf, 0, sz);
	}

	return pbuf;
}

void	_rtw_mfree(void *pbuf)
{
	kfree(pbuf);
}


void *rtw_malloc2d(int h, int w, int size)
{
	int j;

	void **a = (void **) rtw_zmalloc(h*sizeof(void *) + h*w*size);

	if (a == NULL) {
		DBG_871X("%s: alloc memory fail!\n", __FUNCTION__);
		return NULL;
	}

	for (j = 0; j < h; j++)
		a[j] = ((char *)(a+h)) + j * w * size;

	return a;
}

void rtw_mfree2d(void *pbuf, int h, int w, int size)
{
	rtw_mfree(pbuf);
}

int _rtw_memcmp(void *dst, void *src, u32 sz)
{
	if (!(memcmp(dst, src, sz)))
		return _TRUE;
	else
		return _FALSE;
}

void rtw_init_timer(_timer *ptimer, void *rtlpriv, void *pfunc)
{
	struct rtl_priv *adapter = (struct rtl_priv *)rtlpriv;

	_init_timer(ptimer, adapter->ndev, pfunc, adapter);
}

void _rtw_init_queue(struct __queue *pqueue)
{

	INIT_LIST_HEAD(&(pqueue->queue));

	spin_lock_init(&(pqueue->lock));

}

u32 _rtw_queue_empty(struct __queue *pqueue)
{
	return list_empty(&(pqueue->queue));
}

u32 rtw_end_of_queue_search(struct list_head *head, struct list_head *plist)
{
	if (head == plist)
		return _TRUE;
	else
		return _FALSE;
}

inline u32 rtw_systime_to_ms(u32 systime)
{
	return systime * 1000 / HZ;
}

inline u32 rtw_ms_to_systime(u32 ms)
{
	return ms * HZ / 1000;
}

/*
 *  the input parameter start use the same unit as returned by jiffies
 */
inline int32_t rtw_get_passing_time_ms(u32 start)
{
	return rtw_systime_to_ms(jiffies-start);
}

inline int32_t rtw_get_time_interval_ms(u32 start, u32 end)
{
	return rtw_systime_to_ms(end-start);
}


void rtw_sleep_schedulable(int ms)
{
	u32 delta;

	delta = (ms * HZ)/1000;	/* (ms) */
	if (delta == 0) {
		delta = 1;	/* 1 ms */
	}
	set_current_state(TASK_INTERRUPTIBLE);
	if (schedule_timeout(delta) != 0) {
		return;
	}
	return;
}

void rtw_usleep_os(int us)
{
	if (1 < (us/1000))
		msleep(1);
	else
		msleep((us/1000) + 1);
}

void rtw_yield_os(void)
{
	yield();
}

u64 rtw_modular64(u64 x, u64 y)
{
	return do_div(x, y);
}

void rtw_buf_free(u8 **buf, u32 *buf_len)
{
	u32 ori_len;

	if (!buf || !buf_len)
		return;

	ori_len = *buf_len;

	if (*buf) {
		*buf_len = 0;
		_rtw_mfree(*buf);
		*buf = NULL;
	}
}

void rtw_buf_update(u8 **buf, u32 *buf_len, u8 *src, u32 src_len)
{
	u32 ori_len = 0, dup_len = 0;
	u8 *ori = NULL;
	u8 *dup = NULL;

	if (!buf || !buf_len)
		return;

	if (!src || !src_len)
		goto keep_ori;

	/* duplicate src */
	dup = rtw_malloc(src_len);
	if (dup) {
		dup_len = src_len;
		memcpy(dup, src, dup_len);
	}

keep_ori:
	ori = *buf;
	ori_len = *buf_len;

	/* replace buf with dup */
	*buf_len = 0;
	*buf = dup;
	*buf_len = dup_len;

	/* free ori */
	if (ori && ori_len > 0) {
	/* ULLI check usage of param ori_len */
		_rtw_mfree(ori);
	}
}


/**
 * rtw_cbuf_full - test if cbuf is full
 * @cbuf: pointer of struct rtw_cbuf
 *
 * Returns: _TRUE if cbuf is full
 */
inline bool rtw_cbuf_full(struct rtw_cbuf *cbuf)
{
	return (cbuf->write == cbuf->read-1) ? _TRUE : _FALSE;
}

/**
 * rtw_cbuf_empty - test if cbuf is empty
 * @cbuf: pointer of struct rtw_cbuf
 *
 * Returns: _TRUE if cbuf is empty
 */
inline bool rtw_cbuf_empty(struct rtw_cbuf *cbuf)
{
	return (cbuf->write == cbuf->read) ? _TRUE : _FALSE;
}

/**
 * rtw_cbuf_push - push a pointer into cbuf
 * @cbuf: pointer of struct rtw_cbuf
 * @buf: pointer to push in
 *
 * Lock free operation, be careful of the use scheme
 * Returns: _TRUE push success
 */
bool rtw_cbuf_push(struct rtw_cbuf *cbuf, void *buf)
{
	if (rtw_cbuf_full(cbuf))
		return _FAIL;

	if (0)
		DBG_871X("%s on %u\n", __func__, cbuf->write);
	cbuf->bufs[cbuf->write] = buf;
	cbuf->write = (cbuf->write+1)%cbuf->size;

	return _SUCCESS;
}

/**
 * rtw_cbuf_pop - pop a pointer from cbuf
 * @cbuf: pointer of struct rtw_cbuf
 *
 * Lock free operation, be careful of the use scheme
 * Returns: pointer popped out
 */
void *rtw_cbuf_pop(struct rtw_cbuf *cbuf)
{
	void *buf;
	if (rtw_cbuf_empty(cbuf))
		return NULL;

	if (0)
		DBG_871X("%s on %u\n", __func__, cbuf->read);
	buf = cbuf->bufs[cbuf->read];
	cbuf->read = (cbuf->read+1)%cbuf->size;

	return buf;
}

/**
 * rtw_cbuf_alloc - allocte a rtw_cbuf with given size and do initialization
 * @size: size of pointer
 *
 * Returns: pointer of srtuct rtw_cbuf, NULL for allocation failure
 */
struct rtw_cbuf *rtw_cbuf_alloc(u32 size)
{
	struct rtw_cbuf *cbuf;

	cbuf = (struct rtw_cbuf *)rtw_malloc(sizeof(*cbuf) + sizeof(void *) * size);

	if (cbuf) {
		cbuf->write = cbuf->read = 0;
		cbuf->size = size;
	}

	return cbuf;
}

/**
 * rtw_cbuf_free - free the given rtw_cbuf
 * @cbuf: pointer of struct rtw_cbuf to free
 */
void rtw_cbuf_free(struct rtw_cbuf *cbuf)
{
	rtw_mfree(cbuf);
}

