#include "whis/attr.h"
#include "whis/event.h"
#include "whis/math.h"
#include "whx/util.h"
#include "whx/winbuf.h"
#include "whx/window.h"

#include <xcb/xcb.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <xcb/xproto.h>

struct whx_window
{
	xcb_connection_t *conn;
	xcb_window_t handle;
	const char *title;
	uint32_t width;
	uint32_t height;
	bool focused;
	bool ptrin;
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

static struct whx_window *windump = NULL;

WHIS_EXPORT
whx_window *whx_get_freeaddr(void)
{
	if (!Whx_Winbuf.freemask)
		return NULL;

	uint8_t fi = WHIS_CTZ64(Whx_Winbuf.freemask);
	WHIS_CLR_BIT64(Whx_Winbuf.freemask, fi);
	Whx_Winbuf.usedmask = ~Whx_Winbuf.freemask;

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
		Whx_Winbuf.usedmask = ~Whx_Winbuf.freemask;
	}
}

WHIS_EXPORT
whx_window *whx_create_window(const uint32_t width, const uint32_t height,
			      const char *title)
{
	register xcb_connection_t *cnn = xcb_connect(NULL, NULL);
	if (xcb_connection_has_error(cnn))
		return NULL;

	register struct whx_window *win = whx_get_freeaddr();
	if (!win)
		return NULL;

	win->width = width;
	win->height = height;

	win->conn = cnn;

	const xcb_setup_t *setup = xcb_get_setup(cnn);
	xcb_screen_iterator_t scriter = xcb_setup_roots_iterator(setup);
	xcb_screen_t *scr = scriter.data;

	xcb_cw_t mask = XCB_CW_EVENT_MASK;

	static xcb_event_mask_t evtmask =
		XCB_EVENT_MASK_BUTTON_PRESS |
		XCB_EVENT_MASK_BUTTON_RELEASE |
		XCB_EVENT_MASK_ENTER_WINDOW |
		XCB_EVENT_MASK_LEAVE_WINDOW |
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
void whx_poll_for_event(whx_window *window, wh_evt_callback callback,
			void *userdata)
{
	if (!window)
		return;

	register xcb_generic_event_t *evt;
	register xcb_connection_t *cnn = window->conn;
	register uint32_t rt = 0;

	while ((evt = xcb_poll_for_event(cnn)))
	{
		rt = evt->response_type & ~(0x80);
		switch(rt)
		{
			case XCB_KEY_PRESS:
			case XCB_KEY_RELEASE:
			{
				xcb_key_press_event_t *kpevt =
					(xcb_key_press_event_t *)(evt);

				struct wh_event whevt = {
					.type = (rt == XCB_KEY_PRESS) ?
						WHIS_EVENT_KEY_PRESS :
						WHIS_EVENT_KEY_RELEASE,
					.key.id = whx_cvt_keycode(kpevt->detail)
				};

				if (whevt.key.id)
					callback(&whevt, userdata);

				break;
			}

			case XCB_BUTTON_PRESS:
			case XCB_BUTTON_RELEASE:
			{
				xcb_button_press_event_t *bpevt =
					(xcb_button_press_event_t *)(evt);

				struct wh_event whevt = {
					.type = (rt == XCB_BUTTON_PRESS) ?
						WHIS_EVENT_BTN_PRESS :
						WHIS_EVENT_BTN_RELEASE,
					.ptr.id = whx_cvt_btncode(bpevt->detail)
				};

				if (whevt.ptr.id)
					callback(&whevt, userdata);

				break;
			}

			case XCB_MOTION_NOTIFY:
			{
				xcb_motion_notify_event_t *mevt =
					(xcb_motion_notify_event_t *)(evt);

				struct wh_event whevt = {
					.type = WHIS_EVENT_PTR_MOVE,
					.ptr.x = mevt->event_x,
					.ptr.y = mevt->event_y
				};

				callback(&whevt, userdata);

				break;
			}

			case XCB_FOCUS_IN:
			{
				window->focused = true;

				struct wh_event whevt = {
					.type = WHIS_EVENT_FOCUS_IN
				};

				callback(&whevt, userdata);

				break;
			}
			case XCB_FOCUS_OUT:
			{
				window->focused = false;

				struct wh_event whevt = {
					.type = WHIS_EVENT_FOCUS_OUT
				};

				callback(&whevt, userdata);

				break;
			}

			case XCB_ENTER_NOTIFY:
			{
				window->ptrin = true;

				struct wh_event whevt = {
					.type = WHIS_EVENT_PTR_IN
				};

				callback(&whevt, userdata);

				break;
			}
			case XCB_LEAVE_NOTIFY:
			{
				window->ptrin = false;

				struct wh_event whevt = {
					.type = WHIS_EVENT_PTR_OUT
				};

				callback(&whevt, userdata);

				break;
			}

			case XCB_CONFIGURE_NOTIFY:
			{
				xcb_configure_notify_event_t *cevt =
					(xcb_configure_notify_event_t *)(evt);

				window->height = cevt->width;
				window->height = cevt->height;

				struct wh_event whevt = {
					.type = WHIS_EVENT_WINDOW_CONFIGURE
				};

				callback(&whevt, userdata);

				break;
			}

			default:
				break;

		}
		free(evt);
	}
}

WHIS_EXPORT
void whx_pollevents(wh_evt_callback callback, void *userdata)
{
	uint8_t i = 0;
	uint64_t umask = Whx_Winbuf.usedmask;

	while(umask)
	{
		i = WHIS_CTZ64(umask);
		windump = &Whx_Winbuf.winbuf[i];
		whx_poll_for_event(windump, callback, userdata);
		WHIS_CLR_BIT64(umask, i);
	}
}

WHIS_EXPORT
wh_fnresult whx_get_framebuffer_size(whx_window *win, uint32_t *width,
			      uint32_t *height)
{
	if (!win || !width || !height)
		return WHIS_INVARG;



	xcb_get_geometry_cookie_t cookie = xcb_get_geometry(win->conn,
							     win->handle);
	xcb_get_geometry_reply_t *reply = xcb_get_geometry_reply(win->conn,
								 cookie, NULL);

	if (!reply)
		return WHIS_FAILURE;

	*width = reply->width;
	*height = reply ->height;

	free(reply);

	return WHIS_SUCCESS;
}

WHIS_EXPORT
bool whx_window_in_focus(whx_window *window)
{
	return window->focused;
}

WHIS_EXPORT
bool whx_ptr_in_window(whx_window *window)
{
	return window->ptrin;
}

WHIS_EXPORT
void whx_destroy_window(whx_window *win)
{
	if (!win)
		return;

	uint8_t i = 0;

	xcb_destroy_window(win->conn, win->handle);
	xcb_disconnect(win->conn);
	whx_remove_window(win);
}
