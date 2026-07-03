#include "whis/attr.h"
#include "whis/keymap.h"

#include <xcb/xcb.h>

#include <stdint.h>

wh_keycode whx_cvt_keycode(xcb_keycode_t keycode);

wh_btncode whx_cvt_btncode(xcb_button_t btncode);
