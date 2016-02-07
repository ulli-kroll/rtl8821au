#ifndef __RTW_CAM_H__
#define __RTW_CAM_H__

#define	CAM_CONTENT_COUNT		8
#define	CFG_VALID			BIT(15)
#define	CAM_WRITE			BIT(16)
#define	CAM_POLLINIG			BIT(31)

void write_cam(struct rtl_priv *rtlpriv, uint8_t entry, u16 ctrl, uint8_t *mac, uint8_t *key);
void clear_cam_entry(struct rtl_priv *rtlpriv, uint8_t entry);

void invalidate_cam_all(struct rtl_priv *rtlpriv);
void CAM_empty_entry(struct rtl_priv *rtlpriv, uint8_t ucIndex);

#endif
