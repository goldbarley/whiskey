#include "whis/attr.h"
#include "whis/math.h"
#include "whx/winbuf.h"
#include "whx/window.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <xcb/xcb.h>

struct whx_window
{
	xcb_connection_t *conn;
	xcb_window_t handle;
	const char *title;
	uint32_t width;
	uint32_t height;
};

#include <stdint.h>

static struct whx_winbuf
{
	struct whx_window winbuf[WHIS_MAX_X11_WINDOW_COUNT];
	uint64_t freemask;
	uint64_t usedmask;
} Whx_Winbuf = {
	.winbuf = {0},
	.freemask = UINT64_MAX
};

WHIS_EXPORT
whx_window *whx_get_freeaddr(void)
{
	if (!Whx_Winbuf.freemask)
		return NULL;

	uint8_t fi = WHIS_CTZ64(Whx_Winbuf.freemask);
	WHIS_CLR_BIT64(Whx_Winbuf.freemask, fi);
	Whx_Winbuf.usedmask = Whx_Winbuf.freemask;

	return &Whx_Winbuf.winbuf[fi];
}

WHIS_EXPORT
void whx_remove_window(whx_window *win)
{
	if (Whx_Winbuf.freemask == UINT64_MAX || !win)
		return;

	uint8_t wi = win - Whx_Winbuf.winbuf;
	if (wi >= WHIS_MAX_X11_WINDOW_COUNT)
		return;

	if (!(Whx_Winbuf.freemask & (UINT64_C(1) << wi)))
	{
		/* whx_destroy_window(win); */
		WHIS_SET_BIT64(Whx_Winbuf.freemask, wi);
		memset(win, 0, sizeof(struct whx_window));
		Whx_Winbuf.usedmask = Whx_Winbuf.freemask;
	}
}

WHIS_EXPORT
whx_window *whx_create_window(const uint32_t width, const uint32_t height,
			      const char *title)
{
	register struct whx_window *win = whx_get_freeaddr();
	if (!win)
		return NULL;

	win->width = width;
	win->height = height;

	register xcb_connection_t *cnn = win->conn;
	if (xcb_connection_has_error(cnn))
		return NULL;

	const xcb_setup_t *setup = xcb_get_setup(cnn);
	xcb_screen_iterator_t scriter = xcb_setup_roots_iterator(setup);
	xcb_screen_t *scr = scriter.data;

	xcb_cw_t mask = XCB_CW_EVENT_MASK;

	static xcb_event_mask_t evtmask =
		XCB_EVENT_MASK_BUTTON_PRESS |
		XCB_EVENT_MASK_BUTTON_RELEASE |
		XCB_EVENT_MASK_EXPOSURE |
		XCB_EVENT_MASK_FOCUS_CHANGE |
		XCB_EVENT_MASK_KEY_PRESS |
		XCB_EVENT_MASK_KEY_RELEASE |
		XCB_EVENT_MASK_POINTER_MOTION |
		XCB_EVENT_MASK_STRUCTURE_NOTIFY;

	xcb_window_t xwin = xcb_generate_id(cnn);
	win->handle = xwin;

	xcb_void_cookie_t cookie = xcb_create_window_checked(
		cnn, XCB_COPY_FROM_PARENT, xwin, scr->root, 0, 0,
		width, height, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
		scr->root_visual, mask, &evtmask);

	xcb_generic_error_t *error = xcb_request_check(cnn, cookie);
	if (error)
	{
		free(error);
		xcb_disconnect(cnn);
		whx_remove_window(win);

		return NULL;
	}

	xcb_map_window(cnn, xwin);
	xcb_flush(cnn);

	return win;
}

WHIS_EXPORT
void whx_poll_for_event(whx_window *win)
{
	if (!win)
		return;

	register xcb_generic_event_t *evt;
	register xcb_connection_t *cnn = win->conn;
	while ((evt = xcb_poll_for_event(cnn)))
	{
		switch(evt->response_type & ~0x80)
		{
			case XCB_BUTTON_PRESS:
			{

			}
		}
	}
}

WHIS_EXPORT
void whx_destroy_window(whx_window *win)
{
	if (!win)
		return;

	xcb_destroy_window(win->conn, win->handle);
	xcb_disconnect(win->conn);
	whx_remove_window(win);
}
