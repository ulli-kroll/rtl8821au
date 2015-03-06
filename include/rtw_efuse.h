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
#ifndef __RTW_EFUSE_H__
#define __RTW_EFUSE_H__


#define	EFUSE_ERROE_HANDLE		1

#define	PG_STATE_HEADER 		0x01
#define	PG_STATE_WORD_0		0x02
#define	PG_STATE_WORD_1		0x04
#define	PG_STATE_WORD_2		0x08
#define	PG_STATE_WORD_3		0x10
#define	PG_STATE_DATA			0x20

#define	PG_SWBYTE_H			0x01
#define	PG_SWBYTE_L			0x02

#define	PGPKT_DATA_SIZE		8

#define	EFUSE_WIFI				0
#define	EFUSE_BT				1

enum _EFUSE_DEF_TYPE {
	TYPE_EFUSE_MAX_SECTION				= 0,
	TYPE_EFUSE_REAL_CONTENT_LEN			= 1,
	TYPE_AVAILABLE_EFUSE_BYTES_BANK		= 2,
	TYPE_AVAILABLE_EFUSE_BYTES_TOTAL	= 3,
	TYPE_EFUSE_MAP_LEN					= 4,
	TYPE_EFUSE_PROTECT_BYTES_BANK		= 5,
	TYPE_EFUSE_CONTENT_LEN_BANK			= 6,
};

#define		EFUSE_MAX_MAP_LEN		256
#define		EFUSE_MAX_HW_SIZE		512
#define		EFUSE_MAX_SECTION_BASE	16

#define EXT_HEADER(header) ((header & 0x1F ) == 0x0F)
#define ALL_WORDS_DISABLED(wde)	((wde & 0x0F) == 0x0F)
#define GET_HDR_OFFSET_2_0(header) ( (header & 0xE0) >> 5)

#define		EFUSE_REPEAT_THRESHOLD_			3

//=============================================
//	The following is for BT Efuse definition
//=============================================
#define		EFUSE_BT_MAX_MAP_LEN		1024
#define		EFUSE_MAX_BANK			4
#define		EFUSE_MAX_BT_BANK		(EFUSE_MAX_BANK-1)
//=============================================
/*--------------------------Define Parameters-------------------------------*/
#define		EFUSE_MAX_WORD_UNIT			4

/*------------------------------Define structure----------------------------*/
typedef struct PG_PKT_STRUCT_A{
	uint8_t offset;
	uint8_t word_en;
	uint8_t data[8];
	uint8_t word_cnts;
}PGPKT_STRUCT,*PPGPKT_STRUCT;

/*------------------------------Define structure----------------------------*/


/*------------------------Export global variable----------------------------*/

u16	efuse_GetMaxSize(struct rtl_priv *rtlpriv);
uint8_t	rtw_efuse_map_read(struct rtl_priv *rtlpriv, u16 addr, u16 cnts, uint8_t *data);
uint8_t	rtw_efuse_map_write(struct rtl_priv *rtlpriv, u16 addr, u16 cnts, uint8_t *data);
uint8_t	rtw_BT_efuse_map_read(struct rtl_priv *rtlpriv, u16 addr, u16 cnts, uint8_t *data);
uint8_t 	rtw_BT_efuse_map_write(struct rtl_priv *rtlpriv, u16 addr, u16 cnts, uint8_t *data);

uint8_t	Efuse_CalculateWordCnts(uint8_t word_en);
void	ReadEFuseByte(struct rtl_priv *rtlpriv, u16 _offset, uint8_t *pbuf) ;
uint8_t	efuse_OneByteRead(struct rtl_priv *rtlpriv, u16 addr, uint8_t *data);
uint8_t	efuse_OneByteWrite(struct rtl_priv *rtlpriv, u16 addr, uint8_t data);

int 	Efuse_PgPacketRead(struct rtl_priv *rtlpriv, uint8_t offset, uint8_t *data);
int 	Efuse_PgPacketWrite(struct rtl_priv *rtlpriv, uint8_t offset, uint8_t word_en, uint8_t *data);
void	efuse_WordEnableDataRead(uint8_t word_en, uint8_t *sourdata, uint8_t *targetdata);
uint8_t	Efuse_WordEnableDataWrite(struct rtl_priv *rtlpriv, u16 efuse_addr, uint8_t word_en, uint8_t *data);

uint8_t	EFUSE_Read1Byte(struct rtl_priv *rtlpriv, u16 Address);
void	EFUSE_ShadowMapUpdate(struct rtl_priv *rtlpriv, uint8_t efuseType);
void	EFUSE_ShadowRead(struct rtl_priv *rtlpriv, uint8_t Type, u16 Offset, u32 *Value);
#endif

