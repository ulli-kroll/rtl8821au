#include <drv_types.h>
#include "wifi.h"
#include <rtl8812a_spec.h>
#include <rtl8812a_hal.h>

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

static void efuse_power_switch(struct rtl_priv *rtlpriv, u8 write, u8 pwrstate)
{
	uint8_t	tempval;
	u16	tmpV16;
#define EFUSE_ACCESS_ON_JAGUAR 0x69
#define EFUSE_ACCESS_OFF_JAGUAR 0x00
	if (pwrstate) {
		rtl_write_byte(rtlpriv, rtlpriv->cfg->maps[EFUSE_ACCESS], EFUSE_ACCESS_ON_JAGUAR);

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
		if ((!(tmpV16 & rtlpriv->cfg->maps[EFUSE_LOADER_CLK_EN])) || 
		    (!(tmpV16 & rtlpriv->cfg->maps[EFUSE_ANA8M]))) {
			tmpV16 |= (rtlpriv->cfg->maps[EFUSE_LOADER_CLK_EN] | 
			           rtlpriv->cfg->maps[EFUSE_ANA8M]);
			rtl_write_word(rtlpriv, REG_SYS_CLKR, tmpV16);
		}

		if (write) {
			/* Enable LDO 2.5V before read/write action */
			tempval = rtl_read_byte(rtlpriv, rtlpriv->cfg->maps[EFUSE_TEST]+3);
			tempval &= ~(BIT3|BIT4|BIT5|BIT6);
			tempval |= (VOLTAGE_V25 << 3);
			tempval |= BIT7;
			rtl_write_byte(rtlpriv, rtlpriv->cfg->maps[EFUSE_TEST] + 3, tempval);
		}
	} else {
		rtl_write_byte(rtlpriv, REG_EFUSE_BURN_GNT_8812, EFUSE_ACCESS_OFF_JAGUAR);

		if (write) {
			/* Disable LDO 2.5V after read/write action */
			tempval = rtl_read_byte(rtlpriv, rtlpriv->cfg->maps[EFUSE_TEST] + 3);
			rtl_write_byte(rtlpriv, rtlpriv->cfg->maps[EFUSE_TEST] + 3, (tempval & 0x7F));
		}
	}
}


static void read_efuse(struct rtl_priv *rtlpriv, u16 _offset,  u16 _size_byte, u8 *pbuf)
{
	uint8_t	*efuseTbl = NULL;
	u16	eFuse_Addr = 0;
	uint8_t	offset = 0, wden = 0;
	u16	i, j;
	u16	**eFuseWord = NULL;
	u16	efuse_utilized = 0;
	uint8_t	efuse_usage = 0;
	uint8_t	offset_2_0 = 0;
	uint8_t	efuseHeader = 0, efuseExtHdr = 0, efuseData = 0;

	/*
	 * Do NOT excess total size of EFuse table. Added by Roger, 2008.11.10.
	 */
	if ((_offset + _size_byte) > EFUSE_MAP_LEN_JAGUAR) {
		/* total E-Fuse table is 512bytes */
		dev_err(&(rtlpriv->ndev->dev), "Hal_EfuseReadEFuse8812A(): Invalid offset(%#x) with read bytes(%#x)!!\n", _offset, _size_byte);
		goto exit;
	}

	efuseTbl = (uint8_t *) rtw_zmalloc(EFUSE_MAP_LEN_JAGUAR);
	if (efuseTbl == NULL) {
		dev_err(&(rtlpriv->ndev->dev), "%s: alloc efuseTbl fail!\n", __FUNCTION__);
		goto exit;
	}

	eFuseWord = (u16 **) rtw_malloc2d(EFUSE_MAX_SECTION_JAGUAR, EFUSE_MAX_WORD_UNIT, sizeof(u16));
	if (eFuseWord == NULL) {
		dev_err(&(rtlpriv->ndev->dev), "%s: alloc eFuseWord fail!\n", __FUNCTION__);
		goto exit;
	}

	/* 0. Refresh efuse init map as all oxFF. */
	for (i = 0; i < EFUSE_MAX_SECTION_JAGUAR; i++)
		for (j = 0; j < EFUSE_MAX_WORD_UNIT; j++)
			eFuseWord[i][j] = 0xFFFF;

	/*
	 * 1. Read the first byte to check if efuse is empty!!!
	 */
	efuse_OneByteRead(rtlpriv, eFuse_Addr++, &efuseHeader);

	if (efuseHeader != 0xFF) {
		efuse_utilized++;
	} else {
		dev_err(&(rtlpriv->ndev->dev), "EFUSE is empty\n");
		efuse_utilized = 0;
		goto exit;
	}
	/* RT_DISP(FEEPROM, EFUSE_READ_ALL, ("Hal_EfuseReadEFuse8812A(): efuse_utilized: %d\n", efuse_utilized)); */

	/*
	 * 2. Read real efuse content. Filter PG header and every section data.
	 */
	while ((efuseHeader != 0xFF) && AVAILABLE_EFUSE_ADDR_8812(eFuse_Addr)) {
		/* RTPRINT(FEEPROM, EFUSE_READ_ALL, ("efuse_Addr-%d efuse_data=%x\n", eFuse_Addr-1, *rtemp8)); */

		/* Check PG header for section num. */
		if (EXT_HEADER(efuseHeader)) {	/* extended header */
			offset_2_0 = GET_HDR_OFFSET_2_0(efuseHeader);
			/* RT_DISP(FEEPROM, EFUSE_READ_ALL, ("extended header offset_2_0=%X\n", offset_2_0)); */

			efuse_OneByteRead(rtlpriv, eFuse_Addr++, &efuseExtHdr);

			/* RT_DISP(FEEPROM, EFUSE_READ_ALL, ("efuse[%X]=%X\n", eFuse_Addr-1, efuseExtHdr)); */

			if (efuseExtHdr != 0xff) {
				efuse_utilized++;
				if (ALL_WORDS_DISABLED(efuseExtHdr)) {
					efuse_OneByteRead(rtlpriv, eFuse_Addr++, &efuseHeader);
					if (efuseHeader != 0xff) {
						efuse_utilized++;
					}
					break;
				} else {
					offset = ((efuseExtHdr & 0xF0) >> 1) | offset_2_0;
					wden = (efuseExtHdr & 0x0F);
				}
			} else 	{
				dev_err(&(rtlpriv->ndev->dev), "Error condition, extended = 0xff\n");
				/* We should handle this condition. */
				break;
			}
		} else {
			offset = ((efuseHeader >> 4) & 0x0f);
			wden = (efuseHeader & 0x0f);
		}

		if (offset < EFUSE_MAX_SECTION_JAGUAR) {
			/* Get word enable value from PG header */
			/* RT_DISP(FEEPROM, EFUSE_READ_ALL, ("Offset-%X Worden=%X\n", offset, wden)); */

			for (i = 0; i < EFUSE_MAX_WORD_UNIT; i++) {
				/* Check word enable condition in the section */
				if (!(wden & (0x01 << i))) {
					efuse_OneByteRead(rtlpriv, eFuse_Addr++, &efuseData);
					/* RT_DISP(FEEPROM, EFUSE_READ_ALL, ("efuse[%X]=%X\n", eFuse_Addr-1, efuseData)); */
					efuse_utilized++;
					eFuseWord[offset][i] = (efuseData & 0xff);

					if (!AVAILABLE_EFUSE_ADDR_8812(eFuse_Addr))
						break;

					efuse_OneByteRead(rtlpriv, eFuse_Addr++, &efuseData);
					/* RT_DISP(FEEPROM, EFUSE_READ_ALL, ("efuse[%X]=%X\n", eFuse_Addr-1, efuseData)); */
					efuse_utilized++;
					eFuseWord[offset][i] |= (((u16)efuseData << 8) & 0xff00);

					if (!AVAILABLE_EFUSE_ADDR_8812(eFuse_Addr))
						break;
				}
			}
		}

		/* Read next PG header */
		efuse_OneByteRead(rtlpriv, eFuse_Addr++, &efuseHeader);
		/* RTPRINT(FEEPROM, EFUSE_READ_ALL, ("Addr=%d rtemp 0x%x\n", eFuse_Addr, *rtemp8)); */

		if (efuseHeader != 0xFF) {
			efuse_utilized++;
		}
	}

	/*
	 * 3. Collect 16 sections and 4 word unit into Efuse map.
	 */
	for (i = 0; i < EFUSE_MAX_SECTION_JAGUAR; i++) {
		for (j = 0; j < EFUSE_MAX_WORD_UNIT; j++) {
			efuseTbl[(i*8)+(j*2)] = (eFuseWord[i][j] & 0xff);
			efuseTbl[(i*8)+((j*2)+1)] = ((eFuseWord[i][j] >> 8) & 0xff);
		}
	}

	/* RT_DISP(FEEPROM, EFUSE_READ_ALL, ("Hal_EfuseReadEFuse8812A(): efuse_utilized: %d\n", efuse_utilized)); */

	/*
	 * 4. Copy from Efuse map to output pointer memory!!!
	 */
	for (i = 0; i < _size_byte; i++) {
		pbuf[i] = efuseTbl[_offset+i];
	}

	/*
	 * 5. Calculate Efuse utilization.
	 */
	efuse_usage = (u8)((eFuse_Addr*100)/EFUSE_REAL_CONTENT_LEN_JAGUAR);
	rtw_hal_set_hwreg(rtlpriv, HW_VAR_EFUSE_BYTES, (uint8_t *)&eFuse_Addr);

exit:
	if (efuseTbl)
		rtw_mfree(efuseTbl);

	if (eFuseWord)
		rtw_mfree2d((void *)eFuseWord, EFUSE_MAX_SECTION_JAGUAR, EFUSE_MAX_WORD_UNIT, sizeof(u16));
}



/* Do not support BT */
static void EFUSEGetEfuseDefinition(struct rtl_priv *rtlpriv,
	u8 type, void *pOut)
{
	switch (type) {
	case TYPE_EFUSE_MAX_SECTION:
		{
			uint8_t *pMax_section;
			pMax_section = (uint8_t *) pOut;
			*pMax_section = EFUSE_MAX_SECTION_JAGUAR;
		}
		break;
	case TYPE_EFUSE_REAL_CONTENT_LEN:
		{
			u16 *pu2Tmp;
			pu2Tmp = (u16 *) pOut;
			*pu2Tmp = EFUSE_REAL_CONTENT_LEN_JAGUAR;
		}
		break;
	case TYPE_EFUSE_CONTENT_LEN_BANK:
		{
			u16 *pu2Tmp;
			pu2Tmp = (u16 *) pOut;
			*pu2Tmp = EFUSE_REAL_CONTENT_LEN_JAGUAR;
		}
		break;
	case TYPE_AVAILABLE_EFUSE_BYTES_BANK:
		{
			u16 *pu2Tmp;
			pu2Tmp = (u16 *) pOut;
			*pu2Tmp = (u16) (EFUSE_REAL_CONTENT_LEN_JAGUAR-EFUSE_OOB_PROTECT_BYTES_JAGUAR);
		}
		break;
	case TYPE_AVAILABLE_EFUSE_BYTES_TOTAL:
		{
			u16 *pu2Tmp;
			pu2Tmp = (u16 *) pOut;
			*pu2Tmp = (u16) (EFUSE_REAL_CONTENT_LEN_JAGUAR-EFUSE_OOB_PROTECT_BYTES_JAGUAR);
		}
		break;
	case TYPE_EFUSE_MAP_LEN:
		{
			u16 *pu2Tmp;
			pu2Tmp = (u16 *) pOut;
			*pu2Tmp = (u16) EFUSE_MAP_LEN_JAGUAR;
		}
		break;
	case TYPE_EFUSE_PROTECT_BYTES_BANK:
		{
			uint8_t *pu1Tmp;
			pu1Tmp = (uint8_t *) pOut;
			*pu1Tmp = (uint8_t) (EFUSE_OOB_PROTECT_BYTES_JAGUAR);
		}
		break;
	default:
		{
			uint8_t *pu1Tmp;
			pu1Tmp = (uint8_t *) pOut;
			*pu1Tmp = 0;
		}
		break;
	}
}


static void efuse_read_all_map(struct rtl_priv *rtlpriv, uint8_t *Efuse)
{
	u16	mapLen=0;

	efuse_power_switch(rtlpriv, false, true);

	EFUSEGetEfuseDefinition(rtlpriv, TYPE_EFUSE_MAP_LEN, (void *)&mapLen);

	read_efuse(rtlpriv, 0, mapLen, Efuse);

	efuse_power_switch(rtlpriv, false, false);
}

uint8_t rtw_efuse_map_read(struct rtl_priv *rtlpriv, u16 addr, u16 cnts, uint8_t *data)
{
	u16	mapLen=0;

	EFUSEGetEfuseDefinition(rtlpriv, TYPE_EFUSE_MAP_LEN, (void *)&mapLen);

	if ((addr + cnts) > mapLen)
		return _FAIL;

	efuse_power_switch(rtlpriv, false, true);

	read_efuse(rtlpriv, addr, cnts, data);

	efuse_power_switch(rtlpriv, false, false);

	return _SUCCESS;
}

void EFUSE_ShadowMapUpdate(struct rtl_priv *rtlpriv)
{
	struct rtl_efuse *efuse = rtl_efuse(rtlpriv);
	u16	mapLen=0;

	EFUSEGetEfuseDefinition(rtlpriv, TYPE_EFUSE_MAP_LEN, (void *)&mapLen);

	if (efuse->autoload_failflag == _TRUE)
		memset(&efuse->efuse_map[0][0], 0xFF, mapLen);
	else
		efuse_read_all_map(rtlpriv, efuse->efuse_map[0]);

	//PlatformMoveMemory((void *)&pHalData->EfuseMap[EFUSE_MODIFY_MAP][0],
	//(void *)&pHalData->EfuseMap[EFUSE_INIT_MAP][0], mapLen);
}// EFUSE_ShadowMapUpdate

uint8_t
EFUSE_Read1Byte(
		struct rtl_priv *rtlpriv,
		u16		Address)
{
	uint8_t	data;
	uint8_t	Bytetemp = {0x00};
	uint8_t	temp = {0x00};
	uint32_t	k=0;
	u16	contentLen=0;

	EFUSEGetEfuseDefinition(rtlpriv, TYPE_EFUSE_REAL_CONTENT_LEN, (void *)&contentLen);

	if (Address < contentLen)	//E-fuse 512Byte
	{
		//Write E-fuse Register address bit0~7
		temp = Address & 0xFF;
		rtl_write_byte(rtlpriv, rtlpriv->cfg->maps[EFUSE_CTRL]+1, temp);
		Bytetemp = rtl_read_byte(rtlpriv, rtlpriv->cfg->maps[EFUSE_CTRL]+2);
		//Write E-fuse Register address bit8~9
		temp = ((Address >> 8) & 0x03) | (Bytetemp & 0xFC);
		rtl_write_byte(rtlpriv, rtlpriv->cfg->maps[EFUSE_CTRL]+2, temp);

		//Write 0x30[31]=0
		Bytetemp = rtl_read_byte(rtlpriv, rtlpriv->cfg->maps[EFUSE_CTRL]+3);
		temp = Bytetemp & 0x7F;
		rtl_write_byte(rtlpriv, rtlpriv->cfg->maps[EFUSE_CTRL]+3, temp);

		//Wait Write-ready (0x30[31]=1)
		Bytetemp = rtl_read_byte(rtlpriv, rtlpriv->cfg->maps[EFUSE_CTRL]+3);
		while(!(Bytetemp & 0x80))
		{
			Bytetemp = rtl_read_byte(rtlpriv, rtlpriv->cfg->maps[EFUSE_CTRL]+3);
			k++;
			if(k==1000)
			{
				k=0;
				break;
			}
		}
		data=rtl_read_byte(rtlpriv, rtlpriv->cfg->maps[EFUSE_CTRL]);
		return data;
	}
	else
		return 0xFF;

}/* EFUSE_Read1Byte */

void
EFUSE_Write1Byte(
		struct rtl_priv *rtlpriv,
		u16		Address,
		uint8_t		Value)
{
	uint8_t	Bytetemp = {0x00};
	uint8_t	temp = {0x00};
	uint32_t	k=0;
	u16	contentLen=0;

	EFUSEGetEfuseDefinition(rtlpriv, TYPE_EFUSE_REAL_CONTENT_LEN, (void *)&contentLen);

	if( Address < contentLen)	//E-fuse 512Byte
	{
		rtl_write_byte(rtlpriv, rtlpriv->cfg->maps[EFUSE_CTRL], Value);

		//Write E-fuse Register address bit0~7
		temp = Address & 0xFF;
		rtl_write_byte(rtlpriv, rtlpriv->cfg->maps[EFUSE_CTRL]+1, temp);
		Bytetemp = rtl_read_byte(rtlpriv, rtlpriv->cfg->maps[EFUSE_CTRL]+2);

		//Write E-fuse Register address bit8~9
		temp = ((Address >> 8) & 0x03) | (Bytetemp & 0xFC);
		rtl_write_byte(rtlpriv, rtlpriv->cfg->maps[EFUSE_CTRL]+2, temp);

		//Write 0x30[31]=1
		Bytetemp = rtl_read_byte(rtlpriv, rtlpriv->cfg->maps[EFUSE_CTRL]+3);
		temp = Bytetemp | 0x80;
		rtl_write_byte(rtlpriv, rtlpriv->cfg->maps[EFUSE_CTRL]+3, temp);

		//Wait Write-ready (0x30[31]=0)
		Bytetemp = rtl_read_byte(rtlpriv, rtlpriv->cfg->maps[EFUSE_CTRL]+3);
		while(Bytetemp & 0x80)
		{
			Bytetemp = rtl_read_byte(rtlpriv, rtlpriv->cfg->maps[EFUSE_CTRL]+3);
			k++;
			if(k==100)
			{
				k=0;
				break;
			}
		}
	}
}/* EFUSE_Write1Byte */

//------------------------------------------------------------------------------
u16 efuse_GetMaxSize(struct rtl_priv *rtlpriv)
{
	u16	max_size;
	EFUSEGetEfuseDefinition(rtlpriv, TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, (void *)&max_size);
	return max_size;
}
