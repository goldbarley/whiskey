#ifndef WHIS_WINDOW_H_
#define WHIS_WINDOW_H_ 1

#include "attr.h"

#include <stdint.h>

#if defined(WHIS_PLATFORM_X11)
#include "whx/types.h"
#include "whx/winbuf.h"
#include "whx/window.h"
typedef whx_window wh_window;
#elif defined(WHIS_PLATFORM_WAYLAND)
#include "whwl/types.h"
#include "whwl/window.h"
typedef whwl_window wh_window;
#elif defined(WHIS_PLATFORM_WIN32)s
#include "whw32/types.h"
#include "whw32/window.h"
typedef whw32_window wh_window;
#else
#error "Unsupported platform."
#endif /* Platforms */

WHIS_EXPORT
wh_window *wh_create_window(uint32_t width, uint32_t height,
			    const char *title);

WHIS_EXPORT
void wh_destroy_window(wh_window *win);

WHIS_EXPORT
void wh_poll_for_event(wh_window *win, wh_evt_callback callback,
			void *userdata);

WHIS_EXPORT
void wh_pollevents(wh_evt_callback callback, void *userdata);

#endif /* WHIS_WINDOW_H_ */
