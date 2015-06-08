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
/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	HalPwrSeqCmd.c

Abstract:
	Implement HW Power sequence configuration CMD handling routine for Realtek devices.

Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2011-10-26 Lucas            Modify to be compatible with SD4-CE driver.
	2011-07-07 Roger            Create.

--*/
#include <HalPwrSeqCmd.h>
#include <rtw_debug.h>

//
//	Description:
//		This routine deal with the Power Configuration CMDs parsing for RTL8723/RTL8188E Series IC.
//
//	Assumption:
//		We should follow specific format which was released from HW SD.
//
//	2011.07.07, added by Roger.
//
uint8_t HalPwrSeqCmdParsing(struct rtl_priv *rtlpriv, uint8_t CutVersion,
	uint8_t FabVersion, uint8_t InterfaceType, struct wlan_pwr_cfg PwrSeqCmd[])
{
	struct wlan_pwr_cfg PwrCfgCmd = {0};
	uint8_t	bPollingBit = _FALSE;
	uint32_t AryIdx = 0;
	uint8_t value = 0;
	uint32_t offset = 0;
	uint32_t pollingCount = 0; 	/* polling autoload done. */
	uint32_t maxPollingCnt = 5000;

	do {
		PwrCfgCmd = PwrSeqCmd[AryIdx];

		/* 2 Only Handle the command whose FAB, CUT, and Interface are matched */
		if ((GET_PWR_CFG_FAB_MASK(PwrCfgCmd) & FabVersion)
		   && (GET_PWR_CFG_CUT_MASK(PwrCfgCmd) & CutVersion)
		   && (GET_PWR_CFG_INTF_MASK(PwrCfgCmd) & InterfaceType)) {
			switch (GET_PWR_CFG_CMD(PwrCfgCmd)) {
			case PWR_CMD_READ:
				break;

			case PWR_CMD_WRITE:
				offset = GET_PWR_CFG_OFFSET(PwrCfgCmd);

				{
					/* Read the value from system register */
					value = rtl_read_byte(rtlpriv, offset);

					value=value&(~(GET_PWR_CFG_MASK(PwrCfgCmd)));
					value=value|(GET_PWR_CFG_VALUE(PwrCfgCmd)&GET_PWR_CFG_MASK(PwrCfgCmd));

					/* Write the value back to sytem register */
					rtl_write_byte(rtlpriv, offset, value);
				}
				break;

			case PWR_CMD_POLLING:
				bPollingBit = _FALSE;
				offset = GET_PWR_CFG_OFFSET(PwrCfgCmd);
				do {
						value = rtl_read_byte(rtlpriv, offset);

					value=value&GET_PWR_CFG_MASK(PwrCfgCmd);
					if (value == (GET_PWR_CFG_VALUE(PwrCfgCmd) & GET_PWR_CFG_MASK(PwrCfgCmd)))
						bPollingBit = _TRUE;
					else
						udelay(10);

					if (pollingCount++ > maxPollingCnt) {
						DBG_871X("Fail to polling Offset[%#x]\n", offset);
						return _FALSE;
					}
				} while (!bPollingBit);

				break;

			case PWR_CMD_DELAY:
				if (GET_PWR_CFG_VALUE(PwrCfgCmd) == PWRSEQ_DELAY_US)
					udelay(GET_PWR_CFG_OFFSET(PwrCfgCmd));
				else
					udelay(GET_PWR_CFG_OFFSET(PwrCfgCmd)*1000);
				break;

			case PWR_CMD_END:
				/* When this command is parsed, end the process */
				return _TRUE;
				break;

			default:
				break;
			}
		}

		AryIdx++;		/* Add Array Index */
	} while(1);

	return _TRUE;
}


