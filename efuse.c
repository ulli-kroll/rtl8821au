#include <drv_types.h>
#include "wifi.h"

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

static void efuse_read_all_map(struct rtl_priv *rtlpriv, uint8_t efuseType,
		uint8_t	*Efuse)
{
	u16	mapLen=0;

	rtlpriv->cfg->ops->EfusePowerSwitch(rtlpriv,_FALSE, _TRUE);

	rtlpriv->cfg->ops->EFUSEGetEfuseDefinition(rtlpriv, efuseType, TYPE_EFUSE_MAP_LEN, (void *)&mapLen);

	rtlpriv->cfg->ops->ReadEFuse(rtlpriv, efuseType, 0, mapLen, Efuse);

	rtlpriv->cfg->ops->EfusePowerSwitch(rtlpriv,_FALSE, _FALSE);
}

uint8_t rtw_efuse_map_read(struct rtl_priv *rtlpriv, u16 addr, u16 cnts, uint8_t *data)
{
	u16	mapLen=0;

	rtlpriv->cfg->ops->EFUSEGetEfuseDefinition(rtlpriv, EFUSE_WIFI, TYPE_EFUSE_MAP_LEN, (void *)&mapLen);

	if ((addr + cnts) > mapLen)
		return _FAIL;

	rtlpriv->cfg->ops->EfusePowerSwitch(rtlpriv, _FALSE, _TRUE);

	rtlpriv->cfg->ops->ReadEFuse(rtlpriv, EFUSE_WIFI, addr, cnts, data);

	rtlpriv->cfg->ops->EfusePowerSwitch(rtlpriv, _FALSE, _FALSE);

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

