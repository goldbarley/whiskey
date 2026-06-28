#ifndef WHIS_X11_WINBUF_H
#define WHIS_X11_WINBUF_H

#include "types.h"
#include "whis/attr.h"

#define WHIS_MAX_X11_WINDOW_COUNT (64U)

WHIS_EXPORT
whx_window *whx_get_freeaddr(void);

WHIS_EXPORT
void whx_remove_window(whx_window *win);

#endif /* WHIS_X11_WINBUF_H */
