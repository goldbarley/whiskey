#include "whis/window.h"

WHIS_EXPORT
wh_window *wh_create_window(uint32_t width, uint32_t height,
			    const char *title)
{
#if defined(WHIS_PLATFORM_X11)
	return whx_create_window(width, height, title);
#elif defined(WHIS_PLATFORM_WAYLAND)
	return whwl_create_window(width, height, title);
#elif defined(WHIS_PLATFORM_WIN32)
	return whw32_create_window(width, height, title);
#else
#error "Unsupported platform."
#endif /* Platforms */
}

WHIS_EXPORT
void wh_destroy_window(wh_window *win)
{
#if defined(WHIS_PLATFORM_X11)
	return whx_destroy_window(win);
#elif defined(WHIS_PLATFORM_WAYLAND)
	return whwl_destroy_window(win);
#elif defined(WHIS_PLATFORM_WIN32)
	return whw32_destroy_window(win);
#else
#error "Unsupported platform."
#endif /* Platforms */
}

WHIS_EXPORT
void wh_poll_for_event(wh_window *win, wh_evt_callback callback,
		       void *userdata)
{
#if defined(WHIS_PLATFORM_X11)
	return whx_poll_for_event(win, callback, userdata);
#elif defined(WHIS_PLATFORM_WAYLAND)
	return whwl_poll_for_event(win, callback, userdata);
#elif defined(WHIS_PLATFORM_WIN32)
	return whw32_poll_for_event(win, callback, userdata);
#else
#error "Unsupported platform."
#endif /* Platforms */
}

WHIS_EXPORT
void wh_pollevents(wh_evt_callback callback, void *userdata)
{
#if defined(WHIS_PLATFORM_X11)
	return whx_pollevents(callback, userdata);
#elif defined(WHIS_PLATFORM_WAYLAND)
	return whwl_pollevents(callback, userdata);
#elif defined(WHIS_PLATFORM_WIN32)
	return whw32_pollevents(callback, userdata);
#else
#error "Unsupported platform."
#endif /* Platforms */
}
