#ifndef WHISKEY_H_
#define WHISKEY_H_ 1

#include "attr.h"
#include "event.h"
#include "keymap.h"
#include "math.h"
#include "types.h"
#include "window.h"

#ifdef __linux__
#include "linux.h"
#if defined(WHIS_PLATFORM_X11)
#include "whx/util.h"
#elif defined(WHIS_PLATFORM_WAYLAND)
#endif /* WHIS_PLATFORM_* */
#endif /* __linux__ */

#endif /* WHISKEY_H_ */
