#include "sw.h"

#include "phy.h"
#include "reg.h"
#include "trx.h"
#include "hw.h"


void rtl8812_free_hal_data(struct rtl_priv *padapter);
void UpdateHalRAMask8812A(struct rtl_priv *padapter, uint32_t mac_id, uint8_t rssi_level);
void ReadChipVersion8812A(struct rtl_priv *Adapter);
VOID rtl8812_EfusePowerSwitch(struct rtl_priv *pAdapter, uint8_t bWrite, uint8_t PwrState);
VOID rtl8812_ReadEFuse(struct rtl_priv *Adapter, uint8_t efuseType, u16 _offset, u16 	_size_byte, uint8_t *pbuf);
VOID rtl8812_EFUSE_GetEfuseDefinition(struct rtl_priv *pAdapter, uint8_t efuseType, uint8_t type, void *pOut);
u16 rtl8812_EfuseGetCurrentSize(struct rtl_priv *pAdapter, uint8_t efuseType);
int rtl8812_Efuse_PgPacketRead(struct rtl_priv *pAdapter, uint8_t offset, uint8_t *data);
int rtl8812_Efuse_PgPacketWrite(struct rtl_priv *pAdapter, uint8_t offset, uint8_t word_en, uint8_t *data);
u8 rtl8812_Efuse_WordEnableDataWrite(struct rtl_priv *pAdapter, u16 efuse_addr, uint8_t word_en, uint8_t *data);
void rtl8812_GetHalODMVar(struct rtl_priv *Adapter, HAL_ODM_VARIABLE eVariable, PVOID pValue1, BOOLEAN bSet);
void rtl8812_SetHalODMVar(struct rtl_priv *Adapter, HAL_ODM_VARIABLE eVariable,	PVOID 	pValue1, BOOLEAN bSet);
void hal_notch_filter_8812(struct rtl_priv *adapter, bool enable);



struct rtl_hal_ops rtl8812au_hal_ops = {
	/*
	 * New HAL functions with struct net_device  as first param
	 * this can be (hopefully)switched to struct ieee80211_hw
	 */

	.init_sw_leds = 		rtl8821au_init_sw_leds,
	.get_bbreg = 	rtl8821au_phy_query_bb_reg,
	.set_bbreg = 	rtl8821au_phy_set_bb_reg,
	.get_rfreg = 	PHY_QueryRFReg8812,
	.set_rfreg = 	PHY_SetRFReg8812,

	.fill_fake_txdesc	= rtl8821au_fill_fake_txdesc,

	/* Old HAL functions */

	.hal_init =	 	rtl8812au_hal_init,
	.hal_deinit = 		rtl8812au_hal_deinit,

	/* .free_hal_data = rtl8192c_free_hal_data, */

	.inirp_init =		rtl8812au_inirp_init,
	.inirp_deinit =		rtl8812au_inirp_deinit,

	.init_xmit_priv =	rtl8812au_init_xmit_priv,
	.free_xmit_priv =	rtl8812au_free_xmit_priv,

	.init_recv_priv =	rtl8812au_init_recv_priv,
	.free_recv_priv =	rtl8812au_free_recv_priv,
	.DeInitSwLeds =		rtl8812au_DeInitSwLeds,
	.init_default_value =	rtl8812au_init_default_value,
	.intf_chip_configure =	rtl8812au_interface_configure,
	.read_adapter_info =	ReadAdapterInfo8812AU,

	/* .set_bwmode_handler = 	PHY_SetBWMode8192C; */
	/* .set_channel_handler = 	PHY_SwChnl8192C; */

	/* .hal_dm_watchdog = 	rtl8192c_HalDmWatchDog; */


	.SetHwRegHandler = 	SetHwReg8812AU,
	.GetHwRegHandler = 	GetHwReg8812AU,
	.GetHalDefVarHandler = 	GetHalDefVar8812AUsb,
	.SetHalDefVarHandler = 	SetHalDefVar8812AUsb,

	.SetBeaconRelatedRegistersHandler = 	SetBeaconRelatedRegisters8812A,

	/* .Add_RateATid = &rtl8192c_Add_RateATid, */

	.hal_xmit = 		rtl8812au_hal_xmit,
	.mgnt_xmit = 		rtl8812au_mgnt_xmit,
	.hal_xmitframe_enqueue = 	rtl8812au_hal_xmitframe_enqueue,

	.interface_ps_func = 	rtl8812au_ps_func,

	.free_hal_data =	rtl8812_free_hal_data,

	.dm_init =		rtl8812_init_dm_priv,
	.dm_deinit =		rtl8812_deinit_dm_priv,

	.UpdateRAMaskHandler =	UpdateHalRAMask8812A,

	.read_chip_version =	ReadChipVersion8812A,

	.set_bwmode_handler =	PHY_SetBWMode8812,
	.set_channel_handler =	PHY_SwChnl8812,
	.set_chnl_bw_handler =	PHY_SetSwChnlBWMode8812,

	.hal_dm_watchdog =	rtl8812_HalDmWatchDog,

	.Add_RateATid =		rtl8812_Add_RateATid,
#ifdef CONFIG_ANTENNA_DIVERSITY
	.AntDivBeforeLinkHandler =	AntDivBeforeLink8812,
	.AntDivCompareHandler =		AntDivCompare8812,
#endif

	/* Efuse related function */
	.EfusePowerSwitch =	rtl8812_EfusePowerSwitch,
	.ReadEFuse =		rtl8812_ReadEFuse,
	.EFUSEGetEfuseDefinition =	rtl8812_EFUSE_GetEfuseDefinition,
	.EfuseGetCurrentSize =	rtl8812_EfuseGetCurrentSize,
	.Efuse_PgPacketRead =	rtl8812_Efuse_PgPacketRead,
	.Efuse_PgPacketWrite =	rtl8812_Efuse_PgPacketWrite,
	.Efuse_WordEnableDataWrite = &rtl8812_Efuse_WordEnableDataWrite,

#ifdef DBG_CONFIG_ERROR_DETECT
	.sreset_init_value =	sreset_init_value,
	.sreset_reset_value =	sreset_reset_value,
	.silentreset =		sreset_reset,
	.sreset_xmit_status_check =	rtl8812_sreset_xmit_status_check,
	.sreset_linked_status_check =	rtl8812_sreset_linked_status_check,
	.sreset_get_wifi_status  =	sreset_get_wifi_status,
	.sreset_inprogress =		sreset_inprogress,
#endif

	.GetHalODMVarHandler = rtl8812_GetHalODMVar,
	.SetHalODMVarHandler = rtl8812_SetHalODMVar,
	.hal_notch_filter = hal_notch_filter_8812,

	.SetBeaconRelatedRegistersHandler =	SetBeaconRelatedRegisters8812A,
};

