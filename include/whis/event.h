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
	WH_EVENT_PTR_MOVE
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
			int32_t x;
			int32_t y;
		} ptr;
	};
};

typedef void(*wh_evt_callback)(struct wh_event *, void *userdata);

#endif /* WHIS_EVENT_H */
