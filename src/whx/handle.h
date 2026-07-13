#ifndef WHIS_X11_HANDLE_H_
#define WHIS_X11_HANDLE_H_ 1

#include "whis/types.h"

#include <xcb/xcb.h>

#include <stdint.h>

struct wh_window
{
	xcb_connection_t *connection;
	xcb_window_t window;
	const char *title;
	uint32_t width;
	uint32_t height;
	int8_t wid;
	wh_bool focused;
	wh_bool ptrin;
};

#endif /* WHIS_X11_HANDLE_H_ */
