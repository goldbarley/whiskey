#ifndef WHIS_WINDOW_H_
#define WHIS_WINDOW_H_ 1

#include "attr.h"
#include "event.h"
#include "types.h"

#define WHIS_MAX_WINDOW_COUNT (64U)

WHIS_EXPORT
wh_window *wh_create_window(const uint32_t width, const uint32_t height,
			    const char *title);

WHIS_EXPORT
void wh_destroy_window(wh_window *window);

WHIS_EXPORT
void wh_poll_for_event(wh_window *window, wh_evt_callback callback,
		       void *userdata);

WHIS_EXPORT
void wh_pollevents(wh_evt_callback callback, void *userdata);

WHIS_EXPORT
wh_fnresult wh_get_framebuffer_size(wh_window *window, uint32_t *width,
				    uint32_t *height);

WHIS_EXPORT
wh_bool wh_window_in_focus(wh_window *window);

WHIS_EXPORT
wh_bool wh_ptr_in_window(wh_window *window);

WHIS_EXPORT
void wh_shutdown(void);

#endif /* WHIS_WINDOW_H_ */
