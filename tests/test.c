#define _GNU_SOURCE

#include <whis/whiskey.h>

#include <stdio.h>

#include <unistd.h>

static uint64_t cnt = 0;

int event_handler(struct wh_event *evt, uint8_t *actwins)
{
	switch (evt->type)
	{
		case WHIS_EVENT_KEY_PRESS:
			printf("KEY PRESSED: %x\n", evt->key.id);
			fflush(stdout);
			break;
		case WHIS_EVENT_KEY_RELEASE:
			printf("KEY RELEASED: %x\n", evt->key.id);
			fflush(stdout);
			break;
		case WHIS_EVENT_BTN_PRESS:
			printf("BUTTON PRESSED: %x\n", evt->ptr.id);
			fflush(stdout);
			break;
		case WHIS_EVENT_BTN_RELEASE:
			printf("BUTTON RELEASED: %x\n", evt->ptr.id);
			fflush(stdout);
			break;
		case WHIS_EVENT_FOCUS_IN:
			printf("%lu FOCUSED: %i\n", cnt, wh_get_window_id(evt->window));
			fflush(stdout);
			break;
		case WHIS_EVENT_FOCUS_OUT:
			printf("%lu UNFOCUSED: %i\n", cnt, wh_get_window_id(evt->window));
			fflush(stdout);
			break;
		case WHIS_EVENT_PTR_MOVE:
			// printf("X: %.3f Y: %.3f\n", evt->ptr.x, evt->ptr.y);
			// fflush(stdout);
			break;
		case WHIS_EVENT_PTR_IN:
			printf("%lu POINTER IN: %i\n", cnt, wh_get_window_id(evt->window));
			fflush(stdout);
			break;
		case WHIS_EVENT_PTR_OUT:
			printf("%lu POINTER OU: %i\n", cnt, wh_get_window_id(evt->window));
			fflush(stdout);
			break;
		case WHIS_EVENT_WINDOW_CONFIGURE:
			puts("WINDOW RESIZE!");
			fflush(stdout);
			wh_restore_window_size(evt->window);
			break;
		case WHIS_EVENT_WINDOW_CLOSE:
			printf("WINDOW CLOSE: %i\n", wh_get_window_id(evt->window));
			--(*actwins);
			wh_destroy_window(evt->window);
			break;
		default:
			return -1;
	}

	++cnt;

	return 0;
}

int main(void)
{
	wh_window *window1 = wh_create_window(1280, 720, "XWINDOW1");
	if (!window1)
		return -1;

	wh_window *window2 = wh_create_window(800, 600, "XWINDOW2");
	if (!window2)
		return -1;


	struct wh_event evt = {0};
	uint8_t actwins = wh_get_nwin();

	while (actwins)
	{
		while (wh_poll_event(&evt))
			event_handler(&evt, &actwins);
	}

	wh_shutdown();

	return 0;
}
