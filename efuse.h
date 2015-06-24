#ifndef __RTL_EFUSE_H_
#define __RTL_EFUSE_H_

#define EFUSE_INIT_MAP			0
#define EFUSE_MODIFY_MAP		1

uint8_t	efuse_OneByteRead(struct rtl_priv *rtlpriv, u16 addr, uint8_t *data);
uint8_t	efuse_OneByteWrite(struct rtl_priv *rtlpriv, u16 addr, uint8_t data);

enum {
	VOLTAGE_V25	= 0x03,
	LDOE25_SHIFT	= 28 ,
	};

#endif
