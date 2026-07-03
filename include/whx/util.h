#ifndef WHIS_X11_UTIL_H_
#define WHIS_X11_UTIL_H_ 1

#include "whis/keymap.h"

#include <xcb/xcb.h>

#include <stdint.h>

wh_keycode whx_cvt_keycode(xcb_keycode_t keycode);

wh_btncode whx_cvt_btncode(xcb_button_t btncode);

#endif /* WHIS_X11_UTIL_H_ */
