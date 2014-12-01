#ifndef __RTL8821AU_TRX_H__
#define __RTL8821AU_TRX_H__

void rtl8821au_fill_fake_txdesc(struct rtl_priv *padapter, uint8_t *pDesc,
	uint32_t BufferLen, uint8_t IsPsPoll, uint8_t IsBTQosNull);


/* TX desc macros */

// Dword 0
#define GET_TX_DESC_OWN_8812(__pdesc)				\
	LE_BITS_TO_4BYTE(__pdesc, 31, 1)
#define SET_TX_DESC_PKT_SIZE_8812(__pdesc, __Value) 		\
	SET_BITS_TO_LE_4BYTE(__pdesc, 0, 16, __Value)
#define SET_TX_DESC_OFFSET_8812(__pdesc, __Value) 		\
	SET_BITS_TO_LE_4BYTE(__pdesc, 16, 8, __Value)
#define SET_TX_DESC_BMC_8812(__pdesc, __Value) 		\
	SET_BITS_TO_LE_4BYTE(__pdesc, 24, 1, __Value)
#define SET_TX_DESC_HTC_8812(__pdesc, __Value) 		\
	SET_BITS_TO_LE_4BYTE(__pdesc, 25, 1, __Value)
#define SET_TX_DESC_LAST_SEG_8812(__pdesc, __Value) 		\
	SET_BITS_TO_LE_4BYTE(__pdesc, 26, 1, __Value)
#define SET_TX_DESC_FIRST_SEG_8812(__pdesc, __Value) 		\
	SET_BITS_TO_LE_4BYTE(__pdesc, 27, 1, __Value)
#define SET_TX_DESC_LINIP_8812(__pdesc, __Value) 		\
	SET_BITS_TO_LE_4BYTE(__pdesc, 28, 1, __Value)
#define SET_TX_DESC_NO_ACM_8812(__pdesc, __Value) 		\
	SET_BITS_TO_LE_4BYTE(__pdesc, 29, 1, __Value)
#define SET_TX_DESC_GF_8812(__pdesc, __Value) 		\
	SET_BITS_TO_LE_4BYTE(__pdesc, 30, 1, __Value)
#define SET_TX_DESC_OWN_8812(__pdesc, __Value) 		\
	SET_BITS_TO_LE_4BYTE(__pdesc, 31, 1, __Value)

// Dword 1
#define SET_TX_DESC_MACID_8812(__pdesc, __Value) 		\
	SET_BITS_TO_LE_4BYTE(__pdesc+4, 0, 7, __Value)
#define SET_TX_DESC_QUEUE_SEL_8812(__pdesc, __Value) 		\
	SET_BITS_TO_LE_4BYTE(__pdesc+4, 8, 5, __Value)
#define SET_TX_DESC_RDG_NAV_EXT_8812(__pdesc, __Value) 	\
	SET_BITS_TO_LE_4BYTE(__pdesc+4, 13, 1, __Value)
#define SET_TX_DESC_LSIG_TXOP_EN_8812(__pdesc, __Value) 	\
	SET_BITS_TO_LE_4BYTE(__pdesc+4, 14, 1, __Value)
#define SET_TX_DESC_PIFS_8812(__pdesc, __Value) 		\
	SET_BITS_TO_LE_4BYTE(__pdesc+4, 15, 1, __Value)
#define SET_TX_DESC_RATE_ID_8812(__pdesc, __Value) 		\
	SET_BITS_TO_LE_4BYTE(__pdesc+4, 16, 5, __Value)
#define SET_TX_DESC_EN_DESC_ID_8812(__pdesc, __Value) 	\
	SET_BITS_TO_LE_4BYTE(__pdesc+4, 21, 1, __Value)
#define SET_TX_DESC_SEC_TYPE_8812(__pdesc, __Value) 		\
	SET_BITS_TO_LE_4BYTE(__pdesc+4, 22, 2, __Value)
#define SET_TX_DESC_PKT_OFFSET_8812(__pdesc, __Value) 	\
	SET_BITS_TO_LE_4BYTE(__pdesc+4, 24, 5, __Value)

// Dword 2
#define SET_TX_DESC_PAID_8812(__pdesc, __Value) 		\
	SET_BITS_TO_LE_4BYTE(__pdesc+8, 0,  9, __Value)
#define SET_TX_DESC_CCA_RTS_8812(__pdesc, __Value) 		\
	SET_BITS_TO_LE_4BYTE(__pdesc+8, 10, 2, __Value)
#define SET_TX_DESC_AGG_ENABLE_8812(__pdesc, __Value)		\
	SET_BITS_TO_LE_4BYTE(__pdesc+8, 12, 1, __Value)
#define SET_TX_DESC_RDG_ENABLE_8812(__pdesc, __Value) 	\
	SET_BITS_TO_LE_4BYTE(__pdesc+8, 13, 1, __Value)
#define SET_TX_DESC_AGG_BREAK_8812(__pdesc, __Value)		\
	SET_BITS_TO_LE_4BYTE(__pdesc+8, 16, 1, __Value)
#define SET_TX_DESC_MORE_FRAG_8812(__pdesc, __Value) 		\
	SET_BITS_TO_LE_4BYTE(__pdesc+8, 17, 1, __Value)
#define SET_TX_DESC_RAW_8812(__pdesc, __Value) 		\
	SET_BITS_TO_LE_4BYTE(__pdesc+8, 18, 1, __Value)
#define SET_TX_DESC_SPE_RPT_8812(__pdesc, __Value) 		\
	SET_BITS_TO_LE_4BYTE(__pdesc+8, 19, 1, __Value)
#define SET_TX_DESC_AMPDU_DENSITY_8812(__pdesc, __Value) 	\
	SET_BITS_TO_LE_4BYTE(__pdesc+8, 20, 3, __Value)
#define SET_TX_DESC_BT_INT_8812(__pdesc, __Value) 		\
	SET_BITS_TO_LE_4BYTE(__pdesc+8, 23, 1, __Value)
#define SET_TX_DESC_GID_8812(__pdesc, __Value) 		\
	SET_BITS_TO_LE_4BYTE(__pdesc+8, 24, 6, __Value)

// Dword 3
#define SET_TX_DESC_WHEADER_LEN_8812(__pdesc, __Value) 	\
	SET_BITS_TO_LE_4BYTE(__pdesc+12, 0, 4, __Value)
#define SET_TX_DESC_CHK_EN_8812(__pdesc, __Value) 		\
	SET_BITS_TO_LE_4BYTE(__pdesc+12, 4, 1, __Value)
#define SET_TX_DESC_EARLY_MODE_8812(__pdesc, __Value) 	\
	SET_BITS_TO_LE_4BYTE(__pdesc+12, 5, 1, __Value)
#define SET_TX_DESC_HWSEQ_SEL_8812(__pdesc, __Value) 		\
	SET_BITS_TO_LE_4BYTE(__pdesc+12, 6, 2, __Value)
#define SET_TX_DESC_USE_RATE_8812(__pdesc, __Value) 		\
	SET_BITS_TO_LE_4BYTE(__pdesc+12, 8, 1, __Value)
#define SET_TX_DESC_DISABLE_RTS_FB_8812(__pdesc, __Value)	\
	SET_BITS_TO_LE_4BYTE(__pdesc+12, 9, 1, __Value)
#define SET_TX_DESC_DISABLE_FB_8812(__pdesc, __Value)		\
	SET_BITS_TO_LE_4BYTE(__pdesc+12, 10, 1, __Value)
#define SET_TX_DESC_CTS2SELF_8812(__pdesc, __Value) 		\
	SET_BITS_TO_LE_4BYTE(__pdesc+12, 11, 1, __Value)
#define SET_TX_DESC_RTS_ENABLE_8812(__pdesc, __Value) 	\
	SET_BITS_TO_LE_4BYTE(__pdesc+12, 12, 1, __Value)
#define SET_TX_DESC_HW_RTS_ENABLE_8812(__pdesc, __Value) 	\
	SET_BITS_TO_LE_4BYTE(__pdesc+12, 13, 1, __Value)
#define SET_TX_DESC_NAV_USE_HDR_8812(__pdesc, __Value) 	\
	SET_BITS_TO_LE_4BYTE(__pdesc+12, 15, 1, __Value)
#define SET_TX_DESC_USE_MAX_LEN_8812(__pdesc, __Value) 	\
	SET_BITS_TO_LE_4BYTE(__pdesc+12, 16, 1, __Value)
#define SET_TX_DESC_MAX_AGG_NUM_8812(__pdesc, __Value) 	\
	SET_BITS_TO_LE_4BYTE(__pdesc+12, 17, 5, __Value)
#define SET_TX_DESC_NDPA_8812(__pdesc, __Value) 		\
	SET_BITS_TO_LE_4BYTE(__pdesc+12, 22, 2, __Value)
#define SET_TX_DESC_AMPDU_MAX_TIME_8812(__pdesc, __Value)	\
	SET_BITS_TO_LE_4BYTE(__pdesc+12, 24, 8, __Value)

// Dword 4
#define SET_TX_DESC_TX_RATE_8812(__pdesc, __Value) 		\
	SET_BITS_TO_LE_4BYTE(__pdesc+16, 0, 7, __Value)
#define SET_TX_DESC_DATA_RATE_FB_LIMIT_8812(__pdesc, __Value) \
	SET_BITS_TO_LE_4BYTE(__pdesc+16, 8, 5, __Value)
#define SET_TX_DESC_RTS_RATE_FB_LIMIT_8812(__pdesc, __Value) 	\
	SET_BITS_TO_LE_4BYTE(__pdesc+16, 13, 4, __Value)
#define SET_TX_DESC_RETRY_LIMIT_ENABLE_8812(__pdesc, __Value) 	\
	SET_BITS_TO_LE_4BYTE(__pdesc+16, 17, 1, __Value)
#define SET_TX_DESC_DATA_RETRY_LIMIT_8812(__pdesc, __Value) 	\
	SET_BITS_TO_LE_4BYTE(__pdesc+16, 18, 6, __Value)
#define SET_TX_DESC_RTS_RATE_8812(__pdesc, __Value) 		\
	SET_BITS_TO_LE_4BYTE(__pdesc+16, 24, 5, __Value)

// Dword 5
#define SET_TX_DESC_DATA_SC_8812(__pdesc, __Value) 		\
	SET_BITS_TO_LE_4BYTE(__pdesc+20, 0, 4, __Value)
#define SET_TX_DESC_DATA_SHORT_8812(__pdesc, __Value) 	\
	SET_BITS_TO_LE_4BYTE(__pdesc+20, 4, 1, __Value)
#define SET_TX_DESC_DATA_BW_8812(__pdesc, __Value) 		\
	SET_BITS_TO_LE_4BYTE(__pdesc+20, 5, 2, __Value)
#define SET_TX_DESC_DATA_LDPC_8812(__pdesc, __Value) 		\
	SET_BITS_TO_LE_4BYTE(__pdesc+20, 7, 1, __Value)
#define SET_TX_DESC_DATA_STBC_8812(__pdesc, __Value) 		\
	SET_BITS_TO_LE_4BYTE(__pdesc+20, 8, 2, __Value)
#define SET_TX_DESC_CTROL_STBC_8812(__pdesc, __Value) 	\
	SET_BITS_TO_LE_4BYTE(__pdesc+20, 10, 2, __Value)
#define SET_TX_DESC_RTS_SHORT_8812(__pdesc, __Value) 		\
	SET_BITS_TO_LE_4BYTE(__pdesc+20, 12, 1, __Value)
#define SET_TX_DESC_RTS_SC_8812(__pdesc, __Value) 		\
	SET_BITS_TO_LE_4BYTE(__pdesc+20, 13, 4, __Value)

// Dword 6
#define SET_TX_DESC_MBSSID_8821(__pdesc, __Value) 		\
	SET_BITS_TO_LE_4BYTE(__pdesc+24, 12, 4, __Value)

// Dword 7
#define SET_TX_DESC_TX_BUFFER_SIZE_8812(__pdesc, __Value) 	\
	SET_BITS_TO_LE_4BYTE(__pdesc+28, 0, 16, __Value)
#define SET_TX_DESC_TX_DESC_CHECKSUM_8812(__pdesc, __Value) 	\
	SET_BITS_TO_LE_4BYTE(__pdesc+28, 0, 16, __Value)
#define SET_TX_DESC_USB_TXAGG_NUM_8812(__pdesc, __Value) 	\
	SET_BITS_TO_LE_4BYTE(__pdesc+28, 24, 8, __Value)

// Dword 8
#define SET_TX_DESC_HWSEQ_EN_8812(__pdesc, __Value) 		\
	SET_BITS_TO_LE_4BYTE(__pdesc+32, 15, 1, __Value)

// Dword 9
#define SET_TX_DESC_SEQ_8812(__pdesc, __Value) 		\
	SET_BITS_TO_LE_4BYTE(__pdesc+36, 12, 12, __Value)

// Dword 10
#define SET_TX_DESC_TX_BUFFER_ADDRESS_8812(__pdesc, __Value) 	\
	SET_BITS_TO_LE_4BYTE(__pdesc+40, 0, 32, __Value)

// Dword 11
#define SET_TX_DESC_NEXT_DESC_ADDRESS_8812(__pdesc, __Value) 	\
	SET_BITS_TO_LE_4BYTE(__pdesc+48, 0, 32, __Value)

/* RX desc macros */


//DWORD 0
#define SET_RX_STATUS_DESC_PKT_LEN_8812(__pdesc, __Value)	\
	SET_BITS_TO_LE_4BYTE( __pdesc, 0, 14, __Value)
#define SET_RX_STATUS_DESC_EOR_8812(__pdesc, __Value)	\
	SET_BITS_TO_LE_4BYTE( __pdesc, 30, 1, __Value)
#define SET_RX_STATUS_DESC_OWN_8812(__pdesc, __Value)	\
	SET_BITS_TO_LE_4BYTE( __pdesc, 31, 1, __Value)

#define GET_RX_STATUS_DESC_PKT_LEN_8812(__pdesc)	\
	LE_BITS_TO_4BYTE( __pdesc, 0, 14)
#define GET_RX_STATUS_DESC_CRC32_8812(__pdesc)		\
	LE_BITS_TO_4BYTE( __pdesc, 14, 1)
#define GET_RX_STATUS_DESC_ICV_8812(__pdesc)		\
	LE_BITS_TO_4BYTE( __pdesc, 15, 1)
#define GET_RX_STATUS_DESC_DRVINFO_SIZE_8812(__pdesc)	\
	LE_BITS_TO_4BYTE( __pdesc, 16, 4)
#define GET_RX_STATUS_DESC_SECURITY_8812(__pdesc)	\
	LE_BITS_TO_4BYTE( __pdesc, 20, 3)
#define GET_RX_STATUS_DESC_QOS_8812(__pdesc)		\
	LE_BITS_TO_4BYTE( __pdesc, 23, 1)
#define GET_RX_STATUS_DESC_SHIFT_8812(__pdesc)		\
	LE_BITS_TO_4BYTE( __pdesc, 24, 2)
#define GET_RX_STATUS_DESC_PHY_STATUS_8812(__pdesc)	\
	LE_BITS_TO_4BYTE( __pdesc, 26, 1)
#define GET_RX_STATUS_DESC_SWDEC_8812(__pdesc)		\
	LE_BITS_TO_4BYTE( __pdesc, 27, 1)
#define GET_RX_STATUS_DESC_LAST_SEG_8812(__pdesc)	\
	LE_BITS_TO_4BYTE( __pdesc, 28, 1)
#define GET_RX_STATUS_DESC_FIRST_SEG_8812(__pdesc)	\
	LE_BITS_TO_4BYTE( __pdesc, 29, 1)
#define GET_RX_STATUS_DESC_EOR_8812(__pdesc)		\
	LE_BITS_TO_4BYTE( __pdesc, 30, 1)
#define GET_RX_STATUS_DESC_OWN_8812(__pdesc)		\
	LE_BITS_TO_4BYTE( __pdesc, 31, 1)

//DWORD 1
#define GET_RX_STATUS_DESC_MACID_8812(__pdesc) 		\
	LE_BITS_TO_4BYTE(__pdesc+4, 0, 7)
#define GET_RX_STATUS_DESC_TID_8812(__pdesc) 			\
	LE_BITS_TO_4BYTE(__pdesc+4, 8, 4)
#define GET_RX_STATUS_DESC_AMSDU_8812(__pdesc) 		\
	LE_BITS_TO_4BYTE(__pdesc+4, 13, 1)
#define GET_RX_STATUS_DESC_RXID_MATCH_8812(__pdesc)		\
	LE_BITS_TO_4BYTE( __pdesc+4, 14, 1)
#define GET_RX_STATUS_DESC_PAGGR_8812(__pdesc)		\
	LE_BITS_TO_4BYTE( __pdesc+4, 15, 1)
#define GET_RX_STATUS_DESC_A1_FIT_8812(__pdesc)		\
	LE_BITS_TO_4BYTE( __pdesc+4, 16, 4)
#define GET_RX_STATUS_DESC_CHKERR_8812(__pdesc)		\
	LE_BITS_TO_4BYTE( __pdesc+4, 20, 1)
#define GET_RX_STATUS_DESC_IPVER_8812(__pdesc)		\
	LE_BITS_TO_4BYTE(__pdesc+4, 21, 1)
#define GET_RX_STATUS_DESC_IS_TCPUDP__8812(__pdesc)		\
	LE_BITS_TO_4BYTE(__pdesc+4, 22, 1)
#define GET_RX_STATUS_DESC_CHK_VLD_8812(__pdesc)		\
	LE_BITS_TO_4BYTE(__pdesc+4, 23, 1)
#define GET_RX_STATUS_DESC_PAM_8812(__pdesc)			\
	LE_BITS_TO_4BYTE( __pdesc+4, 24, 1)
#define GET_RX_STATUS_DESC_PWR_8812(__pdesc)			\
	LE_BITS_TO_4BYTE( __pdesc+4, 25, 1)
#define GET_RX_STATUS_DESC_MORE_DATA_8812(__pdesc)		\
	LE_BITS_TO_4BYTE( __pdesc+4, 26, 1)
#define GET_RX_STATUS_DESC_MORE_FRAG_8812(__pdesc)		\
	LE_BITS_TO_4BYTE( __pdesc+4, 27, 1)
#define GET_RX_STATUS_DESC_TYPE_8812(__pdesc)			\
	LE_BITS_TO_4BYTE( __pdesc+4, 28, 2)
#define GET_RX_STATUS_DESC_MC_8812(__pdesc)			\
	LE_BITS_TO_4BYTE( __pdesc+4, 30, 1)
#define GET_RX_STATUS_DESC_BC_8812(__pdesc)			\
	LE_BITS_TO_4BYTE( __pdesc+4, 31, 1)

//DWORD 2
#define GET_RX_STATUS_DESC_SEQ_8812(__pdesc)		\
	LE_BITS_TO_4BYTE( __pdesc+8, 0, 12)
#define GET_RX_STATUS_DESC_FRAG_8812(__pdesc)		\
	LE_BITS_TO_4BYTE( __pdesc+8, 12, 4)
#define GET_RX_STATUS_DESC_RX_IS_QOS_8812(__pdesc)	\
	LE_BITS_TO_4BYTE( __pdesc+8, 16, 1)
#define GET_RX_STATUS_DESC_WLANHD_IV_LEN_8812(__pdesc)	\
	LE_BITS_TO_4BYTE( __pdesc+8, 18, 6)
#define GET_RX_STATUS_DESC_RPT_SEL_8812(__pdesc)	\
	LE_BITS_TO_4BYTE( __pdesc+8, 28, 1)

//DWORD 3
#define GET_RX_STATUS_DESC_RX_RATE_8812(__pdesc)	\
	LE_BITS_TO_4BYTE( __pdesc+12, 0, 7)
#define GET_RX_STATUS_DESC_HTC_8812(__pdesc)		\
	LE_BITS_TO_4BYTE( __pdesc+12, 10, 1)
#define GET_RX_STATUS_DESC_EOSP_8812(__pdesc)		\
	LE_BITS_TO_4BYTE( __pdesc+12, 11, 1)
#define GET_RX_STATUS_DESC_BSSID_FIT_8812(__pdesc)	\
	LE_BITS_TO_4BYTE( __pdesc+12, 12, 2)
#ifdef CONFIG_USB_RX_AGGREGATION
#define GET_RX_STATUS_DESC_USB_AGG_PKTNUM_8812(__pdesc)	\
	LE_BITS_TO_4BYTE( __pdesc+12, 16, 8)
#endif
#define GET_RX_STATUS_DESC_PATTERN_MATCH_8812(__pdesc)	\
		LE_BITS_TO_4BYTE( __pdesc+12, 29, 1)
#define GET_RX_STATUS_DESC_UNICAST_MATCH_8812(__pdesc)	\
		LE_BITS_TO_4BYTE( __pdesc+12, 30, 1)
#define GET_RX_STATUS_DESC_MAGIC_MATCH_8812(__pdesc)		\
	LE_BITS_TO_4BYTE( __pdesc+12, 31, 1)

//DWORD 6
#define GET_RX_STATUS_DESC_SPLCP_8812(__pdesc)		\
	LE_BITS_TO_4BYTE( __pdesc+16, 0, 1)
#define GET_RX_STATUS_DESC_LDPC_8812(__pdesc)			\
	LE_BITS_TO_4BYTE( __pdesc+16, 1, 1)
#define GET_RX_STATUS_DESC_STBC_8812(__pdesc)			\
	LE_BITS_TO_4BYTE( __pdesc+16, 2, 1)
#define GET_RX_STATUS_DESC_BW_8812(__pdesc)			\
	LE_BITS_TO_4BYTE( __pdesc+16, 4, 2)

//DWORD 5
#define GET_RX_STATUS_DESC_TSFL_8812(__pdesc)		\
	LE_BITS_TO_4BYTE( __pdesc+20, 0, 32)
#define GET_RX_STATUS_DESC_BUFF_ADDR_8812(__pdesc) 		\
	LE_BITS_TO_4BYTE(__pdesc+24, 0, 32)
#define GET_RX_STATUS_DESC_BUFF_ADDR64_8812(__pdesc) 		\
	LE_BITS_TO_4BYTE(__pdesc+28, 0, 32)
#define SET_RX_STATUS_DESC_BUFF_ADDR_8812(__pdesc, __Value) 	\
	SET_BITS_TO_LE_4BYTE(__pdesc+24, 0, 32, __Value)






#define SET_EARLYMODE_PKTNUM_8812(__pAddr, __Value)		\
	SET_BITS_TO_LE_4BYTE(__pAddr, 0, 4, __Value)
#define SET_EARLYMODE_LEN0_8812(__pAddr, __Value) 		\
	SET_BITS_TO_LE_4BYTE(__pAddr, 4, 15, __Value)
#define SET_EARLYMODE_LEN1_1_8812(__pAddr, __Value) 		\
	SET_BITS_TO_LE_4BYTE(__pAddr, 19, 13, __Value)
#define SET_EARLYMODE_LEN1_2_8812(__pAddr, __Value) 		\
	SET_BITS_TO_LE_4BYTE(__pAddr+4, 0, 2, __Value)
#define SET_EARLYMODE_LEN2_8812(__pAddr, __Value) 		\
	SET_BITS_TO_LE_4BYTE(__pAddr+4, 2, 15,  __Value)
#define SET_EARLYMODE_LEN3_8812(__pAddr, __Value) 		\
	SET_BITS_TO_LE_4BYTE(__pAddr+4, 17, 15, __Value)


struct tx_desc_8821au {
	// Offset 0
	u32 pktlen:16;
	u32 offset:8;
	u32 bmc:1;
	u32 htc:1;
	u32 ls:1;
	u32 fs:1;
	u32 linip:1;
	u32 noacm:1;
	u32 gf:1;
	u32 own:1;

	// Offset 4
	u32 macid:6;
	u32 rsvd0406:2;
	u32 qsel:5;
	u32 rd_nav_ext:1;
	u32 lsig_txop_en:1;
	u32 pifs:1;
	u32 rate_id:4;
	u32 navusehdr:1;
	u32 en_desc_id:1;
	u32 sectype:2;
	u32 rsvd0424:2;
	u32 pkt_offset:5;	// unit: 8 bytes
	u32 rsvd0431:1;

	// Offset 8
	u32 rts_rc:6;
	u32 data_rc:6;
	u32 agg_en:1;
	u32 rd_en:1;
	u32 bar_rty_th:2;
	u32 bk:1;
	u32 morefrag:1;
	u32 raw:1;
	u32 ccx:1;
	u32 ampdu_density:3;
	u32 bt_null:1;
	u32 ant_sel_a:1;
	u32 ant_sel_b:1;
	u32 tx_ant_cck:2;
	u32 tx_antl:2;
	u32 tx_ant_ht:2;

	// Offset 12
	u32 nextheadpage:8;
	u32 tailpage:8;
	u32 seq:12;
	u32 cpu_handle:1;
	u32 tag1:1;
	u32 trigger_int:1;
	u32 hwseq_en:1;

	// Offset 16
	u32 rtsrate:5;
	u32 ap_dcfe:1;
	u32 hwseq_sel:2;
	u32 userate:1;
	u32 disrtsfb:1;
	u32 disdatafb:1;
	u32 cts2self:1;
	u32 rtsen:1;
	u32 hw_rts_en:1;
	u32 port_id:1;
	u32 pwr_status:3;
	u32 wait_dcts:1;
	u32 cts2ap_en:1;
	u32 data_sc:2;
	u32 data_stbc:2;
	u32 data_short:1;
	u32 data_bw:1;
	u32 rts_short:1;
	u32 rts_bw:1;
	u32 rts_sc:2;
	u32 vcs_stbc:2;

	// Offset 20
	u32 datarate:6;
	u32 sgi:1;
	u32 try_rate:1;
	u32 data_ratefb_lmt:5;
	u32 rts_ratefb_lmt:4;
	u32 rty_lmt_en:1;
	u32 data_rt_lmt:6;
	u32 usb_txagg_num:8;

	// Offset 24
	u32 txagg_a:5;
	u32 txagg_b:5;
	u32 use_max_len:1;
	u32 max_agg_num:5;
	u32 mcsg1_max_len:4;
	u32 mcsg2_max_len:4;
	u32 mcsg3_max_len:4;
	u32 mcs7_sgi_max_len:4;

	// Offset 28
	u32 checksum:16;	// TxBuffSize(PCIe)/CheckSum(USB)
	u32 mcsg4_max_len:4;
	u32 mcsg5_max_len:4;
	u32 mcsg6_max_len:4;
	u32 mcs15_sgi_max_len:4;

	// Offset 32
	u32 rsvd32;

	// Offset 36
	u32 rsvd36;
};



#endif
