#ifndef __RTL8821AU_REG_H__
#define __RTL8821AU_REG_H__

//
// 3. Page8(0x800)
//
#define RFPGA0_RFMOD			0x800

#define RFPGA0_TXINFO			0x804
#define RFPGA0_PSDFUNCTION		0x808

#define RFPGA0_TXGAINSTAGE		0x80c

#define RFPGA0_XA_HSSIPARAMETER1	0x820
#define RFPGA0_XA_HSSIPARAMETER2	0x824
#define RFPGA0_XB_HSSIPARAMETER1	0x828
#define RFPGA0_XB_HSSIPARAMETER2	0x82c

#define RFPGA0_XAB_SWITCHCONTROL	0x858
#define RFPGA0_XCD_SWITCHCONTROL	0x85c

#define rFPGA0_XAB_RFParameter		0x878	// RF Parameter
#define rFPGA0_XCD_RFParameter		0x87c

#define rFPGA0_AnalogParameter1	0x880	// Crystal cap setting RF-R/W protection for parameter4??
#define rFPGA0_AnalogParameter2	0x884
#define rFPGA0_AnalogParameter3	0x888
#define rFPGA0_AdDaClockEn			0x888	// enable ad/da clock1 for dual-phy
#define rFPGA0_AnalogParameter4	0x88c
#define rFPGA0_XB_LSSIReadBack		0x8a4
//
// 4. Page9(0x900)
//
#define rFPGA1_RFMOD				0x900	//RF mode & OFDM TxSC // RF BW Setting??

#define rFPGA1_TxBlock				0x904	// Useless now
#define rFPGA1_DebugSelect			0x908	// Useless now
#define rFPGA1_TxInfo				0x90c	// Useless now // Status report??

//
// PageA(0xA00)
//
#define rCCK0_System				0xa00
#define rCCK0_AFESetting				0xa04	// Disable init gain now // Select RX path by RSSI
#define rCCK0_TxFilter1				0xa20
#define rCCK0_TxFilter2				0xa24
#define rCCK0_DebugPort				0xa28	//debug port and Tx filter3

//
// PageB(0xB00)
//
#define rPdp_AntA      				0xb00  
#define rPdp_AntA_4    				0xb04
#define rConfig_Pmpd_AntA 			0xb28
#define rConfig_AntA 					0xb68
#define rConfig_AntB 					0xb6c
#define rPdp_AntB 					0xb70
#define rPdp_AntB_4 					0xb74
#define rConfig_Pmpd_AntB			0xb98
#define rAPK							0xbd8


//
// 6. PageC(0xC00)
//
#define rOFDM0_LSTF					0xc00

#define rOFDM0_TRxPathEnable		0xc04
#define rOFDM0_TRMuxPar			0xc08
#define rOFDM0_TRSWIsolation		0xc0c

#define rOFDM0_XARxAFE				0xc10  //RxIQ DC offset, Rx digital filter, DC notch filter
#define rOFDM0_XARxIQImbalance    	0xc14  //RxIQ imblance matrix
#define rOFDM0_XBRxAFE            		0xc18
#define rOFDM0_XBRxIQImbalance    	0xc1c
#define rOFDM0_XCRxAFE            		0xc20
#define rOFDM0_XCRxIQImbalance    	0xc24
#define rOFDM0_XDRxAFE            		0xc28
#define rOFDM0_XDRxIQImbalance    	0xc2c

#define rOFDM0_RxDetector1			0xc30  //PD,BW & SBD	// DM tune init gain
#define rOFDM0_RxDetector2			0xc34  //SBD & Fame Sync. 
#define rOFDM0_RxDetector3			0xc38  //Frame Sync.
#define rOFDM0_RxDetector4			0xc3c  //PD, SBD, Frame Sync & Short-GI

#define rOFDM0_RxDSP				0xc40  //Rx Sync Path
#define rOFDM0_CFOandDAGC			0xc44  //CFO & DAGC
#define rOFDM0_CCADropThreshold	0xc48 //CCA Drop threshold
#define rOFDM0_ECCAThreshold		0xc4c // energy CCA

#define rOFDM0_XAAGCCore1			0xc50	// DIG
#define rOFDM0_XAAGCCore2			0xc54
#define rOFDM0_XBAGCCore1			0xc58
#define rOFDM0_XBAGCCore2			0xc5c
#define rOFDM0_XCAGCCore1			0xc60
#define rOFDM0_XCAGCCore2			0xc64
#define rOFDM0_XDAGCCore1			0xc68
#define rOFDM0_XDAGCCore2			0xc6c

#define rOFDM0_AGCParameter1		0xc70
#define rOFDM0_AGCParameter2		0xc74
#define rOFDM0_AGCRSSITable		0xc78
#define rOFDM0_HTSTFAGC			0xc7c

#define rOFDM0_XATxIQImbalance		0xc80	// TX PWR TRACK and DIG
#define rOFDM0_XATxAFE				0xc84
#define rOFDM0_XBTxIQImbalance		0xc88
#define rOFDM0_XBTxAFE				0xc8c
#define rOFDM0_XCTxIQImbalance		0xc90
#define rOFDM0_XCTxAFE            		0xc94
#define rOFDM0_XDTxIQImbalance		0xc98
#define rOFDM0_XDTxAFE				0xc9c

#define rOFDM0_RxIQExtAnta			0xca0
#define rOFDM0_TxCoeff1				0xca4
#define rOFDM0_TxCoeff2				0xca8
#define rOFDM0_TxCoeff3				0xcac
#define rOFDM0_TxCoeff4				0xcb0
#define rOFDM0_TxCoeff5				0xcb4
#define rOFDM0_TxCoeff6				0xcb8
#define rOFDM0_RxHPParameter		0xce0
#define rOFDM0_TxPseudoNoiseWgt	0xce4
#define rOFDM0_FrameSync			0xcf0
#define rOFDM0_DFSReport			0xcf4

//
// 7. PageD(0xD00)
//
#define rOFDM1_LSTF					0xd00
#define rOFDM1_TRxPathEnable		0xd04

//
// 8. PageE(0xE00)
//
#define rTxAGC_A_Rate18_06			0xe00
#define rTxAGC_A_Rate54_24			0xe04
#define rTxAGC_A_CCK1_Mcs32		0xe08
#define rTxAGC_A_Mcs03_Mcs00		0xe10
#define rTxAGC_A_Mcs07_Mcs04		0xe14
#define rTxAGC_A_Mcs11_Mcs08		0xe18
#define rTxAGC_A_Mcs15_Mcs12		0xe1c

#define rTxAGC_B_Rate18_06			0x830
#define rTxAGC_B_Rate54_24			0x834
#define rTxAGC_B_CCK1_55_Mcs32	0x838
#define rTxAGC_B_Mcs03_Mcs00		0x83c
#define rTxAGC_B_Mcs07_Mcs04		0x848
#define rTxAGC_B_Mcs11_Mcs08		0x84c
#define rTxAGC_B_Mcs15_Mcs12		0x868
#define rTxAGC_B_CCK11_A_CCK2_11	0x86c

#define rFPGA0_IQK					0xe28
#define rTx_IQK_Tone_A				0xe30
#define rRx_IQK_Tone_A				0xe34
#define rTx_IQK_PI_A				0xe38
#define rRx_IQK_PI_A				0xe3c

#define rTx_IQK 						0xe40
#define rRx_IQK						0xe44
#define rIQK_AGC_Pts					0xe48
#define rIQK_AGC_Rsp				0xe4c
#define rTx_IQK_Tone_B				0xe50
#define rRx_IQK_Tone_B				0xe54
#define rTx_IQK_PI_B					0xe58
#define rRx_IQK_PI_B					0xe5c
#define rIQK_AGC_Cont				0xe60

#define rBlue_Tooth					0xe6c
#define rRx_Wait_CCA				0xe70
#define rTx_CCK_RFON				0xe74
#define rTx_CCK_BBON				0xe78
#define rTx_OFDM_RFON				0xe7c
#define rTx_OFDM_BBON				0xe80
#define rTx_To_Rx					0xe84
#define rTx_To_Tx					0xe88
#define rRx_CCK						0xe8c

#define rTx_Power_Before_IQK_A		0xe94
#define rTx_Power_After_IQK_A		0xe9c

#define rRx_Power_Before_IQK_A		0xea0
#define rRx_Power_Before_IQK_A_2	0xea4
#define rRx_Power_After_IQK_A		0xea8
#define rRx_Power_After_IQK_A_2		0xeac

#define rTx_Power_Before_IQK_B		0xeb4
#define rTx_Power_After_IQK_B		0xebc

#define rRx_Power_Before_IQK_B		0xec0
#define rRx_Power_Before_IQK_B_2	0xec4
#define rRx_Power_After_IQK_B		0xec8
#define rRx_Power_After_IQK_B_2		0xecc

#define rRx_OFDM					0xed0
#define rRx_Wait_RIFS 				0xed4
#define rRx_TO_Rx 					0xed8
#define rStandby 						0xedc
#define rSleep 						0xee0
#define rPMPD_ANAEN				0xeec



// TX AGC 
#define RTXAGC_A_CCK11_CCK1				0xc20
#define RTXAGC_A_OFDM18_OFDM6				0xc24
#define RTXAGC_A_OFDM54_OFDM24				0xc28
#define RTXAGC_A_MCS03_MCS00				0xc2c
#define RTXAGC_A_MCS07_MCS04				0xc30
#define RTXAGC_A_MCS11_MCS08				0xc34
#define RTXAGC_A_MCS15_MCS12				0xc38
#define RTXAGC_A_NSS1INDEX3_NSS1INDEX0			0xc3c
#define RTXAGC_A_NSS1INDEX7_NSS1INDEX4			0xc40
#define RTXAGC_A_NSS2INDEX1_NSS1INDEX8			0xc44
#define RTXAGC_A_NSS2INDEX5_NSS2INDEX2			0xc48
#define RTXAGC_A_NSS2INDEX9_NSS2INDEX6			0xc4c


#define RTXAGC_B_CCK11_CCK1				0xe20
#define RTXAGC_B_OFDM18_OFDM6				0xe24
#define RTXAGC_B_OFDM54_OFDM24				0xe28
#define RTXAGC_B_MCS03_MCS00				0xe2c
#define RTXAGC_B_MCS07_MCS04				0xe30
#define RTXAGC_B_MCS11_MCS08				0xe34
#define RTXAGC_B_MCS15_MCS12				0xe38
#define RTXAGC_B_NSS1INDEX3_NSS1INDEX0			0xe3c
#define RTXAGC_B_NSS1INDEX7_NSS1INDEX4			0xe40
#define RTXAGC_B_NSS2INDEX1_NSS1INDEX8			0xe44
#define RTXAGC_B_NSS2INDEX5_NSS2INDEX2			0xe48
#define RTXAGC_B_NSS2INDEX9_NSS2INDEX6			0xe4c

#define MASKBYTE0                		0xff	// Reg 0xc50 rOFDM0_XAAGCCore~0xC6f
#define MASKBYTE1                		0xff00
#define MASKBYTE2                		0xff0000
#define MASKBYTE3                		0xff000000

#define bTxAGC_byte0_Jaguar							0xff
#define bTxAGC_byte1_Jaguar							0xff00
#define bTxAGC_byte2_Jaguar							0xff0000
#define bTxAGC_byte3_Jaguar							0xff000000

#endif
