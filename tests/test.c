#include "whis/event.h"
#include <whis/whiskey.h>

#include <stdio.h>

int evt_callback(struct wh_event *evt, void *userdata)
{
	switch (evt->type)
	{
		case WHIS_EVENT_KEY_PRESS:
			printf("KEY PRESSED: %x\n", evt->key.id);
			break;
		case WHIS_EVENT_KEY_RELEASE:
			printf("KEY RELEASED: %x\n", evt->key.id);
			break;
		case WHIS_EVENT_BTN_PRESS:
			printf("BUTTON PRESSED: %x\n", evt->ptr.id);
			break;
		case WHIS_EVENT_BTN_RELEASE:
			printf("BUTTON RELEASED: %x\n", evt->ptr.id);
			break;
		case WHIS_EVENT_FOCUS_IN:
			puts("FOCUSED");
			break;
		case WHIS_EVENT_FOCUS_OUT:
			puts("UNFOCUSED");
			break;
		case WHIS_EVENT_PTR_MOVE:
			printf("X: %.3f Y: %.3f\n", evt->ptr.x, evt->ptr.y);
			break;
		case WHIS_EVENT_PTR_IN:
			puts("POINTER IN");
			break;
		case WHIS_EVENT_PTR_OUT:
			puts("POINTER OUT");
			break;
		default:
			return -1;
	}

	return 0;
}

int main(void)
{
	wh_window *window1 = wh_create_window(600, 800, "First window");
	if (!window1)
		return -1;

	struct wh_event evt = {0};

	while(true)
	{
		wh_pollevents(evt_callback, NULL);
	}

	wh_destroy_window(window1);

	return 0;
}
