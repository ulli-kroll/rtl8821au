#ifndef __RTW_CAM_H__
#define __RTW_CAM_H__

void write_cam(struct rtl_priv *rtlpriv, uint8_t entry, u16 ctrl, uint8_t *mac, uint8_t *key);
void clear_cam_entry(struct rtl_priv *rtlpriv, uint8_t entry);

void invalidate_cam_all(struct rtl_priv *rtlpriv);
void CAM_empty_entry(struct rtl_priv *rtlpriv, uint8_t ucIndex);

#endif
