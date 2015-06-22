#include <drv_types.h>
#include "wifi.h"
#include <rtl8812a_spec.h>

static void efuse_ShadowRead1Byte(struct rtl_priv *rtlpriv, u16	Offset,
	 	u8 *Value)
{
	struct rtl_efuse *rtlefuse = rtl_efuse(rtlpriv);

	*Value = rtlefuse->efuse_map[0][Offset];
}

static void efuse_ShadowRead2Byte(struct rtl_priv *rtlpriv, u16	Offset,
		u16 *Value)
{
	struct rtl_efuse *rtlefuse = rtl_efuse(rtlpriv);

	*Value = rtlefuse->efuse_map[0][Offset];
	*Value |= rtlefuse->efuse_map[0][Offset+1] << 8;
}

//---------------Read Four Bytes
static void efuse_ShadowRead4Byte(struct rtl_priv *rtlpriv, u16	Offset,
	 	u32 *Value)
{
	struct rtl_efuse *rtlefuse = rtl_efuse(rtlpriv);

	*Value = rtlefuse->efuse_map[0][Offset];
	*Value |= rtlefuse->efuse_map[0][Offset + 1] << 8;
	*Value |= rtlefuse->efuse_map[0][Offset + 2] << 16;
	*Value |= rtlefuse->efuse_map[0][Offset + 3] << 24;
}

static void efuse_ShadowWrite1Byte(struct rtl_priv *rtlpriv, u16 Offset,
		u8 Value)
{
	struct rtl_efuse *rtlefuse = rtl_efuse(rtlpriv);

	rtlefuse->efuse_map[0][Offset] = Value;

}

static void efuse_ShadowWrite2Byte(struct rtl_priv *rtlpriv, u16 Offset,
		u16 Value)
{
	struct rtl_efuse *rtlefuse = rtl_efuse(rtlpriv);

	rtlefuse->efuse_map[0][Offset] = Value & 0x0ff;
	rtlefuse->efuse_map[0][Offset+1] = Value >> 8;
}

static void efuse_ShadowWrite4Byte(struct rtl_priv *rtlpriv, u16 Offset,
		u32 Value)
{
	struct rtl_efuse *rtlefuse = rtl_efuse(rtlpriv);

	rtlefuse->efuse_map[0][Offset] = (u8) (Value & 0x0000000ff);
	rtlefuse->efuse_map[0][Offset+1] = (u8) ((Value >> 8) & 0x0000ff);
	rtlefuse->efuse_map[0][Offset+2] = (u8) ((Value >> 16) & 0x00ff);
	rtlefuse->efuse_map[0][Offset+3] = (u8) ((Value >> 24) & 0xff);
}

void EFUSE_ShadowRead(struct rtl_priv *rtlpriv, u8 Type,
		u16 Offset, u32 *Value)
{
	if (Type == 1)
		efuse_ShadowRead1Byte(rtlpriv, Offset, (uint8_t *)Value);
	else if (Type == 2)
		efuse_ShadowRead2Byte(rtlpriv, Offset, (u16 *)Value);
	else if (Type == 4)
		efuse_ShadowRead4Byte(rtlpriv, Offset, (uint32_t *)Value);

}	// EFUSE_ShadowRead

static void EfusePowerSwitch(struct rtl_priv *rtlpriv, uint8_t bWrite, uint8_t PwrState)
{
	uint8_t	tempval;
	u16	tmpV16;
#define EFUSE_ACCESS_ON_JAGUAR 0x69
#define EFUSE_ACCESS_OFF_JAGUAR 0x00
	if (PwrState == _TRUE) {
		rtl_write_byte(rtlpriv, REG_EFUSE_BURN_GNT_8812, EFUSE_ACCESS_ON_JAGUAR);

		/* 1.2V Power: From VDDON with Power Cut(0x0000h[15]), defualt valid */
		tmpV16 = rtl_read_word(rtlpriv, REG_SYS_ISO_CTRL);
		if (!(tmpV16 & rtlpriv->cfg->maps[EFUSE_PWC_EV12V])) {
			tmpV16 |= rtlpriv->cfg->maps[EFUSE_PWC_EV12V];
			/* rtl_write_word(rtlpriv,REG_SYS_ISO_CTRL,tmpV16); */
		}
		/* Reset: 0x0000h[28], default valid */
		tmpV16 =  rtl_read_word(rtlpriv, REG_SYS_FUNC_EN);
		if (!(tmpV16 & rtlpriv->cfg->maps[EFUSE_FEN_ELDR])) {
			tmpV16 |= rtlpriv->cfg->maps[EFUSE_FEN_ELDR] ;
			rtl_write_word(rtlpriv, REG_SYS_FUNC_EN, tmpV16);
		}

		/* Clock: Gated(0x0008h[5]) 8M(0x0008h[1]) clock from ANA, default valid */
		tmpV16 = rtl_read_word(rtlpriv, REG_SYS_CLKR);
		if ((!(tmpV16 & LOADER_CLK_EN)) || (!(tmpV16 & ANA8M))) {
			tmpV16 |= (LOADER_CLK_EN | ANA8M);
			rtl_write_word(rtlpriv, REG_SYS_CLKR, tmpV16);
		}

		if (bWrite == _TRUE) {
			/* Enable LDO 2.5V before read/write action */
			tempval = rtl_read_byte(rtlpriv, rtlpriv->cfg->maps[EFUSE_TEST]+3);
			tempval &= ~(BIT3|BIT4|BIT5|BIT6);
			tempval |= (VOLTAGE_V25 << 3);
			tempval |= BIT7;
			rtl_write_byte(rtlpriv, rtlpriv->cfg->maps[EFUSE_TEST] + 3, tempval);
		}
	} else {
		rtl_write_byte(rtlpriv, REG_EFUSE_BURN_GNT_8812, EFUSE_ACCESS_OFF_JAGUAR);

		if (bWrite == _TRUE) {
			/* Disable LDO 2.5V after read/write action */
			tempval = rtl_read_byte(rtlpriv, rtlpriv->cfg->maps[EFUSE_TEST] + 3);
			rtl_write_byte(rtlpriv, rtlpriv->cfg->maps[EFUSE_TEST] + 3, (tempval & 0x7F));
		}
	}
}


static void efuse_read_all_map(struct rtl_priv *rtlpriv, uint8_t efuseType,
		uint8_t	*Efuse)
{
	u16	mapLen=0;

	EfusePowerSwitch(rtlpriv,_FALSE, _TRUE);

	rtlpriv->cfg->ops->EFUSEGetEfuseDefinition(rtlpriv, efuseType, TYPE_EFUSE_MAP_LEN, (void *)&mapLen);

	rtlpriv->cfg->ops->ReadEFuse(rtlpriv, efuseType, 0, mapLen, Efuse);

	EfusePowerSwitch(rtlpriv,_FALSE, _FALSE);
}

uint8_t rtw_efuse_map_read(struct rtl_priv *rtlpriv, u16 addr, u16 cnts, uint8_t *data)
{
	u16	mapLen=0;

	rtlpriv->cfg->ops->EFUSEGetEfuseDefinition(rtlpriv, EFUSE_WIFI, TYPE_EFUSE_MAP_LEN, (void *)&mapLen);

	if ((addr + cnts) > mapLen)
		return _FAIL;

	EfusePowerSwitch(rtlpriv, _FALSE, _TRUE);

	rtlpriv->cfg->ops->ReadEFuse(rtlpriv, EFUSE_WIFI, addr, cnts, data);

	EfusePowerSwitch(rtlpriv, _FALSE, _FALSE);

	return _SUCCESS;
}

void EFUSE_ShadowMapUpdate(
	 struct rtl_priv *rtlpriv,
	 uint8_t		efuseType)
{
	struct rtl_efuse *efuse = rtl_efuse(rtlpriv);
	u16	mapLen=0;

	rtlpriv->cfg->ops->EFUSEGetEfuseDefinition(rtlpriv, efuseType, TYPE_EFUSE_MAP_LEN, (void *)&mapLen);

	if (efuse->autoload_failflag == _TRUE)
		memset(&efuse->efuse_map[0][0], 0xFF, mapLen);
	else
		efuse_read_all_map(rtlpriv, efuseType, efuse->efuse_map[0]);

	//PlatformMoveMemory((void *)&pHalData->EfuseMap[EFUSE_MODIFY_MAP][0],
	//(void *)&pHalData->EfuseMap[EFUSE_INIT_MAP][0], mapLen);
}// EFUSE_ShadowMapUpdate

