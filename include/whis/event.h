#ifndef WHIS_EVENT_H
#define WHIS_EVENT_H

#include "keymap.h"

#include <stdbool.h>

enum wh_event_type
{
	WHIS_EVENT_KEY_PRESS = 0,
	WHIS_EVENT_KEY_RELEASE,
	WHIS_EVENT_BTN_PRESS,
	WHIS_EVENT_BTN_RELEASE,
	WHIS_EVENT_PTR_MOVE,
	WHIS_EVENT_SCROLL,
	WHIS_EVENT_FOCUS_IN,
	WHIS_EVENT_FOCUS_OUT,
	WHIS_EVENT_PTR_IN,
	WHIS_EVENT_PTR_OUT,
	WHIS_EVENT_WINDOW_CONFIGURE,
};

struct wh_event
{
	enum wh_event_type type;

	union
	{
		struct
		{
			wh_keycode id;
		} key;

		struct
		{
			wh_btncode id;
			float x;
			float y;
		} ptr;

		struct
		{
			float dx;
			float dy;
		} scroll;
	};
};

typedef int(*wh_evt_callback)(struct wh_event *, void *userdata);

#endif /* WHIS_EVENT_H */
