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
#define _RTL8812A_HAL_INIT_C_
#include <rtl8812a_hal.h>

/*
 *-------------------------------------------------------------------------
 *
 * LLT R/W/Init function
 *
 *-------------------------------------------------------------------------
 */
static int32_t _LLTWrite(struct rtl_priv *padapter, uint32_t address, uint32_t data)
{
	int32_t	status = _SUCCESS;
	int32_t	count = 0;
	uint32_t	value = _LLT_INIT_ADDR(address) | _LLT_INIT_DATA(data) | _LLT_OP(_LLT_WRITE_ACCESS);
	u16	LLTReg = REG_LLT_INIT;

	rtl_write_dword(padapter, LLTReg, value);

	/* polling */
	do {
		value = rtl_read_dword(padapter, LLTReg);
		if (_LLT_NO_ACTIVE == _LLT_OP_VALUE(value)) {
			break;
		}

		if (count > POLLING_LLT_THRESHOLD) {
			RT_TRACE(_module_hal_init_c_, _drv_err_, ("Failed to polling write LLT done at address %d!\n", address));
			status = _FAIL;
			break;
		}
	} while (count++);

	return status;
}

uint8_t _LLTRead(struct rtl_priv *padapter, uint32_t address)
{
	int32_t	count = 0;
	uint32_t	value = _LLT_INIT_ADDR(address) | _LLT_OP(_LLT_READ_ACCESS);
	u16	LLTReg = REG_LLT_INIT;


	rtl_write_dword(padapter, LLTReg, value);

	/* polling and get value */
	do {
		value = rtl_read_dword(padapter, LLTReg);
		if (_LLT_NO_ACTIVE == _LLT_OP_VALUE(value)) {
			return (uint8_t)value;
		}

		if (count > POLLING_LLT_THRESHOLD) {
			RT_TRACE(_module_hal_init_c_, _drv_err_, ("Failed to polling read LLT done at address %d!\n", address));
			break;
		}
	} while (count++);

	return 0xFF;
}

int32_t InitLLTTable8812(struct rtl_priv *padapter, uint8_t txpktbuf_bndy)
{
	int32_t	status = _FAIL;
	uint32_t	i;
	uint32_t	Last_Entry_Of_TxPktBuf = LAST_ENTRY_OF_TX_PKT_BUFFER_8812;
	 struct _rtw_hal *pHalData	= GET_HAL_DATA(padapter);

	for (i = 0; i < (txpktbuf_bndy - 1); i++) {
		status = _LLTWrite(padapter, i, i + 1);
		if (_SUCCESS != status) {
			return status;
		}
	}

	/* end of list */
	status = _LLTWrite(padapter, (txpktbuf_bndy - 1), 0xFF);
	if (_SUCCESS != status) {
		return status;
	}

	/*
	 * Make the other pages as ring buffer
	 * This ring buffer is used as beacon buffer if we config this MAC as two MAC transfer.
	 * Otherwise used as local loopback buffer.
	 */
	for (i = txpktbuf_bndy; i < Last_Entry_Of_TxPktBuf; i++) {
		status = _LLTWrite(padapter, i, (i + 1));
		if (_SUCCESS != status) {
			return status;
		}
	}

	/*  Let last entry point to the start entry of ring buffer */
	status = _LLTWrite(padapter, Last_Entry_Of_TxPktBuf, txpktbuf_bndy);
	if (_SUCCESS != status) {
		return status;
	}

	return status;
}

BOOLEAN HalDetectPwrDownMode8812(struct rtl_priv *Adapter)
{
	uint8_t tmpvalue = 0;
	 struct _rtw_hal *pHalData = GET_HAL_DATA(Adapter);
	struct pwrctrl_priv *pwrctrlpriv = &Adapter->pwrctrlpriv;

	EFUSE_ShadowRead(Adapter, 1, EEPROM_RF_OPT3_92C, (uint32_t *)&tmpvalue);

	/* 2010/08/25 MH INF priority > PDN Efuse value. */
	if (tmpvalue & BIT(4) && pwrctrlpriv->reg_pdnmode)
		pHalData->pwrdown = _TRUE;
	else
		pHalData->pwrdown = _FALSE;

	DBG_8192C("HalDetectPwrDownMode(): PDN=%d\n", pHalData->pwrdown);

	return pHalData->pwrdown;
}


/*====================================================================================
 *
 * 20100209 Joseph:
 * This function is used only for 92C to set REG_BCN_CTRL(0x550) register.
 * We just reserve the value of the register in variable pHalData->RegBcnCtrlVal and then operate
 * the value of the register via atomic operation.
 * This prevents from race condition when setting this register.
 * The value of pHalData->RegBcnCtrlVal is initialized in HwConfigureRTL8192CE() function.
 *
 */

void SetBcnCtrlReg(struct rtl_priv *padapter, uint8_t SetBits, uint8_t ClearBits)
{
	struct _rtw_hal *pHalData;

	pHalData = GET_HAL_DATA(padapter);

	pHalData->RegBcnCtrlVal |= SetBits;
	pHalData->RegBcnCtrlVal &= ~ClearBits;

	rtl_write_byte(padapter, REG_BCN_CTRL, (uint8_t)pHalData->RegBcnCtrlVal);
}

static VOID _FWDownloadEnable_8812(struct rtl_priv *padapter, BOOLEAN enable)
{
	uint8_t	tmp;

	if (enable) {
		/* MCU firmware download enable. */
		tmp = rtl_read_byte(padapter, REG_MCUFWDL);
		rtl_write_byte(padapter, REG_MCUFWDL, tmp|0x01);

		/* 8051 reset */
		tmp = rtl_read_byte(padapter, REG_MCUFWDL+2);
		rtl_write_byte(padapter, REG_MCUFWDL+2, tmp&0xf7);
	} else {

		/* MCU firmware download disable. */
		tmp = rtl_read_byte(padapter, REG_MCUFWDL);
		rtl_write_byte(padapter, REG_MCUFWDL, tmp&0xfe);
	}
}
#define MAX_REG_BOLCK_SIZE	196

static int _BlockWrite_8812(struct rtl_priv *padapter, PVOID buffer, uint32_t buffSize)
{
	int ret = _SUCCESS;

	uint32_t blockSize_p1 = 4;	/* (Default) Phase #1 : PCI muse use 4-byte write to download FW */
	uint32_t blockSize_p2 = 8;	/* Phase #2 : Use 8-byte, if Phase#1 use big size to write FW. */
	uint32_t blockSize_p3 = 1;	/* Phase #3 : Use 1-byte, the remnant of FW image. */
	uint32_t blockCount_p1 = 0, blockCount_p2 = 0, blockCount_p3 = 0;
	uint32_t remainSize_p1 = 0, remainSize_p2 = 0;
	uint8_t	 *bufferPtr	= (uint8_t *)buffer;
	uint32_t i = 0, offset = 0;

	blockSize_p1 = MAX_REG_BOLCK_SIZE;

	/* 3 Phase #1 */
	blockCount_p1 = buffSize / blockSize_p1;
	remainSize_p1 = buffSize % blockSize_p1;

	if (blockCount_p1) {
		RT_TRACE(_module_hal_init_c_, _drv_notice_,
				("_BlockWrite: [P1] buffSize(%d) blockSize_p1(%d) blockCount_p1(%d) remainSize_p1(%d)\n",
				buffSize, blockSize_p1, blockCount_p1, remainSize_p1));
	}

	for (i = 0; i < blockCount_p1; i++) {
		ret = rtw_writeN(padapter, (FW_START_ADDRESS + i * blockSize_p1), blockSize_p1, (bufferPtr + i * blockSize_p1));

		if (ret == _FAIL)
			goto exit;
	}


	/* 3 Phase #2 */
	if (remainSize_p1) {
		offset = blockCount_p1 * blockSize_p1;

		blockCount_p2 = remainSize_p1/blockSize_p2;
		remainSize_p2 = remainSize_p1%blockSize_p2;

		if (blockCount_p2) {
				RT_TRACE(_module_hal_init_c_, _drv_notice_,
						("_BlockWrite: [P2] buffSize_p2(%d) blockSize_p2(%d) blockCount_p2(%d) remainSize_p2(%d)\n",
						(buffSize-offset), blockSize_p2, blockCount_p2, remainSize_p2));
		}

		for (i = 0; i < blockCount_p2; i++) {
			ret = rtw_writeN(padapter, (FW_START_ADDRESS + offset + i*blockSize_p2), blockSize_p2, (bufferPtr + offset + i*blockSize_p2));

			if (ret == _FAIL)
				goto exit;
		}
	}

	/* 3 Phase #3 */
	if (remainSize_p2) {
		offset = (blockCount_p1 * blockSize_p1) + (blockCount_p2 * blockSize_p2);

		blockCount_p3 = remainSize_p2 / blockSize_p3;

		RT_TRACE(_module_hal_init_c_, _drv_notice_,
				("_BlockWrite: [P3] buffSize_p3(%d) blockSize_p3(%d) blockCount_p3(%d)\n",
				(buffSize-offset), blockSize_p3, blockCount_p3));

		for (i = 0 ; i < blockCount_p3; i++) {
			ret = rtl_write_byte(padapter, (FW_START_ADDRESS + offset + i), *(bufferPtr + offset + i));

			if (ret == _FAIL)
				goto exit;
		}
	}

exit:
	return ret;
}

static int _PageWrite_8812(struct rtl_priv *padapter, uint32_t page,
	PVOID buffer, uint32_t size)
{
	uint8_t value8;
	uint8_t u8Page = (uint8_t) (page & 0x07) ;

	value8 = (rtl_read_byte(padapter, REG_MCUFWDL+2) & 0xF8) | u8Page ;
	rtl_write_byte(padapter, REG_MCUFWDL+2, value8);

	return _BlockWrite_8812(padapter, buffer, size);
}


static int _WriteFW_8812(struct rtl_priv *padapter, PVOID buffer, uint32_t size)
{
	/*
	 * Since we need dynamic decide method of dwonload fw, so we call this function to get chip version.
	 * We can remove _ReadChipVersion from ReadpadapterInfo8192C later.
	 */

	int	ret = _SUCCESS;
	uint32_t pageNums, remainSize;
	uint32_t page, offset;
	uint8_t	*bufferPtr = (uint8_t *)buffer;

	pageNums = size / MAX_DLFW_PAGE_SIZE ;
	/*
	 * RT_ASSERT((pageNums <= 4), ("Page numbers should not greater then 4 \n"));
	 */

	remainSize = size % MAX_DLFW_PAGE_SIZE;

	for (page = 0; page < pageNums; page++) {
		offset = page * MAX_DLFW_PAGE_SIZE;
		ret = _PageWrite_8812(padapter, page, bufferPtr+offset, MAX_DLFW_PAGE_SIZE);

		if (ret == _FAIL)
			goto exit;
	}
	if (remainSize) {
		offset = pageNums * MAX_DLFW_PAGE_SIZE;
		page = pageNums;
		ret = _PageWrite_8812(padapter, page, bufferPtr+offset, remainSize);

		if (ret == _FAIL)
			goto exit;

	}
	RT_TRACE(_module_hal_init_c_, _drv_info_, ("_WriteFW Done- for Normal chip.\n"));

exit:
	return ret;
}

void _8051Reset8812(struct rtl_priv *padapter)
{
	struct rtl_hal *rtlhal = rtl_hal(padapter);
	uint8_t u1bTmp, u1bTmp2;

	/* Reset MCU IO Wrapper- sugggest by SD1-Gimmy */
	if (IS_HARDWARE_TYPE_8812(rtlhal)) {
		u1bTmp2 = rtl_read_byte(padapter, REG_RSV_CTRL+1);
		rtl_write_byte(padapter, REG_RSV_CTRL + 1, u1bTmp2&(~BIT3));
	} else if (IS_HARDWARE_TYPE_8821(rtlhal)) {
		u1bTmp2 = rtl_read_byte(padapter, REG_RSV_CTRL+1);
		rtl_write_byte(padapter, REG_RSV_CTRL + 1, u1bTmp2&(~BIT0));
	}

	u1bTmp = rtl_read_byte(padapter, REG_SYS_FUNC_EN+1);
	rtl_write_byte(padapter, REG_SYS_FUNC_EN+1, u1bTmp&(~BIT2));

	/* Enable MCU IO Wrapper */
	if (IS_HARDWARE_TYPE_8812(rtlhal)) {
		u1bTmp2 = rtl_read_byte(padapter, REG_RSV_CTRL+1);
		rtl_write_byte(padapter, REG_RSV_CTRL + 1, u1bTmp2 | (BIT3));
	} else if (IS_HARDWARE_TYPE_8821(rtlhal)) {
		u1bTmp2 = rtl_read_byte(padapter, REG_RSV_CTRL+1);
		rtl_write_byte(padapter, REG_RSV_CTRL + 1, u1bTmp2 | (BIT0));
	}

	rtl_write_byte(padapter, REG_SYS_FUNC_EN+1, u1bTmp|(BIT2));

	DBG_871X("=====> _8051Reset8812(): 8051 reset success .\n");
}

static int32_t _FWFreeToGo8812(struct rtl_priv *padapter)
{
	uint32_t	counter = 0;
	uint32_t	value32;
	uint8_t 	value8;

	/* polling CheckSum report */
	do {
		value32 = rtl_read_dword(padapter, REG_MCUFWDL);
		if (value32 & FWDL_ChkSum_rpt)
			break;
	} while (counter++ < 6000);

	if (counter >= 6000) {
		DBG_871X("%s: chksum report fail! REG_MCUFWDL:0x%08x\n", __FUNCTION__, value32);
		return _FAIL;
	}
	DBG_871X("%s: Checksum report OK! REG_MCUFWDL:0x%08x\n", __FUNCTION__, value32);

	value32 = rtl_read_dword(padapter, REG_MCUFWDL);
	value32 |= MCUFWDL_RDY;
	value32 &= ~WINTINI_RDY;
	rtl_write_dword(padapter, REG_MCUFWDL, value32);

	_8051Reset8812(padapter);

	/* polling for FW ready */
	counter = 0;
	do {
		value32 = rtl_read_dword(padapter, REG_MCUFWDL);
		if (value32 & WINTINI_RDY) {
			DBG_871X("%s: Polling FW ready success!! REG_MCUFWDL:0x%08x\n", __FUNCTION__, value32);
			return _SUCCESS;
		}
		udelay(5);
	} while (counter++ < 6000);

	DBG_871X ("%s: Polling FW ready fail!! REG_MCUFWDL:0x%08x\n", __FUNCTION__, value32);
	return _FAIL;
}

int32_t FirmwareDownload8812(struct rtl_priv *Adapter, BOOLEAN bUsedWoWLANFw)
{
	struct rtl_hal *rtlhal = rtl_hal(Adapter);
	int32_t	rtStatus = _SUCCESS;
	uint8_t	writeFW_retry = 0;
	uint32_t fwdl_start_time;
	struct _rtw_hal *pHalData = GET_HAL_DATA(Adapter);
	struct rtl_dm *	pDM_Odm;
	uint8_t				*pFwHdr = NULL;

	pDM_Odm = &pHalData->odmpriv;

	RT_TRACE(_module_hal_init_c_, _drv_info_, ("+%s\n", __FUNCTION__));

	if (pDM_Odm->SupportICType == ODM_RTL8812)
		ODM_ReadFirmware_MP_8812A_FW_NIC(&rtlhal->pfirmware, &rtlhal->fwsize);
	if (pDM_Odm->SupportICType == ODM_RTL8821)
			ODM_ReadFirmware_MP_8821A_FW_NIC(&rtlhal->pfirmware, &rtlhal->fwsize);

	DBG_871X(" ===> FirmwareDownload8812() fw:%s, size: %d\n", "Firmware for NIC", rtlhal->fwsize);

	if (rtlhal->fwsize > FW_SIZE_8812) {
			rtStatus = _FAIL;
			RT_TRACE(_module_hal_init_c_, _drv_err_, ("Firmware size exceed 0x%X. Check it.\n", FW_SIZE_8812));
			goto Exit;
		}


	{
		DBG_871X_LEVEL(_drv_info_, "+%s: !bUsedWoWLANFw, FmrmwareLen:%d+\n", __func__, rtlhal->fwsize);
		/* To Check Fw header. Added by tynli. 2009.12.04. */
		pFwHdr = (uint8_t *) rtlhal->pfirmware;
	}

	rtlhal->fw_version =  (u16)GET_FIRMWARE_HDR_VERSION_8812(pFwHdr);
	rtlhal->fw_subversion = (u16)GET_FIRMWARE_HDR_SUB_VER_8812(pFwHdr);
/*	
	pHalData->FirmwareSignature = (u16)GET_FIRMWARE_HDR_SIGNATURE_8812(pFwHdr);

	DBG_871X ("%s: fw_ver=%d fw_subver=%d sig=0x%x\n",
		  __FUNCTION__, pHalData->FirmwareVersion, pHalData->FirmwareSubVersion, pHalData->FirmwareSignature);
*/
	DBG_871X ("%s: fw_ver=%d fw_subver=%d\n",
		  __FUNCTION__, rtlhal->fw_version, rtlhal->fw_subversion);


	if (IS_FW_HEADER_EXIST_8812(pFwHdr) || IS_FW_HEADER_EXIST_8821(pFwHdr)) {
		/* Shift 32 bytes for FW header */
		rtlhal->pfirmware += 32;
		rtlhal->fwsize -= 32;
	}

	/*
	 * Suggested by Filen. If 8051 is running in RAM code, driver should inform Fw to reset by itself,
	 * or it will cause download Fw fail. 2010.02.01. by tynli.
	 */
	if (rtl_read_byte(Adapter, REG_MCUFWDL) & BIT7) { /* 8051 RAM code */
		rtl_write_byte(Adapter, REG_MCUFWDL, 0x00);
		_8051Reset8812(Adapter);
	}

	_FWDownloadEnable_8812(Adapter, _TRUE);
	fwdl_start_time = jiffies;
	while (1) {
		/* reset the FWDL chksum */
		rtl_write_byte(Adapter, REG_MCUFWDL, rtl_read_byte(Adapter, REG_MCUFWDL)|FWDL_ChkSum_rpt);

		rtStatus = _WriteFW_8812(Adapter, rtlhal->pfirmware, rtlhal->fwsize);

		if (rtStatus == _SUCCESS
		   || (rtw_get_passing_time_ms(fwdl_start_time) > 500 && writeFW_retry++ >= 3))
			break;

		DBG_871X("%s writeFW_retry:%u, time after fwdl_start_time:%ums\n", __FUNCTION__
			, writeFW_retry, rtw_get_passing_time_ms(fwdl_start_time)
		);
	}
	_FWDownloadEnable_8812(Adapter, _FALSE);
	if (_SUCCESS != rtStatus) {
		DBG_871X("DL Firmware failed!\n");
		goto Exit;
	}

	rtStatus = _FWFreeToGo8812(Adapter);
	if (_SUCCESS != rtStatus) {
		DBG_871X("DL Firmware failed!\n");
		goto Exit;
	}
	RT_TRACE(_module_hal_init_c_, _drv_info_, ("Firmware is ready to run!\n"));

Exit:

	/*
	 * RT_TRACE(COMP_INIT, DBG_LOUD, (" <=== FirmwareDownload91C()\n"));
	 */

	return rtStatus;
}


void InitializeFirmwareVars8812(struct rtl_priv *padapter)
{
	struct _rtw_hal *pHalData = GET_HAL_DATA(padapter);
	struct pwrctrl_priv *pwrpriv;
	pwrpriv = &padapter->pwrctrlpriv;

	/* Init Fw LPS related. */
	padapter->pwrctrlpriv.bFwCurrentInPSMode = _FALSE;
	/* Init H2C counter. by tynli. 2009.12.09. */
	pHalData->LastHMEBoxNum = 0;
}

void rtl8812_free_hal_data(struct rtl_priv *padapter)
{
	if (padapter->HalData) {
			rtw_mfree(padapter->HalData);
		padapter->HalData = NULL;
	}
}

/*
 * ===========================================================
 * 				Efuse related code
 * ===========================================================
 */
BOOLEAN Hal_GetChnlGroup8812A(uint8_t Channel, uint8_t *pGroup)
{
	BOOLEAN bIn24G = _TRUE;

	if (Channel <= 14) {
		bIn24G = _TRUE;

		if (1 <= Channel && Channel <= 2)
			*pGroup = 0;
		else if (3  <= Channel && Channel <= 5)
			*pGroup = 1;
		else if (6  <= Channel && Channel <= 8)
			*pGroup = 2;
		else if (9  <= Channel && Channel <= 11)
			*pGroup = 3;
		else if (12 <= Channel && Channel <= 14)
			*pGroup = 4;
		else {
			DBG_871X("==>mpt_GetChnlGroup8812A in 2.4 G, but Channel %d in Group not found \n", Channel);
		}
	} else {
		bIn24G = _FALSE;

		if      (36   <= Channel && Channel <=  42)
			*pGroup = 0;
		else if (44   <= Channel && Channel <=  48)
			*pGroup = 1;
		else if (50   <= Channel && Channel <=  58)
			*pGroup = 2;
		else if (60   <= Channel && Channel <=  64)
			*pGroup = 3;
		else if (100  <= Channel && Channel <= 106)
			*pGroup = 4;
		else if (108  <= Channel && Channel <= 114)
			*pGroup = 5;
		else if (116  <= Channel && Channel <= 122)
			*pGroup = 6;
		else if (124  <= Channel && Channel <= 130)
			*pGroup = 7;
		else if (132  <= Channel && Channel <= 138)
			*pGroup = 8;
		else if (140  <= Channel && Channel <= 144)
			*pGroup = 9;
		else if (149  <= Channel && Channel <= 155)
			*pGroup = 10;
		else if (157  <= Channel && Channel <= 161)
			*pGroup = 11;
		else if (165  <= Channel && Channel <= 171)
			*pGroup = 12;
		else if (173  <= Channel && Channel <= 177)
			*pGroup = 13;
		else {
			DBG_871X("==>mpt_GetChnlGroup8812A in 5G, but Channel %d in Group not found \n", Channel);
		}

	}
	/* DBG_871X("<==mpt_GetChnlGroup8812A,  (%s) Channel = %d, Group =%d,\n", (bIn24G) ? "2.4G" : "5G", Channel, *pGroup); */

	return bIn24G;
}

static void
hal_ReadPowerValueFromPROM8812A(struct rtl_priv *Adapter, PTxPowerInfo24G pwrInfo24G,
	PTxPowerInfo5G	pwrInfo5G, uint8_t *PROMContent,
	BOOLEAN	AutoLoadFail)
{
	 struct _rtw_hal	*pHalData = GET_HAL_DATA(Adapter);
	uint32_t rfPath, eeAddr = EEPROM_TX_PWR_INX_8812, group, TxCount = 0;

	memset(pwrInfo24G, 0, sizeof(TxPowerInfo24G));
	memset(pwrInfo5G, 0, sizeof(TxPowerInfo5G));

	/* DBG_871X("hal_ReadPowerValueFromPROM8812A(): PROMContent[0x%x]=0x%x\n", (eeAddr+1), PROMContent[eeAddr+1]); */
	if (0xFF == PROMContent[eeAddr+1])  /* YJ,add,120316 */
		AutoLoadFail = _TRUE;

	if (AutoLoadFail) {
		DBG_871X("hal_ReadPowerValueFromPROM8812A(): Use Default value!\n");
		for (rfPath = 0 ; rfPath < MAX_RF_PATH ; rfPath++) {
			/*  2.4G default value */
			for (group = 0 ; group < MAX_CHNL_GROUP_24G; group++) {
				pwrInfo24G->IndexCCK_Base[rfPath][group] =	EEPROM_DEFAULT_24G_INDEX;
				pwrInfo24G->IndexBW40_Base[rfPath][group] =	EEPROM_DEFAULT_24G_INDEX;
			}
			for (TxCount = 0; TxCount < MAX_TX_COUNT; TxCount++) {
				if (TxCount == 0) {
					pwrInfo24G->BW20_Diff[rfPath][0] =	EEPROM_DEFAULT_24G_HT20_DIFF;
					pwrInfo24G->OFDM_Diff[rfPath][0] =	EEPROM_DEFAULT_24G_OFDM_DIFF;
				} else {
					pwrInfo24G->BW20_Diff[rfPath][TxCount] =	EEPROM_DEFAULT_DIFF;
					pwrInfo24G->BW40_Diff[rfPath][TxCount] =	EEPROM_DEFAULT_DIFF;
					pwrInfo24G->CCK_Diff[rfPath][TxCount] =	EEPROM_DEFAULT_DIFF;
					pwrInfo24G->OFDM_Diff[rfPath][TxCount] =	EEPROM_DEFAULT_DIFF;
				}
			}

			/* 5G default value */
			for (group = 0 ; group < MAX_CHNL_GROUP_5G; group++) {
				pwrInfo5G->IndexBW40_Base[rfPath][group] =		EEPROM_DEFAULT_5G_INDEX;
			}

			for (TxCount = 0; TxCount < MAX_TX_COUNT; TxCount++) {
				if (TxCount == 0) {
					pwrInfo5G->OFDM_Diff[rfPath][0] =	EEPROM_DEFAULT_5G_OFDM_DIFF;
					pwrInfo5G->BW20_Diff[rfPath][0] =	EEPROM_DEFAULT_5G_HT20_DIFF;
					pwrInfo5G->BW80_Diff[rfPath][0] =	EEPROM_DEFAULT_DIFF;
					pwrInfo5G->BW160_Diff[rfPath][0] =	EEPROM_DEFAULT_DIFF;
				} else {
					pwrInfo5G->OFDM_Diff[rfPath][TxCount] =	EEPROM_DEFAULT_DIFF;
					pwrInfo5G->BW20_Diff[rfPath][TxCount] =	EEPROM_DEFAULT_DIFF;
					pwrInfo5G->BW40_Diff[rfPath][TxCount] =	EEPROM_DEFAULT_DIFF;
					pwrInfo5G->BW80_Diff[rfPath][TxCount] =	EEPROM_DEFAULT_DIFF;
					pwrInfo5G->BW160_Diff[rfPath][TxCount] =	EEPROM_DEFAULT_DIFF;

				}
			}

		}

		/* pHalData->bNOPG = _TRUE; */
		return;
	}

	pHalData->bTXPowerDataReadFromEEPORM = _TRUE;		/* YJ,move,120316 */

	for (rfPath = 0; rfPath < MAX_RF_PATH; rfPath++) {
		/*  2.4G default value */
		for (group = 0; group < MAX_CHNL_GROUP_24G; group++) {
			pwrInfo24G->IndexCCK_Base[rfPath][group] =	PROMContent[eeAddr++];
			if (pwrInfo24G->IndexCCK_Base[rfPath][group] == 0xFF) {
				pwrInfo24G->IndexCCK_Base[rfPath][group] = EEPROM_DEFAULT_24G_INDEX;
				/* pHalData->bNOPG = _TRUE; */
			}
			/*
			 * DBG_871X("8812-2G RF-%d-G-%d CCK-Addr-%x BASE=%x\n",
			 * rfPath, group, eeAddr-1, pwrInfo24G->IndexCCK_Base[rfPath][group]);
			 */
		}

		for (group = 0; group < MAX_CHNL_GROUP_24G-1; group++) {
			pwrInfo24G->IndexBW40_Base[rfPath][group] =	PROMContent[eeAddr++];
			if (pwrInfo24G->IndexBW40_Base[rfPath][group] == 0xFF)
				pwrInfo24G->IndexBW40_Base[rfPath][group] = EEPROM_DEFAULT_24G_INDEX;
			/*
			 * DBG_871X("8812-2G RF-%d-G-%d BW40-Addr-%x BASE=%x\n",
			 * rfPath, group, eeAddr-1, pwrInfo24G->IndexBW40_Base[rfPath][group]);
			 */
		}

		for (TxCount = 0; TxCount < MAX_TX_COUNT; TxCount++) {
			if (TxCount == 0) {
				pwrInfo24G->BW40_Diff[rfPath][TxCount] = 0;

				{
					pwrInfo24G->BW20_Diff[rfPath][TxCount] = (PROMContent[eeAddr] & 0xf0) >> 4;
					if (pwrInfo24G->BW20_Diff[rfPath][TxCount] & BIT3)		/* 4bit sign number to 8 bit sign number */
						pwrInfo24G->BW20_Diff[rfPath][TxCount] |= 0xF0;
				}

				/*
				 * DBG_871X("8812-2G RF-%d-SS-%d BW20-Addr-%x DIFF=%d\n",
				 * rfPath, TxCount, eeAddr, pwrInfo24G->BW20_Diff[rfPath][TxCount]);
				 */

				{
					pwrInfo24G->OFDM_Diff[rfPath][TxCount] =	(PROMContent[eeAddr]&0x0f);
					if (pwrInfo24G->OFDM_Diff[rfPath][TxCount] & BIT3)		/* 4bit sign number to 8 bit sign number */
						pwrInfo24G->OFDM_Diff[rfPath][TxCount] |= 0xF0;
				}
				/*
				 * DBG_871X("8812-2G RF-%d-SS-%d LGOD-Addr-%x DIFF=%d\n",
				 * rfPath, TxCount, eeAddr, pwrInfo24G->OFDM_Diff[rfPath][TxCount]);
				 */

				pwrInfo24G->CCK_Diff[rfPath][TxCount] = 0;
				eeAddr++;
			} else {

				{
					pwrInfo24G->BW40_Diff[rfPath][TxCount] =	(PROMContent[eeAddr]&0xf0)>>4;
					if (pwrInfo24G->BW40_Diff[rfPath][TxCount] & BIT3)		/* 4bit sign number to 8 bit sign number */
						pwrInfo24G->BW40_Diff[rfPath][TxCount] |= 0xF0;
				}
				/*
				 * DBG_871X("8812-2G RF-%d-SS-%d BW40-Addr-%x DIFF=%d\n",
				 * rfPath, TxCount, eeAddr, pwrInfo24G->BW40_Diff[rfPath][TxCount]);
				 */


				{
					pwrInfo24G->BW20_Diff[rfPath][TxCount] =	(PROMContent[eeAddr]&0x0f);
					if (pwrInfo24G->BW20_Diff[rfPath][TxCount] & BIT3)		/* 4bit sign number to 8 bit sign number */
						pwrInfo24G->BW20_Diff[rfPath][TxCount] |= 0xF0;
				}
				/*
				 * DBG_871X("8812-2G RF-%d-SS-%d BW20-Addr-%x DIFF=%d\n",
				 * rfPath, TxCount, eeAddr, pwrInfo24G->BW20_Diff[rfPath][TxCount]);
				 */

				eeAddr++;


				{
					pwrInfo24G->OFDM_Diff[rfPath][TxCount] =	(PROMContent[eeAddr]&0xf0)>>4;
					if (pwrInfo24G->OFDM_Diff[rfPath][TxCount] & BIT3)		/* 4bit sign number to 8 bit sign number */
						pwrInfo24G->OFDM_Diff[rfPath][TxCount] |= 0xF0;
				}
				/*
				 * DBG_871X("8812-2G RF-%d-SS-%d LGOD-Addr-%x DIFF=%d\n",
				 * rfPath, TxCount, eeAddr, pwrInfo24G->BW20_Diff[rfPath][TxCount]);
				 */

				{
					pwrInfo24G->CCK_Diff[rfPath][TxCount] =	(PROMContent[eeAddr]&0x0f);
					if (pwrInfo24G->CCK_Diff[rfPath][TxCount] & BIT3)		/* 4bit sign number to 8 bit sign number */
						pwrInfo24G->CCK_Diff[rfPath][TxCount] |= 0xF0;
				}
				/*
				 * DBG_871X("8812-2G RF-%d-SS-%d CCK-Addr-%x DIFF=%d\n",
				 * rfPath, TxCount, eeAddr, pwrInfo24G->CCK_Diff[rfPath][TxCount]);
				 */

				eeAddr++;
			}
		}

		/* 5G default value */
		for (group = 0 ; group < MAX_CHNL_GROUP_5G; group++) {
			pwrInfo5G->IndexBW40_Base[rfPath][group] =		PROMContent[eeAddr++];
			if (pwrInfo5G->IndexBW40_Base[rfPath][group] == 0xFF)
				pwrInfo5G->IndexBW40_Base[rfPath][group] = EEPROM_DEFAULT_DIFF;

			/*
			 * DBG_871X("8812-5G RF-%d-G-%d BW40-Addr-%x BASE=%x\n",
			 * rfPath, TxCount, eeAddr-1, pwrInfo5G->IndexBW40_Base[rfPath][group]);
			 */
		}

		for (TxCount = 0; TxCount < MAX_TX_COUNT; TxCount++) {
			if (TxCount == 0) {
				pwrInfo5G->BW40_Diff[rfPath][TxCount] = 0;
				{
					pwrInfo5G->BW20_Diff[rfPath][0] = (PROMContent[eeAddr] & 0xf0) >> 4;
					if (pwrInfo5G->BW20_Diff[rfPath][TxCount] & BIT3)		/* 4bit sign number to 8 bit sign number */
						pwrInfo5G->BW20_Diff[rfPath][TxCount] |= 0xF0;
				}
				/*
				 * DBG_871X("8812-5G RF-%d-SS-%d BW20-Addr-%x DIFF=%d\n",
				 * rfPath, TxCount, eeAddr, pwrInfo5G->BW20_Diff[rfPath][TxCount]);
				 */
				{
					pwrInfo5G->OFDM_Diff[rfPath][0] = (PROMContent[eeAddr] & 0x0f);
					if (pwrInfo5G->OFDM_Diff[rfPath][TxCount] & BIT3)		/* 4bit sign number to 8 bit sign number */
						pwrInfo5G->OFDM_Diff[rfPath][TxCount] |= 0xF0;
				}
				/*
				 * DBG_871X("8812-5G RF-%d-SS-%d LGOD-Addr-%x DIFF=%d\n",
				 * rfPath, TxCount, eeAddr, pwrInfo5G->OFDM_Diff[rfPath][TxCount]);
				 */

				eeAddr++;
			} else {
				{
					pwrInfo5G->BW40_Diff[rfPath][TxCount] = (PROMContent[eeAddr] & 0xf0) >> 4;
					if (pwrInfo5G->BW40_Diff[rfPath][TxCount] & BIT3)		/* 4bit sign number to 8 bit sign number */
						pwrInfo5G->BW40_Diff[rfPath][TxCount] |= 0xF0;
				}
				/*
				 * DBG_871X("8812-5G RF-%d-SS-%d BW40-Addr-%x DIFF=%d\n",
				 * rfPath, TxCount, eeAddr, pwrInfo5G->BW40_Diff[rfPath][TxCount]);
				 */

				{
					pwrInfo5G->BW20_Diff[rfPath][TxCount] = (PROMContent[eeAddr] & 0x0f);
					if (pwrInfo5G->BW20_Diff[rfPath][TxCount] & BIT3)		/* 4bit sign number to 8 bit sign number */
						pwrInfo5G->BW20_Diff[rfPath][TxCount] |= 0xF0;
				}
				/*
				 * DBG_871X("8812-5G RF-%d-SS-%d BW20-Addr-%x DIFF=%d\n",
				 * rfPath, TxCount, eeAddr, pwrInfo5G->BW20_Diff[rfPath][TxCount]);
				 */

				eeAddr++;
			}
		}


		{
			pwrInfo5G->OFDM_Diff[rfPath][1] =	(PROMContent[eeAddr] & 0xf0) >> 4;
			pwrInfo5G->OFDM_Diff[rfPath][2] =	(PROMContent[eeAddr] & 0x0f);
		}
		/*
		 * DBG_871X("8812-5G RF-%d-SS-%d LGOD-Addr-%x DIFF=%d\n",
		 * rfPath, 2, eeAddr, pwrInfo5G->OFDM_Diff[rfPath][2]);
		 */
		eeAddr++;


			pwrInfo5G->OFDM_Diff[rfPath][3] =	(PROMContent[eeAddr] & 0x0f);

		/*
		 * DBG_871X("8812-5G RF-%d-SS-%d LGOD-Addr-%x DIFF=%d\n",
		 * rfPath, 3, eeAddr, pwrInfo5G->OFDM_Diff[rfPath][3]);
		 */
		eeAddr++;

		for (TxCount = 1; TxCount < MAX_TX_COUNT; TxCount++) {
			if (pwrInfo5G->OFDM_Diff[rfPath][TxCount] & BIT3)		/* 4bit sign number to 8 bit sign number */
				pwrInfo5G->OFDM_Diff[rfPath][TxCount] |= 0xF0;

			/*
			 * DBG_871X("8812-5G RF-%d-SS-%d LGOD-Addr-%x DIFF=%d\n",
			 * rfPath, TxCount, eeAddr, pwrInfo5G->OFDM_Diff[rfPath][TxCount]);
			 */
		}

		for (TxCount = 0; TxCount < MAX_TX_COUNT; TxCount++) {
			{
				pwrInfo5G->BW80_Diff[rfPath][TxCount] =	(PROMContent[eeAddr] & 0xf0) >> 4;
				if (pwrInfo5G->BW80_Diff[rfPath][TxCount] & BIT3)		/* 4bit sign number to 8 bit sign number */
					pwrInfo5G->BW80_Diff[rfPath][TxCount] |= 0xF0;
			}
			/*
			 * DBG_871X("8812-5G RF-%d-SS-%d BW80-Addr-%x DIFF=%d\n",
			 * rfPath, TxCount, eeAddr, pwrInfo5G->BW80_Diff[rfPath][TxCount]);
			 */
			{
				pwrInfo5G->BW160_Diff[rfPath][TxCount] =	(PROMContent[eeAddr] & 0x0f);
				if (pwrInfo5G->BW160_Diff[rfPath][TxCount] & BIT3)		/* 4bit sign number to 8 bit sign number */
					pwrInfo5G->BW160_Diff[rfPath][TxCount] |= 0xF0;
			}
			/*
			 * DBG_871X("8812-5G RF-%d-SS-%d BW160-Addr-%x DIFF=%d\n",
			 * rfPath, TxCount, eeAddr, pwrInfo5G->BW160_Diff[rfPath][TxCount]);
			 */
			eeAddr++;
		}
	}

}

VOID Hal_EfuseParseBTCoexistInfo8812A(struct rtl_priv *Adapter, u8 *hwinfo,
	BOOLEAN	AutoLoadFail)
{
}

void Hal_EfuseParseIDCode8812A(struct rtl_priv *padapter, uint8_t *hwinfo)
{
	EEPROM_EFUSE_PRIV *pEEPROM = GET_EEPROM_EFUSE_PRIV(padapter);
	u16			EEPROMId;

	/*  Checl 0x8129 again for making sure autoload status!! */
	EEPROMId = le16_to_cpu(*((u16 *)hwinfo));
	if (EEPROMId != RTL_EEPROM_ID) {
		DBG_8192C("EEPROM ID(%#x) is invalid!!\n", EEPROMId);
		pEEPROM->bautoload_fail_flag = _TRUE;
	} else {
		pEEPROM->bautoload_fail_flag = _FALSE;
	}

	DBG_8192C("EEPROM ID=0x%04x\n", EEPROMId);
}

VOID Hal_ReadPROMVersion8812A(struct rtl_priv *Adapter, uint8_t *PROMContent,
	BOOLEAN AutoloadFail)
{
	 struct _rtw_hal	*pHalData = GET_HAL_DATA(Adapter);

	if (AutoloadFail) {
		pHalData->EEPROMVersion = EEPROM_Default_Version;
	} else{
		pHalData->EEPROMVersion = *(uint8_t *)&PROMContent[EEPROM_VERSION_8812];
		if (pHalData->EEPROMVersion == 0xFF)
			pHalData->EEPROMVersion = EEPROM_Default_Version;
	}
	/* DBG_871X("pHalData->EEPROMVersion is 0x%x\n", pHalData->EEPROMVersion); */
}

void Hal_ReadTxPowerInfo8812A(struct rtl_priv *Adapter, uint8_t *PROMContent,
	BOOLEAN	AutoLoadFail)
{
	 struct _rtw_hal	*pHalData = GET_HAL_DATA(Adapter);
	TxPowerInfo24G	pwrInfo24G;
	TxPowerInfo5G	pwrInfo5G;
	uint8_t	rfPath, ch, group, TxCount;
	uint8_t	channel5G[CHANNEL_MAX_NUMBER_5G] = {
		 36,  38,  40,  42,  44,  46,  48,  50,  52,  54,  56,  58,
		 60,  62,  64, 100, 102, 104, 106, 108, 110, 112, 114, 116,
		118, 120, 122, 124, 126, 128, 130, 132, 134, 136, 138, 140,
		142, 144, 149, 151, 153, 155, 157, 159, 161, 163, 165, 167,
		168, 169, 171, 173, 175, 177 };

	uint8_t	channel5G_80M[CHANNEL_MAX_NUMBER_5G_80M] = {
		42, 58, 106, 122, 138, 155, 171};

	hal_ReadPowerValueFromPROM8812A(Adapter, &pwrInfo24G, &pwrInfo5G, PROMContent, AutoLoadFail);

	/*
	 * if(!AutoLoadFail)
	 * 	pHalData->bTXPowerDataReadFromEEPORM = _TRUE;
	 */

	for (rfPath = 0; rfPath < MAX_RF_PATH; rfPath++) {
		for (ch = 0 ; ch < CHANNEL_MAX_NUMBER_2G; ch++) {
			Hal_GetChnlGroup8812A(ch+1, &group);

			if (ch == 14-1) {
				pHalData->Index24G_CCK_Base[rfPath][ch] = pwrInfo24G.IndexCCK_Base[rfPath][5];
				pHalData->Index24G_BW40_Base[rfPath][ch] = pwrInfo24G.IndexBW40_Base[rfPath][group];
			} else {
				pHalData->Index24G_CCK_Base[rfPath][ch] = pwrInfo24G.IndexCCK_Base[rfPath][group];
				pHalData->Index24G_BW40_Base[rfPath][ch] = pwrInfo24G.IndexBW40_Base[rfPath][group];
			}

			/*
			 * DBG_871X("======= Path %d, ChannelIndex %d, Group %d=======\n",rfPath,ch, group);
			 * DBG_871X("Index24G_CCK_Base[%d][%d] = 0x%x\n",rfPath,ch ,pHalData->Index24G_CCK_Base[rfPath][ch]);
			 * DBG_871X("Index24G_BW40_Base[%d][%d] = 0x%x\n",rfPath,ch,pHalData->Index24G_BW40_Base[rfPath][ch]);
			 */
		}

		for (ch = 0 ; ch < CHANNEL_MAX_NUMBER_5G; ch++) {
			Hal_GetChnlGroup8812A(channel5G[ch], &group);

			pHalData->Index5G_BW40_Base[rfPath][ch] = pwrInfo5G.IndexBW40_Base[rfPath][group];

			/*
			 * DBG_871X("======= Path %d, ChannelIndex %d, Group %d=======\n",rfPath,ch, group);
			 * DBG_871X("Index5G_BW40_Base[%d][%d] = 0x%x\n",rfPath,ch,pHalData->Index5G_BW40_Base[rfPath][ch]);
			 */
		}
		for (ch = 0 ; ch < CHANNEL_MAX_NUMBER_5G_80M; ch++) {
			uint8_t	upper, lower;

			Hal_GetChnlGroup8812A(channel5G_80M[ch], &group);
			upper = pwrInfo5G.IndexBW40_Base[rfPath][group];
			lower = pwrInfo5G.IndexBW40_Base[rfPath][group+1];

			pHalData->Index5G_BW80_Base[rfPath][ch] = (upper + lower) / 2;

			/*
			 * DBG_871X("======= Path %d, ChannelIndex %d, Group %d=======\n",rfPath,ch, group);
			 * DBG_871X("Index5G_BW80_Base[%d][%d] = 0x%x\n",rfPath,ch,pHalData->Index5G_BW80_Base[rfPath][ch]);
			 */
		}

		for (TxCount = 0; TxCount < MAX_TX_COUNT; TxCount++) {
			pHalData->CCK_24G_Diff[rfPath][TxCount]  = pwrInfo24G.CCK_Diff[rfPath][TxCount];
			pHalData->OFDM_24G_Diff[rfPath][TxCount] = pwrInfo24G.OFDM_Diff[rfPath][TxCount];
			pHalData->BW20_24G_Diff[rfPath][TxCount] = pwrInfo24G.BW20_Diff[rfPath][TxCount];
			pHalData->BW40_24G_Diff[rfPath][TxCount] = pwrInfo24G.BW40_Diff[rfPath][TxCount];

			pHalData->OFDM_5G_Diff[rfPath][TxCount] = pwrInfo5G.OFDM_Diff[rfPath][TxCount];
			pHalData->BW20_5G_Diff[rfPath][TxCount] = pwrInfo5G.BW20_Diff[rfPath][TxCount];
			pHalData->BW40_5G_Diff[rfPath][TxCount] = pwrInfo5G.BW40_Diff[rfPath][TxCount];
			pHalData->BW80_5G_Diff[rfPath][TxCount] = pwrInfo5G.BW80_Diff[rfPath][TxCount];
/* #if DBG */
		}
	}


	/* 2010/10/19 MH Add Regulator recognize for CU. */
	if (!AutoLoadFail) {
		struct registry_priv  *registry_par = &Adapter->registrypriv;
		if (registry_par->regulatory_tid == 0xff) {

			if (PROMContent[EEPROM_RF_BOARD_OPTION_8812] == 0xFF)
				pHalData->EEPROMRegulatory = (EEPROM_DEFAULT_BOARD_OPTION&0x7);	/* bit0~2 */
			else
				pHalData->EEPROMRegulatory = (PROMContent[EEPROM_RF_BOARD_OPTION_8812]&0x7);	/* bit0~2 */
		} else{
			pHalData->EEPROMRegulatory = registry_par->regulatory_tid;
		}

		/* 2012/09/26 MH Add for TX power calibrate rate. */
		pHalData->TxPwrCalibrateRate = PROMContent[EEPROM_TX_PWR_CALIBRATE_RATE_8812];
	} else {
		pHalData->EEPROMRegulatory = 0;
		/* 2012/09/26 MH Add for TX power calibrate rate. */
		pHalData->TxPwrCalibrateRate = EEPROM_DEFAULT_TX_CALIBRATE_RATE;
	}
	DBG_871X("EEPROMRegulatory = 0x%x TxPwrCalibrateRate=0x%x\n", pHalData->EEPROMRegulatory, pHalData->TxPwrCalibrateRate);

}

VOID Hal_ReadBoardType8812A(struct rtl_priv *Adapter, uint8_t *PROMContent,
	BOOLEAN	AutoloadFail)
{
	 struct _rtw_hal	*pHalData = GET_HAL_DATA(Adapter);

	if (!AutoloadFail) {
		pHalData->InterfaceSel = (PROMContent[EEPROM_RF_BOARD_OPTION_8812]&0xE0)>>5;
		if (PROMContent[EEPROM_RF_BOARD_OPTION_8812] == 0xFF)
			pHalData->InterfaceSel = (EEPROM_DEFAULT_BOARD_OPTION&0xE0)>>5;
	} else {
		pHalData->InterfaceSel = 0;
	}
	DBG_871X("Board Type: 0x%2x\n", pHalData->InterfaceSel);

}

VOID Hal_ReadThermalMeter_8812A(struct rtl_priv *Adapter, uint8_t *PROMContent,
	BOOLEAN	AutoloadFail)
{
	 struct _rtw_hal	*pHalData = GET_HAL_DATA(Adapter);
	/* uint8_t	tempval; */

	/*
	 * ThermalMeter from EEPROM
	 */
	if (!AutoloadFail)
		pHalData->EEPROMThermalMeter = PROMContent[EEPROM_THERMAL_METER_8812];
	else
		pHalData->EEPROMThermalMeter = EEPROM_Default_ThermalMeter_8812;
	/* pHalData->EEPROMThermalMeter = (tempval&0x1f);	//[4:0] */

	if (pHalData->EEPROMThermalMeter == 0xff || AutoloadFail) {
		pHalData->bAPKThermalMeterIgnore = _TRUE;
		pHalData->EEPROMThermalMeter = 0xFF;
	}

	/* pHalData->ThermalMeter[0] = pHalData->EEPROMThermalMeter; */
	DBG_871X("ThermalMeter = 0x%x\n", pHalData->EEPROMThermalMeter);
}

VOID Hal_ReadChannelPlan8812A(struct rtl_priv *padapter, uint8_t *hwinfo,
	BOOLEAN	AutoLoadFail)
{
	padapter->mlmepriv.ChannelPlan = hal_com_get_channel_plan(
		padapter
		, hwinfo?hwinfo[EEPROM_ChannelPlan_8812]:0xFF
		, padapter->registrypriv.channel_plan
		, RT_CHANNEL_DOMAIN_REALTEK_DEFINE
		, AutoLoadFail
	);

	DBG_871X("mlmepriv.ChannelPlan = 0x%02x\n", padapter->mlmepriv.ChannelPlan);
}

VOID Hal_EfuseParseXtal_8812A(struct rtl_priv *pAdapter, uint8_t *hwinfo,
	BOOLEAN	AutoLoadFail)
{
	 struct _rtw_hal	*pHalData = GET_HAL_DATA(pAdapter);

	if (!AutoLoadFail) {
		pHalData->CrystalCap = hwinfo[EEPROM_XTAL_8812];
		if (pHalData->CrystalCap == 0xFF)
			pHalData->CrystalCap = EEPROM_Default_CrystalCap_8812;	 /* what value should 8812 set? */
	} else {
		pHalData->CrystalCap = EEPROM_Default_CrystalCap_8812;
	}
	DBG_871X("CrystalCap: 0x%2x\n", pHalData->CrystalCap);
}

VOID Hal_ReadAntennaDiversity8812A(IN struct rtl_priv *pAdapter,
	uint8_t *PROMContent, BOOLEAN AutoLoadFail)
{
	 struct _rtw_hal	*pHalData = GET_HAL_DATA(pAdapter);
	struct registry_priv	*registry_par = &pAdapter->registrypriv;

	if (!AutoLoadFail) {
		/*  Antenna Diversity setting. */
		if (registry_par->antdiv_cfg == 2) {
			pHalData->AntDivCfg = (PROMContent[EEPROM_RF_BOARD_OPTION_8812]&0x18)>>3;
			if (PROMContent[EEPROM_RF_BOARD_OPTION_8812] == 0xFF)
				pHalData->AntDivCfg = (EEPROM_DEFAULT_BOARD_OPTION&0x18)>>3;;
		} else {
			pHalData->AntDivCfg = registry_par->antdiv_cfg;
		}

		pHalData->TRxAntDivType = PROMContent[EEPROM_RF_ANTENNA_OPT_8812];  /* todo by page */
		if (pHalData->TRxAntDivType == 0xFF)
			pHalData->TRxAntDivType = FIXED_HW_ANTDIV; /* For 88EE, 1Tx and 1RxCG are fixed.(1Ant, Tx and RxCG are both on aux port) */
	} else {
		pHalData->AntDivCfg = 0;
		/* pHalData->TRxAntDivType = pHalData->TRxAntDivType; // ????? */
	}

	DBG_871X("SWAS: bHwAntDiv = %x, TRxAntDivType = %x\n", pHalData->AntDivCfg, pHalData->TRxAntDivType);
}

VOID
Hal_ReadPAType_8812A(struct rtl_priv *Adapter, uint8_t *PROMContent,
	BOOLEAN	AutoloadFail)
{
	struct rtl_hal *rtlhal = rtl_hal(Adapter);
	struct _rtw_hal		*pHalData = GET_HAL_DATA(Adapter);

	if (!AutoloadFail) {
		if (GetRegAmplifierType2G(Adapter) == 0) {
			/* AUTO */
			rtlhal->pa_type_2g = EF1Byte(*(uint8_t *)&PROMContent[EEPROM_PA_TYPE_8812AU]);
			rtlhal->lna_type_2g = EF1Byte(*(uint8_t *)&PROMContent[EEPROM_LNA_TYPE_2G_8812AU]);
			if (rtlhal->pa_type_2g == 0xFF && rtlhal->lna_type_2g == 0xFF) {
				rtlhal->pa_type_2g = 0;
				rtlhal->lna_type_2g = 0;
			}
			rtlhal->external_pa_2g = ((rtlhal->pa_type_2g & BIT5) && (rtlhal->pa_type_2g & BIT4)) ? 1 : 0;
			pHalData->ExternalLNA_2G = ((rtlhal->lna_type_2g & BIT7) && (rtlhal->lna_type_2g & BIT3)) ? 1 : 0;
		} else 	{
			rtlhal->external_pa_2g  = (GetRegAmplifierType2G(Adapter)&ODM_BOARD_EXT_PA)  ? 1 : 0;
			pHalData->ExternalLNA_2G = (GetRegAmplifierType2G(Adapter)&ODM_BOARD_EXT_LNA) ? 1 : 0;
		}

		if (GetRegAmplifierType5G(Adapter) == 0) {
			/* AUTO */
			rtlhal->pa_type_5g = EF1Byte(*(uint8_t *)&PROMContent[EEPROM_PA_TYPE_8812AU]);
			rtlhal->lna_type_5g = EF1Byte(*(uint8_t *)&PROMContent[EEPROM_LNA_TYPE_5G_8812AU]);
			if (rtlhal->pa_type_5g == 0xFF && rtlhal->lna_type_5g == 0xFF) {
				rtlhal->pa_type_5g = 0;
				rtlhal->lna_type_5g = 0;
			}
			rtlhal->external_pa_5g = ((rtlhal->pa_type_5g & BIT1) && (rtlhal->pa_type_5g & BIT0)) ? 1 : 0;
			pHalData->ExternalLNA_5G = ((rtlhal->lna_type_5g & BIT7) && (rtlhal->lna_type_5g & BIT3)) ? 1 : 0;
		} else {
			rtlhal->external_pa_5g  = (GetRegAmplifierType5G(Adapter)&ODM_BOARD_EXT_PA_5G)  ? 1 : 0;
			pHalData->ExternalLNA_5G = (GetRegAmplifierType5G(Adapter)&ODM_BOARD_EXT_LNA_5G) ? 1 : 0;
		}
	} else {
		rtlhal->external_pa_2g  = EEPROM_Default_PAType;
		rtlhal->external_pa_5g  = 0xFF;
		pHalData->ExternalLNA_2G = EEPROM_Default_LNAType;
		pHalData->ExternalLNA_5G = 0xFF;

		if (GetRegAmplifierType2G(Adapter) == 0) {
			/* AUTO */
			rtlhal->external_pa_2g  = 0;
			pHalData->ExternalLNA_2G = 0;
		} else {
			rtlhal->external_pa_2g  = (GetRegAmplifierType2G(Adapter)&ODM_BOARD_EXT_PA)  ? 1 : 0;
			pHalData->ExternalLNA_2G = (GetRegAmplifierType2G(Adapter)&ODM_BOARD_EXT_LNA) ? 1 : 0;
		}
		if (GetRegAmplifierType5G(Adapter) == 0) {
			/* AUTO */
			rtlhal->external_pa_5g  = 0;
			pHalData->ExternalLNA_5G = 0;
		} else 	{
			rtlhal->external_pa_5g  = (GetRegAmplifierType5G(Adapter)&ODM_BOARD_EXT_PA_5G)  ? 1 : 0;
			pHalData->ExternalLNA_5G = (GetRegAmplifierType5G(Adapter)&ODM_BOARD_EXT_LNA_5G) ? 1 : 0;
		}
	}
	DBG_871X("pHalData->PAType_2G is 0x%x, pHalData->ExternalPA_2G = %d\n", rtlhal->pa_type_2g, rtlhal->external_pa_2g);
	DBG_871X("pHalData->PAType_5G is 0x%x, pHalData->ExternalPA_5G = %d\n", rtlhal->pa_type_5g, rtlhal->external_pa_5g);
	DBG_871X("pHalData->LNAType_2G is 0x%x, pHalData->ExternalLNA_2G = %d\n", rtlhal->lna_type_2g, pHalData->ExternalLNA_2G);
	DBG_871X("pHalData->LNAType_5G is 0x%x, pHalData->ExternalLNA_5G = %d\n", rtlhal->lna_type_5g, pHalData->ExternalLNA_5G);
}

VOID
Hal_ReadPAType_8821A(struct rtl_priv *Adapter, uint8_t *PROMContent,
	BOOLEAN	 AutoloadFail)
{
	struct rtl_hal *rtlhal = rtl_hal(Adapter);
	struct _rtw_hal	*pHalData = GET_HAL_DATA(Adapter);

	if (!AutoloadFail) {
		if (GetRegAmplifierType2G(Adapter) == 0) {
			/* AUTO */

			rtlhal->pa_type_2g = EF1Byte(*(uint8_t *) &PROMContent[EEPROM_PA_TYPE_8812AU]);
			rtlhal->lna_type_2g = EF1Byte(*(uint8_t *) &PROMContent[EEPROM_LNA_TYPE_2G_8812AU]);
			if (rtlhal->pa_type_2g == 0xFF && rtlhal->lna_type_2g == 0xFF) {
				rtlhal->pa_type_2g = 0;
				rtlhal->lna_type_2g = 0;
			}
			rtlhal->external_pa_2g = (rtlhal->pa_type_2g & BIT4) ? 1 : 0;
			pHalData->ExternalLNA_2G = (rtlhal->lna_type_2g & BIT3) ? 1 : 0;
		} else {
			rtlhal->external_pa_2g  = (GetRegAmplifierType2G(Adapter)&ODM_BOARD_EXT_PA)  ? 1 : 0;
			pHalData->ExternalLNA_2G = (GetRegAmplifierType2G(Adapter)&ODM_BOARD_EXT_LNA) ? 1 : 0;
		}

		if (GetRegAmplifierType5G(Adapter) == 0) {
			/* AUTO */
			rtlhal->pa_type_5g = EF1Byte(*(uint8_t *) &PROMContent[EEPROM_PA_TYPE_8812AU]);
			rtlhal->lna_type_5g = EF1Byte(*(uint8_t *) &PROMContent[EEPROM_LNA_TYPE_5G_8812AU]);
			if (rtlhal->pa_type_5g == 0xFF && rtlhal->lna_type_5g == 0xFF) {
				rtlhal->pa_type_5g = 0;
				rtlhal->lna_type_5g = 0;
			}
			rtlhal->external_pa_5g = (rtlhal->pa_type_5g & BIT0) ? 1 : 0;
			pHalData->ExternalLNA_5G = (rtlhal->lna_type_5g & BIT3) ? 1 : 0;
		} else {
			rtlhal->external_pa_5g  = (GetRegAmplifierType5G(Adapter)&ODM_BOARD_EXT_PA_5G)  ? 1 : 0;
			pHalData->ExternalLNA_5G = (GetRegAmplifierType5G(Adapter)&ODM_BOARD_EXT_LNA_5G) ? 1 : 0;
		}
	} else {
		rtlhal->external_pa_2g  = EEPROM_Default_PAType;
		rtlhal->external_pa_5g  = 0xFF;
		pHalData->ExternalLNA_2G = EEPROM_Default_LNAType;
		pHalData->ExternalLNA_5G = 0xFF;

		if (GetRegAmplifierType2G(Adapter) == 0) {
			/* AUTO */
			rtlhal->external_pa_2g  = 0;
			pHalData->ExternalLNA_2G = 0;
		} else {
			rtlhal->external_pa_2g  = (GetRegAmplifierType2G(Adapter)&ODM_BOARD_EXT_PA)  ? 1 : 0;
			pHalData->ExternalLNA_2G = (GetRegAmplifierType2G(Adapter)&ODM_BOARD_EXT_LNA) ? 1 : 0;
		}
		if (GetRegAmplifierType5G(Adapter) == 0) {
			/* AUTO */
			rtlhal->external_pa_5g  = 0;
			pHalData->ExternalLNA_5G = 0;
		} else {
			rtlhal->external_pa_5g  = (GetRegAmplifierType5G(Adapter)&ODM_BOARD_EXT_PA_5G)  ? 1 : 0;
			pHalData->ExternalLNA_5G = (GetRegAmplifierType5G(Adapter)&ODM_BOARD_EXT_LNA_5G) ? 1 : 0;
		}
	}
	DBG_871X("pHalData->PAType_2G is 0x%x, pHalData->ExternalPA_2G = %d\n", rtlhal->pa_type_2g, rtlhal->external_pa_2g);
	DBG_871X("pHalData->PAType_5G is 0x%x, pHalData->ExternalPA_5G = %d\n", rtlhal->pa_type_5g, rtlhal->external_pa_5g);
	DBG_871X("pHalData->LNAType_2G is 0x%x, pHalData->ExternalLNA_2G = %d\n", rtlhal->lna_type_2g, pHalData->ExternalLNA_2G);
	DBG_871X("pHalData->LNAType_5G is 0x%x, pHalData->ExternalLNA_5G = %d\n", rtlhal->lna_type_5g, pHalData->ExternalLNA_5G);
}

VOID
Hal_ReadRFEType_8812A(struct rtl_priv *Adapter, uint8_t *PROMContent,
	BOOLEAN	AutoloadFail)
{
	struct rtl_hal *rtlhal = rtl_hal(Adapter);
	 struct _rtw_hal	*pHalData = GET_HAL_DATA(Adapter);

	if (!AutoloadFail) {
		if (GetRegRFEType(Adapter) != 64)
			pHalData->RFEType = GetRegRFEType(Adapter);
		else if (PROMContent[EEPROM_RFE_OPTION_8812] & BIT7) {
			if (pHalData->ExternalLNA_5G) {
				if (rtlhal->external_pa_5g) {
					if (pHalData->ExternalLNA_2G && rtlhal->external_pa_2g)
						pHalData->RFEType = 3;
					else
						pHalData->RFEType = 0;
				} else
					pHalData->RFEType = 2;
			} else
				pHalData->RFEType = 4;
		} else {
			pHalData->RFEType = PROMContent[EEPROM_RFE_OPTION_8812]&0x3F;

			/*
			 * 2013/03/19 MH Due to othe customer already use incorrect EFUSE map
			 * to for their product. We need to add workaround to prevent to modify
			 * spec and notify all customer to revise the IC 0xca content. After
			 * discussing with Willis an YN, revise driver code to prevent.
			 */
			if (pHalData->RFEType == 4 &&
			   (rtlhal->external_pa_5g == _TRUE || rtlhal->external_pa_2g == _TRUE ||
			    pHalData->ExternalLNA_5G == _TRUE || pHalData->ExternalLNA_2G == _TRUE)) {
				if (IS_HARDWARE_TYPE_8812AU(rtlhal))
					pHalData->RFEType = 0;
				else if (IS_HARDWARE_TYPE_8812E(rtlhal))
					pHalData->RFEType = 2;
			}
		}
	} else {
		if (GetRegRFEType(Adapter) != 64)
			pHalData->RFEType = GetRegRFEType(Adapter);
		else
			pHalData->RFEType = EEPROM_DEFAULT_RFE_OPTION;
	}

	DBG_871X("RFE Type: 0x%2x\n", pHalData->RFEType);
}

/*
 * 2013/04/15 MH Add 8812AU- VL/VS/VN for different board type.
 */
VOID
hal_ReadUsbType_8812AU(struct rtl_priv *Adapter, uint8_t *PROMContent,
	BOOLEAN AutoloadFail)
{
	/* if (IS_HARDWARE_TYPE_8812AU(Adapter) && Adapter->UsbModeMechanism.RegForcedUsbMode == 5) */
	{
		struct _rtw_hal *pHalData = GET_HAL_DATA(Adapter);
		uint8_t	reg_tmp, i, j, antenna = 0, wmode = 0;
		/* Read anenna type from EFUSE 1019/1018 */
		for (i = 0; i < 2; i++) {
			/* Check efuse address 1019 */
			/* Check efuse address 1018 */
			efuse_OneByteRead(Adapter, 1019-i, &reg_tmp);

			for (j = 0; j < 2; j++) {
				/* CHeck bit 7-5 */
				/* Check bit 3-1 */
				antenna = ((reg_tmp&0xee) >> (5-(j*4)));
				if (antenna == 0)
					continue;
				else {
					break;
				}
			}
		}

		/* Read anenna type from EFUSE 1021/1020 */
		for (i = 0; i < 2; i++) {
			/* Check efuse address 1019 */
			/* Check efuse address 1018 */
			efuse_OneByteRead(Adapter, 1021-i, &reg_tmp);

			for (j = 0; j < 2; j++) {
				/* CHeck bit 3-2 */
				/* Check bit 1-0 */
				wmode = ((reg_tmp&0x0f) >> (2-(j*2)));
				if (wmode)
					continue;
				else {
					break;
				}
			}
		}

		/* Ulli Antenna Mode */
		/* Antenna == 1 WMODE = 3 RTL8812AU-VL 11AC + USB2.0 Mode */
		if (antenna == 1) {
			/* Config 8812AU as 1*1 mode AC mode. */
			Adapter->phy.rf_type = RF_1T1R;
			/* UsbModeSwitch_SetUsbModeMechOn(Adapter, FALSE); */
			/* pHalData->EFUSEHidden = EFUSE_HIDDEN_812AU_VL; */
			DBG_871X("%s(): EFUSE_HIDDEN_812AU_VL\n", __FUNCTION__);
		} else if (antenna == 2) {
			if (wmode == 3) {
				if (PROMContent[EEPROM_USB_MODE_8812] == 0x2) {
					/*
					 * RTL8812AU Normal Mode. No further action.
					 * pHalData->EFUSEHidden = EFUSE_HIDDEN_812AU;
					 */
					DBG_871X("%s(): EFUSE_HIDDEN_812AU\n", __FUNCTION__);
				} else {
					/*
					 * Antenna == 2 WMODE = 3 RTL8812AU-VS 11AC + USB2.0 Mode
					 * Driver will not support USB automatic switch
					 * UsbModeSwitch_SetUsbModeMechOn(Adapter, FALSE);
					 * pHalData->EFUSEHidden = EFUSE_HIDDEN_812AU_VS;
					 */
					DBG_871X("%s(): EFUSE_HIDDEN_812AU_VS\n", __FUNCTION__);
				}
			} else
				if (wmode == 2) {
				/*
				 * Antenna == 2 WMODE = 2 RTL8812AU-VN 11N only + USB2.0 Mode
				 * UsbModeSwitch_SetUsbModeMechOn(Adapter, FALSE);
				 * pHalData->EFUSEHidden = EFUSE_HIDDEN_812AU_VN;
				 */
				DBG_871X("%s(): EFUSE_HIDDEN_812AU_VN\n", __FUNCTION__);
			}
		}
	}
}

enum {
	VOLTAGE_V25	= 0x03,
	LDOE25_SHIFT	= 28 ,
	};

/*
 * ULLI need to refactoring for rtlwifi-lib
 * static bool _rt8812au_phy_set_rf_power_state(struct ieee80211_hw *hw,
 * 					    enum rf_pwrstate rfpwr_state)
 *
 */

VOID
rtl8812_EfusePowerSwitch(struct rtl_priv *pAdapter, uint8_t bWrite, uint8_t PwrState)
{
	uint8_t	tempval;
	u16	tmpV16;
#define EFUSE_ACCESS_ON_JAGUAR 0x69
#define EFUSE_ACCESS_OFF_JAGUAR 0x00
	if (PwrState == _TRUE) {
		rtl_write_byte(pAdapter, REG_EFUSE_BURN_GNT_8812, EFUSE_ACCESS_ON_JAGUAR);

		/* 1.2V Power: From VDDON with Power Cut(0x0000h[15]), defualt valid */
		tmpV16 = rtl_read_word(pAdapter, REG_SYS_ISO_CTRL);
		if (!(tmpV16 & PWC_EV12V)) {
			tmpV16 |= PWC_EV12V ;
			/* rtl_write_word(pAdapter,REG_SYS_ISO_CTRL,tmpV16); */
		}
		/* Reset: 0x0000h[28], default valid */
		tmpV16 =  rtl_read_word(pAdapter, REG_SYS_FUNC_EN);
		if (!(tmpV16 & FEN_ELDR)) {
			tmpV16 |= FEN_ELDR ;
			rtl_write_word(pAdapter, REG_SYS_FUNC_EN, tmpV16);
		}

		/* Clock: Gated(0x0008h[5]) 8M(0x0008h[1]) clock from ANA, default valid */
		tmpV16 = rtl_read_word(pAdapter, REG_SYS_CLKR);
		if ((!(tmpV16 & LOADER_CLK_EN)) || (!(tmpV16 & ANA8M))) {
			tmpV16 |= (LOADER_CLK_EN | ANA8M);
			rtl_write_word(pAdapter, REG_SYS_CLKR, tmpV16);
		}

		if (bWrite == _TRUE) {
			/* Enable LDO 2.5V before read/write action */
			tempval = rtl_read_byte(pAdapter, EFUSE_TEST+3);
			tempval &= ~(BIT3|BIT4|BIT5|BIT6);
			tempval |= (VOLTAGE_V25 << 3);
			tempval |= BIT7;
			rtl_write_byte(pAdapter, EFUSE_TEST + 3, tempval);
		}
	} else {
		rtl_write_byte(pAdapter, REG_EFUSE_BURN_GNT_8812, EFUSE_ACCESS_OFF_JAGUAR);

		if (bWrite == _TRUE) {
			/* Disable LDO 2.5V after read/write action */
			tempval = rtl_read_byte(pAdapter, EFUSE_TEST + 3);
			rtl_write_byte(pAdapter, EFUSE_TEST + 3, (tempval & 0x7F));
		}
	}
}

static BOOLEAN
Hal_EfuseSwitchToBank8812A(struct rtl_priv *pAdapter, u8 bank)
{
	return _FALSE;
}

static VOID
Hal_EfuseReadEFuse8812A(struct rtl_priv *Adapter, u16 _offset,
	u16 _size_byte, uint8_t *pbuf)
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
		DBG_8192C("Hal_EfuseReadEFuse8812A(): Invalid offset(%#x) with read bytes(%#x)!!\n", _offset, _size_byte);
		goto exit;
	}

	efuseTbl = (uint8_t *) rtw_zmalloc(EFUSE_MAP_LEN_JAGUAR);
	if (efuseTbl == NULL) {
		DBG_871X("%s: alloc efuseTbl fail!\n", __FUNCTION__);
		goto exit;
	}

	eFuseWord = (u16 **) rtw_malloc2d(EFUSE_MAX_SECTION_JAGUAR, EFUSE_MAX_WORD_UNIT, sizeof(u16));
	if (eFuseWord == NULL) {
		DBG_871X("%s: alloc eFuseWord fail!\n", __FUNCTION__);
		goto exit;
	}

	/* 0. Refresh efuse init map as all oxFF. */
	for (i = 0; i < EFUSE_MAX_SECTION_JAGUAR; i++)
		for (j = 0; j < EFUSE_MAX_WORD_UNIT; j++)
			eFuseWord[i][j] = 0xFFFF;

	/*
	 * 1. Read the first byte to check if efuse is empty!!!
	 */
	efuse_OneByteRead(Adapter, eFuse_Addr++, &efuseHeader);

	if (efuseHeader != 0xFF) {
		efuse_utilized++;
	} else {
		DBG_871X("EFUSE is empty\n");
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

			efuse_OneByteRead(Adapter, eFuse_Addr++, &efuseExtHdr);

			/* RT_DISP(FEEPROM, EFUSE_READ_ALL, ("efuse[%X]=%X\n", eFuse_Addr-1, efuseExtHdr)); */

			if (efuseExtHdr != 0xff) {
				efuse_utilized++;
				if (ALL_WORDS_DISABLED(efuseExtHdr)) {
					efuse_OneByteRead(Adapter, eFuse_Addr++, &efuseHeader);
					if (efuseHeader != 0xff) {
						efuse_utilized++;
					}
					break;
				} else {
					offset = ((efuseExtHdr & 0xF0) >> 1) | offset_2_0;
					wden = (efuseExtHdr & 0x0F);
				}
			} else 	{
				DBG_871X("Error condition, extended = 0xff\n");
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
					efuse_OneByteRead(Adapter, eFuse_Addr++, &efuseData);
					/* RT_DISP(FEEPROM, EFUSE_READ_ALL, ("efuse[%X]=%X\n", eFuse_Addr-1, efuseData)); */
					efuse_utilized++;
					eFuseWord[offset][i] = (efuseData & 0xff);

					if (!AVAILABLE_EFUSE_ADDR_8812(eFuse_Addr))
						break;

					efuse_OneByteRead(Adapter, eFuse_Addr++, &efuseData);
					/* RT_DISP(FEEPROM, EFUSE_READ_ALL, ("efuse[%X]=%X\n", eFuse_Addr-1, efuseData)); */
					efuse_utilized++;
					eFuseWord[offset][i] |= (((u16)efuseData << 8) & 0xff00);

					if (!AVAILABLE_EFUSE_ADDR_8812(eFuse_Addr))
						break;
				}
			}
		}

		/* Read next PG header */
		efuse_OneByteRead(Adapter, eFuse_Addr++, &efuseHeader);
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
	rtw_hal_set_hwreg(Adapter, HW_VAR_EFUSE_BYTES, (uint8_t *)&eFuse_Addr);

exit:
	if (efuseTbl)
		rtw_mfree(efuseTbl);

	if (eFuseWord)
		rtw_mfree2d((void *)eFuseWord, EFUSE_MAX_SECTION_JAGUAR, EFUSE_MAX_WORD_UNIT, sizeof(u16));
}

VOID
rtl8812_ReadEFuse(struct rtl_priv *Adapter, uint8_t efuseType, u16	_offset,
	u16 _size_byte, uint8_t *pbuf)
{
	Hal_EfuseReadEFuse8812A(Adapter, _offset, _size_byte, pbuf);
}

/* Do not support BT */
static VOID Hal_EFUSEGetEfuseDefinition8812A(struct rtl_priv *pAdapter,
	u8 efuseType, u8 type, PVOID pOut)
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

VOID rtl8812_EFUSE_GetEfuseDefinition(struct rtl_priv *pAdapter, uint8_t efuseType,
	uint8_t	 type, void *pOut)
{
	Hal_EFUSEGetEfuseDefinition8812A(pAdapter, efuseType, type, pOut);
}

static u8 Hal_EfuseWordEnableDataWrite8812A(struct rtl_priv *pAdapter,
	u16 efuse_addr, uint8_t word_en, uint8_t *data)
{
	u16 tmpaddr = 0;
	u16 start_addr = efuse_addr;
	uint8_t	badworden = 0x0F;
	uint8_t	tmpdata[8];

	memset((PVOID)tmpdata, 0xff, PGPKT_DATA_SIZE);

	/*
	 * RT_TRACE(COMP_EFUSE, DBG_LOUD, ("word_en = %x efuse_addr=%x\n", word_en, efuse_addr));
	 */

	if (!(word_en & BIT0)) {
		tmpaddr = start_addr;
		efuse_OneByteWrite(pAdapter, start_addr++, data[0]);
		efuse_OneByteWrite(pAdapter, start_addr++, data[1]);

		efuse_OneByteRead(pAdapter, tmpaddr, &tmpdata[0]);
		efuse_OneByteRead(pAdapter, tmpaddr+1, &tmpdata[1]);
		if ((data[0] != tmpdata[0]) || (data[1] != tmpdata[1])) {
			badworden &= (~BIT0);
		}
	}
	if (!(word_en & BIT1)) {
		tmpaddr = start_addr;
		efuse_OneByteWrite(pAdapter, start_addr++, data[2]);
		efuse_OneByteWrite(pAdapter, start_addr++, data[3]);

		efuse_OneByteRead(pAdapter, tmpaddr    , &tmpdata[2]);
		efuse_OneByteRead(pAdapter, tmpaddr+1, &tmpdata[3]);
		if ((data[2] != tmpdata[2]) || (data[3] != tmpdata[3])) {
			badworden &= (~BIT1);
		}
	}
	if (!(word_en & BIT2)) {
		tmpaddr = start_addr;
		efuse_OneByteWrite(pAdapter, start_addr++, data[4]);
		efuse_OneByteWrite(pAdapter, start_addr++, data[5]);

		efuse_OneByteRead(pAdapter, tmpaddr, &tmpdata[4]);
		efuse_OneByteRead(pAdapter, tmpaddr+1, &tmpdata[5]);
		if ((data[4] != tmpdata[4]) || (data[5] != tmpdata[5])) {
			badworden &= (~BIT2);
		}
	}
	if (!(word_en & BIT3)) {
		tmpaddr = start_addr;
		efuse_OneByteWrite(pAdapter, start_addr++, data[6]);
		efuse_OneByteWrite(pAdapter, start_addr++, data[7]);

		efuse_OneByteRead(pAdapter, tmpaddr, &tmpdata[6]);
		efuse_OneByteRead(pAdapter, tmpaddr+1, &tmpdata[7]);
		if ((data[6] != tmpdata[6]) || (data[7] != tmpdata[7])) {
			badworden &= (~BIT3);
		}
	}
	return badworden;
}

u8 rtl8812_Efuse_WordEnableDataWrite(struct rtl_priv *pAdapter,
	u16 efuse_addr, uint8_t word_en, uint8_t *data)
{
	uint8_t	ret = 0;

	ret = Hal_EfuseWordEnableDataWrite8812A(pAdapter, efuse_addr, word_en, data);

	return ret;
}


static u16 hal_EfuseGetCurrentSize_8812A(struct rtl_priv *pAdapter)
{
	int	bContinual = _TRUE;
	u16	efuse_addr = 0;
	uint8_t	hoffset = 0, hworden = 0;
	uint8_t	efuse_data, word_cnts = 0;

	rtw_hal_get_hwreg(pAdapter, HW_VAR_EFUSE_BYTES, (uint8_t *)&efuse_addr);

	/* RTPRINT(FEEPROM, EFUSE_PG, ("hal_EfuseGetCurrentSize_8723A(), start_efuse_addr = %d\n", efuse_addr)); */

	while (bContinual && efuse_OneByteRead(pAdapter, efuse_addr, &efuse_data)
	     && (efuse_addr  < EFUSE_REAL_CONTENT_LEN_JAGUAR)) {
		if (efuse_data != 0xFF) {
			if ((efuse_data & 0x1F) == 0x0F) {	/* extended header */
				hoffset = efuse_data;
				efuse_addr++;
				efuse_OneByteRead(pAdapter, efuse_addr, &efuse_data);
				if ((efuse_data & 0x0F) == 0x0F) {
					efuse_addr++;
					continue;
				} else {
					hoffset = ((hoffset & 0xE0) >> 5) | ((efuse_data & 0xF0) >> 1);
					hworden = efuse_data & 0x0F;
				}
			} else {
				hoffset = (efuse_data >> 4) & 0x0F;
				hworden =  efuse_data & 0x0F;
			}
			word_cnts = Efuse_CalculateWordCnts(hworden);
			/* read next header */
			efuse_addr = efuse_addr + (word_cnts*2)+1;
		} else {
			bContinual = _FALSE ;
		}
	}

	rtw_hal_set_hwreg(pAdapter, HW_VAR_EFUSE_BYTES, (uint8_t *)&efuse_addr);

	return efuse_addr;
}

u16 rtl8812_EfuseGetCurrentSize(struct rtl_priv *pAdapter, uint8_t efuseType)
{
	u16 ret = 0;

	ret = hal_EfuseGetCurrentSize_8812A(pAdapter);

	return ret;
}


static int hal_EfusePgPacketRead_8812A(struct rtl_priv *pAdapter,
	uint8_t offset, uint8_t *data)
{
	uint8_t	ReadState = PG_STATE_HEADER;

	int	bContinual = _TRUE;
	int	bDataEmpty = _TRUE ;

	uint8_t	efuse_data, word_cnts = 0;
	u16 efuse_addr = 0;
	uint8_t	hoffset = 0, hworden = 0;
	uint8_t	tmpidx = 0;
	uint8_t	tmpdata[8];
	uint8_t	max_section = 0;
	uint8_t	tmp_header = 0;

	if (data == NULL)
		return _FALSE;
	if (offset > EFUSE_MAX_SECTION_JAGUAR)
		return _FALSE;

	memset((PVOID)data, 0xff, sizeof(uint8_t) * PGPKT_DATA_SIZE);
	memset((PVOID)tmpdata, 0xff, sizeof(uint8_t) * PGPKT_DATA_SIZE);


	/*
	 * <Roger_TODO> Efuse has been pre-programmed dummy 5Bytes at the end of Efuse by CP.
	 * Skip dummy parts to prevent unexpected data read from Efuse.
	 *  By pass right now. 2009.02.19.
	 */
	while (bContinual && (efuse_addr  < EFUSE_REAL_CONTENT_LEN_JAGUAR)) {
		/* -------  Header Read ------------- */
		if (ReadState & PG_STATE_HEADER) {
			if (efuse_OneByteRead(pAdapter, efuse_addr, &efuse_data)
			    && (efuse_data != 0xFF)) {
				if (EXT_HEADER(efuse_data)) {
					tmp_header = efuse_data;
					efuse_addr++;
					efuse_OneByteRead(pAdapter, efuse_addr, &efuse_data);
					if (!ALL_WORDS_DISABLED(efuse_data)) {
						hoffset = ((tmp_header & 0xE0) >> 5) | ((efuse_data & 0xF0) >> 1);
						hworden = efuse_data & 0x0F;
					} else {
						DBG_8192C("Error, All words disabled\n");
						efuse_addr++;
						break;
					}
				} else {
					hoffset = (efuse_data >> 4) & 0x0F;
					hworden =  efuse_data & 0x0F;
				}
				word_cnts = Efuse_CalculateWordCnts(hworden);
				bDataEmpty = _TRUE ;

				if (hoffset == offset) {
					for (tmpidx = 0; tmpidx < word_cnts*2; tmpidx++) {
						if (efuse_OneByteRead(pAdapter, efuse_addr+1+tmpidx, &efuse_data)) {
							tmpdata[tmpidx] = efuse_data;
							if (efuse_data != 0xff) {
								bDataEmpty = _FALSE;
							}
						}
					}
					if (bDataEmpty == _FALSE) {
						ReadState = PG_STATE_DATA;
					} else {	/* read next header */
						efuse_addr = efuse_addr + (word_cnts*2)+1;
						ReadState = PG_STATE_HEADER;
					}
				} else {	/*read next header */
					efuse_addr = efuse_addr + (word_cnts*2)+1;
					ReadState = PG_STATE_HEADER;
				}

			} else{
				bContinual = _FALSE ;
			}
		} else if (ReadState & PG_STATE_DATA) {
			/* -------  Data section Read ------------- */
			efuse_WordEnableDataRead(hworden, tmpdata, data);
			efuse_addr = efuse_addr + (word_cnts*2)+1;
			ReadState = PG_STATE_HEADER;
		}

	}

	if ((data[0] == 0xff) && (data[1] == 0xff) && (data[2] == 0xff) &&
	    (data[3] == 0xff) && (data[4] == 0xff) && (data[5] == 0xff) &&
	    (data[6] == 0xff) && (data[7] == 0xff))
		return _FALSE;
	else
		return _TRUE;

}

int rtl8812_Efuse_PgPacketRead(struct rtl_priv *pAdapter, uint8_t offset,
	uint8_t	*data)
{
	int ret = 0;

	ret = hal_EfusePgPacketRead_8812A(pAdapter, offset, data);

	return ret;
}

static int
hal_EfusePgPacketWrite_8812A(IN	struct rtl_priv *pAdapter, uint8_t offset,
	uint8_t	word_en, uint8_t *data)
{
	uint8_t WriteState = PG_STATE_HEADER;

	int bContinual = _TRUE, bDataEmpty = _TRUE;
	/* int bResult = _TRUE; */
	u16 efuse_addr = 0;
	uint8_t	efuse_data;

	uint8_t	pg_header = 0, pg_header_temp = 0;

	uint8_t	tmp_word_cnts = 0, target_word_cnts = 0;
	uint8_t	tmp_header, match_word_en, tmp_word_en;

	PGPKT_STRUCT target_pkt;
	PGPKT_STRUCT tmp_pkt;

	uint8_t	originaldata[sizeof(uint8_t) * 8];
	uint8_t	tmpindex = 0, badworden = 0x0F;

	static int repeat_times = 0;

	BOOLEAN	bExtendedHeader = _FALSE;
	uint8_t	efuseType = EFUSE_WIFI;

	/*
	 * <Roger_Notes> Efuse has been pre-programmed dummy 5Bytes at the end of Efuse by CP.
	 * So we have to prevent unexpected data string connection, which will cause
	 * incorrect data auto-load from HW. The total size is equal or smaller than 498bytes
	 * (i.e., offset 0~497, and dummy 1bytes) expected after CP test.
	 * 2009.02.19.
	 */
	if (pAdapter->HalFunc->EfuseGetCurrentSize(pAdapter, efuseType) >= (EFUSE_REAL_CONTENT_LEN_JAGUAR-EFUSE_OOB_PROTECT_BYTES_JAGUAR)) {
		DBG_871X("hal_EfusePgPacketWrite_8812A() error: %x >= %x\n", pAdapter->HalFunc->EfuseGetCurrentSize(pAdapter, efuseType), (EFUSE_REAL_CONTENT_LEN_JAGUAR-EFUSE_OOB_PROTECT_BYTES_JAGUAR));
		return _FALSE;
	}

	/* Init the 8 bytes content as 0xff */
	target_pkt.offset = offset;
	target_pkt.word_en = word_en;
	/* Initial the value to avoid compile warning */
	tmp_pkt.offset = 0;
	tmp_pkt.word_en = 0;

	/* DBG_871X("hal_EfusePgPacketWrite_8812A target offset 0x%x word_en 0x%x \n", target_pkt.offset, target_pkt.word_en); */

	memset((PVOID)target_pkt.data, 0xFF, sizeof(uint8_t)*8);

	efuse_WordEnableDataRead(word_en, data, target_pkt.data);
	target_word_cnts = Efuse_CalculateWordCnts(target_pkt.word_en);

	/*
	 * <Roger_Notes> Efuse has been pre-programmed dummy 5Bytes at the end of Efuse by CP.
	 * So we have to prevent unexpected data string connection, which will cause
	 * incorrect data auto-load from HW. Dummy 1bytes is additional.
	 * 2009.02.19.
	 */
	while (bContinual && (efuse_addr  < (EFUSE_REAL_CONTENT_LEN_JAGUAR-EFUSE_OOB_PROTECT_BYTES_JAGUAR))) {
		if (WriteState == PG_STATE_HEADER) {
			bDataEmpty = _TRUE;
			badworden = 0x0F;
			/* ************	so ******************* */
			/* DBG_871X("EFUSE PG_STATE_HEADER\n"); */
			if (efuse_OneByteRead(pAdapter, efuse_addr, &efuse_data) &&
			   (efuse_data != 0xFF)) {
				if ((efuse_data&0x1F) == 0x0F) {		/* extended header */
					tmp_header = efuse_data;
					efuse_addr++;
					efuse_OneByteRead(pAdapter, efuse_addr, &efuse_data);
					if ((efuse_data & 0x0F) == 0x0F) {	/* wren fail */
						uint8_t next = 0, next_next = 0, data = 0, i = 0;
						uint8_t s = ((tmp_header & 0xF0) >> 4);
						efuse_OneByteRead(pAdapter, efuse_addr+1, &next);
						efuse_OneByteRead(pAdapter, efuse_addr+2, &next_next);
						if (next == 0xFF && next_next == 0xFF) {
							/* Have enough space to make fake data to recover bad header. */
							switch (s) {
							case 0x0:
							case 0x2:
							case 0x4:
							case 0x6:
							case 0x8:
							case 0xA:
							case 0xC:
								for (i = 0; i < 3; ++i) {
								efuse_OneByteWrite(pAdapter, efuse_addr, 0x27);
									efuse_OneByteRead(pAdapter, efuse_addr, &data);
									if (data == 0x27)
										break;
								}
								break;
							case 0xE:
								for (i = 0; i < 3; ++i) {
								efuse_OneByteWrite(pAdapter, efuse_addr, 0x17);
									efuse_OneByteRead(pAdapter, efuse_addr, &data);
									if (data == 0x17)
										break;
								}
								break;
							default:
								break;
							}
							efuse_OneByteWrite(pAdapter, efuse_addr+1, 0xFF);
							efuse_OneByteWrite(pAdapter, efuse_addr+2, 0xFF);
							efuse_addr += 3;
						} else {
							efuse_addr++;
						}
						continue;
					} else {
						tmp_pkt.offset = ((tmp_header & 0xE0) >> 5) | ((efuse_data & 0xF0) >> 1);
						tmp_pkt.word_en = efuse_data & 0x0F;
					}
				} else {
					uint8_t i = 0, data = 0;
					tmp_header	=  efuse_data;
					tmp_pkt.offset	= (tmp_header>>4) & 0x0F;
					tmp_pkt.word_en = tmp_header & 0x0F;

					if (tmp_pkt.word_en == 0xF) {
						uint8_t next = 0;
						efuse_OneByteRead(pAdapter, efuse_addr+1, &next);
						if (next == 0xFF) { 	/* Have enough space to make fake data to recover bad header. */
							tmp_header = (tmp_header & 0xF0) | 0x7;
							for (i = 0; i < 3; ++i) {
							efuse_OneByteWrite(pAdapter, efuse_addr, tmp_header);
								efuse_OneByteRead(pAdapter, efuse_addr, &data);
								if (data == tmp_header)
									break;
							}
							efuse_OneByteWrite(pAdapter, efuse_addr+1, 0xFF);
							efuse_OneByteWrite(pAdapter, efuse_addr+2, 0xFF);
							efuse_addr += 2;
						}
					}
				}
				tmp_word_cnts =  Efuse_CalculateWordCnts(tmp_pkt.word_en);

				/* DBG_871X("section offset 0x%x worden 0x%x\n", tmp_pkt.offset, tmp_pkt.word_en); */

				/* ************	so-1 ******************* */
				if (tmp_pkt.offset  != target_pkt.offset) {
					efuse_addr = efuse_addr + (tmp_word_cnts * 2) + 1; /* Next pg_packet */
#if (EFUSE_ERROE_HANDLE == 1)
					WriteState = PG_STATE_HEADER;
#endif
				} else {
					/* write the same offset */
					/* DBG_871X("hal_EfusePgPacketWrite_8812A section offset the same\n"); */
					/* ************	so-2 ******************* */
					for (tmpindex = 0; tmpindex < (tmp_word_cnts * 2) ; tmpindex++) {
						if (efuse_OneByteRead(pAdapter, (efuse_addr + 1 + tmpindex), &efuse_data) && (efuse_data != 0xFF)) {
							bDataEmpty = _FALSE;
						}
					}
					/* ***********	so-2-1 ******************* */
					if (bDataEmpty == _FALSE) {
						/* DBG_871X("hal_EfusePgPacketWrite_8812A section offset the same and data is NOT empty\n"); */
						efuse_addr = efuse_addr + (tmp_word_cnts*2) + 1; /* Next pg_packet */
#if (EFUSE_ERROE_HANDLE == 1)
						WriteState = PG_STATE_HEADER;
#endif
					} else {
						/* ************  so-2-2 ******************* */
						/* DBG_871X("hal_EfusePgPacketWrite_8812A section data empty\n"); */
						match_word_en = 0x0F;			/* same bit as original wren */
						if (!((target_pkt.word_en & BIT0) | (tmp_pkt.word_en & BIT0))) {
							 match_word_en &= (~BIT0);
						}
						if (!((target_pkt.word_en & BIT1) | (tmp_pkt.word_en & BIT1))) {
							 match_word_en &= (~BIT1);
						}
						if (!((target_pkt.word_en & BIT2) | (tmp_pkt.word_en & BIT2))) {
							 match_word_en &= (~BIT2);
						}
						if (!((target_pkt.word_en & BIT3) | (tmp_pkt.word_en & BIT3))) {
							 match_word_en &= (~BIT3);
						}

						/* ***********	so-2-2-A ******************* */
						if ((match_word_en&0x0F) != 0x0F) {
							badworden = Efuse_WordEnableDataWrite(pAdapter, efuse_addr + 1, tmp_pkt.word_en, target_pkt.data);

							/************	so-2-2-A-1 ******************* */
							/* ############################ */
							if (0x0F != (badworden & 0x0F)) {
								uint8_t	reorg_offset = offset;
								uint8_t	reorg_worden = badworden;
								Efuse_PgPacketWrite(pAdapter, reorg_offset, reorg_worden, target_pkt.data);
							}
							/* ############################ */

							tmp_word_en = 0x0F;		/* not the same bit as original wren */
							if ((target_pkt.word_en&BIT0) ^ (match_word_en&BIT0)) {
								tmp_word_en &= (~BIT0);
							}
							if ((target_pkt.word_en&BIT1) ^ (match_word_en&BIT1)) {
								tmp_word_en &=	(~BIT1);
							}
							if ((target_pkt.word_en&BIT2) ^ (match_word_en&BIT2)) {
								tmp_word_en &= (~BIT2);
							}
							if ((target_pkt.word_en&BIT3) ^ (match_word_en&BIT3)) {
								tmp_word_en &= (~BIT3);
							}

							/* ************	so-2-2-A-2 ******************* */
							if ((tmp_word_en & 0x0F) != 0x0F) {
								/* reorganize other pg packet */
								/* efuse_addr = efuse_addr + (2*tmp_word_cnts) +1;//next pg packet addr */
								efuse_addr = pAdapter->HalFunc->EfuseGetCurrentSize(pAdapter, efuseType);
								/* =========================== */
								target_pkt.offset = offset;
								target_pkt.word_en = tmp_word_en;
								/* =========================== */
							} else {
								bContinual = _FALSE;
							}
#if (EFUSE_ERROE_HANDLE == 1)
							WriteState = PG_STATE_HEADER;
							repeat_times++;
							if (repeat_times > EFUSE_REPEAT_THRESHOLD_) {
								bContinual = _FALSE;
								/* bResult = _FALSE; */
							}
#endif
						} else{
							/* ************  so-2-2-B ******************* */
							/* reorganize other pg packet */
							efuse_addr = efuse_addr + (2 * tmp_word_cnts) + 1;	/* next pg packet addr */
							/* =========================== */
							target_pkt.offset = offset;
							target_pkt.word_en = target_pkt.word_en;
							/* =========================== */
#if (EFUSE_ERROE_HANDLE == 1)
							WriteState = PG_STATE_HEADER;
#endif
						}
					}
				}
				DBG_871X("EFUSE PG_STATE_HEADER-1\n");
			} else	{
				/* ***********	s1: header == oxff	******************* */
				bExtendedHeader = _FALSE;

				if (target_pkt.offset >= EFUSE_MAX_SECTION_BASE) {
					pg_header = ((target_pkt.offset & 0x07) << 5) | 0x0F;

					/* DBG_871X("hal_EfusePgPacketWrite_8812A extended pg_header[2:0] |0x0F 0x%x \n", pg_header); */

					efuse_OneByteWrite(pAdapter, efuse_addr, pg_header);
					efuse_OneByteRead(pAdapter, efuse_addr, &tmp_header);

					while (tmp_header == 0xFF) {
						/* DBG_871X("hal_EfusePgPacketWrite_8812A extended pg_header[2:0] wirte fail \n"); */

						repeat_times++;

						if (repeat_times > EFUSE_REPEAT_THRESHOLD_) {
							bContinual = _FALSE;
							/* bResult = _FALSE; */
							efuse_addr++;
							break;
						}
						efuse_OneByteWrite(pAdapter, efuse_addr, pg_header);
						efuse_OneByteRead(pAdapter, efuse_addr, &tmp_header);
					}

					if (!bContinual)
						break;

					if (tmp_header == pg_header) {
						efuse_addr++;
						pg_header_temp = pg_header;
						pg_header = ((target_pkt.offset & 0x78) << 1) | target_pkt.word_en;

						/* DBG_871X("hal_EfusePgPacketWrite_8812A extended pg_header[6:3] | worden 0x%x word_en 0x%x \n", pg_header, target_pkt.word_en); */

						efuse_OneByteWrite(pAdapter, efuse_addr, pg_header);
						efuse_OneByteRead(pAdapter, efuse_addr, &tmp_header);

						while (tmp_header == 0xFF) {
							repeat_times++;

							if (repeat_times > EFUSE_REPEAT_THRESHOLD_) {
								bContinual = _FALSE;
								/* bResult = _FALSE; */
								break;
							}
							efuse_OneByteWrite(pAdapter, efuse_addr, pg_header);
							efuse_OneByteRead(pAdapter, efuse_addr, &tmp_header);
						}

						if (!bContinual)
							break;

						if ((tmp_header & 0x0F) == 0x0F) {
							/* wren PG fail */
							repeat_times++;

							if (repeat_times > EFUSE_REPEAT_THRESHOLD_) {
								bContinual = _FALSE;
								/* bResult = _FALSE; */
								break;
							} else {
								efuse_addr++;
								continue;
							}
						} else if (pg_header != tmp_header) {	/* offset PG fail */
							bExtendedHeader = _TRUE;
							tmp_pkt.offset = ((pg_header_temp & 0xE0) >> 5) | ((tmp_header & 0xF0) >> 1);
							tmp_pkt.word_en =  tmp_header & 0x0F;
							tmp_word_cnts =  Efuse_CalculateWordCnts(tmp_pkt.word_en);
						}
					} else if ((tmp_header & 0x1F) == 0x0F) {	/* wrong extended header */
						efuse_addr += 2;
						continue;
					}
				} else {
					pg_header = ((target_pkt.offset << 4) & 0xf0) | target_pkt.word_en;
					efuse_OneByteWrite(pAdapter, efuse_addr, pg_header);
					efuse_OneByteRead(pAdapter, efuse_addr, &tmp_header);
				}

				if (tmp_header == pg_header) {
					/* ***********  s1-1******************* */
					WriteState = PG_STATE_DATA;
				}
#if (EFUSE_ERROE_HANDLE == 1)
				else if (tmp_header == 0xFF) {
					/************	s1-3: if Write or read func doesn't work ******************* */
					/* efuse_addr doesn't change */
					WriteState = PG_STATE_HEADER;
					repeat_times++;
					if (repeat_times > EFUSE_REPEAT_THRESHOLD_) {
						bContinual = _FALSE;
						/* bResult = _FALSE; */
					}
				}
#endif
				else {
					/* ***********  s1-2 : fixed the header procedure ******************* */
					if (!bExtendedHeader) {
						tmp_pkt.offset = (tmp_header >> 4) & 0x0F;
						tmp_pkt.word_en =  tmp_header & 0x0F;
						tmp_word_cnts =  Efuse_CalculateWordCnts(tmp_pkt.word_en);
					}

					/* ************	s1-2-A :cover the exist data ******************* */
					memset(originaldata, 0xff, sizeof(uint8_t) * 8);

					if (Efuse_PgPacketRead(pAdapter, tmp_pkt.offset, originaldata)) {
						/* check if data exist */
						badworden = Efuse_WordEnableDataWrite(pAdapter, efuse_addr+1, tmp_pkt.word_en, originaldata);
						/* ############################ */
						if (0x0F != (badworden & 0x0F)) {
							uint8_t	reorg_offset = tmp_pkt.offset;
							uint8_t	reorg_worden = badworden;
							Efuse_PgPacketWrite(pAdapter, reorg_offset, reorg_worden, originaldata);
							efuse_addr = pAdapter->HalFunc->EfuseGetCurrentSize(pAdapter, efuseType);
						} else {
							/* ############################ */
							efuse_addr = efuse_addr + (tmp_word_cnts*2) + 1; /* Next pg_packet */
						}
					} else {
						/* ************  s1-2-B: wrong address******************* */
						efuse_addr = efuse_addr + (tmp_word_cnts*2) + 1; /* Next pg_packet */
					}

#if (EFUSE_ERROE_HANDLE == 1)
					WriteState = PG_STATE_HEADER;
					repeat_times++;
					if (repeat_times > EFUSE_REPEAT_THRESHOLD_) {
						bContinual = _FALSE;
						/* bResult = _FALSE; */
					}
#endif

					/* DBG_871X("EFUSE PG_STATE_HEADER-2\n"); */
				}

			}

		}
		/* write data state */
		else if (WriteState == PG_STATE_DATA) {
			/* ************	s1-1  ******************* */
			/* DBG_871X("EFUSE PG_STATE_DATA\n"); */
			badworden = 0x0f;
			badworden = Efuse_WordEnableDataWrite(pAdapter, efuse_addr+1, target_pkt.word_en, target_pkt.data);
			if ((badworden & 0x0F) == 0x0F) {
				/* ************  s1-1-A ******************* */
				bContinual = _FALSE;
			} else {
				/* reorganize other pg packet  */
				/* ***********  s1-1-B ******************* */
				efuse_addr = efuse_addr + (2 * target_word_cnts) + 1;	/* next pg packet addr */

				/* =========================== */
				target_pkt.offset = offset;
				target_pkt.word_en = badworden;
				target_word_cnts = Efuse_CalculateWordCnts(target_pkt.word_en);
				/* =========================== */
#if (EFUSE_ERROE_HANDLE == 1)
				WriteState = PG_STATE_HEADER;
				repeat_times++;
				if (repeat_times > EFUSE_REPEAT_THRESHOLD_) {
					bContinual = _FALSE;
					/* bResult = _FALSE; */
				}
#endif
				/* DBG_871X("EFUSE PG_STATE_HEADER-3\n"); */
			}
		}
	}

	if (efuse_addr >= (EFUSE_REAL_CONTENT_LEN_JAGUAR-EFUSE_OOB_PROTECT_BYTES_JAGUAR)) {
		DBG_871X("hal_EfusePgPacketWrite_8812A(): efuse_addr(%#x) Out of size!!\n", efuse_addr);
	}
	return _TRUE;
}

int
rtl8812_Efuse_PgPacketWrite(struct rtl_priv *pAdapter, uint8_t offset,
	uint8_t	word_en, IN uint8_t *data)
{
	int	ret;

	ret = hal_EfusePgPacketWrite_8812A(pAdapter, offset, word_en, data);

	return ret;
}

void InitRDGSetting8812A(struct rtl_priv *padapter)
{
	rtl_write_byte(padapter, REG_RD_CTRL, 0xFF);
	rtl_write_word(padapter, REG_RD_NAV_NXT, 0x200);
	rtl_write_byte(padapter, REG_RD_RESP_PKT_TH, 0x05);
}

void ReadRFType8812A(struct rtl_priv *padapter)
{
	struct _rtw_hal *pHalData = GET_HAL_DATA(padapter);

	pHalData->rf_chip = RF_6052;

	/*
	 * if (pHalData->rf_type == RF_1T1R){
	 *	pHalData->bRFPathRxEnable[0] = _TRUE;
	 * }
	 * else {	// Default unknow type is 2T2r
	 *	pHalData->bRFPathRxEnable[0] = pHalData->bRFPathRxEnable[1] = _TRUE;
	 *}
	 */

	if (IsSupported24G(padapter->registrypriv.wireless_mode) &&
		IsSupported5G(padapter->registrypriv.wireless_mode))
		pHalData->BandSet = BAND_ON_BOTH;
	else if (IsSupported5G(padapter->registrypriv.wireless_mode))
		pHalData->BandSet = BAND_ON_5G;
	else
		pHalData->BandSet = BAND_ON_2_4G;

	/*
	 * if(padapter->bInHctTest)
	 * 	pHalData->BandSet = BAND_ON_2_4G;
	 */
}

void rtl8812_GetHalODMVar(struct rtl_priv *Adapter, HAL_ODM_VARIABLE eVariable,
	PVOID pValue1, BOOLEAN bSet)
{
	 struct _rtw_hal	*pHalData = GET_HAL_DATA(Adapter);
	struct rtl_dm *podmpriv = &pHalData->odmpriv;

	switch (eVariable) {
	case HAL_ODM_STA_INFO:
		break;
	default:
		break;
	}
}

void rtl8812_SetHalODMVar(struct rtl_priv *Adapter, HAL_ODM_VARIABLE eVariable,
	PVOID pValue1, BOOLEAN bSet)
{
	 struct _rtw_hal	*pHalData = GET_HAL_DATA(Adapter);
	struct rtl_dm *podmpriv = &pHalData->odmpriv;
	/* _irqL irqL; */
	switch (eVariable) {
	case HAL_ODM_STA_INFO:
		{
			struct sta_info *psta = (struct sta_info *)pValue1;
			if (bSet) {
				DBG_8192C("### Set STA_(%d) info\n", psta->mac_id);
				ODM_CmnInfoPtrArrayHook(podmpriv, ODM_CMNINFO_STA_STATUS, psta->mac_id, psta);
#if (RATE_ADAPTIVE_SUPPORT == 1)
				ODM_RAInfo_Init(podmpriv, psta->mac_id);
#endif
			} else {
				DBG_8192C("### Clean STA_(%d) info\n", psta->mac_id);
				/* _enter_critical_bh(&pHalData->odm_stainfo_lock, &irqL); */
				ODM_CmnInfoPtrArrayHook(podmpriv, ODM_CMNINFO_STA_STATUS, psta->mac_id, NULL);

				/* _exit_critical_bh(&pHalData->odm_stainfo_lock, &irqL); */
			}
		}
		break;
	case HAL_ODM_P2P_STATE:
			ODM_CmnInfoUpdate(podmpriv, ODM_CMNINFO_WIFI_DIRECT, bSet);
		break;
	case HAL_ODM_WIFI_DISPLAY_STATE:
			ODM_CmnInfoUpdate(podmpriv, ODM_CMNINFO_WIFI_DISPLAY, bSet);
		break;
	default:
		break;
	}
}

void hal_notch_filter_8812(struct rtl_priv *adapter, bool enable)
{
	if (enable) {
		DBG_871X("Enable notch filter\n");
		/* rtl_write_byte(adapter, rOFDM0_RxDSP+1, rtl_read_byte(adapter, rOFDM0_RxDSP+1) | BIT1); */
	} else {
		DBG_871X("Disable notch filter\n");
		/* rtl_write_byte(adapter, rOFDM0_RxDSP+1, rtl_read_byte(adapter, rOFDM0_RxDSP+1) & ~BIT1); */
	}
}

u8 GetEEPROMSize8812A(struct rtl_priv *Adapter)
{
	uint8_t	size = 0;
	uint32_t	curRCR;

	curRCR = rtl_read_word(Adapter, REG_SYS_EEPROM_CTRL);
	size = (curRCR & EEPROMSEL) ? 6 : 4; /* 6: EEPROM used is 93C46, 4: boot from E-Fuse. */

	DBG_871X("EEPROM type is %s\n", size == 4 ? "E-FUSE" : "93C46");
	/* return size; */
	return 4; /*  <20120713, Kordan> The default value of HW is 6 ?!! */
}

void CheckAutoloadState8812A(struct rtl_priv *padapter)
{
	PEEPROM_EFUSE_PRIV pEEPROM;
	uint8_t val8;


	pEEPROM = GET_EEPROM_EFUSE_PRIV(padapter);

	/* check system boot selection */
	val8 = rtl_read_byte(padapter, REG_9346CR);
	pEEPROM->EepromOrEfuse = (val8 & BOOT_FROM_EEPROM) ? _TRUE : _FALSE;
	pEEPROM->bautoload_fail_flag = (val8 & EEPROM_EN) ? _FALSE : _TRUE;

	DBG_8192C("%s: 9346CR(%#x)=0x%02x, Boot from %s, Autoload %s!\n",
			__FUNCTION__, REG_9346CR, val8,
			(pEEPROM->EepromOrEfuse ? "EEPROM" : "EFUSE"),
			(pEEPROM->bautoload_fail_flag ? "Fail" : "OK"));
}

void InitPGData8812A(struct rtl_priv *padapter)
{
	PEEPROM_EFUSE_PRIV pEEPROM;
	uint32_t i;
	u16 val16;


	pEEPROM = GET_EEPROM_EFUSE_PRIV(padapter);

}

void ReadChipVersion8812A(struct rtl_priv *Adapter)
{
	uint32_t	value32;
	HAL_VERSION		ChipVersion;
	struct _rtw_hal *pHalData;
	struct rtl_hal *rtlhal = rtl_hal(Adapter);

	pHalData = GET_HAL_DATA(Adapter);

	value32 = rtl_read_dword(Adapter, REG_SYS_CFG);
	DBG_8192C("%s SYS_CFG(0x%X)=0x%08x \n", __FUNCTION__, REG_SYS_CFG, value32);

	if (IS_HARDWARE_TYPE_8812(rtlhal))
		ChipVersion.ICType = CHIP_8812;
	else
		ChipVersion.ICType = CHIP_8821;

	ChipVersion.ChipType = ((value32 & RTL_ID) ? TEST_CHIP : NORMAL_CHIP);

	if (Adapter->registrypriv.rf_config == RF_MAX_TYPE) {
		if (IS_HARDWARE_TYPE_8812(rtlhal))
			ChipVersion.RFType = RF_TYPE_2T2R;	/* RF_2T2R; */
		else
			ChipVersion.RFType = RF_TYPE_1T1R;	/*RF_1T1R; */
	}

	if (IS_HARDWARE_TYPE_8812(rtlhal))
		ChipVersion.VendorType = ((value32 & VENDOR_ID) ? CHIP_VENDOR_UMC : CHIP_VENDOR_TSMC);
	else {
		uint32_t vendor;

		vendor = (value32 & EXT_VENDOR_ID) >> EXT_VENDOR_ID_SHIFT;
		switch (vendor) {
		case 0:
			vendor = CHIP_VENDOR_TSMC;
			break;
		case 1:
			vendor = CHIP_VENDOR_SMIC;
			break;
		case 2:
			vendor = CHIP_VENDOR_UMC;
			break;
		}
		ChipVersion.VendorType = vendor;
	}
	ChipVersion.CUTVersion = (value32 & CHIP_VER_RTL_MASK)>>CHIP_VER_RTL_SHIFT; /* IC version (CUT) */
	if (IS_HARDWARE_TYPE_8812(rtlhal))
		ChipVersion.CUTVersion += 1;

	/* value32 = rtl_read_dword(Adapter, REG_GPIO_OUTSTS); */
	ChipVersion.ROMVer = 0;	/* ROM code version. */

	/* For multi-function consideration. Added by Roger, 2010.10.06. */
	pHalData->MultiFunc = RT_MULTI_FUNC_NONE;
	value32 = rtl_read_dword(Adapter, REG_MULTI_FUNC_CTRL);
	pHalData->MultiFunc |= ((value32 & WL_FUNC_EN) ? RT_MULTI_FUNC_WIFI : 0);
	pHalData->MultiFunc |= ((value32 & BT_FUNC_EN) ? RT_MULTI_FUNC_BT : 0);
	pHalData->PolarityCtl = ((value32 & WL_HWPDN_SL) ? RT_POLARITY_HIGH_ACT : RT_POLARITY_LOW_ACT);

#if 1
	dump_chip_info(ChipVersion);
#endif

	memcpy(&pHalData->VersionID, &ChipVersion, sizeof(HAL_VERSION));

	if (IS_1T2R(ChipVersion)) {
		Adapter->phy.rf_type = RF_1T2R;
		pHalData->NumTotalRFPath = 2;
		DBG_8192C("==> RF_Type : 1T2R\n");
	} else if (IS_2T2R(ChipVersion)) {
		Adapter->phy.rf_type = RF_2T2R;
		pHalData->NumTotalRFPath = 2;
		DBG_8192C("==> RF_Type : 2T2R\n");
	} else {
		Adapter->phy.rf_type = RF_1T1R;
		pHalData->NumTotalRFPath = 1;
		DBG_8192C("==> RF_Type 1T1R\n");
	}

	DBG_8192C("RF_Type is %x!!\n", Adapter->phy.rf_type);
}


void UpdateHalRAMask8812A(struct rtl_priv *padapter, uint32_t mac_id, uint8_t rssi_level)
{
	/* volatile unsigned int result; */
	uint8_t	init_rate = 0;
	uint8_t	networkType, raid;
	uint32_t	mask, rate_bitmap;
	uint8_t	shortGIrate = _FALSE;
	int	supportRateNum = 0;
	uint8_t	arg[4] = {0};
	struct sta_info	*psta;
	 struct _rtw_hal	*pHalData = GET_HAL_DATA(padapter);
	/* struct dm_priv	*pdmpriv = &pHalData->dmpriv; */
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX 		*cur_network = &(pmlmeinfo->network);

	if (mac_id >= NUM_STA) {	/* CAM_SIZE */
		return;
	}

	psta = pmlmeinfo->FW_sta_info[mac_id].psta;
	if (psta == NULL) {
		return;
	}

	switch (mac_id) {
	case 0:
		/* for infra mode */
		supportRateNum = rtw_get_rateset_len(cur_network->SupportedRates);
		networkType = judge_network_type(padapter, cur_network->SupportedRates, supportRateNum);
		/* pmlmeext->cur_wireless_mode = networkType; */
		/* raid = networktype_to_raid(networkType); */
		raid = rtw_hal_networktype_to_raid(padapter, networkType);

		mask = update_supported_rate(cur_network->SupportedRates, supportRateNum);
#ifdef CONFIG_80211AC_VHT
		if (pmlmeinfo->VHT_enable) {
			mask |= (rtw_vht_rate_to_bitmap(psta->vhtpriv.vht_mcs_map) << 12);
			shortGIrate = psta->vhtpriv.sgi;
		} else
#endif
			{
				mask |= (pmlmeinfo->HT_enable) ? update_MCS_rate(&(pmlmeinfo->HT_caps)) : 0;
				if (support_short_GI(padapter, &(pmlmeinfo->HT_caps)))
					shortGIrate = _TRUE;
			}

		break;

	case 1:
		/* for broadcast/multicast */
		supportRateNum = rtw_get_rateset_len(pmlmeinfo->FW_sta_info[mac_id].SupportedRates);
		if (pmlmeext->cur_wireless_mode & WIRELESS_11B)
			networkType = WIRELESS_11B;
		else
			networkType = WIRELESS_11G;

		/* raid = networktype_to_raid(networkType); */
		raid = rtw_hal_networktype_to_raid(padapter, networkType);
		mask = update_basic_rate(cur_network->SupportedRates, supportRateNum);

		break;

	default:
		/*for each sta in IBSS */
		supportRateNum = rtw_get_rateset_len(pmlmeinfo->FW_sta_info[mac_id].SupportedRates);
		networkType = judge_network_type(padapter, pmlmeinfo->FW_sta_info[mac_id].SupportedRates, supportRateNum) & 0xf;
		/*
		 * pmlmeext->cur_wireless_mode = networkType;
		 * raid = networktype_to_raid(networkType);
		 */
		raid = rtw_hal_networktype_to_raid(padapter, networkType);
		mask = update_supported_rate(cur_network->SupportedRates, supportRateNum);

		/* todo: support HT in IBSS */

		break;
	}

	/* mask &=0x0fffffff; */
	rate_bitmap = 0xffffffff;
#ifdef	CONFIG_ODM_REFRESH_RAMASK
	{
		rate_bitmap = ODM_Get_Rate_Bitmap(&pHalData->odmpriv, mac_id, mask, rssi_level);
		printk("%s => mac_id:%d, networkType:0x%02x, mask:0x%08x\n\t ==> rssi_level:%d, rate_bitmap:0x%08x\n",
			__FUNCTION__, mac_id, networkType, mask, rssi_level, rate_bitmap);
	}
#endif

	mask &= rate_bitmap;
	init_rate = get_highest_rate_idx(mask)&0x3f;

	/*
	 * arg[0] = macid
	 * arg[1] = raid
	 * arg[2] = shortGIrate
	 * arg[3] = init_rate
	 */

	arg[0] = mac_id;
	arg[1] = raid;
	arg[2] = shortGIrate;
	arg[3] = init_rate;

	rtl8812_set_raid_cmd(padapter, mask, arg);

	/* set ra_id */
	psta->raid = raid;
	psta->init_rate = init_rate;
}

void InitDefaultValue8821A(struct rtl_priv *padapter)
{
	struct _rtw_hal *pHalData;
	struct pwrctrl_priv *pwrctrlpriv;
	struct dm_priv *pdmpriv;
	uint8_t i;

	pHalData = GET_HAL_DATA(padapter);
	pwrctrlpriv = &padapter->pwrctrlpriv;
	pdmpriv = &pHalData->dmpriv;

	/* init default value */
	pHalData->fw_ractrl = _FALSE;
	if (!pwrctrlpriv->bkeepfwalive)
		pHalData->LastHMEBoxNum = 0;

	/* init dm default value */
	pHalData->bChnlBWInitialzed = _FALSE;
	pHalData->odmpriv.RFCalibrateInfo.bIQKInitialized = _FALSE;
	pHalData->odmpriv.RFCalibrateInfo.TM_Trigger = 0;	/* for IQK */
	pHalData->pwrGroupCnt = 0;
	pHalData->PGMaxGroup = MAX_PG_GROUP;
	pHalData->odmpriv.RFCalibrateInfo.ThermalValue_HP_index = 0;
	for (i = 0; i < HP_THERMAL_NUM; i++)
		pHalData->odmpriv.RFCalibrateInfo.ThermalValue_HP[i] = 0;
}

VOID _InitBeaconParameters_8812A(struct rtl_priv *Adapter)
{
	 struct _rtw_hal	*pHalData = GET_HAL_DATA(Adapter);

	rtl_write_word(Adapter, REG_BCN_CTRL, 0x1010);

	/* TODO: Remove these magic number */
	rtl_write_word(Adapter, REG_TBTT_PROHIBIT, 0x6404);		/* ms */
	rtl_write_byte(Adapter, REG_DRVERLYINT, DRIVER_EARLY_INT_TIME_8812);	/* 5ms */
	rtl_write_byte(Adapter, REG_BCNDMATIM, BCN_DMA_ATIME_INT_TIME_8812); 	/* 2ms */

	/*
	 *  Suggested by designer timchen. Change beacon AIFS to the largest number
	 *  beacause test chip does not contension before sending beacon. by tynli. 2009.11.03
	 */
	rtl_write_word(Adapter, REG_BCNTCFG, 0x660F);

	pHalData->RegBcnCtrlVal = rtl_read_byte(Adapter, REG_BCN_CTRL);
	pHalData->RegTxPause = rtl_read_byte(Adapter, REG_TXPAUSE);
	pHalData->RegFwHwTxQCtrl = rtl_read_byte(Adapter, REG_FWHW_TXQ_CTRL+2);
	pHalData->RegReg542 = rtl_read_byte(Adapter, REG_TBTT_PROHIBIT+2);
	pHalData->RegCR_1 = rtl_read_byte(Adapter, REG_CR+1);
}

static void hw_var_set_correct_tsf(struct rtl_priv *Adapter, uint8_t variable, uint8_t *val)
{
}

static void hw_var_set_mlme_disconnect(struct rtl_priv *Adapter, uint8_t variable, uint8_t *val)
{
}


static void hw_var_set_mlme_join(struct rtl_priv *Adapter, uint8_t variable, uint8_t *val)
{
}


/*
 *	Description:
 *		Change default setting of specified variable.
 */
uint8_t SetHalDefVar8812A(struct rtl_priv *padapter, HAL_DEF_VARIABLE variable, void *pval)
{
	struct _rtw_hal *pHalData;
	uint8_t bResult;


	pHalData = GET_HAL_DATA(padapter);
	bResult = _SUCCESS;

	switch (variable) {
	case HAL_DEF_DBG_DM_FUNC:
		{
			uint8_t dm_func;
			struct dm_priv *pdmpriv;
			struct rtl_dm *podmpriv;


			dm_func = *((uint8_t *)pval);
			pdmpriv = &pHalData->dmpriv;
			podmpriv = &pHalData->odmpriv;

			if (dm_func == 0) {
				/* disable all dynamic func */
				podmpriv->SupportAbility = DYNAMIC_FUNC_DISABLE;
				DBG_8192C("==> Disable all dynamic function...\n");
			} else if (dm_func == 1) {
				/* disable DIG */
				podmpriv->SupportAbility &= (~DYNAMIC_BB_DIG);
				DBG_8192C("==> Disable DIG...\n");
			} else if (dm_func == 2) {
				/* disable High power */
				podmpriv->SupportAbility &= (~DYNAMIC_BB_DYNAMIC_TXPWR);
			} else if (dm_func == 3) {
				/* disable tx power tracking */
				podmpriv->SupportAbility &= (~DYNAMIC_RF_CALIBRATION);
				DBG_8192C("==> Disable tx power tracking...\n");
			} else if (dm_func == 5) {
				/* disable antenna diversity */
				podmpriv->SupportAbility &= (~DYNAMIC_BB_ANT_DIV);
			} else if (dm_func == 6) {
				/* turn on all dynamic func */
				if (!(podmpriv->SupportAbility & DYNAMIC_BB_DIG)) {
					pDIG_T pDigTable = &podmpriv->DM_DigTable;
					pDigTable->CurIGValue = rtl_read_byte(padapter, 0xc50);
				}
				/* pdmpriv->DMFlag |= DYNAMIC_FUNC_BT; */
				podmpriv->SupportAbility = DYNAMIC_ALL_FUNC_ENABLE;
				DBG_8192C("==> Turn on all dynamic function...\n");
			}
		}
		break;

	case HAL_DEF_DBG_DUMP_RXPKT:
		pHalData->bDumpRxPkt = *(uint8_t *)pval;
		break;

	case HAL_DEF_DBG_DUMP_TXPKT:
		pHalData->bDumpTxPkt = *(uint8_t *)pval;
		break;

	case HW_DEF_FA_CNT_DUMP:
		{
			uint8_t mac_id;
			struct rtl_dm *pDM_Odm;


			mac_id = *(uint8_t *)pval;
			pDM_Odm = &pHalData->odmpriv;

			if (padapter->bLinkInfoDump & BIT(1))
				pDM_Odm->DebugComponents |=	ODM_COMP_DIG;
			else
				pDM_Odm->DebugComponents &= ~ODM_COMP_DIG;

			if (padapter->bLinkInfoDump & BIT(2))
				pDM_Odm->DebugComponents |=	ODM_COMP_FA_CNT;
			else
				pDM_Odm->DebugComponents &= ~ODM_COMP_FA_CNT;
		}
		break;

	case HW_DEF_ODM_DBG_FLAG:
		{
			u64 DebugComponents;
			struct rtl_dm *pDM_Odm;


			DebugComponents = *((u64 *)pval);
			pDM_Odm = &pHalData->odmpriv;
			pDM_Odm->DebugComponents = DebugComponents;
		}
		break;

	default:
		DBG_8192C("%s: [ERROR] HAL_DEF_VARIABLE(%d) not defined!\n", __FUNCTION__, variable);
		bResult = _FAIL;
		break;
	}

	return bResult;
}

/*
 *	Description:
 *		Query setting of specified variable.
 */
uint8_t GetHalDefVar8812A(struct rtl_priv *padapter, HAL_DEF_VARIABLE variable, void *pval)
{
	struct _rtw_hal *pHalData;
	uint8_t bResult;
	struct rtl_hal *rtlhal = rtl_hal(padapter);

	pHalData = GET_HAL_DATA(padapter);
	
	bResult = _SUCCESS;

	switch (variable) {
	case HAL_DEF_UNDERCORATEDSMOOTHEDPWDB:
		{
			struct mlme_priv *pmlmepriv;
			struct sta_priv *pstapriv;
			struct sta_info *psta;

			pmlmepriv = &padapter->mlmepriv;
			pstapriv = &padapter->stapriv;
			psta = rtw_get_stainfo(pstapriv, pmlmepriv->cur_network.network.MacAddress);
			if (psta) {
				*((int *) pval) = psta->rssi_stat.UndecoratedSmoothedPWDB;
			}
		}
		break;

#ifdef CONFIG_ANTENNA_DIVERSITY
	case HAL_DEF_IS_SUPPORT_ANT_DIV:
		*((uint8_t *)pval) = (pHalData->AntDivCfg == 0) ? _FALSE : _TRUE;
		break;
#endif

#ifdef CONFIG_ANTENNA_DIVERSITY
	case HAL_DEF_CURRENT_ANTENNA:
		*((uint8_t *)pval) = pHalData->CurAntenna;
		break;
#endif

	case HAL_DEF_DRVINFO_SZ:
		*((u32 *)pval) = DRVINFO_SZ;
		break;

	case HAL_DEF_MAX_RECVBUF_SZ:
		*((u32 *)pval) = MAX_RECVBUF_SZ;
		break;

	case HAL_DEF_RX_PACKET_OFFSET:
		*((u32 *)pval) = RXDESC_SIZE + DRVINFO_SZ;
		break;

	case HAL_DEF_DBG_DM_FUNC:
		*((u32 *)pval) = pHalData->odmpriv.SupportAbility;
		break;

#if (RATE_ADAPTIVE_SUPPORT == 1)
	case HAL_DEF_RA_DECISION_RATE:
		{
			uint8_t MacID = *(uint8_t *)pval;
			*((uint8_t *)pval) = ODM_RA_GetDecisionRate_8812A(&pHalData->odmpriv, MacID);
		}
		break;

	case HAL_DEF_RA_SGI:
		{
			uint8_t MacID = *(uint8_t *)pval;
			*((uint8_t *)pval) = ODM_RA_GetShortGI_8812A(&pHalData->odmpriv, MacID);
		}
		break;
#endif

#if (POWER_TRAINING_ACTIVE == 1)
	case HAL_DEF_PT_PWR_STATUS:
		{
			uint8_t MacID = *(uint8_t *)pval;
			*(uint8_t *)pval = ODM_RA_GetHwPwrStatus_8812A(&pHalData->odmpriv, MacID);
		}
		break;
#endif
	case HW_VAR_MAX_RX_AMPDU_FACTOR:
		*((u32 *)pval) = MAX_AMPDU_FACTOR_64K;
		break;

	case HAL_DEF_LDPC:
		if (IS_VENDOR_8812A_C_CUT(padapter))
			*(uint8_t *)pval = _TRUE;
		else if (IS_HARDWARE_TYPE_8821(rtlhal))
			*(uint8_t *)pval = _FALSE;
		else
			*(uint8_t *)pval = _FALSE;
		break;

	case HAL_DEF_TX_STBC:
		if (padapter->phy.rf_type == RF_2T2R)
			*(uint8_t *)pval = 1;
		else
			*(uint8_t *)pval = 0;
		break;

	case HAL_DEF_RX_STBC:
		*(uint8_t *)pval = 1;
		break;

	case HW_DEF_RA_INFO_DUMP:
		{
			uint8_t mac_id = *(uint8_t *)pval;
			uint32_t cmd = 0x40000400 | mac_id;
			uint32_t ra_info1, ra_info2;
			uint32_t rate_mask1, rate_mask2;

			if ((padapter->bLinkInfoDump & BIT(0))
			    && (check_fwstate(&padapter->mlmepriv, _FW_LINKED) == _TRUE)) {
				DBG_8192C("============ RA status check  Mac_id:%d ===================\n", mac_id);

				rtl_write_dword(padapter, REG_HMEBOX_E2_E3_8812, cmd);
				ra_info1 = rtl_read_dword(padapter, REG_RSVD5_8812);
				ra_info2 = rtl_read_dword(padapter, REG_RSVD6_8812);
				rate_mask1 = rtl_read_dword(padapter, REG_RSVD7_8812);
				rate_mask2 = rtl_read_dword(padapter, REG_RSVD8_8812);

				DBG_8192C("[ ra_info1:0x%08x ] =>RSSI=%d, BW_setting=0x%02x, DISRA=0x%02x, VHT_EN=0x%02x\n",
					ra_info1,
					ra_info1&0xFF,
					(ra_info1>>8)  & 0xFF,
					(ra_info1>>16) & 0xFF,
					(ra_info1>>24) & 0xFF);

				DBG_8192C("[ ra_info2:0x%08x ] =>hight_rate=0x%02x, lowest_rate=0x%02x, SGI=0x%02x, RateID=%d\n",
					ra_info2,
					ra_info2&0xFF,
					(ra_info2>>8)  & 0xFF,
					(ra_info2>>16) & 0xFF,
					(ra_info2>>24) & 0xFF);

				DBG_8192C("rate_mask2=0x%08x, rate_mask1=0x%08x\n", rate_mask2, rate_mask1);
			}
		}
		break;

	case HAL_DEF_DBG_DUMP_RXPKT:
		*(uint8_t *)pval = pHalData->bDumpRxPkt;
		break;

	case HAL_DEF_DBG_DUMP_TXPKT:
		*(uint8_t *)pval = pHalData->bDumpTxPkt;
		break;

	case HW_DEF_ODM_DBG_FLAG:
		{
			u64	DebugComponents;
			struct rtl_dm *pDM_Odm;

			pDM_Odm = &pHalData->odmpriv;
			DebugComponents = pDM_Odm->DebugComponents;
			DBG_8192C("%s: pDM_Odm->DebugComponents=0x%llx\n", __FUNCTION__, DebugComponents);
			*((u64 *)pval) = DebugComponents;
		}
		break;

	case HAL_DEF_TX_PAGE_BOUNDARY:
		if (!padapter->registrypriv.wifi_spec) 	{
			if (IS_HARDWARE_TYPE_8812(rtlhal))
				*(uint8_t *)pval = TX_PAGE_BOUNDARY_8812;
			else
				*(uint8_t *)pval = TX_PAGE_BOUNDARY_8821;
		} else 	{
			if (IS_HARDWARE_TYPE_8812(rtlhal))
				*(uint8_t *)pval = WMM_NORMAL_TX_PAGE_BOUNDARY_8812;
			else
				*(uint8_t *)pval = WMM_NORMAL_TX_PAGE_BOUNDARY_8821;
		}
		break;

	case HAL_DEF_TX_PAGE_BOUNDARY_WOWLAN:
		*(uint8_t *)pval = TX_PAGE_BOUNDARY_WOWLAN_8812;
		break;

	default:
		DBG_8192C("%s: [ERROR] HAL_DEF_VARIABLE(%d) not defined!\n", __FUNCTION__, variable);
		bResult = _FAIL;
		break;
	}

	return bResult;
}

