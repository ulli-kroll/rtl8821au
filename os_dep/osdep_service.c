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

#define RT_TAG	'1178'



#if defined(PLATFORM_LINUX)
/*
* Translate the OS dependent @param error_code to OS independent RTW_STATUS_CODE
* @return: one of RTW_STATUS_CODE
*/
inline int RTW_STATUS_CODE(int error_code){
	if(error_code >=0)
		return _SUCCESS;

	switch(error_code) {
		//case -ETIMEDOUT:
		//	return RTW_STATUS_TIMEDOUT;
		default:
			return _FAIL;
	}
}
#else
inline int RTW_STATUS_CODE(int error_code){
	return error_code;
}
#endif

u32 rtw_atoi(u8* s)
{

	int num=0,flag=0;
	int i;
	for(i=0;i<=strlen(s);i++)
	{
	  if(s[i] >= '0' && s[i] <= '9')
		 num = num * 10 + s[i] -'0';
	  else if(s[0] == '-' && i==0)
		 flag =1;
	  else
		  break;
	 }

	if(flag == 1)
	   num = num * -1;

	 return(num);

}

inline u8* _rtw_vmalloc(u32 sz)
{
	u8 	*pbuf;
#ifdef PLATFORM_LINUX
	pbuf = vmalloc(sz);
#endif



	return pbuf;
}

inline u8* _rtw_zvmalloc(u32 sz)
{
	u8 	*pbuf;
#ifdef PLATFORM_LINUX
	pbuf = _rtw_vmalloc(sz);
	if (pbuf != NULL)
		memset(pbuf, 0, sz);
#endif

	return pbuf;
}

inline void _rtw_vmfree(void *pbuf)
{
#ifdef	PLATFORM_LINUX
	vfree(pbuf);
#endif

}

u8* _rtw_malloc(u32 sz)
{

	u8 	*pbuf=NULL;

#ifdef PLATFORM_LINUX
		pbuf = kmalloc(sz,in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);

#endif


	return pbuf;

}


u8* _rtw_zmalloc(u32 sz)
{
	u8 	*pbuf = _rtw_malloc(sz);

	if (pbuf != NULL) {

#ifdef PLATFORM_LINUX
		memset(pbuf, 0, sz);
#endif


	}

	return pbuf;
}

void	_rtw_mfree(void *pbuf)
{

#ifdef	PLATFORM_LINUX
		kfree(pbuf);

#endif


}


void* rtw_malloc2d(int h, int w, int size)
{
	int j;

	void **a = (void **) rtw_zmalloc( h*sizeof(void *) + h*w*size );
	if(a == NULL)
	{
		DBG_871X("%s: alloc memory fail!\n", __FUNCTION__);
		return NULL;
	}

	for( j=0; j<h; j++ )
		a[j] = ((char *)(a+h)) + j*w*size;

	return a;
}

void rtw_mfree2d(void *pbuf, int h, int w, int size)
{
	/* ULLI check usage of param h, w, size */

	rtw_mfree(pbuf);
}

int	_rtw_memcmp(void *dst, void *src, u32 sz)
{

#if defined (PLATFORM_LINUX)
//under Linux/GNU/GLibc, the return value of memcmp for two same mem. chunk is 0

	if (!(memcmp(dst, src, sz)))
		return _TRUE;
	else
		return _FALSE;
#endif


}





void _rtw_init_listhead(struct list_head *list)
{

#ifdef PLATFORM_LINUX

        INIT_LIST_HEAD(list);

#endif


}


/*
For the following list_xxx operations,
caller must guarantee the atomic context.
Otherwise, there will be racing condition.
*/
u32	rtw_is_list_empty(struct list_head *phead)
{

#ifdef PLATFORM_LINUX

	if (list_empty(phead))
		return _TRUE;
	else
		return _FALSE;

#endif



}

void rtw_list_insert_head(struct list_head *plist, struct list_head *phead)
{

#ifdef PLATFORM_LINUX
	list_add(plist, phead);
#endif

}

void rtw_list_insert_tail(struct list_head *plist, struct list_head *phead)
{

#ifdef PLATFORM_LINUX

	list_add_tail(plist, phead);

#endif

}

void rtw_init_timer(_timer *ptimer, void *padapter, void *pfunc)
{
	_adapter *adapter = (_adapter *)padapter;

#ifdef PLATFORM_LINUX
	_init_timer(ptimer, adapter->ndev, pfunc, adapter);
#endif
}

/*

Caller must check if the list is empty before calling rtw_list_delete

*/

void	_rtw_spinlock_init(_lock *plock)
{

#ifdef PLATFORM_LINUX

	spin_lock_init(plock);

#endif

}

void	_rtw_spinlock_free(_lock *plock)
{


}

void	_rtw_spinlock(_lock	*plock)
{

#ifdef PLATFORM_LINUX

	spin_lock(plock);

#endif

}

void	_rtw_spinunlock(_lock *plock)
{

#ifdef PLATFORM_LINUX

	spin_unlock(plock);

#endif
}


void	_rtw_spinlock_ex(_lock	*plock)
{

#ifdef PLATFORM_LINUX

	spin_lock(plock);

#endif

}

void	_rtw_spinunlock_ex(_lock *plock)
{

#ifdef PLATFORM_LINUX

	spin_unlock(plock);

#endif
}



void	_rtw_init_queue(_queue	*pqueue)
{

	_rtw_init_listhead(&(pqueue->queue));

	_rtw_spinlock_init(&(pqueue->lock));

}

u32	  _rtw_queue_empty(_queue	*pqueue)
{
	return (rtw_is_list_empty(&(pqueue->queue)));
}


u32 rtw_end_of_queue_search(struct list_head *head, struct list_head *plist)
{
	if (head == plist)
		return _TRUE;
	else
		return _FALSE;
}


u32	rtw_get_current_time(void)
{

#ifdef PLATFORM_LINUX
	return jiffies;
#endif
}

inline u32 rtw_systime_to_ms(u32 systime)
{
#ifdef PLATFORM_LINUX
	return systime * 1000 / HZ;
#endif
}

inline u32 rtw_ms_to_systime(u32 ms)
{
#ifdef PLATFORM_LINUX
	return ms * HZ / 1000;
#endif
}

// the input parameter start use the same unit as returned by rtw_get_current_time
inline int32_t rtw_get_passing_time_ms(u32 start)
{
#ifdef PLATFORM_LINUX
	return rtw_systime_to_ms(jiffies-start);
#endif
}

inline int32_t rtw_get_time_interval_ms(u32 start, u32 end)
{
#ifdef PLATFORM_LINUX
	return rtw_systime_to_ms(end-start);
#endif
}


void rtw_sleep_schedulable(int ms)
{

#ifdef PLATFORM_LINUX

    u32 delta;

    delta = (ms * HZ)/1000;//(ms)
    if (delta == 0) {
        delta = 1;// 1 ms
    }
    set_current_state(TASK_INTERRUPTIBLE);
    if (schedule_timeout(delta) != 0) {
        return ;
    }
    return;

#endif


}


void rtw_msleep_os(int ms)
{

#ifdef PLATFORM_LINUX

  	msleep((unsigned int)ms);

#endif


}
void rtw_usleep_os(int us)
{

#ifdef PLATFORM_LINUX

      // msleep((unsigned int)us);
      if ( 1 < (us/1000) )
                msleep(1);
      else
		msleep( (us/1000) + 1);

#endif


}


#ifdef DBG_DELAY_OS
void _rtw_mdelay_os(int ms, const char *func, const int line)
{
	#if 0
	if(ms>10)
		DBG_871X("%s:%d %s(%d)\n", func, line, __FUNCTION__, ms);
		rtw_msleep_os(ms);
	return;
	#endif


	DBG_871X("%s:%d %s(%d)\n", func, line, __FUNCTION__, ms);

#if defined(PLATFORM_LINUX)

   	mdelay((unsigned long)ms);

#endif


}
void _rtw_udelay_os(int us, const char *func, const int line)
{

	#if 0
	if(us > 1000) {
	DBG_871X("%s:%d %s(%d)\n", func, line, __FUNCTION__, us);
		rtw_usleep_os(us);
		return;
	}
	#endif


	DBG_871X("%s:%d %s(%d)\n", func, line, __FUNCTION__, us);


#if defined(PLATFORM_LINUX)

      udelay((unsigned long)us);

#endif

}
#else
void rtw_mdelay_os(int ms)
{

#ifdef PLATFORM_LINUX

   	mdelay((unsigned long)ms);

#endif


}
void rtw_udelay_os(int us)
{

#ifdef PLATFORM_LINUX

      udelay((unsigned long)us);

#endif

}
#endif

void rtw_yield_os()
{
#ifdef PLATFORM_LINUX
	yield();
#endif
}

#define RTW_SUSPEND_LOCK_NAME "rtw_wifi"

#ifdef CONFIG_WAKELOCK
static struct wake_lock rtw_suspend_lock;
#elif defined(CONFIG_ANDROID_POWER)
static android_suspend_lock_t rtw_suspend_lock ={
	.name = RTW_SUSPEND_LOCK_NAME
};
#endif

inline void rtw_suspend_lock_init()
{
	#ifdef CONFIG_WAKELOCK
	wake_lock_init(&rtw_suspend_lock, WAKE_LOCK_SUSPEND, RTW_SUSPEND_LOCK_NAME);
	#elif defined(CONFIG_ANDROID_POWER)
	android_init_suspend_lock(&rtw_suspend_lock);
	#endif
}

inline void rtw_suspend_lock_uninit()
{
	#ifdef CONFIG_WAKELOCK
	wake_lock_destroy(&rtw_suspend_lock);
	#elif defined(CONFIG_ANDROID_POWER)
	android_uninit_suspend_lock(&rtw_suspend_lock);
	#endif
}

inline void rtw_lock_suspend()
{
	#ifdef CONFIG_WAKELOCK
	wake_lock(&rtw_suspend_lock);
	#elif defined(CONFIG_ANDROID_POWER)
	android_lock_suspend(&rtw_suspend_lock);
	#endif

	#if  defined(CONFIG_WAKELOCK) || defined(CONFIG_ANDROID_POWER)
	//DBG_871X("####%s: suspend_lock_count:%d####\n", __FUNCTION__, rtw_suspend_lock.stat.count);
	#endif
}

inline void rtw_unlock_suspend()
{
	#ifdef CONFIG_WAKELOCK
	wake_unlock(&rtw_suspend_lock);
	#elif defined(CONFIG_ANDROID_POWER)
	android_unlock_suspend(&rtw_suspend_lock);
	#endif

	#if  defined(CONFIG_WAKELOCK) || defined(CONFIG_ANDROID_POWER)
	//DBG_871X("####%s: suspend_lock_count:%d####\n", __FUNCTION__, rtw_suspend_lock.stat.count);
	#endif
}


#ifdef PLATFORM_LINUX
/*
* Open a file with the specific @param path, @param flag, @param mode
* @param fpp the pointer of struct file pointer to get struct file pointer while file opening is success
* @param path the path of the file to open
* @param flag file operation flags, please refer to linux document
* @param mode please refer to linux document
* @return Linux specific error code
*/
static int openFile(struct file **fpp, char *path, int flag, int mode)
{
	struct file *fp;

	fp=filp_open(path, flag, mode);
	if(IS_ERR(fp)) {
		*fpp=NULL;
		return PTR_ERR(fp);
	}
	else {
		*fpp=fp;
		return 0;
	}
}

/*
* Close the file with the specific @param fp
* @param fp the pointer of struct file to close
* @return always 0
*/
static int closeFile(struct file *fp)
{
	filp_close(fp,NULL);
	return 0;
}

static int readFile(struct file *fp,char *buf,int len)
{
	int rlen=0, sum=0;

	if (!fp->f_op || !fp->f_op->read)
		return -EPERM;

	while(sum<len) {
		rlen=fp->f_op->read(fp,buf+sum,len-sum, &fp->f_pos);
		if(rlen>0)
			sum+=rlen;
		else if(0 != rlen)
			return rlen;
		else
			break;
	}

	return  sum;

}

static int writeFile(struct file *fp,char *buf,int len)
{
	int wlen=0, sum=0;

	if (!fp->f_op || !fp->f_op->write)
		return -EPERM;

	while(sum<len) {
		wlen=fp->f_op->write(fp,buf+sum,len-sum, &fp->f_pos);
		if(wlen>0)
			sum+=wlen;
		else if(0 != wlen)
			return wlen;
		else
			break;
	}

	return sum;

}

/*
* Test if the specifi @param path is a file and readable
* @param path the path of the file to test
* @return Linux specific error code
*/
static int isFileReadable(char *path)
{
	struct file *fp;
	int ret = 0;
	mm_segment_t oldfs;
	char buf;

	fp=filp_open(path, O_RDONLY, 0);
	if(IS_ERR(fp)) {
		ret = PTR_ERR(fp);
	}
	else {
		oldfs = get_fs(); set_fs(get_ds());

		if(1!=readFile(fp, &buf, 1))
			ret = PTR_ERR(fp);

		set_fs(oldfs);
		filp_close(fp,NULL);
	}
	return ret;
}

/*
* Open the file with @param path and retrive the file content into memory starting from @param buf for @param sz at most
* @param path the path of the file to open and read
* @param buf the starting address of the buffer to store file content
* @param sz how many bytes to read at most
* @return the byte we've read, or Linux specific error code
*/
static int retriveFromFile(char *path, u8* buf, u32 sz)
{
	int ret =-1;
	mm_segment_t oldfs;
	struct file *fp;

	if(path && buf) {
		if( 0 == (ret=openFile(&fp,path, O_RDONLY, 0)) ){
			DBG_871X("%s openFile path:%s fp=%p\n",__FUNCTION__, path ,fp);

			oldfs = get_fs(); set_fs(get_ds());
			ret=readFile(fp, buf, sz);
			set_fs(oldfs);
			closeFile(fp);

			DBG_871X("%s readFile, ret:%d\n",__FUNCTION__, ret);

		} else {
			DBG_871X("%s openFile path:%s Fail, ret:%d\n",__FUNCTION__, path, ret);
		}
	} else {
		DBG_871X("%s NULL pointer\n",__FUNCTION__);
		ret =  -EINVAL;
	}
	return ret;
}

/*
* Open the file with @param path and wirte @param sz byte of data starting from @param buf into the file
* @param path the path of the file to open and write
* @param buf the starting address of the data to write into file
* @param sz how many bytes to write at most
* @return the byte we've written, or Linux specific error code
*/
static int storeToFile(char *path, u8* buf, u32 sz)
{
	int ret =0;
	mm_segment_t oldfs;
	struct file *fp;

	if(path && buf) {
		if( 0 == (ret=openFile(&fp, path, O_CREAT|O_WRONLY, 0666)) ) {
			DBG_871X("%s openFile path:%s fp=%p\n",__FUNCTION__, path ,fp);

			oldfs = get_fs(); set_fs(get_ds());
			ret=writeFile(fp, buf, sz);
			set_fs(oldfs);
			closeFile(fp);

			DBG_871X("%s writeFile, ret:%d\n",__FUNCTION__, ret);

		} else {
			DBG_871X("%s openFile path:%s Fail, ret:%d\n",__FUNCTION__, path, ret);
		}
	} else {
		DBG_871X("%s NULL pointer\n",__FUNCTION__);
		ret =  -EINVAL;
	}
	return ret;
}
#endif //PLATFORM_LINUX

/*
* Test if the specifi @param path is a file and readable
* @param path the path of the file to test
* @return _TRUE or _FALSE
*/
int rtw_is_file_readable(char *path)
{
#ifdef PLATFORM_LINUX
	if(isFileReadable(path) == 0)
		return _TRUE;
	else
		return _FALSE;
#else
	//Todo...
	return _FALSE;
#endif
}

/*
* Open the file with @param path and retrive the file content into memory starting from @param buf for @param sz at most
* @param path the path of the file to open and read
* @param buf the starting address of the buffer to store file content
* @param sz how many bytes to read at most
* @return the byte we've read
*/
int rtw_retrive_from_file(char *path, u8* buf, u32 sz)
{
#ifdef PLATFORM_LINUX
	int ret =retriveFromFile(path, buf, sz);
	return ret>=0?ret:0;
#else
	//Todo...
	return 0;
#endif
}

/*
* Open the file with @param path and wirte @param sz byte of data starting from @param buf into the file
* @param path the path of the file to open and write
* @param buf the starting address of the data to write into file
* @param sz how many bytes to write at most
* @return the byte we've written
*/
int rtw_store_to_file(char *path, u8* buf, u32 sz)
{
#ifdef PLATFORM_LINUX
	int ret =storeToFile(path, buf, sz);
	return ret>=0?ret:0;
#else
	//Todo...
	return 0;
#endif
}

#ifdef PLATFORM_LINUX
struct net_device *rtw_alloc_etherdev_with_old_priv(struct _ADAPTER *old_priv)
{
	struct net_device *ndev;
	struct rtw_netdev_priv_indicator *pnpi;

	ndev = alloc_etherdev_mq(sizeof(struct rtw_netdev_priv_indicator), 4);
	if (!ndev)
		goto RETURN;

	pnpi = netdev_priv(ndev);
	pnpi->priv=old_priv;

RETURN:
	return ndev;
}

struct net_device *rtw_alloc_etherdev(int sizeof_priv)
{
	struct net_device *ndev;
	struct rtw_netdev_priv_indicator *pnpi;

	ndev = alloc_etherdev_mq(sizeof(struct rtw_netdev_priv_indicator), 4);
	if (!ndev)
		goto RETURN;

	pnpi = netdev_priv(ndev);

	pnpi->priv = rtw_zvmalloc(sizeof_priv);
	if (!pnpi->priv) {
		free_netdev(ndev);
		ndev = NULL;
		goto RETURN;
	}

RETURN:
	return ndev;
}

void rtw_free_netdev(struct net_device * netdev)
{
	struct rtw_netdev_priv_indicator *pnpi;

	if(!netdev)
		goto RETURN;

	pnpi = netdev_priv(netdev);

	if(!pnpi->priv)
		goto RETURN;

	/* ULLI check usage of pnpi->sizeof_priv */
	rtw_vmfree(pnpi->priv);
	free_netdev(netdev);

RETURN:
	return;
}

/*
* Jeff: this function should be called under ioctl (rtnl_lock is accquired) while
* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26)
*/
int rtw_change_ifname(_adapter *padapter, const char *ifname)
{
	struct net_device *ndev;
	struct net_device *cur_ndev = padapter->ndev;
	struct rereg_nd_name_data *rereg_priv;
	int ret;

	if(!padapter)
		goto error;

	rereg_priv = &padapter->rereg_nd_name_priv;

	//free the old_ndev
	if(rereg_priv->old_ndev) {
		free_netdev(rereg_priv->old_ndev);
		rereg_priv->old_ndev = NULL;
	}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26))
	if(!rtnl_is_locked())
		unregister_netdev(cur_ndev);
	else
#endif
		unregister_netdevice(cur_ndev);

	rtw_proc_remove_one(cur_ndev);

	rereg_priv->old_ndev=cur_ndev;

	ndev = rtw_init_netdev(padapter);
	if (!ndev)  {
		ret = -1;
		goto error;
	}

	SET_NETDEV_DEV(ndev, dvobj_to_dev(adapter_to_dvobj(padapter)));

	rtw_init_netdev_name(ndev, ifname);

	memcpy(ndev->dev_addr, padapter->eeprompriv.mac_addr, ETH_ALEN);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26))
	if(!rtnl_is_locked())
		ret = register_netdev(ndev);
	else
#endif
		ret = register_netdevice(ndev);

	if ( ret != 0) {
		RT_TRACE(_module_hci_intfs_c_,_drv_err_,("register_netdev() failed\n"));
		goto error;
	}

	rtw_proc_init_one(ndev);

	return 0;

error:

	return -1;

}
#endif

u64 rtw_modular64(u64 x, u64 y)
{
#ifdef PLATFORM_LINUX
	return do_div(x, y);
#endif
}

u64 rtw_division64(u64 x, u64 y)
{
#ifdef PLATFORM_LINUX
	do_div(x, y);
	return x;
#endif
}

void rtw_buf_free(u8 **buf, u32 *buf_len)
{
	u32 ori_len;

	if (!buf || !buf_len)
		return;

	ori_len = *buf_len;

	if (*buf) {
		*buf_len = 0;
		/* ULLI check usage of param *buf_len */
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
	return (cbuf->write == cbuf->read-1)? _TRUE : _FALSE;
}

/**
 * rtw_cbuf_empty - test if cbuf is empty
 * @cbuf: pointer of struct rtw_cbuf
 *
 * Returns: _TRUE if cbuf is empty
 */
inline bool rtw_cbuf_empty(struct rtw_cbuf *cbuf)
{
	return (cbuf->write == cbuf->read)? _TRUE : _FALSE;
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

	cbuf = (struct rtw_cbuf *)rtw_malloc(sizeof(*cbuf) + sizeof(void*)*size);

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
	/* ULLI check usage of cbuf->size */
	rtw_mfree(cbuf);
}

