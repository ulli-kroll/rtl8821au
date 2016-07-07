#include "pwrseqcmd.h"

bool rtw_hal_pwrseqcmdparsing(struct rtl_priv *rtlpriv, u8 cut_version,
				 u8 fabversion, u8 interface_type,
				 struct wlan_pwr_cfg pwrcfgcmd[])
{
	struct wlan_pwr_cfg cfg_cmd = {0};
	uint8_t	bPollingBit = false;
	uint32_t AryIdx = 0;
	uint8_t value = 0;
	uint32_t offset = 0;
	uint32_t pollingCount = 0; 	/* polling autoload done. */
	uint32_t maxPollingCnt = 5000;

	do {
		cfg_cmd = pwrcfgcmd[AryIdx];

		/* 2 Only Handle the command whose FAB, CUT, and Interface are matched */
		if ((GET_PWR_CFG_FAB_MASK(cfg_cmd) & fabversion)
		   && (GET_PWR_CFG_CUT_MASK(cfg_cmd) & cut_version)
		   && (GET_PWR_CFG_INTF_MASK(cfg_cmd) & interface_type)) {
			switch (GET_PWR_CFG_CMD(cfg_cmd)) {
			case PWR_CMD_READ:
				break;

			case PWR_CMD_WRITE:
				offset = GET_PWR_CFG_OFFSET(cfg_cmd);

				{
					/* Read the value from system register */
					value = rtl_read_byte(rtlpriv, offset);

					value=value&(~(GET_PWR_CFG_MASK(cfg_cmd)));
					value=value|(GET_PWR_CFG_VALUE(cfg_cmd)&GET_PWR_CFG_MASK(cfg_cmd));

					/* Write the value back to sytem register */
					rtl_write_byte(rtlpriv, offset, value);
				}
				break;

			case PWR_CMD_POLLING:
				bPollingBit = false;
				offset = GET_PWR_CFG_OFFSET(cfg_cmd);
				do {
						value = rtl_read_byte(rtlpriv, offset);

					value=value&GET_PWR_CFG_MASK(cfg_cmd);
					if (value == (GET_PWR_CFG_VALUE(cfg_cmd) & GET_PWR_CFG_MASK(cfg_cmd)))
						bPollingBit = true;
					else
						udelay(10);

					if (pollingCount++ > maxPollingCnt) {
						RT_TRACE(rtlpriv, COMP_ERR, DBG_LOUD, "Fail to polling Offset[%#x]\n", offset);
						return false;
					}
				} while (!bPollingBit);

				break;

			case PWR_CMD_DELAY:
				if (GET_PWR_CFG_VALUE(cfg_cmd) == PWRSEQ_DELAY_US)
					udelay(GET_PWR_CFG_OFFSET(cfg_cmd));
				else
					udelay(GET_PWR_CFG_OFFSET(cfg_cmd)*1000);
				break;

			case PWR_CMD_END:
				/* When this command is parsed, end the process */
				return true;
				break;

			default:
				break;
			}
		}

		AryIdx++;		/* Add Array Index */
	} while(1);

	return true;
}

