#include <drv_types.h>
#include <../usb.h>
#include <../wifi.h>
#include <../debug.h>
#include <../cam.h>
#include <../rtl8821au/reg.h>


void CAM_empty_entry(
	struct rtl_priv *    	rtlpriv,
	uint8_t 			ucIndex
)
{
	rtlpriv->cfg->ops->set_hw_reg(rtlpriv, HW_VAR_CAM_EMPTY_ENTRY, (uint8_t *)(&ucIndex));
}

void invalidate_cam_all(struct rtl_priv *rtlpriv)
{
	rtlpriv->cfg->ops->set_hw_reg(rtlpriv, HW_VAR_CAM_INVALID_ALL, 0);
}

void write_cam(struct rtl_priv *rtlpriv, uint8_t entry, u16 ctrl, uint8_t *mac, uint8_t *key)
{
	unsigned int	i, val, addr;
	//unsigned int    cmd;
	int j;
	uint32_t	cam_val[2];

	addr = entry << 3;

	for (j = 5; j >= 0; j--)
	{
		switch (j)
		{
			case 0:
				val = (ctrl | (mac[0] << 16) | (mac[1] << 24) );
				break;

			case 1:
				val = (mac[2] | ( mac[3] << 8) | (mac[4] << 16) | (mac[5] << 24));
				break;

			default:
				i = (j - 2) << 2;
				val = (key[i] | (key[i+1] << 8) | (key[i+2] << 16) | (key[i+3] << 24));
				break;

		}

		cam_val[0] = val;
		cam_val[1] = addr + (unsigned int)j;

		rtlpriv->cfg->ops->set_hw_reg(rtlpriv, HW_VAR_CAM_WRITE, (uint8_t *)cam_val);

		//rtw_write32(rtlpriv, WCAMI, val);

		//cmd = CAM_POLLINIG | CAM_WRITE | (addr + j);
		//rtw_write32(rtlpriv, RWCAM, cmd);

		//DBG_871X("%s=> cam write: %x, %x\n",__FUNCTION__, cmd, val);

	}

}

void clear_cam_entry(struct rtl_priv *rtlpriv, uint8_t entry)
{

	unsigned char null_sta[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	unsigned char null_key[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00};

	write_cam(rtlpriv, entry, 0, null_sta, null_key);
}
