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
#define _RTW_EFUSE_C_

#include <drv_types.h>
#include <rtw_debug.h>


//------------------------------------------------------------------------------
#define REG_EFUSE_CTRL		0x0030
#define EFUSE_CTRL			REG_EFUSE_CTRL		// E-Fuse Control.
//------------------------------------------------------------------------------


/*  11/16/2008 MH Add description. Get current efuse area enabled word!!. */
uint8_t
Efuse_CalculateWordCnts( uint8_t	word_en)
{
	uint8_t word_cnts = 0;
	if(!(word_en & BIT(0)))	word_cnts++; // 0 : write enable
	if(!(word_en & BIT(1)))	word_cnts++;
	if(!(word_en & BIT(2)))	word_cnts++;
	if(!(word_en & BIT(3)))	word_cnts++;
	return word_cnts;
}

//
//	Description:
//		Execute E-Fuse read byte operation.
//		Refered from SD1 Richard.
//
//	Assumption:
//		1. Boot from E-Fuse and successfully auto-load.
//		2. PASSIVE_LEVEL (USB interface)
//
//	Created by Roger, 2008.10.21.
//
void
ReadEFuseByte(
		struct rtl_priv *rtlpriv,
		u16 			_offset,
		uint8_t 			*pbuf)
{
	uint32_t	value32;
	uint8_t	readbyte;
	u16	retry;
	//uint32_t start=jiffies;

	//Write Address
	rtl_write_byte(rtlpriv, EFUSE_CTRL+1, (_offset & 0xff));
	readbyte = rtl_read_byte(rtlpriv, EFUSE_CTRL+2);
	rtl_write_byte(rtlpriv, EFUSE_CTRL+2, ((_offset >> 8) & 0x03) | (readbyte & 0xfc));

	//Write bit 32 0
	readbyte = rtl_read_byte(rtlpriv, EFUSE_CTRL+3);
	rtl_write_byte(rtlpriv, EFUSE_CTRL+3, (readbyte & 0x7f));

	//Check bit 32 read-ready
	retry = 0;
	value32 = rtl_read_dword(rtlpriv, EFUSE_CTRL);
	//while(!(((value32 >> 24) & 0xff) & 0x80)  && (retry<10))
	while(!(((value32 >> 24) & 0xff) & 0x80)  && (retry<10000))
	{
		value32 = rtl_read_dword(rtlpriv, EFUSE_CTRL);
		retry++;
	}

	// 20100205 Joseph: Add delay suggested by SD1 Victor.
	// This fix the problem that Efuse read error in high temperature condition.
	// Designer says that there shall be some delay after ready bit is set, or the
	// result will always stay on last data we read.
	udelay(50);
	value32 = rtl_read_dword(rtlpriv, EFUSE_CTRL);

	*pbuf = (uint8_t)(value32 & 0xff);
	//DBG_871X("ReadEFuseByte _offset:%08u, in %d ms\n",_offset ,rtw_get_passing_time_ms(start));

}

/*-----------------------------------------------------------------------------
 * Function:	EFUSE_Read1Byte
 *
 * Overview:	Copy from WMAC fot EFUSE read 1 byte.
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 09/23/2008 	MHC		Copy from WMAC.
 *
 *---------------------------------------------------------------------------*/
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

	rtlpriv->cfg->ops->EFUSEGetEfuseDefinition(rtlpriv, EFUSE_WIFI , TYPE_EFUSE_REAL_CONTENT_LEN, (void *)&contentLen);

	if (Address < contentLen)	//E-fuse 512Byte
	{
		//Write E-fuse Register address bit0~7
		temp = Address & 0xFF;
		rtl_write_byte(rtlpriv, EFUSE_CTRL+1, temp);
		Bytetemp = rtl_read_byte(rtlpriv, EFUSE_CTRL+2);
		//Write E-fuse Register address bit8~9
		temp = ((Address >> 8) & 0x03) | (Bytetemp & 0xFC);
		rtl_write_byte(rtlpriv, EFUSE_CTRL+2, temp);

		//Write 0x30[31]=0
		Bytetemp = rtl_read_byte(rtlpriv, EFUSE_CTRL+3);
		temp = Bytetemp & 0x7F;
		rtl_write_byte(rtlpriv, EFUSE_CTRL+3, temp);

		//Wait Write-ready (0x30[31]=1)
		Bytetemp = rtl_read_byte(rtlpriv, EFUSE_CTRL+3);
		while(!(Bytetemp & 0x80))
		{
			Bytetemp = rtl_read_byte(rtlpriv, EFUSE_CTRL+3);
			k++;
			if(k==1000)
			{
				k=0;
				break;
			}
		}
		data=rtl_read_byte(rtlpriv, EFUSE_CTRL);
		return data;
	}
	else
		return 0xFF;

}/* EFUSE_Read1Byte */

/*-----------------------------------------------------------------------------
 * Function:	EFUSE_Write1Byte
 *
 * Overview:	Copy from WMAC fot EFUSE write 1 byte.
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 09/23/2008 	MHC		Copy from WMAC.
 *
 *---------------------------------------------------------------------------*/

void
EFUSE_Write1Byte(
		struct rtl_priv *rtlpriv,
		u16		Address,
		uint8_t		Value);
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

	rtlpriv->cfg->ops->EFUSEGetEfuseDefinition(rtlpriv, EFUSE_WIFI , TYPE_EFUSE_REAL_CONTENT_LEN, (void *)&contentLen);

	if( Address < contentLen)	//E-fuse 512Byte
	{
		rtl_write_byte(rtlpriv, EFUSE_CTRL, Value);

		//Write E-fuse Register address bit0~7
		temp = Address & 0xFF;
		rtl_write_byte(rtlpriv, EFUSE_CTRL+1, temp);
		Bytetemp = rtl_read_byte(rtlpriv, EFUSE_CTRL+2);

		//Write E-fuse Register address bit8~9
		temp = ((Address >> 8) & 0x03) | (Bytetemp & 0xFC);
		rtl_write_byte(rtlpriv, EFUSE_CTRL+2, temp);

		//Write 0x30[31]=1
		Bytetemp = rtl_read_byte(rtlpriv, EFUSE_CTRL+3);
		temp = Bytetemp | 0x80;
		rtl_write_byte(rtlpriv, EFUSE_CTRL+3, temp);

		//Wait Write-ready (0x30[31]=0)
		Bytetemp = rtl_read_byte(rtlpriv, EFUSE_CTRL+3);
		while(Bytetemp & 0x80)
		{
			Bytetemp = rtl_read_byte(rtlpriv, EFUSE_CTRL+3);
			k++;
			if(k==100)
			{
				k=0;
				break;
			}
		}
	}
}/* EFUSE_Write1Byte */


/*
 *
 * ULLI need this rtlwifi-lib
 * u8 efuse_read_1byte(struct ieee80211_hw *hw, u16 address)
 */

/*  11/16/2008 MH Read one byte from real Efuse. */
uint8_t
efuse_OneByteRead(
		struct rtl_priv *rtlpriv,
		u16			addr,
		uint8_t			*data)
{
	uint8_t	tmpidx = 0;
	uint8_t	bResult;

	//DBG_871X("===> EFUSE_OneByteRead(), addr = %x\n", addr);
	//DBG_871X("===> EFUSE_OneByteRead() start, 0x34 = 0x%X\n", rtl_read_dword(rtlpriv, EFUSE_TEST));

	// -----------------e-fuse reg ctrl ---------------------------------
	//address
	rtl_write_byte(rtlpriv, EFUSE_CTRL+1, (uint8_t)(addr&0xff));
	rtl_write_byte(rtlpriv, EFUSE_CTRL+2, ((uint8_t)((addr>>8) &0x03) ) |
	(rtl_read_byte(rtlpriv, EFUSE_CTRL+2)&0xFC ));

	rtl_write_byte(rtlpriv, EFUSE_CTRL+3,  0x72);//read cmd

	while(!(0x80 &rtl_read_byte(rtlpriv, EFUSE_CTRL+3))&&(tmpidx<1000))
	{
		mdelay(1);
		tmpidx++;
	}
	if(tmpidx<1000)
	{
		*data=rtl_read_byte(rtlpriv, EFUSE_CTRL);
		bResult = _TRUE;
	}
	else
	{
		*data = 0xff;
		bResult = 0;
	}

	return bResult;
}


/*
 * ULLI need this for rtlwifi-lib
 * void efuse_write_1byte(struct ieee80211_hw *hw, u16 address, u8 value)
 */
/*  11/16/2008 MH Write one byte to reald Efuse. */
uint8_t
efuse_OneByteWrite(
		struct rtl_priv *rtlpriv,
		u16			addr,
		uint8_t			data)
{
	uint8_t	tmpidx = 0;
	uint8_t	bResult= 0;
	uint32_t efuseValue = 0;

	//DBG_871X("===> EFUSE_OneByteWrite(), addr = %x data=%x\n", addr, data);
	//DBG_871X("===> EFUSE_OneByteWrite() start, 0x34 = 0x%X\n", rtl_read_dword(rtlpriv, EFUSE_TEST));

	// -----------------e-fuse reg ctrl ---------------------------------
	//address


	efuseValue = rtl_read_dword(rtlpriv, EFUSE_CTRL);
	efuseValue |= (BIT21|BIT31);
	efuseValue &= ~(0x3FFFF);
	efuseValue |= ((addr<<8 | data) & 0x3FFFF);

	{
		rtl_write_dword(rtlpriv, EFUSE_CTRL, efuseValue);
	}

	while((0x80 &  rtl_read_byte(rtlpriv, EFUSE_CTRL+3)) && (tmpidx<100) ){
		mdelay(1);
		tmpidx++;
	}

	if(tmpidx<100)
	{
		bResult = _TRUE;
	}
	else
	{
		bResult = 0;
	}

	return bResult;
}

int
Efuse_PgPacketRead(		struct rtl_priv *rtlpriv,
						uint8_t			offset,
						uint8_t			*data)
{
	int	ret=0;

	ret =  rtlpriv->cfg->ops->Efuse_PgPacketRead(rtlpriv, offset, data);

	return ret;
}

int
Efuse_PgPacketWrite(	struct rtl_priv *rtlpriv,
						uint8_t 			offset,
						uint8_t			word_en,
						uint8_t			*data)
{
	int ret;

	ret =  rtlpriv->cfg->ops->Efuse_PgPacketWrite(rtlpriv, offset, word_en, data);

	return ret;
}


int
Efuse_PgPacketWrite_BT(	struct rtl_priv *rtlpriv,
						uint8_t 			offset,
						uint8_t			word_en,
						uint8_t			*data)
{
	int ret;

	ret =  rtlpriv->cfg->ops->Efuse_PgPacketWrite_BT(rtlpriv, offset, word_en, data);

	return ret;
}

/*-----------------------------------------------------------------------------
 * Function:	efuse_WordEnableDataRead
 *
 * Overview:	Read allowed word in current efuse section data.
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/16/2008 	MHC		Create Version 0.
 * 11/21/2008 	MHC		Fix Write bug when we only enable late word.
 *
 *---------------------------------------------------------------------------*/
void
efuse_WordEnableDataRead(	uint8_t	word_en,
								uint8_t	*sourdata,
								uint8_t	*targetdata)
{
	if (!(word_en&BIT(0)))
	{
		targetdata[0] = sourdata[0];
		targetdata[1] = sourdata[1];
	}
	if (!(word_en&BIT(1)))
	{
		targetdata[2] = sourdata[2];
		targetdata[3] = sourdata[3];
	}
	if (!(word_en&BIT(2)))
	{
		targetdata[4] = sourdata[4];
		targetdata[5] = sourdata[5];
	}
	if (!(word_en&BIT(3)))
	{
		targetdata[6] = sourdata[6];
		targetdata[7] = sourdata[7];
	}
}


uint8_t
Efuse_WordEnableDataWrite(		struct rtl_priv *rtlpriv,
								u16		efuse_addr,
								uint8_t		word_en,
								uint8_t		*data)
{
	uint8_t	ret=0;

	ret =  rtlpriv->cfg->ops->Efuse_WordEnableDataWrite(rtlpriv, efuse_addr, word_en, data);

	return ret;
}
//------------------------------------------------------------------------------
u16 efuse_GetMaxSize(struct rtl_priv *rtlpriv)
{
	u16	max_size;
	rtlpriv->cfg->ops->EFUSEGetEfuseDefinition(rtlpriv, EFUSE_WIFI , TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, (void *)&max_size);
	return max_size;
}
//------------------------------------------------------------------------------
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

uint8_t rtw_BT_efuse_map_read(struct rtl_priv *rtlpriv, u16 addr, u16 cnts, uint8_t *data)
{
	u16	mapLen=0;

	rtlpriv->cfg->ops->EFUSEGetEfuseDefinition(rtlpriv, EFUSE_BT, TYPE_EFUSE_MAP_LEN, (void *)&mapLen);

	if ((addr + cnts) > mapLen)
		return _FAIL;

	rtlpriv->cfg->ops->EfusePowerSwitch(rtlpriv, _FALSE, _TRUE);

	rtlpriv->cfg->ops->ReadEFuse(rtlpriv, EFUSE_BT, addr, cnts, data);

	rtlpriv->cfg->ops->EfusePowerSwitch(rtlpriv, _FALSE, _FALSE);

	return _SUCCESS;
}
//------------------------------------------------------------------------------
uint8_t rtw_efuse_map_write(struct rtl_priv *rtlpriv, u16 addr, u16 cnts, uint8_t *data)
{
	uint8_t	offset, word_en;
	uint8_t	*map;
	uint8_t	newdata[PGPKT_DATA_SIZE];
	int32_t	i, j, idx;
	uint8_t	ret = _SUCCESS;
	u16	mapLen=0;

	rtlpriv->cfg->ops->EFUSEGetEfuseDefinition(rtlpriv, EFUSE_WIFI, TYPE_EFUSE_MAP_LEN, (void *)&mapLen);

	if ((addr + cnts) > mapLen)
		return _FAIL;

	map = rtw_zmalloc(mapLen);
	if(map == NULL){
		return _FAIL;
	}

	ret = rtw_efuse_map_read(rtlpriv, 0, mapLen, map);
	if (ret == _FAIL) goto exit;

	rtlpriv->cfg->ops->EfusePowerSwitch(rtlpriv, _TRUE, _TRUE);

	offset = (addr >> 3);
	word_en = 0xF;
	memset(newdata, 0xFF, PGPKT_DATA_SIZE);
	i = addr & 0x7;	// index of one package
	j = 0;		// index of new package
	idx = 0;	// data index

	if (i & 0x1) {
		// odd start
		if (data[idx] != map[addr+idx]) {
			word_en &= ~BIT(i >> 1);
			newdata[i-1] = map[addr+idx-1];
			newdata[i] = data[idx];
		}
		i++;
		idx++;
	}
	do {
		for (; i < PGPKT_DATA_SIZE; i += 2)
		{
			if (cnts == idx) break;
			if ((cnts - idx) == 1) {
				if (data[idx] != map[addr+idx]) {
					word_en &= ~BIT(i >> 1);
					newdata[i] = data[idx];
					newdata[i+1] = map[addr+idx+1];
				}
				idx++;
				break;
			} else {
				if ((data[idx] != map[addr+idx]) ||
				    (data[idx+1] != map[addr+idx+1]))
				{
					word_en &= ~BIT(i >> 1);
					newdata[i] = data[idx];
					newdata[i+1] = data[idx + 1];
				}
				idx += 2;
			}
			if (idx == cnts) break;
		}

		if (word_en != 0xF) {
			ret = Efuse_PgPacketWrite(rtlpriv, offset, word_en, newdata);
			DBG_871X("offset=%x \n",offset);
			DBG_871X("word_en=%x \n",word_en);

			for(i=0;i<PGPKT_DATA_SIZE;i++)
			{
				DBG_871X("data=%x \t",newdata[i]);
			}
			if (ret == _FAIL) break;
		}

		if (idx == cnts) break;

		offset++;
		i = 0;
		j = 0;
		word_en = 0xF;
		memset(newdata, 0xFF, PGPKT_DATA_SIZE);
	} while (1);

	rtlpriv->cfg->ops->EfusePowerSwitch(rtlpriv, _TRUE, _FALSE);

exit:
	/* ULLI check usage of mapLen */
	rtw_mfree(map);

	return ret;
}


//------------------------------------------------------------------------------
uint8_t rtw_BT_efuse_map_write(struct rtl_priv *rtlpriv, u16 addr, u16 cnts, uint8_t *data)
{
	uint8_t	offset, word_en;
	uint8_t	*map;
	uint8_t	newdata[PGPKT_DATA_SIZE];
	int32_t	i, j, idx;
	uint8_t	ret = _SUCCESS;
	u16	mapLen=0;

	rtlpriv->cfg->ops->EFUSEGetEfuseDefinition(rtlpriv, EFUSE_BT, TYPE_EFUSE_MAP_LEN, (void *)&mapLen);

	if ((addr + cnts) > mapLen)
		return _FAIL;

	map = rtw_zmalloc(mapLen);
	if(map == NULL){
		return _FAIL;
	}

	ret = rtw_BT_efuse_map_read(rtlpriv, 0, mapLen, map);
	if (ret == _FAIL) goto exit;

	rtlpriv->cfg->ops->EfusePowerSwitch(rtlpriv, _TRUE, _TRUE);

	offset = (addr >> 3);
	word_en = 0xF;
	memset(newdata, 0xFF, PGPKT_DATA_SIZE);
	i = addr & 0x7;	// index of one package
	j = 0;		// index of new package
	idx = 0;	// data index

	if (i & 0x1) {
		// odd start
		if (data[idx] != map[addr+idx]) {
			word_en &= ~BIT(i >> 1);
			newdata[i-1] = map[addr+idx-1];
			newdata[i] = data[idx];
		}
		i++;
		idx++;
	}
	do {
		for (; i < PGPKT_DATA_SIZE; i += 2)
		{
			if (cnts == idx) break;
			if ((cnts - idx) == 1) {
				if (data[idx] != map[addr+idx]) {
					word_en &= ~BIT(i >> 1);
					newdata[i] = data[idx];
					newdata[i+1] = map[addr+idx+1];
				}
				idx++;
				break;
			} else {
				if ((data[idx] != map[addr+idx]) ||
				    (data[idx+1] != map[addr+idx+1]))
				{
					word_en &= ~BIT(i >> 1);
					newdata[i] = data[idx];
					newdata[i+1] = data[idx + 1];
				}
				idx += 2;
			}
			if (idx == cnts) break;
		}

		if (word_en != 0xF)
		{
			DBG_871X("%s: offset=%#X\n", __FUNCTION__, offset);
			DBG_871X("%s: word_en=%#X\n", __FUNCTION__, word_en);
			DBG_871X("%s: data=", __FUNCTION__);
			for (i=0; i<PGPKT_DATA_SIZE; i++)
			{
				DBG_871X("0x%02X ", newdata[i]);
			}
			DBG_871X("\n");

			ret = Efuse_PgPacketWrite_BT(rtlpriv, offset, word_en, newdata);
			if (ret == _FAIL) break;
		}

		if (idx == cnts) break;

		offset++;
		i = 0;
		j = 0;
		word_en = 0xF;
		memset(newdata, 0xFF, PGPKT_DATA_SIZE);
	} while (1);

	rtlpriv->cfg->ops->EfusePowerSwitch(rtlpriv, _TRUE, _FALSE);

exit:
	/* ULLI check usage of mapLen */
	rtw_mfree(map);

	return ret;
}

/*-----------------------------------------------------------------------------
 * Function:	Efuse_ReadAllMap
 *
 * Overview:	Read All Efuse content
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/11/2008 	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
static void
Efuse_ReadAllMap(
			struct rtl_priv *rtlpriv,
			uint8_t		efuseType,
	 	uint8_t		*Efuse)
{
	u16	mapLen=0;

	rtlpriv->cfg->ops->EfusePowerSwitch(rtlpriv,_FALSE, _TRUE);

	rtlpriv->cfg->ops->EFUSEGetEfuseDefinition(rtlpriv, efuseType, TYPE_EFUSE_MAP_LEN, (void *)&mapLen);

	rtlpriv->cfg->ops->ReadEFuse(rtlpriv, efuseType, 0, mapLen, Efuse);

	rtlpriv->cfg->ops->EfusePowerSwitch(rtlpriv,_FALSE, _FALSE);
}

/*-----------------------------------------------------------------------------
 * Function:	efuse_ShadowRead1Byte
 *			efuse_ShadowRead2Byte
 *			efuse_ShadowRead4Byte
 *
 * Overview:	Read from efuse init map by one/two/four bytes !!!!!
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/12/2008 	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
static void
efuse_ShadowRead1Byte(
		struct rtl_priv *rtlpriv,
		u16		Offset,
	 	uint8_t		*Value)
{
	EEPROM_EFUSE_PRIV *pEEPROM = GET_EEPROM_EFUSE_PRIV(rtlpriv);

	*Value = pEEPROM->efuse_eeprom_data[Offset];

}	// EFUSE_ShadowRead1Byte

//---------------Read Two Bytes
static void
efuse_ShadowRead2Byte(
		struct rtl_priv *rtlpriv,
		u16		Offset,
	 	u16		*Value)
{
	EEPROM_EFUSE_PRIV *pEEPROM = GET_EEPROM_EFUSE_PRIV(rtlpriv);

	*Value = pEEPROM->efuse_eeprom_data[Offset];
	*Value |= pEEPROM->efuse_eeprom_data[Offset+1]<<8;

}	// EFUSE_ShadowRead2Byte

//---------------Read Four Bytes
static void
efuse_ShadowRead4Byte(
		struct rtl_priv *rtlpriv,
		u16		Offset,
	 	uint32_t		*Value)
{
	EEPROM_EFUSE_PRIV *pEEPROM = GET_EEPROM_EFUSE_PRIV(rtlpriv);

	*Value = pEEPROM->efuse_eeprom_data[Offset];
	*Value |= pEEPROM->efuse_eeprom_data[Offset+1]<<8;
	*Value |= pEEPROM->efuse_eeprom_data[Offset+2]<<16;
	*Value |= pEEPROM->efuse_eeprom_data[Offset+3]<<24;

}	// efuse_ShadowRead4Byte


/*-----------------------------------------------------------------------------
 * Function:	efuse_ShadowWrite1Byte
 *			efuse_ShadowWrite2Byte
 *			efuse_ShadowWrite4Byte
 *
 * Overview:	Write efuse modify map by one/two/four byte.
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/12/2008 	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
#ifdef PLATFORM
static void
efuse_ShadowWrite1Byte(
		struct rtl_priv *rtlpriv,
		u16		Offset,
	 	uint8_t		Value);
#endif //PLATFORM
static void
efuse_ShadowWrite1Byte(
		struct rtl_priv *rtlpriv,
		u16		Offset,
	 	uint8_t		Value)
{
	EEPROM_EFUSE_PRIV *pEEPROM = GET_EEPROM_EFUSE_PRIV(rtlpriv);

	pEEPROM->efuse_eeprom_data[Offset] = Value;

}	// efuse_ShadowWrite1Byte

//---------------Write Two Bytes
static void
efuse_ShadowWrite2Byte(
		struct rtl_priv *rtlpriv,
		u16		Offset,
	 	u16		Value)
{
	EEPROM_EFUSE_PRIV *pEEPROM = GET_EEPROM_EFUSE_PRIV(rtlpriv);

	pEEPROM->efuse_eeprom_data[Offset] = Value&0x00FF;
	pEEPROM->efuse_eeprom_data[Offset+1] = Value>>8;

}	// efuse_ShadowWrite1Byte

//---------------Write Four Bytes
static void
efuse_ShadowWrite4Byte(
		struct rtl_priv *rtlpriv,
		u16		Offset,
		uint32_t		Value)
{
	EEPROM_EFUSE_PRIV *pEEPROM = GET_EEPROM_EFUSE_PRIV(rtlpriv);

	pEEPROM->efuse_eeprom_data[Offset] = (uint8_t)(Value&0x000000FF);
	pEEPROM->efuse_eeprom_data[Offset+1] = (uint8_t)((Value>>8)&0x0000FF);
	pEEPROM->efuse_eeprom_data[Offset+2] = (uint8_t)((Value>>16)&0x00FF);
	pEEPROM->efuse_eeprom_data[Offset+3] = (uint8_t)((Value>>24)&0xFF);

}	// efuse_ShadowWrite1Byte

/*-----------------------------------------------------------------------------
 * Function:	EFUSE_ShadowMapUpdate
 *
 * Overview:	Transfer current EFUSE content to shadow init and modify map.
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/13/2008 	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
void EFUSE_ShadowMapUpdate(
	 struct rtl_priv *rtlpriv,
	 uint8_t		efuseType)
{
	struct rtl_efuse *efuse = rtl_efuse(rtlpriv);
	EEPROM_EFUSE_PRIV *pEEPROM = GET_EEPROM_EFUSE_PRIV(rtlpriv);
	u16	mapLen=0;

	rtlpriv->cfg->ops->EFUSEGetEfuseDefinition(rtlpriv, efuseType, TYPE_EFUSE_MAP_LEN, (void *)&mapLen);

	if (efuse->autoload_failflag == _TRUE)
	{
		memset(pEEPROM->efuse_eeprom_data, 0xFF, mapLen);
	}
	else
	{
		Efuse_ReadAllMap(rtlpriv, efuseType, pEEPROM->efuse_eeprom_data);

	}

	//PlatformMoveMemory((void *)&pHalData->EfuseMap[EFUSE_MODIFY_MAP][0],
	//(void *)&pHalData->EfuseMap[EFUSE_INIT_MAP][0], mapLen);
}// EFUSE_ShadowMapUpdate


/*-----------------------------------------------------------------------------
 * Function:	EFUSE_ShadowRead
 *
 * Overview:	Read from efuse init map !!!!!
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/12/2008 	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
void
EFUSE_ShadowRead(
			struct rtl_priv *rtlpriv,
			uint8_t		Type,
			u16		Offset,
	 	uint32_t		*Value	)
{
	if (Type == 1)
		efuse_ShadowRead1Byte(rtlpriv, Offset, (uint8_t *)Value);
	else if (Type == 2)
		efuse_ShadowRead2Byte(rtlpriv, Offset, (u16 *)Value);
	else if (Type == 4)
		efuse_ShadowRead4Byte(rtlpriv, Offset, (uint32_t *)Value);

}	// EFUSE_ShadowRead

