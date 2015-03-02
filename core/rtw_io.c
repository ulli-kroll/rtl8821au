/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
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
/*

The purpose of rtw_io.c

a. provides the API

b. provides the protocol engine

c. provides the software interface between caller and the hardware interface


Compiler Flag Option:


2. CONFIG_USB_HCI:
   a. USE_ASYNC_IRP: Both sync/async operations are provided.

3. CONFIG_CFIO_HCI:
   b. USE_SYNC_IRP: Only sync operations are provided.


Only sync read/rtw_write_mem operations are provided.

jackson@realtek.com.tw

*/

#define _RTW_IO_C_

#include <drv_types.h>


int rtw_writeN(struct rtl_priv *adapter, uint32_t addr, uint32_t length,
	uint8_t *pdata)
{
	struct io_priv *pio_priv = &adapter->iopriv;
	struct	intf_hdl *pintfhdl = &(pio_priv->intf);
	int ret;

	ret = pintfhdl->io_ops._writeN(pintfhdl, addr, length, pdata);

	return RTW_STATUS_CODE(ret);
}

int _rtw_write8_async(struct rtl_priv *adapter, uint32_t addr, uint8_t val)
{
	struct io_priv *pio_priv = &adapter->iopriv;
	struct intf_hdl	*pintfhdl = &(pio_priv->intf);
	int ret;

	ret = pintfhdl->io_ops._write8_async(pintfhdl, addr, val);

	return RTW_STATUS_CODE(ret);
}

int _rtw_write16_async(struct rtl_priv *adapter, uint32_t addr, u16 val)
{
	struct io_priv *pio_priv = &adapter->iopriv;
	struct intf_hdl	*pintfhdl = &pio_priv->intf;
	int ret;

	val = cpu_to_le16(val);
	ret = pintfhdl->io_ops._write16_async(pintfhdl, addr, val);

	return RTW_STATUS_CODE(ret);
}

int _rtw_write32_async(struct rtl_priv *adapter, uint32_t addr, uint32_t val)
{
	struct io_priv *pio_priv = &adapter->iopriv;
	struct intf_hdl *pintfhdl = &pio_priv->intf;
	int ret;

	val = cpu_to_le32(val);
	ret = pintfhdl->io_ops._write32_async(pintfhdl, addr, val);

	return RTW_STATUS_CODE(ret);
}

void _rtw_read_port(struct rtl_priv *adapter, uint32_t addr, uint32_t cnt,
	uint8_t *pmem)
{
	struct io_priv *pio_priv = &adapter->iopriv;
	struct intf_hdl	*pintfhdl = &pio_priv->intf;

	if ((adapter->bDriverStopped == _TRUE)
	   || (adapter->bSurpriseRemoved == _TRUE)) {
		return;
	}

	pintfhdl->io_ops._read_port(pintfhdl, addr, cnt, pmem);

}

void _rtw_read_port_cancel(struct rtl_priv *adapter)
{
	struct io_priv *pio_priv = &adapter->iopriv;
	struct intf_hdl *pintfhdl = &(pio_priv->intf);

	if (pintfhdl->io_ops._read_port_cancel)
		pintfhdl->io_ops._read_port_cancel(pintfhdl);
}

uint32_t _rtw_write_port(struct rtl_priv *adapter, uint32_t addr,
	uint32_t cnt, struct xmit_buf *pxmitbuf)
{
	struct io_priv *pio_priv = &adapter->iopriv;
	struct	intf_hdl *pintfhdl = &(pio_priv->intf);
	uint32_t ret = _SUCCESS;

	pintfhdl->io_ops._write_port(pintfhdl, addr, cnt, pxmitbuf);

	return ret;
}

uint32_t _rtw_write_port_and_wait(struct rtl_priv *adapter, uint32_t addr,
	uint32_t cnt, struct xmit_buf *pxmitbuf, int timeout_ms)
{
	int ret = _SUCCESS;
	struct submit_ctx sctx;

	rtw_sctx_init(&sctx, timeout_ms);
	pxmitbuf->sctx = &sctx;

	ret = _rtw_write_port(adapter, addr, cnt, pxmitbuf);

	if (ret == _SUCCESS)
		ret = rtw_sctx_wait(&sctx);

	 return ret;
}

void _rtw_write_port_cancel(struct rtl_priv *adapter)
{
	void (*_write_port_cancel)(struct intf_hdl *pintfhdl);
	struct io_priv *pio_priv = &adapter->iopriv;
	struct intf_hdl *pintfhdl = &(pio_priv->intf);

	_write_port_cancel = pintfhdl->io_ops._write_port_cancel;

	if (_write_port_cancel)
		_write_port_cancel(pintfhdl);

}

