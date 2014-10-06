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



/*------------------------Define local variable------------------------------*/
uint8_t	fakeEfuseBank=0;
uint32_t	fakeEfuseUsedBytes=0;
uint8_t	fakeEfuseContent[EFUSE_MAX_HW_SIZE]={0};
uint8_t	fakeEfuseInitMap[EFUSE_MAX_MAP_LEN]={0};
uint8_t	fakeEfuseModifiedMap[EFUSE_MAX_MAP_LEN]={0};

uint32_t	BTEfuseUsedBytes=0;
uint8_t	BTEfuseContent[EFUSE_MAX_BT_BANK][EFUSE_MAX_HW_SIZE];
uint8_t	BTEfuseInitMap[EFUSE_BT_MAX_MAP_LEN]={0};
uint8_t	BTEfuseModifiedMap[EFUSE_BT_MAX_MAP_LEN]={0};

uint32_t	fakeBTEfuseUsedBytes=0;
uint8_t	fakeBTEfuseContent[EFUSE_MAX_BT_BANK][EFUSE_MAX_HW_SIZE];
uint8_t	fakeBTEfuseInitMap[EFUSE_BT_MAX_MAP_LEN]={0};
uint8_t	fakeBTEfuseModifiedMap[EFUSE_BT_MAX_MAP_LEN]={0};
/*------------------------Define local variable------------------------------*/

//------------------------------------------------------------------------------
#define REG_EFUSE_CTRL		0x0030
#define EFUSE_CTRL			REG_EFUSE_CTRL		// E-Fuse Control.
//------------------------------------------------------------------------------


/*-----------------------------------------------------------------------------
 * Function:	efuse_GetCurrentSize
 *
 * Overview:	Get current efuse size!!!
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
 *
 *---------------------------------------------------------------------------*/
u16
Efuse_GetCurrentSize(
	 PADAPTER		padapter,
	 uint8_t			efuseType)
{
	uint16_t ret=0;

	ret = padapter->HalFunc->EfuseGetCurrentSize(padapter, efuseType);

	return ret;
}

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
VOID
ReadEFuseByte(
		PADAPTER	padapter,
		uint16_t 			_offset,
		uint8_t 			*pbuf)
{
	uint32_t	value32;
	uint8_t	readbyte;
	uint16_t	retry;
	//uint32_t start=rtw_get_current_time();

	//Write Address
	rtw_write8(padapter, EFUSE_CTRL+1, (_offset & 0xff));
	readbyte = rtw_read8(padapter, EFUSE_CTRL+2);
	rtw_write8(padapter, EFUSE_CTRL+2, ((_offset >> 8) & 0x03) | (readbyte & 0xfc));

	//Write bit 32 0
	readbyte = rtw_read8(padapter, EFUSE_CTRL+3);
	rtw_write8(padapter, EFUSE_CTRL+3, (readbyte & 0x7f));

	//Check bit 32 read-ready
	retry = 0;
	value32 = rtw_read32(padapter, EFUSE_CTRL);
	//while(!(((value32 >> 24) & 0xff) & 0x80)  && (retry<10))
	while(!(((value32 >> 24) & 0xff) & 0x80)  && (retry<10000))
	{
		value32 = rtw_read32(padapter, EFUSE_CTRL);
		retry++;
	}

	// 20100205 Joseph: Add delay suggested by SD1 Victor.
	// This fix the problem that Efuse read error in high temperature condition.
	// Designer says that there shall be some delay after ready bit is set, or the
	// result will always stay on last data we read.
	rtw_udelay_os(50);
	value32 = rtw_read32(padapter, EFUSE_CTRL);

	*pbuf = (uint8_t)(value32 & 0xff);
	//DBG_871X("ReadEFuseByte _offset:%08u, in %d ms\n",_offset ,rtw_get_passing_time_ms(start));

}


//
//	Description:
//		1. Execute E-Fuse read byte operation according as map offset and
//		    save to E-Fuse table.
//		2. Refered from SD1 Richard.
//
//	Assumption:
//		1. Boot from E-Fuse and successfully auto-load.
//		2. PASSIVE_LEVEL (USB interface)
//
//	Created by Roger, 2008.10.21.
//
//	2008/12/12 MH 	1. Reorganize code flow and reserve bytes. and add description.
//					2. Add efuse utilization collect.
//	2008/12/22 MH	Read Efuse must check if we write section 1 data again!!! Sec1
//					write addr must be after sec5.
//

static VOID
efuse_ReadEFuse(
	PADAPTER	padapter,
	uint8_t		efuseType,
	uint16_t		_offset,
	uint16_t 		_size_byte,
	uint8_t      	*pbuf
	)
{
	padapter->HalFunc->ReadEFuse(padapter, efuseType, _offset, _size_byte, pbuf);
}

VOID
EFUSE_GetEfuseDefinition(
			PADAPTER	padapter,
			uint8_t		efuseType,
			uint8_t		type,
			void		*pOut
	)
{
	padapter->HalFunc->EFUSEGetEfuseDefinition(padapter, efuseType, type, pOut);
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
		PADAPTER	padapter,
		uint16_t		Address)
{
	uint8_t	data;
	uint8_t	Bytetemp = {0x00};
	uint8_t	temp = {0x00};
	uint32_t	k=0;
	uint16_t	contentLen=0;

	EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI , TYPE_EFUSE_REAL_CONTENT_LEN, (PVOID)&contentLen);

	if (Address < contentLen)	//E-fuse 512Byte
	{
		//Write E-fuse Register address bit0~7
		temp = Address & 0xFF;
		rtw_write8(padapter, EFUSE_CTRL+1, temp);
		Bytetemp = rtw_read8(padapter, EFUSE_CTRL+2);
		//Write E-fuse Register address bit8~9
		temp = ((Address >> 8) & 0x03) | (Bytetemp & 0xFC);
		rtw_write8(padapter, EFUSE_CTRL+2, temp);

		//Write 0x30[31]=0
		Bytetemp = rtw_read8(padapter, EFUSE_CTRL+3);
		temp = Bytetemp & 0x7F;
		rtw_write8(padapter, EFUSE_CTRL+3, temp);

		//Wait Write-ready (0x30[31]=1)
		Bytetemp = rtw_read8(padapter, EFUSE_CTRL+3);
		while(!(Bytetemp & 0x80))
		{
			Bytetemp = rtw_read8(padapter, EFUSE_CTRL+3);
			k++;
			if(k==1000)
			{
				k=0;
				break;
			}
		}
		data=rtw_read8(padapter, EFUSE_CTRL);
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
		PADAPTER	padapter,
		uint16_t		Address,
		uint8_t		Value);
void
EFUSE_Write1Byte(
		PADAPTER	padapter,
		uint16_t		Address,
		uint8_t		Value)
{
	uint8_t	Bytetemp = {0x00};
	uint8_t	temp = {0x00};
	uint32_t	k=0;
	uint16_t	contentLen=0;

	//RT_TRACE(COMP_EFUSE, DBG_LOUD, ("Addr=%x Data =%x\n", Address, Value));
	EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI , TYPE_EFUSE_REAL_CONTENT_LEN, (PVOID)&contentLen);

	if( Address < contentLen)	//E-fuse 512Byte
	{
		rtw_write8(padapter, EFUSE_CTRL, Value);

		//Write E-fuse Register address bit0~7
		temp = Address & 0xFF;
		rtw_write8(padapter, EFUSE_CTRL+1, temp);
		Bytetemp = rtw_read8(padapter, EFUSE_CTRL+2);

		//Write E-fuse Register address bit8~9
		temp = ((Address >> 8) & 0x03) | (Bytetemp & 0xFC);
		rtw_write8(padapter, EFUSE_CTRL+2, temp);

		//Write 0x30[31]=1
		Bytetemp = rtw_read8(padapter, EFUSE_CTRL+3);
		temp = Bytetemp | 0x80;
		rtw_write8(padapter, EFUSE_CTRL+3, temp);

		//Wait Write-ready (0x30[31]=0)
		Bytetemp = rtw_read8(padapter, EFUSE_CTRL+3);
		while(Bytetemp & 0x80)
		{
			Bytetemp = rtw_read8(padapter, EFUSE_CTRL+3);
			k++;
			if(k==100)
			{
				k=0;
				break;
			}
		}
	}
}/* EFUSE_Write1Byte */


/*  11/16/2008 MH Read one byte from real Efuse. */
uint8_t
efuse_OneByteRead(
		PADAPTER	padapter,
		uint16_t			addr,
		uint8_t			*data)
{
	uint8_t	tmpidx = 0;
	uint8_t	bResult;

	//DBG_871X("===> EFUSE_OneByteRead(), addr = %x\n", addr);
	//DBG_871X("===> EFUSE_OneByteRead() start, 0x34 = 0x%X\n", rtw_read32(pAdapter, EFUSE_TEST));

	// <20130227, Kordan> 8192E MP chip A-cut had better not set 0x34[11] until B-Cut.
/*
	if ( IS_HARDWARE_TYPE_8723B(pAdapter))
	{
		// <20130121, Kordan> For SMIC EFUSE specificatoin.
		//0x34[11]: SW force PGMEN input of efuse to high. (for the bank selected by 0x34[9:8])
		//PHY_SetMacReg(pAdapter, 0x34, BIT11, 0);
		rtw_write16(pAdapter, 0x34, rtw_read16(pAdapter,0x34)& (~BIT11) );
	}
*/
	// -----------------e-fuse reg ctrl ---------------------------------
	//address
	rtw_write8(padapter, EFUSE_CTRL+1, (uint8_t)(addr&0xff));
	rtw_write8(padapter, EFUSE_CTRL+2, ((uint8_t)((addr>>8) &0x03) ) |
	(rtw_read8(padapter, EFUSE_CTRL+2)&0xFC ));

	rtw_write8(padapter, EFUSE_CTRL+3,  0x72);//read cmd

	while(!(0x80 &rtw_read8(padapter, EFUSE_CTRL+3))&&(tmpidx<1000))
	{
		rtw_mdelay_os(1);
		tmpidx++;
	}
	if(tmpidx<1000)
	{
		*data=rtw_read8(padapter, EFUSE_CTRL);
		bResult = _TRUE;
	}
	else
	{
		*data = 0xff;
		bResult = 0;
	}

	return bResult;
}

/*  11/16/2008 MH Write one byte to reald Efuse. */
uint8_t
efuse_OneByteWrite(
		PADAPTER	padapter,
		uint16_t			addr,
		uint8_t			data)
{
	uint8_t	tmpidx = 0;
	uint8_t	bResult= 0;
	uint32_t efuseValue = 0;

	//DBG_871X("===> EFUSE_OneByteWrite(), addr = %x data=%x\n", addr, data);
	//DBG_871X("===> EFUSE_OneByteWrite() start, 0x34 = 0x%X\n", rtw_read32(pAdapter, EFUSE_TEST));

	// -----------------e-fuse reg ctrl ---------------------------------
	//address

	// <20121213, Kordan> 8723BE SMIC TestChip workaround.
/*
	if (IS_HARDWARE_TYPE_8723B(pAdapter) && ! pHalData->bIsMPChip)
	{
		//PlatformEFIOWrite1Byte(pAdapter, REG_MULTI_FUNC_CTRL+1, 0x69); // Turn off EFUSE protection.
		rtw_write16(pAdapter, REG_SYS_ISO_CTRL, rtw_read16(pAdapter, REG_SYS_ISO_CTRL)| BIT14); // Turn on power cut

		EFUSE_PowerSwitch(pAdapter,TRUE, TRUE); // LDO 2.6V, this must follows after power cut.
	}
*/

	efuseValue = rtw_read32(padapter, EFUSE_CTRL);
	efuseValue |= (BIT21|BIT31);
	efuseValue &= ~(0x3FFFF);
	efuseValue |= ((addr<<8 | data) & 0x3FFFF);

/*
	// <20130227, Kordan> 8192E MP chip A-cut had better not set 0x34[11] until B-Cut.
	if (IS_HARDWARE_TYPE_8723B(pAdapter) && pHalData->bIsMPChip)
	{
		// <20130121, Kordan> For SMIC EFUSE specificatoin.
		//0x34[11]: SW force PGMEN input of efuse to high. (for the bank selected by 0x34[9:8])
		PHY_SetMacReg(pAdapter, 0x34, BIT11, 1);
		PlatformEFIOWrite4Byte(pAdapter, EFUSE_CTRL, 0x90600000|((addr<<8 | data)) );
	}
	else
*/
	{
		rtw_write32(padapter, EFUSE_CTRL, efuseValue);
	}

	while((0x80 &  rtw_read8(padapter, EFUSE_CTRL+3)) && (tmpidx<100) ){
		rtw_mdelay_os(1);
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
Efuse_PgPacketRead(		PADAPTER	padapter,
						uint8_t			offset,
						uint8_t			*data)
{
	int	ret=0;

	ret =  padapter->HalFunc->Efuse_PgPacketRead(padapter, offset, data);

	return ret;
}

int
Efuse_PgPacketWrite(	PADAPTER	padapter,
						uint8_t 			offset,
						uint8_t			word_en,
						uint8_t			*data)
{
	int ret;

	ret =  padapter->HalFunc->Efuse_PgPacketWrite(padapter, offset, word_en, data);

	return ret;
}


int
Efuse_PgPacketWrite_BT(	PADAPTER	padapter,
						uint8_t 			offset,
						uint8_t			word_en,
						uint8_t			*data)
{
	int ret;

	ret =  padapter->HalFunc->Efuse_PgPacketWrite_BT(padapter, offset, word_en, data);

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
Efuse_WordEnableDataWrite(		PADAPTER	padapter,
								uint16_t		efuse_addr,
								uint8_t		word_en,
								uint8_t		*data)
{
	uint8_t	ret=0;

	ret =  padapter->HalFunc->Efuse_WordEnableDataWrite(padapter, efuse_addr, word_en, data);

	return ret;
}

static uint8_t efuse_read8(PADAPTER padapter, uint16_t address, uint8_t *value)
{
	return efuse_OneByteRead(padapter,address, value);
}

static uint8_t efuse_write8(PADAPTER padapter, uint16_t address, uint8_t *value)
{
	return efuse_OneByteWrite(padapter,address, *value);
}

/*
 * read/wirte raw efuse data
 */
uint8_t rtw_efuse_access(PADAPTER padapter, uint8_t bWrite, uint16_t start_addr, uint16_t cnts, uint8_t *data)
{
	int i = 0;
	uint16_t	real_content_len = 0, max_available_size = 0;
	uint8_t res = _FAIL ;
	uint8_t (*rw8)(PADAPTER, u16, uint8_t*);

	EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_EFUSE_REAL_CONTENT_LEN, (PVOID)&real_content_len);
	EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, (PVOID)&max_available_size);

	if (start_addr > real_content_len)
		return _FAIL;

	if (_TRUE == bWrite) {
		if ((start_addr + cnts) > max_available_size)
			return _FAIL;
		rw8 = &efuse_write8;
	} else
		rw8 = &efuse_read8;

	padapter->HalFunc->EfusePowerSwitch(padapter, bWrite, _TRUE);

	// e-fuse one byte read / write
	for (i = 0; i < cnts; i++) {
		if (start_addr >= real_content_len) {
			res = _FAIL;
			break;
		}

		res = rw8(padapter, start_addr++, data++);
		if (_FAIL == res) break;
	}

	padapter->HalFunc->EfusePowerSwitch(padapter, bWrite, _FALSE);

	return res;
}
//------------------------------------------------------------------------------
uint16_t efuse_GetMaxSize(PADAPTER padapter)
{
	uint16_t	max_size;
	EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI , TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, (PVOID)&max_size);
	return max_size;
}
//------------------------------------------------------------------------------
uint8_t efuse_GetCurrentSize(PADAPTER padapter, uint16_t *size)
{
	padapter->HalFunc->EfusePowerSwitch(padapter, _FALSE, _TRUE);
	*size = Efuse_GetCurrentSize(padapter, EFUSE_WIFI);
	padapter->HalFunc->EfusePowerSwitch(padapter, _FALSE, _FALSE);

	return _SUCCESS;
}
//------------------------------------------------------------------------------
uint8_t rtw_efuse_map_read(PADAPTER padapter, uint16_t addr, uint16_t cnts, uint8_t *data)
{
	uint16_t	mapLen=0;

	EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_EFUSE_MAP_LEN, (PVOID)&mapLen);

	if ((addr + cnts) > mapLen)
		return _FAIL;

	padapter->HalFunc->EfusePowerSwitch(padapter, _FALSE, _TRUE);

	efuse_ReadEFuse(padapter, EFUSE_WIFI, addr, cnts, data);

	padapter->HalFunc->EfusePowerSwitch(padapter, _FALSE, _FALSE);

	return _SUCCESS;
}

uint8_t rtw_BT_efuse_map_read(PADAPTER padapter, uint16_t addr, uint16_t cnts, uint8_t *data)
{
	uint16_t	mapLen=0;

	EFUSE_GetEfuseDefinition(padapter, EFUSE_BT, TYPE_EFUSE_MAP_LEN, (PVOID)&mapLen);

	if ((addr + cnts) > mapLen)
		return _FAIL;

	padapter->HalFunc->EfusePowerSwitch(padapter, _FALSE, _TRUE);

	efuse_ReadEFuse(padapter, EFUSE_BT, addr, cnts, data);

	padapter->HalFunc->EfusePowerSwitch(padapter, _FALSE, _FALSE);

	return _SUCCESS;
}
//------------------------------------------------------------------------------
uint8_t rtw_efuse_map_write(PADAPTER padapter, uint16_t addr, uint16_t cnts, uint8_t *data)
{
	uint8_t	offset, word_en;
	uint8_t	*map;
	uint8_t	newdata[PGPKT_DATA_SIZE];
	int32_t	i, j, idx;
	uint8_t	ret = _SUCCESS;
	uint16_t	mapLen=0;

	EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_EFUSE_MAP_LEN, (PVOID)&mapLen);

	if ((addr + cnts) > mapLen)
		return _FAIL;

	map = rtw_zmalloc(mapLen);
	if(map == NULL){
		return _FAIL;
	}

	ret = rtw_efuse_map_read(padapter, 0, mapLen, map);
	if (ret == _FAIL) goto exit;

	padapter->HalFunc->EfusePowerSwitch(padapter, _TRUE, _TRUE);

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
			ret = Efuse_PgPacketWrite(padapter, offset, word_en, newdata);
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

	padapter->HalFunc->EfusePowerSwitch(padapter, _TRUE, _FALSE);

exit:
	/* ULLI check usage of mapLen */
	rtw_mfree(map);

	return ret;
}


//------------------------------------------------------------------------------
uint8_t rtw_BT_efuse_map_write(PADAPTER padapter, uint16_t addr, uint16_t cnts, uint8_t *data)
{
	uint8_t	offset, word_en;
	uint8_t	*map;
	uint8_t	newdata[PGPKT_DATA_SIZE];
	int32_t	i, j, idx;
	uint8_t	ret = _SUCCESS;
	uint16_t	mapLen=0;

	EFUSE_GetEfuseDefinition(padapter, EFUSE_BT, TYPE_EFUSE_MAP_LEN, (PVOID)&mapLen);

	if ((addr + cnts) > mapLen)
		return _FAIL;

	map = rtw_zmalloc(mapLen);
	if(map == NULL){
		return _FAIL;
	}

	ret = rtw_BT_efuse_map_read(padapter, 0, mapLen, map);
	if (ret == _FAIL) goto exit;

	padapter->HalFunc->EfusePowerSwitch(padapter, _TRUE, _TRUE);

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

			ret = Efuse_PgPacketWrite_BT(padapter, offset, word_en, newdata);
			if (ret == _FAIL) break;
		}

		if (idx == cnts) break;

		offset++;
		i = 0;
		j = 0;
		word_en = 0xF;
		memset(newdata, 0xFF, PGPKT_DATA_SIZE);
	} while (1);

	padapter->HalFunc->EfusePowerSwitch(padapter, _TRUE, _FALSE);

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
static VOID
Efuse_ReadAllMap(
			PADAPTER	padapter,
			uint8_t		efuseType,
	 	uint8_t		*Efuse)
{
	uint16_t	mapLen=0;

	padapter->HalFunc->EfusePowerSwitch(padapter,_FALSE, _TRUE);

	EFUSE_GetEfuseDefinition(padapter, efuseType, TYPE_EFUSE_MAP_LEN, (PVOID)&mapLen);

	efuse_ReadEFuse(padapter, efuseType, 0, mapLen, Efuse);

	padapter->HalFunc->EfusePowerSwitch(padapter,_FALSE, _FALSE);
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
static VOID
efuse_ShadowRead1Byte(
		PADAPTER	padapter,
		uint16_t		Offset,
	 	uint8_t		*Value)
{
	EEPROM_EFUSE_PRIV *pEEPROM = GET_EEPROM_EFUSE_PRIV(padapter);

	*Value = pEEPROM->efuse_eeprom_data[Offset];

}	// EFUSE_ShadowRead1Byte

//---------------Read Two Bytes
static VOID
efuse_ShadowRead2Byte(
		PADAPTER	padapter,
		uint16_t		Offset,
	 	uint16_t		*Value)
{
	EEPROM_EFUSE_PRIV *pEEPROM = GET_EEPROM_EFUSE_PRIV(padapter);

	*Value = pEEPROM->efuse_eeprom_data[Offset];
	*Value |= pEEPROM->efuse_eeprom_data[Offset+1]<<8;

}	// EFUSE_ShadowRead2Byte

//---------------Read Four Bytes
static VOID
efuse_ShadowRead4Byte(
		PADAPTER	padapter,
		uint16_t		Offset,
	 	uint32_t		*Value)
{
	EEPROM_EFUSE_PRIV *pEEPROM = GET_EEPROM_EFUSE_PRIV(padapter);

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
static VOID
efuse_ShadowWrite1Byte(
		PADAPTER	padapter,
		uint16_t		Offset,
	 	uint8_t		Value);
#endif //PLATFORM
static VOID
efuse_ShadowWrite1Byte(
		PADAPTER	padapter,
		uint16_t		Offset,
	 	uint8_t		Value)
{
	EEPROM_EFUSE_PRIV *pEEPROM = GET_EEPROM_EFUSE_PRIV(padapter);

	pEEPROM->efuse_eeprom_data[Offset] = Value;

}	// efuse_ShadowWrite1Byte

//---------------Write Two Bytes
static VOID
efuse_ShadowWrite2Byte(
		PADAPTER	padapter,
		uint16_t		Offset,
	 	uint16_t		Value)
{
	EEPROM_EFUSE_PRIV *pEEPROM = GET_EEPROM_EFUSE_PRIV(padapter);

	pEEPROM->efuse_eeprom_data[Offset] = Value&0x00FF;
	pEEPROM->efuse_eeprom_data[Offset+1] = Value>>8;

}	// efuse_ShadowWrite1Byte

//---------------Write Four Bytes
static VOID
efuse_ShadowWrite4Byte(
		PADAPTER	padapter,
		uint16_t		Offset,
		uint32_t		Value)
{
	EEPROM_EFUSE_PRIV *pEEPROM = GET_EEPROM_EFUSE_PRIV(padapter);

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
	 PADAPTER	padapter,
	 uint8_t		efuseType)
{
	EEPROM_EFUSE_PRIV *pEEPROM = GET_EEPROM_EFUSE_PRIV(padapter);
	uint16_t	mapLen=0;

	EFUSE_GetEfuseDefinition(padapter, efuseType, TYPE_EFUSE_MAP_LEN, (PVOID)&mapLen);

	if (pEEPROM->bautoload_fail_flag == _TRUE)
	{
		memset(pEEPROM->efuse_eeprom_data, 0xFF, mapLen);
	}
	else
	{
		#ifdef CONFIG_ADAPTOR_INFO_CACHING_FILE
		if(_SUCCESS != retriveAdaptorInfoFile(padapter->registrypriv.adaptor_info_caching_file_path, pEEPROM)) {
		#endif

		Efuse_ReadAllMap(padapter, efuseType, pEEPROM->efuse_eeprom_data);

		#ifdef CONFIG_ADAPTOR_INFO_CACHING_FILE
			storeAdaptorInfoFile(padapter->registrypriv.adaptor_info_caching_file_path, pEEPROM);
		}
		#endif
	}

	//PlatformMoveMemory((PVOID)&pHalData->EfuseMap[EFUSE_MODIFY_MAP][0],
	//(PVOID)&pHalData->EfuseMap[EFUSE_INIT_MAP][0], mapLen);
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
			PADAPTER	padapter,
			uint8_t		Type,
			uint16_t		Offset,
	 	uint32_t		*Value	)
{
	if (Type == 1)
		efuse_ShadowRead1Byte(padapter, Offset, (uint8_t *)Value);
	else if (Type == 2)
		efuse_ShadowRead2Byte(padapter, Offset, (uint16_t *)Value);
	else if (Type == 4)
		efuse_ShadowRead4Byte(padapter, Offset, (uint32_t *)Value);

}	// EFUSE_ShadowRead

/*-----------------------------------------------------------------------------
 * Function:	EFUSE_ShadowWrite
 *
 * Overview:	Write efuse modify map for later update operation to use!!!!!
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
VOID
EFUSE_ShadowWrite(
		PADAPTER	padapter,
		uint8_t		Type,
		uint16_t		Offset,
	 	uint32_t		Value);
VOID
EFUSE_ShadowWrite(
		PADAPTER	padapter,
		uint8_t		Type,
		uint16_t		Offset,
	 	uint32_t		Value)
{
#if (MP_DRIVER == 0)
	return;
#endif
	if ( padapter->registrypriv.mp_mode == 0)
		return;


	if (Type == 1)
		efuse_ShadowWrite1Byte(padapter, Offset, (uint8_t)Value);
	else if (Type == 2)
		efuse_ShadowWrite2Byte(padapter, Offset, (u16)Value);
	else if (Type == 4)
		efuse_ShadowWrite4Byte(padapter, Offset, (u32)Value);

}	// EFUSE_ShadowWrite

VOID
Efuse_InitSomeVar(
			PADAPTER	padapter
	);
VOID
Efuse_InitSomeVar(
			PADAPTER	padapter
	)
{
	uint8_t i;

	memset((PVOID)&fakeEfuseContent[0], 0xff, EFUSE_MAX_HW_SIZE);
	memset((PVOID)&fakeEfuseInitMap[0], 0xff, EFUSE_MAX_MAP_LEN);
	memset((PVOID)&fakeEfuseModifiedMap[0], 0xff, EFUSE_MAX_MAP_LEN);

	for(i=0; i<EFUSE_MAX_BT_BANK; i++)
	{
		memset((PVOID)&BTEfuseContent[i][0], EFUSE_MAX_HW_SIZE, 0xff);
	}
	memset((PVOID)&BTEfuseInitMap[0], 0xff, EFUSE_BT_MAX_MAP_LEN);
	memset((PVOID)&BTEfuseModifiedMap[0], 0xff, EFUSE_BT_MAX_MAP_LEN);

	for(i=0; i<EFUSE_MAX_BT_BANK; i++)
	{
		memset((PVOID)&fakeBTEfuseContent[i][0], 0xff, EFUSE_MAX_HW_SIZE);
	}
	memset((PVOID)&fakeBTEfuseInitMap[0], 0xff, EFUSE_BT_MAX_MAP_LEN);
	memset((PVOID)&fakeBTEfuseModifiedMap[0], 0xff, EFUSE_BT_MAX_MAP_LEN);
}

#ifdef PLATFORM_LINUX
#ifdef CONFIG_ADAPTOR_INFO_CACHING_FILE
//#include <rtw_eeprom.h>

 int isAdaptorInfoFileValid(void)
{
	return _TRUE;
}

int storeAdaptorInfoFile(char *path, struct eeprom_priv * eeprom_priv)
{
	int ret =_SUCCESS;

	if(path && eeprom_priv) {
		ret = rtw_store_to_file(path, eeprom_priv->efuse_eeprom_data, EEPROM_MAX_SIZE_512);
		if(ret == EEPROM_MAX_SIZE)
			ret = _SUCCESS;
		else
			ret = _FAIL;
	} else {
		DBG_871X("%s NULL pointer\n",__FUNCTION__);
		ret =  _FAIL;
	}
	return ret;
}

int retriveAdaptorInfoFile(char *path, struct eeprom_priv * eeprom_priv)
{
	int ret = _SUCCESS;
	mm_segment_t oldfs;
	struct file *fp;

	if(path && eeprom_priv) {

		ret = rtw_retrive_from_file(path, eeprom_priv->efuse_eeprom_data, EEPROM_MAX_SIZE);

		if(ret == EEPROM_MAX_SIZE)
			ret = _SUCCESS;
		else
			ret = _FAIL;

		#if 0
		if(isAdaptorInfoFileValid()) {
			return 0;
		} else {
			return _FAIL;
		}
		#endif

	} else {
		DBG_871X("%s NULL pointer\n",__FUNCTION__);
		ret = _FAIL;
	}
	return ret;
}
#endif //CONFIG_ADAPTOR_INFO_CACHING_FILE
#endif //PLATFORM_LINUX


