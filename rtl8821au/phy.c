#include <rtl8812a_hal.h>
#include "phy.h"
//
// 1. BB register R/W API
//

u32 rtl8821au_phy_query_bb_reg(struct rtl_priv *Adapter, uint32_t RegAddr, uint32_t BitMask)
{
	uint32_t ReturnValue = 0, OriginalValue, BitShift;

#if (DISABLE_BB_RF == 1)
	return 0;
#endif

	//DBG_871X("--->PHY_QueryBBReg8812(): RegAddr(%#x), BitMask(%#x)\n", RegAddr, BitMask);


	OriginalValue = rtw_read32(Adapter, RegAddr);
	BitShift = PHY_CalculateBitShift(BitMask);
	ReturnValue = (OriginalValue & BitMask) >> BitShift;

	//DBG_871X("BBR MASK=0x%x Addr[0x%x]=0x%x\n", BitMask, RegAddr, OriginalValue);
	return (ReturnValue);
}


void rtl8821au_phy_set_bb_reg(struct rtl_priv *Adapter, u32 RegAddr, u32 BitMask, u32 Data)
{
	uint32_t OriginalValue, BitShift;

#if (DISABLE_BB_RF == 1)
	return;
#endif

	if(BitMask!= bMaskDWord)
	{//if not "double word" write
		OriginalValue = rtw_read32(Adapter, RegAddr);
		BitShift = PHY_CalculateBitShift(BitMask);
		Data = ((OriginalValue) & (~BitMask)) |( ((Data << BitShift)) & BitMask);
	}

	rtw_write32(Adapter, RegAddr, Data);

	//DBG_871X("BBW MASK=0x%x Addr[0x%x]=0x%x\n", BitMask, RegAddr, Data);
}

#if 0

/* from linux-master */

static void _rtl8821ae_phy_iq_calibrate(struct ieee80211_hw *hw)
{
	u32	macbb_backup[MACBB_REG_NUM];
	u32 afe_backup[AFE_REG_NUM];
	u32 rfa_backup[RF_REG_NUM];
	u32 rfb_backup[RF_REG_NUM];
	u32 backup_macbb_reg[MACBB_REG_NUM] = {
		0xb00, 0x520, 0x550, 0x808, 0x90c, 0xc00, 0xc50,
		0xe00, 0xe50, 0x838, 0x82c
	};
	u32 backup_afe_reg[AFE_REG_NUM] = {
		0xc5c, 0xc60, 0xc64, 0xc68, 0xc6c, 0xc70, 0xc74,
		0xc78, 0xc7c, 0xc80, 0xc84, 0xcb8
	};
	u32	backup_rf_reg[RF_REG_NUM] = {0x65, 0x8f, 0x0};

	_rtl8821ae_iqk_backup_macbb(hw, macbb_backup, backup_macbb_reg,
				    MACBB_REG_NUM);
	_rtl8821ae_iqk_backup_afe(hw, afe_backup, backup_afe_reg, AFE_REG_NUM);
	_rtl8821ae_iqk_backup_rf(hw, rfa_backup, rfb_backup, backup_rf_reg,
				 RF_REG_NUM);

	_rtl8821ae_iqk_configure_mac(hw);
	_rtl8821ae_iqk_tx(hw, RF90_PATH_A);
	_rtl8821ae_iqk_restore_rf(hw, RF90_PATH_A, backup_rf_reg, rfa_backup,
				  RF_REG_NUM);

	_rtl8821ae_iqk_restore_afe(hw, afe_backup, backup_afe_reg, AFE_REG_NUM);
	_rtl8821ae_iqk_restore_macbb(hw, macbb_backup, backup_macbb_reg,
				     MACBB_REG_NUM);
}

#endif
