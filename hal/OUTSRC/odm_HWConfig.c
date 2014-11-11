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

/*
 * ============================================================
 *  include files
 * ============================================================
 */


#include "odm_precomp.h"
#include "../../rtl8821au/phy.h"

u8 odm_QueryRxPwrPercentage(s1Byte AntPower)
{
	if ((AntPower <= -100) || (AntPower >= 20)) {
		return	0;
	} else if (AntPower >= 0) {
		return	100;
	} else {
		return	(100+AntPower);
	}

}

/*
 * 2012/01/12 MH MOve some signal strength smooth method to MP HAL layer.
 * IF other SW team do not support the feature, remove this section.??
 */

static int32_t odm_SignalScaleMapping_92CSeries(struct rtl_dm *pDM_Odm, int32_t CurrSig)
{
	int32_t RetSig;

#if ((DEV_BUS_TYPE == RT_USB_INTERFACE))
	if ((pDM_Odm->SupportInterface  == ODM_ITRF_USB)) {
		if (CurrSig >= 51 && CurrSig <= 100) {
			RetSig = 100;
		} else if (CurrSig >= 41 && CurrSig <= 50) {
			RetSig = 80 + ((CurrSig - 40)*2);
		} else if (CurrSig >= 31 && CurrSig <= 40) {
			RetSig = 66 + (CurrSig - 30);
		} else if (CurrSig >= 21 && CurrSig <= 30) {
			RetSig = 54 + (CurrSig - 20);
		} else if (CurrSig >= 10 && CurrSig <= 20) {
			RetSig = 42 + (((CurrSig - 10) * 2) / 3);
		} else if (CurrSig >= 5 && CurrSig <= 9) {
			RetSig = 22 + (((CurrSig - 5) * 3) / 2);
		} else if (CurrSig >= 1 && CurrSig <= 4) {
			RetSig = 6 + (((CurrSig - 1) * 3) / 2);
		} else {
			RetSig = CurrSig;
		}
	}

#endif
	return RetSig;
}

int32_t odm_SignalScaleMapping(struct rtl_dm *pDM_Odm, int32_t CurrSig)
{
	return odm_SignalScaleMapping_92CSeries(pDM_Odm, CurrSig);
}

static u8 odm_EVMdbToPercentage(s1Byte Value)
{
	/*
	 *  -33dB~0dB to 0%~99%
	 */
	s1Byte ret_val;

	ret_val = Value;
	/* ret_val /= 2; */

	/* ODM_RT_DISP(FRX, RX_PHY_SQ, ("EVMdbToPercentage92C Value=%d / %x \n", ret_val, ret_val)); */

	if (ret_val >= 0)
		ret_val = 0;
	if (ret_val <= -33)
		ret_val = -33;

	ret_val = 0 - ret_val;
	ret_val *= 3;

	if (ret_val == 99)
		ret_val = 100;

	return ret_val;
}

static u8 odm_EVMdbm_JaguarSeries(s1Byte Value)
{
	s1Byte ret_val = Value;

	/* -33dB~0dB to 33dB ~ 0dB */
	if (ret_val == -128)
		ret_val = 127;
	else if (ret_val < 0)
		ret_val = 0 - ret_val;

	ret_val  = ret_val >> 1;
	return ret_val;
}

static uint16_t odm_Cfo(s1Byte Value)
{
	s2Byte  ret_val;

	if (Value < 0) {
		ret_val = 0 - Value;
		ret_val = (ret_val << 1) + (ret_val >> 1);	/*  *2.5~=312.5/2^7 */
		ret_val = ret_val | BIT12;  			/* set bit12 as 1 for negative cfo */
	} else {
		ret_val = Value;
		ret_val = (ret_val << 1) + (ret_val>>1);	/*  *2.5~=312.5/2^7 */
	}
	return ret_val;
}

void odm_RxPhyStatusJaguarSeries_Parsing(struct rtl_dm *pDM_Odm,
	PODM_PHY_INFO_T pPhyInfo, u8 *pPhyStatus, PODM_PACKET_INFO_T pPktinfo)
{
	u8	i, Max_spatial_stream;
	s1Byte	rx_pwr[4], rx_pwr_all = 0;
	u8	EVM, EVMdbm, PWDB_ALL = 0, PWDB_ALL_BT;
	u8	RSSI, total_rssi = 0;
	u8	isCCKrate = 0;
	u8	rf_rx_num = 0;
	u8	cck_highpwr = 0;
	u8	LNA_idx, VGA_idx;

	PPHY_STATUS_RPT_8812_T pPhyStaRpt = (PPHY_STATUS_RPT_8812_T)pPhyStatus;

	if (pPktinfo->DataRate <= DESC_RATE54M) {
		switch (pPhyStaRpt->r_RFMOD) {
		case 1:
			if (pPhyStaRpt->sub_chnl == 0)
				pPhyInfo->BandWidth = 1;
			else
				pPhyInfo->BandWidth = 0;
			break;

		case 2:
			if (pPhyStaRpt->sub_chnl == 0)
				pPhyInfo->BandWidth = 2;
			else if (pPhyStaRpt->sub_chnl == 9 || pPhyStaRpt->sub_chnl == 10)
				pPhyInfo->BandWidth = 1;
			else
				pPhyInfo->BandWidth = 0;
			break;

		default:
		case 0:
			pPhyInfo->BandWidth = 0;
			break;
		}
	}

	if (pPktinfo->DataRate <= DESC_RATE11M)
		isCCKrate = TRUE;
	else
		isCCKrate = FALSE;

	pPhyInfo->RxMIMOSignalQuality[ODM_RF_PATH_A] = -1;
	pPhyInfo->RxMIMOSignalQuality[ODM_RF_PATH_B] = -1;

	if (isCCKrate) {
		u8 cck_agc_rpt;

		pDM_Odm->PhyDbgInfo.NumQryPhyStatusCCK++;
		/*
		 *  (1)Hardware does not provide RSSI for CCK
		 *  (2)PWDB, Average PWDB cacluated by hardware (for rate adaptive)
		 */

		/* if (pHalData->eRFPowerState == eRfOn) */
			cck_highpwr = pDM_Odm->bCckHighPower;
		/*
		 * else
		 * 	cck_highpwr = FALSE;
		 */

		cck_agc_rpt =  pPhyStaRpt->cfosho[0] ;

		LNA_idx = ((cck_agc_rpt & 0xE0) >> 5);
		VGA_idx = (cck_agc_rpt & 0x1F);
		if (pDM_Odm->SupportICType == ODM_RTL8812) {
			switch (LNA_idx) {
			case 7:
				if (VGA_idx <= 27)
					rx_pwr_all = -100 + 2*(27-VGA_idx);	/* VGA_idx = 27~2 */
				else
					rx_pwr_all = -100;
				break;

			case 6:
				rx_pwr_all = -48 + 2*(2-VGA_idx);	/* VGA_idx = 2~0 */
				break;

			case 5:
				rx_pwr_all = -42 + 2*(7-VGA_idx);	/* VGA_idx = 7~5 */
				break;

			case 4:
				rx_pwr_all = -36 + 2*(7-VGA_idx);	/* VGA_idx = 7~4 */
				break;

			case 3:
				/* rx_pwr_all = -28 + 2*(7-VGA_idx); */	/* VGA_idx = 7~0 */
				rx_pwr_all = -24 + 2*(7-VGA_idx); 	/* VGA_idx = 7~0 */
				break;

			case 2:
				if (cck_highpwr)
					rx_pwr_all = -12 + 2*(5-VGA_idx);	/* VGA_idx = 5~0 */
				else
					rx_pwr_all = -6 + 2*(5-VGA_idx);
				break;
			case 1:
				rx_pwr_all = 8-2*VGA_idx;
				break;
			case 0:
				rx_pwr_all = 14-2*VGA_idx;
				break;
			default:
				/* DbgPrint("CCK Exception default\n"); */
				break;
			}
			rx_pwr_all += 6;
			PWDB_ALL = odm_QueryRxPwrPercentage(rx_pwr_all);
			if (cck_highpwr == FALSE) {
				if (PWDB_ALL >= 80)
					PWDB_ALL = ((PWDB_ALL-80)<<1)+((PWDB_ALL-80)>>1)+80;
				else if ((PWDB_ALL <= 78) && (PWDB_ALL >= 20))
					PWDB_ALL += 3;

				if (PWDB_ALL > 100)
					PWDB_ALL = 100;
			}
		} else if (pDM_Odm->SupportICType == ODM_RTL8821) {
			s1Byte Pout = -6;

			switch (LNA_idx) {
			case 5:
				rx_pwr_all = Pout - 32 - (2*VGA_idx);
				break;
			case 4:
				rx_pwr_all = Pout - 24 - (2*VGA_idx);
				break;
			case 2:
				rx_pwr_all = Pout - 11 - (2*VGA_idx);
				break;
			case 1:
				rx_pwr_all = Pout + 5 - (2*VGA_idx);
				break;
			case 0:
				rx_pwr_all = Pout + 21 - (2*VGA_idx);
				break;
			}
			PWDB_ALL = odm_QueryRxPwrPercentage(rx_pwr_all);
		}

		pPhyInfo->RxPWDBAll = PWDB_ALL;
		/*
		 * if (pPktinfo->StationID == 0) {
		 * 	DbgPrint("CCK: LNA_idx = %d, VGA_idx = %d, pPhyInfo->RxPWDBAll = %d\n",
		 * 		LNA_idx, VGA_idx, pPhyInfo->RxPWDBAll);
		 * }
		 */
#if (DM_ODM_SUPPORT_TYPE &  (ODM_CE))
		pPhyInfo->BTRxRSSIPercentage = PWDB_ALL;
		pPhyInfo->RecvSignalPower = rx_pwr_all;
#endif
		/*
		 * (3) Get Signal Quality (EVM)
		 */

		if (pPktinfo->bPacketMatchBSSID) {
			u8	SQ, SQ_rpt;

			if (pPhyInfo->RxPWDBAll > 40 && !pDM_Odm->bInHctTest) {
				SQ = 100;
			} else {
				SQ_rpt = pPhyStaRpt->pwdb_all;

				if (SQ_rpt > 64)
					SQ = 0;
				else if (SQ_rpt < 20)
					SQ = 100;
				else
					SQ = ((64-SQ_rpt) * 100) / 44;

			}

			/* DbgPrint("cck SQ = %d\n", SQ); */
			pPhyInfo->SignalQuality = SQ;
			pPhyInfo->RxMIMOSignalQuality[ODM_RF_PATH_A] = SQ;
			pPhyInfo->RxMIMOSignalQuality[ODM_RF_PATH_B] = -1;
		}
	} else {
		/* is OFDM rate */
		pDM_Odm->PhyDbgInfo.NumQryPhyStatusOFDM++;

		/*
		 * (1)Get RSSI for OFDM rate
		 */

		for (i = ODM_RF_PATH_A; i < ODM_RF_PATH_MAX; i++) {
			/*
			 *  2008/01/30 MH we will judge RF RX path now.
			 * DbgPrint("pDM_Odm->RFPathRxEnable = %x\n", pDM_Odm->RFPathRxEnable);
			 */
			if (pDM_Odm->RFPathRxEnable & BIT(i)) {
				rf_rx_num++;
			}
			/*
			 * else
			 * 	continue;
			 */

			/*
			 * 2012.05.25 LukeLee: Testchip AGC report is wrong, it should be restored back to old formula in MP chip
			 * if ((pDM_Odm->SupportICType & (ODM_RTL8812|ODM_RTL8821)) && (!pDM_Odm->bIsMPChip))
			 * 	rx_pwr[i] = (pPhyStaRpt->gain_trsw[i]&0x7F) - 110;
			 * else
			 * 	rx_pwr[i] = ((pPhyStaRpt->gain_trsw[i]& 0x3F)*2) - 110;  //OLD FORMULA
			 */

#if (DM_ODM_SUPPORT_TYPE & (ODM_CE))
			pPhyInfo->RxPwr[i] = rx_pwr[i];
#endif

			/* Translate DBM to percentage. */
			RSSI = odm_QueryRxPwrPercentage(rx_pwr[i]);

			total_rssi += RSSI;
			/* RT_DISP(FRX, RX_PHY_SS, ("RF-%d RXPWR=%x RSSI=%d\n", i, rx_pwr[i], RSSI)); */



			pPhyInfo->RxMIMOSignalStrength[i] = (u8) RSSI;

#if (DM_ODM_SUPPORT_TYPE &  (ODM_CE))
			/* Get Rx snr value in DB */
			pPhyInfo->RxSNR[i] = pDM_Odm->PhyDbgInfo.RxSNRdB[i] = pPhyStaRpt->rxsnr[i]/2;
#endif

			/*
			 *  (2) CFO_short  & CFO_tail
			 */

			pPhyInfo->Cfo_short[i] = odm_Cfo((pPhyStaRpt->cfosho[i]));
			pPhyInfo->Cfo_tail[i] = odm_Cfo((pPhyStaRpt->cfotail[i]));

			/* Record Signal Strength for next packet */
			if (pPktinfo->bPacketMatchBSSID) {
				;
			}
		}


		/*
		 *  (3)PWDB, Average PWDB cacluated by hardware (for rate adaptive)
		 *
		 * 2012.05.25 LukeLee: Testchip AGC report is wrong, it should be restored back to old formula in MP chip
		 */
		if ((pDM_Odm->SupportICType & (ODM_RTL8812|ODM_RTL8821)) && (!pDM_Odm->bIsMPChip))
			rx_pwr_all = (pPhyStaRpt->pwdb_all & 0x7f) - 110;
		else
			rx_pwr_all = (((pPhyStaRpt->pwdb_all) >> 1) & 0x7f) - 110;	 /* OLD FORMULA */

		PWDB_ALL_BT = PWDB_ALL = odm_QueryRxPwrPercentage(rx_pwr_all);


		pPhyInfo->RxPWDBAll = PWDB_ALL;
		/* ODM_RT_TRACE(pDM_Odm,ODM_COMP_RSSI_MONITOR, ODM_DBG_LOUD, ("ODM OFDM RSSI=%d\n",pPhyInfo->RxPWDBAll)); */
#if (DM_ODM_SUPPORT_TYPE &  (ODM_CE))
		pPhyInfo->BTRxRSSIPercentage = PWDB_ALL_BT;
		pPhyInfo->RxPower = rx_pwr_all;
		pPhyInfo->RecvSignalPower = rx_pwr_all;
#endif

		/*
		 * DbgPrint("OFDM: pPhyInfo->RxPWDBAll = %d, pPhyInfo->RxMIMOSignalStrength[0] = %d, pPhyInfo->RxMIMOSignalStrength[1] = %d\n",
		 *	pPhyInfo->RxPWDBAll, pPhyInfo->RxMIMOSignalStrength[0], pPhyInfo->RxMIMOSignalStrength[1]);
		 */


		{	/* pMgntInfo->CustomerID != RT_CID_819x_Lenovo */
			/*
			 * (4)EVM of OFDM rate
			 */

			if ((pPktinfo->DataRate >= DESC_RATEMCS8)
			 && (pPktinfo->DataRate <= DESC_RATEMCS15))
				Max_spatial_stream = 2;
			else if ((pPktinfo->DataRate >= DESC_RATEVHTSS2MCS0)
			      && (pPktinfo->DataRate <= DESC_RATEVHTSS2MCS9))
				Max_spatial_stream = 2;
			else
				Max_spatial_stream = 1;

			for (i = 0; i < Max_spatial_stream; i++) {
				/*
				 *  Do not use shift operation like "rx_evmX >>= 1" because the compilor of free build environment
				 *  fill most significant bit to "zero" when doing shifting operation which may change a negative
				 *  value to positive one, then the dbm value (which is supposed to be negative)  is not correct anymore.
				 */
				EVM = odm_EVMdbToPercentage((pPhyStaRpt->rxevm[i]));	/* dbm */
				EVMdbm = odm_EVMdbm_JaguarSeries(pPhyStaRpt->rxevm[i]);
				/*
				 * RT_DISP(FRX, RX_PHY_SQ, ("RXRATE=%x RXEVM=%x EVM=%s%d\n",
				 * pPktinfo->DataRate, pPhyStaRpt->rxevm[i], "%", EVM));
				 */

				if (pPktinfo->bPacketMatchBSSID) {
					if (i == ODM_RF_PATH_A) {	/* Fill value in RFD, Get the first spatial stream only */
						pPhyInfo->SignalQuality = EVM;
					}
					pPhyInfo->RxMIMOSignalQuality[i] = EVM;
					pPhyInfo->RxMIMOEVMdbm[i] = EVMdbm;
				}
			}
		}
		/* 2 For dynamic ATC switch */
		if (pDM_Odm->SupportAbility & ODM_BB_DYNAMIC_ATC) {
			if (pPktinfo->bPacketMatchBSSID) {
				/* 3 Update CFO report for path-A & path-B */
				for (i = ODM_RF_PATH_A; i < ODM_RF_PATH_MAX; i++) {
					pDM_Odm->CFO_tail[i] = (int)pPhyStaRpt->cfotail[i];
				}

				/* 3 Update packet counter */
				if (pDM_Odm->packetCount == 0xffffffff)
					pDM_Odm->packetCount = 0;
				else
					pDM_Odm->packetCount++;

				/*
				 * ODM_RT_TRACE(pDM_Odm, ODM_COMP_DYNAMIC_ATC, ODM_DBG_LOUD,
				 * 	("pPhyStaRpt->path_cfotail[i] = 0x%x, pDM_Odm->CFO_tail[i] = 0x%x\n", pPhyStaRpt->path_cfotail[0], pDM_Odm->CFO_tail[1]));
				 */
			}
		}
	}
	/* DbgPrint("isCCKrate= %d, pPhyInfo->SignalStrength=%d % PWDB_AL=%d rf_rx_num=%d\n", isCCKrate, pPhyInfo->SignalStrength, PWDB_ALL, rf_rx_num); */

#if (DM_ODM_SUPPORT_TYPE &  (ODM_CE))
	/*
	 * UI BSS List signal strength(in percentage), make it good looking, from 0~100.
	 * It is assigned to the BSS List in GetValueFromBeaconOrProbeRsp().
	 */
	if (isCCKrate) {
		pPhyInfo->SignalStrength = (u8)(odm_SignalScaleMapping(pDM_Odm, PWDB_ALL));	/* PWDB_ALL; */
	} else {
		if (rf_rx_num != 0) {
			/* ULLI crap inside this call */
			pPhyInfo->SignalStrength = (u8)(odm_SignalScaleMapping(pDM_Odm, total_rssi /= rf_rx_num));
		}
	}
#endif
	pDM_Odm->RxPWDBAve = pDM_Odm->RxPWDBAve + pPhyInfo->RxPWDBAll;

	pDM_Odm->DM_FatTable.antsel_rx_keep_0 = pPhyStaRpt->antidx_anta;

	/*
	 * DbgPrint("pPhyStaRpt->antidx_anta = %d, pPhyStaRpt->antidx_antb = %d, pPhyStaRpt->resvd_1 = %d",
	 * 	pPhyStaRpt->antidx_anta, pPhyStaRpt->antidx_antb, pPhyStaRpt->resvd_1);
	 */

	/*
	 * DbgPrint("----------------------------\n");
	 * DbgPrint("pPktinfo->StationID=%d, pPktinfo->DataRate=0x%x\n",pPktinfo->StationID, pPktinfo->DataRate);
	 * DbgPrint("pPhyStaRpt->gain_trsw[0]=0x%x, pPhyStaRpt->gain_trsw[1]=0x%x, pPhyStaRpt->pwdb_all=0x%x\n",
	 *	pPhyStaRpt->gain_trsw[0],pPhyStaRpt->gain_trsw[1], pPhyStaRpt->pwdb_all);
	 * DbgPrint("pPhyInfo->RxMIMOSignalStrength[0]=%d, pPhyInfo->RxMIMOSignalStrength[1]=%d, RxPWDBAll=%d\n",
	 *	pPhyInfo->RxMIMOSignalStrength[0], pPhyInfo->RxMIMOSignalStrength[1], pPhyInfo->RxPWDBAll);
	 */

}


void odm_Process_RSSIForDM(struct rtl_dm *pDM_Odm, PODM_PHY_INFO_T pPhyInfo,
	PODM_PACKET_INFO_T pPktinfo)
{
	int32_t		UndecoratedSmoothedPWDB, UndecoratedSmoothedCCK, UndecoratedSmoothedOFDM, RSSI_Ave;
	u8		isCCKrate = 0;
	u8		RSSI_max, RSSI_min, i;
	uint32_t	OFDM_pkt = 0;
	uint32_t	Weighting = 0;

	PSTA_INFO_T   	pEntry;

	if (pPktinfo->StationID == 0xFF)
		return;

	pEntry = pDM_Odm->pODM_StaInfo[pPktinfo->StationID];

	if (!IS_STA_VALID(pEntry)) {
		return;
	}
	if ((!pPktinfo->bPacketMatchBSSID)) {
		return;
	}

	if (pPktinfo->bPacketBeacon)
		pDM_Odm->PhyDbgInfo.NumQryBeaconPkt++;
	isCCKrate = (pPktinfo->DataRate <= DESC92C_RATE11M) ? TRUE : FALSE;
	pDM_Odm->RxRate = pPktinfo->DataRate;
	/*
	if (!isCCKrate)
	{
		DbgPrint("OFDM: pPktinfo->StationID=%d, isCCKrate=%d, pPhyInfo->RxPWDBAll=%d\n",
			pPktinfo->StationID, isCCKrate, pPhyInfo->RxPWDBAll);
	}
	*/
#if (defined(CONFIG_HW_ANTENNA_DIVERSITY))
	/* -----------------Smart Antenna Debug Message------------------ */
#if (RTL8821A_SUPPORT == 1)
	if (pDM_Odm->SupportICType == ODM_RTL8821) {
		pFAT_T	pDM_FatTable = &pDM_Odm->DM_FatTable;

		if (pPktinfo->bPacketToSelf || pPktinfo->bPacketMatchBSSID) {
			if (pPktinfo->DataRate > DESC8812_RATE11M)
				ODM_AntselStatistics_8821A(pDM_Odm, pDM_FatTable->antsel_rx_keep_0, pPktinfo->StationID, pPhyInfo->RxPWDBAll);
		}
	}
#endif

#endif
#if (RTL8812A_SUPPORT == 1)
/*
	if (pDM_Odm->SupportICType == ODM_RTL8812)
	{
		pPATHDIV_T	pDM_PathDiv = &pDM_Odm->DM_PathDiv;
		if (pPktinfo->bPacketToSelf || pPktinfo->bPacketMatchBSSID)
		{
			if (pPktinfo->DataRate > DESC8812_RATE11M)
				ODM_PathStatistics_8812A(pDM_Odm, pPktinfo->StationID,
				pPhyInfo->RxMIMOSignalStrength[ODM_RF_PATH_A], pPhyInfo->RxMIMOSignalStrength[ODM_RF_PATH_B]);
		}
	}
*/
#endif
	/* -----------------Smart Antenna Debug Message------------------ */

	UndecoratedSmoothedCCK =  pEntry->rssi_stat.UndecoratedSmoothedCCK;
	UndecoratedSmoothedOFDM = pEntry->rssi_stat.UndecoratedSmoothedOFDM;
	UndecoratedSmoothedPWDB = pEntry->rssi_stat.UndecoratedSmoothedPWDB;

	if (pPktinfo->bPacketToSelf || pPktinfo->bPacketBeacon) {
		if (!isCCKrate) {	/* ofdm rate */
			if (pPhyInfo->RxMIMOSignalStrength[ODM_RF_PATH_B] == 0) {
				RSSI_Ave = pPhyInfo->RxMIMOSignalStrength[ODM_RF_PATH_A];
				pDM_Odm->RSSI_A = pPhyInfo->RxMIMOSignalStrength[ODM_RF_PATH_A];
				pDM_Odm->RSSI_B = 0;
			} else{
				/*
				 * DbgPrint("pRfd->Status.RxMIMOSignalStrength[0] = %d, pRfd->Status.RxMIMOSignalStrength[1] = %d \n",
				 * 	pRfd->Status.RxMIMOSignalStrength[0], pRfd->Status.RxMIMOSignalStrength[1]);
				 */
				pDM_Odm->RSSI_A =  pPhyInfo->RxMIMOSignalStrength[ODM_RF_PATH_A];
				pDM_Odm->RSSI_B = pPhyInfo->RxMIMOSignalStrength[ODM_RF_PATH_B];

				if (pPhyInfo->RxMIMOSignalStrength[ODM_RF_PATH_A] > pPhyInfo->RxMIMOSignalStrength[ODM_RF_PATH_B]) {
					RSSI_max = pPhyInfo->RxMIMOSignalStrength[ODM_RF_PATH_A];
					RSSI_min = pPhyInfo->RxMIMOSignalStrength[ODM_RF_PATH_B];
				} else {
					RSSI_max = pPhyInfo->RxMIMOSignalStrength[ODM_RF_PATH_B];
					RSSI_min = pPhyInfo->RxMIMOSignalStrength[ODM_RF_PATH_A];
				}

				if ((RSSI_max - RSSI_min) < 3)
					RSSI_Ave = RSSI_max;
				else if ((RSSI_max - RSSI_min) < 6)
					RSSI_Ave = RSSI_max - 1;
				else if ((RSSI_max - RSSI_min) < 10)
					RSSI_Ave = RSSI_max - 2;
				else
					RSSI_Ave = RSSI_max - 3;
			}

			/* 1 Process OFDM RSSI */
			if (UndecoratedSmoothedOFDM <= 0) {
				/* initialize */
				UndecoratedSmoothedOFDM = pPhyInfo->RxPWDBAll;
			} else {
				if (pPhyInfo->RxPWDBAll > (uint32_t)UndecoratedSmoothedOFDM) {
					UndecoratedSmoothedOFDM =
						(((UndecoratedSmoothedOFDM)*(Rx_Smooth_Factor-1))
						+ (RSSI_Ave)) / (Rx_Smooth_Factor);
					UndecoratedSmoothedOFDM = UndecoratedSmoothedOFDM + 1;
				} else {
					UndecoratedSmoothedOFDM =
						(((UndecoratedSmoothedOFDM)*(Rx_Smooth_Factor-1))
						+ (RSSI_Ave)) / (Rx_Smooth_Factor);
				}
			}

			pEntry->rssi_stat.PacketMap = (pEntry->rssi_stat.PacketMap<<1) | BIT0;
		} else {
			RSSI_Ave = pPhyInfo->RxPWDBAll;
			pDM_Odm->RSSI_A = (u8) pPhyInfo->RxPWDBAll;
			pDM_Odm->RSSI_B = 0xFF;

			/* 1 Process CCK RSSI */
			if (UndecoratedSmoothedCCK <= 0)	{	/*  initialize */
				UndecoratedSmoothedCCK = pPhyInfo->RxPWDBAll;
			} else 	{
				if (pPhyInfo->RxPWDBAll > (uint32_t)UndecoratedSmoothedCCK) {
					UndecoratedSmoothedCCK =
						(((UndecoratedSmoothedCCK)*(Rx_Smooth_Factor-1))
						+ (pPhyInfo->RxPWDBAll)) / (Rx_Smooth_Factor);
					UndecoratedSmoothedCCK = UndecoratedSmoothedCCK + 1;
				} else {
					UndecoratedSmoothedCCK =
						(((UndecoratedSmoothedCCK)*(Rx_Smooth_Factor-1))
						+ (pPhyInfo->RxPWDBAll)) / (Rx_Smooth_Factor);
				}
			}
			pEntry->rssi_stat.PacketMap = pEntry->rssi_stat.PacketMap<<1;
		}

		/* if (pEntry) */
		{
			/* 2011.07.28 LukeLee: modified to prevent unstable CCK RSSI */
			if (pEntry->rssi_stat.ValidBit >= 64)
				pEntry->rssi_stat.ValidBit = 64;
			else
				pEntry->rssi_stat.ValidBit++;

			for (i = 0; i < pEntry->rssi_stat.ValidBit; i++)
				OFDM_pkt += (u8)(pEntry->rssi_stat.PacketMap>>i)&BIT0;

			if (pEntry->rssi_stat.ValidBit == 64) {
				Weighting = ((OFDM_pkt<<4) > 64)?64:(OFDM_pkt<<4);
				UndecoratedSmoothedPWDB = (Weighting*UndecoratedSmoothedOFDM+(64-Weighting)*UndecoratedSmoothedCCK)>>6;
			} else {
				if (pEntry->rssi_stat.ValidBit != 0)
					UndecoratedSmoothedPWDB = (OFDM_pkt*UndecoratedSmoothedOFDM+(pEntry->rssi_stat.ValidBit-OFDM_pkt)*UndecoratedSmoothedCCK)/pEntry->rssi_stat.ValidBit;
				else
					UndecoratedSmoothedPWDB = 0;
			}

			pEntry->rssi_stat.UndecoratedSmoothedCCK = UndecoratedSmoothedCCK;
			pEntry->rssi_stat.UndecoratedSmoothedOFDM = UndecoratedSmoothedOFDM;
			pEntry->rssi_stat.UndecoratedSmoothedPWDB = UndecoratedSmoothedPWDB;

			/*
			 * DbgPrint("OFDM_pkt=%d, Weighting=%d\n", OFDM_pkt, Weighting);
			 * DbgPrint("UndecoratedSmoothedOFDM=%d, UndecoratedSmoothedPWDB=%d, UndecoratedSmoothedCCK=%d\n",
			 * 	UndecoratedSmoothedOFDM, UndecoratedSmoothedPWDB, UndecoratedSmoothedCCK);
			 */

		}

	}
}

/*
 * Endianness before calling this API
 */

void ODM_PhyStatusQuery_JaguarSeries(struct rtl_dm *	pDM_Odm,
	PODM_PHY_INFO_T pPhyInfo, u8 *pPhyStatus, PODM_PACKET_INFO_T pPktinfo)
{
	odm_RxPhyStatusJaguarSeries_Parsing(pDM_Odm, pPhyInfo,
					pPhyStatus, pPktinfo);

	odm_Process_RSSIForDM(pDM_Odm, pPhyInfo, pPktinfo);
}

void ODM_PhyStatusQuery(struct rtl_dm *pDM_Odm, PODM_PHY_INFO_T pPhyInfo,
	u8 *pPhyStatus, PODM_PACKET_INFO_T pPktinfo)
{
	ODM_PhyStatusQuery_JaguarSeries(pDM_Odm, pPhyInfo, pPhyStatus, pPktinfo);
}


/*
 * If you want to add a new IC, Please follow below template and generate a new one.
 */

HAL_STATUS ODM_ConfigRFWithHeaderFile(struct rtl_dm *pDM_Odm,
	ODM_RF_Config_Type ConfigType, ODM_RF_RADIO_PATH_E eRFPath)
{
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_LOUD,
		("===>ODM_ConfigRFWithHeaderFile (%s)\n", (pDM_Odm->bIsMPChip) ? "MPChip" : "TestChip"));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_LOUD,
		("pDM_Odm->SupportPlatform: 0x%X, pDM_Odm->SupportInterface: 0x%X, pDM_Odm->BoardType: 0x%X\n",
		pDM_Odm->SupportPlatform, pDM_Odm->SupportInterface, pDM_Odm->BoardType));

#if (RTL8812A_SUPPORT == 1)
	if (pDM_Odm->SupportICType == ODM_RTL8812) {
		switch (ConfigType) {
		case CONFIG_RF_RADIO:
			rtl8812au_phy_config_rf_with_headerfile(pDM_Odm, eRFPath);
			break;

		default:
			;
		}
	}
#endif

#if (RTL8821A_SUPPORT == 1)
	if (pDM_Odm->SupportICType == ODM_RTL8821) {
		switch (ConfigType) {
		case CONFIG_RF_RADIO:
			if (eRFPath == ODM_RF_PATH_A) {
				ODM_ReadAndConfig_MP_8821A_RadioA(pDM_Odm, ODM_RF_PATH_A);
			}
			break;
		default:
			;
		}
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_LOUD, ("<===8821_ODM_ConfigRFWithHeaderFile\n"));
	}
#endif
	return HAL_STATUS_SUCCESS;
}

HAL_STATUS ODM_ConfigRFWithTxPwrTrackHeaderFile(struct rtl_dm *pDM_Odm)
{
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_LOUD,
		("===>ODM_ConfigRFWithTxPwrTrackHeaderFile (%s)\n", (pDM_Odm->bIsMPChip) ? "MPChip" : "TestChip"));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_LOUD,
		("pDM_Odm->SupportPlatform: 0x%X, pDM_Odm->SupportInterface: 0x%X, pDM_Odm->BoardType: 0x%X\n",
		pDM_Odm->SupportPlatform, pDM_Odm->SupportInterface, pDM_Odm->BoardType));
#if (RTL8821A_SUPPORT == 1)
	if (pDM_Odm->SupportICType == ODM_RTL8821) {
		switch (pDM_Odm->SupportInterface) {
		case ODM_ITRF_USB:
			ODM_ReadAndConfig_MP_8821A_TxPowerTrack_USB(pDM_Odm);
			break;

		default:
			;
		}
	}
#endif

#if (RTL8812A_SUPPORT == 1)
	if (pDM_Odm->SupportICType == ODM_RTL8812) {
		switch (pDM_Odm->SupportInterface) {
		case ODM_ITRF_USB:
			if (pDM_Odm->RFEType == 3 && pDM_Odm->bIsMPChip)
				ODM_ReadAndConfig_MP_8812A_TxPowerTrack_USB_RFE3(pDM_Odm);
			else
				ODM_ReadAndConfig_MP_8812A_TxPowerTrack_USB(pDM_Odm);
			break;

		default:
			;
		}

	}
#endif

	return HAL_STATUS_SUCCESS;
}

HAL_STATUS ODM_ConfigBBWithHeaderFile(struct rtl_dm *pDM_Odm, ODM_BB_Config_Type ConfigType)
{
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_LOUD,
		("===>ODM_ConfigBBWithHeaderFile (%s)\n", (pDM_Odm->bIsMPChip) ? "MPChip" : "TestChip"));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_LOUD,
		("pDM_Odm->SupportPlatform: 0x%X, pDM_Odm->SupportInterface: 0x%X, pDM_Odm->BoardType: 0x%X\n",
		pDM_Odm->SupportPlatform, pDM_Odm->SupportInterface, pDM_Odm->BoardType));

#if (RTL8812A_SUPPORT == 1)
	if (pDM_Odm->SupportICType == ODM_RTL8812) {
		switch (ConfigType) {
		case CONFIG_BB_PHY_REG:
			ODM_ReadAndConfig_MP_8812A_PHY_REG(pDM_Odm);
			break;

		case CONFIG_BB_AGC_TAB:
			ODM_ReadAndConfig_MP_8812A_AGC_TAB(pDM_Odm);
			break;

		case CONFIG_BB_PHY_REG_PG:
			if (pDM_Odm->RFEType == 3 && pDM_Odm->bIsMPChip)
				ODM_ReadAndConfig_MP_8812A_PHY_REG_PG_ASUS(pDM_Odm);
			else
				ODM_ReadAndConfig_MP_8812A_PHY_REG_PG(pDM_Odm);
			break;

		default:
			;
		}
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_LOUD, (" ===> phy_ConfigBBWithHeaderFile() phy:Rtl8812AGCTABArray\n"));
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_LOUD, (" ===> phy_ConfigBBWithHeaderFile() agc:Rtl8812PHY_REGArray\n"));
	}
#endif

#if (RTL8821A_SUPPORT == 1)
	if (pDM_Odm->SupportICType == ODM_RTL8821) {
		switch (ConfigType) {
		case CONFIG_BB_PHY_REG:
			ODM_ReadAndConfig_MP_8821A_PHY_REG(pDM_Odm);
			break;

		case CONFIG_BB_AGC_TAB:
			ODM_ReadAndConfig_MP_8821A_AGC_TAB(pDM_Odm);
			break;

		case CONFIG_BB_PHY_REG_PG:
			ODM_ReadAndConfig_MP_8821A_PHY_REG_PG(pDM_Odm);
			break;

		default:
			;
		}
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_LOUD, (" ===> phy_ConfigBBWithHeaderFile() phy:Rtl8821AGCTABArray\n"));
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_LOUD, (" ===> phy_ConfigBBWithHeaderFile() agc:Rtl8821PHY_REGArray\n"));
	}
#endif
	return HAL_STATUS_SUCCESS;
}

