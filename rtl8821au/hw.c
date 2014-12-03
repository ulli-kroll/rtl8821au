#include "hw.h"


void rtl8821au_set_hw_reg(struct rtl_priv *Adapter, uint8_t variable, u8 *val)
{
	 struct rtw_hal	*pHalData = GET_HAL_DATA(Adapter);
	struct dm_priv	*pdmpriv = &pHalData->dmpriv;
	struct rtl_dm *podmpriv = &pHalData->odmpriv;

	switch (variable) {
	case HW_VAR_RXDMA_AGG_PG_TH:
#ifdef CONFIG_USB_RX_AGGREGATION
		{
			/*uint8_t	threshold = *((uint8_t *)val);
			if ( threshold == 0)
			{
				threshold = pHalData->UsbRxAggPageCount;
			}
			rtw_write8(Adapter, REG_RXDMA_AGG_PG_TH, threshold);*/
		}
#endif
		break;
	case HW_VAR_SET_RPWM:
		break;
	case HW_VAR_USB_MODE:
		if (*val == 1)
			rtw_write8(Adapter, REG_OPT_CTRL_8812, 0x4);
		else if (*val == 2)
			rtw_write8(Adapter, REG_OPT_CTRL_8812, 0x8);

		rtw_write8(Adapter, REG_SDIO_CTRL_8812, 0x2);
		rtw_write8(Adapter, REG_ACLK_MON, 0x1);
		/*
		 * 2013/01/29 MH Test with chunchu/cheng, in Alpha AMD platform. when
		 * U2/U3 switch 8812AU will be recognized as another card and then
		 * OS will treat it as a new card and assign a new GUID. Then SWUSB
		 * service can not work well. We need to delay the card switch time to U3
		 * Then OS can unload the previous U2 port card and load new U3 port card later.
		 * The strange sympton can disappear.
		 */
		rtw_write8(Adapter, REG_CAL_TIMER+1, 0x40);
		/* rtw_write8(Adapter, REG_CAL_TIMER+1, 0x3); */
		rtw_write8(Adapter, REG_APS_FSMCO+1, 0x80);
		break;
	default:
		SetHwReg8812A(Adapter, variable, val);
		break;
	}
}

void rtl8821au_get_hw_reg(struct rtl_priv *Adapter, uint8_t variable, u8 *val)
{
		struct rtw_hal	*pHalData = GET_HAL_DATA(Adapter);
		struct rtl_dm *podmpriv = &pHalData->odmpriv;

		switch (variable) {
		default:
			GetHwReg8812A(Adapter, variable, val);
			break;
		}
}
