#include "whx/util.h"
#include "whis/keymap.h"

#ifdef __linux__
#include "whis/linux.h"

wh_keycode whx_cvt_keycode(uint8_t keycode)
{
	return keycode < 8 ? 0 : Wh_Evdev2Whis_Keymap[keycode - 8];
}

wh_btncode whx_cvt_btncode(uint8_t btncode)
{
	switch (btncode)
	{
		case 1:
			return WHIS_BTN_1;
		case 2:
			return WHIS_BTN_3;
		case 3:
			return WHIS_BTN_2;
		case 8:
			return WHIS_BTN_4;
		case 9:
			return WHIS_BTN_5;
		default:
			if (btncode > 9)
				return WHIS_BTN_X;
		return 0;
	}
}
#endif /* __linux__ */
