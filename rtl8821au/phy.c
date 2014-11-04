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
