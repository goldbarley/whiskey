#ifndef WHIS_X11_WINDOW_H
#define WHIS_X11_WINDOW_H

#include "types.h"
#include "whis/attr.h"
#include "whis/event.h"

#include <stdint.h>
#include <stdbool.h>

WHIS_EXPORT
whx_window *whx_create_window(const uint32_t width, const uint32_t height,
			      const char *title);

WHIS_EXPORT
void whx_destroy_window(whx_window *win);

WHIS_EXPORT
void whx_poll_for_event(whx_window *win, wh_evt_callback callback,
			void *userdata);

WHIS_EXPORT
void whx_pollevents(wh_evt_callback callback, void *userdata);

#endif /* WHIS_X11_WINDOW_H */
