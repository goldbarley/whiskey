#ifndef WHIS_VULKAN_H_
#define WHIS_VULKAN_H_ 1

#if defined(WHIS_PLATFORM_X11)
#define VK_USE_PLATFORM_XCB_KHR
#elif defined(WHIS_PLATFORM_WAYLAND)
#define VK_USE_PLATFORM_WAYLAND_KHR
#elif defined(WHIS_PLATFORM_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#endif /* WHIS_PLATFORM_* */

#include "attr.h"
#include "types.h"

#include <vulkan/vulkan.h>

WHIS_EXPORT
wh_fnresult wh_create_vulkan_surface(VkSurfaceKHR *surface, wh_window *window,
				     VkInstance instance,
				     PFN_vkCreateXcbSurfaceKHR fp,
				     VkAllocationCallbacks *allocator);

#endif /* WHIS_VULKAN_H_ */
