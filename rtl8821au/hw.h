#ifndef __RTL8821AU_HW_H__
#define __RTL8821AU_HW_H__


#include <rtl8812a_hal.h>

void SetHwReg8812AU(struct rtl_priv *Adapter, uint8_t variable, u8 *val);
void GetHwReg8812AU(struct rtl_priv *Adapter, uint8_t variable, u8 *val);

#endif
