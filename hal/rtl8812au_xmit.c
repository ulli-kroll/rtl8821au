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
#define _RTL8812AU_XMIT_C_

#include <rtl8812a_hal.h>
#include <../rtl8821au/trx.h>

int32_t	rtl8812au_init_xmit_priv(struct rtl_priv *rtlpriv)
{
	struct xmit_priv	*pxmitpriv = &rtlpriv->xmitpriv;
	 struct _rtw_hal	*pHalData = GET_HAL_DATA(rtlpriv);

#ifdef PLATFORM_LINUX
	tasklet_init(&pxmitpriv->xmit_tasklet,
	     (void(*)(unsigned long))rtl8812au_xmit_tasklet,
	     (unsigned long)rtlpriv);
#endif
	return _SUCCESS;
}

void	rtl8812au_free_xmit_priv(struct rtl_priv *rtlpriv)
{
}


