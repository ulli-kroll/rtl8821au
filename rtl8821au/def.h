#ifndef __RTL8821AU_DEF_H__
#define __RTL8821AU_DEF_H__

#include <linux/types.h>

/* BIT 7 HT Rate*/
// TxHT = 0
#define	MGN_1M				0x02
#define	MGN_2M				0x04
#define	MGN_5_5M			0x0b
#define	MGN_11M			0x16

#define	MGN_6M				0x0c
#define	MGN_9M				0x12
#define	MGN_12M			0x18
#define	MGN_18M			0x24
#define	MGN_24M			0x30
#define	MGN_36M			0x48
#define	MGN_48M			0x60
#define	MGN_54M			0x6c

// TxHT = 1
#define	MGN_MCS0			0x80
#define	MGN_MCS1			0x81
#define	MGN_MCS2			0x82
#define	MGN_MCS3			0x83
#define	MGN_MCS4			0x84
#define	MGN_MCS5			0x85
#define	MGN_MCS6			0x86
#define	MGN_MCS7			0x87
#define	MGN_MCS8			0x88
#define	MGN_MCS9			0x89
#define	MGN_MCS10			0x8a
#define	MGN_MCS11			0x8b
#define	MGN_MCS12			0x8c
#define	MGN_MCS13			0x8d
#define	MGN_MCS14			0x8e
#define	MGN_MCS15			0x8f
#define	MGN_VHT1SS_MCS0		0x90
#define	MGN_VHT1SS_MCS1		0x91
#define	MGN_VHT1SS_MCS2		0x92
#define	MGN_VHT1SS_MCS3		0x93
#define	MGN_VHT1SS_MCS4		0x94
#define	MGN_VHT1SS_MCS5		0x95
#define	MGN_VHT1SS_MCS6		0x96
#define	MGN_VHT1SS_MCS7		0x97
#define	MGN_VHT1SS_MCS8		0x98
#define	MGN_VHT1SS_MCS9		0x99
#define	MGN_VHT2SS_MCS0		0x9a
#define	MGN_VHT2SS_MCS1		0x9b
#define	MGN_VHT2SS_MCS2		0x9c
#define	MGN_VHT2SS_MCS3		0x9d
#define	MGN_VHT2SS_MCS4		0x9e
#define	MGN_VHT2SS_MCS5		0x9f
#define	MGN_VHT2SS_MCS6		0xa0
#define	MGN_VHT2SS_MCS7		0xa1
#define	MGN_VHT2SS_MCS8		0xa2
#define	MGN_VHT2SS_MCS9		0xa3

#define	MGN_MCS0_SG			0xc0
#define	MGN_MCS1_SG			0xc1
#define	MGN_MCS2_SG			0xc2
#define	MGN_MCS3_SG			0xc3
#define	MGN_MCS4_SG			0xc4
#define	MGN_MCS5_SG			0xc5
#define	MGN_MCS6_SG			0xc6
#define	MGN_MCS7_SG			0xc7
#define	MGN_MCS8_SG			0xc8
#define	MGN_MCS9_SG			0xc9
#define	MGN_MCS10_SG		0xca
#define	MGN_MCS11_SG		0xcb
#define	MGN_MCS12_SG		0xcc
#define	MGN_MCS13_SG		0xcd
#define	MGN_MCS14_SG		0xce
#define	MGN_MCS15_SG		0xcf
enum rtl_desc_qsel {
	QSLT_BK = 0x2,
	QSLT_BE = 0x0,
	QSLT_VI = 0x5,
	QSLT_VO = 0x7,
	QSLT_BEACON = 0x10,
	QSLT_HIGH = 0x11,
	QSLT_MGNT = 0x12,
	QSLT_CMD = 0x13,
};







//HAL_CHIP_TYPE_E
typedef enum tag_HAL_CHIP_Type_Definition
{
	TEST_CHIP 		=	0,
	NORMAL_CHIP 	=	1,
}HAL_CHIP_TYPE_E;

//HAL_CUT_VERSION_E
typedef enum tag_HAL_Cut_Version_Definition
{
	A_CUT_VERSION 		=	0,
	B_CUT_VERSION 		=	1,
	C_CUT_VERSION 		=	2,
	D_CUT_VERSION 		=	3,
	E_CUT_VERSION 		=	4,
	F_CUT_VERSION 		=	5,
	G_CUT_VERSION 		=	6,
}HAL_CUT_VERSION_E;

// HAL_Manufacturer
typedef enum tag_HAL_Manufacturer_Version_Definition
{
	CHIP_VENDOR_TSMC 	=	0,
	CHIP_VENDOR_UMC 	=	1,
	CHIP_VENDOR_SMIC 	=	2,
}HAL_VENDOR_E;

typedef enum tag_HAL_RF_Type_Definition
{
	RF_TYPE_1T1R 	=	0,
	RF_TYPE_1T2R 	=	1,
	RF_TYPE_2T2R	=	2,
	RF_TYPE_2T3R	=	3,
	RF_TYPE_2T4R	=	4,
	RF_TYPE_3T3R	=	5,
	RF_TYPE_3T4R	=	6,
	RF_TYPE_4T4R	=	7,
}HAL_RF_TYPE_E;

struct HAL_VERSION
{
	int			ICType;
	HAL_CHIP_TYPE_E		ChipType;
	HAL_CUT_VERSION_E	CUTVersion;
	HAL_VENDOR_E		VendorType;
	HAL_RF_TYPE_E		RFType;
	u8					ROMVer;
};

//VERSION_8192C			VersionID;
//HAL_VERSION			VersionID;

#define CHIP_8812		BIT(2)
#define CHIP_8821		(BIT(0)|BIT(2))

/* MASK */
#define IC_TYPE_MASK			(BIT(0)|BIT(1)|BIT(2))

// Get element
#define GET_CVID_IC_TYPE(version)			((version).ICType & IC_TYPE_MASK)
#define GET_CVID_CHIP_TYPE(version)			((HAL_CHIP_TYPE_E)((version).ChipType)	)
#define GET_CVID_RF_TYPE(version)			((HAL_RF_TYPE_E)((version).RFType))
#define GET_CVID_MANUFACTUER(version)		((HAL_VENDOR_E)((version).VendorType))
#define GET_CVID_CUT_VERSION(version)		((HAL_CUT_VERSION_E)((version).CUTVersion))
#define GET_CVID_ROM_VERSION(version)		(((version).ROMVer) & ROM_VERSION_MASK)

//----------------------------------------------------------------------------
//Common Macro. --
//----------------------------------------------------------------------------
//HAL_VERSION VersionID

// HAL_IC_TYPE_E
#define IS_8812_SERIES(version)			((GET_CVID_IC_TYPE(version) == CHIP_8812)? true : false)
#define IS_8821_SERIES(version)			((GET_CVID_IC_TYPE(version) == CHIP_8821)? true : false)


//HAL_CHIP_TYPE_E
#define IS_TEST_CHIP(version)			((GET_CVID_CHIP_TYPE(version)==TEST_CHIP)? TRUE: FALSE)
#define IS_NORMAL_CHIP(version)			((GET_CVID_CHIP_TYPE(version)==NORMAL_CHIP)? TRUE: FALSE)

//HAL_CUT_VERSION_E
#define IS_A_CUT(version)				((GET_CVID_CUT_VERSION(version) == A_CUT_VERSION) ? TRUE : FALSE)
#define IS_B_CUT(version)				((GET_CVID_CUT_VERSION(version) == B_CUT_VERSION) ? TRUE : FALSE)
#define IS_C_CUT(version)				((GET_CVID_CUT_VERSION(version) == C_CUT_VERSION) ? TRUE : FALSE)
#define IS_D_CUT(version)				((GET_CVID_CUT_VERSION(version) == D_CUT_VERSION) ? TRUE : FALSE)
#define IS_E_CUT(version)					((GET_CVID_CUT_VERSION(version) == E_CUT_VERSION) ? TRUE : FALSE)


//HAL_VENDOR_E
#define IS_CHIP_VENDOR_TSMC(version)	((GET_CVID_MANUFACTUER(version) == CHIP_VENDOR_TSMC)? TRUE: FALSE)
#define IS_CHIP_VENDOR_UMC(version)	((GET_CVID_MANUFACTUER(version) == CHIP_VENDOR_UMC)? TRUE: FALSE)
#define IS_CHIP_VENDOR_SMIC(version)	((GET_CVID_MANUFACTUER(version) == CHIP_VENDOR_SMIC)? TRUE: FALSE)

//HAL_RF_TYPE_E
#define IS_1T1R(version)					((GET_CVID_RF_TYPE(version) == RF_TYPE_1T1R)? TRUE : FALSE )
#define IS_1T2R(version)					((GET_CVID_RF_TYPE(version) == RF_TYPE_1T2R)? TRUE : FALSE)
#define IS_2T2R(version)					((GET_CVID_RF_TYPE(version) == RF_TYPE_2T2R)? TRUE : FALSE)


//----------------------------------------------------------------------------
//Chip version Macro. --
//----------------------------------------------------------------------------
#define IS_81XXC_TEST_CHIP(version)		((IS_81XXC(version) && (!IS_NORMAL_CHIP(version)))? TRUE: FALSE)

#define IS_92C_SERIAL(version)   					((IS_81XXC(version) && IS_2T2R(version)) ? TRUE : FALSE)
#define IS_81xxC_VENDOR_UMC_A_CUT(version)	(IS_81XXC(version)?(IS_CHIP_VENDOR_UMC(version) ? (IS_A_CUT(version) ? TRUE : FALSE) : FALSE): FALSE)
#define IS_81xxC_VENDOR_UMC_B_CUT(version)	(IS_81XXC(version)?(IS_CHIP_VENDOR_UMC(version) ? (IS_B_CUT(version) ? TRUE : FALSE) : FALSE): FALSE)
#define IS_81xxC_VENDOR_UMC_C_CUT(version)	(IS_81XXC(version)?(IS_CHIP_VENDOR_UMC(version) ? (IS_C_CUT(version) ? TRUE : FALSE) : FALSE): FALSE)

#define IS_VENDOR_8812A_TEST_CHIP(_Adapter)		((IS_8812_SERIES(GET_HAL_DATA(_Adapter)->VersionID)) ? ((IS_NORMAL_CHIP(GET_HAL_DATA(_Adapter)->VersionID)) ? FALSE : TRUE) : FALSE)
#define IS_VENDOR_8812A_MP_CHIP(_Adapter)		((IS_8812_SERIES(GET_HAL_DATA(_Adapter)->VersionID)) ? ((IS_NORMAL_CHIP(GET_HAL_DATA(_Adapter)->VersionID)) ? TRUE : FALSE) : FALSE)
#define IS_VENDOR_8812A_C_CUT(_Adapter)			((IS_8812_SERIES(GET_HAL_DATA(_Adapter)->VersionID)) ? ((GET_CVID_CUT_VERSION(GET_HAL_DATA(_Adapter)->VersionID) == C_CUT_VERSION) ? TRUE : FALSE) : FALSE)

#define IS_VENDOR_8821A_TEST_CHIP(_Adapter)	((IS_8821_SERIES(GET_HAL_DATA(_Adapter)->VersionID)) ? ((IS_NORMAL_CHIP(GET_HAL_DATA(_Adapter)->VersionID)) ? FALSE : TRUE) : FALSE)
#define IS_VENDOR_8821A_MP_CHIP(_Adapter)		((IS_8821_SERIES(GET_HAL_DATA(_Adapter)->VersionID)) ? ((IS_NORMAL_CHIP(GET_HAL_DATA(_Adapter)->VersionID)) ? TRUE : FALSE) : FALSE)




#endif
