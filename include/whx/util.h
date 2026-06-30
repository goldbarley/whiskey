#include "whis/attr.h"
#include "whis/keymap.h"

#include <xcb/xcb.h>

#include <stdint.h>

WHIS_EXPORT
wh_keycode whx_cvt_keycode(xcb_keycode_t keycode);
