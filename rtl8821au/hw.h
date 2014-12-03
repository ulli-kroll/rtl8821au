#ifndef __RTL8821AU_HW_H__
#define __RTL8821AU_HW_H__


#include <rtl8812a_hal.h>

void rtl8821au_set_hw_reg(struct rtl_priv *padapter, u8 variable,u8 *val);
void rtl8821au_get_hw_reg(struct rtl_priv *padapter, u8 variable,u8 *val);

#endif
