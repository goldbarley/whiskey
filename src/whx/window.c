#define VK_USE_PLATFORM_XCB_KHR

#include "whis/attr.h"
#include "whis/event.h"
#include "whis/math.h"
#include "whx/util.h"
#include "whis/vulkan.h"
#include "whis/window.h"

#include <xcb/xcb.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <xcb/xproto.h>

struct wh_window
{
	xcb_connection_t *connection;
	xcb_window_t window;
	const char *title;
	uint32_t width;
	uint32_t height;
	bool focused;
	bool ptrin;
};

#include <stdint.h>

static struct wh_winbuf
{
	struct wh_window winbuf[WHIS_MAX_WINDOW_COUNT];
	xcb_connection_t *connection;
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
void wh_poll_for_event(wh_window *window, wh_evt_callback callback,
			void *userdata)
{
	if (!window)
		return;

	register xcb_generic_event_t *evt;
	register xcb_connection_t *cnn = window->connection;
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
				window->focused = WHIS_TRUE;

				struct wh_event whevt = {
					.type = WHIS_EVENT_FOCUS_IN
				};

				callback(&whevt, userdata);

				break;
			}
			case XCB_FOCUS_OUT:
			{
				window->focused = WHIS_FALSE;

				struct wh_event whevt = {
					.type = WHIS_EVENT_FOCUS_OUT
				};

				callback(&whevt, userdata);

				break;
			}

			case XCB_ENTER_NOTIFY:
			{
				window->ptrin = WHIS_TRUE;

				struct wh_event whevt = {
					.type = WHIS_EVENT_PTR_IN
				};

				callback(&whevt, userdata);

				break;
			}
			case XCB_LEAVE_NOTIFY:
			{
				window->ptrin = WHIS_FALSE;

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

				window->width = cevt->width;
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
void wh_pollevents(wh_evt_callback callback, void *userdata)
{
	uint8_t i = 0;
	uint64_t umask = Whx_Winbuf.usedmask;

	while(umask)
	{
		i = WHIS_CTZ64(umask);
		windump = &Whx_Winbuf.winbuf[i];
		wh_poll_for_event(windump, callback, userdata);
		WHIS_CLR_BIT64(umask, i);
	}
}

WHIS_EXPORT
wh_fnresult whx_get_framebuffer_size(wh_window *win, uint32_t *width,
			      uint32_t *height)
{
	if (!win || !width || !height)
		return WHIS_INVARG;



	xcb_get_geometry_cookie_t cookie = xcb_get_geometry(win->connection,
							     win->window);
	xcb_get_geometry_reply_t *reply = xcb_get_geometry_reply(win->connection,
								 cookie, NULL);

	if (!reply)
		return WHIS_FAILURE;

	*width = reply->width;
	*height = reply ->height;

	free(reply);

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
void wh_destroy_window(wh_window *win)
{
	if (!win)
		return;

	xcb_destroy_window(win->connection, win->window);
	whx_remove_window(win);
}

WHIS_EXPORT
wh_fnresult wh_create_vulkan_surface(VkSurfaceKHR *surface, wh_window *window,
				     VkInstance instance,
				     PFN_vkCreateXcbSurfaceKHR fp,
				     VkAllocationCallbacks *allocator)
{
	if (!surface || !window)
		return WHIS_INVARG;

	VkResult vkres = VK_SUCCESS;

	VkXcbSurfaceCreateInfoKHR createinfo = {
		.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
		.pNext = NULL,
		.connection = window->connection,
		.window = window->window
	};

	vkres = fp(instance, &createinfo, allocator, surface);
	if (vkres != VK_SUCCESS)
		return WHIS_FAILURE;

	return WHIS_SUCCESS;
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

	xcb_disconnect(Whx_Winbuf.connection);
}
