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

#include <odm_precomp.h>
#include <../rtl8821au/table.h>

#undef ODM_RT_TRACE
#define ODM_RT_TRACE(x, ...)	do {} while (0);

static BOOLEAN CheckCondition(const uint32_t  Condition, const uint32_t  Hex)
{
	uint32_t _board     = (Hex & 0x000000FF);
	uint32_t _interface = (Hex & 0x0000FF00) >> 8;
	uint32_t _platform  = (Hex & 0x00FF0000) >> 16;
	uint32_t cond = Condition;

	if (Condition == 0xCDCDCDCD)
		return TRUE;

	cond = Condition & 0x000000FF;
	if ((_board != cond) && (cond != 0xFF))
		return FALSE;

	cond = Condition & 0x0000FF00;
	cond = cond >> 8;
	if (((_interface & cond) == 0) && (cond != 0x07))
		return FALSE;

	cond = Condition & 0x00FF0000;
	cond = cond >> 16;
	if (((_platform & cond) == 0) && (cond != 0x0F))
		return FALSE;

	return TRUE;
}


/******************************************************************************
*                           AGC_TAB.TXT
******************************************************************************/



/******************************************************************************
*                           PHY_REG.TXT
******************************************************************************/

void ODM_ReadAndConfig_MP_8812A_PHY_REG(struct rtl_priv *rtlpriv)
{
	struct rtl_hal	*rtlhal = rtl_hal(rtlpriv);
	
	#define READ_NEXT_PAIR(v1, v2, i) do { i += 2; v1 = Array[i]; v2 = Array[i+1]; } while(0)

	uint32_t hex = 0;
	uint32_t i = 0;
	u16 count = 0;
	uint32_t *ptr_array   = NULL;

	/* ULLI : fixed values ?? */
	u8 platform = ODM_CE;
	u8 _interface = RTW_USB;
	u8  board = rtlhal->board_type;
	uint32_t ArrayLen    = RTL8812AU_PHY_REG_ARRAY_LEN;
	uint32_t *Array       = RTL8812AU_PHY_REG_ARRAY;


	hex += board;
	hex += _interface << 8;
	hex += platform << 16;
	hex += 0xFF000000;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_TRACE, ("===> ODM_ReadAndConfig_MP_8812A_PHY_REG, hex = 0x%X\n", hex));

	for (i = 0; i < ArrayLen; i += 2) {
		uint32_t v1 = Array[i];
		uint32_t v2 = Array[i+1];

		// This (offset, data) pair meets the condition.
		if ( v1 < 0xCDCDCDCD ) {
			odm_ConfigBB_PHY_8821A(rtlpriv, v1, bMaskDWord, v2);
			continue;
		} else {
			// This line is the start line of branch.
			if (!CheckCondition(Array[i], hex)) {
				// Discard the following (offset, data) pairs.
				READ_NEXT_PAIR(v1, v2, i);
				while (v2 != 0xDEAD && v2 != 0xCDEF &&
					v2 != 0xCDCD && i < ArrayLen -2) {
						READ_NEXT_PAIR(v1, v2, i);
				}
				i -= 2; // prevent from for-loop += 2
			} else {
				// Configure matched pairs and skip to end of if-else.
				READ_NEXT_PAIR(v1, v2, i);
				while (v2 != 0xDEAD && v2 != 0xCDEF &&
					v2 != 0xCDCD && i < ArrayLen -2) {
						odm_ConfigBB_PHY_8821A(rtlpriv, v1, bMaskDWord, v2);

						READ_NEXT_PAIR(v1, v2, i);
				}

				while (v2 != 0xDEAD && i < ArrayLen -2) {
					READ_NEXT_PAIR(v1, v2, i);
				}

			}
		}
	}

}



/******************************************************************************
*                           PHY_REG_PG.TXT
******************************************************************************/


void ODM_ReadAndConfig_MP_8812A_PHY_REG_PG(struct rtl_priv *rtlpriv)
{
	struct rtl_hal	*rtlhal = rtl_hal(rtlpriv);
	
	uint32_t hex = 0;
	uint32_t i = 0;
	u16 count = 0;
	uint32_t *ptr_array   = NULL;

	/* ULLI : fixed values ?? */
	u8  platform = ODM_CE;
	u8 _interface = RTW_USB;
	u8 board = rtlhal->board_type;

	uint32_t     ArrayLen    = RTL8812AU_PHY_REG_PG_ARRAY_LEN;
	uint32_t    *Array       = RTL8812AU_PHY_REG_PG_ARRAY;

#if 0
	pDM_Odm->PhyRegPgValueType = PHY_REG_PG_EXACT_VALUE;
#endif	
	hex += board;
	hex += _interface << 8;
	hex += platform << 16;
	hex += 0xFF000000;

	for (i = 0; i < ArrayLen; i += 3) {
		uint32_t v1 = Array[i];
		uint32_t v2 = Array[i+1];
		uint32_t v3 = Array[i+2];

		// this line is a line of pure_body
		if ( v1 < 0xCDCDCDCD ) {
			odm_ConfigBB_PHY_REG_PG_8821A(rtlpriv, v1, v2, v3);
			continue;
		} else {
			// this line is the start of branch
			if ( !CheckCondition(Array[i], hex) ) {
				// don't need the hw_body

				/* ULLI : BUG ?? must be i += 3 ?? */

				i += 2; // skip the pair of expression

				v1 = Array[i];
				v2 = Array[i+1];
				v3 = Array[i+2];
				while (v2 != 0xDEAD) {
					i += 3;

					v1 = Array[i];
					v2 = Array[i+1];
					v3 = Array[i+1];
				}
			}
		}
	}
}



/******************************************************************************
*                           PHY_REG_PG_ASUS.TXT
******************************************************************************/


void ODM_ReadAndConfig_MP_8812A_PHY_REG_PG_ASUS(struct rtl_priv *rtlpriv)
{
	struct rtl_hal	*rtlhal = rtl_hal(rtlpriv);
	
	#define READ_NEXT_PAIR(v1, v2, i) do { i += 2; v1 = Array[i]; v2 = Array[i+1]; } while(0)

	uint32_t hex = 0;
	uint32_t i = 0;
	u16 count       = 0;
	uint32_t *ptr_array   = NULL;

	/* ULLI : fixed values ?? */
	u8  platform = ODM_CE;
	u8 _interface = RTW_USB;
	u8 board = rtlhal->board_type;

	uint32_t     ArrayLen    = RTL8812AU_PHY_REG_PG_ASUS_ARRAY_LEN;
	uint32_t    *Array       = RTL8812AU_PHY_REG_PG_ASUS_ARRAY;

	hex += board;
	hex += _interface << 8;
	hex += platform << 16;
	hex += 0xFF000000;
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_TRACE, ("===> ODM_ReadAndConfig_MP_8812A_PHY_REG_PG_ASUS, hex = 0x%X\n", hex));

	for (i = 0; i < ArrayLen; i += 2) {
		uint32_t v1 = Array[i];
		uint32_t v2 = Array[i+1];

		// This (offset, data) pair meets the condition.
		if (v1 < 0xCDCDCDCD ) {
			/* ULLI : why not odm_ConfigBB_PHY_REG_PG_8821A() ?? */
			odm_ConfigBB_PHY_8821A(rtlpriv, v1, bMaskDWord, v2);
			continue;
		} else {
			// This line is the start line of branch.
			if (!CheckCondition(Array[i], hex)) {
				// Discard the following (offset, data) pairs.
				READ_NEXT_PAIR(v1, v2, i);
				while (v2 != 0xDEAD && v2 != 0xCDEF &&
					v2 != 0xCDCD && i < ArrayLen -2) {
						READ_NEXT_PAIR(v1, v2, i);
				}
				i -= 2; // prevent from for-loop += 2
			} else {
				// Configure matched pairs and skip to end of if-else.
				READ_NEXT_PAIR(v1, v2, i);
				while (v2 != 0xDEAD && v2 != 0xCDEF &&
					v2 != 0xCDCD && i < ArrayLen -2) {
					/* ULLI : why not odm_ConfigBB_PHY_REG_PG_8821A() ?? */
					odm_ConfigBB_PHY_8821A(rtlpriv, v1, bMaskDWord, v2);
					READ_NEXT_PAIR(v1, v2, i);
				}

				while (v2 != 0xDEAD && i < ArrayLen -2) {
					READ_NEXT_PAIR(v1, v2, i);
				}
			}
		}
	}

}


