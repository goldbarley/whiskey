#ifndef WHIS_X11_WINDOW_H
#define WHIS_X11_WINDOW_H_ 1

#include "types.h"
#include "whis/attr.h"
#include "whis/event.h"
#include "whis/types.h"

#include <stdint.h>
#include <stdbool.h>

WHIS_EXPORT
whx_window *whx_create_window(const uint32_t width, const uint32_t height,
			      const char *title);

WHIS_EXPORT
void whx_destroy_window(whx_window *window);

WHIS_EXPORT
void whx_poll_for_event(whx_window *window, wh_evt_callback callback,
			void *userdata);

WHIS_EXPORT
void whx_pollevents(wh_evt_callback callback, void *userdata);

WHIS_EXPORT
wh_fnresult whx_get_framebuffer_size(whx_window *window, uint32_t *width,
				     uint32_t *height);

WHIS_EXPORT
bool whx_window_in_focus(whx_window *window);

WHIS_EXPORT
bool whx_ptr_in_window(whx_window *window);

#endif /* WHIS_X11_WINDOW_H_ */
