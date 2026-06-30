#include "whx/util.h"
#include "whis/keymap.h"

#ifdef __linux__
wh_keycode whx_cvt_keycode(xcb_keycode_t keycode)
{
	return keycode < 8 ? 0 : Wh_Evdev2Whis_Keymap[keycode - 8];
}
#endif /* __linux__ */
