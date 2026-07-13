#include "handle.h"
#include "whis/attr.h"
#include "whis/event.h"
#include "whis/math.h"
#include "whis/types.h"
#include "whx/util.h"
#include "whis/window.h"

#include <xcb/xcb.h>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdint.h>

static struct whx_winbuf
{
	struct wh_window winbuf[WHIS_MAX_WINDOW_COUNT];
	xcb_connection_t *connection;
	xcb_atom_t wm_protocols;
	xcb_atom_t wm_del_win;
	uint64_t freemask;
	uint64_t usedmask;
	wh_bool connected;
} Whx_Winbuf = {
	.winbuf = {0},
	.freemask = UINT64_MAX
};

static struct wh_window *windump = NULL;

wh_window *whx_get_freeaddr(void)
{
	if (!Whx_Winbuf.freemask)
		return NULL;

	uint8_t fi = WHIS_CTZ64(Whx_Winbuf.freemask);
	WHIS_CLR_BIT64(Whx_Winbuf.freemask, fi);
	Whx_Winbuf.usedmask = ~Whx_Winbuf.freemask;
	Whx_Winbuf.winbuf[fi].wid = (int8_t)(fi);

	return &Whx_Winbuf.winbuf[fi];
}

void whx_remove_window(struct wh_window *window)
{
	if (Whx_Winbuf.freemask == UINT64_MAX || !window)
		return;

	uint8_t wi = (uint8_t)(window - Whx_Winbuf.winbuf);
	if (wi >= WHIS_MAX_WINDOW_COUNT)
		return;

	if (!(Whx_Winbuf.freemask & (UINT64_C(1) << wi)))
	{
		/* whx_destroy_window(win); */
		WHIS_SET_BIT64(Whx_Winbuf.freemask, wi);
		memset(window, 0, sizeof(struct wh_window));
		Whx_Winbuf.usedmask = ~Whx_Winbuf.freemask;
	}
}

WHIS_EXPORT
wh_window *wh_create_window(const uint32_t width, const uint32_t height,
			      const char *title)
{
	static xcb_connection_t *cnn;

	if (!Whx_Winbuf.connected)
	{
		cnn = xcb_connect(NULL, NULL);
		if (xcb_connection_has_error(cnn))
			return NULL;

		xcb_intern_atom_cookie_t wm_protocols =
			xcb_intern_atom(cnn, 0, 12, "WM_PROTOCOLS");

		xcb_intern_atom_cookie_t wm_del_win =
			xcb_intern_atom(cnn, 0, 16, "WM_DELETE_WINDOW");

		xcb_intern_atom_reply_t *reply_protocols =
			xcb_intern_atom_reply(cnn, wm_protocols, NULL);

		xcb_intern_atom_reply_t *reply_del_win =
			xcb_intern_atom_reply(cnn, wm_del_win, NULL);

		if (!reply_protocols || !reply_del_win)
		{
			free(reply_protocols);
			free(reply_del_win);
			xcb_disconnect(cnn);

			return NULL;
		}

		Whx_Winbuf.wm_protocols = reply_protocols->atom;
		Whx_Winbuf.wm_del_win = reply_del_win->atom;
		Whx_Winbuf.connected = WHIS_TRUE;

		Whx_Winbuf.connection = cnn;
	}

	register struct wh_window *win = whx_get_freeaddr();
	if (!win)
		return NULL;

	win->width = width;
	win->height = height;

	win->connection = cnn;

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
	win->window = xwin;

	xcb_create_window(cnn, XCB_COPY_FROM_PARENT, xwin, scr->root, 0, 0,
			  width, height, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
			  scr->root_visual, mask, &evtmask);

	if (title)
		xcb_change_property(cnn, XCB_PROP_MODE_REPLACE, xwin,
				    XCB_ATOM_WM_NAME, XCB_ATOM_STRING,
				    8, strlen(title), title);

	xcb_change_property(cnn, XCB_PROP_MODE_REPLACE, xwin,
			    Whx_Winbuf.wm_protocols, XCB_ATOM_ATOM, 32, 1,
			    &Whx_Winbuf.wm_del_win);

	xcb_map_window(cnn, xwin);
	xcb_flush(cnn);

	return win;
}

struct wh_window *whx_get_window(xcb_window_t xwin)
{
	uint64_t umask = Whx_Winbuf.usedmask;
	uint8_t i = 0;

	while(umask)
	{
		i = WHIS_CTZ64(umask);
		if (Whx_Winbuf.winbuf[i].window == xwin)
			return &Whx_Winbuf.winbuf[i];
		WHIS_CLR_BIT64(umask, i);
	}

	return NULL;
}

WHIS_EXPORT
void wh_poll_events(struct wh_event *event)
{
	if (!event || !Whx_Winbuf.connected)
		return;

	xcb_generic_event_t *evt;

	struct wh_window *window = NULL;
	register struct wh_event *revent = event;

	revent->window = NULL;
	revent->type = WHIS_EVENT_UNKNOWN;

	uint32_t rt = 0;
	while ((evt = xcb_poll_for_event(Whx_Winbuf.connection)))
	{
		rt = evt->response_type & ~(0x80);
		switch(rt)
		{
			case XCB_KEY_PRESS:
			case XCB_KEY_RELEASE:
			{
				xcb_key_press_event_t *kpevt =
					(xcb_key_press_event_t *)(evt);

				window = whx_get_window(kpevt->event);
				if (!window)
					break;

				revent->window = window;
				revent->type = (rt == XCB_KEY_PRESS) ?
					WHIS_EVENT_KEY_PRESS :
					WHIS_EVENT_KEY_RELEASE;
				revent->key.id = whx_cvt_keycode(kpevt->detail);

				break;
			}

			case XCB_BUTTON_PRESS:
			case XCB_BUTTON_RELEASE:
			{
				xcb_button_press_event_t *bpevt =
					(xcb_button_press_event_t *)(evt);

				window = whx_get_window(bpevt->event);
				if (!window)
					break;

				revent->window = window;
				revent->type = (rt == XCB_BUTTON_PRESS) ?
					WHIS_EVENT_BTN_PRESS :
					WHIS_EVENT_BTN_RELEASE;
				revent->ptr.id = whx_cvt_btncode(bpevt->detail);

				break;
			}

			case XCB_MOTION_NOTIFY:
			{
				xcb_motion_notify_event_t *mevt =
					(xcb_motion_notify_event_t *)(evt);

				window = whx_get_window(mevt->event);
				if (!window)
					break;

				revent->window = window;
				revent->type = WHIS_EVENT_PTR_MOVE;
				revent->ptr.x = mevt->event_x;
				revent->ptr.y = mevt->event_y;

				break;
			}

			case XCB_FOCUS_IN:
			{
				xcb_focus_in_event_t *fievt =
					(xcb_focus_in_event_t *)(evt);

				window = whx_get_window(fievt->event);
				if (!window)
					break;
				window->focused = WHIS_TRUE;

				revent->window = window;
				revent->type = WHIS_EVENT_FOCUS_IN;

				break;
			}
			case XCB_FOCUS_OUT:
			{
				xcb_focus_out_event_t *foevt =
					(xcb_focus_out_event_t *)(evt);

				window = whx_get_window(foevt->event);
				if (!window)
					break;
				window->focused = WHIS_FALSE;

				revent->window = window;
				revent->type = WHIS_EVENT_FOCUS_OUT;

				break;
			}

			case XCB_ENTER_NOTIFY:
			{
				xcb_enter_notify_event_t *enevt =
					(xcb_enter_notify_event_t *)(evt);

				window = whx_get_window(enevt->event);
				if (!window)
					break;
				window->ptrin = WHIS_TRUE;

				revent->window = window;
				revent->type = WHIS_EVENT_PTR_IN;

				break;
			}
			case XCB_LEAVE_NOTIFY:
			{
				xcb_leave_notify_event_t *lnevt =
					(xcb_leave_notify_event_t *)(evt);

				window = whx_get_window(lnevt->event);
				if (!window)
					break;
				window->ptrin = WHIS_FALSE;

				revent->window = window;
				revent->type = WHIS_EVENT_PTR_OUT;

				break;
			}

			case XCB_CONFIGURE_NOTIFY:
			{
				xcb_configure_notify_event_t *cevt =
					(xcb_configure_notify_event_t *)(evt);

				window = whx_get_window(cevt->event);
				if (!window)
					break;

				window->width = cevt->width;
				window->height = cevt->height;

				revent->window = window;
				revent->type = WHIS_EVENT_WINDOW_CONFIGURE;

				break;
			}

			case XCB_CLIENT_MESSAGE:
			{
				xcb_client_message_event_t *cmevt =
					(xcb_client_message_event_t *)(evt);

				window = whx_get_window(cmevt->window);
				if (!window)
					break;

				revent->window = window;

				if (cmevt->data.data32[0] == Whx_Winbuf.wm_del_win)
				{
					revent->type = WHIS_EVENT_WINDOW_CLOSE;

					break;
				}

				revent->type = WHIS_EVENT_UNKNOWN;

				break;
			}

			default:
				break;

		}
		free(evt);
		if (revent->type != WHIS_EVENT_UNKNOWN)
			return;
	}
}

WHIS_EXPORT
wh_fnresult wh_get_framebuffer_size(wh_window *win, uint32_t *width,
			      uint32_t *height)
{
	if (!win || !width || !height)
		return WHIS_INVARG;

	*width = win->width;
	*height = win->height;

	return WHIS_SUCCESS;
}

WHIS_EXPORT
wh_bool wh_window_in_focus(wh_window *window)
{
	return window->focused;
}

WHIS_EXPORT
wh_bool wh_ptr_in_window(wh_window *window)
{
	return window->ptrin;
}

WHIS_EXPORT
wh_fnresult wh_resize_window(wh_window *window, const uint32_t width,
			     const uint32_t height)
{
	if (!window)
		return WHIS_INVARG;

	register struct wh_window *rwin = window;
	const uint32_t values[2] = { width, height };


	xcb_configure_window(rwin->connection,
			     rwin->window,
			     XCB_CONFIG_WINDOW_WIDTH |
			     XCB_CONFIG_WINDOW_HEIGHT,
		             values);

	return WHIS_SUCCESS;
}

WHIS_EXPORT
wh_fnresult wh_restore_window_size(wh_window *window)
{
	if (!window)
		return WHIS_INVARG;

	const uint32_t values[2] = { window->width, window->height };

	xcb_configure_window(window->connection,
			     window->window,
			     XCB_CONFIG_WINDOW_WIDTH |
			     XCB_CONFIG_WINDOW_HEIGHT,
			     values);

	return WHIS_SUCCESS;
}

WHIS_EXPORT
int8_t wh_get_window_id(wh_window *window)
{
	if (!window)
		return -1;

	return window->wid;
}

WHIS_EXPORT
void wh_destroy_window(wh_window *win)
{
	if (!win)
		return;

	xcb_unmap_window(win->connection, win->window);
	xcb_flush(win->connection);
	whx_remove_window(win);
}

WHIS_EXPORT
void wh_shutdown(void)
{
	uint8_t i = 0;
	uint64_t umask = Whx_Winbuf.usedmask;

	while(umask)
	{
		i = WHIS_CTZ64(umask);
		windump = &Whx_Winbuf.winbuf[i];
		wh_destroy_window(windump);
		WHIS_CLR_BIT64(umask, i);
	}

	if (Whx_Winbuf.connection)
		xcb_disconnect(Whx_Winbuf.connection);

	memset(&Whx_Winbuf, 0, sizeof(Whx_Winbuf));
}
